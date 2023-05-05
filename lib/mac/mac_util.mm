// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2017 University of California
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

//  mac_util.mm

#include "mac_util.h"
#import <Cocoa/Cocoa.h>
#define DLOPEN_NO_WARN
#include <mach-o/dyld.h>
#include <dlfcn.h>


// Returns time in seconds since system was booted
//
double getTimeSinceBoot() {
    return [[NSProcessInfo processInfo] systemUptime];
}


void getPathToThisApp(char* pathBuf, size_t bufSize) {
    // Get the app's main bundle
    NSBundle *main = [NSBundle mainBundle];
    NSString *thePath = [main bundlePath];
    strlcpy(pathBuf, [thePath UTF8String], bufSize);
}


void BringAppToFront() {
    [ [NSRunningApplication currentApplication] activateWithOptions:NSApplicationActivateIgnoringOtherApps | NSApplicationActivateAllWindows ];
}


void BringAppWithPidToFront(pid_t pid) {
    NSRunningApplication * theRunningApp = [NSRunningApplication runningApplicationWithProcessIdentifier:pid];
    if (theRunningApp) {
        [ theRunningApp activateWithOptions:NSApplicationActivateIgnoringOtherApps | NSApplicationActivateAllWindows ];
    }
}


pid_t getActiveAppPid() {
    NSArray * runningApps = [[NSWorkspace sharedWorkspace] runningApplications];
    unsigned long i;
    unsigned long n = [ runningApps count ];
    for (i=0; i<n; i++) {
        NSRunningApplication * theApp = (NSRunningApplication *)[ runningApps objectAtIndex:i ];
        if ([ theApp isActive ]) {
            return [theApp processIdentifier];
        }
    }
    return 0;
}


pid_t getPidIfRunning(char * bundleID) {
    NSString *NSBundleID = [[NSString alloc] initWithUTF8String:bundleID];
    NSArray * runningApps = [NSRunningApplication runningApplicationsWithBundleIdentifier:NSBundleID];
    if (runningApps) {
        if ([runningApps count] > 0) {
            return [((NSRunningApplication *)[runningApps firstObject]) processIdentifier];
        }
    }
    return 0;
}


// Find the path to the app with the bundle identifier and (optionally) creator code.
// The creator code can be NULL.
OSStatus GetPathToAppFromID(OSType creator, CFStringRef bundleID, char *path, size_t maxLen) {
    CFURLRef                appURL = NULL;
    OSErr                   err;

    // We must launch the System Events application for the target user
    err = noErr;
    *path = '\0';

    // LSCopyApplicationURLsForBundleIdentifier is not available before OS 10.10
    CFArrayRef (*LSCopyAppURLsForBundleID)(CFStringRef, CFErrorRef) = NULL;
    void *LSlib = dlopen("/System/Library/Frameworks/CoreServices.framework/Frameworks/LaunchServices.framework/LaunchServices", RTLD_NOW | RTLD_NODELETE);
    if (LSlib) {
        LSCopyAppURLsForBundleID = (CFArrayRef(*)(CFStringRef, CFErrorRef)) dlsym(LSlib, "LSCopyApplicationURLsForBundleIdentifier");
    }
    if (LSCopyAppURLsForBundleID == NULL) {
        err = fnfErr;
    }

#if __MAC_OS_X_VERSION_MIN_REQUIRED < 101000
    if (err != noErr) {     // LSCopyAppURLsForBundleID == NULL
        //LSFindApplicationForInfo is deprecated in OS 10.10, so may not be available in the future
        OSStatus (*LSFindAppForInfo)(OSType, CFStringRef, CFStringRef, FSRef*, CFURLRef*) = NULL;
        if (LSlib) {
            LSFindAppForInfo = (OSStatus(*)(OSType, CFStringRef, CFStringRef, FSRef*, CFURLRef*))
                        dlsym(LSlib, "LSFindApplicationForInfo");
        }
        if (LSFindAppForInfo == NULL) {
            if (LSlib) dlclose(LSlib);
            return fnfErr;
        }
        err = (*LSFindAppForInfo)(creator, bundleID, NULL, NULL, &appURL);
    } else  // if (LSCopyApplicationURLsForBundleIdentifier != NULL)
#endif
    {
        if (err == noErr) {
            CFArrayRef appRefs = (*LSCopyAppURLsForBundleID)(bundleID, NULL);
            if (appRefs == NULL) {
                err = fnfErr;
            } else {
                appURL = (CFURLRef)CFArrayGetValueAtIndex(appRefs, 0);
                CFRetain(appURL);
                CFRelease(appRefs);
            }
        }
        if (err != noErr) {
            if (LSlib) dlclose(LSlib);
            return err;
        }
    }   // end if (LSCopyApplicationURLsForBundleIdentifier != NULL)

    if (err == noErr) {
        CFStringRef CFPath = CFURLCopyFileSystemPath(appURL, kCFURLPOSIXPathStyle);
        CFStringGetCString(CFPath, path, maxLen, kCFStringEncodingUTF8);
        CFRelease(CFPath);
    }
    if (appURL) {
        CFRelease(appURL);
    }
    if (LSlib) dlclose(LSlib);
    return err;
}

// Test OS version number on all versions of OS X without using deprecated Gestalt
// compareOSVersionTo(x, y) returns:
// -1 if the OS version we are running on is less than x.y
//  0 if the OS version we are running on is equal to x.y
// +1 if the OS version we are running on is lgreater than x.y
int compareOSVersionTo(int toMajor, int toMinor) {
    static SInt32 major = -1;
    static SInt32 minor = -1;

    if (major < 0) {
        char vers[100], *p1 = NULL;
        FILE *f;
        vers[0] = '\0';
        f = popen("sw_vers -productVersion", "r");
        if (f) {
            fscanf(f, "%s", vers);
            pclose(f);
        }
        if (vers[0] == '\0') {
            fprintf(stderr, "popen(\"sw_vers -productVersion\" failed\n");
            fflush(stderr);
            return 0;
        }
        // Extract the major system version number
        major = atoi(vers);
        // Extract the minor system version number
        p1 = strchr(vers, '.');
        minor = atoi(p1+1);
    }

    if (major < toMajor) return -1;
    if (major > toMajor) return 1;
    // if (major == toMajor) compare minor version numbers
    if (minor < toMinor) return -1;
    if (minor > toMinor) return 1;
    return 0;
}
