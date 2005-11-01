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

#ifndef _SCHED_CONFIG_
#define _SCHED_CONFIG_

// parsed version of server configuration file
//
class SCHED_CONFIG {
public:
    char long_name[256];
    char db_name[256];
    char db_user[256];
    char db_passwd[256];
    char db_host[256];
    int shmem_key;
    char key_dir[256];
    char download_url[256];
    char download_dir[256];
    char download_dir_alt[256];
        // old download dir, assumed to be flat
        // (file deleter looks here if not in main dir)
    char upload_url[256];
    char upload_dir[256];
    char sched_lockfile_dir[256];
    bool one_result_per_user_per_wu;
    bool msg_to_host;
    int min_sendwork_interval;
    int max_wus_to_send;
    bool non_cpu_intensive;
    bool homogeneous_redundancy;
    bool locality_scheduling;
    bool ignore_upload_certificates;
    bool dont_generate_upload_certificates;
    bool ignore_delay_bound;
#if 0
    bool deletion_policy_priority;
    bool deletion_policy_expire;
    bool delete_from_self;
#endif
    bool use_transactions;
    int daily_result_quota;     // max results per host per day
    int uldl_dir_fanout;        // fanout of ul/dl dirs; 0 if none
    int locality_scheduling_wait_period;
    int locality_scheduling_send_timeout;
    int min_core_client_version;
    int min_core_client_version_announced;
    int min_core_client_upgrade_deadline;
    bool choose_download_url_by_timezone;
    bool cache_md5_info;
    bool nowork_skip;
    bool resend_lost_results;
    bool use_benchmark_weights;
    double fp_benchmark_weight;

    int parse(char*);
    int parse_file(const char* dir=".");
};

// get the project's home directory
// (assumed to be the parent of the CWD)
//
void get_project_dir(char*, int);

#endif
