#include <vector>

#include "app.h"
#include "time_stats.h"

using std::vector;

class CLIENT_STATE {
public:
    double now;
    vector<PROJECT*> projects;
    vector<WORKUNIT*> workunits;
    vector<RESULT*> results;
    ACTIVE_TASK_SET active_tasks;
    GLOBAL_PREFS global_prefs;
    HOST_INFO host_info;
    TIME_STATS time_stats;

private:
    double app_started;
public:
    ACTIVE_TASK* lookup_active_task_by_result(RESULT*);
    int report_result_error(RESULT&, const char *format, ...);
    double available_ram();
    double max_available_ram();
    void set_client_state_dirty(const char*);

// cpu_sched.C
private:
    double debt_interval_start;
    double total_wall_cpu_time_this_debt_interval;
        // "wall CPU time" accumulated since last adjust_debts()
    double total_cpu_time_this_debt_interval;
    double cpu_shortfall;
    bool work_fetch_no_new_work;
    bool must_enforce_cpu_schedule;
    bool must_schedule_cpus;
    bool must_check_work_fetch;
    std::vector <RESULT*> ordered_scheduled_results;
    void assign_results_to_projects();
    RESULT* largest_debt_project_best_result();
    RESULT* earliest_deadline_result();
    void adjust_debts();
    bool possibly_schedule_cpus();
    void schedule_cpus();
    bool enforce_schedule();
    bool no_work_for_a_cpu();
    void rr_simulation();
    void make_running_task_heap(vector<ACTIVE_TASK*>&);
    void print_deadline_misses();
    inline double work_buf_min() {
        return global_prefs.work_buf_min_days * 86400;
    }
public:
    double overall_cpu_frac();
    void request_enforce_schedule(const char*);
    void request_schedule_cpus(const char*);

// --------------- cs_apps.C:
private:
    double total_resource_share();
    double potentially_runnable_resource_share();
    double nearly_runnable_resource_share();
public:
    double runnable_resource_share();
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
    int input_files_available(RESULT*, bool);
    ACTIVE_TASK* get_next_graphics_capable_app();
    int ncpus;
        // number of usable cpus
private:
    int nslots;

    int app_finished(ACTIVE_TASK&);
    bool start_apps();
    bool handle_finished_apps();
public:
    ACTIVE_TASK* get_task(RESULT*);
};

extern CLIENT_STATE gstate;
