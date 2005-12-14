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

#include "parse.h"
#include "error_numbers.h"
#include "client_msgs.h"
#include "util.h"
#include "file_names.h"
#include "filesys.h"
#include "client_state.h"
#include "gui_http.h"
#include "crypt.h"

#include "acct_mgr.h"

static const char *run_mode_name[] = {"", "always", "auto", "never"};

int ACCT_MGR_OP::do_rpc(
    std::string url, std::string name, std::string password_hash
) {
    int retval;
    unsigned int i;
    char buf[256];

    strcpy(buf, url.c_str());

    error_num = ERR_IN_PROGRESS;

    if (!strlen(buf) && strlen(gstate.acct_mgr_info.acct_mgr_url)) {
        msg_printf(NULL, MSG_INFO, "Removing account manager info");
        gstate.acct_mgr_info.clear();
        boinc_delete_file(ACCT_MGR_URL_FILENAME);
        boinc_delete_file(ACCT_MGR_LOGIN_FILENAME);
        error_num = 0;
        return 0;
    }

    canonicalize_master_url(buf);
    if (!valid_master_url(buf)) {
        error_num = ERR_INVALID_URL;
        return 0;
    }
    strcpy(ami.acct_mgr_url, url.c_str());
    strcpy(ami.acct_mgr_name, "");
    strcpy(ami.login_name, name.c_str());
    strcpy(ami.password_hash, password_hash.c_str());

    FILE* f = boinc_fopen(ACCT_MGR_REQUEST_FILENAME, "w");
    if (!f) return ERR_FOPEN;
    fprintf(f,
        "<acct_mgr_request>\n"
        "   <name>%s</name>\n"
        "   <password_hash>%s</password_hash>\n"
        "   <host_cpid>%s</host_cpid>\n"
        "   <client_version>%d.%d.%d</client_version>\n"
        "   <run_mode>%s</run_mode>\n",
        name.c_str(), password_hash.c_str(),
        gstate.host_info.host_cpid,
        gstate.core_client_major_version,
        gstate.core_client_minor_version,
        gstate.core_client_release,
        run_mode_name[gstate.user_run_request]
    );
    for (i=0; i<gstate.projects.size(); i++) {
        PROJECT* p = gstate.projects[i];
        if (p->attached_via_acct_mgr) {
            fprintf(f,
                "   <project>\n"
                "      <url>%s</url>\n"
                "      <project_name>%s</project_name>\n"
                "      <suspended_via_gui>%d</suspended_via_gui>\n"
                "      <account_key>%s</account_key>\n"
                "   </project>\n",
                p->master_url,
                p->project_name,
                p->suspended_via_gui,
                p->authenticator
            );
        }
    }
    fprintf(f, "</acct_mgr_request>\n");
    fclose(f);
    sprintf(buf, "%srpc.php", url.c_str());
    retval = gstate.gui_http.do_rpc_post(
        this, buf, ACCT_MGR_REQUEST_FILENAME, ACCT_MGR_REPLY_FILENAME
    );
    if (retval) {
        error_num = retval;
        return retval;
    }
    msg_printf(NULL, MSG_INFO, "Contacting account manager at %s", url.c_str());

    return 0;
}



int AM_ACCOUNT::parse(MIOFILE& in) {
    char buf[256];
    detach = false;
    url = "";
    url_signature = "";
    authenticator = "";

    while (in.fgets(buf, sizeof(buf))) {
        if (match_tag(buf, "</account>")) {
            if (url.length() && authenticator.length()) return 0;
            return ERR_XML_PARSE;
        }
        if (parse_str(buf, "<url>", url)) continue;
        if (parse_str(buf, "<url_signature>", url_signature)) continue;
        if (parse_str(buf, "<authenticator>", authenticator)) continue;
        if (parse_bool(buf, "detach", detach)) continue;
    }
    return ERR_XML_PARSE;
}

int ACCT_MGR_OP::parse(MIOFILE& in) {
    char buf[256];
    int retval;

    accounts.clear();
    error_str = "";
    repeat_sec = 0;
    while (in.fgets(buf, sizeof(buf))) {
        if (match_tag(buf, "</acct_mgr_reply>")) return 0;
        if (parse_str(buf, "<name>", ami.acct_mgr_name, 256)) continue;
        if (parse_str(buf, "<error>", error_str)) continue;
        if (parse_double(buf, "<repeat_sec>", repeat_sec)) continue;
        if (parse_str(buf, "<signing_key>", ami.signing_key, sizeof(ami.signing_key))) continue;
        if (match_tag(buf, "<account>")) {
            AM_ACCOUNT account;
            retval = account.parse(in);
            if (!retval) accounts.push_back(account);
        }
    }
    return ERR_XML_PARSE;
}

void ACCT_MGR_OP::handle_reply(int http_op_retval) {
    unsigned int i;
    int retval;
    bool verified;
    PROJECT* pp;

    if (http_op_retval == 0) {
        FILE* f = fopen(ACCT_MGR_REPLY_FILENAME, "r");
        if (f) {
            MIOFILE mf;
            mf.init_file(f);
            retval = parse(mf);
            fclose(f);
        } else {
            retval = ERR_FOPEN;
        }
    } else if (error_str.size()) {
        retval = ERR_XML_PARSE;     // ?? what should we use here ??
    } else {
        retval = http_op_retval;
    }
    error_num = retval;
    if (retval) return;

    // demand a signing key
    //
    if (!strlen(ami.signing_key)) {
        msg_printf(NULL, MSG_ERROR, "No signing key from account manager");
        return;
    }

    // don't accept new signing key if we already have one
    //
    if (strlen(gstate.acct_mgr_info.signing_key)
        && strcmp(gstate.acct_mgr_info.signing_key, ami.signing_key)
    ) {
        msg_printf(NULL, MSG_ERROR, "Inconsistent signing key from account manager");
        return;
    }
    gstate.acct_mgr_info = ami;
    gstate.acct_mgr_info.write_info();

    // attach to new projects
    //
    for (i=0; i<accounts.size(); i++) {
        AM_ACCOUNT& acct = accounts[i];
        retval = verify_string2(acct.url.c_str(), acct.url_signature.c_str(), ami.signing_key, verified);
        if (retval || !verified) {
            msg_printf(NULL, MSG_ERROR, "Failed to verify URL %s", acct.url.c_str());
            continue;
        }
        pp = gstate.lookup_project(acct.url.c_str());
        if (pp) {
            if (acct.detach) {
                gstate.detach_project(pp);
            } else {
                if (strcmp(pp->authenticator, acct.authenticator.c_str())) {
                    msg_printf(pp, MSG_ERROR,
                        "Already attached under another account"
                    );
                } else {
                    msg_printf(pp, MSG_INFO, "Already attached");
                    pp->attached_via_acct_mgr = true;
                }
            }
        } else {
            if (!acct.detach) {
                msg_printf(NULL, MSG_INFO, "Attaching to %s", acct.url.c_str());
                gstate.add_project(acct.url.c_str(), acct.authenticator.c_str(), true);
            }
        }
    }

    if (repeat_sec) {
        gstate.acct_mgr_info.next_rpc_time = gstate.now + repeat_sec;
    } else {
        gstate.acct_mgr_info.next_rpc_time = gstate.now + 86400;
    }
    gstate.set_client_state_dirty("account manager RPC");
    gstate.acct_mgr_info.write_info();
}

int ACCT_MGR_INFO::write_info() {
    FILE* p;
    if (strlen(acct_mgr_url)) {
        p = fopen(ACCT_MGR_URL_FILENAME, "w");
        if (p) {
            fprintf(p, 
                "<acct_mgr>\n"
                "    <name>%s</name>\n"
                "    <url>%s</url>\n",
                acct_mgr_name,
                acct_mgr_url
            );
            if (strlen(signing_key)) {
                fprintf(p, 
                    "    <signing_key>%s</signing_key>\n",
                    signing_key
                );
            }
            fprintf(p, 
                "</acct_mgr>\n"
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
                "    <password_hash>%s</password_hash>\n"
                "    <next_rpc_time>%f</next_rpc_time>\n"
                "</acct_mgr_login>\n",
                login_name,
                password_hash,
                next_rpc_time
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
    strcpy(password_hash, "");
    strcpy(signing_key, "");
    next_rpc_time = 0;
}

ACCT_MGR_INFO::ACCT_MGR_INFO() {
    clear();
}

int ACCT_MGR_INFO::init() {
    char    buf[256];
    MIOFILE mf;
    FILE*   p;
    int retval;

    clear();
    p = fopen(ACCT_MGR_URL_FILENAME, "r");
    if (!p) return 0;
    mf.init_file(p);
    while(mf.fgets(buf, sizeof(buf))) {
        if (match_tag(buf, "</acct_mgr>")) break;
        else if (parse_str(buf, "<name>", acct_mgr_name, 256)) continue;
        else if (parse_str(buf, "<url>", acct_mgr_url, 256)) continue;
        else if (match_tag(buf, "<signing_key>")) {
            retval = copy_element_contents(
                p,
                "</signing_key>",
                signing_key,
                sizeof(signing_key)
            );
            if (retval) return retval;
        }
    }
    fclose(p);

    p = fopen(ACCT_MGR_LOGIN_FILENAME, "r");
    if (p) {
        mf.init_file(p);
        while(mf.fgets(buf, sizeof(buf))) {
            if (match_tag(buf, "</acct_mgr_login>")) break;
            else if (parse_str(buf, "<login>", login_name, 256)) continue;
            else if (parse_str(buf, "<password_hash>", password_hash, 256)) continue;
            else if (parse_double(buf, "<next_rpc_time>", next_rpc_time)) continue;
        }
        fclose(p);
    }
    return 0;
}

bool ACCT_MGR_INFO::poll() {
    if (gstate.acct_mgr_op.error_num == ERR_IN_PROGRESS) return false;
    if (gstate.now > next_rpc_time) {
        next_rpc_time = gstate.now + 86400;
        gstate.acct_mgr_op.do_rpc(acct_mgr_url, login_name, password_hash);
        return true;
    }
    return false;
}

const char *BOINC_RCSID_8fd9e873bf="$Id$";
