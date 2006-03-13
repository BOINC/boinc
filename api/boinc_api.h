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

#ifndef _BOINC_API_
#define _BOINC_API_

#ifdef __APPLE__
#include <Carbon/Carbon.h>
#endif

// ANSI C API BEGINS HERE

#ifdef __cplusplus
extern "C" {
#endif
struct BOINC_OPTIONS {
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
	int terminate_on_exit;
	    // some applications have unpredictable issues with calling
	    // exit when various resources haven't already been cleaned
	    // up, such as threads and memory.  So terminate the app
	    // and use a checkpoint to recover from the issue during
	    // the next execution.
};

struct BOINC_STATUS {
    int no_heartbeat;
    int suspended;
    int quit_request;
    int reread_init_data_file;
    int abort_request;
};

extern int boinc_init(void);
extern int boinc_finish(int status);
extern void boinc_exit (int status);
extern int boinc_resolve_filename(const char*, char*, int len);
extern int boinc_parse_init_data_file(void);
extern int boinc_write_init_data_file(void);
extern int boinc_send_trickle_up(char* variety, char* text);
extern int boinc_checkpoint_completed(void);
extern int boinc_fraction_done(double);
extern int boinc_suspend_other_activities(void);
extern int boinc_resume_other_activities(void);
extern int boinc_report_app_status(double, double, double);
extern int boinc_time_to_checkpoint();
extern void boinc_begin_critical_section();
extern void boinc_end_critical_section();
extern void boinc_not_using_cpu();
extern void boinc_using_cpu();
extern void boinc_need_network();
extern int boinc_network_poll();
extern void boinc_network_done();
extern int boinc_is_standalone(void);
extern void boinc_ops_per_cpu_sec(double fp, double integer);
extern void boinc_ops_cumulative(double fp, double integer);
extern int boinc_receive_trickle_down(char* buf, int len);
extern int boinc_init_options(BOINC_OPTIONS*);
extern int boinc_get_status(BOINC_STATUS*);

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
extern int boinc_upload_file(std::string& name);
extern int boinc_upload_status(std::string& name);

/////////// API ENDS HERE

/////////// IMPLEMENTATION STUFF BEGINS HERE

extern void options_defaults(BOINC_OPTIONS&);
extern APP_CLIENT_SHM *app_client_shm;
#ifdef _WIN32
extern HANDLE worker_thread_handle;
#else
extern void block_sigalrm();
#endif
extern int suspend_activities(void);
extern int resume_activities(void);
extern int restore_activities(void);
extern int boinc_init_options_general(BOINC_OPTIONS& opt);
extern int set_worker_timer(void);
extern void (*stop_graphics_thread_ptr)();

inline void boinc_options_defaults(BOINC_OPTIONS& b) {
    b.main_program = true;
    b.check_heartbeat = true;
    b.handle_trickle_ups = true;
    b.handle_trickle_downs = true;
    b.handle_process_control = true;
    b.send_status_msgs = true;
    b.direct_process_action = true;
	b.terminate_on_exit = false;
}


/////////// IMPLEMENTATION STUFF ENDS HERE

#endif // C++ part

#endif // double-inclusion protection
