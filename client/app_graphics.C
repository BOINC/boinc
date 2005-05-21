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

// graphics-related interaction with running apps

#include "cpp.h"

#ifdef _WIN32
#include "boinc_win.h"
#endif

#include "exception.h"
#include "diagnostics.h"
#include "client_state.h"
#include "client_msgs.h"

#include "app.h"
#include "util.h"

//#define SS_DEBUG 1

void ACTIVE_TASK::request_graphics_mode(GRAPHICS_MSG& m) {
    char buf[MSG_CHANNEL_SIZE], buf2[256];
    SCOPE_MSG_LOG scope_messages(log_messages, CLIENT_MSG_LOG::DEBUG_SCRSAVE);

    if (!app_client_shm.shm) return;

    graphics_msg = m;       // save graphics_station, desktop, display

    strcpy(buf, xml_graphics_modes[m.mode]);
    if (strlen(m.window_station)) {
        sprintf(buf2, "<window_station>%s</window_station>", m.window_station);
        strcat(buf, buf2);
    }
    if (strlen(m.desktop)) {
        sprintf(buf2, "<desktop>%s</desktop>", m.desktop);
        strcat(buf, buf2);
    }
    if (strlen(m.display)) {
        sprintf(buf2, "<display>%s</display>", m.display);
        strcat(buf, buf2);
    }

    scope_messages.printf(
        "ACTIVE_TASK::request_graphics_mode(): requesting graphics mode %s for %s\n",
        xml_graphics_modes[m.mode], result->name
    );
    graphics_request_queue.msg_queue_send(
        buf,
        app_client_shm.shm->graphics_request
    );
}


void ACTIVE_TASK::check_graphics_mode_ack() {
    GRAPHICS_MSG gm;
    char buf[MSG_CHANNEL_SIZE];
    SCOPE_MSG_LOG scope_messages(log_messages, CLIENT_MSG_LOG::DEBUG_SCRSAVE);

    if (!app_client_shm.shm) return;
    if (app_client_shm.shm->graphics_reply.get_msg(buf)) {
        app_client_shm.decode_graphics_msg(buf, gm);
        scope_messages.printf(
            "ACTIVE_TASK::check_graphics_mode_ack(): got graphics ack %s for %s, previous mode %s\n",
            buf, result->name, xml_graphics_modes[graphics_mode_acked]
        );
        if (is_ss_app && (gm.mode != MODE_FULLSCREEN) && (gm.mode != MODE_REREAD_PREFS)) {
            gstate.ss_logic.stop_ss();
            scope_messages.printf(
                "ACTIVE_TASK::check_graphics_mode_ack(): shutting down the screensaver\n"
            );
        }
        if (gm.mode != MODE_REREAD_PREFS) {
            graphics_mode_acked = gm.mode;
        }
    }
}

// return the active task that's currently acting as screensaver
//
ACTIVE_TASK* ACTIVE_TASK_SET::get_ss_app() {
    unsigned int i;

    for (i=0; i<active_tasks.size(); i++) {
        ACTIVE_TASK* atp = active_tasks[i];
        if (atp->is_ss_app) {
            return atp;
        }
    }
    return NULL;
}

// remember graphics state of apps.
// Called when entering screensaver mode,
// so that we can return to same state later
//
void ACTIVE_TASK_SET::save_app_modes() {
    unsigned int i;
    ACTIVE_TASK* atp;
    SCOPE_MSG_LOG scope_messages(log_messages, CLIENT_MSG_LOG::DEBUG_SCRSAVE);

    for (i=0; i<active_tasks.size(); i++) {
        atp = active_tasks[i];

        // nothing should be in fullscreen mode.
        //
        if (atp->graphics_mode_acked == MODE_FULLSCREEN) {
            atp->graphics_mode_acked = MODE_HIDE_GRAPHICS;
        }
        atp->graphics_mode_before_ss = atp->graphics_mode_acked;
        scope_messages.printf(
            "ACTIVE_TASK_SET::save_app_modes(): saved mode %d\n", atp->graphics_mode_acked
        );
    }
}

void ACTIVE_TASK_SET::hide_apps() {
    unsigned int i;
    ACTIVE_TASK* atp;

    for (i=0; i<active_tasks.size(); i++) {
        atp = active_tasks[i];
        atp->graphics_msg.mode = MODE_HIDE_GRAPHICS;
        atp->request_graphics_mode(atp->graphics_msg);
    }
}

// return apps to the mode they were in before screensaving started
//
void ACTIVE_TASK_SET::restore_apps() {
    unsigned int i;
    ACTIVE_TASK* atp;

    for (i=0; i<active_tasks.size(); i++) {
        atp = active_tasks[i];
        if (atp->graphics_mode_before_ss == MODE_WINDOW) {
            atp->graphics_msg.mode = MODE_WINDOW;
            atp->request_graphics_mode(atp->graphics_msg);
        }
    }
}

void ACTIVE_TASK_SET::graphics_poll() {
    unsigned int i;
    ACTIVE_TASK* atp;

    for (i=0; i<active_tasks.size(); i++) {
        atp = active_tasks[i];
        if (!atp->process_exists()) continue;
        atp->graphics_request_queue.msg_queue_poll(
            atp->app_client_shm.shm->graphics_request
        );
        atp->check_graphics_mode_ack();
    }
}

bool ACTIVE_TASK::supports_graphics() {
    return (graphics_mode_acked != MODE_UNSUPPORTED);
}

// Return the next graphics-capable running app.
// Always try to choose a new project.
// Preferences goes to apps with pre-ss mode WINDOW,
// then apps with pre-ss mode HIDE
//
ACTIVE_TASK* CLIENT_STATE::get_next_graphics_capable_app() {
    static int project_index=0;
    unsigned int i, j;
    ACTIVE_TASK *atp, *best_atp;
    PROJECT *p;
    SCOPE_MSG_LOG scope_messages(log_messages, CLIENT_MSG_LOG::DEBUG_SCRSAVE);

    // check to see if the applications have changed the graphics ack
    // since they were first started, this can happen if their is a
    // failure to find the target desktop
    //
    for (i=0; i<active_tasks.active_tasks.size(); i++) {
        atp = active_tasks.active_tasks[i];
        atp->check_graphics_mode_ack();
    }

    // loop through all projects starting with the one at project_index
    //
    for (i=0; i<projects.size(); ++i) {
        project_index %= projects.size();
        p = projects[project_index++];

        best_atp = NULL;
        for (j=0; j<active_tasks.active_tasks.size(); j++) {
            atp = active_tasks.active_tasks[j];
            if (atp->scheduler_state != CPU_SCHED_SCHEDULED) continue;
            // don't choose an application that doesn't support graphics
            if (atp->graphics_mode_acked == MODE_UNSUPPORTED) continue;
            // don't choose an application that hasn't ack'ed the
            // hide graphics request
            if (atp->graphics_mode_acked != MODE_HIDE_GRAPHICS) continue;
            if (atp->result->project != p) continue;

            if (!best_atp && atp->graphics_mode_before_ss == MODE_WINDOW) {
                best_atp = atp;
            }
            if (!best_atp && atp->graphics_mode_before_ss == MODE_HIDE_GRAPHICS) {
                best_atp = atp;
            }
            if (!best_atp) {
                best_atp = atp;
            }
            if (best_atp) {
                scope_messages.printf(
                    "CLIENT_STATE::get_next_graphics_capable_app(): get_next_app: %s\n", best_atp->result->name
                );
                return atp;
            }
        }
    }
    scope_messages.printf(
        "CLIENT_STATE::get_next_graphics_capable_app(): get_next_app: none\n"
    );
    return NULL;
}

const char *BOINC_RCSID_71e9cd9f4d = "$Id$";
