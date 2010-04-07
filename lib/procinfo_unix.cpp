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


#include "config.h"

#ifdef HAVE_PROCFS_H
// Can't use large file calls with solaris procfs.
#if defined(_FILE_OFFSET_BITS) && ( _FILE_OFFSET_BITS == 64 )
#undef _FILE_OFFSET_BITS
#undef _LARGE_FILES
#undef _LARGEFILE_SOURCE
#undef _LARGEFILE64_SOURCE
#endif
#endif

#include <cstdio>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <dirent.h>

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#ifdef HAVE_PROCFS_H
#include <procfs.h>  // definitions for solaris /proc structs
#endif

#include "procinfo.h"
#include "str_util.h"
#include "str_replace.h"
#include "client_msgs.h"

using std::vector;


// see:
// man 5 proc
// /usr/src/linux/fs/proc/array.C

struct PROC_STAT {
    int pid;
    char comm[256];
    char state;
    int ppid;
    int pgrp;
    int session;
    int tty_nr;
    int tpgid;
    unsigned long flags;
    unsigned long minflt;
    unsigned long cminflt;
    unsigned long majflt;
    unsigned long cmajflt;
    unsigned long utime;
    unsigned long stime;
    int cutime;
    int cstime;
    int priority;
    int nice;
    int zero;
    int itrealvalue;
    unsigned long starttime;
    unsigned long vsize;
    int rss;
    unsigned long rlim;
    unsigned long startcode;
    unsigned long endcode;
    unsigned long startstack;
    unsigned long kstkesp;
    unsigned long kstkeip;
    unsigned long signal;
    unsigned long blocked;
    unsigned long sigignore;
    unsigned long sigcatch;
    unsigned long wchan;
    unsigned long nswap;
    unsigned long cnswap;
    int exit_signal;
    int processor;

    int parse(char*);
};

int PROC_STAT::parse(char* buf) {
    int n = sscanf(buf,
        "%d (%s %c %d %d %d %d %d "
        "%lu %lu %lu %lu %lu %lu %lu "
        "%d %d %d %d %d %d "
        "%lu %lu "
        "%d "
        "%lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu "
        "%d %d",
        &pid,
        comm,
        &state,
        &ppid,
        &pgrp,
        &session,
        &tty_nr,
        &tpgid,
        &flags,
        &minflt,
        &cminflt,
        &majflt,
        &cmajflt,
        &utime,
        &stime,
        &cutime,
        &cstime,
        &priority,
        &nice,
        &zero,
        &itrealvalue,
        &starttime,
        &vsize,
        &rss,
        &rlim,
        &startcode,
        &endcode,
        &startstack,
        &kstkesp,
        &kstkeip,
        &signal,
        &blocked,
        &sigignore,
        &sigcatch,
        &wchan,
        &nswap,
        &cnswap,
        &exit_signal,
        &processor
    );
    if (n == 39) {
        char* p = strchr(comm, ')');
        if (p) *p = 0;
        return 0;
    }

    // I don't see a good choice of ERR_ for this...
    //
    return 1;
}

// build table of all processes in system
//
int procinfo_setup(vector<PROCINFO>& pi) {

#ifdef HAVE_DIRENT_H
    DIR *dir;
    dirent *piddir;
    FILE* fd;
    PROC_STAT ps;
    PROCINFO p;
    char pidpath[1024];
    char buf[1024];
    int pid = getpid();

    dir = opendir("/proc");
    if (!dir) return 0;

    while (1) {
        piddir = readdir(dir);
        if (piddir) {
            if (isdigit(piddir->d_name[0])) {

#if defined(HAVE_PROCFS_H) && defined(HAVE__PROC_SELF_PSINFO)  // solaris
                psinfo_t psinfo;
                sprintf(pidpath, "/proc/%s/psinfo", piddir->d_name);
                fd = fopen(pidpath, "r");
                if (fd) {
                    if (fread(&psinfo, sizeof(psinfo_t), 1, fd) == 1) {
                        p.id = psinfo.pr_pid;
                        p.parentid = psinfo.pr_ppid;
                        p.swap_size = psinfo.pr_size*1024.;
                        p.working_set_size = psinfo.pr_rssize * 1024.;
                        strlcpy(p.command, psinfo.pr_fname, sizeof(p.command));
                    }
                    fclose(fd);
                    sprintf(pidpath, "/proc/%s/usage", piddir->d_name);
                    prusage_t prusage;
                    fd = fopen(pidpath, "r");
                    if (fd) {
                        if (fread(&prusage, sizeof(prusage_t), 1, fd) == 1) {
                            p.user_time = (float)prusage.pr_utime.tv_sec +
                                ((float)prusage.pr_utime.tv_nsec)/1e+9;
                            p.kernel_time = (float)prusage.pr_stime.tv_sec +
                                ((float)prusage.pr_utime.tv_nsec)/1e+9;
                            // page faults: I/O + non I/O
                            p.page_fault_count = prusage.pr_majf + prusage.pr_minf;
                        }
                        fclose(fd);
                        p.is_boinc_app = (p.id == pid || strcasestr(p.command, "boinc"));
                        pi.push_back(p);
                    }
                }
#else  // linux
                sprintf(pidpath, "/proc/%s/stat", piddir->d_name);
                fd = fopen(pidpath, "r");
                if (fd) {
                    fgets(buf, sizeof(buf), fd);
                    ps.parse(buf);
                    fclose(fd);

                    p.id = ps.pid;
                    p.parentid = ps.ppid;
                    p.swap_size = ps.vsize;
                    // rss = pages, need bytes
                    // assumes page size = 4k
                    p.working_set_size = ps.rss * (float)getpagesize();
                    // page faults: I/O + non I/O
                    p.page_fault_count = ps.majflt + ps.minflt;
                    // times are in jiffies, need seconds
                    // assumes 100 jiffies per second
                    p.user_time = ps.utime / 100.;
                    p.kernel_time = ps.stime / 100.;
                    strlcpy(p.command, ps.comm, sizeof(p.command));
                    p.is_boinc_app = (p.id == pid || strcasestr(p.command, "boinc"));
                    pi.push_back(p);
                }
#endif

            }
        } else {
            closedir(dir);
            return 0;
        }
    }

#endif
    return 0;

}

// Scan the process table adding in CPU time and mem usage.
// Loop thru entire table as the entries aren't in order.
// Recurse at most 4 times to get additional child processes
//
void add_child_totals(PROCINFO& pi, vector<PROCINFO>& piv, int pid, int rlvl) {
    unsigned int i;

    if (rlvl > 3) {
        return;
    }
    for (i=0; i<piv.size(); i++) {
        PROCINFO& p = piv[i];
        if (p.parentid == pid) {
            pi.kernel_time += p.kernel_time;
            pi.user_time += p.user_time;

            // only count process with most swap and memory
            if (p.swap_size > pi.swap_size) {
                pi.swap_size = p.swap_size;
            }
            if (p.working_set_size > pi.working_set_size) {
                pi.working_set_size = p.working_set_size;
            }

            p.is_boinc_app = true;
            // look for child process of this one
            add_child_totals(pi, piv, p.id, rlvl+1); // recursion - woo hoo!
        }
    }
}

// fill in the given PROCINFO (which initially is zero except for id)
// with totals from that process and all its descendants
//
void procinfo_app(
    PROCINFO& pi, vector<PROCINFO>& piv, char* graphics_exec_file
) {
    unsigned int i;

    for (i=0; i<piv.size(); i++) {
        PROCINFO& p = piv[i];
        if (p.id == pi.id) {
            pi.kernel_time += p.kernel_time;
            pi.user_time += p.user_time;
            pi.swap_size += p.swap_size;
            pi.working_set_size += p.working_set_size;
            p.is_boinc_app = true;

            // look for child processes
            //
            add_child_totals(pi, piv, pi.id, 0);
            return;
        }
        if (!strcmp(p.command, graphics_exec_file)) {
            p.is_boinc_app = true;
        }
    }
}

void procinfo_other(PROCINFO& pi, vector<PROCINFO>& piv) {
    unsigned int i;

    memset(&pi, 0, sizeof(pi));
    for (i=0; i<piv.size(); i++) {
        PROCINFO& p = piv[i];
        if (!p.is_boinc_app) {
            pi.kernel_time += p.kernel_time;
            pi.user_time += p.user_time;
            pi.swap_size += p.swap_size;
            pi.working_set_size += p.working_set_size;
        }
    }
}
