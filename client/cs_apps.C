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

// The "policy" part of task execution is here.
// The "mechanism" part is in app.C
//

#include "cpp.h"

#ifdef _WIN32
#include "boinc_win.h"
#endif

#ifndef _WIN32
#include "config.h"
#include <cassert>
#include <csignal>
#endif

#include "md5_file.h"
#include "util.h"
#include "error_numbers.h"
#include "file_names.h"
#include "filesys.h"
#include "shmem.h"
#include "log_flags.h"
#include "client_msgs.h"
#include "client_state.h"

using std::vector;

// Quit running applications, quit benchmarks,
// write the client_state.xml file
// (in principle we could also terminate net_xfers here,
// e.g. flush buffers, but why bother)
//
int CLIENT_STATE::quit_activities() {
    int retval;

    // calculate long-term debts (for state file)
    //
    adjust_debts();

    retval = active_tasks.exit_tasks();
    if (retval) {
        msg_printf(NULL, MSG_ERROR,
            "Couldn't exit tasks: %s", boincerror(retval)
        );
    }
    write_state_file();
    abort_cpu_benchmarks();
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

    // scan the output files, check if missing or too big
    // Don't bother doing this if result was aborted via GUI

    if (rp->exit_status != ERR_ABORTED_VIA_GUI) {
        for (i=0; i<rp->output_files.size(); i++) {
            fip = rp->output_files[i].file_info;
            if (fip->uploaded) continue;
            get_pathname(fip, path);
            retval = file_size(path, size);
            if (retval) {
                // an output file is unexpectedly absent.
                //
                fip->status = retval;
                had_error = true;
            } else if (size > fip->max_nbytes) {
                // Note: this is only checked when the application finishes.
                // The total disk space is checked while the application is running.
                //
                msg_printf(
                    rp->project, MSG_INFO,
                    "Output file %s for task %s exceeds size limit.",
                    fip->name, rp->name
                );
                msg_printf(
                    rp->project, MSG_INFO,
                    "File size: %f bytes.  Limit: %f bytes",
                    size, fip->max_nbytes
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
    }

    if (rp->exit_status != 0) {
        had_error = true;
    }

    if (had_error) {
        rp->state = RESULT_COMPUTE_ERROR;
    } else {
        rp->state = RESULT_FILES_UPLOADING;

        rp->project->update_duration_correction_factor(rp);
    }

    double wall_cpu_time = now - cpu_sched_last_time;
    at.result->project->wall_cpu_time_this_period += wall_cpu_time;
    total_wall_cpu_time_this_period += wall_cpu_time;
    total_cpu_time_this_period += at.current_cpu_time - at.cpu_time_at_last_sched;

    return 0;
}

// clean up after finished apps
//
bool CLIENT_STATE::handle_finished_apps() {
    unsigned int i;
    ACTIVE_TASK* atp;
    bool action = false;
    static double last_time = 0;
    if (gstate.now - last_time < 1.0) return false;
    last_time = gstate.now;

    SCOPE_MSG_LOG scope_messages(log_messages, CLIENT_MSG_LOG::DEBUG_TASK);

    for (i=0; i<active_tasks.active_tasks.size(); i++) {
        atp = active_tasks.active_tasks[i];
        switch (atp->task_state) {
        case PROCESS_EXITED:
        case PROCESS_WAS_SIGNALED:
        case PROCESS_EXIT_UNKNOWN:
        case PROCESS_COULDNT_START:
        case PROCESS_ABORTED:
            msg_printf(atp->wup->project, MSG_INFO,
                "Computation for task %s finished", atp->result->name
            );
            scope_messages.printf(
                "CLIENT_STATE::handle_finished_apps(): task finished; pid %d, status %d\n",
                atp->pid, atp->result->exit_status
            );
            app_finished(*atp);
            active_tasks.remove(atp);
            delete atp;
            set_client_state_dirty("handle_finished_apps");
            action = true;
        }
    }
    return action;
}

// Returns true iff all the input files for a result are present
// (both WU and app version)
//
bool CLIENT_STATE::input_files_available(RESULT* rp) {
    WORKUNIT* wup = rp->wup;
    FILE_INFO* fip;
    unsigned int i;
    APP_VERSION* avp;
    FILE_REF fr;
    PROJECT* project = rp->project;

    avp = wup->avp;
    for (i=0; i<avp->app_files.size(); i++) {
        fr = avp->app_files[i];
        fip = fr.file_info;
        if (fip->status != FILE_PRESENT) return false;

        // don't check file size for anonymous platform
        //
        if (!project->anonymous_platform) {
            if (fip->verify_file(false)) return false;
        }
    }

    for (i=0; i<wup->input_files.size(); i++) {
        fip = wup->input_files[i].file_info;
        if (fip->generated_locally) continue;
        if (fip->status != FILE_PRESENT) return false;
        if (fip->verify_file(false)) return false;
    }
    return true;
}


// if there's not an active task for the result, make one
//
int CLIENT_STATE::schedule_result(RESULT* rp) {
    ACTIVE_TASK *atp = lookup_active_task_by_result(rp);
    if (!atp) {
        atp = new ACTIVE_TASK;
        atp->init(rp);
        atp->slot = active_tasks.get_free_slot();
        get_slot_dir(atp->slot, atp->slot_dir);
        active_tasks.active_tasks.push_back(atp);
    }
    atp->next_scheduler_state = CPU_SCHED_SCHEDULED;
    return 0;
}


// find total resource shares of all projects
//
double CLIENT_STATE::total_resource_share() {
    double x = 0;
    for (unsigned int i=0; i<projects.size(); i++) {
        if (!projects[i]->non_cpu_intensive ) {
            x += projects[i]->resource_share;
        }
    }
    return x;
}

// same, but only runnable projects (can use CPU right now)
//
double CLIENT_STATE::runnable_resource_share() {
    double x = 0;
    for (unsigned int i=0; i<projects.size(); i++) {
        PROJECT* p = projects[i];
        if (p->runnable()) {
            x += p->resource_share;
        }
    }
    return x;
}

// same, but potentially runnable (could ask for work right now)
//
double CLIENT_STATE::potentially_runnable_resource_share() {
    double x = 0;
    for (unsigned int i=0; i<projects.size(); i++) {
        PROJECT* p = projects[i];
        if (p->potentially_runnable()) {
            x += p->resource_share;
        }
    }
    return x;
}


// This is called when the client is initialized.
// Try to restart any tasks that were running when we last shut down.
//
int CLIENT_STATE::restart_tasks() {
    return active_tasks.restart_tasks(ncpus);
}

void CLIENT_STATE::set_ncpus() {
    if (host_info.p_ncpus > 0) {
        ncpus = host_info.p_ncpus;
    } else {
        ncpus = 1;
    }
    if (ncpus > global_prefs.max_cpus) ncpus = global_prefs.max_cpus;
}

inline double force_fraction(double f) {
    if (f < 0) return 0;
    if (f > 1) return 1;
    return f;
}

double CLIENT_STATE::get_fraction_done(RESULT* result) {
    ACTIVE_TASK* atp = active_tasks.lookup_result(result);
    return atp ? force_fraction(atp->fraction_done) : 0.0;
}

// Decide which app version to use for a WU.
// Return -1 if can't find one
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
        msg_printf(0, MSG_ERROR,
            "No version found for application %s", app_name
        );
    }
    return best;
}

// handle file-transfer applications
//
void CLIENT_STATE::handle_file_xfer_apps() {
    unsigned int i;
    for (i=0; i<results.size(); i++) {
        RESULT* rp = results[i];
        if (rp->wup->avp->app_files.size() == 0 && rp->state == RESULT_FILES_DOWNLOADED) {
            rp->state = RESULT_FILES_UPLOADING;
            rp->reset_files();
        }
    }
}

void CLIENT_STATE::request_schedule_cpus(const char* where) {
    must_schedule_cpus = true;
    msg_printf(0, MSG_INFO, "Rescheduling CPU: %s", where);
}

const char *BOINC_RCSID_7bf63ad771 = "$Id$";
