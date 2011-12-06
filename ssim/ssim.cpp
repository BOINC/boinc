// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2011 University of California
//
// BOINC is free software; you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation,
// either version 3 of the License, or (at your option) any later version.
//
// BOINC is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with BOINC.  If not, see <http://www.gnu.org/licenses/>.

// ssim - simulator for distributed storage
//
// Simulates the storage of files on a dynamic set of hosts.
//
// The model of the host population is:
// - hosts arrival is a Poisson process
// - host lifetime is exponentially distributed
// - the time needed to download n bytes of data to/from a host is
//   U()*n/DOWNLOAD_BYTES_SEC
//   where U() is a uniform random var
// TODO: add a factor corresponding to host availability

#define HOSTS_PER_DAY   10.
#define HOST_LIFE_MEAN  100.*86400
#define UPLOAD_BYTES_SEC  1e6
#define DOWNLOAD_BYTES_SEC  5e6

// We simulate policies based on coding and replication.
//
// Coding means that data is divided into M = N+K units,
// of which any N are sufficient to reconstruct the original data.
// 
// The units in an encoding can themselves be encoded.
// In general we model C levels of encoding.
//
// The bottom-level data units ("chunks") are stored on hosts,
// possibly with replication

#define ENCODING_N          4
#define ENCODING_K          2
#define ENCODING_M          6
#define ENCODING_LEVELS     1
#define REPLICATION_LEVEL   1

// When we need to reconstruct an encoded unit on the server,
// we try to upload N_UPLOAD subunits,
// where N <= N_UPLOAD <= M

#define N_UPLOAD            5

// Terminology:
//
// A chunk may or may not be "present_on_server".
// An encoded data unit is "present_on_server" if at least N
// of its subunits are present_on_server.

// A data unit is "recoverable" if it can be reconstruct on the server,
// based on current state.
// A chunk is "recoverable" if it is assigned at least 1 host.
// (if it is downloading, it's still present on the server)
// An encoded data unit is "recoverable" if at least N
// of its subunits are recoverable.

// A chunk is "uploading" if at least one of its instances
// is being uploaded to the server.
// An encoded data unit is "uploading" if at least
// 1 of its subunits is uploading,
// and at least N of its subunits are either present_on_server or uploading

// The scheduling policy can be briefly described as:
// 1) distribute chunks to hosts when possible, up to the replication level
//    Put at most 1 chunk of a file on a given host.
// 2) if a data unit becomes unrecoverable,
//    upload its parent unit, reconstruct the data, then do 1)

// Figures of merit
//
// for each file, we compute:
// - the average and peak server network rate, up and down
// - the average and peak disk usage
// - the average and min fault tolerance level
//   (i.e. number of simultaneous host failures needed to lose the file)
//
// These are measured starting from the time when the file's
// initial downloads have all succeeded or failed

#include <math.h>
#include <stdio.h>
#include <set>

#include "des.h"

using std::set;

#define K 15
    // this many packets per meta-packet
#define N 10
    // need this many to reconstruct the meta-packet
#define META_K   15
    // similar, meta-packets per file
#define META_N   10


SIMULATOR sim;

inline double drand() {
    return (double)rand()/(double)RAND_MAX;
}

double ran_exp(double mean) {
    return -log(drand())*mean;
}

char* time_str(double t) {
    static char buf[256];
    struct tm;
    int n = (int)t;
    int nsec = n % 60;
    n /= 60;
    int nmin = n % 60;
    n /= 60;
    int nhour = n % 24;
    n /= 24;
    sprintf(buf, "%4d days %02d:%02d:%02d", n, nhour, nmin, nsec);
    return buf;
}

char* now_str() {
    return time_str(sim.now);
}

struct CHUNK;
struct META_CHUNK;
struct DFILE;
struct HOST;
set<HOST*> hosts;

struct CHUNK_ON_HOST : public EVENT {
    HOST* host;
    CHUNK* chunk;
    char name[256];
    bool present_on_host;
    bool transfer_in_progress;  // upload if present_on_host, else download
    virtual void handle();
    inline bool download_in_progress() {
        return (transfer_in_progress && !present_on_host);
    }
};

static int next_host_id=0;
struct HOST : public EVENT {
    int id;
    set<CHUNK_ON_HOST*> chunks;
    virtual void handle();
    HOST() {
        t = sim.now + ran_exp(HOST_LIFE_MEAN);
        id = next_host_id++;
        hosts.insert(this);
    }
};

struct HOST_ARRIVAL : public EVENT {
    virtual void handle() {
        sim.insert(new HOST);
        t += ran_exp(86400./HOSTS_PER_DAY);
        sim.insert(this);
    }
};

struct REPORT_STATS : public EVENT {
    virtual void handle() {
        printf("%f: %lu hosts\n", t, hosts.size());
        t += 86400;
        sim.insert(this);
    }
};

void die(const char* msg) {
    printf("%s: %s\n", now_str(), msg);
    exit(1);
}

// base class for chunks and meta-chunks
//
struct DATA_UNIT {
    virtual bool recoverable(){die("recoverable undef"); return false;};
        // can be reconstructed w/o reconstructing parent,
        // assuming that current downloads succeed
    virtual void start_upload(){die("start_upload undef"); };
    virtual void assign(){die("assign undef"); };
    virtual bool is_present_on_server(){die("pos undef"); return false;};
    virtual void delete_chunks_from_server(){die("dcfs undef");};
    virtual void now_present(){die("now_present undef");};
    bool is_uploading;
    char name[64];
};

struct CHUNK : DATA_UNIT {
    set<CHUNK_ON_HOST*> hosts;
    META_CHUNK* parent;
    double size;
    bool _is_present_on_server;

    CHUNK(META_CHUNK* mc, double s, int index);

    virtual void assign();
    void host_failed(CHUNK_ON_HOST* p);
    void upload_complete();
    virtual bool recoverable() {
        return (!hosts.empty());
    }
    virtual bool is_present_on_server() {
        return _is_present_on_server;
    }
    virtual void start_upload() {
        // if no upload of this chunk is in progress, start one.
        // NOTE: all instances are inherently present_on_host
        //
        CHUNK_ON_HOST* c;
        set<CHUNK_ON_HOST*>::iterator i;
        for (i=hosts.begin(); i!=hosts.end(); i++) {
            c = *i;
            if (c->transfer_in_progress) return;
        }
        c = *(hosts.begin());
        c->transfer_in_progress = true;
        c->t = sim.now + (drand()+.5)*size/UPLOAD_BYTES_SEC;
        printf("%s: starting upload of %s\n", now_str(), c->name);
        sim.insert(c);
    }
    void download_complete();
    virtual void delete_chunks_from_server() {
        set<CHUNK_ON_HOST*>::iterator i;
        for (i=hosts.begin(); i!=hosts.end(); i++) {
            CHUNK_ON_HOST* c = *i;
            if (c->download_in_progress()) return;
        }
        printf("%s: deleting %s from server\n", now_str(), name);
        _is_present_on_server = false;
    }
    virtual void now_present() {
        _is_present_on_server = true;
    }
};

struct META_CHUNK : DATA_UNIT {
    vector<DATA_UNIT*> children;
    META_CHUNK* parent;
    int n_children_present;
    DFILE* dfile;
    bool uploading;

    META_CHUNK(
        DFILE* d, META_CHUNK* par, double size, int encoding_level, int index
    );

    virtual bool is_present_on_server() {
        int n=0;
        for (int i=0; i<ENCODING_M; i++) {
            if (children[i]->is_present_on_server()) {
                n++;
                if (n == ENCODING_N) return true;
            }
        }
        return false;
    }

    virtual void now_present() {
        for (int i=0; i<ENCODING_M; i++) {
            children[i]->now_present();
        }
    }

    int n_recoverable_children() {
        int n = 0;
        for (int i=0; i<ENCODING_M; i++) {
            if (children[i]->recoverable()) {
                n++;
            }
        }
        return n;
    }

    // a child has become unrecoverable.
    // reconstruct this data unit if we still can.
    //
    void child_unrecoverable() {
        int n = n_recoverable_children();
        printf("%s: a child of %s has become unrecoverable\n", now_str(), name);
        if (n >= ENCODING_N) {
            uploading = true;
            for (unsigned int i=0; i<children.size(); i++) {
                DATA_UNIT* c = children[i];
                if (c->recoverable()) {
                    c->start_upload();
                }
            }
        } else {
            printf("%s: only %d recoverable children\n", now_str(), n);
        }
    }

    // start download of descendant chunks as needed
    //
    virtual void assign() {
        for (unsigned int i=0; i<children.size(); i++) {
            children[i]->assign();
        }
    }

    virtual void delete_chunks_from_server() {
        for (unsigned int i=0; i<children.size(); i++) {
            children[i]->delete_chunks_from_server();
        }
    }

    // this is called only if we're uploading
    //
    void child_upload_complete() {
        printf("%s: child upload complete for %s\n", now_str(), name);
        int n = 0;
        for (unsigned int i=0; i<children.size(); i++) {
            DATA_UNIT* c = children[i];
            if (c->is_present_on_server()) {
                n++;
            }
        }
        if (n >= ENCODING_N) {
            now_present();
            assign();
            if (parent && parent->uploading) {
                parent->child_upload_complete();
            } else {
                // if we're not reconstructing parent,
                // delete any chunks not being downloaded
                //
                delete_chunks_from_server();
            }
        }
    }
};

static int next_file_id=0;

// keeps track of a time-varying property of a file
// (server disk usage, up/download rate, fault tolerance)
//
struct STATS_ITEM {
    double value;
    double integral;
    double max_val;
    double max_val_time;
    double prev_t;
    double start_time;
    bool first;

    STATS_ITEM() {
        value = 0;
        integral = 0;
        max_val = 0;
        max_val_time = 0;
        first = true;
    }

    void sample(double v, bool collecting_stats) {
        double old_val = value;
        value = v;
        if (!collecting_stats) return;
        if (first) {
            first = false;
            prev_t = sim.now;
            return;
        }
        double dt = sim.now - prev_t;
        prev_t = sim.now;
        integral += dt*old_val;
        if (v > max_val) {
            max_val = v;
            max_val_time = sim.now;
        }
    }

    void sample_inc(double inc, bool collecting_stats) {
        sample(value+inc, collecting_stats);
    }

    void print() {
        sample_inc(0, true);
        double dt = sim.now - start_time;
        printf("    mean: %f\n", integral/dt);
        printf("    max: %f\n", max_val);
        printf("    time of max: %s\n", time_str(max_val_time));
    }
};

struct DFILE : EVENT {
    META_CHUNK* meta_chunk;
    double size;
    int id;
    set<HOST*> unused_hosts;
        // hosts that don't have any packets of this file
    int pending_init_downloads;
        // # of initial downloads pending.
        // When this is zero, we start collecting stats for the file
    double accounting_start_time;
    STATS_ITEM disk_usage;
    STATS_ITEM upload_rate;
    STATS_ITEM download_rate;
    STATS_ITEM fault_tolerance;

    DFILE(double s) {
        id = next_file_id++;
        unused_hosts = hosts;
        size = s;
    }

    // the creation of a file
    //
    virtual void handle() {
        printf("creating file %d\n", id);
        meta_chunk = new META_CHUNK(this, NULL, size, ENCODING_LEVELS, id);
        meta_chunk->assign();
    }

    inline bool collecting_stats() {
        return (pending_init_downloads == 0);
    }

    void print_stats() {
        printf("Statistics for file %d\n", id);
        printf("  Server disk usage:\n");
        disk_usage.print();
        printf("  Upload rate:\n");
        upload_rate.print();
        printf("  Download rate:\n");
        download_rate.print();
        printf("  Fault tolerance level:\n");
        fault_tolerance.print();
    }
};

//////////////////// method defs ////////////////////

// transfer has finished
//
void CHUNK_ON_HOST::handle() {
    transfer_in_progress = false;
    if (present_on_host) {
        // it was an upload
        printf("%s: upload of %s completed\n", now_str(), name);
        chunk->upload_complete();    // create new replicas if needed
    } else {
        present_on_host = true;
        printf("%s: download of %s completed\n", now_str(), name);
        chunk->download_complete();
    }
}

// the host has failed
//
void HOST::handle() {
    set<HOST*>::iterator i = hosts.find(this);
    hosts.erase(i);

    printf("%s: host %d failed\n", now_str(), id);
    set<CHUNK_ON_HOST*>::iterator p;
    for (p = chunks.begin(); p != chunks.end(); p++) {
        CHUNK_ON_HOST* c = *p;
        c->chunk->host_failed(c);
        if (c->transfer_in_progress) {
            sim.remove(c);
        }
        delete c;
    }
}

CHUNK::CHUNK(META_CHUNK* mc, double s, int index) {
    parent = mc;
    _is_present_on_server = true;
    size = s;
    sprintf(name, "%s.%d", parent->name, index);
    parent->dfile->pending_init_downloads += REPLICATION_LEVEL;
    parent->dfile->disk_usage.sample_inc(size, false);
}

void CHUNK::host_failed(CHUNK_ON_HOST* p) {
    set<CHUNK_ON_HOST*>::iterator i = hosts.find(p);
    hosts.erase(i);
    printf("%s: handling loss of %s\n", now_str(), p->name);
    if (_is_present_on_server) {
        // if data is on server, make a new replica
        //
        assign();
    } else if (!hosts.empty()) {
        start_upload();
    } else {
        parent->child_unrecoverable();
    }
}

void CHUNK::upload_complete() {
    if (!_is_present_on_server) {
        _is_present_on_server = true;
        parent->dfile->disk_usage.sample_inc(
            size,
            parent->dfile->collecting_stats()
        );
    }
    assign();
    if (parent->uploading) {
        parent->child_upload_complete();
    }
}

void CHUNK::download_complete() {
    // we can remove chunk from server if enough replicas
    // have downloaded
    //
    int n=0;
    set<CHUNK_ON_HOST*>::iterator i;
    for (i=hosts.begin(); i!=hosts.end(); i++) {
        CHUNK_ON_HOST* c = *i;
        if (c->present_on_host) {
            n++;
        }
    }
    if (n >= REPLICATION_LEVEL) {
        printf("%s: %s replicated, removing from server\n", now_str(), name);
        _is_present_on_server = false;
        parent->dfile->disk_usage.sample_inc(
            -size,
            parent->dfile->collecting_stats()
        );
    }
    if (parent->dfile->pending_init_downloads) {
        parent->dfile->pending_init_downloads--;
    }
}

void CHUNK::assign() {
    if (!_is_present_on_server) return;
    while (hosts.size() < REPLICATION_LEVEL) {
        if (parent->dfile->unused_hosts.size() == 0) {
            die("no more hosts!\n");
        }
        set<HOST*>::iterator i = parent->dfile->unused_hosts.begin();
        HOST* h = *i;
        parent->dfile->unused_hosts.erase(i);
        CHUNK_ON_HOST *c = new CHUNK_ON_HOST();
        sprintf(c->name, "chunk %s on host %d", name, h->id);
        printf("%s: assigning chunk %s to host %d\n", now_str(), name, h->id);
        c->host = h;
        c->chunk = this;
        c->t = sim.now + (drand()+.5)*size/DOWNLOAD_BYTES_SEC;
        hosts.insert(c);
        h->chunks.insert(c);
        c->transfer_in_progress = true;
        sim.insert(c);
    }
}

META_CHUNK::META_CHUNK(
    DFILE* d, META_CHUNK* par, double size, int encoding_level, int index
) {
    dfile = d;
    parent = par;
    if (parent) {
        sprintf(name, "%s.%d", parent->name, index);
    } else {
        sprintf(name, "%d", index);
    }
    if (encoding_level) {
        for (int j=0; j<ENCODING_M; j++) {
            children.push_back(new META_CHUNK(
                d,
                this,
                size/ENCODING_N,
                encoding_level-1,
                j
            ));
        }
    } else {
        for (int j=0; j<ENCODING_M; j++) {
            children.push_back(new CHUNK(this, size/ENCODING_N, j));
        }
    }
}

set<DFILE*> dfiles;

int main() {
#if 0
    HOST_ARRIVAL *h = new HOST_ARRIVAL;
    h->t = 0;
    sim.insert(h);
    REPORT_STATS* r = new REPORT_STATS;
    r->t = 0;
    sim.insert(r);
#endif

    for (int i=0; i<100; i++) {
        sim.insert(new HOST);
    }
    DFILE* dfile = new DFILE(1e2);
    sim.insert(dfile);

    sim.simulate(200*86400);

    printf("%s: simulation finished\n", now_str());
    dfile->print_stats();
}
