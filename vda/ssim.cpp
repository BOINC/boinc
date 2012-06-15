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

#include <set>
#include <limits.h>

#include "des.h"
#include "stats.h"
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

char* now_str() {
    return time_str(sim.now);
}

void show_msg(char* msg) {
    printf("%s: %s", time_str(sim.now), msg);
}

struct CHUNK;
struct CHUNK_ON_HOST;
struct META_CHUNK;
struct SIM_HOST;
set<SIM_HOST*> hosts;

// Represents a host.
// The associated EVENT is the disappearance of the host
//
struct SIM_HOST : EVENT {
    int id;
    double transfer_rate;
    set<CHUNK_ON_HOST*> chunks;     // chunks present or downloading
    virtual void handle();
    SIM_HOST() {
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
struct HOST_ARRIVAL : EVENT {
    virtual void handle() {
        sim.insert(new SIM_HOST);
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
struct CHUNK_ON_HOST : VDA_CHUNK_HOST, EVENT {
    SIM_HOST* host;
    CHUNK* chunk;
    virtual void handle();
    void start_upload();
    void start_download();
    void remove();
};

// represents a file to be stored.
// The associated EVENT is the arrival of the file
//
struct SIM_FILE : VDA_FILE_AUX, EVENT {
    double size;
    int id;
#if 0
    set<SIM_HOST*> unused_hosts;
        // hosts that don't have any chunks of this file
#endif

    SIM_FILE(double s) {
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

    // the first event is the creation of a file;
    // the 2nd is its retrieval
    //
    virtual void handle() {
        if (meta_chunk) {
            printf("%s: Retrieving file\n", now_str());
            meta_chunk->data_needed = true;
        } else {
            meta_chunk = new META_CHUNK(this, NULL, size, 0, id);
#ifdef EVENT_DEBUG
            printf("created file %d: size %f GB encoded size %f GB\n",
                id, size/1e9, disk_usage.value/1e9
            );
            t = sim.now + 500.*86400;
            sim.insert(this);
#endif
        }
        meta_chunk->recovery_plan();
        meta_chunk->recovery_action(sim.now);
        if (meta_chunk->data_now_present) {
            printf("File is present on server\n");
        }
    }

    void recover() {
        meta_chunk->recovery_plan();
        meta_chunk->recovery_action(sim.now);
        fault_tolerance.sample(
            meta_chunk->min_failures-1, collecting_stats(), sim.now
        );
    }

    void print_stats(double now) {
        printf("Statistics for file %d\n", id);
        printf("  Server disk usage:\n");
        disk_usage.print(now);
        printf("  Upload rate:\n");
        upload_rate.print(now);
        printf("  Download rate:\n");
        download_rate.print(now);
        printf("  Fault tolerance level:\n");
        fault_tolerance.print(now);

        FILE* f = fopen("summary.txt", "w");
        fault_tolerance.print_summary(f, now);
        disk_usage.print_summary(f, now);
        upload_rate.print_summary(f, now);
        download_rate.print_summary(f, now);
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
                chunk->parent->dfile->collecting_stats(),
                sim.now
            );
        } else {
#ifdef EVENT_DEBUG
            printf("%s: starting download of %s\n", now_str(), name);
#endif
            chunk->parent->dfile->download_rate.sample_inc(
                host->transfer_rate,
                chunk->parent->dfile->collecting_stats(),
                sim.now
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
            chunk->parent->dfile->collecting_stats(),
            sim.now
        );
        chunk->upload_complete();
    } else {
        present_on_host = true;
#ifdef EVENT_DEBUG
        printf("%s: download of %s completed\n", now_str(), name);
#endif
        chunk->parent->dfile->download_rate.sample_inc(
            -host->transfer_rate,
            chunk->parent->dfile->collecting_stats(),
            sim.now
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
                    chunk->parent->dfile->collecting_stats(),
                    sim.now
                );
            } else {
                chunk->parent->dfile->download_rate.sample_inc(
                    -host->transfer_rate,
                    chunk->parent->dfile->collecting_stats(),
                    sim.now
                );
            }
        }
    }
}

// the host has failed
//
void SIM_HOST::handle() {
    set<SIM_HOST*>::iterator i = hosts.find(this);
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
    VDA_FILE_AUX* fp = parent->dfile;
    fp->pending_init_downloads += fp->policy.replication;
    fp->disk_usage.sample_inc(size, false, sim.now);
}

// if there aren't enough replicas of this chunk,
// pick new hosts and start downloads
//
int CHUNK::assign() {
    if (!present_on_server) return 0;
    VDA_FILE_AUX* fp = parent->dfile;
    while ((int)(hosts.size()) < fp->policy.replication) {
#if 0
        if (parent->dfile->unused_hosts.size() == 0) {
            die("no more hosts!\n");
        }
        set<SIM_HOST*>::iterator i = fp->unused_hosts.begin();
        SIM_HOST* h = *i;
        fp->unused_hosts.erase(i);
#else
        SIM_HOST* h = new SIM_HOST;
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
    return 0;
}

int CHUNK::start_upload() {
    // if no upload of this chunk is in progress, start one.
    // NOTE: all instances are inherently present_on_host,
    // since this is only called if chunk is not present on server
    //
    set<VDA_CHUNK_HOST*>::iterator i;
    for (i=hosts.begin(); i!=hosts.end(); i++) {
        CHUNK_ON_HOST* c = (CHUNK_ON_HOST*)*i;
        if (c->transfer_in_progress) return 0;
    }
    CHUNK_ON_HOST* c = (CHUNK_ON_HOST*)*(hosts.begin());
    c->start_upload();
    return 0;
}

void CHUNK::host_failed(VDA_CHUNK_HOST* p) {
    set<VDA_CHUNK_HOST*>::iterator i = hosts.find(p);
    hosts.erase(i);
#ifdef EVENT_DEBUG
    printf("%s: handling loss of %s\n", now_str(), p->name);
#endif
    SIM_FILE* sfp = (SIM_FILE*)parent->dfile;
    sfp->recover();
}

void CHUNK::upload_complete() {
    if (!present_on_server) {
        present_on_server = true;
        parent->dfile->disk_usage.sample_inc(
            size,
            parent->dfile->collecting_stats(),
            sim.now
        );
    }
    SIM_FILE* sfp = (SIM_FILE*)parent->dfile;
    sfp->recover();
}

void CHUNK::download_complete() {
    if (parent->dfile->pending_init_downloads) {
        parent->dfile->pending_init_downloads--;
    }
    SIM_FILE* sfp = (SIM_FILE*)parent->dfile;
    sfp->recover();
}

int META_CHUNK::encode() {
    printf("%s: encoding metachunk %s\n", now_str(), name);
    return 0;
}

int META_CHUNK::decode() {
    printf("%s: decoding metachunk %s\n", now_str(), name);
    return 0;
}

int DATA_UNIT::delete_file() {
    return 0;
}

set<SIM_FILE*> dfiles;

int main(int argc, char** argv) {
    POLICY policy;

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
        sim.insert(new SIM_HOST);
    }
#endif
    SIM_FILE* dfile = new SIM_FILE(params.file_size);
    dfile->policy = policy;
    sim.insert(dfile);

    sim.simulate(params.sim_duration);

    printf("%s: simulation finished\n", now_str());
    dfile->print_stats(sim.now);
}
