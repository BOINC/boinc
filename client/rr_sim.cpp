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
    if (avp->ncudas) {
        sprintf(buf, "%.2f CPU + %.2f NV", avp->avg_ncpus, avp->ncudas);
    } else if (avp->natis) {
        sprintf(buf, "%.2f CPU + %.2f ATI", avp->avg_ncpus, avp->natis);
    } else {
        sprintf(buf, "%.2f CPU", avp->avg_ncpus);
    }
}

// this is here (rather than rr_sim.h) because its inline functions
// refer to RESULT
//
struct RR_SIM_STATUS {
    std::vector<RESULT*> active;
    double active_ncpus;
    double active_cudas;
    double active_atis;

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
        cpu_work_fetch.sim_nused += rp->avp->avg_ncpus;
        cuda_work_fetch.sim_nused += rp->avp->ncudas;
        ati_work_fetch.sim_nused += rp->avp->natis;
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
        cpu_work_fetch.sim_nused -= rpbest->avp->avg_ncpus;
        cuda_work_fetch.sim_nused -= rpbest->avp->ncudas;
        ati_work_fetch.sim_nused -= rpbest->avp->natis;
    }

    RR_SIM_STATUS() {
        active_ncpus = 0;
        active_cudas = 0;
        active_atis = 0;
    }
    ~RR_SIM_STATUS() {}
};

void RR_SIM_PROJECT_STATUS::activate(RESULT* rp) {
    active.push_back(rp);
    rp->project->cpu_pwf.sim_nused += rp->avp->avg_ncpus;
    rp->project->cuda_pwf.sim_nused += rp->avp->ncudas;
    rp->project->ati_pwf.sim_nused += rp->avp->natis;
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
    rp->project->cpu_pwf.sim_nused -= rp->avp->avg_ncpus;
    rp->project->cuda_pwf.sim_nused -= rp->avp->ncudas;
    rp->project->ati_pwf.sim_nused -= rp->avp->natis;
}

// estimate the rate (FLOPS) that this job will get long-term
// with weighted round-robin scheduling
//
void set_rrsim_flops(RESULT* rp) {
    // For coproc jobs, use app version estimate
    //
    if (rp->uses_coprocs()) {
        rp->rrsim_flops = rp->avp->flops * gstate.overall_cpu_frac();
        return;
    }
    PROJECT* p = rp->project;

    // For CPU jobs, estimate how many CPU seconds per second this job would get
    // running with other jobs of this project, ignoring other factors
    //
    double x = 1;
    if (p->cpu_pwf.sim_nused > gstate.ncpus) {
        x = gstate.ncpus/p->cpu_pwf.sim_nused;
    }
    double r1 = x*rp->avp->avg_ncpus;

    // if the project's total CPU usage is more than its share, scale
    //
    double share_cpus = p->cpu_pwf.runnable_share*gstate.ncpus;
    if (!share_cpus) share_cpus = gstate.ncpus;
        // deal with projects w/ resource share = 0
    double r2 = r1;
    if (p->cpu_pwf.sim_nused > share_cpus) {
        r2 *= (share_cpus / p->cpu_pwf.sim_nused);
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
        if (p->cpu_pwf.deadlines_missed) {
            msg_printf(p, MSG_INFO,
                "[cpu_sched] Project has %d projected CPU deadline misses",
                p->cpu_pwf.deadlines_missed
            );
        }
        if (p->cuda_pwf.deadlines_missed) {
            msg_printf(p, MSG_INFO,
                "[cpu_sched] Project has %d projected NVIDIA GPU deadline misses",
                p->cuda_pwf.deadlines_missed
            );
        }
        if (p->ati_pwf.deadlines_missed) {
            msg_printf(p, MSG_INFO,
                "[cpu_sched] Project has %d projected ATI GPU deadline misses",
                p->ati_pwf.deadlines_missed
            );
        }
    }
}

#if 0
// compute a per-app-version "temporary DCF" based on the elapsed time
// and fraction done of running jobs
//
void compute_temp_dcf() {
    unsigned int i;
    for (i=0; i<gstate.app_versions.size(); i++) {
        gstate.app_versions[i]->temp_dcf = 1;
    }
    for (i=0; i<gstate.active_tasks.active_tasks.size(); i++) {
        ACTIVE_TASK* atp = gstate.active_tasks.active_tasks[i];
        double x = atp->est_dur(false) / atp->result->estimated_duration(false);
        APP_VERSION* avp = atp->result->avp;
        if (x < avp->temp_dcf) {
            avp->temp_dcf = x;
        }
    }
}
#endif

void CLIENT_STATE::rr_simulation() {
    PROJECT* p, *pbest;
    RESULT* rp, *rpbest;
    RR_SIM_STATUS sim_status;
    unsigned int i;

    double ar = available_ram();

    work_fetch.rr_init();
    //compute_temp_dcf();

    if (log_flags.rr_simulation) {
        msg_printf(0, MSG_INFO,
            "[rr_sim] rr_sim start: work_buf_total %.2f on_frac %.3f active_frac %.3f",
            work_buf_total(), time_stats.on_frac, time_stats.active_frac
        );
    }

    for (i=0; i<projects.size(); i++) {
        p = projects[i];
        if (p->non_cpu_intensive) continue;
        p->rr_sim_status.clear();
    }

    // Decide what jobs to include in the simulation,
    // and pick the ones that are initially running.
    // NOTE: "results" is sorted by increasing arrival time
    //
    for (i=0; i<results.size(); i++) {
        rp = results[i];
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
        p->cpu_pwf.nused_total += rp->avp->avg_ncpus;
        if (rp->uses_cuda() && coproc_cuda) {
            p->cuda_pwf.nused_total += rp->avp->ncudas;
            p->cuda_pwf.has_runnable_jobs = true;
            if (cuda_work_fetch.sim_nused < coproc_cuda->count) {
                sim_status.activate(rp, 0);
                p->rr_sim_status.activate(rp);
            } else {
                cuda_work_fetch.pending.push_back(rp);
            }
        } else if (rp->uses_ati() && coproc_ati) {
            p->ati_pwf.nused_total += rp->avp->natis;
            p->ati_pwf.has_runnable_jobs = true;
            if (ati_work_fetch.sim_nused < coproc_ati->count) {
                sim_status.activate(rp, 0);
                p->rr_sim_status.activate(rp);
            } else {
                ati_work_fetch.pending.push_back(rp);
            }
        } else {
            p->cpu_pwf.has_runnable_jobs = true;
            if (p->cpu_pwf.sim_nused < ncpus) {
                sim_status.activate(rp, 0);
                p->rr_sim_status.activate(rp);
            } else {
                p->rr_sim_status.add_pending(rp);
            }
        }
    }

    // note the number of idle instances
    //
    cpu_work_fetch.nidle_now = ncpus - cpu_work_fetch.sim_nused;
    if (cpu_work_fetch.nidle_now < 0) cpu_work_fetch.nidle_now = 0;
    if (coproc_cuda) {
        cuda_work_fetch.nidle_now = coproc_cuda->count - cuda_work_fetch.sim_nused;
        if (cuda_work_fetch.nidle_now < 0) cuda_work_fetch.nidle_now = 0;
    }
    if (coproc_ati) {
        ati_work_fetch.nidle_now = coproc_ati->count - ati_work_fetch.sim_nused;
        if (ati_work_fetch.nidle_now < 0) ati_work_fetch.nidle_now = 0;
    }

    work_fetch.compute_shares();

    // Simulation loop.  Keep going until all work done
    //
    double buf_end = now + work_buf_total();
    double sim_now = now;
    while (sim_status.active.size()) {

        // compute finish times and see which result finishes first
        //
        rpbest = NULL;
        for (i=0; i<sim_status.active.size(); i++) {
            rp = sim_status.active[i];
            set_rrsim_flops(rp);
            //rp->rrsim_finish_delay = rp->avp->temp_dcf*rp->rrsim_flops_left/rp->rrsim_flops;
            rp->rrsim_finish_delay = rp->rrsim_flops_left/rp->rrsim_flops;
            if (!rpbest || rp->rrsim_finish_delay < rpbest->rrsim_finish_delay) {
                rpbest = rp;
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
            if (atp && atp->procinfo.working_set_size_smoothed > ar) {
                if (log_flags.rr_simulation) {
                    msg_printf(pbest, MSG_INFO,
                        "[rr_sim] %s misses deadline but too large to run",
                        rpbest->name
                    );
                }
            } else {
                rpbest->rr_sim_misses_deadline = true;
                if (rpbest->uses_cuda()) {
                    pbest->cuda_pwf.deadlines_missed++;
                    cuda_work_fetch.deadline_missed_instances += rpbest->avp->ncudas;
                } else if (rpbest->uses_ati()) {
                    pbest->ati_pwf.deadlines_missed++;
                    ati_work_fetch.deadline_missed_instances += rpbest->avp->natis;
                } else {
                    pbest->cpu_pwf.deadlines_missed++;
                    cpu_work_fetch.deadline_missed_instances += rpbest->avp->avg_ncpus;
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
        cpu_work_fetch.update_saturated_time(x);
        if (coproc_cuda) {
            cuda_work_fetch.update_saturated_time(x);
        }
        if (coproc_ati) {
            ati_work_fetch.update_saturated_time(x);
        }

        // update busy time
        //
        if (rpbest->rr_sim_misses_deadline) {
            double dur = rpbest->estimated_time_remaining(false) / gstate.overall_cpu_frac();
            cpu_work_fetch.update_busy_time(dur, rpbest->avp->avg_ncpus);
            if (rpbest->uses_cuda()) {
                cuda_work_fetch.update_busy_time(dur, rpbest->avp->ncudas);
            }
            if (rpbest->uses_ati()) {
                ati_work_fetch.update_busy_time(dur, rpbest->avp->natis);
            }
        }

        // increment resource shortfalls
        //
        if (sim_now < buf_end) {
            if (end_time > buf_end) end_time = buf_end;
            double d_time = end_time - sim_now;

            cpu_work_fetch.accumulate_shortfall(d_time);

            if (coproc_cuda) {
                cuda_work_fetch.accumulate_shortfall(d_time);
            }
            if (coproc_ati) {
                ati_work_fetch.accumulate_shortfall(d_time);
            }
        }

        sim_status.remove_active(rpbest);
        pbest->rr_sim_status.remove_active(rpbest);

        sim_now += rpbest->rrsim_finish_delay;

        // start new jobs; may need to start more than one
        // if this job used multiple resource instances
        //
        if (rpbest->uses_cuda()) {
            while (1) {
                if (cuda_work_fetch.sim_nused >= coproc_cuda->count) break;
                if (!cuda_work_fetch.pending.size()) break;
                RESULT* rp = cuda_work_fetch.pending[0];
                cuda_work_fetch.pending.erase(cuda_work_fetch.pending.begin());
                sim_status.activate(rp, sim_now-now);
                pbest->rr_sim_status.activate(rp);
            }
        } else if (rpbest->uses_ati()) {
            while (1) {
                if (ati_work_fetch.sim_nused >= coproc_ati->count) break;
                if (!ati_work_fetch.pending.size()) break;
                RESULT* rp = ati_work_fetch.pending[0];
                ati_work_fetch.pending.erase(ati_work_fetch.pending.begin());
                sim_status.activate(rp, sim_now-now);
                pbest->rr_sim_status.activate(rp);
            }
        } else {
            while (1) {
                if (pbest->cpu_pwf.sim_nused >= ncpus) break;
                RESULT* rp = pbest->rr_sim_status.get_pending();
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
        cpu_work_fetch.accumulate_shortfall(d_time);
        if (coproc_cuda) {
            cuda_work_fetch.accumulate_shortfall(d_time);
        }
        if (coproc_ati) {
            ati_work_fetch.accumulate_shortfall(d_time);
        }
    }
}
