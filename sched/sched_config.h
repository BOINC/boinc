// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2023 University of California
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

// parsed version of config.xml

#ifndef BOINC_SCHED_CONFIG_H
#define BOINC_SCHED_CONFIG_H

#include <regex.h>
#include <vector>
#include <cstdio>

#include "sched_limit.h"

using std::vector;

// constants related to consecutive_valid.
// These could be made configurable.

#define CONS_VALID_RELIABLE 10
    // host is eligible to be considered "reliable"
#define CONS_VALID_HOST_SCALE 10
    // host is eligible for host scaling of credit
#define CONS_VALID_UNREPLICATED 10
    // host is eligible for single replication

// server configuration
// MISNAMED - should be SERVER_CONFIG,
// and should factor out scheduler-specific stuff into SCHED_CONFIG
//
struct SCHED_CONFIG {
    char master_url[256];
    char long_name[256];
    char db_name[256];
    char db_user[256];
    char db_passwd[256];
    char db_host[256];
    char replica_db_name[256];
    char replica_db_user[256];
    char replica_db_passwd[256];
    char replica_db_host[256];
    int shmem_key;
    char project_dir[256];
    char key_dir[256];
    char download_url[256];
    char download_dir[256];
    char upload_url[256];
    char upload_dir[256];
    int report_grace_period;
        // grace period for reporting results;
        // server's report deadline is client's deadline + this
    double delete_delay;
    bool msg_to_host;
    bool non_cpu_intensive;
    bool strict_memory_bound;
    bool verify_files_on_app_start;
    int homogeneous_redundancy;
    bool hr_allocate_slots;
    bool hr_class_static;
    bool ignore_upload_certificates;
    bool dont_generate_upload_certificates;
    int uldl_dir_fanout;        // fanout of ul/dl dirs; 0 if none
    bool cache_md5_info;
    int fuh_debug_level;
    int fuh_set_completed_permission;
    int fuh_set_initial_permission;
    int reliable_priority_on_over;
        // additional results generated after at least one result
        // is over will have their priority boosted by this amount
    int reliable_priority_on_over_except_error;
        // additional results generated after at least one result is over
        // (unless there is an error) will have their priority boosted
        // by this amount
    int reliable_on_priority;
        // results with a priority equal or greater than this value
        // will be sent to reliable hosts
    bool distinct_beta_apps;
        // allow users to select beta apps independently
    bool ended;
        // Project has ended - tell clients to detach
    int shmem_work_items;
        // number of work items in shared memory
    int feeder_query_size;
        // number of work items to request in each feeder query
    char httpd_user[256];
        // user name under which web server runs (default: apache)
    bool enable_assignment;
    bool enable_vda;
    double vda_host_timeout;
    bool enable_assignment_multi;
    bool job_size_matching;
    bool dont_send_jobs;
    bool user_url;          // whether to export user.url in db dump
    bool user_country;

    //////////// STUFF RELEVANT ONLY TO SCHEDULER FOLLOWS ///////////

    vector<regex_t> *ban_cpu;
    vector<regex_t> *ban_os;
    int daily_result_quota;         // max results per day is this * mult
    char debug_req_reply_dir[256];
        // keep sched_request and sched_reply in files in this directory
    double default_disk_min_free_gb;
    vector<int> dont_search_host_for_userid;
    bool dont_store_success_stderr;
    int file_deletion_strategy;
        // select method of automatically deleting files from host
    int gpu_multiplier;             // mult is NCPUS + this*NGPUS
    bool ignore_delay_bound;
    bool locality_scheduling;
    double locality_scheduler_fraction;
    bool locality_scheduling_sorted_order;
    int locality_scheduling_wait_period;
    int locality_scheduling_send_timeout;
    vector<regex_t> *locality_scheduling_workunit_file;
    vector<regex_t> *locality_scheduling_sticky_file;
    bool sched_old;
    int max_download_urls_per_file;
    int max_ncpus;
    JOB_LIMITS max_jobs_in_progress;
    int max_results_accepted;
        // skip reported jobs beyond this limit
        // (they'll get reported in the next RPC)
        // This limits the memory usage of the scheduler;
        // otherwise it can crash if the client is reporting thousands of jobs.
    int max_wus_to_send;            // max results per RPC is this * mult
    int min_core_client_version;
    int min_core_client_version_announced;
    int min_core_client_upgrade_deadline;
    int min_sendwork_interval;
    double next_rpc_delay;
    bool no_amd_k6;
        // don't allow AMD K6 CPUs
    bool no_vista_sandbox;
    bool nowork_skip;
    bool one_result_per_host_per_wu;
    bool one_result_per_user_per_wu;
    int reliable_max_avg_turnaround;
        // max average turnaround for a host to be declared reliable
    double reliable_max_error_rate;
        // DEPRECATED
        // max error rate for a host to be declared reliable
    double reliable_reduced_delay_bound;
        // Reduce the delay bounds for reliable hosts by this percent
    char replace_download_url_by_timezone[256];
    bool prefer_primary_platform;
        // When selecting app versions,
        // use the client's primary platform if a version exists.
        // e.g. send 64-bit versions to 64-bit clients,
        // rather than trying the 32-bit version to see if it's faster.
        // Do this only if you're sure that the 64-bit versions are
        // always faster than the corresponding 32-bit versions
    double version_select_random_factor;
        // in deciding what version is fastest,
        // multiply projected FLOPS by a random var with mean 1 and this stddev.
    int report_max;
    bool request_time_stats_log;
    bool resend_lost_results;
    int sched_debug_level;
    int scheduler_log_buffer;
    char sched_lockfile_dir[256];
    bool send_result_abort;
    char symstore[256];
    bool user_filter;
        // send a job to a user only if wu.batch == user.id
        // DEPRECATED: use assignment instead
    bool workload_sim;
        // Do workload simulation in deciding whether to send a result
    bool estimate_flops_from_hav_pfc;
        // Use host_app_version peak flop count rather than elapsed time
        // to calculate projected_flops when choosing version.
    bool credit_by_app;
        // store per-app credit info in credit_user and credit_team
    bool keyword_sched;
        // score jobs based on keywords
    bool rte_no_stats;
        // don't use statistics in job runtime estimation
    bool batch_accel;
        // accelerate batch completion:
        // send high-prio jobs only to low-turnaround hosts
    bool size_classes;
        // use size classes

    // time intervals
    double maintenance_delay;
        // if stop_sched is set, tell clients to delay this much

    // scheduler log flags
    //
    bool debug_assignment;
    bool debug_credit;
    bool debug_edf_sim_detail;      // show details of EDF sim
    bool debug_edf_sim_workload;    // show workload for EDF sim
    bool debug_fcgi;
    bool debug_client_files;        // stuff related to sticky files on client
    bool debug_handle_results;
    bool debug_keyword;
    bool debug_locality;            // locality scheduling
    bool debug_locality_lite;       // locality scheduling Lite
    bool debug_prefs;
    bool debug_quota;
    bool debug_request_details;
    bool debug_request_headers;
    bool debug_resend;
    bool debug_send;
        // job dispatch, high-level stuff, e.g. request params and jobs sent
    bool debug_send_scan;
        // job dispatch at the level of scans through array
    bool debug_send_job;
        // job dispatch: why individual jobs weren't sent (most verbose)
    bool debug_user_messages;
    bool debug_vda;
    bool debug_version_select;

    int parse(FILE*);
    int parse_aux(FILE*);
    int parse_file(const char *dir = 0);

    int upload_path(const char*, char*);
    int download_path(const char*, char*);

    const char *project_path(const char *, ...);
};

extern SCHED_CONFIG config;

#endif
