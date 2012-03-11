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

#include "stats.h"
#include "vda_policy.h"

// a host with rpc_time < now-HOST_TIMEOUT is considered dead.
// Make sure you set next_rpc_delay accordingly (e.g., to 86400)
//
#define VDA_HOST_TIMEOUT (86400*4)

struct META_CHUNK;

struct VDA_FILE_AUX : VDA_FILE {
    POLICY policy;
    META_CHUNK* meta_chunk;

    VDA_FILE_AUX(){
        meta_chunk = NULL;
    }
    VDA_FILE_AUX(DB_VDA_FILE f) : VDA_FILE(f){}

    // the following stuff is for the simulator
    //
    double accounting_start_time;
    STATS_ITEM disk_usage;
    STATS_ITEM upload_rate;
    STATS_ITEM download_rate;
    STATS_ITEM fault_tolerance;

    int pending_init_downloads;
        // # of initial downloads pending.
        // When this is zero, we start collecting stats for the file

    inline bool collecting_stats() {
        return (pending_init_downloads == 0);
    }

    // the following for vdad
    //
    std::vector<int> available_hosts;
        // list of IDs of hosts with no chunks of this file
    int init();
    int get_state();
    int choose_host();
};

#define PRESENT 0
#define RECOVERABLE 1
#define UNRECOVERABLE 2

// base class for chunks and meta-chunks
//
struct DATA_UNIT {
    virtual int recovery_plan(){return 0;};
    virtual int recovery_action(double){return 0;};
    int status;
    bool in_recovery_set;
    bool data_now_present;
    bool data_needed;
    double cost;
    int min_failures;
        // min # of host failures that would make this unrecoverable
    char name[64];
    char dir[1024];
    bool keep_present;
    bool need_present;

    int delete_file();
};

struct META_CHUNK : DATA_UNIT {
    std::vector<DATA_UNIT*> children;
    META_CHUNK* parent;
    int n_children_present;
    bool have_unrecoverable_children;
    VDA_FILE_AUX* dfile;
    bool uploading;
    CODING coding;
    bool bottom_level;
    bool need_reconstruct;
    bool needed_by_parent;
    double child_size;

    // used by ssim
    META_CHUNK(
        VDA_FILE_AUX* d, META_CHUNK* par, double size,
        int coding_level, int index
    );

    // used by vdad
    META_CHUNK(VDA_FILE_AUX* d, META_CHUNK* p, int index);
    int init(const char* dir, POLICY&, int level);
    int get_state(const char* dir, POLICY&, int level);

    virtual int recovery_plan();
    virtual int recovery_action(double);

    void decide_reconstruct();
    int reconstruct_and_cleanup();
    int expand();
    bool some_child_is_unrecoverable() {
        for (unsigned int i=0; i<children.size(); i++) {
            DATA_UNIT& d = *(children[i]);
            if (d.status == UNRECOVERABLE) return true;
        }
        return false;
    }
    int decode();
    int encode();
};

struct CHUNK : DATA_UNIT {
    std::set<VDA_CHUNK_HOST*> hosts;
    META_CHUNK* parent;
    double size;
    bool present_on_server;
    bool new_present_on_server;

    CHUNK(META_CHUNK* mc, double s, int index);

    int start_upload();
    void host_failed(VDA_CHUNK_HOST* p);
    bool download_in_progress();
    void upload_complete();
    void download_complete();
    int assign();
    virtual int recovery_plan();
    virtual int recovery_action(double);
    bool need_more_replicas() {
        return ((int)hosts.size() < parent->dfile->policy.replication);
    }
};
