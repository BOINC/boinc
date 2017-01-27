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

    double              getTimeSinceBoot(void);
    void                getPathToThisApp(char* pathBuf, size_t bufSize);
    void                BringAppToFront();
    pid_t               getPidIfRunning(char * bundleID);

#endif      // _MAC_UTIL_H_
