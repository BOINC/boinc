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

#include "client_state.h"
#include "client_msgs.h"

struct RR_SIM_STATUS {
    std::vector<RESULT*> active;
    COPROCS coprocs;

    inline bool can_run(RESULT* rp) {
        return coprocs.sufficient_coprocs(
            rp->avp->coprocs, log_flags.rr_simulation, "rr_simulation"
        );
    }
    inline void activate(RESULT* rp) {
        coprocs.reserve_coprocs(
            rp->avp->coprocs, rp, log_flags.rr_simulation, "rr_simulation"
        );
        active.push_back(rp);
    }
    // remove *rpbest from active set,
    // and adjust CPU time left for other results
    //
    inline void remove_active(RESULT* rpbest) {
        coprocs.free_coprocs(rpbest->avp->coprocs, rpbest, log_flags.rr_simulation, "rr_simulation");
        vector<RESULT*>::iterator it = active.begin();
        while (it != active.end()) {
            RESULT* rp = *it;
            if (rp == rpbest) {
                it = active.erase(it);
            } else {
                rp->rrsim_cpu_left -= rp->project->rr_sim_status.proc_rate*rpbest->rrsim_finish_delay;
                it++;
            }
        }
    }
    inline int nactive() {
        return (int) active.size();
    }
    ~RR_SIM_STATUS() {
        coprocs.delete_coprocs();
    }
};


// Do a simulation of the current workload
// with weighted round-robin (WRR) scheduling.
// Include jobs that are downloading.
//
// For efficiency, we simulate a crude approximation of WRR.
// We don't model time-slicing.
// Instead we use a continuous model where, at a given point,
// each project has a set of running jobs that uses all CPUs
// (and obeys coprocessor limits).
// These jobs are assumed to run at a rate proportionate to their avg_ncpus,
// and each project gets CPU proportionate to its RRS.
//
// Outputs are changes to global state:
// For each project p:
//   p->rr_sim_deadlines_missed
//   p->cpu_shortfall
// For each result r:
//   r->rr_sim_misses_deadline
//   r->last_rr_sim_missed_deadline
// gstate.cpu_shortfall
//
// Deadline misses are not counted for tasks
// that are too large to run in RAM right now.
//
void CLIENT_STATE::rr_simulation() {
    double rrs = nearly_runnable_resource_share();
    double trs = total_resource_share();
    PROJECT* p, *pbest;
    RESULT* rp, *rpbest;
    RR_SIM_STATUS sim_status;
    unsigned int i;

    sim_status.coprocs.clone(coprocs, false);
    double ar = available_ram();

    if (log_flags.rr_simulation) {
        msg_printf(0, MSG_INFO,
            "[rr_sim] rr_sim start: work_buf_total %f rrs %f trs %f ncpus %d",
            work_buf_total(), rrs, trs, ncpus
        );
    }

    for (i=0; i<projects.size(); i++) {
        p = projects[i];
        p->rr_sim_status.clear();
    }

    // Decide what jobs to include in the simulation,
    // and pick the ones that are initially running
    //
    for (i=0; i<results.size(); i++) {
        rp = results[i];
        if (!rp->nearly_runnable()) continue;
        if (rp->some_download_stalled()) continue;
        if (rp->project->non_cpu_intensive) continue;
        rp->rrsim_cpu_left = rp->estimated_cpu_time_remaining(false);
        p = rp->project;
        if (p->rr_sim_status.can_run(rp, gstate.ncpus) && sim_status.can_run(rp)) {
            sim_status.activate(rp);
            p->rr_sim_status.activate(rp);
        } else {
            p->rr_sim_status.add_pending(rp);
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
        if (p->rr_sim_status.none_active()) {
            double rsf = trs ? p->resource_share/trs : 1;
            p->rr_sim_status.cpu_shortfall = work_buf_total() * overall_cpu_frac() * ncpus * rsf;
            if (log_flags.rr_simulation) {
                msg_printf(p, MSG_INFO,
                    "[rr_sim] no results; shortfall %f wbt %f ocf %f rsf %f",
                    p->rr_sim_status.cpu_shortfall, work_buf_total(), overall_cpu_frac(), rsf
                );
            }
        }
    }

    double buf_end = now + work_buf_total();

    // Simulation loop.  Keep going until work done
    //
    double sim_now = now;
    cpu_shortfall = 0;
    while (sim_status.nactive()) {

        // compute finish times and see which result finishes first
        //
        rpbest = NULL;
        for (i=0; i<sim_status.active.size(); i++) {
            rp = sim_status.active[i];
            p = rp->project;
            rp->rrsim_finish_delay = rp->rrsim_cpu_left/p->rr_sim_status.proc_rate;
            if (!rpbest || rp->rrsim_finish_delay < rpbest->rrsim_finish_delay) {
                rpbest = rp;
            }
        }

        pbest = rpbest->project;

        if (log_flags.rr_simulation) {
            msg_printf(pbest, MSG_INFO,
                "[rr_sim] result %s finishes after %f (%f/%f)",
                rpbest->name, rpbest->rrsim_finish_delay,
                rpbest->rrsim_cpu_left, pbest->rr_sim_status.proc_rate
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
                pbest->rr_sim_status.deadlines_missed++;
                if (log_flags.rr_simulation) {
                    msg_printf(pbest, MSG_INFO,
                        "[rr_sim] result %s misses deadline by %f",
                        rpbest->name, diff
                    );
                }
            }
        }

        int last_active_size = sim_status.nactive();
        int last_proj_active_size = pbest->rr_sim_status.cpus_used();

        sim_status.remove_active(rpbest);
 
        pbest->rr_sim_status.remove_active(rpbest);

        // If project has more results, add one or more to active set.
        //
        while (1) {
            rp = pbest->rr_sim_status.get_pending();
            if (!rp) break;
            if (pbest->rr_sim_status.can_run(rp, gstate.ncpus) && sim_status.can_run(rp)) {
                sim_status.activate(rp);
                pbest->rr_sim_status.activate(rp);
            } else {
                pbest->rr_sim_status.add_pending(rp);
                break;
            }
        }

        // If all work done for a project, subtract that project's share
        // and recompute processing rates
        //
        if (pbest->rr_sim_status.none_active()) {
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
                pbest->rr_sim_status.cpu_shortfall += d_time*(proj_cpu_share - last_proj_active_size);
                if (log_flags.rr_simulation) {
                    msg_printf(pbest, MSG_INFO,
                        "[rr_sim] new shortfall %f d_time %f proj_cpu_share %f lpas %d",
                        pbest->rr_sim_status.cpu_shortfall, d_time, proj_cpu_share, last_proj_active_size
                    );
                }
            }

            if (end_time < buf_end) {
                d_time = buf_end - end_time;
                // if this is the last result for this project, account for the tail
                if (pbest->rr_sim_status.none_active()) { 
                    pbest->rr_sim_status.cpu_shortfall += d_time * proj_cpu_share;
                    if (log_flags.rr_simulation) {
                         msg_printf(pbest, MSG_INFO, "[rr_sim] proj out of work; shortfall %f d %f pcs %f",
                             pbest->rr_sim_status.cpu_shortfall, d_time, proj_cpu_share
                         );
                    }
                }
            }
            if (log_flags.rr_simulation) {
                msg_printf(0, MSG_INFO,
                    "[rr_sim] total: idle cpus %d, last active %d, active %d, shortfall %f",
                    nidle_cpus, last_active_size, sim_status.nactive(),
                    cpu_shortfall
                    
                );
                msg_printf(0, MSG_INFO,
                    "[rr_sim] proj %s: last active %d, active %d, shortfall %f",
                    pbest->get_project_name(), last_proj_active_size,
                    pbest->rr_sim_status.cpus_used(),
                    pbest->rr_sim_status.cpu_shortfall
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
            if (p->rr_sim_status.cpu_shortfall) {
                msg_printf(p, MSG_INFO,
                    "[rr_sim] shortfall %f\n", p->rr_sim_status.cpu_shortfall
                );
            }
        }
        msg_printf(NULL, MSG_INFO,
            "[rr_sim] done; total shortfall %f\n",
            cpu_shortfall
        );
    }
}
