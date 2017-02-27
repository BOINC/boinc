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
    [ [NSRunningApplication runningApplicationWithProcessIdentifier:pid] activateWithOptions:NSApplicationActivateIgnoringOtherApps | NSApplicationActivateAllWindows ];
}


pid_t getPidIfRunning(char * bundleID) {
    NSString *NSBundleID = [[NSString alloc] initWithUTF8String:bundleID];
    NSArray * runningApps = [NSRunningApplication runningApplicationsWithBundleIdentifier:NSBundleID];
    if ([runningApps count] > 0) {
        return [((NSRunningApplication *)[runningApps firstObject]) processIdentifier];
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
    void *LSlib = dlopen("/System/Library/Frameworks/CoreServices.framework/Frameworks/LaunchServices.framework/LaunchServices", RTLD_NOW);
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
                CFRelease(appRefs);
            }
        }
        if (err != noErr) {
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
    return err;
}
