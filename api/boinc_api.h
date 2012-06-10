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

#ifndef _BOINC_API_
#define _BOINC_API_

#ifdef _WIN32
#include "boinc_win.h"
#endif
#include "app_ipc.h"

// ANSI C API BEGINS HERE
// Do not put implementation stuff here

#ifdef __cplusplus
extern "C" {
#endif

// parameters passed to the BOINC runtime system
//
typedef struct BOINC_OPTIONS {
    // the following are booleans, implemented as ints for portability
    int normal_thread_priority;
        // run worker thread at normal thread priority on Win.
        // (default is idle priority)
    int main_program;
        // this is the main program, so
        // - lock a lock file in the slot directory
        // - write finish file on successful boinc_finish()
    int check_heartbeat;
        // check for timeout of heartbeats from the client;
        // action is determined by direct_process_action (see below)
    int handle_trickle_ups;
        // periodically check for trickle-up msgs from the app
        // must set this to use boinc_send_trickle_up()
    int handle_trickle_downs;
        // this process is allowed to call boinc_receive_trickle_down()
    int handle_process_control;
        // whether runtime system should read suspend/resume/quit/abort
        // msgs from client.
        // action is determined by direct_process_action (see below)
    int send_status_msgs;
        // whether runtime system should send CPU time / fraction done msgs
    int direct_process_action;
        // if heartbeat fail, or get process control msg, take
        // direction action (exit, suspend, resume).
        // Otherwise just set flag in BOINC status
    int multi_thread;
        // set this if application creates threads in main process
    int multi_process;
        // set this if application creates subprocesses.
} BOINC_OPTIONS;

typedef struct BOINC_STATUS {
    int no_heartbeat;
    int suspended;
    int quit_request;
    int reread_init_data_file;
    int abort_request;
    double working_set_size;
    double max_working_set_size;
    int network_suspended;
} BOINC_STATUS;

extern volatile BOINC_STATUS boinc_status;

typedef void (*FUNC_PTR)();

struct APP_INIT_DATA;

extern int boinc_init(void);
extern int boinc_finish(int status);
extern int boinc_get_init_data_p(struct APP_INIT_DATA*);
extern int boinc_parse_init_data_file(void);
extern int boinc_send_trickle_up(char* variety, char* text);
extern int boinc_checkpoint_completed(void);
extern int boinc_fraction_done(double);
extern int boinc_suspend_other_activities(void);
extern int boinc_resume_other_activities(void);
extern int boinc_report_app_status(
    double cpu_time, double checkpoint_cpu_time, double _fraction_done
);
extern int boinc_time_to_checkpoint();
extern void boinc_begin_critical_section();
extern int boinc_try_critical_section();
extern void boinc_end_critical_section();
extern void boinc_need_network();
extern int boinc_network_poll();
extern void boinc_network_done();
extern void boinc_network_usage(double sent, double received);
extern int boinc_is_standalone(void);
extern void boinc_ops_per_cpu_sec(double fp, double integer);
extern void boinc_ops_cumulative(double fp, double integer);
extern void boinc_set_credit_claim(double credit);
extern int boinc_receive_trickle_down(char* buf, int len);
extern int boinc_init_options(BOINC_OPTIONS*);
extern int boinc_get_status(BOINC_STATUS*);
extern double boinc_get_fraction_done();
extern void boinc_register_timer_callback(FUNC_PTR);
extern double boinc_worker_thread_cpu_time();
extern int boinc_init_parallel();
extern void boinc_web_graphics_url(char*);
extern void boinc_remote_desktop_addr(char*);

#ifdef __APPLE__
extern int setMacPList(void);
extern int setMacIcon(char *filename, char *iconData, long iconSize);
#endif

#ifdef __cplusplus
} // extern "C" {
#endif

// C++ API follows 
#ifdef __cplusplus
#include <string>

extern int boinc_get_init_data(APP_INIT_DATA&);
extern int boinc_wu_cpu_time(double&);
extern double boinc_elapsed_time();
extern int boinc_upload_file(std::string& name);
extern int boinc_upload_status(std::string& name);
extern char* boinc_msg_prefix(char*, int);
extern int boinc_report_app_status_aux(
    double cpu_time, double checkpoint_cpu_time, double _fraction_done,
    int other_pid, double bytes_sent, double bytes_received
);
extern int boinc_temporary_exit(int delay, const char* reason=NULL);

/////////// API ENDS HERE

/////////// IMPLEMENTATION STUFF BEGINS HERE

extern void options_defaults(BOINC_OPTIONS&);
extern APP_CLIENT_SHM *app_client_shm;
#ifdef _WIN32
extern HANDLE worker_thread_handle;
#endif
extern int boinc_init_options_general(BOINC_OPTIONS& opt);
extern int start_timer_thread();
extern bool g_sleep;

inline void boinc_options_defaults(BOINC_OPTIONS& b) {
    b.main_program = 1;
    b.check_heartbeat = 1;
    b.handle_trickle_ups = 1;
    b.handle_trickle_downs = 1;
    b.handle_process_control = 1;
    b.send_status_msgs = 1;
    b.direct_process_action = 1;
    b.normal_thread_priority = 0;
    b.multi_thread = 0;
    b.multi_process = 0;
}


/////////// IMPLEMENTATION STUFF ENDS HERE

#endif // C++ part

#endif // double-inclusion protection
