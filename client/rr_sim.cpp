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

// Simulate the processing of the current workload
// (include jobs that are downloading)
// with weighted round-robin (WRR) scheduling.
//
// Outputs are changes to global state:
// - deadline misses (per-project count, per-result flag)
//      Deadline misses are not counted for tasks
//      that are too large to run in RAM right now.
// - number of runnable jobs per project
//      p.pwf.n_runnable_jobs
// - for each resource type (in RSC_WORK_FETCH):
//    - shortfall
//    - nidle_now: # of idle instances
//    - sim_excluded_instances: bitmap of instances idle because of exclusions
//
// For coprocessors, we saturate the resource if possible;
// i.e. with 2 GPUs, we'd let a 1-GPU app and a 2-GPU app run together.
// Otherwise, there'd be the possibility of computing
// a nonzero shortfall inappropriately.
//

#include "cpp.h"

#ifdef _WIN32
#include "boinc_win.h"
#else
#include "config.h"
#endif

#include "client_msgs.h"
#include "client_state.h"
#include "coproc.h"
#include "project.h"
#include "result.h"

using std::vector;

// set "nused" bits of the source bitmap in the dest bitmap
//
static inline void set_bits(
    COPROC_INSTANCE_BITMAP src, double nused, COPROC_INSTANCE_BITMAP& dst
) {
    // if all bits are already set, we're done
    //
    if ((src&dst) == src) return;
    COPROC_INSTANCE_BITMAP bit = 1;
    for (int i=0; i<MAX_COPROC_INSTANCES; i++) {
        if (nused <= 0) break;
        if (bit & src) {
            dst |= bit;
            nused -= 1;
        }
        bit <<= 1;
    }
}

// this is here (rather than rr_sim.h) because its inline functions
// refer to RESULT
//
struct RR_SIM {
    vector<RESULT*> active_jobs;

    inline void activate(RESULT* rp) {
        PROJECT* p = rp->project;
        active_jobs.push_back(rp);
        int rt = rp->resource_usage.rsc_type;

        // if this is a GPU app and GPU computing is suspended,
        // don't count its CPU usage.
        // That way we'll fetch more CPU work if needed.
        //
        if (!rt || !gpu_suspend_reason) {
            rsc_work_fetch[0].sim_nused += rp->resource_usage.avg_ncpus;
            p->rsc_pwf[0].sim_nused += rp->resource_usage.avg_ncpus;
        }

        if (rt) {
            rsc_work_fetch[rt].sim_nused += rp->resource_usage.coproc_usage;
            p->rsc_pwf[rt].sim_nused += rp->resource_usage.coproc_usage;
            if (rsc_work_fetch[rt].has_exclusions) {
                set_bits(
                    rp->app->non_excluded_instances[rt],
                    p->rsc_pwf[rt].nused_total,
                    rsc_work_fetch[rt].sim_used_instances
                );
#if 0
                msg_printf(p, MSG_INFO, "%d non_excl %d used %d",
                    rt,
                    rp->app->non_excluded_instances[rt],
                    rsc_work_fetch[rt].sim_used_instances
                );
#endif
            }
        }
        if (have_max_concurrent) {
            max_concurrent_inc(rp);
        }
    }

    void init_pending_lists();
    void pick_jobs_to_run(double reltime);
    void simulate();

    RR_SIM() {}
    ~RR_SIM() {}

};

// estimate the long-term FLOPS that this job will get
// (counting unavailability)
//
void set_rrsim_flops(RESULT* rp) {
    // For coproc jobs, use app version estimate
    //
    if (rp->uses_gpu()) {
        rp->rrsim_flops = rp->resource_usage.flops * gstate.overall_gpu_frac();
    } else if (rp->avp->needs_network) {
        rp->rrsim_flops =  rp->resource_usage.flops * gstate.overall_cpu_and_network_frac();
    } else {
        rp->rrsim_flops =  rp->resource_usage.flops * gstate.overall_cpu_frac();
    }
    if (rp->rrsim_flops == 0) {
        rp->rrsim_flops = 1e6;      // just in case
    }
}

void print_deadline_misses() {
    unsigned int i;
    RESULT* rp;
    PROJECT* p;
    for (i=0; i<gstate.results.size(); i++){
        rp = gstate.results[i];
        if (rp->rr_sim_misses_deadline) {
            msg_printf(rp->project, MSG_INFO,
                "[rr_sim] Result %s projected to miss deadline.",
                rp->name
            );
        }
    }
    for (i=0; i<gstate.projects.size(); i++) {
        p = gstate.projects[i];
        for (int j=0; j<coprocs.n_rsc; j++) {
            if (p->rsc_pwf[j].deadlines_missed) {
                msg_printf(p, MSG_INFO,
                    "[rr_sim] Project has %d projected %s deadline misses",
                    p->rsc_pwf[j].deadlines_missed,
                    rsc_name_long(j)
                );
            }
        }
    }
}

// Decide what jobs to include in the simulation;
// build the "pending" lists for each (project, processor type) pair.
// NOTE: "results" is sorted by increasing arrival time.
//
void RR_SIM::init_pending_lists() {
    for (unsigned int i=0; i<gstate.projects.size(); i++) {
        PROJECT* p = gstate.projects[i];
        for (int j=0; j<coprocs.n_rsc; j++) {
            p->rsc_pwf[j].pending.clear();
            p->rsc_pwf[j].queue_est = 0;
        }
    }
    for (unsigned int i=0; i<gstate.results.size(); i++) {
        RESULT* rp = gstate.results[i];
        rp->rr_sim_misses_deadline = false;
        rp->already_selected = false;
        if (!rp->nearly_runnable()) continue;
        if (rp->some_download_stalled()) continue;
        if (rp->always_run()) continue;
        rp->rrsim_flops_left = rp->estimated_flops_remaining();

        //if (rp->rrsim_flops_left <= 0) continue;
            // job may have fraction_done=1 but not be done;
            // if it's past its deadline, we need to mark it as such

        PROJECT* p = rp->project;
        p->pwf.n_runnable_jobs++;
        p->rsc_pwf[0].nused_total += rp->resource_usage.avg_ncpus;
        set_rrsim_flops(rp);
        int rt = rp->resource_usage.rsc_type;
        if (rt) {
            p->rsc_pwf[rt].nused_total += rp->resource_usage.coproc_usage;
            p->rsc_pwf[rt].n_runnable_jobs++;
            p->rsc_pwf[rt].queue_est += rp->rrsim_flops_left/rp->rrsim_flops;
        }
        p->rsc_pwf[rt].pending.push_back(rp);
        rp->rrsim_done = false;
    }
}

// Pick jobs to run from pending lists, putting them in "active_jobs" list.
// Approximate what the job scheduler would do:
// pick a job from the project P with highest scheduling priority,
// then adjust P's scheduling priority.
//
// This is called:
// - at the start of the simulation
//      It will pick jobs to use all resources
// - each time a job finishes in the simulation
//      It will generally pick one new job to use the resource just freed
//
void RR_SIM::pick_jobs_to_run(double reltime) {
    if (log_flags.rr_simulation) {
        msg_printf(NULL, MSG_INFO, "pick_jobs_to_run() start");
    }
    active_jobs.clear();

    if (have_max_concurrent) {
        max_concurrent_init();
    }

    // save and restore rec_temp
    //
    for (unsigned int i=0; i<gstate.projects.size(); i++) {
        PROJECT* p = gstate.projects[i];
        p->pwf.rec_temp_save = p->pwf.rec_temp;
        p->pwf.at_max_concurrent_limit = false;
    }

    rsc_work_fetch[0].sim_nused = 0;

    // loop over resource types; do the GPUs first
    //
    for (int rt=coprocs.n_rsc-1; rt>=0; rt--) {
        vector<PROJECT*> project_heap;

        if (rt) rsc_work_fetch[rt].sim_nused = 0;

        // Make a heap of projects with runnable jobs for this resource,
        // ordered by scheduling priority.
        // Clear usage counts.
        // Initialize iterators to the pending list of each project.
        //
        for (unsigned int i=0; i<gstate.projects.size(); i++) {
            PROJECT* p = gstate.projects[i];
            RSC_PROJECT_WORK_FETCH& rsc_pwf = p->rsc_pwf[rt];
            size_t s = rsc_pwf.pending.size();
#if 0
            if (log_flags.rrsim_detail) {
                msg_printf(p, MSG_INFO, "[rr_sim] %u jobs for rsc %zu", s, rt);
            }
#endif
            if (s == 0) continue;
            rsc_pwf.pending_iter = rsc_pwf.pending.begin();
            rsc_pwf.sim_nused = 0;
            p->pwf.rec_temp = p->pwf.rec;
            p->compute_sched_priority();
            project_heap.push_back(p);
        }
        make_heap(project_heap.begin(), project_heap.end());

        // Loop over jobs.
        // Keep going until the resource is saturated or there are no more jobs.
        //
        while (1) {
            if (project_heap.empty()) break;

            // p is the highest-priority project with work for this resource
            //
            PROJECT* p = project_heap.front();
            RSC_PROJECT_WORK_FETCH& rsc_pwf = p->rsc_pwf[rt];
            RESULT* rp = *rsc_pwf.pending_iter;

            // garbage-collect jobs that already completed in our simulation
            // (this is just a handy place to do this)
            //
            if (rp->rrsim_done) {
                rsc_pwf.pending_iter = rsc_pwf.pending.erase(
                    rsc_pwf.pending_iter
                );
            } else {
                // add job to active_jobs list, and adjust project priority
                //
                activate(rp);
                adjust_rec_sched(rp);
                if (log_flags.rrsim_detail && !rp->already_selected) {
                    char buf[256];
                    rp->rsc_string(buf, sizeof(buf));
                    msg_printf(rp->project, MSG_INFO,
                        "[rr_sim_detail] %.2f: starting %s (%s) (%.2fG/%.2fG)",
                        reltime, rp->name, buf, rp->rrsim_flops_left/1e9,
                        rp->rrsim_flops/1e9
                    );
                    rp->already_selected = true;
                }

                // Check if project is at a max_concurrent limit
                //
                if (have_max_concurrent) {
                    switch (max_concurrent_exceeded(rp)) {
                    case CONCURRENT_LIMIT_PROJECT:
                        rsc_pwf.last_mc_limit_reltime = reltime;
                        p->pwf.at_max_concurrent_limit = true;
                        if (log_flags.rr_simulation) {
                            msg_printf(p, MSG_INFO,
                                "[rr_sim] at project max concurrent: t %f",
                                reltime
                            );
                        }
                        break;
                    case CONCURRENT_LIMIT_APP:
                        // no more jobs for this project/app
                        //
                        p->pwf.at_max_concurrent_limit = true;
                        rsc_pwf.last_mc_limit_reltime = reltime;
                        if (log_flags.rr_simulation) {
                            msg_printf(p, MSG_INFO,
                                "[rr_sim] at app max concurrent for %s; t %f",
                                rp->app->name, reltime
                            );
                        }
                        break;
                    }
                }

                // check whether resource is saturated
                //
                if (rt) {
                    if (rsc_work_fetch[rt].sim_nused >= coprocs.coprocs[rt].count) {
                        break;
                    }

                    // if a GPU isn't saturated but this project is using
                    // its max given exclusions, remove it from project heap
                    //
                    if (rsc_pwf.sim_nused >= coprocs.coprocs[rt].count - p->rsc_pwf[rt].ncoprocs_excluded) {
                        pop_heap(project_heap.begin(), project_heap.end());
                        project_heap.pop_back();
                        continue;
                    }
                } else {
                    if (rsc_work_fetch[rt].sim_nused >= gstate.n_usable_cpus) break;
                }
                ++rsc_pwf.pending_iter;
            }

            if (rsc_pwf.pending_iter == rsc_pwf.pending.end()
                || p->pwf.at_max_concurrent_limit
            ) {
                // if this project now has no more jobs for the resource,
                // remove it from the project heap
                //
                pop_heap(project_heap.begin(), project_heap.end());
                project_heap.pop_back();
            } else if (!rp->rrsim_done) {
                // Otherwise reshuffle the project heap
                //
                make_heap(project_heap.begin(), project_heap.end());
            }
        }
    }

    for (unsigned int i=0; i<gstate.projects.size(); i++) {
        PROJECT* p = gstate.projects[i];
        p->pwf.rec_temp = p->pwf.rec_temp_save;
    }
    if (log_flags.rr_simulation) {
        msg_printf(NULL, MSG_INFO, "pick_jobs_to_run() end");
    }
}

// compute the number of idle instances (count - nused)
// Called at the start of RR simulation,
// after the initial assignment of jobs
//
static void record_nidle_now() {
    rsc_work_fetch[0].nidle_now = gstate.n_usable_cpus - rsc_work_fetch[0].sim_nused;
    if (rsc_work_fetch[0].nidle_now < 0) rsc_work_fetch[0].nidle_now = 0;
    for (int i=1; i<coprocs.n_rsc; i++) {
        rsc_work_fetch[i].nidle_now = coprocs.coprocs[i].count - rsc_work_fetch[i].sim_nused;
        if (rsc_work_fetch[i].nidle_now < 0) rsc_work_fetch[i].nidle_now = 0;
    }
}

static void handle_missed_deadline(RESULT* rpbest, double diff, double ar) {
    ACTIVE_TASK* atp = gstate.lookup_active_task_by_result(rpbest);
    PROJECT* pbest = rpbest->project;
    if (atp) {
        atp->last_deadline_miss_time = gstate.now;
    }
    if (atp && atp->procinfo.working_set_size_smoothed > ar) {
        if (log_flags.rr_simulation) {
            msg_printf(pbest, MSG_INFO,
                "[rr_sim] %s misses deadline but too large to run",
                rpbest->name
            );
        }
    } else {
        rpbest->rr_sim_misses_deadline = true;
        int rt = rpbest->resource_usage.rsc_type;
        if (rt) {
            pbest->rsc_pwf[rt].deadlines_missed++;
            rsc_work_fetch[rt].deadline_missed_instances += rpbest->resource_usage.coproc_usage;
        } else {
            pbest->rsc_pwf[0].deadlines_missed++;
            rsc_work_fetch[0].deadline_missed_instances += rpbest->resource_usage.avg_ncpus;
        }
        if (log_flags.rr_simulation) {
            msg_printf(pbest, MSG_INFO,
                "[rr_sim] %s misses deadline by %.2f",
                rpbest->name, diff
            );
        }
    }
}

// update "MC shortfall" for projects with max concurrent restrictions
//
static void mc_update_stats(double sim_now, double dt, double buf_end) {
    for (unsigned int i=0; i<gstate.projects.size(); i++) {
        PROJECT* p = gstate.projects[i];
        if (!p->app_configs.project_has_mc) continue;
        for (int rt=0; rt<coprocs.n_rsc; rt++) {
            RSC_PROJECT_WORK_FETCH& rsc_pwf = p->rsc_pwf[rt];

            // x is the number of instances this project isn't using but could
            // (given MC constraints)
            //
            double x = rsc_pwf.mc_max_could_use - rsc_pwf.sim_nused;
            if (x > 1e-6 && sim_now < buf_end) {
                double dt2;
                if (sim_now + dt > buf_end) {
                    dt2 = buf_end - sim_now;
                } else {
                    dt2 = dt;
                }
                rsc_pwf.mc_shortfall += x*dt2;
            }
        }
    }
}

// do a round_robin simulation,
// for either CPU scheduling (to find deadline misses)
// or work fetch (do compute idleness and shortfall)
//
void RR_SIM::simulate() {
    PROJECT* pbest;
    RESULT* rp, *rpbest;
    unsigned int u;

    double ar = gstate.available_ram();

    // initialize work-fetch data structures in either case
    //
    work_fetch.rr_init();

    if (log_flags.rr_simulation) {
        msg_printf(0, MSG_INFO,
            "[rr_sim] start: work_buf min %.0f additional %.0f total %.0f on_frac %.3f active_frac %.3f",
            gstate.work_buf_min(),
            gstate.work_buf_additional(),
            gstate.work_buf_total(),
            gstate.time_stats.on_frac,
            gstate.time_stats.active_frac
        );
    }

    project_priority_init(false);
    init_pending_lists();

    if (have_max_concurrent) {
        for (unsigned int i=0; i<gstate.projects.size(); i++) {
            PROJECT* p = gstate.projects[i];
            p->pwf.at_max_concurrent_limit = false;
        }
    }

    // Simulation loop.  Keep going until all jobs done
    //
    double buf_end = gstate.now + gstate.work_buf_total();
    double sim_now = gstate.now;
    bool first = true;
    while (1) {
        pick_jobs_to_run(sim_now-gstate.now);
        if (first) {
            record_nidle_now();
            first = false;
        }

        if (!active_jobs.size()) break;

        // compute finish times and see which job finishes first
        //
        rpbest = NULL;
        for (u=0; u<active_jobs.size(); u++) {
            rp = active_jobs[u];
            rp->rrsim_finish_delay = rp->rrsim_flops_left/rp->rrsim_flops;
            if (!rpbest || rp->rrsim_finish_delay < rpbest->rrsim_finish_delay) {
                rpbest = rp;
            }
        }

        // see if we finish a time slice before first job ends
        //
        double delta_t = rpbest->rrsim_finish_delay;
        if (log_flags.rrsim_detail) {
            msg_printf(NULL, MSG_INFO,
                "[rrsim_detail] next job to finish: %s (will finish in  %.2f sec)",
                rpbest->name,
                delta_t
            );
        }
        if (delta_t > 3600) {
            rpbest = 0;

            // limit the granularity
            //
            if (delta_t > 36000) {
                delta_t /= 10;
            } else {
                delta_t = 3600;
            }
            if (log_flags.rrsim_detail) {
                msg_printf(NULL, MSG_INFO,
                    "[rrsim_detail] taking time-slice step of %.2f sec", delta_t
                );
            }
        } else {
            rpbest->rrsim_done = true;
            pbest = rpbest->project;
            if (log_flags.rr_simulation) {
                char buf[256];
                rpbest->rsc_string(buf, sizeof(buf));
                msg_printf(pbest, MSG_INFO,
                    "[rr_sim] %.2f: %s finishes (%s) (%.2fG/%.2fG)",
                    sim_now + delta_t - gstate.now,
                    rpbest->name,
                    buf,
                    rpbest->estimated_flops_remaining()/1e9, rpbest->rrsim_flops/1e9
                );
            }

            // Does it miss its deadline?
            //
            double diff = (sim_now + rpbest->rrsim_finish_delay) - rpbest->computation_deadline();
            if (diff > 0) {
                handle_missed_deadline(rpbest, diff, ar);

                // update busy time of relevant processor types
                //
                double frac = rpbest->uses_gpu()?gstate.overall_gpu_frac():gstate.overall_cpu_frac();
                double dur = rpbest->estimated_runtime_remaining() / frac;
                rsc_work_fetch[0].update_busy_time(dur, rpbest->resource_usage.avg_ncpus);
                int rt = rpbest->resource_usage.rsc_type;
                if (rt) {
                    rsc_work_fetch[rt].update_busy_time(dur, rpbest->resource_usage.coproc_usage);
                }
            }
        }

        // adjust FLOPS left of other active jobs
        //
        for (unsigned int i=0; i<active_jobs.size(); i++) {
            rp = active_jobs[i];
            rp->rrsim_flops_left -= rp->rrsim_flops*delta_t;

            // can be slightly less than 0 due to roundoff
            //
            if (rp->rrsim_flops_left < -1e6) {
                if (log_flags.rr_simulation) {
                    msg_printf(rp->project, MSG_INTERNAL_ERROR,
                        "%s: negative FLOPs left %f", rp->name, rp->rrsim_flops_left
                    );
                }
            }
            if (rp->rrsim_flops_left < 0) {
                rp->rrsim_flops_left = 0;
            }
        }

        // update shortfall and saturated time for each resource
        //
        for (int i=0; i<coprocs.n_rsc; i++) {
            rsc_work_fetch[i].update_stats(sim_now, delta_t, buf_end);
        }
        if (have_max_concurrent) {
            mc_update_stats(sim_now, delta_t, buf_end);
        }

        // update project REC
        //
        double f = gstate.host_info.p_fpops;
        for (unsigned int i=0; i<gstate.projects.size(); i++) {
            PROJECT* p = gstate.projects[i];
            double dtemp = sim_now;
            double x = 0;
            for (int j=0; j<coprocs.n_rsc; j++) {
                x += p->rsc_pwf[j].sim_nused * delta_t * f * rsc_work_fetch[j].relative_speed;
            }
            x *= COBBLESTONE_SCALE;
            update_average(
                sim_now+delta_t,
                sim_now,
                x,
                cc_config.rec_half_life,
                p->pwf.rec_temp,
                dtemp
            );
            p->compute_sched_priority();
        }

        sim_now += delta_t;
    }

    // identify GPU instances starved because of exclusions
    //
    for (int i=1; i<coprocs.n_rsc; i++) {
        RSC_WORK_FETCH& rwf = rsc_work_fetch[i];
        if (!rwf.has_exclusions) continue;
        COPROC& cp = coprocs.coprocs[i];
        COPROC_INSTANCE_BITMAP mask = 0;
        for (int j=0; j<cp.count; j++) {
            mask |= ((COPROC_INSTANCE_BITMAP)1)<<j;
        }
        rwf.sim_excluded_instances = ~(rwf.sim_used_instances) & mask;
        if (log_flags.rrsim_detail) {
            msg_printf(0, MSG_INFO,
                "[rrsim_detail] rsc %d: sim_used_inst %lld mask %lld sim_excluded_instances %lld",
                i, rwf.sim_used_instances, mask, rwf.sim_excluded_instances
            );
        }
    }

    // if simulation ends before end of buffer, take the tail into account
    //
    if (sim_now < buf_end) {
        double d_time = buf_end - sim_now;
        for (int i=0; i<coprocs.n_rsc; i++) {
            rsc_work_fetch[i].update_stats(sim_now, d_time, buf_end);
        }
        if (have_max_concurrent) {
            mc_update_stats(sim_now, d_time, buf_end);
        }
    }
    if (log_flags.rr_simulation) {
        msg_printf(0, MSG_INFO,
            "[rr_sim] end"
        );
    }
}

void rr_simulation(const char* why) {
    static double last_time=0;

    if (log_flags.rr_simulation) {
        msg_printf(0, MSG_INFO, "[rr_sim] doing sim: %s", why);
    }

    // CPU sched and work fetch both call this.
    // We only need to do one simulation per moment.
    //
    if (last_time == gstate.now) {
        if (log_flags.rr_simulation) {
            msg_printf(0, MSG_INFO, "[rr_sim] already did at this time");
        }
        return;
    }
    last_time = gstate.now;
    RR_SIM rr_sim;
    rr_sim.simulate();
}

// Compute the number resources with > 0 idle instance
// Put results in global state (rsc_work_fetch[].nidle_now)
// This is called from the account manager logic,
// to decide if we need to get new projects from the AM.
//
int n_idle_resources() {
    int nidle_rsc = 0;
    coprocs.coprocs[0].count = gstate.n_usable_cpus;
    for (int i=0; i<coprocs.n_rsc; i++) {
        int c = coprocs.coprocs[i].count;
        rsc_work_fetch[i].nidle_now = c;
        if (c > 0) nidle_rsc++;
    }
    for (unsigned int i=0; i<gstate.results.size(); i++) {
        RESULT* rp = gstate.results[i];
        if (!rp->nearly_runnable()) continue;
        if (rp->some_download_stalled()) continue;
        if (rsc_work_fetch[0].nidle_now) {
            rsc_work_fetch[0].nidle_now -= rp->resource_usage.avg_ncpus;
            if (rsc_work_fetch[0].nidle_now <= 0) {
                nidle_rsc--;
                rsc_work_fetch[0].nidle_now = 0;
            }
        }
        int j = rp->resource_usage.rsc_type;
        if (!j) {
            continue;
        }
        if (rsc_work_fetch[j].nidle_now) {
            rsc_work_fetch[j].nidle_now -= rp->resource_usage.coproc_usage;
            if (rsc_work_fetch[j].nidle_now <= 0) {
                nidle_rsc--;
                rsc_work_fetch[j].nidle_now = 0;
            }
        }
        if (nidle_rsc == 0) {
            // no idle resources - no need to look further
            //
            break;
        }
    }
    return nidle_rsc;
}
