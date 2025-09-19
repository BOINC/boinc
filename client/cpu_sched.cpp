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
//  - create an ordered "run list" (make_run_list()).
//      The ordering is roughly as follows:
//          - GPU jobs first, then CPU jobs
//          - for a given resource, jobs in deadline danger first
//          - jobs from projects with lower recent est. credit first
//      In principle, the run list could include all runnable jobs.
//      For efficiency, we stop adding:
//          - GPU jobs: when all GPU instances used
//          - CPU jobs: when the # of CPUs allocated to single-thread jobs,
//              OR the # allocated to multi-thread jobs, exceeds # CPUs
//              (ensure we have enough single-thread jobs
//              in case we can't run the multi-thread jobs)
//      NOTE: RAM usage is not taken into consideration
//          in the process of building this list.
//          It's possible that we include a bunch of jobs that can't run
//          because of memory limits,
//          even though there are other jobs that could run.
//      - add running jobs to the list
//          (in case they haven't finished time slice or checkpointed)
//      - sort the list according to "more_important()"
//      - shuffle the list to avoid starving multi-thread jobs
//
//  - scan through the resulting list, running the jobs and preempting
//      other jobs (enforce_run_list).
//      Don't run a job if
//      - its GPUs can't be assigned (possible if need >1 GPU)
//      - it's a multi-thread job, and CPU usage would be #CPUs+1 or more
//      - it's a single-thread job, don't oversaturate CPU
//          (details depend on whether a MT job is running)
//      - its memory usage would exceed RAM limits
//          If there's a running job using a given app version,
//          unstarted jobs using that app version
//          are assumed to have the same working set size.

#include "cpp.h"

#ifdef _WIN32
#include "boinc_win.h"
#include "sysmon_win.h"
#else
#include "config.h"
#include <string>
#include <cstring>
#include <list>
#endif


#include "coproc.h"
#include "error_numbers.h"
#include "filesys.h"
#include "str_util.h"
#include "util.h"

#include "app.h"
#include "app_config.h"
#include "client_msgs.h"
#include "client_state.h"
#include "coproc_sched.h"
#include "log_flags.h"
#include "project.h"
#include "result.h"


using std::vector;
using std::list;

static double rec_sum;

// used in make_run_list() to keep track of resources used
// by jobs tentatively scheduled so far
//
struct PROC_RESOURCES {
    int ncpus;
    double ncpus_used_st;   // #CPUs of GPU or single-thread jobs
    double ncpus_used_mt;   // #CPUs of multi-thread jobs
    COPROCS pr_coprocs;

    void init() {
        ncpus = gstate.n_usable_cpus;
        ncpus_used_st = 0;
        ncpus_used_mt = 0;
        pr_coprocs.clone(coprocs, false);
        pr_coprocs.clear_usage();
    }

    // should we stop scanning jobs?
    //
    inline bool stop_scan_cpu() {
        if (ncpus_used_st >= ncpus) return true;
        if (ncpus_used_mt >= 2*ncpus) return true;
            // kind of arbitrary, but need to have some limit
            // in case there are only MT jobs, and lots of them
        return false;
    }

    inline bool stop_scan_coproc(int rsc_type) {
        COPROC& cp = pr_coprocs.coprocs[rsc_type];
        for (int i=0; i<cp.count; i++) {
            if (cp.usage[i] < 1) return false;
        }
        return true;
    }

    // should we consider scheduling this job?
    // (i.e add it to the runnable list; not actually run it)
    //
    bool can_schedule(RESULT* rp, ACTIVE_TASK* atp) {
        if (atp) {
            // don't schedule if something's pending
            //
            switch (atp->task_state()) {
            case PROCESS_ABORT_PENDING:
            case PROCESS_QUIT_PENDING:
                return false;
            }
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
        }
        if (rp->schedule_backoff > gstate.now) return false;
        if (rp->uses_gpu()) {
            if (gpu_suspend_reason) return false;
        }
        if (rp->uses_coprocs()) {
            if (sufficient_coprocs(*rp)) {
                return true;
            } else {
                return false;
            }
        } else if (rp->resource_usage.avg_ncpus > 1) {
            if (ncpus_used_mt == 0) return true;
            return (ncpus_used_mt + rp->resource_usage.avg_ncpus <= ncpus);
        } else {
            return (ncpus_used_st < ncpus);
        }
    }

    // we've decided to add this to the runnable list; update bookkeeping
    //
    void schedule(RESULT* rp, ACTIVE_TASK* atp, bool is_edf) {
        int rt = rp->resource_usage.rsc_type;

        // see if it's possible this job will be ruled out
        // when we try to actually run it
        // (e.g. it won't fit in RAM, or it uses GPU type w/ exclusions)
        // If so, don't reserve CPU/GPU for it, to avoid starvation scenario
        //
        bool may_not_run = false;
        if (atp && atp->too_large) {
            may_not_run = true;
        }
        if (rt) {
            PROJECT* p = rp->project;
            if (p->rsc_pwf[rt].ncoprocs_excluded > 0) {
                may_not_run = true;
            }
        }

        if (!may_not_run) {
            if (rt) {
                reserve_coprocs(*rp);
                // don't increment CPU usage.
                // This may seem odd; the reason is the following scenario:
                // - this job uses lots of CPU (say, a whole one)
                // - there's an uncheckpointed GPU job that uses little CPU
                // - we end up running the uncheckpointed job
                // - this causes all or part of a CPU to be idle
                //
            } else if (rp->resource_usage.avg_ncpus > 1) {
                ncpus_used_mt += rp->resource_usage.avg_ncpus;
            } else {
                ncpus_used_st += rp->resource_usage.avg_ncpus;
            }
        }
        if (log_flags.cpu_sched_debug) {
            msg_printf(rp->project, MSG_INFO,
                "[cpu_sched_debug] add to run list: %s (%s, %s) (prio %f)",
                rp->name,
                rsc_name_long(rt),
                is_edf?"EDF":"FIFO",
                rp->project->sched_priority
            );
        }

        adjust_rec_sched(rp);
    }

    bool sufficient_coprocs(RESULT& r) {
        int rt = r.resource_usage.rsc_type;
        if (!rt) return true;
        double x = r.resource_usage.coproc_usage;
        COPROC& cp = pr_coprocs.coprocs[rt];
        for (int i=0; i<cp.count; i++) {
            if (gpu_excluded(r.app, cp, i)) continue;
            double unused = 1 - cp.usage[i];
            x -= unused;
            if (x <= 0) return true;
        }
        if (log_flags.cpu_sched_debug) {
            msg_printf(r.project, MSG_INFO,
                "[cpu_sched_debug] insufficient %s for %s",
                cp.type, r.name
            );
        }
        return false;
    }

    void reserve_coprocs(RESULT& r) {
        double x;
        int rt = r.resource_usage.rsc_type;
        COPROC& cp = pr_coprocs.coprocs[rt];
        x = r.resource_usage.coproc_usage;
        for (int i=0; i<cp.count; i++) {
            if (gpu_excluded(r.app, cp, i)) continue;
            double unused = 1 - cp.usage[i];
            if (unused >= x) {
                cp.usage[i] += x;
                break;
            } else {
                cp.usage[i] = 1;
                x -= unused;
            }
        }
        if (log_flags.cpu_sched_debug) {
            msg_printf(r.project, MSG_INFO,
                "[cpu_sched_debug] reserving %f of coproc %s",
                r.resource_usage.coproc_usage, cp.type
            );
        }
    }
};

bool gpus_usable = true;

#ifndef SIM
// see whether there's been a change in coproc usability;
// if so set or clear "missing_coproc" flags and return true.
//
bool check_coprocs_usable() {
#ifdef _WIN32
    if (cc_config.no_rdp_check) return false;
    unsigned int i;
    bool new_usable = !is_remote_desktop();
    if (gpus_usable) {
        if (!new_usable) {
            gpus_usable = false;
            for (i=0; i<gstate.results.size(); i++) {
                RESULT* rp = gstate.results[i];
                if (rp->resource_usage.rsc_type) {
                    rp->resource_usage.missing_coproc = true;
                }
            }
            msg_printf(NULL, MSG_INFO,
                "Remote desktop in use; disabling GPU tasks"
            );
            return true;
        }
    } else {
        if (new_usable) {
            gpus_usable = true;
            for (i=0; i<gstate.results.size(); i++) {
                RESULT* rp = gstate.results[i];
                if (rp->resource_usage.rsc_type) {
                    rp->resource_usage.missing_coproc = false;
                }
            }
            msg_printf(NULL, MSG_INFO,
                "Remote desktop not in use; enabling GPU tasks"
            );
            return true;
        }
    }
#endif
    return false;
}
#endif

// return true if the task has finished its time slice
// and has checkpointed since the end of the time slice
// (called only for scheduled tasks)
//
static inline bool finished_time_slice(ACTIVE_TASK* atp) {
    double time_slice_end = atp->run_interval_start_wall_time + gstate.global_prefs.cpu_scheduling_period();
    bool running_beyond_sched_period = gstate.now > time_slice_end;
    bool checkpointed = atp->checkpoint_wall_time > time_slice_end;
    if (running_beyond_sched_period && !checkpointed) {
        atp->overdue_checkpoint = true;
    }
    return (running_beyond_sched_period && checkpointed);
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
        if (atp->always_run()) continue;
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
// find the project P with the largest priority,
// and return its next runnable result
//
RESULT* CLIENT_STATE::highest_prio_project_best_result() {
    PROJECT *best_project = NULL;
    double best_prio = 0;
    bool first = true;
    unsigned int i;

    for (i=0; i<projects.size(); i++) {
        PROJECT* p = projects[i];
        if (!p->next_runnable_result) continue;
        if (first || p->sched_priority > best_prio) {
            first = false;
            best_project = p;
            best_prio = p->sched_priority;
        }
    }
    if (!best_project) return NULL;

    RESULT* rp = best_project->next_runnable_result;
    best_project->next_runnable_result = 0;
    return rp;
}

// Return a job of the given type according to the following criteria
// (desc priority):
//  - from project with higher priority
//  - already-started job
//  - earlier received_time
//  - lexicographically earlier name
//
// Give priority to already-started jobs because of the following scenario:
// - client gets several jobs in a sched reply and starts downloading files
// - a later job finishes downloading and starts
// - an earlier finishes downloading and preempts
//
RESULT* first_coproc_result(int rsc_type) {
    unsigned int i;
    RESULT* best = NULL;
    double best_prio=0, prio;
    for (i=0; i<gstate.results.size(); i++) {
        RESULT* rp = gstate.results[i];
        if (rp->resource_type() != rsc_type) continue;
        if (!rp->runnable()) {
            //msg_printf(rp->project, MSG_INFO, "not runnable: %s", rp->name);
            continue;
        }
        if (rp->always_run()) continue;
        if (rp->already_selected) continue;
        prio = rp->project->sched_priority;
        if (!best) {
            best = rp;
            best_prio = prio;
            continue;
        }

        if (prio < best_prio) {
            continue;
        }
        if (prio > best_prio) {
            best = rp;
            best_prio = prio;
            continue;
        }

        bool bs = !best->not_started;
        bool rs = !rp->not_started;
        if (rs && !bs) {
            best = rp;
            best_prio = prio;
            continue;
        }
        if (!rs && bs) {
            continue;
        }

        // else used "arrived first" order
        //
        if (rp->index < best->index) {
            best = rp;
            best_prio = prio;
        }
    }
    return best;
}

// Return earliest-deadline result for given resource type;
// return only results projected to miss their deadline,
// or from projects with extreme DCF
//
static RESULT* earliest_deadline_result(int rsc_type) {
    RESULT *best_result = NULL;
    ACTIVE_TASK* best_atp = NULL;
    unsigned int i;

    for (i=0; i<gstate.results.size(); i++) {
        RESULT* rp = gstate.results[i];
        if (rp->resource_type() != rsc_type) continue;
        if (rp->already_selected) continue;
        if (!rp->runnable()) continue;
        if (rp->always_run()) continue;
        PROJECT* p = rp->project;

        // Skip this job if the project's deadline-miss count is zero.
        // If the project's DCF is > 90 (and we're not ignoring it)
        // treat all jobs as deadline misses
        //
        if (p->dont_use_dcf || p->duration_correction_factor < 90.0) {
            if (p->rsc_pwf[rsc_type].deadlines_missed_copy <= 0) {
                continue;
            }
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
            best_atp = gstate.lookup_active_task_by_result(rp);
            continue;
        }
        if (rp->report_deadline > best_result->report_deadline) {
            continue;
        }

        // If there's a tie, pick the job with the least remaining time
        // (but don't pick an unstarted job over one that's started)
        //
        ACTIVE_TASK* atp = gstate.lookup_active_task_by_result(rp);
        if (best_atp && !atp) continue;
        if (rp->estimated_runtime_remaining() < best_result->estimated_runtime_remaining()
            || (!best_atp && atp)
        ) {
            best_result = rp;
            best_atp = atp;
        }
    }
    if (!best_result) return NULL;

    return best_result;
}

void CLIENT_STATE::reset_rec_accounting() {
    unsigned int i;
    for (i=0; i<projects.size(); i++) {
        PROJECT* p = projects[i];
        for (int j=0; j<coprocs.n_rsc; j++) {
            p->rsc_pwf[j].reset_rec_accounting();
        }
    }
    for (int j=0; j<coprocs.n_rsc; j++) {
        rsc_work_fetch[j].reset_rec_accounting();
    }
    rec_interval_start = now;
}

// update per-project accounting:
//  - recent estimated credit (EC)
//  - total CPU and GPU EC
//  - total CPU and GPU time
//
static void update_rec() {
    double f = gstate.host_info.p_fpops;
    double on_frac = gstate.current_cpu_usage_limit() / 100;

    for (unsigned int i=0; i<gstate.projects.size(); i++) {
        PROJECT* p = gstate.projects[i];

        double x = 0;
        for (int j=0; j<coprocs.n_rsc; j++) {
            double dt = p->rsc_pwf[j].secs_this_rec_interval * on_frac;
            double flops = dt * f * rsc_work_fetch[j].relative_speed;
            x += flops;
            if (j) {
                p->gpu_ec += flops*COBBLESTONE_SCALE;
                p->gpu_time += dt;
            } else {
                p->cpu_ec += flops*COBBLESTONE_SCALE;
                p->cpu_time += dt;
            }
        }
        x *= COBBLESTONE_SCALE;
        double old = p->pwf.rec;

        // start averages at zero
        //
        if (p->pwf.rec_time == 0) {
            p->pwf.rec_time = gstate.rec_interval_start;
        }

        update_average(
            gstate.now,
            gstate.rec_interval_start,
            x,
            cc_config.rec_half_life,
            p->pwf.rec,
            p->pwf.rec_time
        );

        if (log_flags.priority_debug) {
            double dt = gstate.now - gstate.rec_interval_start;
            msg_printf(p, MSG_INFO,
                "[prio] recent est credit: %.2fG in %.2f sec, %f + %f ->%f",
                x, dt, old, p->pwf.rec-old, p->pwf.rec
            );
        }
    }
}

static double peak_flops(RESULT *rp) {
    double f = gstate.host_info.p_fpops;
    double x = f * rp->resource_usage.avg_ncpus;
    int rt = rp->resource_usage.rsc_type;
    if (rt) {
        x += f * rp->resource_usage.coproc_usage * rsc_work_fetch[rt].relative_speed;
    }
    return x;
}

double total_peak_flops() {
    static bool first=true;
    static double tpf;
    if (first) {
        first = false;
        tpf = gstate.host_info.p_fpops * gstate.n_usable_cpus;
        for (int i=1; i<coprocs.n_rsc; i++) {
            COPROC& cp = coprocs.coprocs[i];
            tpf += rsc_work_fetch[i].relative_speed * gstate.host_info.p_fpops * cp.count;
        }
    }
    return tpf;
}

// Initialize project "priorities" based on REC:
// compute resource share and REC fractions
// among compute-intensive, non-suspended projects
//
void project_priority_init(bool for_work_fetch) {
    double rs_sum = 0;
    rec_sum = 0;
    for (unsigned int i=0; i<gstate.projects.size(); i++) {
        PROJECT* p = gstate.projects[i];
        if (p->non_cpu_intensive) continue;
        if (for_work_fetch) {
            if (!p->can_request_work()) continue;
        } else {
            if (!p->runnable(RSC_TYPE_ANY)) continue;
        }
        p->pwf.rec_temp = p->pwf.rec;
        rs_sum += p->resource_share;
        rec_sum += p->pwf.rec_temp;
    }
    if (rec_sum == 0) {
        rec_sum = 1;
    }
    for (unsigned int i=0; i<gstate.projects.size(); i++) {
        PROJECT* p = gstate.projects[i];
        if (p->non_cpu_intensive || p->suspended_via_gui || rs_sum==0) {
            p->resource_share_frac = 0;
            p->sched_priority = 0;
        } else {
            p->resource_share_frac = p->resource_share/rs_sum;
            p->compute_sched_priority();
            if (log_flags.priority_debug) {
                msg_printf(p, MSG_INFO, "[prio] %f rsf %f rt %f rs %f",
                    p->sched_priority, p->resource_share_frac,
                    p->pwf.rec_temp, rec_sum
                );
            }
        }
    }
}

void PROJECT::compute_sched_priority() {
    double rec_frac = pwf.rec_temp/rec_sum;

    // projects with zero resource share are always lower priority
    // than those with positive resource share
    //
    if (resource_share == 0) {
        sched_priority = -1e3 - rec_frac;
    } else {
        sched_priority = - rec_frac/resource_share_frac;
    }
}

// called from the scheduler's job-selection loop;
// we plan to run this job;
// bump the project's temp REC by the estimated credit for 1 hour.
// This encourages a mixture jobs from different projects.
//
void adjust_rec_sched(RESULT* rp) {
    PROJECT* p = rp->project;
    p->pwf.rec_temp += peak_flops(rp)/total_peak_flops() * rec_sum/24;
    p->compute_sched_priority();
}

// make this a variable so simulator can change it
//
double rec_adjust_period = REC_ADJUST_PERIOD;

// adjust project REC
//
void CLIENT_STATE::adjust_rec() {
    unsigned int i;
    double elapsed_time = now - rec_interval_start;

    // If the elapsed time is negative or more than 2*REC_ADJUST_PERIOD
    // it must be because either
    // - the system clock was changed.
    // - the host was suspended for a long time.
    // In either case, ignore the last period
    //
    if (elapsed_time > 2*rec_adjust_period || elapsed_time < 0) {
        if (log_flags.priority_debug) {
            msg_printf(NULL, MSG_INFO,
                "[priority] adjust_rec: elapsed time (%.0f) negative or longer than sched enforce period(%.0f).  Ignoring this period.",
                elapsed_time, rec_adjust_period
            );
        }
        reset_rec_accounting();
        return;
    }

    // skip small intervals
    //
    if (elapsed_time < 1) {
        return;
    }

    // total up how many instance-seconds projects got
    //
    for (i=0; i<active_tasks.active_tasks.size(); i++) {
        ACTIVE_TASK* atp = active_tasks.active_tasks[i];
        if (atp->scheduler_state != CPU_SCHED_SCHEDULED) continue;
        if (atp->non_cpu_intensive()) continue;
        work_fetch.accumulate_inst_sec(atp, elapsed_time);
    }

    update_rec();

    reset_rec_accounting();
}


// Possibly do job scheduling.
// This is called periodically.
//
bool CLIENT_STATE::schedule_cpus() {
    double elapsed_time;
    static double last_reschedule=0;
    vector<RESULT*> run_list;

    if (projects.size() == 0) return false;
    if (results.size() == 0) return false;

    // Reschedule every CPU_SCHED_PERIOD seconds,
    // or if must_schedule_cpus is set
    // (meaning a new result is available, or a CPU has been freed).
    //
    elapsed_time = now - last_reschedule;
    if (gstate.clock_change || elapsed_time >= CPU_SCHED_PERIOD) {
        request_schedule_cpus("periodic CPU scheduling");
    }

    if (!must_schedule_cpus) return false;
    last_reschedule = now;
    must_schedule_cpus = false;

    // NOTE: there's an assumption that REC is adjusted at
    // least as often as the CPU sched period (see client_state.h).
    // If you remove the following, make changes accordingly
    //
    adjust_rec();

    // this may run tasks that are currently throttled.
    // Clear flag so that we throttle them again if needed
    //
    tasks_throttled = false;

    make_run_list(run_list);
    return enforce_run_list(run_list);
}

// Mark a job J as a deadline miss if either
// - it once ran in EDF, and its project has another job
//   of the same resource type marked as deadline miss.
//   This avoids domino-effect preemption
// - it was recently marked as a deadline miss by RR sim.
//   This avoids "thrashing" if a job oscillates between miss and not miss.
//
static void promote_once_ran_edf() {
    for (unsigned int i=0; i<gstate.active_tasks.active_tasks.size(); i++) {
        ACTIVE_TASK* atp = gstate.active_tasks.active_tasks[i];
        if (atp->result->rr_sim_misses_deadline) continue;
        if (atp->once_ran_edf) {
            RESULT* rp = atp->result;
            PROJECT* p = rp->project;
            if (p->deadlines_missed(rp->resource_usage.rsc_type)) {
                if (log_flags.cpu_sched_debug) {
                    msg_printf(p, MSG_INFO,
                        "[cpu_sched_debug] domino prevention: mark %s as deadline miss",
                        rp->name
                    );
                }
                rp->rr_sim_misses_deadline = true;
                continue;
            }
        }
        if (gstate.now - atp->last_deadline_miss_time < gstate.global_prefs.cpu_scheduling_period()) {
            if (log_flags.cpu_sched_debug) {
                RESULT* rp = atp->result;
                PROJECT* p = rp->project;
                msg_printf(p, MSG_INFO,
                    "[cpu_sched_debug] thrashing prevention: mark %s as deadline miss",
                    rp->name
                );
            }
            atp->result->rr_sim_misses_deadline = true;
        }
    }
}

void add_coproc_jobs(
    vector<RESULT*>& run_list, int rsc_type, PROC_RESOURCES& proc_rsc
) {
    ACTIVE_TASK* atp;
    RESULT* rp;

#ifdef SIM
    if (!cpu_sched_rr_only) {
#endif
    // choose coproc jobs from projects with coproc deadline misses
    //
    while (!proc_rsc.stop_scan_coproc(rsc_type)) {
        rp = earliest_deadline_result(rsc_type);
        if (!rp) break;
        rp->already_selected = true;
        atp = gstate.lookup_active_task_by_result(rp);
        if (!proc_rsc.can_schedule(rp, atp)) continue;
        proc_rsc.schedule(rp, atp, true);
        rp->project->rsc_pwf[rsc_type].deadlines_missed_copy--;
        rp->edf_scheduled = true;
        run_list.push_back(rp);
    }
#ifdef SIM
    }
#endif

    // then coproc jobs in FIFO order
    //
    while (!proc_rsc.stop_scan_coproc(rsc_type)) {
        rp = first_coproc_result(rsc_type);
        if (!rp) break;
        rp->already_selected = true;
        atp = gstate.lookup_active_task_by_result(rp);
        if (!proc_rsc.can_schedule(rp, atp)) continue;
        proc_rsc.schedule(rp, atp, false);
        run_list.push_back(rp);
    }
}

// Make an ordered list of jobs to run.
//
void CLIENT_STATE::make_run_list(vector<RESULT*>& run_list) {
    RESULT* rp;
    PROJECT* p;
    unsigned int i;
    PROC_RESOURCES proc_rsc;
    ACTIVE_TASK* atp;

    if (log_flags.cpu_sched_debug) {
        msg_printf(0, MSG_INFO, "[cpu_sched_debug] schedule_cpus(): start");
    }

    proc_rsc.init();

    // if there are sporadic apps,
    // subtract the resource usage of those that are computing
    //
    if (have_sporadic_app) {
        proc_rsc.ncpus -= sporadic_resources.ncpus_used;
        for (int rt=1; rt<proc_rsc.pr_coprocs.n_rsc; rt++) {
            COPROC& cp = proc_rsc.pr_coprocs.coprocs[rt];
            COPROC& cp2 = sporadic_resources.sr_coprocs.coprocs[rt];
            for (int j=0; j<cp.count; j++) {
                cp.usage[j] = cp2.usage[j];
            }
        }
    }

    // do round-robin simulation to find what results miss deadline
    //
    rr_simulation("CPU sched");
    if (log_flags.rr_simulation) {
        print_deadline_misses();
    }

    // avoid preemption of jobs that once ran EDF
    //
    promote_once_ran_edf();

    // set temporary variables
    //
    project_priority_init(false);
    for (i=0; i<results.size(); i++) {
        rp = results[i];
        rp->already_selected = false;
        rp->edf_scheduled = false;
        rp->not_started = !rp->computing_done();
    }
    for (i=0; i<projects.size(); i++) {
        p = projects[i];
        p->next_runnable_result = NULL;
        for (int j=0; j<coprocs.n_rsc; j++) {
            p->rsc_pwf[j].deadlines_missed_copy = p->rsc_pwf[j].deadlines_missed;
        }
    }

    // compute max working set size for app versions
    // (max of working sets of currently running jobs)
    //
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
        atp->result->not_started = false;
    }

    // first, add GPU jobs

    for (int j=1; j<coprocs.n_rsc; j++) {
        add_coproc_jobs(run_list, j, proc_rsc);
    }

    // enforce max concurrent specs for CPU jobs,
    // to avoid having jobs in the run list that we can't actually run.
    // Don't do this for GPU jobs; GPU exclusions screw things up
    //
    if (have_max_concurrent) {
        max_concurrent_init();
    }

    // then add CPU jobs.
    // Note: the jobs that actually get run are not necessarily
    // an initial segment of this list;
    // e.g. a multithread job may not get run because it has
    // a high-priority single-thread job ahead of it.

    // choose CPU jobs from projects with CPU deadline misses
    //
#ifdef SIM
    if (!cpu_sched_rr_only) {
#endif
    while (!proc_rsc.stop_scan_cpu()) {
        rp = earliest_deadline_result(RSC_TYPE_CPU);
        if (!rp) break;
        rp->already_selected = true;
        if (have_max_concurrent && max_concurrent_exceeded(rp)) {
            continue;
        }
        atp = lookup_active_task_by_result(rp);
        if (!proc_rsc.can_schedule(rp, atp)) continue;
        proc_rsc.schedule(rp, atp, true);
        rp->project->rsc_pwf[0].deadlines_missed_copy--;
        rp->edf_scheduled = true;
        run_list.push_back(rp);
        if (have_max_concurrent) {
            max_concurrent_inc(rp);
        }
    }
#ifdef SIM
    }
#endif

    // Next, choose CPU jobs from highest priority projects
    //
    while (1) {
        if (proc_rsc.stop_scan_cpu()) {
            break;
        }
        assign_results_to_projects();
        rp = highest_prio_project_best_result();
        if (!rp) {
            break;
        }
        if (have_max_concurrent && max_concurrent_exceeded(rp)) {
            continue;
        }
        atp = lookup_active_task_by_result(rp);
        if (!proc_rsc.can_schedule(rp, atp)) continue;
        proc_rsc.schedule(rp, atp, false);
        run_list.push_back(rp);
        if (have_max_concurrent) {
            max_concurrent_inc(rp);
        }
    }
}

static inline bool in_run_list(vector<RESULT*>& run_list, ACTIVE_TASK* atp) {
    for (unsigned int i=0; i<run_list.size(); i++) {
        if (atp->result == run_list[i]) return true;
    }
    return false;
}

#if 0
// scan the runnable list, keeping track of CPU usage X.
// if find a MT job J, and X < ncpus, move J before all non-MT jobs
// But don't promote a MT job ahead of a job in EDF
//
// This is needed because there may always be a 1-CPU job
// in the middle of its time-slice, and MT jobs could starve.
//
static void promote_multi_thread_jobs(vector<RESULT*>& runnable_jobs) {
    double cpus_used = 0;
    vector<RESULT*>::iterator first_non_mt = runnable_jobs.end();
    vector<RESULT*>::iterator cur = runnable_jobs.begin();
    while(1) {
        if (cur == runnable_jobs.end()) break;
        if (cpus_used >= gstate.n_usable_cpus) break;
        RESULT* rp = *cur;
        if (rp->rr_sim_misses_deadline) break;
        double nc = rp->avp->avg_ncpus;
        if (nc > 1) {
            if (first_non_mt != runnable_jobs.end()) {
                cur = runnable_jobs.erase(cur);
                runnable_jobs.insert(first_non_mt, rp);
                cpus_used = 0;
                first_non_mt = runnable_jobs.end();
                cur = runnable_jobs.begin();
                continue;
            }
        } else {
            if (first_non_mt == runnable_jobs.end()) {
                first_non_mt = cur;
            }
        }
        cpus_used += nc;
        cur++;
    }
}
#endif

// return true if r0 is more important to run than r1
//
static inline bool more_important(RESULT* r0, RESULT* r1) {
    // favor jobs in danger of deadline miss
    //
    bool miss0 = r0->edf_scheduled;
    bool miss1 = r1->edf_scheduled;
    if (miss0 && !miss1) return true;
    if (!miss0 && miss1) return false;

    // favor coproc jobs, so that e.g. if we're RAM-limited
    // we'll use the GPU instead of the CPU
    //
    bool cp0 = r0->uses_coprocs();
    bool cp1 = r1->uses_coprocs();
    if (cp0 && !cp1) return true;
    if (!cp0 && cp1) return false;

    // favor jobs in the middle of time slice,
    // or that haven't checkpointed since start of time slice
    //
    bool unfin0 = r0->unfinished_time_slice;
    bool unfin1 = r1->unfinished_time_slice;
    if (unfin0 && !unfin1) return true;
    if (!unfin0 && unfin1) return false;

    // for CPU jobs, favor jobs that use more CPUs
    //
    if (!cp0) {
        if (r0->resource_usage.avg_ncpus > r1->resource_usage.avg_ncpus) return true;
        if (r1->resource_usage.avg_ncpus > r0->resource_usage.avg_ncpus) return false;
    }

    // favor jobs selected first by schedule_cpus()
    // (e.g., because their project has high sched priority)
    //
    if (r0->seqno < r1->seqno) return true;
    if (r0->seqno > r1->seqno) return false;

    // tie breaker
    return (r0 < r1);
}

static void print_job_list(vector<RESULT*>& jobs) {
    char buf[256];
    for (unsigned int i=0; i<jobs.size(); i++) {
        RESULT* rp = jobs[i];
        rp->rsc_string(buf, 256);
        msg_printf(rp->project, MSG_INFO,
            "[cpu_sched_debug] %d: %s (%s; MD: %s; UTS: %s)",
            i, rp->name,
            buf,
            rp->edf_scheduled?"yes":"no",
            rp->unfinished_time_slice?"yes":"no"
        );
    }
}

// find running jobs that haven't finished their time slice.
// Mark them as such, and add to list if not already there
//
void CLIENT_STATE::append_unfinished_time_slice(vector<RESULT*> &run_list) {
    unsigned int i;
    int seqno = (int)run_list.size();

    for (i=0; i<active_tasks.active_tasks.size(); i++) {
        ACTIVE_TASK* atp = active_tasks.active_tasks[i];
        atp->overdue_checkpoint = false;
        if (!atp->result->runnable()) continue;
        if (atp->result->uses_gpu() && gpu_suspend_reason) continue;
        if (atp->result->always_run()) continue;
        if (atp->scheduler_state != CPU_SCHED_SCHEDULED) continue;
        if (finished_time_slice(atp)) continue;
        atp->result->unfinished_time_slice = true;
        if (in_run_list(run_list, atp)) continue;
        run_list.push_back(atp->result);
        atp->result->seqno = seqno;
    }
}

// Enforce the CPU schedule.
// Inputs:
//   run_list: list of runnable jobs, ordered by decreasing project priority
//      (created by make_run_list())
//      Doesn't include all jobs, but enough to fill CPUs even in MT scenarios.
//
// - append running jobs that haven't finished their time slice
// - order the list by "important" (which includes various factor)
// - then scan the list and run jobs
//      - until we've used all resources
//      - skip jobs that would exceed mem limits
//
// Initially, each task's scheduler_state is PREEMPTED or SCHEDULED
// depending on whether or not it is running.
// This function sets each task's next_scheduler_state,
// and at the end it starts/resumes and preempts tasks
// based on scheduler_state and next_scheduler_state.
//
bool CLIENT_STATE::enforce_run_list(vector<RESULT*>& run_list) {
    unsigned int i;
    int retval;
    double ncpus_used=0;
    ACTIVE_TASK* atp;

    bool action = false;

    if (have_max_concurrent) {
        max_concurrent_init();
    }

#ifndef SIM
    // check whether GPUs are usable
    //
    if (check_coprocs_usable()) {
        request_schedule_cpus("GPU usability change");
        return true;
    }
#endif

    if (log_flags.cpu_sched_debug) {
        msg_printf(0, MSG_INFO, "[cpu_sched_debug] enforce_run_list(): start");
        msg_printf(0, MSG_INFO, "[cpu_sched_debug] preliminary job list:");
        print_job_list(run_list);
    }

    // Set next_scheduler_state to PREEMPT for all tasks
    //
    for (i=0; i< active_tasks.active_tasks.size(); i++) {
        atp = active_tasks.active_tasks[i];
        atp->next_scheduler_state = CPU_SCHED_PREEMPTED;
    }

    for (i=0; i<run_list.size(); i++) {
        RESULT* rp = run_list[i];
        rp->seqno = i;
        rp->unfinished_time_slice = false;
    }

    // add running jobs not done with time slice to the run list
    //
    append_unfinished_time_slice(run_list);

    // sort run list by decreasing importance
    //
    std::sort(
        run_list.begin(),
        run_list.end(),
        more_important
    );

#if 0
    promote_multi_thread_jobs(run_list);
#endif

    if (log_flags.cpu_sched_debug) {
        msg_printf(0, MSG_INFO, "[cpu_sched_debug] final job list:");
        print_job_list(run_list);
    }

    double ram_left = available_ram();
    if (have_sporadic_app) {
        ram_left -= sporadic_resources.mem_used;
    }
    double swap_left = (global_prefs.vm_max_used_frac)*host_info.m_swap;

    if (log_flags.mem_usage_debug) {
        msg_printf(0, MSG_INFO,
            "[mem_usage] enforce: available RAM %.2fMB swap %.2fMB",
            ram_left/MEGA, swap_left/MEGA
        );
    }

    // schedule non-CPU-intensive and sporadic tasks
    //
    for (i=0; i<results.size(); i++) {
        RESULT* rp = results[i];
        if (rp->always_run() && rp->runnable()) {
            atp = get_task(rp);
            if (!atp) {
                msg_printf(rp->project, MSG_INTERNAL_ERROR,
                    "Can't create task for %s", rp->name
                );
                continue;
            }
            atp->next_scheduler_state = CPU_SCHED_SCHEDULED;

            // don't count RAM usage because it's used sporadically,
            // and doing so can starve other jobs
            //
            //ram_left -= atp->procinfo.working_set_size_smoothed;
            swap_left -= atp->procinfo.swap_size;
        }
    }

    // assign coprocessors to coproc jobs,
    // and prune those that can't be assigned
    //
    assign_coprocs(run_list);

    // scan the run list
    //
    for (i=0; i<run_list.size(); i++) {
        RESULT* rp = run_list[i];

        if (have_max_concurrent && max_concurrent_exceeded(rp)) {
            if (log_flags.cpu_sched_debug) {
                msg_printf(rp->project, MSG_INFO,
                    "[cpu_sched_debug] skipping %s; max concurrent limit reached",
                    rp->name
                );
            }
            continue;
        }

        atp = lookup_active_task_by_result(rp);

        // if we're already using all the CPUs, don't allow additional CPU jobs;
        // allow coproc jobs if the resulting CPU load is at most ncpus+1
        //
        if (ncpus_used >= n_usable_cpus) {
            if (rp->uses_coprocs()) {
                if (ncpus_used + rp->resource_usage.avg_ncpus > n_usable_cpus+1) {
                    if (log_flags.cpu_sched_debug) {
                        msg_printf(rp->project, MSG_INFO,
                            "[cpu_sched_debug] skipping GPU job %s; CPU committed",
                            rp->name
                        );
                    }
                    continue;
                }
            } else {
                if (log_flags.cpu_sched_debug) {
                    msg_printf(rp->project, MSG_INFO,
                        "[cpu_sched_debug] all CPUs used (%.2f >= %d), skipping %s",
                        ncpus_used, n_usable_cpus,
                        rp->name
                    );
                }
                continue;
            }
        }

        // There's a possibility that this job is MT
        // and would overcommit the CPUs by > 1.
        // Options are:
        // 1) run it anyway, and overcommit the CPUs
        // 2) don't run it.
        //      This can result in starvation.
        // 3) don't run it if there are additional 1-CPU jobs.
        //      The problem here is that we may never run the MT job
        //      until it reaches deadline pressure.
        // So we'll go with 1).

        // skip jobs whose 'expected working set size' (EWSS)
        // is too large to fit in available RAM.
        // To compute EWSS, we start with
        // - if the job has already run, its recent average WSS
        // - else if other jobs of this app version have run recently,
        //      the max of their WSSs
        // - else the WU's rsc_memory_bound
        // If project->strict_memory_bound is set,
        // we max the above with wu.rsc_memory_bound.
        // This is to handle apps (like CPDN) whose WSS is small for a while
        // and then gets big.
        //
        double ewss = 0;
        if (atp) {
            atp->too_large = false;
            ewss = atp->procinfo.working_set_size_smoothed;
        } else {
            ewss = rp->avp->max_working_set_size;
        }
        if (rp->project->strict_memory_bound) {
            ewss = std::max(ewss, rp->wup->rsc_memory_bound);
        } else {
            if (ewss == 0) {
                ewss = rp->wup->rsc_memory_bound;
            }
        }

        if (ewss > ram_left) {
            if (atp) {
                atp->too_large = true;
            }
            if (log_flags.cpu_sched_debug || log_flags.mem_usage_debug) {
                msg_printf(rp->project, MSG_INFO,
                    "[cpu_sched_debug] skipping %s: estimated WSS (%.2fMB) exceeds RAM left (%.2fMB)",
                    rp->name,  ewss/MEGA, ram_left/MEGA
                );
            }
            continue;
        }

        // We've decided to run this job
        //
        if (log_flags.cpu_sched_debug) {
            msg_printf(rp->project, MSG_INFO,
                "[cpu_sched_debug] scheduling %s%s",
                rp->name, rp->edf_scheduled?" (high priority)":""
            );
        }

        // create an ACTIVE_TASK if needed.
        //
        if (!atp) {
            atp = get_task(rp);
        }
        if (!atp) {
            msg_printf(rp->project, MSG_INTERNAL_ERROR,
                "Can't create task for %s", rp->name
            );
            continue;
        }

        ncpus_used += rp->resource_usage.avg_ncpus;
        atp->next_scheduler_state = CPU_SCHED_SCHEDULED;
        ram_left -= ewss;
        if (have_max_concurrent) {
            max_concurrent_inc(rp);
        }
    }

    // if CPUs are starved, ask for more jobs
    //
    if (ncpus_used < n_usable_cpus) {
        if (ncpus_used < n_usable_cpus) {
            request_work_fetch("CPUs idle");
        }
        if (log_flags.cpu_sched_debug) {
            msg_printf(0, MSG_INFO, "[cpu_sched_debug] using only %.2f out of %d CPUs",
                ncpus_used, n_usable_cpus
            );
        }
    }

    bool check_swap = (host_info.m_swap != 0);
        // in case couldn't measure swap on this host

    // TODO: enforcement of swap space is broken right now

    // preempt tasks as needed, and note whether there are any coproc jobs
    // in QUIT_PENDING state (in which case we won't start new coproc jobs)
    //
    bool coproc_quit_pending = false;
    for (i=0; i<active_tasks.active_tasks.size(); i++) {
        atp = active_tasks.active_tasks[i];
#if 0
        if (log_flags.cpu_sched_debug) {
            msg_printf(atp->result->project, MSG_INFO,
                "[cpu_sched_debug] %s sched state %d next %d task state %d",
                atp->result->name, atp->scheduler_state,
                atp->next_scheduler_state, atp->task_state()
            );
        }
#endif
        PREEMPT_TYPE preempt_type = REMOVE_MAYBE_SCHED;
        if (atp->next_scheduler_state == CPU_SCHED_PREEMPTED) {
            switch (atp->task_state()) {
            case PROCESS_EXECUTING:
                action = true;
                if (check_swap && swap_left < 0) {
                    if (log_flags.mem_usage_debug) {
                        msg_printf(atp->result->project, MSG_INFO,
                            "[mem_usage] out of swap space, will preempt by quit"
                        );
                    }
                    preempt_type = REMOVE_ALWAYS;
                }
                if (atp->too_large) {
                    if (log_flags.mem_usage_debug) {
                        msg_printf(atp->result->project, MSG_INFO,
                            "[mem_usage] job using too much memory, will preempt by quit"
                        );
                    }
                    preempt_type = REMOVE_ALWAYS;
                }
                atp->preempt(preempt_type);
                break;
            case PROCESS_SUSPENDED:
                // remove from memory GPU jobs that were suspended by CPU throttling
                // and are now unscheduled.
                //
                if (atp->result->uses_gpu()) {
                    atp->preempt(REMOVE_ALWAYS);
                    request_schedule_cpus("removed suspended GPU task");
                    break;
                }

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
        }
        if (atp->result->uses_coprocs() && atp->task_state() == PROCESS_QUIT_PENDING) {
            coproc_quit_pending = true;
        }
    }

    bool coproc_start_deferred = false;
    for (i=0; i<active_tasks.active_tasks.size(); i++) {
        atp = active_tasks.active_tasks[i];
        if (atp->next_scheduler_state != CPU_SCHED_SCHEDULED) continue;
        int ts = atp->task_state();
        if (ts == PROCESS_UNINITIALIZED || ts == PROCESS_SUSPENDED) {
            // If there's a quit pending for a coproc job,
            // don't start new ones since they may bomb out
            // on memory allocation.  Instead, trigger a retry
            //
            if (atp->result->uses_coprocs() && coproc_quit_pending) {
                coproc_start_deferred = true;
                continue;
            }
            action = true;

            bool first_time;
            // GPU tasks can get suspended before they're ever run,
            // so the only safe way of telling whether this is the
            // first time the app is run is to check
            // whether the slot dir is empty
            //
#ifdef SIM
            first_time = atp->scheduler_state == CPU_SCHED_UNINITIALIZED;
#else
            first_time = is_dir_empty(atp->slot_dir);
#endif
            retval = atp->resume_or_start(first_time);
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
                char err_msg[4096];
                snprintf(err_msg, sizeof(err_msg),
                    "Couldn't start or resume: %d", retval
                );
                report_result_error(*(atp->result), err_msg);
                request_schedule_cpus("start failed");
                continue;
            }
            if (atp->result->rr_sim_misses_deadline) {
                atp->once_ran_edf = true;
            }
            atp->run_interval_start_wall_time = now;
            app_started = now;
        }
        if (log_flags.cpu_sched_status) {
            msg_printf(atp->result->project, MSG_INFO,
                "[css] running %s (%s)",
                atp->result->name, atp->result->resources
            );
        }
        atp->scheduler_state = CPU_SCHED_SCHEDULED;
        swap_left -= atp->procinfo.swap_size;

#ifndef SIM
        // if we've been in this loop for > 10 secs,
        // break out of it and arrange for another schedule()
        // so that we don't miss GUI RPCs, heartbeats etc.
        //
        if (dtime() - now > MAX_STARTUP_TIME) {
            if (log_flags.cpu_sched_debug) {
                msg_printf(0, MSG_INFO,
                    "[cpu_sched_debug] app startup took %f secs", dtime() - now
                );
            }
            request_schedule_cpus("slow app startup");
            break;
        }
#endif

    }
    if (action) {
        set_client_state_dirty("enforce_cpu_schedule");
    }
    if (log_flags.cpu_sched_debug) {
        msg_printf(0, MSG_INFO, "[cpu_sched_debug] enforce_run_list: end");
    }
    if (coproc_start_deferred) {
        if (log_flags.cpu_sched_debug) {
            msg_printf(0, MSG_INFO,
                "[cpu_sched_debug] coproc quit pending, deferring start"
            );
        }
        request_schedule_cpus("coproc quit retry");
    }
    return action;
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

// find total resource shares of all CPU-intensive projects
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
double CLIENT_STATE::runnable_resource_share(int rsc_type) {
    double x = 0;
    for (unsigned int i=0; i<projects.size(); i++) {
        PROJECT* p = projects[i];
        if (p->non_cpu_intensive) continue;
        if (p->runnable(rsc_type)) {
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

// if there's not an active task for the result, make one
//
ACTIVE_TASK* CLIENT_STATE::get_task(RESULT* rp) {
    ACTIVE_TASK *atp = lookup_active_task_by_result(rp);
    if (!atp) {
        atp = new ACTIVE_TASK;
        if (!rp->project->app_test) {
            int retval = atp->get_free_slot(rp);
            if (retval) {
                delete atp;
                return NULL;
            }
        }
        atp->init(rp);
        active_tasks.active_tasks.push_back(atp);
    }
    return atp;
}

// called:
// - at startup (after get_host_info())
// - when general prefs have been parsed
// - when user_active changes
// NOTE: n_usable_cpus MUST BE 1 OR MORE; WE DIVIDE BY IT IN A COUPLE OF PLACES
//
void CLIENT_STATE::set_n_usable_cpus() {
    int ncpus_old = n_usable_cpus;

    // config file can say to act like host has N CPUs
    //
    static bool first = true;
    static int original_p_ncpus;
    if (first) {
        original_p_ncpus = host_info.p_ncpus;
        first = false;
    }
    if (cc_config.ncpus>0) {
        n_usable_cpus = cc_config.ncpus;
        host_info.p_ncpus = n_usable_cpus;  // use this in scheduler requests
    } else {
        host_info.p_ncpus = original_p_ncpus;
        n_usable_cpus = host_info.p_ncpus;
    }

    double p = global_prefs.max_ncpus_pct;
    if (!user_active && global_prefs.niu_max_ncpus_pct>=0) {
        p = global_prefs.niu_max_ncpus_pct;
    }
    if (p) {
        n_usable_cpus = (int)((n_usable_cpus * p)/100);
    }

    if (n_usable_cpus <= 0) {
        n_usable_cpus = 1;
    }

    if (initialized && n_usable_cpus != ncpus_old) {
        msg_printf(0, MSG_INFO,
            "Number of usable CPUs has changed from %d to %d.",
            ncpus_old, n_usable_cpus
        );
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
    if (dont_use_dcf) return;
    RESULT* rp = atp->result;
    double raw_ratio = atp->elapsed_time/rp->estimated_runtime_uncorrected();
    double adj_ratio = atp->elapsed_time/rp->estimated_runtime();
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
