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

#include "cpp.h"

#ifdef _WIN32
#include "boinc_win.h"
#else
#include "config.h"
#endif

#include "util.h"

#include "client_msgs.h"
#include "client_state.h"
#include "project.h"
#include "result.h"
#include "scheduler_op.h"

#include "work_fetch.h"

using std::vector;

bool use_hyst_fetch = true;

RSC_WORK_FETCH rsc_work_fetch[MAX_RSC];
WORK_FETCH work_fetch;

#define FETCH_IF_IDLE_INSTANCE          0
    // If resource has an idle instance,
    // get work for it from the project with greatest LTD,
    // even if it's overworked.
#define FETCH_IF_MAJOR_SHORTFALL        1
    // If resource is saturated for less than work_buf_min(),
    // get work for it from the project with greatest LTD,
    // even if it's overworked.
#define FETCH_IF_MINOR_SHORTFALL        2
    // If resource is saturated for less than work_buf_total(),
    // get work for it from the non-overworked project with greatest LTD.
#define FETCH_IF_PROJECT_STARVED        3
    // If any project is not overworked and has too few jobs
    // to use its instance share,
    // get work from the one with greatest LTD.

static const char* criterion_name(int criterion) {
    switch (criterion) {
    case FETCH_IF_IDLE_INSTANCE: return "idle instance";
    case FETCH_IF_MAJOR_SHORTFALL: return "major shortfall";
    case FETCH_IF_MINOR_SHORTFALL: return "minor shortfall";
    case FETCH_IF_PROJECT_STARVED: return "starved";
    }
    return "unknown";
}

inline bool dont_fetch(PROJECT* p, int rsc_type) {
    if (p->no_rsc_pref[rsc_type]) return true;
    if (p->no_rsc_config[rsc_type]) return true;
    if (p->no_rsc_apps[rsc_type]) return true;
    if (p->no_rsc_ams[rsc_type]) return true;
    return false;
}

// if the configuration file disallows the use of a GPU type
// for a project, set a flag to that effect
//
void set_no_rsc_config() {
    for (unsigned int i=0; i<gstate.projects.size(); i++) {
        PROJECT& p = *gstate.projects[i];
        for (int j=1; j<coprocs.n_rsc; j++) {
            bool allowed[MAX_COPROC_INSTANCES];
            memset(allowed, 0, sizeof(allowed));
            COPROC& c = coprocs.coprocs[j];
            for (int k=0; k<c.count; k++) {
                allowed[c.device_nums[k]] = true;
            }
            for (unsigned int k=0; k<config.exclude_gpus.size(); k++) {
                EXCLUDE_GPU& e = config.exclude_gpus[k];
                if (strcmp(e.url.c_str(), p.master_url)) continue;
                if (!e.type.empty() && strcmp(e.type.c_str(), c.type)) continue;
                if (!e.appname.empty()) continue;
                if (e.device_num < 0) {
                    memset(allowed, 0, sizeof(allowed));
                    break;
                }
                allowed[e.device_num] = false;
            }
            p.no_rsc_config[j] = true;
            for (int k=0; k<c.count; k++) {
                if (allowed[c.device_nums[k]]) {
                    p.no_rsc_config[j] = false;
                    break;
                }
            }
        }
    }
}

// does the project have a downloading or runnable job?
//
static bool has_a_job(PROJECT* p) {
    for (unsigned int j=0; j<gstate.results.size(); j++) {
        RESULT* rp = gstate.results[j];
        if (rp->project != p) continue;
        if (rp->state() <= RESULT_FILES_DOWNLOADED) {
            return true;
        }
    }
    return false;
}

inline bool has_coproc_app(PROJECT* p, int rsc_type) {
    unsigned int i;
    for (i=0; i<gstate.app_versions.size(); i++) {
        APP_VERSION* avp = gstate.app_versions[i];
        if (avp->project != p) continue;
        if (avp->gpu_usage.rsc_type == rsc_type) return true;
    }
    return false;
}

///////////////  RSC_PROJECT_WORK_FETCH  ///////////////

bool RSC_PROJECT_WORK_FETCH::compute_may_have_work(PROJECT* p, int rsc_type) {
    if (dont_fetch(p, rsc_type)) return false;
    if (p->rsc_defer_sched[rsc_type]) return false;
    return (backoff_time < gstate.now);
}

void RSC_PROJECT_WORK_FETCH::rr_init(PROJECT* p, int rsc_type) {
    may_have_work = compute_may_have_work(p, rsc_type);
    fetchable_share = 0;
    n_runnable_jobs = 0;
    sim_nused = 0;
    nused_total = 0;
    deadlines_missed = 0;
}

void RSC_PROJECT_WORK_FETCH::backoff(PROJECT* p, const char* name) {
    if (backoff_interval) {
        backoff_interval *= 2;
        if (backoff_interval > WF_MAX_BACKOFF_INTERVAL) backoff_interval = WF_MAX_BACKOFF_INTERVAL;
    } else {
        backoff_interval = WF_MIN_BACKOFF_INTERVAL;
    }
    double x = (.5 + drand())*backoff_interval;
    backoff_time = gstate.now + x;
    if (log_flags.work_fetch_debug) {
        msg_printf(p, MSG_INFO,
            "[work_fetch] backing off %s %.0f sec", name, x
        );
    }
}

///////////////  RSC_WORK_FETCH  ///////////////

RSC_PROJECT_WORK_FETCH& RSC_WORK_FETCH::project_state(PROJECT* p) {
    return p->rsc_pwf[rsc_type];
}

#if 0
bool RSC_WORK_FETCH::may_have_work(PROJECT* p) {
    if (dont_fetch(p, rsc_type)) return false;
    RSC_PROJECT_WORK_FETCH& w = project_state(p);
    return (w.backoff_time < gstate.now);
}
#endif

void RSC_WORK_FETCH::rr_init() {
    shortfall = 0;
    nidle_now = 0;
    sim_nused = 0;
    total_fetchable_share = 0;
    deadline_missed_instances = 0;
    saturated_time = 0;
    busy_time_estimator.reset();
    sim_used_instances = 0;
}

void RSC_WORK_FETCH::accumulate_shortfall(double d_time) {
    double idle = ninstances - sim_nused;
    if (idle > 1e-6) {
        //msg_printf(0, MSG_INFO, "adding shortfall %d %f", rsc_type, idle*d_time);
        shortfall += idle*d_time;
    }
#if 0
    msg_printf(0, MSG_INFO, "accum shortf (%s): idle %f dt %f sf %f",
        rsc_name(rsc_type), idle, d_time, shortfall
    );
#endif
}

void RSC_WORK_FETCH::update_saturated_time(double dt) {
    double idle = ninstances - sim_nused;
    //msg_printf(0, MSG_INFO, "update_saturated rsc %d idle %f dt %f", rsc_type, idle, dt);
    if (idle < 1e-6) {
        saturated_time = dt;
    }
}

void RSC_WORK_FETCH::update_busy_time(double dur, double nused) {
    busy_time_estimator.update(dur, nused);
}

static bool wacky_dcf(PROJECT* p) {
    if (p->dont_use_dcf) return false;
    double dcf = p->duration_correction_factor;
    return (dcf < 0.02 || dcf > 80.0);
}

// If this resource is below min buffer level,
// return the highest-priority project that may have jobs for it.
//
// It the resource has instanced starved because of exclusions,
// return the highest-priority project that may have jobs
// and doesn't exclude those instances.
//
// If strict is true, enforce hysteresis and backoff rules
// (which are there to limit rate of scheduler RPCs).
// Otherwise, we're going to do a scheduler RPC anyway
// and we're deciding whether to piggyback a work request,
// so there is no reason to enforce these rules.
//
PROJECT* RSC_WORK_FETCH::choose_project_hyst(bool strict) {
    PROJECT* pbest = NULL;
    bool buffer_low = true;
    if (strict) {
        if (saturated_time > gstate.work_buf_min()) buffer_low = false;
    } else {
        if (saturated_time > gstate.work_buf_total()) buffer_low = false;
    }

    if (log_flags.work_fetch_debug) {
        msg_printf(0, MSG_INFO,
            "[work_fetch] %s: buffer_low: %s; sim_excluded_instances %d\n",
            rsc_name(rsc_type), buffer_low?"yes":"no", sim_excluded_instances
        );
    }

    if (!buffer_low && !sim_excluded_instances) return NULL;

    for (unsigned i=0; i<gstate.projects.size(); i++) {
        PROJECT* p = gstate.projects[i];

        // check whether we can fetch work of any type from this project
        //
        if (p->pwf.cant_fetch_work_reason) continue;

        // check whether we can fetch work of this type
        //
        if (dont_fetch(p, rsc_type)) continue;

        // if strict, check backoff
        //
        if (strict) {
            if (project_state(p).backoff_time > gstate.now) {
                continue;
            }
        }

        // if project has zero resource share,
        // only fetch work if a device is idle
        //
        if (p->resource_share == 0 && nidle_now == 0) {
            continue;
        }

        // if project has excluded GPUs of this type,
        // and it has more runnable jobs than non-excluded instances,
        // don't fetch work for it.
        // TODO: THIS IS CRUDE. Making it smarter would require
        // computing shortfall etc. on a per-project basis
        //
        if (rsc_type) {
            int n_not_excluded = ninstances - p->rsc_pwf[rsc_type].ncoprocs_excluded;
            if (n_not_excluded == 0) {
                continue;
            }
            if (p->rsc_pwf[rsc_type].ncoprocs_excluded
                && p->rsc_pwf[rsc_type].n_runnable_jobs > n_not_excluded
            ) {
                continue;
            }
        }

        RSC_PROJECT_WORK_FETCH& rpwf = project_state(p);
        if (rpwf.anon_skip) continue;

        // if we're sending work only because of exclusion starvation,
        // make sure this project can use the starved instances
        //
        if (!buffer_low) {
            if ((sim_excluded_instances & rpwf.non_excluded_instances) == 0) {
                continue;
            }
        }

        if (pbest) {
            if (pbest->sched_priority > p->sched_priority) {
                continue;
            }
        }
        pbest = p;
    }
    if (!pbest) return NULL;
    work_fetch.clear_request();
    if (buffer_low) {
        work_fetch.set_all_requests_hyst(pbest, rsc_type);
    } else {
        set_request_excluded(pbest);
    }
    return pbest;
}

// Choose the best project to ask for work for this resource,
// given the specific criterion
//
PROJECT* RSC_WORK_FETCH::choose_project(int criterion) {
    PROJECT* pbest = NULL;

    switch (criterion) {
    case FETCH_IF_IDLE_INSTANCE:
        if (nidle_now == 0) return NULL;
        break;
    case FETCH_IF_MAJOR_SHORTFALL:
        if (saturated_time > gstate.work_buf_min()) return NULL;
        break;
    case FETCH_IF_MINOR_SHORTFALL:
        if (saturated_time > gstate.work_buf_total()) return NULL;
        break;
    case FETCH_IF_PROJECT_STARVED:
        if (deadline_missed_instances >= ninstances) return NULL;
        break;
    }

    for (unsigned i=0; i<gstate.projects.size(); i++) {
        PROJECT* p = gstate.projects[i];
        if (p->pwf.cant_fetch_work_reason) continue;
        if (!project_state(p).may_have_work) continue;
        RSC_PROJECT_WORK_FETCH& rpwf = project_state(p);
        if (rpwf.anon_skip) continue;
        switch (criterion) {
        case FETCH_IF_MINOR_SHORTFALL:
            if (wacky_dcf(p)) continue;
            if (!p->resource_share) continue;
            break;
        case FETCH_IF_MAJOR_SHORTFALL:
            if (wacky_dcf(p)) continue;
            if (!p->resource_share) continue;
            break;
        case FETCH_IF_PROJECT_STARVED:
            if (p->sched_priority < 0) continue;
            if (rpwf.nused_total >= ninstances) continue;
            if (!p->resource_share) continue;
            break;
        }

        if (pbest) {
            if (!p->resource_share) {
                continue;
            }
            if (pbest->sched_priority > p->sched_priority) {
                continue;
            }
        }
        pbest = p;
    }
    if (!pbest) return NULL;

    // decide how much work to request from each resource
    //
    work_fetch.clear_request();
    switch (criterion) {
    case FETCH_IF_IDLE_INSTANCE:
    case FETCH_IF_MAJOR_SHORTFALL:
        set_request(pbest);
        break;
    case FETCH_IF_PROJECT_STARVED:
        set_request(pbest);
        break;
    case FETCH_IF_MINOR_SHORTFALL:
        // in this case, potentially request work for all resources
        //
        if (pbest->sched_priority < 0) {
            set_request(pbest);
        } else {
            work_fetch.set_all_requests(pbest);
        }
        break;
    }
    // in principle there should be a nonzero request.
    // check, just in case
    //
    if (!req_secs && !req_instances) {
        if (log_flags.work_fetch_debug) {
            msg_printf(pbest, MSG_INFO,
                "[work_fetch] error: project chosen but zero request"
            );
        }
        return 0;
    }

    if (log_flags.work_fetch_debug) {
        msg_printf(pbest, MSG_INFO,
            "[work_fetch] chosen: %s %s: %.2f inst, %.2f sec",
            criterion_name(criterion), rsc_name(rsc_type),
            req_instances, req_secs
        );
    }

    return pbest;
}

// request this project's share of shortfall and instances.
// don't request anything if project is overworked or backed off.
//
void RSC_WORK_FETCH::set_request(PROJECT* p) {
    if (dont_fetch(p, rsc_type)) return;

    // if backup project, fetch 1 job per idle instance
    //
    if (p->resource_share == 0) {
        req_instances = nidle_now;
        req_secs = 1;
        return;
    }
    if (config.fetch_minimal_work) {
        req_instances = ninstances;
        req_secs = 1;
        return;
    }
    RSC_PROJECT_WORK_FETCH& w = project_state(p);
    if (!w.may_have_work) return;
    if (w.anon_skip) return;
    if (shortfall) {
        if (wacky_dcf(p)) {
            // if project's DCF is too big or small,
            // its completion time estimates are useless; just ask for 1 second
            //
            req_secs = 1;
        } else {
            req_secs = shortfall;
        }
    }

    // our share of the idle instances
    //
    req_instances = nidle_now * w.fetchable_share;

    if (log_flags.work_fetch_debug) {
        msg_printf(0, MSG_INFO,
            "[work_fetch] set_request(): ninst %d nused_total %f nidle_now %f fetch share %f req_inst %f",
            ninstances, w.nused_total, nidle_now, w.fetchable_share, req_instances
        );
    }
    if (req_instances && !req_secs) {
        req_secs = 1;
    }
}

// We're fetching work because some instances are starved because
// of exclusions.
// See how many N of these instances are not excluded for this project.
// Ask for N instances and for N*work_buf_min seconds.
//
void RSC_WORK_FETCH::set_request_excluded(PROJECT* p) {
    RSC_PROJECT_WORK_FETCH& pwf = project_state(p);

    int inst_mask = sim_excluded_instances & pwf.non_excluded_instances;
    int n = 0;
    for (int i=0; i<ninstances; i++) {
        if ((i<<i) & inst_mask) {
            n++;
        }
    }
    req_instances = n;
    if (p->resource_share == 0 || config.fetch_minimal_work) {
        req_secs = 1;
    } else {
        req_secs = n*gstate.work_buf_total();
    }
}

void RSC_WORK_FETCH::print_state(const char* name) {
    msg_printf(0, MSG_INFO, "[work_fetch] --- state for %s ---", name);
    msg_printf(0, MSG_INFO,
        "[work_fetch] shortfall %.2f nidle %.2f saturated %.2f busy %.2f",
        shortfall, nidle_now, saturated_time,
        busy_time_estimator.get_busy_time()
    );
    for (unsigned int i=0; i<gstate.projects.size(); i++) {
        char buf[256];
        PROJECT* p = gstate.projects[i];
        if (p->non_cpu_intensive) continue;
        RSC_PROJECT_WORK_FETCH& pwf = project_state(p);
        bool no_rsc_pref = p->no_rsc_pref[rsc_type];
        bool no_rsc_config = p->no_rsc_config[rsc_type];
        bool no_rsc_apps = p->no_rsc_apps[rsc_type];
        bool no_rsc_ams = p->no_rsc_ams[rsc_type];
        double bt = pwf.backoff_time>gstate.now?pwf.backoff_time-gstate.now:0;
        if (bt) {
            sprintf(buf, " (resource backoff: %.2f)", bt);
        } else {
            strcpy(buf, "");
        }
        msg_printf(p, MSG_INFO,
            "[work_fetch] fetch share %.3f%s%s%s%s%s",
            pwf.fetchable_share,
            buf,
            no_rsc_pref?" (blocked by prefs)":"",
            no_rsc_apps?" (no apps)":"",
            no_rsc_ams?" (blocked by account manager)":"",
            no_rsc_config?" (blocked by configuration file)":""
        );
    }
}

void RSC_WORK_FETCH::clear_request() {
    req_secs = 0;
    req_instances = 0;
}

///////////////  PROJECT_WORK_FETCH  ///////////////

int PROJECT_WORK_FETCH::compute_cant_fetch_work_reason(PROJECT* p) {
    if (p->non_cpu_intensive) return CANT_FETCH_WORK_NON_CPU_INTENSIVE;
    if (p->suspended_via_gui) return CANT_FETCH_WORK_SUSPENDED_VIA_GUI;
    if (p->master_url_fetch_pending) return CANT_FETCH_WORK_MASTER_URL_FETCH_PENDING;
    if (p->dont_request_more_work) return CANT_FETCH_WORK_DONT_REQUEST_MORE_WORK;
    if (p->some_download_stalled()) return CANT_FETCH_WORK_DOWNLOAD_STALLED;
    if (p->some_result_suspended()) return CANT_FETCH_WORK_RESULT_SUSPENDED;
    if (p->too_many_uploading_results) return CANT_FETCH_WORK_TOO_MANY_UPLOADS;

    // this goes last
    //
    if (p->min_rpc_time > gstate.now) return CANT_FETCH_WORK_MIN_RPC_TIME;
    return 0;
}

void PROJECT_WORK_FETCH::reset(PROJECT* p) {
    for (int i=0; i<coprocs.n_rsc; i++) {
        p->rsc_pwf[i].reset();
    }
}

///////////////  WORK_FETCH  ///////////////

// mark the projects from which we can fetch work
//
void WORK_FETCH::compute_cant_fetch_work_reason() {
    for (unsigned int i=0; i<gstate.projects.size(); i++) {
        PROJECT* p = gstate.projects[i];
        p->pwf.cant_fetch_work_reason = p->pwf.compute_cant_fetch_work_reason(p);
    }
}

void WORK_FETCH::rr_init() {
    for (int i=0; i<coprocs.n_rsc; i++) {
        rsc_work_fetch[i].rr_init();
    }
    compute_cant_fetch_work_reason();
    for (unsigned int i=0; i<gstate.projects.size(); i++) {
        PROJECT* p = gstate.projects[i];
        p->pwf.n_runnable_jobs = 0;
        for (int j=0; j<coprocs.n_rsc; j++) {
            p->rsc_pwf[j].rr_init(p, j);
        }
    }
}

// if the given project is highest-priority among the projects
// eligible for the resource, set request fields
//
void RSC_WORK_FETCH::supplement(PROJECT* pp) {
    double x = pp->sched_priority;
    for (unsigned i=0; i<gstate.projects.size(); i++) {
        PROJECT* p = gstate.projects[i];
        if (p == pp) continue;
        if (p->pwf.cant_fetch_work_reason) continue;
        if (!project_state(p).may_have_work) continue;
        RSC_PROJECT_WORK_FETCH& rpwf = project_state(p);
        if (rpwf.anon_skip) continue;
        if (p->sched_priority > x) {
            return;
        }
    }
    // didn't find a better project; ask for work
    //
    set_request(pp);
}

// we're going to ask the given project for work of the given type.
// (or -1 if none)
// Set requests for this type and perhaps other types
//
void WORK_FETCH::set_all_requests_hyst(PROJECT* p, int rsc_type) {
    for (int i=0; i<coprocs.n_rsc; i++) {
        if (i == rsc_type) {
            rsc_work_fetch[i].set_request(p);
        } else {
            // don't fetch work for a resource if the buffer is above max
            //
            if (rsc_work_fetch[i].saturated_time > gstate.work_buf_total()) {
                continue;
            }
            
            // don't fetch work if backup project and no idle instances
            //
            if (p->resource_share==0 && rsc_work_fetch[i].nidle_now==0) {
                continue;
            }

            if (i>0 && !gpus_usable) {
                continue;
            }
            rsc_work_fetch[i].supplement(p);
        }
    }
}

void WORK_FETCH::set_all_requests(PROJECT* p) {
    for (int i=0; i<coprocs.n_rsc; i++) {
        if (i==0 || gpus_usable) {
            rsc_work_fetch[i].set_request(p);
        }
    }
}

void WORK_FETCH::print_state() {
    msg_printf(0, MSG_INFO, "[work_fetch] ------- start work fetch state -------");
    msg_printf(0, MSG_INFO, "[work_fetch] target work buffer: %.2f + %.2f sec",
        gstate.work_buf_min(), gstate.work_buf_additional()
    );
    msg_printf(0, MSG_INFO, "[work_fetch] --- project states ---");
    for (unsigned int i=0; i<gstate.projects.size(); i++) {
        PROJECT* p = gstate.projects[i];
        char buf[256];
        if (p->pwf.cant_fetch_work_reason) {
            sprintf(buf, "can't req work: %s",
                cant_fetch_work_string(p->pwf.cant_fetch_work_reason)
            );
        } else {
            strcpy(buf, "can req work");
        }
        msg_printf(p, MSG_INFO, "[work_fetch] REC %.3f prio %.6f %s",
            p->pwf.rec,
            p->sched_priority,
            buf
        );
    }
    for (int i=0; i<coprocs.n_rsc; i++) {
        rsc_work_fetch[i].print_state(rsc_name(i));
    }
    msg_printf(0, MSG_INFO, "[work_fetch] ------- end work fetch state -------");
}

void WORK_FETCH::clear_request() {
    for (int i=0; i<coprocs.n_rsc; i++) {
        rsc_work_fetch[i].clear_request();
    }
}

// we're going to contact this project for reasons other than work fetch;
// decide if we should piggy-back a work fetch request.
//
void WORK_FETCH::piggyback_work_request(PROJECT* p) {
    clear_request();
    if (config.fetch_minimal_work && gstate.had_or_requested_work) return;
    if (p->dont_request_more_work) return;
    if (p->non_cpu_intensive) {
        if (!has_a_job(p)) {
            rsc_work_fetch[0].req_secs = 1;
        }
        return;
    }

    compute_cant_fetch_work_reason();
    PROJECT* bestp = choose_project(false);
    if (p != bestp) {
        if (p->pwf.cant_fetch_work_reason == 0) {
            if (bestp) {
                p->pwf.cant_fetch_work_reason = CANT_FETCH_WORK_NOT_HIGHEST_PRIORITY;
                if (log_flags.work_fetch_debug) {
                    msg_printf(0, MSG_INFO,
                        "[work_fetch] not piggybacking work req: %s has higher priority",
                        bestp->get_project_name()
                    );
                }
            } else {
                p->pwf.cant_fetch_work_reason = CANT_FETCH_WORK_DONT_NEED;
            }
        }
        clear_request();
    }
}

// see if there's a fetchable non-CPU-intensive project without work
//
PROJECT* WORK_FETCH::non_cpu_intensive_project_needing_work() {
    for (unsigned int i=0; i<gstate.projects.size(); i++) {
        PROJECT* p = gstate.projects[i];
        if (!p->non_cpu_intensive) continue;
        if (!p->can_request_work()) continue;
        if (p->rsc_pwf[0].backoff_time > gstate.now) continue;
        if (has_a_job(p)) continue;
        clear_request();
        rsc_work_fetch[0].req_secs = 1;
        return p;
    }
    return 0;
}

// choose a project to fetch work from,
// and set the request fields of resource objects.
// If strict is true, enforce hysteresis and backoff rules
//
PROJECT* WORK_FETCH::choose_project(bool strict) {
    PROJECT* p;

    if (log_flags.work_fetch_debug) {
        msg_printf(0, MSG_INFO, "[work_fetch] work fetch start");
    }

    p = non_cpu_intensive_project_needing_work();
    if (p) return p;

    gstate.compute_nuploading_results();

    rr_simulation();
    compute_shares();
    project_priority_init(true);

    // Decrement the priority of projects that have a lot of work queued.
    // Specifically, subtract
    // (FLOPs queued for P)/(FLOPs of max queue)
    // which will generally be between 0 and 1.
    // This is a little arbitrary but I can't think of anything better.
    //
    double max_queued_flops = gstate.work_buf_total()*total_peak_flops();
    for (unsigned int i=0; i<gstate.results.size(); i++) {
        RESULT* rp = gstate.results[i];
        p = rp->project;
        p->sched_priority -= rp->estimated_flops_remaining()/max_queued_flops;
    }

    p = 0;
if (use_hyst_fetch) {
    if (gpus_usable) {
        for (int i=1; i<coprocs.n_rsc; i++) {
            p = rsc_work_fetch[i].choose_project_hyst(strict);
            if (p) break;
        }
    }
    if (!p) {
        p = rsc_work_fetch[0].choose_project_hyst(strict);
    }
} else {
    if (gpus_usable) {
        for (int i=1; i<coprocs.n_rsc; i++) {
            p = rsc_work_fetch[i].choose_project(FETCH_IF_IDLE_INSTANCE);
            if (p) break;
        }
    }
    if (!p) {
        p = rsc_work_fetch[0].choose_project(FETCH_IF_IDLE_INSTANCE);
    }

    if (!p && gpus_usable) {
        for (int i=1; i<coprocs.n_rsc; i++) {
            p = rsc_work_fetch[i].choose_project(FETCH_IF_MAJOR_SHORTFALL);
            if (p) break;
        }
    }
    if (!p) {
        p = rsc_work_fetch[0].choose_project(FETCH_IF_MAJOR_SHORTFALL);
    }
    
    if (!p && gpus_usable) {
        for (int i=1; i<coprocs.n_rsc; i++) {
            p = rsc_work_fetch[i].choose_project(FETCH_IF_MINOR_SHORTFALL);
            if (p) break;
        }
    }
    if (!p) {
        p = rsc_work_fetch[0].choose_project(FETCH_IF_MINOR_SHORTFALL);
    }

    if (!p && gpus_usable) {
        for (int i=1; i<coprocs.n_rsc; i++) {
            p = rsc_work_fetch[i].choose_project(FETCH_IF_PROJECT_STARVED);
            if (p) break;
        }
    }
    if (!p) {
        p = rsc_work_fetch[0].choose_project(FETCH_IF_PROJECT_STARVED);
    }
}

    if (log_flags.work_fetch_debug) {
        print_state();
        if (!p) {
            msg_printf(0, MSG_INFO, "[work_fetch] No project chosen for work fetch");
        }
    }

    return p;
}

void WORK_FETCH::accumulate_inst_sec(ACTIVE_TASK* atp, double dt) {
    APP_VERSION* avp = atp->result->avp;
    PROJECT* p = atp->result->project;
    double x = dt*avp->avg_ncpus;
    p->rsc_pwf[0].secs_this_debt_interval += x;
    rsc_work_fetch[0].secs_this_debt_interval += x;
    int rt = avp->gpu_usage.rsc_type;
    if (rt) {
        x = dt*avp->gpu_usage.usage;
        p->rsc_pwf[rt].secs_this_debt_interval += x;
        rsc_work_fetch[rt].secs_this_debt_interval += x;
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
        if (p->pwf.cant_fetch_work_reason) continue;
        for (int j=0; j<coprocs.n_rsc; j++) {
            if (p->rsc_pwf[j].may_have_work) {
                rsc_work_fetch[j].total_fetchable_share += p->resource_share;
            }
        }
    }
    for (i=0; i<gstate.projects.size(); i++) {
        p = gstate.projects[i];
        if (p->non_cpu_intensive) continue;
        if (p->pwf.cant_fetch_work_reason) continue;
        for (int j=0; j<coprocs.n_rsc; j++) {
            if (p->rsc_pwf[j].may_have_work) {
                p->rsc_pwf[j].fetchable_share = rsc_work_fetch[j].total_fetchable_share?p->resource_share/rsc_work_fetch[j].total_fetchable_share:1;
            }
        }
    }
}

void WORK_FETCH::request_string(char* buf) {
    char buf2[256];
    sprintf(buf,
        "[work_fetch] request: CPU (%.2f sec, %.2f inst)",
        rsc_work_fetch[0].req_secs, rsc_work_fetch[0].req_instances
    );
    for (int i=1; i<coprocs.n_rsc; i++) {
        sprintf(buf2, " %s (%.2f sec, %.2f inst)",
            rsc_name(i), rsc_work_fetch[i].req_secs, rsc_work_fetch[i].req_instances
        );
        strcat(buf, buf2);
    }
}

void WORK_FETCH::write_request(FILE* f, PROJECT* p) {
    double work_req = rsc_work_fetch[0].req_secs;

    // if project is anonymous platform, set the overall work req
    // to the max of the requests of resource types for which we have versions.
    // Otherwise projects with old schedulers won't send us work.
    // THIS CAN BE REMOVED AT SOME POINT
    //
    if (p->anonymous_platform) {
        for (int i=1; i<coprocs.n_rsc; i++) {
            if (has_coproc_app(p, i)) {
                if (rsc_work_fetch[i].req_secs > work_req) {
                    work_req = rsc_work_fetch[i].req_secs;
                }
            }
        }
    }
    fprintf(f,
        "    <work_req_seconds>%f</work_req_seconds>\n"
        "    <cpu_req_secs>%f</cpu_req_secs>\n"
        "    <cpu_req_instances>%f</cpu_req_instances>\n"
        "    <estimated_delay>%f</estimated_delay>\n",
        work_req,
        rsc_work_fetch[0].req_secs,
        rsc_work_fetch[0].req_instances,
        rsc_work_fetch[0].req_secs?rsc_work_fetch[0].busy_time_estimator.get_busy_time():0
    );
    if (log_flags.work_fetch_debug) {
        char buf[256];
        request_string(buf);
        msg_printf(p, MSG_INFO, buf);
    }
}

// we just got a scheduler reply with the given jobs; update backoffs
//
void WORK_FETCH::handle_reply(
    PROJECT* p, SCHEDULER_REPLY*, vector<RESULT*> new_results
) {
    bool got_rsc[MAX_RSC];
    for (int i=0; i<coprocs.n_rsc; i++) {
        got_rsc[i] = false;
    }

    // if didn't get any jobs, back off on requested resource types
    //
    if (!new_results.size()) {
        // but not if RPC was requested by project
        //
        if (p->sched_rpc_pending != RPC_REASON_PROJECT_REQ) {
            for (int i=0; i<coprocs.n_rsc; i++) {
                if (rsc_work_fetch[i].req_secs) {
                    p->rsc_pwf[i].backoff(p, rsc_name(i));
                }
            }
        }
        return;
    }

    // if we did get jobs, clear backoff on resource types
    //
    for (unsigned int i=0; i<new_results.size(); i++) {
        RESULT* rp = new_results[i];
        got_rsc[rp->avp->gpu_usage.rsc_type] = true;
    }
    for (int i=0; i<coprocs.n_rsc; i++) {
        if (got_rsc[i]) p->rsc_pwf[i].clear_backoff();
    }
}

// set up for initial RPC.
// arrange to always get one job, even if we don't need it or can't handle it.
// (this is probably what user wants)
//
void WORK_FETCH::set_initial_work_request(PROJECT* p) {
    for (int i=0; i<coprocs.n_rsc; i++) {
        rsc_work_fetch[i].req_secs = 1;
        if (i) {
            RSC_WORK_FETCH& rwf = rsc_work_fetch[i];
            if (rwf.ninstances ==  p->rsc_pwf[i].ncoprocs_excluded) {
                rsc_work_fetch[i].req_secs = 0;
            }
        }
        rsc_work_fetch[i].req_instances = 0;
        rsc_work_fetch[i].busy_time_estimator.reset();
    }
}

// called once, at client startup
//
void WORK_FETCH::init() {
    rsc_work_fetch[0].init(0, gstate.ncpus, 1);
    double cpu_flops = gstate.host_info.p_fpops;

    // use 20% as a rough estimate of GPU efficiency

    for (int i=1; i<coprocs.n_rsc; i++) {
        rsc_work_fetch[i].init(
            i, coprocs.coprocs[i].count,
            coprocs.coprocs[i].count*0.2*coprocs.coprocs[i].peak_flops/cpu_flops
        );
    }

    // see what resources anon platform projects can use
    //
    unsigned int i, j;
    for (i=0; i<gstate.projects.size(); i++) {
        PROJECT* p = gstate.projects[i];
        if (!p->anonymous_platform) continue;
        for (int k=0; k<coprocs.n_rsc; k++) {
            p->rsc_pwf[k].anon_skip = true;
        }
        for (j=0; j<gstate.app_versions.size(); j++) {
            APP_VERSION* avp = gstate.app_versions[j];
            if (avp->project != p) continue;
            p->rsc_pwf[avp->gpu_usage.rsc_type].anon_skip = false;
        }
    }
}

// clear backoff for app's resource
//
void WORK_FETCH::clear_backoffs(APP_VERSION& av) {
    av.project->rsc_pwf[av.gpu_usage.rsc_type].clear_backoff();
}

////////////////////////

void CLIENT_STATE::compute_nuploading_results() {
    unsigned int i;

    for (i=0; i<projects.size(); i++) {
        projects[i]->nuploading_results = 0;
        projects[i]->too_many_uploading_results = false;
    }
    for (i=0; i<results.size(); i++) {
        RESULT* rp = results[i];
        if (rp->state() == RESULT_FILES_UPLOADING) {
            rp->project->nuploading_results++;
        }
    }
    int n = gstate.ncpus;
    for (int j=1; j<coprocs.n_rsc; j++) {
        if (coprocs.coprocs[j].count > n) {
            n = coprocs.coprocs[j].count;
        }
    }
    n *= 2;
    for (i=0; i<projects.size(); i++) {
        if (projects[i]->nuploading_results > n) {
            projects[i]->too_many_uploading_results = true;
        }
    }
}

// Returns the estimated total elapsed time of this task.
// Compute this as a weighted average of estimates based on
// 1) the workunit's flops count (static estimate)
// 2) the current elapsed time and fraction done (dynamic estimate)
//
double ACTIVE_TASK::est_dur() {
    if (fraction_done >= 1) return elapsed_time;
    double wu_est = result->estimated_runtime();
    if (fraction_done <= 0) return wu_est;
    if (wu_est < elapsed_time) wu_est = elapsed_time;
    double frac_est = fraction_done_elapsed_time / fraction_done;
    double fd_weight = fraction_done * fraction_done;
    double wu_weight = 1 - fd_weight;
    double x = fd_weight*frac_est + wu_weight*wu_est;
#if 0
    //if (log_flags.rr_simulation) {
        msg_printf(result->project, MSG_INFO,
            "[rr_sim] %s frac_est %f = %f/%f",
            result->name, frac_est, fraction_done_elapsed_time, fraction_done
        );
        msg_printf(result->project, MSG_INFO,
            "[rr_sim] %s dur: %.2f = %.3f*%.2f + %.3f*%.2f",
            result->name, x, fd_weight, frac_est, wu_weight, wu_est
        );
    //}
#endif
    return x;
}

// the fraction of time BOINC is processing
//
double CLIENT_STATE::overall_cpu_frac() {
    double x = time_stats.on_frac * time_stats.active_frac;
    if (x < 0.01) x = 0.01;
    if (x > 1) x = 1;
    return x;
}
double CLIENT_STATE::overall_gpu_frac() {
    double x = time_stats.on_frac * time_stats.gpu_active_frac;
    if (x < 0.01) x = 0.01;
    if (x > 1) x = 1;
    return x;
}
double CLIENT_STATE::overall_cpu_and_network_frac() {
    double x = time_stats.on_frac * time_stats.cpu_and_network_available_frac;
    if (x < 0.01) x = 0.01;
    if (x > 1) x = 1;
    return x;
}

// called when benchmarks change
//
void CLIENT_STATE::scale_duration_correction_factors(double factor) {
    if (factor <= 0) return;
    for (unsigned int i=0; i<projects.size(); i++) {
        PROJECT* p = projects[i];
        if (p->dont_use_dcf) continue;
        p->duration_correction_factor *= factor;
    }
    if (log_flags.dcf_debug) {
        msg_printf(NULL, MSG_INFO,
            "[dcf] scaling all duration correction factors by %f",
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
