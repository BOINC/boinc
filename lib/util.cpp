// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2023 University of California
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

#if defined(_WIN32)
#include "boinc_win.h"
#include "str_util.h"
#include "win_util.h"
#endif

#if defined(_MSC_VER) || defined(__MINGW32__)
#define finite   _finite
#define snprintf _snprintf
#endif

#ifndef M_LN2
#define M_LN2      0.693147180559945309417
#endif

#include "boinc_stdio.h"

#ifndef _WIN32
#include "config.h"
#if defined(__APPLE__)
#include <mach-o/dyld.h>
#endif
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/time.h>
#include <sys/wait.h>
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

#include "base64.h"
#include "common_defs.h"
#include "error_numbers.h"
#include "filesys.h"
#include "mfile.h"
#include "miofile.h"
#include "parse.h"
#include "hostinfo.h"
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

void boinc_crash() {
#ifdef _WIN32
    DebugBreak();
#else
    abort();
#endif
}

// chdir into the given directory, and run a program there.
// Don't wait for it to exit.
// argv is Unix-style, i.e. argv[0] is the program name
//
#ifdef _WIN32
int run_program(
    const char* dir, const char* file, int argc, char *const argv[], HANDLE& id
) {
    int retval;
    PROCESS_INFORMATION process_info;
    STARTUPINFOA startup_info;
    char cmdline[1024];
    char error_msg[1024];

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
        FALSE,  // don't inherit handles
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

    if (process_info.hThread) CloseHandle(process_info.hThread);
    id = process_info.hProcess;
    return 0;
}
#else
int run_program(
    const char* dir, const char* file, int , char *const argv[], int& id
) {
    int retval;
    int pid = fork();
    if (pid == 0) {
        if (dir) {
            retval = chdir(dir);
            if (retval) return retval;
        }
        execvp(file, argv);
        boinc::perror("execvp");
        boinc::fprintf(stderr, "couldn't exec %s: %d\n", file, errno);
        exit(errno);
    }
    id = pid;
    return 0;
}
#endif

// Run command, wait for exit.
// Return its output as vector of lines.
// Win: output includes stdout and stderr
// Unix: if you want stderr too, add 2>&1 to command
// Return error if command failed
//
int run_command(char *cmd, vector<string> &out) {
    out.clear();
#ifdef _WIN32
    HANDLE pipe_read, pipe_write;
    SECURITY_ATTRIBUTES sa;
    STARTUPINFO si;
    PROCESS_INFORMATION pi;

    memset(&si, 0, sizeof(si));
    memset(&pi, 0, sizeof(pi));
    memset(&sa, 0, sizeof(sa));

    sa.nLength = sizeof(SECURITY_ATTRIBUTES);
    sa.bInheritHandle = TRUE;
    sa.lpSecurityDescriptor = NULL;

    if (!CreatePipe(&pipe_read, &pipe_write, &sa, 0)) return -1;
    SetHandleInformation(pipe_read, HANDLE_FLAG_INHERIT, 0);

    si.cb = sizeof(STARTUPINFO);
    si.dwFlags |= STARTF_FORCEOFFFEEDBACK | STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
    si.wShowWindow = SW_HIDE;
    si.hStdOutput = pipe_write;
    si.hStdError = pipe_write;
    si.hStdInput = NULL;

    if (!CreateProcess(
        NULL,
        (LPTSTR)cmd,
        NULL,
        NULL,
        TRUE,   // inherit handles
        CREATE_NO_WINDOW,
        NULL,
        NULL,
        &si,
        &pi
    )) {
        return -1;
    }

    // wait for command to finish
    //
    WaitForSingleObject(pi.hProcess, INFINITE);

    unsigned long exit_code;
    GetExitCodeProcess(pi.hProcess, &exit_code);
    if (exit_code) return -1;

    DWORD count, nread;
    PeekNamedPipe(pipe_read, NULL, NULL, NULL, &count, NULL);
    if (count == 0) {
        return 0;
    }
    char* buf = (char*)malloc(count+1);
    if (!ReadFile(pipe_read, buf, count, &nread, NULL)) {
        free(buf);
        return -1;
    }
    buf[nread] = 0;
    char* p = buf;
    while (*p) {
        char* q = strchr(p, '\n');
        if (!q) break;
        *q = 0;
        out.push_back(string(p));
        p = q + 1;
    }
    free(buf);
#else
#ifndef _USING_FCGI_
    char buf[256];
    FILE* fp = popen(cmd, "r");
    if (!fp) {
        fprintf(stderr, "popen() failed: %s\n", cmd);
        return ERR_FOPEN;
    }
    while (fgets(buf, 256, fp)) {
        out.push_back(buf);
    }
    pclose(fp);
    if (errno) {
        fprintf(stderr, "popen() failed errno %d: %s\n", errno, cmd);
        return -1;
    }
#endif
#endif
    return 0;
}

#ifdef _WIN32

// run the program, and return handles to write to and read from it
//
int run_program_pipe(
    char *cmd, HANDLE &write_handle, HANDLE &read_handle, HANDLE &proc_handle
) {
    HANDLE in_read, in_write, out_read, out_write;

    SECURITY_ATTRIBUTES sa;
    STARTUPINFO si;
    PROCESS_INFORMATION pi;

    memset(&si, 0, sizeof(si));
    memset(&pi, 0, sizeof(pi));
    memset(&sa, 0, sizeof(sa));

    sa.nLength = sizeof(SECURITY_ATTRIBUTES);
    sa.bInheritHandle = TRUE;
    sa.lpSecurityDescriptor = NULL;

    if (!CreatePipe(&out_read, &out_write, &sa, 0)) return -1;
    if (!SetHandleInformation(out_read, HANDLE_FLAG_INHERIT, 0)) return -1;
    if (!CreatePipe(&in_read, &in_write, &sa, 0)) return -1;
    if (!SetHandleInformation(in_write, HANDLE_FLAG_INHERIT, 0)) return -1;

    si.cb = sizeof(STARTUPINFO);
    si.dwFlags |= STARTF_FORCEOFFFEEDBACK | STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
    si.wShowWindow = SW_HIDE;
    si.hStdOutput = out_write;
    si.hStdError = out_write;
    si.hStdInput = in_read;

    if (!CreateProcess(
        NULL,
        (LPTSTR)cmd,
        NULL,
        NULL,
        TRUE,   // inherit handles
        CREATE_NO_WINDOW,
        NULL,
        NULL,
        &si,
        &pi
    )) {
        return -1;
    }

    write_handle = in_write;
    read_handle = out_read;
    proc_handle = pi.hProcess;
    return 0;
}

int kill_process_with_status(int pid, int exit_code) {
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

int kill_process(HANDLE pid) {
    if (TerminateProcess(pid, 0)) return 0;
    return ERR_KILL;
}

#else
int kill_process(int pid) {
    if (kill(pid, SIGKILL)) {
        if (errno == ESRCH) return 0;
        return ERR_KILL;
    }
    return 0;
}
#endif

#ifdef _WIN32
int get_exit_status(HANDLE pid_handle, int &status, double dt) {
    if (dt>=0) {
        DWORD dt_msec = (DWORD)dt*1000;
        DWORD ret = WaitForSingleObject(pid_handle, dt_msec);
        if (ret == WAIT_TIMEOUT) {
            return ERR_NOT_FOUND;
        }
    } else {
        WaitForSingleObject(pid_handle, INFINITE);
    }
    unsigned long stat=1;
    GetExitCodeProcess(pid_handle, &stat);
    status = (int) stat;
    return 0;
}
#else
int get_exit_status(int pid, int &status, double dt) {
    if (dt>=0) {
        while (1) {
            int ret = waitpid(pid, &status, WNOHANG);
            if (ret > 0) return 0;
            dt -= 1;
            if (dt<0) break;
            boinc_sleep(1);
        }
        return ERR_NOT_FOUND;
    } else {
        waitpid(pid, &status, 0);
    }
    return 0;
}
#endif

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

// get the path of the calling process's executable
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
                boinc::perror("readlink");
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

#ifdef _WIN32

int boinc_thread_cpu_time(HANDLE thread_handle, double& cpu) {
    FILETIME creationTime, exitTime, kernelTime, userTime;

    if (GetThreadTimes(
        thread_handle, &creationTime, &exitTime, &kernelTime, &userTime)
        ) {
        ULARGE_INTEGER tKernel, tUser;
        LONGLONG totTime;

        tKernel.LowPart = kernelTime.dwLowDateTime;
        tKernel.HighPart = kernelTime.dwHighDateTime;
        tUser.LowPart = userTime.dwLowDateTime;
        tUser.HighPart = userTime.dwHighDateTime;
        totTime = tKernel.QuadPart + tUser.QuadPart;

        // Runtimes in 100-nanosecond units
        cpu = totTime / 1.e7;
    }
    else {
        return -1;
    }
    return 0;
}

static void get_elapsed_time(double& cpu) {
    static double start_time;

    double now = dtime();
    if (start_time) {
        cpu = now - start_time;
    }
    else {
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

int boinc_process_cpu_time(HANDLE process_handle, double& cpu) {
    FILETIME creationTime, exitTime, kernelTime, userTime;

    if (GetProcessTimes(
        process_handle, &creationTime, &exitTime, &kernelTime, &userTime)
        ) {
        ULARGE_INTEGER tKernel, tUser;
        LONGLONG totTime;

        tKernel.LowPart = kernelTime.dwLowDateTime;
        tKernel.HighPart = kernelTime.dwHighDateTime;
        tUser.LowPart = userTime.dwLowDateTime;
        tUser.HighPart = userTime.dwHighDateTime;
        totTime = tKernel.QuadPart + tUser.QuadPart;

        // Runtimes in 100-nanosecond units
        cpu = totTime / 1.e7;
    }
    else {
        return -1;
    }
    return 0;
}

bool process_exists(HANDLE h) {
    unsigned long status = 1;
    if (GetExitCodeProcess(h, &status)) {
        if (status == STILL_ACTIVE) return true;
    }
    return false;
}

#else   // _WIN32

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

#ifndef _USING_FCGI_
// (linux) return current CPU time of the given process
//
double linux_cpu_time(int pid) {
    FILE* file;
    char file_name[24];
    unsigned long utime = 0, stime = 0;
    int n;

    snprintf(file_name, sizeof(file_name), "/proc/%d/stat", pid);
    if ((file = fopen(file_name, "r")) != NULL) {
        n = fscanf(file, "%*s%*s%*s%*s%*s%*s%*s%*s%*s%*s%*s%*s%*s%lu%lu", &utime, &stime);
        fclose(file);
        if (n != 2) return 0;
    }
    return (double)(utime + stime) / 100;
}
#endif

bool process_exists(int pid) {
    int retval = kill(pid, 0);
    if (retval == -1 && errno == ESRCH) return false;
    return true;
}

#endif  // _WIN32

#ifndef _USING_FCGI_

string parse_ldd_libc(const char* input) {
    char *q = (char*)strchr(input, '\n');
    if (q) *q = 0;
    const char *p = strrchr(input, ' ');
    if (!p) return "";
    int maj, min;
    if (sscanf(p, "%d.%d", &maj, &min) != 2) return "";
    string s = (string)p;
    strip_whitespace(s);
    return s;
}

// Set up to issue Docker commands.
// On Win this requires connecting to a shell in the WSL distro
//
#ifdef _WIN32
int DOCKER_CONN::init(
    DOCKER_TYPE docker_type, string distro_name, bool _verbose
) {
    string err_msg;
    cli_prog = docker_cli_prog(docker_type);
    if (docker_type == DOCKER) {
        int retval = ctl_wc.setup(err_msg);
        if (retval) return retval;
        retval = ctl_wc.run_program_in_wsl(distro_name, "", true);
        if (retval) return retval;
    } else if (docker_type == PODMAN) {
        int retval = ctl_wc.setup_root(distro_name.c_str());
        if (retval) return retval;
    } else {
        return -1;
    }
    verbose = _verbose;
    return 0;
}
#else
int DOCKER_CONN::init(DOCKER_TYPE docker_type, bool _verbose) {
    cli_prog = docker_cli_prog(docker_type);
    verbose = _verbose;
    return 0;
}
#endif

// issue a Docker command and return its output in out
//
int DOCKER_CONN::command(const char* cmd, vector<string> &out) {
    char buf[1024];
    int retval;
    if (verbose) {
        fprintf(stderr, "running docker command: %s\n", cmd);
    }
#ifdef _WIN32
    string output;

    sprintf(buf, "%s %s; echo EOM\n", cli_prog, cmd);
    write_to_pipe(ctl_wc.in_write, buf);
    retval = read_from_pipe(
        ctl_wc.out_read, ctl_wc.proc_handle, output, TIMEOUT, "EOM"
    );
    if (retval) {
        fprintf(stderr, "read_from_pipe() error: %s\n", boincerror(retval));
        return retval;
    }
    out = split(output, '\n');
#else
    sprintf(buf, "%s %s\n", cli_prog, cmd);
    retval = run_command(buf, out);
    if (retval) {
        if (verbose) {
            fprintf(stderr, "command failed: %s\n", boincerror(retval));
        }
        return retval;
    }
#endif
    if (verbose) {
        fprintf(stderr, "command output:\n");
        for (string line: out) {
            fprintf(stderr, "%s\n", line.c_str());
        }
    }
    return 0;
}

// parse the output of 'docker images'
// from the following, return 'boinc__app_test__test_wu'
//
// REPOSITORY                          TAG         IMAGE ID      CREATED       SIZE
// localhost/boinc__app_test__test_wu  latest      cbc1498dfc49  43 hours ago  121 MB
//
int DOCKER_CONN::parse_image_name(string line, string &name) {
    char buf[1024];
    strcpy(buf, line.c_str());
    if (strstr(buf, "REPOSITORY")) return -1;
    if (strstr(buf, "localhost/") != buf) return -1;
    char *p = buf + strlen("localhost/");
    char *q = strstr(p, " ");
    if (!q) return -1;
    *q = 0;
    name = (string)p;
    return 0;
}

// parse the output of 'docker ps -all'.
// from the following, return boinc__app_test__test_result
//
// CONTAINER ID  IMAGE                                      COMMAND               CREATED        STATUS                   PORTS       NAMES
// 6d4877e0d071  localhost/boinc__app_test__test_wu:latest  /bin/sh -c ./work...  43 hours ago   Exited (0) 21 hours ago              boinc__app_test__test_result
//
int DOCKER_CONN::parse_container_name(string line, string &name) {
    char buf[1024];
    strcpy(buf, line.c_str());
    if (strstr(buf, "CONTAINER")) return -1;
    char *p = strrchr(buf, ' ');
    if (!p) return -1;
    name = (string)(p+1);
    return 0;
}

// we name Docker images so that they're
// - distinguishable from non-BOINC images (hence boinc__)
// - unique per WU (hence projurl__wuname)
// - lowercase (required by Docker)
//
string docker_image_name(
    const char* proj_url_esc, const char* wu_name
) {
    char buf[1024], url_buf[1024], wu_buf[1024];

    safe_strcpy(url_buf, proj_url_esc);
    downcase_string(url_buf);
    safe_strcpy(wu_buf, wu_name);
    downcase_string(wu_buf);

    sprintf(buf, "boinc__%s__%s", url_buf, wu_buf);
    return string(buf);
}

// similar for Docker container names,
// but they're unique per result rather than per WU
//
string docker_container_name(
    const char* proj_url_esc, const char* result_name
){
    char buf[1024], url_buf[1024], result_buf[1024];

    safe_strcpy(url_buf, proj_url_esc);
    downcase_string(url_buf);
    safe_strcpy(result_buf, result_name);
    downcase_string(result_buf);

    sprintf(buf, "boinc__%s__%s", url_buf, result_buf);
    return string(buf);
}

bool docker_is_boinc_name(const char* name) {
    return strstr(name, "boinc__") == name;
}
#endif  // _USING_FCGI
