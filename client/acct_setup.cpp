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
#include "project.h"

#include "acct_setup.h"

void ACCOUNT_IN::parse(XML_PARSER& xp) {
    url.clear();
    email_addr.clear();
    passwd_hash.clear();
    user_name.clear();
    team_name.clear();
    server_cookie.clear();
    ldap_auth = false;
    server_assigned_cookie = false;
    consented_to_terms = false;

    while (!xp.get_tag()) {
        if (xp.parse_string("url", url)) continue;
        if (xp.parse_string("email_addr", email_addr)) continue;
        if (xp.parse_string("passwd_hash", passwd_hash)) continue;
        if (xp.parse_string("user_name", user_name)) continue;
        if (xp.parse_string("team_name", team_name)) continue;
        if (xp.parse_string("server_cookie", server_cookie)) continue;
        if (xp.parse_bool("ldap_auth", ldap_auth)) continue;
        if (xp.parse_bool("server_assigned_cookie", server_assigned_cookie)) continue;
        if (xp.parse_bool("consented_to_terms", consented_to_terms)) continue;
    }
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

    retval = gui_http->do_rpc(
        this, url.c_str(), GET_PROJECT_CONFIG_FILENAME, false
    );
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
    url += "lookup_account.php";

    if (ai.ldap_auth && !strchr(ai.email_addr.c_str(), '@')) {
        // LDAP case
        //
        if (!is_https(ai.url.c_str())) return ERR_NEED_HTTPS;
        url += "?ldap_auth=1&ldap_uid=";
        parameter = ai.email_addr;
        escape_url(parameter);
        url += parameter;

        url += "&passwd=";
        parameter = ai.passwd_hash;
        escape_url(parameter);
        url += parameter;
    } else if (ai.server_assigned_cookie) {
        // Project assigned cookie
        //
        url += "?server_assigned_cookie=1&server_cookie=";
        parameter = ai.server_cookie;
        escape_url(parameter);
        url += parameter;
    } else {
        url += "?email_addr=";
        parameter = ai.email_addr;
        escape_url(parameter);
        url += parameter;

        url += "&passwd_hash=";
        parameter = ai.passwd_hash;
        escape_url(parameter);
        url += parameter;
    }

    retval = gui_http->do_rpc(
        this, url.c_str(), LOOKUP_ACCOUNT_FILENAME, false
    );
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

int CREATE_ACCOUNT_OP::do_rpc(ACCOUNT_IN& ai, string rpc_client_name) {
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

    if (ai.consented_to_terms) {
        parameter = rpc_client_name;
        escape_url(parameter);
        url += "&consent_flag=1&source=" + parameter;
    }

    retval = gui_http->do_rpc(
        this, url.c_str(), CREATE_ACCOUNT_FILENAME, false
    );
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

int GET_PROJECT_LIST_OP::do_rpc() {
    int retval;
    char buf[256];

    snprintf(buf, sizeof(buf), "https://boinc.berkeley.edu/project_list.php");
    retval = gui_http->do_rpc(
        this, buf, ALL_PROJECTS_LIST_FILENAME_TEMP, true
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
    error_num = 0;
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
            error_num = ERR_XML_PARSE;
            error = true;
        }
    }
    // if error, try again in a day
    //
    if (error) {
        gstate.all_projects_list_check_time =
            gstate.now - ALL_PROJECTS_LIST_CHECK_PERIOD + SECONDS_PER_DAY;
    }

    // were we initiated by autologin?
    //
    if (gstate.autologin_fetching_project_list) {
        gstate.process_autologin(false);
    }
}

void CLIENT_STATE::all_projects_list_check() {
    if (cc_config.dont_contact_ref_site) return;
    if (get_project_list_op.gui_http->gui_http_state == GUI_HTTP_STATE_BUSY) return;
    if (all_projects_list_check_time) {
        if (now - all_projects_list_check_time < ALL_PROJECTS_LIST_CHECK_PERIOD) {
            return;
        }
    }
    get_project_list_op.do_rpc();
}

// called at startup (first=true)
// or on completion of get project list RPC (first=false).
// check for installer filename file.
// If present, parse project ID and login token,
// and initiate RPC to look up token.
//
void CLIENT_STATE::process_autologin(bool first) {
    static int project_id, user_id;
    static char login_token[256];

    int n, retval;
    char buf[256], *p;

    if (first) {
        // read and parse autologin file
        //
        FILE* f = boinc_fopen(ACCOUNT_DATA_FILENAME, "r");
        if (!f) return;
        p = fgets(buf, 256, f);
        fclose(f);
        if (p == NULL) {
            return;
        }
        p = strstr(buf, "__");
        if (!p) {
            boinc_delete_file(ACCOUNT_DATA_FILENAME);
            return;
        }
        msg_printf(NULL, MSG_INFO, "Read account data file");
        p += 2;
        n = sscanf(p, "%d_%d_%[^. ]", &project_id, &user_id, login_token);
            // don't include the ".exe" or the " (1)"
        if (n != 3) {
            msg_printf(NULL, MSG_INFO, "bad account data: %s", buf);
            boinc_delete_file(ACCOUNT_DATA_FILENAME);
            return;
        }
        strip_whitespace(login_token);
    } else {
        // here the get project list RPC finished.
        // check whether it failed.
        //
        autologin_in_progress = false;
        if (get_project_list_op.error_num) {
            msg_printf(NULL, MSG_INFO,
                "get project list RPC failed: %s",
                boincerror(get_project_list_op.error_num)
            );
            boinc_delete_file(ACCOUNT_DATA_FILENAME);
            return;
        }
    }

    // check that project ID is valid, get URL
    //
    retval = project_list.read_file();       // get project list
    if (retval) {
        msg_printf(NULL, MSG_INFO,
            "Error reading project list: %s", boincerror(retval)
        );
        boinc_delete_file(ACCOUNT_DATA_FILENAME);
        return;
    }
    PROJECT_LIST_ITEM *pli = project_list.lookup(project_id);
    if (!pli) {
        if (first) {
            // we may have an outdated project list.
            // Initiate RPC to get newest version
            //
            retval = get_project_list_op.do_rpc();
            if (retval) {
                msg_printf(NULL, MSG_INFO,
                    "Get project list RPC failed: %s",
                    boincerror(retval)
                );
                boinc_delete_file(ACCOUNT_DATA_FILENAME);
                return;
            }
            autologin_in_progress = true;
                // defer GUI RPCs
            autologin_fetching_project_list = true;
                // tell RPC handler to call us when done
            return;
        } else {
            msg_printf(NULL, MSG_INFO, "Unknown project ID: %d", project_id);
            boinc_delete_file(ACCOUNT_DATA_FILENAME);
            return;
        }
    }

    if (!pli->is_account_manager) {
        if (lookup_project(pli->master_url.c_str())) {
            msg_printf(NULL, MSG_INFO,
                "Already attached to %s", pli->name.c_str()
            );
            boinc_delete_file(ACCOUNT_DATA_FILENAME);
            return;
        }
    }

    // Initiate lookup-token RPC.
    // The reply handler will take it from there.
    //
    msg_printf(NULL, MSG_INFO,
        "Doing token lookup RPC to %s", pli->name.c_str()
    );
    retval = lookup_login_token_op.do_rpc(pli, user_id, login_token);
    if (retval) {
        msg_printf(NULL, MSG_INFO,
            "token lookup RPC failed: %s", boincerror(retval)
        );
        boinc_delete_file(ACCOUNT_DATA_FILENAME);
    }

    // disable GUI RPCs until we get an RPC reply
    //
    gstate.autologin_in_progress = true;
}

int LOOKUP_LOGIN_TOKEN_OP::do_rpc(
    PROJECT_LIST_ITEM* _pli, int user_id, const char* login_token
) {
    char url[1024];
    pli = _pli;
    snprintf(url, sizeof(url), "%slogin_token_lookup.php?user_id=%d&token=%s",
        pli->master_url.c_str(), user_id, login_token
    );
    return gui_http->do_rpc(this, url, LOGIN_TOKEN_LOOKUP_REPLY, false);
}

// Handle lookup login token reply.
// If everything checks out, attach to account manager or project.
//
void LOOKUP_LOGIN_TOKEN_OP::handle_reply(int http_op_retval) {
    string user_name;
    string team_name, authenticator;

    gstate.autologin_in_progress = false;

    if (http_op_retval) {
        msg_printf(NULL, MSG_INFO,
            "token lookup RPC failed: %s", boincerror(http_op_retval)
        );
        return;
    }
    FILE* f = boinc_fopen(LOGIN_TOKEN_LOOKUP_REPLY, "r");
    if (!f) {
        msg_printf(NULL, MSG_INFO, "token lookup RPC: no reply file");
        boinc_delete_file(ACCOUNT_DATA_FILENAME);
        return;
    }
    MIOFILE mf;
    mf.init_file(f);
    XML_PARSER xp(&mf);
    string error_msg;
    while (!xp.get_tag()) {
        if (xp.parse_string("user_name", user_name)) {
            continue;
        } else if (xp.parse_string("team_name", team_name)) {
            continue;
        } else if (xp.parse_string("authenticator", authenticator)) {
            continue;
        } else if (xp.parse_string("error_msg", error_msg)) {
            continue;
        }
    }
    fclose(f);

    if (!user_name.size() || !authenticator.size()) {
        msg_printf(NULL, MSG_INFO, "Account lookup failed: %s", error_msg.c_str());
        boinc_delete_file(ACCOUNT_DATA_FILENAME);
        return;
    }

    if (pli->is_account_manager) {
        msg_printf(NULL, MSG_INFO,
            "Using account manager %s", pli->name.c_str()
        );
        safe_strcpy(gstate.acct_mgr_info.project_name, pli->name.c_str());
        safe_strcpy(gstate.acct_mgr_info.master_url, pli->master_url.c_str());
        safe_strcpy(gstate.acct_mgr_info.user_name, user_name.c_str());
        safe_strcpy(gstate.acct_mgr_info.authenticator, authenticator.c_str());
        gstate.acct_mgr_info.write_info();
    } else {
        msg_printf(NULL, MSG_INFO, "Attaching to project %s", pli->name.c_str());
        gstate.add_project(
            pli->master_url.c_str(), authenticator.c_str(),
            pli->name.c_str(), "", false
        );
        PROJECT *p = gstate.lookup_project(pli->master_url.c_str());
        if (p) {
            safe_strcpy(p->user_name, user_name.c_str());
            safe_strcpy(p->team_name, team_name.c_str());
            xml_unescape(p->user_name);
            xml_unescape(p->team_name);
        }
    }

    // at this point we're done with installer filename.
    //
    boinc_delete_file(ACCOUNT_DATA_FILENAME);
}
