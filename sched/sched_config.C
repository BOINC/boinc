// The contents of this file are subject to the BOINC Public License
// Version 1.0 (the "License"); you may not use this file except in
// compliance with the License. You may obtain a copy of the License at
// http://boinc.berkeley.edu/license_1.0.txt
//
// Software distributed under the License is distributed on an "AS IS"
// basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the
// License for the specific language governing rights and limitations
// under the License.
//
// The Original Code is the Berkeley Open Infrastructure for Network Computing.
//
// The Initial Developer of the Original Code is the SETI@home project.
// Portions created by the SETI@home project are Copyright (C) 2002
// University of California at Berkeley. All Rights Reserved.
//
// Contributor(s):
//

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
static void parse_bool(char* buf, char* tag, bool& result) {
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
    parse_bool(buf, "enforce_delay_bound", enforce_delay_bound);
    parse_bool(buf, "use_transactions", use_transactions);
    parse_int(buf, "<min_sendwork_interval>", min_sendwork_interval);
    parse_int(buf, "<max_wus_to_send>", max_wus_to_send);
    parse_int(buf, "<daily_result_quota>", daily_result_quota);
    parse_int(buf, "<uldl_dir_fanout>", uldl_dir_fanout);
    parse_int(buf, "<locality_scheduling_wait_period>", locality_scheduling_wait_period);
    parse_int(buf, "<locality_scheduling_send_timeout>", locality_scheduling_send_timeout);
    parse_int(buf, "<min_core_client_version>", min_core_client_version);
    if (match_tag(buf, "</config>")) {
        char hostname[256];
        gethostname(hostname, 256);
        if (!strcmp(hostname, db_host)) strcpy(db_host, "localhost");
        return 0;
    }
    return ERR_XML_PARSE;
}

int SCHED_CONFIG::parse_file(char* dir) {
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
