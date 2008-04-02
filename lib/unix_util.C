// Berkeley Open Infrastructure for Network Computing
// http://boinc.berkeley.edu
// Copyright (C) 2008 University of California
//
// This is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation;
// either version 2.1 of the License, or (at your option) any later version.
//
// This software is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//
// To view the GNU Lesser General Public License visit
// http://www.gnu.org/copyleft/lesser.html
// or write to the Free Software Foundation, Inc.,
// 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

#include "unix_util.h"

#ifndef HAVE_DAEMON

#include <cstdio>
#include <cstdlib>

static FILE *stderr_null, *stdout_null;

int daemon(int nochdir, int noclose) {
    pid_t childpid,sessionid;
    if (!nochdir) {
        chdir("/");
    }
    if (!noclose) {
        stderr_null = freopen("/dev/null", "w", stderr);
        stdout_null = freopen("/dev/null", "w", stdout);
    }
    childpid = fork();
    if (childpid>0) {
        // Fork successful.    We are the parent process.
        _exit(0);
    } 
    if (childpid < 0) {
        // Fork unsuccessful.    Return -1
        return -1;
    }

    // Fork successful, We are the child. Make us the lead process of a new
    // session.
    sessionid = setsid();
    if (sessionid <= 0) {
        // setsid() failed
        return -1;
    }
 
    // success
    return 0;
}    

#endif /* HAVE_DAEMON */
