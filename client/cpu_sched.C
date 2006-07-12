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

static bool more_preemptable(ACTIVE_TASK* t0, ACTIVE_TASK* t1) {
    // returning true means t1 is more preemptable than t0,
    // the "largest" result is at the front of a heap, 
    // and we want the best replacement at the front,
    //
    if (t0->result->project->deadlines_missed && !t1->result->project->deadlines_missed) return false;
    if (!t0->result->project->deadlines_missed && t1->result->project->deadlines_missed) return true;
    if (t0->result->project->deadlines_missed && t1->result->project->deadlines_missed) {
        if (t0->result->report_deadline > t1->result->report_deadline) return true;
        return false;
    } else {
        double t0_episode_time = gstate.now - t0->episode_start_wall_time;
        double t1_episode_time = gstate.now - t1->episode_start_wall_time;
        if (t0_episode_time < t1_episode_time) return false;
        if (t0_episode_time > t1_episode_time) return true;
        if (t0->result->report_deadline > t1->result->report_deadline) return true;
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

// Among projects with a "next runnable result",
// find the project P with the greatest anticipated debt,
// and return its next runnable result
//
RESULT* CLIENT_STATE::largest_debt_project_best_result() {
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
    if (!best_project) return NULL;

    if (log_flags.cpu_sched_debug) {
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

// Return earliest-deadline result from a project with deadlines_missed>0
//
RESULT* CLIENT_STATE::earliest_deadline_result() {
    RESULT *best_result = NULL;
    unsigned int i;

    for (i=0; i<results.size(); i++) {
        RESULT* rp = results[i];
        if (!rp->runnable()) continue;
        if (rp->project->non_cpu_intensive) continue;
        if (rp->already_selected) continue;
        if (!rp->project->deadlines_missed) continue;
        if (!best_result || rp->report_deadline < best_result->report_deadline) {
            best_result = rp;
        }
    }
    if (!best_result) return NULL;

    if (log_flags.cpu_sched_debug) {
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
        if (atp->wup->project->non_cpu_intensive) continue;

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
        }

        p->long_term_debt -= avg_long_term_debt;
        if (log_flags.debt_debug) {
            msg_printf(0, MSG_INFO,
                "adjust_debts(): project %s: STD %f, LTD %f",
                p->project_name, p->short_term_debt, p->long_term_debt
            );
        }
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

void CLIENT_STATE::print_deadline_misses() {
    unsigned int i;
    RESULT* rp;
    PROJECT* p;
    for (i=0; i<results.size(); i++){
        rp = results[i];
        if (rp->rr_sim_misses_deadline && !rp->last_rr_sim_missed_deadline) {
            msg_printf(rp->project, MSG_INFO,
                "Result %s projected to miss deadline.", rp->name
            );
        }
        else if (!rp->rr_sim_misses_deadline && rp->last_rr_sim_missed_deadline) {
            msg_printf(rp->project, MSG_INFO,
                "Result %s projected to meet deadline.", rp->name
            );
        }
    }
    for (i=0; i<projects.size(); i++) {
        p = projects[i];
        if (p->rr_sim_deadlines_missed) {
            msg_printf(p, MSG_INFO,
                "Project has %d projected deadline misses",
                p->rr_sim_deadlines_missed
            );
        }
    }
}

// CPU scheduler - decide which results to run.
// output: sets ordered_scheduled_result.
//
void CLIENT_STATE::schedule_cpus() {
    RESULT* rp;
    PROJECT* p;
    double expected_pay_off;
    unsigned int i;
    double rrs = runnable_resource_share();

    // do round-robin simulation to find what results miss deadline,
    //
    if (log_flags.cpu_sched_debug) {
        msg_printf(0, MSG_INFO,
            "rr_simulation: calling from cpu_scheduler"
        );
    }
    rr_simulation(avg_proc_rate()/ncpus, runnable_resource_share());
    if (log_flags.cpu_sched_debug) {
        print_deadline_misses();
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
        p->deadlines_missed = p->rr_sim_deadlines_missed;
    }
    for (i=0; i<file_xfers->file_xfers.size(); i++) {
        FILE_XFER* fxp = file_xfers->file_xfers[i];
        if (fxp->is_upload) {
            fxp->fip->project->nactive_uploads++;
        }
    }

    // adjust long and short term debts
    // based on the work done during the last period
    //
    adjust_debts();

    cpu_sched_last_time = gstate.now;
    expected_pay_off = gstate.global_prefs.cpu_scheduling_period_minutes * 60;
    ordered_scheduled_results.clear();

    // First choose results from projects with P.deadlines_missed>0
    //
    while ((int)ordered_scheduled_results.size() < ncpus) {
        rp = earliest_deadline_result();
        if (!rp) break;

        // If such an R exists, decrement P.deadlines_missed 
        //
        rp->already_selected = true;
        rp->project->anticipated_debt -= (1 - rp->project->resource_share / rrs) * expected_pay_off;
        rp->project->deadlines_missed--;
        ordered_scheduled_results.push_back(rp);
    }

    // Next, choose results from projects with large debt
    //
    while ((int)ordered_scheduled_results.size() < ncpus) {
        assign_results_to_projects();
        rp = largest_debt_project_best_result();
        if (!rp) break;
        rp->project->anticipated_debt -= (1 - rp->project->resource_share / rrs) * expected_pay_off;
        ordered_scheduled_results.push_back(rp);
    }

    request_enforce_schedule("schedule_cpus");
    set_client_state_dirty("schedule_cpus");
}

// make a list of preemptable tasks, ordered by their preemptability.
//
void CLIENT_STATE::make_running_task_heap(
    vector<ACTIVE_TASK*> &running_tasks
) {
    unsigned int i;
    ACTIVE_TASK* atp;

    for (i=0; i<active_tasks.active_tasks.size(); i++) {
        atp = active_tasks.active_tasks[i];
        if (atp->result->project->non_cpu_intensive) continue;
        if (atp->next_scheduler_state == CPU_SCHED_PREEMPTED) continue;
        if (atp->scheduler_state != CPU_SCHED_SCHEDULED) continue;
        running_tasks.push_back(atp);
    }

    std::make_heap(
        running_tasks.begin(),
        running_tasks.end(),
        more_preemptable
    );
}

// Enforce the CPU schedule.
// Inputs:
//   ordered_scheduled_results
//      List of tasks that should (ideally) run, set by schedule_cpus().
//      Most important tasks (e.g. early deadline) are first
// Method:
//   Make a list "running_tasks" of currently running tasks
//   Most preemptable tasks are first in list.
// Details:
//   Initially, each task's scheduler_state is PREEMPTED or SCHEDULED
//     depending on whether or not it is running.
//     This function sets each task's next_scheduler_state,
//     and at the end it starts/resumes and preempts tasks
//     based on scheduler_state and next_scheduler_state.
// 
bool CLIENT_STATE::enforce_schedule() {
    unsigned int i;
    ACTIVE_TASK* atp;
    vector<ACTIVE_TASK*> running_tasks;

    // Don't do this once per second.
    //
    if (!must_enforce_cpu_schedule) return false;
    must_enforce_cpu_schedule = false;
    bool action = false;

    if (log_flags.cpu_sched) {
        msg_printf(0, MSG_INFO, "Enforcing schedule");
    }

    // set temporary variables
    //
    for (i=0; i<projects.size(); i++){
        projects[i]->deadlines_missed = projects[i]->rr_sim_deadlines_missed;
    }
    for (i=0; i< active_tasks.active_tasks.size(); i++) {
        atp = active_tasks.active_tasks[i];
        if (atp->result->runnable()) {
            atp->next_scheduler_state = atp->scheduler_state;
        } else {
            atp->next_scheduler_state = CPU_SCHED_PREEMPTED;
        }
    }

    // make list of preemptable tasks
    //
    make_running_task_heap(running_tasks);

    // if there are more running tasks than ncpus,
    // then mark the extras for preemption 
    //
    while (running_tasks.size() > (unsigned int)ncpus) {
        running_tasks[0]->next_scheduler_state = CPU_SCHED_PREEMPTED;
        std::pop_heap(
            running_tasks.begin(),
            running_tasks.end(),
            more_preemptable
        );
        running_tasks.pop_back();
    }

    // count of how many tasks have next_scheduler_state = SCHEDULED
    //
    int nrunning = running_tasks.size();

    // Loop through the scheduled results
    // to see if they should preempt something
    //
    for (i=0; i<ordered_scheduled_results.size(); i++) {
        RESULT* rp = ordered_scheduled_results[i];

        // See if the result is already running.
        //
        atp = NULL;
        for (vector<ACTIVE_TASK*>::iterator it = running_tasks.begin(); it != running_tasks.end(); it++) {
            ACTIVE_TASK *atp1 = *it;
            if (atp1 && atp1->result == rp) {
                // The task is already running; remove it from the heap
                //
                atp = atp1;
                running_tasks.erase(it);
                std::make_heap(
                    running_tasks.begin(),
                    running_tasks.end(),
                    more_preemptable
                );
                break;
            }
        }
        if (atp) continue;  // the scheduled result is already running.

        // The scheduled result is not already running.  
        // Preempt something if needed and possible.
        //
        bool run_task = false;
        bool need_to_preempt = (nrunning==ncpus) && running_tasks.size();
            // the 2nd half of the above is redundant
        if (need_to_preempt) {
            atp = running_tasks[0];
            bool running_beyond_sched_period =
                gstate.now - atp->episode_start_wall_time
                > gstate.global_prefs.cpu_scheduling_period_minutes*60;
            bool checkpointed_recently =
                atp->checkpoint_wall_time > atp->episode_start_wall_time;
            if (rp->project->deadlines_missed
                || (running_beyond_sched_period && checkpointed_recently)
            ) {
                // only deadlines_missed results from a project
                // will qualify for immediate enforcement.
                //
                if (rp->project->deadlines_missed) {
                    rp->project->deadlines_missed--;
                }
                atp->next_scheduler_state = CPU_SCHED_PREEMPTED;
                nrunning--;
                std::pop_heap(
                    running_tasks.begin(),
                    running_tasks.end(),
                    more_preemptable
                );
                running_tasks.pop_back();
                run_task = true;
            }
        } else {
            run_task = true;
        }
        if (run_task) {
            schedule_result(rp);
            nrunning++;
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

    // preempt and start tasks as needed
    //
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
        if (!rp->nearly_runnable()) continue;
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

    if (log_flags.rr_simulation) {
        msg_printf(0, MSG_INFO,
            "rr_simulation: start"
        );
    }

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
        if (!rp->nearly_runnable()) continue;
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
    cpu_shortfall = 0;
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
            if (log_flags.rr_simulation) {
                msg_printf(0, MSG_INFO,
                    "rr_simulation: result %s misses deadline by %f\n",
                    rpbest->name, diff
                );
            }
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
            int nidle_cpus = ncpus - last_active_size;
            if (nidle_cpus) cpu_shortfall += dtime*nidle_cpus;

            double proj_cpu_share = ncpus*pbest->resource_share/saved_rrs;
            if (last_proj_active_size < proj_cpu_share) {
                pbest->cpu_shortfall += dtime*(proj_cpu_share - last_proj_active_size);
            }
        }

        sim_now += rpbest->rrsim_finish_delay;
    }
    if (log_flags.rr_simulation) {
        for (i=0; i<projects.size(); i++) {
            p = projects[i];
            if (p->cpu_shortfall) {
                msg_printf(NULL, MSG_INFO,
                    "rr_simulation: shortfall for %s is %f\n",
                    p->project_name, p->cpu_shortfall
                );
            }
        }
        msg_printf(NULL, MSG_INFO,
            "rr_simulation: end; returning %s; cpu_shortfall %f\n",
            rval?"true":"false",
            cpu_shortfall
        );
    }
    return rval;
}

const char *BOINC_RCSID_e830ee1 = "$Id$";
