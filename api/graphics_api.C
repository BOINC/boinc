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

#ifdef _WIN32
#include <afxwin.h>
extern DWORD WINAPI win_graphics_event_loop( LPVOID duff );
HANDLE graphics_threadh=NULL;
#endif

#ifdef __APPLE_CC__
#include "mac_app_opengl.h"
#endif

#include <string.h>
#include <stdarg.h>

#ifdef HAVE_PTHREAD
#include <pthread.h>
#endif

#include "parse.h"
#include "util.h"
#include "app_ipc.h"
#include "error_numbers.h"
#include "boinc_api.h"
#include "graphics_api.h"

double boinc_max_fps = 30.;
double boinc_max_gfx_cpu_frac = 0.5;

#ifdef _WIN32
HANDLE hQuitEvent;
#endif

GRAPHICS_INFO gi;
bool graphics_inited = false;

int boinc_init_graphics() {
#ifdef HAVE_GL_LIB
    FILE* f;
    int retval;

    f = boinc_fopen(GRAPHICS_DATA_FILE, "r");
    if (!f) {
        fprintf(stderr, "boinc_init(): can't open graphics data file\n");
        fprintf(stderr, "Using default graphics settings.\n");
        gi.refresh_period = 0.1; // 1/10th of a second
        gi.xsize = 640;
        gi.ysize = 480;
    } else {
        retval = parse_graphics_file(f, &gi);
        if (retval) {
            fprintf(stderr, "boinc_init(): can't parse graphics data file\n");
            return retval;
        }
        fclose(f);
    }

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
    graphics_threadh = CreateThread(
        NULL, 0, win_graphics_event_loop, &gi, CREATE_SUSPENDED, &threadId
    );

    // lower priority of worker thread (i.e. current thread)
    //
    HANDLE h = GetCurrentThread();
    SetThreadPriority(h, THREAD_PRIORITY_LOWEST);

    // Raise graphics thread priority
    //
    SetThreadPriority(graphics_threadh, THREAD_PRIORITY_HIGHEST);

    // Start the graphics thread
    //
    ResumeThread(graphics_threadh);
#endif

#ifdef __APPLE_CC__
    OSErr     theErr = noErr;
    ThreadID    graphicsThreadID = 0;
    ThreadEntryUPP entry_proc;
    
    entry_proc = NewThreadEntryUPP( mac_graphics_event_loop );
    
    // Create the thread in a suspended state
    theErr = NewThread ( kCooperativeThread, entry_proc,
        (void *)(&gi), 0, kNewSuspend | kCreateIfNeeded, NULL, &graphicsThreadID
    );
    if (theErr != noErr) return ERR_THREAD;
    
    // In theory we could do customized scheduling or install thread disposal routines here
    
    // Put the graphics event loop into the ready state
    SetThreadState(graphicsThreadID, kReadyThreadState, kNoThreadID);
    
    YieldToAnyThread();
#endif

#ifdef _PTHREAD_H
    pthread_t graphics_thread;
    pthread_attr_t graphics_thread_attr;

    pthread_attr_init( &graphics_thread_attr );
    retval = pthread_create( &graphics_thread, &graphics_thread_attr, p_graphics_loop, &gi );
    if (retval) return ERR_THREAD;
    pthread_attr_destroy( &graphics_thread_attr );
#endif
    
    graphics_inited = true;
#else
    graphics_inited = false;
#endif
    return !graphics_inited;
}

int boinc_finish_graphics() {
#ifdef _WIN32
    if (graphics_inited) {
        win_loop_done = TRUE;
        if (hQuitEvent != NULL) {
            WaitForSingleObject(hQuitEvent, 1000);  // Wait up to 1000 ms
        }
    }
#endif
	
    return 0;
}

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
            boinc_thread_cpu_time(t0, m);
        }
        app_graphics_render(x, y, t);
        if (boinc_max_gfx_cpu_frac) {
            boinc_thread_cpu_time(t1, m);
            total_render_time += t1 - t0;
        }
        return true;
    }
    return false;
}

#ifdef HAVE_GL_LIB
// Custom GL "Print" Routine
GLvoid glPrint(GLuint font, const char *fmt, ...) {
	/*
    char		text[256];			// Holds Our String
    va_list		ap;				// Pointer To List Of Arguments

    if (fmt == NULL)					// If There's No Text
        return;						// Do Nothing

    va_start(ap, fmt);					// Parses The String For Variables
        vsprintf(text, fmt, ap);			// And Converts Symbols To Actual Numbers
    va_end(ap);						// Results Are Stored In Text

    glPushAttrib(GL_LIST_BIT);				// Pushes The Display List Bits
    glListBase(font);					// Sets The Base Character
    glCallLists(strlen(text), GL_UNSIGNED_BYTE, text);	// Draws The Display List Text
    glPopAttrib();					// Pops The Display List Bits
	*/
}

// All Setup For OpenGL Goes Here
//
GLenum InitGL(GLvoid) {
    glShadeModel(GL_SMOOTH);				// Enable Smooth Shading
    glClearColor(0.0f, 0.0f, 0.0f, 0.5f);		// Black Background
    glClearDepth(1.0f);					// Depth Buffer Setup
    glEnable(GL_DEPTH_TEST);				// Enables Depth Testing
    glDepthFunc(GL_LEQUAL);				// The Type Of Depth Testing To Do
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);	// Really Nice Perspective Calculations
    return GL_NO_ERROR;					// Initialization Went OK
}

// Resize And Initialize The GL Window
// 
GLenum ReSizeGLScene(GLsizei width, GLsizei height) {
    GLenum err;
	glViewport(0,0,(int)width,(int)(height));

    err = glGetError();
    if (err) return err;

	app_graphics_resize(width,height);
    return GL_NO_ERROR;
}
#endif
