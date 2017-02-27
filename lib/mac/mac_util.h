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

//  mac_util.h

#ifndef _MAC_UTIL_H_
#define _MAC_UTIL_H_

#include <Carbon/Carbon.h>

    double  getTimeSinceBoot(void);
    void    getPathToThisApp(char* pathBuf, size_t bufSize);
    void    BringAppToFront();
    void    BringAppWithPidToFront(pid_t pid);
    pid_t   getPidIfRunning(char * bundleID);

    OSStatus    GetPathToAppFromID(OSType creator, CFStringRef bundleID, char *path, size_t maxLen);

// Macros to test OS version number on all versions of OS X without using deprecated Gestalt
// compareOSVersionTo(x, y) returns:
// -1 if the OS version we are running on is less than 10.x.y
//  0 if the OS version we are running on is equal to 10.x.y
// +1 if the OS version we are running on is lgreater than 10.x.y
//
#define MAKECFVERSIONNUMBER(x, y) floor(kCFCoreFoundationVersionNumber##x##_##y)
#define compareOSVersionTo(toMajor, toMinor) \
(floor(kCFCoreFoundationVersionNumber) > MAKECFVERSIONNUMBER(toMajor, toMinor) ? 1 : \
(floor(kCFCoreFoundationVersionNumber) < MAKECFVERSIONNUMBER(toMajor, toMinor) ? -1 : 0))

// Allow this to be built using Xcode 5.0.2
#ifndef kCFCoreFoundationVersionNumber10_9
#define kCFCoreFoundationVersionNumber10_9      855.11
#endif
#ifndef kCFCoreFoundationVersionNumber10_10
#define kCFCoreFoundationVersionNumber10_10     1151.16
#endif
#ifndef kCFCoreFoundationVersionNumber10_11
#define kCFCoreFoundationVersionNumber10_11     1253
#endif

#endif      // _MAC_UTIL_H_
