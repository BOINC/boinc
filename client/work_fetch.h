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

// Work fetch logic for CPU, GPU, and other processing resources.
// See http://boinc.berkeley.edu/trac/wiki/GpuWorkFetch

#ifndef _WORK_FETCH_
#define _WORK_FETCH_

#include <vector>

#define RSC_TYPE_CPU    0
#define RSC_TYPE_CUDA   1

class PROJECT;
struct RESULT;
class ACTIVE_TASK;
struct RSC_WORK_FETCH;

// state per (resource, project) pair
//
struct RSC_PROJECT_WORK_FETCH {
    // the following are persistent (saved in state file)
    double backoff_time;
    double backoff_interval;
    double debt;

    // the following used by debt accounting
    double secs_this_debt_interval;
    inline void reset_debt_accounting() {
        secs_this_debt_interval = 0;
    }

    // the following are used by rr_simulation()
    //
    double runnable_share;
        // this project's share relative to projects that have
        // nearly runnable jobs for this resource;
        // determines processing rate for CPU
    double fetchable_share;
        // this project's share relative to projects from which
        // we could probably get work for this resource;
        // determines how many instances this project deserves
    bool has_runnable_jobs;
    double sim_nused;
    int deadlines_missed;
    int deadlines_missed_copy;
        // copy of the above used during enforce_schedule()

    RSC_PROJECT_WORK_FETCH() {
        memset(this, 0, sizeof(*this));
    }

    // whether this project is accumulating debt for this resource
    bool debt_eligible(PROJECT*, RSC_WORK_FETCH&);
    inline void reset() {
        backoff_time = 0;
        backoff_interval = 0;
        debt = 0;
    }

    bool may_have_work;
    bool compute_may_have_work();
    void backoff(PROJECT*, const char*);
    void rr_init();
    void clear_backoff() {
        backoff_time = 0;
        backoff_interval = 0;
    }
    bool overworked();
};

// per-resource state
//
struct RSC_WORK_FETCH {
    int rsc_type;
    int ninstances;
    double speed;   // relative to CPU

    // the following used/set by rr_simulation():
    //
    double shortfall;
        // seconds of idle instances between now and now+work_buf_total()
    double nidle_now;
    double sim_nused;
    double total_fetchable_share;
        // total RS of projects from which we could fetch jobs for this device
    double total_runnable_share;
        // total RS of projects with runnable jobs for this device
    double saturated_time;
        // estimated time until resource is not saturated
        // used to calculate work request
    double busy_time;
        // estimated time until a new job would start;
        // passed to scheduler for crude deadline check.
        // This can't be estimated with any kind of precision.
        // Instead we calculate it as the sum of instance-secs
        // used be missed-deadline jobs, divided by # instances
    double deadline_missed_instances;
        // instance count for jobs that miss deadline
    std::vector<RESULT*> pending;

    // the following specify the work request for this resource
    //
    double req_secs;
    int req_instances;

    // debt accounting
    double secs_this_debt_interval;
    inline void reset_debt_accounting() {
        this->secs_this_debt_interval = 0;
    }

    void rr_init();
    void accumulate_shortfall(double d_time);
    void update_saturated_time(double dt);
    void update_busy_time(double dur, double nused);
    PROJECT* choose_project(int);
    void accumulate_debt();
    RSC_PROJECT_WORK_FETCH& project_state(PROJECT*);
    void update_debts();
    void print_state(const char*);
    void clear_request();
    void set_request(PROJECT*, double);
    double share_request(PROJECT*);
    void set_shortfall_request(PROJECT*);
    bool may_have_work(PROJECT*);
    RSC_WORK_FETCH() {
        memset(this, 0, sizeof(*this));
    }
};


// per project state
//
struct PROJECT_WORK_FETCH {
    double overall_debt;
    bool can_fetch_work;
    bool compute_can_fetch_work(PROJECT*);
    bool has_runnable_jobs;
    PROJECT_WORK_FETCH() {
        memset(this, 0, sizeof(*this));
    }
    void reset(PROJECT*);
};

// global work fetch state
//
struct WORK_FETCH {
    void set_overall_debts();
    PROJECT* choose_project();
        // find a project to ask for work
    PROJECT* non_cpu_intensive_project_needing_work();
    void compute_work_request(PROJECT*);
        // we're going to contact this project anyway;
        // decide how much work to task for
    void accumulate_inst_sec(ACTIVE_TASK*, double dt);
    void write_request(FILE*);
    void handle_reply(PROJECT*, std::vector<RESULT*>new_results);
    void set_initial_work_request();
    void set_shortfall_requests(PROJECT*);
    void print_state();
    void init();
    void rr_init();
    void clear_request();
    void compute_shares();
    void zero_debts();
};

extern RSC_WORK_FETCH cuda_work_fetch;
extern RSC_WORK_FETCH cpu_work_fetch;
extern WORK_FETCH work_fetch;

#endif
