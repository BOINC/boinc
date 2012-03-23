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
//  - create an ordered "run list" (schedule_cpus).
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
//          It's possible that include a bunch of jobs that can't run
//          because of memory limits,
//          even though there are other jobs that could run.
//      - add running jobs to the list
//          (in case they haven't finished time slice or checkpointed)
//      - sort the list according to "more_important()"
//      - shuffle the list to avoid starving multi-thread jobs
//
//  - scan through the resulting list,
//      running the jobs and preempting other jobs.
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
#include "win_util.h"
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

#include "client_msgs.h"
#include "log_flags.h"
#include "app.h"

#include "client_state.h"

using std::vector;
using std::list;

#ifdef __APPLE__
using std::isnan;
#endif

static double rec_sum;

#define DEADLINE_CUSHION    0
    // try to finish jobs this much in advance of their deadline

// used in schedule_cpus() to keep track of resources used
// by jobs tentatively scheduled so far
//
struct PROC_RESOURCES {
    int ncpus;
    double ncpus_used_st;   // #CPUs of GPU or single-thread jobs
    double ncpus_used_mt;   // #CPUs of multi-thread jobs
    COPROCS pr_coprocs;
    double ram_left;

    void init() {
        ncpus = gstate.ncpus;
        ncpus_used_st = 0;
        ncpus_used_mt = 0;
        pr_coprocs.clone(coprocs, false);
        pr_coprocs.clear_usage();
        ram_left = gstate.available_ram();
    }

    // should we stop scanning jobs?
    //
    inline bool stop_scan_cpu() {
        return ncpus_used_st >= ncpus;
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
        double wss;
        if (atp) {
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
            wss = atp->procinfo.working_set_size_smoothed;
        } else {
            wss = rp->avp->max_working_set_size;
        }
        if (wss > ram_left) return false;
        if (rp->schedule_backoff > gstate.now) return false;
        if (rp->uses_coprocs()) {
            if (gpu_suspend_reason) return false;
            if (sufficient_coprocs(*rp)) {
                return true;
            } else {
                return false;
            }
        } else if (rp->avp->avg_ncpus > 1) {
            return (ncpus_used_mt + rp->avp->avg_ncpus <= ncpus);
        } else {
            return (ncpus_used_st < ncpus);
        }
    }

    // we've decided to add this to the runnable list; update bookkeeping
    //
    void schedule(RESULT* rp, ACTIVE_TASK* atp, const char* description) {
        if (log_flags.cpu_sched_debug) {
            msg_printf(rp->project, MSG_INFO,
                "[cpu_sched_debug] scheduling %s (%s) (prio %f)",
                rp->name, description,
                rp->project->sched_priority
            );
        }
        reserve_coprocs(*rp);
        if (rp->uses_coprocs()) {
            ncpus_used_st += rp->avp->avg_ncpus;
        } else if (rp->avp->avg_ncpus > 1) {
            ncpus_used_mt += rp->avp->avg_ncpus;
        } else {
            ncpus_used_st += rp->avp->avg_ncpus;
        }
        double wss;
        if (atp) {
            wss = atp->procinfo.working_set_size_smoothed;
        } else {
            wss = rp->avp->max_working_set_size;
        }
        ram_left -= wss;

        adjust_rec_sched(rp);
    }

    bool sufficient_coprocs(RESULT& r) {
        double x;
        APP_VERSION& av = *r.avp;
        int rt = av.gpu_usage.rsc_type;
        if (!rt) return true;
        x = av.gpu_usage.usage;
        COPROC& cp = pr_coprocs.coprocs[rt];
        for (int i=0; i<cp.count; i++) {
            if (gpu_excluded(r.app, cp, i)) continue;
            if (cp.usage[i]+x <=1) return true;
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
        APP_VERSION& av = *r.avp;
        int rt = av.gpu_usage.rsc_type;
        if (!rt) return;
        COPROC& cp = pr_coprocs.coprocs[rt];
        x = av.gpu_usage.usage;
        for (int i=0; i<cp.count; i++) {
            if (gpu_excluded(r.app, cp, i)) continue;
            if (cp.usage[i]+x >1) continue;
            cp.usage[i] += x;
            break;
        }
        if (log_flags.cpu_sched_debug) {
            msg_printf(r.project, MSG_INFO,
                "[cpu_sched_debug] reserving %f of coproc %s", x, cp.type
            );
        }
    }
};

bool gpus_usable = true;

#ifndef SIM
// see whether there's been a change in coproc usability;
// if so set or clear "coproc_missing" flags and return true.
//
bool check_coprocs_usable() {
#ifdef _WIN32
    unsigned int i;
    bool new_usable = !is_remote_desktop();
    if (gpus_usable) {
        if (!new_usable) {
            gpus_usable = false;
            for (i=0; i<gstate.results.size(); i++) {
                RESULT* rp = gstate.results[i];
                if (rp->avp->gpu_usage.rsc_type) {
                    rp->coproc_missing = true;
                }
            }
            msg_printf(NULL, MSG_INFO,
                "GPUs have become unusable; disabling tasks"
            );
            return true;
        }
    } else {
        if (new_usable) {
            gpus_usable = true;
            for (i=0; i<gstate.results.size(); i++) {
                RESULT* rp = gstate.results[i];
                if (rp->avp->gpu_usage.rsc_type) {
                    rp->coproc_missing = false;
                }
            }
            msg_printf(NULL, MSG_INFO,
                "GPUs have become usable; enabling tasks"
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
RESULT* CLIENT_STATE::largest_debt_project_best_result() {
    PROJECT *best_project = NULL;
    double best_debt = 0;
    bool first = true;
    unsigned int i;

    for (i=0; i<projects.size(); i++) {
        PROJECT* p = projects[i];
        if (!p->next_runnable_result) continue;
        if (p->non_cpu_intensive) continue;
        if (first || p->sched_priority > best_debt) {
            first = false;
            best_project = p;
            best_debt = p->sched_priority;
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
        if (!rp->runnable()) continue;
        if (rp->non_cpu_intensive()) continue;
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
        if (rp->non_cpu_intensive()) continue;
        PROJECT* p = rp->project;

        // treat projects with DCF>90 as if they had deadline misses
        //
        if (!p->dont_use_dcf && p->duration_correction_factor < 90.0) {
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
        if (rp->estimated_time_remaining() < best_result->estimated_time_remaining()
            || (!best_atp && atp)
        ) {
            best_result = rp;
            best_atp = atp;
        }
    }
    if (!best_result) return NULL;

    if (log_flags.cpu_sched_debug) {
        msg_printf(best_result->project, MSG_INFO,
            "[cpu_sched_debug] earliest deadline: %.0f %s",
            best_result->report_deadline, best_result->name
        );
    }

    return best_result;
}

void CLIENT_STATE::reset_debt_accounting() {
    unsigned int i;
    for (i=0; i<projects.size(); i++) {
        PROJECT* p = projects[i];
        for (int j=0; j<coprocs.n_rsc; j++) {
            p->rsc_pwf[j].reset_debt_accounting();
        }
    }
    for (int j=0; j<coprocs.n_rsc; j++) {
        rsc_work_fetch[j].reset_debt_accounting();
    }
    debt_interval_start = now;
}

// update REC (recent estimated credit)
//
static void update_rec() {
    double f = gstate.host_info.p_fpops;

    for (unsigned int i=0; i<gstate.projects.size(); i++) {
        PROJECT* p = gstate.projects[i];

        double x = 0;
        for (int j=0; j<coprocs.n_rsc; j++) {
            x += p->rsc_pwf[j].secs_this_debt_interval * f * rsc_work_fetch[j].relative_speed;
        }
        x *= COBBLESTONE_SCALE;
        double old = p->pwf.rec;

        // start averages at zero
        //
        if (p->pwf.rec_time == 0) {
            p->pwf.rec_time = gstate.debt_interval_start;
        }

        update_average(
            gstate.now,
            gstate.debt_interval_start,
            x,
            config.rec_half_life,
            p->pwf.rec,
            p->pwf.rec_time
        );

        if (log_flags.priority_debug) {
            double dt = gstate.now - gstate.debt_interval_start;
            msg_printf(p, MSG_INFO,
                "[prio] recent est credit: %.2fG in %.2f sec, %f + %f ->%f",
                x, dt, old, p->pwf.rec-old, p->pwf.rec
            );
        }
    }
}

static double peak_flops(APP_VERSION* avp) {
    double f = gstate.host_info.p_fpops;
    double x = f * avp->avg_ncpus;
    int rt = avp->gpu_usage.rsc_type;
    if (rt) {
        x += f * avp->gpu_usage.usage * rsc_work_fetch[rt].relative_speed;
    }
    return x;
}

double total_peak_flops() {
    static bool first=true;
    static double tpf;
    if (first) {
        first = false;
        tpf = gstate.host_info.p_fpops * gstate.ncpus;
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
    p->pwf.rec_temp += peak_flops(rp->avp)/total_peak_flops() * rec_sum/24;
    p->compute_sched_priority();
}

// make this a variable so simulator can change it
//
double debt_adjust_period = DEBT_ADJUST_PERIOD;

// adjust project REC
//
void CLIENT_STATE::adjust_rec() {
    unsigned int i;
    double elapsed_time = now - debt_interval_start;

    // If the elapsed time is more than 2*DEBT_ADJUST_PERIOD
    // it must be because the host was suspended for a long time.
    // In this case, ignore the last period
    //
    if (elapsed_time > 2*debt_adjust_period || elapsed_time < 0) {
        if (log_flags.priority_debug) {
            msg_printf(NULL, MSG_INFO,
                "[priority] adjust_rec: elapsed time (%d) longer than sched enforce period(%d).  Ignoring this period.",
                (int)elapsed_time, (int)debt_adjust_period
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

    // total up how many instance-seconds projects got
    //
    for (i=0; i<active_tasks.active_tasks.size(); i++) {
        ACTIVE_TASK* atp = active_tasks.active_tasks[i];
        if (atp->scheduler_state != CPU_SCHED_SCHEDULED) continue;
        PROJECT* p = atp->result->project;
        if (p->non_cpu_intensive) continue;
        work_fetch.accumulate_inst_sec(atp, elapsed_time);
    }

    update_rec();

    reset_debt_accounting();
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
    if (elapsed_time >= CPU_SCHED_PERIOD) {
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
            if (p->deadlines_missed(rp->avp->rsc_type())) {
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
        proc_rsc.schedule(rp, atp, "coprocessor job, EDF");
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
        proc_rsc.schedule(rp, atp, "coprocessor job, FIFO");
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

    // do round-robin simulation to find what results miss deadline
    //
    rr_simulation();
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
        atp = lookup_active_task_by_result(rp);
        if (!proc_rsc.can_schedule(rp, atp)) continue;
        proc_rsc.schedule(rp, atp, "CPU job, EDF");
        rp->project->rsc_pwf[0].deadlines_missed_copy--;
        rp->edf_scheduled = true;
        run_list.push_back(rp);
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
        if (!proc_rsc.can_schedule(rp, atp)) continue;
        proc_rsc.schedule(rp, atp, "CPU job, priority order");
        run_list.push_back(rp);
    }

}

static inline bool in_run_list(vector<RESULT*>& run_list, ACTIVE_TASK* atp) {
    for (unsigned int i=0; i<run_list.size(); i++) {
        if (atp->result == run_list[i]) return true;
    }
    return false;
}

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
        if (cpus_used >= gstate.ncpus) break;
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

    // favor jobs selected first by schedule_cpus()
    // (e.g., because their project has high STD)
    //
    if (r0->seqno < r1->seqno) return true;
    if (r0->seqno > r1->seqno) return false;

    // tie breaker
    return (r0 < r1);
}

static void print_job_list(vector<RESULT*>& jobs) {
    for (unsigned int i=0; i<jobs.size(); i++) {
        RESULT* rp = jobs[i];
        msg_printf(rp->project, MSG_INFO,
            "[cpu_sched_debug] %d: %s (MD: %s; UTS: %s)",
            i, rp->name,
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
        if (atp->result->uses_coprocs() && gpu_suspend_reason) continue;
        if (atp->result->non_cpu_intensive()) continue;
        if (atp->scheduler_state != CPU_SCHED_SCHEDULED) continue;
        if (finished_time_slice(atp)) continue;
        atp->result->unfinished_time_slice = true;
        if (in_run_list(run_list, atp)) continue;
        run_list.push_back(atp->result);
        atp->result->seqno = seqno;
    }
}

////////// Coprocessor scheduling ////////////////
//
// theory of operations:
//
// Jobs can use one or more integral instances, or a fractional instance
//
// RESULT::coproc_indices
//    for a running job, the coprocessor instances it's using
// COPROC::pending_usage[]: for each instance, its usage by running jobs
// CORPOC::usage[]: for each instance, its usage
//
// enforce_schedule() calls assign_coprocs(),
// which assigns coproc instances to scheduled jobs,
// and prunes jobs for which we can't make an assignment
// (the job list is in order of decreasing priority)
//
// assign_coprocs():
//     clear usage and pending_usage of all instances
//     for each running job J
//         increment pending_usage for the instances assigned to J
//     for each scheduled job J
//         if J is running
//             if J's assignment fits
//                 confirm assignment: dev pending_usage, inc usage
//             else
//                 prune J
//         else
//             if J.usage is fractional
//                look for an instance that's already fractionally assigned
//                if that fails, look for a free instance
//                if that fails, prune J
//             else
//                if there are enough instances with usage=0
//                    assign instances with pending_usage = usage = 0
//                        (avoid preempting running jobs)
//                    if need more, assign instances with usage = 0
//                else
//                    prune J

static inline void increment_pending_usage(
    RESULT* rp, double usage, COPROC* cp
) {
    double x = (usage<1)?usage:1;
    for (int i=0; i<usage; i++) {
        int j = rp->coproc_indices[i];
        cp->pending_usage[j] += x;
        if (cp->pending_usage[j] > 1) {
            if (log_flags.coproc_debug) {
                msg_printf(rp->project, MSG_INFO,
                    "[coproc] huh? %s %d %s pending usage > 1",
                    cp->type, i, rp->name
                );
            }
        }
    }
}

// check the GPU assignment for a currently-running app.
// Note: don't check available RAM.
// It may not be known (e.g. NVIDIA) and in any case,
// if the app is still running, it has enough RAM
//
static inline bool current_assignment_ok(
    RESULT* rp, double usage, COPROC* cp, bool& defer_sched
) {
    defer_sched = false;
    double x = (usage<1)?usage:1;
    for (int i=0; i<usage; i++) {
        int j = rp->coproc_indices[i];
        if (cp->usage[j] + x > 1) {
            if (log_flags.coproc_debug) {
                msg_printf(rp->project, MSG_INFO,
                    "[coproc] %s device %d already assigned: task %s",
                    cp->type, j, rp->name
                );
            }
            return false;
        }
    }
    return true;
}

static inline void confirm_current_assignment(
    RESULT* rp, double usage, COPROC* cp
) {
    double x = (usage<1)?usage:1;
    for (int i=0; i<usage; i++) {
        int j = rp->coproc_indices[i];
        cp->usage[j] +=x;
        cp->pending_usage[j] -=x;
        if (log_flags.coproc_debug) {
            msg_printf(rp->project, MSG_INFO,
                "[coproc] %s instance %d: confirming for %s",
                cp->type, j, rp->name
            );
        }
#if DEFER_ON_GPU_AVAIL_RAM
        cp->available_ram_temp[j] -= rp->avp->gpu_ram;
#endif
    }
}

static inline bool get_fractional_assignment(
    RESULT* rp, double usage, COPROC* cp, bool& defer_sched
) {
    int i;
    defer_sched = false;

    // try to assign an instance that's already fractionally assigned
    //
    for (i=0; i<cp->count; i++) {
        if (gpu_excluded(rp->app, *cp, i)) {
            continue;
        }
        if ((cp->usage[i] || cp->pending_usage[i])
            && (cp->usage[i] + cp->pending_usage[i] + usage <= 1)
        ) {
#if DEFER_ON_GPU_AVAIL_RAM
            if (rp->avp->gpu_ram > cp->available_ram_temp[i]) {
                defer_sched = true;
                continue;
            }
            cp->available_ram_temp[i] -= rp->avp->gpu_ram;
#endif
            rp->coproc_indices[0] = i;
            cp->usage[i] += usage;
            if (log_flags.coproc_debug) {
                msg_printf(rp->project, MSG_INFO,
                    "[coproc] Assigning %f of %s instance %d to %s",
                    usage, cp->type, i, rp->name
                );
            }
            return true;
        }
    }

    // failing that, assign an unreserved instance
    //
    for (i=0; i<cp->count; i++) {
        if (gpu_excluded(rp->app, *cp, i)) {
            continue;
        }
        if (!cp->usage[i]) {
#if DEFER_ON_GPU_AVAIL_RAM
            if (rp->avp->gpu_ram > cp->available_ram_temp[i]) {
                defer_sched = true;
                continue;
            }
            cp->available_ram_temp[i] -= rp->avp->gpu_ram;
#endif
            rp->coproc_indices[0] = i;
            cp->usage[i] += usage;
            if (log_flags.coproc_debug) {
                msg_printf(rp->project, MSG_INFO,
                    "[coproc] Assigning %f of %s free instance %d to %s",
                    usage, cp->type, i, rp->name
                );
            }
            return true;
        }
    }
    if (log_flags.coproc_debug) {
        msg_printf(rp->project, MSG_INFO,
            "[coproc] Insufficient %s for %s: need %f",
            cp->type, rp->name, usage
        );
    }

    return false;
}

static inline bool get_integer_assignment(
    RESULT* rp, double usage, COPROC* cp, bool& defer_sched
) {
    int i;
    defer_sched = false;

    // make sure we have enough free instances
    //
    int nfree = 0;
    for (i=0; i<cp->count; i++) {
        if (gpu_excluded(rp->app, *cp, i)) {
            continue;
        }
        if (!cp->usage[i]) {
#if DEFER_ON_GPU_AVAIL_RAM
            if (rp->avp->gpu_ram > cp->available_ram_temp[i]) {
                defer_sched = true;
                if (log_flags.coproc_debug) {
                    msg_printf(rp->project, MSG_INFO,
                        "[coproc]  task %s needs %.0fMB RAM, %s GPU %d has %.0fMB available",
                        rp->name, rp->avp->gpu_ram/MEGA, cp->type, i, cp->available_ram_temp[i]/MEGA
                    );
                }
                continue;
            };
#endif
            nfree++;
        }
    }
    if (nfree < usage) {
        if (log_flags.coproc_debug) {
            msg_printf(rp->project, MSG_INFO,
                "[coproc] Insufficient %s for %s; need %d, available %d",
                cp->type, rp->name, (int)usage, nfree
            );
            if (defer_sched) {
                msg_printf(rp->project, MSG_INFO,
                    "[coproc] some instances lack available memory"
                );
            }
        }
        return false;
    }

    int n = 0;

    // assign non-pending instances first

    for (i=0; i<cp->count; i++) {
        if (gpu_excluded(rp->app, *cp, i)) {
            continue;
        }
        if (!cp->usage[i]
            && !cp->pending_usage[i]
#if DEFER_ON_GPU_AVAIL_RAM
            && (rp->avp->gpu_ram <= cp->available_ram_temp[i])
#endif
        ) {
            cp->usage[i] = 1;
#if DEFER_ON_GPU_AVAIL_RAM
            cp->available_ram_temp[i] -= rp->avp->gpu_ram;
#endif
            rp->coproc_indices[n++] = i;
            if (log_flags.coproc_debug) {
                msg_printf(rp->project, MSG_INFO,
                    "[coproc] Assigning %s instance %d to %s",
                    cp->type, i, rp->name
                );
            }
            if (n == usage) return true;
        }
    }

    // if needed, assign pending instances

    for (i=0; i<cp->count; i++) {
        if (gpu_excluded(rp->app, *cp, i)) {
            continue;
        }
        if (!cp->usage[i]
#if DEFER_ON_GPU_AVAIL_RAM
            && (rp->avp->gpu_ram <= cp->available_ram_temp[i])
#endif
        ) {
            cp->usage[i] = 1;
#if DEFER_ON_GPU_AVAIL_RAM
            cp->available_ram_temp[i] -= rp->avp->gpu_ram;
#endif
            rp->coproc_indices[n++] = i;
            if (log_flags.coproc_debug) {
                msg_printf(rp->project, MSG_INFO,
                    "[coproc] Assigning %s pending instance %d to %s",
                    cp->type, i, rp->name
                );
            }
            if (n == usage) return true;
        }
    }
    if (log_flags.coproc_debug) {
        msg_printf(rp->project, MSG_INFO,
            "[coproc] huh??? ran out of %s instances for %s",
            cp->type, rp->name
        );
    }
    return false;
}

static inline void mark_as_defer_sched(RESULT* rp) {
    int i = rp->avp->gpu_usage.rsc_type;
    if (i) {
        rp->project->rsc_defer_sched[i] = true;
    }
    rp->schedule_backoff = gstate.now + 300; // try again in 5 minutes
    gstate.request_schedule_cpus("insufficient GPU RAM");
}

#if DEFER_ON_GPU_AVAIL_RAM
static void copy_available_ram(COPROC& cp, const char* name) {
    int rt = rsc_index(name);
    if (rt > 0) {
        for (int i=0; i<MAX_COPROC_INSTANCES; i++) {
            coprocs.coprocs[rt].available_ram_temp[i] = cp.available_ram;
        }
    }
}
#endif

static inline void assign_coprocs(vector<RESULT*>& jobs) {
    unsigned int i;
    COPROC* cp;
    double usage;

    coprocs.clear_usage();
#if DEFER_ON_GPU_AVAIL_RAM
    if (coprocs.have_nvidia()) {
        copy_available_ram(coprocs.nvidia, GPU_TYPE_NVIDIA);
    }
    if (coprocs.have_ati()) {
        copy_available_ram(coprocs.ati, GPU_TYPE_ATI);
    }
#endif

    // fill in pending usage
    //
    for (i=0; i<jobs.size(); i++) {
        RESULT* rp = jobs[i];
        APP_VERSION* avp = rp->avp;
        int rt = avp->gpu_usage.rsc_type;
        if (rt) {
            usage = avp->gpu_usage.usage;
            cp = &coprocs.coprocs[rt];
        } else {
            continue;
        }
        ACTIVE_TASK* atp = gstate.lookup_active_task_by_result(rp);
        if (!atp) continue;
        if (atp->task_state() != PROCESS_EXECUTING) continue;
        increment_pending_usage(rp, usage, cp);
    }

    vector<RESULT*>::iterator job_iter;
    job_iter = jobs.begin();
    while (job_iter != jobs.end()) {
        RESULT* rp = *job_iter;
        APP_VERSION* avp = rp->avp;
        int rt = avp->gpu_usage.rsc_type;
        if (rt) {
            usage = avp->gpu_usage.usage;
            cp = &coprocs.coprocs[rt];
        } else {
            job_iter++;
            continue;
        }

        ACTIVE_TASK* atp = gstate.lookup_active_task_by_result(rp);
        bool defer_sched;
        if (atp && atp->task_state() == PROCESS_EXECUTING) {
            if (current_assignment_ok(rp, usage, cp, defer_sched)) {
                confirm_current_assignment(rp, usage, cp);
                job_iter++;
            } else {
                if (defer_sched) {
                    mark_as_defer_sched(rp);
                }
                job_iter = jobs.erase(job_iter);
            }
        } else {
            if (usage < 1) {
                if (get_fractional_assignment(rp, usage, cp, defer_sched)) {
                    job_iter++;
                } else {
                    if (defer_sched) {
                        mark_as_defer_sched(rp);
                    }
                    job_iter = jobs.erase(job_iter);
                }
            } else {
                if (get_integer_assignment(rp, usage, cp, defer_sched)) {
                    job_iter++;
                } else {
                    if (defer_sched) {
                        mark_as_defer_sched(rp);
                    }
                    job_iter = jobs.erase(job_iter);
                }
            }
        }
    }

#if 0
    // enforce "don't use GPUs while active" pref in NVIDIA case;
    // it applies only to GPUs running a graphics app
    //
    if (gstate.host_info.coprocs.nvidia.count && gstate.user_active && !gstate.global_prefs.run_gpu_if_user_active) {
        job_iter = jobs.begin();
        while (job_iter != jobs.end()) {
            RESULT* rp = *job_iter;
            if (!rp->avp->ncudas) {
                job_iter++;
                continue;
            }
            ACTIVE_TASK* atp = gstate.lookup_active_task_by_result(rp);
            bool some_gpu_busy = false;
            for (i=0; i<rp->avp->ncudas; i++) {
                int dev = atp->coproc_indices[i];
                if (gstate.host_info.coprocs.cuda.running_graphics_app[dev]) {
                    some_gpu_busy = true;
                    break;
                }
            }
            if (some_gpu_busy) {
                job_iter = jobs.erase(job_iter);
            } else {
                job_iter++;
            }
        }
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
bool CLIENT_STATE::enforce_run_list(vector<RESULT*>& run_list) {
    unsigned int i;
    vector<ACTIVE_TASK*> preemptable_tasks;
    int retval;
    double ncpus_used=0;
    ACTIVE_TASK* atp;

    bool action = false;

#ifndef SIM
    // check whether GPUs are usable
    //
    if (check_coprocs_usable()) {
        request_schedule_cpus("GPU usability change");
        return true;
    }
#endif

    if (log_flags.cpu_sched_debug) {
        msg_printf(0, MSG_INFO, "[cpu_sched_debug] enforce_schedule(): start");
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

    // append running jobs not done with time slice to the to-run list
    //
    append_unfinished_time_slice(run_list);

    // sort to-run list by decreasing importance
    //
    std::sort(
        run_list.begin(),
        run_list.end(),
        more_important
    );

    promote_multi_thread_jobs(run_list);

    if (log_flags.cpu_sched_debug) {
        msg_printf(0, MSG_INFO, "[cpu_sched_debug] final job list:");
        print_job_list(run_list);
    }

    double ram_left = available_ram();
    double swap_left = (global_prefs.vm_max_used_frac)*host_info.m_swap;

    if (log_flags.mem_usage_debug) {
        msg_printf(0, MSG_INFO,
            "[mem_usage] enforce: available RAM %.2fMB swap %.2fMB",
            ram_left/MEGA, swap_left/MEGA
        );
    }

    for (i=0; i<projects.size(); i++) {
        for (int j=1; j<coprocs.n_rsc; j++) {
            projects[i]->rsc_defer_sched[j] = false;
        }
    }

    // schedule non-CPU-intensive tasks,
    // and look for backed-off GPU jobs
    //
    for (i=0; i<results.size(); i++) {
        RESULT* rp = results[i];
        if (rp->non_cpu_intensive() && rp->runnable()) {
            atp = get_task(rp);
            atp->next_scheduler_state = CPU_SCHED_SCHEDULED;
            ram_left -= atp->procinfo.working_set_size_smoothed;
            swap_left -= atp->procinfo.swap_size;
        }
        if (rp->schedule_backoff) {
            if (rp->schedule_backoff > gstate.now) {
                int r = rp->avp->gpu_usage.rsc_type;
                if (r) {
                    rp->project->rsc_defer_sched[r] = true;
                }
            } else {
                rp->schedule_backoff = 0;
                request_schedule_cpus("schedule backoff finished");
            }
        }
    }

    // assign coprocessors to coproc jobs,
    // and prune those that can't be assigned
    //
    assign_coprocs(run_list);
    bool scheduled_mt = false;

    // prune jobs that don't fit in RAM or that exceed CPU usage limits.
    // Mark the rest as SCHEDULED
    //
    for (i=0; i<run_list.size(); i++) {
        RESULT* rp = run_list[i];
        atp = lookup_active_task_by_result(rp);

        // if we're already using all the CPUs,
        // don't allow additional CPU jobs;
        // allow GPU jobs if the resulting CPU load is at most ncpus+1
        //
        if (ncpus_used >= ncpus) {
            if (rp->uses_coprocs()) {
                if (ncpus_used + rp->avp->avg_ncpus > ncpus+1) {
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
                        ncpus_used, ncpus,
                        rp->name
                    );
                }
            }
            continue;
        }

        // don't overcommit CPUs if a MT job is scheduled
        //
        if (scheduled_mt || (rp->avp->avg_ncpus > 1)) {
            if (ncpus_used + rp->avp->avg_ncpus > ncpus) {
                if (log_flags.cpu_sched_debug) {
                    msg_printf(rp->project, MSG_INFO,
                        "[cpu_sched_debug] avoid MT overcommit: skipping %s",
                        rp->name
                    );
                }
                continue;
            }
        }

        double wss = 0;
        if (atp) {
            atp->too_large = false;
            wss = atp->procinfo.working_set_size_smoothed;
        } else {
            wss = rp->avp->max_working_set_size;
        }
        if (wss > ram_left) {
            if (atp) {
                atp->too_large = true;
            }
            if (log_flags.mem_usage_debug) {
                msg_printf(rp->project, MSG_INFO,
                    "[mem_usage] enforce: result %s can't run, too big %.2fMB > %.2fMB",
                    rp->name,  wss/MEGA, ram_left/MEGA
                );
            }
            continue;
        }

        if (log_flags.cpu_sched_debug) {
            msg_printf(rp->project, MSG_INFO,
                "[cpu_sched_debug] scheduling %s", rp->name
            );
        }

        // We've decided to run this job; create an ACTIVE_TASK if needed.
        //
        if (!atp) {
            atp = get_task(rp);
        }

        if (rp->avp->avg_ncpus > 1) {
            scheduled_mt = true;
        }
        ncpus_used += rp->avp->avg_ncpus;
        atp->next_scheduler_state = CPU_SCHED_SCHEDULED;
        ram_left -= wss;
    }

    if (log_flags.cpu_sched_debug && ncpus_used < ncpus) {
        msg_printf(0, MSG_INFO, "[cpu_sched_debug] using %.2f out of %d CPUs",
            ncpus_used, ncpus
        );
        if (ncpus_used < ncpus) {
            request_work_fetch("CPUs idle");
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
                report_result_error(
                    *(atp->result), "Couldn't start or resume: %d", retval
                );
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
        msg_printf(0, MSG_INFO, "[cpu_sched_debug] enforce_schedule: end");
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
        atp->get_free_slot(rp);
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
            "[task] result state=%s for %s from %s",
            result_state_name(val), name, where
        );
    }
}

// called at startup (after get_host_info())
// and when general prefs have been parsed.
// NOTE: GSTATE.NCPUS MUST BE 1 OR MORE; WE DIVIDE BY IT IN A COUPLE OF PLACES
//
void CLIENT_STATE::set_ncpus() {
    int ncpus_old = ncpus;

    if (config.ncpus>0) {
        ncpus = config.ncpus;
        host_info.p_ncpus = ncpus;
    } else if (host_info.p_ncpus>0) {
        ncpus = host_info.p_ncpus;
    } else {
        ncpus = 1;
    }

    if (global_prefs.max_ncpus_pct) {
        ncpus = (int)((ncpus * global_prefs.max_ncpus_pct)/100);
        if (ncpus == 0) ncpus = 1;
    } else if (global_prefs.max_ncpus && global_prefs.max_ncpus < ncpus) {
        ncpus = global_prefs.max_ncpus;
    }

    if (initialized && ncpus != ncpus_old) {
        msg_printf(0, MSG_INFO,
            "Number of usable CPUs has changed from %d to %d.",
            ncpus_old, ncpus
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
    double raw_ratio = atp->elapsed_time/rp->estimated_duration_uncorrected();
    double adj_ratio = atp->elapsed_time/rp->estimated_duration();
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

