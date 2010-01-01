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

#ifdef __APPLE__
#include <Carbon/Carbon.h>
#endif

#ifdef _WIN32
#include "boinc_win.h"
#endif

// ANSI C API BEGINS HERE
// Do not put implementation stuff here

#ifdef __cplusplus
extern "C" {
#endif
typedef struct BOINC_OPTIONS {
    // the following are booleans, implemented as ints for portability
    int backwards_compatible_graphics;
        // V6 apps should set this so that "Show Graphics" will work
        // with pre-V6 clients
    int normal_thread_priority;
        // run app at normal thread priority on Win.
        // (default is idle priority)
    int main_program;
        // this is the main program, so
        // - lock a lock file in the slot directory
        // - write finish file on successful boinc_finish()
    int check_heartbeat;
        // action is determined by direct_process_action (see below)
    int handle_trickle_ups;
        // this process is allowed to call boinc_send_trickle_up()
    int handle_trickle_downs;
        // this process is allowed to call boinc_receive_trickle_down()
    int handle_process_control;
        // action is determined by direct_process_action (see below)
    int send_status_msgs;
        // send CPU time / fraction done msgs
    int direct_process_action;
        // if heartbeat fail, or get process control msg, take
        // direction action (exit, suspend, resume).
        // Otherwise just set flag in BOINC status
} BOINC_OPTIONS;

typedef struct BOINC_STATUS {
    int no_heartbeat;
    int suspended;
    int quit_request;
    int reread_init_data_file;
    int abort_request;
    double working_set_size;
    double max_working_set_size;
} BOINC_STATUS;

typedef void (*FUNC_PTR)();

struct APP_INIT_DATA;

extern int boinc_init(void);
extern int boinc_finish(int status);
extern int boinc_temporary_exit(int delay);
extern int boinc_resolve_filename(const char*, char*, int len);
extern int boinc_get_init_data_p(struct APP_INIT_DATA*);
extern int boinc_parse_init_data_file(void);
extern int boinc_send_trickle_up(char* variety, char* text);
extern int boinc_checkpoint_completed(void);
extern int boinc_fraction_done(double);
extern int boinc_suspend_other_activities(void);
extern int boinc_resume_other_activities(void);
extern int boinc_report_app_status(double, double, double);
extern int boinc_time_to_checkpoint();
extern void boinc_begin_critical_section();
extern int boinc_try_critical_section();
extern void boinc_end_critical_section();
extern void boinc_need_network();
extern int boinc_network_poll();
extern void boinc_network_done();
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
extern void boinc_exit(int);    // deprecated
extern int boinc_init_parallel();

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

#include "app_ipc.h"
extern int boinc_resolve_filename_s(const char*, std::string&);
extern int boinc_get_init_data(APP_INIT_DATA&);
extern int boinc_wu_cpu_time(double&);
extern double boinc_elapsed_time();
extern int boinc_upload_file(std::string& name);
extern int boinc_upload_status(std::string& name);
extern int boinc_write_init_data_file(APP_INIT_DATA&);
extern char* boinc_msg_prefix();
extern int suspend_activities();   // deprecated
extern int resume_activities();    // deprecated
extern int restore_activities();    //deprecated

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
    b.backwards_compatible_graphics = 1;
    b.normal_thread_priority = 0;
}


/////////// IMPLEMENTATION STUFF ENDS HERE

#endif // C++ part

#endif // double-inclusion protection
