// The contents of this file are subject to the BOINC Public License
// Version 1.0 (the "License"); you may not use this file except in
// compliance with the License. You may obtain a copy of the License at
// http://boinc.berkeley.edu/license_1.0.txt
//
// Software distributed under the License is distributed on an "AS IS"
// basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the
// License for the specific language governing rights and limitations
// under the License.
//
// The Original Code is the Berkeley Open Infrastructure for Network Computing.
//
// The Initial Developer of the Original Code is the SETI@home project.
// Portions created by the SETI@home project are Copyright (C) 2002
// University of California at Berkeley. All Rights Reserved.
//
// Contributor(s):
//

#ifndef _TASK_
#define _TASK_

#ifndef _WIN32
#include <stdio.h>
#include <vector>
#endif

#include "client_types.h"
#include "app_ipc.h"

class CLIENT_STATE;
typedef int PROCESS_ID;

// Possible states of a process in an ACTIVE_TASK
#define PROCESS_UNINITIALIZED   0
#define PROCESS_RUNNING         1
#define PROCESS_EXITED          2
#define PROCESS_WAS_SIGNALED    3
#define PROCESS_EXIT_UNKNOWN    4
#define PROCESS_ABORT_PENDING   5
    // process exceeded limits; killed it, waiting to exit
#define PROCESS_ABORTED         6
    // process has exited
#define PROCESS_COULDNT_START   7


// Represents a task in progress.
// The execution of a task may be divided into many "episodes"
// (if the host is turned off/on, e.g.)
// A task may checkpoint now and then.
// Each episode begins with the state of the last checkpoint.
//
class ACTIVE_TASK {
public:
#ifdef _WIN32
    HANDLE pid_handle, thread_handle, quitRequestEvent, shm_handle;
#else
    key_t shm_key;
#endif
    RESULT* result;
    WORKUNIT* wup;
    APP_VERSION* app_version;
    PROCESS_ID pid;
    int slot;   // which slot (determines directory)
    int state;
    int exit_status;
    int signal;
    double fraction_done;
        // App's estimate of how much of the work unit is done.
        // Passed from the application via an API call;
        // will be zero if the app doesn't use this call
    double frac_rate_of_change;
        // How much the percent done changes per second,
        // based on a recent exponential weighted average
    double last_frac_done, recent_change;
    double last_frac_update;
    double starting_cpu_time;
        // total CPU time at the start of current episode
    double checkpoint_cpu_time;
        // total CPU at the last checkpoint
    double current_cpu_time;
        // most recent total CPU time reported by app
    double working_set_size;
        // most recent size of RAM working set in bytes
    int current_disk_usage(double&);
        // disk used by output files and temp files of this task
    char slot_dir[256];      // directory where process runs
    double max_cpu_time;    // abort if total CPU exceeds this
    double max_disk_usage;  // abort if disk usage (in+out+temp) exceeds this
    double max_mem_usage;   // abort if memory usage exceeds this

    APP_CLIENT_SHM app_client_shm;        // core/app shared mem
//    time_t last_status_msg_time;

    // info related to app's graphics mode (win, screensaver, etc.)
    int graphics_requested_mode;        // our last request to this app
    int graphics_request_time;            // when we sent it
    int graphics_acked_mode;            // most recent mode reported by app
    int graphics_mode_before_ss;        // mode before last screensaver request
    void request_graphics_mode(int);
    int request_reread_prefs();
    void check_graphics_mode_ack();
    int link_user_files();

    ACTIVE_TASK();
	~ACTIVE_TASK();
    int init(RESULT*);

    int start(bool first_time);         // start the task running
    int request_exit();                 // Send a SIGQUIT signal or equivalent
    int kill_task();                    // send a SIGKILL signal or equivalent
    int suspend();                      // send a SIGSTOP signal or equivalent
    int unsuspend();                    // send a SIGCONT signal or equivalent
    int abort_task(char*);       // flag as abort pending and send kill signal
    bool task_exited();                 // return true if this task has exited

    bool check_max_cpu_exceeded();
    bool check_max_disk_exceeded();
    bool check_max_mem_exceeded();

    void estimate_frac_rate_of_change(double);
    bool get_status_msg();
    double est_time_to_completion();
    bool read_stderr_file();
    bool supports_graphics();
    int write_app_init_file(APP_INIT_DATA&);
    int move_trickle_file();

    int write(FILE*);
    int parse(FILE*, CLIENT_STATE*);
};

class ACTIVE_TASK_SET {
public:
    typedef vector<ACTIVE_TASK*> active_tasks_v;
    active_tasks_v active_tasks;
    int insert(ACTIVE_TASK*);
    int remove(ACTIVE_TASK*);
    ACTIVE_TASK* lookup_pid(int);
    ACTIVE_TASK* lookup_result(RESULT*);
    bool poll();
    void suspend_all();
    void unsuspend_all();
    int restart_tasks();
    void request_tasks_exit(PROJECT* p=0);
    int wait_for_exit(double, PROJECT* p=0);
    int exit_tasks(PROJECT* p=0);
    void kill_tasks(PROJECT* p=0);
    int abort_project(PROJECT*);
    bool get_status_msgs();
    bool check_app_exited();
    bool check_rsc_limits_exceeded();
    int get_free_slot(int total_slots);

    // screensaver-related functions
    ACTIVE_TASK* get_graphics_capable_app();
    ACTIVE_TASK* get_app_requested(int req_mode);
    void save_app_modes();
    void hide_apps();
    void restore_apps();
    void check_graphics_mode_ack();
    void request_reread_prefs(PROJECT*);

    int write(FILE*);
    int parse(FILE*, CLIENT_STATE*);
};

#endif
