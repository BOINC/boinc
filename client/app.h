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

#ifndef _TASK_
#define _TASK_

#ifndef _WIN32
#include <cstdio>
#include <vector>
#endif

#include "client_types.h"
#include "common_defs.h"
#include "app_ipc.h"
#include "procinfo.h"

// values for preempt_type
//
#define REMOVE_NEVER        0
#define REMOVE_MAYBE_USER   1
#define REMOVE_MAYBE_SCHED  2
#define REMOVE_ALWAYS       3

class CLIENT_STATE;
typedef int PROCESS_ID;

#define MAX_STDERR_LEN  65536
    // The stderr output of an application is truncated to this length
    // before sending to server,
    // to protect against apps that write unbounded amounts.

// Represents a job in progress.

// When an active task is created, it is assigned a "slot"
// which determines the directory it runs in.
// This doesn't change over the life of the active task;
// thus the task can use the slot directory for temp files
// that BOINC doesn't know about.

struct ACTIVE_TASK {
#ifdef _WIN32
    HANDLE process_handle, shm_handle;
    bool kill_all_children();
#endif
    SHMEM_SEG_NAME shmem_seg_name;
    RESULT* result;
    WORKUNIT* wup;
    APP_VERSION* app_version;
    PROCESS_ID pid;
	PROCINFO procinfo;

    // START OF ITEMS SAVED IN STATE FILE
    int _task_state;
    int slot;
        // subdirectory of slots/ where this runs
    double checkpoint_cpu_time;
        // CPU at the last checkpoint
        // Note: "CPU time" refers to the sum over all episodes.
        // (not counting the "lost" time after the last checkpoint
        // in episodes before the current one)
        // TODO: debt should be based on FLOPs, not CPU time
        // CPU time at the start of current episode
    double checkpoint_elapsed_time;
        // elapsed time at last checkpoint
    double fraction_done;
        // App's estimate of how much of the work unit is done.
        // Passed from the application via an API call;
        // will be zero if the app doesn't use this call
    double current_cpu_time;
        // most recent CPU time reported by app
    bool once_ran_edf;

    // END OF ITEMS SAVED IN STATE FILE

    int scheduler_state;
    int next_scheduler_state; // temp
    int signal;
    double run_interval_start_wall_time;
        // Wall time at the start of the current run interval
    double checkpoint_wall_time;
        // wall time at the last checkpoint
    double elapsed_time;
        // current total elapsed (running) time
    int current_disk_usage(double&);
        // disk used by output files and temp files of this task
    char slot_dir[256];
        // directory where process runs (relative)
    char slot_path[512];
        // same, absolute
        // This is used only to run graphics apps
        // (that way don't have to worry about top-level dirs
        // being non-readable, etc).
    double max_elapsed_time;
        // abort if elapsed time exceeds this
    double max_disk_usage;
        // abort if disk usage (in+out+temp) exceeds this
    double max_mem_usage;
        // abort if memory usage exceeds this
    bool have_trickle_down;
    bool send_upload_file_status;
    bool too_large;
        // working set too large to run now
    bool needs_shmem;               // waiting for a free shared memory segment
    int want_network;
        // This task wants to do network comm (for F@h)
        // this is passed via share-memory message (app_status channel)
    double abort_time;
        // when we sent an abort message to this app
        // kill it 5 seconds later if it doesn't exit
    double quit_time;
    int premature_exit_count;
        // when we sent a quit message; kill if still there after 10 sec

    APP_CLIENT_SHM app_client_shm;
        // core/app shared mem segment
    MSG_QUEUE graphics_request_queue;
    MSG_QUEUE process_control_queue;

    void set_task_state(int, const char*);
    inline int task_state() {
        return _task_state;
    }

    // info related to app's graphics mode (win, screensaver, etc.)
    //
    int graphics_mode_acked;
        // mode acked by app
    int graphics_mode_before_ss;
        // mode before last screensaver request
    double graphics_mode_ack_timeout;

#ifdef SIM
    double cpu_time_left;
#endif

#if (defined (__APPLE__) && (defined(__i386__) || defined(__x86_64__)))
    // PowerPC apps emulated on i386 Macs crash if running graphics
    int powerpc_emulated_on_i386;
    int is_native_i386_app(char*);
#endif
    GRAPHICS_MSG graphics_msg;
    void request_graphics_mode(GRAPHICS_MSG&);
    int request_reread_prefs();
    int request_reread_app_info();
    void check_graphics_mode_ack();
    int link_user_files();
    int get_shmem_seg_name();
    bool runnable() {
        return _task_state == PROCESS_UNINITIALIZED
            || _task_state == PROCESS_EXECUTING
            || _task_state == PROCESS_SUSPENDED;
    }

    ACTIVE_TASK();
	~ACTIVE_TASK();
    int init(RESULT*);
    void cleanup_task();

    void get_free_slot(RESULT*);
    int start(bool first_time);         // start a process
    int request_exit();
        // ask the process to exit gracefully,
        // i.e. by sending a <quit> message
    int request_abort();                // send "abort" message
    bool process_exists();

    int kill_task(bool restart);
        // Kill process forcibly,
		// otherwise it ends with an error
        // Unix: send a SIGKILL signal, Windows: TerminateProcess()
		// if restart is true, arrange for resulted to get restarted;

    int suspend();
        // tell a process to stop executing (but stay in mem)
        // Done by sending it a <suspend> message
    int unsuspend();
        // Undo a suspend: send a <resume> message
    int abort_task(int exit_status, const char*);
        // can be called whether or not process exists
    bool has_task_exited();
        // return true if this task has exited
    int preempt(int preempt_type);
        // preempt (via suspend or quit) a running task
    int resume_or_start(bool);
    void send_network_available();
#ifdef _WIN32
    void handle_exited_app(unsigned long);
#else
    void handle_exited_app(int stat);
#endif
    void handle_premature_exit(bool&);

    bool check_max_disk_exceeded();

    bool get_app_status_msg();
    bool get_trickle_up_msg();
    double est_dur(bool for_work_fetch);
    bool read_stderr_file();
    bool finish_file_present();
    bool temporary_exit_file_present(double&);
    bool supports_graphics();
    int write_app_init_file();
    int move_trickle_file();
    int handle_upload_files();
    void upload_notify_app(const FILE_INFO*, const FILE_REF*);
    int copy_output_files();
    void write_task_state_file();
    void read_task_state_file();

    int write(MIOFILE&);
    int write_gui(MIOFILE&);
    int parse(MIOFILE&);
};

// Represents the set of all jobs in progress

class ACTIVE_TASK_SET {
public:
    typedef std::vector<ACTIVE_TASK*> active_tasks_v;
    active_tasks_v active_tasks;
    ACTIVE_TASK* lookup_pid(int);
    ACTIVE_TASK* lookup_result(RESULT*);
    void init();
    bool poll();
    void suspend_all(int reason);
    void unsuspend_all();
    bool is_task_executing();
    void request_tasks_exit(PROJECT* p=0);
    int wait_for_exit(double, PROJECT* p=0);
    int exit_tasks(PROJECT* p=0);
    void kill_tasks(PROJECT* p=0);
    int abort_project(PROJECT*);
    void get_msgs();
    bool check_app_exited();
    bool check_rsc_limits_exceeded();
    bool check_quit_timeout_exceeded();
    bool is_slot_in_use(int);
    bool is_slot_dir_in_use(char*);
    void send_heartbeats();
    void send_trickle_downs();
    void report_overdue();
    void handle_upload_files();
    void upload_notify_app(FILE_INFO*);
    bool want_network();    // does any task want network?
    void network_available();   // notify tasks that network is available
    void free_mem();
    bool slot_taken(int);
    void get_memory_usage();

    // graphics-related functions
    void graphics_poll();
    void process_control_poll();
    void request_reread_prefs(PROJECT*);
    void request_reread_app_info();

    int write(MIOFILE&);
    int parse(MIOFILE&);
};

extern bool exclusive_app_running;
extern bool exclusive_gpu_app_running;
extern int gpu_suspend_reason;
extern double non_boinc_cpu_usage;

#endif
