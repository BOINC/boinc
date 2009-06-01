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

// Parse a server configuration file

#ifdef _USING_FCGI_
#include "boinc_fcgi.h"
#endif

#include <cstring>
#include <string>
#include <unistd.h>

#include "parse.h"
#include "error_numbers.h"
#include "filesys.h"
#include "str_util.h"

#include "sched_msgs.h"
#include "sched_util.h"
#include "sched_config.h"


const char* CONFIG_FILE = "config.xml";

SCHED_CONFIG config;

const int MAX_NCPUS = 8;
    // max multiplier for daily_result_quota.
    // need to change as multicore processors expand

int SCHED_CONFIG::parse(FILE* f) {
    char tag[1024], buf[256];
    bool is_tag;
    MIOFILE mf;
    XML_PARSER xp(&mf);
    int retval;
    regex_t re;

    mf.init_file(f);

    memset(this, 0, sizeof(*this));
    ban_os = new vector<regex_t>;
    ban_cpu = new vector<regex_t>;
    locality_scheduling_workunit_file = new vector<regex_t>;
    locality_scheduling_sticky_file = new vector<regex_t>;
    max_wus_to_send = 10;
    default_disk_max_used_gb = 100.;
    default_disk_max_used_pct = 50.;
    default_disk_min_free_gb = .001;
    sched_debug_level = MSG_NORMAL;
    fuh_debug_level = MSG_NORMAL;
    strcpy(httpd_user, "apache");
    max_ncpus = MAX_NCPUS;

    if (!xp.parse_start("boinc")) return ERR_XML_PARSE;
    if (!xp.parse_start("config")) return ERR_XML_PARSE;
    while (!xp.get(tag, sizeof(tag), is_tag)) {
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
        if (xp.parse_str(tag, "master_url", master_url, sizeof(master_url))) continue;
        if (xp.parse_str(tag, "long_name", long_name, sizeof(long_name))) continue;
        if (xp.parse_str(tag, "db_name", db_name, sizeof(db_name))) continue;
        if (xp.parse_str(tag, "db_user", db_user, sizeof(db_user))) continue;
        if (xp.parse_str(tag, "db_passwd", db_passwd, sizeof(db_passwd))) continue;
        if (xp.parse_str(tag, "db_host", db_host, sizeof(db_host))) continue;
        if (xp.parse_str(tag, "project_dir", project_dir, sizeof(project_dir))) continue;
        if (xp.parse_int(tag, "shmem_key", shmem_key)) continue;
        if (xp.parse_str(tag, "key_dir", key_dir, sizeof(key_dir))) continue;
        if (xp.parse_str(tag, "download_url", download_url, sizeof(download_url))) continue;
        if (xp.parse_str(tag, "download_dir", download_dir, sizeof(download_dir))) continue;
        if (xp.parse_str(tag, "upload_url", upload_url, sizeof(upload_url))) continue;
        if (xp.parse_str(tag, "upload_dir", upload_dir, sizeof(upload_dir))) continue;
        if (xp.parse_str(tag, "sched_lockfile_dir", sched_lockfile_dir, sizeof(sched_lockfile_dir))) continue;
        if (xp.parse_bool(tag, "one_result_per_user_per_wu", one_result_per_user_per_wu)) continue;
        if (xp.parse_bool(tag, "one_result_per_host_per_wu", one_result_per_host_per_wu)) continue;
        if (xp.parse_bool(tag, "non_cpu_intensive", non_cpu_intensive)) continue;
        if (xp.parse_bool(tag, "verify_files_on_app_start", verify_files_on_app_start)) continue;
        if (xp.parse_int(tag, "homogeneous_redundancy", homogeneous_redundancy)) continue;
        if (xp.parse_bool(tag, "locality_scheduling", locality_scheduling)) continue;
        if (xp.parse_bool(tag, "locality_scheduling_sorted_order", locality_scheduling_sorted_order)) continue;
        if (xp.parse_bool(tag, "msg_to_host", msg_to_host)) continue;
        if (xp.parse_bool(tag, "ignore_upload_certificates", ignore_upload_certificates)) continue;
        if (xp.parse_bool(tag, "dont_generate_upload_certificates", dont_generate_upload_certificates)) continue;
        if (xp.parse_bool(tag, "ignore_delay_bound", ignore_delay_bound)) continue;
        if (xp.parse_int(tag, "min_sendwork_interval", min_sendwork_interval)) continue;
        if (xp.parse_int(tag, "max_wus_to_send", max_wus_to_send)) continue;
        if (xp.parse_int(tag, "max_wus_in_progress", max_wus_in_progress)) continue;
        if (xp.parse_int(tag, "max_wus_in_progress_gpu", max_wus_in_progress_gpu)) continue;
        if (xp.parse_int(tag, "daily_result_quota", daily_result_quota)) continue;
        if (xp.parse_int(tag, "gpu_multiplier", gpu_multiplier)) continue;
        if (xp.parse_int(tag, "uldl_dir_fanout", uldl_dir_fanout)) continue;
        if (xp.parse_int(tag, "locality_scheduling_wait_period", locality_scheduling_wait_period)) continue;
        if (xp.parse_int(tag, "locality_scheduling_send_timeout", locality_scheduling_send_timeout)) continue;
        if (xp.parse_str(tag, "locality_scheduling_workunit_file", buf, sizeof(buf))) {
            retval = regcomp(&re, buf, REG_EXTENDED|REG_NOSUB);
            if (retval) {
                log_messages.printf(MSG_CRITICAL, "BAD REGEXP: %s\n", buf);
            } else {
                locality_scheduling_workunit_file->push_back(re);
            }
            continue;
        }
        if (xp.parse_str(tag, "locality_scheduling_sticky_file", buf, sizeof(buf))) {
            retval = regcomp(&re, buf, REG_EXTENDED|REG_NOSUB);
            if (retval) {
                log_messages.printf(MSG_CRITICAL, "BAD REGEXP: %s\n", buf);
            } else {
                locality_scheduling_sticky_file->push_back(re);
            }
            continue;
        }
        if (xp.parse_double(tag, "locality_scheduler_fraction", locality_scheduler_fraction)) continue;
        if (xp.parse_int(tag, "min_core_client_version", min_core_client_version)) continue;
        if (xp.parse_int(tag, "min_core_client_version_announced", min_core_client_version_announced)) continue;
        if (xp.parse_int(tag, "min_core_client_upgrade_deadline", min_core_client_upgrade_deadline)) continue;
        if (xp.parse_str(tag, "replace_download_url_by_timezone", replace_download_url_by_timezone, sizeof(replace_download_url_by_timezone))) continue;
        if (xp.parse_bool(tag, "cache_md5_info", cache_md5_info)) continue;
        if (xp.parse_bool(tag, "nowork_skip", nowork_skip)) continue;
        if (xp.parse_bool(tag, "resend_lost_results", resend_lost_results)) continue;
        if (xp.parse_bool(tag, "send_result_abort", send_result_abort)) continue;
        if (xp.parse_double(tag, "fp_benchmark_weight", fp_benchmark_weight)) {
            if (fp_benchmark_weight < 0 || fp_benchmark_weight > 1) {
                fprintf(stderr,
                    "CONFIG FILE ERROR: fp_benchmark_weight outside of 0..1"
                );
            } else {
                use_benchmark_weights = true;
            }
            continue;
        }
        if (xp.parse_double(tag, "default_disk_max_used_gb", default_disk_max_used_gb)) continue;
        if (xp.parse_double(tag, "default_disk_max_used_pct", default_disk_max_used_pct)) continue;
        if (xp.parse_double(tag, "default_disk_min_free_gb", default_disk_min_free_gb)) continue;
        if (xp.parse_str(tag, "symstore", symstore, sizeof(symstore))) continue;
        if (xp.parse_double(tag, "next_rpc_delay", next_rpc_delay)) continue;
        if (xp.parse_int(tag, "sched_debug_level", sched_debug_level)) continue;
        if (xp.parse_int(tag, "fuh_debug_level", fuh_debug_level)) continue;
        if (xp.parse_int(tag, "reliable_max_avg_turnaround", reliable_max_avg_turnaround)) continue;
        if (xp.parse_double(tag, "reliable_max_error_rate", reliable_max_error_rate)) continue;
        if (xp.parse_int(tag, "reliable_priority_on_over", reliable_priority_on_over)) continue;
        if (xp.parse_int(tag, "reliable_priority_on_over_except_error", reliable_priority_on_over_except_error)) continue;
        if (xp.parse_int(tag, "reliable_on_priority", reliable_on_priority)) continue;
        if (xp.parse_double(tag, "reliable_reduced_delay_bound", reliable_reduced_delay_bound)) continue;
        if (xp.parse_int(tag, "grace_period_hours", grace_period_hours)) continue;
        if (xp.parse_int(tag, "delete_delay_hours", delete_delay_hours)) continue;
        if (xp.parse_bool(tag, "distinct_beta_apps", distinct_beta_apps)) continue;
        if (xp.parse_bool(tag, "workload_sim", workload_sim)) continue;
        if (xp.parse_bool(tag, "ended", ended)) continue;
        if (xp.parse_int(tag, "shmem_work_items", shmem_work_items)) continue;
        if (xp.parse_int(tag, "feeder_query_size", feeder_query_size)) continue;
        if (xp.parse_int(tag, "granted_credit_ramp_up", granted_credit_ramp_up)) continue;
        if (xp.parse_double(tag, "granted_credit_weight", granted_credit_weight)) continue;
        if (xp.parse_bool(tag, "no_amd_k6", no_amd_k6)) {
            if (no_amd_k6) {
                regcomp(&re, ".*AMD.*\t.*Family 5 Model 8 Stepping 0.*", REG_EXTENDED|REG_NOSUB);
                ban_cpu->push_back(re);
            }
            continue;
        }
        if (xp.parse_str(tag, "httpd_user", httpd_user, sizeof(httpd_user))) continue;
        if (xp.parse_int(tag, "file_deletion_strategy", file_deletion_strategy)) continue;
        if (xp.parse_bool(tag, "request_time_stats_log", request_time_stats_log)) continue;
        if (xp.parse_bool(tag, "enable_assignment", enable_assignment)) continue;
        if (xp.parse_int(tag, "max_ncpus", max_ncpus)) continue;
        if (xp.parse_str(tag, "ban_os", buf, sizeof(buf))) {
            retval = regcomp(&re, buf, REG_EXTENDED|REG_NOSUB);
            if (retval) {
                log_messages.printf(MSG_CRITICAL, "BAD REGEXP: %s\n", buf);
            } else {
                ban_os->push_back(re);
            }
            continue;
        }
        if (xp.parse_str(tag, "ban_cpu", buf, sizeof(buf))) {
            retval = regcomp(&re, buf, REG_EXTENDED|REG_NOSUB);
            if (retval) {
                log_messages.printf(MSG_CRITICAL, "BAD REGEXP: %s\n", buf);
            } else {
                ban_cpu->push_back(re);
            }
            continue;
        }
        if (xp.parse_bool(tag, "matchmaker", matchmaker)) continue;
        if (xp.parse_int(tag, "mm_min_slots", mm_min_slots)) continue;
        if (xp.parse_int(tag, "mm_max_slots", mm_max_slots)) continue;
        if (xp.parse_bool(tag, "job_size_matching", job_size_matching)) continue;
        if (xp.parse_bool(tag, "use_credit_multiplier", use_credit_multiplier)) continue;
        if (xp.parse_bool(tag, "multiple_clients_per_host", multiple_clients_per_host)) continue;
        if (xp.parse_bool(tag, "no_vista_sandbox", no_vista_sandbox)) continue;
        if (xp.parse_bool(tag, "ignore_dcf", ignore_dcf)) continue;
        if (xp.parse_int(tag, "report_max", report_max)) continue;

        if (xp.parse_bool(tag, "debug_version_select", debug_version_select)) continue;
        if (xp.parse_bool(tag, "debug_assignment", debug_assignment)) continue;
        if (xp.parse_bool(tag, "debug_prefs", debug_prefs)) continue;
        if (xp.parse_bool(tag, "debug_send", debug_send)) continue;
        if (xp.parse_bool(tag, "debug_resend", debug_resend)) continue;
        if (xp.parse_bool(tag, "debug_request_headers", debug_request_headers)) continue;
        if (xp.parse_bool(tag, "debug_user_messages", debug_user_messages)) continue;
        if (xp.parse_bool(tag, "debug_request_details", debug_request_details)) continue;
        if (xp.parse_bool(tag, "debug_handle_results", debug_handle_results)) continue;
        if (xp.parse_bool(tag, "debug_edf_sim_workload", debug_edf_sim_workload)) continue;
        if (xp.parse_bool(tag, "debug_edf_sim_detail", debug_edf_sim_detail)) continue;
        if (xp.parse_bool(tag, "debug_locality", debug_locality)) continue;
        if (xp.parse_bool(tag, "debug_array", debug_array)) continue;

        // don't complain about unparsed XML;
        // there are lots of tags the scheduler doesn't know about

        xp.skip_unexpected(
            tag, log_messages.debug_level==3, "SCHED_CONFIG::parse"
        );
    }   
    return ERR_XML_PARSE;
}

int SCHED_CONFIG::parse_file(const char* dir) {
    char path[256];
    int retval;

    if (dir && dir[0]) {
        snprintf(path, sizeof(path), "%s/%s", dir, CONFIG_FILE);
    } else {
        strcpy(path, project_path(CONFIG_FILE));
    }
#ifndef _USING_FCGI_
    FILE* f = fopen(path, "r");
#else
    FCGI_FILE *f = FCGI::fopen(path, "r");
#endif
    if (!f) return ERR_FOPEN;
    retval = parse(f);
    fclose(f);
    return retval;
}

int SCHED_CONFIG::upload_path(const char* filename, char* path) {
    return ::dir_hier_path(filename, upload_dir, uldl_dir_fanout, path, true);
}

int SCHED_CONFIG::download_path(const char* filename, char* path) {
    return ::dir_hier_path(filename, download_dir, uldl_dir_fanout, path, true);
}

// Does 2 things:
// - locate project directory.  This is either
//      a) env var BOINC_PROJECT_DIR, if defined
//      b) current dir, if config.xml exists there
//      c) parent dir, if config.xml exists there
// - returns a path relative to the project dir,
//      specified by a format string + args
//
const char *SCHED_CONFIG::project_path(const char *fmt, ...) {
    static char path[1024];
    va_list ap;

    if (!strlen(project_dir)) {
        char *p = getenv("BOINC_PROJECT_DIR");
        if (p) {
            strlcpy(project_dir, p, sizeof(project_dir));
        } else if (boinc_file_exists(CONFIG_FILE)) {
            strcpy(project_dir, ".");
        } else {
            strcpy(project_dir, "..");
        }
    }

    va_start(ap, fmt);
    snprintf(path, sizeof(path), "%s/", project_dir);
    vsnprintf(path + strlen(path), sizeof(path) - strlen(path), fmt, ap);
    va_end(ap);
    return (const char *)path;
}

const char *BOINC_RCSID_3704204cfd = "$Id$";
