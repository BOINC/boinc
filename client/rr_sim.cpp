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
// - resource shortfalls (per-project and total)
// - counts of resources idle now
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
struct RR_SIM {
    std::vector<RESULT*> active;

    inline void activate(RESULT* rp) {
        PROJECT* p = rp->project;
        active.push_back(rp);
        rsc_work_fetch[0].sim_nused += rp->avp->avg_ncpus;
        p->rsc_pwf[0].sim_nused += rp->avp->avg_ncpus;
        int rt = rp->avp->gpu_usage.rsc_type;
        if (rt) {
            rsc_work_fetch[rt].sim_nused += rp->avp->gpu_usage.usage;
            p->rsc_pwf[rt].sim_nused += rp->avp->gpu_usage.usage;
        }
    }

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
    if (rp->uses_coprocs()) {
        rp->rrsim_flops = rp->avp->flops * gstate.overall_gpu_frac();
    } else {
        rp->rrsim_flops =  rp->avp->flops * gstate.overall_cpu_frac();
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
                "[cpu_sched] Result %s projected to miss deadline.",
                rp->name
            );
        }
    }
    for (i=0; i<gstate.projects.size(); i++) {
        p = gstate.projects[i];
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

// Decide what jobs to include in the simulation;
// build the "pending" lists of the respective processor types.
// NOTE: "results" is sorted by increasing arrival time
//
static inline void init_pending_lists() {
    for (unsigned int i=0; i<gstate.results.size(); i++) {
        RESULT* rp = gstate.results[i];
        rp->rr_sim_misses_deadline = false;
        if (!rp->nearly_runnable()) continue;
        if (rp->some_download_stalled()) continue;
        if (rp->project->non_cpu_intensive) continue;
        rp->rrsim_flops_left = rp->estimated_flops_remaining();

        //if (rp->rrsim_flops_left <= 0) continue;
            // job may have fraction_done=1 but not be done;
            // if it's past its deadline, we need to mark it as such

        PROJECT* p = rp->project;
        p->pwf.has_runnable_jobs = true;
        p->rsc_pwf[0].nused_total += rp->avp->avg_ncpus;
        int rt = rp->avp->gpu_usage.rsc_type;
        if (rt) {
            p->rsc_pwf[rt].nused_total += rp->avp->gpu_usage.usage;
            p->rsc_pwf[rt].has_runnable_jobs = true;
        }
        rsc_work_fetch[rt].pending.push_back(rp);
        set_rrsim_flops(rp);
        rp->rrsim_done = false;
    }
}

bool result_less_than(RESULT* r1, RESULT* r2) {
    if (r2->rrsim_done) return true;
    if (r1->rrsim_done) return false;
    return (r1->project->sched_priority > r2->project->sched_priority);
}

static void sort_pending_lists() {
    for (int i=0; i<coprocs.n_rsc; i++) {
        vector<RESULT*>& v = rsc_work_fetch[i].pending;
        std::sort(v.begin(), v.end(), result_less_than);

        // trim off finished jobs
        //
        while (v.size() && (v.back())->rrsim_done) {
            v.pop_back();
        }
    }
}

void RR_SIM::pick_jobs_to_run(double reltime) {
    active.clear();

    // do the GPUs first
    //
    for (int rt=coprocs.n_rsc-1; rt>=0; rt--) {
        // clear usage counts
        //
        rsc_work_fetch[rt].sim_nused = 0;
        for (unsigned int i=0; i<gstate.projects.size(); i++) {
            PROJECT* p = gstate.projects[i];
            p->rsc_pwf[rt].sim_nused = 0;
        }
        for (unsigned int i=0; i<rsc_work_fetch[rt].pending.size(); i++) {
            RESULT* rp = rsc_work_fetch[rt].pending[i];
            if (rt) {
                PROJECT* p = rp->project;
                if (rsc_work_fetch[rt].sim_nused >= coprocs.coprocs[rt].count - p->ncoprocs_excluded[rt]) break;
            } else {
                if (rsc_work_fetch[rt].sim_nused >= gstate.ncpus) break;
            }
            activate(rp);
            if (log_flags.rrsim_detail) {
                char buf[256];
                rsc_string(rp, buf);
                msg_printf(rp->project, MSG_INFO,
                    "[rr_sim_detail] %.2f: starting %s (%s)",
                    reltime, rp->name, buf
                );
            }
        }
    }
}

static void record_nidle_now() {
    // note the number of idle instances
    //
    rsc_work_fetch[0].nidle_now = gstate.ncpus - rsc_work_fetch[0].sim_nused;
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

void RR_SIM::simulate() {
    PROJECT* pbest;
    RESULT* rp, *rpbest;
    unsigned int u;

    double ar = gstate.available_ram();

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

    init_pending_lists();

    // Simulation loop.  Keep going until all jobs done
    //
    double buf_end = gstate.now + gstate.work_buf_total();
    double sim_now = gstate.now;
    bool first = true;
    project_priority_init(false);
    while (1) {
        sort_pending_lists();   // sort jobs by project priority
        pick_jobs_to_run(sim_now-gstate.now);
        if (first) {
            record_nidle_now();
            first = false;
        }

        if (!active.size()) break;

        // compute finish times and see which job finishes first
        //
        rpbest = NULL;
        for (u=0; u<active.size(); u++) {
            rp = active[u];
            if (rp->rrsim_flops) {
                rp->rrsim_finish_delay = rp->rrsim_flops_left/rp->rrsim_flops;
                if (!rpbest || rp->rrsim_finish_delay < rpbest->rrsim_finish_delay) {
                    rpbest = rp;
                }
            }
        }

        // see if we finish a time slice before first job ends
        //
        double delta_t = rpbest->rrsim_finish_delay;
        if (delta_t > 3600) {
            rpbest = 0;

            // limit the granularity
            //
            if (delta_t > 36000) {
                delta_t /= 10;
            } else {
                delta_t = 3600;
            }
        } else {
            rpbest->rrsim_done = true;
            pbest = rpbest->project;
            if (log_flags.rr_simulation) {
                msg_printf(pbest, MSG_INFO,
                    "[rr_sim] %.2f: %s finishes (%.2fG/%.2fG)",
                    sim_now - gstate.now,
                    rpbest->name,
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
                double frac = rpbest->uses_coprocs()?gstate.overall_gpu_frac():gstate.overall_cpu_frac();
                double dur = rpbest->estimated_time_remaining() / frac;
                rsc_work_fetch[0].update_busy_time(dur, rpbest->avp->avg_ncpus);
                int rt = rpbest->avp->gpu_usage.rsc_type;
                if (rt) {
                    rsc_work_fetch[rt].update_busy_time(dur, rpbest->avp->gpu_usage.usage);
                }
            }
        }
        // adjust FLOPS left
        for (unsigned int i=0; i<active.size(); i++) {
            rp = active[i];
            rp->rrsim_flops_left -= rp->rrsim_flops*delta_t;

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
        }

        // update saturated time
        //
        double end_time = sim_now + delta_t;
        double x = end_time - gstate.now;
        for (int i=0; i<coprocs.n_rsc; i++) {
            rsc_work_fetch[i].update_saturated_time(x);
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

        // update project REC
        //
        double f = gstate.host_info.p_fpops;
        for (unsigned int i=0; i<gstate.projects.size(); i++) {
            PROJECT* p = gstate.projects[i];
            double dtemp;
            x = 0;
            for (int j=0; j<coprocs.n_rsc; j++) {
                x += p->rsc_pwf[j].sim_nused * delta_t * f * rsc_work_fetch[j].relative_speed;
            }
            x *= COBBLESTONE_SCALE;
            update_average(
                sim_now+delta_t,
                sim_now,
                x,
                config.rec_half_life,
                p->pwf.rec_temp,
                dtemp
            );
            p->compute_sched_priority();
        }

        sim_now += delta_t;
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

void rr_simulation() {
    RR_SIM rr_sim;
    rr_sim.simulate();
}
