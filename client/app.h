// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2020 University of California
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

#ifndef BOINC_APP_H
#define BOINC_APP_H

#ifndef _WIN32
#include <cstdio>
#include <vector>
#endif

#include "app_ipc.h"
#include "common_defs.h"
#include "procinfo.h"

#include "client_types.h"
#include "result.h"

// values for preempt_type (see ACTIVE_TASK::preempt())
//
enum PREEMPT_TYPE {
    REMOVE_NEVER        = 0,
        // don't remove from memory
    REMOVE_MAYBE_USER   = 1,
        // remove from mem if GPU; don't remove if never checkpointed
    REMOVE_MAYBE_SCHED  = 2,
        // ditto
    REMOVE_ALWAYS       = 3
        // remove from memory
};

struct ASYNC_COPY;
typedef int PROCESS_ID;

#define MAX_STDERR_LEN  65536
    // The stderr output of an application is truncated to this length
    // before sending to server,
    // to protect against apps that write unbounded amounts.

// Represents a job in progress.

// When a job is started, it is assigned a "slot"
// which determines the directory it runs in.
// This doesn't change over the life of the job;
// so it can use the slot directory for temp files
// that BOINC doesn't know about.

// If you add anything, initialize it in the constructor
//
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

    // START OF ITEMS SAVED IN TASK STATE FILE
    // (in addition to result name and project URL)

    double checkpoint_cpu_time;
        // CPU at the last checkpoint
        // Note: "CPU time" refers to the sum over all episodes.
        // (not counting the "lost" time after the last checkpoint
        // in episodes before the current one)
    double checkpoint_elapsed_time;
        // elapsed time at last checkpoint
    double peak_working_set_size;
    double peak_swap_size;
    double peak_disk_usage;
        // based on real (not allocated/compressed) file sizes

    // START OF ITEMS ALSO SAVED IN CLIENT STATE FILE

    int _task_state;
        // PROCESS_*; see common_defs.h
    int slot;
        // subdirectory of slots/ where this runs
    double checkpoint_fraction_done;
        // fraction done at last checkpoint
    double checkpoint_fraction_done_elapsed_time;
        // fraction done elapsed time at last checkpoint
    double current_cpu_time;
        // most recent CPU time reported by app
    bool once_ran_edf;

    // END OF ITEMS SAVED IN STATE FILES

    double wss_from_app;
        // work set size reported by the app
        // (e.g. docker_wrapper does this).
        // If nonzero, use this instead of procinfo data
    double fraction_done;
        // App's estimate of how much of the work unit is done.
        // Passed from the application via an API call;
        // will be zero if the app doesn't use this call
    double fraction_done_elapsed_time;
        // elapsed time when fraction done was last reported
    double first_fraction_done;
        // first frac done reported during this run of task
    double first_fraction_done_elapsed_time;
        // elapsed time when the above was reported
    SCHEDULER_STATE scheduler_state;
    SCHEDULER_STATE next_scheduler_state; // temp
    int signal;
    double run_interval_start_wall_time;
        // Wall time at the start of the current run interval
    double checkpoint_wall_time;
        // wall time at the last checkpoint
    double elapsed_time;
        // current total running time, adjusted for CPU throttling
    double bytes_sent_episode;
        // bytes sent in current episode of job,
        // as (optionally) reported by boinc_network_usage()
    double bytes_received_episode;
    double bytes_sent;
        // bytes in all episodes
    double bytes_received;
    char slot_dir[256];
        // directory where process runs (relative)
    char slot_path[MAXPATHLEN];
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
        // Working set too large to run now; waiting for RAM
        // This is a slight misnomer.
        // It doesn't mean that this job itself is too large;
        // rather, it means that the last time we did CPU scheduling,
        // the set of jobs we tried to run was too big,
        // and this one came after we ran out of mem.
    bool needs_shmem;
        // waiting for a free shared memory segment
    int want_network;
        // This task wants to do network comm (for F@h)
        // this is passed via share-memory message (app_status channel)
    double abort_time;
        // when we sent an abort message to this app
        // kill it 5 seconds later if it doesn't exit
    double quit_time;
        // when we sent a quit message; kill if still there after 10 sec
    int premature_exit_count;
        // how many times app has exited without finish file.
        // abort job if 100 exits w/o checkpoint
    bool overdue_checkpoint;
        // running past end of time slice because not checkpointed;
        // when we do checkpoint, reschedule
    double last_deadline_miss_time;

    APP_CLIENT_SHM app_client_shm;
        // core/app shared mem segment
    MSG_QUEUE graphics_request_queue;
    MSG_QUEUE process_control_queue;
    std::vector<int> other_pids;
        // IDs of processes that are part of this task
        // but not descendants of the main process
        // (e.g. VMs created by vboxwrapper)
        // These are communicated via the app_status message channel
    char web_graphics_url[256];
    char remote_desktop_addr[256];
    ASYNC_COPY* async_copy;
    double finish_file_time;
        // time when we saw finish file in slot dir.
        // Used to kill apps that hang after writing finished file
    int graphics_pid;
        // PID of running graphics app (Mac)
    SPORADIC_CA_STATE sporadic_ca_state;
    SPORADIC_AC_STATE sporadic_ac_state;
    double sporadic_ignore_until;

    void set_task_state(int, const char*);
    inline int task_state() {
        return _task_state;
    }
    inline bool sporadic() {
        return wup->app->sporadic;
    }
    inline bool non_cpu_intensive() {
        return result->non_cpu_intensive();
    }
    inline bool always_run() {
        return sporadic() || non_cpu_intensive();
    }
    inline bool dont_throttle() {
        return result->dont_throttle();
    }
    int request_reread_prefs();
    int request_reread_app_info();
    int link_user_files();
    int get_shmem_seg_name();
    bool runnable() {
        return _task_state == PROCESS_UNINITIALIZED
            || _task_state == PROCESS_EXECUTING
            || _task_state == PROCESS_SUSPENDED;
    }
    void copy_final_info();
        // copy final CPU time etc. to result

    ACTIVE_TASK();
    ~ACTIVE_TASK();
    int init(RESULT*);
    void cleanup_task();

    int current_disk_usage(double&);
        // total sizes of output files and temp files of this task
        // This is compared with project-specified limits
        // to decide whether to abort job; no other use.
    int get_free_slot(RESULT*);
    int setup_slot_dir(char *buf, unsigned int size);
    int start();

    // Termination stuff.
    // Terminology:
    // "kill": forcibly kill the main process and all its descendants.
    // "request exit": send a request-exit message, and enumerate descendants.
    //      If after 15 secs any processes remain, kill them
    //      called from:
    //          task preemption
    //          project detach or reset
    //      implementation:
    //          sends msg, sets quit_time, state QUIT_PENDING;
    //              get list of descendants
    //          normal exit handled in handle_premature_exit()
    //          timeout handled in ACTIVE_TASK_SET::poll()
    // "abort_task": like request exit,
    //      but the app is supposed to write a stack trace to stderr
    //      called from: rsc exceeded; got ack of running task;
    //          intermediate upload failure
    //          client exiting w/ abort_jobs_on_exit set
    //
    int request_exit();
    int request_abort();
    int kill_running_task(bool will_restart);
        // Kill process and subsidiary processes forcibly.
        // Unix: send a SIGKILL signal, Windows: TerminateProcess()
    int kill_subsidiary_processes();
        // kill subsidiary processes of a job
        // whose main process has already exited
    int abort_task(int exit_status, const char*);
        // can be called whether or not process exists

    // is the GPU task running or suspended (due to CPU throttling)
    //
    inline bool is_gpu_task_running() {
        int s = task_state();
        return s == PROCESS_EXECUTING || s == PROCESS_SUSPENDED;
    }

    // Implementation stuff related to termination
    //
    std::vector<int> descendants;
        // PIDs of descendants, computed every 10 sec or so
        // during resource usage computation.
    bool process_exists();
    bool has_task_exited();
        // return true if this task has exited

    int suspend();
        // tell a process to stop executing (but stay in mem)
        // Done by sending it a <suspend> message
    int unsuspend(int reason=0);
        // Undo a suspend: send a <resume> message
    int preempt(PREEMPT_TYPE preempt_type, int reason=0);
        // preempt (via suspend or quit) a running task
    int resume_or_start(bool);
    void send_network_available();
#ifdef _WIN32
    void handle_exited_app(unsigned long);
#else
    void handle_exited_app(int stat);
#endif
    void handle_premature_exit(bool&);
    void handle_temporary_exit(bool&, double, const char*, bool);

    bool check_max_disk_exceeded();

    bool get_app_status_msg();
    bool get_trickle_up_msg();
    void get_graphics_msg();
    double est_dur();
    int read_stderr_file();
    bool finish_file_present(int&);
    bool temporary_exit_file_present(double&, char*, bool&);
    void init_app_init_data(APP_INIT_DATA&);
    int write_app_init_file(APP_INIT_DATA&);
    int move_trickle_file();
    int handle_upload_files();
    void upload_notify_app(const FILE_INFO*, const FILE_REF*);
    int copy_output_files();
    int setup_file(FILE_INFO*, FILE_REF&, char*, bool, bool);
    bool must_copy_file(FILE_REF&, bool);
    void write_task_state_file();
    void read_task_state_file();

    int write(MIOFILE&);
    int write_gui(MIOFILE&);
    int parse(XML_PARSER&);
};

// Represents the set of all jobs in progress

class ACTIVE_TASK_SET {
public:
    typedef std::vector<ACTIVE_TASK*> active_tasks_v;
    active_tasks_v active_tasks;
    ACTIVE_TASK* lookup_pid(int);
    ACTIVE_TASK* lookup_result(RESULT*);
    ACTIVE_TASK* lookup_slot(int);
    void init();
    bool poll();
    void suspend_all(int reason);
    void unsuspend_all(int reason=0);
    bool is_task_executing();
    void request_tasks_exit(PROJECT* p=0);
    int wait_for_exit(double, PROJECT* p=0);
    int exit_tasks(PROJECT* p=0);
    void kill_tasks(PROJECT* p=0);
    int abort_project(PROJECT*);
    void get_msgs();
    bool check_app_exited();
    bool check_rsc_limits_exceeded();
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
    void check_for_finished_jobs();

    void process_control_poll();
    void request_reread_prefs(PROJECT*);
    void request_reread_app_info();

    int write(MIOFILE&);
    int parse(XML_PARSER&);
};

extern double exclusive_app_running;    // last time an exclusive app was running
extern double exclusive_gpu_app_running;
extern int gpu_suspend_reason;
extern double non_boinc_cpu_usage;

extern void run_test_app();

#ifdef _WIN32
extern DWORD WINAPI throttler(void*);
#else
extern void* throttler(void*);
#endif

#endif
