#include "graphics_api.h"
#include "error_numbers.h"

#include "parse.h"

#ifdef __APPLE_CC__
#include <Carbon/Carbon.h>
#include <CoreServices/CoreServices.h>
#include "mac_app_opengl.h"
#endif

extern GRAPHICS_INFO gi;

int boinc_init_opengl() {
#ifdef BOINC_APP_GRAPHICS
#ifdef __APPLE_CC__
    OSErr     theErr = noErr;
    ThreadID    graphicsThreadID = 0;
    ThreadEntryUPP entry_proc;
    
    entry_proc = NewThreadEntryUPP( mac_graphics_event_loop );
    
    // Create the thread in a suspended state
    theErr = NewThread ( kCooperativeThread, entry_proc,
        (void *)(&gi), 0, kNewSuspend, NULL, &graphicsThreadID );
    if (theErr != noErr) return ERR_THREAD;
    
    // In theory we could do customized scheduling or install thread disposal routines here
    
    // Put the graphics event loop into the ready state
    SetThreadState( graphicsThreadID, kReadyThreadState, kNoThreadID );
    
    YieldToAnyThread();
    
#endif
#endif
    
    return 0;
}

int boinc_finish_opengl() {
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
        if (match_tag(buf, "</graphics_info>")) return 0;
        else if (parse_int(buf, "<graphics_xsize>", gi->xsize)) continue;
        else if (parse_int(buf, "<graphics_ysize>", gi->ysize)) continue;
        else if (parse_int(buf, "<graphics_mode>", gi->graphics_mode)) continue;
        else if (parse_double(buf, "<graphics_refresh_period>", gi->refresh_period)) continue;
        else fprintf(stderr, "parse_core_file: unrecognized %s", buf);
    }
    return -1;
}

