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

#define MAX_DEBT    (86400)
    // maximum project debt

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
            gstate.now - rp->final_cpu_time,
                // KLUDGE - should be result start time
            rp->final_cpu_time,
            CPU_HALF_LIFE,
            p->exp_avg_cpu,
            p->exp_avg_mod_time
        );
    }

    double wall_cpu_time = now - cpu_sched_last_time;
    at.result->project->wall_cpu_time_this_period += wall_cpu_time;
    total_wall_cpu_time_this_period += wall_cpu_time;

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
                "Computation for result %s finished", atp->result->name
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
            if (fip->verify_file(false)) return false;
        }
    }

    for (i=0; i<wup->input_files.size(); i++) {
        fip = wup->input_files[i].file_info;
        if (fip->status != FILE_PRESENT) return false;
        if (fip->verify_file(false)) return false;
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
        rp = atp->result;
        if (rp->already_selected) continue;
        if (!rp->runnable()) continue;
        project = rp->project;
        if (!project->next_runnable_result) {
            project->next_runnable_result = rp;
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
        if (rp->already_selected) continue;
        if (lookup_active_task_by_result(rp)) continue;
        if (!rp->runnable()) continue;

        project = rp->project;
        if (project->next_runnable_result) continue;

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

// Schedule the active task with the earliest deadline
// Return true iff a task was scheduled.
//
bool CLIENT_STATE::schedule_earliest_deadline_result(double expected_pay_off) {
    PROJECT *best_project = NULL;
    RESULT *best_result = NULL;
    double earliest_deadline=0;
    bool first = true;
    unsigned int i;

    for (i=0; i < results.size(); ++i) {
        RESULT *rp = results[i];
        if (!rp->runnable()) continue;
        if (rp->project->non_cpu_intensive) continue;
        if (rp->already_selected) continue;
        if (first || rp->report_deadline < earliest_deadline) {
            first = false;
            best_project = rp->project;
            best_result = rp;
            earliest_deadline = rp->report_deadline;
        }
    }
    if (!best_result) return false;

//    msg_printf(0, MSG_INFO, "earliest deadline: %f %s", earliest_deadline, best_result->name);
    schedule_result(best_result);
    best_result->already_selected = true;
    best_project->anticipated_debt -= expected_pay_off;
    best_project->next_runnable_result = 0;
    return true;
}

// adjust project debts (short, long-term)
//
void CLIENT_STATE::adjust_debts() {
    unsigned int i;
    bool first = true;
    double total_long_term_debt = 0;
    double potentially_runnable_resource_share = 0;
    double runnable_resource_share = 0;
    int count_cpu_intensive = 0;
    PROJECT *p;
    double min_short_term_debt=0, share_frac;
    double wall_cpu_time = gstate.now - cpu_sched_last_time;

    SCOPE_MSG_LOG scope_messages(log_messages, CLIENT_MSG_LOG::DEBUG_TASK);

    // Total up total and per-project "wall CPU" since last CPU reschedule.
    // "Wall CPU" is the wall time during which a task was
    // runnable (at the OS level).
    //
    // We use wall CPU for debt calculation
    // (instead of reported actual CPU) for two reasons:
    // 1) the process might have paged a lot, so the actual CPU
    //    may be a lot less than wall CPU
    // 2) BOINC relies on apps to report their CPU time.
    //    Sometimes there are bugs and apps report zero CPU.
    //    It's safer not to trust them.
    //
    for (i=0; i<active_tasks.active_tasks.size(); i++) {
        ACTIVE_TASK* atp = active_tasks.active_tasks[i];
        if (atp->scheduler_state != CPU_SCHED_SCHEDULED) continue;
        if (atp->result->project->non_cpu_intensive) continue;

        atp->result->project->wall_cpu_time_this_period += wall_cpu_time;
        total_wall_cpu_time_this_period += wall_cpu_time;
    }

    // find total resource shares of runnable and potentially runnable projects
    //
    for (i=0; i<projects.size(); ++i) {
        p = projects[i];
        if (p->runnable()) {
            runnable_resource_share += p->resource_share;
        }
        if (p->potentially_runnable()) {
            potentially_runnable_resource_share += p->resource_share;
        }
    }

    for (i=0; i<projects.size(); i++) {
        p = projects[i];
        if (p->non_cpu_intensive) continue;
        count_cpu_intensive++;

        // adjust long-term debts
        //
        if (p->potentially_runnable()) {
            share_frac = p->resource_share/potentially_runnable_resource_share;
            p->long_term_debt += share_frac*total_wall_cpu_time_this_period
                - p->wall_cpu_time_this_period
            ;
        }
        total_long_term_debt += p->long_term_debt;

        // adjust short term debts
        //
        if (!p->runnable()) {
            p->short_term_debt = 0;
            p->anticipated_debt = 0;
        } else {
            share_frac = p->resource_share/runnable_resource_share;
            p->short_term_debt += share_frac*total_wall_cpu_time_this_period
                - p->wall_cpu_time_this_period
            ;
            if (first) {
                first = false;
                min_short_term_debt = p->short_term_debt;
            } else if (p->short_term_debt < min_short_term_debt) {
                min_short_term_debt = p->short_term_debt;
            }
        }
        scope_messages.printf(
            "CLIENT_STATE::schedule_cpus(): project %s: short-term debt %f\n",
            p->project_name, p->short_term_debt
        );
    }

    // long-term debt:
    //  normalize so mean is zero,
    // short-term debt:
    //  normalize so min is zero, and cap at MAX_DEBT
    //
    double avg_long_term_debt = total_long_term_debt / count_cpu_intensive;
    for (i=0; i<projects.size(); i++) {
        p = projects[i];
        if (p->non_cpu_intensive) continue;
        if (p->runnable()) {
            p->short_term_debt -= min_short_term_debt;
            if (p->short_term_debt > MAX_DEBT) {
                p->short_term_debt = MAX_DEBT;
            }
            p->anticipated_debt = p->short_term_debt;
            //msg_printf(p, MSG_INFO, "debt %f", p->short_term_debt);
        }
        p->long_term_debt -= avg_long_term_debt;
    }
}


// Schedule active tasks to be run and preempted.
// This is called in the do_something() loop
//
bool CLIENT_STATE::schedule_cpus() {
    double expected_pay_off;
    ACTIVE_TASK *atp;
    PROJECT *p;
    int retval, j;
    double vm_limit, elapsed_time;
    unsigned int i;

    SCOPE_MSG_LOG scope_messages(log_messages, CLIENT_MSG_LOG::DEBUG_TASK);

    if (projects.size() == 0) return false;
    if (results.size() == 0) return false;

    // Reschedule every cpu_sched_period seconds,
    // or if must_schedule_cpus is set
    // (meaning a new result is available, or a CPU has been freed).
    //
    elapsed_time = gstate.now - cpu_sched_last_time;
    if (must_schedule_cpus) {
        must_schedule_cpus = false;
//        msg_printf(0, MSG_INFO, "schedule_cpus: must schedule");
    } else {
        if (elapsed_time < (global_prefs.cpu_scheduling_period_minutes*60)) {
            return false;
        }
//        msg_printf(0, MSG_INFO, "schedule_cpus: time %f", elapsed_time);
    }
    cpu_sched_last_time = gstate.now;

    // mark file xfer results as completed;
    // TODO: why do this here??
    //
    handle_file_xfer_apps();

    // clear temporary variables
    //
    for (i=0; i<projects.size(); i++) {
        projects[i]->next_runnable_result = NULL;
    }
    for (i=0; i<results.size(); i++) {
        results[i]->already_selected = false;
    }

    set_cpu_scheduler_modes();
    adjust_debts();

    // mark active tasks as preempted
    // MUST DO THIS AFTER accumulate_work()
    //
    for (i=0; i<active_tasks.active_tasks.size(); i++) {
        atp = active_tasks.active_tasks[i];
        atp->next_scheduler_state = CPU_SCHED_PREEMPTED;
    }
    expected_pay_off = total_wall_cpu_time_this_period / ncpus;
    for (j=0; j<ncpus; j++) {
        if (cpu_earliest_deadline_first) {
            if (!schedule_earliest_deadline_result(expected_pay_off)) break;
        } else {
            assign_results_to_projects();
            if (!schedule_largest_debt_project(expected_pay_off)) break;
        }
    }

    // schedule non CPU intensive tasks
    //
    for (i=0; i<results.size(); i++) {
        RESULT* rp = results[i];
        if (rp->project->non_cpu_intensive && rp->runnable()) {
            schedule_result(rp);
        }
    }

    // preempt, start, and resume tasks
    //
    vm_limit = (global_prefs.vm_max_used_pct/100.)*host_info.m_swap;
    for (i=0; i<active_tasks.active_tasks.size(); i++) {
        atp = active_tasks.active_tasks[i];
        //msg_printf(p, MSG_INFO, "result %s state %d", atp->result->name, atp->scheduler_state);
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
                request_schedule_cpus("start failed");
                continue;
            }
            atp->scheduler_state = CPU_SCHED_SCHEDULED;
            app_started = gstate.now;
        }
        atp->cpu_time_at_last_sched = atp->current_cpu_time;
    }

    // reset work accounting
    // doing this at the end of schedule_cpus() because
    // wall_cpu_time_this_period's can change as apps finish
    //
    for (i=0; i<projects.size(); i++) {
        p = projects[i];
        p->wall_cpu_time_this_period = 0;
    }
    total_wall_cpu_time_this_period = 0;

    set_client_state_dirty("schedule_cpus");
    return true;
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
    msg_printf(0, MSG_INFO, "request_reschedule_cpus: %s", where);
}

const char *BOINC_RCSID_7bf63ad771 = "$Id$";
