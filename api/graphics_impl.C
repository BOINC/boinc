// Berkeley Open Infrastructure for Network Computing
// http://boinc.berkeley.edu
// Copyright (C) 2005 University of California
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

// The part of the BOINC API having to do with graphics.
// This is the implementation level,
// which doesn't directly refer to boinc_init_options()
// and thus can be put in a shared library,
// separate from the application.

#include "config.h"

#if defined(_WIN32) && !defined(__STDWX_H__) && !defined(_BOINC_WIN_) && !defined(_AFX_STDAFX_H_)
#include "boinc_win.h"
#endif

#ifdef _WIN32
extern void win_graphics_event_loop();
#else
#include <cstring>
#include <cstdarg>
#include <pthread.h>
#include <sched.h>
#include <signal.h>
#include "x_opengl.h"
#endif

#include "parse.h"
#include "util.h"
#include "app_ipc.h"
#include "error_numbers.h"
#include "filesys.h"
#include "graphics_api.h"
#include "graphics_impl.h"


double boinc_max_fps = 30.;
double boinc_max_gfx_cpu_frac = 0.5;

#ifdef _WIN32
HANDLE hQuitEvent;
#endif

BOINC_MAIN_STATE* g_bmsp;
static WORKER_FUNC_PTR worker_main;

#ifndef _WIN32
pthread_t worker_thread;
pthread_t graphics_thread;
#endif

#ifdef _WIN32
DWORD WINAPI foobar(LPVOID) {
#else
void* foobar(void*) {
#endif
    g_bmsp->set_worker_timer_hook();
    worker_main();
    return 0;
}

// the following function can be in a shared library,
// so it calls boinc_init_options_general() via a pointer instead of directly
//
int boinc_init_graphics_impl(WORKER_FUNC_PTR worker, BOINC_MAIN_STATE* bmsp) {
    BOINC_OPTIONS opt;
    boinc_options_defaults(opt);
    return boinc_init_options_graphics_impl(opt, worker, bmsp);
}

int start_worker_thread(WORKER_FUNC_PTR _worker_main) {
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

    // Create and start worker thread
    //
    worker_thread_handle = CreateThread(
        NULL, 0, foobar, 0, CREATE_SUSPENDED, &threadId
    );
    ResumeThread(worker_thread_handle);

#else

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

    // initialize ID of calling thread (the graphics-thread!)
    graphics_thread = pthread_self();
    
    retval = pthread_create(&worker_thread, &worker_thread_attr, foobar, 0);
    if (retval) return ERR_THREAD;
    pthread_attr_destroy( &worker_thread_attr );

    // block SIGLARM, so that the worker thread will be forced to handle it
    //
    sigset_t mask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGALRM);
    sigprocmask(SIG_BLOCK, &mask, NULL);
#endif
    return 0;
}

int boinc_init_options_graphics_impl(
    BOINC_OPTIONS& opt,
    WORKER_FUNC_PTR _worker_main,
    BOINC_MAIN_STATE* bmsp
) {
    int retval;

    g_bmsp = bmsp;
    retval = g_bmsp->boinc_init_options_general_hook(opt);
    if (retval) return retval;
    if (_worker_main) {
        retval = start_worker_thread(_worker_main);
        if (retval) return retval;
    }
#ifdef _WIN32
    win_graphics_event_loop();
	fprintf(stderr, "Graphics event loop returned\n");
#else
    xwin_graphics_event_loop();
	fprintf(stderr, "Graphics event loop returned\n");
    pthread_exit(0);
#endif

    // normally we never get here
    return 0;
}

#ifndef _WIN32
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
    double now, t0, t1, diff, frac;
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
            boinc_calling_thread_cpu_time(t0);
        }
        app_graphics_render(x, y, t);
        if (boinc_max_gfx_cpu_frac) {
            boinc_calling_thread_cpu_time(t1);
            total_render_time += t1 - t0;
        }
        return true;
    }
    return false;
}

const char *BOINC_RCSID_6e92742852 = "$Id$";
