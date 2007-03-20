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
// 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

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

// Ideally, we would access this using wxGetApp().m_use_sandbox in the Manager
// and gstate.m_use_sandbox in the Client, but it is used by some source files
// (filesys.C, check_security.C) that are linked with both Manager and Client 
// so the most practical solution is to use a global.
extern int      g_use_sandbox;


extern double dtime();
extern double dday();
extern void boinc_sleep(double);
extern int read_file_string(const char* pathname, std::string& result);
extern void escape_project_url(char *in, char* out);
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
extern int lookup_group(char*, gid_t& gid);
extern int check_security(int use_sandbox, int isManager);

// setpriority(2) arg to run in background
// (don't use 20 because
static const int PROCESS_IDLE_PRIORITY = 19;
#endif

extern void update_average(double, double, double, double&, double&);

extern int boinc_calling_thread_cpu_time(double&);

// convert UNIX time to MySQL timestamp (yyyymmddhhmmss)
//
extern void mysql_timestamp(double, char*);

#ifdef HAVE_PTHREAD
extern pthread_mutex_t getrusage_mutex;
#endif

extern int run_program(char* path, char* cdir, int argc, char** argv);
extern int wait_client_mutex(const char* dir, double timeout);

#endif
