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

//  mac_util.h

#ifndef _MAC_UTIL_H_
#define _MAC_UTIL_H_

#include <Carbon/Carbon.h>

#ifdef __cplusplus
extern "C" {
#endif

    double      getTimeSinceBoot(void);
    void        getPathToThisApp(char* pathBuf, size_t bufSize);
    void        BringAppToFront();
    void        BringAppWithPidToFront(pid_t pid);
    pid_t       getActiveAppPid();
    pid_t       getPidIfRunning(char * bundleID);

    OSStatus    GetPathToAppFromID(OSType creator, CFStringRef bundleID, char *path, size_t maxLen);

    int         compareOSVersionTo(int toMajor, int toMinor);

#ifdef __cplusplus
}	// extern "C"
#endif

#endif      // _MAC_UTIL_H_
