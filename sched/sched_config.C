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
    }   
    return ERR_XML_PARSE;
}

#if 0
int SCHED_CONFIG::parse(char* buf) {
    // fill in defaults
    //
    memset(this, 0, sizeof(SCHED_CONFIG));
    max_wus_to_send = 10;
    default_disk_max_used_gb = 100.;
    default_disk_max_used_pct = 50.;
    default_disk_min_free_gb = .001;

    parse_str(buf, "<master_url>", master_url, sizeof(master_url));
    parse_str(buf, "<long_name>", long_name, sizeof(long_name));
    parse_str(buf, "<db_name>", db_name, sizeof(db_name));
    parse_str(buf, "<db_user>", db_user, sizeof(db_user));
    parse_str(buf, "<db_passwd>", db_passwd, sizeof(db_passwd));
    parse_str(buf, "<db_host>", db_host, sizeof(db_host));
    parse_int(buf, "<shmem_key>", shmem_key);
    parse_str(buf, "<key_dir>", key_dir, sizeof(key_dir));
    parse_str(buf, "<download_url>", download_url, sizeof(download_url));
    parse_str(buf, "<download_dir>", download_dir, sizeof(download_dir));
    parse_str(buf, "<download_dir_alt>", download_dir_alt, sizeof(download_dir_alt));
    parse_str(buf, "<upload_url>", upload_url, sizeof(upload_url));
    parse_str(buf, "<upload_dir>", upload_dir, sizeof(upload_dir));
    parse_str(buf, "<sched_lockfile_dir>", sched_lockfile_dir, sizeof(sched_lockfile_dir));
    parse_bool(buf, "one_result_per_user_per_wu", one_result_per_user_per_wu);
    parse_bool(buf, "non_cpu_intensive", non_cpu_intensive);
    parse_bool(buf, "homogeneous_redundancy", homogeneous_redundancy);
    parse_bool(buf, "locality_scheduling", locality_scheduling);
    parse_bool(buf, "msg_to_host", msg_to_host);
    parse_bool(buf, "ignore_upload_certificates", ignore_upload_certificates);
    parse_bool(buf, "dont_generate_upload_certificates", dont_generate_upload_certificates);
    parse_bool(buf, "ignore_delay_bound", ignore_delay_bound);
    parse_bool(buf, "use_transactions", use_transactions);
    parse_int(buf, "<min_sendwork_interval>", min_sendwork_interval);
    parse_int(buf, "<max_wus_to_send>", max_wus_to_send);
    parse_int(buf, "<daily_result_quota>", daily_result_quota);
    parse_int(buf, "<uldl_dir_fanout>", uldl_dir_fanout);
    parse_int(buf, "<locality_scheduling_wait_period>", locality_scheduling_wait_period);
    parse_int(buf, "<locality_scheduling_send_timeout>", locality_scheduling_send_timeout);
    parse_int(buf, "<min_core_client_version>", min_core_client_version);
    parse_int(buf, "<min_core_client_version_announced>", min_core_client_version_announced);
    parse_int(buf, "<min_core_client_upgrade_deadline>", min_core_client_upgrade_deadline);
    parse_int(buf, "<max_claimed_credit>", max_claimed_credit);
    parse_bool(buf, "choose_download_url_by_timezone", choose_download_url_by_timezone);
    parse_bool(buf, "cache_md5_info", cache_md5_info);
    parse_bool(buf, "nowork_skip", nowork_skip);
    parse_bool(buf, "resend_lost_results", resend_lost_results);
    parse_bool(buf, "grant_claimed_credit", grant_claimed_credit);
    if (parse_double(buf, "<fp_benchmark_weight>", fp_benchmark_weight)) {
        use_benchmark_weights = true;
    }
    parse_double(buf, "<default_disk_max_used_gb>", default_disk_max_used_gb);
    parse_double(buf, "<default_disk_max_used_pct>", default_disk_max_used_pct);
    parse_double(buf, "<default_disk_min_free_gb>", default_disk_min_free_gb);
    parse_str(buf, "<symstore>", symstore, sizeof(symstore));

    if (match_tag(buf, "</config>")) {
        char hostname[256];
        gethostname(hostname, 256);
        if (!strcmp(hostname, db_host)) strcpy(db_host, "localhost");
        return 0;
    }
    return ERR_XML_PARSE;
}
#endif

int SCHED_CONFIG::parse_file(const char* dir) {
    char* p;
    char path[256];
    int retval;

    sprintf(path, "%s/%s", dir, CONFIG_FILE);
#if 0
    retval = read_file_malloc(path, p);
    if (retval) return retval;
    retval =  parse(p);
    free(p);
    return retval;
#endif
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
