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

#ifdef BOINC_APP_GRAPHICS
#ifdef __APPLE_CC__
    #include <OpenGL/gl.h>
#endif

#ifdef _WIN32
#include <gl\gl.h>            // Header File For The OpenGL32 Library
#include <gl\glu.h>            // Header File For The GLu32 Library
#include <gl\glaux.h>        // Header File For The Glaux Library
#endif
#endif

#include "graphics_api.h"
#include "error_numbers.h"

#include "parse.h"

#ifdef _WIN32
#include <afxwin.h>
DWORD WINAPI win_graphics_event_loop( LPVOID duff );
#endif

#ifdef __APPLE_CC__
#include "mac_app_opengl.h"
#endif

extern GRAPHICS_INFO gi;

int boinc_init_opengl() {
#ifdef BOINC_APP_GRAPHICS
#ifdef _WIN32
    DWORD threadId;
    HANDLE hThread;

    // Create the graphics thread, passing it the graphics info
    hThread = CreateThread(
        NULL, 0, win_graphics_event_loop, &gi, CREATE_SUSPENDED, &threadId
    );

    // Set it to idle priority
    SetThreadPriority(hThread, THREAD_PRIORITY_HIGHEST);

    // Start the graphics thread
    ResumeThread(hThread);
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
#endif
    
    return 0;
}

int boinc_finish_opengl() {
    return 0;
}

int write_graphics_file(FILE* f, GRAPHICS_INFO* gi) {
    fprintf(f,
        "<graphics_info>\n"
        "    <graphics_xsize>%d</graphics_xsize>\n"
        "    <graphics_ysize>%d</graphics_ysize>\n"
        "    <graphics_mode>%d</graphics_mode>\n"
        "    <graphics_refresh_period>%f</graphics_refresh_period>\n"
        "</graphics_info>\n",
        gi->xsize,
        gi->ysize,
        gi->graphics_mode,
        gi->refresh_period
    );

    return 0;
}

int parse_graphics_file(FILE* f, GRAPHICS_INFO* gi) {
    char buf[256];
    while (fgets(buf, 256, f)) {
        if (match_tag(buf, "<graphics_info>")) continue;
        if (match_tag(buf, "</graphics_info>")) return 0;
        else if (parse_int(buf, "<graphics_xsize>", gi->xsize)) continue;
        else if (parse_int(buf, "<graphics_ysize>", gi->ysize)) continue;
        else if (parse_int(buf, "<graphics_mode>", gi->graphics_mode)) continue;
        else if (parse_double(buf, "<graphics_refresh_period>", gi->refresh_period)) continue;
        else fprintf(stderr, "parse_graphics_file: unrecognized %s", buf);
    }
    return -1;
}

#ifdef BOINC_APP_GRAPHICS

GLvoid glPrint(GLuint font, const char *fmt, ...)	// Custom GL "Print" Routine
{
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
}

#endif
