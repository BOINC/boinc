// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2022 University of California
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


// process-enumeration stuff for Unix other than Mac OS X

#include "config.h"

#if HAVE_PROCFS_H
// Can't use large file calls with solaris procfs.
#if defined(_FILE_OFFSET_BITS) && ( _FILE_OFFSET_BITS == 64 ) && defined(__sun)
#undef _FILE_OFFSET_BITS
#undef _LARGE_FILES
#undef _LARGEFILE_SOURCE
#undef _LARGEFILE64_SOURCE
#endif
#endif

#ifdef __FreeBSD__
#include <errno.h>
#include <sys/sysctl.h>
#include <sys/time.h>       // struct clockinfo
#include <sys/resource.h>   // CPUSTATES, CP_USER, CP_NICE, CP_SYS, CP_INTR, CP_IDLE
#include <sys/user.h>       // struct kinfo_proc
#include "util.h"
#include <vector>
#endif

#include <cstdio>
#include <string.h>
#include <sys/param.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <dirent.h>
#include <signal.h>
#include <cstdlib>

#if HAVE_UNISTD_H
#include <unistd.h>
#endif

#if HAVE_PROCFS_H
#include <procfs.h>  // definitions for solaris /proc structs
#endif

#include "error_numbers.h"
#include "filesys.h"
#include "str_util.h"
#include "str_replace.h"

#include "procinfo.h"

using std::vector;

// see:
// http://git.kernel.org/cgit/linux/kernel/git/torvalds/linux.git/tree/fs/proc/array.c
//
// Interesting note: the command part of /proc/PID/stat is the first
// 15 characters of the executable filename.
// If you want the entire filename, or the rest of the cmdline,
// you need to parse /proc/PID/cmdline,
// which is the cmdline with NULL separators

struct PROC_STAT {
    int pid;
    char command[256];
    int ppid;
    int priority;
    unsigned long utime;
    unsigned long stime;
    // the following defined only for BOINC-related processes
    double resident_set_size;
    double virtual_size;
    double swap_usage;

    int parse_stat(char*);
    int parse_status(const char*);
};


static inline char* skip_spaces(char* p, int n) {
    for (int i=0; i<n; i++) {
        p = strchr(p, ' ');
        if (!p) return NULL;
        p++;
    }
    return p;
}

// parse a /proc/pid/stat file (1 line)
// see https://man7.org/linux/man-pages/man5/proc_pid_stat.5.html
// parse only pid, comm, ppid, utime, stime, priority
// 1909 (mysqld) S 1 1909 1909 0 -1 4194560 ...
//
int PROC_STAT::parse_stat(char* buf) {
    pid = atoi(buf);
    char *p = strchr(buf, '(');
    if (!p) return 1;
    char *q = strchr(p, ')');
    if (!q) return 1;
    *q = 0;
    safe_strcpy(command, p+1);
    q += 4;
    ppid = atoi(q);
    q = skip_spaces(q, 10);
    if (!q) return 1;
    utime = strtol(q, &p, 0);
    stime = atoi(p);
    q = skip_spaces(q, 4);
    if (!q) return 1;
    priority = atoi(q);
    return 0;
}

// parse /proc/pid/status fields like
// VmRSS:    279424 kB
//
static int get_field(const char* buf, const char* prefix) {
    const char *p = strstr(buf, prefix);
    if (!p) return 0;
    return atoi(p+strlen(prefix));
}

void PROCINFO::get_mem_info() {
    char buf[2048], path[256];
    sprintf(path, "/proc/%d/status", id);
    FILE* f = fopen(path, "r");
    if (!f) return;
    size_t n = fread(buf, 1, sizeof(buf)-1, f);
    if (n == 0) {
        fclose(f);
        return;
    }
    buf[n] = 0;
    virtual_size = get_field(buf, "VmSize:")*1024.;
    rss = get_field(buf, "VmRSS:")*1024.;
    swap_usage = get_field(buf, "VmSwap:")*1024.;
    fclose(f);
}

// build a map pid=>descriptor of all processes in system.
// set descriptor.is_boinc_app if the proc is the calling proc
// or its command contains 'boinc'
//
int procinfo_setup(PROC_MAP& pm) {
#ifdef __FreeBSD__
// FreeBSD-native process enumeration, replacing the /proc-based approach
// used on Linux/Solaris. This requires no /proc mount of any kind (native
// procfs or linprocfs) and works correctly and identically whether running
// on bare metal or inside a jail.
//
    int mib[3] = { CTL_KERN, KERN_PROC, KERN_PROC_PROC };
    size_t len;
    std::vector<struct kinfo_proc> kprocs;

    for (;;) {
        len = 0;
        if (sysctl(mib, 3, NULL, &len, NULL, 0) != 0) {
            fprintf(stderr,
                "procinfo_setup(): sysctl(KERN_PROC_PROC) size query failed\n");
            return ERR_IO;
        }
        kprocs.resize((len + sizeof(struct kinfo_proc) - 1) / sizeof(struct kinfo_proc));
        size_t buflen = kprocs.size() * sizeof(struct kinfo_proc);
        if (sysctl(mib, 3, kprocs.data(), &buflen, NULL, 0) == 0) {
            len = buflen;
            break;
        }
        if (errno != ENOMEM) {
            fprintf(stderr,
                "procinfo_setup(): sysctl(KERN_PROC_PROC) failed\n");
            return ERR_IO;
        }
    }

    int nprocs = len / sizeof(struct kinfo_proc);
    struct kinfo_proc* kp = kprocs.data();
    int mypid = getpid();
    long page_size = sysconf(_SC_PAGESIZE);

    for (int i = 0; i < nprocs; i++) {
        // Guard against a userland/kernel struct-layout mismatch
        if (kp[i].ki_structsize != sizeof(struct kinfo_proc)) continue;

        PROCINFO p;
        p.clear();
        p.id = kp[i].ki_pid;
        p.parentid = kp[i].ki_ppid;
        safe_strcpy(p.command, kp[i].ki_comm);
        p.user_time = kp[i].ki_rusage.ru_utime.tv_sec
            + kp[i].ki_rusage.ru_utime.tv_usec / 1e6;
        p.kernel_time = kp[i].ki_rusage.ru_stime.tv_sec
            + kp[i].ki_rusage.ru_stime.tv_usec / 1e6;
        // ki_rssize is in pages; PROCINFO::rss wants bytes.
        p.rss = (double)kp[i].ki_rssize * page_size;
        // No direct per-process swap-usage accounting on FreeBSD
        p.swap_usage = 0;
        p.is_boinc_app = (p.id == mypid || strcasestr(p.command, "boinc"));
        p.is_low_priority = (kp[i].ki_nice >= PROCESS_IDLE_PRIORITY);
        pm.insert(std::pair<int, PROCINFO>(p.id, p));
    }
#else // !__FreeBSD__

    DIR *dir;
    dirent *piddir;
    FILE* f;
    char pidpath[MAXPATHLEN];
    char buf[2048];
    int pid = getpid();
    int retval;

    dir = opendir("/proc");
    if (!dir) {
        fprintf(stderr, "procinfo_setup(): can't open /proc\n");
        return 0;
    }

    while (1) {
        piddir = readdir(dir);
        if (!piddir) break;
        if (!isdigit(piddir->d_name[0])) continue;

#if defined(HAVE_PROCFS_H) && defined(HAVE__PROC_SELF_PSINFO)  // solaris
        psinfo_t psinfo;
        snprintf(pidpath, sizeof(pidpath), "/proc/%s/psinfo", piddir->d_name);
        f = fopen(pidpath, "r");
        if (!f) continue;
        PROCINFO p;
        p.clear();
        if (fread(&psinfo, sizeof(psinfo_t), 1, f) == 1) {
            p.id = psinfo.pr_pid;
            p.parentid = psinfo.pr_ppid;
            p.virtual_size = psinfo.pr_size*1024.;
            p.rss = psinfo.pr_rssize * 1024.;
            safe_strcpy(p.command, psinfo.pr_fname);
        }
        fclose(f);
        snprintf(pidpath, sizeof(pidpath), "/proc/%s/usage", piddir->d_name);
        prusage_t prusage;
        f = fopen(pidpath, "r");
        if (!f) continue;
        if (fread(&prusage, sizeof(prusage_t), 1, f) == 1) {
            p.user_time = (float)prusage.pr_utime.tv_sec
                + ((float)prusage.pr_utime.tv_nsec)/1e+9
            ;
            p.kernel_time = (float)prusage.pr_stime.tv_sec
                + ((float)prusage.pr_stime.tv_nsec)/1e+9
            ;
        }
        fclose(f);
        p.is_boinc_app = (p.id == pid || strcasestr(p.command, "boinc"));
        pm.insert(std::pair<int, PROCINFO>(p.id, p));
#else  // linux
        PROC_STAT ps;
        snprintf(pidpath, sizeof(pidpath), "/proc/%s/stat", piddir->d_name);
        f = fopen(pidpath, "r");
        if (!f) continue;
        if (fgets(buf, sizeof(buf), f) == NULL) {
            retval = ERR_NULL;
        } else {
            retval = ps.parse_stat(buf);
        }
        fclose(f);

        if (retval) {
            continue;
        }
        PROCINFO p;
        p.clear();
        p.id = ps.pid;
        p.parentid = ps.ppid;
        // times are in jiffies, need seconds
        // assumes 100 jiffies per second
        p.user_time = (double)ps.utime / 100.;
        p.kernel_time = (double)ps.stime / 100.;
        safe_strcpy(p.command, ps.command);
        p.is_boinc_app = (p.id == pid || strcasestr(p.command, "boinc"));
        p.is_low_priority = (ps.priority == 39);
            // Internally Linux stores the process priority as nice + 20
            // as negative values are error codes.
            // This gives a process priority range of 39..0
        pm.insert(std::pair<int, PROCINFO>(p.id, p));
#endif
    }
    closedir(dir);
#endif
    find_children(pm);
    return 0;
}

// get total CPU time (user + kernel)
// see https://www.baeldung.com/linux/get-cpu-usage
//
double total_cpu_time() {
#ifdef __FreeBSD__
// Host-wide (jail-transparent) CPU busy time via kern.cp_time.
// Unlike a /proc-based per-process walk, this is a single set of
// kernel-global tick counters covering every process on the host --
// including sibling jails and the host's own processes. This is the
// same source top(1) uses for its aggregate CPU line, and jails do not
// restrict or virtualize this sysctl the way they do the process table.
//
    long cp_time[CPUSTATES];
    size_t len = sizeof(cp_time);
    static double tick_scale = 0;

    if (!tick_scale) {
        struct clockinfo ci;
        size_t cilen = sizeof(ci);
        if (sysctlbyname("kern.clockrate", &ci, &cilen, NULL, 0) == 0
            && ci.stathz > 0
        ) {
            tick_scale = 1.0 / ci.stathz;
        } else {
            fprintf(stderr,
                "total_cpu_time(): kern.clockrate/stathz unavailable, assuming 100Hz\n"
            );
            tick_scale = 1.0 / 100.0;
        }
    }

    if (sysctlbyname("kern.cp_time", cp_time, &len, NULL, 0) != 0) {
        fprintf(stderr, "total_cpu_time(): sysctl kern.cp_time failed\n");
        return 0;
    }

    double busy_ticks = (double)(
        cp_time[CP_USER] + cp_time[CP_NICE] + cp_time[CP_SYS] + cp_time[CP_INTR]
    );
    return busy_ticks * tick_scale;

#else // !__FreeBSD__
    char buf[1024];
    static FILE *f=NULL;
    static double scale;
    if (!f) {
        f = fopen("/proc/stat", "r");
        if (!f) {
            fprintf(stderr, "can't open /proc/stat\n");
            return 0;
        }
        long hz = sysconf(_SC_CLK_TCK);
        scale = 1./(double)hz;
    } else {
        fflush(f);
        rewind(f);
    }
    if (!fgets(buf, 256, f)) {
        fprintf(stderr, "can't read /proc/stat\n");
        return 0;
    }
    double user, nice, kernel;
    int n = sscanf(buf, "cpu %lf %lf %lf", &user, &nice, &kernel);
    if (n != 3) {
        fprintf(stderr, "can't parse /proc/stat: %s\n", buf);
        return 0;
    }
    return (user+nice+kernel)*scale;
#endif
}
