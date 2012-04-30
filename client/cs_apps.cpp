// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2008 University of California
//
// BOINC is free software; you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation,
// either version 3 of the License, or (at your option) any later version.
//
// BOINC is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with BOINC.  If not, see <http://www.gnu.org/licenses/>.

// The "policy" part of task execution is here.
// The "mechanism" part is in app.C
//

#include "cpp.h"

#ifdef _WIN32
#include "boinc_win.h"
#else
#include "config.h"
#include <cassert>
#include <csignal>
#endif

#include "error_numbers.h"
#include "filesys.h"
#include "md5_file.h"
#include "shmem.h"
#include "util.h"

#include "client_msgs.h"
#include "client_state.h"
#include "file_names.h"
#include "log_flags.h"
#include "project.h"
#include "result.h"

using std::vector;

// clean up after finished apps
//
bool CLIENT_STATE::handle_finished_apps() {
    ACTIVE_TASK* atp;
    bool action = false;
    static double last_time = 0;
    if (now - last_time < HANDLE_FINISHED_APPS_PERIOD) return false;
    last_time = now;

    vector<ACTIVE_TASK*>::iterator iter;

    iter = active_tasks.active_tasks.begin();
    while (iter != active_tasks.active_tasks.end()) {
        atp = *iter;
        switch (atp->task_state()) {
        case PROCESS_EXITED:
        case PROCESS_WAS_SIGNALED:
        case PROCESS_EXIT_UNKNOWN:
        case PROCESS_COULDNT_START:
        case PROCESS_ABORTED:
            if (log_flags.task) {
                msg_printf(atp->wup->project, MSG_INFO,
                    "Computation for task %s finished", atp->result->name
                );
            }
            app_finished(*atp);
            if (!action) {
                adjust_rec();     // update debts before erasing ACTIVE_TASK
            }
            iter = active_tasks.active_tasks.erase(iter);
            delete atp;
            set_client_state_dirty("handle_finished_apps");

            // the following is critical; otherwise the result is
            // still in the "scheduled" list and enforce_schedule()
            // will try to run it again.
            //
            request_schedule_cpus("handle_finished_apps");
            action = true;
            break;
        default:
            iter++;
        }
    }
    return action;
}

// Handle a task that has finished.
// Mark its output files as present, and delete scratch files.
// Don't delete input files because they might be shared with other WUs.
// Update state of result record.
//
int CLIENT_STATE::app_finished(ACTIVE_TASK& at) {
    RESULT* rp = at.result;
    bool had_error = false;

#ifndef SIM
    FILE_INFO* fip;
    unsigned int i;
    char path[256];
    int retval;
    double size;

    // scan the output files, check if missing or too big.
    // Don't bother doing this if result was aborted via GUI or by project
    //
    switch (rp->exit_status) {
    case EXIT_ABORTED_VIA_GUI:
    case EXIT_ABORTED_BY_PROJECT:
        break;
    default:
        for (i=0; i<rp->output_files.size(); i++) {
            FILE_REF& fref = rp->output_files[i];
            fip = fref.file_info;
            if (fip->uploaded) continue;
            get_pathname(fip, path, sizeof(path));
            retval = file_size(path, size);
            if (retval) {
                if (fref.optional) {
                    fip->upload_urls.clear();
                    continue;
                }

                // an output file is unexpectedly absent.
                //
                fip->status = retval;
                had_error = true;
                msg_printf(
                    rp->project, MSG_INFO,
                    "Output file %s for task %s absent",
                    fip->name, rp->name
                );
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
                if (!fip->uploadable() && !fip->sticky) {
                    fip->delete_file();     // sets status to NOT_PRESENT
                } else {
                    retval = 0;
                    if (fip->gzip_when_done) {
                        retval = fip->gzip();
                    }
                    if (!retval) {
                        retval = md5_file(path, fip->md5_cksum, fip->nbytes);
                    }
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
#endif

    if (rp->exit_status != 0) {
        had_error = true;
    }

    if (had_error) {
        switch (rp->exit_status) {
        case EXIT_ABORTED_VIA_GUI:
        case EXIT_ABORTED_BY_PROJECT:
            rp->set_state(RESULT_ABORTED, "CS::app_finished");
            break;
        default:
            rp->set_state(RESULT_COMPUTE_ERROR, "CS::app_finished");
        }
    } else {
#ifdef SIM
        rp->set_state(RESULT_FILES_UPLOADED, "CS::app_finished");
        rp->set_ready_to_report();
        rp->completed_time = now;
#else
        rp->set_state(RESULT_FILES_UPLOADING, "CS::app_finished");
        rp->append_log_record();
#endif
        rp->project->last_upload_start = gstate.now;
        rp->project->update_duration_correction_factor(&at);
    }

    double elapsed_time = now - debt_interval_start;
    work_fetch.accumulate_inst_sec(&at, elapsed_time);

    return 0;
}

// Returns zero iff all the input files for a result are present
// (both WU and app version)
// Called from CLIENT_STATE::update_results (with verify=false)
// to transition result from DOWNLOADING to DOWNLOADED.
// Called from ACTIVE_TASK::start() (with verify=true)
// when project has verify_files_on_app_start set.
//
// If fipp is nonzero, return a pointer to offending FILE_INFO on error
//
int CLIENT_STATE::input_files_available(
    RESULT* rp, bool verify_contents, FILE_INFO** fipp
) {
    WORKUNIT* wup = rp->wup;
    FILE_INFO* fip;
    unsigned int i;
    APP_VERSION* avp;
    FILE_REF fr;
    PROJECT* project = rp->project;
    int retval;

    avp = rp->avp;
    for (i=0; i<avp->app_files.size(); i++) {
        fr = avp->app_files[i];
        fip = fr.file_info;
        if (fip->status != FILE_PRESENT) {
            if (fipp) *fipp = fip;
            return ERR_FILE_MISSING;
        }

        // don't verify app files if using anonymous platform
        //
        if (verify_contents && !project->anonymous_platform) {
            retval = fip->verify_file(true, true, false);
            if (retval) {
                if (fipp) *fipp = fip;
                return retval;
            }
        }
    }

    for (i=0; i<wup->input_files.size(); i++) {
        fip = wup->input_files[i].file_info;
        if (fip->status != FILE_PRESENT) {
            if (wup->input_files[i].optional) continue;
            if (fipp) *fipp = fip;
            return ERR_FILE_MISSING;
        }
        if (verify_contents) {
            retval = fip->verify_file(true, true, false);
            if (retval) {
                if (fipp) *fipp = fip;
                return retval;
            }
        }
    }
    return 0;
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

// Find latest version of app for given platform
// or -1 if can't find one
//
int CLIENT_STATE::latest_version(APP* app, char* platform) {
    unsigned int i;
    int best = -1;

    for (i=0; i<app_versions.size(); i++) {
        APP_VERSION* avp = app_versions[i];
        if (avp->app != app) continue;
        if (strcmp(platform, avp->platform)) continue;
        if (avp->version_num < best) continue;
        best = avp->version_num;
    }
    return best;
}

// Find the ACTIVE_TASK in the current set with the matching PID
//
ACTIVE_TASK* ACTIVE_TASK_SET::lookup_pid(int pid) {
    unsigned int i;
    ACTIVE_TASK* atp;

    for (i=0; i<active_tasks.size(); i++) {
        atp = active_tasks[i];
        if (atp->pid == pid) return atp;
    }
    return NULL;
}

// Find the ACTIVE_TASK in the current set with the matching result
//
ACTIVE_TASK* ACTIVE_TASK_SET::lookup_result(RESULT* result) {
    unsigned int i;
    ACTIVE_TASK* atp;

    for (i=0; i<active_tasks.size(); i++) {
        atp = active_tasks[i];
        if (atp->result == result) {
            return atp;
        }
    }
    return NULL;
}
