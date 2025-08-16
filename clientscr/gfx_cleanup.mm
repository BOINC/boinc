// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2025 University of California
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
#define FOR_TESTING_ONLY 0

#define CREATE_LOG 0

#if CREATE_LOG
void print_to_log_file(const char *format, ...);
#else
#define print_to_log_file(...)
#endif

//From mscglutfix.m:
// The standard ScreenSaverView class actually sets the window
// level to 2002, not the 1000 defined by NSScreenSaverWindowLevel
// and kCGScreenSaverWindowLevel
#define RealSaverLevel 2002
#define gfx_app_widowlevel (RealSaverLevel+20)


NSRect myWindowRect;
NSWindow* myWindow = NULL;
RPC_CLIENT *rpc;
pid_t parentPid;
int GFX_PidFromScreensaver = 0;
pthread_t MonitorParentThread = 0;
pthread_t MonitorStdinThread = 0;
bool quit_MonitorParentThread = false;
char userName[64];

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

#if FOR_TESTING_ONLY
typedef void (*handler_t)(int, siginfo_t*, void *);

void boinc_catch_signal(int signal, siginfo_t*, void *) {
    print_to_log_file("caught signal %s", signal);
}

extern "C" void boinc_set_signal_handler(int sig, handler_t handler) {
    struct sigaction temp;
    sigaction(sig, NULL, &temp);
    temp.sa_sigaction = handler;
    sigaction(sig, &temp, NULL);
}
#endif

void killGfxApp(pid_t thePID) {
    print_to_log_file("in gfx_cleanup: killGfxApp()");
#if 0
    kill(thePID, SIGKILL);
#else
    char buf[256];

    chdir("/Library/Application Support/BOINC Data");
    safe_strcpy(buf, "");
#if CREATE_LOG
    int retval =
#endif
    rpc->run_graphics_app("stop", thePID, userName);
    print_to_log_file("in gfx_cleanup: killGfxApp(): rpc->run_graphics_app(stop) returned retval=%d", retval);

    // Wait until graphics app has exited before closing our own black fullscreen
    // window to prevent an ugly white flash [see comment in main() below].
    int i;
    for (i=0; i<100; ++i) {
        boinc_sleep(0.1);
        if (ss_shmem->gfx_pid == 0) break;
    }

    // Graphics apps called by screensaver or Manager (via Show
    // Graphics button) now write files in their slot directory as
    // the logged in user, not boinc_master. This ugly hack tells
    // BOINC client to fix all ownerships in this slot directory
    if (ss_shmem) {
        rpc->run_graphics_app("stop", ss_shmem->gfx_slot, "");
        ss_shmem->gfx_slot = -1;
        ss_shmem->major_version = 0;
        ss_shmem->minor_version = 0;
        ss_shmem->release = 0;
    }
#endif
    return;
}

void * MonitorParent(void* param) {
    print_to_log_file("in gfx_cleanup: Starting MonitorParent: parentpid=%d", parentPid);
    while (true) {
        boinc_sleep(0.25);  // Test every 1/4 second
        if (getppid() != parentPid) {
            print_to_log_file("in gfx_cleanup: MonitorParent: getppd=%d, parentpid=%d", getppid(), parentPid);
            if (GFX_PidFromScreensaver) {
                print_to_log_file("in gfx_cleanup: MonitorParent: calling killGfxApp for GFX_PidFromScreensaver %d", GFX_PidFromScreensaver);
                killGfxApp(GFX_PidFromScreensaver);
            }
            if (quit_MonitorParentThread) {
                return 0;
            }

            rpc->close();
            print_to_log_file("in gfx_cleanup: MonitorParent calling exit(0)");
            exit(0);
        }
    }

}
void * MonitorStdin(void* param) {
    char buf[256];
    print_to_log_file("in gfx_cleanup: Starting MonitorStdin");

    while (true) {
        fgets(buf, sizeof(buf), stdin);
        print_to_log_file("in gfx_cleanup: buf=%s", buf);
        if (feof(stdin)) {
            print_to_log_file("in gfx_cleanup: got eof");
            break;
        }
        if (ferror(stdin) && (errno != EINTR)) {
            fprintf(stderr, "in gfx_cleanup: fgets got error %d %s", errno, strerror(errno));
            print_to_log_file("in gfx_cleanup: fgets got error %d %s", errno, strerror(errno));
            break;
        }

        if (!strcmp(buf, "Quit\n")) {
            print_to_log_file("in gfx_cleanup: parent sent Quit to child");
            break;
        }
        GFX_PidFromScreensaver = atoi(buf);

        // Under MacOS 14.0, the legacyScreenSaver continues to run after the
        // screensaver is dismissed. Our code elsewhere now kills it, but if
        // that were to fail this black cover would block the user from
        // accessing the desktop, so create the cover only when we have launched
        // a graphics app and remove it right after the grapics app exits.
        if (GFX_PidFromScreensaver) {
            if (!myWindow) {
                dispatch_sync(dispatch_get_main_queue(), ^{
                    myWindow = [[NSWindow alloc] initWithContentRect:myWindowRect
                                                                   styleMask:NSWindowStyleMaskBorderless
                                                                     backing:NSBackingStoreBuffered
                                                                       defer:NO];
                    [myWindow setBackgroundColor:[NSColor blackColor]];
                    [myWindow setLevel:gfx_app_widowlevel+20];    // Slightly above the graphics app's window
                    [myWindow orderFrontRegardless];
                });
                print_to_log_file("in gfx_cleanup: MonitorStdin created covering window");
            }
        } else if (ss_shmem->gfx_pid == 0) {
            print_to_log_file("in gfx_cleanup: MonitorStdin closing covering window");
            dispatch_sync(dispatch_get_main_queue(), ^{
                [myWindow close];
                myWindow = NULL;
            });
        }
    }
    print_to_log_file("in gfx_cleanup: MonitorStdin exited while loop");

    if (GFX_PidFromScreensaver) {
        print_to_log_file("in gfx_cleanup: MonitorStdin calling killGfxApp");
        killGfxApp(GFX_PidFromScreensaver);
    }

    quit_MonitorParentThread = true;

    rpc->close();
//    [NSApp stop:self];

print_to_log_file("in gfx_cleanup: MonitorStdin calling exit(0)");
    exit(0);


}

@interface AppDelegate : NSObject <NSApplicationDelegate, NSUserNotificationCenterDelegate>
{
}
@end


@implementation AppDelegate

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification
{
    int retval __attribute__((unused)) = 0;

    retval = pthread_create(&MonitorParentThread, NULL, MonitorParent, 0);
    retval = pthread_create(&MonitorStdinThread, NULL, MonitorStdin, 0);
}
@end

int main(int argc, char* argv[]) {
    print_to_log_file("Entered gfx_cleanup");
    parentPid = getppid();

#if FOR_TESTING_ONLY
    freopen("/Users/Shared/stderr_gfx_cleanup.txt", "a", stderr);
    freopen("/Users/Shared/stdout_gfx_cleanup.txt", "a", stdout);

    for (int i=1; i<NSIG; ++i) {
        boinc_set_signal_handler(i, boinc_catch_signal);
    }
#endif
    // Create shared app instance
    [NSApplication sharedApplication];

    // Because prpject graphics applications under OS 10.13+ draw to an IOSurface,
    // the application's own window is white, but is normally covered by the
    // ScreensaverEngine's window. If the ScreensaverEngine exits without first
    // calling [ScreenSaverView stopAnimation], the white fullscreen window will
    // briefly be visible until we kill the graphics app, causing an ugly and
    // annoying white flash. So we hide that with our own black fullscreen window
    // to prevent the white flash.
    NSArray *allScreens = [NSScreen screens];
    myWindowRect = [ allScreens[0] frame ];

    CFStringRef cf_gUserName = SCDynamicStoreCopyConsoleUser(NULL, NULL, NULL);
    CFStringGetCString(cf_gUserName, userName, sizeof(userName), kCFStringEncodingUTF8);

    char shmem_name[MAXPATHLEN];
    snprintf(shmem_name, sizeof(shmem_name), "/tmp/boinc_ss_%s", userName);
    attach_shmem_mmap(shmem_name, (void**)&ss_shmem);

    rpc = new RPC_CLIENT;
    if (rpc->init(NULL)) {     // Initialize communications with Core Client
        fprintf(stderr, "in gfx_cleanup: killGfxApp(): rpc->init(NULL) failed");
        print_to_log_file("in gfx_cleanup: killGfxApp(): rpc->init(NULL) failed");
        return -1;
    }

    AppDelegate *myDelegate = [[AppDelegate alloc] init];
    [ NSApp setDelegate:myDelegate];

    [NSApp run];

    print_to_log_file("exiting gfx_cleanup after handling %d",GFX_PidFromScreensaver);

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
    f = fopen("/Users/Shared/test_log_gfx_cleanup.txt", "a");
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
    fflush(f);
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
