// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2019 University of California
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

// The part of the BOINC API having to do with graphics.
// This is the implementation level,
// which doesn't directly refer to boinc_init_options()
// and thus can be put in a shared library,
// separate from the application.

// DEPRECATED - use separate graphics app

#ifdef _WIN32
#include "boinc_win.h"
extern void win_graphics_event_loop();
#else
#include "config.h"
#include <cstring>
#include <cstdarg>
#include <pthread.h>
#include <sched.h>
#include <signal.h>
#include <sys/resource.h>
#include "x_opengl.h"
#endif

#include "boinc_api.h"
#include "diagnostics.h"
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
    g_bmsp->start_timer_thread_hook();
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

static int start_worker_thread(
    WORKER_FUNC_PTR _worker_main, BOINC_OPTIONS& options
) {
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

    // set worker stack size if specified
    //
    if (options.worker_thread_stack_size) {
        pthread_attr_setstacksize(
            &worker_thread_attr, options.worker_thread_stack_size
        );
    }

    retval = pthread_create(&worker_thread, &worker_thread_attr, foobar, 0);
    if (retval) return ERR_THREAD;
    pthread_attr_destroy( &worker_thread_attr );

    block_sigalrm();
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
        retval = start_worker_thread(_worker_main, opt);
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
    bool ok_to_render = true;

    now = dtime();
    diff = now - last_now;
    last_now = now;

    // ignore interval if negative or more than 1 second
    //
    if ((diff<0) || (diff>1.0)) {
        diff = 0;
    }

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

void get_window_title(APP_INIT_DATA& aid, char* buf, int len) {
    if (aid.app_version) {
        snprintf(buf, len,
            "%s version %.2f [workunit: %s]",
            aid.app_name, aid.app_version/100.0, aid.wu_name
        );
    } else {
        snprintf(buf, len,
            "%s [workunit: %s]",
            aid.app_name, aid.wu_name
        );
    }
}
