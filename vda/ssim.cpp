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

// usage: ssim
//  [--policy filename]
//  [--host_life_mean x]
//  [--connect_interval x]
//  [--mean_xfer_rate x]
//  [--file_size x]
//
// outputs:
//   stdout: log info
//   summary.txt: format
//      fault tolerance min
//      disk_usage mean
//      upload_mean
//      download_mean

#include <math.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <set>

#include "des.h"
#include "vda_lib.h"

using std::set;

// We simulate policies based on coding and replication.
//
// Coding means that data is divided into M = N+K units,
// of which any N are sufficient to reconstruct the original data.
// When we need to reconstruct an encoded unit on the server,
// we try to upload N_UPLOAD subunits,
// where N <= N_UPLOAD <= M
// The units in an encoding can themselves be encoded.
//
// The bottom-level data units ("chunks") are stored on hosts,
// possibly with replication

struct PARAMS {
    // The model of the host population is:
    // - the population is unbounded
    // - host lifetime is exponentially distributed
    // - the time needed to transfer n bytes of data to/from a host is
    //   U1*connect_interval + (U2+.5)*n/mean_xfer_rate;
    //   where U1 and U2 are uniform random vars
    //   (U1 is per-transfer, U2 is per-host)
    //
    double host_life_mean;
    double connect_interval;
    double mean_xfer_rate;
    double file_size;
    double sim_duration;

    PARAMS() {
        // default parameters
        //
        host_life_mean = 100.*86400;
        connect_interval = 86400.;
        mean_xfer_rate = .2e6;
        file_size = 1e12;
        sim_duration = 1000.*86400;
    }
} params;

POLICY policy;

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
    double transfer_rate;
    set<CHUNK_ON_HOST*> chunks;     // chunks present or downloading
    virtual void handle();
    HOST() {
        t = sim.now + ran_exp(params.host_life_mean);
        id = next_host_id++;
        transfer_rate = params.mean_xfer_rate*(drand() + .5);
        hosts.insert(this);
    }
};

#if 0
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
#endif

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
    bool transfer_wait;         // waiting to start transfer
    bool transfer_in_progress;  // upload if present_on_host, else download
    virtual void handle();
    void start_upload();
    void start_download();
    inline bool download_in_progress() {
        return (transfer_in_progress && !present_on_host);
    }
    void remove();
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
    int min_failures;
        // min # of host failures that would make this unrecoverable
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
    CODING coding;

    META_CHUNK(
        DFILE* d, META_CHUNK* par, double size, int coding_level, int index
    );

    virtual void recovery_plan();
    virtual void recovery_action();
};

// keeps track of a time-varying property of a file
// (server disk usage, up/download rate, fault tolerance level)
//

typedef enum {DISK, NETWORK, FAULT_TOLERANCE} STATS_KIND;

struct STATS_ITEM {
    STATS_KIND kind;
    double value;
    double integral;
    double extreme_val;
    double extreme_val_time;
    double prev_t;
    double start_time;
    bool first;
    char name[256];
    FILE* f;

    void init(const char* n, const char* filename, STATS_KIND k) {
        f = fopen(filename, "w");
        strcpy(name, n);
        kind = k;
        value = 0;
        integral = 0;
        switch (kind) {
        case DISK:
        case NETWORK:
            extreme_val = 0;
            break;
        case FAULT_TOLERANCE:
            extreme_val = INT_MAX;
            break;
        }
        extreme_val_time = 0;
        first = true;
    }

    void sample(double v, bool collecting_stats) {
#ifdef SAMPLE_DEBUG
        switch (kind) {
        case DISK:
            printf("%s: %s: %fGB -> %fGB\n", now_str(), name, value/1e9, v/1e9);
            break;
        case NETWORK:
            printf("%s: %s: %fMbps -> %fMbps\n", now_str(), name, value/1e6, v/1e6);
            break;
        case FAULT_TOLERANCE:
            printf("%s: %s: %.0f -> %.0f\n", now_str(), name, value, v);
            break;
        }
#endif
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
        switch (kind) {
        case DISK:
        case NETWORK:
            if (v > extreme_val) {
                extreme_val = v;
                extreme_val_time = sim.now;
            }
            break;
        case FAULT_TOLERANCE:
            if (v < extreme_val) {
                extreme_val = v;
                extreme_val_time = sim.now;
            }
            break;
        }

        fprintf(f, "%f %f\n", sim.now, old_val);
        fprintf(f, "%f %f\n", sim.now, v);
    }

    void sample_inc(double inc, bool collecting_stats) {
        sample(value+inc, collecting_stats);
    }

    void print() {
        sample_inc(0, true);
        double dt = sim.now - start_time;
        switch (kind) {
        case DISK:
            printf("    mean: %fGB.  Max: %fGB at %s\n",
                (integral/dt)/1e9, extreme_val/1e9, time_str(extreme_val_time)
            );
            break;
        case NETWORK:
            printf("    mean: %fMbps.  Max: %fMbps at %s\n",
                (integral/dt)/1e6, extreme_val/1e6, time_str(extreme_val_time)
            );
            break;
        case FAULT_TOLERANCE:
            printf("    mean: %.2f.  Min: %.0f at %s\n",
                integral/dt, extreme_val, time_str(extreme_val_time)
            );
            break;
        }
    }
    void print_summary(FILE* f) {
        double dt = sim.now - start_time;
        switch (kind) {
        case DISK:
            fprintf(f, "%f\n", integral/dt);
            break;
        case NETWORK:
            fprintf(f, "%f\n", integral/dt);
            break;
        case FAULT_TOLERANCE:
            fprintf(f, "%f\n", extreme_val);
            break;
        }
    }
};

// represents a file to be stored.
// The associated EVENT is the arrival of the file
//
struct DFILE : EVENT {
    META_CHUNK* meta_chunk;
    double size;
    int id;
#if 0
    set<HOST*> unused_hosts;
        // hosts that don't have any chunks of this file
#endif
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
#if 0
        unused_hosts = hosts;
#endif
        size = s;
        disk_usage.init("Disk usage", "disk.dat", DISK);
        upload_rate.init("Upload rate", "upload.dat", NETWORK);
        download_rate.init("Download rate", "download.dat", NETWORK);
        fault_tolerance.init("Fault tolerance", "fault_tol.dat", FAULT_TOLERANCE);
    }

    // the creation of a file
    //
    virtual void handle() {
        meta_chunk = new META_CHUNK(this, NULL, size, 0, id);
#ifdef EVENT_DEBUG
        printf("created file %d: size %f encoded size %f\n",
            id, size, disk_usage.value
        );
#endif
        meta_chunk->recovery_plan();
        meta_chunk->recovery_action();
    }

    inline bool collecting_stats() {
        return (pending_init_downloads == 0);
    }

    void recover() {
        meta_chunk->recovery_plan();
        meta_chunk->recovery_action();
        fault_tolerance.sample(meta_chunk->min_failures-1, collecting_stats());
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

        FILE* f = fopen("summary.txt", "w");
        fault_tolerance.print_summary(f);
        disk_usage.print_summary(f);
        upload_rate.print_summary(f);
        download_rate.print_summary(f);
        fclose(f);
    }
};

//////////////////// method defs ////////////////////

void CHUNK_ON_HOST::start_upload() {
    transfer_in_progress = true;
    transfer_wait = true;
    t = sim.now + drand()*params.connect_interval;
#ifdef EVENT_DEBUG
    printf("%s: waiting to start upload of %s\n", now_str(), name);
#endif
    sim.insert(this);
}

void CHUNK_ON_HOST::start_download() {
    transfer_in_progress = true;
    transfer_wait = true;
    t = sim.now + drand()*params.connect_interval;
#ifdef EVENT_DEBUG
    printf("%s: waiting to start download of %s\n", now_str(), name);
#endif
    sim.insert(this);
}


// transfer or transfer wait has finished
//
void CHUNK_ON_HOST::handle() {
    if (transfer_wait) {
        transfer_wait = false;
        if (present_on_host) {
#ifdef EVENT_DEBUG
            printf("%s: starting upload of %s\n", now_str(), name);
#endif
            chunk->parent->dfile->upload_rate.sample_inc(
                host->transfer_rate,
                chunk->parent->dfile->collecting_stats()
            );
        } else {
#ifdef EVENT_DEBUG
            printf("%s: starting download of %s\n", now_str(), name);
#endif
            chunk->parent->dfile->download_rate.sample_inc(
                host->transfer_rate,
                chunk->parent->dfile->collecting_stats()
            );
        }
        t = sim.now + chunk->size/host->transfer_rate;
        sim.insert(this);
        return;
    }
    transfer_in_progress = false;
    if (present_on_host) {
        // it was an upload
#ifdef EVENT_DEBUG
        printf("%s: upload of %s completed\n", now_str(), name);
#endif
        chunk->parent->dfile->upload_rate.sample_inc(
            -host->transfer_rate,
            chunk->parent->dfile->collecting_stats()
        );
        chunk->upload_complete();
    } else {
        present_on_host = true;
#ifdef EVENT_DEBUG
        printf("%s: download of %s completed\n", now_str(), name);
#endif
        chunk->parent->dfile->download_rate.sample_inc(
            -host->transfer_rate,
            chunk->parent->dfile->collecting_stats()
        );
        chunk->download_complete();
    }
}

void CHUNK_ON_HOST::remove() {
    if (transfer_in_progress) {
        sim.remove(this);
        if (!transfer_wait) {
            if (present_on_host) {
                chunk->parent->dfile->upload_rate.sample_inc(
                    -host->transfer_rate,
                    chunk->parent->dfile->collecting_stats()
                );
            } else {
                chunk->parent->dfile->download_rate.sample_inc(
                    -host->transfer_rate,
                    chunk->parent->dfile->collecting_stats()
                );
            }
        }
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
        c->remove();
        delete c;
    }
}

CHUNK::CHUNK(META_CHUNK* mc, double s, int index) {
    parent = mc;
    present_on_server = true;
    size = s;
    sprintf(name, "%s.%d", parent->name, index);
    parent->dfile->pending_init_downloads += policy.replication;
    parent->dfile->disk_usage.sample_inc(size, false);
}

// if there aren't enough replicas of this chunk,
// pick new hosts and start downloads
//
void CHUNK::assign() {
    if (!present_on_server) return;
    while ((int)(hosts.size()) < policy.replication) {
#if 0
        if (parent->dfile->unused_hosts.size() == 0) {
            die("no more hosts!\n");
        }
        set<HOST*>::iterator i = parent->dfile->unused_hosts.begin();
        HOST* h = *i;
        parent->dfile->unused_hosts.erase(i);
#else
        HOST* h = new HOST;
        sim.insert(h);
#endif
        CHUNK_ON_HOST *c = new CHUNK_ON_HOST();
        sprintf(c->name, "chunk %s on host %d", name, h->id);
#ifdef EVENT_DEBUG
        printf("%s: assigning chunk %s to host %d\n", now_str(), name, h->id);
#endif
        c->host = h;
        c->chunk = this;
        h->chunks.insert(c);
        hosts.insert(c);
        c->start_download();
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
    c->start_upload();
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
    DFILE* d, META_CHUNK* par, double size, int coding_level, int index
) {
    dfile = d;
    parent = par;
    coding = policy.codings[coding_level];
    if (parent) {
        sprintf(name, "%s.%d", parent->name, index);
    } else {
        sprintf(name, "%d", index);
    }
    if (coding_level<policy.coding_levels-1) {
        for (int j=0; j<coding.m; j++) {
            children.push_back(new META_CHUNK(
                d,
                this,
                size/coding.n,
                coding_level+1,
                j
            ));
        }
    } else {
        for (int j=0; j<coding.m; j++) {
            children.push_back(
                new CHUNK(this, size/coding.n, j)
            );
        }
    }
}

// sort by increasing cost
//
bool compare_cost(const DATA_UNIT* d1, const DATA_UNIT* d2) {
    return d1->cost < d2->cost;
}

// sort by increase min_failures
//
bool compare_min_failures(const DATA_UNIT* d1, const DATA_UNIT* d2) {
    return d1->min_failures < d2->min_failures;
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
    if ((int)(present.size()) >= coding.n) {
        status = PRESENT;
        sort(present.begin(), present.end(), compare_cost);
        present.resize(coding.n);
        cost = 0;
        for (i=0; i<present.size(); i++) {
            DATA_UNIT* c= present[i];
            cost += c->cost;
            c->in_recovery_set = true;
        }
    } else if ((int)(present.size() + recoverable.size()) >= coding.n) {
        status = RECOVERABLE;
        unsigned int j = coding.n - present.size();
        sort(recoverable.begin(), recoverable.end(), compare_cost);
        cost = 0;
        for (i=0; i<present.size(); i++) {
            DATA_UNIT* c= present[i];
            c->in_recovery_set = true;
        }
        for (i=0; i<j; i++) {
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
        min_failures = INT_MAX;
    } else if (hosts.size() > 0) {
        status = RECOVERABLE;
        cost = size;
        if ((int)(hosts.size()) < policy.replication) {
            data_needed = true;
        }
        min_failures = hosts.size();
    } else {
        status = UNRECOVERABLE;
        min_failures = 0;
    }
#ifdef DEBUG_RECOVERY
    printf("chunk plan %s: status %s\n", name, status_str(status));
#endif
}

void META_CHUNK::recovery_action() {
    unsigned int i;
    if (data_now_present) {
        status = PRESENT;
    }
#ifdef DEBUG_RECOVERY
    printf("meta chunk action %s state %s unrec children %d\n",
        name, status_str(status), have_unrecoverable_children
    );
#endif
    for (i=0; i<children.size(); i++) {
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

    // because of recovery action, some of our children may have changed
    // status and fault tolerance, source may have changed too.
    // Recompute them.
    //
    vector<DATA_UNIT*> recoverable;
    vector<DATA_UNIT*> present;
    for (i=0; i<children.size(); i++) {
        DATA_UNIT* c = children[i];
        switch (c->status) {
        case PRESENT:
            present.push_back(c);
            break;
        case RECOVERABLE:
            recoverable.push_back(c);
            break;
        }
    }
    if ((int)(present.size()) >= coding.n) {
        status = PRESENT;
        min_failures = INT_MAX;
    } else if ((int)(present.size() + recoverable.size()) >= coding.n) {
        status = RECOVERABLE;

        // our min_failures is the least X such that some X host failures
        // would make this node unrecoverable
        //
        sort(recoverable.begin(), recoverable.end(), compare_min_failures);
        min_failures = 0;
        unsigned int k = coding.n - present.size();
            // we'd need to recover K recoverable children
        unsigned int j = recoverable.size() - k + 1;
            // a loss of J recoverable children would make this impossible

        // the loss of J recoverable children would make us unrecoverable
        // Sum the min_failures of the J children with smallest min_failures
        //
        for (i=0; i<j; i++) {
            DATA_UNIT* c = recoverable[i];
            printf("  Min failures of %s: %d\n", c->name, c->min_failures);
            min_failures += c->min_failures;
        }
        printf("  our min failures: %d\n", min_failures);
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
    if (status == PRESENT && (int)(hosts.size()) < policy.replication) {
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
            status = RECOVERABLE;
            min_failures = policy.replication;
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

int main(int argc, char** argv) {

    // default policy
    //
    policy.replication = 2;
    policy.coding_levels = 1;
    policy.codings[0].n = 10;
    policy.codings[0].k = 6;
    policy.codings[0].m = 16;
    policy.codings[0].n_upload = 12;

    for (int i=1; i<argc; i++) {
        if (!strcmp(argv[i], "--policy")) {
            int retval = policy.parse(argv[++i]);
            if (retval) exit(1);
        } else if (!strcmp(argv[i], "--host_life_mean")) {
            params.host_life_mean = atof(argv[++i]);
        } else if (!strcmp(argv[i], "--connect_interval")) {
            params.connect_interval = atof(argv[++i]);
        } else if (!strcmp(argv[i], "--mean_xfer_rate")) {
            params.mean_xfer_rate = atof(argv[++i]);
        } else if (!strcmp(argv[i], "--file_size")) {
            params.file_size = atof(argv[++i]);
        } else {
            fprintf(stderr, "bad arg %s\n", argv[i]);
            exit(1);
        }
    }
#if 0
    HOST_ARRIVAL *h = new HOST_ARRIVAL;
    h->t = 0;
    sim.insert(h);
#endif

#if 0
    for (int i=0; i<500; i++) {
        sim.insert(new HOST);
    }
#endif
    DFILE* dfile = new DFILE(params.file_size);
    sim.insert(dfile);

    sim.simulate(params.sim_duration);

    printf("%s: simulation finished\n", now_str());
    dfile->print_stats();
}
