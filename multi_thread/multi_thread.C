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

// Example multi-thread BOINC application
// The main thread spawns N threads, each of which computers for X seconds
// It handles BOINC's process control messages itself.

#include <stdio.h>
#include <vector>
#include <signal.h>
#include <pthread.h>

#include "boinc_api.h"

using std::vector;

int ngflops = 10;

#ifdef _WIN32
typedef UINT THREAD_ID;
#else
typedef pthread_t THREAD_ID;
#endif
#define THREAD_ID_NULL  0

// An abstraction of threads.
// A thread function is passed a pointer to its own object,
// and sets its ID to THREAD_ID_NULL when it's finished.
//
typedef void* (*THREAD_FUNC)(void*);

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
THREAD_ID create_thread(void *func()) {
    uintptr_t thread;
    UINT thread_id;
    thread = _beginthreadex(
        NULL,
        16384,
        func,
        0,
        0,
        &thread_id
    );
    if (!thread) return THREAD_ID_NULL;
    return thread_id;
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

static void giga_flop() {
    double j = 3.14159;
    int i;
    for (i=0; i<500000000; i++) {
        j += 5.12313123;
        j *= 0.5398394834;
    }
}

void* worker(void* p) {
    THREAD* t = (THREAD*)p;
    for (int i=0; i<ngflops; i++) {
        giga_flop();
        fprintf(stderr, "thread %d finished %d\n", t->index, i);
    }
    t->id = THREAD_ID_NULL;
}

int main() {
    int i;
    THREAD_SET thread_set;
    BOINC_OPTIONS options;
    BOINC_STATUS status;
    APP_INIT_DATA aid;
    int nthreads = 4;

    options.direct_process_action = false;
    boinc_init_options(&options);
    boinc_get_status(&status);
    boinc_get_init_data(aid);

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
    }
    printf("All done.\n");
}
