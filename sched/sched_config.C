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

#include <stdio.h>
#include <string.h>

#include "parse.h"
#include "error_numbers.h"

#include "sched_config.h"

const char* CONFIG_FILE = "config.xml";

int SCHED_CONFIG::parse(FILE* in) {
    char buf[256];

    memset(this, 0, sizeof(SCHED_CONFIG));
    while (fgets(buf, 256, in)) {
        if (match_tag(buf, "</config>")) return 0;
        else if (parse_str(buf, "<db_name>", db_name, sizeof(db_name))) continue;
        else if (parse_str(buf, "<db_passwd>", db_passwd, sizeof(db_passwd))) continue;
        else if (parse_int(buf, "<shmem_key>", shmem_key)) continue;
        else if (parse_str(buf, "<key_dir>", key_dir, sizeof(key_dir))) continue;
        else if (parse_str(buf, "<download_url>", download_url, sizeof(download_url))) continue;
        else if (parse_str(buf, "<download_dir>", download_dir, sizeof(download_dir))) continue;
        else if (parse_str(buf, "<upload_url>", upload_url, sizeof(upload_url))) continue;
        else if (parse_str(buf, "<upload_dir>", upload_dir, sizeof(upload_dir))) continue;
        else if (parse_str(buf, "<user_name>", user_name, sizeof(user_name))) continue;
    }
    return ERR_XML_PARSE;
}

int SCHED_CONFIG::parse_file(char* dir) {
    FILE* f;
    char path[256];
    int retval;

    sprintf(path, "%s/%s", dir, CONFIG_FILE);
    f = fopen(path, "r");
    if (!f) return ERR_FOPEN;
    retval = parse(f);
    fclose(f);
    return retval;
}
