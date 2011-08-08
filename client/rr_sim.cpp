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
// For efficiency, we simulate an approximation of WRR.
// We don't model time-slicing.
// Instead we use a continuous model where, at a given point,
// each project has a set of running jobs that uses at most all CPUs.
// These jobs are assumed to run at a rate proportionate to their avg_ncpus,
// and each project gets total CPU proportionate to its RRS.
//
// For coprocessors, we saturate the resource;
// i.e. with 2 GPUs, we'd let a 1-GPU app and a 2-GPU app run together.
// Otherwise, there'd be the possibility of computing
// a nonzero shortfall inappropriately.
//
// Outputs are changes to global state:
// - deadline misses (per-project count, per-result flag)
//      Deadline misses are not counted for tasks
//      that are too large to run in RAM right now.
// - resource shortfalls (per-project and total)
// - counts of resources idle now
//

#include "cpp.h"

#ifdef _WIN32
#include "boinc_win.h"
#else
#include "config.h"
#endif

#include "client_state.h"
#include "coproc.h"
#include "client_msgs.h"

inline void rsc_string(RESULT* rp, char* buf) {
    APP_VERSION* avp = rp->avp;
    if (avp->gpu_usage.rsc_type) {
        sprintf(buf, "%.2f CPU + %.2f %s",
            avp->avg_ncpus, avp->gpu_usage.usage,
            rsc_name(avp->gpu_usage.rsc_type)
        );
    } else {
        sprintf(buf, "%.2f CPU", avp->avg_ncpus);
    }
}

// this is here (rather than rr_sim.h) because its inline functions
// refer to RESULT
//
struct RR_SIM_STATUS {
    std::vector<RESULT*> active;
    double active_rsc[MAX_RSC];

    inline void activate(RESULT* rp, double when) {
        PROJECT* p = rp->project;
        if (log_flags.rr_simulation) {
            char buf[256];
            rsc_string(rp, buf);
            msg_printf(p, MSG_INFO,
                "[rr_sim] %.2f: starting %s (%s)",
                when, rp->name, buf
            );
        }
        active.push_back(rp);
        rsc_work_fetch[0].sim_nused += rp->avp->avg_ncpus;
        int rt = rp->avp->gpu_usage.rsc_type;
        if (rt) {
            rsc_work_fetch[rt].sim_nused += rp->avp->gpu_usage.usage;
        }
    }
    // remove *rpbest from active set,
    // and adjust FLOPS left for other results
    //
    inline void remove_active(RESULT* rpbest) {
        vector<RESULT*>::iterator it = active.begin();
        while (it != active.end()) {
            RESULT* rp = *it;
            if (rp == rpbest) {
                it = active.erase(it);
            } else {
                rp->rrsim_flops_left -= rp->rrsim_flops*rpbest->rrsim_finish_delay;

                // can be slightly less than 0 due to roundoff
                //
                if (rp->rrsim_flops_left < -1) {
                    msg_printf(rp->project, MSG_INTERNAL_ERROR,
                        "%s: negative FLOPs left %f", rp->name, rp->rrsim_flops_left
                    );
                }
                if (rp->rrsim_flops_left < 0) {
                    rp->rrsim_flops_left = 0;
                }
                it++;
            }
        }
        rsc_work_fetch[0].sim_nused -= rpbest->avp->avg_ncpus;
        int rt = rpbest->avp->gpu_usage.rsc_type;
        if (rt) {
            rsc_work_fetch[rt].sim_nused -= rpbest->avp->gpu_usage.usage;
        }
    }

    RR_SIM_STATUS() {
        for (int i=0; i<coprocs.n_rsc; i++) {
            active_rsc[i] = 0;
        }
    }
    ~RR_SIM_STATUS() {}

    // compute shares of projects with active CPU jobs
    //
    inline void get_cpu_shares() {
        unsigned int i;
        PROJECT *p;
        for (i=0; i<gstate.projects.size(); i++) {
            p = gstate.projects[i];
            p->rr_sim_cpu_share = 0;
            p->rr_sim_active = false;

        }

        double nz_ncpus = 0;
            // CPU usage of nonzero resource share projects

        for (i=0; i<active.size(); i++) {
            RESULT* rp = active[i];
            p = rp->project;
            if (p->resource_share) {
                nz_ncpus += rp->avp->avg_ncpus;
            }
            p->rr_sim_active = true;
        }

        double sum=0;
        int z_count = 0;
        for (i=0; i<gstate.projects.size(); i++) {
            p = gstate.projects[i];
            if (p->rr_sim_active) {
                if (p->resource_share) {
                    sum += p->resource_share;
                } else {;
                    z_count++;
                }
            }
        }

        if (nz_ncpus < gstate.ncpus) {
            double z_frac = 1-(nz_ncpus/gstate.ncpus);
            for (i=0; i<gstate.projects.size(); i++) {
                p = gstate.projects[i];
                if (p->resource_share) {
                    p->rr_sim_cpu_share = 1;
                } else {
                    p->rr_sim_cpu_share = z_frac/z_count;
                }
            }
        } else {
            for (i=0; i<gstate.projects.size(); i++) {
                p = gstate.projects[i];
                if (p->resource_share) {
                    p->rr_sim_cpu_share = p->resource_share/sum;
                }
            }
        }
    }
};

void RR_SIM_PROJECT_STATUS::activate(RESULT* rp) {
    active.push_back(rp);
    rp->project->rsc_pwf[0].sim_nused += rp->avp->avg_ncpus;
    int rt = rp->avp->gpu_usage.rsc_type;
    if (rt) {
        rp->project->rsc_pwf[rt].sim_nused += rp->avp->gpu_usage.usage;
    }
}

void RR_SIM_PROJECT_STATUS::remove_active(RESULT* rp) {
    std::vector<RESULT*>::iterator it = active.begin();
    while (it != active.end()) {
        if (*it == rp) {
            it = active.erase(it);
        } else {
            it++;
        }
    }

    rp->project->rsc_pwf[0].sim_nused -= rp->avp->avg_ncpus;
    int rt = rp->avp->gpu_usage.rsc_type;
    if (rt) {
        rp->project->rsc_pwf[rt].sim_nused -= rp->avp->gpu_usage.usage;
    }
}

// estimate the rate (FLOPS) that this job will get long-term
// with weighted round-robin scheduling
//
void set_rrsim_flops(RESULT* rp) {
    // For coproc jobs, use app version estimate
    //
    if (rp->uses_coprocs()) {
        rp->rrsim_flops = rp->avp->flops * gstate.overall_gpu_frac();
        return;
    }
    PROJECT* p = rp->project;

    // For CPU jobs, estimate how many CPU seconds per second this job would get
    // running with other jobs of this project, ignoring other factors
    //
    double x = 1;
    if (p->rsc_pwf[0].sim_nused > gstate.ncpus) {
        x = gstate.ncpus/p->rsc_pwf[0].sim_nused;
    }
    double r1 = x*rp->avp->avg_ncpus;

    // if the project's total CPU usage is more than its share, scale
    //
    double share_cpus = p->rr_sim_cpu_share*gstate.ncpus;
    double r2 = r1;
    if (p->rsc_pwf[0].sim_nused > share_cpus) {
        r2 *= (share_cpus / p->rsc_pwf[0].sim_nused);
    }

    // scale by overall CPU availability
    //
    double r3 = r2 * gstate.overall_cpu_frac();

    rp->rrsim_flops = r3 * rp->avp->flops;
#if 0
    if (log_flags.rr_simulation) {
        msg_printf(p, MSG_INFO,
            "[rr_sim] set_rrsim_flops: %.2fG (r1 %.4f r2 %.4f r3 %.4f)",
            rp->rrsim_flops/1e9, r1, r2, r3
        );
    }
#endif
}

void CLIENT_STATE::print_deadline_misses() {
    unsigned int i;
    RESULT* rp;
    PROJECT* p;
    for (i=0; i<results.size(); i++){
        rp = results[i];
        if (rp->rr_sim_misses_deadline) {
            msg_printf(rp->project, MSG_INFO,
                "[cpu_sched] Result %s projected to miss deadline.",
                rp->name
            );
        }
    }
    for (i=0; i<projects.size(); i++) {
        p = projects[i];
        for (int j=0; j<coprocs.n_rsc; j++) {
            if (p->rsc_pwf[j].deadlines_missed) {
                msg_printf(p, MSG_INFO,
                    "[cpu_sched] Project has %d projected %s deadline misses",
                    p->rsc_pwf[j].deadlines_missed,
                    rsc_name(j)
                );
            }
        }
    }
}

void CLIENT_STATE::rr_simulation() {
    PROJECT* p, *pbest;
    RESULT* rp, *rpbest;
    RR_SIM_STATUS sim_status;
    unsigned int u;

    double ar = available_ram();

    work_fetch.rr_init();

    if (log_flags.rr_simulation) {
        msg_printf(0, MSG_INFO,
            "[rr_sim] start: work_buf min %.0f additional %.0f total %.0f on_frac %.3f active_frac %.3f",
            work_buf_min(), work_buf_additional(), work_buf_total(),
            time_stats.on_frac, time_stats.active_frac
        );
    }

    for (u=0; u<projects.size(); u++) {
        p = projects[u];
        if (p->non_cpu_intensive) continue;
        p->rr_sim_status.clear();
    }

    // Decide what jobs to include in the simulation,
    // and pick the ones that are initially running.
    // NOTE: "results" is sorted by increasing arrival time
    //
    for (u=0; u<results.size(); u++) {
        rp = results[u];
        rp->rr_sim_misses_deadline = false;
        if (!rp->nearly_runnable()) continue;
        if (rp->some_download_stalled()) continue;
        if (rp->project->non_cpu_intensive) continue;
        rp->rrsim_flops_left = rp->estimated_flops_remaining();

        //if (rp->rrsim_flops_left <= 0) continue;
            // job may have fraction_done=1 but not be done;
            // if it's past its deadline, we need to mark it as such

        p = rp->project;
        p->pwf.has_runnable_jobs = true;
        p->rsc_pwf[0].nused_total += rp->avp->avg_ncpus;
        int rt = rp->avp->gpu_usage.rsc_type;
        if (rt) {
            p->rsc_pwf[rt].nused_total += rp->avp->gpu_usage.usage;
            p->rsc_pwf[rt].has_runnable_jobs = true;
            if (rsc_work_fetch[rt].sim_nused < coprocs.coprocs[rt].count) {
                sim_status.activate(rp, 0);
                p->rr_sim_status.activate(rp);
            } else {
                rsc_work_fetch[rt].pending.push_back(rp);
            }
        } else {
            p->rsc_pwf[0].has_runnable_jobs = true;
            if (p->rsc_pwf[0].sim_nused < ncpus) {
                sim_status.activate(rp, 0);
                p->rr_sim_status.activate(rp);
            } else {
                p->rr_sim_status.add_pending(rp);
            }
        }
    }

    // note the number of idle instances
    //
    rsc_work_fetch[0].nidle_now = ncpus - rsc_work_fetch[0].sim_nused;
    if (rsc_work_fetch[0].nidle_now < 0) rsc_work_fetch[0].nidle_now = 0;
    for (int i=1; i<coprocs.n_rsc; i++) {
        rsc_work_fetch[i].nidle_now = coprocs.coprocs[i].count - rsc_work_fetch[i].sim_nused;
        if (rsc_work_fetch[i].nidle_now < 0) rsc_work_fetch[i].nidle_now = 0;
    }

    // Simulation loop.  Keep going until all work done
    //
    double buf_end = now + work_buf_total();
    double sim_now = now;
    while (sim_status.active.size()) {

        sim_status.get_cpu_shares();

        // compute finish times and see which result finishes first
        //
        rpbest = NULL;
        for (u=0; u<sim_status.active.size(); u++) {
            rp = sim_status.active[u];
            set_rrsim_flops(rp);
            if (rp->rrsim_flops) {
                rp->rrsim_finish_delay = rp->rrsim_flops_left/rp->rrsim_flops;
                if (!rpbest || rp->rrsim_finish_delay < rpbest->rrsim_finish_delay) {
                    rpbest = rp;
                }
            }
        }

        pbest = rpbest->project;

        if (log_flags.rr_simulation) {
            msg_printf(pbest, MSG_INFO,
                "[rr_sim] %.2f: %s finishes after %.2f (%.2fG/%.2fG)",
                sim_now - now,
                rpbest->name, rpbest->rrsim_finish_delay,
                rpbest->rrsim_flops_left/1e9, rpbest->rrsim_flops/1e9
            );
        }

        // "rpbest" is first result to finish.  Does it miss its deadline?
        //
        double diff = (sim_now + rpbest->rrsim_finish_delay) - rpbest->computation_deadline();
        if (diff > 0) {
            ACTIVE_TASK* atp = lookup_active_task_by_result(rpbest);
            if (atp) {
                atp->last_deadline_miss_time = now;
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
                int rt = rpbest->avp->gpu_usage.rsc_type;
                if (rt) {
                    pbest->rsc_pwf[rt].deadlines_missed++;
                    rsc_work_fetch[rt].deadline_missed_instances += rpbest->avp->gpu_usage.usage;
                } else {
                    pbest->rsc_pwf[0].deadlines_missed++;
                    rsc_work_fetch[0].deadline_missed_instances += rpbest->avp->avg_ncpus;
                }
                if (log_flags.rr_simulation) {
                    msg_printf(pbest, MSG_INFO,
                        "[rr_sim] %s misses deadline by %.2f",
                        rpbest->name, diff
                    );
                }
            }
        }

        // update saturated time
        //
        double end_time = sim_now + rpbest->rrsim_finish_delay;
        double x = end_time - gstate.now;
        for (int i=0; i<coprocs.n_rsc; i++) {
            rsc_work_fetch[i].update_saturated_time(x);
        }

        // update busy time
        //
        if (rpbest->rr_sim_misses_deadline) {
            double frac = rpbest->uses_coprocs()?gstate.overall_gpu_frac():gstate.overall_cpu_frac();
            double dur = rpbest->estimated_time_remaining() / frac;
            rsc_work_fetch[0].update_busy_time(dur, rpbest->avp->avg_ncpus);
            int rt = rpbest->avp->gpu_usage.rsc_type;
            if (rt) {
                rsc_work_fetch[rt].update_busy_time(dur, rpbest->avp->gpu_usage.usage);
            }
        }

        // increment resource shortfalls
        //
        if (sim_now < buf_end) {
            if (end_time > buf_end) end_time = buf_end;
            double d_time = end_time - sim_now;

            for (int i=0; i<coprocs.n_rsc; i++) {
                rsc_work_fetch[i].accumulate_shortfall(d_time);
            }
        }

        sim_status.remove_active(rpbest);
        pbest->rr_sim_status.remove_active(rpbest);

        sim_now += rpbest->rrsim_finish_delay;

        // start new jobs; may need to start more than one
        // if this job used multiple resource instances
        //
        int rt = rpbest->avp->gpu_usage.rsc_type;
        if (rt) {
            while (1) {
                if (rsc_work_fetch[rt].sim_nused >= coprocs.coprocs[rt].count) break;
                if (!rsc_work_fetch[rt].pending.size()) break;
                rp = rsc_work_fetch[rt].pending[0];
                rsc_work_fetch[rt].pending.erase(rsc_work_fetch[rt].pending.begin());
                sim_status.activate(rp, sim_now-now);
                pbest->rr_sim_status.activate(rp);
            }
        } else {
            while (1) {
                if (pbest->rsc_pwf[0].sim_nused >= ncpus) break;
                rp = pbest->rr_sim_status.get_pending();
                if (!rp) break;
                sim_status.activate(rp, sim_now-now);
                pbest->rr_sim_status.activate(rp);
            }
        }
    }

    // if simulation ends before end of buffer, take the tail into account
    //
    if (sim_now < buf_end) {
        double d_time = buf_end - sim_now;
        for (int i=0; i<coprocs.n_rsc; i++) {
            rsc_work_fetch[i].accumulate_shortfall(d_time);
        }
    }
}
