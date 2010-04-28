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
//
#include <vector>

#include "app.h"
#include "time_stats.h"
#include "client_types.h"
#include "../sched/edf_sim.h"
#include "rr_sim.h"

using std::vector;

#define WORK_FETCH_DONT_NEED 0
    // project: suspended, deferred, or no new work (can't ask for more work)
    // overall: not work_fetch_ok (from CPU policy)
#define WORK_FETCH_OK        1
    // project: has more than min queue * share, not suspended/def/nonewwork
    // overall: at least min queue, work fetch OK
#define WORK_FETCH_NEED      2
    // project: less than min queue * resource share of DL/runnable results
    // overall: less than min queue
#define WORK_FETCH_NEED_IMMEDIATELY 3
    // project: no downloading or runnable results
    // overall: at least one idle CPU

struct SIM_RESULTS {
    double cpu_used;
    double cpu_wasted;
    double cpu_idle;
    int nresults_met_deadline;
    int nresults_missed_deadline;
    double share_violation;
    double monotony;
    double cpu_wasted_frac;
    double cpu_idle_frac;

    void compute();
    void print(FILE* f, const char* title=0);
    void parse(FILE* f);
    void add(SIM_RESULTS& r);
    void divide(int);
    void clear();
};

struct PROJECT_RESULTS {
    double cpu_used;
    double cpu_wasted;
    int nresults_met_deadline;
    int nresults_missed_deadline;
};

struct NORMAL_DIST {
    double mean;
    double stdev;
    int parse(XML_PARSER&, char* end_tag);
    double sample();
};

struct UNIFORM_DIST {
    double lo;
    double hi;
    int parse(XML_PARSER&, char* end_tag);
    double sample();
};

class RANDOM_PROCESS {
    double last_time;
    double time_left;
    bool value;
    double off_lambda;
public:
    double frac;
    double lambda;
    int parse(XML_PARSER&, char* end_tag);
    bool sample(double);
    void init(double);
    RANDOM_PROCESS();
};

struct SIM_APP: public APP {
    double latency_bound;
    double fpops_est;
    NORMAL_DIST fpops;
    NORMAL_DIST checkpoint_period;
    double working_set;
    double weight;

    SIM_APP(){}
    int parse(XML_PARSER&);
};

struct SIM_PROJECT: public PROJECT {
    RANDOM_PROCESS available;
    int index;
    int result_index;
    double idle_time;
    double idle_time_sumsq;
    bool idle;
    int max_infeasible_count;
    // for DCF variants:
    int completed_task_count;
    double completions_ratio_mean;
    double completions_ratio_s;
    double completions_ratio_stdev;
    double completions_required_stdevs;

    int parse(XML_PARSER&);
    PROJECT_RESULTS project_results;
    void print_results(FILE*, SIM_RESULTS&);
    void init();
    void backoff();
    void update_dcf_stats(RESULT*);
};

struct SIM_GPU : public COPROC {
    double flops;
    int parse(XML_PARSER&, const char*);
};

struct SIM_HOST: public HOST_INFO {
    RANDOM_PROCESS available;
    RANDOM_PROCESS idle;
    double connection_interval;
        // min time between network connections
    int parse(XML_PARSER&);
};

class CLIENT_STATE {
public:
    double now;
    vector<PROJECT*> projects;
    vector<WORKUNIT*> workunits;
    vector<RESULT*> results;
    vector<APP_VERSION*> app_versions;
    vector<APP*> apps;
    ACTIVE_TASK_SET active_tasks;
    GLOBAL_PREFS global_prefs;
    SIM_HOST host_info;
    TIME_STATS time_stats;
    COPROCS coprocs;
    CLIENT_STATE();
    bool initialized;
    bool run_cpu_benchmarks;
    FILE* html_out;
    void html_start(bool);
    void html_rec();
    void html_end(bool);
    std::string html_msg;
    double share_violation();
    double monotony();
    bool user_active;
    bool skip_cpu_benchmarks;
    bool unsigned_apps_ok;
    bool exit_after_finish;

private:
    double app_started;
public:
    ACTIVE_TASK* lookup_active_task_by_result(RESULT*);
    int report_result_error(RESULT&, const char *format, ...);
    double available_ram();
    double max_available_ram();
    void set_client_state_dirty(const char*);
    RESULT* lookup_result(PROJECT*, const char*);

// cpu_sched.C
private:
    double debt_interval_start;
    double total_wall_cpu_time_this_debt_interval;
        // "wall CPU time" accumulated since last adjust_debts()
    double fetchable_resource_share();
    double total_cpu_time_this_debt_interval;
    double cpu_shortfall;
    bool work_fetch_no_new_work;
    bool must_enforce_cpu_schedule;
    bool must_schedule_cpus;
    bool must_check_work_fetch;
    void assign_results_to_projects();
    RESULT* largest_debt_project_best_result();
    RESULT* earliest_deadline_result(bool);
    void reset_debt_accounting();
    bool possibly_schedule_cpus();
    void schedule_cpus();
    bool enforce_schedule();
    bool no_work_for_a_cpu();
    void append_unfinished_time_slice(vector<RESULT*> &runnable_jobs);
    void print_deadline_misses();
public:
    void adjust_debts();
    void rr_simulation();
    std::vector <RESULT*> ordered_scheduled_results;
    double retry_shmem_time;
    inline double work_buf_min() {
        return global_prefs.work_buf_min_days * 86400;
    }
    double work_buf_additional() {
        return global_prefs.work_buf_additional_days * 86400;
    }
    inline double work_buf_total() {
        double x = work_buf_min() + work_buf_additional();
        if (x < 1) x = 1;
        return x;
    }

    void request_enforce_schedule(PROJECT*, const char*);
    void request_schedule_cpus(const char*);
    bool sufficient_coprocs(APP_VERSION&);
    void reserve_coprocs(APP_VERSION&);
    void free_coprocs(APP_VERSION&);

// --------------- cs_apps.C:
private:
    double total_resource_share();
    double potentially_runnable_resource_share();
    double nearly_runnable_resource_share();
public:
    double runnable_resource_share(int);
    void request_work_fetch(const char*);
        // Check if work fetch needed.  Called when:
        // - core client starts (CS::init())
        // - task is completed or fails
        // - tasks are killed
        // - an RPC completes
        // - project suspend/detch/attach/reset GUI RPC
        // - result suspend/abort GUI RPC
    int quit_activities();
    void set_ncpus();
    double estimate_cpu_time(WORKUNIT&);
    double get_fraction_done(RESULT* result);
    int input_files_available(RESULT*, bool, FILE_INFO** f=0);
    int ncpus;
        // number of usable cpus
private:
    int nslots;

    int app_finished(ACTIVE_TASK&);
    bool start_apps();
    bool handle_finished_apps();
public:
    ACTIVE_TASK* get_task(RESULT*);

// --------------- cs_scheduler.C
private:
    bool contacted_sched_server;
    int overall_work_fetch_urgency;
    double avg_proc_rate();

// --------------- work_fetch.C:
public:
    int proj_min_results(PROJECT*, double);
	void check_project_timeout();
    PROJECT* next_project_master_pending();
    PROJECT* next_project_sched_rpc_pending();
    PROJECT* next_project_trickle_up_pending();
    PROJECT* next_project_need_work();
    PROJECT* find_project_with_overdue_results();
	double overall_cpu_frac();
    double time_until_work_done(PROJECT*, int, double);
    bool compute_work_requests();
    void scale_duration_correction_factors(double);
    void generate_new_host_cpid();
    void compute_nuploading_results();

//////////////////
    void make_job(SIM_PROJECT*, WORKUNIT*, RESULT*);
    void handle_completed_results();
    void get_workload(vector<IP_RESULT>&);
    int parse_projects(char*);
    int parse_host(char*);
    void simulate();
    bool scheduler_rpc_poll();
    bool simulate_rpc(PROJECT*);
    void print_project_results(FILE*);
    bool in_abort_sequence;
};

class NET_STATUS {
public:
    bool have_sporadic_connection;
};

extern CLIENT_STATE gstate;
extern COPROC_CUDA* coproc_cuda;
extern COPROC_ATI* coproc_ati;
extern NET_STATUS net_status;
extern FILE* logfile;
extern bool user_active;
extern SIM_RESULTS sim_results;
extern double calculate_exponential_backoff(
    int n, double MIN, double MAX
);

extern bool dcf_dont_use;
extern bool dcf_stats;
extern bool cpu_sched_rr_only;
extern bool dual_dcf;
extern bool work_fetch_old;
extern bool gpus_usable;

#define SCHEDULER_RPC_POLL_PERIOD   5.0

#define WORK_FETCH_PERIOD   60

#define CPU_SCHED_ENFORCE_PERIOD    60
    // enforce CPU schedule at least this often

#define DEBT_ADJUST_PERIOD CPU_SCHED_ENFORCE_PERIOD
    // debt is adjusted at least this often,
    // since adjust_debts() is called from enforce_schedule()
#define HANDLE_FINISHED_APPS_PERIOD 1.0

#define MAX_STD   (86400)
    // maximum short-term debt

#define START_TIME  946684800
    // Jan 1 2000
