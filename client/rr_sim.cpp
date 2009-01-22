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

#include "coproc.h"
#include "client_msgs.h"

// this is here (rather than rr_sim.h) because its inline functions
// refer to RESULT
//
struct RR_SIM_STATUS {
    std::vector<RESULT*> active;
    COPROCS coprocs;
	double active_ncpus;
    double active_cudas;

    inline bool can_run(RESULT* rp) {
        return coprocs.sufficient_coprocs(
            rp->avp->coprocs, false, "rr_sim"
        );
    }
    inline void activate(RESULT* rp, double when) {
        coprocs.reserve_coprocs(
            rp->avp->coprocs, rp, false, "rr_sim"
        );
		if (log_flags.rr_simulation) {
			msg_printf(rp->project, MSG_INFO,
				"[rr_sim] starting at %f: %s", when, rp->name
			);
		}
        active.push_back(rp);
		active_ncpus += rp->avp->avg_ncpus;
        active_cudas += rp->avp->ncudas;
    }
    // remove *rpbest from active set,
    // and adjust CPU time left for other results
    //
    inline void remove_active(RESULT* rpbest) {
        coprocs.free_coprocs(rpbest->avp->coprocs, rpbest, false, "rr_sim");
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
		active_ncpus -= rpbest->avp->avg_ncpus;
        active_cudas -= rpbest->avp->ncudas;
    }

	RR_SIM_STATUS() {
		active_ncpus = 0;
        active_cudas = 0;
	}
    ~RR_SIM_STATUS() {
        coprocs.delete_coprocs();
    }
};

void RR_SIM_PROJECT_STATUS::activate(RESULT* rp) {
    active.push_back(rp);
	active_ncpus += rp->avp->avg_ncpus;
    active_cudas += rp->avp->ncudas;
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
    active_cudas -= r->avp->ncudas;
}

// estimate the rate (FLOPS) that this job will get long-term
// with weighted round-robin scheduling
//
void set_rrsim_flops(RESULT* rp) {
	// if it's a coproc job, use app version estimate
    if (rp->uses_coprocs()) {
        rp->rrsim_flops = rp->avp->flops;
		return;
    }
    PROJECT* p = rp->project;

	// first, estimate how many CPU seconds per second this job would get
	// running with other jobs of this project, ignoring other factors
	//
	double x = 1;
	if (p->rr_sim_status.active_ncpus > gstate.ncpus) {
		x = gstate.ncpus/p->rr_sim_status.active_ncpus;
	}
	double r1 = x*rp->avp->avg_ncpus;

	// if the project's total CPU usage is more than its share, scale
	//
	double share_cpus = p->cpu_pwf.runnable_share*gstate.ncpus;
	double r2 = r1;
	if (p->rr_sim_status.active_ncpus > share_cpus) {
		r2 *= (share_cpus / p->rr_sim_status.active_ncpus);
	}

	// scale by overall CPU availability
	//
	double r3 = r2 * gstate.overall_cpu_frac();

    rp->rrsim_flops = r3 * gstate.host_info.p_fpops;
#if 0
    if (log_flags.rr_simulation) {
        msg_printf(p, MSG_INFO,
            "[rr_sim] set_rrsim_flops: %f (r1 %f r2 %f r3 %f)",
            rp->rrsim_flops, r1, r2, r3
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
                "[cpu_sched_debug] Result %s projected to miss deadline.",
                rp->name
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
// For efficiency, we simulate an approximation of WRR.
// We don't model time-slicing.
// Instead we use a continuous model where, at a given point,
// each project has a set of running jobs that uses at most all CPUs
// (and obeys coprocessor limits).
// These jobs are assumed to run at a rate proportionate to their avg_ncpus,
// and each project gets CPU proportionate to its RRS.
//
// Outputs are changes to global state:
// - deadline misses (per-project count, per-result flag)
//      Deadline misses are not counted for tasks
//      that are too large to run in RAM right now.
// - resource shortfalls (per-project and total)
// - counts of resources idle now
//
void CLIENT_STATE::rr_simulation() {
    PROJECT* p, *pbest;
    RESULT* rp, *rpbest;
    RR_SIM_STATUS sim_status;
    unsigned int i;

    sim_status.coprocs.clone(coprocs, false);
    double ar = available_ram();

    work_fetch.rr_init();

    if (log_flags.rr_simulation) {
        msg_printf(0, MSG_INFO,
            "[rr_sim] rr_sim start: now %.2f work_buf_total %.2f",
            now, work_buf_total()
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
        rp->rrsim_flops_left = rp->estimated_flops_remaining();
        if (rp->rrsim_flops_left <= 0) continue;
        p = rp->project;
        if (p->rr_sim_status.can_run(rp, gstate.ncpus) && sim_status.can_run(rp)) {
            sim_status.activate(rp, now);
            p->rr_sim_status.activate(rp);
        } else {
            p->rr_sim_status.add_pending(rp);
        }
        rp->rr_sim_misses_deadline = false;
		if (rp->uses_coprocs()) {
			p->rr_sim_status.has_cuda_jobs = true;
		} else {
			p->rr_sim_status.has_cpu_jobs = true;
		}
    }

    // note the number of idle instances
    //
    cpu_work_fetch.nidle_now = ncpus - sim_status.active_ncpus;
    if (cpu_work_fetch.nidle_now < 0) cpu_work_fetch.nidle_now = 0;
    if (coproc_cuda) {
        cuda_work_fetch.nidle_now = coproc_cuda->count - coproc_cuda->used;
        if (cuda_work_fetch.nidle_now < 0) cuda_work_fetch.nidle_now = 0;
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
            rp->rrsim_finish_delay = rp->rrsim_flops_left/rp->rrsim_flops;
            if (!rpbest || rp->rrsim_finish_delay < rpbest->rrsim_finish_delay) {
                rpbest = rp;
            }
        }

        pbest = rpbest->project;

        if (log_flags.rr_simulation) {
            msg_printf(pbest, MSG_INFO,
                "[rr_sim] %s finishes after %.2f (%.2fG/%.2fG)",
                rpbest->name, rpbest->rrsim_finish_delay,
                rpbest->rrsim_flops_left/1e9, rpbest->rrsim_flops/1e9
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
                        "[rr_sim] %s misses deadline but too large to run",
                        rpbest->name
                    );
                }
            } else {
                rpbest->rr_sim_misses_deadline = true;
                pbest->rr_sim_status.deadlines_missed++;
                if (log_flags.rr_simulation) {
                    msg_printf(pbest, MSG_INFO,
                        "[rr_sim] %s misses deadline by %.2f",
                        rpbest->name, diff
                    );
                }
            }
        }

        // increment resource shortfalls
        //
        if (sim_now < buf_end) {
            double end_time = sim_now + rpbest->rrsim_finish_delay;
            if (end_time > buf_end) end_time = buf_end;
            double d_time = end_time - sim_now;

            if (sim_status.active_ncpus >= ncpus) {
                work_fetch.estimated_delay = end_time - gstate.now;
            }

            cpu_work_fetch.accumulate_shortfall(d_time, sim_status.active_ncpus);

            if (coproc_cuda) {
                cuda_work_fetch.accumulate_shortfall(d_time, sim_status.active_cudas);
            }
        }

        sim_status.remove_active(rpbest);
        pbest->rr_sim_status.remove_active(rpbest);

        // If project has more results, add one or more to active set.
		// TODO: do this for other projects too, since coproc may have been freed
        //
        while (1) {
            rp = pbest->rr_sim_status.get_pending();
            if (!rp) break;
            if (pbest->rr_sim_status.can_run(rp, gstate.ncpus) && sim_status.can_run(rp)) {
                sim_status.activate(rp, sim_now);
                pbest->rr_sim_status.activate(rp);
            } else {
                pbest->rr_sim_status.add_pending(rp);
                break;
            }
        }
        sim_now += rpbest->rrsim_finish_delay;
    }

	// if simulation ends before end of buffer, take the tail into account
	//
    if (sim_now < buf_end) {
		double d_time = buf_end - sim_now;
        cpu_work_fetch.accumulate_shortfall(d_time, 0);
        if (coproc_cuda) {
            cuda_work_fetch.accumulate_shortfall(d_time, 0);
        }
    }
}
