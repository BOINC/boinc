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

    // scan the output files, check if missing or too big
    // Don't bother doing this if result was aborted via GUI

    if (rp->exit_status != ERR_ABORTED_VIA_GUI) {
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

        // if success, update average CPU time per result for project
        //
        PROJECT* p = rp->project;
        update_average(
            dtime()-rp->final_cpu_time,
                // KLUDGE - should be result start time
            rp->final_cpu_time,
            CPU_HALF_LIFE,
            p->exp_avg_cpu,
            p->exp_avg_mod_time
        );
    }

    task_cpu_time = at.current_cpu_time - at.cpu_time_at_last_sched;
    at.result->project->work_done_this_period += task_cpu_time;
    cpu_sched_work_done_this_period += task_cpu_time;

    return 0;
}

// clean up after finished apps
//
bool CLIENT_STATE::handle_finished_apps(double now) {
    unsigned int i;
    ACTIVE_TASK* atp;
    bool action = false;
    static double last_time = 0;
    if (now - last_time < 1.0) return false;
    last_time = now;

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
                "Computation for result %s finished", atp->wup->name
            );
            scope_messages.printf(
                "CLIENT_STATE::handle_finished_apps(): task finished; pid %d, status %d\n",
                atp->pid, atp->result->exit_status
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

// Returns true if all the input files for a result are present
// (both WU and app version)
// false otherwise
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


// Choose a "best" runnable result for each project
//
// Values are returned in project->next_runnable_result
// (skip projects for which this is already non-NULL)
//
// Don't choose results with already_selected == true;
// mark chosen results as already_selected.
//
// The preference order:
// 1. results with active tasks that are running
// 2. results with active tasks that are preempted (but have a process)
// 3. results with active tasks that have no process
// 4. results with no active task
//
void CLIENT_STATE::assign_results_to_projects() {
    unsigned int i;
    RESULT* rp;
    PROJECT* project;

    // scan results with an ACTIVE_TASK
    //
    for (i=0; i<active_tasks.active_tasks.size(); ++i) {
        ACTIVE_TASK *atp = active_tasks.active_tasks[i];
        if (atp->result->suspended_via_gui) continue;
        if (atp->result->already_selected) continue;
        project = atp->wup->project;
        if (project->suspended_via_gui) continue;
        if (!project->next_runnable_result) {
            project->next_runnable_result = atp->result;
            continue;
        }

        // see if this task is "better" than the one currently
        // selected for this project
        //
        ACTIVE_TASK *next_atp = lookup_active_task_by_result(
            project->next_runnable_result
        );
        assert(next_atp != NULL);

        if ((next_atp->task_state == PROCESS_UNINITIALIZED && atp->process_exists())
            || (next_atp->scheduler_state == CPU_SCHED_PREEMPTED
            && atp->scheduler_state == CPU_SCHED_SCHEDULED)
        ) {
            project->next_runnable_result = atp->result;
        }
    }

    // Now consider results that don't have an active task
    //
    for (i=0; i<results.size(); i++) {
        rp = results[i];

        project = rp->wup->project;
        if (project->suspended_via_gui) continue;
        if (project->next_runnable_result) continue;

        if (rp->already_selected) continue;
        if (rp->suspended_via_gui) continue;
        if (rp->state != RESULT_FILES_DOWNLOADED) continue;
        if (lookup_active_task_by_result(rp)) continue;
        project->next_runnable_result = rp;
    }

    // mark selected results, so CPU scheduler won't try to consider
    // a result more than once
    //
    for (i=0; i<projects.size(); i++) {
        project = projects[i];
        if (project->next_runnable_result) {
            project->next_runnable_result->already_selected = true;
        }
    }
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

// Schedule an active task for the project with the largest anticipated debt
// among those that have a runnable result.
// Return true iff a task was scheduled.
//
bool CLIENT_STATE::schedule_largest_debt_project(double expected_pay_off) {
    PROJECT *best_project = NULL;
    double best_debt = 0;
    bool first = true;
    unsigned int i;

    for (i=0; i<projects.size(); i++) {
        PROJECT* p = projects[i];
        if (!p->next_runnable_result) continue;
        if (p->non_cpu_intensive) continue;
        if (first || p->anticipated_debt > best_debt) {
            first = false;
            best_project = p;
            best_debt = p->anticipated_debt;
        }
    }
    if (!best_project) return false;

    schedule_result(best_project->next_runnable_result);
    best_project->anticipated_debt -= expected_pay_off;
    best_project->next_runnable_result = 0;
    return true;
}

// Schedule active tasks to be run and preempted.
//
// This is called every second in the do_something() loop
// (with must_reschedule=false)
// and whenever all the input files for a result finish downloading
// (with must_reschedule=true)
//
bool CLIENT_STATE::schedule_cpus(double now) {
    double expected_pay_off;
    ACTIVE_TASK *atp;
    PROJECT *p;
    bool some_app_started = false, first;
    double local_total_resource_share;
    int retval, j;
    double min_debt=0;
    double vm_limit, elapsed_time;
    unsigned int i;

    SCOPE_MSG_LOG scope_messages(log_messages, CLIENT_MSG_LOG::DEBUG_TASK);

    if (projects.size() == 0) return false;
    if (results.size() == 0) return false;

    // Reschedule every cpu_sched_period seconds,
    // or if must_schedule_cpus is set
    // (meaning a new result is available, or a CPU has been freed).
    //
    elapsed_time = now - cpu_sched_last_time;
    if (must_schedule_cpus) {
        must_schedule_cpus = false;
    } else {
        if (elapsed_time < (global_prefs.cpu_scheduling_period_minutes*60)) {
            return false;
        }
    }
    cpu_sched_last_time = now;

    // mark file xfer results as completed;
    // TODO: why do this here??
    //
    handle_file_xfer_apps();

    // tell app doing screensaver (fullscreen) graphics to stop
    // TODO: this interrupts the graphics, even if it's
    // the only app running.  DO THIS A DIFFERENT WAY
    //
    ss_logic.reset();

    // clear temporary variables
    //
    for (i=0; i<projects.size(); i++) {
        projects[i]->next_runnable_result = NULL;
    }
    for (i=0; i<results.size(); i++) {
        results[i]->already_selected = false;
    }

    // do work accounting for active tasks
    //
    for (i=0; i<active_tasks.active_tasks.size(); i++) {
        atp = active_tasks.active_tasks[i];
        if (atp->scheduler_state != CPU_SCHED_SCHEDULED) continue;
        //double task_cpu_time = atp->current_cpu_time - atp->cpu_time_at_last_sched;
        double task_cpu_time = elapsed_time;
        atp->result->project->work_done_this_period += task_cpu_time;
        cpu_sched_work_done_this_period += task_cpu_time;
        atp->next_scheduler_state = CPU_SCHED_PREEMPTED;
    }

    // compute total resource share among projects with runnable results
    //
    assign_results_to_projects();   // see which projects have work
    local_total_resource_share = 0;
    for (i=0; i<projects.size(); i++) {
        p = projects[i];
        if (p->non_cpu_intensive) continue;
        if (p->next_runnable_result) {
            local_total_resource_share += projects[i]->resource_share;
        }
    }

    // adjust project debts
    // reset debts for projects with no runnable results
    // reset temporary fields
    //
    first = true;
    for (i=0; i<projects.size(); i++) {
        p = projects[i];
        if (p->non_cpu_intensive) continue;
        if (!p->next_runnable_result) {
            p->debt = 0;
            p->anticipated_debt = 0;
        } else {
            p->debt +=
                (p->resource_share/local_total_resource_share)
                * cpu_sched_work_done_this_period
                - p->work_done_this_period;
            if (first) {
                first = false;
                min_debt = p->debt;
            } else if (p->debt < min_debt) {
                min_debt = p->debt;
            }
        }
        scope_messages.printf(
            "CLIENT_STATE::schedule_cpus(): overall project debt; project '%s', debt '%f'\n",
            p->project_name, p->debt
        );
    }

    // Normalize debts to zero
    //
    for (i=0; i<projects.size(); i++) {
        p = projects[i];
        if (p->non_cpu_intensive) continue;
        if (p->next_runnable_result) {
            p->debt -= min_debt;
            p->anticipated_debt = p->debt;
            //msg_printf(p, MSG_INFO, "debt %f", p->debt);
            p->next_runnable_result = NULL;
        }
    }

    // schedule tasks for projects in order of decreasing anticipated debt
    //
    for (i=0; i<results.size(); i++) {
        results[i]->already_selected = false;
    }

    expected_pay_off = cpu_sched_work_done_this_period / ncpus;
    for (j=0; j<ncpus; j++) {
        assign_results_to_projects();
        if (!schedule_largest_debt_project(expected_pay_off)) break;
    }

    // schedule non CPU intensive tasks
    //
    for (i=0; i<projects.size(); i++) {
        p = projects[i];
        if (p->non_cpu_intensive && p->next_runnable_result) {
            schedule_result(p->next_runnable_result);
        }
    }

    // preempt, start, and resume tasks
    //
    vm_limit = (global_prefs.vm_max_used_pct/100.)*host_info.m_swap;
    for (i=0; i<active_tasks.active_tasks.size(); i++) {
        atp = active_tasks.active_tasks[i];
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
                report_result_error(
                    *(atp->result), "Couldn't start or resume: %d", retval
                );

                // if we couldn't run something, reschedule
                //
                must_schedule_cpus = true;
                continue;
            }
            atp->scheduler_state = CPU_SCHED_SCHEDULED;
            some_app_started = true;
        }
        atp->cpu_time_at_last_sched = atp->current_cpu_time;
    }

    // reset work accounting
    // doing this at the end of schedule_cpus() because
    // work_done_this_period's can change as apps finish
    //
    for (i=0; i<projects.size(); i++) {
        p = projects[i];
        p->work_done_this_period = 0;
    }
    cpu_sched_work_done_this_period = 0;

    if (some_app_started) {
        app_started = now;
    }

    // debts and active_tasks can only change if some project had a runnable result
    // (and thus if local_total_resource_share is positive)
    //
    if (local_total_resource_share > 0) {
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

const char *BOINC_RCSID_7bf63ad771 = "$Id$";
