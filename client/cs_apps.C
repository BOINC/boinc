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
#include "boinc_win.h"
#endif

#ifndef _WIN32
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
// (should we also terminate net_xfers here?)
//
int CLIENT_STATE::quit_activities() {
    int retval;

    retval = active_tasks.exit_tasks();
    if (retval) {
        msg_printf(NULL, MSG_ERROR, "CLIENT_STATE.quit_activities: exit_tasks failed\n");
    }
    retval = write_state_file();
    if (retval) {
        msg_printf(NULL, MSG_ERROR, "CLIENT_STATE.quit_activities: write_state_file failed\n");
    }
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
    double task_cpu_time;

    bool had_error = false;

    if (at.exit_status != 0 && at.exit_status != ERR_QUIT_REQUEST) {
        had_error = true;
    }

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
            // Note: this is only checked when the application finishes.
            // The total disk space is checked while the application is running.
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
    PROJECT* p = rp->project;
    update_average(
        dtime()-rp->final_cpu_time,    // KLUDGE - should be result start time
        rp->final_cpu_time,
        CPU_HALF_LIFE,
        p->exp_avg_cpu,
        p->exp_avg_mod_time
    );

    task_cpu_time = at.current_cpu_time - at.cpu_time_at_last_sched;
    at.result->project->work_done_this_period += task_cpu_time;
    cpu_sched_work_done_this_period += task_cpu_time;

    return 0;
}

// clean up after finished apps
//
bool CLIENT_STATE::handle_finished_apps() {
    unsigned int i;
    ACTIVE_TASK* atp;
    bool action = false;

    SCOPE_MSG_LOG scope_messages(log_messages, CLIENT_MSG_LOG::DEBUG_TASK);

    for (i=0; i<active_tasks.active_tasks.size(); i++) {
        atp = active_tasks.active_tasks[i];
        switch (atp->state) {
        case PROCESS_EXITED:
        case PROCESS_WAS_SIGNALED:
        case PROCESS_EXIT_UNKNOWN:
        case PROCESS_COULDNT_START:
        case PROCESS_ABORTED:
            msg_printf(atp->wup->project, MSG_INFO,
                "Computation for result %s finished", atp->wup->name
            );
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
    PROJECT* project = rp->project;

    avp = wup->avp;
    for (i=0; i<avp->app_files.size(); i++) {
        fr = avp->app_files[i];
        fip = fr.file_info;
        if (fip->status != FILE_PRESENT) return false;

        // don't check file size for anonymous platform
        //
        if (!project->anonymous_platform) {
            if (!fip->verify_existing_file()) return false;
        }
    }

    for (i=0; i<wup->input_files.size(); i++) {
        fip = wup->input_files[i].file_info;
        if (fip->status != FILE_PRESENT) return false;
        if (!fip->verify_existing_file()) return false;
    }
    return true;
}


// Return true iff there are fewer scheduled tasks than available CPUs
//
bool CLIENT_STATE::have_free_cpu() {
    int num_running_tasks = 0;
    for (unsigned int i=0; i<active_tasks.active_tasks.size(); ++i) {
        if (active_tasks.active_tasks[i]->scheduler_state == CPU_SCHED_SCHEDULED) {
            ++num_running_tasks;
        }
    }
    return num_running_tasks < ncpus;
}

// Choose the next runnable result for each project with this
// preference order:
// 1. results with active tasks that are running
// 2. results with active tasks that are preempted (but have a process)
// 3. results with active tasks that have no process
// 4. results with no active task
//
void CLIENT_STATE::assign_results_to_projects() {

    // Before assigning a result to an active task,
    // mark file xfer results as completed;
    // TODO: why do this in this function??
    //
    handle_file_xfer_apps();

    for (unsigned int i=0; i<active_tasks.active_tasks.size(); ++i) {
        ACTIVE_TASK *atp = active_tasks.active_tasks[i];
        if (atp->result->already_selected) continue;
        PROJECT *p = atp->wup->project;
        if (p->next_runnable_result == NULL) {
            p->next_runnable_result = atp->result;
            continue;
        }

        // see if this task is "better" than the one currently
        // selected for this project
        //
        ACTIVE_TASK *next_atp = lookup_active_task_by_result(
            p->next_runnable_result
        );
        //assert(next_atp != NULL);

        if ((next_atp->state == PROCESS_UNINITIALIZED && atp->process_exists())
            || (next_atp->scheduler_state == CPU_SCHED_PREEMPTED
            && atp->scheduler_state == CPU_SCHED_SCHEDULED)
        ) {
            p->next_runnable_result = atp->result;
        }
    }
    // Note: !results[i]->is_active is true for results with preempted
    // active tasks, but all of those were already been considered in the
    // previous loop.
    // So p->next_runnable_result will not be NULL if there were any.
    //
    for (unsigned int i=0; i<results.size(); ++i) {
        if (results[i]->already_selected) continue;
        PROJECT *p = results[i]->wup->project;
        if (p->next_runnable_result == NULL
            && !results[i]->is_active
            && results[i]->state == RESULT_FILES_DOWNLOADED
        ){
            p->next_runnable_result = results[i];
        }
    }

    // mark selected results, so CPU scheduler won't try to consider
    // a result more than once (i.e. for running on another CPU)
    //
    for (unsigned int i=0; i<projects.size(); ++i) {
        if (projects[i]->next_runnable_result != NULL) {
            projects[i]->next_runnable_result->already_selected = true;
        }
    }
}

// Schedule an active task for the project with the largest anticipated debt
// among those that have a runnable result.
// Return true iff a task was scheduled.
//
bool CLIENT_STATE::schedule_largest_debt_project(double expected_pay_off) {
    PROJECT *best_project = NULL;
    double best_debt = 0.; // initial value doesn't matter
    bool first = true;

    for (unsigned int i=0; i < projects.size(); ++i) {
        if (projects[i]->next_runnable_result == NULL) continue;
        if (!input_files_available(projects[i]->next_runnable_result)) {
            report_result_error(
                *(projects[i]->next_runnable_result), ERR_FILE_MISSING,
                "One or more missing files"
            );
            projects[i]->next_runnable_result = NULL;
            continue;
        }
        if (first || projects[i]->anticipated_debt > best_debt) {
            first = false;
            best_project = projects[i];
            best_debt = best_project->anticipated_debt;
        }
    }
    if (!best_project) return false;

    ACTIVE_TASK *atp = lookup_active_task_by_result(best_project->next_runnable_result);
    if (!atp) {
        atp = new ACTIVE_TASK;
        atp->init(best_project->next_runnable_result);
        atp->slot = active_tasks.get_free_slot();
        get_slot_dir(atp->slot, atp->slot_dir);
        atp->result->is_active = true;
        active_tasks.active_tasks.push_back(atp);
    }
    best_project->anticipated_debt -= expected_pay_off;
    best_project->next_runnable_result = false;
    atp->next_scheduler_state = CPU_SCHED_SCHEDULED;
    return true;
}

// Schedule active tasks to be run and preempted.
//
// This is called every second in the do_something() loop
// (with must_reschedule=false)
// and whenever all the input files for a result finish downloading
// (with must_reschedule=true)
//
bool CLIENT_STATE::schedule_cpus(bool must_reschedule) {
    double expected_pay_off;
    vector<ACTIVE_TASK*>::iterator iter;
    ACTIVE_TASK *atp;
    PROJECT *p;
    bool some_app_started = false;
    double adjusted_total_resource_share;
    int retval, elapsed_time;
    double max_debt = SECONDS_PER_DAY * ncpus;
    double vm_limit;
    unsigned int i;

    // Reschedule every cpu_sched_period seconds or as needed
    //
    elapsed_time = time(0) - cpu_sched_last_time;
    if ((elapsed_time<cpu_sched_period && !have_free_cpu() && !must_reschedule)
        || projects.size() < 1
        || results.size() < 1
    ) {
        return false;
    }

    // tell app doing screensaver (fullscreen) graphics to stop
    // TODO: this interrupts the graphics, even if it's
    // the only app running.  DO THIS A DIFFERENT WAY
    //
    ss_logic.reset();

    // do work accounting for active tasks, reset temporary fields
    //
    for (i=0; i < active_tasks.active_tasks.size(); ++i) {
        atp = active_tasks.active_tasks[i];
        if (atp->scheduler_state != CPU_SCHED_SCHEDULED) continue;
        double task_cpu_time = atp->current_cpu_time - atp->cpu_time_at_last_sched;
        atp->result->project->work_done_this_period += task_cpu_time;
        cpu_sched_work_done_this_period += task_cpu_time;
        atp->next_scheduler_state = CPU_SCHED_PREEMPTED;
    }

    // compute total resource share among projects with runnable results
    //
    assign_results_to_projects(); // do this to see which projects have work
    adjusted_total_resource_share = 0;
    for (i=0; i < projects.size(); ++i) {
        p = projects[i];
        if (p->next_runnable_result != NULL) {
            adjusted_total_resource_share += projects[i]->resource_share;
        }
    }

    // adjust project debts
    // reset debts for projects with no runnable results
    // reset temporary fields
    //
    for (i=0; i < projects.size(); ++i) {
        p = projects[i];
        if (p->next_runnable_result == NULL) {
            p->debt = 0;
            p->anticipated_debt = 0;
        } else {
            p->debt +=
                (p->resource_share/adjusted_total_resource_share)
                * cpu_sched_work_done_this_period
                - p->work_done_this_period;
            if (p->debt < -max_debt) {
                p->debt = -max_debt;
            } else if (p->debt > max_debt) {
                p->debt = max_debt;
            }
            p->anticipated_debt = p->debt;
        }
        p->next_runnable_result = NULL;
    }

    // schedule tasks for projects in order of decreasing anticipated debt
    //
    for (i=0; i<results.size(); ++i) {
        results[i]->already_selected = false;
    }
    expected_pay_off = cpu_sched_work_done_this_period / ncpus;
    for (int j=0; j<ncpus; ++j) {
        assign_results_to_projects();
        if (!schedule_largest_debt_project(expected_pay_off)) break;
    }

    // preempt, start, and resume tasks
    //
    vm_limit = global_prefs.vm_max_used_pct / 100.0 * host_info.m_swap;
    iter = active_tasks.active_tasks.begin();
    while (iter != active_tasks.active_tasks.end()) {
        atp = *iter;
        if (atp->scheduler_state == CPU_SCHED_SCHEDULED
            && atp->next_scheduler_state == CPU_SCHED_PREEMPTED
        ) {
            bool preempt_by_quit = !global_prefs.leave_apps_in_memory;
            preempt_by_quit |= active_tasks.vm_limit_exceeded(vm_limit);
            atp->preempt(preempt_by_quit);
        } else if (atp->scheduler_state != CPU_SCHED_SCHEDULED
            && atp->next_scheduler_state == CPU_SCHED_SCHEDULED
        ) {
            retval = atp->resume_or_start();
            if (retval) {
                atp->state = PROCESS_COULDNT_START;
                atp->result->active_task_state = PROCESS_COULDNT_START;
                report_result_error(
                    *(atp->result), retval,
                    "Couldn't start the app for this result: error %d", retval
                );
                iter = active_tasks.active_tasks.erase(iter);
                delete atp;
                continue;
            }
            atp->scheduler_state = CPU_SCHED_SCHEDULED;
            some_app_started = true;
        }
        iter++;
        atp->cpu_time_at_last_sched = atp->current_cpu_time;
    }

    // reset work accounting
    // doing this at the end of schedule_cpus() because
    // work_done_this_period's can change as apps finish
    //
    for (i=0; i < projects.size(); ++i) {
        p = projects[i];
        p->work_done_this_period = 0;
    }
    cpu_sched_work_done_this_period = 0;

    cpu_sched_last_time = time(0);
    if (some_app_started) {
        app_started = cpu_sched_last_time;
    }

    // debts and active_tasks can only change if some project had
    // a runnable result (and thus if adjusted_total_resource_share
    // is positive)
    //
    if (adjusted_total_resource_share > 0.0) {
        set_client_state_dirty("schedule_cpus");
        return true;
    } else {
        return false;
    }
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
        msg_printf(0, MSG_ERROR, "CLIENT_STATE::latest_version_num: no version\n");
    }
    return best;
}

// goes through results and checks if the associated apps has no app files
// then there is nothing to do, never start the app, close the result
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
