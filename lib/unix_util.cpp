// DEPRECATED

// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2008 University of California
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

// Note.  This already has an ifdef around it. If it is causing problem
// then HAVE_SETENV should be defined in your configuration files.
#ifndef HAVE_SETENV

#include <vector>
#include <cstring>
#include <cerrno>
#include <cstdlib>
#include <stdio.h>

#include "unix_util.h"

static std::vector<char *> envstrings;

// In theory setenv() is posix, but some implementations of unix
// don't have it.  The implementation isn't trivial because of
// differences in how putenv() behaves on different systems.
int setenv(const char *name, const char *value, int overwrite) {
    char *buf;
    int rv;
    // Name can't contant an equal sign.
    if (strchr(name,'=')) {
        errno=EINVAL;
        return -1;
    }
    // get any existing environment string.
    buf=getenv(name);
    if (!buf) {
        // no existing string, allocate a new one.
        buf=(char *)malloc(strlen(name)+strlen(value)+2);
        if (buf) envstrings.push_back(buf);
    } else {
        // we do have an existing string.
        // Are we allowed to overwrite it?  If not return success.
        if (!overwrite) return 0;
        // is it long enough to hold our current string?
        if (strlen(buf)<(strlen(name)+strlen(value)+1)) {
            // no.  See if we originally allocated this string.
            std::vector<char *>::iterator i=envstrings.begin();
            for (;i!=envstrings.end();i++) {
                if (*i == buf) break;
            }
            if (i!=envstrings.end()) {
                // we allocated this string.  Reallocate it.
                buf=(char *)realloc(buf,strlen(name)+strlen(value)+2);
                *i=buf;
            } else {
                // someone else allocated the string.  Allocate new memory.
                buf=(char *)malloc(strlen(name)+strlen(value)+2);
                if (buf) envstrings.push_back(buf);
            }
        }
    }
    if (!buf) {
        errno=ENOMEM;
        return -1;
    }
    sprintf(buf,"%s=%s",name,value);
    rv=putenv(buf);
    // Yes, there is a potential memory leak here.  Some versions of operating
    // systems copy the string into the environment, others make the
    // existing string part of the environment.  If we were to
    // implement unsetenv(), it might be possible to recover some of
    // this memory.  But unsetenv() is even less trivial than setenv.
    return rv;
}
#endif /*  !HAVE_SETENV */

#ifndef HAVE_DAEMON

#include <cstdio>
#include <cstdlib>
#if HAVE_UNISTD_H
#include <unistd.h>
#endif

using std::FILE;
using std::freopen;

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

#endif /* !HAVE_DAEMON */
