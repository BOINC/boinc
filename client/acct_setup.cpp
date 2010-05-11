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

#include "cpp.h"

#ifdef _WIN32
#include "boinc_win.h"
#else
#include "config.h"
#include <cstring>
#endif

#include "client_state.h"
#include "file_names.h"
#include "parse.h"
#include "filesys.h"
#include "str_util.h"
#include "url.h"
#include "util.h"
#include "client_msgs.h"
#include "log_flags.h"

#include "acct_setup.h"

void PROJECT_INIT::clear() {
    strcpy(url, "");
    strcpy(name, "");
    strcpy(account_key, "");
    strcpy(team_name, "");
}

PROJECT_INIT::PROJECT_INIT() {
    clear();
}

int PROJECT_INIT::init() {
    char    buf[256];
    MIOFILE mf;
    FILE*   p;

    clear();
    p = fopen(PROJECT_INIT_FILENAME, "r");
    if (p) {
        mf.init_file(p);
        while(mf.fgets(buf, sizeof(buf))) {
            if (match_tag(buf, "</project_init>")) break;
            else if (parse_str(buf, "<name>", name, 256)) continue;
            else if (parse_str(buf, "<team_name>", team_name, 256)) continue;
            else if (parse_str(buf, "<url>", url, 256)) {
                canonicalize_master_url(url);
                continue;
            } else if (parse_str(buf, "<account_key>", account_key, 256)) {
                continue;
            }
        }
        fclose(p);
        msg_printf(0, MSG_INFO, "Found project_init.xml for %s", url);
    }
    return 0;
}

int PROJECT_INIT::remove() {
    clear();
    return boinc_delete_file(PROJECT_INIT_FILENAME);
}

void ACCOUNT_IN::parse(char* buf) {
    url = "";
    email_addr = "";
    passwd_hash = "";
    user_name = "";

    parse_str(buf, "<url>", url);
    parse_str(buf, "<email_addr>", email_addr);
    parse_str(buf, "<passwd_hash>", passwd_hash);
    parse_str(buf, "<user_name>", user_name);
    parse_str(buf, "<team_name>", team_name);
    canonicalize_master_url(url);
}

int GET_PROJECT_CONFIG_OP::do_rpc(string master_url) {
    int retval;
    string url;

    url = master_url;
    canonicalize_master_url(url);

    url += "get_project_config.php";

    msg_printf(NULL, MSG_INFO,
        "Fetching configuration file from %s", url.c_str()
    );

    retval = gui_http->do_rpc(this, (char*)url.c_str(), GET_PROJECT_CONFIG_FILENAME);
    if (retval) {
        error_num = retval;
    } else {
        error_num = ERR_IN_PROGRESS;
    }
    return retval;
}

void GET_PROJECT_CONFIG_OP::handle_reply(int http_op_retval) {
    if (http_op_retval) {
        error_num = http_op_retval;
    } else {
        error_num = read_file_string(GET_PROJECT_CONFIG_FILENAME, reply);
    }
}

int LOOKUP_ACCOUNT_OP::do_rpc(ACCOUNT_IN& ai) {
    int retval;
    string url;
    string parameter;

    url = ai.url;
    canonicalize_master_url(url);

    url += "lookup_account.php?email_addr=";
    parameter = ai.email_addr;
    escape_url(parameter);
    url += parameter;

    url += "&passwd_hash=";
    parameter = ai.passwd_hash;
    escape_url(parameter);
    url += parameter;

    retval = gui_http->do_rpc(this, (char*)url.c_str(), LOOKUP_ACCOUNT_FILENAME);
    if (retval) {
        error_num = retval;
    } else {
        error_num = ERR_IN_PROGRESS;
    }
    return retval;
}

void LOOKUP_ACCOUNT_OP::handle_reply(int http_op_retval) {
    if (http_op_retval) {
        error_num = http_op_retval;
    } else {
        error_num = read_file_string(LOOKUP_ACCOUNT_FILENAME, reply);
    }
}

int CREATE_ACCOUNT_OP::do_rpc(ACCOUNT_IN& ai) {
    int retval;
    string url;
    string parameter;

    url = ai.url;
    canonicalize_master_url(url);

    url += "create_account.php?email_addr=";
    parameter = ai.email_addr;
    escape_url(parameter);
    url += parameter;

    url += "&passwd_hash=";
    parameter = ai.passwd_hash;
    escape_url(parameter);
    url += parameter;

    url += "&user_name=";
    parameter = ai.user_name;
    escape_url(parameter);
    url += parameter;

    if (!ai.team_name.empty()) {
        url += "&team_name=";
        parameter = ai.team_name;
        escape_url(parameter);
        url += parameter;
    }
    retval = gui_http->do_rpc(this, (char*)url.c_str(), CREATE_ACCOUNT_FILENAME);
    if (retval) {
        error_num = retval;
    } else {
        error_num = ERR_IN_PROGRESS;
    }
    return retval;
}

void CREATE_ACCOUNT_OP::handle_reply(int http_op_retval) {
    if (http_op_retval) {
        error_num = http_op_retval;
    } else {
        error_num = read_file_string(CREATE_ACCOUNT_FILENAME, reply);
    }
}

int GET_CURRENT_VERSION_OP::do_rpc() {
    int retval;

    retval = gui_http->do_rpc(
        this, (char*)config.client_version_check_url.c_str(), GET_CURRENT_VERSION_FILENAME
    );
    if (retval) {
        error_num = retval;
    } else {
        error_num = ERR_IN_PROGRESS;
    }
    return retval;
}

static bool is_version_newer(char* p) {
    int maj=0, min=0, rel=0;

    sscanf(p, "%d.%d.%d", &maj, &min, &rel);
    if (maj > gstate.core_client_version.major) return true;
    if (maj < gstate.core_client_version.major) return false;
    if (min > gstate.core_client_version.minor) return true;
    if (min < gstate.core_client_version.minor) return false;
    if (rel > gstate.core_client_version.release) return true;
    return false;
}

static bool parse_version(FILE* f, char* new_version) {
    char buf[256], buf2[256];
    bool same_platform = false, newer_version = false;
    while (fgets(buf, 256, f)) {
        if (match_tag(buf, "</version>")) {
            return (same_platform && newer_version);
        }
        if (parse_str(buf, "<dbplatform>", buf2, sizeof(buf2))) {
            same_platform = (strcmp(buf2, gstate.get_primary_platform())==0);
        }
        if (parse_str(buf, "<version_num>", buf2, sizeof(buf2))) {
            newer_version = is_version_newer(buf2);
            strcpy(new_version, buf2);
        }
    }
    return false;
}

void GET_CURRENT_VERSION_OP::handle_reply(int http_op_retval) {
    char buf[256], new_version[256];
    if (http_op_retval) {
        error_num = http_op_retval;
        return;
    }
    gstate.new_version_check_time = gstate.now;
    FILE* f = boinc_fopen(GET_CURRENT_VERSION_FILENAME, "r");
    if (!f) return;
    while (fgets(buf, 256, f)) {
        if (match_tag(buf, "<version>")) {
            if (parse_version(f, new_version)) {
                msg_printf(0, MSG_USER_ALERT,
                    "A new version of BOINC (%s) is available",
                    new_version
                );

                msg_printf(0, MSG_USER_ALERT,
                    "Visit %s to download it",
                    config.client_download_url.c_str()
                );
                gstate.newer_version = string(new_version);
                break;
            }
        }
    }
    fclose(f);
}

#define NEW_VERSION_CHECK_PERIOD (14*86400)

void CLIENT_STATE::new_version_check() {
    if (( new_version_check_time == 0) ||
        (now - new_version_check_time > NEW_VERSION_CHECK_PERIOD)) {
            // get_current_version_op.handle_reply() will update new_version_check_time
            get_current_version_op.do_rpc();
        }
}

int GET_PROJECT_LIST_OP::do_rpc() {
    int retval;
    char buf[256];

    sprintf(buf, "http://boinc.berkeley.edu/project_list.php");
    retval = gui_http->do_rpc(
        this, buf, ALL_PROJECTS_LIST_FILENAME_TEMP
    );
    if (retval) {
        error_num = retval;
    } else {
        error_num = ERR_IN_PROGRESS;
    }
    return retval;
}

#define ALL_PROJECTS_LIST_CHECK_PERIOD (14*86400)

void GET_PROJECT_LIST_OP::handle_reply(int http_op_retval) {
    bool error = false;
    if (http_op_retval) {
        error_num = http_op_retval;
        error = true;
    } else {
        string s;
        read_file_string(ALL_PROJECTS_LIST_FILENAME_TEMP, s);
        if (strstr(s.c_str(), "</projects>")) {
            boinc_rename(
                ALL_PROJECTS_LIST_FILENAME_TEMP, ALL_PROJECTS_LIST_FILENAME
            );
            gstate.all_projects_list_check_time = gstate.now;
        } else {
            error = true;
        }
    }
    // if error, try again in a day
    //
    if (error) {
        gstate.all_projects_list_check_time =
            gstate.now - ALL_PROJECTS_LIST_CHECK_PERIOD + SECONDS_PER_DAY;
    }
}

void CLIENT_STATE::all_projects_list_check() {
    if (config.dont_contact_ref_site) return;
    if (all_projects_list_check_time) {
        if (now - all_projects_list_check_time < ALL_PROJECTS_LIST_CHECK_PERIOD) {
            return;
        }
    }
    get_project_list_op.do_rpc();
}

