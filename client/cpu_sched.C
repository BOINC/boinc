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
#include "util.h"
#include "log_flags.h"

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
    for (i=0; i<active_tasks.active_tasks.size(); i++) {
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

    if (log_flags.cpu_sched_detail) {
        msg_printf(best_project, MSG_INFO,
            "highest debt: %f %s",
            best_project->anticipated_debt,
            best_project->next_runnable_result->name
        );
    }
    ordered_scheduled_results.push_back(best_project->next_runnable_result);
    best_project->anticipated_debt -= (1 - best_project->resource_share / gstate.runnable_resource_share()) * expected_pay_off;
    best_project->next_runnable_result = 0;
    return true;
}

// Schedule the active task with the earliest deadline
// Return true iff a task was scheduled.
//
bool CLIENT_STATE::schedule_earliest_deadline_result(double expected_pay_off) {
    RESULT *best_result = NULL;
    unsigned int i;

    for (i=0; i < results.size(); i++) {
        RESULT *rp = results[i];
        if (!rp->runnable()) continue;
        if (rp->project->non_cpu_intensive) continue;
        if (rp->already_selected) continue;
        if (!rp->project->cpu_scheduler_deadlines_missed_scratch ) continue;
        if (!best_result || rp->report_deadline < best_result->report_deadline) {
            best_result = rp;
        }
    }
    if (!best_result) return false;

    if (log_flags.cpu_sched_detail) {
        msg_printf(best_result->project, MSG_INFO,
            "earliest deadline: %f %s",
            best_result->report_deadline, best_result->name
        );
    }
    ordered_scheduled_results.push_back(best_result);
    best_result->already_selected = true;
    best_result->project->anticipated_debt -= (1 - best_result->project->resource_share / gstate.runnable_resource_share()) * expected_pay_off;
    best_result->project->cpu_scheduler_deadlines_missed_scratch--;

    return true;
}

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

        // potentially_runnable() can be false right after a result completes,
        // but we still need to update its LTD.
        // In this case its wall_cpu_time_this_period will be nonzero.
        //
        if (!(p->potentially_runnable()) && p->wall_cpu_time_this_period) {
            prrs += p->resource_share;
        }
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
                - p->wall_cpu_time_this_period;
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
            //msg_printf(p, MSG_INFO, "debt %f", p->short_term_debt);
        }

        p->long_term_debt -= avg_long_term_debt;
    }

    // reset work accounting
    //
    for (i=0; i<projects.size(); i++) {
        p = projects[i];
        p->wall_cpu_time_this_period = 0.0;
    }
    total_wall_cpu_time_this_period = 0.0;
    total_cpu_time_this_period = 0.0;
}


// Schedule active tasks to be run and preempted.
// This is called in the do_something() loop
//
bool CLIENT_STATE::schedule_cpus() {
    double expected_pay_off;
    PROJECT *p;
    double elapsed_time;
    unsigned int i;

    SCOPE_MSG_LOG scope_messages(log_messages, CLIENT_MSG_LOG::DEBUG_SCHED_CPU);

    if (projects.size() == 0) return false;
    if (results.size() == 0) return false;

    // Reschedule every cpu_sched_period seconds,
    // or if must_schedule_cpus is set
    // (meaning a new result is available, or a CPU has been freed).
    //
    elapsed_time = gstate.now - cpu_sched_last_check;
    if (elapsed_time >= gstate.global_prefs.cpu_scheduling_period_minutes * 60) {
        // The CPU scheduler runs ...,
        // when the end of the user-specified scheduling period is reached...
        // check at least once every scheduler period.
        //
        request_schedule_cpus("Process swap time reached.");
        cpu_sched_last_check = now;
    }

    // if the count of running tasks is not either ncpus
    // or the count of runnable results a re-schedule is mandatory.
    //
    if (!must_schedule_cpus) {
        int count_running_tasks = 0;
        for (i=0; i<active_tasks.active_tasks.size(); i++) {
            if (!active_tasks.active_tasks[i] || !active_tasks.active_tasks[i]->result) continue;
            if (CPU_SCHED_SCHEDULED != active_tasks.active_tasks[i]->scheduler_state) continue;
            if (active_tasks.active_tasks[i]->result->project->non_cpu_intensive) continue;
            count_running_tasks++;
        }
        if (count_running_tasks != ncpus){
            int count_runnable_results = 0;
            for (i=0; i<results.size(); i++) {
                if (!results[i]->runnable()) continue;
                if (results[i]->project->non_cpu_intensive) continue;
                count_runnable_results++;
            }
            if (count_running_tasks != count_runnable_results) {
                must_schedule_cpus = true;
            }
        }
    }

    // it has not been requested, and there is no immediate apparent need, so...
    //
    if (!must_schedule_cpus) return false;

    must_schedule_cpus = false;

    // call the rr simulator to calculate what results miss deadline,
    // and which projects need extra CPU attention.
    //
    scope_messages.printf("rr_sim: calling from cpu_scheduler");
    rr_misses_deadline(
        avg_proc_rate()/ncpus, runnable_resource_share(), false, true
    );
    if (log_flags.cpu_sched_detail) {
        for (i=0; i<results.size(); i++){
            RESULT * rp = results[i];
            if (rp->rr_sim_misses_deadline && !rp->last_rr_sim_missed_deadline) {
                    msg_printf(rp->project, MSG_INFO,
                        "Result %s now misses deadline.", rp->name
                    );
            }
            else if (!rp->rr_sim_misses_deadline && rp->last_rr_sim_missed_deadline) {
                    msg_printf(rp->project, MSG_INFO,
                        "Result %s now meets deadline.", rp->name
                    );
            }
        }
        for (i=0; i<projects.size(); i++) {
            p = projects[i];
            if (p->rr_sim_deadlines_missed) {
                msg_printf(p, MSG_INFO,
                    "Project has %d deadline misses",
                    p->rr_sim_deadlines_missed
                );
            }
        }
    }

    // mark file xfer results as completed;
    // TODO: why do this here??
    //
    handle_file_xfer_apps();

    // set temporary variables
    //
    for (i=0; i<results.size(); i++) {
        results[i]->already_selected = false;
    }
    for (i=0; i<projects.size(); i++) {
        projects[i]->next_runnable_result = NULL;
        projects[i]->nactive_uploads = 0;
        projects[i]->anticipated_debt = projects[i]->short_term_debt;
        projects[i]->cpu_scheduler_deadlines_missed_scratch = projects[i]->rr_sim_deadlines_missed;
    }
    for (i=0; i<file_xfers->file_xfers.size(); i++) {
        FILE_XFER* fxp = file_xfers->file_xfers[i];
        if (fxp->is_upload) {
            fxp->fip->project->nactive_uploads++;
        }
    }

    // calculate the current long and short term debts
    // based on the work done during the last period
    //
    adjust_debts();

    cpu_sched_last_time = gstate.now;

    // this is set by set_scheduler_mode, and we don't want to do it twice.
    // note that since this is the anticipated debt,
    // we should be basing this on the anticipated time to crunch,
    // not the time in the last period.
    // The expectation is that each period will be full.
    //
    expected_pay_off = gstate.global_prefs.cpu_scheduling_period_minutes * 60;

    ordered_scheduled_results.clear();
    // Let P be the project with the earliest-deadline runnable result
    // among projects with deadlines_missed(P)>0.
    // Let R be P's earliest-deadline runnable result not scheduled yet.
    // Tiebreaker: least index in result array.
    // If such an R exists, schedule R, decrement P's anticipated debt,
    // and decrement deadlines_missed(P). 
    // If there are more CPUs, and projects with deadlines_missed(P)>0, go to 1. 
    //
    while ((int)ordered_scheduled_results.size() < ncpus) {
        if (!schedule_earliest_deadline_result(expected_pay_off)) break;
    };

    // If all CPUs are scheduled, stop. 
    // If there is a result R that is currently running,
    // and has been running for less than the CPU scheduling period,
    // schedule R and go to 5. 
    // Find the project P with the greatest anticipated debt,
    // select one of P's runnable results
    // (picking one that is already running, if possible,
    // else the result with earliest deadline) and schedule that result. 
    // Decrement P's anticipated debt by the 'expected payoff'
    // (the scheduling period divided by NCPUS). 
    // Go to 5.
    //
    while ((int)ordered_scheduled_results.size() < ncpus) {
        assign_results_to_projects();
        if (!schedule_largest_debt_project(expected_pay_off)) break;
    };

    request_enforce_schedule("");
    set_client_state_dirty("schedule_cpus");
    return true;
}

// preempt, start, and resume tasks
//
bool CLIENT_STATE::enforce_schedule() {
    if (!must_enforce_cpu_schedule)
        return false;

    must_enforce_cpu_schedule = false;
    bool rslt = false;

    if (log_flags.task) {
        msg_printf(0, MSG_INFO, "Enforcing schedule");
    }

    // set temporary variables
    unsigned int i;
    for (i=0; i<projects.size(); i++){
        projects[i]->enforcement_deadlines_missed_scratch = projects[i]->rr_sim_deadlines_missed;
    }
    for (i=0; i< active_tasks.active_tasks.size(); i++) {
        // assume most things don't change.
        active_tasks.active_tasks[i]->next_scheduler_state = active_tasks.active_tasks[i]->scheduler_state;
    }

    std::vector<ACTIVE_TASK *> running_task_heap;
        // this needs to be searchable so it is NOT a priority_queue.
    for (i=0; i<active_tasks.active_tasks.size(); i++) {
        ACTIVE_TASK *atp = active_tasks.active_tasks[i];
        if (atp->result->project->non_cpu_intensive) continue;
        if (!atp->result->runnable()) {
            atp->next_scheduler_state = CPU_SCHED_PREEMPTED;
        }
        else if (CPU_SCHED_SCHEDULED == atp->scheduler_state) {
            running_task_heap.push_back(atp);
        }
    }
    for (i=running_task_heap.size(); (int)i<ncpus; i++) {
        running_task_heap.push_back(NULL);
    }
    std::make_heap(
        running_task_heap.begin(),
        running_task_heap.end(),
        running_task_sort_pred
    );
    while (running_task_heap.size() > (unsigned int)ncpus) {
        if (running_task_heap[0] != NULL) {
            running_task_heap[0]->next_scheduler_state = CPU_SCHED_PREEMPTED;
        }
        std::pop_heap(
            running_task_heap.begin(),
            running_task_heap.end(),
            running_task_sort_pred
        );
        running_task_heap.pop_back();  // get rid of the tail that is not needed.
    }

    // now that things are set up,
    // compare the heap to the prepared vector from the cpu_scheduler.
    //
    for (i=0; i<ordered_scheduled_results.size(); i++) {
        RESULT * rp = ordered_scheduled_results[i];
        ACTIVE_TASK *atp = NULL;
        for (std::vector<ACTIVE_TASK*>::iterator it = running_task_heap.begin(); it != running_task_heap.end(); it++) {
            ACTIVE_TASK *atp1 = *it;
            if (atp1 && atp1->result == rp) {
                atp = atp1;
                running_task_heap.erase(it);
                std::make_heap(running_task_heap.begin(), running_task_heap.end(), running_task_sort_pred);
                break;
            }
        }
        if (atp) continue;  // the next one to be scheduled is already running.
        if (rp->project->enforcement_deadlines_missed_scratch
            || !running_task_heap[0]
            || (gstate.now - running_task_heap[0]->episode_start_wall_time > gstate.global_prefs.cpu_scheduling_period_minutes && gstate.now - running_task_heap[0]->checkpoint_wall_time < 10 )
        ) {
            if (rp->project->enforcement_deadlines_missed_scratch) {
                --rp->project->enforcement_deadlines_missed_scratch;
            }
            // we now have some enforcement to do.
            //
            if (running_task_heap[0]) {
                // we have a task to unschedule.
                running_task_heap[0]->next_scheduler_state = CPU_SCHED_PREEMPTED;
            }
            schedule_result(rp);
            std::pop_heap(
                running_task_heap.begin(),
                running_task_heap.end(),
                running_task_sort_pred
            );
            running_task_heap.pop_back();
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

    double vm_limit = (global_prefs.vm_max_used_pct/100.0)*host_info.m_swap;

    ACTIVE_TASK *atp;

    for (i=0; i<active_tasks.active_tasks.size(); i++) {
        atp = active_tasks.active_tasks[i];
        if (atp->scheduler_state == CPU_SCHED_SCHEDULED
            && atp->next_scheduler_state == CPU_SCHED_PREEMPTED
        ) {
            rslt = true;
            bool preempt_by_quit = !global_prefs.leave_apps_in_memory;
            preempt_by_quit |= active_tasks.vm_limit_exceeded(vm_limit);

            atp->preempt(preempt_by_quit);
        } else if (atp->scheduler_state != CPU_SCHED_SCHEDULED
            && atp->next_scheduler_state == CPU_SCHED_SCHEDULED
        ) {
            rslt = true;
            int retval = atp->resume_or_start();
            if (retval) {
                report_result_error(
                    *(atp->result), "Couldn't start or resume: %d", retval
                );

                request_schedule_cpus("start failed");
                continue;
            }
            atp->scheduler_state = CPU_SCHED_SCHEDULED;
            atp->episode_start_wall_time = now;
            app_started = gstate.now;
        }
        atp->cpu_time_at_last_sched = atp->current_cpu_time;
    }
    set_client_state_dirty("must_enforce_cpu_schedule");
    return rslt;
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
bool CLIENT_STATE::rr_misses_deadline(
    double per_cpu_proc_rate, double rrs, bool set_shortfall,
    bool set_deadline_misses
) {
    double saved_rrs = rrs;
    PROJECT* p, *pbest;
    RESULT* rp, *rpbest;
    vector<RESULT*> active;
    unsigned int i;
    double x;
    vector<RESULT*>::iterator it;
    bool rval = false;

    SCOPE_MSG_LOG scope_messages(log_messages, CLIENT_MSG_LOG::DEBUG_SCHED_CPU);

    if (set_shortfall) cpu_shortfall = 0;

    // Initilize the "active" and "pending" lists for each project.
    // These keep track of that project's results
    //
    for (i=0; i<projects.size(); i++) {
        p = projects[i];
        p->active.clear();
        p->pending.clear();
        if (set_deadline_misses) p->rr_sim_deadlines_missed = 0;
        if (set_shortfall) p->cpu_shortfall = 0;
    }

    for (i=0; i<results.size(); i++) {
        rp = results[i];
        if (!rp->runnable()) continue;
        if (rp->project->non_cpu_intensive) continue;
        rp->rrsim_cpu_left = rp->estimated_cpu_time_remaining();
        p = rp->project;
        if (p->active.size() < (unsigned int)ncpus) {
            active.push_back(rp);
            p->active.push_back(rp);
        } else {
            p->pending.push_back(rp);
        }
        if (set_deadline_misses) {
            rp->last_rr_sim_missed_deadline = rp->rr_sim_misses_deadline;
            rp->rr_sim_misses_deadline = false;
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
            if (set_deadline_misses) {
                rpbest->rr_sim_misses_deadline = true;
                ++rpbest->project->rr_sim_deadlines_missed;
            }
            rval = true;
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

        int last_active_size = active.size();
        int last_proj_active_size = pbest->active.size();
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

        if (set_shortfall){
            if ((int)active.size() < last_active_size && 
                (int)active.size() < ncpus && 
                sim_now < now + global_prefs.work_buf_min_days * SECONDS_PER_DAY){

                cpu_shortfall += (now + global_prefs.work_buf_min_days * SECONDS_PER_DAY) - sim_now;
            }

            if ((int)p->active.size() < last_proj_active_size &&
                (int)p->active.size() < proj_min_results(pbest, saved_rrs) &&
                sim_now < now + global_prefs.work_buf_min_days * SECONDS_PER_DAY * p->resource_share / saved_rrs) {

                p->cpu_shortfall += now + global_prefs.work_buf_min_days * SECONDS_PER_DAY * p->resource_share / saved_rrs - sim_now;
            }
        }

    }
    if (!rval)
        scope_messages.printf( "rr_sim: deadlines met\n");
    return rval;
}

bool CLIENT_STATE::running_task_sort_pred(
    ACTIVE_TASK * rhs, ACTIVE_TASK * lhs
) {
    // returning true means "less than",
    // the "largest" result is at the front of a heap, 
    // and we want the best replacement at the front,
    // so this is going to look backwards
    //
    if (rhs == NULL) return false;  // null is always the best replacement
    if (lhs == NULL) return true;
    if (rhs->result->project->enforcement_deadlines_missed_scratch && !lhs->result->project->enforcement_deadlines_missed_scratch) return false;
    if (!rhs->result->project->enforcement_deadlines_missed_scratch && lhs->result->project->enforcement_deadlines_missed_scratch) return true;
    if (rhs->result->project->enforcement_deadlines_missed_scratch && lhs->result->project->enforcement_deadlines_missed_scratch) {
        if (rhs->result->report_deadline > lhs->result->report_deadline) return true;
        return false;
    } else {
        double rhs_episode_time = gstate.now - rhs->episode_start_wall_time;
        double lhs_episode_time = gstate.now - lhs->episode_start_wall_time;
        if (rhs_episode_time < lhs_episode_time) return false;
        if (rhs_episode_time > lhs_episode_time) return true;
        if (rhs->result->report_deadline > lhs->result->report_deadline) return true;
        return false;
    }
}
