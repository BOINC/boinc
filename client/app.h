// The contents of this file are subject to the Mozilla Public License
// Version 1.0 (the "License"); you may not use this file except in
// compliance with the License. You may obtain a copy of the License at
// http://www.mozilla.org/MPL/ 
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

#include "windows_cpp.h"
#ifdef _WIN32
#include <afxwin.h>
#endif

#include <stdio.h>
#include <vector>
#include "client_types.h"
#include "boinc_api.h"

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
    APP_CLIENT_SHM *app_client_shm;
    int slot;   // which slot (determines directory)
    int state;
    int exit_status;
    int signal;
    double fraction_done;
        // App's estimate of how much of the work unit is done.
        // Passed from the application via an API call;
        // will be zero if the app doesn't use this call
    double starting_cpu_time;
        // total CPU time at the start of current episode
    double checkpoint_cpu_time;
        // total CPU at the last checkpoint
    double current_cpu_time;
        // most recent total CPU time reported by app
    int current_disk_usage(double&);
        // disk used by output files and temp files of this task
    char slot_dir[256];      // directory where process runs
    double max_cpu_time;
    double max_disk_usage;

    ACTIVE_TASK();
    int init(RESULT*);

    int start(bool first_time);          // start the task running
    int request_exit();
        // ask a task to exit.  doesn't wait for it to do so.
    int request_pause();
        // ask a task to pause.  doesn't wait for it to do so.
    int kill_task();
        // externally kill the task.  doesn't wait for exit
    bool task_exited();
        // return true if this task has exited
    int abort();
        // kill, and flag as abort pending

    int suspend();
    int unsuspend();

    int get_cpu_time();
    bool check_app_status();
    double est_time_to_completion();
    bool read_stderr_file();

    int write(FILE*);
    int parse(FILE*, CLIENT_STATE*);
};

class ACTIVE_TASK_SET {
public:
    vector<ACTIVE_TASK*> active_tasks;
    int insert(ACTIVE_TASK*);
    int remove(ACTIVE_TASK*);
    int wait_for_exit(double);
    ACTIVE_TASK* lookup_pid(int);
    bool poll();
    bool poll_time();
    void suspend_all();
    void unsuspend_all();
    int restart_tasks();
    void request_tasks_exit();
    void kill_tasks();
    void check_apps();
    int get_free_slot(int total_slots);
	void free_mem();

    int write(FILE*);
    int parse(FILE*, CLIENT_STATE*);
};

#endif
