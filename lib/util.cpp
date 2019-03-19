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

#ifdef  _WIN32
#ifndef __STDWX_H__
#include "boinc_win.h"
#else
#include "stdwx.h"
#endif
#include "str_replace.h"
#include "win_util.h"
#endif

#if defined(_MSC_VER) || defined(__MINGW32__)
#define finite   _finite
#define snprintf _snprintf
#endif

#ifndef M_LN2
#define M_LN2      0.693147180559945309417
#endif

#ifdef _USING_FCGI_
#include "boinc_fcgi.h"
#define perror FCGI::perror
#endif

#ifndef _WIN32
#include "config.h"
#if defined(__APPLE__)
#include <mach-o/dyld.h>
#endif
#if HAVE_UNISTD_H
#include <unistd.h>
#endif
#if HAVE_SYS_SYSCTL_H
#include <sys/sysctl.h>
#endif
#include <sys/types.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <signal.h>
#include <sys/resource.h>
#include <errno.h>
#include <string>
#include <cstring>
#include <cmath>
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
#include "base64.h"
#include "mfile.h"
#include "miofile.h"
#include "parse.h"

#include "util.h"

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

    snprintf(file_name, sizeof(file_name), "/proc/%d/stat", pid);
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
    abort();
#endif
}

// read file (at most max_len chars, if nonzero) into malloc'd buf
//
#ifdef _USING_FCGI_
int read_file_malloc(const char* path, char*& buf, size_t, bool) {
#else
int read_file_malloc(const char* path, char*& buf, size_t max_len, bool tail) {
#endif
    int retval;
    double size;

    // Win: if another process has this file open for writing,
    // wait for up to 5 seconds.
    // This is because when a job exits, the write to stderr.txt
    // sometimes (inexplicably) doesn't appear immediately

#ifdef _WIN32
    for (int i=0; i<5; i++) {
        HANDLE h = CreateFileA(
            path,
            GENERIC_WRITE,
            0,
            NULL,
            OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL,
            NULL
        );
        if (h != INVALID_HANDLE_VALUE) {
            CloseHandle(h);
            break;
        }
        boinc_sleep(1);
    }
#endif

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

    // lpApplicationName needs to be NULL for CreateProcess to search path
    // but argv[0] may be full path or just filename
    // 'file' should be something runnable so use that as program name
    snprintf(cmdline, sizeof(cmdline), "\"%s\"", file);
    for (int i=1; i<argc; i++) {
        safe_strcat(cmdline, " ");
        safe_strcat(cmdline, argv[i]);
    }

    retval = CreateProcessA(
        NULL,
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
        windows_format_error_string(GetLastError(), error_msg, sizeof(error_msg));
        fprintf(stderr,
            "%s: CreateProcess failed: '%s'\n",
            time_to_string(dtime()), error_msg
        );
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
    if (process_info.hThread) CloseHandle(process_info.hThread);
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
        execvp(file, argv);
#ifdef _USING_FCGI_
        FCGI::perror("execvp");
#else
        perror("execvp");
        fprintf(stderr, "couldn't exec %s: %d\n", file, errno);
#endif
        exit(errno);
    }

    if (nsecs) {
        boinc_sleep(nsecs);
        if (waitpid(pid, 0, WNOHANG) == pid) {
            return -1;
        }
    }
    id = pid;
    return 0;
}
#endif

#ifdef _WIN32
int kill_program(int pid, int exit_code) {
    int retval;

    HANDLE h = OpenProcess(PROCESS_TERMINATE, false, pid);
    if (h == NULL) return 0;
        // process isn't there, so no error

    if (TerminateProcess(h, exit_code)) {
        retval = 0;
    } else {
        retval = ERR_KILL;
    }
    CloseHandle(h);
    return retval;
}

int kill_program(HANDLE pid) {
    if (TerminateProcess(pid, 0)) return 0;
    return ERR_KILL;
}

#else
int kill_program(int pid) {
    if (kill(pid, SIGKILL)) {
        if (errno == ESRCH) return 0;
        return ERR_KILL;
    }
    return 0;
}
#endif

#ifdef _WIN32
int get_exit_status(HANDLE pid_handle) {
    unsigned long status=1;
    WaitForSingleObject(pid_handle, INFINITE);
    GetExitCodeProcess(pid_handle, &status);
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
    int retval = kill(pid, 0);
    if (retval == -1 && errno == ESRCH) return false;
    return true;
}
#endif

#ifdef _WIN32
static int get_client_mutex(const char*) {
    char buf[MAX_PATH] = "";
    
    // Global mutex on Win2k and later
    //
    safe_strcpy(buf, "Global\\");
    safe_strcat(buf, RUN_MUTEX);

    HANDLE h = CreateMutexA(NULL, true, buf);
    if ((h==0) || (GetLastError() == ERROR_ALREADY_EXISTS)) {
        return ERR_ALREADY_RUNNING;
    }
#else
static int get_client_mutex(const char* dir) {
    char path[MAXPATHLEN];
    static FILE_LOCK file_lock;

    snprintf(path, sizeof(path), "%s/%s", dir, LOCK_FILE_NAME);
    path[sizeof(path)-1] = 0;

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
#elif defined (__APPLE__)
    // finite() is deprecated in OS 10.9
    return std::isfinite(x) != 0;
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

// determines the real path and filename of the current process
// not the current working directory
//
int get_real_executable_path(char* path, size_t max_len) {
#if defined(__APPLE__)
    uint32_t size = (uint32_t)max_len;
    if (_NSGetExecutablePath(path, &size)) {
        return ERR_BUFFER_OVERFLOW;
    }
    return BOINC_SUCCESS;
#elif (defined(__DragonFly__) || defined(__FreeBSD__) || defined(__NetBSD__)) && defined(KERN_PROC_PATHNAME)
#if defined(__DragonFly__) || defined(__FreeBSD__)
    int name[4] = { CTL_KERN, KERN_PROC, KERN_PROC_PATHNAME, -1 };
#else
    int name[4] = { CTL_KERN, KERN_PROC_ARGS, -1, KERN_PROC_PATHNAME };
#endif
    if (sysctl(name, 4, path, &max_len, NULL, 0)) {
        return errno == ENOMEM ? ERR_BUFFER_OVERFLOW : ERR_PROC_PARSE;
    }
    return BOINC_SUCCESS;
#elif defined(_WIN32)
    DWORD length = GetModuleFileNameA(NULL, path, (DWORD)max_len);
    if (!length) {
        return ERR_PROC_PARSE;
    } else if (length == (DWORD)max_len) {
        return ERR_BUFFER_OVERFLOW;
    }
    return BOINC_SUCCESS;
#else
    const char* links[] = { "/proc/self/exe", "/proc/curproc/exe", "/proc/self/path/a.out", "/proc/curproc/file" };
    for (unsigned int i = 0; i < sizeof(links) / sizeof(links[0]); ++i) {
        ssize_t ret = readlink(links[i], path, max_len - 1);
        if (ret < 0) {
            if (errno != ENOENT) {
#ifdef _USING_FCGI_
                FCGI::perror("readlink");
#else
                perror("readlink");
#endif
            }
            continue;
        } else if ((size_t)ret == max_len - 1) {
            return ERR_BUFFER_OVERFLOW;
        }
        path[ret] = '\0'; // readlink does not null terminate
        return BOINC_SUCCESS;
    }
    return ERR_NOT_IMPLEMENTED;
#endif
}
