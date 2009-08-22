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

// Example multi-thread BOINC application.
// It does 64 "units" of computation, where each units is about 1 GFLOP.
// It divides this among N "worker" threads.
// N is passed in the command line, and defaults to 1.
//
// The main issue is how to suspend/resume the threads.
// The standard BOINC API doesn't work - it assumes that
// the initial thread is the only one.
// On Linux, there's no API to suspend/resume threads.
// All you can do is SIGSTOP/SIGCONT, which affects the whole process.
// So we use the following process/thread structure:
//
// Windows:
// Initial thread:
//  - launches worker threads,
//  - in polling loop, checks for suspend/resume messages
//    from the BOINC client, and handles them itself.
// Unix:
//  Initial process
//    - forks worker process
//    - in polling loop, checks for worker process completion
//    - doesn't send status msgs
//  Worker process
//    Initial thread:
//    - forks worker threads, wait for them to finish, exit
//    - uses BOINC runtime to send status messages (frac done, CPU time)
//
// Doesn't do checkpointing.

#include <stdio.h>
#include <vector>
#ifdef _WIN32
#include "boinc_win.h"
#else
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <pthread.h>
#endif

#include "util.h"
#include "str_util.h"
#include "boinc_api.h"

using std::vector;

#define DEFAULT_NTHREADS 1
#define TOTAL_UNITS 64

int units_per_thread;

#ifdef _WIN32
typedef HANDLE THREAD_ID;
typedef UINT (__stdcall *THREAD_FUNC)(void*);
#else
typedef void* (*THREAD_FUNC)(void*);
typedef pthread_t THREAD_ID;
#endif
#define THREAD_ID_NULL  0

// An abstraction of threads.
// A thread function is passed a pointer to its own object,
// and sets its ID to THREAD_ID_NULL when it's finished.
//

struct THREAD {
    THREAD_ID id;
    int index;
    int units_done;

    THREAD(THREAD_FUNC func, int i) {
        index = i;
        units_done = 0;
#ifdef _WIN32
        id = (HANDLE) _beginthreadex(
            NULL,
            16384,
            func,
            this,
            0,
            NULL
        );
        if (!id) {
            fprintf(stderr, "%s Can't start thread\n", boinc_msg_prefix());
            exit(1);
        }
#else
        int retval;
        retval = pthread_create(&id, 0, func, (void*)this);
        if (retval) {
            fprintf(stderr, "%s can't start thread\n", boinc_msg_prefix());
            exit(1);
        }
#endif
    }

#ifdef _WIN32
    void suspend(bool if_susp) {
        if (if_susp) {
            SuspendThread(id);
        } else {
            ResumeThread(id);
        }
    }
#endif
};

struct THREAD_SET {
    vector<THREAD*> threads;
#ifdef _WIN32
    void suspend(bool if_susp) {
        for (unsigned int i=0; i<threads.size(); i++) {
            THREAD* t = threads[i];
            if (t->id != THREAD_ID_NULL) t->suspend(if_susp);
        }
        fprintf(stderr, "%s suspended all\n", boinc_msg_prefix());
    }
#endif
    bool all_done() {
        for (unsigned int i=0; i<threads.size(); i++) {
            if (threads[i]->id != THREAD_ID_NULL) return false;
        }
        return true;
    }
    int units_done() {
        int count = 0;
        for (unsigned int i=0; i<threads.size(); i++) {
            count += threads[i]->units_done;
        }
        return count;
    }
};

// do a billion floating-point ops
// (note: I needed to add an arg to this;
// otherwise the MS C++ compiler optimizes away
// all but the first call to it!)
//
static double do_a_giga_flop(int foo) {
    double x = 3.14159*foo;
    int i;
    for (i=0; i<500000000; i++) {
        x += 5.12313123;
        x *= 0.5398394834;
    }
    return x;
}

#ifdef _WIN32
UINT WINAPI worker(void* p) {
#else
void* worker(void* p) {
#endif
    THREAD* t = (THREAD*)p;
    for (int i=0; i<units_per_thread; i++) {
        double x = do_a_giga_flop(i);
        t->units_done++;
        fprintf(stderr, "%s thread %d finished %d: %f\n",
            boinc_msg_prefix(), t->index, i, x
        );
    }
    t->id = THREAD_ID_NULL;
#ifdef _WIN32
    return 0;
#endif
}

void main_thread(int nthreads) {
    int i;
#ifdef _WIN32
    static BOINC_STATUS status;
#endif
    THREAD_SET thread_set;
    for (i=0; i<nthreads; i++) {
        thread_set.threads.push_back(new THREAD(worker, i));
    }
    while (1) {
        double f = thread_set.units_done()/((double)TOTAL_UNITS);
        boinc_fraction_done(f);
        if (thread_set.all_done()) break;
#ifdef _WIN32
        int old_susp = status.suspended;
        boinc_get_status(&status);
        if (status.quit_request || status.abort_request || status.no_heartbeat) {
            exit(0);
        }
        if (status.suspended != old_susp) {
            thread_set.suspend(status.suspended != 0);
        }
        boinc_sleep(0.1);
#else
        boinc_sleep(1.0);
#endif
    }
}

int main(int argc, char** argv) {
    BOINC_OPTIONS options;
    int nthreads = DEFAULT_NTHREADS;
    double start_time = dtime();

    boinc_options_defaults(options);
    options.direct_process_action = 0;

    for (int i=1; i<argc; i++) {
        if (!strcmp(argv[i], "--nthreads")) {
            nthreads = atoi(argv[++i]);
        } else {
            fprintf(stderr, "%s unrecognized arg: %s\n",
                boinc_msg_prefix(), argv[i]
            );
        }
    }

    units_per_thread = TOTAL_UNITS/nthreads;

#ifdef _WIN32
    boinc_init_options(&options);
    main_thread(nthreads);
#else
    options.send_status_msgs = 0;
    boinc_init_options(&options);
    int pid = fork();
    if (pid) {          // parent
        BOINC_STATUS status;
        boinc_get_status(&status);
        int exit_status;
        while (1) {
            bool old_susp = status.suspended;
            boinc_get_status(&status);
            if (status.quit_request || status.abort_request || status.no_heartbeat) {
                kill(pid, SIGKILL);
                exit(0);
            }
            if (status.suspended != old_susp) {
                kill(pid, status.suspended?SIGSTOP:SIGCONT);
            }
            if (waitpid(pid, &exit_status, WNOHANG) == pid) {
                break;
            }
            boinc_sleep(0.1);
        }
    } else {            // child (worker)
        memset(&options, 0, sizeof(options));
        options.send_status_msgs = 1;
        boinc_init_options(&options);
        main_thread(nthreads);
    }
#endif

    double elapsed_time = dtime()-start_time;
    fprintf(stderr,
        "%s All done.  Used %d threads.  Elapsed time %f\n",
        boinc_msg_prefix(), nthreads, elapsed_time
    );
    boinc_finish(0);
}

#ifdef _WIN32
int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR Args, int WinMode) {
    LPSTR command_line;
    char* argv[100];
    int argc;

    command_line = GetCommandLine();
    argc = parse_command_line( command_line, argv );
    return main(argc, argv);
}
#endif
