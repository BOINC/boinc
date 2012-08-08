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
// and the VDA server software (vdad.cpp, sched_vda.cpp)

#include <set>
#include <vector>

#include "boinc_db.h"

#include "stats.h"
#include "vda_policy.h"

// a host with rpc_time < now-HOST_TIMEOUT is considered dead.
// Make sure you set next_rpc_delay accordingly (e.g., to 86400)
//
#define VDA_HOST_TIMEOUT (86400*4)

extern void show_msg(char*);
extern char* time_str(double);
extern const char* status_str(int status);

struct META_CHUNK;

struct VDA_FILE_AUX : VDA_FILE {
    POLICY policy;
    META_CHUNK* meta_chunk;

    VDA_FILE_AUX(){
        meta_chunk = NULL;
    }
    VDA_FILE_AUX(DB_VDA_FILE f) : VDA_FILE(f){}

    // the following for the simulator
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
    // this unit is present on the server
    // (in the case of meta-chunks, this means that enough chunks
    // to reconstruct the meta-chunk are present on the server)
#define RECOVERABLE 1
    // this unit is not present, but could be recovered from data on clients
#define UNRECOVERABLE 2
    // not present or recoverable

// base class for chunks and meta-chunks
//
struct DATA_UNIT {
    virtual int recovery_plan(){return 0;};
    virtual int recovery_action(double){return 0;};

    char name[64];
    char dir[1024];

    // the following are determined during recovery_plan()
    // and used during recovery_action()
    //
    int status;
    bool in_recovery_set;
        // if we need to reconstruct the parent, we'll use this unit
    bool data_now_present;
        // vdad: this unit was initially unrecoverable,
        // but the parent has become present so new this unit is present
    bool data_needed;
        // we need to take action (e.g. start uploads)
        // to make this unit present
    double cost;
    bool keep_present;
        // this unit is present and we need to keep it present
    bool need_present;
        // delete this??
    int min_failures;
        // min # of host failures that would make this unrecoverable

    int delete_file();
        // delete the file on server corresponding to this unit
};

struct META_CHUNK : DATA_UNIT {
    std::vector<DATA_UNIT*> children;
    META_CHUNK* parent;
    int n_children_present;
    bool have_unrecoverable_children;
    VDA_FILE_AUX* dfile;
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

    int decide_reconstruct();
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

    // used by vda
    void print_status(int indent_level);
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

    // used by vda
    void print_status(int indent_level);
};

// names
//
// chunk name: c1.c2.cn
// (in VDA_CHUNK_HOST)
//
// chunk/file name: c1.c2.cn_filename.ext
//
// physical file name: vda_hostid_c1.c2.cn_filename.ext

inline void physical_file_name(
    int hostid, char* chunk_name, char* file_name, char* buf
) {
    sprintf(buf, "vda_%d_%s_%s", hostid, chunk_name, file_name);
}

inline void physical_file_name(
    int hostid, char* chunk_file_name, char* buf
) {
    sprintf(buf, "vda_%d_%s", hostid, chunk_file_name);
}

//
// download WU/result name: vda_download_c1.c2.cn_filename.ext
//
// upload WU/result name: vda_upload_c1.c2.cn_filename.ext
