// Berkeley Open Infrastructure for Network Computing
// http://boinc.berkeley.edu
// Copyright (C) 2008 University of California
//
// This is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation;
// either version 2.1 of the License, or (at your option) any later version.
//
// This software is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//
// To view the GNU Lesser General Public License visit
// http://www.gnu.org/copyleft/lesser.html
// or write to the Free Software Foundation, Inc.,
// 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

// Example multi-thread BOINC application.
// It does 64 "blocks" of computation, where each block is about 1 GFLOP.
// It divides this among a number N of "worker" threads.
// N is passed in through init_data.xml, and defaults to 4.
//
// The main issue is how to suspend/resume the threads.
// The standard BOINC API doesn't work - it assumes that
// the initial thread is the only one.
// What we do instead is to have our initial thread launch the worker threads,
// then go into a polling loop where it checks for suspend/resume messages
// from the BOINC client, and handles them itself.

#include <stdio.h>
#include <vector>
#ifdef _WIN32
#else
#include <signal.h>
#include <pthread.h>
#endif

#include "util.h"
#include "str_util.h"
#include "boinc_api.h"

using std::vector;

#define DEFAULT_NTHREADS 1
#define TOTAL_GFLOPS 64

int gflops_per_thread;

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

    void start(THREAD_FUNC);
    void suspend(bool);
};

struct THREAD_SET {
    vector<THREAD*> threads;
    void suspend(bool if_susp) {
        for (unsigned int i=0; i<threads.size(); i++) {
            THREAD* t = threads[i];
            if (t->id != THREAD_ID_NULL) t->suspend(if_susp);
        }
    }
    bool done() {
        for (unsigned int i=0; i<threads.size(); i++) {
            THREAD* t = threads[i];
            if (t->id != THREAD_ID_NULL) return false;
        }
        return true;
    }
};

#ifdef _WIN32
void THREAD::start(THREAD_FUNC func) {
    id = (HANDLE) _beginthreadex(
        NULL,
        16384,
        func,
        this,
        0,
        NULL
    );
    if (!id) {
        fprintf(stderr, "Can't start thread\n");
        exit(1);
    }
}
void THREAD::suspend(bool if_susp) {
    if (if_susp) {
        SuspendThread(id);
    } else {
        ResumeThread(id);
    }
}
#else
void THREAD::start(THREAD_FUNC func) {
    int retval;
    retval = pthread_create(&id, 0, func, (void*)this);
    if (retval) {
        fprintf(stderr, "can't start thread\n");
        exit(1);
    }
}
void THREAD::suspend(bool if_susp) {
    pthread_kill(id, if_susp?SIGSTOP:SIGCONT);
}
#endif

// do a billion floating-point ops
//
static void giga_flop() {
    double j = 3.14159;
    int i;
    for (i=0; i<500000000; i++) {
        j += 5.12313123;
        j *= 0.5398394834;
    }
}

#ifdef _WIN32
UINT WINAPI worker(void* p) {
#else
void* worker(void* p) {
#endif
    THREAD* t = (THREAD*)p;
    for (int i=0; i<gflops_per_thread; i++) {
        giga_flop();
        fprintf(stderr, "thread %d finished %d\n", t->index, i);
    }
    t->id = THREAD_ID_NULL;
#ifdef _WIN32
    return 0;
#endif
}

int main(int argc, char** argv) {
    int i;
    THREAD_SET thread_set;
    BOINC_OPTIONS options;
    BOINC_STATUS status;
    APP_INIT_DATA aid;
    int nthreads = DEFAULT_NTHREADS;
    double start_time = dtime();

    boinc_options_defaults(options);
    options.direct_process_action = false;
    boinc_init_options(&options);
    boinc_get_status(&status);
    boinc_get_init_data(aid);
    if (strlen(aid.opaque)) {
        parse_int(aid.opaque, "<nthreads>", nthreads);
    }
    gflops_per_thread = TOTAL_GFLOPS/nthreads;

    for (i=0; i<nthreads; i++) {
        THREAD* t = new THREAD;
        t->index = i;
        t->start(worker);
        thread_set.threads.push_back(t);
    }

    while (1) {
        if (thread_set.done()) break;
        bool old_susp = status.suspended;
        boinc_get_status(&status);
        if (status.suspended != old_susp) {
            thread_set.suspend(status.suspended);
        }
        boinc_sleep(0.1);
    }
    double elapsed_time = dtime()-start_time;
    fprintf(stderr,
        "All done.  Used %d threads.  Elapsed time %f\n",
        nthreads, elapsed_time
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
