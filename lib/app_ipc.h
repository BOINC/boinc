// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2008 University of California
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

#ifndef BOINC_APP_IPC_H
#define BOINC_APP_IPC_H

#ifdef __cplusplus

#include <vector>
#include <string>
#include <cstdio>

#include "filesys.h"
#include "hostinfo.h"
#include "proxy_info.h"
#include "prefs.h"
#include "common_defs.h"

// Communication between the core client and the BOINC app library.
// This code is linked into both core client and app lib.
//
// Some apps may involve separate "coordinator" and "worker" programs.
// The coordinator runs one or more worker programs in sequence,
// and don't do work themselves.
//
// Includes the following:
// - shared memory (APP_CLIENT_SHM)
// - main init file
// - fd init file
// - graphics init file
// - conversion of symbolic links

// Shared memory is a set of MSG_CHANNELs.
// First byte of a channel is nonzero if
// the channel contains an unread data.
// This is set by the sender and cleared by the receiver.
// The sender doesn't write if the flag is set.
// Remaining 1023 bytes contain data.
//
#define MSG_CHANNEL_SIZE 1024

struct MSG_CHANNEL {
    char buf[MSG_CHANNEL_SIZE];
    bool get_msg(char*);    // returns a message and clears pending flag
    inline bool has_msg() {
        return buf[0]?true:false;
    }
    bool send_msg(const char*);   // if there is not a message in the segment,
                            // writes specified message and sets pending flag
    void send_msg_overwrite(const char*);
                            // write message, overwriting any msg already there
};

struct SHARED_MEM {
    MSG_CHANNEL process_control_request;
        // core->app
        // <quit/>
        // <suspend/>
        // <resume/>
    MSG_CHANNEL process_control_reply;
        // app->core
    MSG_CHANNEL graphics_request;
        // core->app
        // not currently used
    MSG_CHANNEL graphics_reply;
        // app->core
        // <web_graphics_url>
        // <remote_desktop_addr>
    MSG_CHANNEL heartbeat;
        // core->app
        // <heartbeat/>         sent every second, even while app is suspended
        // <wss>                app's current working set size
        // <max_wss>            max working set size
    MSG_CHANNEL app_status;
        // app->core
        // status message every second, of the form
        // <current_cpu_time>...
        // <checkpoint_cpu_time>...
        // <working_set_size>...
        // <fraction_done> ...
    MSG_CHANNEL trickle_up;
        // app->core
        // <have_new_trickle_up/>
    MSG_CHANNEL trickle_down;
        // core->app
        // <have_new_trickle_down/>
};

// MSG_QUEUE provides a queuing mechanism for shared-mem messages
// (which don't have one otherwise)
//
struct MSG_QUEUE {
    std::vector<std::string> msgs;
    char name[256];
	double last_block;	// last time we found message channel full
	void init(char*);
    void msg_queue_send(const char*, MSG_CHANNEL& channel);
    void msg_queue_poll(MSG_CHANNEL& channel);
	int msg_queue_purge(const char*);
	bool timeout(double);
};

#define DEFAULT_CHECKPOINT_PERIOD               300

#define SHM_PREFIX          "shm_"
#define QUIT_PREFIX         "quit_"

class APP_CLIENT_SHM {
public:
    SHARED_MEM *shm;

    void reset_msgs();        // resets all messages and clears their flags

    APP_CLIENT_SHM();
};

#ifdef _WIN32
    typedef char SHMEM_SEG_NAME[256];
#else
    typedef int SHMEM_SEG_NAME;
#endif

// parsed version of main init file
// If you add anything here, update copy()
//
struct APP_INIT_DATA {
    int major_version;          // BOINC client version info
    int minor_version;
    int release;
    int app_version;
    char app_name[256];
    char symstore[256];         // symstore URL (Windows)
    char acct_mgr_url[256];
        // if client is using account manager, its URL
    char* project_preferences;
        // project prefs XML
    int userid;
        // project's DB ID for this user/team/host
    int teamid;
    int hostid;
    char user_name[256];
    char team_name[256];
    char project_dir[256];      // where project files are stored on host
    char boinc_dir[MAXPATHLEN];        // BOINC data directory
    char wu_name[256];          // workunit name
    char result_name[256];
    char authenticator[256];    // user's authenticator
    int slot;                   // the slot this job is running in (0, 1, ...)
    int client_pid;             // process ID of BOINC client
    double user_total_credit;
    double user_expavg_credit;
    double host_total_credit;
    double host_expavg_credit;
    double resource_share_fraction;     // this project's resource share frac
    HOST_INFO host_info;
    PROXY_INFO proxy_info;      // in case app wants to use network
    GLOBAL_PREFS global_prefs;
    double starting_elapsed_time;   // elapsed time, counting previous episodes
    bool using_sandbox;         // client is using account-based sandboxing
    bool vm_extensions_disabled;
        // client has already been notified that the VM extensions of
        // the processor have been disabled

    // info about the WU
    double rsc_fpops_est;
    double rsc_fpops_bound;
    double rsc_memory_bound;
    double rsc_disk_bound;
    double computation_deadline;

    // the following are used for compound apps,
    // where each stage of the computation is a fixed fraction of the total.
    //
    double fraction_done_start;
    double fraction_done_end;

    // info for GPU apps
    //
    char gpu_type[64];
    int gpu_device_num;
    int gpu_opencl_dev_index;
    double gpu_usage;   // APP_VERSION.gpu_usage.usage

    // info for multicore apps: how many cores to use
    //
    double ncpus;

    // client configuration info
    //
    bool vbox_window;       // whether to open a console window for VM apps

    // list of files in the app version (for wrappers)
    //
    std::vector<std::string> app_files;

    // Items used by the BOINC runtime system
    //
    double checkpoint_period;     // recommended checkpoint period
    SHMEM_SEG_NAME shmem_seg_name;
    double wu_cpu_time;       // cpu time from previous episodes

    APP_INIT_DATA();
    APP_INIT_DATA(const APP_INIT_DATA&);  // copy constructor
    APP_INIT_DATA &operator=(const APP_INIT_DATA&);
    void copy(const APP_INIT_DATA&);      // actually do the copy here
    void clear();
    ~APP_INIT_DATA();
};

struct GRAPHICS_INFO {
    int xsize;
    int ysize;
    double refresh_period;
};

typedef struct GRAPHICS_INFO GRAPHICS_INFO;

int write_init_data_file(FILE* f, APP_INIT_DATA&);
int parse_init_data_file(FILE* f, APP_INIT_DATA&);
int write_graphics_file(FILE* f, GRAPHICS_INFO* gi);
int parse_graphics_file(FILE* f, GRAPHICS_INFO* gi);

// filenames used in the slot directory
//
#define INIT_DATA_FILE    "init_data.xml"
#define BOINC_FINISH_CALLED_FILE "boinc_finish_called"
#define TEMPORARY_EXIT_FILE "boinc_temporary_exit"
#define TRICKLE_UP_FILENAME "trickle_up.xml"
#define STDERR_FILE           "stderr.txt"
#define STDOUT_FILE           "stdout.txt"
#define LOCKFILE               "boinc_lockfile"
#define UPLOAD_FILE_REQ_PREFIX      "boinc_ufr_"
#define UPLOAD_FILE_STATUS_PREFIX   "boinc_ufs_"

// other filenames
#define PROJECT_DIR "projects"

extern int boinc_link(const char* phys_name, const char* logical_name);
extern int boinc_resolve_filename_s(const char*, std::string&);
extern std::string resolve_soft_link(const char* project_dir, const char* file);
extern void url_to_project_dir(char* url, char* dir, int dirsize);

extern "C" {
#endif

extern int boinc_resolve_filename(const char*, char*, int len);

#ifdef __cplusplus
} // extern "C" {
#endif

#endif
