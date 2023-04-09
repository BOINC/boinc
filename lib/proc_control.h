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

// functions for creating, controlling, and querying processes

#ifndef BOINC_PROC_CONTROL_H
#define BOINC_PROC_CONTROL_H

#include <vector>
#ifdef _WIN32
#include <windows.h>
#endif

#ifdef _WIN32
#include "boinc_win.h"
extern int boinc_thread_cpu_time(HANDLE thread_handle, double& cpu);
extern int boinc_process_cpu_time(HANDLE process_handle, double& cpu);
#else
// setpriority(2) arg to run in background
// (don't use 20 because
//
static const int PROCESS_IDLE_PRIORITY = 19;
static const int PROCESS_MEDIUM_PRIORITY = 10;
static const int PROCESS_NORMAL_PRIORITY = 0;
static const int PROCESS_ABOVE_NORMAL_PRIORITY = -10;
static const int PROCESS_HIGH_PRIORITY = -15;
static const int PROCESS_REALTIME_PRIORITY = -20;
extern double linux_cpu_time(int pid);
#endif

extern int boinc_calling_thread_cpu_time(double&);

#ifdef _WIN32
extern int run_program(
    const char* dir,    // directory to run program in; NULL if current dir
    const char* file,   // path of executable
    int argc, char *const argv[],   // cmdline args, UNIX-style
    double,             // if nonzero, wait for X seconds, then check
                        // whether process is still running, return error if not
    HANDLE&             // process handle
);

extern int kill_program(HANDLE);
extern int kill_program(int, int exit_code=0);
extern int get_exit_status(HANDLE);
extern bool process_exists(HANDLE);

#else
// like Win version, but returns PID
extern int run_program(
    const char* dir, const char* file, int argc, char *const argv[], double, int&
);
extern int kill_program(int);
extern int get_exit_status(int);
extern bool process_exists(int);
#endif

extern int get_real_executable_path(char* path, size_t max_len);

extern void get_descendants(int pid, std::vector<int>& pids);
extern bool any_process_exists(std::vector<int>& pids);
extern void kill_all(std::vector<int>& pids);
#ifdef _WIN32
extern void kill_descendants();
extern int suspend_or_resume_threads(
    std::vector<int> pids, DWORD threadid, bool resume, bool check_exempt
);
#else
extern void kill_descendants(int child_pid=0);
#endif
extern void suspend_or_resume_descendants(bool resume);
extern void suspend_or_resume_process(int pid, bool resume);

extern int process_priority_value(int);

#endif
