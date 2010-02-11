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

#define RSC_TYPE_ANY    0
#define RSC_TYPE_CPU    1
#define RSC_TYPE_CUDA   2
#define RSC_TYPE_ATI    3

struct PROJECT;
struct RESULT;
struct ACTIVE_TASK;
struct RSC_WORK_FETCH;
struct SCHEDULER_REPLY;

// state per (resource, project) pair
//
struct RSC_PROJECT_WORK_FETCH {
    // the following are persistent (saved in state file)
    double backoff_time;
    double backoff_interval;
    double long_term_debt;
    double short_term_debt;

    // the following used by debt accounting
    double anticipated_debt;
        // short-term debt, adjusted by scheduled jobs
    double secs_this_debt_interval;
    inline void reset_debt_accounting() {
        secs_this_debt_interval = 0;
    }
    double queue_est;
        // an estimate of instance-secs of queued work;
        // a temp used in computing overall debts
    bool anon_skip;
        // set if this project is anonymous platform
        // and it has no app version that uses this resource

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
    double nused_total;     // sum of instances over all runnable jobs
    int deadlines_missed;
    int deadlines_missed_copy;
        // copy of the above used during schedule_cpus()

    RSC_PROJECT_WORK_FETCH() {
        memset(this, 0, sizeof(*this));
    }

    // whether this project should accumulate debt for this resource
    //
    bool debt_eligible(PROJECT*, RSC_WORK_FETCH&);

    inline void zero_debt() {
        long_term_debt = 0;
        short_term_debt = 0;
    }

    inline void reset() {
        backoff_time = 0;
        backoff_interval = 0;
        long_term_debt = 0;
        short_term_debt = 0;
        anticipated_debt = 0;
    }

    bool may_have_work;
    bool compute_may_have_work(PROJECT*, int rsc_type);
    void backoff(PROJECT*, const char*);
    void rr_init(PROJECT*, int rsc_type);
    void clear_backoff() {
        backoff_time = 0;
        backoff_interval = 0;
    }
    bool overworked();
};

// estimate the time a resources will be saturated
// with high-priority jobs.
//
struct BUSY_TIME_ESTIMATOR {
    std::vector<double> busy_time;
    int ninstances;
    inline void reset() {
        for (int i=0; i<ninstances; i++) {
            busy_time[i] = 0;
        }
    }
    inline void init(int n) {
        ninstances = n;
        busy_time.resize(n);
        reset();
    }
    // called for each high-priority job.
    // Find the least-busy instance, and put this job
    // on that and following instances
    //
    inline void update(double dur, double nused) {
        int i, j;
        if (nused < 1) return;
        double best = busy_time[0];
        int ibest = 0;
        for (i=1; i<ninstances; i++) {
            if (busy_time[i] < best) {
                best = busy_time[i];
                ibest = i;
            }
        }
        int inused = (int) nused;     // ignore fractional usage
        for (i=0; i<inused; i++) {
            j = (ibest + i) % ninstances;
            busy_time[j] += dur;
        }
    }

    // the overall busy time is the busy time of
    // the least busy instance
    //
    inline double get_busy_time() {
        if (!ninstances) return 0;
        double best = busy_time[0];
        for (int i=1; i<ninstances; i++) {
            if (busy_time[i] < best) {
                best = busy_time[i];
            }
        }
        return best;
    }
};

// per-resource state
//
struct RSC_WORK_FETCH {
    int rsc_type;
    int ninstances;
    double relative_speed;   // total FLOPS relative to CPU total FLOPS

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
    double deadline_missed_instances;
        // instance count for jobs that miss deadline
    std::vector<RESULT*> pending;
    BUSY_TIME_ESTIMATOR busy_time_estimator;

    void init(int t, int n, double sp) {
        rsc_type = t;
        ninstances = n;
        relative_speed = sp;
        busy_time_estimator.init(n);
    }
    // the following specify the work request for this resource
    //
    double req_secs;
    double req_instances;

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
    void update_long_term_debts();
    void update_short_term_debts();
    void print_state(const char*);
    void clear_request();
    void set_request(PROJECT*, bool allow_overworked);
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
    void write_request(FILE*, PROJECT*);
    void handle_reply(
        PROJECT*, SCHEDULER_REPLY*, std::vector<RESULT*>new_results
    );
    void set_initial_work_request();
    void set_all_requests(PROJECT*);
    void print_state();
    void init();
    void rr_init();
    void clear_request();
    void compute_shares();
    void zero_debts();
};

extern RSC_WORK_FETCH cuda_work_fetch;
extern RSC_WORK_FETCH ati_work_fetch;
extern RSC_WORK_FETCH cpu_work_fetch;
extern WORK_FETCH work_fetch;

#endif
