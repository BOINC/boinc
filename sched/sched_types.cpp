// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2012 University of California
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

#include "config.h"
#include <cstdlib>
#include <cassert>
#include <vector>
#include <string>
#include <cstring>

#include "parse.h"
#include "error_numbers.h"
#include "str_util.h"
#include "util.h"
#include "boinc_db.h"

#include "sched_main.h"
#include "sched_util.h"
#include "sched_msgs.h"
#include "sched_send.h"
#include "time_stats_log.h"
#include "sched_types.h"

#ifdef _USING_FCGI_
#include "boinc_fcgi.h"
#endif

using std::string;

SCHEDULER_REQUEST* g_request;
SCHEDULER_REPLY* g_reply;
WORK_REQ* g_wreq;

const char* proc_type_name(int pt) {
    switch(pt) {
    case PROC_TYPE_NVIDIA: return "CUDA";
    case PROC_TYPE_AMD: return "ATI";
    case PROC_TYPE_INTEL: return "INTEL";
    }
    return "unknown";
}

// remove (by truncating) any quotes from the given string.
// This is for things (e.g. authenticator) that will be used in
// a SQL query, to prevent SQL injection attacks
//
void remove_quotes(char* p) {
    int i, n=strlen(p);
    for (i=0; i<n; i++) {
        if (p[i]=='\'' || p[i]=='"') {
            p[i] = 0;
            return;
        }
    }
}

int CLIENT_APP_VERSION::parse(XML_PARSER& xp) {
    double x;

    memset(this, 0, sizeof(*this));
    host_usage.avg_ncpus = 1;
    while (!xp.get_tag()) {
        if (xp.match_tag("/app_version")) {
            app = ssp->lookup_app_name(app_name);
            if (!app) return ERR_NOT_FOUND;

            double pf = host_usage.avg_ncpus * g_reply->host.p_fpops;
            switch (host_usage.proc_type) {
            case PROC_TYPE_NVIDIA:
                if (g_request->coprocs.nvidia.count) {
                    pf += host_usage.gpu_usage*g_request->coprocs.nvidia.peak_flops;
                }
                break;
            case PROC_TYPE_AMD:
                if (g_request->coprocs.ati.count) {
                    pf += host_usage.gpu_usage*g_request->coprocs.ati.peak_flops;
                }
                break;
            case PROC_TYPE_INTEL:
                if (g_request->coprocs.intel.count) {
                    pf += host_usage.gpu_usage*g_request->coprocs.intel.peak_flops;
                }
                break;
            }
            host_usage.peak_flops = pf;
            return 0;
        }
        if (xp.parse_str("app_name", app_name, 256)) continue;
        if (xp.parse_str("platform", platform, 256)) continue;
        if (xp.parse_str("plan_class", plan_class, 256)) continue;
        if (xp.parse_int("version_num", version_num)) continue;
        if (xp.parse_double("avg_ncpus", x)) {
            if (x>0) host_usage.avg_ncpus = x;
            continue;
        }
        if (xp.parse_double("flops", x)) {
            if (x>0) host_usage.projected_flops = x;
            continue;
        }
        if (xp.match_tag("coproc")) {
            COPROC_REQ coproc_req;
            int retval = coproc_req.parse(xp);
            if (!retval) {
                host_usage.gpu_usage = coproc_req.count;
                if (!strcmp(coproc_req.type, "CUDA") || !strcmp(coproc_req.type, "NVIDIA")) {
                host_usage.proc_type = PROC_TYPE_NVIDIA;
            } else if (!strcmp(coproc_req.type, "ATI")) {
                host_usage.proc_type = PROC_TYPE_AMD;
            } else if (!strcmp(coproc_req.type, "INTEL")) {
                host_usage.proc_type = PROC_TYPE_INTEL;
            }
            continue;
        }
    }
    return ERR_XML_PARSE;
}

int FILE_INFO::parse(XML_PARSER& xp) {
    memset(this, 0, sizeof(*this));
    while (!xp.get_tag()) {
        if (xp.match_tag("/file_info")) {
            if (!strlen(name)) return ERR_XML_PARSE;
            return 0;
        }
        if (xp.parse_str("name", name, 256)) continue;
    }
    return ERR_XML_PARSE;
}

int OTHER_RESULT::parse(XML_PARSER& xp) {
    strcpy(name, "");
    have_plan_class = false;
    app_version = -1;
    while (!xp.get_tag()) {
        if (xp.match_tag("/other_result")) {
            if (!strcmp(name, "")) return ERR_XML_PARSE;
            return 0;
        }
        if (xp.parse_str("name", name, sizeof(name))) continue;
        if (xp.parse_int("app_version", app_version)) continue;
        if (xp.parse_str("plan_class", plan_class, sizeof(plan_class))) {
            have_plan_class = true;
            continue;
        }
    }
    return ERR_XML_PARSE;
}

int IP_RESULT::parse(XML_PARSER& xp) {
    report_deadline = 0;
    cpu_time_remaining = 0;
    strcpy(name, "");
    while (!xp.get_tag()) {
        if (xp.match_tag("/ip_result")) return 0;
        if (xp.parse_str("name", name, sizeof(name))) continue;
        if (xp.parse_double("report_deadline", report_deadline)) continue;
        if (xp.parse_double("cpu_time_remaining", cpu_time_remaining)) continue;
    }
    return ERR_XML_PARSE;
}

int CLIENT_PLATFORM::parse(XML_PARSER& xp) {
    strcpy(name, "");
    while (!xp.get_tag()) {
        if (xp.match_tag("/alt_platform")) return 0;
        if (xp.parse_str("name", name, sizeof(name))) continue;
    }
    return ERR_XML_PARSE;
}


void WORK_REQ::add_no_work_message(const char* message) {
    for (unsigned int i=0; i<no_work_messages.size(); i++) {
        if (!strcmp(message, no_work_messages.at(i).message.c_str())){
            return;
        }
    }
    no_work_messages.push_back(USER_MESSAGE(message, "notice"));
}

// return an error message or NULL
//
const char* SCHEDULER_REQUEST::parse(XML_PARSER& xp) {
    SCHED_DB_RESULT result;
    int retval;

    strcpy(authenticator, "");
    strcpy(platform.name, "");
    strcpy(cross_project_id, "");
    hostid = 0;
    core_client_major_version = 0;
    core_client_minor_version = 0;
    core_client_release = 0;
    core_client_version = 0;
    rpc_seqno = 0;
    work_req_seconds = 0;
    cpu_req_secs = 0;
    cpu_req_instances = 0;
    resource_share_fraction = 1.0;
    rrs_fraction = 1.0;
    prrs_fraction = 1.0;
    cpu_estimated_delay = 0;
    strcpy(global_prefs_xml, "");
    strcpy(working_global_prefs_xml, "");
    strcpy(code_sign_key, "");
    memset(&global_prefs, 0, sizeof(global_prefs));
    memset(&host, 0, sizeof(host));
    have_other_results_list = false;
    have_ip_results_list = false;
    have_time_stats_log = false;
    client_cap_plan_class = false;
    sandbox = -1;
    allow_multiple_clients = -1;
    results_truncated = false;

    if (xp.get_tag()) {
        return "xp.get_tag() failed";
    }
    if (xp.match_tag("?xml")) {
        xp.get_tag();
    }
    if (!xp.match_tag("scheduler_request")) return "no start tag";
    while (!xp.get_tag()) {
        if (xp.match_tag("/scheduler_request")) {
            core_client_version = 10000*core_client_major_version + 100*core_client_minor_version + core_client_release;
            return NULL;
        }
        if (xp.parse_str("authenticator", authenticator, sizeof(authenticator))) {
            remove_quotes(authenticator);
            continue;
        }
        if (xp.parse_str("cross_project_id", cross_project_id, sizeof(cross_project_id))) continue;
        if (xp.parse_int("hostid", hostid)) continue;
        if (xp.parse_int("rpc_seqno", rpc_seqno)) continue;
        if (xp.parse_str("platform_name", platform.name, sizeof(platform.name))) continue;
        if (xp.match_tag("alt_platform")) {
            CLIENT_PLATFORM cp;
            retval = cp.parse(xp);
            if (!retval) {
                alt_platforms.push_back(cp);
            }
            continue;
        }
        if (xp.match_tag("app_versions")) {
            while (!xp.get_tag()) {
                if (xp.match_tag("/app_versions")) break;
                if (xp.match_tag("app_version")) {
                    CLIENT_APP_VERSION cav;
                    retval = cav.parse(xp);
                    if (retval) {
                        if (!strcmp(platform.name, "anonymous")) {
                            if (retval == ERR_NOT_FOUND) {
                                g_reply->insert_message(
                                    _("Unknown app name in app_info.xml"),
                                    "notice"
                                );
                            } else {
                                g_reply->insert_message(
                                    _("Syntax error in app_info.xml"),
                                    "notice"
                                );
                            }
                        } else {
                            // this case happens if the app version
                            // refers to a deprecated app
                        }
                        cav.app = 0;
                    }
                    // store the CLIENT_APP_VERSION even if it didn't parse.
                    // This is necessary to maintain the correspondence
                    // with result.app_version
                    //
                    client_app_versions.push_back(cav);
                }
            }
            continue;
        }
        if (xp.parse_int("core_client_major_version", core_client_major_version)) continue;
        if (xp.parse_int("core_client_minor_version", core_client_minor_version)) continue;
        if (xp.parse_int("core_client_release", core_client_release)) continue;
        if (xp.parse_double("work_req_seconds", work_req_seconds)) continue;
        if (xp.parse_double("cpu_req_secs", cpu_req_secs)) continue;
        if (xp.parse_double("cpu_req_instances", cpu_req_instances)) continue;
        if (xp.parse_double("resource_share_fraction", resource_share_fraction)) continue;
        if (xp.parse_double("rrs_fraction", rrs_fraction)) continue;
        if (xp.parse_double("prrs_fraction", prrs_fraction)) continue;
        if (xp.parse_double("estimated_delay", cpu_estimated_delay)) continue;
        if (xp.parse_double("duration_correction_factor", host.duration_correction_factor)) continue;
        if (xp.match_tag("global_preferences")) {
            strcpy(global_prefs_xml, "<global_preferences>\n");
            char buf[BLOB_SIZE];
            retval = xp.element_contents(
                "</global_preferences>", buf, sizeof(buf)
            );
            if (retval) return "error copying global prefs";
            safe_strcat(global_prefs_xml, buf);
            safe_strcat(global_prefs_xml, "</global_preferences>\n");
            continue;
        }
        if (xp.match_tag("working_global_preferences")) {
            retval = xp.element_contents(
                "</working_global_preferences>",
                working_global_prefs_xml,
                sizeof(working_global_prefs_xml)
            );
            if (retval) return "error copying working global prefs";
            continue;
        }
        if (xp.parse_str("global_prefs_source_email_hash", global_prefs_source_email_hash, sizeof(global_prefs_source_email_hash))) continue;
        if (xp.match_tag("host_info")) {
            host.parse(xp);
            continue;
        }
        if (xp.match_tag("time_stats")) {
            host.parse_time_stats(xp);
            continue;
        }
        if (xp.match_tag("time_stats_log")) {
            handle_time_stats_log(xp.f->f);
            have_time_stats_log = true;
            continue;
        }
        if (xp.match_tag("net_stats")) {
            host.parse_net_stats(xp);
            continue;
        }
        if (xp.match_tag("disk_usage")) {
            host.parse_disk_usage(xp);
            continue;
        }
        if (xp.match_tag("result")) {
            retval = result.parse_from_client(xp);
            if (retval) continue;
            if (strstr(result.name, "download") || strstr(result.name, "upload")) {
                file_xfer_results.push_back(result);
                continue;
            }
            if (config.max_results_accepted && (int)(results.size()) >= config.max_results_accepted) {
                results_truncated = true;
                continue;
            }
            // check if client is sending the same result twice.
            // Shouldn't happen, but if it does bad things will happen
            //
            bool found = false;
            for (unsigned int i=0; i<results.size(); i++) {
                if (!strcmp(results[i].name, result.name)) {
                    found = true;
                    break;
                }
            }
            if (!found) {
                results.push_back(result);
            }
            continue;
        }
        if (xp.match_tag("code_sign_key")) {
            copy_element_contents(xp.f->f, "</code_sign_key>", code_sign_key, sizeof(code_sign_key));
            strip_whitespace(code_sign_key);
            continue;
        }
        if (xp.match_tag("msg_from_host")) {
            MSG_FROM_HOST_DESC md;
            retval = md.parse(xp);
            if (!retval) {
                msgs_from_host.push_back(md);
            }
            continue;
        }
        if (xp.match_tag("file_info")) {
            FILE_INFO fi;
            retval = fi.parse(xp);
            if (!retval) {
                file_infos.push_back(fi);
            }
            continue;
        }
        if (xp.match_tag("host_venue")) {
            continue;
        }
        if (xp.match_tag("other_results")) {
            have_other_results_list = true;
            while (!xp.get_tag()) {
                if (xp.match_tag("/other_results")) break;
                if (xp.match_tag("other_result")) {
                    OTHER_RESULT o_r;
                    retval = o_r.parse(xp);
                    if (!retval) {
                        other_results.push_back(o_r);
                    }
                }
            }
            continue;
        }
        if (xp.match_tag("in_progress_results")) {
            have_ip_results_list = true;
            int i = 0;
            double now = time(0);
            while (!xp.get_tag()) {
                if (xp.match_tag("/in_progress_results")) break;
                if (xp.match_tag("ip_result")) {
                    IP_RESULT ir;
                    retval = ir.parse(xp);
                    if (!retval) {
                        if (!strlen(ir.name)) {
                            sprintf(ir.name, "ip%d", i++);
                        }
                        ir.report_deadline -= now;
                        ip_results.push_back(ir);
                    }
                }
            }
            continue;
        }
        if (xp.match_tag("coprocs")) {
            coprocs.parse(xp);
            continue;
        }
        if (xp.parse_bool("client_cap_plan_class", client_cap_plan_class)) continue;
        if (xp.parse_int("sandbox", sandbox)) continue;
        if (xp.parse_int("allow_multiple_clients", allow_multiple_clients)) continue;
        if (xp.parse_string("client_opaque", client_opaque)) continue;

        if (xp.match_tag("active_task_set")) continue;
        if (xp.match_tag("app")) continue;
        if (xp.match_tag("app_version")) continue;
        if (xp.match_tag("duration_variability")) continue;
        if (xp.match_tag("new_version_check_time")) continue;
        if (xp.match_tag("newer_version")) continue;
        if (xp.match_tag("project")) continue;
        if (xp.match_tag("project_files")) continue;
        if (xp.match_tag("proxy_info")) continue;
        if (xp.match_tag("user_network_request")) continue;
        if (xp.match_tag("user_run_request")) continue;
        if (xp.match_tag("master_url")) continue;
        if (xp.match_tag("project_name")) continue;
        if (xp.match_tag("user_name")) continue;
        if (xp.match_tag("team_name")) continue;
        if (xp.match_tag("email_hash")) continue;
        if (xp.match_tag("user_total_credit")) continue;
        if (xp.match_tag("user_expavg_credit")) continue;
        if (xp.match_tag("user_create_time")) continue;
        if (xp.match_tag("host_total_credit")) continue;
        if (xp.match_tag("host_expavg_credit")) continue;
        if (xp.match_tag("host_create_time")) continue;
        if (xp.match_tag("nrpc_failures")) continue;
        if (xp.match_tag("master_fetch_failures")) continue;
        if (xp.match_tag("min_rpc_time")) continue;
        if (xp.match_tag("short_term_debt")) continue;
        if (xp.match_tag("long_term_debt")) continue;
        if (xp.match_tag("resource_share")) continue;
        if (xp.match_tag("scheduler_url")) continue;
        if (xp.match_tag("/project")) continue;
        if (xp.match_tag("?xml")) continue;

        log_messages.printf(MSG_NORMAL,
            "SCHEDULER_REQUEST::parse(): unexpected: %s\n", xp.parsed_tag
        );
        xp.skip_unexpected();
    }
    return "no end tag";
}

// I'm not real sure why this is here.
// Why not copy the request message directly?
//
int SCHEDULER_REQUEST::write(FILE* fout) {
    unsigned int i;

    fprintf(fout,
        "<scheduler_request>\n"
        "  <authenticator>%s</authentiicator>\n"
        "  <platform_name>%s</platform_name>\n"
        "  <cross_project_id>%s</cross_project_id>\n"
        "  <hostid>%d</hostid>\n"
        "  <core_client_major_version>%d</core_client_major_version>\n"
        "  <core_client_minor_version>%d</core_client_minor_version>\n"
        "  <core_client_release>%d</core_client_release>\n"
        "  <rpc_seqno>%d</rpc_seqno>\n"
        "  <work_req_seconds>%.15f</work_req_seconds>\n"
        "  <resource_share_fraction>%.15f</resource_share_fraction>\n"
        "  <rrs_fraction>%.15f</rrs_fraction>\n"
        "  <prrs_fraction>%.15f</prrs_fraction>\n"
        "  <estimated_delay>%.15f</estimated_delay>\n"
        "  <code_sign_key>%s</code_sign_key>\n"
        "  <anonymous_platform>%s</anonymous_platform>\n",
        authenticator,
        platform.name,
        cross_project_id,
        hostid,
        core_client_major_version,
        core_client_minor_version,
        core_client_release,
        rpc_seqno,
        work_req_seconds,
        resource_share_fraction,
        rrs_fraction,
        prrs_fraction,
        cpu_estimated_delay,
        code_sign_key,
        is_anonymous(platforms.list[0])?"true":"false"
    );

    for (i=0; i<client_app_versions.size(); i++) {
        fprintf(fout,
            "  <app_version>\n"
            "    <app_name>%s</app_name>\n"
            "    <version_num>%d</version_num>\n"
            "  </app_version>\n",
            client_app_versions[i].app_name,
            client_app_versions[i].version_num
        );
    }

    fprintf(fout,
        "  <global_prefs_xml>\n"
        "    %s"
        "  </globals_prefs_xml>\n",
        global_prefs_xml
    );
  
    fprintf(fout,
        "  <global_prefs_source_email_hash>%s</global_prefs_source_email_hash>\n",
        global_prefs_source_email_hash
    );
  
    fprintf(fout,
        "  <host>\n"
        "    <id>%d</id>\n"
        "    <rpc_time>%d</rpc_time>\n"
        "    <timezone>%d</timezone>\n"
        "    <d_total>%.15f</d_total>\n"
        "    <d_free>%.15f</d_free>\n"
        "    <d_boinc_used_total>%.15f</d_boinc_used_total>\n"
        "    <d_boinc_used_project>%.15f</d_boinc_used_project>\n"
        "    <d_boinc_max>%.15f</d_boinc_max>\n",
        host.id,
        host.rpc_time,
        host.timezone,
        host.d_total,
        host.d_free,
        host.d_boinc_used_total,
        host.d_boinc_used_project,
        host.d_boinc_max
    );

    for (i=0; i<results.size(); i++) {
        fprintf(fout,
            "  <result>\n"
            "    <name>%s</name>\n"
            "    <client_state>%d</client_state>\n"
            "    <cpu_time>%.15f</cpu_time>\n"
            "    <exit_status>%d</exit_status>\n"
            "    <app_version_num>%d</app_version_num>\n"
            "  </result>\n",
            results[i].name,
            results[i].client_state,
            results[i].cpu_time,
            results[i].exit_status,
            results[i].app_version_num
        );
    }
  
    for (i=0; i<msgs_from_host.size(); i++) {
        fprintf(fout,
            "  <msg_from_host>\n"
            "    <variety>%s</variety>\n"
            "    <msg_text>%s</msg_text>\n"
            "  </msg_from_host>\n",
            msgs_from_host[i].variety,
            msgs_from_host[i].msg_text.c_str()
        );
    }

    for (i=0; i<file_infos.size(); i++) {
        fprintf(fout,
            "  <file_info>\n"
            "    <name>%s</name>\n"
            "  </file_info>\n",
            file_infos[i].name
        );
        fprintf(fout, "</scheduler_request>\n");
    }
    return 0;
}

int MSG_FROM_HOST_DESC::parse(XML_PARSER& xp) {
    char buf[256];

    msg_text = "";
    MIOFILE& in = *(xp.f);
    while (in.fgets(buf, sizeof(buf))) {
        if (match_tag(buf, "</msg_from_host>")) return 0;
        if (parse_str(buf, "<variety>", variety, sizeof(variety))) continue;
        msg_text += buf;
    }
    return ERR_XML_PARSE;
}

SCHEDULER_REPLY::SCHEDULER_REPLY() {
    memset(&wreq, 0, sizeof(wreq));
    memset(&disk_limits, 0, sizeof(disk_limits));
    request_delay = 0;
    hostid = 0;
    send_global_prefs = false;
    strcpy(code_sign_key, "");
    strcpy(code_sign_key_signature, "");
    memset(&user, 0, sizeof(user));
    memset(&host, 0, sizeof(host));
    memset(&team, 0, sizeof(team));
    nucleus_only = false;
    project_is_down = false;
    send_msg_ack = false;
    strcpy(email_hash, "");
}

int SCHEDULER_REPLY::write(FILE* fout, SCHEDULER_REQUEST& sreq) {
    unsigned int i;
    char buf[BLOB_SIZE];

    // Note: at one point we had
    // "<?xml version=\"1.0\" encoding=\"ISO-8859-1\" ?>\n"
    // after the Content-type (to make it legit XML),
    // but this broke 4.19 clients
    //
    fprintf(fout,
        "Content-type: text/xml\n\n"
        "<scheduler_reply>\n"
        "<scheduler_version>%d</scheduler_version>\n",
        BOINC_MAJOR_VERSION*100+BOINC_MINOR_VERSION
    );
    if (sreq.core_client_version >= 70028) {
        fprintf(fout, "<dont_use_dcf/>\n");
    }
    if (strlen(config.master_url)) {
        fprintf(fout,
            "<master_url>%s</master_url>\n",
            config.master_url
        );
    }
    if (config.ended) {
        fprintf(fout, "   <ended>1</ended>\n");
    }

    // if the scheduler has requested a delay OR the sysadmin has configured
    // the scheduler with a minimum time between RPCs, send a delay request.
    // Make it 1% larger than the min required to take care of time skew.
    // If this is less than one second bigger, bump up by one sec.
    //
    if (request_delay || config.min_sendwork_interval) {
        double min_delay_needed = 1.01*config.min_sendwork_interval;
        if (min_delay_needed < config.min_sendwork_interval+1) {
            min_delay_needed = config.min_sendwork_interval+1;
        }
        if (request_delay<min_delay_needed) {
            request_delay = min_delay_needed; 
        }
        fprintf(fout, "<request_delay>%f</request_delay>\n", request_delay);
    }
    log_messages.printf(MSG_NORMAL,
        "Sending reply to [HOST#%d]: %d results, delay req %.2f\n",
        host.id, wreq.njobs_sent, request_delay
    );

    if (sreq.core_client_version <= 41900) {
        string msg;
        string pri = "low";
        for (i=0; i<messages.size(); i++) {
            USER_MESSAGE& um = messages[i];
            msg += um.message + string(" ");
            if (um.priority == "notice") {
                pri = "notice";
            }
        }
        if (messages.size()>0) {
            // any newlines will break message printing under 4.19 and under!
            // replace them with spaces.
            //
            while (1) {
                string::size_type pos = msg.find("\n", 0);
                if (pos == string::npos) break;
                msg.replace(pos, 1, " ");
            }
            fprintf(fout,
                "<message priority=\"%s\">%s</message>\n",
                pri.c_str(), msg.c_str()
            );
        }
    } else if (sreq.core_client_version <= 61100) {
        char prio[256];
        for (i=0; i<messages.size(); i++) {
            USER_MESSAGE& um = messages[i];
            strcpy(prio, um.priority.c_str());
            if (!strcmp(prio, "notice")) {
                strcpy(prio, "high");
            }
            fprintf(fout,
                "<message priority=\"%s\">%s</message>\n",
                prio,
                um.message.c_str()
            );
        }
    } else {
        for (i=0; i<messages.size(); i++) {
            USER_MESSAGE& um = messages[i];
            fprintf(fout,
                "<message priority=\"%s\">%s</message>\n",
                um.priority.c_str(),
                um.message.c_str()
            );
        }
    }
    fprintf(fout,
        "<project_name>%s</project_name>\n",
        config.long_name
    );

    if (config.request_time_stats_log) {
        if (!have_time_stats_log()) {
            fprintf(fout, "<send_time_stats_log>1</send_time_stats_log>\n");
        }
    }

    if (project_is_down) {
        fprintf(fout,"<project_is_down/>\n");
        goto end;
    }
    if (config.workload_sim) {
        fprintf(fout, "<send_full_workload/>\n");
    }

    if (nucleus_only) goto end;

    if (strlen(config.symstore)) {
        fprintf(fout, "<symstore>%s</symstore>\n", config.symstore);
    }
    if (config.next_rpc_delay) {
        fprintf(fout, "<next_rpc_delay>%f</next_rpc_delay>\n", config.next_rpc_delay);
    }
    if (user.id) {
        xml_escape(user.name, buf, sizeof(buf));
        fprintf(fout,
            "<userid>%d</userid>\n"
            "<user_name>%s</user_name>\n"
            "<user_total_credit>%f</user_total_credit>\n"
            "<user_expavg_credit>%f</user_expavg_credit>\n"
            "<user_create_time>%d</user_create_time>\n",
            user.id,
            buf,
            user.total_credit,
            user.expavg_credit,
            user.create_time
        );
        // be paranoid about the following to avoid sending null
        //
        if (strlen(email_hash)) {
            fprintf(fout,
                "<email_hash>%s</email_hash>\n",
                email_hash
            );
        }
        if (strlen(user.cross_project_id)) {
            fprintf(fout,
                "<cross_project_id>%s</cross_project_id>\n",
                user.cross_project_id
            );
        }

        if (send_global_prefs) {
            fputs(user.global_prefs, fout);
            fputs("\n", fout);
        }

        // always send project prefs
        //
        fputs(user.project_prefs, fout);
        fputs("\n", fout);

    }
    if (hostid) {
        fprintf(fout,
            "<hostid>%d</hostid>\n",
            hostid
        );
    }
    fprintf(fout,
        "<host_total_credit>%f</host_total_credit>\n"
        "<host_expavg_credit>%f</host_expavg_credit>\n"
        "<host_venue>%s</host_venue>\n"
        "<host_create_time>%d</host_create_time>\n",
        host.total_credit,
        host.expavg_credit,
        host.venue,
        host.create_time
    );

    // might want to send team credit too.
    //
    if (team.id) {
        xml_escape(team.name, buf, sizeof(buf));
        fprintf(fout,
            "<teamid>%d</teamid>\n"
            "<team_name>%s</team_name>\n",
            team.id,
            buf
        );
    } else {
        fprintf(fout,
            "<team_name></team_name>\n"
        );
    }

    // acknowledge results
    //
    for (i=0; i<result_acks.size(); i++) {
        fprintf(fout,
            "<result_ack>\n"
            "    <name>%s</name>\n"
            "</result_ack>\n",
            result_acks[i].c_str()
        );
    }

    // abort results
    //
    for (i=0; i<result_aborts.size(); i++) {
        fprintf(fout,
            "<result_abort>\n"
            "    <name>%s</name>\n"
            "</result_abort>\n",
            result_aborts[i].c_str()
        );
    }

    // abort results not started
    //
    for (i=0; i<result_abort_if_not_starteds.size(); i++) {
        fprintf(fout,
            "<result_abort_if_not_started>\n"
            "    <name>%s</name>\n"
            "</result_abort_if_not_started>\n",
            result_abort_if_not_starteds[i].c_str()
        );
    }

    for (i=0; i<apps.size(); i++) {
        apps[i].write(fout);
    }

    for (i=0; i<app_versions.size(); i++) {
        app_versions[i].write(fout);
    }

    for (i=0; i<wus.size(); i++) {
        fputs(wus[i].xml_doc, fout);
        fputs("\n", fout);  // for old clients
    }

    for (i=0; i<results.size(); i++) {
        results[i].write_to_client(fout);
    }

    if (strlen(code_sign_key)) {
        fputs("<code_sign_key>\n", fout);
        fputs(code_sign_key, fout);
        fputs("\n</code_sign_key>\n", fout);
    }

    if (strlen(code_sign_key_signature)) {
        fputs("<code_sign_key_signature>\n", fout);
        fputs(code_sign_key_signature, fout);
        fputs("</code_sign_key_signature>\n", fout);
    }

    if (send_msg_ack) {
        fputs("<message_ack/>\n", fout);
    }

    for (i=0; i<msgs_to_host.size(); i++) {
        MSG_TO_HOST& md = msgs_to_host[i];
        fprintf(fout, "%s\n", md.xml);
    }

    if (config.non_cpu_intensive) {
        fprintf(fout, "<non_cpu_intensive/>\n");
    }

    if (config.verify_files_on_app_start) {
        fprintf(fout, "<verify_files_on_app_start/>\n");
    }

    for (i=0; i<file_deletes.size(); i++) {
        fprintf(fout,
            "<delete_file_info>%s</delete_file_info>\n",
            file_deletes[i].name
        );
    }
    for (i=0; i<file_transfer_requests.size(); i++) {
        fprintf(fout, "%s", file_transfer_requests[i].c_str());
    }

    fprintf(fout,
        "<no_cpu_apps>%d</no_cpu_apps>\n"
        "<no_cuda_apps>%d</no_cuda_apps>\n"
        "<no_ati_apps>%d</no_ati_apps>\n"
        "<no_intel_apps>%d</no_intel_apps>\n",
        ssp->have_apps_for_proc_type[PROC_TYPE_CPU]?0:1,
        ssp->have_apps_for_proc_type[PROC_TYPE_NVIDIA]?0:1,
        ssp->have_apps_for_proc_type[PROC_TYPE_AMD]?0:1,
        ssp->have_apps_for_proc_type[PROC_TYPE_INTEL]?0:1,
        ssp->
    );
    gui_urls.get_gui_urls(user, host, team, buf);
    fputs(buf, fout);
    if (project_files.text) {
        fputs(project_files.text, fout);
        fprintf(fout, "\n");
    }

end:
    fprintf(fout,
        "</scheduler_reply>\n"
    );
    return 0;
}

// set delay to the MAX of the existing value or the requested value
// never send a delay request longer than two days.
//
void SCHEDULER_REPLY::set_delay(double delay) {
    if (request_delay < delay) {
        request_delay = delay;
    }
    if (request_delay > DELAY_MAX) {
        request_delay = DELAY_MAX;
    }
} 


void SCHEDULER_REPLY::insert_app_unique(APP& app) {
    unsigned int i;
    for (i=0; i<apps.size(); i++) {
        if (app.id == apps[i].id) return;
    }
    apps.push_back(app);
}

void SCHEDULER_REPLY::insert_app_version_unique(APP_VERSION& av) {
    unsigned int i;
    for (i=0; i<app_versions.size(); i++) {
        if (av.id == app_versions[i].id) return;
    }
    app_versions.push_back(av);
}

void SCHEDULER_REPLY::insert_workunit_unique(WORKUNIT& wu) {
    unsigned int i;
    for (i=0; i<wus.size(); i++) {
        if (wu.id == wus[i].id) return;
    }
    wus.push_back(wu);
}

void SCHEDULER_REPLY::insert_result(SCHED_DB_RESULT& result) {
    results.push_back(result);
}

void SCHEDULER_REPLY::insert_message(const char* msg, const char* prio) {
    messages.push_back(USER_MESSAGE(msg, prio));
}

void SCHEDULER_REPLY::insert_message(USER_MESSAGE& um) {
    messages.push_back(um);
}

USER_MESSAGE::USER_MESSAGE(const char* m, const char* p) {
    if (g_request->core_client_version < 61200) {
        char buf[1024];
        strcpy(buf, m);
        strip_translation(buf);
        message = buf;
    } else {
        message = m;
    }
    priority = p;
}

int APP::write(FILE* fout) {
    fprintf(fout,
        "<app>\n"
        "    <name>%s</name>\n"
        "    <user_friendly_name>%s</user_friendly_name>\n"
        "</app>\n",
        name, user_friendly_name
    );
    return 0;
}

int APP_VERSION::write(FILE* fout) {
    char buf[APP_VERSION_XML_BLOB_SIZE];

    strcpy(buf, xml_doc);
    char* p = strstr(buf, "</app_version>");
    if (!p) {
        fprintf(stderr, "ERROR: app version %d XML has no end tag!\n", id);
        return -1;
    }
    *p = 0;
    fputs(buf, fout);
    PLATFORM* pp = ssp->lookup_platform_id(platformid);
    fprintf(fout, "    <platform>%s</platform>\n", pp->name);
    if (strlen(plan_class)) {
        fprintf(fout, "    <plan_class>%s</plan_class>\n", plan_class);
    }
    fprintf(fout,
        "    <avg_ncpus>%f</avg_ncpus>\n"
        "    <max_ncpus>%f</max_ncpus>\n"
        "    <flops>%f</flops>\n",
        bavp->host_usage.avg_ncpus,
        bavp->host_usage.max_ncpus,
        bavp->host_usage.projected_flops
    );
    if (strlen(bavp->host_usage.cmdline)) {
        fprintf(fout,
            "    <cmdline>%s</cmdline>\n",
            bavp->host_usage.cmdline
        );
    }
    int pt = bavp->host_usage.proc_type;
    if (pt != PROC_TYPE_CPU) {
        fprintf(fout,
            "    <coproc>\n"
            "        <type>%s</type>\n"
            "        <count>%f</count>\n"
            "    </coproc>\n",
            proc_type_name(pt),
            bavp->host_usage.gpu_usage
        );
    }
    if (bavp->host_usage.gpu_ram) {
        fprintf(fout,
            "    <gpu_ram>%f</gpu_ram>\n",
            bavp->host_usage.gpu_ram
        );
    }
    fputs("</app_version>\n", fout);
    return 0;
}

int SCHED_DB_RESULT::write_to_client(FILE* fout) {
    char buf[BLOB_SIZE];

    strcpy(buf, xml_doc_in);
    char* p = strstr(buf, "</result>");
    if (!p) {
        fprintf(stderr, "ERROR: result %d XML has no end tag!\n", id);
        return -1;
    }
    *p = 0;
    fputs(buf, fout);
    fputs("\n", fout);  // for old clients

    APP_VERSION* avp = bav.avp;
    CLIENT_APP_VERSION* cavp = bav.cavp;
    if (avp) {
        PLATFORM* pp = ssp->lookup_platform_id(avp->platformid);
        fprintf(fout,
            "    <platform>%s</platform>\n"
            "    <version_num>%d</version_num>\n"
            "    <plan_class>%s</plan_class>\n",
            pp->name, avp->version_num, avp->plan_class
        );
    } else if (cavp) {
        fprintf(fout,
            "    <platform>%s</platform>\n"
            "    <version_num>%d</version_num>\n"
            "    <plan_class>%s</plan_class>\n",
            cavp->platform, cavp->version_num, cavp->plan_class
        );
    }

    fputs("</result>\n", fout);
    return 0;
}

int SCHED_DB_RESULT::parse_from_client(XML_PARSER& xp) {
    double dtemp;
    bool btemp;
    string stemp;
    int itemp;

    // should be non-zero if exit_status is not found
    exit_status = ERR_NO_EXIT_STATUS;
    memset(this, 0, sizeof(*this));
    while (!xp.get_tag()) {
        if (xp.match_tag("/result")) {
            return 0;
        }
        if (xp.parse_str("name", name, sizeof(name))) continue;
        if (xp.parse_int("state", client_state)) continue;
        if (xp.parse_double("final_cpu_time", cpu_time)) continue;
        if (xp.parse_double("final_elapsed_time", elapsed_time)) continue;
        if (xp.parse_int("exit_status", exit_status)) continue;
        if (xp.parse_int("app_version_num", app_version_num)) continue;
        if (xp.match_tag("file_info")) {
            string s;
            xp.copy_element(s);
            safe_strcat(xml_doc_out, s.c_str());
            continue;
        }
        if (xp.match_tag("stderr_out" )) {
            copy_element_contents(xp.f->f, "</stderr_out>", stderr_out, sizeof(stderr_out));
            continue;
        }
        if (xp.parse_string("platform", stemp)) continue;
        if (xp.parse_int("version_num", itemp)) continue;
        if (xp.parse_string("plan_class", stemp)) continue;
        if (xp.parse_double("completed_time", dtemp)) continue;
        if (xp.parse_string("file_name", stemp)) continue;
        if (xp.match_tag("file_ref")) {
            xp.copy_element(stemp);
            continue;
        }
        if (xp.parse_string("open_name", stemp)) continue;
        if (xp.parse_bool("ready_to_report", btemp)) continue;
        if (xp.parse_double("report_deadline", dtemp)) continue;
        if (xp.parse_string("wu_name", stemp)) continue;

        // deprecated stuff
        if (xp.parse_double("fpops_per_cpu_sec", dtemp)) continue;
        if (xp.parse_double("fpops_cumulative", dtemp)) continue;
        if (xp.parse_double("intops_per_cpu_sec", dtemp)) continue;
        if (xp.parse_double("intops_cumulative", dtemp)) continue;

        log_messages.printf(MSG_NORMAL,
            "RESULT::parse_from_client(): unrecognized: %s\n",
            xp.parsed_tag
        );
    }
    return ERR_XML_PARSE;
}

int HOST::parse(XML_PARSER& xp) {
    p_ncpus = 1;
    double dtemp;
    string stemp;
    while (!xp.get_tag()) {
        if (xp.match_tag("/host_info")) return 0;
        if (xp.parse_int("timezone", timezone)) continue;
        if (xp.parse_str("domain_name", domain_name, sizeof(domain_name))) continue;
        if (xp.parse_str("ip_addr", last_ip_addr, sizeof(last_ip_addr))) continue;
        if (xp.parse_str("host_cpid", host_cpid, sizeof(host_cpid))) continue;
        if (xp.parse_int("p_ncpus", p_ncpus)) continue;
        if (xp.parse_str("p_vendor", p_vendor, sizeof(p_vendor))) continue;
        if (xp.parse_str("p_model", p_model, sizeof(p_model))) continue;
        if (xp.parse_double("p_fpops", p_fpops)) continue;
        if (xp.parse_double("p_iops", p_iops)) continue;
        if (xp.parse_double("p_membw", p_membw)) continue;
        if (xp.parse_str("os_name", os_name, sizeof(os_name))) continue;
        if (xp.parse_str("os_version", os_version, sizeof(os_version))) continue;
        if (xp.parse_double("m_nbytes", m_nbytes)) continue;
        if (xp.parse_double("m_cache", m_cache)) continue;
        if (xp.parse_double("m_swap", m_swap)) continue;
        if (xp.parse_double("d_total", d_total)) continue;
        if (xp.parse_double("d_free", d_free)) continue;
        if (xp.parse_double("n_bwup", n_bwup)) continue;
        if (xp.parse_double("n_bwdown", n_bwdown)) continue;
        if (xp.parse_str("p_features", p_features, sizeof(p_features))) continue;
        if (xp.parse_str("virtualbox_version", virtualbox_version, sizeof(virtualbox_version))) continue;
        if (xp.parse_bool("p_vm_extensions_disabled", p_vm_extensions_disabled)) continue;

        // parse deprecated fields to avoid error messages
        //
        if (xp.parse_double("p_calculated", dtemp)) continue;
        if (xp.match_tag("p_fpop_err")) continue;
        if (xp.match_tag("p_iop_err")) continue;
        if (xp.match_tag("p_membw_err")) continue;

        // fields reported by 5.5+ clients, not currently used
        //
        if (xp.parse_string("p_capabilities", stemp)) continue;
        if (xp.parse_string("accelerators", stemp)) continue;

#if 1
        // not sure where these fields belong in the above categories
        //
        if (xp.parse_string("cpu_caps", stemp)) continue;
        if (xp.parse_string("cache_l1", stemp)) continue;
        if (xp.parse_string("cache_l2", stemp)) continue;
        if (xp.parse_string("cache_l3", stemp)) continue;
#endif

        log_messages.printf(MSG_NORMAL,
            "HOST::parse(): unrecognized: %s\n", xp.parsed_tag
        );
    }
    return ERR_XML_PARSE;
}


int HOST::parse_time_stats(XML_PARSER& xp) {
    while (!xp.get_tag()) {
        if (xp.match_tag("/time_stats")) return 0;
        if (xp.parse_double("on_frac", on_frac)) continue;
        if (xp.parse_double("connected_frac", connected_frac)) continue;
        if (xp.parse_double("active_frac", active_frac)) continue;
#if 0
        if (xp.match_tag("outages")) continue;
        if (xp.match_tag("outage")) continue;
        if (xp.match_tag("start")) continue;
        if (xp.match_tag("end")) continue;
        log_messages.printf(MSG_NORMAL,
            "HOST::parse_time_stats(): unrecognized: %s\n",
            xp.parsed_tag
        );
#endif
    }
    return ERR_XML_PARSE;
}

int HOST::parse_net_stats(XML_PARSER& xp) {
    double dtemp;
    while (!xp.get_tag()) {
        if (xp.match_tag("/net_stats")) return 0;
        if (xp.parse_double("bwup", n_bwup)) continue;
        if (xp.parse_double("bwdown", n_bwdown)) continue;

        // items reported by 5.10+ clients, not currently used
        //
        if (xp.parse_double("avg_time_up", dtemp)) continue;
        if (xp.parse_double("avg_up", dtemp)) continue;
        if (xp.parse_double("avg_time_down", dtemp)) continue;
        if (xp.parse_double("avg_down", dtemp)) continue;

        log_messages.printf(MSG_NORMAL,
            "HOST::parse_net_stats(): unrecognized: %s\n",
            xp.parsed_tag
        );
    }
    return ERR_XML_PARSE;
}

int HOST::parse_disk_usage(XML_PARSER& xp) {
    while (!xp.get_tag()) {
        if (xp.match_tag("/disk_usage")) return 0;
        if (xp.parse_double("d_boinc_used_total", d_boinc_used_total)) continue;
        if (xp.parse_double("d_boinc_used_project", d_boinc_used_project)) continue;
        if (xp.parse_double("d_project_share", d_project_share)) continue;
        log_messages.printf(MSG_NORMAL,
            "HOST::parse_disk_usage(): unrecognized: %s\n",
            xp.parsed_tag
        );
    }
    return ERR_XML_PARSE;
}

void GLOBAL_PREFS::parse(const char* buf, const char* venue) {
    char buf2[BLOB_SIZE];
    double dtemp;

    defaults();

    if (parse_double(buf, "<mod_time>", mod_time)) {
        // mod_time is outside of venue
        if (mod_time > dtime()) mod_time = dtime();
    }
    extract_venue(buf, venue, buf2);
    parse_double(buf2, "<disk_max_used_gb>", disk_max_used_gb);
    parse_double(buf2, "<disk_max_used_pct>", disk_max_used_pct);
    parse_double(buf2, "<disk_min_free_gb>", disk_min_free_gb);
    parse_double(buf2, "<work_buf_min_days>", work_buf_min_days);
    if (parse_double(buf2, "<ram_max_used_busy_pct>", dtemp)) {
        ram_max_used_busy_frac = dtemp/100.;
    }
    if (parse_double(buf2, "<ram_max_used_idle_pct>", dtemp)) {
        ram_max_used_idle_frac = dtemp/100.;
    }
    parse_double(buf2, "<max_ncpus_pct>", max_ncpus_pct);
}

void GLOBAL_PREFS::defaults() {
    memset(this, 0, sizeof(GLOBAL_PREFS));
}

void GUI_URLS::init() {
    text = 0;
    read_file_malloc(config.project_path("gui_urls.xml"), text);
}

void GUI_URLS::get_gui_urls(USER& user, HOST& host, TEAM& team, char* buf) {
    bool found;
    char userid[256], teamid[256], hostid[256], weak_auth[256], rss_auth[256];
    strcpy(buf, "");
    if (!text) return;
    strcpy(buf, text);

    sprintf(userid, "%d", user.id);
    sprintf(hostid, "%d", host.id);
    if (user.teamid) {
        sprintf(teamid, "%d", team.id);
    } else {
        strcpy(teamid, "0");
        while (remove_element(buf, "<ifteam>", "</ifteam>")) {
            continue;
        }
    }

    get_weak_auth(user, weak_auth);
    get_rss_auth(user, rss_auth);
    while (1) {
        found = false;
        found |= str_replace(buf, "<userid/>", userid);
        found |= str_replace(buf, "<user_name/>", user.name);
        found |= str_replace(buf, "<hostid/>", hostid);
        found |= str_replace(buf, "<teamid/>", teamid);
        found |= str_replace(buf, "<team_name/>", team.name);
        found |= str_replace(buf, "<authenticator/>", user.authenticator);
        found |= str_replace(buf, "<weak_auth/>", weak_auth);
        found |= str_replace(buf, "<rss_auth/>", rss_auth);
        if (!found) break;
    }
}

void PROJECT_FILES::init() {
    text = 0;
    read_file_malloc(config.project_path("project_files.xml"), text);
}

void get_weak_auth(USER& user, char* buf) {
    char buf2[256], out[256];
    sprintf(buf2, "%s%s", user.authenticator, user.passwd_hash);
    md5_block((unsigned char*)buf2, strlen(buf2), out);
    sprintf(buf, "%d_%s", user.id, out);
}

void get_rss_auth(USER& user, char* buf) {
    char buf2[256], out[256];
    sprintf(buf2, "%s%s%s", user.authenticator, user.passwd_hash, "notify_rss");
    md5_block((unsigned char*)buf2, strlen(buf2), out);
    sprintf(buf, "%d_%s", user.id, out);
}

void read_host_app_versions() {
    DB_HOST_APP_VERSION hav;
    char clause[256];

    sprintf(clause, "where host_id=%d", g_reply->host.id);
    while (!hav.enumerate(clause)) {
        g_wreq->host_app_versions.push_back(hav);
    }
    g_wreq->host_app_versions_orig = g_wreq->host_app_versions;
}

DB_HOST_APP_VERSION* gavid_to_havp(int gavid) {
    for (unsigned int i=0; i<g_wreq->host_app_versions.size(); i++) {
        DB_HOST_APP_VERSION& hav = g_wreq->host_app_versions[i];
        if (hav.app_version_id == gavid) return &hav;
    }
    return NULL;
}

void write_host_app_versions() {
    for (unsigned int i=0; i<g_wreq->host_app_versions.size(); i++) {
        DB_HOST_APP_VERSION& hav = g_wreq->host_app_versions[i];
        DB_HOST_APP_VERSION& hav_orig = g_wreq->host_app_versions_orig[i];

        int retval = hav.update_scheduler(hav_orig);
        if (retval) {
            log_messages.printf(MSG_CRITICAL,
                "CRITICAL: hav.update_sched() error: %s\n", boincerror(retval)
            );
        }
    }
}

DB_HOST_APP_VERSION* BEST_APP_VERSION::host_app_version() {
    if (cavp) {
        return gavid_to_havp(
            generalized_app_version_id(host_usage.resource_type(), appid)
        );
    } else {
        return gavid_to_havp(avp->id);
    }
}

// return some HAV for which quota was exceeded
//
DB_HOST_APP_VERSION* quota_exceeded_version() {
    for (unsigned int i=0; i<g_wreq->host_app_versions.size(); i++) {
        DB_HOST_APP_VERSION& hav = g_wreq->host_app_versions[i];
        if (hav.daily_quota_exceeded) return &hav;
    }
    return NULL;
}

double capped_host_fpops() {
    double x = g_request->host.p_fpops;
    if (!ssp) return x;
    if (x <= 0) {
        return ssp->perf_info.host_fpops_50_percentile;
    }
    if (x > ssp->perf_info.host_fpops_95_percentile*1.1) {
        return ssp->perf_info.host_fpops_95_percentile*1.1;
    }
    return x;
}

const char *BOINC_RCSID_ea659117b3 = "$Id$";
