// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2024 University of California
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
#include <SystemConfiguration/SystemConfiguration.h>

#include <stdio.h>
#include <pthread.h>

#include "gui_rpc_client.h"
#include "util.h"
#include "mac_util.h"
#include "shmem.h"

#define CREATE_LOG 0
#define USE_TIMER 0

#if CREATE_LOG
void print_to_log_file(const char *format, ...);
#else
#define print_to_log_file(...)
#endif

pid_t parentPid;
int GFX_PidFromScreensaver = 0;
pthread_t MonitorParentThread = 0;
bool quit_MonitorParentThread = false;

// struct ss_shmem_data must be kept in sync in these files:
// screensaver.cpp
// gfx_switcher.cpp
// gfx_cleanup.mm
// graphics2_unix.cpp
struct ss_shmem_data {
    pid_t gfx_pid;
    int gfx_slot;
    int major_version;
    int minor_version;
    int release;
};

static struct ss_shmem_data* ss_shmem = NULL;

#if USE_TIMER
time_t startTime = 0;
time_t endTime = 0;
time_t elapsedTime = 0;
#endif

void killGfxApp(pid_t thePID) {
#if 0
    print_to_log_file("in gfx_cleanup: killGfxApp()");
    kill(thePID, SIGKILL);
#else
    char buf[256];
    char userName[64];
    RPC_CLIENT *rpc;
    int retval;
    std::string msg;

    chdir("/Library/Application Support/BOINC Data");
    safe_strcpy(buf, "");
    read_gui_rpc_password(buf, msg);

    rpc = new RPC_CLIENT;
    if (rpc->init(NULL)) {     // Initialize communications with Core Client
        fprintf(stderr, "in gfx_cleanup: killGfxApp(): rpc->init(NULL) failed");
        return;
    }
    if (strlen(buf)) {
        retval = rpc->authorize(buf);
        if (retval) {
            fprintf(stderr, "in gfx_cleanup: killGfxApp(): authorization failure: %d\n", retval);
            rpc->close();
            return;
        }
    }

    CFStringRef cf_gUserName = SCDynamicStoreCopyConsoleUser(NULL, NULL, NULL);
    CFStringGetCString(cf_gUserName, userName, sizeof(userName), kCFStringEncodingUTF8);

    retval = rpc->run_graphics_app("stop", thePID, userName);
    print_to_log_file("in gfx_cleanup: killGfxApp(): rpc->run_graphics_app(stop) returned retval=%d", retval);

    // Wait until graphics app has exited before closing our own black fullscreen
    // window to prevent an ugly white flash [see comment in main() below].
    int i;
    pid_t p = 0;
    for (i=0; i<100; ++i) {
        boinc_sleep(0.1);
        p = thePID;
        // On OS 10.15+ (Catalina), it might be more efficient to get this from shared memory
        retval = rpc->run_graphics_app("test", p, userName);
        if (retval || (p==0)) break;
    }
 print_to_log_file("in gfx_cleanup: killGfxApp(%d): rpc->run_graphics_app(test) returned pid %d, retval %d when i = %d", thePID, p, retval, i);

    // Graphics apps called by screensaver or Manager (via Show
    // Graphics button) now write files in their slot directory as
    // the logged in user, not boinc_master. This ugly hack tells
    // BOINC client to fix all ownerships in this slot directory
    char shmem_name[MAXPATHLEN];
    snprintf(shmem_name, sizeof(shmem_name), "/tmp/boinc_ss_%s", userName);
    retval = attach_shmem_mmap(shmem_name, (void**)&ss_shmem);
    if (ss_shmem) {
        rpc->run_graphics_app("stop", ss_shmem->gfx_slot, "");
        ss_shmem->gfx_slot = -1;
        ss_shmem->major_version = 0;
        ss_shmem->minor_version = 0;
        ss_shmem->release = 0;
    }

    rpc->close();
#endif
    return;
}

void * MonitorParent(void* param) {
    print_to_log_file("in gfx_cleanup: Starting MonitorParent");
    while (true) {
        boinc_sleep(0.25);  // Test every 1/4 second
        if (getppid() != parentPid) {
            if (GFX_PidFromScreensaver) {
                killGfxApp(GFX_PidFromScreensaver);
            }
            if (quit_MonitorParentThread) {
                return 0;
            }
            print_to_log_file("in gfx_cleanup: parent died, exiting (child) after handling %d",GFX_PidFromScreensaver);
#if USE_TIMER
            endTime = time(NULL);
            elapsedTime = endTime - startTime;
            print_to_log_file("elapsed time=%d", (int) elapsedTime);
#endif
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
    int retval __attribute__((unused)) = 0;
    char buf[256];

    retval = pthread_create(&MonitorParentThread, NULL, MonitorParent, 0);

    while (true) {
        fgets(buf, sizeof(buf), stdin);
        print_to_log_file("in gfx_cleanup: parent sent %d to child buf=%s", GFX_PidFromScreensaver, buf);
        if (feof(stdin)) {
            print_to_log_file("in gfx_cleanup: got eof");
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
        print_to_log_file("in gfx_cleanup: parent sent %d to child buf=%s", GFX_PidFromScreensaver, buf);
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
    print_to_log_file("Entered gfx_cleanup");
#if USE_TIMER
    startTime = time(NULL);
#endif
    parentPid = getppid();

    // Under MacOS 14.0, the legacyScreenSaver continues to run after the
    // screensaver is dismissed. Our code elsewhere now kills it, but if
    // that were to fail this black cover would block the user from
    // accessing the desktop, so don't use the cover in MacOS 14 for now.

    bool cover_gfx_window = (compareOSVersionTo(10, 13) >= 0) &&
                                (compareOSVersionTo(10, 14) < 0);

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

    print_to_log_file("exiting gfx_cleanup after handling %d",GFX_PidFromScreensaver);
#if USE_TIMER
    endTime = time(NULL);
    elapsedTime = endTime - startTime;
    print_to_log_file("elapsed time=%d", (int) elapsedTime);
#endif

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
