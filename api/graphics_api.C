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
DWORD WINAPI win_graphics_event_loop( LPVOID duff );
HANDLE graphics_threadh=NULL;
#endif


#include "graphics_api.h"
#include "error_numbers.h"

#include "parse.h"

#include <string.h>
#include <stdarg.h>

#ifdef __APPLE_CC__
#include "mac_app_opengl.h"
#endif

#ifdef HAVE_PTHREAD
#include <pthread.h>
#endif

extern GRAPHICS_INFO gi;

int boinc_init_opengl() {
#ifdef BOINC_APP_GRAPHICS
#ifdef _WIN32
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
        (void *)(&gi), 0, kNewSuspend | kCreateIfNeeded, NULL, &graphicsThreadID );
    if (theErr != noErr) return ERR_THREAD;
    
    // In theory we could do customized scheduling or install thread disposal routines here
    
    // Put the graphics event loop into the ready state
    SetThreadState(graphicsThreadID, kReadyThreadState, kNoThreadID);
    
    YieldToAnyThread();
#endif

#ifdef _PTHREAD_H
    pthread_t graphics_thread;
    pthread_attr_t graphics_thread_attr;
    int retval;

    pthread_attr_init( &graphics_thread_attr );
    retval = pthread_create( &graphics_thread, &graphics_thread_attr, p_graphics_loop, &gi );
    if (retval) return ERR_THREAD;
    pthread_attr_destroy( &graphics_thread_attr );
#endif
    
#endif
    
    return 0;
}

int boinc_finish_opengl() {
	
    return 0;
}

#ifdef BOINC_APP_GRAPHICS

GLvoid glPrint(GLuint font, const char *fmt, ...)    // Custom GL "Print" Routine
{
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

GLenum InitGL(GLvoid) {					// All Setup For OpenGL Goes Here
    GLenum err;
    
    glShadeModel(GL_SMOOTH);				// Enable Smooth Shading
    if (err=glGetError()) return err;
    
    glClearColor(0.0f, 0.0f, 0.0f, 0.5f);		// Black Background
    if (err=glGetError()) return err;
    
    glClearDepth(1.0f);					// Depth Buffer Setup
    if (err=glGetError()) return err;
    
    glEnable(GL_DEPTH_TEST);				// Enables Depth Testing
    if (err=glGetError()) return err;
    
    glDepthFunc(GL_LEQUAL);				// The Type Of Depth Testing To Do
    if (err=glGetError()) return err;
    
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);	// Really Nice Perspective Calculations
    if (err=glGetError()) return err;
   	

    return GL_NO_ERROR;					// Initialization Went OK
}

GLenum ReSizeGLScene(GLsizei width, GLsizei height) {	// Resize And Initialize The GL Window
    GLenum err;
    double aspect_ratio = 4.0/3.0;

    if (height<=0) height=1;		// Prevent A Divide By Zero By Making Height Equal One
    if (width<=0) width=1;

    if (height*aspect_ratio > width)
        glViewport(0,0,(int)width,(int)(width/aspect_ratio));	// Reset The Current Viewport
    else
        glViewport(0,0,(int)(height*aspect_ratio),(height));	// Reset The Current Viewport
    
    if (err=glGetError()) return err;
	app_resize(width,height);
    
    return GL_NO_ERROR;
}

#endif
