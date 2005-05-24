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

// look for a boolean; accepts either <foobar/> or <foobar>1</foobar>
//
static void parse_bool(char* buf, const char* tag, bool& result) {
    char single_tag[256], start_tag[256];
    int x;

    sprintf(single_tag, "<%s/>", tag);
    if (match_tag(buf, single_tag)) {
        result = true;
        return;
    }
    sprintf(start_tag, "<%s>", tag);
    if (parse_int(buf, start_tag, x)) {
        result = (x != 0);
    }
}

int SCHED_CONFIG::parse(char* buf) {
    // fill in defaults
    //
    memset(this, 0, sizeof(SCHED_CONFIG));
    max_wus_to_send = 10;

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
    parse_bool(buf,"homogeneous_redundancy", homogeneous_redundancy);
    parse_bool(buf, "locality_scheduling", locality_scheduling);
    parse_bool(buf, "msg_to_host", msg_to_host);
    parse_bool(buf, "ignore_upload_certificates", ignore_upload_certificates);
    parse_bool(buf, "dont_generate_upload_certificates", dont_generate_upload_certificates);
#if 0
    parse_bool(buf, "deletion_policy_priority", deletion_policy_priority);
    parse_bool(buf, "deletion_policy_expire", deletion_policy_expire);
    parse_bool(buf, "delete_from_self", delete_from_self);
#endif
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
    parse_bool(buf, "choose_download_url_by_timezone", choose_download_url_by_timezone);
    parse_bool(buf, "cache_md5_info", cache_md5_info);
    parse_bool(buf, "nowork_skip", nowork_skip);

    if (match_tag(buf, "</config>")) {
        char hostname[256];
        gethostname(hostname, 256);
        if (!strcmp(hostname, db_host)) strcpy(db_host, "localhost");
        return 0;
    }
    return ERR_XML_PARSE;
}

int SCHED_CONFIG::parse_file(const char* dir) {
    char* p;
    char path[256];
    int retval;

    sprintf(path, "%s/%s", dir, CONFIG_FILE);
    retval = read_file_malloc(path, p);
    if (retval) return retval;
    retval =  parse(p);
    free(p);
    return retval;
}

void get_project_dir(char* p, int len) {
    getcwd(p, len);
    char* q = strrchr(p, '/');
    if (q) *q = 0;
}

const char *BOINC_RCSID_3704204cfd = "$Id$";
