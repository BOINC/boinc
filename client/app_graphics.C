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

// graphics-related interaction with running apps

#include "cpp.h"

#ifdef _WIN32
#include "boinc_win.h"
#endif

#include "client_state.h"
#include "client_msgs.h"

#include "app.h"

void ACTIVE_TASK::request_graphics_mode(int mode) {
    if (!app_client_shm.shm) return;
    graphics_request_queue.msg_queue_send(
        xml_graphics_modes[mode],
        app_client_shm.shm->graphics_request
    );
}


void ACTIVE_TASK::check_graphics_mode_ack() {
    int mode;
    char buf[MSG_CHANNEL_SIZE];

    if (!app_client_shm.shm) return;
    if (app_client_shm.shm->graphics_reply.get_msg(buf)) {
        mode = app_client_shm.decode_graphics_msg(buf);
        //msg_printf(NULL, MSG_INFO, "got graphics ack %s for %s", buf, result->name);
        if (mode != MODE_REREAD_PREFS) {
            graphics_mode_acked = mode;
        }
    }
}

// return an app (if any) with given requested mode
//
ACTIVE_TASK* ACTIVE_TASK_SET::get_ss_app() {
    unsigned int i;
    ACTIVE_TASK* atp;

    for (i=0; i<active_tasks.size(); i++) {
        atp = active_tasks[i];
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

    for (i=0; i<active_tasks.size(); i++) {
        atp = active_tasks[i];

        // nothing should be in fullscreen mode.
        //
        if (atp->graphics_mode_acked == MODE_FULLSCREEN) {
            atp->graphics_mode_acked = MODE_HIDE_GRAPHICS;
        }
        atp->graphics_mode_before_ss = atp->graphics_mode_acked;
        //msg_printf(NULL, MSG_INFO, "saved mode %d", atp->graphics_mode_acked);
    }
}

void ACTIVE_TASK_SET::hide_apps() {
    unsigned int i;
    ACTIVE_TASK* atp;

    for (i=0; i<active_tasks.size(); i++) {
        atp = active_tasks[i];
        atp->request_graphics_mode(MODE_HIDE_GRAPHICS);
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
            atp->request_graphics_mode(MODE_WINDOW);
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

    // loop through all projects starting with the one at project_index
    //
    for (i=0; i<projects.size(); ++i) {
        project_index %= projects.size();
        p = projects[project_index++];

        best_atp = NULL;
        for (j=0; j<active_tasks.active_tasks.size(); j++) {
            atp = active_tasks.active_tasks[j];
            if (atp->scheduler_state != CPU_SCHED_SCHEDULED) continue;
            if (atp->result->project != p) continue;
            if (!best_atp && atp->graphics_mode_before_ss == MODE_WINDOW) {
                best_atp = atp;
            }
            if (!best_atp && atp->graphics_mode_before_ss == MODE_HIDE_GRAPHICS) {
                best_atp = atp;
            }
            if (!best_atp && atp->graphics_mode_acked != MODE_UNSUPPORTED) {
                best_atp = atp;
            }
            if (best_atp) {
                 //msg_printf(NULL, MSG_INFO, "chose SS: %s", best_atp->result->name);
                 return atp;
           }
        }
    }
    //msg_printf(NULL, MSG_INFO, "no SS to choose");
    return NULL;
}
