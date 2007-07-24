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

// CPU scheduling logic.
//
// Terminology:
//
// Episode
// The execution of a task is divided into "episodes".
// An episode starts then the application is executed,
// and ends when it exits or dies
// (e.g., because it's preempted and not left in memory,
// or the user quits BOINC, or the host is turned off).
// A task may checkpoint now and then.
// Each episode begins with the state of the last checkpoint.
//
// Debt interval
// The interval between consecutive executions of adjust_debts()
//
// Run interval
// If an app is running (not suspended), the interval
// during which it's been running.

#ifdef _WIN32
#include "boinc_win.h"
#endif

#ifdef SIM
#include "sim.h"
#else
#include "client_state.h"
#endif
#include "client_msgs.h"
#include "str_util.h"
#include "util.h"
#include "error_numbers.h"
#include "log_flags.h"

using std::vector;

#define MAX_STD   (86400)
    // maximum short-term debt

#define CPU_PESSIMISM_FACTOR 0.9
    // assume actual CPU utilization will be this multiple
    // of what we've actually measured recently

#define DEADLINE_CUSHION    0
    // try to finish jobs this much in advance of their deadline

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
        double t0_episode_time = gstate.now - t0->run_interval_start_wall_time;
        double t1_episode_time = gstate.now - t1->run_interval_start_wall_time;
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
// TODO: this is called in a loop over NCPUs, which is silly. 
// Should call it once, and have it make an ordered list per project.
//
void CLIENT_STATE::assign_results_to_projects() {
    unsigned int i;
    RESULT* rp;
    PROJECT* project;

    // scan results with an ACTIVE_TASK
    //
    for (i=0; i<active_tasks.active_tasks.size(); i++) {
        ACTIVE_TASK *atp = active_tasks.active_tasks[i];
        if (!atp->runnable()) continue;
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

        if ((next_atp->task_state() == PROCESS_UNINITIALIZED && atp->process_exists())
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

// Among projects with a "next runnable result",
// find the project P with the greatest anticipated debt,
// and return its next runnable result
//
RESULT* CLIENT_STATE::largest_debt_project_best_result() {
    PROJECT *best_project = NULL;
    double best_debt = -MAX_STD;
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
            "[cpu_sched_debug] highest debt: %f %s",
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

        // Pick the earliest deadline result.
        // If there is a tie, pick the one with the least remaining CPU time.
        //
        if (!best_result
            || rp->report_deadline < best_result->report_deadline
            || (rp->report_deadline == best_result->report_deadline
                && rp->estimated_cpu_time_remaining(false) < best_result->estimated_cpu_time_remaining(false)
                )
        ) {
            best_result = rp;
        }
    }
    if (!best_result) return NULL;

    if (log_flags.cpu_sched_debug) {
        msg_printf(best_result->project, MSG_INFO,
            "[cpu_sched_debug] earliest deadline: %f %s",
            best_result->report_deadline, best_result->name
        );
    }

    return best_result;
}

void CLIENT_STATE::reset_debt_accounting() {
    unsigned int i;
    for (i=0; i<projects.size(); i++) {
        PROJECT* p = projects[i];
        p->wall_cpu_time_this_debt_interval = 0.0;
    }
    for (i = 0; i < active_tasks.active_tasks.size(); ++i) {
        ACTIVE_TASK* atp = active_tasks.active_tasks[i];
        atp->debt_interval_start_cpu_time = atp->current_cpu_time;
    }
    total_wall_cpu_time_this_debt_interval = 0.0;
    total_cpu_time_this_debt_interval = 0.0;
    debt_interval_start = now;
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
    double wall_cpu_time = now - debt_interval_start;

    if (wall_cpu_time < 1) {
        return;
    }

    // if the elapsed time is more than the scheduling period,
    // it must be because the host was suspended for a long time.
    // Currently we don't have a way to estimate how long this was for,
    // so ignore the last period and reset counters.
    //
    if (wall_cpu_time > global_prefs.cpu_scheduling_period()*2) {
        if (log_flags.debt_debug) {
            msg_printf(NULL, MSG_INFO,
                "[debt_debug] adjust_debt: elapsed time (%d) longer than sched period (%d).  Ignoring this period.",
                (int)wall_cpu_time, (int)global_prefs.cpu_scheduling_period()
            );
        }
        reset_debt_accounting();
        return;
    }

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

        atp->result->project->wall_cpu_time_this_debt_interval += wall_cpu_time;
        total_wall_cpu_time_this_debt_interval += wall_cpu_time;
        total_cpu_time_this_debt_interval += atp->current_cpu_time - atp->debt_interval_start_cpu_time;
    }

    time_stats.update_cpu_efficiency(
        total_wall_cpu_time_this_debt_interval, total_cpu_time_this_debt_interval
    );

    rrs = runnable_resource_share();
    prrs = potentially_runnable_resource_share();

    for (i=0; i<projects.size(); i++) {
        p = projects[i];

        // potentially_runnable() can be false right after a result completes,
        // but we still need to update its LTD.
        // In this case its wall_cpu_time_this_debt_interval will be nonzero.
        //
        if (!(p->potentially_runnable()) && p->wall_cpu_time_this_debt_interval) {
            prrs += p->resource_share;
        }
    }

    for (i=0; i<projects.size(); i++) {
        p = projects[i];
        if (p->non_cpu_intensive) continue;
        nprojects++;

        // adjust long-term debts
        //
        if (p->potentially_runnable() || p->wall_cpu_time_this_debt_interval) {
            share_frac = p->resource_share/prrs;
            p->long_term_debt += share_frac*total_wall_cpu_time_this_debt_interval
                - p->wall_cpu_time_this_debt_interval;
        }
        total_long_term_debt += p->long_term_debt;

        // adjust short term debts
        //
        if (p->runnable()) {
            nrprojects++;
            share_frac = p->resource_share/rrs;
            p->short_term_debt += share_frac*total_wall_cpu_time_this_debt_interval
                - p->wall_cpu_time_this_debt_interval;
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
    //  normalize so mean is zero, and limit abs value at MAX_STD
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
            if (p->short_term_debt > MAX_STD) {
                p->short_term_debt = MAX_STD;
            }
            if (p->short_term_debt < -MAX_STD) {
                p->short_term_debt = -MAX_STD;
            }
        }

        p->long_term_debt -= avg_long_term_debt;
        if (log_flags.debt_debug) {
            msg_printf(0, MSG_INFO,
                "[debt_debug] adjust_debts(): project %s: STD %f, LTD %f",
                p->project_name, p->short_term_debt, p->long_term_debt
            );
        }
    }

    reset_debt_accounting();
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
    elapsed_time = now - last_reschedule;
    if (elapsed_time >= global_prefs.cpu_scheduling_period()) {
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
                "[cpu_sched_debug] Result %s projected to miss deadline.", rp->name
            );
        }
        else if (!rp->rr_sim_misses_deadline && rp->last_rr_sim_missed_deadline) {
            msg_printf(rp->project, MSG_INFO,
                "[cpu_sched_debug] Result %s projected to meet deadline.", rp->name
            );
        }
    }
    for (i=0; i<projects.size(); i++) {
        p = projects[i];
        if (p->rr_sim_deadlines_missed) {
            msg_printf(p, MSG_INFO,
                "[cpu_sched_debug] Project has %d projected deadline misses",
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
	ACTIVE_TASK* atp;
    double expected_pay_off;
    unsigned int i;
    double rrs = runnable_resource_share();

    if (log_flags.cpu_sched_debug) {
		msg_printf(0, MSG_INFO, "[cpu_sched_debug] schedule_cpus(): start");
    }

	// do round-robin simulation to find what results miss deadline,
    //
    rr_simulation();
    if (log_flags.cpu_sched_debug) {
        print_deadline_misses();
    }

    adjust_debts();

    // set temporary variables
    //
    for (i=0; i<results.size(); i++) {
        rp = results[i];
        rp->already_selected = false;
        rp->edf_scheduled = false;
    }
    for (i=0; i<projects.size(); i++) {
        p = projects[i];
        p->next_runnable_result = NULL;
        p->anticipated_debt = p->short_term_debt;
        p->deadlines_missed = p->rr_sim_deadlines_missed;
    }
	for (i=0; i<active_tasks.active_tasks.size(); i++) {
		active_tasks.active_tasks[i]->too_large = false;
	}

    expected_pay_off = global_prefs.cpu_scheduling_period();
    ordered_scheduled_results.clear();
	double ram_left = available_ram();

    // First choose results from projects with P.deadlines_missed>0
    //
#ifdef SIM
    if (!cpu_sched_rr_only) {
#endif
    while ((int)ordered_scheduled_results.size() < ncpus) {
        rp = earliest_deadline_result();
        if (!rp) break;
        rp->already_selected = true;

		// see if it fits in available RAM
		//
		atp = lookup_active_task_by_result(rp);
		if (atp) {
			if (atp->procinfo.working_set_size_smoothed > ram_left) {
				if (log_flags.cpu_sched_debug) {
					msg_printf(rp->project, MSG_INFO,
						"[cpu_sched_debug]  %s misses deadline but too large: %.2fMB",
						rp->name, atp->procinfo.working_set_size_smoothed/MEGA
					);
				}
                atp->too_large = true;
				continue;
			}
            atp->too_large = false;
            
            // TODO: merge this chunk of code with its clone
            if (gstate.retry_shmem_time > gstate.now) {
                if (atp->app_client_shm.shm == NULL) {
                    atp->needs_shmem = true;
                    continue;
                }
                atp->needs_shmem = false;
            }
			ram_left -= atp->procinfo.working_set_size_smoothed;
		}

        rp->project->anticipated_debt -= (rp->project->resource_share / rrs) * expected_pay_off;
        rp->project->deadlines_missed--;
        rp->edf_scheduled = true;
        if (log_flags.cpu_sched_debug) {
            msg_printf(rp->project, MSG_INFO,
				"[cpu_sched_debug] scheduling (deadline) %s",
				rp->name
			);
        }
        ordered_scheduled_results.push_back(rp);
    }
#ifdef SIM
    }
#endif

    // Next, choose results from projects with large debt
    //
    while ((int)ordered_scheduled_results.size() < ncpus) {
        assign_results_to_projects();
        rp = largest_debt_project_best_result();
        if (!rp) break;
		atp = lookup_active_task_by_result(rp);
		if (atp) {
			if (atp->procinfo.working_set_size_smoothed > ram_left) {
				if (log_flags.cpu_sched_debug) {
					msg_printf(NULL, MSG_INFO,
						"[cpu_sched_debug]  %s too large: %.2fMB",
						rp->name, atp->procinfo.working_set_size_smoothed/MEGA
					);
				}
                atp->too_large = true;
				continue;
			}
            atp->too_large = false;

            // don't select if it would need a new shared-mem seg
            // and we're out of them
            //
            if (gstate.retry_shmem_time > gstate.now) {
                if (atp->app_client_shm.shm == NULL) {
                    atp->needs_shmem = true;
                    continue;
                }
                atp->needs_shmem = false;
            }
			ram_left -= atp->procinfo.working_set_size_smoothed;
		}
        double xx = (rp->project->resource_share / rrs) * expected_pay_off;
        rp->project->anticipated_debt -= xx;
        if (log_flags.cpu_sched_debug) {
            msg_printf(NULL, MSG_INFO, "[cpu_sched_debug] scheduling (regular) %s", rp->name);
        }
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
        if (!atp->result->runnable()) continue;
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
	static double last_time = 0;
    int retval;

    // Do this when requested, and once a minute as a safety net
    //
	if (now - last_time > 60) {
		must_enforce_cpu_schedule = true;
	}
    if (!must_enforce_cpu_schedule) return false;
    must_enforce_cpu_schedule = false;
	last_time = now;
    bool action = false;

    if (log_flags.cpu_sched_debug) {
		msg_printf(0, MSG_INFO, "[cpu_sched_debug] enforce_schedule(): start");
        for (i=0; i<ordered_scheduled_results.size(); i++) {
            RESULT* rp = ordered_scheduled_results[i];
            msg_printf(rp->project, MSG_INFO,
                "[cpu_sched_debug] want to run: %s",
                rp->name
            );
        }
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

    // make list of currently running tasks
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

	double ram_left = available_ram();

    if (log_flags.mem_usage_debug) {
        msg_printf(0, MSG_INFO,
            "[mem_usage_debug] enforce: available RAM %.2fMB",
            ram_left/MEGA
        );
    }

    // keep track of how many tasks we plan on running
    // (i.e. have next_scheduler_state = SCHEDULED)
    //
    int nrunning = (int)running_tasks.size();

    // Loop through the scheduled results
    // to see if they should preempt a running task
    //
    for (i=0; i<ordered_scheduled_results.size(); i++) {
        RESULT* rp = ordered_scheduled_results[i];
        if (log_flags.cpu_sched_debug) {
            msg_printf(rp->project, MSG_INFO,
                "[cpu_sched_debug] processing %s", rp->name
            );
        }

        // See if the result is already running.
        //
        atp = NULL;
        for (vector<ACTIVE_TASK*>::iterator it = running_tasks.begin(); it != running_tasks.end(); it++) {
            ACTIVE_TASK *atp1 = *it;
            if (atp1 && atp1->result == rp) {
                // The task is already running; remove it from the heap
                //
                atp = atp1;
                it = running_tasks.erase(it);
                std::make_heap(
                    running_tasks.begin(),
                    running_tasks.end(),
                    more_preemptable
                );
                break;
            }
        }
        if (atp) {
            if (log_flags.cpu_sched_debug) {
                msg_printf(rp->project, MSG_INFO,
                    "[cpu_sched_debug] %s is already running", rp->name
                );
            }

            // the scheduled result is already running.
            // see if it fits in mem
            //
            if (atp->procinfo.working_set_size_smoothed > ram_left) {
                atp->next_scheduler_state = CPU_SCHED_PREEMPTED;
                atp->too_large = true;
                nrunning--;
                if (log_flags.mem_usage_debug) {
                    msg_printf(rp->project, MSG_INFO,
                        "[mem_usage_debug] enforce: result %s can't continue, too big %.2fMB > %.2fMB",
                        rp->name,  atp->procinfo.working_set_size_smoothed/MEGA, ram_left/MEGA
                    );
                }
            } else {
                ram_left -= atp->procinfo.working_set_size_smoothed;
                atp->too_large = false;
            }
            continue;
        }

        // Here if the result is not already running.
        // If it already has a (non-running) active task,
        // see if it fits in mem
        //
        atp = lookup_active_task_by_result(rp);
        if (atp) {
            if (atp->procinfo.working_set_size_smoothed > ram_left) {
                atp->too_large = true;
                if (log_flags.mem_usage_debug) {
                    msg_printf(rp->project, MSG_INFO,
                        "[mem_usage_debug] enforce: result %s can't start, too big %.2fMB > %.2fMB",
                        rp->name, atp->procinfo.working_set_size_smoothed/MEGA, ram_left/MEGA
                    );
                }
                continue;
            } else {
                atp->too_large = false;
            }
        }

        // The scheduled result is not already running.  
        // Preempt something if needed and possible.
        //
        bool run_task = false;
        bool need_to_preempt = (nrunning==ncpus) && running_tasks.size();
            // the 2nd half of the above is redundant
        if (need_to_preempt) {
            // examine the most preemptable task.
            // Preempt it if either
            // 1) it's completed its time slice and has checkpointed recently
            // 2) the scheduled result is in deadline trouble
            //
            atp = running_tasks[0];
            double time_running = now - atp->run_interval_start_wall_time;
            bool running_beyond_sched_period = time_running >= global_prefs.cpu_scheduling_period();
            double time_since_checkpoint = now - atp->checkpoint_wall_time;
            bool checkpointed_recently = time_since_checkpoint < 10;
            if (rp->project->deadlines_missed
                || (running_beyond_sched_period && checkpointed_recently)
            ) {
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
                if (log_flags.cpu_sched_debug) {
                    msg_printf(rp->project, MSG_INFO,
                        "[cpu_sched_debug] preempting %s",
                        atp->result->name
                    );
                }
            } else {
                if (log_flags.cpu_sched_debug) {
                    msg_printf(rp->project, MSG_INFO,
                        "[cpu_sched_debug] didn't preempt %s: tr %f tsc %f",
                        atp->result->name, time_running, time_since_checkpoint
                    );
                }
            }
        } else {
            run_task = true;
        }
        if (run_task) {
            atp = get_task(rp);
            atp->next_scheduler_state = CPU_SCHED_SCHEDULED;
            nrunning++;
            ram_left -= atp->procinfo.working_set_size_smoothed;
        }
    }
    if (log_flags.cpu_sched_debug) {
        msg_printf(0, MSG_INFO,
            "[cpu_sched_debug] finished preempt loop, nrunning %d",
            nrunning
        );
    }

    // make sure we don't exceed RAM limits
    //
    for (i=0; i<running_tasks.size(); i++) {
        atp = running_tasks[i];
        if (atp->procinfo.working_set_size_smoothed > ram_left) {
            atp->next_scheduler_state = CPU_SCHED_PREEMPTED;
            atp->too_large = true;
            if (log_flags.mem_usage_debug) {
                msg_printf(atp->result->project, MSG_INFO,
                    "[mem_usage_debug] enforce: result %s can't keep, too big %.2fMB > %.2fMB",
                    atp->result->name, atp->procinfo.working_set_size_smoothed/MEGA, ram_left/MEGA
                );
            }
        } else {
            atp->too_large = false;
            ram_left -= atp->procinfo.working_set_size_smoothed;
        }
    }

    if (log_flags.cpu_sched_debug && nrunning < ncpus) {
        msg_printf(0, MSG_INFO, "[cpu_sched_debug] Some CPUs idle (%d<%d)",
            nrunning, ncpus
        );
		request_work_fetch("CPUs idle");
    }
    if (log_flags.cpu_sched_debug && nrunning > ncpus) {
        msg_printf(0, MSG_INFO, "[cpu_sched_debug] Too many tasks started (%d>%d)",
            nrunning, ncpus
        );
    }

    // schedule new non CPU intensive tasks
    //
    for (i=0; i<results.size(); i++) {
        RESULT* rp = results[i];
        if (rp->project->non_cpu_intensive && rp->runnable()) {
            atp = get_task(rp);
            atp->next_scheduler_state = CPU_SCHED_SCHEDULED;
        }
    }

    double swap_left = (global_prefs.vm_max_used_frac)*host_info.m_swap;
    bool check_swap = (host_info.m_swap != 0);
        // in case couldn't measure swap on this host

    // preempt and start tasks as needed
    //
    for (i=0; i<active_tasks.active_tasks.size(); i++) {
        atp = active_tasks.active_tasks[i];
        if (log_flags.cpu_sched_debug) {
            msg_printf(atp->result->project, MSG_INFO,
                "[cpu_sched_debug] %s sched state %d next %d task state %d",
                atp->result->name, atp->scheduler_state,
                atp->next_scheduler_state, atp->task_state()
            );
        }
        switch (atp->next_scheduler_state) {
        case CPU_SCHED_PREEMPTED:
            switch (atp->task_state()) {
            case PROCESS_EXECUTING:
                action = true;
                bool preempt_by_quit = !global_prefs.leave_apps_in_memory;
                if (check_swap && swap_left < 0) {
                    if (log_flags.mem_usage_debug) {
                        msg_printf(atp->result->project, MSG_INFO,
                            "[mem_usage_debug] out of swap space, will preempt by quit"
                        );
                    }
                    preempt_by_quit = true;
                }
                if (atp->too_large) {
                    if (log_flags.mem_usage_debug) {
                        msg_printf(atp->result->project, MSG_INFO,
                            "[mem_usage_debug] job using too much memory, will preempt by quit"
                        );
                    }
                    preempt_by_quit = true;
                }
                atp->preempt(preempt_by_quit);
                break;
            }
            atp->scheduler_state = CPU_SCHED_PREEMPTED;
            break;
        case CPU_SCHED_SCHEDULED:
            switch (atp->task_state()) {
            case PROCESS_UNINITIALIZED:
            case PROCESS_SUSPENDED:
                action = true;
                retval = atp->resume_or_start(
                    atp->scheduler_state == CPU_SCHED_UNINITIALIZED
                );
                if ((retval == ERR_SHMGET) || (retval == ERR_SHMAT)) {
                    // Assume no additional shared memory segs
                    // will be available in the next 10 seconds
                    // (run only tasks which are already attached to shared memory).
                    //
                    if (gstate.retry_shmem_time < gstate.now) {
                        request_schedule_cpus("no more shared memory");
                    }
                    gstate.retry_shmem_time = gstate.now + 10.0;
                    continue;
                }
                if (retval) {
                    report_result_error(
                        *(atp->result), "Couldn't start or resume: %d", retval
                    );
                    request_schedule_cpus("start failed");
                    continue;
                }
                atp->run_interval_start_wall_time = now;
                app_started = now;
            }
            atp->scheduler_state = CPU_SCHED_SCHEDULED;
            swap_left -= atp->procinfo.swap_size;
            break;
        }
    }
    if (action) {
        set_client_state_dirty("enforce_cpu_schedule");
    }
    if (log_flags.cpu_sched_debug) {
        msg_printf(0, MSG_INFO, "[cpu_sched_debug] enforce_schedule: end");
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
void PROJECT::set_rrsim_proc_rate(double rrs) {
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
	rrsim_proc_rate = x*gstate.overall_cpu_frac();
    if (log_flags.rr_simulation) {
        msg_printf(this, MSG_INFO,
            "[rr_sim] set_rrsim_proc_rate: %f (rrs %f, rs %f, nactive %d, ocf %f",
            rrsim_proc_rate, rrs, resource_share, nactive, gstate.overall_cpu_frac()
        );
    }
}

// Do a simulation of weighted round-robin scheduling.
//
// Inputs:
// per_cpu_proc_rate:
//   the expected number of CPU seconds per wall second on each CPU
// rrs:
//   the total resource share of relevant projects
//   (runnable when called from CPU sched,
//   nearly runnable when called from work fetch)
//   NOTE: this may be zero, e.g. if no projects have results
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
// NOTE: deadline misses are not counted for tasks
// that are too large to run in RAM right now.
//
void CLIENT_STATE::rr_simulation() {
	double rrs = nearly_runnable_resource_share();
    double trs = total_resource_share();
    PROJECT* p, *pbest;
    RESULT* rp, *rpbest;
    vector<RESULT*> active;
    unsigned int i;
    double x;
    vector<RESULT*>::iterator it;

	double ar = available_ram();

	if (log_flags.rr_simulation) {
		msg_printf(0, MSG_INFO,
            "[rr_sim] rr_sim start: work_buf_total %f rrs %f trs %f ncpus %d",
            work_buf_total(), rrs, trs, ncpus
        );
	}

    // Initialize result lists for each project:
	// "active" is what's currently running (in the simulation)
	// "pending" is what's queued
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
        if (rp->some_download_stalled()) continue;
        if (rp->project->non_cpu_intensive) continue;
        rp->rrsim_cpu_left = rp->estimated_cpu_time_remaining(false);
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
        p->set_rrsim_proc_rate(rrs);
        // if there are no results for a project,
        // the shortfall is its entire share.
        //
        if (!p->active.size()) {
			double rsf = trs ? p->resource_share/trs : 1;
            p->cpu_shortfall = work_buf_total() * overall_cpu_frac() * ncpus * rsf;
            if (log_flags.rr_simulation) {
                msg_printf(p, MSG_INFO,
                    "[rr_sim] no results; shortfall %f wbt %f ocf %f rsf %f",
                    p->cpu_shortfall, work_buf_total(), overall_cpu_frac(), rsf
                );
            }
        }
    }

    double buf_end = now + work_buf_total();

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

        if (log_flags.rr_simulation) {
            msg_printf(pbest, MSG_INFO,
                "[rr_sim] result %s finishes after %f (%f/%f)",
				rpbest->name, rpbest->rrsim_finish_delay, rpbest->rrsim_cpu_left, pbest->rrsim_proc_rate
            );
        }

        // "rpbest" is first result to finish.  Does it miss its deadline?
        //
        double diff = sim_now + rpbest->rrsim_finish_delay - ((rpbest->computation_deadline()-now)*CPU_PESSIMISM_FACTOR + now);
        if (diff > 0) {
			ACTIVE_TASK* atp = lookup_active_task_by_result(rpbest);
			if (atp && atp->procinfo.working_set_size_smoothed > ar) {
				if (log_flags.rr_simulation) {
					msg_printf(pbest, MSG_INFO,
						"[rr_sim] result %s misses deadline but too large to run",
						rpbest->name
					);
				}
			} else {
				rpbest->rr_sim_misses_deadline = true;
				pbest->rr_sim_deadlines_missed++;
				if (log_flags.rr_simulation) {
					msg_printf(pbest, MSG_INFO,
						"[rr_sim] result %s misses deadline by %f",
						rpbest->name, diff
					);
				}
			}
        }

        int last_active_size = (int)active.size();
        int last_proj_active_size = (int)pbest->active.size();

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
            if (log_flags.rr_simulation) {
                msg_printf(pbest, MSG_INFO,
                    "[rr_sim] decr rrs by %f, new value %f",
                    pbest->resource_share, rrs
                );
            }
            for (i=0; i<projects.size(); i++) {
                p = projects[i];
                p->set_rrsim_proc_rate(rrs);
            }
        }

        // increment CPU shortfalls if necessary
        //
        if (sim_now < buf_end) {
            double end_time = sim_now + rpbest->rrsim_finish_delay;
            if (end_time > buf_end) end_time = buf_end;
            double d_time = end_time - sim_now;
            int nidle_cpus = ncpus - last_active_size;
            if (nidle_cpus<0) nidle_cpus = 0;
            if (nidle_cpus > 0) cpu_shortfall += d_time*nidle_cpus;

			double rsf = trs?pbest->resource_share/trs:1;
            double proj_cpu_share = ncpus*rsf;

            if (last_proj_active_size < proj_cpu_share) {
                pbest->cpu_shortfall += d_time*(proj_cpu_share - last_proj_active_size);
				if (log_flags.rr_simulation) {
					msg_printf(pbest, MSG_INFO,
						"[rr_sim] new shortfall %f d_time %f proj_cpu_share %f lpas %d",
						pbest->cpu_shortfall, d_time, proj_cpu_share, last_proj_active_size
					);
				}
            }

			if (end_time < buf_end) {
                d_time = buf_end - end_time;
                // if this is the last result for this project, account for the tail
                if (!pbest->active.size()) { 
                    pbest->cpu_shortfall += d_time * proj_cpu_share;
					if (log_flags.rr_simulation) {
						 msg_printf(pbest, MSG_INFO, "[rr_sim] proj out of work; shortfall %f d %f pcs %f",
							 pbest->cpu_shortfall, d_time, proj_cpu_share
					     );
					}
                }
            }
            if (log_flags.rr_simulation) {
                msg_printf(0, MSG_INFO,
                    "[rr_sim] total: idle cpus %d, last active %d, active %d, shortfall %f",
                    nidle_cpus, last_active_size, (int)active.size(), cpu_shortfall
					
				);
				msg_printf(0, MSG_INFO,
					"[rr_sim] proj %s: last active %d, active %d, shortfall %f",
					pbest->get_project_name(), last_proj_active_size, (int)pbest->active.size(),
					pbest->cpu_shortfall
				);
            }
        }

        sim_now += rpbest->rrsim_finish_delay;
    }

    if (sim_now < buf_end) {
        cpu_shortfall += (buf_end - sim_now) * ncpus;
    }

    if (log_flags.rr_simulation) {
        for (i=0; i<projects.size(); i++) {
            p = projects[i];
            if (p->cpu_shortfall) {
                msg_printf(p, MSG_INFO,
                    "[rr_sim] shortfall %f\n", p->cpu_shortfall
                );
            }
        }
        msg_printf(NULL, MSG_INFO,
            "[rr_sim] done; total shortfall %f\n",
            cpu_shortfall
        );
    }
}

// trigger CPU schedule enforcement.
// Called when a new schedule is computed,
// and when an app checkpoints.
//
void CLIENT_STATE::request_enforce_schedule(const char* where) {
    if (log_flags.cpu_sched_debug) {
        msg_printf(0, MSG_INFO, "[cpu_sched_debug] Request enforce CPU schedule: %s", where);
    }
    must_enforce_cpu_schedule = true;
}

// trigger CPU scheduling.
// Called when a result is completed, 
// when new results become runnable, 
// or when the user performs a UI interaction
// (e.g. suspending or resuming a project or result).
//
void CLIENT_STATE::request_schedule_cpus(const char* where) {
    if (log_flags.cpu_sched_debug) {
        msg_printf(0, MSG_INFO, "[cpu_sched_debug] Request CPU reschedule: %s", where);
    }
    must_schedule_cpus = true;
}

// Find the active task for a given result
//
ACTIVE_TASK* CLIENT_STATE::lookup_active_task_by_result(RESULT* rep) {
    for (unsigned int i = 0; i < active_tasks.active_tasks.size(); i ++) {
        if (active_tasks.active_tasks[i]->result == rep) {
            return active_tasks.active_tasks[i];
        }
    }
    return NULL;
}

bool RESULT::computing_done() {
    return (state() >= RESULT_COMPUTE_ERROR || ready_to_report);
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
        if (p->non_cpu_intensive) continue;
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
        if (p->non_cpu_intensive) continue;
        if (p->potentially_runnable()) {
            x += p->resource_share;
        }
    }
    return x;
}

double CLIENT_STATE::fetchable_resource_share() {
    double x = 0;
    for (unsigned int i=0; i<projects.size(); i++) {
        PROJECT* p = projects[i];
        if (p->non_cpu_intensive) continue;
        if (p->long_term_debt < -global_prefs.cpu_scheduling_period()) continue;
        if (p->contactable()) {
            x += p->resource_share;
        }
    }
    return x;
}

// same, but nearly runnable (could be downloading work right now)
//
double CLIENT_STATE::nearly_runnable_resource_share() {
    double x = 0;
    for (unsigned int i=0; i<projects.size(); i++) {
        PROJECT* p = projects[i];
        if (p->non_cpu_intensive) continue;
        if (p->nearly_runnable()) {
            x += p->resource_share;
        }
    }
    return x;
}

bool ACTIVE_TASK::process_exists() {
    switch (task_state()) {
    case PROCESS_EXECUTING:
    case PROCESS_SUSPENDED:
    case PROCESS_ABORT_PENDING:
    case PROCESS_QUIT_PENDING:
        return true;
    }
    return false;
}

// if there's not an active task for the result, make one
//
ACTIVE_TASK* CLIENT_STATE::get_task(RESULT* rp) {
    ACTIVE_TASK *atp = lookup_active_task_by_result(rp);
    if (!atp) {
        atp = new ACTIVE_TASK;
        atp->slot = active_tasks.get_free_slot();
        atp->init(rp);
        active_tasks.active_tasks.push_back(atp);
    }
    return atp;
}

// Results must be complete early enough to report before the report deadline.
// Not all hosts are connected all of the time.
//
double RESULT::computation_deadline() {
    return report_deadline - (
        gstate.work_buf_min()
            // Seconds that the host will not be connected to the Internet
        + gstate.global_prefs.cpu_scheduling_period()
            // Seconds that the CPU may be busy with some other result
        + DEADLINE_CUSHION
    );
}

static const char* result_state_name(int val) {
    switch (val) {
    case RESULT_NEW: return "NEW";
    case RESULT_FILES_DOWNLOADING: return "FILES_DOWNLOADING";
    case RESULT_FILES_DOWNLOADED: return "FILES_DOWNLOADED";
    case RESULT_COMPUTE_ERROR: return "COMPUTE_ERROR";
    case RESULT_FILES_UPLOADING: return "FILES_UPLOADING";
    case RESULT_FILES_UPLOADED: return "FILES_UPLOADED";
    case RESULT_ABORTED: return "FILES_ABORTED";
    }
    return "Unknown";
}

void RESULT::set_state(int val, const char* where) {
    _state = val;
    if (log_flags.task_debug) {
        msg_printf(project, MSG_INFO,
            "[task_debug] result state=%s for %s from %s",
            result_state_name(val), name, where
        );
    }
}

// called at startup (after get_host_info())
// and when general prefs have been parsed
//
void CLIENT_STATE::set_ncpus() {
    int ncpus_old = ncpus;

    if (config.ncpus>0) {
        ncpus = config.ncpus;
    } else if (host_info.p_ncpus>0) {
        ncpus = host_info.p_ncpus;
    } else {
        ncpus = 1;
    }
    if (ncpus > global_prefs.max_cpus) ncpus = global_prefs.max_cpus;

    if (initialized && ncpus != ncpus_old) {
        msg_printf(0, MSG_INFO,
            "Number of usable CPUs has changed from %d to %d.  Running benchmarks.",
            ncpus_old, ncpus
        );
        run_cpu_benchmarks = true;
        request_schedule_cpus("Number of usable CPUs has changed");
        request_work_fetch("Number of usable CPUs has changed");
    }
}

// preempt this task
// called from the CLIENT_STATE::schedule_cpus()
// if quit_task is true do this by quitting
//
int ACTIVE_TASK::preempt(bool quit_task) {
    int retval;

    // If the app hasn't checkpoint yet, suspend instead of quit
    // (accommodate apps that never checkpoint)
    //
    if (quit_task && (checkpoint_cpu_time>0)) {
        if (log_flags.cpu_sched) {
            msg_printf(result->project, MSG_INFO,
                "[cpu_sched] Preempting %s (removed from memory)",
                result->name
            );
        }
        set_task_state(PROCESS_QUIT_PENDING, "preempt");
        retval = request_exit();
    } else {
        if (log_flags.cpu_sched) {
			if (quit_task) {
				msg_printf(result->project, MSG_INFO,
					"[cpu_sched] Preempting %s (left in memory because no checkpoint yet)",
					result->name
				);
			} else {
				msg_printf(result->project, MSG_INFO,
					"[cpu_sched] Preempting %s (left in memory)",
					result->name
				);
			}
		}
        retval = suspend();
    }
    return 0;
}

// The given result has just completed successfully.
// Update the correction factor used to predict
// completion time for this project's results
//
void PROJECT::update_duration_correction_factor(RESULT* rp) {
#ifdef SIM
	if (dcf_dont_use) {
		duration_correction_factor = 1.0;
		return;
	}
	if (dcf_stats) {
		((SIM_PROJECT*)this)->update_dcf_stats(rp);
        return;
	}
#endif
    double raw_ratio = rp->final_cpu_time/rp->estimated_cpu_time_uncorrected();
    double adj_ratio = rp->final_cpu_time/rp->estimated_cpu_time(false);
	double old_dcf = duration_correction_factor;

    // it's OK to overestimate completion time,
    // but bad to underestimate it.
    // So make it easy for the factor to increase,
    // but decrease it with caution
    //
    if (adj_ratio > 1.1) {
        duration_correction_factor = raw_ratio;
    } else {
        // in particular, don't give much weight to results
        // that completed a lot earlier than expected
        //
        if (adj_ratio < 0.1) {
            duration_correction_factor = duration_correction_factor*0.99 + 0.01*raw_ratio;
        } else {
            duration_correction_factor = duration_correction_factor*0.9 + 0.1*raw_ratio;
        }
    }
    // limit to [.01 .. 100]
    //
    if (duration_correction_factor > 100) duration_correction_factor = 100;
    if (duration_correction_factor < 0.01) duration_correction_factor = 0.01;

	if (log_flags.cpu_sched_debug || log_flags.work_fetch_debug) {
		msg_printf(this, MSG_INFO,
            "[csd|wfd] DCF: %f->%f, raw_ratio %f, adj_ratio %f",
			old_dcf, duration_correction_factor, raw_ratio, adj_ratio
		);
	}
}

const char *BOINC_RCSID_e830ee1 = "$Id$";
