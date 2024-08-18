// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2017 University of California
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

// code for communicating with account managers (AMs)

#include "cpp.h"

#ifdef _WIN32
#include "boinc_win.h"
#else
#include "config.h"
#include <cstring>
#endif

#ifdef _MSC_VER
#define snprintf _snprintf
#endif

#include "crypt.h"
#include "error_numbers.h"
#include "parse.h"
#include "str_util.h"
#include "str_replace.h"
#include "url.h"

#include "client_msgs.h"
#include "client_state.h"
#include "cs_notice.h"
#include "file_names.h"
#include "filesys.h"
#include "gui_http.h"
#include "log_flags.h"
#include "project.h"

#include "acct_mgr.h"

static const char *run_mode_name[] = {"", "always", "auto", "never"};

// do an account manager RPC;
// if URL is null, detach from current account manager
//
int ACCT_MGR_OP::do_rpc(ACCT_MGR_INFO& _ami, bool _via_gui) {
    int retval;
    unsigned int i;
    char buf[1024];

    ami = _ami;

    error_num = ERR_IN_PROGRESS;
    error_str.clear();
    via_gui = _via_gui;
    global_prefs_xml.clear();

    // if null URL, detach from current AMS
    //
    if (!strlen(ami.master_url) && strlen(gstate.acct_mgr_info.master_url)) {
        msg_printf(NULL, MSG_INFO, "Removing account manager info");
        gstate.acct_mgr_info.clear();
        boinc_delete_file(ACCT_MGR_URL_FILENAME);
        boinc_delete_file(ACCT_MGR_LOGIN_FILENAME);
        error_num = 0;
        for (i=0; i<gstate.projects.size(); i++) {
            gstate.projects[i]->detach_ams();
        }
        ::rss_feeds.update_feed_list();
        gstate.set_client_state_dirty("detach from AMS");
        return 0;
    }

    canonicalize_master_url(ami.master_url, sizeof(ami.master_url));
    if (!valid_master_url(ami.master_url)) {
        error_num = ERR_INVALID_URL;
        return 0;
    }

    FILE* f = boinc_fopen(ACCT_MGR_REQUEST_FILENAME, "w");
    if (!f) return ERR_FOPEN;
    fprintf(f,
        "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n"
        "<acct_mgr_request>\n"
    );
    if (strlen(ami.authenticator)) {
        fprintf(f,
            "   <authenticator>%s</authenticator>\n",
            ami.authenticator
        );
    } else {
        fprintf(f,
            "   <name>%s</name>\n"
            "   <password_hash>%s</password_hash>\n",
            ami.login_name, ami.password_hash
        );
    }
    fprintf(f,
        "   <host_cpid>%s</host_cpid>\n"
        "   <domain_name>%s</domain_name>\n"
        "   <client_version>%d.%d.%d</client_version>\n"
        "   <run_mode>%s</run_mode>\n",
        gstate.host_info.host_cpid,
        gstate.host_info.domain_name,
        gstate.core_client_version.major,
        gstate.core_client_version.minor,
        gstate.core_client_version.release,
        run_mode_name[gstate.cpu_run_mode.get_perm()]
    );
    gstate.write_platforms(NULL, f);
    if (strlen(gstate.acct_mgr_info.previous_host_cpid)) {
        fprintf(f,
            "   <previous_host_cpid>%s</previous_host_cpid>\n",
            gstate.acct_mgr_info.previous_host_cpid
        );
    }

    // If the AMS requested it, send GUI RPC port and password.
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
            char gui_rpc_password[256];
            safe_strcpy(gui_rpc_password, "");
            FILE* pwdf = fopen(GUI_RPC_PASSWD_FILE, "r");
            if (pwdf) {
                if (fgets(gui_rpc_password, 256, pwdf)) {
                    strip_whitespace(gui_rpc_password);
                }
                fclose(pwdf);
            }
            fprintf(f,
                "   <gui_rpc_password>%s</gui_rpc_password>\n",
                gui_rpc_password
            );
        }
    }
    for (i=0; i<gstate.projects.size(); i++) {
        PROJECT* p = gstate.projects[i];
        double not_started_dur, in_progress_dur;
        p->get_task_durs(not_started_dur, in_progress_dur);
        fprintf(f,
            "   <project>\n"
            "      <url>%s</url>\n"
            "      <project_name>%s</project_name>\n"
            "      <suspended_via_gui>%d</suspended_via_gui>\n"
            "      <hostid>%d</hostid>\n"
            "      <not_started_dur>%f</not_started_dur>\n"
            "      <in_progress_dur>%f</in_progress_dur>\n"
            "      <attached_via_acct_mgr>%d</attached_via_acct_mgr>\n"
            "      <dont_request_more_work>%d</dont_request_more_work>\n"
            "      <detach_when_done>%d</detach_when_done>\n"
            "      <ended>%d</ended>\n"
            "      <resource_share>%f</resource_share>\n"
            "      <disk_usage>%f</disk_usage>\n"
            "      <disk_share>%f</disk_share>\n",
            p->master_url,
            p->project_name,
            p->suspended_via_gui?1:0,
            p->hostid,
            not_started_dur,
            in_progress_dur,
            p->attached_via_acct_mgr?1:0,
            p->dont_request_more_work?1:0,
            p->detach_when_done?1:0,
            p->ended?1:0,
            p->resource_share,
            p->disk_usage,
            p->disk_share
        );

        // send work and starvation-related info
        //
        if (ami.dynamic) {
            fprintf(f,
                "      <nrpc_failures>%d</nrpc_failures>\n"
                "      <cpu_ec>%f</cpu_ec>\n"
                "      <cpu_time>%f</cpu_time>\n"
                "      <gpu_ec>%f</gpu_ec>\n"
                "      <gpu_time>%f</gpu_time>\n"
                "      <njobs_success>%d</njobs_success>\n"
                "      <njobs_error>%d</njobs_error>\n",
                p->nrpc_failures,
                p->cpu_ec,
                p->cpu_time,
                p->gpu_ec,
                p->gpu_time,
                p->njobs_success,
                p->njobs_error
            );
            for (int j=0; j<coprocs.n_rsc; j++) {
                if (p->sched_req_no_work[j]) {
                    fprintf(f,
                        "    <sched_req_no_work>%s</sched_req_no_work>\n",
                        coprocs.coprocs[j].type
                    );
                }
            }
        }

        if (p->attached_via_acct_mgr) {
            fprintf(f,
                "      <account_key>%s</account_key>\n",
                p->authenticator
            );
        }
        fprintf(f,
            "   </project>\n"
        );
    }
    MIOFILE mf;
    mf.init_file(f);

    // send working prefs
    //
    fprintf(f, "<working_global_preferences>\n");
    gstate.global_prefs.write(mf);
    fprintf(f, "</working_global_preferences>\n");

    if (boinc_file_exists(GLOBAL_PREFS_FILE_NAME)) {
        FILE* fprefs = fopen(GLOBAL_PREFS_FILE_NAME, "r");
        if (fprefs) {
            copy_stream(fprefs, f);
            fclose(fprefs);
        }
    }
    gstate.host_info.write(mf, !cc_config.suppress_net_info, true);
    if (strlen(gstate.acct_mgr_info.opaque)) {
        fprintf(f,
            "   <opaque>\n%s\n"
            "   </opaque>\n",
            gstate.acct_mgr_info.opaque
        );
    }
    gstate.time_stats.write(mf, true);
    gstate.net_stats.write(mf);

#ifndef SIM
    // send task descriptions if requested by AM
    //
    if (ami.send_tasks_all || ami.send_tasks_active) {
        mf.printf("<results>\n");
        gstate.write_tasks_gui(mf, !ami.send_tasks_all);
        mf.printf("</results>\n");
    }
#endif

    fprintf(f, "</acct_mgr_request>\n");
    fclose(f);
    snprintf(buf, sizeof(buf), "%srpc.php", ami.master_url);
    retval = gui_http->do_rpc_post(
        this, buf, ACCT_MGR_REQUEST_FILENAME, ACCT_MGR_REPLY_FILENAME, true
    );
    if (retval) {
        error_num = retval;
        return retval;
    }
    msg_printf(NULL, MSG_INFO, "Contacting account manager at %s", ami.master_url);

    return 0;
}

void AM_ACCOUNT::handle_no_rsc(const char* name, bool value) {
    int i = rsc_index(name);
    if (i < 0) return;
    no_rsc[i] = value;
}

// parse a project account from AM reply
//
int AM_ACCOUNT::parse(XML_PARSER& xp) {
    char buf[256];
    bool btemp;
    int retval;
    double dtemp;

    detach = false;
    update = false;
    memset(no_rsc, 0, sizeof(no_rsc));
    dont_request_more_work.init();
    detach_when_done.init();
    suspend.init();
    abort_not_started.init();
    url.clear();
    safe_strcpy(url_signature, "");
    authenticator.clear();
    resource_share.init();

    while (!xp.get_tag()) {
        if (!xp.is_tag) {
            if (log_flags.unparsed_xml) {
                msg_printf(0, MSG_INFO,
                    "[unparsed_xml] AM_ACCOUNT::parse: unexpected text %s",
                    xp.parsed_tag
                );
            }
            continue;
        }
        if (xp.match_tag("/account")) {
            if (url.length()) return 0;
            return ERR_XML_PARSE;
        }
        if (xp.parse_string("url", url)) continue;
        if (xp.match_tag("url_signature")) {
            retval = xp.element_contents("</url_signature>", url_signature, sizeof(url_signature));
            if (retval) return retval;
            safe_strcat(url_signature, "\n");
            continue;
        }
        if (xp.parse_string("authenticator", authenticator)) continue;
        if (xp.parse_bool("detach", detach)) continue;
        if (xp.parse_bool("update", update)) continue;
        if (xp.parse_bool("no_cpu", btemp)) {
            handle_no_rsc("CPU", btemp);
            continue;
        }

        // deprecated
        if (xp.parse_bool("no_cuda", btemp)) {
            handle_no_rsc(GPU_TYPE_NVIDIA, btemp);
            continue;
        }
        if (xp.parse_bool("no_ati", btemp)) {
            handle_no_rsc(GPU_TYPE_NVIDIA, btemp);
            continue;
        }

        if (xp.parse_str("no_rsc", buf, sizeof(buf))) {
            handle_no_rsc(buf, true);
            continue;
        }
        if (xp.parse_bool("dont_request_more_work", btemp)) {
            dont_request_more_work.set(btemp);
            continue;
        }
        if (xp.parse_bool("detach_when_done", btemp)) {
            detach_when_done.set(btemp);
            continue;
        }
        if (xp.parse_double("resource_share", dtemp)) {
            if (dtemp >= 0) {
                resource_share.set(dtemp);
            } else {
                msg_printf(NULL, MSG_INFO,
                    "Resource share out of range: %f", dtemp
                );
            }
            continue;
        }
        if (xp.parse_bool("suspend", btemp)) {
            suspend.set(btemp);
            continue;
        }
        if (xp.parse_bool("abort_not_started", btemp)) {
            abort_not_started.set(btemp);
            continue;
        }
        if (log_flags.unparsed_xml) {
            msg_printf(NULL, MSG_INFO,
                "[unparsed_xml] AM_ACCOUNT: unrecognized %s", xp.parsed_tag
            );
        }
        xp.skip_unexpected(log_flags.unparsed_xml, "AM_ACCOUNT::parse");
    }
    return ERR_XML_PARSE;
}

// parse RPC reply from account manager
//
int ACCT_MGR_OP::parse(FILE* f) {
    string message;
    int retval;
    MIOFILE mf;
    mf.init_file(f);
    XML_PARSER xp(&mf);

    accounts.clear();
    error_str.clear();
    error_num = 0;
    repeat_sec = 0;
    safe_strcpy(host_venue, "");
    safe_strcpy(ami.opaque, "");
    ami.no_project_notices = false;
    ami.dynamic = false;
    ami.send_tasks_all = false;
    ami.send_tasks_active = false;
    rss_feeds.clear();
    if (!xp.parse_start("acct_mgr_reply")) return ERR_XML_PARSE;
    while (!xp.get_tag()) {
        if (!xp.is_tag) {
            if (log_flags.unparsed_xml) {
                msg_printf(0, MSG_INFO,
                    "[unparsed_xml] ACCT_MGR_OP::parse: unexpected text %s",
                    xp.parsed_tag
                );
            }
            continue;
        }
        if (xp.match_tag("/acct_mgr_reply")) return 0;
        if (xp.parse_str("name", ami.project_name, 256)) continue;
        if (xp.parse_str("user_name", ami.user_name, sizeof(ami.user_name))) {
            xml_unescape(ami.user_name);
            continue;
        }
        if (xp.parse_str("team_name", ami.team_name, sizeof(ami.team_name))) {
            xml_unescape(ami.team_name);
            continue;
        }
        if (xp.parse_str("authenticator", ami.authenticator, 256)) continue;
        if (xp.parse_int("error_num", error_num)) continue;
        if (xp.parse_string("error", error_str)) continue;
        if (xp.parse_string("error_msg", error_str)) continue;
        if (xp.parse_double("repeat_sec", repeat_sec)) continue;
        if (xp.parse_bool("dynamic", ami.dynamic)) continue;
        if (xp.parse_bool("send_tasks_active", ami.send_tasks_active)) continue;
        if (xp.parse_bool("send_tasks_all", ami.send_tasks_all)) continue;
        if (xp.parse_string("message", message)) {
            msg_printf(NULL, MSG_INFO, "Account manager: %s", message.c_str());
            continue;
        }
        if (xp.match_tag("opaque")) {
            retval = xp.element_contents("</opaque>", ami.opaque, sizeof(ami.opaque));
            if (retval) return retval;
            continue;
        }
        if (xp.match_tag("signing_key")) {
            retval = xp.element_contents("</signing_key>", ami.signing_key, sizeof(ami.signing_key));
            if (retval) return retval;
            continue;
        }
        if (xp.match_tag("account")) {
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
        if (xp.match_tag("global_preferences")) {
            retval = copy_element_contents(
                f,
                "</global_preferences>",
                global_prefs_xml
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
        if (xp.parse_str("host_venue", host_venue, sizeof(host_venue))) {
            continue;
        }
        if (xp.match_tag("rss_feeds")) {
            got_rss_feeds = true;
            parse_rss_feed_descs(xp, rss_feeds);
            continue;
        }
        if (xp.parse_bool("no_project_notices", ami.no_project_notices)) {
            continue;
        }
        if (xp.match_tag("user_keywords")) {
            retval = ami.user_keywords.parse(xp);
            if (retval) return retval;
            continue;
        }
        if (log_flags.unparsed_xml) {
            msg_printf(NULL, MSG_INFO,
                "[unparsed_xml] ACCT_MGR_OP::parse: unrecognized tag <%s>",
                xp.parsed_tag
            );
        }
        xp.skip_unexpected(log_flags.unparsed_xml, "ACCT_MGR_OP::parse");
    }
    return ERR_XML_PARSE;
}

static inline bool is_weak_auth(const char* auth) {
    return (strchr(auth, '_') != NULL);
}

#ifdef SIM
void ACCT_MGR_OP::handle_reply(int ) {
}
#else
void ACCT_MGR_OP::handle_reply(int http_op_retval) {
    unsigned int i;
    int retval;
    bool verified;
    PROJECT* pp;
    bool sig_ok;
    bool got_error = false;

    // check for failures of HTTP OP, reply parse
    //
    if (http_op_retval) {
        msg_printf(&ami, MSG_INFO, "AM RPC HTTP failure: %s",
            boincerror(http_op_retval)
        );
        got_error = true;
    } else {
        FILE* f = fopen(ACCT_MGR_REPLY_FILENAME, "r");
        if (f) {
            retval = parse(f);
            if (retval) {
                got_error = true;
                msg_printf(&ami, MSG_INFO, "AM reply parse error");
            }
            fclose(f);
        } else {
            msg_printf(&ami, MSG_INFO, "AM reply file missing");
            got_error = true;
        }
    }

    // if no errors so far, check for errors from AM
    //
    if (!got_error) {
        gstate.acct_mgr_info.password_error = false;
        if (error_num == ERR_BAD_PASSWD && !via_gui) {
            gstate.acct_mgr_info.password_error = true;
        }

        // Show error message from AM if available.
        // check both error_str and error_num since an account manager may only
        // return a BOINC based error code for password failures or invalid
        // email addresses
        //
        if (error_str.size()) {
            msg_printf(&ami, MSG_USER_ALERT,
                "%s: %s",
                _("Message from account manager"),
                error_str.c_str()
            );
            got_error = true;
        } else if (error_num) {
            msg_printf(&ami, MSG_USER_ALERT,
                "%s: %s",
                _("Message from account manager"),
                boincerror(error_num)
            );
            got_error = true;
        }
    }

    if (got_error) {
        gstate.acct_mgr_info.next_rpc_time =
            gstate.now
            + calculate_exponential_backoff(
                gstate.acct_mgr_info.nfailures,
                ACCT_MGR_MIN_BACKOFF, ACCT_MGR_MAX_BACKOFF
            )
        ;
        gstate.acct_mgr_info.nfailures++;
        return;
    }

    // The RPC was successful
    //
    // Detach projects that are
    // - detach_when_done
    // - done
    // - attached via AM
    //
    while (1) {
        bool found = false;
        for (i=0; i<gstate.projects.size(); i++) {
            PROJECT* p = gstate.projects[i];
            if (p->detach_when_done && !gstate.nresults_for_project(p) && p->attached_via_acct_mgr) {
                gstate.detach_project(p);
                found = true;
            }
        }
        if (!found) break;
    }

    gstate.acct_mgr_info.nfailures = 0;

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
        // if the AM RPC had an error, some items may be missing; don't copy
        //
        if (strlen(ami.project_name)) {
            safe_strcpy(gstate.acct_mgr_info.project_name, ami.project_name);
        }
        if (strlen(ami.signing_key)) {
            safe_strcpy(gstate.acct_mgr_info.signing_key, ami.signing_key);
        }
        if (strlen(ami.opaque)) {
            safe_strcpy(gstate.acct_mgr_info.opaque, ami.opaque);
        }
        safe_strcpy(gstate.acct_mgr_info.master_url, ami.master_url);
        safe_strcpy(gstate.acct_mgr_info.login_name, ami.login_name);
        safe_strcpy(gstate.acct_mgr_info.password_hash, ami.password_hash);
        safe_strcpy(gstate.acct_mgr_info.authenticator, ami.authenticator);
        gstate.acct_mgr_info.no_project_notices = ami.no_project_notices;
        gstate.acct_mgr_info.dynamic = ami.dynamic;
        gstate.acct_mgr_info.send_tasks_active = ami.send_tasks_active;
        gstate.acct_mgr_info.send_tasks_all = ami.send_tasks_all;

        // process projects
        //
        for (i=0; i<accounts.size(); i++) {
            AM_ACCOUNT& acct = accounts[i];
            pp = gstate.lookup_project(acct.url.c_str());
            if (pp) {
                if (acct.detach) {
                    if (pp->attached_via_acct_mgr) {
                        gstate.detach_project(pp);
                    }
                } else {
                    // The AM can leave authenticator blank if request message
                    // had the current account info
                    //
                    if (acct.authenticator.size()) {
                        if (strcmp(pp->authenticator, acct.authenticator.c_str())) {
                            // if old and new auths are both weak,
                            // use the new one
                            //
                            if (is_weak_auth(pp->authenticator)
                                && is_weak_auth(acct.authenticator.c_str())
                            ) {
                                safe_strcpy(pp->authenticator, acct.authenticator.c_str());
                                msg_printf(pp, MSG_INFO,
                                    "Received new authenticator from account manager"
                                );
                            } else {
                                // otherwise keep using the old one
                                //
                                msg_printf(pp, MSG_INFO,
                                    "Already attached to a different account"
                                );
                            }
                        }
                    }
                    pp->attached_via_acct_mgr = true;
                    if (acct.dont_request_more_work.present) {
                        pp->dont_request_more_work = acct.dont_request_more_work.value;
                    } else {
                        pp->dont_request_more_work = false;
                    }
                    if (acct.detach_when_done.present) {
                        pp->detach_when_done = acct.detach_when_done.value;
                        if (pp->detach_when_done) {
                            pp->dont_request_more_work = true;
                        }
                    } else {
                        pp->detach_when_done = false;
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
                            safe_strcpy(p2.master_url, pp->master_url);
                            retval = p2.parse_account_file();
                            if (!retval) {
                                pp->resource_share = p2.resource_share;
                            } else {
                                pp->resource_share = 100;
                            }
                        }
                    }

                    if (acct.suspend.present) {
                        if (acct.suspend.value) {
                            pp->suspend();
                        } else {
                            pp->resume();
                        }
                    }
                    if (acct.abort_not_started.present) {
                        if (acct.abort_not_started.value) {
                            pp->abort_not_started();
                        }
                    }
                    for (int j=0; j<MAX_RSC; j++) {
                        pp->no_rsc_ams[j] = acct.no_rsc[j];
                    }
                }
            } else {
                // here we don't already have the project.
                //
                if (acct.detach || (acct.detach_when_done.present && acct.detach_when_done.value)) {
                    continue;
                }

                retval = check_string_signature2(
                    acct.url.c_str(), acct.url_signature, ami.signing_key, verified
                );
                if (retval || !verified) {
                    msg_printf(NULL, MSG_INTERNAL_ERROR,
                        "Bad signature for URL %s", acct.url.c_str()
                    );
                    continue;
                }
                if (acct.authenticator.empty()) {
                    msg_printf(NULL, MSG_INFO,
                        "Account manager reply missing authenticator for %s",
                        acct.url.c_str()
                    );
                    continue;
                }

                // Attach to it
                //
                msg_printf(NULL, MSG_INFO,
                    "Attaching to %s", acct.url.c_str()
                );
                gstate.add_project(
                    acct.url.c_str(), acct.authenticator.c_str(), "", "", true
                );
                pp = gstate.lookup_project(acct.url.c_str());
                if (pp) {
                    for (int j=0; j<MAX_RSC; j++) {
                        pp->no_rsc_ams[j] = acct.no_rsc[j];
                    }
                    if (acct.dont_request_more_work.present) {
                        pp->dont_request_more_work = acct.dont_request_more_work.value;
                    }
                    if (acct.suspend.present && acct.suspend.value) {
                        pp->suspend();
                    }
                } else {
                    msg_printf(NULL, MSG_INTERNAL_ERROR,
                        "Failed to add project: %s",
                        acct.url.c_str()
                    );
                }
            }
        }

#ifdef USE_NET_PREFS
        bool read_prefs = false;
        if (strlen(host_venue) && strcmp(host_venue, gstate.main_host_venue)) {
            safe_strcpy(gstate.main_host_venue, host_venue);
            read_prefs = true;
        }

        // process prefs if any
        //
        if (!global_prefs_xml.empty()) {
            bool use_am_prefs;
            // if dynamic AM (like SU) its prefs are our net prefs
            //
            if (ami.dynamic) {
                use_am_prefs = true;
            } else {
                double mod_time = GLOBAL_PREFS::parse_mod_time(
                    global_prefs_xml.c_str()
                );
                use_am_prefs = mod_time > gstate.global_prefs.mod_time;
            }
            if (use_am_prefs) {
                retval = gstate.save_global_prefs(
                    global_prefs_xml.c_str(), ami.master_url, ami.master_url
                );
                if (retval) {
                    msg_printf(NULL, MSG_INTERNAL_ERROR, "Can't save global prefs");
                }
                read_prefs = true;
            }
        }

        // process prefs if prefs or venue changed
        //
        if (read_prefs) {
            gstate.read_global_prefs();
        }
#endif

        handle_sr_feeds(rss_feeds, &gstate.acct_mgr_info);

        // in case no_project_notices changed
        //
        ::rss_feeds.update_feed_list();
    }

    safe_strcpy(gstate.acct_mgr_info.user_name, ami.user_name);
    safe_strcpy(gstate.acct_mgr_info.team_name, ami.team_name);
    safe_strcpy(
        gstate.acct_mgr_info.previous_host_cpid, gstate.host_info.host_cpid
    );
    if (repeat_sec) {
        gstate.acct_mgr_info.next_rpc_time = gstate.now + repeat_sec;
    } else {
        gstate.acct_mgr_info.next_rpc_time = gstate.now + 86400;
    }
    gstate.acct_mgr_info.user_keywords = ami.user_keywords;
    gstate.acct_mgr_info.write_info();
    gstate.set_client_state_dirty("account manager RPC");
}
#endif

// write AM info to files.
// This is done after each AM RPC.
//
int ACCT_MGR_INFO::write_info() {
    FILE* f;
    if (strlen(master_url)) {
        f = fopen(ACCT_MGR_URL_FILENAME, "w");
        if (!f) {
            msg_printf(NULL, MSG_USER_ALERT,
                "Can't write to %s; check file and directory permissions",
                ACCT_MGR_URL_FILENAME
            );
            return ERR_FOPEN;
        }
        fprintf(f,
            "<acct_mgr>\n"
            "    <name>%s</name>\n"
            "    <url>%s</url>\n",
            project_name,
            master_url
        );
        if (send_gui_rpc_info) {
            fprintf(f, "    <send_gui_rpc_info/>\n");
        }
        if (strlen(signing_key)) {
            fprintf(f,
                "    <signing_key>\n%s\n</signing_key>\n",
                signing_key
            );
        }
        fprintf(f,
            "</acct_mgr>\n"
        );
        fclose(f);
    }

    if (strlen(login_name) || strlen(authenticator)) {
        f = fopen(ACCT_MGR_LOGIN_FILENAME, "w");
        if (!f) {
            msg_printf(NULL, MSG_USER_ALERT,
                "Can't write to %s; check file and directory permissions",
                ACCT_MGR_LOGIN_FILENAME
            );
            return ERR_FOPEN;
        }
        fprintf(f,
            "<acct_mgr_login>\n"
        );
        if (strlen(authenticator)) {
            fprintf(f,
                "    <authenticator>%s</authenticator>\n",
                authenticator
            );
        } else {
            fprintf(f,
                "    <login>%s</login>\n"
                "    <password_hash>%s</password_hash>\n",
                login_name,
                password_hash
            );
        }
        fprintf(f,
            "    <previous_host_cpid>%s</previous_host_cpid>\n"
            "    <next_rpc_time>%f</next_rpc_time>\n"
            "    <opaque>\n%s\n"
            "    </opaque>\n"
            "    <no_project_notices>%d</no_project_notices>\n"
            "    <dynamic>%d</dynamic>\n",
            previous_host_cpid,
            next_rpc_time,
            opaque,
            no_project_notices?1:0,
            dynamic?1:0
        );
        char buf[4096];
        if (strlen(user_name)) {
            xml_escape(user_name, buf, sizeof(buf));
            fprintf(f,    "<user_name>%s</user_name>\n", buf);
        }
        if (strlen(team_name)) {
            xml_escape(team_name, buf, sizeof(buf));
            fprintf(f,    "<team_name>%s</team_name>\n", buf);
        }


        user_keywords.write(f);
        fprintf(f,
            "</acct_mgr_login>\n"
        );
        fclose(f);
    }
    return 0;
}

void ACCT_MGR_INFO::clear() {
    safe_strcpy(project_name, "");
    safe_strcpy(master_url, "");
    safe_strcpy(login_name, "");
    safe_strcpy(user_name, "");
    safe_strcpy(team_name, "");
    safe_strcpy(password_hash, "");
    safe_strcpy(authenticator, "");
    safe_strcpy(signing_key, "");
    safe_strcpy(previous_host_cpid, "");
    safe_strcpy(opaque, "");
    safe_strcpy(cookie_failure_url, "");
    next_rpc_time = 0;
    nfailures = 0;
    send_gui_rpc_info = false;
    password_error = false;
    no_project_notices = false;
    cookie_required = false;
    user_keywords.clear();
    first_starved = 0;
    starved_rpc_backoff = 0;
    starved_rpc_min_time = 0;
    dynamic = false;
    send_tasks_active = false;
    send_tasks_all = false;
}

ACCT_MGR_INFO::ACCT_MGR_INFO() {
    clear();
}

int ACCT_MGR_INFO::parse_login_file(FILE* p) {
    MIOFILE mf;
    int retval;

    mf.init_file(p);
    XML_PARSER xp(&mf);
    if (!xp.parse_start("acct_mgr_login")) {
        msg_printf(NULL, MSG_INTERNAL_ERROR,
            "missing start tag in account manager login file"
        );
    }
    while (!xp.get_tag()) {
        if (!xp.is_tag) {
            printf("unexpected text: %s\n", xp.parsed_tag);
            continue;
        }
        if (xp.match_tag("/acct_mgr_login")) break;
        if (xp.parse_str("login", login_name, 256)) continue;
        if (xp.parse_str("password_hash", password_hash, 256)) continue;
        if (xp.parse_str("authenticator", authenticator, 256)) continue;
        if (xp.parse_str("previous_host_cpid", previous_host_cpid, sizeof(previous_host_cpid))) continue;
        if (xp.parse_double("next_rpc_time", next_rpc_time)) continue;
        if (xp.match_tag("opaque")) {
            retval = xp.element_contents("</opaque>", opaque, sizeof(opaque));
            if (retval) {
                msg_printf(NULL, MSG_INFO,
                    "error parsing <opaque> in acct_mgr_login.xml"
                );
            }
            continue;
        }
        if (xp.parse_str("user_name", user_name, sizeof(user_name))) {
            xml_unescape(user_name);
            continue;
        }
        if (xp.parse_str("team_name", team_name, sizeof(team_name))) {
            xml_unescape(team_name);
            continue;
        }
        if (xp.parse_bool("no_project_notices", no_project_notices)) continue;
        if (xp.parse_bool("dynamic", dynamic)) continue;
        if (xp.match_tag("user_keywords")) {
            retval = user_keywords.parse(xp);
            if (retval) {
                msg_printf(NULL, MSG_INFO,
                    "error parsing user keywords in acct_mgr_login.xml"
                );
            }
            continue;
        }
        if (log_flags.unparsed_xml) {
            msg_printf(NULL, MSG_INFO,
                "[unparsed_xml] unrecognized %s in acct_mgr_login.xml",
                xp.parsed_tag
            );
        }
        xp.skip_unexpected(
            log_flags.unparsed_xml, "ACCT_MGR_INFO::parse_login_file"
        );
    }
    return 0;
}

// called at client startup.
// If currently using an AM, read its URL and login files
//
int ACCT_MGR_INFO::init() {
    MIOFILE mf;
    FILE*   p;
    int retval;

    clear();
    p = fopen(ACCT_MGR_URL_FILENAME, "r");
    if (!p) {
        // if not using acct mgr, make sure projects not flagged,
        // otherwise won't be able to detach them.
        //
        for (unsigned int i=0; i<gstate.projects.size(); i++) {
            gstate.projects[i]->attached_via_acct_mgr = false;
        }
        return 0;
    }
    mf.init_file(p);
    XML_PARSER xp(&mf);
    if (!xp.parse_start("acct_mgr")) {
        //
    }
    while (!xp.get_tag()) {
        if (!xp.is_tag) {
            printf("unexpected text: %s\n", xp.parsed_tag);
            continue;
        }
        if (xp.match_tag("/acct_mgr")) break;
        else if (xp.parse_str("name", project_name, 256)) continue;
        else if (xp.parse_str("url", master_url, 256)) continue;
        else if (xp.parse_bool("send_gui_rpc_info", send_gui_rpc_info)) continue;
        else if (xp.match_tag("signing_key")) {
            retval = xp.element_contents("</signing_key>", signing_key, sizeof(signing_key));
            if (retval) {
                msg_printf(NULL, MSG_INFO,
                    "error parsing <signing_key> in acct_mgr_url.xml"
                );
            }
            continue;
        }
        else if (xp.parse_bool("cookie_required", cookie_required)) continue;
        else if (xp.parse_str("cookie_failure_url", cookie_failure_url, 256)) continue;
        if (log_flags.unparsed_xml) {
            msg_printf(NULL, MSG_INFO,
                "[unparsed_xml] ACCT_MGR_INFO::init: unrecognized %s",
                xp.parsed_tag
            );
        }
        xp.skip_unexpected(log_flags.unparsed_xml, "ACCT_MGR_INFO::init");
    }
    fclose(p);

    p = fopen(ACCT_MGR_LOGIN_FILENAME, "r");
    if (p) {
        parse_login_file(p);
        fclose(p);
    }
    if (using_am()) {
        msg_printf(NULL, MSG_INFO, "Using account manager %s", project_name);
        if (strlen(user_name)) {
            msg_printf(NULL, MSG_INFO, "Account manager login: %s", user_name);
        }
    }
    return 0;
}

#define STARVED_RPC_DELAY   600
    // do RPC after this much starvation

// See if we need to contact the account manager.
// called once a second
//
bool ACCT_MGR_INFO::poll() {
    if (!using_am()) return false;
    if (gstate.acct_mgr_op.gui_http->is_busy()) {
        return false;
    }

    // see if time for a periodic RPC
    //
    if (gstate.now > next_rpc_time) {
        // default synch period is 1 day; the AM can override this
        //
        next_rpc_time = gstate.now + 86400;
        gstate.acct_mgr_op.do_rpc(*this, false);
        return true;
    }

    // if not dynamic AM, we're done
    // ("dynamic" means the AM can change set of projects)
    //
    if (!dynamic) {
        return false;
    }

    // it's possible that the set of projects given us by the AM
    // is starving a resources
    // e.g. those projects don't currently have jobs, or are down.
    // In that case contact the AM, asking for other projects.
    // Do this with exponential backoff to avoid overloading the AM

    // do this check once a minute
    //
    static int idle_timer = 0;
    if (++idle_timer < 60) {
        return false;
    }
    idle_timer = 0;
    if (n_idle_resources()>0) {
        if (log_flags.work_fetch_debug) {
            msg_printf(NULL, MSG_INFO,
                "[work_fetch] Using dynamic AM and some device is idle"
            );
        }

        // "first_starved" is the time when starvation began.
        // Let 10 min pass before contacting the AM
        // (e.g. in case a scheduler request fails for some reason)

        if (first_starved == 0) {
            first_starved = gstate.now;
            starved_rpc_backoff = STARVED_RPC_DELAY;
            starved_rpc_min_time = gstate.now + STARVED_RPC_DELAY;
            if (log_flags.work_fetch_debug) {
                msg_printf(NULL, MSG_INFO,
                    "[work_fetch] First time - delaying RPC for %d sec",
                    STARVED_RPC_DELAY
                );
            }
        } else {
            if (gstate.now < starved_rpc_min_time) {
                if (log_flags.work_fetch_debug) {
                    msg_printf(NULL, MSG_INFO,
                        "[work_fetch] AM RPC backed off for %.0f sec",
                        starved_rpc_min_time - gstate.now
                    );
                }
                return false;
            }
            msg_printf(NULL, MSG_INFO,
                "Some devices idle - requesting new projects from %s",
                gstate.acct_mgr_info.project_name
            );
            gstate.acct_mgr_op.do_rpc(*this, false);
            starved_rpc_backoff *= 2;
            if (starved_rpc_backoff > 86400) {
                starved_rpc_backoff = 86400;
            }
            starved_rpc_min_time = gstate.now + starved_rpc_backoff;
            return true;
        }
    } else {
        first_starved = 0;
    }
    return false;
}
