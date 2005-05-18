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
#include "util.h"

SS_LOGIC::SS_LOGIC() {
    do_ss = false;
    blank_time = 0;
    ack_deadline = 0;
    ss_status = 0;
}

void SS_LOGIC::ask_app(ACTIVE_TASK* atp, GRAPHICS_MSG& m) {
    SCOPE_MSG_LOG scope_messages(log_messages, CLIENT_MSG_LOG::DEBUG_SCRSAVE);

    atp->request_graphics_mode(m);
    atp->is_ss_app = true;
    ack_deadline = dtime() + 5.0;
    scope_messages.printf("SS_LOGIC::ask_app(): starting %s\n", atp->result->name);
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
    saved_graphics_msg = m;
    if (!gstate.activities_suspended) {
        atp = gstate.get_next_graphics_capable_app();
        if (atp) {
            ask_app(atp, m);
        }
    }
}

// called in response to a set_screensaver_mode RPC without <enabled>
// or ACTIVE_TASK::check_graphics_mode_ack() when mode == MODE_QUIT
// Stop providing screensaver graphics
//
void SS_LOGIC::stop_ss() {
    SCOPE_MSG_LOG scope_messages(log_messages, CLIENT_MSG_LOG::DEBUG_SCRSAVE);

    if (!do_ss) return;
    reset();
    do_ss = false;
    ss_status = SS_STATUS_QUIT;
    scope_messages.printf("SS_LOGIC::stop_ss(): stopping screen saver\n");
    gstate.active_tasks.restore_apps();
}

// If an app is acting as screensaver, tell it to stop.
// Called from CLIENT_STATE::schedule_cpus()
//
void SS_LOGIC::reset() {
    GRAPHICS_MSG m;
    SCOPE_MSG_LOG scope_messages(log_messages, CLIENT_MSG_LOG::DEBUG_SCRSAVE);

    m.mode = MODE_HIDE_GRAPHICS;
    ACTIVE_TASK* atp = gstate.active_tasks.get_ss_app();
    if (atp) {
        scope_messages.printf("SS_LOGIC::reset(): resetting %s\n", atp->result->name);
        atp->request_graphics_mode(m);
        atp->is_ss_app = false;
    }
}

// called 10X per second
//
void SS_LOGIC::poll(double now) {
    ACTIVE_TASK* atp;
    GRAPHICS_MSG m;
    static double last_time=0;
    SCOPE_MSG_LOG scope_messages(log_messages, CLIENT_MSG_LOG::DEBUG_SCRSAVE);

    double dt = now - last_time;
    if (dt < 1) return;
    last_time = now;

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
    if (blank_time && (dtime() > blank_time)) {
        if (SS_STATUS_BLANKED != ss_status) {
            scope_messages.printf("SS_LOGIC::poll(): going to black\n");
            reset();
            ss_status = SS_STATUS_BLANKED;
        }
    } else {
        atp = gstate.active_tasks.get_ss_app();
        if (atp) {
            bool stop_app_ss = false;
            if (atp->graphics_mode_acked == MODE_FULLSCREEN) {
                if (atp->scheduler_state != CPU_SCHED_SCHEDULED) {
                    scope_messages.printf("SS_LOGIC::poll(): app %s not scheduled\n", atp->result->name);
                    stop_app_ss = true;
                }
            } else {
                if (dtime() > ack_deadline) {
                    scope_messages.printf("SS_LOGIC::poll(): app %s not respond\n", atp->result->name);
                    stop_app_ss = true;
                }
            }
            if (!stop_app_ss) return;

            // app failed to go into fullscreen, or is preempted.
            // tell it not to do SSG
            //
            m.mode = MODE_HIDE_GRAPHICS;
            atp->request_graphics_mode(m);
            atp->is_ss_app = false;
        }

        // here if no app currently doing SSG
        // try to find one.
        //
        atp = gstate.get_next_graphics_capable_app();
        if (atp) {
            scope_messages.printf("SS_LOGIC::poll(): picked %s, request restart\n", atp->result->name);
            ask_app(atp, saved_graphics_msg);
        } else {
            scope_messages.printf("SS_LOGIC::poll(): no app found\n");
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

const char *BOINC_RCSID_dd5060e766 = "$Id$";
