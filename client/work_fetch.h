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
#include <deque>

extern bool use_hyst_fetch;

#define RSC_TYPE_ANY    -1
#define RSC_TYPE_CPU    0

// reasons for not being able to fetch work
//
#define CANT_FETCH_WORK_NON_CPU_INTENSIVE   1
#define CANT_FETCH_WORK_SUSPENDED_VIA_GUI   2
#define CANT_FETCH_WORK_MASTER_URL_FETCH_PENDING   3
#define CANT_FETCH_WORK_MIN_RPC_TIME        4
#define CANT_FETCH_WORK_DONT_REQUEST_MORE_WORK        5
#define CANT_FETCH_WORK_DOWNLOAD_STALLED    6
#define CANT_FETCH_WORK_RESULT_SUSPENDED    7
#define CANT_FETCH_WORK_TOO_MANY_UPLOADS    8
#define CANT_FETCH_WORK_NOT_HIGHEST_PRIORITY    9
#define CANT_FETCH_WORK_DONT_NEED           10

inline const char* cant_fetch_work_string(int reason) {
    switch (reason) {
    case CANT_FETCH_WORK_NON_CPU_INTENSIVE:
        return "non CPU intensive";
    case CANT_FETCH_WORK_SUSPENDED_VIA_GUI:
        return "suspended via Manager";
    case CANT_FETCH_WORK_MASTER_URL_FETCH_PENDING:
        return "master URL fetch pending";
    case CANT_FETCH_WORK_MIN_RPC_TIME:
        return "scheduler RPC backoff";
    case CANT_FETCH_WORK_DONT_REQUEST_MORE_WORK:
        return "\"no new tasks\" requested via Manager";
    case CANT_FETCH_WORK_DOWNLOAD_STALLED:
        return "some download is stalled";
    case CANT_FETCH_WORK_RESULT_SUSPENDED:
        return "some task is suspended via Manager";
    case CANT_FETCH_WORK_TOO_MANY_UPLOADS:
        return "too many uploads in progress";
    case CANT_FETCH_WORK_NOT_HIGHEST_PRIORITY:
        return "project is not highest priority";
    case CANT_FETCH_WORK_DONT_NEED:
        return "don't need";
    }
    return "";
}

struct PROJECT;
struct RESULT;
struct ACTIVE_TASK;
struct RSC_WORK_FETCH;
struct SCHEDULER_REPLY;
struct APP_VERSION;

// state per (resource, project) pair
//
struct RSC_PROJECT_WORK_FETCH {
    // the following are persistent (saved in state file)
    double backoff_time;
    double backoff_interval;

    // the following used by debt accounting
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
    std::deque<RESULT*> pending;
    std::deque<RESULT*>::iterator pending_iter;

    RSC_PROJECT_WORK_FETCH() {
        backoff_time = 0;
        backoff_interval = 0;
        secs_this_debt_interval = 0;
        queue_est = 0;
        anon_skip = false;
        fetchable_share = 0;
        has_runnable_jobs = false;
        sim_nused = 0;
        nused_total = 0;
        deadlines_missed = 0;
        deadlines_missed_copy = 0;
    }

    inline void reset() {
        backoff_time = 0;
        backoff_interval = 0;
    }

    bool may_have_work;
    bool compute_may_have_work(PROJECT*, int rsc_type);
    void backoff(PROJECT*, const char*);
    void rr_init(PROJECT*, int rsc_type);
    void clear_backoff() {
        backoff_time = 0;
        backoff_interval = 0;
    }
};

// estimate the time a resource will be saturated
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
        if (ninstances==0) return;
        int i, j;
        if (nused < 1) return;
        double best = 0;
        int ibest = 0;
        for (i=0; i<ninstances; i++) {
            if (!i || busy_time[i] < best) {
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
        double best = 0;
        for (int i=0; i<ninstances; i++) {
            if (!i || busy_time[i] < best) {
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
    double saturated_time;
        // estimated time until resource is not saturated
        // used to calculate work request
    double deadline_missed_instances;
        // instance count for jobs that miss deadline
    BUSY_TIME_ESTIMATOR busy_time_estimator;
#ifdef SIM
    double estimated_delay;
#endif

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
    PROJECT* choose_project_hyst(bool enforce_hyst);
    PROJECT* choose_project(int);
    void supplement(PROJECT*);
    RSC_PROJECT_WORK_FETCH& project_state(PROJECT*);
    void print_state(const char*);
    void clear_request();
    void set_request(PROJECT*);
    bool may_have_work(PROJECT*);
    RSC_WORK_FETCH() {
        rsc_type = 0;
        ninstances = 0;
        relative_speed = 0;
        shortfall = 0;
        nidle_now = 0;
        sim_nused = 0;
        total_fetchable_share = 0;
        saturated_time = 0;
        deadline_missed_instances = 0;
    }
};


// per project state
//
struct PROJECT_WORK_FETCH {
    double rec;
        // recent estimated credit
    double rec_time;
        // when it was last updated
    double rec_temp;
        // temporary copy used during schedule_cpus() and work fetch
    double rec_temp_save;
        // temporary used during RR simulation
    int cant_fetch_work_reason;
    int compute_cant_fetch_work_reason(PROJECT*);
    bool has_runnable_jobs;
    PROJECT_WORK_FETCH() {
        memset(this, 0, sizeof(*this));
    }
    void reset(PROJECT*);
};

// global work fetch state
//
struct WORK_FETCH {
    PROJECT* choose_project(bool enforce_hyst);
        // find a project to ask for work
        // if enforce_hystis false,
        // consider requesting work even if buffer is above min level
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
    void set_all_requests_hyst(PROJECT*, int rsc_type);
    void print_state();
    void init();
    void rr_init();
    void clear_request();
    void compute_shares();
    void clear_backoffs(APP_VERSION&);
    void request_string(char*);
};

extern RSC_WORK_FETCH rsc_work_fetch[MAX_RSC];
extern WORK_FETCH work_fetch;

extern void set_no_rsc_config();

extern void project_priority_init(bool for_work_fetch);
extern double project_priority(PROJECT*);
extern void adjust_rec_sched(RESULT*);
extern void adjust_rec_work_fetch(RESULT*);

extern double total_peak_flops();

#endif
