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

// Parse a server configuration file

#include <cstring>
#include <string>
#include <unistd.h>

#include "parse.h"
#include "error_numbers.h"

#include "sched_msgs.h"
#include "sched_config.h"

#ifdef _USING_FCGI_
#include "fcgi_stdio.h"
#endif

const char* CONFIG_FILE = "config.xml";

int SCHED_CONFIG::parse(FILE* f) {
    char tag[1024], temp[1024];
    bool is_tag;
    MIOFILE mf;
    XML_PARSER xp(&mf);

    mf.init_file(f);
    memset(this, 0, sizeof(SCHED_CONFIG));
    max_wus_to_send = 10;
    default_disk_max_used_gb = 100.;
    default_disk_max_used_pct = 50.;
    default_disk_min_free_gb = .001;
    sched_debug_level = SCHED_MSG_LOG::MSG_NORMAL;
    fuh_debug_level = SCHED_MSG_LOG::MSG_NORMAL;


    if (!xp.parse_start("boinc")) return ERR_XML_PARSE;
    if (!xp.parse_start("config")) return ERR_XML_PARSE;
    while (!xp.get(tag, is_tag)) {
        if (!is_tag) {
            fprintf(stderr, "SCHED_CONFIG::parse(): unexpected text %s\n", tag);
            continue;
        }
        if (!strcmp(tag, "/config")) {
            char hostname[256];
            gethostname(hostname, 256);
            if (!strcmp(hostname, db_host)) strcpy(db_host, "localhost");
            return 0;
        }
        else if (xp.parse_str(tag, "master_url", master_url)) continue;
        else if (xp.parse_str(tag, "long_name", long_name)) continue;
        else if (xp.parse_str(tag, "db_name", db_name)) continue;
        else if (xp.parse_str(tag, "db_user", db_user)) continue;
        else if (xp.parse_str(tag, "db_passwd", db_passwd)) continue;
        else if (xp.parse_str(tag, "db_host", db_host)) continue;
        else if (xp.parse_int(tag, "shmem_key", shmem_key)) continue;
        else if (xp.parse_str(tag, "key_dir", key_dir)) continue;
        else if (xp.parse_str(tag, "download_url", download_url)) continue;
        else if (xp.parse_str(tag, "download_dir", download_dir)) continue;
        else if (xp.parse_str(tag, "download_dir_alt", download_dir_alt)) continue;
        else if (xp.parse_str(tag, "upload_url", upload_url)) continue;
        else if (xp.parse_str(tag, "upload_dir", upload_dir)) continue;
        else if (xp.parse_str(tag, "sched_lockfile_dir", sched_lockfile_dir)) continue;
        else if (xp.parse_bool(tag, "one_result_per_user_per_wu", one_result_per_user_per_wu)) continue;
        else if (xp.parse_bool(tag, "non_cpu_intensive", non_cpu_intensive)) continue;
        else if (xp.parse_bool(tag, "verify_files_on_app_start", verify_files_on_app_start)) continue;
        else if (xp.parse_bool(tag, "homogeneous_redundancy", homogeneous_redundancy)) continue;
        else if (xp.parse_bool(tag, "locality_scheduling", locality_scheduling)) continue;
        else if (xp.parse_bool(tag, "msg_to_host", msg_to_host)) continue;
        else if (xp.parse_bool(tag, "ignore_upload_certificates", ignore_upload_certificates)) continue;
        else if (xp.parse_bool(tag, "dont_generate_upload_certificates", dont_generate_upload_certificates)) continue;
        else if (xp.parse_bool(tag, "ignore_delay_bound", ignore_delay_bound)) continue;
        else if (xp.parse_bool(tag, "use_transactions", use_transactions)) continue;
        else if (xp.parse_int(tag, "min_sendwork_interval", min_sendwork_interval)) continue;
        else if (xp.parse_int(tag, "max_wus_to_send", max_wus_to_send)) continue;
        else if (xp.parse_int(tag, "daily_result_quota", daily_result_quota)) continue;
        else if (xp.parse_int(tag, "uldl_dir_fanout", uldl_dir_fanout)) continue;
        else if (xp.parse_int(tag, "locality_scheduling_wait_period", locality_scheduling_wait_period)) continue;
        else if (xp.parse_int(tag, "locality_scheduling_send_timeout", locality_scheduling_send_timeout)) continue;
        else if (xp.parse_int(tag, "min_core_client_version", min_core_client_version)) continue;
        else if (xp.parse_int(tag, "min_core_client_version_announced", min_core_client_version_announced)) continue;
        else if (xp.parse_int(tag, "min_core_client_upgrade_deadline", min_core_client_upgrade_deadline)) continue;
        else if (xp.parse_int(tag, "max_claimed_credit", max_claimed_credit)) continue;
        else if (xp.parse_bool(tag, "choose_download_url_by_timezone", choose_download_url_by_timezone)) continue;
        else if (xp.parse_bool(tag, "cache_md5_info", cache_md5_info)) continue;
        else if (xp.parse_bool(tag, "nowork_skip", nowork_skip)) continue;
        else if (xp.parse_bool(tag, "resend_lost_results", resend_lost_results)) continue;
        else if (xp.parse_bool(tag, "grant_claimed_credit", grant_claimed_credit)) continue;
        else if (xp.parse_double(tag, "fp_benchmark_weight", fp_benchmark_weight)) {
            if (fp_benchmark_weight < 0 || fp_benchmark_weight > 1) {
                fprintf(stderr,
                    "CONFIG FILE ERROR: fp_benchmark_weight outside of 0..1"
                );
            } else {
                use_benchmark_weights = true;
            }
        }
        else if (xp.parse_double(tag, "default_disk_max_used_gb", default_disk_max_used_gb)) continue;
        else if (xp.parse_double(tag, "default_disk_max_used_pct", default_disk_max_used_pct)) continue;
        else if (xp.parse_double(tag, "default_disk_min_free_gb", default_disk_min_free_gb)) continue;
        else if (xp.parse_str(tag, "symstore", symstore)) continue;
        else if (xp.parse_double(tag, "next_rpc_delay", next_rpc_delay)) continue;
        else if (xp.parse_bool(tag, "dont_delete_batches", dont_delete_batches)) continue;
        else if (xp.parse_int(tag, "sched_debug_level", sched_debug_level)) continue;
        else if (xp.parse_int(tag, "fuh_debug_level", fuh_debug_level)) continue;

        // tags the scheduler doesn't care about
        //
        else if (xp.parse_str(tag, "cgi_url", temp)) continue;
        else if (xp.parse_str(tag, "log_dir", temp)) continue;
        else if (xp.parse_str(tag, "app_dir", temp)) continue;
        else if (xp.parse_str(tag, "show_results", temp)) continue;
        else if (xp.parse_str(tag, "host", temp)) continue;
        else if (xp.parse_str(tag, "output_level", temp)) continue;
        else if (xp.parse_str(tag, "profile_screening", temp)) continue;
        else if (xp.parse_str(tag, "min_passwd_length", temp)) continue;
        else if (xp.parse_str(tag, "disable_account_creation", temp)) continue;
        else fprintf(stderr, "unknown tag: %s\n", tag);
    }   
    return ERR_XML_PARSE;
}

int SCHED_CONFIG::parse_file(const char* dir) {
    char path[256];
    int retval;

    sprintf(path, "%s/%s", dir, CONFIG_FILE);
    FILE* f = fopen(path, "r");
    if (!f) return ERR_FOPEN;
    retval = parse(f);
    fclose(f);
    return retval;
}

void get_project_dir(char* p, int len) {
    getcwd(p, len);
    char* q = strrchr(p, '/');
    if (q) *q = 0;
}

const char *BOINC_RCSID_3704204cfd = "$Id$";
