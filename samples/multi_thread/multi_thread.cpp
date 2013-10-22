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
// This app defines its own classes (THREAD, THREAD_SET) for managing threads.
// You can also use libraries such as OpenMP.
// Just make sure you call boinc_init_parallel().
//
// This app does 64 "units" of computation, where each units is about 1 GFLOP.
// It divides this among N "worker" threads.
// N is passed in the command line, and defaults to 1.
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

#define DEFAULT_NTHREADS 4
#define TOTAL_UNITS 16

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
        char buf[256];

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
            fprintf(stderr, "%s Can't start thread\n",
                boinc_msg_prefix(buf, sizeof(buf))
            );
            exit(1);
        }
#else
        int retval;
        retval = pthread_create(&id, 0, func, (void*)this);
        if (retval) {
            fprintf(stderr, "%s can't start thread\n",
                boinc_msg_prefix(buf, sizeof(buf))
            );
            exit(1);
        }
#endif
    }
};

struct THREAD_SET {
    vector<THREAD*> threads;
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
    char buf[256];
    THREAD* t = (THREAD*)p;
    for (int i=0; i<units_per_thread; i++) {
        double x = do_a_giga_flop(i);
        t->units_done++;
        fprintf(stderr, "%s thread %d finished %d: %f\n",
            boinc_msg_prefix(buf, sizeof(buf)), t->index, i, x
        );
    }
    t->id = THREAD_ID_NULL;
#ifdef _WIN32
    return 0;
#endif
}

int main(int argc, char** argv) {
    int i, nthreads = DEFAULT_NTHREADS;
    double start_time = dtime();
    char buf[256];

    BOINC_OPTIONS options;
    boinc_options_defaults(options);
    options.multi_thread = true;
    boinc_init_options(&options);

    for (i=1; i<argc; i++) {
        if (!strcmp(argv[i], "--nthreads")) {
            nthreads = atoi(argv[++i]);
        } else {
            fprintf(stderr, "%s unrecognized arg: %s\n",
                boinc_msg_prefix(buf, sizeof(buf)), argv[i]
            );
        }
    }

    units_per_thread = TOTAL_UNITS/nthreads;

    THREAD_SET thread_set;
    for (i=0; i<nthreads; i++) {
        thread_set.threads.push_back(new THREAD(worker, i));
    }
    while (1) {
        double f = thread_set.units_done()/((double)TOTAL_UNITS);
        boinc_fraction_done(f);
        if (thread_set.all_done()) break;
        boinc_sleep(1.0);
    }

    double elapsed_time = dtime()-start_time;
    fprintf(stderr,
        "%s All done.  Used %d threads.  Elapsed time %f\n",
        boinc_msg_prefix(buf, sizeof(buf)), nthreads, elapsed_time
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
