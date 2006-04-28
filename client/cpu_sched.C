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

#ifdef _WIN32
#include "boinc_win.h"
#endif

#include "client_msgs.h"
#include "client_state.h"

using std::vector;

#define MAX_DEBT    (86400)
    // maximum project debt

#define CPU_PESSIMISM_FACTOR 0.9
    // assume actual CPU utilization will be this multiple
    // of what we've actually measured recently

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

// adjust project debts (short, long-term)
// NOTE: currently there's the assumption that the only
// non-final call is from schedule_cpus(),
// since that's where total_wall_cpu_time_this_period etc. are zeroed.
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
        // potentially_runnable() can be false right after a result completes,
        // but we still need to update its LTD.
        // In this case its wall_cpu_time_this_period will be nonzero.
        //
        if (!(p->potentially_runnable()) && p->wall_cpu_time_this_period)
            prrs += p->resource_share;
    }

    for (i=0; i<projects.size(); i++) {
        p = projects[i];
        if (p->non_cpu_intensive) continue;
        nprojects++;

        // adjust long-term debts
        //
        if (p->potentially_runnable() || p->wall_cpu_time_this_period) {
            share_frac = p->resource_share/prrs;
            p->long_term_debt += share_frac*total_wall_cpu_time_this_period
                - p->wall_cpu_time_this_period;
        }
        total_long_term_debt += p->long_term_debt;

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
    int j;
    double elapsed_time;
    unsigned int i;

    if (projects.size() == 0) return false;
    if (results.size() == 0) return false;

    // Reschedule every cpu_sched_period seconds,
    // or if must_schedule_cpus is set
    // (meaning a new result is available, or a CPU has been freed).
    //

    elapsed_time = gstate.now - cpu_sched_last_time;
    if (must_schedule_cpus) {
        must_schedule_cpus = false;
    } else {
        if (elapsed_time < (global_prefs.cpu_scheduling_period_minutes*60)) {
            return false;
        }
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

    set_scheduler_mode();
    adjust_debts();

    // mark active tasks as preempted
    // MUST DO THIS AFTER adjust_debts()
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

    enforce_schedule();

    // reset work accounting
    // do this at the end of schedule_cpus() because
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

// preempt, start, and resume tasks
//
void CLIENT_STATE::enforce_schedule() {
    double vm_limit = (global_prefs.vm_max_used_pct/100.)*host_info.m_swap;
    unsigned int i;
    ACTIVE_TASK *atp;
    int retval;

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

                request_schedule_cpus("start failed");
                continue;
            }
            atp->scheduler_state = CPU_SCHED_SCHEDULED;
            app_started = gstate.now;
        }
        atp->cpu_time_at_last_sched = atp->current_cpu_time;
    }
}

// return true if we don't have enough runnable tasks to keep all CPUs busy
//
bool CLIENT_STATE::no_work_for_a_cpu() {
    unsigned int i;
    int count = 0;

    for (i=0; i< results.size(); i++){
        RESULT* rp = results[i];
        if (!rp->runnable_soon()) continue;
        if (rp->project->non_cpu_intensive) continue;
        count++;
    }
    return ncpus > count;
}

// Set the project's rrsim_proc_rate:
// the fraction of each CPU that it will get in round-robin mode.
// Precondition: the project's "active" array is populated
//
void PROJECT::set_rrsim_proc_rate(double per_cpu_proc_rate, double rrs) {
    int nactive = (int)active.size();
    if (nactive == 0) return;
    double x;

    if (rrs) {
        x = resource_share/rrs;
    } else {
        x = 1;      // pathological case; maybe should be 1/# runnable projects
    }

    // if this project has fewer active results than CPUs,
    // scale up its share to reflect this
    //
    if (nactive < gstate.ncpus) {
        x *= ((double)gstate.ncpus)/nactive;
    }

    // But its rate on a given CPU can't exceed 1
    //
    if (x>1) {
        x = 1;
    }
    rrsim_proc_rate = x*per_cpu_proc_rate*CPU_PESSIMISM_FACTOR;
}

// return true if round-robin scheduling will miss a deadline.
// per_cpu_proc_rate is the expected number of CPU seconds per wall second
// on each CPU; rrs is the resource share of runnable projects
//
bool CLIENT_STATE::rr_misses_deadline(double per_cpu_proc_rate, double rrs) {
    PROJECT* p, *pbest;
    RESULT* rp, *rpbest;
    vector<RESULT*> active;
    unsigned int i;
    double x;
    vector<RESULT*>::iterator it;

    SCOPE_MSG_LOG scope_messages(log_messages, CLIENT_MSG_LOG::DEBUG_SCHED_CPU);

    // Initilize the "active" and "pending" lists for each project.
    // These keep track of that project's results
    //
    for (i=0; i<projects.size(); i++) {
        p = projects[i];
        p->active.clear();
        p->pending.clear();
    }

    for (i=0; i<results.size(); i++) {
        rp = results[i];
        if (rp->aborted_via_gui) continue;
        if (!rp->runnable()) continue;
        if (rp->aborted_via_gui) continue;
        if (rp->project->non_cpu_intensive) continue;
        rp->rrsim_cpu_left = rp->estimated_cpu_time_remaining();
        p = rp->project;
        if (p->active.size() < (unsigned int)ncpus) {
            active.push_back(rp);
            p->active.push_back(rp);
        } else {
            p->pending.push_back(rp);
        }
    }

    for (i=0; i<projects.size(); i++) {
        p = projects[i];
        p->set_rrsim_proc_rate(per_cpu_proc_rate, rrs);
    }

    // Simulation loop.  Keep going until work done
    //
    double sim_now = now;
    while (active.size()) {

        // compute finish times and see which result finishes first
        //
        rpbest = NULL;
        for (i=0; i<active.size(); i++) {
            rp = active[i];
            p = rp->project;
            rp->rrsim_finish_delay = rp->rrsim_cpu_left/p->rrsim_proc_rate;
            if (!rpbest || rp->rrsim_finish_delay < rpbest->rrsim_finish_delay) {
                rpbest = rp;
            }
        }

        // "rpbest" is first result to finish.  Does it miss its deadline?
        //
        double diff = sim_now + rpbest->rrsim_finish_delay - rpbest->computation_deadline();
        if (diff > 0) {
            scope_messages.printf(
                "rr_sim: result %s misses deadline by %f\n", rpbest->name, diff
            );
            return true;
        }

        // remove *rpbest from active set,
        // and adjust CPU time left for other results
        //
        it = active.begin();
        while (it != active.end()) {
            rp = *it;
            if (rp == rpbest) {
                it = active.erase(it);
            } else {
                x = rp->project->rrsim_proc_rate*rpbest->rrsim_finish_delay;
                rp->rrsim_cpu_left -= x;
                it++;
            }
        }

        pbest = rpbest->project;

        // remove *rpbest from its project's active set
        //
        it = pbest->active.begin();
        while (it != pbest->active.end()) {
            rp = *it;
            if (rp == rpbest) {
                it = pbest->active.erase(it);
            } else {
                it++;
            }
        }

        // If project has more results, add one to active set.
        //
        if (pbest->pending.size()) {
            rp = pbest->pending[0];
            pbest->pending.erase(pbest->pending.begin());
            active.push_back(rp);
            pbest->active.push_back(rp);
        }

        // If all work done for a project, subtract that project's share
        // and recompute processing rates
        //
        if (pbest->active.size() == 0) {
            rrs -= pbest->resource_share;
            for (i=0; i<projects.size(); i++) {
                p = projects[i];
                p->set_rrsim_proc_rate(per_cpu_proc_rate, rrs);
            }
        }

        sim_now += rpbest->rrsim_finish_delay;
    }
    scope_messages.printf( "rr_sim: deadlines met\n");
    return false;
}

// Decide on CPU sched policy
// Namely, set the variable cpu_earliest_deadline_first
// and print a message if we're changing its value
//
void CLIENT_STATE::set_scheduler_mode() {
    bool use_earliest_deadline_first = false;
    double per_cpu_proc_rate = avg_proc_rate()/ncpus;
        // how many CPU seconds per wall second we get on each CPU,
        // taking into account on_frac, active_frac, and cpu_efficiency

    double rrs = runnable_resource_share();

    if (rr_misses_deadline(per_cpu_proc_rate, rrs)) {
        // if round robin would miss a deadline, use EDF
        //
        use_earliest_deadline_first = true;
    }


    if (cpu_earliest_deadline_first && !use_earliest_deadline_first) {
        msg_printf(NULL, MSG_INFO,
            "Resuming round-robin CPU scheduling."
        );
    }
    if (!cpu_earliest_deadline_first && use_earliest_deadline_first) {
        msg_printf(NULL, MSG_INFO,
            "Using earliest-deadline-first scheduling because computer is overcommitted."
        );
    }
    cpu_earliest_deadline_first = use_earliest_deadline_first;
}

