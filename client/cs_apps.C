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

#include "windows_cpp.h"

#include "md5_file.h"
#include "log_flags.h"
#include "file_names.h"

#include "client_state.h"

// Make a directory for each of the available slots specified
// in the client state
//
int CLIENT_STATE::make_slot_dirs() {
    unsigned int i;
    for (i=0; i<nslots; i++) {
        make_slot_dir(i);
    }
    return 0;
}

// Perform a graceful shutdown of the client, including quitting
// all applications, checking their final status, and writing
// the client_state.xml file (should we also terminate net_xfers here?)
//
int CLIENT_STATE::exit() {
    int retval;
    retval = exit_tasks();
    if (retval) {
    fprintf(stderr, "error: CLIENT_STATE.exit: exit_tasks failed\n");
        return retval;
    }
    retval = write_state_file();
    if (retval) { 
    fprintf(stderr, "error: CLIENT_STATE.exit: write_state_file failed\n");
        return retval;
    }
    return 0;
}

int CLIENT_STATE::exit_tasks() {
    active_tasks.exit_tasks();
    return 0;
}

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
    int retval;

    for (i=0; i<rp->output_files.size(); i++) {
        fip = rp->output_files[i].file_info;
        fip->status = FILE_PRESENT;
        if (!fip->upload_when_present && !fip->sticky) {
            fip->delete_file();
        } else {
            get_pathname(fip, path);
            retval = md5_file(path, fip->md5_cksum, fip->nbytes);
            if (retval) {
                // an output file is unexpectedly absent.
                // 
                fip->status = retval;
            }
        }
    }

    at.result->is_active = false;
    at.result->state = RESULT_COMPUTE_DONE;
    update_avg_cpu(at.result->project);
    at.result->project->exp_avg_cpu += at.result->final_cpu_time;
    return 0;
}

// poll status of existing apps and and clean up after them
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
            set_client_state_dirty("handle_running_apps");
            action = true;
        }
    }
    return action;
}

// Returns true if all the input files for a result are available
// locally, false otherwise
//
bool CLIENT_STATE::input_files_available(RESULT* rp) {
    WORKUNIT* wup = rp->wup;
    FILE_INFO* fip;
    unsigned int i;
    APP_VERSION* avp;
    avp = wup->avp;
    for (i=0; i<avp->app_files.size(); i++) {
        fip = avp->app_files[i].file_info;
        if (fip->status != FILE_PRESENT) return false;
    }

    for (i=0; i<wup->input_files.size(); i++) {
        fip = wup->input_files[i].file_info;
        if (fip->status != FILE_PRESENT) return false;
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
    int open_slot;
    int retval;

    for (i=0; i<results.size(); i++) {

        // If all the app slots are already used, we can't start a new app
        //
        open_slot = active_tasks.get_free_slot(nslots);
        if (open_slot < 0) {
            if (log_flags.task_debug) {
                printf("start_apps(): all slots full\n");
            }
            return false;
        }
        rp = results[i];

        // Start the application to compute a result if:
        // 1) the result isn't done yet;
        // 2) the application isn't currently computing the result;
        // 3) all the input files for the result are locally available
        //
        if (rp->state == RESULT_FILES_DOWNLOADED && !rp->is_active ) {
            if (log_flags.task_debug) {
                printf("starting application for result %s\n", rp->name);
            }
            rp->is_active = true;
            atp = new ACTIVE_TASK;
            atp->slot = open_slot;
            atp->init(rp);
            retval = active_tasks.insert(atp);
	    //couldn't start process
	    if(retval) {
		atp->state = PROCESS_COULDNT_START;
		atp->result->active_task_state = PROCESS_COULDNT_START;
	
		report_project_error(*(atp->result),retval,"Couldn't start the app for this result.\n",CLIENT_COMPUTING);
	    }
            action = true;
            set_client_state_dirty("start_apps");
            app_started = time(0);
        }
    }
    return action;
}

// This is called when the client is initialized.
// Try to restart any tasks that were running when we last shut down.
//
int CLIENT_STATE::restart_tasks() {
    return active_tasks.restart_tasks();
}
