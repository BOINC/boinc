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
// The "policy" part of task execution is here.
// The "mechanism" part is in app.C
//

#include "cpp.h"

#ifdef _WIN32
#include "stdafx.h"
#endif

#ifndef _WIN32
#if HAVE_SIGNAL_H
#include <signal.h>
#endif
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
        fip = fr.file_info;
        if (fip->status != FILE_PRESENT) return false;
        if (fip->approval_pending) return false;
    }

    for (i=0; i<wup->input_files.size(); i++) {
        fip = wup->input_files[i].file_info;
        if (fip->status != FILE_PRESENT) return false;
    }
    return true;
}

RESULT* CLIENT_STATE::next_result_to_start() const {
    int earliest_deadline = INT_MAX;
    RESULT* rp_earliest_deadline = NULL;

    for (vector<RESULT*>::const_iterator i = results.begin();
         i != results.end(); ++i)
    {
        RESULT* rp = *i;
        if (rp->state == RESULT_FILES_DOWNLOADED && !rp->is_active) {
            if (rp->report_deadline < earliest_deadline) {
                earliest_deadline = rp->report_deadline;
                rp_earliest_deadline = rp;
            }
        }
    }
    return rp_earliest_deadline;
}

// start new app if possible
//
bool CLIENT_STATE::start_apps() {
    bool action = false;
    RESULT* rp;
    int open_slot;

    while ( (open_slot = active_tasks.get_free_slot(nslots)) >= 0 &&
            (rp = next_result_to_start()) != NULL)
    {
        int retval;
        // Start the application to compute a result if:
        // 1) the result isn't done yet;
        // 2) the application isn't currently computing the result;
        // 3) all the input files for the result are locally available
        //
        rp->is_active = true;
        ACTIVE_TASK* atp = new ACTIVE_TASK;
        atp->slot = open_slot;
        atp->init(rp);

        msg_printf(atp->wup->project, MSG_INFO,
            "Starting computation for result %s using %s version %.2f",
            atp->result->name,
            atp->app_version->app->name,
            atp->app_version->version_num/100.
        );

        retval = active_tasks.insert(atp);

        // couldn't start process
        //
        if (retval) {
            atp->state = PROCESS_COULDNT_START;
            atp->result->active_task_state = PROCESS_COULDNT_START;
            report_result_error(
                *(atp->result), retval,
                "Couldn't start the app for this result: error %d", retval
            );
            delete atp;
        }
        action = true;
        set_client_state_dirty("start_apps");
        app_started = time(0);
    }
    return action;
}

// This is called when the client is initialized.
// Try to restart any tasks that were running when we last shut down.
//
int CLIENT_STATE::restart_tasks() {
    return active_tasks.restart_tasks();
}

int CLIENT_STATE::set_nslots() {
    int retval;

    // Set nslots to actual # of CPUs (or less, depending on prefs)
    //
    if (host_info.p_ncpus > 0) {
        nslots = host_info.p_ncpus;
    } else {
        nslots = 1;
    }
    if (nslots > global_prefs.max_cpus) nslots = global_prefs.max_cpus;

    retval = make_slot_dirs();
    if (retval) return retval;

    return 0;
}

// estimate how long a WU will take on this host
//
double CLIENT_STATE::estimate_cpu_time(WORKUNIT& wu) {
    double x;

    x = wu.rsc_fpops_est/host_info.p_fpops;
    return x;
}

inline double force_fraction(double f) {
    if (f < 0) return 0;
    if (f > 1) return 1;
    return f;
}

double CLIENT_STATE::get_percent_done(RESULT* result) {
    ACTIVE_TASK* atp = active_tasks.lookup_result(result);
    return atp ? force_fraction(atp->fraction_done) : 0.0;
}

// Decide which app version to use for a WU.
//
int CLIENT_STATE::choose_version_num(char* app_name, SCHEDULER_REPLY& sr) {
    unsigned int i;
    int best = -1;
    APP_VERSION* avp;

    // First look in the scheduler reply
    //
    for (i=0; i<sr.app_versions.size(); i++) {
        avp = &sr.app_versions[i];
        if (!strcmp(app_name, avp->app_name)) {
            return avp->version_num;
        }
    }

    // If not there, use the latest one in our state
    //
    for (i=0; i<app_versions.size(); i++) {
        avp = app_versions[i];
        if (strcmp(avp->app_name, app_name)) continue;
        if (avp->version_num < best) continue;
        best = avp->version_num;
    }
    if (best < 0) {
        msg_printf(0, MSG_ERROR, "CLIENT_STATE::latest_version_num: no version\n");
    }
    return best;
}

