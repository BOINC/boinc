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

#ifdef _WIN32
#include "boinc_win.h"
#endif

#ifdef SIM
#include "sim.h"
#else
#include "client_state.h"
#endif
#include "client_msgs.h"

struct RR_SIM_STATUS {
    std::vector<RESULT*> active;
    COPROCS coprocs;
	double active_ncpus;

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
		active_ncpus += rp->avp->avg_ncpus;
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
                rp->rrsim_cpu_left -= rp->rrsim_rate*rpbest->rrsim_finish_delay;

                // can be slightly less than 0 due to roundoff
                //
                if (rp->rrsim_cpu_left < -1) {
                    msg_printf(rp->project, MSG_INTERNAL_ERROR,
                        "%s: negative CPU time left %f", rp->name, rp->rrsim_cpu_left
                    );
                }
                if (rp->rrsim_cpu_left < 0) {
                    rp->rrsim_cpu_left = 0;
                }
                it++;
            }
        }
		active_ncpus -= rpbest->avp->avg_ncpus;
    }
#if 0
    inline int nactive() {
        return (int) active.size();
    }
#endif
	RR_SIM_STATUS() {
		active_ncpus = 0;
	}
    ~RR_SIM_STATUS() {
        coprocs.delete_coprocs();
    }
};

void RR_SIM_PROJECT_STATUS::activate(RESULT* rp) {
    active.push_back(rp);
	active_ncpus += rp->avp->avg_ncpus;
}

bool RR_SIM_PROJECT_STATUS::can_run(RESULT* rp, int ncpus) {
	if (rp->uses_coprocs()) return true;
    return active_ncpus < ncpus;
}
void RR_SIM_PROJECT_STATUS::remove_active(RESULT* r) {
    std::vector<RESULT*>::iterator it = active.begin();
    while (it != active.end()) {
        if (*it == r) {
            it = active.erase(it);
        } else {
            it++;
        }
    }
	active_ncpus -= r->avp->avg_ncpus;
}

// Set the project's rrsim_proc_rate:
// the fraction of CPU that it will get in round-robin mode.
//
void PROJECT::set_rrsim_proc_rate(double rrs) {
    double x;

    if (rrs) {
        x = resource_share/rrs;
    } else {
        x = 1;      // pathological case; maybe should be 1/# runnable projects
    }

    rr_sim_status.proc_rate = x*gstate.overall_cpu_frac();
    if (log_flags.rr_simulation) {
        msg_printf(this, MSG_INFO,
            "[rr_sim] set_rrsim_proc_rate: %f (rrs %f, rs %f, ocf %f",
            rr_sim_status.proc_rate, rrs, resource_share, gstate.overall_cpu_frac()
        );
    }
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
        if (p->rr_sim_status.deadlines_missed) {
            msg_printf(p, MSG_INFO,
                "[cpu_sched_debug] Project has %d projected deadline misses",
                p->rr_sim_status.deadlines_missed
            );
        }
    }
}

// Do a simulation of the current workload
// with weighted round-robin (WRR) scheduling.
// Include jobs that are downloading.
//
// For efficiency, we simulate a crude approximation of WRR.
// We don't model time-slicing.
// Instead we use a continuous model where, at a given point,
// each project has a set of running jobs that uses at most all CPUs
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
        if (p->non_cpu_intensive) continue;
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
        if (rp->rrsim_cpu_left <= 0) continue;
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
        if (p->non_cpu_intensive) continue;
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
	bool any_pending = false;
    while (sim_status.active.size()) {

        // compute finish times and see which result finishes first
        //
        rpbest = NULL;
        for (i=0; i<sim_status.active.size(); i++) {
            rp = sim_status.active[i];
            p = rp->project;
			rp->rrsim_rate = p->rr_sim_status.proc_rate;
			if (p->rr_sim_status.active_ncpus < ncpus) {
				rp->rrsim_rate *= (ncpus/p->rr_sim_status.active_ncpus);
			}
			rp->rrsim_rate *= rp->avp->avg_ncpus/p->rr_sim_status.active_ncpus;
			if (rp->rrsim_rate > rp->avp->avg_ncpus * overall_cpu_frac()) {
				rp->rrsim_rate = rp->avp->avg_ncpus * overall_cpu_frac();
			}
            rp->rrsim_finish_delay = rp->rrsim_cpu_left/rp->rrsim_rate;
            if (!rpbest || rp->rrsim_finish_delay < rpbest->rrsim_finish_delay) {
                rpbest = rp;
            }
        }

        pbest = rpbest->project;

        if (log_flags.rr_simulation) {
            msg_printf(pbest, MSG_INFO,
                "[rr_sim] result %s finishes after %f (%f/%f)",
                rpbest->name, rpbest->rrsim_finish_delay,
                rpbest->rrsim_cpu_left, rpbest->rrsim_rate
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

        double last_active_ncpus = sim_status.active_ncpus;
        double last_proj_active_ncpus = pbest->rr_sim_status.active_ncpus;

        sim_status.remove_active(rpbest);
 
        pbest->rr_sim_status.remove_active(rpbest);

		// record whether project has pending jobs;
		// if so, we won't increment CPU shortfall
		//
		bool pbest_had_pending = (pbest->rr_sim_status.pending.size() > 0);
		any_pending = false;
        for (i=0; i<projects.size(); i++) {
            p = projects[i];
            if (p->non_cpu_intensive) continue;
			if (p->rr_sim_status.pending.size()) {
				any_pending = true;
				break;
			}
		}

        // If project has more results, add one or more to active set.
		// TODO: do this for other projects too, since coproc may have been freed
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
                if (p->non_cpu_intensive) continue;
                p->set_rrsim_proc_rate(rrs);
            }
        }

        // increment CPU shortfalls if necessary
        //
        if (sim_now < buf_end) {
            double end_time = sim_now + rpbest->rrsim_finish_delay;
            if (end_time > buf_end) end_time = buf_end;
            double d_time = end_time - sim_now;
            double nidle_cpus = ncpus - last_active_ncpus;
            if (nidle_cpus<0) nidle_cpus = 0;
			if (nidle_cpus > 0 && !any_pending) {
				cpu_shortfall += d_time*nidle_cpus;
			}

            double rsf = trs?pbest->resource_share/trs:1;
            double proj_cpu_share = ncpus*rsf;

            if (!pbest_had_pending && last_proj_active_ncpus < proj_cpu_share) {
                pbest->rr_sim_status.cpu_shortfall += d_time*(proj_cpu_share - last_proj_active_ncpus);
                if (log_flags.rr_simulation) {
                    msg_printf(pbest, MSG_INFO,
                        "[rr_sim] new shortfall %f d_time %f proj_cpu_share %f lpan %f",
                        pbest->rr_sim_status.cpu_shortfall, d_time, proj_cpu_share, last_proj_active_ncpus
                    );
                }
            }

            if (end_time < buf_end) {
                d_time = buf_end - end_time;
                // if this is the last result for this project, account for the tail
                if (pbest->rr_sim_status.none_active() && pbest->rr_sim_status.none_pending()) { 
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
                    "[rr_sim] total: idle cpus %f, last active %f, active %f, shortfall %f",
                    nidle_cpus, last_active_ncpus, sim_status.active_ncpus,
                    cpu_shortfall
                    
                );
                msg_printf(pbest, MSG_INFO,
                    "[rr_sim] %s: last active %f, active %f, shortfall %f",
                    pbest->get_project_name(), last_proj_active_ncpus,
                    pbest->rr_sim_status.active_ncpus,
                    pbest->rr_sim_status.cpu_shortfall
                );
            }
        }

        sim_now += rpbest->rrsim_finish_delay;
    }

    if (sim_now < buf_end && !any_pending) {
        cpu_shortfall += (buf_end - sim_now) * ncpus;
    }

    if (log_flags.rr_simulation) {
        for (i=0; i<projects.size(); i++) {
            p = projects[i];
            if (p->non_cpu_intensive) continue;
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
