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

// do an account manager RPC;
// if url is null, detach from current account manager
//
int ACCT_MGR_OP::do_rpc(
    std::string url, std::string name, std::string password_hash,
    bool _via_gui
) {
    int retval;
    unsigned int i;
    char buf[256], password[256];
    FILE *pwdf;

    strlcpy(buf, url.c_str(), sizeof(buf));

    error_num = ERR_IN_PROGRESS;
    via_gui = _via_gui;

    if (!strlen(buf) && strlen(gstate.acct_mgr_info.acct_mgr_url)) {
        msg_printf(NULL, MSG_INFO, "Removing account manager info");
        gstate.acct_mgr_info.clear();
        boinc_delete_file(ACCT_MGR_URL_FILENAME);
        boinc_delete_file(ACCT_MGR_LOGIN_FILENAME);
        error_num = 0;
        for (i=0; i<gstate.projects.size(); i++) {
            PROJECT* p = gstate.projects[i];
            p->attached_via_acct_mgr = false;
        }
        return 0;
    }

    canonicalize_master_url(buf);
    if (!valid_master_url(buf)) {
        error_num = ERR_INVALID_URL;
        return 0;
    }

    strlcpy(ami.acct_mgr_url, url.c_str(), sizeof(ami.acct_mgr_url));
    strlcpy(ami.acct_mgr_name, "", sizeof(ami.acct_mgr_name));
    strlcpy(ami.login_name, name.c_str(), sizeof(ami.login_name));
    strlcpy(ami.password_hash, password_hash.c_str(), sizeof(ami.password_hash));

    FILE* f = boinc_fopen(ACCT_MGR_REQUEST_FILENAME, "w");
    if (!f) return ERR_FOPEN;
    fprintf(f,
        "<acct_mgr_request>\n"
        "   <name>%s</name>\n"
        "   <password_hash>%s</password_hash>\n"
        "   <host_cpid>%s</host_cpid>\n"
        "   <domain_name>%s</domain_name>\n"
        "   <client_version>%d.%d.%d</client_version>\n"
        "   <run_mode>%s</run_mode>\n"
        "   <prefs_mod_time>%d</prefs_mod_time>\n"
        "   <prefs_source_project>%s</prefs_source_project>\n",
        name.c_str(), password_hash.c_str(),
        gstate.host_info.host_cpid,
        gstate.host_info.domain_name,
        gstate.core_client_major_version,
        gstate.core_client_minor_version,
        gstate.core_client_release,
        run_mode_name[gstate.user_run_request],
        gstate.global_prefs.mod_time,
        gstate.global_prefs.source_project
    );
    if (strlen(gstate.acct_mgr_info.previous_host_cpid)) {
        fprintf(f,
            "   <previous_host_cpid>%s</previous_host_cpid>\n",
            gstate.acct_mgr_info.previous_host_cpid
        );
    }
    if (gstate.acct_mgr_info.send_gui_rpc_info) {
        // send GUI RPC port and password hash.
        // User must enable this by hand
        // this is for the "farm" account manager so it
        // can know where to send gui rpc requests to
        // without having to configure each host
        //
        if (gstate.cmdline_gui_rpc_port) {
            fprintf(f,"   <gui_rpc_port>%d</gui_rpc_port>\n", gstate.cmdline_gui_rpc_port);
        } else {
            fprintf(f,"   <gui_rpc_port>%d</gui_rpc_port>\n", GUI_RPC_PORT);
        }
        if (boinc_file_exists(GUI_RPC_PASSWD_FILE)) {
            strcpy(password, "");
            pwdf = fopen(GUI_RPC_PASSWD_FILE, "r");
            if (pwdf) {
                fgets(password, 256, pwdf);
                strip_whitespace(password);
                fclose(pwdf);
            }
            fprintf(f,"   <gui_rpc_password>%s</gui_rpc_password>\n", password);
        }
    }
    for (i=0; i<gstate.projects.size(); i++) {
        PROJECT* p = gstate.projects[i];
        fprintf(f,
            "   <project>\n"
            "      <url>%s</url>\n"
            "      <project_name>%s</project_name>\n"
            "      <suspended_via_gui>%d</suspended_via_gui>\n"
            "      <account_key>%s</account_key>\n"
            "      <hostid>%d</hostid>\n"
            "%s"
            "   </project>\n",
            p->master_url,
            p->project_name,
            p->suspended_via_gui,
            p->authenticator,
            p->hostid,
            p->attached_via_acct_mgr?"      <attached_via_acct_mgr/>\n":""
        );
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



int AM_ACCOUNT::parse(FILE* f) {
    char buf[256];
    int retval;

    detach = false;
    update = false;
    url = "";
    strcpy(url_signature, "");
    authenticator = "";

    while (fgets(buf, sizeof(buf), f)) {
        if (match_tag(buf, "</account>")) {
            if (url.length() && authenticator.length()) return 0;
            return ERR_XML_PARSE;
        }
        if (parse_str(buf, "<url>", url)) continue;
        if (match_tag(buf, "<url_signature>")) {
            retval = copy_element_contents(
                f,
                "</url_signature>",
                url_signature,
                sizeof(url_signature)
            );
            if (retval) return retval;
            continue;
        }
        if (parse_str(buf, "<authenticator>", authenticator)) continue;
        if (parse_bool(buf, "detach", detach)) continue;
        if (parse_bool(buf, "update", update)) continue;
    }
    return ERR_XML_PARSE;
}

int ACCT_MGR_OP::parse(FILE* f) {
    char buf[256];
    string message;
    int retval;

    accounts.clear();
    error_str = "";
    error_num = 0;
    repeat_sec = 0;
    while (fgets(buf, sizeof(buf), f)) {
        if (match_tag(buf, "</acct_mgr_reply>")) return 0;
        if (parse_str(buf, "<name>", ami.acct_mgr_name, 256)) continue;
        if (parse_int(buf, "<error_num>", error_num)) continue;
        if (parse_str(buf, "<error>", error_str)) continue;
        if (parse_double(buf, "<repeat_sec>", repeat_sec)) continue;
        if (parse_str(buf, "<message>", message)) {
            msg_printf(NULL, MSG_INFO, "Account manager: %s", message.c_str());
            continue;
        }
        if (match_tag(buf, "<signing_key>")) {
            retval = copy_element_contents(
                f,
                "</signing_key>",
                ami.signing_key,
                sizeof(ami.signing_key)
            );
            if (retval) return retval;
            continue;
        }
        if (match_tag(buf, "<account>")) {
            AM_ACCOUNT account;
            retval = account.parse(f);
            if (!retval) accounts.push_back(account);
            continue;
        }
    }
    return ERR_XML_PARSE;
}

void ACCT_MGR_OP::handle_reply(int http_op_retval) {
    unsigned int i;
    int retval;
    bool verified;
    PROJECT* pp;
    bool sig_ok;

    if (http_op_retval == 0) {
        FILE* f = fopen(ACCT_MGR_REPLY_FILENAME, "r");
        if (f) {
            retval = parse(f);
            fclose(f);
        } else {
            retval = ERR_FOPEN;
        }
    } else {
        error_num = http_op_retval;
    }

    gstate.acct_mgr_info.password_error = false;
    if (error_num == ERR_BAD_PASSWD && !via_gui) {
        gstate.acct_mgr_info.password_error = true;
    }
    // check both error_str and error_num since an account manager may only
    // return a BOINC based error code for password failures or invalid
    // email addresses
    //
    if (error_str.size()) {
        msg_printf(NULL, MSG_ERROR, "Account manager error: %d %s", error_num, error_str.c_str());
        if (!error_num) {
            error_num = ERR_XML_PARSE;
        }
    } else if (error_num) {
        msg_printf(NULL, MSG_ERROR, "Account manager error: %s", boincerror(error_num));
    }

    if (error_num) return;

    msg_printf(NULL, MSG_INFO, "Account manager contact succeeded");

    // demand a signing key
    //
    sig_ok = true;
    if (!strlen(ami.signing_key)) {
        msg_printf(NULL, MSG_ERROR, "No signing key from account manager");
        sig_ok = false;
    }

    // don't accept new signing key if we already have one
    //
    if (strlen(gstate.acct_mgr_info.signing_key)
        && strcmp(gstate.acct_mgr_info.signing_key, ami.signing_key)
    ) {
        msg_printf(NULL, MSG_ERROR, "Inconsistent signing key from account manager");
        sig_ok = false;
    }

    if (sig_ok) {
        strcpy(gstate.acct_mgr_info.acct_mgr_url, ami.acct_mgr_url);
        strcpy(gstate.acct_mgr_info.acct_mgr_name, ami.acct_mgr_name);
        strcpy(gstate.acct_mgr_info.signing_key, ami.signing_key);
        strcpy(gstate.acct_mgr_info.login_name, ami.login_name);
        strcpy(gstate.acct_mgr_info.password_hash, ami.password_hash);

        // attach to new projects
        //
        for (i=0; i<accounts.size(); i++) {
            AM_ACCOUNT& acct = accounts[i];
            retval = verify_string2(acct.url.c_str(), acct.url_signature, ami.signing_key, verified);
            if (retval || !verified) {
                msg_printf(NULL, MSG_ERROR, "Bad signature for URL %s", acct.url.c_str());
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
                        //msg_printf(pp, MSG_INFO, "Already attached");
                        pp->attached_via_acct_mgr = true;

                        // initiate a scheduler RPC if requested by AMS
                        //
                        if (acct.update) {
                            pp->sched_rpc_pending = true;
                            pp->min_rpc_time = 0;
                        }
                    }
                }
            } else {
                if (!acct.detach) {
                    msg_printf(NULL, MSG_INFO,
                        "Attaching to %s", acct.url.c_str()
                    );
                    gstate.add_project(
                        acct.url.c_str(), acct.authenticator.c_str(), true
                    );
                }
            }
        }
    }

    strcpy(gstate.acct_mgr_info.previous_host_cpid, gstate.host_info.host_cpid);
    if (repeat_sec) {
        gstate.acct_mgr_info.next_rpc_time = gstate.now + repeat_sec;
    } else {
        gstate.acct_mgr_info.next_rpc_time = gstate.now + 86400;
    }
    gstate.acct_mgr_info.write_info();
    gstate.set_client_state_dirty("account manager RPC");
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
            if (send_gui_rpc_info) fprintf(p,"    <send_gui_rpc_info/>\n");
            if (strlen(signing_key)) {
                fprintf(p, 
                    "    <signing_key>\n%s</signing_key>\n",
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
                "    <previous_host_cpid>%s</previous_host_cpid>\n"
                "    <next_rpc_time>%f</next_rpc_time>\n"
                "</acct_mgr_login>\n",
                login_name,
                password_hash,
                previous_host_cpid,
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
    strcpy(previous_host_cpid, "");
    next_rpc_time = 0;
    send_gui_rpc_info = false;
    password_error = false;
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
        else if (parse_bool(buf, "send_gui_rpc_info", send_gui_rpc_info)) continue;
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
            else if (parse_str(buf, "<previous_host_cpid>", previous_host_cpid, sizeof(previous_host_cpid))) continue;
            else if (parse_double(buf, "<next_rpc_time>", next_rpc_time)) continue;
        }
        fclose(p);
    }
    return 0;
}

bool ACCT_MGR_INFO::poll() {
    if (gstate.acct_mgr_op.error_num == ERR_IN_PROGRESS) return false;

    // if we do not any any credentials we shouldn't attempt to contact
    // the account manager should should reject us anyway for a bad
    // login.  This also avoids the bug where the content of
    // acct_mgr_url.xml is overwritten with incomplete information such
    // as the account manager name.
    //
    if (!strlen(login_name) && !strlen(password_hash)) return false;

    if (gstate.now > next_rpc_time) {
        next_rpc_time = gstate.now + 86400;
        gstate.acct_mgr_op.do_rpc(
            acct_mgr_url, login_name, password_hash, false
        );
        return true;
    }
    return false;
}

const char *BOINC_RCSID_8fd9e873bf="$Id$";
