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
// High-level logic involving task execution
//

#include "windows_cpp.h"

#ifdef _WIN32
#include <afxwin.h>
#endif
#if HAVE_SIGNAL_H
#include <signal.h>
#endif

#include "md5_file.h"
#include "error_numbers.h"
#include "file_names.h"
#include "filesys.h"
#include "shmem.h"
#include "log_flags.h"
#include "client_state.h"

// Make a directory for each available slot
//
int CLIENT_STATE::make_slot_dirs() {
    int i;
    int retval;
    for (i=0; i<nslots; i++) {
        retval = make_slot_dir(i);
        if (retval) return retval;
    }
    return 0;
}

// Perform a graceful shutdown of the client, including quitting
// all applications, checking their final status, and writing
// the client_state.xml file (should we also terminate net_xfers here?)
//
int CLIENT_STATE::cleanup_and_exit() {
    int retval;

    retval = active_tasks.exit_tasks();
    if (retval) {
        msg_printf(NULL, MSG_ERROR, "CLIENT_STATE.cleanup_and_exit: exit_tasks failed\n");
        // don't return here - we'll exit anyway
    }
    retval = write_state_file();
    if (retval) {
        msg_printf(NULL, MSG_ERROR, "CLIENT_STATE.cleanup_and_exit: write_state_file failed\n");
        // don't return here - we'll exit anyway
    }

    // Stop the CPU benchmark if it's running
    if (cpu_benchmarks_id) {
#ifdef _WIN32
        TerminateThread(cpu_benchmarks_handle, 0);
        CloseHandle(cpu_benchmarks_handle);
#else
        kill(cpu_benchmarks_id, SIGKILL);
#endif
    }
    msg_printf(NULL, MSG_INFO, "Exiting BOINC client");
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
    double size;

    bool had_error = false;

    if (at.exit_status != 0 && at.exit_status != ERR_QUIT_REQUEST) had_error = true;

    for (i=0; i<rp->output_files.size(); i++) {
        fip = rp->output_files[i].file_info;
        get_pathname(fip, path);
        retval = file_size(path, size);
        if (retval) {
            // an output file is unexpectedly absent.
            //
            fip->status = retval;
            had_error = true;
        } else if (size > fip->max_nbytes) {
            // Note: this is only checked when the application finishes. there
            // is also a check_max_disk_exceeded that is checked while the
            // application is running.
            //
            msg_printf(
                rp->project, MSG_INFO,
                "Output file %s for result %s exceeds size limit.",
                fip->name, rp->name
            );

            fip->delete_file();
            fip->status = ERR_FILE_TOO_BIG;
            had_error = true;
        } else {
            if (!fip->upload_when_present && !fip->sticky) {
                fip->delete_file();     // sets status to NOT_PRESENT
            } else {
                retval = md5_file(path, fip->md5_cksum, fip->nbytes);
                if (retval) {
                    fip->status = retval;
                    had_error = true;
                } else {
                    fip->status = FILE_PRESENT;
                }
            }
        }
    }

    rp->is_active = false;
    if (had_error) {
        // dead-end state indicating we had an error at end of computation;
        // do not move to RESULT_FILES_UPLOADING
        rp->state = RESULT_COMPUTE_DONE;
    } else {
        // can now upload files.
        rp->state = RESULT_FILES_UPLOADING;
    }
    update_avg_cpu(rp->project);
    rp->project->exp_avg_cpu += rp->final_cpu_time;
    return 0;
}

// clean up after finished apps
//
bool CLIENT_STATE::handle_finished_apps() {
    unsigned int i;
    ACTIVE_TASK* atp;
    bool action = false;

    ScopeMessages scope_messages(log_messages, ClientMessages::DEBUG_TASK);

    for (i=0; i<active_tasks.active_tasks.size(); i++) {
        atp = active_tasks.active_tasks[i];
        switch (atp->state) {
        case PROCESS_RUNNING:
        case PROCESS_ABORT_PENDING:
            break;
        default:
            msg_printf(atp->wup->project, MSG_INFO, "Computation for result %s finished", atp->wup->name);
            scope_messages.printf(
                "CLIENT_STATE::handle_finished_apps(): task finished; pid %d, status %d\n",
                atp->pid, atp->exit_status
            );
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
    FILE_REF fr;
    avp = wup->avp;
    for (i=0; i<avp->app_files.size(); i++) {
        fr = avp->app_files[i];
        // if it's an optional file and we couldn't download it by deadline,
        // don't let that stop us from running app
        //
        if (fr.optional && time(0) > fr.optional_deadline) {
            continue;
        }
        fip = fr.file_info;
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
    int open_slot, retval;

    for (i=0; i<results.size(); i++) {

        // If all the app slots are already used, we can't start a new app
        //
        open_slot = active_tasks.get_free_slot(nslots);
        if (open_slot < 0) {
            return action;
        }
        rp = results[i];

        // Start the application to compute a result if:
        // 1) the result isn't done yet;
        // 2) the application isn't currently computing the result;
        // 3) all the input files for the result are locally available
        //
        if (rp->state == RESULT_FILES_DOWNLOADED && !rp->is_active ) {
            if (log_flags.task) {
                msg_printf(rp->project, MSG_INFO, "Starting computation for result %s", rp->name);
            }
            rp->is_active = true;
            atp = new ACTIVE_TASK;
            atp->slot = open_slot;
            atp->init(rp);
            retval = active_tasks.insert(atp);

            // couldn't start process
            //
            if (retval) {
                atp->state = PROCESS_COULDNT_START;
                atp->result->active_task_state = PROCESS_COULDNT_START;
                report_result_error(
                    *(atp->result), retval,
                    "Couldn't start the app for this result.\n"
                );
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
