// The contents of this file are subject to the Mozilla Public License
// Version 1.0 (the "License"); you may not use this file except in
// compliance with the License. You may obtain a copy of the License at
// http://www.mozilla.org/MPL/ 
// 
// Software distributed under the License is distributed on an "AS IS"
// basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the
// License for the specific language governing rights and limitations
// under the License. 
// 
// The Original Code is the Berkeley Open Infrastructure for Network Computing. 
// 
// The Initial Developer of the Original Code is the SETI@home project.
// Portions created by the SETI@home project are Copyright (C) 2002
// University of California at Berkeley. All Rights Reserved. 
// 
// Contributor(s):
//

#include <stdio.h>
#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif
#ifdef HAVE_SYS_RESOURCE_H
#include <sys/resource.h>
#endif

#ifndef _BOINC_API
#define _BOINC_API

#define DEFAULT_FRACTION_DONE_UPDATE_PERIOD     1
#define DEFAULT_CHECKPOINT_PERIOD               300

// MFILE supports a primitive form of checkpointing.
// Write all your output (and restart file) to MFILEs.
// The output is buffered in memory.
// Then close or flush all the MFILEs;
// all the buffers will be flushed to disk, almost atomically.

class MFILE {
    char* buf;
    int len;
    FILE* f;
public:
    int open(char* path, char* mode);
    int _putchar(char);
    int puts(char*);
    int printf(char* format, ...);
    size_t write(const void *, size_t size, size_t nitems);
    int close();
    int flush();
};

struct APP_INIT_DATA {
    char app_preferences[4096];
    char user_name[256];
    char team_name[256];
    char comm_obj_name[256];  // name to identify shared memory segments, signals, etc
    double wu_cpu_time;		  // cpu time from previous sessions
    double user_total_credit;
    double user_expavg_credit;
    double host_total_credit;
    double host_expavg_credit;
    double checkpoint_period;     // recommended checkpoint period
    int shm_key;
    double fraction_done_update_period;
};

#define APP_ACCESS_OK		0
#define CLIENT_ACCESS_OK	1

struct APP_CLIENT_SHM {
    int access;
    char message_buf[4096];
};

extern int boinc_init();
extern int boinc_get_init_data(APP_INIT_DATA&);
extern int boinc_finish(int);
extern int boinc_resolve_filename(char*, char*, int len);
extern bool boinc_time_to_checkpoint();
extern int boinc_checkpoint_completed();
extern int boinc_fraction_done(double);
extern int boinc_child_start();
extern int boinc_child_done(double);
extern double boinc_cpu_time();     // CPU time for this process
extern int boinc_install_signal_handlers();

/////////// API ENDS HERE - IMPLEMENTATION STUFF FOLLOWS

int update_app_progress(double, double, double);
int write_init_data_file(FILE* f, APP_INIT_DATA&);
int parse_init_data_file(FILE* f, APP_INIT_DATA&);
int write_fd_init_file(FILE*, char*, int, int);
int parse_fd_init_file(FILE*);

#define INIT_DATA_FILE    "init_data.xml"
#define GRAPHICS_DATA_FILE    "graphics.xml"
#define FD_INIT_FILE    "fd_init.xml"
#define STDERR_FILE             "stderr.txt"

int set_timer(double period);
void setup_shared_mem();
void cleanup_shared_mem();

#endif
