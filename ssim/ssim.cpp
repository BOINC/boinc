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
// - the time needed to upload or download n bytes of data
//   to/from a host is
//   MTD*U() + n/XFER_BYTES_SEC
//   where C1 and C2 are parameters and U() is a uniform random var

#define HOSTS_PER_DAY   10.
#define HOST_LIFE_MEAN  100.*86400
#define MAX_TRANSFER_DELAY  86400
#define UPLOAD_BYTES_SEC  1e6
#define DOWNLOAD_BYTES_SEC  5e6

// We simulate policies based on coding and replication.
//
// Coding means that data is divided into M = N+K units,
// of which any N are sufficient to reconstruct the data.
// 
// The units in an encoding can themselves be encoded.
// In general we model C levels of encoding.
//
// The bottom-level data units ("chunks") are stored on hosts,
// with R-fold replication

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
// of its subunits are present_on_server (recursive definition).

// A data unit is "recoverable" if it can be reconstruct on the server,
// based on current state.
// A chunk is "recoverable" if it is assigned at least 1 host.
//   (if it is downloading, it's still present on the server)
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

// place-holder
double ran_exp(double mean) {
    // gsl_ran_exponential(mean);
    return (drand() + .5)*mean;
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
    printf("%.0f: %s\n", sim.now, msg);
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
    bool present_on_server;
    bool is_uploading;
    char name[64];
};

struct CHUNK : DATA_UNIT {
    set<CHUNK_ON_HOST*> hosts;
    META_CHUNK* parent;
    double size;

    CHUNK(META_CHUNK* mc, double s, int index);

    virtual void assign();
    void host_failed(CHUNK_ON_HOST* p);
    void upload_complete();
    virtual bool recoverable() {
        return (!hosts.empty());
    }
    virtual void start_upload() {
        // if there's another replica, start upload of 1st instance
        // NOTE: all instances are inherently present_on_host
        //
        CHUNK_ON_HOST *c = *(hosts.begin());
        c->transfer_in_progress = true;
        c->t = sim.now + (drand()+.5)*size/UPLOAD_BYTES_SEC;
        printf("%.0f: starting upload of %s\n", sim.now, c->name);
        sim.insert(c);
    }
    void download_complete() {
        // see if we can remove chunk from server
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
            printf("%.0f: %s replicated, removing from server\n", sim.now, name);
            present_on_server = false;
        }

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
    int n_recoverable_children() {
        int n = 0;
        for (int i=0; i<ENCODING_N; i++) {
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
        printf("%.0f: a child of %s has become unrecoverable\n", sim.now, name);
        if (n >= ENCODING_N) {
            uploading = true;
            for (unsigned int i=0; i<children.size(); i++) {
                DATA_UNIT* c = children[i];
                if (c->recoverable()) {
                    c->start_upload();
                }
            }
        } else {
            printf("%.0f: only %d recoverable children\n", sim.now, n);
        }
    }

    virtual void assign() {
        for (unsigned int i=0; i<children.size(); i++) {
            children[i]->assign();
        }
    }

    virtual void cleanup() {
        for (unsigned int i=0; i<children.size(); i++) {
            children[i]->cleanup();
        }
    }

    // this is called only if we're uploading
    //
    void child_upload_complete() {
        int n = 0;
        for (unsigned int i=0; i<children.size(); i++) {
            DATA_UNIT* c = children[i];
            if (c->present_on_server) {
                n++;
            }
        }
        if (n >= ENCODING_N) {
            present_on_server = true;
        }
        assign();
        if (parent && parent->uploading) {
            parent->child_upload_complete();
        } else {
            // if we're not reconstructing parent,
            // delete any chunks not being downloaded
            //
            cleanup();
        }
    }
};

static int next_file_id=0;

struct DFILE : EVENT {
    META_CHUNK* meta_chunk;
    double size;
    int id;
    set<HOST*> unused_hosts;
        // hosts that don't have any packets of this file

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
};

//////////////////// method defs ////////////////////

// transfer has finished
//
void CHUNK_ON_HOST::handle() {
    transfer_in_progress = false;
    if (present_on_host) {
        // it was an upload
        chunk->upload_complete();    // create new replicas if needed
        printf("%.f: upload of %s completed\n", sim.now, name);
    } else {
        present_on_host = true;
        printf("%.f: download of %s completed\n", sim.now, name);
        chunk->download_complete();
    }
}

// the host has departed
//
void HOST::handle() {
    set<HOST*>::iterator i = hosts.find(this);
    hosts.erase(i);

    printf("%.0f: host %d failed\n", sim.now, id);
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
}

void CHUNK::host_failed(CHUNK_ON_HOST* p) {
    set<CHUNK_ON_HOST*>::iterator i = hosts.find(p);
    hosts.erase(i);
    printf("%.0f: handling loss of %s\n", sim.now, p->name);
    if (present_on_server) {
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
    present_on_server = true;
    assign();
    if (parent->uploading) {
        parent->child_upload_complete();
    }
}

void CHUNK::assign() {
    while (hosts.size() < REPLICATION_LEVEL) {
        if (parent->dfile->unused_hosts.size() == 0) {
            die("no more hosts!\n");
        }
        set<HOST*>::iterator i = parent->dfile->unused_hosts.begin();
        HOST* h = *i;
        parent->dfile->unused_hosts.erase(i);
        CHUNK_ON_HOST *c = new CHUNK_ON_HOST();
        sprintf(c->name, "chunk %s on host %d", name, h->id);
        printf("%.0f: assigning chunk %s to host %d\n", sim.now, name, h->id);
        c->host = h;
        c->chunk = this;
        c->t = sim.now + (drand()+.5)*size/DOWNLOAD_BYTES_SEC;
        hosts.insert(c);
        h->chunks.insert(c);
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
    sim.insert(new DFILE(1e12));

    sim.simulate(200*86400);
}
