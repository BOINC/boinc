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
    double instances_used;
    //double shortfall;
    //double nidle_now;

    RSC_PROJECT_WORK_FETCH() {
        memset(this, 0, sizeof(*this));
    }

    // whether this project is accumulating debt for this resource
    bool debt_eligible(PROJECT*);
    bool fetchable(PROJECT*);
    inline void clear_perm() {
        backoff_time = 0;
        backoff_interval = 0;
        debt = 0;
    }
    //void accumulate_shortfall(RSC_WORK_FETCH&, PROJECT*, double dt, double nused);
    bool overworked();
	void backoff(PROJECT*, char*);
	void clear_backoff();
    void rr_init();
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
    double nidle_now;
    double total_fetchable_share;
        // total RS of projects from which we could fetch jobs for this device
    double total_runnable_share;
        // total RS of projects with runnable jobs for this device

    // the following specify the work request for this resource
    //
    double req_secs;
    int req_instances;

    // debt accounting
    double secs_this_debt_interval;
    inline void reset_debt_accounting() {
        secs_this_debt_interval = 0;
    }
    void normalize_debt();

    void rr_init();
    void accumulate_shortfall(double d_time, double nused=0);
    PROJECT* choose_project();
    void accumulate_debt();
    RSC_PROJECT_WORK_FETCH& project_state(PROJECT*);
    bool may_have_work(PROJECT*);
    void update_debts();
    void print_state(char*);
    void clear_request();
    void set_request(PROJECT*);
    RSC_WORK_FETCH() {
        memset(this, 0, sizeof(*this));
    }
};


// per project state
//
struct PROJECT_WORK_FETCH {
    double overall_debt;
    PROJECT_WORK_FETCH() {
        memset(this, 0, sizeof(*this));
    }
};

// global work fetch state
//
struct WORK_FETCH {
    double estimated_delay;
    void set_overall_debts();
    PROJECT* choose_project();
        // find a project to ask for work
    void compute_work_request(PROJECT*);
        // we're going to contact this project anyway;
        // decide how much work to task for
    void accumulate_inst_sec(ACTIVE_TASK*, double dt);
    void write_request(PROJECT*, FILE*);
    void handle_reply(PROJECT*, std::vector<RESULT*>new_results);
    void set_initial_work_request(PROJECT* p);
    void print_state();
    void init();
    void rr_init();
    void clear_request();
    void compute_shares();
};

extern RSC_WORK_FETCH cuda_work_fetch;
extern RSC_WORK_FETCH cpu_work_fetch;
extern WORK_FETCH work_fetch;

#endif
