// The contents of this file are subject to the BOINC Public License
// Version 1.0 (the "License"); you may not use this file except in
// compliance with the License. You may obtain a copy of the License at
// http://boinc.berkeley.edu/license_1.0.txt
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

#ifndef _BOINC_API_
#define _BOINC_API_

#include <string>

#include "app_ipc.h"

/////////// API BEGINS HERE

struct BOINC_OPTIONS {
    bool main_program;
        // this is the main program, so
        // - lock a lock file in the slot directory
        // - write finish file on successful boinc_finish()
    bool check_heartbeat;
        // action is determined by direct_process_action (see below)
    bool handle_trickle_ups;
        // this process is allowed to call boinc_send_trickle_up()
    bool handle_trickle_downs;
        // this process is allowed to call boinc_receive_trickle_down()
    bool handle_process_control;
        // action is determined by direct_process_action (see below)
    bool send_status_msgs;
        // send CPU time / fraction done msgs
    bool direct_process_action;
        // if heartbeat fail, or get process control msg, take
        // direction action (exit, suspend, resume).
        // Otherwise just set flag in BOINC status

    void defaults();
};

struct BOINC_STATUS {
    bool no_heartbeat;
    bool suspended;
    bool quit_request;
};

extern "C" {
    extern int boinc_init();
    extern int boinc_init_options(BOINC_OPTIONS&);
    extern int boinc_finish(int status);
    extern int boinc_get_status(BOINC_STATUS&);
    extern bool boinc_is_standalone();
    extern int boinc_resolve_filename(const char*, char*, int len);
    extern int boinc_resolve_filename_s(const char*, std::string&);
    extern int boinc_parse_init_data_file();
    extern int boinc_write_init_data_file();
    extern int boinc_get_init_data(APP_INIT_DATA&);

    extern int boinc_send_trickle_up(char* variety, char* text);
    extern bool boinc_receive_trickle_down(char* buf, int len);

    extern bool boinc_time_to_checkpoint();
    extern int boinc_checkpoint_completed();

    extern int boinc_fraction_done(double);

    extern int boinc_wu_cpu_time(double&);
    extern int boinc_calling_thread_cpu_time(double&, double&);

    extern int boinc_suspend_other_activities();
    extern int boinc_resume_other_activities();
} // extern "C"

extern APP_INIT_DATA aid;

/////////// API ENDS HERE

/////////// IMPLEMENTATION STUFF BEGINS HERE

extern APP_CLIENT_SHM *app_client_shm;
#ifdef _WIN32
extern HANDLE worker_thread_handle;
#endif
extern int set_worker_timer();

/////////// IMPLEMENTATION STUFF ENDS HERE

#endif
