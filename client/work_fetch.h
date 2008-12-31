#ifndef _WORK_FETCH_
#define _WORK_FETCH_

#include <vector>

#define RSC_TYPE_CPU    0
#define RSC_TYPE_CUDA   1

class PROJECT;
struct RESULT;
class ACTIVE_TASK;
struct RSC_WORK_FETCH;

// per (resource, project) state
//
struct RSC_PROJECT_WORK_FETCH {
    // the following are permanent (saved in state file)
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
    double share;
    double instances_used;
    double shortfall;
    double nidle_now;

    RSC_PROJECT_WORK_FETCH() {
        memset(this, 0, sizeof(*this));
    }

    // whether this project is accumulating debt for this resource
    bool debt_eligible(PROJECT* p);
    inline void clear_perm() {
        backoff_time = 0;
        backoff_interval = 0;
        debt = 0;
    }
    void accumulate_shortfall(RSC_WORK_FETCH&, PROJECT*, double dt, double nused);
    bool overworked();
	void backoff();
	void clear_backoff();
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
    double total_resource_share;
        // total RS of projects debt-eligible for this device
        // determines share
    double runnable_resource_share;
        // total RS of projects with runnable jobs for this device
        // determines estimated processing rate for CPU

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
    void accumulate_inst_sec(ACTIVE_TASK*, double dt);
    void write_request(PROJECT*, FILE*);
    void handle_reply(PROJECT*, std::vector<RESULT*>new_results);
    void set_initial_work_request(PROJECT* p);
    void print_state();
    void init();
    void rr_init();
};

extern RSC_WORK_FETCH cuda_work_fetch;
extern RSC_WORK_FETCH cpu_work_fetch;
extern WORK_FETCH work_fetch;

#endif
