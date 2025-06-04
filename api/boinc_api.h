// This file is part of BOINC.
// https://boinc.berkeley.edu
// Copyright (C) 2025 University of California
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

// The basic API functions are compiled with C linkage (no name munging)
// so that they can be used from C programs

#ifndef BOINC_BOINC_API_H
#define BOINC_BOINC_API_H

#ifdef _WIN32
#include "boinc_win.h"
#endif
#include "common_defs.h"

// Basic (C linkage) functions

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
        // This is true for regular apps, false for wrappers
    int multi_thread;
        // set this if application creates threads in main process
    int multi_process;
        // set this if application creates subprocesses.
} BOINC_OPTIONS;

// info passed from client to app in heartbeat message
//
typedef struct BOINC_STATUS {
    int no_heartbeat;
    int suspended;
    int quit_request;
    int reread_init_data_file;
    int abort_request;
    double working_set_size;
    double max_working_set_size;
    int network_suspended;
    enum SPORADIC_CA_STATE ca_state;
} BOINC_STATUS;

extern volatile BOINC_STATUS boinc_status;

typedef void (*FUNC_PTR)(void);

struct APP_INIT_DATA;

extern int boinc_init(void);
extern int boinc_finish(int status);
extern int boinc_get_init_data_p(struct APP_INIT_DATA*);
extern int boinc_parse_init_data_file(void);
extern int boinc_resolve_filename(const char*, char*, int len);
extern int boinc_send_trickle_up(char* variety, char* text);
extern int boinc_set_min_checkpoint_period(int);
extern int boinc_checkpoint_completed(void);
extern int boinc_fraction_done(double);
extern int boinc_suspend_other_activities(void);
extern int boinc_resume_other_activities(void);
extern int boinc_report_app_status(
    double cpu_time, double checkpoint_cpu_time, double _fraction_done
);
extern int boinc_time_to_checkpoint(void);
extern void boinc_begin_critical_section(void);
extern void boinc_end_critical_section(void);
extern void boinc_need_network(void);
extern int boinc_network_poll(void);
extern void boinc_network_done(void);
extern void boinc_network_usage(double sent, double received);
extern int boinc_is_standalone(void);
extern void boinc_ops_per_cpu_sec(double fp, double integer);
extern void boinc_ops_cumulative(double fp, double integer);
extern void boinc_set_credit_claim(double credit);
extern int boinc_receive_trickle_down(char* buf, int len);
extern int boinc_init_options(BOINC_OPTIONS*);
extern int boinc_get_status(BOINC_STATUS*);
extern double boinc_get_fraction_done(void);
extern void boinc_register_timer_callback(FUNC_PTR);
extern double boinc_worker_thread_cpu_time(void);
extern int boinc_init_parallel(void);
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
#include "app_ipc.h"

extern int boinc_resolve_filename_s(const char*, std::string&);
extern int boinc_get_init_data(APP_INIT_DATA&);
extern int boinc_wu_cpu_time(double&);
extern double boinc_elapsed_time(void);
extern int boinc_upload_file(std::string& name);
extern int boinc_upload_status(std::string& name);
extern char* boinc_msg_prefix(char*, int);
extern int boinc_report_app_status_aux(
    double cpu_time, double checkpoint_cpu_time, double _fraction_done,
    int other_pid, double bytes_sent, double bytes_received,
    double wss
);
extern int boinc_temporary_exit(
    int delay, const char* reason=0, bool is_notice=false
);
extern int boinc_finish_message(
    int status, const char* message, bool is_notice
);
extern void boinc_sporadic_set_ac_state(SPORADIC_AC_STATE);
extern SPORADIC_CA_STATE boinc_sporadic_get_ca_state();
extern int boinc_sporadic_dir(const char*);

/////////// API ENDS HERE

/////////// IMPLEMENTATION STUFF BEGINS HERE

extern void options_defaults(BOINC_OPTIONS&);
extern APP_CLIENT_SHM *app_client_shm;
#ifdef _WIN32
extern HANDLE worker_thread_handle;
#endif
extern int boinc_init_options_general(BOINC_OPTIONS& opt);
extern int start_timer_thread(void);
extern bool boinc_disable_timer_thread;

inline void boinc_options_defaults(BOINC_OPTIONS& b) {
    b.main_program = 1;
    b.check_heartbeat = 1;
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
