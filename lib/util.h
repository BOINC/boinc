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

#ifndef UTIL_H
#define UTIL_H

#include <stdlib.h>
#include <string>
#include <vector>

extern double dtime();
extern double dday();
extern void boinc_sleep(double);
extern void push_unique(std::string, std::vector<std::string>&);

// NOTE: use #include <functional>   to get max,min

#define SECONDS_PER_DAY 86400
#define KILO (1024.0)
#define MEGA (1048576.0)
#define GIGA (1024.*1048576.0)

static inline double drand() {
    return (double)rand()/(double)RAND_MAX;
}
extern double rand_normal();

#ifdef _WIN32
#include <windows.h>

extern char* windows_error_string(char* pszBuf, int iSize);
extern char* windows_format_error_string(
    unsigned long dwError, char* pszBuf, int iSize
);
extern int boinc_thread_cpu_time(HANDLE thread_handle, double& cpu);
extern int boinc_process_cpu_time(HANDLE process_handle, double& cpu);
#else
// setpriority(2) arg to run in background
// (don't use 20 because
//
static const int PROCESS_IDLE_PRIORITY = 19;
static const int PROCESS_MEDIUM_PRIORITY = 10;
extern double linux_cpu_time(int pid);
#endif

extern void update_average(double, double, double, double, double&, double&);

extern int boinc_calling_thread_cpu_time(double&);

inline bool in_vector(int n, std::vector<int>& v) {
    for (unsigned int i=0; i<v.size(); i++) {
        if (n == v[i]) return true;
    }
    return false;
}

// fake a crash
//
extern void boinc_crash();

// read files into memory.
// Use only for non-binary files; returns null-terminated string.
//
extern int read_file_malloc(
    const char* path, char*& result, size_t max_len=0, bool tail=false
);
extern int read_file_string(
    const char* path, std::string& result, size_t max_len=0, bool tail=false
);

#ifdef _WIN32

extern int run_program(
    const char* dir, const char* file, int argc, char *const argv[], double, HANDLE&
);

extern void kill_program(HANDLE);
extern int get_exit_status(HANDLE);
extern bool process_exists(HANDLE);

#else
extern int run_program(
    const char* dir, const char* file, int argc, char *const argv[], double, int&
);
extern void kill_program(int);
extern int get_exit_status(int);
extern bool process_exists(int);
#endif

extern int wait_client_mutex(const char* dir, double timeout);

#ifdef GCL_SIMULATOR
extern double simtime;
#define time(x) ((int)simtime)
#endif

#endif
