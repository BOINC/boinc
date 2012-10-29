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

#ifndef _CLIENT_STATE_
#define _CLIENT_STATE_

#ifndef _WIN32
#include <string>
#include <vector>
#include <ctime>
#endif

using std::string;
using std::vector;

#include "coproc.h"
#include "util.h"

#include "acct_mgr.h"
#include "acct_setup.h"
#include "app.h"
#include "client_types.h"
#include "current_version.h"
#include "file_xfer.h"
#include "file_names.h"
#include "gui_rpc_server.h"
#include "gui_http.h"
#include "hostinfo.h"
#include "miofile.h"
#include "net_stats.h"
#include "pers_file_xfer.h"
#include "prefs.h"
#include "scheduler_op.h"
#include "time_stats.h"

#ifdef SIM
#include "../sched/edf_sim.h"
#endif

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

// encapsulates the global variables of the core client.
// If you add anything here, initialize it in the constructor
//
struct CLIENT_STATE {
    vector<PLATFORM> platforms;
    vector<PROJECT*> projects;
    vector<APP*> apps;
    vector<FILE_INFO*> file_infos;
    vector<APP_VERSION*> app_versions;
    vector<WORKUNIT*> workunits;
    vector<RESULT*> results;
        // list of jobs, ordered by increasing arrival time

    PERS_FILE_XFER_SET* pers_file_xfers;
    HTTP_OP_SET* http_ops;
    FILE_XFER_SET* file_xfers;
#ifndef SIM
    GUI_RPC_CONN_SET gui_rpcs;
#endif
    GUI_HTTP gui_http;
#ifdef ENABLE_AUTO_UPDATE
    AUTO_UPDATE auto_update;
#endif
    LOOKUP_WEBSITE_OP lookup_website_op;
    GET_CURRENT_VERSION_OP get_current_version_op;
    GET_PROJECT_LIST_OP get_project_list_op;
    ACCT_MGR_OP acct_mgr_op;

    CLIENT_TIME_STATS time_stats;
    GLOBAL_PREFS global_prefs;
    NET_STATS net_stats;
    ACTIVE_TASK_SET active_tasks;
    HOST_INFO host_info;

    VERSION_INFO core_client_version;
    string statefile_platform_name;
    int file_xfer_giveup_period;
    RUN_MODE cpu_run_mode;
    RUN_MODE gpu_run_mode;
    RUN_MODE network_run_mode;
    bool started_by_screensaver;
    bool check_all_logins;
    bool user_active;       // there has been recent mouse/kbd input
    int cmdline_gui_rpc_port;
    bool show_projects;
    bool requested_exit;
        // we should exit now.  Set when
        // - got a "quit" GUI RPC
        // - (Unix) got a HUP, INT, QUIT, TERM, or PWR signal
        // - (Win) got CTRL_LOGOFF, CTRL_C, CTRL_BREAK, etc. event
        // - (Mac) client was started from screensaver,
        //   which has since exited
    bool os_requested_suspend;
        // we should suspend for OS reasonts (used on Win only).
        // Set when
        // - got BATTERY_LOW, SUSPEND, SERVICE_CONTROL_PAUSE
    double os_requested_suspend_time;
    bool cleanup_completed;
    bool in_abort_sequence;
        // Determine when it is safe to leave the quit_client() handler
        // and to finish cleaning up.
    char detach_project_url[256];
        // stores URL for -detach_project option
    char reset_project_url[256];
        // stores URL for -reset_project option
    char update_prefs_url[256];
        // stores URL for -update_prefs option
    char main_host_venue[256];
        // venue from project or AMS that gave us general prefs
    char attach_project_url[256];
    char attach_project_auth[256];
    bool exit_before_upload;
        // exit when about to upload a file
#ifndef _WIN32
    gid_t boinc_project_gid;
#endif

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
        // Don't run apps.
    int suspend_reason;

    bool network_suspended;
        // Don't use network.
    bool file_xfers_suspended;
        // Don't do file xfers (but allow other network activity).
    int network_suspend_reason;

    bool executing_as_daemon;
        // true if --daemon is on the commandline
        // this means we are running as a daemon on unix,
        // or as a service on Windows
    bool redirect_io;
        // redirect stdout, stderr to log files
    bool disable_graphics;
        // a condition has occurred in which we know graphics will
        // not be displayable, so GUIs shouldn't offer graphics.
    bool detach_console;
    bool launched_by_manager;
        // this affects auto-update
    bool run_by_updater;
    double now;
    double last_wakeup_time;
    bool initialized;
    bool cant_write_state_file;
        // failed to write state file.
        // In this case we continue to run for 1 minute,
        // handling GUI RPCs but doing nothing else,
        // so that the Manager can tell the user what the problem is

    bool client_state_dirty;
    int old_major_version;
    int old_minor_version;
    int old_release;
    bool run_cpu_benchmarks;
        // if set, run benchmarks on client startup
    bool cpu_benchmarks_pending;
        // set if a benchmark fails to start because of a job that doesn't exit
        // Persists so that the next start of BOINC runs the benchmarks.

    int exit_after_app_start_secs;
        // if nonzero, exit this many seconds after starting an app
    double app_started;
        // when the most recent app was started

// --------------- acct_mgr.cpp:
    ACCT_MGR_INFO acct_mgr_info;

// --------------- acct_setup.cpp:
    PROJECT_INIT project_init;
    PROJECT_ATTACH project_attach;
    void new_version_check();
    void all_projects_list_check();
    double new_version_check_time;
    double all_projects_list_check_time;
        // the time we last successfully fetched the project list
    string newer_version;

// --------------- client_state.cpp:
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
    APP_VERSION* lookup_app_version(
        APP*, char* platform, int ver, char* plan_class
    );
    int detach_project(PROJECT*);
    int report_result_error(RESULT&, const char *format, ...);
    int reset_project(PROJECT*, bool detaching);
    bool no_gui_rpc;
    void start_abort_sequence();
    bool abort_sequence_done();
    int quit_activities();

    int link_app(PROJECT*, APP*);
    int link_file_info(PROJECT*, FILE_INFO*);
    int link_file_ref(PROJECT*, FILE_REF*);
    int link_app_version(PROJECT*, APP_VERSION*);
    int link_workunit(PROJECT*, WORKUNIT*);
    int link_result(PROJECT*, RESULT*);
    void print_summary();
    bool abort_unstarted_late_jobs();
    bool garbage_collect();
    bool garbage_collect_always();
    bool update_results();
    int nresults_for_project(PROJECT*);
    void check_clock_reset();
    void clear_absolute_times();
    void set_now();
    void log_show_projects();

// --------------- cpu_sched.cpp:
    double total_resource_share();
    double potentially_runnable_resource_share();
    double nearly_runnable_resource_share();
    double fetchable_resource_share();
    double debt_interval_start;
    double total_cpu_time_this_debt_interval;
    bool must_enforce_cpu_schedule;
    bool must_schedule_cpus;
    bool must_check_work_fetch;
    void assign_results_to_projects();
    RESULT* largest_debt_project_best_result();
    void reset_debt_accounting();
    bool schedule_cpus();
    void make_run_list(vector<RESULT*>&);
    bool enforce_run_list(vector<RESULT*>&);
    void append_unfinished_time_slice(vector<RESULT*>&);

    double runnable_resource_share(int);
    void adjust_rec();
    double retry_shmem_time;
        // if we fail to start a task due to no shared-mem segments,
        // wait until at least this time to try running
        // another task that needs a shared-mem seg
    inline double work_buf_min() {
        double x = global_prefs.work_buf_min_days * 86400;
        if (x < 180) x = 180;
        return x;
    }
    inline double work_buf_additional() {
        return global_prefs.work_buf_additional_days *86400;
    }
    inline double work_buf_total() {
        double x = work_buf_min() + work_buf_additional();
        if (x < 1) x = 1;
        return x;
    }

    void request_schedule_cpus(const char*);
        // Reschedule CPUs ASAP.
        // Called when:
        // - core client starts (CS::init())
        // - an app exits (ATS::check_app_exited())
        // - Tasks are killed (ATS::exit_tasks())
        // - a result's input files finish downloading (CS::update_results())
        // - an app fails to start (CS::schedule_cpus())
        // - any project op is done via RPC (suspend/resume)
        // - any result op is done via RPC (suspend/resume)
    void set_ncpus();

// --------------- cs_account.cpp:
    int add_project(
        const char* master_url, const char* authenticator,
        const char* project_name, bool attached_via_acct_mgr
    );

    int parse_account_files();
    int parse_account_files_venue();
    int parse_preferences_for_user_files();
    int parse_statistics_files();
        // should be move to a new file, but this will do it for testing

// --------------- cs_apps.cpp:
    double get_fraction_done(RESULT* result);
    int input_files_available(RESULT*, bool, FILE_INFO** f=0);
    ACTIVE_TASK* lookup_active_task_by_result(RESULT*);
    int ncpus;
        // Act like there are this many CPUs.
        // By default this is the # of physical CPUs,
        // but it can be changed in two ways:
        // - type <ncpus>N</ncpus> in the config file
        // - type the max_ncpus_pct pref

    int latest_version(APP*, char*);
    int app_finished(ACTIVE_TASK&);
    bool start_apps();
    bool handle_finished_apps();

    ACTIVE_TASK* get_task(RESULT*);

// --------------- cs_benchmark.cpp:
    bool should_run_cpu_benchmarks();
    void start_cpu_benchmarks();
    bool cpu_benchmarks_poll();
    void abort_cpu_benchmarks();
    bool are_cpu_benchmarks_running();
    bool cpu_benchmarks_done();
    void cpu_benchmarks_set_defaults();
    void print_benchmark_results();

// --------------- cs_cmdline.cpp:
    void parse_cmdline(int argc, char** argv);
    void parse_env_vars();
    void do_cmdline_actions();

// --------------- cs_files.cpp:
    void check_file_existence();
    bool start_new_file_xfer(PERS_FILE_XFER&);

    int make_project_dirs();
    bool create_and_delete_pers_file_xfers();

// --------------- cs_platforms.cpp:
    const char* get_primary_platform();
    void add_platform(const char*);
    void detect_platforms();
    void write_platforms(PROJECT*, MIOFILE&);
    bool is_supported_platform(const char*);

// --------------- cs_prefs.cpp:
    double client_disk_usage;
        // disk usage not counting projects
        // computed by get_disk_usages()
    double total_disk_usage;
        // client plus projects
    int get_disk_usages();
    void get_disk_shares();
    double allowed_disk_usage(double boinc_total);
    int allowed_project_disk_usage(double&);
    int suspend_tasks(int reason);
    int resume_tasks(int reason=0);
    void read_global_prefs(
        const char* fname = GLOBAL_PREFS_FILE_NAME,
        const char* override_fname = GLOBAL_PREFS_OVERRIDE_FILE
    );
    int save_global_prefs(char* prefs, char* url, char* sched);
    double available_ram();
    double max_available_ram();
    int check_suspend_processing();
    void check_suspend_network();
    void install_global_prefs();
    PROJECT* global_prefs_source_project();
    void show_global_prefs_source(bool);

// --------------- cs_scheduler.cpp:
    void request_work_fetch(const char*);
        // Called when:
        // - core client starts (CS::init())
        // - task is completed or fails
        // - tasks are killed
        // - an RPC completes
        // - project suspend/detch/attach/reset GUI RPC
        // - result suspend/abort GUI RPC
    int make_scheduler_request(PROJECT*);
    int handle_scheduler_reply(PROJECT*, char* scheduler_url);
    SCHEDULER_OP* scheduler_op;
    PROJECT* next_project_master_pending();
    PROJECT* next_project_sched_rpc_pending();
    PROJECT* next_project_trickle_up_pending();
    PROJECT* find_project_with_overdue_results(bool network_suspend_soon);
    bool had_or_requested_work;
    bool scheduler_rpc_poll();

// --------------- cs_statefile.cpp:
    void set_client_state_dirty(const char*);
    int parse_state_file();
    int parse_state_file_aux(const char*);
    int write_state(MIOFILE&);
    int write_state_file();
    int write_state_file_if_needed();
    void check_anonymous();
    int parse_app_info(PROJECT*, FILE*);
    int write_state_gui(MIOFILE&);
    int write_file_transfers_gui(MIOFILE&);
    int write_tasks_gui(MIOFILE&, bool);
    void sort_results();

// --------------- cs_trickle.cpp:
    int read_trickle_files(PROJECT*, FILE*);
    int remove_trickle_files(PROJECT*);
    int handle_trickle_down(PROJECT*, FILE*);

// --------------- check_state.cpp:
// stuff related to data-structure integrity checking
//
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
    void free_mem();

// --------------- work_fetch.cpp:
    int proj_min_results(PROJECT*, double);
    void check_project_timeout();
    double overall_cpu_frac();
    double overall_cpu_and_network_frac();
    double overall_gpu_frac();
    double time_until_work_done(PROJECT*, int, double);
    bool compute_work_requests();
    void scale_duration_correction_factors(double);
    void generate_new_host_cpid();
    void compute_nuploading_results();

#ifdef SIM
    double share_violation();
    double monotony();

    void handle_completed_results(PROJECT*);
    void get_workload(vector<IP_RESULT>&);
    bool simulate_rpc(PROJECT*);
#endif
};

extern CLIENT_STATE gstate;

extern bool gpus_usable;
    // set to false if GPUs not usable because of remote desktop
    // or login situation (Windows)

// return a random double in the range [MIN,min(e^n,MAX))

extern double calculate_exponential_backoff(
    int n, double MIN, double MAX
);

extern void print_suspend_tasks_message(int);


//////// TIME-RELATED CONSTANTS ////////////

//////// CLIENT INTERNAL

#define POLL_INTERVAL   1.0
    // the client will handle I/O (including GUI RPCs)
    // for up to POLL_INTERVAL seconds before calling poll_slow_events()
    // to call the polling functions

#define GARBAGE_COLLECT_PERIOD  10
    // how often to garbage collect

#define TASK_POLL_PERIOD    1.0

#define UPDATE_RESULTS_PERIOD   1.0

#define HANDLE_FINISHED_APPS_PERIOD 1.0

#define BENCHMARK_POLL_PERIOD   1.0

#define PERS_FILE_XFER_START_PERIOD  1.0
#define PERS_FILE_XFER_POLL_PERIOD  1.0

#define SCHEDULER_RPC_POLL_PERIOD   5.0

#define FILE_XFER_POLL_PERIOD   1.0

#define GUI_HTTP_POLL_PERIOD    1.0

#define MEMORY_USAGE_PERIOD     10

//////// WORK FETCH

#define WORK_FETCH_PERIOD   60
    // see if we need to fetch work at least this often
#define WF_MIN_BACKOFF_INTERVAL    600
#define WF_MAX_BACKOFF_INTERVAL    86400
    // if we ask a project for work for a resource and don't get it,
    // we do exponential backoff.
    // This constant is an upper bound for this.
    // E.g., if we need GPU work, we'll end up asking once a day,
    // so if the project develops a GPU app,
    // we'll find out about it within a day.

#define WF_DEFER_INTERVAL   300
    // if a project is uploading, and the last upload started within this interval,
    // don't fetch work from it.
    // This allows the work fetch to be merged with the reporting of the
    // jobs that are currently uploading.

//////// CPU SCHEDULING

#define CPU_SCHED_PERIOD    60
    // do CPU schedule at least this often

#define DEBT_ADJUST_PERIOD CPU_SCHED_PERIOD
    // debt is adjusted at least this often,
    // since adjust_debts() is called from enforce_schedule()

#define DEADLINE_CUSHION    0
    // try to finish jobs this much in advance of their deadline

#define MAX_EXIT_TIME   15
    // if an app takes this long to exit, kill it

#define MAX_STARTUP_TIME    10
    // if app startup takes longer than this, quit loop

//////// NETWORK

#define CONNECT_ERROR_PERIOD    600.0

#define ALLOW_NETWORK_IF_RECENT_RPC_PERIOD  300
    // if there has been a GUI RPC within this period
    // that requires network access (e.g. attach to project)
    // allow it even if setting is "no access"

//////// MISC

#define EXCLUSIVE_APP_WAIT   30
    // if "exclusive app" feature used,
    // wait this long after app exits before restarting jobs

#define DAILY_XFER_HISTORY_PERIOD   60

#define MAX_STD   (86400)
    // maximum short-term debt

#define ACCT_MGR_MIN_BACKOFF    600
#define ACCT_MGR_MAX_BACKOFF    86400
    // min/max account manager RPC backoff

#endif
