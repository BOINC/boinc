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

#include <string>
#include <cstring>

#include "str_util.h"
#include "util.h"
#include "error_numbers.h"
#include "coproc.h"

#include "client_msgs.h"
#include "log_flags.h"

#ifdef SIM
#include "sim.h"
#else
#include "client_state.h"
#endif

using std::vector;

#define MAX_STD   (86400)
    // maximum short-term debt

#define DEADLINE_CUSHION    0
    // try to finish jobs this much in advance of their deadline

bool COPROCS::sufficient_coprocs(
    COPROCS& needed, bool log_flag, const char* prefix
) {
    for (unsigned int i=0; i<needed.coprocs.size(); i++) {
        COPROC* cp = needed.coprocs[i];
        COPROC* cp2 = lookup(cp->type);
        if (!cp2) {
            msg_printf(NULL, MSG_INTERNAL_ERROR,
                "Missing a %s coprocessor", cp->type
            );
            return false;
        }
        if (cp2->used + cp->count > cp2->count) {
			if (log_flag) {
				msg_printf(NULL, MSG_INFO,
					"[%s] rr_sim: insufficient coproc %s (%d + %d > %d)",
					prefix, cp2->type, cp2->used, cp->count, cp2->count
				);
			}
            return false;
        }
    }
    return true;
}

void COPROCS::reserve_coprocs(
    COPROCS& needed, bool log_flag, const char* prefix
) {
    for (unsigned int i=0; i<needed.coprocs.size(); i++) {
        COPROC* cp = needed.coprocs[i];
        COPROC* cp2 = lookup(cp->type);
        if (!cp2) {
            msg_printf(NULL, MSG_INTERNAL_ERROR,
                "Coproc type %s not found", cp->type
            );
            continue;
        }
		if (log_flag) {
			msg_printf(NULL, MSG_INFO,
				"[%s] reserving %d of coproc %s", prefix, cp->count, cp2->type
			);
		}
        cp2->used += cp->count;
    }
}

#if 0
void COPROCS::free_coprocs(
    COPROCS& needed, bool log_flag, const char* prefix
) {
    for (unsigned int i=0; i<needed.coprocs.size(); i++) {
        COPROC* cp = needed.coprocs[i];
        COPROC* cp2 = lookup(cp->type);
        if (!cp2) continue;
		if (log_flag) {
			msg_printf(NULL, MSG_INFO,
				"[%s] freeing %d of coproc %s", prefix, cp->count, cp2->type
			);
		}
        cp2->used -= cp->count;
    }
}
#endif

// return true if the task has finished its time slice
// and has checkpointed in last 10 secs
//
static inline bool finished_time_slice(ACTIVE_TASK* atp) {
    double time_running = gstate.now - atp->run_interval_start_wall_time;
    bool running_beyond_sched_period = time_running >= gstate.global_prefs.cpu_scheduling_period();
    double time_since_checkpoint = gstate.now - atp->checkpoint_wall_time;
    bool checkpointed_recently = time_since_checkpoint < 10;
    return (running_beyond_sched_period && checkpointed_recently);
}

// Choose a "best" runnable CPU job for each project
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
        if (rp->uses_coprocs()) continue;
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
        if (rp->uses_coprocs()) continue;
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
RESULT* CLIENT_STATE::earliest_deadline_result(bool coproc_only) {
    RESULT *best_result = NULL;
    ACTIVE_TASK* best_atp = NULL;
    unsigned int i;

    for (i=0; i<results.size(); i++) {
        RESULT* rp = results[i];
        if (!rp->runnable()) continue;
        if (rp->project->non_cpu_intensive) continue;
        if (rp->already_selected) continue;
        if (coproc_only) {
            if (!rp->uses_coprocs()) continue;
        } else {
            if (rp->uses_coprocs()) continue;
            // treat projects with DCF>90 as if they had deadline misses
            if (!rp->project->deadlines_missed && rp->project->duration_correction_factor < 90.0) continue;
        }

        bool new_best = false;
        if (best_result) {
            if (rp->report_deadline < best_result->report_deadline) {
                new_best = true;
            }
        } else {
            new_best = true;
        }
        if (new_best) {
            best_result = rp;
            best_atp = lookup_active_task_by_result(rp);
            continue;
        }
        if (rp->report_deadline > best_result->report_deadline) {
            continue;
        }

        // If there's a tie, pick the job with the least remaining time
        // (but don't pick an unstarted job over one that's started)
        //
        ACTIVE_TASK* atp = lookup_active_task_by_result(rp);
        if (best_atp && !atp) continue;
        if (rp->estimated_time_remaining(false)
            < best_result->estimated_time_remaining(false)
            || (!best_atp && atp)
        ) {
            best_result = rp;
            best_atp = atp;
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
        p->cpu_pwf.reset_debt_accounting();
        if (coproc_cuda) {
            p->cuda_pwf.reset_debt_accounting();
        }
    }
    cpu_work_fetch.reset_debt_accounting();
    if (coproc_cuda) {
        cuda_work_fetch.reset_debt_accounting();
    }
    debt_interval_start = now;
}

// adjust project debts (short, long-term)
//
void CLIENT_STATE::adjust_debts() {
    unsigned int i;
    double total_short_term_debt = 0;
    double rrs;
    int nprojects=0, nrprojects=0;
    PROJECT *p;
    double share_frac;
    double elapsed_time = now - debt_interval_start;

    // If the elapsed time is more than 2*DEBT_ADJUST_PERIOD
    // it must be because the host was suspended for a long time.
    // In this case, ignore the last period
    //
    if (elapsed_time > 2*DEBT_ADJUST_PERIOD || elapsed_time < 0) {
        if (log_flags.debt_debug) {
            msg_printf(NULL, MSG_INFO,
                "[debt_debug] adjust_debt: elapsed time (%d) longer than sched enforce period(%d).  Ignoring this period.",
                (int)elapsed_time, (int)DEBT_ADJUST_PERIOD
            );
        }
        reset_debt_accounting();
        return;
    }

    // skip small intervals
    //
    if (elapsed_time < 1) {
        return;
    }

    for (i=0; i<active_tasks.active_tasks.size(); i++) {
        ACTIVE_TASK* atp = active_tasks.active_tasks[i];
        if (atp->scheduler_state != CPU_SCHED_SCHEDULED) continue;
        p = atp->result->project;
        if (p->non_cpu_intensive) continue;
        work_fetch.accumulate_inst_sec(atp, elapsed_time);
    }

    // adjust long term debts
    cpu_work_fetch.update_debts();
    if (coproc_cuda) {
        cuda_work_fetch.update_debts();
    }

    // adjust short term debts
    rrs = runnable_resource_share();
    for (i=0; i<projects.size(); i++) {
        p = projects[i];
        if (p->non_cpu_intensive) continue;
        nprojects++;

        if (p->runnable()) {
            nrprojects++;
            share_frac = p->resource_share/rrs;
            p->short_term_debt += share_frac*cpu_work_fetch.secs_this_debt_interval
                - p->cpu_pwf.secs_this_debt_interval;
            total_short_term_debt += p->short_term_debt;
        } else {
            p->short_term_debt = 0;
            p->anticipated_debt = 0;
        }
    }

    // short-term debt:
    //  normalize so mean is zero, and limit abs value at MAX_STD
    //
    if (nrprojects) {
        double avg_short_term_debt = total_short_term_debt / nrprojects;
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
        }
    }

    reset_debt_accounting();
}


// Decide whether to run the CPU scheduler.
// This is called periodically.
// Scheduled tasks are placed in order of urgency for scheduling
// in the ordered_scheduled_results vector
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

struct PROC_RESOURCES {
    int ncpus;
    double ncpus_used;
    double ram_left;
    COPROCS coprocs;

    // should we stop scanning jobs?
    //
    inline bool stop_scan_cpu() {
		return ncpus_used >= ncpus;
    }

    inline bool stop_scan_coproc() {
        return coprocs.fully_used();
    }

    // should we consider scheduling this job?
    //
    bool can_schedule(RESULT* rp) {
        if (rp->uses_coprocs()) {
            if (gstate.user_active && !gstate.global_prefs.run_gpu_if_user_active) {
                return false;
            }
            if (coprocs.sufficient_coprocs(
                rp->avp->coprocs, log_flags.cpu_sched_debug, "cpu_sched_debug")
            ) {
                return true;
            } else {
                if (log_flags.cpu_sched_debug) {
                    msg_printf(rp->project, MSG_INFO,
                        "[cpu_sched_debug] insufficient coprocessors for %s", rp->name
                    );
                }
                return false;
            }
        } else {
            // otherwise, only if CPUs are available
            //
            return (ncpus_used < ncpus);
        }
    }

    // we've decided to run this - update bookkeeping
    //
    void schedule(RESULT* rp) {
        coprocs.reserve_coprocs(
            rp->avp->coprocs, log_flags.cpu_sched_debug, "cpu_sched_debug"
        );
        ncpus_used += rp->avp->avg_ncpus;
    }
};

// Check whether the job can be run:
// - it will fit in RAM
// - we have enough shared-mem segments (old Mac problem)
// If so, update proc_rsc accordingly and return true
//
static bool schedule_if_possible(
    RESULT* rp, ACTIVE_TASK* atp, PROC_RESOURCES& proc_rsc, double rrs, double expected_payoff
) {
    if (atp) {
        // see if it fits in available RAM
        //
        if (atp->procinfo.working_set_size_smoothed > proc_rsc.ram_left) {
            if (log_flags.cpu_sched_debug) {
                msg_printf(rp->project, MSG_INFO,
                    "[cpu_sched_debug]  %s working set too large: %.2fMB",
                    rp->name, atp->procinfo.working_set_size_smoothed/MEGA
                );
            }
            atp->too_large = true;
            return false;
        }
        atp->too_large = false;
        
        if (gstate.retry_shmem_time > gstate.now) {
            if (atp->app_client_shm.shm == NULL) {
                if (log_flags.cpu_sched_debug) {
                    msg_printf(rp->project, MSG_INFO,
                        "[cpu_sched_debug] waiting for shared mem: %s",
                        rp->name
                    );
                }
                atp->needs_shmem = true;
                return false;
            }
            atp->needs_shmem = false;
        }
        proc_rsc.ram_left -= atp->procinfo.working_set_size_smoothed;
    } else {
        if (rp->avp->max_working_set_size > proc_rsc.ram_left) {
            if (log_flags.cpu_sched_debug) {
                msg_printf(rp->project, MSG_INFO,
                    "[cpu_sched_debug]  %s projected working set too large: %.2fMB",
                    rp->name, rp->avp->max_working_set_size/MEGA
                );
            }
            return false;
        }
    }
    if (log_flags.cpu_sched_debug) {
        msg_printf(rp->project, MSG_INFO,
            "[cpu_sched_debug] scheduling %s", rp->name
        );
    }
	proc_rsc.schedule(rp);
    rp->project->anticipated_debt -= (rp->project->resource_share / rrs) * expected_payoff;
    return true;
}

// CPU scheduler - decide which results to run.
// output: sets ordered_scheduled_result.
//
void CLIENT_STATE::schedule_cpus() {
    RESULT* rp;
    PROJECT* p;
    double expected_payoff;
    unsigned int i;
    double rrs = runnable_resource_share();
    PROC_RESOURCES proc_rsc;
    ACTIVE_TASK* atp;

    proc_rsc.ncpus = ncpus;
    proc_rsc.ncpus_used = 0;
    proc_rsc.ram_left = available_ram();
    proc_rsc.coprocs.clone(coprocs, false);

    if (log_flags.cpu_sched_debug) {
        msg_printf(0, MSG_INFO, "[cpu_sched_debug] schedule_cpus(): start");
    }

    // do round-robin simulation to find what results miss deadline
    //
    rr_simulation();
    if (log_flags.cpu_sched_debug) {
        print_deadline_misses();
    }

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
        p->deadlines_missed = p->rr_sim_status.deadlines_missed;
    }
    for (i=0; i<app_versions.size(); i++) {
        app_versions[i]->max_working_set_size = 0;
    }
    for (i=0; i<active_tasks.active_tasks.size(); i++) {
        atp = active_tasks.active_tasks[i];
        atp->too_large = false;
        double w = atp->procinfo.working_set_size_smoothed;
        APP_VERSION* avp = atp->app_version;
        if (w > avp->max_working_set_size) {
            avp->max_working_set_size = w;
        }
    }

    expected_payoff = global_prefs.cpu_scheduling_period();
    ordered_scheduled_results.clear();

    // choose coproc jobs in deadline order until all coprocs are full
    //
    while (!proc_rsc.stop_scan_coproc()) {
        rp = earliest_deadline_result(true);
        if (!rp) break;
        rp->already_selected = true;
        if (!proc_rsc.can_schedule(rp)) continue;
		atp = lookup_active_task_by_result(rp);
        if (!schedule_if_possible(rp, atp, proc_rsc, rrs, expected_payoff)) continue;
        ordered_scheduled_results.push_back(rp);
    }

    // choose CPU jobs from projects with P.deadlines_missed>0
    //
#ifdef SIM
    if (!cpu_sched_rr_only) {
#endif
    while (!proc_rsc.stop_scan_cpu()) {
        rp = earliest_deadline_result(false);
        if (!rp) break;
        rp->already_selected = true;
        if (!proc_rsc.can_schedule(rp)) continue;
		atp = lookup_active_task_by_result(rp);
        if (!schedule_if_possible(rp, atp, proc_rsc, rrs, expected_payoff)) continue;
        rp->project->deadlines_missed--;
        rp->edf_scheduled = true;
        ordered_scheduled_results.push_back(rp);
    }
#ifdef SIM
    }
#endif

    // Next, choose CPU jobs from projects with large debt
    //
    while (!proc_rsc.stop_scan_cpu()) {
        assign_results_to_projects();
        rp = largest_debt_project_best_result();
        if (!rp) break;
		atp = lookup_active_task_by_result(rp);
        if (!proc_rsc.can_schedule(rp)) continue;
        if (!schedule_if_possible(rp, atp, proc_rsc, rrs, expected_payoff)) continue;
        ordered_scheduled_results.push_back(rp);
    }

    request_enforce_schedule("schedule_cpus");
}

static inline bool in_ordered_scheduled_results(ACTIVE_TASK* atp) {
	for (unsigned int i=0; i<gstate.ordered_scheduled_results.size(); i++) {
		if (atp->result == gstate.ordered_scheduled_results[i]) return true;
	}
	return false;
}

// return true if t1 is more preemptable than t0
//
static inline bool more_preemptable(ACTIVE_TASK* t0, ACTIVE_TASK* t1) {
    if (t0->result->project->deadlines_missed && !t1->result->project->deadlines_missed) return true;
    if (!t0->result->project->deadlines_missed && t1->result->project->deadlines_missed) return false;
    if (t0->result->project->deadlines_missed && t1->result->project->deadlines_missed) {
        if (t0->result->report_deadline < t1->result->report_deadline) return true;
        if (t0->result->report_deadline > t1->result->report_deadline) return false;
        return (t0 < t1);
    } else {
		bool fin0 = finished_time_slice(t0);
		bool fin1 = finished_time_slice(t1);
		if (fin1 && !fin0) return true;
		if (fin0 && !fin1) return false;
        if (t0->result->report_deadline < t1->result->report_deadline) return true;
        if (t0->result->report_deadline > t1->result->report_deadline) return false;
        return (t0 < t1);
    }
}

// Make a list of preemptable CPU jobs, in increasing order of preemptability,
// and mark them as scheduled.
// "Preemptable" means: running, not non-CPU-intensive,
// not in the scheduled results list.
//
void CLIENT_STATE::make_preemptable_task_list(
	vector<ACTIVE_TASK*> &preemptable_tasks, double& ncpus_used
) {
    unsigned int i;
    ACTIVE_TASK* atp;

	ncpus_used = 0;
    for (i=0; i<active_tasks.active_tasks.size(); i++) {
        atp = active_tasks.active_tasks[i];
		if (in_ordered_scheduled_results(atp)) continue;
		if (!atp->result->runnable()) continue;
        if (atp->result->project->non_cpu_intensive) continue;
        if (atp->scheduler_state != CPU_SCHED_SCHEDULED) continue;
        if (atp->result->uses_coprocs()) continue;
		ncpus_used += atp->app_version->avg_ncpus;
		atp->next_scheduler_state = CPU_SCHED_SCHEDULED;
        preemptable_tasks.push_back(atp);
#if 0
		msg_printf(0, MSG_INFO, "%s: misses %d deadline %f finished %d ptr %x",
			atp->result->name,
			atp->result->project->deadlines_missed,
			atp->result->report_deadline,
			finished_time_slice(atp), atp
		);
#endif
    }

    std::sort(
        preemptable_tasks.begin(),
        preemptable_tasks.end(),
        more_preemptable
    );
#if 0
	for (i=0; i<preemptable_tasks.size(); i++) {
		atp = preemptable_tasks[i];
		msg_printf(0, MSG_INFO, "list %d: %s", i, atp->result->name);
	}
#endif
}

// Enforce the CPU schedule.
// Inputs:
//   ordered_scheduled_results
//      List of tasks that should (ideally) run, set by schedule_cpus().
//      Most important tasks (e.g. early deadline) are first.
// The set of tasks that actually run may be different:
// - if a task hasn't checkpointed recently we avoid preempting it
// - we don't run tasks that would exceed working-set limits
// Details:
//   Initially, each task's scheduler_state is PREEMPTED or SCHEDULED
//     depending on whether or not it is running.
//     This function sets each task's next_scheduler_state,
//     and at the end it starts/resumes and preempts tasks
//     based on scheduler_state and next_scheduler_state.
// 
bool CLIENT_STATE::enforce_schedule() {
    unsigned int i;
    ACTIVE_TASK* atp, *preempt_atp;
    vector<ACTIVE_TASK*> preemptable_tasks;
    static double last_time = 0;
    int retval;
    double ncpus_used;

    // Do this when requested, and once a minute as a safety net
    //
    if (now - last_time > CPU_SCHED_ENFORCE_PERIOD) {
        must_enforce_cpu_schedule = true;
    }
    if (!must_enforce_cpu_schedule) return false;
    must_enforce_cpu_schedule = false;

    // NOTE: there's an assumption that debt is adjusted at
    // least as often as the CPU sched is enforced.
    // If you remove the following, make changes accordingly
    //
    adjust_debts();
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
        projects[i]->deadlines_missed = projects[i]->rr_sim_status.deadlines_missed;
    }

    // Set next_scheduler_state to preempt
    //
    for (i=0; i< active_tasks.active_tasks.size(); i++) {
        atp = active_tasks.active_tasks[i];
        atp->next_scheduler_state = CPU_SCHED_PREEMPTED;
    }

    make_preemptable_task_list(preemptable_tasks, ncpus_used);

    double ram_left = available_ram();

    if (log_flags.mem_usage_debug) {
        msg_printf(0, MSG_INFO,
            "[mem_usage_debug] enforce: available RAM %.2fMB",
            ram_left/MEGA
        );
    }

    // schedule all non CPU intensive tasks
    //
    for (i=0; i<results.size(); i++) {
        RESULT* rp = results[i];
        if (rp->project->non_cpu_intensive && rp->runnable()) {
            atp = get_task(rp);
            atp->next_scheduler_state = CPU_SCHED_SCHEDULED;
            ram_left -= atp->procinfo.working_set_size_smoothed;
        }
    }

    double swap_left = (global_prefs.vm_max_used_frac)*host_info.m_swap;

    // Loop through the jobs we want to schedule.
    // Invariant: "ncpus_used" is the sum of CPU usage
    // of tasks with next_scheduler_state == CPU_SCHED_SCHEDULED
	// (including preemptable jobs).
	// Win: "new_ncpus_used" is the sum excluding preemptable jobs.
    //
    for (i=0; i<ordered_scheduled_results.size(); i++) {
        RESULT* rp = ordered_scheduled_results[i];
        if (log_flags.cpu_sched_debug) {
            msg_printf(rp->project, MSG_INFO,
                "[cpu_sched_debug] processing %s", rp->name
            );
        }

        atp = lookup_active_task_by_result(rp);
        if (atp) {
            atp->too_large = false;
            if (atp->procinfo.working_set_size_smoothed > ram_left) {
                atp->too_large = true;
                if (log_flags.mem_usage_debug) {
                    msg_printf(rp->project, MSG_INFO,
                        "[mem_usage_debug] enforce: result %s can't run, too big %.2fMB > %.2fMB",
                        rp->name,  atp->procinfo.working_set_size_smoothed/MEGA, ram_left/MEGA
                    );
                }
                continue;
            }
        }

        // Preempt tasks if needed (and possible).
        //
        bool failed_to_preempt = false;
		while (1) {
			if (!preemptable_tasks.size()) break;
            if (ncpus_used < ncpus) break;
            // Preempt the most preemptable task if either
            // 1) it's completed its time slice and has checkpointed recently
            // 2) the scheduled result is in deadline trouble
            //
            preempt_atp = preemptable_tasks.back();
            if (rp->project->deadlines_missed || finished_time_slice(preempt_atp)) {
                if (rp->project->deadlines_missed) {
                    rp->project->deadlines_missed--;
                }
                preempt_atp->next_scheduler_state = CPU_SCHED_PREEMPTED;
                ncpus_used -= preempt_atp->app_version->avg_ncpus;
                preemptable_tasks.pop_back();
                if (log_flags.cpu_sched_debug) {
                    msg_printf(rp->project, MSG_INFO,
                        "[cpu_sched_debug] preempting %s",
                        preempt_atp->result->name
                    );
                }
            } else {
                if (log_flags.cpu_sched_debug) {
                    msg_printf(rp->project, MSG_INFO,
                        "[cpu_sched_debug] didn't preempt %s: tr %f tsc %f",
                        preempt_atp->result->name,
						now - preempt_atp->run_interval_start_wall_time,
						now - preempt_atp->checkpoint_wall_time
                    );
                }
                failed_to_preempt = true;
                break;
            }
        }

        if (failed_to_preempt && !rp->uses_coprocs()) {
            continue;
        }

        // We've decided to run this job; create an ACTIVE_TASK if needed.
        //
        if (!atp) {
			atp = get_task(rp);
        }
        ncpus_used += rp->avp->avg_ncpus;
        atp->next_scheduler_state = CPU_SCHED_SCHEDULED;
        ram_left -= atp->procinfo.working_set_size_smoothed;
    }
    if (log_flags.cpu_sched_debug) {
        msg_printf(0, MSG_INFO,
            "[cpu_sched_debug] finished preempt loop, ncpus_used %f",
            ncpus_used
        );
    }

    // There may be jobs still in the preemptable list at this point.
    // Let them run if they don't exceed RAM limits
    //
    for (i=0; i<preemptable_tasks.size(); i++) {
        atp = preemptable_tasks[i];
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

    if (log_flags.cpu_sched_debug && ncpus_used < ncpus) {
        msg_printf(0, MSG_INFO, "[cpu_sched_debug] using %f out of %d CPUs",
            ncpus_used, ncpus
        );
        if (ncpus_used < ncpus) {
            request_work_fetch("CPUs idle");
        }
    }

    bool check_swap = (host_info.m_swap != 0);
        // in case couldn't measure swap on this host

    // preempt tasks as needed, and note whether there are any coproc jobs
    // in QUIT_PENDING state
    //
    bool coproc_quit_pending = false;
    for (i=0; i<active_tasks.active_tasks.size(); i++) {
        atp = active_tasks.active_tasks[i];
        if (log_flags.cpu_sched_debug) {
            msg_printf(atp->result->project, MSG_INFO,
                "[cpu_sched_debug] %s sched state %d next %d task state %d",
                atp->result->name, atp->scheduler_state,
                atp->next_scheduler_state, atp->task_state()
            );
        }
        int preempt_type = REMOVE_MAYBE_SCHED;
        switch (atp->next_scheduler_state) {
        case CPU_SCHED_PREEMPTED:
            switch (atp->task_state()) {
            case PROCESS_EXECUTING:
                action = true;
                if (check_swap && swap_left < 0) {
                    if (log_flags.mem_usage_debug) {
                        msg_printf(atp->result->project, MSG_INFO,
                            "[mem_usage_debug] out of swap space, will preempt by quit"
                        );
                    }
                    preempt_type = REMOVE_ALWAYS;
                }
                if (atp->too_large) {
                    if (log_flags.mem_usage_debug) {
                        msg_printf(atp->result->project, MSG_INFO,
                            "[mem_usage_debug] job using too much memory, will preempt by quit"
                        );
                    }
                    preempt_type = REMOVE_ALWAYS;
                }
                atp->preempt(preempt_type);
                break;
            case PROCESS_SUSPENDED:
                // Handle the case where user changes prefs from
                // "leave in memory" to "remove from memory";
                // need to quit suspended tasks.
                //
				if (atp->checkpoint_cpu_time && !global_prefs.leave_apps_in_memory) {
                    atp->preempt(REMOVE_ALWAYS);
                }
                break;
            }
            atp->scheduler_state = CPU_SCHED_PREEMPTED;
            break;
        }
        if (atp->result->uses_coprocs() && atp->task_state() == PROCESS_QUIT_PENDING) {
            coproc_quit_pending = true;
        }
    }

    bool coproc_start_deferred = false;
    for (i=0; i<active_tasks.active_tasks.size(); i++) {
        atp = active_tasks.active_tasks[i];
        switch (atp->next_scheduler_state) {
        atp = active_tasks.active_tasks[i];
        case CPU_SCHED_SCHEDULED:
            switch (atp->task_state()) {
            case PROCESS_UNINITIALIZED:
            case PROCESS_SUSPENDED:
                // If there's a quit pending for a coproc job,
                // don't start new ones since they may bomb out
                // on memory allocation.  Instead, trigger a retry
                //
                if (atp->result->uses_coprocs() && coproc_quit_pending) {
                    coproc_start_deferred = true;
                    break;
                }
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
    if (coproc_start_deferred) {
        if (log_flags.cpu_sched_debug) {
            msg_printf(0, MSG_INFO,
                "[cpu_sched_debug] coproc quit pending, deferring start"
            );
        }
        request_enforce_schedule("coproc quit retry");
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

bool RESULT::not_started() {
    if (computing_done()) return false;
    if (gstate.lookup_active_task_by_result(this)) return false;
    return true;
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
    case RESULT_ABORTED: return "ABORTED";
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

    if (config.ncpus>=0) {
        ncpus = config.ncpus;
    } else if (host_info.p_ncpus>0) {
        ncpus = host_info.p_ncpus;
    } else {
        ncpus = 1;
    }

    // if config says no CPUs, honor it
    //
    if (ncpus) {
        if (global_prefs.max_ncpus_pct) {
            ncpus = (int)((ncpus * global_prefs.max_ncpus_pct)/100);
            if (ncpus == 0) ncpus = 1;
        } else if (global_prefs.max_ncpus && global_prefs.max_ncpus < ncpus) {
            ncpus = global_prefs.max_ncpus;
        }
    }

    if (initialized && ncpus != ncpus_old) {
        msg_printf(0, MSG_INFO,
            "Number of usable CPUs has changed from %d to %d.  Running benchmarks.",
            ncpus_old, ncpus
        );
        run_cpu_benchmarks = true;
        request_schedule_cpus("Number of usable CPUs has changed");
        request_work_fetch("Number of usable CPUs has changed");
        work_fetch.init();
    }
}

// The given result has just completed successfully.
// Update the correction factor used to predict
// completion time for this project's results
//
void PROJECT::update_duration_correction_factor(ACTIVE_TASK* atp) {
	RESULT* rp = atp->result;
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
    double raw_ratio = atp->elapsed_time/rp->estimated_duration_uncorrected();
    double adj_ratio = atp->elapsed_time/rp->estimated_duration(false);
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

    if (log_flags.dcf_debug) {
        msg_printf(this, MSG_INFO,
            "[dcf] DCF: %f->%f, raw_ratio %f, adj_ratio %f",
            old_dcf, duration_correction_factor, raw_ratio, adj_ratio
        );
    }
}

const char *BOINC_RCSID_e830ee1 = "$Id$";
