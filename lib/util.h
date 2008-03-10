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

#ifndef UTIL_H
#define UTIL_H

#include <cstdlib>
#include <ctime>
#include <cmath>
#include <cctype>
#include <cstring>
#include <algorithm>
#include <string>
#include <vector>

#ifdef HAVE_PTHREAD
#include <pthread.h>
#endif

extern double dtime();
extern double dday();
extern void boinc_sleep(double);
extern void push_unique(std::string, std::vector<std::string>&);

// NOTE: use #include <functional>   to get max,min

#define SECONDS_PER_DAY 86400

static inline double drand() {
    return (double)rand()/(double)RAND_MAX;
}

#ifdef _WIN32
#include <windows.h>

extern char* windows_error_string(char* pszBuf, int iSize);
extern char* windows_format_error_string(
    unsigned long dwError, char* pszBuf, int iSize
);
extern int boinc_thread_cpu_time(HANDLE thread_handle, double& cpu);
extern int boinc_process_cpu_time(double& cpu);
#else
// setpriority(2) arg to run in background
// (don't use 20 because
//
static const int PROCESS_IDLE_PRIORITY = 19;
extern double linux_cpu_time(int pid);
#endif

extern void update_average(double, double, double, double&, double&);

extern int boinc_calling_thread_cpu_time(double&);

// convert UNIX time to MySQL timestamp (yyyymmddhhmmss)
//
extern void mysql_timestamp(double, char*);

extern void boinc_crash();
extern int read_file_malloc(const char* path, char*&, int max_len=0, bool tail=false);
extern int read_file_string(const char* path, std::string&, int max_len=0, bool tail=false);

#ifdef _WIN32

extern int run_program(
    const char* path, const char* cdir, int argc, char *const argv[], double, HANDLE&
);

extern void kill_program(HANDLE);
extern int get_exit_status(HANDLE);
extern bool process_exists(HANDLE);

#else
extern int run_program(
    const char* path, const char* cdir, int argc, char *const argv[], double, int&
);
extern void kill_program(int);
extern int get_exit_status(int);
extern bool process_exists(int);
#endif

extern int wait_client_mutex(const char* dir, double timeout);
#endif
