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

// This is a test framework for the rr_simulation() function.
// To use it:
// - cut and paste the current code from cpu_sched.C (see below)
// - edit main() to set up your test case

#include <vector>
#include <cstdarg>

using std::vector;

#define CPU_PESSIMISM_FACTOR 1.0
#define SECONDS_PER_DAY 86400

struct RESULT;

struct PROJECT {
    char name[256];
    double resource_share;
    bool non_cpu_intensive;
    vector<RESULT*>active;
    vector<RESULT*>pending;
    double cpu_shortfall;
    double rrsim_proc_rate;
    int rr_sim_deadlines_missed;
    PROJECT(char* n, double rs) {
        strcpy(name,  n);
        resource_share = rs;
        non_cpu_intensive = false;
    }
    char* get_project_name() {
        return name;
    }
    void set_rrsim_proc_rate(double);
};

struct RESULT {
    char name[256];
    double ectr;
    double estimated_cpu_time_remaining() {
        return ectr;
    }
    double rrsim_finish_delay;
    double rrsim_cpu_left;
    double report_deadline;
    bool rr_sim_misses_deadline;
    bool last_rr_sim_missed_deadline;
    PROJECT* project;
    RESULT(PROJECT* p, char* n, double e, double rd) {
        project = p;
        strcpy(name,  n);
        ectr = e;
        report_deadline = rd;
    }
    bool nearly_runnable() {
        return true;
    }
    double computation_deadline();
};

struct FLAGS {
    bool rr_simulation;
};

FLAGS log_flags;

struct PREFS {
    double work_buf_min_days;
    double work_buf_additional_days;
    double cpu_scheduling_period_minutes;
};

struct CLIENT_STATE {
    double nearly_runnable_resource_share();
    double total_resource_share();
    double now;
    int ncpus;
    vector<RESULT*>results;
    vector<PROJECT*>projects;
    PREFS global_prefs;
    double cpu_shortfall;
    double overall_cpu_frac() {
        return 1;
    }
    bool rr_simulation();
    double work_buf_min() {
        return global_prefs.work_buf_min_days*86400;
    }
    double work_buf_additional() {
        return global_prefs.work_buf_additional_days * 86400;
    }
};

struct CLIENT_STATE gstate;

double CLIENT_STATE::nearly_runnable_resource_share() {
    double x=0;
    for (unsigned int i=0; i<projects.size(); i++) {
        x += projects[i]->resource_share;
    }
    return x;
}

double CLIENT_STATE::total_resource_share() {
    return nearly_runnable_resource_share();
}

#define MSG_INFO 0

void msg_printf(PROJECT* p, int, const char* fmt, ...) {
    char buf[8192];
    va_list ap;
    va_start(ap, fmt);
    vsprintf(buf, fmt, ap);
    va_end(ap);
    printf("%s: %s\n", p?p->name:"BOINC", buf);
}

/////////////////////  CUT AND PASTE FROM CPU_SCHED.C //////////////

double RESULT::computation_deadline() {
    return report_deadline - (
        gstate.global_prefs.work_buf_min_days * SECONDS_PER_DAY
            // Seconds that the host will not be connected to the Internet
        + gstate.global_prefs.cpu_scheduling_period()
            // Seconds that the CPU may be busy with some other result
        + SECONDS_PER_DAY
            // Deadline cusion
    );
}

void PROJECT::set_rrsim_proc_rate(double rrs) {
    int nactive = (int)active.size();
    if (nactive == 0) return;
    double x;

    if (rrs) {
        x = resource_share/rrs;
    } else {
        x = 1;      // pathological case; maybe should be 1/# runnable projects
    }

    // if this project has fewer active results than CPUs,
    // scale up its share to reflect this
    //
    if (nactive < gstate.ncpus) {
        x *= ((double)gstate.ncpus)/nactive;
    }

    // But its rate on a given CPU can't exceed 1
    //
    if (x>1) {
        x = 1;
    }
	rrsim_proc_rate = x*gstate.overall_cpu_frac();
    if (log_flags.rr_simulation) {
        msg_printf(this, MSG_INFO,
            "set_rrsim_proc_rate: %f (rrs %f, rs %f, nactive %d, ocf %f",
            rrsim_proc_rate, rrs, resource_share, nactive, gstate.overall_cpu_frac()
        );
    }
}
bool CLIENT_STATE::rr_simulation() {
	double rrs = nearly_runnable_resource_share();
    double trs = total_resource_share();
    PROJECT* p, *pbest;
    RESULT* rp, *rpbest;
    vector<RESULT*> active;
    unsigned int i;
    double x;
    vector<RESULT*>::iterator it;
    bool rval = false;

	if (log_flags.rr_simulation) {
		msg_printf(0, MSG_INFO,
            "rr_sim start: work_buf_min %f rrs %f trs %f",
            work_buf_min(), rrs, trs
        );
	}

    // Initialize result lists for each project:
	// "active" is what's currently running (in the simulation)
	// "pending" is what's queued
    //
    for (i=0; i<projects.size(); i++) {
        p = projects[i];
        p->active.clear();
        p->pending.clear();
        p->rr_sim_deadlines_missed = 0;
        p->cpu_shortfall = 0;
    }

    for (i=0; i<results.size(); i++) {
        rp = results[i];
        if (!rp->nearly_runnable()) continue;
        if (rp->project->non_cpu_intensive) continue;
        rp->rrsim_cpu_left = rp->estimated_cpu_time_remaining();
        p = rp->project;
        if (p->active.size() < (unsigned int)ncpus) {
            active.push_back(rp);
            p->active.push_back(rp);
        } else {
            p->pending.push_back(rp);
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
        if (!p->active.size()) {
			double rsf = trs ? p->resource_share/trs : 1;
            p->cpu_shortfall = (work_buf_min()+work_buf_additional()) * overall_cpu_frac() * ncpus * rsf;
            if (log_flags.rr_simulation) {
                msg_printf(p, MSG_INFO,
                    "no results; shortfall %f wbm %f ocf %f rsf %f",
                    p->cpu_shortfall, work_buf_min(), overall_cpu_frac(), rsf
                );
            }
        }
    }

    double buf_end = now + work_buf_min() + work_buf_additional();

    // Simulation loop.  Keep going until work done
    //
    double sim_now = now;
    cpu_shortfall = 0;
    while (active.size()) {

        // compute finish times and see which result finishes first
        //
        rpbest = NULL;
        for (i=0; i<active.size(); i++) {
            rp = active[i];
            p = rp->project;
            rp->rrsim_finish_delay = rp->rrsim_cpu_left/p->rrsim_proc_rate;
            if (!rpbest || rp->rrsim_finish_delay < rpbest->rrsim_finish_delay) {
                rpbest = rp;
            }
        }

        pbest = rpbest->project;

        if (log_flags.rr_simulation) {
            msg_printf(pbest, MSG_INFO,
                "rr_sim: result %s finishes after %f (%f/%f)",
				rpbest->name, rpbest->rrsim_finish_delay, rpbest->rrsim_cpu_left, pbest->rrsim_proc_rate
            );
        }

        // "rpbest" is first result to finish.  Does it miss its deadline?
        //
        double diff = sim_now + rpbest->rrsim_finish_delay - ((rpbest->computation_deadline()-now)*CPU_PESSIMISM_FACTOR + now);
        if (diff > 0) {
            rpbest->rr_sim_misses_deadline = true;
            pbest->rr_sim_deadlines_missed++;
            rval = true;
			if (log_flags.rr_simulation) {
				msg_printf(0, MSG_INFO,
					"rr_sim: result %s misses deadline by %f",
					rpbest->name, diff
				);
			}
        }

        int last_active_size = active.size();
        int last_proj_active_size = pbest->active.size();

        // remove *rpbest from active set,
        // and adjust CPU time left for other results
        //
        it = active.begin();
        while (it != active.end()) {
            rp = *it;
            if (rp == rpbest) {
                it = active.erase(it);
            } else {
                x = rp->project->rrsim_proc_rate*rpbest->rrsim_finish_delay;
                rp->rrsim_cpu_left -= x;
                it++;
            }
        }

        // remove *rpbest from its project's active set
        //
        it = pbest->active.begin();
        while (it != pbest->active.end()) {
            rp = *it;
            if (rp == rpbest) {
                it = pbest->active.erase(it);
            } else {
                it++;
            }
        }

        // If project has more results, add one to active set.
        //
        if (pbest->pending.size()) {
            rp = pbest->pending[0];
            pbest->pending.erase(pbest->pending.begin());
            active.push_back(rp);
            pbest->active.push_back(rp);
        }

        // If all work done for a project, subtract that project's share
        // and recompute processing rates
        //
        if (pbest->active.size() == 0) {
            rrs -= pbest->resource_share;
            if (log_flags.rr_simulation) {
                msg_printf(pbest, MSG_INFO,
                    "rr_sim: decr rrs by %f, new value %f",
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
                pbest->cpu_shortfall += d_time*(proj_cpu_share - last_proj_active_size);
				if (log_flags.rr_simulation) {
					msg_printf(pbest, MSG_INFO,
						"rr_sim: new shortfall %f d_time %f proj_cpu_share %f lpas %d",
						pbest->cpu_shortfall, d_time, proj_cpu_share, last_proj_active_size
					);
				}
            }

			if (end_time < buf_end) {
                d_time = buf_end - end_time;
                // if this is the last result for this project, account for the tail
                if (!pbest->active.size()) { 
                    pbest->cpu_shortfall += d_time * proj_cpu_share;
					if (log_flags.rr_simulation) {
						 msg_printf(pbest, MSG_INFO, "rr_sim proj out of work; shortfall %f d %f pcs %f",
							 pbest->cpu_shortfall, d_time, proj_cpu_share
					     );
					}
                }
            }
            if (log_flags.rr_simulation) {
                msg_printf(0, MSG_INFO,
                    "rr_sim total: idle cpus %d, last active %d, active %d, shortfall %f",
                    nidle_cpus, last_active_size, (int)active.size(), cpu_shortfall
					
				);
				msg_printf(0, MSG_INFO,
					"rr_sim proj %s: last active %d, active %d, shortfall %f",
					pbest->get_project_name(), last_proj_active_size, (int)pbest->active.size(),
					pbest->cpu_shortfall
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
            if (p->cpu_shortfall) {
                msg_printf(p, MSG_INFO,
                    "rr_sim: shortfall %f\n", p->cpu_shortfall
                );
            }
        }
        msg_printf(NULL, MSG_INFO,
            "rr_simulation: end; returning %s; total shortfall %f\n",
            rval?"true":"false",
            cpu_shortfall
        );
    }
    return rval;
}

////////////////////// END CUT AND PASTE ////////////////

int main() {
    PROJECT* p;
    RESULT* r;
    log_flags.rr_simulation = true;

    gstate.global_prefs.work_buf_min_days = 1;
    gstate.global_prefs.work_buf_additional_days = 1;
    gstate.global_prefs.cpu_scheduling_period_minutes = 60;
    gstate.ncpus = 1;
    gstate.now = 0;

    p = new PROJECT("project A", 33.);
    gstate.projects.push_back(p);

    r = new RESULT(p, "result 1", 9, 1e6);
    gstate.results.push_back(r);
    r = new RESULT(p, "result 2", 9, 1e6);
    gstate.results.push_back(r);
    r = new RESULT(p, "result 3", 9, 1e6);
    gstate.results.push_back(r);

    p = new PROJECT("project B", 33.);
    gstate.projects.push_back(p);
    r = new RESULT(p, "result 4", 20, 1e6);
    gstate.results.push_back(r);

    p = new PROJECT("project C", 33.);
    gstate.projects.push_back(p);
    r = new RESULT(p, "result 5", 30, 1e6);
    gstate.results.push_back(r);

    gstate.rr_simulation();
}
