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

#if   defined(_WIN32) && !defined(__STDWX_H__)
#include "boinc_win.h"
#elif defined(_WIN32) && defined(__STDWX_H__)
#include "stdwx.h"
#endif
#ifdef _WIN32
#include "win_util.h"
#ifdef _MSC_VER
#define finite _finite
#endif
#endif

#ifndef M_LN2
#define M_LN2      0.693147180559945309417
#endif

#ifndef _WIN32
#include "config.h"
#if HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <sys/types.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <signal.h>
#include <sys/resource.h>
#include <errno.h>
#include <string>
#include <cstring>
#if HAVE_IEEEFP_H
#include <ieeefp.h>
extern "C" {
    int finite(double);
}
#endif
#endif

#include "error_numbers.h"
#include "common_defs.h"
#include "filesys.h"
#include "util.h"
#include "base64.h"
#include "mfile.h"
#include "miofile.h"
#include "parse.h"


#ifdef _USING_FCGI_
#include "boinc_fcgi.h"
#define perror FCGI::perror
#endif

using std::min;
using std::string;
using std::vector;

#define EPOCHFILETIME_SEC (11644473600.)
#define TEN_MILLION 10000000.

#ifdef GCL_SIMULATOR
double simtime;
#endif

// return time of day (seconds since 1970) as a double
//
double dtime() {
#ifdef GCL_SIMULATOR
    return simtime;
#else
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
    double end_time = dtime() + seconds - 0.01;
    // sleep() and usleep() can be interrupted by SIGALRM,
    // so we may need multiple calls
    //
    while (1) {
        if (seconds >= 1) {
            sleep((unsigned int) seconds);
        } else {
            usleep((int)fmod(seconds*1000000, 1000000));
        }
        seconds = end_time - dtime();
        if (seconds <= 0) break;
    }
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

int boinc_process_cpu_time(HANDLE process_handle, double& cpu) {
    FILETIME creationTime, exitTime, kernelTime, userTime;

    if (GetProcessTimes(
        process_handle, &creationTime, &exitTime, &kernelTime, &userTime)
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

// Unix: pthreads doesn't provide an API for getting per-thread CPU time,
// so just get the process's CPU time
//
int boinc_calling_thread_cpu_time(double &cpu_t) {
    struct rusage ru;

    int retval = getrusage(RUSAGE_SELF, &ru);
    if (retval) return ERR_GETRUSAGE;
    cpu_t = (double)ru.ru_utime.tv_sec + ((double)ru.ru_utime.tv_usec) / 1e6;
    cpu_t += (double)ru.ru_stime.tv_sec + ((double)ru.ru_stime.tv_usec) / 1e6;
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
    double now,
    double work_start_time,       // when new work was started
                                    // (or zero if no new work)
    double work,                    // amount of new work
    double half_life,
    double& avg,                    // average work per day (in and out)
    double& avg_time                // when average was last computed
) {
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
int read_file_malloc(const char* path, char*& buf, size_t max_len, bool tail) {
    int retval;
    double size;

    retval = file_size(path, size);
    if (retval) return retval;

    // Note: the fseek() below won't work unless we use binary mode in fopen

#ifndef _USING_FCGI_
    FILE *f = fopen(path, "rb");
#else
    FCGI_FILE *f = FCGI::fopen(path, "rb");
#endif
    if (!f) return ERR_FOPEN;

#ifndef _USING_FCGI_
    if (max_len && size > max_len) {
        if (tail) {
            fseek(f, (long)size-(long)max_len, SEEK_SET);
        }
        size = max_len;
    }
#endif
    size_t isize = (size_t)size;
    buf = (char*)malloc(isize+1);
    if (!buf) {
        fclose(f);
        return ERR_MALLOC;
    }
    size_t n = fread(buf, 1, isize, f);
    buf[n] = 0;
    fclose(f);
    return 0;
}

// read file (at most max_len chars, if nonzero) into string
//
int read_file_string(
    const char* path, string& result, size_t max_len, bool tail
) {
    result.erase();
    int retval;
    char* buf;

    retval = read_file_malloc(path, buf, max_len, tail);
    if (retval) return retval;
    result = buf;
    free(buf);
    return 0;
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
    STARTUPINFOA startup_info;
    char cmdline[1024];
    char error_msg[1024];
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

    retval = CreateProcessA(
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
        windows_error_string(error_msg, sizeof(error_msg));
        fprintf(stderr, "CreateProcess failed: '%s'\n", error_msg);
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
            } else {
                break;
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
    strcat(buf, RUN_MUTEX);

    HANDLE h = CreateMutexA(NULL, true, buf);
    if ((h==0) || (GetLastError() == ERROR_ALREADY_EXISTS)) {
        return ERR_ALREADY_RUNNING;
    }
#else
static int get_client_mutex(const char* dir) {
    char path[MAXPATHLEN];
    static FILE_LOCK file_lock;

    sprintf(path, "%s/%s", dir, LOCK_FILE_NAME);
    int retval = file_lock.lock(path);
    if (retval == ERR_FCNTL) {
        return ERR_ALREADY_RUNNING;
    } else if (retval) {
        return retval;
    }
#endif
    return 0;
}

int wait_client_mutex(const char* dir, double timeout) {
    double start = dtime();
    int retval = 0;
    while (1) {
        retval = get_client_mutex(dir);
        if (!retval) return 0;
        boinc_sleep(1);
        if (dtime() - start > timeout) break;
    }
    return retval;
}

bool boinc_is_finite(double x) {
#if defined (HPUX_SOURCE)
    return _Isfinite(x);
    return false;
#else
    return finite(x) != 0;
#endif
}

#define PI2 (2*3.1415926)

// generate normal random numbers using Box-Muller.
// this generates 2 at a time, so cache the other one
//
double rand_normal() {
    static bool cached;
    static double cached_value;
    if (cached) {
        cached = false;
        return cached_value;
    }
    double u1 = drand();
    double u2 = drand();
    double z = sqrt(-2*log(u1));
    cached_value = z*sin(PI2*u2);
    cached = true;
    return z*cos(PI2*u2);
}
