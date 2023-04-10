// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2020 University of California
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

#include <vector>
#ifdef _WIN32
#include "diagnostics.h"
#include "boinc_win.h"
#include "win_util.h"
#else
#include "config.h"
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <unistd.h>

#if HAVE_CSIGNAL
#include <csignal>
#elif HAVE_SYS_SIGNAL_H
#include <sys/signal.h>
#elif HAVE_SIGNAL_H
#include <signal.h>
#endif
#endif

#include "common_defs.h"
#include "procinfo.h"
#include "str_util.h"
#include "util.h"

#include "proc_control.h"

using std::vector;

//#define DEBUG

#ifdef DEBUG
#include <stdio.h>
#endif

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

#endif


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

// chdir into the given directory, and run a program there.
// If nsecs is nonzero, make sure it's still running after that many seconds.
//
// argv is set up Unix-style, i.e. argv[0] is the program name
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

#ifdef _WIN32
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


static void get_descendants_aux(PROC_MAP& pm, int pid, vector<int>& pids) {
    PROC_MAP::iterator i = pm.find(pid);
    if (i == pm.end()) return;
    PROCINFO& p = i->second;
    if (p.scanned) return;  // avoid infinite recursion
    p.scanned = true;
    for (unsigned int j=0; j<p.children.size(); j++) {
        int child_pid = p.children[j];
        pids.push_back(child_pid);
        get_descendants_aux(pm, child_pid, pids);
    }
}

// return a list of all descendants of the given process
//
void get_descendants(int pid, vector<int>& pids) {
    int retval;
    PROC_MAP pm;
    pids.clear();
    retval = procinfo_setup(pm);
    if (retval) return;
    get_descendants_aux(pm, pid, pids);
#ifdef DEBUG
    fprintf(stderr, "descendants of %d:\n", pid);
    for (unsigned int i=0; i<pids.size(); i++) {
        fprintf(stderr, "   %d\n", pids[i]);
    }
#endif
}

#ifdef _WIN32

// Suspend or resume the threads in a set of processes,
// but don't suspend 'calling_thread'.
//
// Called from:
//  API (processes = self)
//      This handles a) throttling; b) suspend/resume by user
//  wrapper (via suspend_or_resume_process()); process = child
//  wrapper, MP case (via suspend_or_resume_decendants());
//      processes = descendants
//
// The only way to do this on Windows is to enumerate
// all the threads in the entire system,
// and identify those belonging to one of the processes (ugh!!)
//
int suspend_or_resume_threads(
    vector<int>pids, DWORD calling_thread_id, bool resume, bool check_exempt
) { 
    HANDLE threads, thread;
    THREADENTRY32 te = {0}; 
    int retval = 0;
    DWORD n;
    static vector<DWORD> suspended_threads;

#ifdef DEBUG
    fprintf(stderr, "start: check_exempt %d %s\n", check_exempt, precision_time_to_string(dtime()));
    fprintf(stderr, "%s processes", resume?"resume":"suspend");
    for (unsigned int i=0; i<pids.size(); i++) {
        fprintf(stderr, " %d", pids[i]);
    }
    fprintf(stderr, "\n");
#endif

    threads = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0); 
    if (threads == INVALID_HANDLE_VALUE) {
        fprintf(stderr, "CreateToolhelp32Snapshot failed\n");
        return -1;
    }
 
    te.dwSize = sizeof(THREADENTRY32); 
    if (!Thread32First(threads, &te)) { 
        fprintf(stderr, "Thread32First failed\n");
        CloseHandle(threads); 
        return -1;
    }

    if (!resume) {
        suspended_threads.clear();
    }

    do { 
        if (check_exempt && !diagnostics_is_thread_exempt_suspend(te.th32ThreadID)) {
#ifdef DEBUG
            fprintf(stderr, "thread is exempt\n");
#endif
            continue;
        }
#if 0
        fprintf(stderr, "thread %d PID %d %s\n",
            te.th32ThreadID, te.th32OwnerProcessID,
            precision_time_to_string(dtime())
        );
#endif
        if (te.th32ThreadID == calling_thread_id) continue;
        if (!in_vector(te.th32OwnerProcessID, pids)) continue;
        thread = OpenThread(THREAD_SUSPEND_RESUME, FALSE, te.th32ThreadID);
        if (resume) {
            // check whether we suspended this thread earlier
            //
            if (std::find(
                suspended_threads.begin(), suspended_threads.end(),
                te.th32ThreadID
            ) != suspended_threads.end()) {
                n = ResumeThread(thread);
#ifdef DEBUG
                fprintf(stderr, "ResumeThread returns %d\n", n);
#endif
            } else {
                n = 0;
            }
        } else {
            n = SuspendThread(thread);
            suspended_threads.push_back(te.th32ThreadID);
#ifdef DEBUG
            fprintf(stderr, "SuspendThread returns %d\n", n);
#endif
        }
        if (n == -1) retval = -1;
        CloseHandle(thread);
    } while (Thread32Next(threads, &te)); 

    CloseHandle (threads); 
#ifdef DEBUG
    fprintf(stderr, "end: %s\n", precision_time_to_string(dtime()));
#endif
    return retval;
} 

#else

bool any_process_exists(vector<int>& pids) {
    int status;
    for (unsigned int i=0; i<pids.size(); i++) {
        if (waitpid(pids[i], &status, WNOHANG) >= 0) {
            return true;
        }
    }
    return false;
}

#endif

void kill_all(vector<int>& pids) {
    for (unsigned int i=0; i<pids.size(); i++) {
#ifdef _WIN32
        HANDLE h = OpenProcess(READ_CONTROL | PROCESS_TERMINATE, false, pids[i]);
        if (h == NULL) continue;
        TerminateProcess(h, 0);
        CloseHandle(h);
#else
        kill(pids[i], SIGTERM);
#endif
    }
}

// Kill the descendants of the calling process.
//
#ifdef _WIN32
void kill_descendants() {
    vector<int> descendants;
    // on Win, kill descendants directly
    //
    get_descendants(GetCurrentProcessId(), descendants);
    kill_all(descendants);
}
#else
// Same, but if child_pid is nonzero,
// give it a chance to exit gracefully on Unix
//
void kill_descendants(int child_pid) {
    vector<int> descendants;
    // on Unix, ask main process nicely.
    // it descendants still exist after 10 sec, use the nuclear option
    //
    get_descendants(getpid(), descendants);
    if (child_pid) {
        ::kill(child_pid, SIGTERM);
        for (int i=0; i<10; i++) {
            if (!any_process_exists(descendants)) {
                return;
            }
            sleep(1);
        }
        kill_all(descendants);
        // kill any processes that might have been created
        // in the last 10 secs
        get_descendants(getpid(), descendants);
    }
    kill_all(descendants);
}
#endif

// suspend/resume the descendants of the calling process
// (or if pid==0, the calling process)
//
void suspend_or_resume_descendants(bool resume) {
    vector<int> descendants;
#ifdef _WIN32
    int pid = GetCurrentProcessId();
    get_descendants(pid, descendants);
    suspend_or_resume_threads(descendants, 0, resume, false);
#else
    int pid = getpid();
    get_descendants(pid, descendants);
    for (unsigned int i=0; i<descendants.size(); i++) {
        kill(descendants[i], resume?SIGCONT:SIGTSTP);
    }
#endif
}

// used by the wrapper
//
void suspend_or_resume_process(int pid, bool resume) {
#ifdef _WIN32
    vector<int> pids;
    pids.push_back(pid);
    suspend_or_resume_threads(pids, 0, resume, false);
#else
    ::kill(pid, resume?SIGCONT:SIGTSTP);
#endif
}

// return OS-specific value associated with priority code
//
int process_priority_value(int priority) {
#ifdef _WIN32
    switch (priority) {
    case PROCESS_PRIORITY_LOWEST: return IDLE_PRIORITY_CLASS;
    case PROCESS_PRIORITY_LOW: return BELOW_NORMAL_PRIORITY_CLASS;
    case PROCESS_PRIORITY_NORMAL: return NORMAL_PRIORITY_CLASS;
    case PROCESS_PRIORITY_HIGH: return ABOVE_NORMAL_PRIORITY_CLASS;
    case PROCESS_PRIORITY_HIGHEST: return HIGH_PRIORITY_CLASS;
    }
    return 0;
#else
    switch (priority) {
    case PROCESS_PRIORITY_LOWEST: return PROCESS_IDLE_PRIORITY;
    case PROCESS_PRIORITY_LOW: return PROCESS_MEDIUM_PRIORITY;
    case PROCESS_PRIORITY_NORMAL: return PROCESS_NORMAL_PRIORITY;
    case PROCESS_PRIORITY_HIGH: return PROCESS_ABOVE_NORMAL_PRIORITY;
    case PROCESS_PRIORITY_HIGHEST: return PROCESS_HIGH_PRIORITY;
    }
    return 0;
#endif
}
