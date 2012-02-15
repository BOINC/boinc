// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2012 University of California
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

// classes for volunteer data archival (VDA)
//
// Note: these classes are used by both the simulator (ssim.cpp)
// and the VDA daemon (vdad.cpp)

#include <set>
#include <vector>

#include "boinc_db.h"

// parameters of 1 level of coding
//
struct CODING {
    int n;
    int k;
    int m;  // n + k
    int n_upload;
};

// description of overall coding
// (possibly w/ multiple coding levels and replication)
//
struct POLICY {
    int replication;
    int coding_levels;
    CODING codings[10];
    double chunk_sizes[10];

    char description[256];  // derived from the above

    int parse(const char*);
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

    void init(const char* n, const char* filename, STATS_KIND k);
    void sample(double v, bool collecting_stats, double now);
    void sample_inc(double inc, bool collecting_stats, double now);
    void print(double now);
    void print_summary(FILE* f, double now);
};

struct META_CHUNK;

struct VDA_FILE_AUX : VDA_FILE {
    POLICY policy;
    META_CHUNK* meta_chunk;

    // the following stuff is for the simulator
    double accounting_start_time;
    STATS_ITEM disk_usage;
    STATS_ITEM upload_rate;
    STATS_ITEM download_rate;
    STATS_ITEM fault_tolerance;

    VDA_FILE_AUX(){}
    VDA_FILE_AUX(DB_VDA_FILE f) :VDA_FILE(f){}
    int pending_init_downloads;
        // # of initial downloads pending.
        // When this is zero, we start collecting stats for the file

    inline bool collecting_stats() {
        return (pending_init_downloads == 0);
    }

    // the following for vdad
    //
    int init();
    int get_state();
};

#define PRESENT 0
#define RECOVERABLE 1
#define UNRECOVERABLE 2

// base class for chunks and meta-chunks
//
struct DATA_UNIT {
    virtual void recovery_plan(){};
    virtual void recovery_action(double){};
    int status;
    bool in_recovery_set;
    bool data_now_present;
    bool data_needed;
    double cost;
    int min_failures;
        // min # of host failures that would make this unrecoverable
    char name[64];
};

struct META_CHUNK : DATA_UNIT {
    std::vector<DATA_UNIT*> children;
    META_CHUNK* parent;
    int n_children_present;
    bool have_unrecoverable_children;
    VDA_FILE_AUX* dfile;
    bool uploading;
    CODING coding;

    // used by ssim
    META_CHUNK(
        VDA_FILE_AUX* d, META_CHUNK* par, double size,
        int coding_level, int index
    );

    // used by vdad
    META_CHUNK(){}
    int init(const char* dir, const char* fname, POLICY&, int level);
    int get_state(const char* dir, const char* fname, POLICY&, int level);

    virtual void recovery_plan();
    virtual void recovery_action(double);
};

struct CHUNK : DATA_UNIT {
    std::set<VDA_CHUNK_HOST*> hosts;
    META_CHUNK* parent;
    double size;
    bool present_on_server;

    CHUNK(META_CHUNK* mc, double s, int index);

    void start_upload();
    void host_failed(VDA_CHUNK_HOST* p);
    bool download_in_progress();
    void upload_complete();
    void download_complete();
    void assign();
    virtual void recovery_plan();
    virtual void recovery_action(double);
};

extern char* time_str(double);
