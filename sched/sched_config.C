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
#include <fstream>
using std::ifstream;

#include "parse.h"
#include "error_numbers.h"

#include "sched_config.h"

const char* CONFIG_FILE = "config.xml";

// ??? why not use read_file_malloc()
//
inline string read_stream(istream& f) {
    string buf;
    buf.reserve(8192);
    char c;
    while (f >> c)
        buf += c;
    return buf;
}

int SCHED_CONFIG::parse(istream& f) {
    string buf = read_stream(f);

    // fill in defaults
    //
    memset(this, 0, sizeof(SCHED_CONFIG));
    max_wus_to_send = 10;

    parse_str(buf.c_str(), "<db_name>", db_name, sizeof(db_name));
    parse_str(buf.c_str(), "<db_user>", db_user, sizeof(db_user));
    parse_str(buf.c_str(), "<db_passwd>", db_passwd, sizeof(db_passwd));
    parse_str(buf.c_str(), "<db_host>", db_host, sizeof(db_host));
    parse_int(buf.c_str(), "<shmem_key>", shmem_key);
    parse_str(buf.c_str(), "<key_dir>", key_dir, sizeof(key_dir));
    parse_str(buf.c_str(), "<download_url>", download_url, sizeof(download_url));
    parse_str(buf.c_str(), "<download_dir>", download_dir, sizeof(download_dir));
    parse_str(buf.c_str(), "<upload_url>", upload_url, sizeof(upload_url));
    parse_str(buf.c_str(), "<upload_dir>", upload_dir, sizeof(upload_dir));
    if (match_tag(buf.c_str(), "<one_result_per_user_per_wu/>")) {
        one_result_per_user_per_wu = true;
    }
    if (match_tag(buf.c_str(), "<trickle_down/>")) {
        trickle_down = true;
    }
    parse_int(buf.c_str(), "<min_sendwork_interval>", min_sendwork_interval);
    parse_int(buf.c_str(), "<max_wus_to_send>", max_wus_to_send);
    if (match_tag(buf.c_str(), "</config>")) return 0;
    return ERR_XML_PARSE;
}

int SCHED_CONFIG::parse_file(char* dir) {
    ifstream f;
    char path[256];

    sprintf(path, "%s/%s", dir, CONFIG_FILE);
    f.open(path);
    if (!f) return ERR_FOPEN;
    return parse(f);
}

void get_project_dir(char* p, int len) {
    getcwd(p, len);
    char* q = strrchr(p, '/');
    if (q) *q = 0;
}
