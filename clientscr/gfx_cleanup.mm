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
//
//  gfx_cleanup.mm
//
// Used by screensaver to work around a bug in OS 10.15 Catalina
// - Detects when ScreensaverEngine exits without calling [ScreenSaverView stopAnimation]
// - If that happens, it sends RPC to BOINC client to kill current graphics app.
// Note: this can rarely happen in earlier versions, but is of main concern under 
// OS 10.13 and later, where it can cause an ugly white full-screen display
//
// Called by CScreensaver via popen(path, "w")
//

#import <Cocoa/Cocoa.h>

#include <stdio.h>
#include <pthread.h>

#include "gui_rpc_client.h"
#include "util.h"
#include "mac_util.h"

#define CREATE_LOG 0
#define USE_TIMER 0

#if CREATE_LOG
void print_to_log_file(const char *format, ...);
#endif

pid_t parentPid;
int GFX_PidFromScreensaver = 0;
pthread_t MonitorParentThread = 0;
bool quit_MonitorParentThread = false;

#if USE_TIMER
time_t startTime = 0;
time_t endTime = 0;
time_t elapsedTime = 0;
#endif

void killGfxApp(pid_t thePID) {
    char passwd_buf[256];
    RPC_CLIENT *rpc;
    int retval;
    
    chdir("/Library/Application Support/BOINC Data");
    safe_strcpy(passwd_buf, "");
    read_gui_rpc_password(passwd_buf);
    
    rpc = new RPC_CLIENT;
    if (rpc->init(NULL)) {     // Initialize communications with Core Client
        fprintf(stderr, "in gfx_cleanup: killGfxApp(): rpc->init(NULL) failed");
        return;
    }
    if (strlen(passwd_buf)) {
        retval = rpc->authorize(passwd_buf);
        if (retval) {
            fprintf(stderr, "in gfx_cleanup: killGfxApp(): authorization failure: %d\n", retval);
            rpc->close();
            return;
        }
    }

    retval = rpc->run_graphics_app(0, thePID, "stop");
    // fprintf(stderr, "in gfx_cleanup: killGfxApp(): rpc->run_graphics_app() returned retval=%d", retval);   
 
    rpc->close();
}

void * MonitorParent(void* param) {
    // fprintf(stderr, "in gfx_cleanup: Starting MonitorParent");
    while (true) {
        boinc_sleep(0.25);  // Test every 1/4 second
        if (getppid() != parentPid) {
#if USE_TIMER
            endTime = time(NULL);
#endif
            if (GFX_PidFromScreensaver) {
                killGfxApp(GFX_PidFromScreensaver);
            }
            if (quit_MonitorParentThread) {
                return 0;
            }
            // fprintf(stderr, "in gfx_cleanup: parent died, exiting (child) after handling %d, elapsed time=%d",GFX_PidFromScreensaver, (int) elapsedTime);
            exit(0);
        }
    }

}

NSWindow* myWindow;

@interface AppDelegate : NSObject <NSApplicationDelegate, NSUserNotificationCenterDelegate>
{
}
@end


@implementation AppDelegate

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification
{
    int retval = 0;
    char buf[256];

    retval = pthread_create(&MonitorParentThread, NULL, MonitorParent, 0);

    while (true) {
        fgets(buf, sizeof(buf), stdin);
        // fprintf(stderr, "in gfx_cleanup: parent sent %d to child buf=%s", GFX_PidFromScreensaver, buf);
        if (feof(stdin)) {
            // fprintf(stderr, "in gfx_cleanup: got eof");
            break;
        }
        if (ferror(stdin) && (errno != EINTR)) {
            fprintf(stderr, "in gfx_cleanup: fgets got error %d %s", errno, strerror(errno));
            break;
        }

        if (!strcmp(buf, "Quit\n")) {
            break;
        }
        GFX_PidFromScreensaver = atoi(buf);
        // fprintf(stderr, "in gfx_cleanup: parent sent %d to child buf=%s", GFX_PidFromScreensaver, buf);
    }

    if (GFX_PidFromScreensaver) {
        killGfxApp(GFX_PidFromScreensaver);
    }
    
    quit_MonitorParentThread = true;
    
//    [NSApp stop:self];
    exit(0);
    
}
@end

int main(int argc, char* argv[]) {
    // fprintf(stderr, "Entered gfx_cleanup");
#if USE_TIMER
    startTime = time(NULL);
#endif
    parentPid = getppid();

    bool cover_gfx_window = (compareOSVersionTo(10, 13) >= 0);

    // Create shared app instance
    [NSApplication sharedApplication];

     // Because prpject graphics applications under OS 10.13+ draw to an IOSurface, 
    // the application's own window is white, but is normally covered by the 
    // ScreensaverEngine's window. If the ScreensaverEngine exits without first
    // calling [ScreenSaverView stopAnimation], the white fullscreen window will 
    // briefly be visible until we kill the graphics app, causing an ugly and 
    // annoying white flash. So we hide that with our own black fullscreen window
    // to prevent the white flash.
   if (cover_gfx_window) {
        NSArray *allScreens = [NSScreen screens];
        NSRect windowRect = [ allScreens[0] frame ];
        NSUInteger windowStyle = NSWindowStyleMaskBorderless;
        NSWindow *myWindow = [[NSWindow alloc] initWithContentRect:windowRect
                                                       styleMask:windowStyle
                                                         backing:NSBackingStoreBuffered
                                                           defer:NO];
        [myWindow setBackgroundColor:[NSColor blackColor]];
        [ myWindow setLevel:kCGOverlayWindowLevel ];    // slightly above the graphics app's window
        [myWindow orderFrontRegardless];
    }

    AppDelegate *myDelegate = [[AppDelegate alloc] init]; 
    [ NSApp setDelegate:myDelegate];
    
    [NSApp run];

#if USE_TIMER
    endTime = time(NULL);
    elapsedTime = endTime - startTime;
#endif
    // fprintf(stderr, "exiting gfx_cleanup after handling %d, elapsed time=%d",GFX_PidFromScreensaver, (int)elapsedTime);

    return 0;
}

// print_to_log_file.c

#if CREATE_LOG
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#include <time.h>
#include <string.h>

#ifdef unix
//#include <unistd.h>
#endif

void strip_cr(char *buf);

#endif    // CREATE_LOG

#if CREATE_LOG
// print_to_log_file - use for debugging.
// prints time stamp plus a formatted string to log file.
// calling syntax: same as printf.
void print_to_log_file(const char *format, ...) {
    FILE *f;
    va_list args;
    char buf[256];
    time_t t;
#if 0
    strcpy(buf, "/Library/Application Support/test_log.txt");
#else
    strcpy(buf, getenv("HOME"));
    strcat(buf, "/Library/Application Support/BOINC/test_log_gfx_cleanup.txt");
#endif
    f = fopen(buf, "a");
//    f = fopen("/Library/Games_Demo/test_log.txt", "a");
    if (!f) return;

    time(&t);
    strcpy(buf, asctime(localtime(&t)));
    strip_cr(buf);

    fputs(buf, f);
    fputs("   ", f);

    va_start(args, format);
    vfprintf(f, format, args);
    va_end(args);
    
    fputs("\n", f);

    fclose(f);
}

void strip_cr(char *buf)
{
    char *theCR;

    theCR = strrchr(buf, '\n');
    if (theCR)
        *theCR = '\0';
    theCR = strrchr(buf, '\r');
    if (theCR)
        *theCR = '\0';
}
#else
void print_to_log_file(const char *, ...) {}

#endif    // CREATE_LOG


#ifdef __cplusplus
extern "C" {
#endif

void PrintBacktrace(void) {
// Dummy routine to satisfy linker
}

#ifdef __cplusplus
}
#endif
