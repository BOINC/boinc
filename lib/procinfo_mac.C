// Berkeley Open Infrastructure for Network Computing
// http://boinc.berkeley.edu
// Copyright (C) 2006 University of California
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

// procinfo_mac.C
//

#define SHOW_TIMING 0

#include "config.h"
#include <stdio.h>

#include <ctype.h>
#include <cerrno>
#include <sys/types.h>
#include <dirent.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/mount.h>

#if SHOW_TIMING
#include <Carbon/Carbon.h>
#endif

#include "procinfo.h"
#include "client_msgs.h"
#include "file_names.h"
#include "client_state.h"
#include "hostinfo.h"

using std::vector;

// Routines to determine cpu time, memory usage and page fault count for 
// BOINC Client's child processes (including all their descendants) and also 
// totals for all other processes.  This also sets host_info.m_swap.
//
// On the Mac, most of this information is only accessible by the super-user, 
// so this code calls a helper application AppStats which is run setuid root 
// to do most of the work.  
//
// Since AppStats is called from the BOINC Client, it gets the BOINC Client's 
// pid using getppid().  It then searches the process list for all other 
// processes with the same parent.  
//
// For each child of BOINC Client, AppStats totals the cpu times, memory usage 
// and page fault counts for the child and all its descendants, and prints the 
// totals to stdout.
//
// Finally, AppStats computes these totals for all remaining processes and 
// prints them to stdout (with a value of 0 for the pid).
//
// This code doesn't use PROCINFO.is_boinc_app, but we set it to be consistent 
// with the code for other platforms.

static int bidirectional_popen(char *path, int *fd);

int procinfo_setup(vector<PROCINFO>& pi) {
    static int fd[2] = {0, 0};
    static int failed_retries = 0;

    char appstats_path[100];
    PROCINFO p;
    int c, result, retry_count = 0;
    char buf[256];
    struct statfs fs_info;
#if SHOW_TIMING
    UnsignedWide start, end, elapsed;

    start = UpTime();
#endif

    if (fd[0] == 0) {
        // Launch AppStats helper application with a bidirectional pipe
RELAUNCH:
        // If we are relaunching, close our open file decriptors
        if (fd[0] != 0)
            close(fd[0]);
        fd[0] = 0;
        
        if (fd[1] != 0)
            close(fd[1]);
        fd[1] = 0;

        if (failed_retries > 4) // Give up after failures on 5 consecutive calls 
            return ERR_EXEC;    //  of procinfo_setup()
            
        if (retry_count > 1) {
            ++failed_retries;
            return ERR_EXEC;
        }
            
        sprintf(appstats_path, "%s/%s", SWITCHER_DIR, APP_STATS_FILE_NAME);
        result = bidirectional_popen(appstats_path, fd);
#if SHOW_TIMING
        msg_printf(NULL, MSG_ERROR, "bidirectional_popen returned %d\n", result);
#endif
        if (result) {
            ++retry_count;
            goto RELAUNCH;
        }
    }

    c = write(fd[0], "\n", 1);  // Request a set of process info from AppStats helper application
    if (c < 0) {                // AppStats application exited
        ++retry_count;
        goto RELAUNCH;
    }

    while (1) {
        memset(&p, 0, sizeof(p));
        for (unsigned int i=0; i<sizeof(buf); i++) {
            c = read(fd[0], buf+i, 1);
            if (c < 0) {                // AppStats application exited
                ++retry_count;
                goto RELAUNCH;
            }

             if (buf[i] == '\n')
                break;
        }
        c = sscanf(buf, "%d %d %lf %lf %lu %lf %lf\n", 
                        &p.id, &p.parentid, &p.working_set_size, &p.swap_size, 
                        &p.page_fault_count, &p.user_time, &p.kernel_time);
                                    
        if (p.id == 0)
            break;
        p.is_boinc_app = false;
        pi.push_back(p);
        
        if (log_flags.mem_usage_debug) {
            msg_printf(
                    NULL, MSG_INFO,
                    "[mem_usage_debug] pid=%d, ppid=%d, rm=%.0lf, vm=%.0lf, pageins=%lu, usertime=%lf, systime=%lf\n",
                    p.id, p.parentid, p.working_set_size, p.swap_size, 
                    p.page_fault_count, p.user_time, p.kernel_time  
            );
        }
    }

    failed_retries = 0;        // Success: reset consecutive failures counter

    statfs(".", &fs_info);
    gstate.host_info.m_swap = (double)fs_info.f_bsize * (double)fs_info.f_bfree;

#if SHOW_TIMING
    end = UpTime();
    elapsed = AbsoluteToNanoseconds(SubAbsoluteFromAbsolute(end, start));
    msg_printf(NULL, MSG_ERROR, "elapsed time = %llu, m_swap = %lf\n", elapsed, gstate.host_info.m_swap);
#endif
    
    return 0;
}

// fill in the given PROCINFO (which initially is zero except for id)
// with totals from that process and all its descendants
//
void procinfo_app(PROCINFO& pi, vector<PROCINFO>& piv) {
    unsigned int i;
    // AppStat returned totals for each of BOINC's children and its descendants
    // as a single PROCINFO struct with the id field set to the pid of BOINC's 
    // direct child. 
    for (i=0; i<piv.size(); i++) {
        PROCINFO& p = piv[i];
        if (p.id == pi.id) {
            pi = p;
            p.is_boinc_app = true;
            return;
        }
    }
}

#if 0
    // the following is not useful because most OSs don't
    // move idle processes out of RAM, so physical memory is always full
    //
    // If you wish to implement this in a future release, you must also 
    // enable the corresponding logic in app_stats_mac.C by defining 
    // GET_NON_BOINC_INFO to 1
    //
void procinfo_other(PROCINFO& pi, vector<PROCINFO>& piv) {
    // AppStat returned total for all other processes as a single PROCINFO 
    // struct with id field set to zero.
    memset(&pi, 0, sizeof(pi));
    procinfo_app(pi, piv);
}
#endif

static int bidirectional_popen(char *path, int *fd) {
    pid_t	pid;
    char        *appname;

    appname = strrchr(path, '/');
    if (! appname)
        appname = path;

    if (socketpair(AF_UNIX, SOCK_STREAM, 0, fd) < 0) {			/* only need a single stream pipe */
        msg_printf(NULL, MSG_INTERNAL_ERROR, "%s: pipe error %d: %s\n", appname, errno, strerror(errno));
        return ERR_SOCKET;
    }
    
    if ( (pid = fork()) < 0) {
        close(fd[0]);
        close(fd[1]);
        fd[0] = 0;
        fd[1] = 0;
        msg_printf(NULL, MSG_INTERNAL_ERROR, "%s: fork error\n", appname);
        return ERR_FORK;
    }
    else if (pid > 0) {							/* parent */
        close(fd[1]);
     } else {								/* child */
        close(fd[0]);
        if (fd[1] != STDIN_FILENO) {
            if (dup2(fd[1], STDIN_FILENO) != STDIN_FILENO) {
               fprintf(stderr, "dup2 error to stdin");
               exit (1);
            }
        }
        if (fd[1] != STDOUT_FILENO) {
            if (dup2(fd[1], STDOUT_FILENO) != STDOUT_FILENO) {
                fprintf(stderr, "dup2 error to stdout");
                exit (1);
            }
        }
        if (execl(path, appname, NULL) < 0) {
            printf("-1\n");
            fprintf(stderr, "execl error");
        }
    }
    
    return 0;
}
