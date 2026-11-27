// This file is part of BOINC.
// https://boinc.berkeley.edu
// Copyright (C) 2026 University of California
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

// initiate deletion of old sticky files on clients.
//
// There are two approaches; you can use one or the other, not both.
//
// 1) the project supplies a file "file_delete_regex",
//      a list of regular expressions for names of files to be deleted.
//      (this seems odd; shouldn't you specify existing files instead?)
// 2) use a DB table of sticky files (BUDA uses this)

#include <regex.h>
#include <stdio.h>
#include <vector>
#include <string>

#include "sched_config.h"
#include "sched_msgs.h"
#include "sched_types.h"
#include "str_util.h"

#include "sched_files.h"

using std::vector;
using std::string;

vector<regex_t> file_delete_regex;
vector<string> file_delete_regex_string;

int init_sticky_delete_regex() {
    char buf[256];
    FILE* f = boinc::fopen("../file_delete_regex", "r");

    if (!f) return -1;
    while (boinc::fgets(buf, sizeof(buf), f)) {
        strip_whitespace(buf);
        if (!strlen(buf)) continue;
        regex_t re;
        if (regcomp(&re, buf, REG_EXTENDED|REG_NOSUB)) {
            log_messages.printf(MSG_CRITICAL,
                "invalid regular expression in file_delete_regex: %s\n", buf
            );
        } else {
            file_delete_regex.push_back(re);
            file_delete_regex_string.push_back(string(buf));
        }
    }
    boinc::fclose(f);
    return 0;
}

int do_sticky_delete_regex() {
    for (const FILE_INFO& fi: g_request->file_infos) {
        bool found = false;
        for (unsigned int j=0; j<file_delete_regex.size(); j++) {
            regex_t& re = file_delete_regex[j];
            if (!regexec(&re, fi.name, 0, NULL, 0)) {
                g_reply->file_deletes.push_back(fi);
                if (config.debug_client_files) {
                    log_messages.printf(MSG_NORMAL,
                        "[client_files] Sticky file %s matched regex %s; deleting\n",
                        fi.name, file_delete_regex_string[j].c_str()
                    );
                }
                found = true;
                break;
            }
        }
        if (config.debug_client_files && !found) {
            log_messages.printf(MSG_NORMAL,
                "[client_files] Sticky file %s didn't match any regex\n", fi.name
            );
        }
    }
    return 0;
}

/////// DB-based method

int sticky_delete_db() {
    vector<STICKY_FILE> sfs;
    DB_STICKY_FILE sf;
    while (1) {
        int retval = sf.enumerate();
        if (retval == ERR_DB_NOT_FOUND) break;
        if (retval) return retval;
        sfs.push_back(sf);
    }
    for (const FILE_INFO& fi: g_request->file_infos) {
        bool found = false;
        for (STICKY_FILE &sf: sfs) {
            if (!strcmp(sf.physical_name, fi.name)) {
                found = true;
                break;
            }
        }
        if (!found) {
            g_reply->file_deletes.push_back(fi);
        }
    }
    return 0;
}
