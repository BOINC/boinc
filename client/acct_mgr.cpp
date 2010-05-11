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

#include "parse.h"
#include "error_numbers.h"
#include "client_msgs.h"
#include "str_util.h"
#include "str_replace.h"
#include "url.h"
#include "file_names.h"
#include "filesys.h"
#include "client_state.h"
#include "gui_http.h"
#include "crypt.h"

#include "acct_mgr.h"

static const char *run_mode_name[] = {"", "always", "auto", "never"};

ACCT_MGR_OP::ACCT_MGR_OP() {
    global_prefs_xml = 0;
}

// do an account manager RPC;
// if URL is null, detach from current account manager
//
int ACCT_MGR_OP::do_rpc(
    std::string _url, std::string name, std::string password_hash,
    bool _via_gui
) {
    int retval;
    unsigned int i;
    char url[256], password[256], buf[256];
    FILE *pwdf;

    strlcpy(url, _url.c_str(), sizeof(url));

    error_num = ERR_IN_PROGRESS;
    via_gui = _via_gui;
    if (global_prefs_xml) {
        free(global_prefs_xml);
        global_prefs_xml = 0;
    }

    // if null URL, detach from current AMS
    //
    if (!strlen(url) && strlen(gstate.acct_mgr_info.acct_mgr_url)) {
        msg_printf(NULL, MSG_INFO, "Removing account manager info");
        gstate.acct_mgr_info.clear();
        boinc_delete_file(ACCT_MGR_URL_FILENAME);
        boinc_delete_file(ACCT_MGR_LOGIN_FILENAME);
        error_num = 0;
        for (i=0; i<gstate.projects.size(); i++) {
            PROJECT* p = gstate.projects[i];
            p->attached_via_acct_mgr = false;
            p->ams_resource_share = -1;
        }
        return 0;
    }

    canonicalize_master_url(url);
    if (!valid_master_url(url)) {
        error_num = ERR_INVALID_URL;
        return 0;
    }

    strlcpy(ami.acct_mgr_url, url, sizeof(ami.acct_mgr_url));
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
        "   <run_mode>%s</run_mode>\n",
        name.c_str(), password_hash.c_str(),
        gstate.host_info.host_cpid,
        gstate.host_info.domain_name,
        gstate.core_client_version.major,
        gstate.core_client_version.minor,
        gstate.core_client_version.release,
        run_mode_name[gstate.run_mode.get_perm()]
    );
    if (strlen(gstate.acct_mgr_info.previous_host_cpid)) {
        fprintf(f,
            "   <previous_host_cpid>%s</previous_host_cpid>\n",
            gstate.acct_mgr_info.previous_host_cpid
        );
    }

    // If the AMS requested it, send GUI RPC port and password hash.
    // This is for the "farm" account manager so it
    // can know where to send GUI RPC requests to
    // without having to configure each host
    //
    if (gstate.acct_mgr_info.send_gui_rpc_info) {
        if (gstate.cmdline_gui_rpc_port) {
            fprintf(f,"   <gui_rpc_port>%d</gui_rpc_port>\n", gstate.cmdline_gui_rpc_port);
        } else {
            fprintf(f,"   <gui_rpc_port>%d</gui_rpc_port>\n", GUI_RPC_PORT);
        }
        if (boinc_file_exists(GUI_RPC_PASSWD_FILE)) {
            strcpy(password, "");
            pwdf = fopen(GUI_RPC_PASSWD_FILE, "r");
            if (pwdf) {
                if (fgets(password, 256, pwdf)) {
                    strip_whitespace(password);
                }
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
    if (boinc_file_exists(GLOBAL_PREFS_FILE_NAME)) {
        FILE* fprefs = fopen(GLOBAL_PREFS_FILE_NAME, "r");
        if (fprefs) {
            copy_stream(fprefs, f);
            fclose(fprefs);
        }
    }
	if (strlen(gstate.acct_mgr_info.opaque)) {
		fprintf(f,
			"   <opaque>\n%s\n"
			"   </opaque>\n",
			gstate.acct_mgr_info.opaque
		);
	}
    fprintf(f, "</acct_mgr_request>\n");
    fclose(f);
    sprintf(buf, "%srpc.php", url);
    retval = gstate.gui_http.do_rpc_post(
        this, buf, ACCT_MGR_REQUEST_FILENAME, ACCT_MGR_REPLY_FILENAME
    );
    if (retval) {
        error_num = retval;
        return retval;
    }
    msg_printf(NULL, MSG_INFO, "Contacting account manager at %s", url);

    return 0;
}

int AM_ACCOUNT::parse(XML_PARSER& xp) {
    char tag[256];
	bool is_tag, btemp;
	int retval;
    double dtemp;

    detach = false;
    update = false;
    dont_request_more_work.init();
    detach_when_done.init();
    url = "";
    strcpy(url_signature, "");
    authenticator = "";
    resource_share.init();

    while (!xp.get(tag, sizeof(tag), is_tag)) {
		if (!is_tag) {
			if (log_flags.unparsed_xml) {
				msg_printf(0, MSG_INFO,
                    "[unparsed_xml] AM_ACCOUNT::parse: unexpected text %s",
                    tag
                );
			}
			continue;
		}
        if (!strcmp(tag, "/account")) {
            if (url.length()) return 0;
            return ERR_XML_PARSE;
        }
        if (xp.parse_string(tag, "url", url)) continue;
        if (!strcmp(tag, "url_signature")) {
            retval = xp.element_contents("</url_signature>", url_signature, sizeof(url_signature));
            if (retval) return retval;
			strcat(url_signature, "\n");
            continue;
        }
        if (xp.parse_string(tag, "authenticator", authenticator)) continue;
        if (xp.parse_bool(tag, "detach", detach)) continue;
        if (xp.parse_bool(tag, "update", update)) continue;
        if (xp.parse_bool(tag, "dont_request_more_work", btemp)) {
            dont_request_more_work.set(btemp);
            continue;
        }
        if (xp.parse_bool(tag, "detach_when_done", btemp)) {
            detach_when_done.set(btemp);
            continue;
        }
        if (xp.parse_double(tag, "resource_share", dtemp)) {
            if (dtemp >= 0) {
                resource_share.set(dtemp);
            } else {
                msg_printf(NULL, MSG_INFO,
                    "Resource share out of range: %f", dtemp
                );
            }
            continue;
        }
        if (log_flags.unparsed_xml) {
            msg_printf(NULL, MSG_INFO,
                "[unparsed_xml] AM_ACCOUNT: unrecognized %s", tag
            );
        }
        xp.skip_unexpected(tag, log_flags.unparsed_xml, "AM_ACCOUNT::parse");
    }
    return ERR_XML_PARSE;
}

int ACCT_MGR_OP::parse(FILE* f) {
    char tag[1024];
	bool is_tag;
    string message;
    int retval;
	MIOFILE mf;
	mf.init_file(f);
	XML_PARSER xp(&mf);

    accounts.clear();
    error_str = "";
    error_num = 0;
    repeat_sec = 0;
    strcpy(host_venue, "");
	strcpy(ami.opaque, "");
	if (!xp.parse_start("acct_mgr_reply")) return ERR_XML_PARSE;
    while (!xp.get(tag, sizeof(tag), is_tag)) {
		if (!is_tag) {
			if (log_flags.unparsed_xml) {
				msg_printf(0, MSG_INFO,
                    "[unparsed_xml] ACCT_MGR_OP::parse: unexpected text %s",
                    tag
                );
			}
			continue;
		}
        if (!strcmp(tag, "/acct_mgr_reply")) return 0;
        if (xp.parse_str(tag, "name", ami.acct_mgr_name, 256)) continue;
        if (xp.parse_int(tag, "error_num", error_num)) continue;
        if (xp.parse_string(tag, "error", error_str)) continue;
        if (xp.parse_double(tag, "repeat_sec", repeat_sec)) continue;
        if (xp.parse_string(tag, "message", message)) {
            msg_printf(NULL, MSG_INFO, "Account manager: %s", message.c_str());
            continue;
        }
		if (!strcmp(tag, "opaque")) {
			retval = xp.element_contents("</opaque>", ami.opaque, sizeof(ami.opaque));
			if (retval) return retval;
			continue;
		}
        if (!strcmp(tag, "signing_key")) {
            retval = xp.element_contents("</signing_key>", ami.signing_key, sizeof(ami.signing_key));
            if (retval) return retval;
            continue;
        }
        if (!strcmp(tag, "account")) {
            AM_ACCOUNT account;
            retval = account.parse(xp);
            if (retval) {
                msg_printf(NULL, MSG_INTERNAL_ERROR,
                    "Can't parse account in account manager reply: %s",
                    boincerror(retval)
                );
            } else {
                accounts.push_back(account);
            }
            continue;
        }
        if (!strcmp(tag, "global_preferences")) {
            retval = dup_element_contents(
                f,
                "</global_preferences>",
                &global_prefs_xml
            );
            if (retval) {
                msg_printf(NULL, MSG_INTERNAL_ERROR,
                    "Can't parse global prefs in account manager reply: %s",
                    boincerror(retval)
                );
                return retval;
            }
            continue;
        }
        if (xp.parse_str(tag, "host_venue", host_venue, sizeof(host_venue))) continue;
        if (log_flags.unparsed_xml) {
            msg_printf(NULL, MSG_INFO,
                "[unparsed_xml] ACCT_MGR_OP::parse: unrecognized %s", tag
            );
        }
        xp.skip_unexpected(tag, log_flags.unparsed_xml, "ACCT_MGR_OP::parse");
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
        msg_printf(NULL, MSG_USER_ALERT,
            "Account manager error: %d %s", error_num, error_str.c_str()
        );
        if (!error_num) {
            error_num = ERR_XML_PARSE;
        }
    } else if (error_num) {
        msg_printf(NULL, MSG_USER_ALERT,
            "Account manager error: %s", boincerror(error_num)
        );
    }

    if (error_num) return;

    msg_printf(NULL, MSG_INFO, "Account manager contact succeeded");

    // demand a signing key
    //
    sig_ok = true;
    if (!strlen(ami.signing_key)) {
        msg_printf(NULL, MSG_INTERNAL_ERROR,
            "No signing key from account manager"
        );
        sig_ok = false;
    }

    // don't accept new signing key if we already have one
    //
    if (strlen(gstate.acct_mgr_info.signing_key)
        && strcmp(gstate.acct_mgr_info.signing_key, ami.signing_key)
    ) {
        msg_printf(NULL, MSG_INTERNAL_ERROR,
            "Inconsistent signing key from account manager"
        );
        sig_ok = false;
    }

    if (sig_ok) {
        strcpy(gstate.acct_mgr_info.acct_mgr_url, ami.acct_mgr_url);
        strcpy(gstate.acct_mgr_info.acct_mgr_name, ami.acct_mgr_name);
        strcpy(gstate.acct_mgr_info.signing_key, ami.signing_key);
        strcpy(gstate.acct_mgr_info.login_name, ami.login_name);
        strcpy(gstate.acct_mgr_info.password_hash, ami.password_hash);
		strcpy(gstate.acct_mgr_info.opaque, ami.opaque);

        // process projects
        //
        for (i=0; i<accounts.size(); i++) {
            AM_ACCOUNT& acct = accounts[i];
            retval = verify_string2(acct.url.c_str(), acct.url_signature, ami.signing_key, verified);
            if (retval || !verified) {
                msg_printf(NULL, MSG_INTERNAL_ERROR,
                    "Bad signature for URL %s", acct.url.c_str()
                );
                continue;
            }
            pp = gstate.lookup_project(acct.url.c_str());
            if (pp) {
                if (acct.detach) {
                    gstate.detach_project(pp);
                } else {
                    // BAM! leaves authenticator blank if our request message
                    // had the current account info
                    //
                    if (acct.authenticator.size() && strcmp(pp->authenticator, acct.authenticator.c_str())) {
                        msg_printf(pp, MSG_INFO,
                            "Already attached under another account"
                        );
                    } else {
                        //msg_printf(pp, MSG_INFO, "Already attached");
                        pp->attached_via_acct_mgr = true;
                        if (acct.dont_request_more_work.present) {
                            pp->dont_request_more_work = acct.dont_request_more_work.value;
                        }
                        if (acct.detach_when_done.present) {
                            pp->detach_when_done = acct.detach_when_done.value;
                            if (pp->detach_when_done) {
                                pp->dont_request_more_work = true;
                            }
                        }

                        // initiate a scheduler RPC if requested by AMS
                        //
                        if (acct.update) {
                            pp->sched_rpc_pending = RPC_REASON_ACCT_MGR_REQ;
                            pp->min_rpc_time = 0;
                        }
                        if (acct.resource_share.present) {
                            pp->ams_resource_share = acct.resource_share.value;
                            pp->resource_share = pp->ams_resource_share;
                        } else {
                            // no host-specific resource share;
                            // if currently have one, restore to value from web
                            //
                            if (pp->ams_resource_share >= 0) {
                                pp->ams_resource_share = -1;
                                PROJECT p2;
                                strcpy(p2.master_url, pp->master_url);
                                retval = p2.parse_account_file();
                                if (!retval) {
                                    pp->resource_share = p2.resource_share;
                                } else {
                                    pp->resource_share = 100;
                                }
                            }
                        }
                    }
                }
            } else {
                if (!acct.detach) {
                    msg_printf(NULL, MSG_INFO,
                        "Attaching to %s", acct.url.c_str()
                    );
                    gstate.add_project(
                        acct.url.c_str(), acct.authenticator.c_str(), "", true
                    );
                    if (acct.dont_request_more_work.present) {
                        pp->dont_request_more_work = acct.dont_request_more_work.value;
                    }
                }
            }
        }

        bool read_prefs = false;
        if (strlen(host_venue) && strcmp(host_venue, gstate.main_host_venue)) {
            strcpy(gstate.main_host_venue, host_venue);
            read_prefs = true;
        }

        // process prefs if any
        //
        if (global_prefs_xml) {
            retval = gstate.save_global_prefs(
                global_prefs_xml, ami.acct_mgr_url, ami.acct_mgr_url
            );
            if (retval) {
                msg_printf(NULL, MSG_INTERNAL_ERROR, "Can't save global prefs");
            }
            read_prefs = true;
        }

        // process prefs if prefs or venue changed
        //
        if (read_prefs) {
            gstate.read_global_prefs();
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
                    "    <signing_key>\n%s\n</signing_key>\n",
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
				"    <opaque>\n%s\n"
				"    </opaque>\n"
                "</acct_mgr_login>\n",
                login_name,
                password_hash,
                previous_host_cpid,
                next_rpc_time,
				opaque
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
	strcpy(opaque, "");
    next_rpc_time = 0;
    send_gui_rpc_info = false;
    password_error = false;
}

ACCT_MGR_INFO::ACCT_MGR_INFO() {
    clear();
}

int ACCT_MGR_INFO::parse_login_file(FILE* p) {
	char tag[1024];
	bool is_tag;
    MIOFILE mf;
    int retval;

    mf.init_file(p);
    XML_PARSER xp(&mf);
    if (!xp.parse_start("acct_mgr_login")) {
        //
    }
    while (!xp.get(tag, sizeof(tag), is_tag)) {
        if (!is_tag) {
            printf("unexpected text: %s\n", tag);
            continue;
        }
        if (!strcmp(tag, "/acct_mgr_login")) break;
        else if (xp.parse_str(tag, "login", login_name, 256)) continue;
        else if (xp.parse_str(tag, "password_hash", password_hash, 256)) continue;
        else if (xp.parse_str(tag, "previous_host_cpid", previous_host_cpid, sizeof(previous_host_cpid))) continue;
        else if (xp.parse_double(tag, "next_rpc_time", next_rpc_time)) continue;
        else if (!strcmp(tag, "opaque")) {
            retval = xp.element_contents("</opaque>", opaque, sizeof(opaque));
            continue;
        }
        if (log_flags.unparsed_xml) {
            msg_printf(NULL, MSG_INFO,
                "[unparsed_xml] ACCT_MGR_INFO::parse_login: unrecognized %s", tag
            );
        }
        xp.skip_unexpected(
            tag, log_flags.unparsed_xml, "ACCT_MGR_INFO::parse_login_file"
        );
    }
    return 0;
}

int ACCT_MGR_INFO::init() {
	char tag[1024];
	bool is_tag;
    MIOFILE mf;
    FILE*   p;
    int retval;

    clear();
    p = fopen(ACCT_MGR_URL_FILENAME, "r");
    if (!p) return 0;
    mf.init_file(p);
	XML_PARSER xp(&mf);
	if (!xp.parse_start("acct_mgr_login")) {
		//
	}
	while (!xp.get(tag, sizeof(tag), is_tag)) {
		if (!is_tag) {
			printf("unexpected text: %s\n", tag);
			continue;
		} 
        if (!strcmp(tag, "/acct_mgr")) break;
        else if (xp.parse_str(tag, "name", acct_mgr_name, 256)) continue;
        else if (xp.parse_str(tag, "url", acct_mgr_url, 256)) continue;
        else if (xp.parse_bool(tag, "send_gui_rpc_info", send_gui_rpc_info)) continue;
        else if (!strcmp(tag, "signing_key")) {
            retval = xp.element_contents("</signing_key>", signing_key, sizeof(signing_key));
            continue;
        }
        else if (xp.parse_bool(tag, "cookie_required", cookie_required)) continue;
        else if (xp.parse_str(tag, "cookie_failure_url", cookie_failure_url, 256)) continue;
        if (log_flags.unparsed_xml) {
            msg_printf(NULL, MSG_INFO,
                "[unparsed_xml] ACCT_MGR_INFO::init: unrecognized %s", tag
            );
        }
        xp.skip_unexpected(tag, log_flags.unparsed_xml, "ACCT_MGR_INFO::init");
    }
    fclose(p);

    p = fopen(ACCT_MGR_LOGIN_FILENAME, "r");
    if (p) {
        parse_login_file(p);
        fclose(p);
    }
    return 0;
}

bool ACCT_MGR_INFO::poll() {
    if (gstate.gui_http.is_busy()) return false;

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

