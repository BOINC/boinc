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

// The part of the BOINC app lib having to do with graphics.
// This code is NOT linked into the core client.

#include "config.h"

#ifdef _WIN32
#include "boinc_win.h"
extern void win_graphics_event_loop();
#endif

#ifdef __APPLE_CC__
#include "mac_app_opengl.h"
#endif

#ifndef _WIN32
#include <cstring>
#include <cstdarg>
#include "x_opengl.h"

#ifdef HAVE_PTHREAD
#include <pthread.h>
#include <sched.h>
#endif
#endif

#include "parse.h"
#include "util.h"
#include "app_ipc.h"
#include "error_numbers.h"
#include "filesys.h"
#include "boinc_api.h"
#include "graphics_api.h"


double boinc_max_fps = 30.;
double boinc_max_gfx_cpu_frac = 0.5;

#ifdef _WIN32
HANDLE hQuitEvent;
#endif

bool graphics_inited = false;

static void (*worker_main)();

#ifdef _WIN32
// glue routine for Windows
DWORD WINAPI foobar(LPVOID) {
    set_worker_timer();
    worker_main();
    return 0;
}
#endif
#ifdef _PTHREAD_H
void* foobar(void*) {
    set_worker_timer();
    worker_main();
    return 0;
}
#endif

int boinc_init_graphics(void (*worker)()) {
    BOINC_OPTIONS opt;
    opt.defaults();
    return boinc_init_options_graphics(opt, worker);
}

int boinc_init_options_graphics(BOINC_OPTIONS& opt, void (*_worker_main)()) {
    boinc_init_options_general(opt);
    worker_main = _worker_main;
#ifdef _WIN32

    // Create the event object used to signal between the
    // worker and event threads

    hQuitEvent = CreateEvent(
        NULL,     // no security attributes
        TRUE,     // manual reset event
        TRUE,     // initial state is signaled
        NULL      // object not named
    );

    DWORD threadId;

    // Create the graphics thread, passing it the graphics info
    // TODO: is it better to use _beginthreadex here?
    //
    worker_thread_handle = CreateThread(
        NULL, 0, foobar, 0, CREATE_SUSPENDED, &threadId
    );

    // raise priority of graphics thread (i.e. current thread)
    //
    HANDLE h = GetCurrentThread();
    SetThreadPriority(h, THREAD_PRIORITY_HIGHEST);

    // lower worker thread priority
    //
    SetThreadPriority(worker_thread_handle, THREAD_PRIORITY_LOWEST);

    // Start the worker thread
    //
    ResumeThread(worker_thread_handle);

    graphics_inited = true;
    win_graphics_event_loop();
#endif

#ifdef __APPLE_CC__
    OSErr     theErr = noErr;
    ThreadID    workerThreadID = 0;
    ThreadEntryUPP entry_proc;

    entry_proc = NewThreadEntryUPP( worker_main );

    // Create the thread in a suspended state
    theErr = NewThread ( kCooperativeThread, entry_proc,
        (void *)(&gi), 0, kNewSuspend | kCreateIfNeeded, NULL, &graphicsThreadID
    );
    if (theErr != noErr) return ERR_THREAD;

    // In theory we could do customized scheduling or install thread disposal routines here

    // Put the graphics event loop into the ready state
    SetThreadState(workerThreadID, kReadyThreadState, kNoThreadID);

    YieldToAnyThread();
    graphics_inited = true;
    mac_graphics_event_loop();
#endif

#ifdef _PTHREAD_H
    pthread_t worker_thread;
    pthread_attr_t worker_thread_attr;
    sched_param param;
    int retval, currentpolicy, minpriority;

    // make the worker thread low priority
    // (current thread, i.e. graphics thread, should remain high priority)
    //
    retval = pthread_attr_init(&worker_thread_attr);
    if (retval) return ERR_THREAD;
 
    retval = pthread_attr_getschedparam(&worker_thread_attr, &param);
    if (retval) return ERR_THREAD;
 
    // Note: this sets the scheduling policy for the worker thread to
    // be the same as the scheduling policy of the main thread.
    // This may not be a wise choice.
    //
    retval = pthread_attr_getschedpolicy(&worker_thread_attr, &currentpolicy);
    if (retval) return ERR_THREAD;
 
    minpriority = sched_get_priority_min(currentpolicy);
    if (minpriority == -1) return ERR_THREAD;
 
    param.sched_priority = minpriority;
    retval = pthread_attr_setschedparam(&worker_thread_attr, &param);
    if (retval) return ERR_THREAD;

    retval = pthread_create(&worker_thread, &worker_thread_attr, foobar, 0);
    if (retval) return ERR_THREAD;
    pthread_attr_destroy( &worker_thread_attr );
    graphics_inited = true;
    xwin_graphics_event_loop();
    pthread_exit(0);
#endif

    // normally we never get here
    return 0;
}

#ifdef _PTHREAD_H
extern "C" {
void glut_quit() {
    pthread_exit(0);
}
}
#endif

bool throttled_app_render(int x, int y, double t) {
    static double total_render_time = 0;
    static double time_until_render = 0;
    static double last_now = 0;
    static double elapsed_time = 0;
    double now, t0, t1, diff, frac, m;
    bool ok_to_render;

    ok_to_render = true;
    now = dtime();
    diff = now - last_now;
    last_now = now;
    if (diff > 1000) diff = 0;     // handle initial case

    // enforce frames/sec restriction
    //
    if (boinc_max_fps) {
        time_until_render -= diff;
        if (time_until_render < 0) {
            time_until_render += 1./boinc_max_fps;
        } else {
            ok_to_render = false;
        }
    }

    // enforce max CPU time restriction
    //
    if (boinc_max_gfx_cpu_frac) {
        elapsed_time += diff;
        if (elapsed_time) {
            frac = total_render_time/elapsed_time;
            if (frac > boinc_max_gfx_cpu_frac) {
                ok_to_render = false;
            }
        }
    }

    // render if allowed
    //
    if (ok_to_render) {
        if (boinc_max_gfx_cpu_frac) {
            boinc_calling_thread_cpu_time(t0, m);
        }
        app_graphics_render(x, y, t);
        if (boinc_max_gfx_cpu_frac) {
            boinc_calling_thread_cpu_time(t1, m);
            total_render_time += t1 - t0;
        }
        return true;
    }
    return false;
}
