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
    char tag[256], contents[1024];

    memset(this, 0, sizeof(SCHED_CONFIG));
    max_wus_to_send = 10;
    default_disk_max_used_gb = 100.;
    default_disk_max_used_pct = 50.;
    default_disk_min_free_gb = .001;
    sched_debug_level = SCHED_MSG_LOG::MSG_NORMAL;
    fuh_debug_level = SCHED_MSG_LOG::MSG_NORMAL;

    get_tag(f, tag);
    if (strstr(tag, "?xml")) get_tag(f, tag);
    if (strcmp(tag, "boinc")) return ERR_XML_PARSE;
    get_tag(f, tag);
    if (strcmp(tag, "config")) return ERR_XML_PARSE;
    while (get_tag(f, tag, contents)) {
        if (!strcmp(tag, "/config")) {
            char hostname[256];
            gethostname(hostname, 256);
            if (!strcmp(hostname, db_host)) strcpy(db_host, "localhost");
            return 0;
        }
        else if (!strcmp(tag, "master_url")) strcpy(master_url, contents);
        else if (!strcmp(tag, "long_name")) strcpy(long_name, contents);
        else if (!strcmp(tag, "db_name")) strcpy(db_name, contents);
        else if (!strcmp(tag, "db_user")) strcpy(db_user, contents);
        else if (!strcmp(tag, "db_passwd")) strcpy(db_passwd, contents);
        else if (!strcmp(tag, "db_host")) strcpy(db_host, contents);
        else if (!strcmp(tag, "shmem_key")) shmem_key = get_int(contents);
        else if (!strcmp(tag, "key_dir")) strcpy(key_dir, contents);
        else if (!strcmp(tag, "download_url")) strcpy(download_url, contents);
        else if (!strcmp(tag, "download_dir")) strcpy(download_dir, contents);
        else if (!strcmp(tag, "download_dir_alt")) strcpy(download_dir_alt, contents);
        else if (!strcmp(tag, "upload_url")) strcpy(upload_url, contents);
        else if (!strcmp(tag, "upload_dir")) strcpy(upload_dir, contents);
        else if (!strcmp(tag, "sched_lockfile_dir")) strcpy(sched_lockfile_dir, contents);
        else if (!strcmp(tag, "one_result_per_user_per_wu")) one_result_per_user_per_wu = get_bool(contents);
        else if (!strcmp(tag, "non_cpu_intensive")) non_cpu_intensive = get_bool(contents);
        else if (!strcmp(tag, "homogeneous_redundancy")) homogeneous_redundancy = get_bool(contents);
        else if (!strcmp(tag, "locality_scheduling")) locality_scheduling = get_bool(contents);
        else if (!strcmp(tag, "msg_to_host")) msg_to_host = get_bool(contents);
        else if (!strcmp(tag, "ignore_upload_certificates")) ignore_upload_certificates = get_bool(contents);
        else if (!strcmp(tag, "dont_generate_upload_certificates")) dont_generate_upload_certificates = get_bool(contents);
        else if (!strcmp(tag, "ignore_delay_bound")) ignore_delay_bound = get_bool(contents);
        else if (!strcmp(tag, "use_transactions")) use_transactions = get_bool(contents);
        else if (!strcmp(tag, "min_sendwork_interval")) min_sendwork_interval = get_int(contents);
        else if (!strcmp(tag, "max_wus_to_send")) max_wus_to_send = get_int(contents);
        else if (!strcmp(tag, "daily_result_quota")) daily_result_quota = get_int(contents);
        else if (!strcmp(tag, "uldl_dir_fanout")) uldl_dir_fanout = get_int(contents);
        else if (!strcmp(tag, "locality_scheduling_wait_period")) locality_scheduling_wait_period = get_int(contents);
        else if (!strcmp(tag, "locality_scheduling_send_timeout")) locality_scheduling_send_timeout = get_int(contents);
        else if (!strcmp(tag, "min_core_client_version")) min_core_client_version = get_int(contents);
        else if (!strcmp(tag, "min_core_client_version_announced")) min_core_client_version_announced = get_int(contents);
        else if (!strcmp(tag, "min_core_client_upgrade_deadline")) min_core_client_upgrade_deadline = get_int(contents);
        else if (!strcmp(tag, "max_claimed_credit")) max_claimed_credit = get_int(contents);
        else if (!strcmp(tag, "choose_download_url_by_timezone")) choose_download_url_by_timezone = get_bool(contents);
        else if (!strcmp(tag, "cache_md5_info")) cache_md5_info = get_bool(contents);
        else if (!strcmp(tag, "nowork_skip")) nowork_skip = get_bool(contents);
        else if (!strcmp(tag, "resend_lost_results")) resend_lost_results = get_bool(contents);
        else if (!strcmp(tag, "grant_claimed_credit")) grant_claimed_credit = get_bool(contents);
        else if (!strcmp(tag, "fp_benchmark_weight")) {
            fp_benchmark_weight = get_double(contents);
            use_benchmark_weights = true;
        }
        else if (!strcmp(tag, "default_disk_max_used_gb")) default_disk_max_used_gb = get_double(contents);
        else if (!strcmp(tag, "default_disk_max_used_pct")) default_disk_max_used_pct = get_double(contents);
        else if (!strcmp(tag, "default_disk_min_free_gb")) default_disk_min_free_gb = get_double(contents);
        else if (!strcmp(tag, "symstore")) strcpy(symstore, contents);
        else if (!strcmp(tag, "next_rpc_delay")) next_rpc_delay = get_double(contents);
        else if (!strcmp(tag, "dont_delete_batches")) dont_delete_batches = get_bool(contents);
        else if (!strcmp(tag, "sched_debug_level")) sched_debug_level = get_int(contents);
        else if (!strcmp(tag, "fuh_debug_level")) fuh_debug_level = get_int(contents);

        // some tags that scheduler doesn't care about
        //
        else if (!strcmp(tag, "cgi_url")) continue;
        else if (!strcmp(tag, "log_dir")) continue;
        else if (!strcmp(tag, "app_dir")) continue;
        else if (!strcmp(tag, "show_results")) continue;
        else if (!strcmp(tag, "host")) continue;
        else if (!strcmp(tag, "output_level")) continue;
        else if (!strcmp(tag, "profile_screening")) continue;
        else if (!strcmp(tag, "min_passwd_length")) continue;
        else if (!strcmp(tag, "disable_account_creation")) continue;
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
