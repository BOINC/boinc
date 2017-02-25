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
