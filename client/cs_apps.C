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


#ifndef NEW_CPU_SCHED

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

        // don't start results if > 2 uploads in progress
        //
        if (project->nactive_uploads > 2) continue;

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

#endif

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

#ifndef NEW_CPU_SCHED

// Schedule an active task for the project with the largest anticipated debt
// among those that have a runnable result.
// Return true iff a task was scheduled.
//
bool CLIENT_STATE::schedule_largest_debt_project(double expected_pay_off) {
    PROJECT *best_project = NULL;
    double best_debt = -MAX_DEBT;
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
bool CLIENT_STATE::schedule_earliest_deadline_result() {
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
    return true;
}
#endif

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

#ifndef NEW_CPU_SCHED

// adjust project debts (short, long-term)
//
void CLIENT_STATE::adjust_debts() {
    unsigned int i;
    double total_long_term_debt = 0;
    double total_short_term_debt = 0;
    double prrs, rrs;
    int nprojects=0, nrprojects=0;
    PROJECT *p;
    double share_frac;
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
        if (atp->non_cpu_intensive) continue;

        atp->result->project->wall_cpu_time_this_period += wall_cpu_time;
        total_wall_cpu_time_this_period += wall_cpu_time;
        total_cpu_time_this_period += atp->current_cpu_time - atp->cpu_time_at_last_sched;
    }

    time_stats.update_cpu_efficiency(
        total_wall_cpu_time_this_period, total_cpu_time_this_period
    );

    rrs = runnable_resource_share();
    prrs = potentially_runnable_resource_share();

    for (i=0; i<projects.size(); i++) {
        p = projects[i];
        if (p->non_cpu_intensive) continue;

        // adjust long-term debts
        //
        if (p->potentially_runnable()) {
            nprojects++;
            share_frac = p->resource_share/prrs;
            p->long_term_debt += share_frac*total_wall_cpu_time_this_period
                - p->wall_cpu_time_this_period;
            total_long_term_debt += p->long_term_debt;
        }

        // adjust short term debts
        //
        if (p->runnable()) {
            nrprojects++;
            share_frac = p->resource_share/rrs;
            p->short_term_debt += share_frac*total_wall_cpu_time_this_period
                - p->wall_cpu_time_this_period
            ;
            total_short_term_debt += p->short_term_debt;
        } else {
            p->short_term_debt = 0;
            p->anticipated_debt = 0;
        }
        scope_messages.printf(
            "CLIENT_STATE::adjust_debts(): project %s: short-term debt %f\n",
            p->project_name, p->short_term_debt
        );
    }

    if (nprojects==0) return;

    // long-term debt:
    //  normalize so mean is zero,
    // short-term debt:
    //  normalize so mean is zero, and limit abs value at MAX_DEBT
    //
    double avg_long_term_debt = total_long_term_debt / nprojects;
    double avg_short_term_debt = 0;
    if (nrprojects) {
        avg_short_term_debt = total_short_term_debt / nrprojects;
    }
    for (i=0; i<projects.size(); i++) {
        p = projects[i];
        if (p->non_cpu_intensive) continue;
        if (p->runnable()) {
            p->short_term_debt -= avg_short_term_debt;
            if (p->short_term_debt > MAX_DEBT) {
                p->short_term_debt = MAX_DEBT;
            }
            if (p->short_term_debt < -MAX_DEBT) {
                p->short_term_debt = -MAX_DEBT;
            }
            p->anticipated_debt = p->short_term_debt;
            //msg_printf(p, MSG_INFO, "debt %f", p->short_term_debt);
        }
        if (p->potentially_runnable()) {
            p->long_term_debt -= avg_long_term_debt;
        }
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
        scope_messages.printf("CLIENT_STATE::schedule_cpus(): must schedule\n");
    } else {
        if (elapsed_time < (global_prefs.cpu_scheduling_period_minutes*60)) {
            return false;
        }
        scope_messages.printf("CLIENT_STATE::schedule_cpus(): time %f\n", elapsed_time);
    }

    // mark file xfer results as completed;
    // TODO: why do this here??
    //
    handle_file_xfer_apps();

    // clear temporary variables
    //
    for (i=0; i<projects.size(); i++) {
        projects[i]->next_runnable_result = NULL;
        projects[i]->nactive_uploads = 0;
    }
    for (i=0; i<results.size(); i++) {
        results[i]->already_selected = false;
    }
    for (i=0; i<file_xfers->file_xfers.size(); i++) {
        FILE_XFER* fxp = file_xfers->file_xfers[i];
        if (fxp->is_upload) {
            fxp->fip->project->nactive_uploads++;
        }
    }

    set_scheduler_modes();
    adjust_debts();

    // mark active tasks as preempted
    // MUST DO THIS AFTER accumulate_work()
    //
    for (i=0; i<active_tasks.active_tasks.size(); i++) {
        atp = active_tasks.active_tasks[i];
        if (atp->non_cpu_intensive) {
            atp->next_scheduler_state = CPU_SCHED_SCHEDULED;
        } else {
            atp->next_scheduler_state = CPU_SCHED_PREEMPTED;
        }
    }

    expected_pay_off = total_wall_cpu_time_this_period / ncpus;
    for (j=0; j<ncpus; j++) {
        if (cpu_earliest_deadline_first) {
            if (!schedule_earliest_deadline_result()) break;
        } else {
            assign_results_to_projects();
            if (!schedule_largest_debt_project(expected_pay_off)) break;
        }
    }

    // schedule new non CPU intensive tasks
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
        scope_messages.printf("CLIENT_STATE::schedule_cpus(): project %s result %s state %d\n", 
            atp->result->project->project_name, atp->result->name, atp->scheduler_state);
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
    total_cpu_time_this_period = 0;
    cpu_sched_last_time = gstate.now;

    set_client_state_dirty("schedule_cpus");
    return true;
}


// This is called when the client is initialized.
// Try to restart any tasks that were running when we last shut down.
//
int CLIENT_STATE::restart_tasks() {
    return active_tasks.restart_tasks(ncpus);
}
#endif

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

#ifdef NEW_CPU_SCHED

void PROJECT::get_ordered_runnable_results(vector<PRESULT>& presults) {
    presults.clear();
    for (unsigned int i=0; i<gstate.results.size(); i++) {
        RESULT* rp = gstate.results[i];
        if (rp->project != this) continue;
        if (!rp->runnable()) continue;
        PRESULT pr;
        pr.p = rp;
        presults.push_back(pr);
    }
    sort(presults.begin(), presults.end());
}

// compute the immediate CPU shares needed for each result,
// and for the project, to meet all deadlines
// and finish past-due work as quickly as possible
// 
void PROJECT::compute_cpu_share_needed() {
    cpu_share = 0;
    double time = gstate.now, dt, wcn, x;
    vector<PRESULT> presults;
    unsigned int i;

    get_ordered_runnable_results(presults);
    for (i=0; i<presults.size(); i++) {
        RESULT* r = presults[i].p;
        if (r->report_deadline < gstate.now) {
            cpu_share += 1;
            r->cpu_share = 1;
        } else {
            dt = r->report_deadline - time;
            if (dt) {
                wcn = r->estimated_cpu_time_remaining();
                double cs = wcn/dt;
                if (cs > cpu_share) {
                    double x = (wcn + cpu_share*(time-gstate.now))/(r->report_deadline-gstate.now);
                    r->cpu_share = x - cpu_share;
                    cpu_share = x;
                } else {
                    r->cpu_share = 0;
                }
            } else {
                x = wcn/(time-gstate.now)-cpu_share;
                cpu_share += x;
                r->cpu_share = x;
            }
        }
        time = r->report_deadline;
    }
    needed_cpu_share = cpu_share;
}

bool PROJECT::has_emergency(double rrs) {
    double cpu_share = gstate.ncpus*resource_share/rrs;
    return needed_cpu_share > cpu_share;
}

// given a CPU share, choose results to run,
// and assign their CPU shares
//
void PROJECT::allocate(double cpu_share_left) {
    vector<PRESULT> presults;

    get_ordered_runnable_results(presults);
    for (unsigned i=0; i<presults.size(); i++) {
        RESULT* r = presults[i].p;
        if (r->cpu_share) {
            gstate.schedule_result(r);
            ACTIVE_TASK* atp = gstate.lookup_active_task_by_result(r);
            atp->scheduler_state = CPU_SCHED_SCHEDULED;
            if (r->cpu_share > cpu_share_left) {
                r->cpu_share = cpu_share_left;
            }
            cpu_share_left -= r->cpu_share;
            if (cpu_share_left == 0) break;
        }
    }
}

#define EMERGENCY_LIMIT 86400*7

void CPU_SCHEDULER::clear_tasks() {
    ACTIVE_TASK* atp;
    unsigned int i;
    for (i=0; i<gstate.active_tasks.active_tasks.size(); i++) {
        atp = gstate.active_tasks.active_tasks[i];
        if (atp->non_cpu_intensive) {
            atp->next_scheduler_state = CPU_SCHED_SCHEDULED;
        } else {
            atp->next_scheduler_state = CPU_SCHED_PREEMPTED;
        }
    }
}

void CPU_SCHEDULER::do_accounting() {
    unsigned int i;
    static double last_time;
    double dt = gstate.now - last_time;
    last_time = gstate.now;

    for (i=0; i<gstate.results.size(); i++) {
        RESULT* r = gstate.results[i];
        ACTIVE_TASK* atp = gstate.lookup_active_task_by_result(r);
        if (!atp) continue;
    //for each running result R
    //    R.std -= dt;
    //for each result R in schedule
    //    R.std += r.cpu_share/dt
    }
#if 0
    for (i=0; i<gstate.projects.size(); i++) {
        PROJECT* p = gstate.projects[i];
        if (p->in_emergency) {
            p->emergency_budget -= dt*p->emergency_resource_share;
            if (p->emergency_budget < 0) {
                p->emergency_eligible = false;
            }
        } else {
            p->emergency_budget += dt;
            if (p->emergency_budget > EMERGENCY_LIMIT) {
                p->emergency_eligible = true;
                p->emergency_budget = EMERGENCY_LIMIT;
            }
        }
    }
#endif
}

// Called whenever something happens that could change the schedule;
// Also called periodically (every few hours)
// in case results in a schedule are going slower or faster
// than expected (which could change the schedule)
//
void CPU_SCHEDULER::make_schedule() {
    bool have_emergency=false;
    double non_emergency_rs = 0, cs, cpu_share_left;
    PROJECT* p;
    unsigned int i;
    double rrs = gstate.runnable_resource_share();

    clear_tasks();

    for (i=0; i<gstate.projects.size(); i++) {
        p = gstate.projects[i];
        if (!p->runnable()) continue;
        p->compute_cpu_share_needed();
        if (p->has_emergency(rrs)) {
            have_emergency = true;
        } else {
            non_emergency_rs += p->resource_share;
        }
    }

    cpu_share_left = gstate.ncpus;
    if (have_emergency) {
        for (i=0; i<gstate.projects.size(); i++) {
            p = gstate.projects[i];
            if (!p->runnable()) continue;
            if (p->has_emergency(rrs)) {
                p->allocate(cpu_share_left);
                if (cpu_share_left <= 0) break;
            }
        }
        if (cpu_share_left) {
            for (i=0; i<gstate.projects.size(); i++) {
                p = gstate.projects[i];
                if (!p->runnable()) continue;
                if (!p->has_emergency(rrs)) {
                    cs = cpu_share_left*p->resource_share/non_emergency_rs;
                    p->allocate(cs);
                }
            }
        }
    } else {
        for (i=0; i<gstate.projects.size(); i++) {
            p = gstate.projects[i];
            if (!p->runnable()) continue;
            cs = cpu_share_left*p->resource_share/non_emergency_rs;
            p->allocate(cs);
        }
    }
}

// called every ~10 seconds to time-slice among results in a schedule
// Handle checkpoint detection here.
// make_schedule() has already been called.
//
void CPU_SCHEDULER::enforce() {
    static bool first = true;

    if (first) {
        //for results R in schedule by decreasing STD
        //    start R
        first = false;
        return;
    }

    do_accounting();

    //for each running task T not in schedule
    //    T.abort
}

#endif

const char *BOINC_RCSID_7bf63ad771 = "$Id$";
