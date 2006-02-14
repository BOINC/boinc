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

#ifdef _WIN32
#include "boinc_win.h"
#endif

#ifndef _WIN32
#include "config.h"
#endif

#include "client_state.h"
#include "file_names.h"
#include "parse.h"
#include "filesys.h"
#include "util.h"
#include "client_msgs.h"

#include "acct_setup.h"

void PROJECT_INIT::clear() {
    strcpy(url, "");
    strcpy(name, "");
    strcpy(account_key, "");
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
            else if (parse_str(buf, "<url>", url, 256)) {
                canonicalize_master_url(url);
                continue;
            } else if (parse_str(buf, "<account_key>", account_key, 256)) {
                continue;
            }
        }
        fclose(p);
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
    canonicalize_master_url(url);
}

int GET_PROJECT_CONFIG_OP::do_rpc(string master_url) {
    int retval;
    string url = master_url + "get_project_config.php";
    msg_printf(NULL, MSG_INFO,
        "Fetching configuration file from %s", url.c_str()
    );
    retval = gstate.gui_http.do_rpc(this, url, GET_PROJECT_CONFIG_FILENAME);
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
        FILE* f = fopen(GET_PROJECT_CONFIG_FILENAME, "r");
        if (f) {
            file_to_str(f, reply);
            fclose(f);
            error_num = 0;
        } else {
            error_num = ERR_FOPEN;
        }
    }
}

int LOOKUP_ACCOUNT_OP::do_rpc(ACCOUNT_IN& ai) {
    int retval;
    string url;

    url = ai.url + "/lookup_account.php?email_addr="+ai.email_addr+"&passwd_hash="+ai.passwd_hash;
    retval = gstate.gui_http.do_rpc(this, url, LOOKUP_ACCOUNT_FILENAME);
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
        FILE* f = fopen(LOOKUP_ACCOUNT_FILENAME, "r");
        if (f) {
            file_to_str(f, reply);
            fclose(f);
            error_num = 0;
        } else {
            error_num = ERR_FOPEN;
        }
    }
}

int CREATE_ACCOUNT_OP::do_rpc(ACCOUNT_IN& ai) {
    int retval;
    string url;

    url = ai.url + "/create_account.php?email_addr="+ai.email_addr+"&passwd_hash="+ai.passwd_hash+"&user_name="+ai.user_name;
    retval = gstate.gui_http.do_rpc(this, url, CREATE_ACCOUNT_FILENAME);
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
        FILE* f = fopen(CREATE_ACCOUNT_FILENAME, "r");
        if (f) {
            file_to_str(f, reply);
            fclose(f);
            error_num = 0;
        } else {
            error_num = ERR_FOPEN;
        }
    }
}

int LOOKUP_WEBSITE_OP::do_rpc(string& url) {
    int retval;

    msg_printf(0, MSG_INFO, "web site RPC to %s", url.c_str());
    retval = gstate.gui_http.do_rpc(this, url, LOOKUP_WEBSITE_FILENAME);
    if (retval) {
        error_num = retval;
    } else {
        error_num = ERR_IN_PROGRESS;
    }
    return retval;
}

void LOOKUP_WEBSITE_OP::handle_reply(int http_op_retval) {
    error_num = http_op_retval;

    // if we couldn't contact a reference web site,
    // we can assume there's a problem that requires user attention
    // (usually no physical network connection).
    // Set a flag that will signal the Manager to that effect
    //
    if (checking_network) {
        if (http_op_retval) {
            gstate.need_physical_connection = true;
            msg_printf(0, MSG_INFO, "Network check: failure");
        } else {
            msg_printf(0, MSG_INFO, "Network check: success");
        }
        checking_network = false;
    }
}

int GET_CURRENT_VERSION_OP::do_rpc() {
    int retval;
    char buf[256];

    sprintf(buf, "http://boinc.berkeley.edu/download.php?xml=1");
    retval = gstate.gui_http.do_rpc(
        this, string(buf), GET_CURRENT_VERSION_FILENAME
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
    if (maj > gstate.core_client_major_version) return true;
    if (maj < gstate.core_client_major_version) return false;
    if (min > gstate.core_client_minor_version) return true;
    if (min < gstate.core_client_minor_version) return false;
    if (rel > gstate.core_client_release) return true;
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
            same_platform = (strcmp(buf2, gstate.platform_name)==0);
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
    if (f) {
        while (fgets(buf, 256, f)) {
            if (match_tag(buf, "<version>")) {
                if (parse_version(f, new_version)) {
                    msg_printf(0, MSG_INFO,
                        "A new version of BOINC (%s) is available for your computer",
                        new_version
                    );
                    msg_printf(0, MSG_INFO,
                        "Visit http://boinc.berkeley.edu/download.php to get it."
                    );
                    gstate.newer_version = string(new_version);
                    break;
                }
            }
        }
        fclose(f);
    }
}

#define NEW_VERSION_CHECK_PERIOD (14*86400)

void CLIENT_STATE::new_version_check() {
    if (new_version_check_time) {
        if (now - new_version_check_time > NEW_VERSION_CHECK_PERIOD) {
            get_current_version_op.do_rpc();
        }
    } else {
        new_version_check_time = now;
    }
}

const char *BOINC_RCSID_84df3fc17e="$Id$";
