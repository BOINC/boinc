// Berkeley Open Infrastructure for Network Computing
// http://boinc.berkeley.edu
// Copyright (C) 2005 University of California
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
// 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

#if defined(_WIN32) && !defined(__STDWX_H__) && !defined(_BOINC_WIN_) && !defined(_AFX_STDAFX_H_)
#include "boinc_win.h"
#endif
#ifdef _WIN32
#include "win_util.h"
#endif

#ifndef M_LN2
#define M_LN2      0.693147180559945309417
#endif

#ifndef _WIN32
#include "config.h"
#include <string>
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <cmath>
#include <algorithm>
#include <iterator>
#include <iostream>
#include <fstream>
#include <cctype>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <grp.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <unistd.h>
#include <sys/file.h>
#endif


#include "error_numbers.h"
#include "common_defs.h"
#include "filesys.h"
#include "util.h"

#ifdef _USING_FCGI_
#include "fcgi_stdio.h"
#endif

using std::min;
using std::string;
using std::vector;

#define EPOCHFILETIME_SEC (11644473600.)
#define TEN_MILLION 10000000.

// return time of day (seconds since 1970) as a double
//
double dtime() {
#ifdef _WIN32
    LARGE_INTEGER time;
    FILETIME sysTime;
    double t;
    GetSystemTimeAsFileTime(&sysTime);
    time.LowPart = sysTime.dwLowDateTime;
    time.HighPart = sysTime.dwHighDateTime;  // Time is in 100 ns units
    t = (double)time.QuadPart;    // Convert to 1 s units
    t /= TEN_MILLION;                /* In seconds */
    t -= EPOCHFILETIME_SEC;     /* Offset to the Epoch time */
    return t;
#else
    struct timeval tv;
    gettimeofday(&tv, 0);
    return tv.tv_sec + (tv.tv_usec/1.e6);
#endif
}

// return time today 0:00 in seconds since 1970 as a double
//
double dday() {
    double now=dtime();
    return (now-fmod(now, SECONDS_PER_DAY));
}

// sleep for a specified number of seconds
//
void boinc_sleep(double seconds) {
#ifdef _WIN32
    ::Sleep((int)(1000*seconds));
#else
    unsigned int rem = (int) seconds;
    while (1) {
        rem = sleep(rem);
        if (rem == 0) break;
        if (rem > seconds) break;   // paranoia
    }
    int x = (int)fmod(seconds*1000000, 1000000);
    if (x) usleep(x);
#endif
}

void push_unique(string s, vector<string>& v) {
    for (unsigned int i=0; i<v.size();i++) {
        if (s == v[i]) return;
    }
    v.push_back(s);
}

#ifdef _WIN32

int boinc_thread_cpu_time(HANDLE thread_handle, double& cpu) {
    FILETIME creationTime, exitTime, kernelTime, userTime;

    if (GetThreadTimes(
        thread_handle, &creationTime, &exitTime, &kernelTime, &userTime)
    ) {
        ULARGE_INTEGER tKernel, tUser;
        LONGLONG totTime;

        tKernel.LowPart  = kernelTime.dwLowDateTime;
        tKernel.HighPart = kernelTime.dwHighDateTime;
        tUser.LowPart    = userTime.dwLowDateTime;
        tUser.HighPart   = userTime.dwHighDateTime;
        totTime = tKernel.QuadPart + tUser.QuadPart;

        // Runtimes in 100-nanosecond units
        cpu = totTime / 1.e7;
    } else {
        return -1;
    }
    return 0;
}

int boinc_process_cpu_time(double& cpu) {
    FILETIME creationTime, exitTime, kernelTime, userTime;

    if (GetProcessTimes(
        GetCurrentProcess(), &creationTime, &exitTime, &kernelTime, &userTime)
    ) {
        ULARGE_INTEGER tKernel, tUser;
        LONGLONG totTime;

        tKernel.LowPart  = kernelTime.dwLowDateTime;
        tKernel.HighPart = kernelTime.dwHighDateTime;
        tUser.LowPart    = userTime.dwLowDateTime;
        tUser.HighPart   = userTime.dwHighDateTime;
        totTime = tKernel.QuadPart + tUser.QuadPart;

        // Runtimes in 100-nanosecond units
        cpu = totTime / 1.e7;
    } else {
        return -1;
    }
    return 0;
}

static void get_elapsed_time(double& cpu) {
    static double start_time;

    double now = dtime();
    if (start_time) {
        cpu = now - start_time;
    } else {
        cpu = 0;
    }
    start_time = now;
}

int boinc_calling_thread_cpu_time(double& cpu) {
    if (boinc_thread_cpu_time(GetCurrentThread(), cpu)) {
        get_elapsed_time(cpu);
    }
    return 0;
}

#else

pthread_mutex_t getrusage_mutex = PTHREAD_MUTEX_INITIALIZER;

// Unix: pthreads doesn't seem to provide an API for getting
// per-thread CPU time.  So just get the process's CPU time
//
int boinc_calling_thread_cpu_time(double &cpu_t) {
    int retval=1;
    struct rusage ru;

    // getrusage can return an error, so try a few times if it returns an error.
    //
    if (!pthread_mutex_trylock(&getrusage_mutex)) {
        for (int i=0; i<10; i++) {
            retval = getrusage(RUSAGE_SELF, &ru);
            if (!retval) break;
        }
        pthread_mutex_unlock(&getrusage_mutex);
    }
    if (retval) {
        return ERR_GETRUSAGE;
    }
    // Sum the user and system time
    //
    cpu_t = (double)ru.ru_utime.tv_sec + (((double)ru.ru_utime.tv_usec) / ((double)1000000.0));
    cpu_t += (double)ru.ru_stime.tv_sec + (((double)ru.ru_stime.tv_usec) / ((double)1000000.0));
    return 0;
}

#endif


// Update an estimate of "units per day" of something (credit or CPU time).
// The estimate is exponentially averaged with a given half-life
// (i.e. if no new work is done, the average will decline by 50% in this time).
// This function can be called either with new work,
// or with zero work to decay an existing average.
//
// NOTE: if you change this, also change update_average in
// html/inc/credit.inc
//
void update_average(
    double work_start_time,       // when new work was started
                                    // (or zero if no new work)
    double work,                    // amount of new work
    double half_life,
    double& avg,                    // average work per day (in and out)
    double& avg_time                // when average was last computed
) {
    double now = dtime();

    if (avg_time) {
        // If an average R already exists, imagine that the new work was done
        // entirely between avg_time and now.
        // That gives a rate R'.
        // Replace R with a weighted average of R and R',
        // weighted so that we get the right half-life if R' == 0.
        //
        // But this blows up if avg_time == now; you get 0*(1/0)
        // So consider the limit as diff->0,
        // using the first-order Taylor expansion of
        // exp(x)=1+x+O(x^2).
        // So to the lowest order in diff:
        // weight = 1 - diff ln(2) / half_life
        // so one has
        // avg += (1-weight)*(work/diff_days)
        // avg += [diff*ln(2)/half_life] * (work*SECONDS_PER_DAY/diff)
        // notice that diff cancels out, leaving
        // avg += [ln(2)/half_life] * work*SECONDS_PER_DAY

        double diff, diff_days, weight;

        diff = now - avg_time;
        if (diff<0) diff=0;

        diff_days = diff/SECONDS_PER_DAY;
        weight = exp(-diff*M_LN2/half_life);

        avg *= weight;

        if ((1.0-weight) > 1.e-6) {
            avg += (1-weight)*(work/diff_days);
        } else {
            avg += M_LN2*work*SECONDS_PER_DAY/half_life;
        }
    } else if (work) {
        // If first time, average is just work/duration
        //
        double dd = (now - work_start_time)/SECONDS_PER_DAY;
        avg = work/dd;
    }
    avg_time = now;
}

// chdir into the given directory, and run a program there.
// If nsecs is nonzero, make sure it's still running after that many seconds.
//
// argv is set up Unix-style, i.e. argv[0] is the program name
//
#ifdef _WIN32
int run_program(
    const char* dir, const char* file, int argc, char *const argv[], double nsecs, HANDLE& id
) {
    int retval;
    PROCESS_INFORMATION process_info;
    STARTUPINFO startup_info;
    char cmdline[1024];
    unsigned long status;

    memset(&process_info, 0, sizeof(process_info));
    memset(&startup_info, 0, sizeof(startup_info));
    startup_info.cb = sizeof(startup_info);
             
    strcpy(cmdline, "");
    for (int i=0; i<argc; i++) {
        strcat(cmdline, argv[i]);
        if (i<argc-1) {
            strcat(cmdline, " ");
        }
    }

    retval = CreateProcess(
        file,
        cmdline,
        NULL,
        NULL,
        FALSE,
        0,
        NULL,
        dir,
        &startup_info,
        &process_info
    );
    if (!retval) {
        return -1; // CreateProcess returns 1 if successful, false if it failed.
    }
    if (nsecs) {
        boinc_sleep(nsecs);
        if (GetExitCodeProcess(process_info.hProcess, &status)) {
            if (status != STILL_ACTIVE) {
                return -1;
            }
        }
    }
    id = process_info.hProcess;
    return 0;
}
#else
int run_program(
    const char* dir, const char* file, int , char *const argv[], double nsecs, int& id
) {
    int retval;
    int pid = fork();
    if (pid == 0) {
        if (dir) {
            retval = chdir(dir);
            if (retval) return retval;
        }
        execv(file, argv);
        perror("execv");
        exit(errno);
    }

    if (nsecs) {
        boinc_sleep(3);
        if (waitpid(pid, 0, WNOHANG) == pid) {
            return -1;
        }
    }
    id = pid;
    return 0;
}
#endif

#ifdef _WIN32
void kill_program(HANDLE pid) {
    TerminateProcess(pid, 0);
}
#else
void kill_program(int pid) {
    kill(pid, SIGKILL);
}
#endif

#ifdef _WIN32
int get_exit_status(HANDLE pid_handle) {
    unsigned long status=1;
    while (1) {
        if (GetExitCodeProcess(pid_handle, &status)) {
            if (status == STILL_ACTIVE) {
                boinc_sleep(1);
            }
        }
    }
    return (int) status;
}
bool process_exists(HANDLE h) {
    unsigned long status=1;
    if (GetExitCodeProcess(h, &status)) {
        if (status == STILL_ACTIVE) return true;
    }
    return false;
}

#else
int get_exit_status(int pid) {
    int status;
    waitpid(pid, &status, 0);
    return status;
}
bool process_exists(int pid) {
    int p = waitpid(pid, 0, WNOHANG);
    if (p == pid) return false;     // process has exited
    if (p == -1) return false;      // PID doesn't exist
    return true;
}
#endif

#ifdef _WIN32
static int get_client_mutex(const char*) {
    char buf[MAX_PATH] = "";
    
    // Global mutex on Win2k and later
    //
    if (IsWindows2000Compatible()) {
        strcpy(buf, "Global\\");
    }
    strcat( buf, RUN_MUTEX);

    HANDLE h = CreateMutex(NULL, true, buf);
    if ((h==0) || (GetLastError() == ERROR_ALREADY_EXISTS)) {
        return ERR_ALREADY_RUNNING;
    }
#else
static int get_client_mutex(const char* dir) {
    char path[1024];
    static FILE_LOCK file_lock;

    sprintf(path, "%s/%s", dir, LOCK_FILE_NAME);
    if (file_lock.lock(path)) {
        return ERR_ALREADY_RUNNING;
    }
#endif
    return 0;
}

int wait_client_mutex(const char* dir, double timeout) {
    double start = dtime();
    while (1) {
        int retval = get_client_mutex(dir);
        if (!retval) return 0;
        boinc_sleep(1);
        if (dtime() - start > timeout) break;
    }
    return ERR_ALREADY_RUNNING;
}

#ifndef _USING_FCGI_
#ifndef _WIN32
// (linux) return current CPU time of the given process
//
double linux_cpu_time(int pid) {
    FILE *file;
    char file_name[24];
    unsigned long utime = 0, stime = 0;
    int n;

    sprintf(file_name,"/proc/%d/stat",pid);
    if ((file = fopen(file_name,"r")) != NULL) {
        n = fscanf(file,"%*s%*s%*s%*s%*s%*s%*s%*s%*s%*s%*s%*s%*s%lu%lu",&utime,&stime);
        fclose(file);
        if (n != 2) return 0;
    }
    return (double)(utime + stime)/100;
}
#endif
#endif

void boinc_crash() {
#ifdef _WIN32
	DebugBreak();
#else
	*(int*)0 = 0;
#endif
}

// read file (at most max_len chars, if nonzero) into malloc'd buf
//
int read_file_malloc(const char* path, char*& buf, int max_len, bool tail) {
    FILE* f;
    int retval, isize;
    double size;

    retval = file_size(path, size);
    if (retval) return retval;

    f = fopen(path, "r");
    if (!f) return ERR_FOPEN;

    if (max_len && size > max_len) {
        if (tail) {
            fseek(f, (long)size-max_len, SEEK_SET);
        }
        size = max_len;
    }
    isize = (int) size;
    buf = (char*)malloc(isize+1);
    size_t n = fread(buf, 1, isize, f);
    buf[n] = 0;
    fclose(f);
    return 0;
}

// read file (at most max_len chars, if nonzero) into string
//
int read_file_string(const char* path, string& result, int max_len, bool tail) {
    result.erase();
    int retval;
    char* buf;

    retval = read_file_malloc(path, buf, max_len, tail);
    if (retval) return retval;
    result = buf;
    free(buf);
    return 0;
}

const char *BOINC_RCSID_ab65c90e1e = "$Id$";
