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
// 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

#ifndef _CLIENT_STATE_
#define _CLIENT_STATE_

//#define NEW_CPU_SCHED

#ifndef _WIN32
#include <vector>
#include <ctime>
#endif

#include "acct_mgr.h"
#include "acct_setup.h"
#include "app.h"
#include "client_types.h"
#include "file_xfer.h"
#include "gui_rpc_server.h"
#include "gui_http.h"
#include "hostinfo.h"
#include "language.h"
#include "miofile.h"
#include "net_stats.h"
#include "pers_file_xfer.h"
#include "prefs.h"
#include "scheduler_op.h"
#include "ss_logic.h"
#include "time_stats.h"
#include "http_curl.h"
#include "net_xfer_curl.h"

#define USER_RUN_REQUEST_ALWAYS     1
#define USER_RUN_REQUEST_AUTO       2
#define USER_RUN_REQUEST_NEVER      3

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

enum SUSPEND_REASON {
    SUSPEND_REASON_BATTERIES = 1,
    SUSPEND_REASON_USER_ACTIVE = 2,
    SUSPEND_REASON_USER_REQ = 4,
    SUSPEND_REASON_TIME_OF_DAY = 8,
    SUSPEND_REASON_BENCHMARKS = 16,
    SUSPEND_REASON_DISK_SIZE = 32
};

#ifdef NEW_CPU_SCHED
struct CPU_SCHEDULER {
    void do_accounting();
    void make_schedule();
    void enforce();
    void clear_tasks();
};
#endif

// CLIENT_STATE encapsulates the global variables of the core client.
// If you add anything here, initialize it in the constructor
//
class CLIENT_STATE {
public:
    std::vector<PROJECT*> projects;
    std::vector<APP*> apps;
    std::vector<FILE_INFO*> file_infos;
    std::vector<APP_VERSION*> app_versions;
    std::vector<WORKUNIT*> workunits;
    std::vector<RESULT*> results;
#ifdef NEW_CPU_SCHED
    CPU_SCHEDULER cpu_scheduler;
#else
    std::vector<CPU> cpus;
#endif

    NET_XFER_SET* net_xfers;
    PERS_FILE_XFER_SET* pers_file_xfers;
    HTTP_OP_SET* http_ops;
    FILE_XFER_SET* file_xfers;
    ACTIVE_TASK_SET active_tasks;
    HOST_INFO host_info;
    GLOBAL_PREFS global_prefs;
    NET_STATS net_stats;
    SS_LOGIC ss_logic;
    GUI_RPC_CONN_SET gui_rpcs;
    LANGUAGE language;
    TIME_STATS time_stats;
    PROXY_INFO proxy_info;
    GUI_HTTP gui_http;

    int core_client_major_version;
    int core_client_minor_version;
    int core_client_release;
    int file_xfer_giveup_period;
    int user_run_request;
        // values above (USER_RUN_REQUEST_*)
    int user_network_request;
        // same, just for network
    bool started_by_screensaver;
    bool exit_when_idle;
    bool check_all_logins;
    bool return_results_immediately;
    bool allow_remote_gui_rpc;
    int cmdline_gui_rpc_port;
    bool show_projects;
    bool requested_exit;
    char detach_project_url[256];
        // stores URL for -detach_project option
    char reset_project_url[256];
        // stores URL for -reset_project option
    char update_prefs_url[256];
        // stores URL for -update_prefs option
    char main_host_venue[256];
        // venue from project that gave us general prefs
    char attach_project_url[256];
    char attach_project_auth[256];
    bool exit_before_upload;
        // exit when about to upload a file

    // backoff-related variables
    //
    int master_fetch_period;
        // fetch project's master URL (and stop doing scheduler RPCs)
        // if get this many successive RPC failures (default 10)
    int retry_cap;
        // cap project->nrpc_failures at this number
    int master_fetch_retry_cap;
        // after this many master-fetch failures,
        // move into a state in which we retry master fetch
        // at the frequency below
    int master_fetch_interval;
        // see above

    int sched_retry_delay_min;
    int sched_retry_delay_max;
    int pers_retry_delay_min;
    int pers_retry_delay_max;
    int pers_giveup;

    bool tasks_suspended;
        // Don't do CPU.  See check_suspend_activities for logic
    bool network_suspended;
        // Don't do network.  See check_suspend_network for logic
    double network_last_unsuspended;
	bool executing_as_daemon;
        // true if --daemon is on the commandline
        // this means we are running as a daemon on unix,
        // or as a service on Windows
    bool redirect_io;
        // redirect stdout, stderr to log files
    bool detach_console;
    double now;
    const char* platform_name;

private:
    bool client_state_dirty;
    int old_major_version;
    int old_minor_version;
    int old_release;
    bool skip_cpu_benchmarks;
        // if set, use hardwired numbers rather than running benchmarks
    bool run_cpu_benchmarks;
        // if set, run benchmarks on client startup
    bool cpu_benchmarks_pending;
        // set if a benchmark fails to start because of a process that doesn't stop.
        // Persists so that the next start of BOINC runs the benchmarks.

    int exit_after_app_start_secs;
        // if nonzero, exit this many seconds after starting an app
    double app_started;
        // when the most recent app was started

    // CPU sched state
    //
    double cpu_sched_last_time;
    double total_wall_cpu_time_this_period;
        // "wall CPU time" accumulated since last schedule_cpus()
    double total_cpu_time_this_period;
	bool work_fetch_no_new_work;
	bool cpu_earliest_deadline_first;
    long rr_last_results_fail_count;
    long rr_results_fail_count;

// --------------- acct_mgr.C:
public:
    ACCT_MGR_OP acct_mgr_op;
    ACCT_MGR_INFO acct_mgr_info;

// --------------- acct_setup.C:
public:
    PROJECT_INIT project_init;
    GET_PROJECT_CONFIG_OP get_project_config_op;
    LOOKUP_ACCOUNT_OP lookup_account_op;
    CREATE_ACCOUNT_OP create_account_op;
    LOOKUP_WEBSITE_OP lookup_website_op;
    PROJECT_ATTACH project_attach;
    GET_CURRENT_VERSION_OP get_current_version_op;
    void new_version_check();
    double new_version_check_time;
    std::string newer_version;

// --------------- client_state.C:
public:
    CLIENT_STATE();
    void show_host_info();
    int init();
    bool poll_slow_events();
        // Never blocks.
        // Returns true if it actually did something,
        // in which case it should be called again immediately.
    void do_io_or_sleep(double dt);
    bool time_to_exit();
    PROJECT* lookup_project(const char*);
    APP* lookup_app(PROJECT*, const char*);
    FILE_INFO* lookup_file_info(PROJECT*, const char* name);
    RESULT* lookup_result(PROJECT*, const char*);
    WORKUNIT* lookup_workunit(PROJECT*, const char*);
    APP_VERSION* lookup_app_version(APP*, int);
    ACTIVE_TASK* lookup_active_task_by_result(RESULT*);
    int detach_project(PROJECT*);
    int report_result_error(RESULT&, const char *format, ...);
    int reset_project(PROJECT*);
    bool want_network_flag;
    bool have_sporadic_connection;
    bool want_network();
    void network_available();
    bool no_gui_rpc;
    bool network_id_intermittent() const;
private:
    int link_app(PROJECT*, APP*);
    int link_file_info(PROJECT*, FILE_INFO*);
    int link_file_ref(PROJECT*, FILE_REF*);
    int link_app_version(PROJECT*, APP_VERSION*);
    int link_workunit(PROJECT*, WORKUNIT*);
    int link_result(PROJECT*, RESULT*);
    void print_summary();
    bool garbage_collect();
    bool garbage_collect_always();
    bool update_results();
    void free_mem();

// --------------- cs_account.C:
public:
    bool have_tentative_project;
    int add_project(
        const char* master_url, const char* authenticator,
        bool attached_via_acct_mgr=false
    );
private:
    int parse_account_files();
    int parse_preferences_for_user_files();
    int parse_statistics_files();
        // should be move to a new file, but this will do it for testing

// --------------- cs_apps.C:
private:
    void adjust_debts();
    bool must_schedule_cpus;
    double total_resource_share();
    double potentially_runnable_resource_share();
public:
    double runnable_resource_share();
    void request_schedule_cpus(const char*);
        // Reschedule CPUs ASAP.  Called when:
        // - core client starts (CS::init())
        // - an app exits (ATS::check_app_exited())
        // - Tasks are killed (ATS::exit_tasks())
        // - a result's input files finish downloading (CS::update_results())
        // - an app fails to start (CS::schedule_cpus())
        // - any project op is done via RPC (suspend/resume)
        // - any result op is done via RPC (suspend/resume)
    int restart_tasks();
    int quit_activities();
    void set_ncpus();
    double estimate_cpu_time(WORKUNIT&);
    double get_fraction_done(RESULT* result);
    bool input_files_available(RESULT*);
    ACTIVE_TASK* get_next_graphics_capable_app();
    int ncpus;
private:
    int nslots;

    int choose_version_num(char*, SCHEDULER_REPLY&);
    int app_finished(ACTIVE_TASK&);
#ifndef NEW_CPU_SCHED
    void assign_results_to_projects();
    bool schedule_largest_debt_project(double expected_pay_off, int cpu_index);
    bool schedule_earliest_deadline_result(int cpu_index);
    bool schedule_cpus();
#endif
    bool start_apps();
    bool handle_finished_apps();
    void handle_file_xfer_apps();
public:
    int schedule_result(RESULT*);

// --------------- cs_benchmark.C:
public:
    bool should_run_cpu_benchmarks();
	void start_cpu_benchmarks();
    bool cpu_benchmarks_poll();
    void abort_cpu_benchmarks();
    bool are_cpu_benchmarks_running();

// --------------- cs_cmdline.C:
public:
    void parse_cmdline(int argc, char** argv);
    void parse_env_vars();
    void do_cmdline_actions();

// --------------- cs_files.C:
public:
    bool start_new_file_xfer(PERS_FILE_XFER&);
private:
    int make_project_dirs();
    bool handle_pers_file_xfers();

// --------------- cs_prefs.C:
public:
    int project_disk_usage(PROJECT*, double&);
    int total_disk_usage(double&);
        // returns the total disk usage of BOINC on this host
    int allowed_disk_usage(double&);
    int allowed_project_disk_usage(double&);
private:
    void check_suspend_activities(int&);
    int suspend_tasks(int reason);
    int resume_tasks();
    void check_suspend_network(int&);
    int suspend_network(int reason);
    int resume_network();
    void install_global_prefs();
    PROJECT* global_prefs_source_project();
    void show_global_prefs_source(bool);

// --------------- cs_scheduler.C:
public:
    double work_needed_secs();
    void force_reschedule_all_cpus();
    PROJECT* next_project_master_pending();
    PROJECT* next_project_need_work();
    int make_scheduler_request(PROJECT*);
    int handle_scheduler_reply(PROJECT*, char* scheduler_url, int& nresults);
    int compute_work_requests();
    bool network_is_intermittent() const;
    SCHEDULER_OP* scheduler_op;
    void scale_duration_correction_factors(double);
private:
    bool contacted_sched_server;
    int overall_work_fetch_urgency;

    PROJECT* find_project_with_overdue_results();
    PROJECT* next_project_sched_rpc_pending();
    PROJECT* next_project_trickle_up_pending();
    bool scheduler_rpc_poll();
    double time_until_work_done(PROJECT*, int, double);
    double avg_proc_rate();
    bool should_get_work();
    bool no_work_for_a_cpu();
    int proj_min_results(PROJECT*, double);
    bool round_robin_misses_deadline(double, double);
    bool rr_misses_deadline(double, double, bool);
    bool edf_misses_deadline(double);
    void set_scheduler_modes();
    void generate_new_host_cpid();

// --------------- cs_statefile.C:
public:
    void set_client_state_dirty(const char*);
    int parse_state_file();
    int write_state(MIOFILE&);
    int write_state_file();
    int write_state_file_if_needed();
	void check_anonymous();
	int parse_app_info(PROJECT*, FILE*);
    int write_state_gui(MIOFILE&);
    int write_file_transfers_gui(MIOFILE&);
    int write_tasks_gui(MIOFILE&);

// --------------- cs_trickle.C:
private:
    int read_trickle_files(PROJECT*, FILE*);
    int remove_trickle_files(PROJECT*);
public:
    int handle_trickle_down(PROJECT*, FILE*);

// --------------- check_state.C:
// stuff related to data-structure integrity checking
//
public:
    void check_project_pointer(PROJECT*);
    void check_app_pointer(APP*);
    void check_file_info_pointer(FILE_INFO*);
    void check_app_version_pointer(APP_VERSION*);
    void check_workunit_pointer(WORKUNIT*);
    void check_result_pointer(RESULT*);
    void check_pers_file_xfer_pointer(PERS_FILE_XFER*);
    void check_file_xfer_pointer(FILE_XFER*);

    void check_app(APP&);
    void check_file_info(FILE_INFO&);
    void check_file_ref(FILE_REF&);
    void check_app_version(APP_VERSION&);
    void check_workunit(WORKUNIT&);
    void check_result(RESULT&);
    void check_active_task(ACTIVE_TASK&);
    void check_pers_file_xfer(PERS_FILE_XFER&);
    void check_file_xfer(FILE_XFER&);

    void check_all();

#if 0
// ------------------ cs_data.C:
// mechanisms for managing data saved on host
//
public:
    bool get_more_disk_space(PROJECT*, double);
    int anything_free(double &);
    int calc_proj_size(PROJECT*);
    int calc_all_proj_size();
    int compute_share_disk_size(PROJECT*);
    int total_potential_offender(PROJECT*, double &);
    int total_potential_self(PROJECT*, double &);
    double select_delete(PROJECT*, double, int);
    double delete_results(PROJECT*, double);
    double compute_resource_share(PROJECT*);
    PROJECT* greatest_offender();
private:
    bool data_manager_poll();
    bool fix_data_overflow(double, double);
    int reset_checks();
    int delete_inactive_results(PROJECT*);
    int unstick_result_files(RESULT*);
    double delete_next_file(PROJECT*, int);
    double delete_expired(PROJECT*);
    double offender(PROJECT*);
    double proj_potentially_free(PROJECT*);
    FILE_INFO* get_priority_or_lru(PROJECT*, int);
#endif

};

extern CLIENT_STATE gstate;

// return a random double in the range [MIN,min(e^n,MAX))
//
extern double calculate_exponential_backoff(
    int n, double MIN, double MAX
);

#endif
