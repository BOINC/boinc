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

#include "client_types.h"
#include "client_msgs.h"
#ifdef SIM
#include "sim.h"
#else
#include "client_state.h"
#endif

#include "work_fetch.h"

using std::vector;

RSC_WORK_FETCH cuda_work_fetch;
RSC_WORK_FETCH cpu_work_fetch;
WORK_FETCH work_fetch;

static inline char* rsc_name(int t) {
    switch (t) {
    case RSC_TYPE_CPU: return "CPU";
    case RSC_TYPE_CUDA: return "CUDA";
    }
    return "Unknown";
}

RSC_PROJECT_WORK_FETCH& RSC_WORK_FETCH::project_state(PROJECT* p) {
    switch(rsc_type) {
    case RSC_TYPE_CUDA: return p->cuda_pwf;
    default: return p->cpu_pwf;
    }
}

bool RSC_WORK_FETCH::may_have_work(PROJECT* p) {
    RSC_PROJECT_WORK_FETCH& w = project_state(p);
    return (w.backoff_time < gstate.now);
}

bool RSC_PROJECT_WORK_FETCH::compute_may_have_work() {
    return (backoff_time < gstate.now);
}

void RSC_PROJECT_WORK_FETCH::rr_init() {
    may_have_work = compute_may_have_work();
}

void RSC_WORK_FETCH::rr_init() {
    shortfall = 0;
    nidle_now = 0;
    total_fetchable_share = 0;
    total_runnable_share = 0;
}

void WORK_FETCH::rr_init() {
    cpu_work_fetch.rr_init();
    if (coproc_cuda) {
        cuda_work_fetch.rr_init();
    }
    estimated_delay = 0;
    for (unsigned int i=0; i<gstate.projects.size(); i++) {
        PROJECT* p = gstate.projects[i];
        p->pwf.can_fetch_work = p->pwf.compute_can_fetch_work(p);
        p->cpu_pwf.rr_init();
        if (coproc_cuda) {
            p->cuda_pwf.rr_init();
        }
    }
}

bool PROJECT_WORK_FETCH::compute_can_fetch_work(PROJECT* p) {
    if (p->non_cpu_intensive) return false;
    if (p->suspended_via_gui) return false;
    if (p->master_url_fetch_pending) return false;
    if (p->min_rpc_time > gstate.now) return false;
    if (p->dont_request_more_work) return false;
    if (p->some_download_stalled()) return false;
    if (p->some_result_suspended()) return false;
    if (p->nuploading_results > 2*gstate.ncpus) return false;
    return true;
}

void PROJECT_WORK_FETCH::clear_backoffs(PROJECT* p) {
    p->cpu_pwf.clear_backoff();
    p->cuda_pwf.clear_backoff();
}

void RSC_WORK_FETCH::accumulate_shortfall(double d_time, double nused) {
    double idle = ninstances - nused;
    if (idle > 0) {
        shortfall += idle*d_time;
    }
}

// choose the best project to ask for work for this resource
//
PROJECT* RSC_WORK_FETCH::choose_project() {
    PROJECT* pbest = NULL;

    for (unsigned i=0; i<gstate.projects.size(); i++) {
        PROJECT* p = gstate.projects[i];
        if (!p->pwf.can_fetch_work) continue;
        if (!project_state(p).may_have_work) continue;
        if (pbest) {
            if (p->deadlines_missed && !pbest->deadlines_missed) {
                continue;
            }
            if (project_state(p).overworked() && !project_state(pbest).overworked()) {
                continue;
            }
            if (pbest->pwf.overall_debt > p->pwf.overall_debt) {
                continue;
            }

        }
        pbest = p;
    }
    return pbest;
}

void WORK_FETCH::set_overall_debts() {
    for (unsigned i=0; i<gstate.projects.size(); i++) {
        PROJECT* p = gstate.projects[i];
        p->pwf.overall_debt = p->cpu_pwf.debt;
        if (coproc_cuda) {
            p->pwf.overall_debt += cuda_work_fetch.speed*p->cuda_pwf.debt;
        }
    }
}

void RSC_WORK_FETCH::print_state(char* name) {
    msg_printf(0, MSG_INFO,
        "[wfd] %s: shortfall %.2f nidle %.2f fetchable RS %.2f runnable RS %.2f",
        name,
        shortfall, nidle_now,
        total_fetchable_share, total_runnable_share
    );
    for (unsigned int i=0; i<gstate.projects.size(); i++) {
        PROJECT* p = gstate.projects[i];
        if (p->non_cpu_intensive) continue;
        RSC_PROJECT_WORK_FETCH& pwf = project_state(p);
        msg_printf(p, MSG_INFO,
            "[wfd] %s: runshare %.2f debt %.2f backoff t %.2f int %.2f",
            name,
            pwf.runnable_share, pwf.debt, pwf.backoff_time, pwf.backoff_interval
        );
    }
}

void WORK_FETCH::print_state() {
    msg_printf(0, MSG_INFO, "[wfd] ------- start work fetch state -------");
    cpu_work_fetch.print_state("CPU");
    if (coproc_cuda) {
        cuda_work_fetch.print_state("CUDA");
    }
    for (unsigned int i=0; i<gstate.projects.size(); i++) {
        PROJECT* p = gstate.projects[i];
        if (p->non_cpu_intensive) continue;
        msg_printf(p, MSG_INFO, "[wfd] overall_debt %f", p->pwf.overall_debt);
    }
    msg_printf(0, MSG_INFO, "[wfd] ------- end work fetch state -------");
}

static void print_req(PROJECT* p) {
    msg_printf(p, MSG_INFO,
        "[wfd] request: CPU (%.2f sec, %d) CUDA (%.2f sec, %d)",
        cpu_work_fetch.req_secs, cpu_work_fetch.req_instances,
        cuda_work_fetch.req_secs, cuda_work_fetch.req_instances
    );
}

void RSC_WORK_FETCH::clear_request() {
    req_secs = 0;
    req_instances = 0;
}

void WORK_FETCH::clear_request() {
    cpu_work_fetch.clear_request();
    cuda_work_fetch.clear_request();
}

// we're going to contact this project; decide how much work to request
//
void WORK_FETCH::compute_work_request(PROJECT* p) {
    // check if this is the project we'd ask for work anyway
    //
    PROJECT* pbest = choose_project();
    if (p == pbest) return;

    // if not, don't request any work
    //
    clear_request();
}

// see if there's a fetchable non-CPU-intensive project without work
//
PROJECT* WORK_FETCH::non_cpu_intensive_project_needing_work() {
    for (unsigned int i=0; i<gstate.projects.size(); i++) {
        PROJECT* p = gstate.projects[i];
        if (!p->non_cpu_intensive) continue;
        if (!p->can_request_work()) continue;
        bool has_work = false;
        for (unsigned int j=0; j<gstate.results.size(); j++) {
            RESULT* rp = gstate.results[j];
            if (rp->project == p) {
                has_work = true;
                break;
            }
        }
        if (!has_work) {
            clear_request();
            cpu_work_fetch.req_secs = 1;
            return p;
        }
    }
    return 0;
}

// choose a project to fetch work from,
// and set the request fields of resource objects
//
PROJECT* WORK_FETCH::choose_project() {
    PROJECT* p = 0;

    p = non_cpu_intensive_project_needing_work();
    if (p) return p;

    gstate.adjust_debts();
    gstate.compute_nuploading_results();

    gstate.rr_simulation();
    set_overall_debts();
    bool request_cpu = true;
    bool request_cuda = (coproc_cuda != NULL);

    // if a resource is currently idle, get work for it;
    // give GPU priority over CPU
    //
    if (coproc_cuda && cuda_work_fetch.nidle_now) {
        p = cuda_work_fetch.choose_project();
        if (p) {
            request_cpu = false;
        }
    }
    if (!p && cpu_work_fetch.nidle_now) {
        p = cpu_work_fetch.choose_project();
        if (p) {
            request_cuda = false;
        }
    }

    // if a resource has a shortfall, get work for it.
    //
    if (!p && coproc_cuda && cuda_work_fetch.shortfall) {
        p = cuda_work_fetch.choose_project();
    }
    if (!p && cpu_work_fetch.shortfall) {
        p = cpu_work_fetch.choose_project();
    }

    // decide how much work to request for each resource
    //
    clear_request();
    if (p) {
        if (request_cpu) {
            cpu_work_fetch.set_request(p);
        }
        if (request_cuda) {
            cuda_work_fetch.set_request(p);
        }
        if (coproc_cuda) {
            coproc_cuda->req_secs = cuda_work_fetch.req_secs;
            coproc_cuda->req_instances = cuda_work_fetch.req_instances;
        }
    }
    if (log_flags.work_fetch_debug) {
        print_state();
        if (p) {
            print_req(p);
        } else {
            msg_printf(0, MSG_INFO, "No project chosen for work fetch");
        }
    }

    return p;
}

void RSC_WORK_FETCH::set_request(PROJECT* p) {
    RSC_PROJECT_WORK_FETCH& w = project_state(p);

    // if project's DCF is too big or small, its completion time estimates
    // are useless; just ask for 1 second
    //
    if (p->duration_correction_factor < 0.02 || p->duration_correction_factor > 80.0) {
        req_secs = 1;
    } else {
        req_secs = gstate.work_buf_total()*w.fetchable_share;
    }
    req_instances = (int)ceil(w.fetchable_share*nidle_now);
}

void WORK_FETCH::accumulate_inst_sec(ACTIVE_TASK* atp, double dt) {
    APP_VERSION* avp = atp->result->avp;
    PROJECT* p = atp->result->project;
    double x = dt*avp->avg_ncpus;
    p->cpu_pwf.secs_this_debt_interval += x;
    cpu_work_fetch.secs_this_debt_interval += x;
    if (coproc_cuda) {
        x = dt*avp->ncudas;
        p->cuda_pwf.secs_this_debt_interval += x;
        cuda_work_fetch.secs_this_debt_interval += x;
    }
}

void RSC_WORK_FETCH::update_debts() {
    unsigned int i;
    int nprojects = 0;
    double ders = 0;
    PROJECT* p;

    for (i=0; i<gstate.projects.size(); i++) {
        p = gstate.projects[i];
        RSC_PROJECT_WORK_FETCH& w = project_state(p);
        if (w.debt_eligible(p)) {
            ders += p->resource_share;
        }
    }
    double total_debt = 0;
    for (i=0; i<gstate.projects.size(); i++) {
        p = gstate.projects[i];
        RSC_PROJECT_WORK_FETCH& w = project_state(p);
        if (w.debt_eligible(p)) {
            double share_frac = p->resource_share/ders;
            double delta = share_frac*secs_this_debt_interval - w.secs_this_debt_interval;
            w.debt += delta;
            if (log_flags.debt_debug) {
                msg_printf(p, MSG_INFO,
                    "[debt] %s debt %.2f delta %.2f share frac %.2f (%.2f/%.2f) secs %.2f rsc_secs %.2f",
                    rsc_name(rsc_type),
                    w.debt, delta, share_frac, p->resource_share, ders, secs_this_debt_interval,
                    w.secs_this_debt_interval
                );
            }
        }
        total_debt += w.debt;
        nprojects++;
    }

    //  normalize so mean is zero, and clamp
    //
    double avg_debt = total_debt / nprojects;
    for (i=0; i<gstate.projects.size(); i++) {
        p = gstate.projects[i];
        RSC_PROJECT_WORK_FETCH& w = project_state(p);
        w.debt -= avg_debt;
    }
}

// find total and per-project resource shares for each resource
//
void WORK_FETCH::compute_shares() {
    unsigned int i;
    PROJECT* p;
    for (i=0; i<gstate.projects.size(); i++) {
        p = gstate.projects[i];
        if (p->non_cpu_intensive) continue;
        if (p->rr_sim_status.has_cpu_jobs) {
            cpu_work_fetch.total_runnable_share += p->resource_share;
        }
        if (p->rr_sim_status.has_cuda_jobs) {
            cuda_work_fetch.total_runnable_share += p->resource_share;
        }
        if (!p->pwf.can_fetch_work) continue;
        if (p->cpu_pwf.may_have_work) {
            cpu_work_fetch.total_fetchable_share += p->resource_share;
        }
        if (coproc_cuda && p->cuda_pwf.may_have_work) {
            cuda_work_fetch.total_fetchable_share += p->resource_share;
        }
    }
    for (i=0; i<gstate.projects.size(); i++) {
        p = gstate.projects[i];
        if (p->non_cpu_intensive) continue;
        if (p->rr_sim_status.has_cpu_jobs) {
            p->cpu_pwf.runnable_share = p->resource_share/cpu_work_fetch.total_runnable_share;
        }
        if (p->rr_sim_status.has_cuda_jobs) {
            p->cuda_pwf.runnable_share = p->resource_share/cuda_work_fetch.total_runnable_share;
        }
        if (!p->pwf.can_fetch_work) continue;
        if (p->cpu_pwf.may_have_work) {
            p->cpu_pwf.fetchable_share = p->resource_share/cpu_work_fetch.total_fetchable_share;
        }
        if (coproc_cuda && p->cuda_pwf.may_have_work) {
            p->cuda_pwf.fetchable_share = p->resource_share/cuda_work_fetch.total_fetchable_share;
        }
    }
}

// should this project be accumulating debt for this resource?
//
bool RSC_PROJECT_WORK_FETCH::debt_eligible(PROJECT* p) {
    if (backoff_time > gstate.now) return false;
    if (p->suspended_via_gui) return false;
    if (p->dont_request_more_work) return false;
    return true;
}

void WORK_FETCH::write_request(FILE* f) {
    fprintf(f,
        "    <work_req_seconds>%f</work_req_seconds>\n"
        "    <cpu_req_secs>%f</cpu_req_secs>\n"
        "    <cpu_req_instances>%d</cpu_req_instances>\n",
        cpu_work_fetch.req_secs,
        cpu_work_fetch.req_secs,
        cpu_work_fetch.req_instances
    );
}

// we just got a scheduler reply with the given jobs; update backoffs
//
void WORK_FETCH::handle_reply(PROJECT* p, vector<RESULT*> new_results) {
    unsigned int i;
    bool got_cpu = false, got_cuda = false;

    // if didn't get any jobs, back off on requested resource types
    //
    if (!new_results.size()) {
        if (cpu_work_fetch.req_secs) {
            p->cpu_pwf.backoff(p, "CPU");
        }
        if (coproc_cuda && coproc_cuda->req_secs) {
            p->cuda_pwf.backoff(p, "CUDA");
        }
        return;
    }

    // if we did get jobs, clear backoff on resource types
    //
    for (i=0; i<new_results.size(); i++) {
        RESULT* rp = new_results[i];
        if (rp->avp->ncudas) got_cuda = true;
        else got_cpu = true;
    }
    if (got_cpu) p->cpu_pwf.clear_backoff();
    if (got_cuda) p->cuda_pwf.clear_backoff();
}

void WORK_FETCH::set_initial_work_request() {
    cpu_work_fetch.req_secs = 1;
    if (coproc_cuda) {
        coproc_cuda->req_secs = 1;
    }
}

// called once, at client startup
//
void WORK_FETCH::init() {
    cpu_work_fetch.rsc_type = RSC_TYPE_CPU;
    cpu_work_fetch.ninstances = gstate.ncpus;

    if (coproc_cuda) {
        cuda_work_fetch.rsc_type = RSC_TYPE_CUDA;
        cuda_work_fetch.ninstances = coproc_cuda->count;
        cuda_work_fetch.speed = coproc_cuda->flops_estimate()/gstate.host_info.p_fpops;
    }
}

void RSC_PROJECT_WORK_FETCH::backoff(PROJECT* p, char* name) {
    if (backoff_interval) {
        backoff_interval *= 2;
        if (backoff_interval > 86400) backoff_interval = 86400;
    } else {
        backoff_interval = 60;
    }
    backoff_time = gstate.now + backoff_interval;
    if (log_flags.work_fetch_debug) {
        msg_printf(p, MSG_INFO,
            "[wfd] backing off %s %f", name, backoff_interval
        );
    }
}

////////////////////////

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

bool PROJECT::can_request_work() {
    if (suspended_via_gui) return false;
    if (master_url_fetch_pending) return false;
    if (min_rpc_time > gstate.now) return false;
    if (dont_request_more_work) return false;
    return true;
}

bool PROJECT::potentially_runnable() {
    if (runnable()) return true;
    if (can_request_work()) return true;
    if (downloading()) return true;
    return false;
}

bool PROJECT::nearly_runnable() {
    if (runnable()) return true;
    if (downloading()) return true;
    return false;
}

bool RSC_PROJECT_WORK_FETCH::overworked() {
    return debt < -gstate.global_prefs.cpu_scheduling_period();
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

double RESULT::estimated_duration_uncorrected() {
    return wup->rsc_fpops_est/avp->flops;
}

// estimate how long a result will take on this host
//
#ifdef SIM
double RESULT::estimated_duration(bool for_work_fetch) {
    SIM_PROJECT* spp = (SIM_PROJECT*)project;
    if (dual_dcf && for_work_fetch && spp->completions_ratio_mean) {
        return estimated_duration_uncorrected()*spp->completions_ratio_mean;
    }
    return estimated_duration_uncorrected()*project->duration_correction_factor;
}
#else
double RESULT::estimated_duration(bool) {
    return estimated_duration_uncorrected()*project->duration_correction_factor;
}
#endif

double RESULT::estimated_time_remaining(bool for_work_fetch) {
    if (computing_done()) return 0;
    ACTIVE_TASK* atp = gstate.lookup_active_task_by_result(this);
    if (atp) {
        return atp->est_time_to_completion(for_work_fetch);
    }
    return estimated_duration(for_work_fetch);
}

// Returns the estimated CPU time to completion (in seconds) of this task.
// Compute this as a weighted average of estimates based on
// 1) the workunit's flops count
// 2) the current reported CPU time and fraction done
//
double ACTIVE_TASK::est_time_to_completion(bool for_work_fetch) {
    if (fraction_done >= 1) return 0;
    double wu_est = result->estimated_duration(for_work_fetch);
    if (fraction_done <= 0) return wu_est;
    double frac_est = (elapsed_time / fraction_done) - elapsed_time;
    double fraction_left = 1-fraction_done;
    double wu_weight = fraction_left * fraction_left;
    double fd_weight = 1 - wu_weight;
    double x = fd_weight*frac_est + wu_weight*fraction_left*wu_est;
    return x;
}

// the fraction of time a given CPU is working for BOINC
//
double CLIENT_STATE::overall_cpu_frac() {
    double running_frac = time_stats.on_frac * time_stats.active_frac;
    if (running_frac < 0.01) running_frac = 0.01;
    if (running_frac > 1) running_frac = 1;
    return running_frac;
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

