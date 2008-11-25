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

// Determine which platforms are supported and provide a way
//   of exposing that information to the rest of the client.

#include "cpp.h"

#ifdef _WIN32
#include "boinc_win.h"
typedef BOOL (WINAPI *LPFN_ISWOW64PROCESS) (HANDLE, PBOOL);
LPFN_ISWOW64PROCESS fnIsWow64Process;
#endif

#ifndef _WIN32
#include "config.h"
#include <stdio.h>
#ifdef HAVE_SIGNAL_H
#include <signal.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_SYS_WAIT_H
#include <sys/wait.h>
#endif
#endif

#include "client_types.h"
#include "client_state.h"
#include "error_numbers.h"
#include "log_flags.h"
#include "str_util.h"
#include "util.h"


// return the primary platform id.
//
const char* CLIENT_STATE::get_primary_platform() {
    return platforms[0].name.c_str();
}


// add a platform to the vector.
//
void CLIENT_STATE::add_platform(const char* platform) {
    PLATFORM pp;
    pp.name = platform;
    platforms.push_back(pp);
}


// determine the list of supported platforms.
//
void CLIENT_STATE::detect_platforms() {

#if defined(_WIN32) && !defined(__CYGWIN32__)
#if defined(_WIN64) && defined(_M_X64)
    add_platform("windows_x86_64");
    add_platform("windows_intelx86");
#else
    // see if 32-bit client is running on 64-bit machine
    //
    BOOL bIsWow64 = FALSE;
    fnIsWow64Process = (LPFN_ISWOW64PROCESS)GetProcAddress(
        GetModuleHandle(TEXT("kernel32")),"IsWow64Process"
    );
    if (fnIsWow64Process) {
        if (fnIsWow64Process(GetCurrentProcess(), &bIsWow64)) {
            if (bIsWow64) {
                add_platform("windows_x86_64");
            }
        }
    }
    add_platform("windows_intelx86");
#endif

#elif defined(__APPLE__)
#if defined(__x86_64__)
    add_platform("x86_64-apple-darwin");
#endif

#if defined(__i386__) || defined(__x86_64__)
    // Supported on both Mac Intel architectures
    add_platform("i686-apple-darwin");
#endif
    // Supported on all 3 Mac architectures
    add_platform("powerpc-apple-darwin");
#elif defined(sun)
    // Check if we can run 64 bit binaries...

#if defined(__sparc) || defined(sparc)
    FILE *f=fopen("/usr/bin/sparcv9/ls","r");
    char *argv[3];
    pid_t pid;
    int rv=0;
    argv[0]="/usr/bin/sparcv9/ls";
    argv[1]=argv[0];
    argv[2]=NULL;
    if (f) {
        fclose(f);
        if (0==(pid=fork())) {
            // we are in child process
            freopen("/dev/null","a",stderr);
            freopen("/dev/null","a",stdout);
            rv=execv(argv[0],argv);
            exit(rv);
        } else {
            // we are in the parent process.
            time_t start=time(0);
            int done;
            // wait 5 seconds or until the process exits.
            do {
                done=waitpid(pid,&rv,WNOHANG);
                sleep(1);
            } while (!done && ((time(0)-start)<5));
            // if we timed out, kill the process	
            if ((time(0)-start)>=5) {
                kill(pid,SIGKILL); 
                done=-1;
            }
            // if we exited with success add the 64 bit platform
            if ((done == pid) && (rv == 0)) {
               add_platform("sparc64-sun-solaris");
            }
        }
    }
    add_platform("sparc-sun-solaris");
    // the following platform is obsolete, but we'll add it anyway.
    add_platform("sparc-sun-solaris2.7");
#else // defined sparc
    add_platform(HOSTTYPE);
#ifdef HOSTTYPEALT
    add_platform(HOSTTYPEALT);
#endif
#endif // else defined sparc      

#else
    // Any other platform, fall back to the previous method
    add_platform(HOSTTYPE);
#ifdef HOSTTYPEALT
    add_platform(HOSTTYPEALT);
#endif

#endif

    if (config.no_alt_platform) {
        PLATFORM p = platforms[0];
        platforms.clear();
        platforms.push_back(p);
    }

    // add platforms listed in cc_config.xml AFTER the above.
    //
    for (unsigned int i=0; i<config.alt_platforms.size(); i++) {
        add_platform(config.alt_platforms[i].c_str());
    }
}


// write XML list of supported platforms
//
void CLIENT_STATE::write_platforms(PROJECT* p, MIOFILE& mf) {

    mf.printf(
        "    <platform_name>%s</platform_name>\n",
        p->anonymous_platform ? "anonymous" : get_primary_platform()
    );

    for (unsigned int i=1; i<platforms.size(); i++) {
        PLATFORM& platform = platforms[i];
        mf.printf(
            "    <alt_platform>\n"
            "        <name>%s</name>\n"
            "    </alt_platform>\n",
            platform.name.c_str()
        );
    }
}

bool CLIENT_STATE::is_supported_platform(const char* p) {
    for (unsigned int i=0; i<platforms.size(); i++) {
        PLATFORM& platform = platforms[i];
        if (!strcmp(p, platform.name.c_str())) {
            return true;
        }
    }
    return false;
}
