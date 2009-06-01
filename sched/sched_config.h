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

#ifndef _SCHED_CONFIG_
#define _SCHED_CONFIG_

#include "regex.h"
#include <vector>
#include <cstdio>

using std::vector;

// parsed version of server configuration file
//
class SCHED_CONFIG {
public:
    char master_url[256];
    char long_name[256];
    char db_name[256];
    char db_user[256];
    char db_passwd[256];
    char db_host[256];
    int shmem_key;
    char project_dir[256];
    char key_dir[256];
    char download_url[256];
    char download_dir[256];
    char upload_url[256];
    char upload_dir[256];
    char sched_lockfile_dir[256];
    int grace_period_hours;
    int delete_delay_hours;
    bool one_result_per_user_per_wu;
    bool one_result_per_host_per_wu;
    bool msg_to_host;
    int min_sendwork_interval;
    int max_wus_in_progress;
    int max_wus_in_progress_gpu;
    bool non_cpu_intensive;
    bool verify_files_on_app_start;
    int homogeneous_redundancy;
    bool locality_scheduling;
    bool locality_scheduling_sorted_order;
    bool ignore_upload_certificates;
    bool dont_generate_upload_certificates;
    bool ignore_delay_bound;
    int gpu_multiplier;             // mult is NCPUS + this*NGPUS
    int daily_result_quota;         // max results per day is this * mult
    int max_wus_to_send;            // max results per RPC is this * mult
    int uldl_dir_fanout;        // fanout of ul/dl dirs; 0 if none
    int uldl_dir_levels;
    int locality_scheduling_wait_period;
    int locality_scheduling_send_timeout;
    vector<regex_t> *locality_scheduling_workunit_file;
    vector<regex_t> *locality_scheduling_sticky_file;
    double locality_scheduler_fraction;
    int min_core_client_version;
    int min_core_client_version_announced;
    int min_core_client_upgrade_deadline;
    char replace_download_url_by_timezone[256];
    bool cache_md5_info;
    bool nowork_skip;
    bool resend_lost_results;
    bool send_result_abort;
    bool use_benchmark_weights;
    double fp_benchmark_weight;
    double default_disk_max_used_gb;
    double default_disk_max_used_pct;
    double default_disk_min_free_gb;
    char symstore[256];
    double next_rpc_delay;
    int sched_debug_level;
    int fuh_debug_level;
    double reliable_max_error_rate;  // max error rate for a host to be declared reliable
    int reliable_max_avg_turnaround;
        // max average turnaround for a host to be declared reliable
    int reliable_priority_on_over;
        // additional results generated after at least one result
        // is over will have their priority boosted by this amount    
    int reliable_priority_on_over_except_error;
        // additional results generated after at least one result is over
        // (unless their is an error) will have their priority boosted
        // by this amount
    int reliable_on_priority;
        // results with a priority equal or greater than this value
        // will be sent to reliable hosts
    double reliable_reduced_delay_bound;
        // Reduce the delay bounds for reliable hosts by this percent
	int granted_credit_ramp_up; 
	double granted_credit_weight;
    bool distinct_beta_apps;
        // allow users to select beta apps independently
    bool workload_sim;
        // Do workload simulation in deciding whether to send a result
    bool ended;
        // Project has ended - tell clients to detach
    int shmem_work_items;
        // number of work items in shared memory
    int feeder_query_size;
        // number of work items to request in each feeder query
    bool no_amd_k6;
        // don't allow AMD K6 CPUs
    char httpd_user[256];
        // user name under which web server runs (default: apache)
    int file_deletion_strategy;
        // select method of automatically deleting files from host
    bool request_time_stats_log;
    bool enable_assignment;
    int max_ncpus;
    vector<regex_t> *ban_os;
    vector<regex_t> *ban_cpu;
    bool matchmaker;
    int mm_min_slots;
    int mm_max_slots;
    bool job_size_matching;
    bool use_credit_multiplier;
    bool multiple_clients_per_host;
    bool no_vista_sandbox;
    bool ignore_dcf;
    int report_max;

    // log flags
    //
    bool debug_version_select;
    bool debug_assignment;
    bool debug_prefs;
    bool debug_send;
    bool debug_resend;
    bool debug_request_headers;
    bool debug_user_messages;
    bool debug_request_details;
    bool debug_handle_results;
    bool debug_edf_sim_workload;    // show workload for EDF sim
    bool debug_edf_sim_detail;      // show details of EDF sim
    bool debug_locality;            // locality scheduling
    bool debug_array;               // debug old-style array scheduling

    int parse(FILE*);
    int parse_file(const char *dir = 0);

    int upload_path(const char*, char*);
    int download_path(const char*, char*);

    const char *project_path(const char *, ...);
};

extern SCHED_CONFIG config;

#endif
