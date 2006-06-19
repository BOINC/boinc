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

static bool running_task_sort_pred(ACTIVE_TASK* rhs, ACTIVE_TASK* lhs) {
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

        // don't start results if project has > 2 uploads in progress.
        // This avoids creating an unbounded number of completed
        // results for a project that can download and compute
        // faster than it can upload.
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

// return a result for the project with the largest anticipated debt
// among those that have a runnable result.
//
RESULT* CLIENT_STATE::find_largest_debt_project_best_result() {
    PROJECT *best_project = NULL;
    double best_debt = -MAX_DEBT;
    bool first = true;
    unsigned int i;

    // Find the project P with the greatest anticipated debt,
    // select one of P's runnable results
    // (picking one that is already running, if possible,
    // else the result with earliest deadline) and schedule that result. 
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
    if (!best_project) return NULL;

    if (log_flags.cpu_sched_detail) {
        msg_printf(best_project, MSG_INFO,
            "highest debt: %f %s",
            best_project->anticipated_debt,
            best_project->next_runnable_result->name
        );
    }
    RESULT* rp = best_project->next_runnable_result;
    best_project->next_runnable_result = 0;
    return rp;
}

// Schedule the active task with the earliest deadline
// Return true iff a task was scheduled.
//
RESULT* CLIENT_STATE::find_earliest_deadline_result() {
    RESULT *best_result = NULL;
    unsigned int i;

    // Let P be the project with the earliest-deadline runnable result
    // among projects with deadlines_missed(P)>0.
    // Let R be P's earliest-deadline runnable result not scheduled yet.
    // Tiebreaker: least index in result array.

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
    if (!best_result) return NULL;

    if (log_flags.cpu_sched_detail) {
        msg_printf(best_result->project, MSG_INFO,
            "earliest deadline: %f %s",
            best_result->report_deadline, best_result->name
        );
    }

    return best_result;
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
    for (i = 0; i < active_tasks.active_tasks.size(); ++i) {
        ACTIVE_TASK* atp = active_tasks.active_tasks[i];
        atp->cpu_time_at_last_sched = atp->current_cpu_time;
    }
    total_wall_cpu_time_this_period = 0.0;
    total_cpu_time_this_period = 0.0;
}


// Decide whether to run the CPU scheduler.
// This is called periodically.
// Scheduled tasks are placed in order of urgency for scheduling in the ordered_scheduled_results vector
//
bool CLIENT_STATE::possibly_schedule_cpus() {
    double elapsed_time;
    static double last_reschedule=0;

    if (projects.size() == 0) return false;
    if (results.size() == 0) return false;

    // Reschedule every cpu_sched_period seconds,
    // or if must_schedule_cpus is set
    // (meaning a new result is available, or a CPU has been freed).
    //
    elapsed_time = gstate.now - last_reschedule;
    if (elapsed_time >= gstate.global_prefs.cpu_scheduling_period_minutes * 60) {
        request_schedule_cpus("Scheduling period elapsed.");
    }

    if (!must_schedule_cpus) return false;
    last_reschedule = now;
    must_schedule_cpus = false;
    schedule_cpus();
    return true;
}

// CPU scheduler - decide which results to run.
//
void CLIENT_STATE::schedule_cpus() {
    RESULT* rp;
    PROJECT* p;
    double expected_pay_off;
    unsigned int i;
    SCOPE_MSG_LOG scope_messages(log_messages, CLIENT_MSG_LOG::DEBUG_SCHED_CPU);

    // call the rr simulator to calculate what results miss deadline,
    //
    scope_messages.printf("rr_sim: calling from cpu_scheduler");
    rr_simulation(avg_proc_rate()/ncpus, runnable_resource_share());
    if (log_flags.cpu_sched_detail) {
        for (i=0; i<results.size(); i++){
            rp = results[i];
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

    // set temporary variables
    //
    for (i=0; i<results.size(); i++) {
        rp = results[i];
        rp->already_selected = false;
    }
    for (i=0; i<projects.size(); i++) {
        p = projects[i];
        p->next_runnable_result = NULL;
        p->nactive_uploads = 0;
        p->anticipated_debt = p->short_term_debt;
        p->cpu_scheduler_deadlines_missed_scratch = p->rr_sim_deadlines_missed;
    }
    for (i=0; i<file_xfers->file_xfers.size(); i++) {
        FILE_XFER* fxp = file_xfers->file_xfers[i];
        if (fxp->is_upload) {
            fxp->fip->project->nactive_uploads++;
        }
    }

    // calculate the current long and short term debts
    // based on the work done during the last period
    // this function call also sets the CPU efficiency.
    //
    adjust_debts();

    cpu_sched_last_time = gstate.now;

    // note that since this is the anticipated debt,
    // we should be basing this on the anticipated time to crunch,
    // not the time in the last period.
    // The expectation is that each period will be full.
    //
    expected_pay_off = gstate.global_prefs.cpu_scheduling_period_minutes * 60;

    ordered_scheduled_results.clear();

    while ((int)ordered_scheduled_results.size() < ncpus) {
        rp = find_earliest_deadline_result();
        if (!rp) break;
        rp->already_selected = true;  // the result is "used" for this scheduling period.

        // If such an R exists, schedule R, decrement P's anticipated debt,
        // and decrement deadlines_missed(P). 
        //
        rp->project->anticipated_debt -= (1 - rp->project->resource_share / gstate.runnable_resource_share()) * expected_pay_off;
        rp->project->cpu_scheduler_deadlines_missed_scratch--;
        ordered_scheduled_results.push_back(rp);
    }

    // If all CPUs are scheduled, or there are no more results to schedule, stop.
    while ((int)ordered_scheduled_results.size() < ncpus) {
        assign_results_to_projects();
        rp = find_largest_debt_project_best_result();
        if (!rp) break;

        // Decrement P's anticipated debt by the 'expected payoff'
        // (the scheduling period divided by NCPUS). 
        //
        rp->project->anticipated_debt -= (1 - rp->project->resource_share / gstate.runnable_resource_share()) * expected_pay_off;
        ordered_scheduled_results.push_back(rp);
    }

    request_enforce_schedule("");
    set_client_state_dirty("schedule_cpus");
}

// preempt, start, and resume tasks
// This is called in the do_something() loop
// tasks that are to be started or continued are in the
// ordered_scheduled_results vector as set in schedule_cpus.
// This function first builds a heap of running tasks
// ordered by pre-emptability.
// It sets a field to indicate the next state so that a task
// is not unscheduled and re-scheduled in the same call to enforce_schedule.
// This could happen if it was top of the pre-emption list and second in
// scheduled list.
// 
bool CLIENT_STATE::enforce_schedule() {
    unsigned int i;
    if (!must_enforce_cpu_schedule) return false;

    // Don't do this once per second.
    must_enforce_cpu_schedule = false;
    bool action = false;

    if (log_flags.task) {
        msg_printf(0, MSG_INFO, "Enforcing schedule");
    }

    // set temporary variables
    //
    for (i=0; i<projects.size(); i++){
        projects[i]->enforcement_deadlines_missed_scratch = projects[i]->rr_sim_deadlines_missed;
    }
    for (i=0; i< active_tasks.active_tasks.size(); i++) {
        // assume most things don't change.
        active_tasks.active_tasks[i]->next_scheduler_state = active_tasks.active_tasks[i]->scheduler_state;
    }

    // first create a heap of the currently running results ordered by their pre-emptability
    // the first step in this is to add all of the currently running tasks to an array
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
    // add enough NULLs to make the heap size match ncpus.
    // NULL indicates an idle CPU.
    for (i=running_task_heap.size(); (int)i<ncpus; i++) {
        running_task_heap.push_back(NULL);
    }
    // Sort the heap.
    // The policy of which task is most pre-emptable is in running_task_sort_pred.
    std::make_heap(
        running_task_heap.begin(),
        running_task_heap.end(),
        running_task_sort_pred
    );
    // if there are more running tasks than ncpus, then mark enough of them for pre-emption 
    // so that there are only ncpus tasks in the heap.
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

    // Now the heap is set up in order such that the most pre-emptable result is first.
    // Loop through the scheduled results checking the most urgent ones to see if they should 
    // pre-empt something else now.
    for (i=0; i<ordered_scheduled_results.size(); i++) {
        RESULT * rp = ordered_scheduled_results[i];
        ACTIVE_TASK *atp = NULL;
        // See if the task that may pre-empt something is already running.
        for (std::vector<ACTIVE_TASK*>::iterator it = running_task_heap.begin(); it != running_task_heap.end(); it++) {
            ACTIVE_TASK *atp1 = *it;
            if (atp1 && atp1->result == rp) {
                // there is a match.  The task that needs to preempt something now is already running.
                // remove it from the heap
                atp = atp1;
                running_task_heap.erase(it);
                std::make_heap(
                    running_task_heap.begin(),
                    running_task_heap.end(),
                    running_task_sort_pred
                );
                break;
            }
        }
        if (atp) continue;  // the next one to be scheduled is already running.
        // The next task to schedule is not already running.  
        // Check to see if it should pre-empt the head of the running tasks heap.
        if (rp->project->enforcement_deadlines_missed_scratch
            || !running_task_heap[0]
            || (gstate.now - running_task_heap[0]->episode_start_wall_time > gstate.global_prefs.cpu_scheduling_period_minutes && gstate.now - running_task_heap[0]->checkpoint_wall_time < 10 )
        ) {
            // only deadlines_missed results from a project will qualify for immediate enforcement.
            if (rp->project->enforcement_deadlines_missed_scratch) {
                rp->project->enforcement_deadlines_missed_scratch++;
            }
            // we now have some enforcement to do.
            //
            if (running_task_heap[0]) {
                // we have a task to unschedule.
                // NULL means that there is an idle CPU.
                running_task_heap[0]->next_scheduler_state = CPU_SCHED_PREEMPTED;
            }
            // set up the result with a slot
            schedule_result(rp);
            // remove the task from the heap (it only gets unscheduled once).
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

    // execute the decisions made earlier in the function.
    for (i=0; i<active_tasks.active_tasks.size(); i++) {
        atp = active_tasks.active_tasks[i];
        if (atp->scheduler_state == CPU_SCHED_SCHEDULED
            && atp->next_scheduler_state == CPU_SCHED_PREEMPTED
        ) {
            action = true;
            bool preempt_by_quit = !global_prefs.leave_apps_in_memory;
            preempt_by_quit |= active_tasks.vm_limit_exceeded(vm_limit);

            atp->preempt(preempt_by_quit);
        } else if (atp->scheduler_state != CPU_SCHED_SCHEDULED
            && atp->next_scheduler_state == CPU_SCHED_SCHEDULED
        ) {
            action = true;
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
    }
    if (action) {
        set_client_state_dirty("enforce_cpu_schedule");
    }
    return action;
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

// Do a simulation of weighted round-robin scheduling.
//
// Inputs:
// per_cpu_proc_rate:
//   the expected number of CPU seconds per wall second on each CPU
// rrs:
//   the total resource share of runnable projects
//
// Outputs (changes to global state):
// For each project p:
//   p->rr_sim_deadlines_missed
//   p->cpu_shortfall
// For each result r:
//   r->rr_sim_misses_deadline
//   r->last_rr_sim_missed_deadline
// gstate.cpu_shortfall
//
// Returns true if some result misses its deadline
//
bool CLIENT_STATE::rr_simulation(double per_cpu_proc_rate, double rrs) {
    double saved_rrs = rrs;
    PROJECT* p, *pbest;
    RESULT* rp, *rpbest;
    vector<RESULT*> active;
    unsigned int i;
    double x;
    vector<RESULT*>::iterator it;
    bool rval = false;

    SCOPE_MSG_LOG scope_messages(log_messages, CLIENT_MSG_LOG::DEBUG_SCHED_CPU);

    // Initialize the "active" and "pending" lists for each project.
    // These keep track of that project's results
    //
    for (i=0; i<projects.size(); i++) {
        p = projects[i];
        p->active.clear();
        p->pending.clear();
        p->rr_sim_deadlines_missed = 0;
        p->cpu_shortfall = 0;
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
        rp->last_rr_sim_missed_deadline = rp->rr_sim_misses_deadline;
        rp->rr_sim_misses_deadline = false;
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

        pbest = rpbest->project;

        // "rpbest" is first result to finish.  Does it miss its deadline?
        //
        double diff = sim_now + rpbest->rrsim_finish_delay - rpbest->computation_deadline();
        if (diff > 0) {
            scope_messages.printf(
                "rr_sim: result %s misses deadline by %f\n", rpbest->name, diff
            );
            rpbest->rr_sim_misses_deadline = true;
            pbest->rr_sim_deadlines_missed++;
            rval = true;
        }

        int last_active_size = active.size();
        int last_proj_active_size = pbest->active.size();

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


        // increment CPU shortfalls if necessary
        //
        double buf_end = now + global_prefs.work_buf_min_days * SECONDS_PER_DAY;
        if (sim_now < buf_end) {
            double end_time = sim_now + rpbest->rrsim_finish_delay;
            if (end_time > buf_end) end_time = buf_end;
            double dtime = (end_time - sim_now);
            int idle_cpus = ncpus - last_active_size;
            cpu_shortfall += dtime*idle_cpus;

            double proj_cpu_share = ncpus*p->resource_share/saved_rrs;
            if (last_proj_active_size < proj_cpu_share) {
                p->cpu_shortfall += dtime*(proj_cpu_share - last_proj_active_size);
            }
        }

        sim_now += rpbest->rrsim_finish_delay;
    }
    if (!rval) {
        scope_messages.printf( "rr_sim: deadlines met\n");
    }
    return rval;
}

const char *BOINC_RCSID_e830ee1 = "$Id$";
