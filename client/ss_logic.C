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

// called in response to a set_screensaver_mode RPC with <enabled>.
// Start providing screensaver graphics
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

// called in response to a set_screensaver_mode RPC without <enabled>
// Stop providing screensaver graphics
//
void SS_LOGIC::stop_ss() {
    if (!do_ss) return;
    reset();
    do_ss = false;
    ss_status = SS_STATUS_QUIT;
    gstate.active_tasks.restore_apps();
}

// If an app is acting as screensaver, tell it to stop.
// Called from CLIENT_STATE::schedule_cpus()
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
                }
            }
        } else {
            atp = gstate.get_next_graphics_capable_app();
            if (atp) {
                do_ss = false;
                ss_status = SS_STATUS_RESTARTREQUEST;
            } else {
                if (gstate.active_tasks.active_tasks.size()==0) {
                    if (gstate.projects.size()>0) {
                        ss_status = SS_STATUS_NOAPPSEXECUTING;
                    } else {
                        ss_status = SS_STATUS_NOPROJECTSDETECTED;
                    }
                } else {
                    ss_status = SS_STATUS_NOGRAPHICSAPPSEXECUTING;
                }
            }
        }
    }
}

const char *BOINC_RCSID_dd5060e766 = "$Id$";
