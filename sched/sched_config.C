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
    if (match_tag(buf, "<one_result_per_user_per_wu/>")) {
        one_result_per_user_per_wu = true;
    }
    if (match_tag(buf, "<non_cpu_intensive/>")) {
        non_cpu_intensive = true;
    }
    if (match_tag(buf, "<homogeneous_redundancy/>")) {
        homogeneous_redundancy  = true;
    }
    if (match_tag(buf, "<locality_scheduling/>")) {
        locality_scheduling  = true;
    }
    if (match_tag(buf, "<msg_to_host/>")) {
        msg_to_host = true;
    }
    if (match_tag(buf, "<ignore_upload_certificates/>")) {
        ignore_upload_certificates = true;
    }
    if (match_tag(buf, "<dont_generate_upload_certificates/>")) {
        dont_generate_upload_certificates = true;
    }
#if 0
    if (match_tag(buf, "<deletion_policy_priority/>")) {
        deletion_policy_priority = true;
    }
    if (match_tag(buf, "<deletion_policy_expire/>")) {
        deletion_policy_expire = true;
    }
    if (match_tag(buf, "<delete_from_self/>")) {
        delete_from_self = true;
    }
#endif
    if (match_tag(buf, "<enforce_delay_bound/>")) {
        enforce_delay_bound = true;
    }
    if (match_tag(buf, "<use_transactions/>")) {
        use_transactions = true;
    }
    parse_int(buf, "<min_sendwork_interval>", min_sendwork_interval);
    parse_int(buf, "<max_wus_to_send>", max_wus_to_send);
    parse_int(buf, "<daily_result_quota>", daily_result_quota);
    parse_int(buf, "<uldl_dir_fanout>", uldl_dir_fanout);
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
