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
#include "str_replace.h"

#include "sched_msgs.h"
#include "sched_util.h"
#include "sched_config.h"

const char* CONFIG_FILE = "config.xml";
const char* CONFIG_FILE_AUX = "config_aux.xml";

SCHED_CONFIG config;

const int MAX_NCPUS = 64;
    // max multiplier for daily_result_quota.
    // need to change as multicore processors expand

int SCHED_CONFIG::parse_aux(FILE* f) {
    MIOFILE mf;
    XML_PARSER xp(&mf);
    mf.init_file(f);
    if (!xp.parse_start("config")) return ERR_XML_PARSE;
    while (!xp.get_tag()) {
        if (!xp.is_tag) {
            fprintf(stderr,
                "SCHED_CONFIG::parse(): unexpected text %s\n",
                xp.parsed_tag
            );
            continue;
        }
        if (xp.match_tag("/config")) {
            return 0;
        }
        if (xp.match_tag("max_jobs_in_progress")) {
            max_jobs_in_progress.parse(xp, "/max_jobs_in_progress");
        }
    }
    return ERR_XML_PARSE;
}

int SCHED_CONFIG::parse(FILE* f) {
    char buf[256];
    MIOFILE mf;
    XML_PARSER xp(&mf);
    int retval, itemp;
    regex_t re;
    double x;

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
    scheduler_log_buffer = 32768;
    version_select_random_factor = .1;

    if (!xp.parse_start("boinc")) return ERR_XML_PARSE;
    if (!xp.parse_start("config")) return ERR_XML_PARSE;
    while (!xp.get_tag()) {
        if (!xp.is_tag) {
            fprintf(stderr,
                "SCHED_CONFIG::parse(): unexpected text %s\n",
                xp.parsed_tag
            );
            continue;
        }
        if (xp.match_tag("/config")) {
            char hostname[256];
            gethostname(hostname, 256);
            if (!strcmp(hostname, db_host)) strcpy(db_host, "localhost");
            if (!strlen(replica_db_host)) {
                strcpy(replica_db_host, db_host);
            }
            if (!strlen(replica_db_name)) {
                strcpy(replica_db_name, db_name);
            }
            if (!strlen(replica_db_user)) {
                strcpy(replica_db_user, db_user);
            }
            if (!strlen(replica_db_passwd)) {
                strcpy(replica_db_passwd, db_passwd);
            }
            return 0;
        }
        if (xp.parse_str("master_url", master_url, sizeof(master_url))) continue;
        if (xp.parse_str("long_name", long_name, sizeof(long_name))) continue;
        if (xp.parse_str("db_name", db_name, sizeof(db_name))) continue;
        if (xp.parse_str("db_user", db_user, sizeof(db_user))) continue;
        if (xp.parse_str("db_passwd", db_passwd, sizeof(db_passwd))) continue;
        if (xp.parse_str("db_host", db_host, sizeof(db_host))) continue;
        if (xp.parse_str("replica_db_name", replica_db_name, sizeof(replica_db_name))) continue;
        if (xp.parse_str("replica_db_user", replica_db_user, sizeof(replica_db_user))) continue;
        if (xp.parse_str("replica_db_passwd", replica_db_passwd, sizeof(replica_db_passwd))) continue;
        if (xp.parse_str("replica_db_host", replica_db_host, sizeof(replica_db_host))) continue;
        if (xp.parse_str("project_dir", project_dir, sizeof(project_dir))) continue;
        if (xp.parse_int("shmem_key", shmem_key)) continue;
        if (xp.parse_str("key_dir", key_dir, sizeof(key_dir))) continue;
        if (xp.parse_str("download_url", download_url, sizeof(download_url))) continue;
        if (xp.parse_str("download_dir", download_dir, sizeof(download_dir))) continue;
        if (xp.parse_str("upload_url", upload_url, sizeof(upload_url))) continue;
        if (xp.parse_str("upload_dir", upload_dir, sizeof(upload_dir))) continue;
        if (xp.parse_bool("non_cpu_intensive", non_cpu_intensive)) continue;
        if (xp.parse_bool("verify_files_on_app_start", verify_files_on_app_start)) continue;
        if (xp.parse_int("homogeneous_redundancy", homogeneous_redundancy)) continue;
        if (xp.parse_bool("hr_allocate_slots", hr_allocate_slots)) continue;
        if (xp.parse_bool("msg_to_host", msg_to_host)) continue;
        if (xp.parse_bool("ignore_upload_certificates", ignore_upload_certificates)) continue;
        if (xp.parse_bool("dont_generate_upload_certificates", dont_generate_upload_certificates)) continue;
        if (xp.parse_int("uldl_dir_fanout", uldl_dir_fanout)) continue;
        if (xp.parse_bool("cache_md5_info", cache_md5_info)) continue;
        if (xp.parse_double("fp_benchmark_weight", fp_benchmark_weight)) {
            if (fp_benchmark_weight < 0 || fp_benchmark_weight > 1) {
                fprintf(stderr,
                    "CONFIG FILE ERROR: fp_benchmark_weight outside of 0..1"
                );
            } else {
                use_benchmark_weights = true;
            }
            continue;
        }
        if (xp.parse_int("fuh_debug_level", fuh_debug_level)) continue;
        if (xp.parse_int("reliable_priority_on_over", reliable_priority_on_over)) continue;
        if (xp.parse_int("reliable_priority_on_over_except_error", reliable_priority_on_over_except_error)) continue;
        if (xp.parse_int("reliable_on_priority", reliable_on_priority)) continue;
        if (xp.parse_double("grace_period_hours", x)) {
            report_grace_period = (int)(x*3600);
            continue;
        }
        if (xp.parse_int("report_grace_period", report_grace_period)) continue;
        if (xp.parse_double("delete_delay_hours", x)) {
            delete_delay = x*3600;
            continue;
        }
        if (xp.parse_bool("distinct_beta_apps", distinct_beta_apps)) continue;
        if (xp.parse_bool("ended", ended)) continue;
        if (xp.parse_int("shmem_work_items", shmem_work_items)) continue;
        if (xp.parse_int("feeder_query_size", feeder_query_size)) continue;
        if (xp.parse_str("httpd_user", httpd_user, sizeof(httpd_user))) continue;
        if (xp.parse_bool("enable_vda", enable_vda)) continue;
        if (xp.parse_bool("enable_assignment", enable_assignment)) continue;
        if (xp.parse_bool("enable_assignment_multi", enable_assignment_multi)) continue;
        if (xp.parse_bool("job_size_matching", job_size_matching)) continue;
        if (xp.parse_bool("dont_send_jobs", dont_send_jobs)) continue;

        //////////// STUFF RELEVANT ONLY TO SCHEDULER STARTS HERE ///////

        if (xp.parse_str("ban_cpu", buf, sizeof(buf))) {
            retval = regcomp(&re, buf, REG_EXTENDED|REG_NOSUB);
            if (retval) {
                log_messages.printf(MSG_CRITICAL, "BAD REGEXP: %s\n", buf);
            } else {
                ban_cpu->push_back(re);
            }
            continue;
        }
        if (xp.parse_str("ban_os", buf, sizeof(buf))) {
            retval = regcomp(&re, buf, REG_EXTENDED|REG_NOSUB);
            if (retval) {
                log_messages.printf(MSG_CRITICAL, "BAD REGEXP: %s\n", buf);
            } else {
                ban_os->push_back(re);
            }
            continue;
        }
        if (xp.parse_int("dont_search_host_for_user", retval)) {
            dont_search_host_for_userid.push_back(retval);
            continue;
        }
        if (xp.parse_int("daily_result_quota", daily_result_quota)) continue;
        if (xp.parse_double("default_disk_max_used_gb", default_disk_max_used_gb)) continue;
        if (xp.parse_double("default_disk_max_used_pct", default_disk_max_used_pct)) continue;
        if (xp.parse_double("default_disk_min_free_gb", default_disk_min_free_gb)) continue;
        if (xp.parse_bool("dont_store_success_stderr", dont_store_success_stderr)) continue;
        if (xp.parse_int("file_deletion_strategy", file_deletion_strategy)) continue;
        if (xp.parse_int("gpu_multiplier", gpu_multiplier)) continue;
        if (xp.parse_bool("ignore_delay_bound", ignore_delay_bound)) continue;
        if (xp.parse_bool("locality_scheduling", locality_scheduling)) continue;
        if (xp.parse_double("locality_scheduler_fraction", locality_scheduler_fraction)) continue;
        if (xp.parse_bool("locality_scheduling_sorted_order", locality_scheduling_sorted_order)) continue;
        if (xp.parse_int("locality_scheduling_wait_period", locality_scheduling_wait_period)) continue;
        if (xp.parse_int("locality_scheduling_send_timeout", locality_scheduling_send_timeout)) continue;
        if (xp.parse_str("locality_scheduling_workunit_file", buf, sizeof(buf))) {
            retval = regcomp(&re, buf, REG_EXTENDED|REG_NOSUB);
            if (retval) {
                log_messages.printf(MSG_CRITICAL, "BAD REGEXP: %s\n", buf);
            } else {
                locality_scheduling_workunit_file->push_back(re);
            }
            continue;
        }
        if (xp.parse_str("locality_scheduling_sticky_file", buf, sizeof(buf))) {
            retval = regcomp(&re, buf, REG_EXTENDED|REG_NOSUB);
            if (retval) {
                log_messages.printf(MSG_CRITICAL, "BAD REGEXP: %s\n", buf);
            } else {
                locality_scheduling_sticky_file->push_back(re);
            }
            continue;
        }
        if (xp.parse_bool("matchmaker", matchmaker)) continue;
        if (xp.parse_int("max_ncpus", max_ncpus)) continue;
        if (xp.parse_int("max_wus_in_progress", itemp)) {
            max_jobs_in_progress.project_limits.cpu.base_limit = itemp;
            max_jobs_in_progress.project_limits.cpu.per_proc = true;
            continue;
        }
        if (xp.parse_int("max_wus_in_progress_gpu", itemp)) {
            max_jobs_in_progress.project_limits.gpu.base_limit = itemp;
            max_jobs_in_progress.project_limits.gpu.per_proc = true;
            continue;
        }
        if (xp.parse_int("max_wus_to_send", max_wus_to_send)) continue;
        if (xp.parse_int("min_core_client_version", min_core_client_version)) {
            if (min_core_client_version && min_core_client_version < 10000) {
                log_messages.printf(MSG_CRITICAL,
                    "min_core_client_version too small; multiplying by 100\n"
                );
                min_core_client_version *= 100;
            }
            continue;
        }
        if (xp.parse_int("min_core_client_version_announced", min_core_client_version_announced)) {
            if (min_core_client_version_announced && min_core_client_version_announced < 10000) {
                log_messages.printf(MSG_CRITICAL,
                    "min_core_client_version_announced too small; multiplying by 100\n"
                );
                min_core_client_version_announced *= 100;
            }
            continue;
        }
        if (xp.parse_int("min_core_client_upgrade_deadline", min_core_client_upgrade_deadline)) continue;
        if (xp.parse_int("min_sendwork_interval", min_sendwork_interval)) continue;
        if (xp.parse_int("mm_min_slots", mm_min_slots)) continue;
        if (xp.parse_int("mm_max_slots", mm_max_slots)) continue;
        if (xp.parse_double("next_rpc_delay", next_rpc_delay)) continue;
        if (xp.parse_bool("no_amd_k6", no_amd_k6)) {
            if (no_amd_k6) {
                regcomp(&re, ".*AMD.*\t.*Family 5 Model 8 Stepping 0.*", REG_EXTENDED|REG_NOSUB);
                ban_cpu->push_back(re);
            }
            continue;
        }
        if (xp.parse_bool("no_vista_sandbox", no_vista_sandbox)) continue;
        if (xp.parse_bool("nowork_skip", nowork_skip)) continue;
        if (xp.parse_bool("one_result_per_host_per_wu", one_result_per_host_per_wu)) continue;
        if (xp.parse_bool("one_result_per_user_per_wu", one_result_per_user_per_wu)) continue;
        if (xp.parse_int("reliable_max_avg_turnaround", reliable_max_avg_turnaround)) continue;
        if (xp.parse_double("reliable_max_error_rate", reliable_max_error_rate)) continue;
        if (xp.parse_double("reliable_reduced_delay_bound", reliable_reduced_delay_bound)) continue;
        if (xp.parse_str("replace_download_url_by_timezone", replace_download_url_by_timezone, sizeof(replace_download_url_by_timezone))) continue;
        if (xp.parse_int("max_download_urls_per_file", max_download_urls_per_file)) continue;
        if (xp.parse_int("report_max", report_max)) continue;
        if (xp.parse_bool("request_time_stats_log", request_time_stats_log)) continue;
        if (xp.parse_bool("resend_lost_results", resend_lost_results)) continue;
        if (xp.parse_int("sched_debug_level", sched_debug_level)) continue;
        if (xp.parse_int("scheduler_log_buffer", scheduler_log_buffer)) continue;
        if (xp.parse_str("sched_lockfile_dir", sched_lockfile_dir, sizeof(sched_lockfile_dir))) continue;
        if (xp.parse_bool("send_result_abort", send_result_abort)) continue;
        if (xp.parse_str("symstore", symstore, sizeof(symstore))) continue;

        if (xp.parse_bool("user_filter", user_filter)) continue;
        if (xp.parse_bool("workload_sim", workload_sim)) continue;
        if (xp.parse_bool("prefer_primary_platform", prefer_primary_platform)) continue;
        if (xp.parse_double("version_select_random_factor", version_select_random_factor)) continue;

        //////////// SCHEDULER LOG FLAGS /////////

        if (xp.parse_bool("debug_array", debug_array)) continue;
        if (xp.parse_bool("debug_assignment", debug_assignment)) continue;
        if (xp.parse_bool("debug_credit", debug_credit)) continue;
        if (xp.parse_bool("debug_edf_sim_detail", debug_edf_sim_detail)) continue;
        if (xp.parse_bool("debug_edf_sim_workload", debug_edf_sim_workload)) continue;
        if (xp.parse_bool("debug_fcgi", debug_fcgi)) continue;
        if (xp.parse_bool("debug_handle_results", debug_handle_results)) continue;
        if (xp.parse_bool("debug_locality", debug_locality)) continue;
        if (xp.parse_bool("debug_prefs", debug_prefs)) continue;
        if (xp.parse_bool("debug_quota", debug_quota)) continue;
        if (xp.parse_bool("debug_request_details", debug_request_details)) continue;
        if (xp.parse_bool("debug_request_headers", debug_request_headers)) continue;
        if (xp.parse_bool("debug_resend", debug_resend)) continue;
        if (xp.parse_bool("debug_send", debug_send)) continue;
        if (xp.parse_bool("debug_user_messages", debug_user_messages)) continue;
        if (xp.parse_bool("debug_vda", debug_vda)) continue;
        if (xp.parse_bool("debug_version_select", debug_version_select)) continue;

        if (xp.parse_str("debug_req_reply_dir", debug_req_reply_dir, sizeof(debug_req_reply_dir))) continue;

        // don't complain about unparsed XML;
        // there are lots of tags the scheduler doesn't know about

        xp.skip_unexpected(false, "SCHED_CONFIG::parse");
    }   
    return ERR_XML_PARSE;
}

int SCHED_CONFIG::parse_file(const char* dir) {
    char path[256], path_aux[256];
    int retval;

    if (dir && strlen(dir)) {
        snprintf(path, sizeof(path), "%s/%s", dir, CONFIG_FILE);
        snprintf(path_aux, sizeof(path_aux), "%s/%s", dir, CONFIG_FILE_AUX);
    } else {
        strcpy(path, project_path(CONFIG_FILE));
        strcpy(path_aux, project_path(CONFIG_FILE_AUX));
    }
#ifndef _USING_FCGI_
    FILE* f = fopen(path, "r");
#else
    FCGI_FILE *f = FCGI::fopen(path, "r");
#endif
    if (!f) return ERR_FOPEN;
    retval = parse(f);
    fclose(f);
    if (retval) return retval;

#ifndef _USING_FCGI_
    FILE* f_aux = fopen(path_aux, "r");
#else
    FCGI_FILE *f_aux = FCGI::fopen(path_aux, "r");
#endif
    if (!f_aux) return 0;
    retval = parse_aux(f_aux);
    fclose(f_aux);
    return retval;
}

int SCHED_CONFIG::upload_path(const char* filename, char* path) {
    return dir_hier_path(filename, upload_dir, uldl_dir_fanout, path, true);
}

int SCHED_CONFIG::download_path(const char* filename, char* path) {
    return dir_hier_path(filename, download_dir, uldl_dir_fanout, path, true);
}

static bool is_project_dir(const char* dir) {
    char buf[1024];
    sprintf(buf, "%s/%s", dir, CONFIG_FILE);
    if (!is_file(buf)) return false;
    sprintf(buf, "%s/cgi-bin", dir);
    if (!is_dir(buf)) return false;
    return true;
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
            if (!is_project_dir(p)) {
                fprintf(stderr, "BOINC_PROJECT_DIR env var exists but is not a project dir\n");
                exit(1);
            }
            strlcpy(project_dir, p, sizeof(project_dir));
        } else if (is_project_dir(".")) {
            strcpy(project_dir, ".");
        } else if (is_project_dir("..")) {
            strcpy(project_dir, "..");
        } else {
            fprintf(stderr, "Not in a project directory or subdirectory\n");
            exit(1);
        }
    }

    va_start(ap, fmt);
    snprintf(path, sizeof(path), "%s/", project_dir);
    vsnprintf(path + strlen(path), sizeof(path) - strlen(path), fmt, ap);
    va_end(ap);
    return (const char *)path;
}

const char *BOINC_RCSID_3704204cfd = "$Id$";
