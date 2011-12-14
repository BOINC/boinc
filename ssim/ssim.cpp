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
#define UPLOAD_BYTES_SEC  (5./3600)
#define DOWNLOAD_BYTES_SEC  (5./3600)

// We simulate policies based on coding and replication.
//
// Coding means that data is divided into M = N+K units,
// of which any N are sufficient to reconstruct the original data.
// When we need to reconstruct an encoded unit on the server,
// we try to upload N_UPLOAD subunits,
// where N <= N_UPLOAD <= M

#define ENCODING_N          4
#define ENCODING_K          2
#define ENCODING_M          6
#define N_UPLOAD            5

// The units in an encoding can themselves be encoded.
// There are LEVELS levels of encoding.

#define ENCODING_LEVELS     1

// The bottom-level data units ("chunks") are stored on hosts,
// possibly with replication

#define REPLICATION_LEVEL   2

// Terminology:
//
// A data unit is "recoverable" if it can be recovered on the server
// by uploading data from hosts.
// A chunk is recoverable if it's present on the server or on at least 1 host.
// (note: if it's downloading, it's still present on the server)
// An encoded data unit is recoverable if at least N
// of its subunits are recoverable.

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

#define EVENT_DEBUG
#define SAMPLE_DEBUG
//#define RECOVERY_DEBUG

SIMULATOR sim;
int next_file_id=0;
int next_host_id=0;

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
struct CHUNK_ON_HOST;
struct META_CHUNK;
struct DFILE;
struct HOST;
set<HOST*> hosts;

// Represents a host.
// The associated EVENT is the disappearance of the host
//
struct HOST : public EVENT {
    int id;
    set<CHUNK_ON_HOST*> chunks;     // chunks present or downloading
    virtual void handle();
    HOST() {
        t = sim.now + ran_exp(HOST_LIFE_MEAN);
        id = next_host_id++;
        hosts.insert(this);
    }
};

// The host arrival process.
// The associated EVENT is the arrival of a host
//
struct HOST_ARRIVAL : public EVENT {
    virtual void handle() {
        sim.insert(new HOST);
        t += ran_exp(86400./HOSTS_PER_DAY);
        sim.insert(this);
    }
};

void die(const char* msg) {
    printf("%s: %s\n", now_str(), msg);
    exit(1);
}

// The status of a chunk on a particular host.
// The associated event is the completion of an upload or download
//
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

#define PRESENT 0
#define RECOVERABLE 1
#define UNRECOVERABLE 2

// base class for chunks and meta-chunks
//
struct DATA_UNIT {
    virtual void recovery_plan(){};
    virtual void recovery_action(){};
    int status;
    bool in_recovery_set;
    bool data_now_present;
    bool data_needed;
    double cost;
    char name[64];
};

struct CHUNK : DATA_UNIT {
    set<CHUNK_ON_HOST*> hosts;
    META_CHUNK* parent;
    double size;
    bool present_on_server;

    CHUNK(META_CHUNK* mc, double s, int index);

    void start_upload();
    void host_failed(CHUNK_ON_HOST* p);
    bool download_in_progress();
    void upload_complete();
    void download_complete();
    void assign();
    virtual void recovery_plan();
    virtual void recovery_action();
};

struct META_CHUNK : DATA_UNIT {
    vector<DATA_UNIT*> children;
    META_CHUNK* parent;
    int n_children_present;
    bool have_unrecoverable_children;
    DFILE* dfile;
    bool uploading;

    META_CHUNK(
        DFILE* d, META_CHUNK* par, double size, int encoding_level, int index
    );

    virtual void recovery_plan();
    virtual void recovery_action();
};

// keeps track of a time-varying property of a file
// (server disk usage, up/download rate, fault tolerance level)
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
#ifdef SAMPLE_DEBUG
        printf("%s: sample_inc: %f %f\n", now_str(), inc, value);
#endif
    }

    void print() {
        sample_inc(0, true);
        double dt = sim.now - start_time;
        printf("    mean: %f\n", integral/dt);
        printf("    max: %f\n", max_val);
        printf("    time of max: %s\n", time_str(max_val_time));
    }
};

// represents a file to be stored.
// The associated EVENT is the arrival of the file
//
struct DFILE : EVENT {
    META_CHUNK* meta_chunk;
    double size;
    int id;
    set<HOST*> unused_hosts;
        // hosts that don't have any chunks of this file
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
        meta_chunk = new META_CHUNK(this, NULL, size, ENCODING_LEVELS, id);
#ifdef EVENT_DEBUG
        printf("created file %d: size %f encoded size %f\n",
            id, size, disk_usage.value
        );
#endif
        meta_chunk->recovery_plan();
        meta_chunk->recovery_action();
    }

    void recover() {
        meta_chunk->recovery_plan();
        meta_chunk->recovery_action();
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
#ifdef EVENT_DEBUG
        printf("%s: upload of %s completed\n", now_str(), name);
#endif
        chunk->upload_complete();    // create new replicas if needed
    } else {
        present_on_host = true;
#ifdef EVENT_DEBUG
        printf("%s: download of %s completed\n", now_str(), name);
#endif
        chunk->download_complete();
    }
}

// the host has failed
//
void HOST::handle() {
    set<HOST*>::iterator i = hosts.find(this);
    hosts.erase(i);

#ifdef EVENT_DEBUG
    printf("%s: host %d failed\n", now_str(), id);
#endif
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
    present_on_server = true;
    size = s;
    sprintf(name, "%s.%d", parent->name, index);
    parent->dfile->pending_init_downloads += REPLICATION_LEVEL;
    parent->dfile->disk_usage.sample_inc(size, false);
}

// if there aren't enough replicas of this chunk,
// pick new hosts and start downloads
//
void CHUNK::assign() {
    if (!present_on_server) return;
    while (hosts.size() < REPLICATION_LEVEL) {
        if (parent->dfile->unused_hosts.size() == 0) {
            die("no more hosts!\n");
        }
        set<HOST*>::iterator i = parent->dfile->unused_hosts.begin();
        HOST* h = *i;
        parent->dfile->unused_hosts.erase(i);
        CHUNK_ON_HOST *c = new CHUNK_ON_HOST();
        sprintf(c->name, "chunk %s on host %d", name, h->id);
#ifdef EVENT_DEBUG
        printf("%s: assigning chunk %s to host %d\n", now_str(), name, h->id);
#endif
        c->host = h;
        c->chunk = this;
        c->t = sim.now + (drand()+.5)*size/DOWNLOAD_BYTES_SEC;
        hosts.insert(c);
        h->chunks.insert(c);
        c->transfer_in_progress = true;
        sim.insert(c);
    }
}

bool CHUNK::download_in_progress() {
    set<CHUNK_ON_HOST*>::iterator i;
    for (i=hosts.begin(); i!=hosts.end(); i++) {
        CHUNK_ON_HOST* c = *i;
        if (c->download_in_progress()) return true;
    }
    return false;
}

void CHUNK::start_upload() {
    // if no upload of this chunk is in progress, start one.
    // NOTE: all instances are inherently present_on_host,
    // since this is only called if chunk is not present on server
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
#ifdef EVENT_DEBUG
    printf("%s: starting upload of %s\n", now_str(), c->name);
#endif
    sim.insert(c);
}

void CHUNK::host_failed(CHUNK_ON_HOST* p) {
    set<CHUNK_ON_HOST*>::iterator i = hosts.find(p);
    hosts.erase(i);
#ifdef EVENT_DEBUG
    printf("%s: handling loss of %s\n", now_str(), p->name);
#endif
    parent->dfile->recover();
}

void CHUNK::upload_complete() {
    if (!present_on_server) {
        present_on_server = true;
        parent->dfile->disk_usage.sample_inc(
            size,
            parent->dfile->collecting_stats()
        );
    }
    parent->dfile->recover();
}

void CHUNK::download_complete() {
    if (parent->dfile->pending_init_downloads) {
        parent->dfile->pending_init_downloads--;
    }
    parent->dfile->recover();
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

// Recovery logic: decide what to do in response to
// host failures and upload/download completions.
//
// One way to do this would be to store a bunch of state info
// with each node in the file's tree,
// and do things by local tree traversal.
//
// However, it's a lot simpler (for me, the programmer)
// to store minimal state info,
// and to reconstruct state info using
// a top-down tree traversal in response to each event.
// Actually we do 2 traversals:
// 1) plan phase:
//      We see whether every node recoverable,
//      and if so compute its "recovery set":
//      the set of children from which it can be recovered
//      with minimal cost (i.e. network traffic).
//      Decide whether each chunk currently on the server needs to remain.
// 2) action phase
//      Based on the results of phase 1,
//      decide whether to start upload/download of chunks,
//      and whether to delete data currently on server
//
void META_CHUNK::recovery_plan() {
    vector<DATA_UNIT*> recoverable;
    vector<DATA_UNIT*> present;

    unsigned int i;
    have_unrecoverable_children = false;

    // make lists of children in various states
    //
    for (i=0; i<children.size(); i++) {
        DATA_UNIT* c = children[i];
        c->in_recovery_set = false;
        c->data_needed = false;
        c->data_now_present = false;
        c->recovery_plan();
        switch (c->status) {
        case PRESENT:
            present.push_back(c);
            break;
        case RECOVERABLE:
            recoverable.push_back(c);
            break;
        case UNRECOVERABLE:
            have_unrecoverable_children = true;
            break;
        }
    }

    // based on states of children, decide what state we're in
    //
    if (present.size() >= ENCODING_N) {
        status = PRESENT;
        sort(present.begin(), present.end());
        present.resize(ENCODING_N);
        cost = 0;
        for (i=0; i<present.size(); i++) {
            DATA_UNIT* c= present[i];
            cost += c->cost;
            c->in_recovery_set = true;
        }
    } else if (present.size() + recoverable.size() >= ENCODING_N) {
        status = RECOVERABLE;
        int j = ENCODING_N - present.size();
        sort(recoverable.begin(), recoverable.end());
        recoverable.resize(j);
        cost = 0;
        for (i=0; i<present.size(); i++) {
            DATA_UNIT* c= present[i];
            c->in_recovery_set = true;
        }
        for (i=0; i<recoverable.size(); i++) {
            DATA_UNIT* c= recoverable[i];
            c->in_recovery_set = true;
            cost += c->cost;
        }
    } else {
        status = UNRECOVERABLE;
    }
}

const char* status_str(int status) {
    switch (status) {
    case PRESENT: return "present";
    case RECOVERABLE: return "recoverable";
    case UNRECOVERABLE: return "unrecoverable";
    }
    return "unknown";
}

void CHUNK::recovery_plan() {
    if (present_on_server) {
        status = PRESENT;
        cost = 0;
    } else if (hosts.size() > 0) {
        status = RECOVERABLE;
        cost = size;
        if (hosts.size() < REPLICATION_LEVEL) {
            data_needed = true;
        }
    } else {
        status = UNRECOVERABLE;
    }
#ifdef DEBUG_RECOVERY
    printf("chunk plan %s: status %s\n", name, status_str(status));
#endif
}

void META_CHUNK::recovery_action() {
    if (data_now_present) {
        status = PRESENT;
    }
#ifdef DEBUG_RECOVERY
    printf("meta chunk action %s state %s unrec children %d\n",
        name, status_str(status), have_unrecoverable_children
    );
#endif
    for (unsigned i=0; i<children.size(); i++) {
        DATA_UNIT* c = children[i];
#ifdef DEBUG_RECOVERY
        printf("  child %s status %s in rec set %d\n",
            c->name, status_str(c->status), c->in_recovery_set
        );
#endif
        switch (status) {
        case PRESENT:
            if (c->status == UNRECOVERABLE) {
                c->data_now_present = true;
            }
            break;
        case RECOVERABLE:
            if (c->in_recovery_set && have_unrecoverable_children) {
                c->data_needed = true;
            }
            break;
        case UNRECOVERABLE:
            break;
        }
        c->recovery_action();
    }
}

void CHUNK::recovery_action() {
    if (data_now_present) {
        present_on_server = true;
        parent->dfile->disk_usage.sample_inc(
            size,
            parent->dfile->collecting_stats()
        );
        status = PRESENT;
    }
    if (status == PRESENT && hosts.size() < REPLICATION_LEVEL) {
        assign();
    }
    if (download_in_progress()) {
        data_needed = true;
    }
#ifdef DEBUG_RECOVERY
    printf("chunk action: %s data_needed %d present_on_server %d\n",
        name, data_needed, present_on_server
    );
#endif
    if (data_needed) {
        if (!present_on_server) {
            start_upload();
        }
    } else {
        if (present_on_server) {
            present_on_server = false;
#ifdef EVENT_DEBUG
            printf("%s: %s replicated, removing from server\n", now_str(), name);
#endif
            parent->dfile->disk_usage.sample_inc(
                -size,
                parent->dfile->collecting_stats()
            );
        }
    }
}


set<DFILE*> dfiles;

int main() {
#if 0
    HOST_ARRIVAL *h = new HOST_ARRIVAL;
    h->t = 0;
    sim.insert(h);
#endif

    for (int i=0; i<500; i++) {
        sim.insert(new HOST);
    }
    DFILE* dfile = new DFILE(1e2);
    sim.insert(dfile);

    sim.simulate(200*86400);

    printf("%s: simulation finished\n", now_str());
    dfile->print_stats();
}
