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

// process states of an ACTIVE_TASK
//
#define PROCESS_UNINITIALIZED   0

// states in which the process exists
#define PROCESS_EXECUTING       1
    // process is running, as far as we know
#define PROCESS_SUSPENDED       9
    // we've sent it a "suspend" message
#define PROCESS_ABORT_PENDING   5
    // process exceeded limits; killed it, waiting to exit

// states in which the process has exited
#define PROCESS_EXITED          2
#define PROCESS_WAS_SIGNALED    3
#define PROCESS_EXIT_UNKNOWN    4
#define PROCESS_ABORTED         6
    // aborted process has exited
#define PROCESS_COULDNT_START   7
#define PROCESS_IN_LIMBO        8
    // process exited zero, but no finish file; leave the task there.

// CPU scheduler states of an ACTIVE_TASK
#define CPU_SCHED_UNINITIALIZED   0
#define CPU_SCHED_PREEMPTED       1
#define CPU_SCHED_SCHEDULED       2


// Represents a task in progress.
// The execution of a task may be divided into many "episodes"
// (if the host is turned off/on, e.g.)
// A task may checkpoint now and then.
// Each episode begins with the state of the last checkpoint.
//
// "CPU time" refers to the sum over all episodes.
// (not counting the "lost" time after the last checkpoint
// in episodes before the current one)
//
// When an active task is created, it is assigned a "slot"
// which determines the directory it runs in.
// This doesn't change over the life of the active task;
// thus the task can use the slot directory for temp files
// that BOINC doesn't know about.
//
class ACTIVE_TASK {
public:
#ifdef _WIN32
    HANDLE pid_handle, thread_handle, quitRequestEvent, shm_handle;
#else
    //key_t shm_key;
#endif
    SHMEM_SEG_NAME shmem_seg_name;
    RESULT* result;
    WORKUNIT* wup;
    APP_VERSION* app_version;
    PROCESS_ID pid;
    int slot;   // which slot (determines directory)
    int state;
    int scheduler_state;
    int next_scheduler_state; // temp
    int exit_status;
    int signal;
    double fraction_done;
        // App's estimate of how much of the work unit is done.
        // Passed from the application via an API call;
        // will be zero if the app doesn't use this call
#if 0
    double frac_rate_of_change;
        // How much the percent done changes per second,
        // based on a recent exponential weighted average
    double last_frac_done, recent_change;
    double last_frac_update;
#endif
    double cpu_time_at_last_sched;
        // CPU time when CPU scheduler last ran
    double episode_start_cpu_time;
        // CPU time at the start of current episode
    double checkpoint_cpu_time;
        // CPU at the last checkpoint
    double current_cpu_time;
        // most recent CPU time reported by app
    double vm_size;
        // most recent virtual memory used in kbytes
    double resident_set_size;
        // most recent resident set size in kbytes
    int current_disk_usage(double&);
        // disk used by output files and temp files of this task
    char slot_dir[256];      // directory where process runs
    double max_cpu_time;    // abort if total CPU exceeds this
    double max_disk_usage;  // abort if disk usage (in+out+temp) exceeds this
    double max_mem_usage;   // abort if memory usage exceeds this
    bool have_trickle_down;
    bool pending_suspend_via_quit;  // waiting for task to suspend via quit
    bool suspended_via_gui;

    APP_CLIENT_SHM app_client_shm;        // core/app shared mem
    MSG_QUEUE graphics_request_queue;
    MSG_QUEUE process_control_queue;

    // info related to app's graphics mode (win, screensaver, etc.)
    //
    int graphics_mode_acked;            // mode acked by app
    int graphics_mode_before_ss;        // mode before last screensaver request
    bool is_ss_app;
    char window_station[256];
    char desktop[256];
    void request_graphics_mode(int, char*, char*);
    int request_reread_prefs();
    void check_graphics_mode_ack();
    int link_user_files();
    int get_shmem_seg_name();

    ACTIVE_TASK();
	~ACTIVE_TASK();
    int init(RESULT*);
    void close_process_handles();
    void detach_and_destroy_shmem();

    int start(bool first_time);         // start the task running
    int request_exit();                 // Send a SIGQUIT signal or equivalent
    bool process_exists();
    int kill_task();                    // send a SIGKILL signal or equivalent
    int suspend();                      // send a SIGSTOP signal or equivalent
    int unsuspend();                    // send a SIGCONT signal or equivalent
    int abort_task(char*);       // flag as abort pending and send kill signal
    bool has_task_exited();             // return true if this task has exited
    int preempt(bool quit_task);        // preempt (via suspend or quit) a running task
    int resume_or_start();
#ifdef _WIN32
    bool handle_exited_app(unsigned long);
#else
    bool handle_exited_app(int stat);
#endif

    bool check_max_cpu_exceeded();
    bool check_max_disk_exceeded();
    bool check_max_mem_exceeded();

    void estimate_frac_rate_of_change(double);
    bool get_app_status_msg();
    bool get_trickle_up_msg();
    double est_cpu_time_to_completion();
    bool read_stderr_file();
    bool finish_file_present();
    bool supports_graphics();
    int write_app_init_file();
    int move_trickle_file();

    int write(MIOFILE&);
    int parse(MIOFILE&);
};

class ACTIVE_TASK_SET {
public:
    typedef std::vector<ACTIVE_TASK*> active_tasks_v;
    active_tasks_v active_tasks;
    int remove(ACTIVE_TASK*);
    ACTIVE_TASK* lookup_pid(int);
    ACTIVE_TASK* lookup_result(RESULT*);
    bool poll(double);
    void suspend_all(bool leave_apps_in_memory=true);
    void unsuspend_all();
    bool is_task_executing();
    int restart_tasks(int max_tasks);
    void request_tasks_exit(PROJECT* p=0);
    int wait_for_exit(double, PROJECT* p=0);
    int exit_tasks(PROJECT* p=0);
    void kill_tasks(PROJECT* p=0);
    int abort_project(PROJECT*);
    bool get_msgs();
    bool check_app_exited();
    bool check_rsc_limits_exceeded();
    bool vm_limit_exceeded(double);
    int get_free_slot();
    void send_heartbeats();
    void send_trickle_downs();

    // screensaver-related functions
    ACTIVE_TASK* get_ss_app();
    void save_app_modes();
    void hide_apps();
    void restore_apps();
    void graphics_poll();
    void process_control_poll();
    void request_reread_prefs(PROJECT*);

    int write(MIOFILE&);
    int parse(MIOFILE&);
};

#endif
