static volatile const char *BOINCrcsid="$Id$";
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

SS_LOGIC::SS_LOGIC() {
    do_ss = false;
    blank_time = 0;
    ack_deadline = 0;
    ss_status = 0;
}

// this is called when the core client receives a message
// from the screensaver module.
//
void SS_LOGIC::start_ss(GRAPHICS_MSG& m, double new_blank_time) {
    ACTIVE_TASK* atp;

    if (do_ss) return;
    do_ss = true;
    ss_status = SS_STATUS_ENABLED;

    blank_time = new_blank_time;
    gstate.active_tasks.save_app_modes();
    gstate.active_tasks.hide_apps();

    m.mode = MODE_FULLSCREEN;
    if (!gstate.activities_suspended) {
        atp = gstate.get_next_graphics_capable_app();
        if (atp) {
            atp->request_graphics_mode(m);
            atp->is_ss_app = true;
            ack_deadline = time(0) + 5;
        }
    }
}

void SS_LOGIC::stop_ss() {
    if (!do_ss) return;
    reset();
    do_ss = false;
    ss_status = SS_STATUS_QUIT;
    gstate.active_tasks.restore_apps();
}

// If an app is acting as screensaver, tell it to stop.
//
void SS_LOGIC::reset() {
    GRAPHICS_MSG m;

    m.mode = MODE_HIDE_GRAPHICS;
    ACTIVE_TASK* atp = gstate.active_tasks.get_ss_app();
    if (atp) {
        atp->request_graphics_mode(m);
        atp->is_ss_app = false;
    }
}

// called every second
//
void SS_LOGIC::poll() {
    ACTIVE_TASK* atp;
    GRAPHICS_MSG m;


#if 0
    // if you want to debug screensaver functionality...
    static int foo=0;
    foo++;
    if (foo == 8) start_ss(time(0)+1000);
#endif

    if (!do_ss) return;

    if (gstate.activities_suspended) {
        reset();
        ss_status = SS_STATUS_BOINCSUSPENDED;
        return;
    }


    // check if it's time to go to black screen
    //
    if (blank_time && (time(0) > blank_time)) {
        if (SS_STATUS_BLANKED != ss_status) {
            reset();
            ss_status = SS_STATUS_BLANKED;
        }
    } else {
        atp = gstate.active_tasks.get_ss_app();
        if (atp) {
            if (atp->graphics_mode_acked != MODE_FULLSCREEN) {
                if (time(0)>ack_deadline) {
                    m.mode = MODE_HIDE_GRAPHICS;
                    atp->request_graphics_mode(m);
                    atp->is_ss_app = false;
                    ss_status = SS_STATUS_NOTGRAPHICSCAPABLE;
                }
            }
        } else {
            atp = gstate.get_next_graphics_capable_app();
            if (atp) {
                do_ss = false;
                ss_status = SS_STATUS_RESTARTREQUEST;
            } else {
                if (gstate.active_tasks.active_tasks.size()==0) {
                    ss_status = SS_STATUS_NOAPPSEXECUTING;
                } else {
                    ss_status = SS_STATUS_NOGRAPHICSAPPSEXECUTING;
                }
            }
        }
    }
}
