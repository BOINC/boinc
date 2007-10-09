// Berkeley Open Infrastructure for Network Computing
// http://boinc.berkeley.edu
// Copyright (C) 2005 University of California
//
// This is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation;
// either version 2.1 of the License, or (at your option) any later version.
//
// This software is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//
// To view the GNU Lesser General Public License visit
// http://www.gnu.org/copyleft/lesser.html
// or write to the Free Software Foundation, Inc.,
// 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

// High-level logic for communicating with scheduling servers,
// and for merging the result of a scheduler RPC into the client state

// The scheduler RPC mechanism is in scheduler_op.C

#include "cpp.h"

#ifdef _WIN32
#include "boinc_win.h"
#endif

#ifndef _WIN32
#include "config.h"
#include <stdio.h>
#include <math.h>
#include <time.h>
#include <strings.h>
#include <map>
#include <set>
#endif

#include "error_numbers.h"
#include "file_names.h"
#include "filesys.h"
#include "parse.h"
#include "str_util.h"
#include "util.h"

#include "client_msgs.h"
#include "scheduler_op.h"

#ifdef SIM
#include "sim.h"
#else
#include "client_state.h"
#endif

using std::max;
using std::vector;
using std::string;

// quantities like avg CPU time decay by a factor of e every week
//
#define EXP_DECAY_RATE  (1./(SECONDS_PER_DAY*7))

// try to report results this much before their deadline
//
#define REPORT_DEADLINE_CUSHION ((double)SECONDS_PER_DAY)

static const char* urgency_name(int urgency) {
    switch(urgency) {
    case WORK_FETCH_DONT_NEED: return "Don't need";
    case WORK_FETCH_OK: return "OK";
    case WORK_FETCH_NEED: return "Need";
    case WORK_FETCH_NEED_IMMEDIATELY: return "Need immediately";
    }
    return "Unknown";
}

// how many CPUs should this project occupy on average,
// based on its resource share relative to a given set
//
int CLIENT_STATE::proj_min_results(PROJECT* p, double subset_resource_share) {
    if (p->non_cpu_intensive) {
        return 1;
    }
    if (!subset_resource_share) return 1;   // TODO - fix
    return (int)(ceil(ncpus*p->resource_share/subset_resource_share));
}

void CLIENT_STATE::check_project_timeout() {
	unsigned int i;
	for (i=0; i<projects.size(); i++) {
		PROJECT* p = projects[i];
		if (p->possibly_backed_off && now > p->min_rpc_time) {
			p->possibly_backed_off = false;
			request_work_fetch("Project backoff ended");
		}
	}
}

void PROJECT::set_min_rpc_time(double future_time, const char* reason) {
    if (future_time > min_rpc_time) {
        min_rpc_time = future_time;
		possibly_backed_off = true;
        msg_printf(this, MSG_INFO,
            "Deferring communication for %s",
            timediff_format(min_rpc_time - gstate.now).c_str()
        );
        msg_printf(this, MSG_INFO, "Reason: %s\n", reason);
    }
}

// Return true if we should not contact the project yet.
//
bool PROJECT::waiting_until_min_rpc_time() {
    return (min_rpc_time > gstate.now);
}

// find a project that needs to have its master file fetched
//
PROJECT* CLIENT_STATE::next_project_master_pending() {
    unsigned int i;
    PROJECT* p;

    for (i=0; i<projects.size(); i++) {
        p = projects[i];
        if (p->waiting_until_min_rpc_time()) continue;
        if (p->suspended_via_gui) continue;
        if (p->master_url_fetch_pending) {
            return p;
        }
    }
    return 0;
}

// find a project for which a scheduler RPC is pending
// and we're not backed off
//
PROJECT* CLIENT_STATE::next_project_sched_rpc_pending() {
    unsigned int i;
    PROJECT* p;

    for (i=0; i<projects.size(); i++) {
        p = projects[i];
        if (p->waiting_until_min_rpc_time()) continue;
        if (p->next_rpc_time && p->next_rpc_time<now) {
            p->sched_rpc_pending = RPC_REASON_PROJECT_REQ;
            p->next_rpc_time = 0;
        }
        // if (p->suspended_via_gui) continue;
        // do the RPC even if suspended.
        // This is critical for acct mgrs, to propagate new host CPIDs
        //
        if (p->sched_rpc_pending) {
            return p;
        }
    }
    return 0;
}

PROJECT* CLIENT_STATE::next_project_trickle_up_pending() {
    unsigned int i;
    PROJECT* p;

    for (i=0; i<projects.size(); i++) {
        p = projects[i];
        if (p->waiting_until_min_rpc_time()) continue;
        if (p->suspended_via_gui) continue;
        if (p->trickle_up_pending) {
            return p;
        }
    }
    return 0;
}

// Return the best project to fetch work from, NULL if none
//
// Pick the one with largest (long term debt - amount of current work)
//
// PRECONDITIONS:
//   - work_request_urgency and work_request set for all projects
//   - CLIENT_STATE::overall_work_fetch_urgency is set
// (by previous call to compute_work_requests())
//
PROJECT* CLIENT_STATE::next_project_need_work() {
    PROJECT *p, *p_prospect = NULL;
    unsigned int i;

    for (i=0; i<projects.size(); i++) {
        p = projects[i];
        if (p->work_request_urgency == WORK_FETCH_DONT_NEED) continue;
        if (p->work_request == 0) continue;
        if (!p->contactable()) continue;

        // if we don't need work, only get work from non-cpu intensive projects.
        //
        if (overall_work_fetch_urgency == WORK_FETCH_DONT_NEED && !p->non_cpu_intensive) continue;

        // if we don't really need work,
        // and we don't really need work from this project, pass.
        //
        if (overall_work_fetch_urgency == WORK_FETCH_OK) {
            if (p->work_request_urgency <= WORK_FETCH_OK) {
                continue;
            }
        }

        if (p_prospect) {
            if (p->work_request_urgency == WORK_FETCH_OK && 
                p_prospect->work_request_urgency > WORK_FETCH_OK
            ) {
                continue;
            }

            if (p->long_term_debt + p->cpu_shortfall < p_prospect->long_term_debt + p_prospect->cpu_shortfall
                && !p->non_cpu_intensive
            ) {
                continue;
            }
        }

        p_prospect = p;
    }
    if (p_prospect && (p_prospect->work_request <= 0)) {
        p_prospect->work_request = 1.0;
        if (log_flags.work_fetch_debug) {
            msg_printf(0, MSG_INFO,
                "[work_fetch_debug] next_project_need_work: project picked %s",
                p_prospect->project_name
            );
        }
    }

    return p_prospect;
}

// find a project with finished results that should be reported.
// This means:
//    - we're not backing off contacting the project
//    - the result is ready_to_report (compute done; files uploaded)
//    - we're within a day of the report deadline,
//      or at least a day has elapsed since the result was completed,
//      or we have a sporadic connection
//
PROJECT* CLIENT_STATE::find_project_with_overdue_results() {
    unsigned int i;
    RESULT* r;

    for (i=0; i<results.size(); i++) {
        r = results[i];
        if (!r->ready_to_report) continue;

        PROJECT* p = r->project;
        if (p->waiting_until_min_rpc_time()) continue;
        if (p->suspended_via_gui) continue;

        if (net_status.have_sporadic_connection) {
            return p;
        }

        double cushion = std::max(REPORT_DEADLINE_CUSHION, work_buf_min());
        if (gstate.now > r->report_deadline - cushion) {
            return p;
        }

        if (gstate.now > r->completed_time + SECONDS_PER_DAY) {
            return p;
        }
    }
    return 0;
}

// the fraction of time a given CPU is working for BOINC
//
double CLIENT_STATE::overall_cpu_frac() {
    double running_frac = time_stats.on_frac * time_stats.active_frac * time_stats.cpu_efficiency;
    if (running_frac < 0.01) running_frac = 0.01;
    if (running_frac > 1) running_frac = 1;
	return running_frac;
}

// the expected number of CPU seconds completed by the client
// in a second of wall-clock time.
// May be > 1 on a multiprocessor.
//
double CLIENT_STATE::avg_proc_rate() {
    return ncpus*overall_cpu_frac();
}

// estimate wall-clock time until the number of uncompleted results
// for project p will reach k,
// given the total resource share of a set of competing projects
//
double CLIENT_STATE::time_until_work_done(
    PROJECT *p, int k, double subset_resource_share
) {
    int num_results_to_skip = k;
    double est = 0;
    
    // total up the estimated time for this project's unstarted
    // and partially completed results,
    // omitting the last k
    //
    for (vector<RESULT*>::reverse_iterator iter = results.rbegin();
         iter != results.rend(); iter++
    ) {
        RESULT *rp = *iter;
        if (rp->project != p
            || rp->state() > RESULT_FILES_DOWNLOADED
            || rp->ready_to_report
        ) continue;
        if (num_results_to_skip > 0) {
            --num_results_to_skip;
            continue;
        }
        if (rp->project->non_cpu_intensive) {
            // if it is a non_cpu intensive project,
            // it needs only one at a time.
            //
            est = max(rp->estimated_cpu_time_remaining(true), work_buf_min());  
        } else {
            est += rp->estimated_cpu_time_remaining(true);
        }
    }
	if (log_flags.work_fetch_debug) {
		msg_printf(NULL, MSG_INFO,
			"[work_fetch_debug] time_until_work_done(): est %f ssr %f apr %f prs %f",
			est, subset_resource_share, avg_proc_rate(), p->resource_share
		);
	}
    if (subset_resource_share) {
        double apr = avg_proc_rate()*p->resource_share/subset_resource_share;
        return est/apr;
    } else {
        return est/avg_proc_rate();     // TODO - fix
    }
}

// Top-level function for work fetch policy.
// Outputs:
// - overall_work_fetch_urgency
// - for each contactable project:
//     - work_request and work_request_urgency
//
// Notes:
// - at most 1 CPU-intensive project will have a nonzero work_request
//      and a work_request_urgency higher than DONT_NEED.
//      This prevents projects with low LTD from getting work
//      even though there was a higher LTD project that should get work.
// - all non-CPU-intensive projects that need work
//      and are contactable will have a work request of 1.
//
// return false
//
bool CLIENT_STATE::compute_work_requests() {
    unsigned int i;
    static double last_time = 0;

    if (gstate.now - last_time >= 60) {
        gstate.request_work_fetch("timer");
    }
    if (!must_check_work_fetch) return false;

    if (log_flags.work_fetch_debug) {
        msg_printf(0, MSG_INFO, "[work_fetch_debug] compute_work_requests(): start");
    }
    last_time = gstate.now;
    must_check_work_fetch = false;

    compute_nuploading_results();
    adjust_debts();

#ifdef SIM
    if (work_fetch_old) {
        // "dumb" version for simulator only.
        // for each project, compute extra work needed to bring it up to
        // total_buf/relative resource share.
        //
        overall_work_fetch_urgency = WORK_FETCH_DONT_NEED;
        double trs = total_resource_share();
        double total_buf = ncpus*(work_buf_min() + work_buf_additional());
        for (i=0; i<projects.size(); i++) {
            PROJECT* p = projects[i];
            double d = 0;
            for (unsigned int j=0; j<results.size(); j++) {
                RESULT* rp = results[j];
                if (rp->project != p) continue;
                d += rp->estimated_cpu_time_remaining(true);
            }
            double rrs = p->resource_share/trs;
            double minq = total_buf*rrs;
            if (d < minq) {
                p->work_request = minq-d;
                p->work_request_urgency = WORK_FETCH_NEED;
                overall_work_fetch_urgency = WORK_FETCH_NEED;
            } else {
                p->work_request = 0;
                p->work_request_urgency = WORK_FETCH_DONT_NEED;
            }
        }
        return false;
    }
#endif

    rr_simulation();

    // compute per-project and overall urgency
    //
    bool possible_deadline_miss = false;
    bool project_shortfall = false;
    bool non_cpu_intensive_needs_work = false;
    for (i=0; i< projects.size(); i++) {
        PROJECT* p = projects[i];
        if (p->non_cpu_intensive) {
            if (p->runnable() || !p->contactable()) {
                p->work_request = 0;
                p->work_request_urgency = WORK_FETCH_DONT_NEED;
            } else {
                p->work_request = 1.0;
                p->work_request_urgency = WORK_FETCH_NEED_IMMEDIATELY;
				non_cpu_intensive_needs_work = true;
				if (log_flags.work_fetch_debug) {
					msg_printf(p, MSG_INFO,
						"[work_fetch_debug] non-CPU-intensive project needs work"
					);
				}
				return false;
            }
        } else {
            p->work_request_urgency = WORK_FETCH_DONT_NEED;
            p->work_request = 0;
			if (p->rr_sim_deadlines_missed) {
				possible_deadline_miss = true;
			}
            if (p->cpu_shortfall && p->long_term_debt > -global_prefs.cpu_scheduling_period()) {
                project_shortfall = true;
            }
        }
    }

    if (cpu_shortfall <= 0.0 && (possible_deadline_miss || !project_shortfall)) {
        overall_work_fetch_urgency = WORK_FETCH_DONT_NEED;
    } else if (no_work_for_a_cpu()) {
        overall_work_fetch_urgency = WORK_FETCH_NEED_IMMEDIATELY;
    } else if (cpu_shortfall > 0) {
        overall_work_fetch_urgency = WORK_FETCH_NEED;
    } else {
        overall_work_fetch_urgency = WORK_FETCH_OK;
    }
    if (log_flags.work_fetch_debug) {
        msg_printf(0, MSG_INFO,
            "[work_fetch_debug] compute_work_requests(): cpu_shortfall %f, overall urgency %s",
            cpu_shortfall, urgency_name(overall_work_fetch_urgency)
        );
    }
    if (overall_work_fetch_urgency == WORK_FETCH_DONT_NEED) {
        if (non_cpu_intensive_needs_work) {
            overall_work_fetch_urgency = WORK_FETCH_NEED_IMMEDIATELY;
        }
        return false;
    }

    // loop over projects, and pick one to get work from
    //
    double prrs = fetchable_resource_share();
    PROJECT *pbest = NULL;
    for (i=0; i<projects.size(); i++) {
        PROJECT *p = projects[i];

        // see if this project can be ruled out completely
        //
        if (p->non_cpu_intensive) continue;
        if (!p->contactable()) {
            if (log_flags.work_fetch_debug) {
                msg_printf(p, MSG_INFO, "[work_fetch_debug] work fetch: project not contactable; skipping");
            }
            continue;
        }
        if ((p->deadlines_missed >= ncpus)
            && overall_work_fetch_urgency != WORK_FETCH_NEED_IMMEDIATELY
        ) {
            if (log_flags.work_fetch_debug) {
                msg_printf(p, MSG_INFO,
                    "[work_fetch_debug] project has %d deadline misses; skipping",
                    p->deadlines_missed
                );
            }
            continue;
        }
        if (p->some_download_stalled()) {
            if (log_flags.work_fetch_debug) {
                msg_printf(p, MSG_INFO,
                    "[work_fetch_debug] project has stalled download; skipping"
                );
            }
            continue;
        }

        if (p->some_result_suspended()) {
            if (log_flags.work_fetch_debug) {
                msg_printf(p, MSG_INFO, "[work_fetch_debug] project has suspended result; skipping");
            }
            continue;
        }

        if (p->overworked() && overall_work_fetch_urgency < WORK_FETCH_NEED) {
            if (log_flags.work_fetch_debug) {
                msg_printf(p, MSG_INFO, "[work_fetch_debug] project is overworked; skipping");
            }
            continue;
        }
        if (p->cpu_shortfall == 0.0 && overall_work_fetch_urgency < WORK_FETCH_NEED) {
            if (log_flags.work_fetch_debug) {
                msg_printf(p, MSG_INFO, "[work_fetch_debug] project has no shortfall; skipping");
            }
            continue;
        }
        if (p->nuploading_results >  2*ncpus) {
            if (log_flags.work_fetch_debug) {
                msg_printf(p, MSG_INFO,
                    "[work_fetch_debug] project has %d uploading results; skipping",
                    p->nuploading_results
                );
            }
            continue;
        }

        // If the project's DCF is outside of reasonable limits,
        // the project's WU FLOP estimates are not useful for predicting
        // completion time.
        // Switch to a simpler policy: ask for 1 sec of work if
        // we don't have any.
        //
        if (p->duration_correction_factor < 0.02 || p->duration_correction_factor > 80.0) {
            if (p->runnable()) {
                if (log_flags.work_fetch_debug) {
                    msg_printf(p, MSG_INFO,
                        "[work_fetch_debug] project DCF %f out of range and have work; skipping",
                        p->duration_correction_factor
                    );
                }
                continue;
            } else {
                if (log_flags.work_fetch_debug) {
                    msg_printf(p, MSG_INFO,
                        "[work_fetch_debug] project DCF %f out of range: changing shortfall %f to 1.0", 
                         p->duration_correction_factor, p->cpu_shortfall
                    );
                }
                p->cpu_shortfall = 1.0;
            }
        }

        // see if this project is better than our current best
        //
        if (pbest) {
            // avoid getting work from a project in deadline trouble
            //
            if (p->deadlines_missed && !pbest->deadlines_missed) {
                if (log_flags.work_fetch_debug) {
                    msg_printf(p, MSG_INFO,
                        "[work_fetch_debug] project has deadline misses, %s doesn't",
                        pbest->get_project_name()
                    );
                }
                continue;
            }
            // avoid getting work from an overworked project
            //
            if (p->overworked() && !pbest->overworked()) {
                if (log_flags.work_fetch_debug) {
                    msg_printf(p, MSG_INFO,
                        "[work_fetch_debug] project is overworked, %s isn't",
                        pbest->get_project_name()
                    );
                }
                continue;
            }
            // get work from project with highest LTD
            //
            if (pbest->long_term_debt + pbest->cpu_shortfall > p->long_term_debt + p->cpu_shortfall) {
                if (log_flags.work_fetch_debug) {
                    msg_printf(p, MSG_INFO,
                        "[work_fetch_debug] project has less LTD than %s",
                        pbest->get_project_name()
                    );
                }
                continue;
            }
        }
        pbest = p;
        if (log_flags.work_fetch_debug) {
            msg_printf(pbest, MSG_INFO, "[work_fetch_debug] best project so far");
        }
    }

    if (pbest) {
        pbest->work_request = max(
            pbest->cpu_shortfall,
            cpu_shortfall * (prrs ? pbest->resource_share/prrs : 1)
        );

        // sanity check
        //
        double x = 1.01*work_buf_total()*ncpus;
            // the 1.01 is for round-off error
        if (pbest->work_request > x) {
        	msg_printf(NULL, MSG_INTERNAL_ERROR,
        	    "Proposed work request %f bigger than max %f",
        	    pbest->work_request, x
        	);
        	pbest->work_request = x;
        }
        if (!pbest->nearly_runnable()) {
            pbest->work_request_urgency = WORK_FETCH_NEED_IMMEDIATELY;
        } else if (pbest->cpu_shortfall) {
            pbest->work_request_urgency = WORK_FETCH_NEED;
        } else {
            pbest->work_request_urgency = WORK_FETCH_OK;
        }

        if (log_flags.work_fetch_debug) {
            msg_printf(pbest, MSG_INFO,
				"[work_fetch_debug] compute_work_requests(): work req %f, shortfall %f, urgency %s\n",
				pbest->work_request, pbest->cpu_shortfall,
                urgency_name(pbest->work_request_urgency)
            );
        }
    } else if (non_cpu_intensive_needs_work) {
        overall_work_fetch_urgency = WORK_FETCH_NEED_IMMEDIATELY;
    }


    return false;
}

// called when benchmarks change
//
void CLIENT_STATE::scale_duration_correction_factors(double factor) {
    if (factor <= 0) return;
    for (unsigned int i=0; i<projects.size(); i++) {
        PROJECT* p = projects[i];
        p->duration_correction_factor *= factor;
    }
	if (log_flags.cpu_sched_debug) {
		msg_printf(NULL, MSG_INFO,
            "[cpu_sched_debug] scaling duration correction factors by %f",
            factor
        );
	}
}

// Choose a new host CPID.
// If using account manager, do scheduler RPCs
// to all acct-mgr-attached projects to propagate the CPID
//
void CLIENT_STATE::generate_new_host_cpid() {
    host_info.generate_host_cpid();
    for (unsigned int i=0; i<projects.size(); i++) {
        if (projects[i]->attached_via_acct_mgr) {
            projects[i]->sched_rpc_pending = RPC_REASON_ACCT_MGR_REQ;
            projects[i]->set_min_rpc_time(now + 15, "Sending new host CPID");
        }
    }
}

void CLIENT_STATE::compute_nuploading_results() {
    unsigned int i;

    for (i=0; i<projects.size(); i++) {
        projects[i]->nuploading_results = 0;
    }
    for (i=0; i<results.size(); i++) {
        RESULT* rp = results[i];
        if (rp->state() == RESULT_FILES_UPLOADING) {
            rp->project->nuploading_results++;
        }
    }
}

bool PROJECT::runnable() {
    if (suspended_via_gui) return false;
    for (unsigned int i=0; i<gstate.results.size(); i++) {
        RESULT* rp = gstate.results[i];
        if (rp->project != this) continue;
        if (rp->runnable()) return true;
    }
    return false;
}

bool PROJECT::downloading() {
    if (suspended_via_gui) return false;
    for (unsigned int i=0; i<gstate.results.size(); i++) {
        RESULT* rp = gstate.results[i];
        if (rp->project != this) continue;
        if (rp->downloading()) return true;
    }
    return false;
}

bool PROJECT::some_result_suspended() {
    unsigned int i;
    for (i=0; i<gstate.results.size(); i++) {
         RESULT *rp = gstate.results[i];
         if (rp->project != this) continue;
         if (rp->suspended_via_gui) return true;
     }
    return false;
}

bool PROJECT::contactable() {
    if (suspended_via_gui) return false;
    if (master_url_fetch_pending) return false;
    if (min_rpc_time > gstate.now) return false;
    if (dont_request_more_work) return false;
    return true;
}

bool PROJECT::potentially_runnable() {
    if (runnable()) return true;
    if (contactable()) return true;
    if (downloading()) return true;
    return false;
}

bool PROJECT::nearly_runnable() {
    if (runnable()) return true;
    if (downloading()) return true;
    return false;
}

bool PROJECT::overworked() {
    return long_term_debt < -gstate.global_prefs.cpu_scheduling_period();
}

bool RESULT::runnable() {
    if (suspended_via_gui) return false;
    if (project->suspended_via_gui) return false;
    if (state() != RESULT_FILES_DOWNLOADED) return false;
    return true;
}

bool RESULT::nearly_runnable() {
    return runnable() || downloading();
}

// Return true if the result is waiting for its files to download,
// and nothing prevents this from happening soon
//
bool RESULT::downloading() {
    if (suspended_via_gui) return false;
    if (project->suspended_via_gui) return false;
    if (state() > RESULT_FILES_DOWNLOADING) return false;
    return true;
}

double RESULT::estimated_cpu_time_uncorrected() {
    return wup->rsc_fpops_est/gstate.host_info.p_fpops;
}

// estimate how long a result will take on this host
//
double RESULT::estimated_cpu_time(bool for_work_fetch) {
#ifdef SIM
    SIM_PROJECT* spp = (SIM_PROJECT*)project;
    if (dual_dcf && for_work_fetch && spp->completions_ratio_mean) {
        return estimated_cpu_time_uncorrected()*spp->completions_ratio_mean;
    }
#endif
    return estimated_cpu_time_uncorrected()*project->duration_correction_factor;
}

double RESULT::estimated_cpu_time_remaining(bool for_work_fetch) {
    if (computing_done()) return 0;
    ACTIVE_TASK* atp = gstate.lookup_active_task_by_result(this);
    if (atp) {
        return atp->est_cpu_time_to_completion(for_work_fetch);
    }
    return estimated_cpu_time(for_work_fetch);
}

// Returns the estimated CPU time to completion (in seconds) of this task.
// Compute this as a weighted average of estimates based on
// 1) the workunit's flops count
// 2) the current reported CPU time and fraction done
//
double ACTIVE_TASK::est_cpu_time_to_completion(bool for_work_fetch) {
    if (fraction_done >= 1) return 0;
    double wu_est = result->estimated_cpu_time(for_work_fetch);
    if (fraction_done <= 0) return wu_est;
    double frac_est = (current_cpu_time / fraction_done) - current_cpu_time;
    double fraction_left = 1-fraction_done;
    double x = fraction_done*frac_est + fraction_left*fraction_left*wu_est;
    return x;
}

// trigger work fetch
// 
void CLIENT_STATE::request_work_fetch(const char* where) {
    if (log_flags.work_fetch_debug) {
        msg_printf(0, MSG_INFO, "[work_fetch_debug] Request work fetch: %s", where);
    }
    must_check_work_fetch = true;
}

const char *BOINC_RCSID_d3a4a7711 = "$Id$";
