// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2021 University of California
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
#ifndef SIM
#include "hostinfo.h"
#endif
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

#ifdef __APPLE__
#include <sys/sysctl.h>
extern int compareOSVersionTo(int toMajor, int toMinor);
#endif

#include "error_numbers.h"
#include "filesys.h"
#include "str_util.h"
#include "str_replace.h"
#include "util.h"

#include "client_types.h"
#include "client_state.h"
#include "client_msgs.h"
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


#if defined (__APPLE__) && defined (__arm64__)
// detect a possibly emulated x86_64 CPU and its features on a Apple Silicon M1 Mac
//
int launch_child_process_to_detect_emulated_cpu() {
    int pid;
    char data_dir[MAXPATHLEN];
    char execpath[MAXPATHLEN];
    int retval = 0;

    retval = boinc_delete_file(EMULATED_CPU_INFO_FILENAME);
    if (retval) {
        msg_printf(0, MSG_INFO,
            "Failed to delete old %s. error code %d",
            EMULATED_CPU_INFO_FILENAME, retval
        );
    } else {
        for (;;) {
            if (!boinc_file_exists(EMULATED_CPU_INFO_FILENAME)) break;
            boinc_sleep(0.01);
        }
    }

    // write the EMULATED_CPU_INFO into the BOINC data dir
    boinc_getcwd(data_dir);

    // the execuable should be in BOINC data dir
    strncpy(execpath, data_dir, sizeof(execpath));
    strncat(execpath, "/" EMULATED_CPU_INFO_EXECUTABLE, sizeof(execpath) - strlen(execpath) - 1);

    if (log_flags.coproc_debug) {
        msg_printf(0, MSG_INFO,
            "[x86_64-M1] launching child process at %s",
            execpath
        );
    }

    int argc = 1;
    char* const argv[2] = {
         const_cast<char *>(execpath),
         NULL
    };

    retval = run_program(
        data_dir,
        execpath,
        argc,
        argv,
        pid
    );

    if (retval) {
        if (log_flags.coproc_debug) {
            msg_printf(0, MSG_INFO,
                "[x86_64-M1] run_program of child process returned error %d",
                retval
            );
        }
        return retval;
    }

    int status;
    retval = get_exit_status(pid, status, 10);
    if (retval) {
        msg_printf(0, MSG_INFO, "CPU emulation check process didn't exit");
        kill_process(pid);
        return retval;
    }

    if (status) {
        char buf[200];
        if (WIFEXITED(status)) {
            int code = WEXITSTATUS(status);
            snprintf(buf, sizeof(buf),
                "process exited with status %d: %s", code, strerror(code)
            );
        } else if (WIFSIGNALED(status)) {
            int sig = WTERMSIG(status);
            snprintf(buf, sizeof(buf),
                "process was terminated by signal %d", sig
            );
        } else {
            snprintf(buf, sizeof(buf), "unknown status %d", status);
        }
        msg_printf(0, MSG_INFO,
            "Emulated CPU detection failed: %s",
            buf
        );
        return -1;
    }

    return 0;
}
#endif

// determine the list of supported platforms.
//
void CLIENT_STATE::detect_platforms() {

#if defined(_WIN32) && !defined(__CYGWIN32__)
#if defined(_ARM64_)
    add_platform("windows_arm64");
#ifndef SIM
    // according to
    // https://learn.microsoft.com/en-us/windows/arm/apps-on-arm-x86-emulation
    // Windows 10 on ARM can run x86 applications,
    // and Windows 11 on ARM can run x86_64 applications
    // so we will add these platfroms as well
    OSVERSIONINFOEX osvi;
    BOOL bOsVersionInfoEx = get_OSVERSIONINFO(osvi);
    if (osvi.dwMajorVersion >= 10) {
        add_platform("windows_intelx86");
        if (osvi.dwBuildNumber >= 22000) {
            add_platform("windows_x86_64");
        }
    }
#endif
#elif defined(_WIN64) && defined(_M_X64)
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

#ifdef __x86_64__
    add_platform("x86_64-apple-darwin");
    if (compareOSVersionTo(10, 15) < 0) {
        add_platform("i686-apple-darwin");
    }
#elif defined(__arm64__)
    add_platform("arm64-apple-darwin");
    if (!launch_child_process_to_detect_emulated_cpu()) {
        add_platform("x86_64-apple-darwin");
    }
#else
#error Mac client now requires a 64-bit system
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
    //
    while (1) {
        if (boinc_file_exists(uname[eno])) break;
        eno++;
        if (uname[eno] == 0) break;
    }

    // run it and check the kernel machine architecture.
    if ( uname[eno] != 0 ) {
        strlcpy(cmdline,uname[eno],256);
        strlcat(cmdline," -m",256);
        if ((f=popen(cmdline,"r"))) {
            while (!std::feof(f)) {
                if (!fgets(cmdline,256,f)) break;
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
            //
            eno=0;
            while (1) {
                if (boinc_file_exists(file[eno])) break;
                eno++;
                if (file[eno] == 0) break;
            }

            // now try to find a 32-bit C library.
            if (file[eno] != 0) {
                int i;
                for (i=0; i < nlibdirs; i++) {
                    struct dirent *entry;
                    DIR *a = opendir(libdir[i]);
                    // if dir doesn't exist, do the next one
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
                                if (!fgets(cmdline,256,f)) break;
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

    if (cc_config.no_alt_platform) {
        PLATFORM p = platforms[0];
        platforms.clear();
        platforms.push_back(p);
    }

    // add platforms listed in cc_config.xml AFTER the above.
    //
    for (unsigned int i=0; i<cc_config.alt_platforms.size(); i++) {
        add_platform(cc_config.alt_platforms[i].c_str());
    }
}


// write XML list of supported platforms
//
void CLIENT_STATE::write_platforms(PROJECT* p, FILE *f) {
    if (p && p->anonymous_platform) {
        fprintf(f, "    <platform_name>anonymous</platform_name>\n");
    } else {
        fprintf(f,
            "    <platform_name>%s</platform_name>\n", get_primary_platform()
        );
        for (unsigned int i=1; i<platforms.size(); i++) {
            PLATFORM& platform = platforms[i];
            fprintf(f,
                "    <alt_platform>\n"
                "        <name>%s</name>\n"
                "    </alt_platform>\n",
                platform.name.c_str()
            );
        }
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
