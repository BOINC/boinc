// The contents of this file are subject to the Mozilla Public License
// Version 1.0 (the "License"); you may not use this file except in
// compliance with the License. You may obtain a copy of the License at
// http://www.mozilla.org/MPL/ 
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

#include "md5_file.h"
#include "log_flags.h"
#include "file_names.h"

#include "client_state.h"

// Handle a task that has finished.
// Mark its output files as present, and delete scratch files.
// Don't delete input files because they might be shared with other WUs.
// Update state of result record.
//
int CLIENT_STATE::app_finished(ACTIVE_TASK& at) {
    RESULT* rp = at.result;
    FILE_INFO* fip;
    unsigned int i;
    char path[256];

    for (i=0; i<rp->output_files.size(); i++) {
        fip = rp->output_files[i].file_info;
        fip->file_present = true;
        if (!fip->upload_when_present && !fip->sticky) {
            fip->delete_file();
        } else {
            get_pathname(fip, path);
            md5_file(path, fip->md5_cksum, fip->nbytes);
        }
    }

    at.result->is_active = false;
    at.result->is_compute_done = true;
    update_avg_cpu(at.result->project);
    at.result->project->exp_avg_cpu += at.result->cpu_time;
    return 0;
}

// poll and clean up after existing apps
//
bool CLIENT_STATE::handle_running_apps() {
    unsigned int i;
    ACTIVE_TASK* atp;
    bool action = false;

    for (i=0; i<active_tasks.active_tasks.size(); i++) {
        atp = active_tasks.active_tasks[i];
        if (atp->state != PROCESS_RUNNING) {
            if (log_flags.task_debug) {
                printf(
                    "task finished; pid %d, status %d\n",
                    atp->pid, atp->exit_status
                );
            }
            app_finished(*atp);
            active_tasks.remove(atp);
            delete atp;
            client_state_dirty = true;
            action = true;
        }
    }
    return action;
}

bool CLIENT_STATE::input_files_available(RESULT* rp) {
    WORKUNIT* wup = rp->wup;
    FILE_INFO* fip;
    unsigned int i;

    if (!wup->avp->file_info->file_present) return false;
    for (i=0; i<wup->input_files.size(); i++) {
        fip = wup->input_files[i].file_info;
        if (!fip->file_present) return false;
    }
    return true;
}

// start new app if possible
//
bool CLIENT_STATE::start_apps() {
    unsigned int i;
    RESULT* rp;
    ACTIVE_TASK* atp;
    bool action = false;

    for (i=0; i<results.size(); i++) {
        if (active_tasks.active_tasks.size() == nslots) return 0;
        rp = results[i];
        if (!rp->is_compute_done && !rp->is_active && input_files_available(rp)) {
            if (log_flags.task_debug) {
                printf("starting application for result %s\n", rp->name);
            }
            rp->is_active = true;
            atp = new ACTIVE_TASK;
            atp->init(rp);
            active_tasks.insert(atp);
            action = true;
            client_state_dirty = true;
        }
    }
    return action;
}

// This is called on initialization.
// Try to restart any tasks that were running when we last shut down.
//
int CLIENT_STATE::restart_tasks() {
    return active_tasks.restart_tasks();
}
