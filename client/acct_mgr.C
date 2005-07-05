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
#include "parse.h"
#include "error_numbers.h"
#include "client_msgs.h"
#include "util.h"
#include "file_names.h"
#include "filesys.h"
#include "client_state.h"

#include "acct_mgr.h"


int ACCT_MGR::do_rpc(std::string url, std::string name, std::string password) {
    int retval;
    char buf[256];

    if (state != ACCT_MGR_STATE_IDLE) {
        msg_printf(NULL, MSG_ALERT_ERROR, "An account manager update is already in progress");
        return 0;
    }
    strcpy(buf, url.c_str());

    if (!strlen(buf) && strlen(gstate.acct_mgr_info.acct_mgr_url)) {
        msg_printf(NULL, MSG_ALERT_INFO, "Removing account manager info");
        gstate.acct_mgr_info.clear();
        boinc_delete_file(ACCT_MGR_URL_FILENAME);
        boinc_delete_file(ACCT_MGR_LOGIN_FILENAME);
        return 0;
    }

    canonicalize_master_url(buf);
    if (!valid_master_url(buf)) {
        msg_printf(NULL, MSG_ALERT_ERROR, "Can't contact account manager:\n'%s' is not a valid URL", url.c_str());
        return 0;
    }
    strcpy(ami.acct_mgr_url, url.c_str());
    strcpy(ami.acct_mgr_name, "");
    strcpy(ami.login_name, name.c_str());
    strcpy(ami.password, password.c_str());

    sprintf(buf, "%s?name=%s&password=%s", url.c_str(), name.c_str(), password.c_str());
    http_op.set_proxy(&gstate.proxy_info);
    boinc_delete_file(ACCT_MGR_REPLY_FILENAME);
    retval = http_op.init_get(buf, ACCT_MGR_REPLY_FILENAME, true);
    if (!retval) retval = gstate.http_ops->insert(&http_op);
    if (retval) {
        msg_printf(NULL, MSG_ALERT_ERROR,
            "Can't contact account manager at '%s'.\nPlease check the URL and try again",
            url.c_str()
        );
        return retval;
    }
    msg_printf(NULL, MSG_INFO, "Doing account manager RPC to %s", url.c_str());
    state = ACCT_MGR_STATE_BUSY;

    return 0;
}

bool ACCT_MGR::poll() {
    int retval;
    if (state == ACCT_MGR_STATE_IDLE) return false;
    static double last_time=0;
    if (gstate.now-last_time < 1) return false;
    last_time = gstate.now;

    if (http_op.http_op_state == HTTP_STATE_DONE) {
        gstate.http_ops->remove(&http_op);
        if (http_op.http_op_retval == 0) {
            FILE* f = fopen(ACCT_MGR_REPLY_FILENAME, "r");
            if (f) {
                MIOFILE mf;
                mf.init_file(f);
                retval = parse(mf);
                fclose(f);
                if (!retval) {
                    handle_reply();     // this generates messages
                }
            } else {
                retval = ERR_FOPEN;
            }
        } else {
            retval = http_op.http_op_retval;
        }
        if (retval) {
            msg_printf(NULL, MSG_ALERT_ERROR, "Account manager update failed:\n%s", boincerror(retval));
        }
        state = ACCT_MGR_STATE_IDLE;
    }
    return true;
}

int ACCOUNT::parse(MIOFILE& in) {
    char buf[256];
    while (in.fgets(buf, sizeof(buf))) {
        if (match_tag(buf, "</account>")) {
            if (url.length() && authenticator.length()) return 0;
            return ERR_XML_PARSE;
        }
        if (parse_str(buf, "<url>", url)) continue;
        if (parse_str(buf, "<authenticator>", authenticator)) continue;
    }
    return ERR_XML_PARSE;
}

ACCOUNT::ACCOUNT() {}
ACCOUNT::~ACCOUNT() {}

ACCT_MGR::ACCT_MGR() {
    state = ACCT_MGR_STATE_IDLE;
}

ACCT_MGR::~ACCT_MGR() {
}

int ACCT_MGR::parse(MIOFILE& in) {
    char buf[256];
    int retval;

    accounts.clear();
    error_str = "";
    while (in.fgets(buf, sizeof(buf))) {
        if (match_tag(buf, "</acct_mgr_reply>")) return 0;
        if (parse_str(buf, "<name>", ami.acct_mgr_name, 256)) continue;
        if (parse_str(buf, "<error>", error_str)) continue;
        if (match_tag(buf, "<account>")) {
            ACCOUNT account;
            retval = account.parse(in);
            if (!retval) accounts.push_back(account);
        }
    }
    return ERR_XML_PARSE;
}

void ACCT_MGR::handle_reply() {
    unsigned int i;

    msg_printf(NULL, MSG_INFO, "Handling account manager RPC reply");
    if (error_str.size()) {
        msg_printf(NULL, MSG_ALERT_ERROR, "Account manager update failed:\n%s", error_str.c_str());
        return;
    }
    msg_printf(NULL, MSG_ALERT_INFO, "Account manager update succeeded.\nSee Messages for more info.");

    gstate.acct_mgr_info = ami;
    gstate.acct_mgr_info.write_info();

    for (i=0; i<accounts.size(); i++) {
        ACCOUNT& acct = accounts[i];
        PROJECT* pp = gstate.lookup_project(acct.url.c_str());
        if (pp) {
            if (strcmp(pp->authenticator, acct.authenticator.c_str())) {
                msg_printf(pp, MSG_ERROR,
                    "You're attached to this project with a different account"
                );
            } else {
                msg_printf(pp, MSG_INFO, "Already attached");
            }
        } else {
            msg_printf(NULL, MSG_INFO, "Attaching to %s", acct.url.c_str());
            gstate.add_project(acct.url.c_str(), acct.authenticator.c_str(), false);
        }
    }
    if (accounts.size() == 0) {
        msg_printf(NULL, MSG_ERROR, "No accounts reported by account manager");
    }
}

int ACCT_MGR_INFO::write_info() {
    FILE* p;
    if (strlen(acct_mgr_url)) {
        p = fopen(ACCT_MGR_URL_FILENAME, "w");
        if (p) {
            fprintf(
                p, 
                "<acct_mgr>\n"
                "    <name>%s</name>\n"
                "    <url>%s</url>\n"
                "</acct_mgr>\n",
                acct_mgr_name,
                acct_mgr_url
            );
            fclose(p);
        }
    }

    if (strlen(login_name)) {
        p = fopen(ACCT_MGR_LOGIN_FILENAME, "w");
        if (p) {
            fprintf(
                p, 
                "<acct_mgr_login>\n"
                "    <login>%s</login>\n"
                "    <password>%s</password>\n"
                "</acct_mgr_login>\n",
                login_name,
                password
            );
            fclose(p);
        }
    }
    return 0;
}

void ACCT_MGR_INFO::clear() {
    strcpy(acct_mgr_name, "");
    strcpy(acct_mgr_url, "");
    strcpy(login_name, "");
    strcpy(password, "");
}

ACCT_MGR_INFO::ACCT_MGR_INFO() {
    clear();
}

int ACCT_MGR_INFO::init() {
    char    buf[256];
    MIOFILE mf;
    FILE*   p;

    clear();
    p = fopen(ACCT_MGR_URL_FILENAME, "r");
    if (p) {
        mf.init_file(p);
        while(mf.fgets(buf, sizeof(buf))) {
            if (match_tag(buf, "</acct_mgr>")) break;
            else if (parse_str(buf, "<name>", acct_mgr_name, 256)) continue;
            else if (parse_str(buf, "<url>", acct_mgr_url, 256)) continue;
        }
        fclose(p);
    } else {
        return 0;
    }

    p = fopen(ACCT_MGR_LOGIN_FILENAME, "r");
    if (p) {
        mf.init_file(p);
        while(mf.fgets(buf, sizeof(buf))) {
            if (match_tag(buf, "</acct_mgr_login>")) break;
            else if (parse_str(buf, "<login>", login_name, 256)) continue;
            else if (parse_str(buf, "<password>", password, 256)) continue;
        }
        fclose(p);
    }
    return 0;
}
const char *BOINC_RCSID_8fd9e873bf="$Id$";
