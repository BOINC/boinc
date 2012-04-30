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
#else
#include "config.h"
#include <cstdio>
#include <cstdlib>
#include <signal.h>
#if HAVE_UNISTD_H
#include <unistd.h>
#endif
#if HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#if HAVE_SYS_WAIT_H
#include <sys/wait.h>
#endif
#endif

#if defined(__APPLE__) && (defined(__i386__) || defined(__x86_64__))
#include <CoreServices/CoreServices.h>
#include <sys/sysctl.h>
#endif

#include "error_numbers.h"
#include "filesys.h"
#include "str_util.h"
#include "str_replace.h"
#include "util.h"

#include "client_types.h"
#include "client_state.h"
#include "log_flags.h"
#include "project.h"

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
#elif defined(ANDROID)
    add_platform("arm-android");

#elif defined(__APPLE__)

#if defined(__i386__) || defined(__x86_64__)
    OSStatus err = noErr;
    SInt32 version = 0;
    int response = 0;
    int retval = 0;
    size_t len = sizeof(response);

    err = Gestalt(gestaltSystemVersion, &version);
    retval = sysctlbyname("hw.optional.x86_64", &response, &len, NULL, 0);
    if ((err == noErr) && (version >= 0x1050) && response && (!retval)) {
        add_platform("x86_64-apple-darwin");
    }

    // Supported on both Mac Intel architectures
    add_platform("i686-apple-darwin");
#else
    // We no longer request PowerPC applications on Intel Macs
    // because all projects supporting Macs should have Intel
    // applications by now, and PowerPC emulation ("Rosetta")
    // is not always supported in newer versions of OS X.
    add_platform("powerpc-apple-darwin");
#endif

#elif defined(__linux__) && ( defined(__i386__) || defined(__x86_64__) )
    // Let's try to support both 32 and 64 bit applications in one client
    // regardless of whether it is a 32 or 64 bit client
    const char *uname[]={"/bin/uname","/usr/bin/uname",0};
    int eno=0, support64=0, support32=0;
    FILE *f;
    char cmdline[256];
    cmdline[0]=0;

    // find the 'uname' executable
    do {
        if (boinc_file_exists(uname[eno])) break;
    } while (uname[++eno] != 0);

    // run it and check the kernel machine architecture.
    if ( uname[eno] != 0 ) {
        strlcpy(cmdline,uname[eno],256);
        strlcat(cmdline," -m",256);
        if ((f=popen(cmdline,"r"))) {
            while (!std::feof(f)) {
                fgets(cmdline,256,f);
                if (strstr(cmdline,"x86_64")) support64=1;
            }
            pclose(f);
        }

        if (!support64) {
            // we're running on a 32 bit kernel, so we will assume
            // we are i686-pc-linux-gnu only.
            support32=1;
        } else {
            // we're running on a 64 bit kernel.
            // Now comes the hard part.  How to tell whether we can run
            // 32-bit binaries.
#if defined(__i386__) && !defined(__x86_64__)
            // If we're a 32 bit binary, then we obviously can.
            support32=1;
#else
            // If we're a 64 bit binary, the check is a bit harder.
            // We'll use the file command to check installation of
            // 32 bit libraries or executables.
            const char *file[]={"/usr/bin/file","/bin/file",0};
            const char *libdir[]={"/lib","/lib32","/lib/32","/usr/lib","/usr/lib32","/usr/lib/32"};
            const int nlibdirs=sizeof(libdir)/sizeof(char *);

            // find 'file'
            eno=0;
            do {
                if (boinc_file_exists(file[eno])) break;
            } while (file[++eno] != 0);

            // now try to find a 32-bit C library.
            if (file[eno] != 0) {
                int i;
                for (i=0; i < nlibdirs; i++) {
                    struct dirent *entry;
                    DIR *a = opendir(libdir[i]);
                    // if dir doesn't exist, do to the next one
                    if (a == 0) continue;
                    // dir exists. read each entry until you find a 32bit lib
                    while ((support32 == 0) && ((entry=readdir(a)) != 0)) {
                        strlcpy(cmdline, file[eno], 256);
                        strlcat(cmdline, " -L ", 256);
                        strlcat(cmdline, libdir[i], 256);
                        strlcat(cmdline, "/", 256);
                        strlcat(cmdline, entry->d_name, 256);
                        f = popen(cmdline, "r");
                        if (f) {
                            while (!std::feof(f)) {
                                fgets(cmdline,256,f);
                                // If the library is 32-bit ELF, then we're
                                // golden.
                                if (strstr(cmdline, "ELF") && strstr(cmdline, "32-bit")) support32=1;
                            }
                            pclose(f);
                        }
                    }
                    closedir(a);
                    if (support32) break;
                }
            }
#endif
        }
    }

    if (support64) {
        add_platform("x86_64-pc-linux-gnu");
    }
    if (support32) {
        add_platform("i686-pc-linux-gnu");
    }

    if (!(support64 || support32)) {
        // Something went wrong. Assume HOSTTYPE and HOSTTYPEALT
        // are correct
        add_platform(HOSTTYPE);
#ifdef HOSTTYPEALT
        add_platform(HOSTTYPEALT);
#endif
    }

#elif defined(sun)
    // Check if we can run 64-bit binaries...
    // this assumes there isn't a 64-bit only solaris.  (Every 64-bit solaris can run 32 bit binaries)

#if defined(__sparc) || defined(sparc)
    char *exe64=const_cast<char *>("/usr/bin/sparcv9/ls");
    char *platform64=const_cast<char *>("sparc64-sun-solaris");
    char *platform32=const_cast<char *>("sparc-sun-solaris");
#elif defined(__i386) || defined(i386) || defined(__amd64) || defined(__x86_64__)
    char *exe64=const_cast<char *>("/usr/bin/amd64/ls");
    char *platform64=const_cast<char *>("x86_64-pc-solaris");
    char *platform32=const_cast<char *>("i686-pc-solaris");
#else
#define UNKNOWN_SOLARIS_PROCESSOR
#endif

#ifndef UNKNOWN_SOLARIS_PROCESSOR
    FILE *f=fopen(exe64,"r");
    char *argv[3];
    pid_t pid;
    int rv=0;
    argv[0]=exe64;
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
               add_platform(platform64);
            }
        }
    }
    add_platform(platform32);
    // the following platform is obsolete, but we'll add it anyway.
#if defined(__sparc) || defined(sparc)
    add_platform("sparc-sun-solaris2.7");
#endif
#else  // !defined(UNKNOWN_SOLARIS_PROCESSOR)

    add_platform(HOSTTYPE);
#ifdef HOSTTYPEALT
    add_platform(HOSTTYPEALT);
#endif

#endif  // !defined(UNKNOWN_SOLARIS_PROCESSOR)

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
