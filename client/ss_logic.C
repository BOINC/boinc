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

#ifdef _WIN32
#include "boinc_win.h"
#endif

#include "client_state.h"
#include "client_msgs.h"
#include "ss_logic.h"

extern void create_curtain();
extern void delete_curtain();

SS_LOGIC::SS_LOGIC() {
    do_ss = false;
    do_boinc_logo_ss = false;
    do_blank = false;
    blank_time = 0;
    ack_deadline = 0;
}

// this is called when the core client receives a message
// from the screensaver module.
//
void SS_LOGIC::start_ss(time_t new_blank_time) {
    ACTIVE_TASK* atp;

    if (do_ss) return;
    do_ss = true;
    do_blank = do_boinc_logo_ss = false;
    strcpy(ss_msg, "");
    blank_time = new_blank_time;
    gstate.active_tasks.save_app_modes();
    gstate.active_tasks.hide_apps();
    create_curtain();
    if (!gstate.activities_suspended) {
        atp = gstate.get_next_graphics_capable_app();
        if (atp) {
            atp->request_graphics_mode(MODE_FULLSCREEN);
            atp->is_ss_app = true;
            ack_deadline = time(0) + 5;
        } else {
            do_boinc_logo_ss = true;
        }
    }
}

void SS_LOGIC::stop_ss() {
    if (!do_ss) return;
    reset();
    do_ss = do_boinc_logo_ss = do_blank = false;
    delete_curtain();
    gstate.active_tasks.restore_apps();
}

// If an app is acting as screensaver, tell it to stop.
//
void SS_LOGIC::reset() {
    ACTIVE_TASK* atp = gstate.active_tasks.get_ss_app();
    if (atp) {
        atp->request_graphics_mode(MODE_HIDE_GRAPHICS);
        atp->is_ss_app = false;
    }
}

// called every second
//
void SS_LOGIC::poll() {
    ACTIVE_TASK* atp;

#if 0
    // if you want to debug screensaver functionality...
    static int foo=0;
    foo++;
    if (foo == 8) start_ss(time(0)+1000);
#endif

    if (!do_ss) return;

    if (gstate.activities_suspended) {
        reset();
        do_boinc_logo_ss = true;
        strcpy(ss_msg, "BOINC activities suspended");
        return;
    }


    // check if it's time to go to black screen
    //
    if (blank_time && (time(0) > blank_time)) {
        if (!do_blank) {
            reset();
            do_blank = true;
        }
        do_boinc_logo_ss = false;
        strcpy(ss_msg, "");
    } else {
        atp = gstate.active_tasks.get_ss_app();
        if (atp) {
            if (atp->graphics_mode_acked == MODE_FULLSCREEN) {
                do_boinc_logo_ss = false;
                strcpy(ss_msg, "");
            } else {
                if (time(0)>ack_deadline) {
                    do_boinc_logo_ss = true;
                    strcpy(ss_msg, "App can't display graphics");
                    atp->request_graphics_mode(MODE_HIDE_GRAPHICS);
                    atp->is_ss_app = false;
                }
            }
        } else {
            atp = gstate.get_next_graphics_capable_app();
            if (atp) {
                atp->request_graphics_mode(MODE_FULLSCREEN);
                atp->is_ss_app = true;
                ack_deadline = time(0) + 5;
            } else {
                do_boinc_logo_ss = true;
                if (gstate.active_tasks.active_tasks.size()==0) {
                    strcpy(ss_msg, "No applications are running");
                } else {
                    strcpy(ss_msg, "No graphics-capable applications are running");
                }
            }
        }
    }
}
