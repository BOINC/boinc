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

SCHEDULER_REQUEST* g_request;
SCHEDULER_REPLY* g_reply;
WORK_REQ* g_wreq;

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

int CLIENT_APP_VERSION::parse(FILE* f) {
    char buf[256];
    double x;

    memset(this, 0, sizeof(CLIENT_APP_VERSION));
    host_usage.avg_ncpus = 1;
    while (fgets(buf, sizeof(buf), f)) {
        if (match_tag(buf, "</app_version>")) {
            app = ssp->lookup_app_name(app_name);
            if (!app) return ERR_NOT_FOUND;

            double f = host_usage.avg_ncpus * g_reply->host.p_fpops;
            if (host_usage.ncudas && g_request->coprocs.cuda.count) {
                f += host_usage.ncudas*g_request->coprocs.cuda.peak_flops();
            }
            if (host_usage.natis && g_request->coprocs.ati.count) {
                f += host_usage.natis*g_request->coprocs.ati.peak_flops();
            }
            host_usage.projected_flops = f;
            host_usage.peak_flops = f;
            return 0;
        }
        if (parse_str(buf, "<app_name>", app_name, 256)) continue;
        if (parse_str(buf, "<platform>", platform, 256)) continue;
        if (parse_str(buf, "<plan_class>", plan_class, 256)) continue;
        if (parse_int(buf, "<version_num>", version_num)) continue;
        if (parse_double(buf, "<avg_ncpus>", x)) {
            if (x>0) host_usage.avg_ncpus = x;
            continue;
        }
        if (match_tag(buf, "<coproc>")) {
            COPROC_REQ coproc_req;
            MIOFILE mf;
            mf.init_file(f);
            int retval = coproc_req.parse(mf);
            if (!retval && !strcmp(coproc_req.type, "CUDA")) {
                host_usage.ncudas = coproc_req.count;
            }
            if (!retval && !strcmp(coproc_req.type, "ATI")) {
                host_usage.natis = coproc_req.count;
            }
        }
    }
    return ERR_XML_PARSE;
}

int FILE_INFO::parse(FILE* f) {
    char buf[256];

    memset(this, 0, sizeof(FILE_INFO));
    while (fgets(buf, sizeof(buf), f)) {
        if (match_tag(buf, "</file_info>")) {
            if (!strlen(name)) return ERR_XML_PARSE;
            return 0;
        }
        if (parse_str(buf, "<name>", name, 256)) continue;
    }
    return ERR_XML_PARSE;
}

int OTHER_RESULT::parse(FILE* f) {
    char buf[256];

    strcpy(name, "");
    have_plan_class = false;
    app_version = -1;
    while (fgets(buf, sizeof(buf), f)) {
        if (match_tag(buf, "</other_result>")) {
            if (!strcmp(name, "")) return ERR_XML_PARSE;
            return 0;
        }
        if (parse_str(buf, "<name>", name, sizeof(name))) continue;
        if (parse_int(buf, "<app_version>", app_version)) continue;
        if (parse_str(buf, "<plan_class>", plan_class, sizeof(plan_class))) {
            have_plan_class = true;
            continue;
        }
    }
    return ERR_XML_PARSE;
}

int IP_RESULT::parse(FILE* f) {
    char buf[256];

    report_deadline = 0;
    cpu_time_remaining = 0;
    strcpy(name, "");
    while (fgets(buf, sizeof(buf), f)) {
        if (match_tag(buf, "</ip_result>")) return 0;
        if (parse_str(buf, "<name>", name, sizeof(name))) continue;
        if (parse_double(buf, "<report_deadline>", report_deadline)) continue;
        if (parse_double(buf, "<cpu_time_remaining>", cpu_time_remaining)) continue;
    }
    return ERR_XML_PARSE;
}

int CLIENT_PLATFORM::parse(FILE* fin) {
    char buf[256];
    strcpy(name, "");
    while (fgets(buf, sizeof(buf), fin)) {
        if (match_tag(buf, "</alt_platform>")) return 0;
        if (parse_str(buf, "<name>", name, sizeof(name))) continue;
    }
    return ERR_XML_PARSE;
}


void WORK_REQ::add_no_work_message(const char* message) {
    for (unsigned int i=0; i<no_work_messages.size(); i++) {
        if (!strcmp(message, no_work_messages.at(i).message.c_str())){
            return;
        }
    }
    no_work_messages.push_back(USER_MESSAGE(message, "high"));
}


SCHEDULER_REQUEST::SCHEDULER_REQUEST() {
}

SCHEDULER_REQUEST::~SCHEDULER_REQUEST() {
}

// return an error message or NULL
//
const char* SCHEDULER_REQUEST::parse(FILE* fin) {
    char buf[256];
    RESULT result;
    int retval;

    strcpy(authenticator, "");
    strcpy(platform.name, "");
    strcpy(cross_project_id, "");
    hostid = 0;
    core_client_major_version = 0;
    core_client_minor_version = 0;
    core_client_release = 0;
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
    coprocs.clear();

    if (!fgets(buf, sizeof(buf), fin)) {
        return "fgets() failed";
    }
    if (!match_tag(buf, "<scheduler_request>")) return "no start tag";
    while (fgets(buf, sizeof(buf), fin)) {
        // If a line is too long, ignore it.
        // This can happen e.g. if the client has bad global_prefs.xml
        // This won't be necessary if we rewrite this using XML_PARSER
        //
        if (!strchr(buf, '\n')) {
            while (fgets(buf, sizeof(buf), fin)) {
                if (strchr(buf, '\n')) break;
            }
            continue;
        }

        if (match_tag(buf, "</scheduler_request>")) {
            core_client_version = 10000*core_client_major_version + 100*core_client_minor_version + core_client_release;
            return NULL;
        }
        if (parse_str(buf, "<authenticator>", authenticator, sizeof(authenticator))) {
            remove_quotes(authenticator);
            continue;
        }
        if (parse_str(buf, "<cross_project_id>", cross_project_id, sizeof(cross_project_id))) continue;
        if (parse_int(buf, "<hostid>", hostid)) continue;
        if (parse_int(buf, "<rpc_seqno>", rpc_seqno)) continue;
        if (parse_str(buf, "<platform_name>", platform.name, sizeof(platform.name))) continue;
        if (match_tag(buf, "<alt_platform>")) {
            CLIENT_PLATFORM cp;
            retval = cp.parse(fin);
            if (!retval) {
                alt_platforms.push_back(cp);
            }
            continue;
        }
        if (match_tag(buf, "<app_versions>")) {
            while (fgets(buf, sizeof(buf), fin)) {
                if (match_tag(buf, "</app_versions>")) break;
                if (match_tag(buf, "<app_version>")) {
                    CLIENT_APP_VERSION cav;
                    retval = cav.parse(fin);
                    if (retval) {
                        g_reply->insert_message(
                            "Invalid app version description", "high"
                        );
                    } else {
                        client_app_versions.push_back(cav);
                    }
                }
            }
            continue;
        }
        if (parse_int(buf, "<core_client_major_version>", core_client_major_version)) continue;
        if (parse_int(buf, "<core_client_minor_version>", core_client_minor_version)) continue;
        if (parse_int(buf, "<core_client_release>", core_client_release)) continue;
        if (parse_double(buf, "<work_req_seconds>", work_req_seconds)) continue;
        if (parse_double(buf, "<cpu_req_secs>", cpu_req_secs)) continue;
        if (parse_double(buf, "<cpu_req_instances>", cpu_req_instances)) continue;
        if (parse_double(buf, "<resource_share_fraction>", resource_share_fraction)) continue;
        if (parse_double(buf, "<rrs_fraction>", rrs_fraction)) continue;
        if (parse_double(buf, "<prrs_fraction>", prrs_fraction)) continue;
        if (parse_double(buf, "<estimated_delay>", cpu_estimated_delay)) continue;
        if (parse_double(buf, "<duration_correction_factor>", host.duration_correction_factor)) continue;
        if (match_tag(buf, "<global_preferences>")) {
            strcpy(global_prefs_xml, "<global_preferences>\n");
            while (fgets(buf, sizeof(buf), fin)) {
                if (strstr(buf, "</global_preferences>")) break;
                safe_strcat(global_prefs_xml, buf);
            }
            safe_strcat(global_prefs_xml, "</global_preferences>\n");
            continue;
        }
        if (match_tag(buf, "<working_global_preferences>")) {
            while (fgets(buf, sizeof(buf), fin)) {
                if (strstr(buf, "</working_global_preferences>")) break;
                safe_strcat(working_global_prefs_xml, buf);
            }
            continue;
        }
        if (parse_str(buf, "<global_prefs_source_email_hash>", global_prefs_source_email_hash, sizeof(global_prefs_source_email_hash))) continue;
        if (match_tag(buf, "<host_info>")) {
            host.parse(fin);
            continue;
        }
        if (match_tag(buf, "<time_stats>")) {
            host.parse_time_stats(fin);
            continue;
        }
        if (match_tag(buf, "<time_stats_log>")) {
            handle_time_stats_log(fin);
            have_time_stats_log = true;
            continue;
        }
        if (match_tag(buf, "<net_stats>")) {
            host.parse_net_stats(fin);
            continue;
        }
        if (match_tag(buf, "<disk_usage>")) {
            host.parse_disk_usage(fin);
            continue;
        }
        if (match_tag(buf, "<result>")) {
            result.parse_from_client(fin);
            results.push_back(result);
            continue;
        }
        if (match_tag(buf, "<code_sign_key>")) {
            copy_element_contents(fin, "</code_sign_key>", code_sign_key, sizeof(code_sign_key));
            continue;
        }
        if (match_tag(buf, "<msg_from_host>")) {
            MSG_FROM_HOST_DESC md;
            retval = md.parse(fin);
            if (!retval) {
                msgs_from_host.push_back(md);
            }
            continue;
        }
        if (match_tag(buf, "<file_info>")) {
            FILE_INFO fi;
            retval = fi.parse(fin);
            if (!retval) {
                file_infos.push_back(fi);
            }
            continue;
        }
        if (match_tag(buf, "<host_venue>")) {
            continue;
        }
        if (match_tag(buf, "<other_results>")) {
            have_other_results_list = true;
            while (fgets(buf, sizeof(buf), fin)) {
                if (match_tag(buf, "</other_results>")) break;
                if (match_tag(buf, "<other_result>")) {
                    OTHER_RESULT o_r;
                    retval = o_r.parse(fin);
                    if (!retval) {
                        other_results.push_back(o_r);
                    }
                }
            }
            continue;
        }
        if (match_tag(buf, "<in_progress_results>")) {
            have_ip_results_list = true;
            int i = 0;
            double now = time(0);
            while (fgets(buf, sizeof(buf), fin)) {
                if (match_tag(buf, "</in_progress_results>")) break;
                if (match_tag(buf, "<ip_result>")) {
                    IP_RESULT ir;
                    retval = ir.parse(fin);
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
        if (match_tag(buf, "coprocs")) {
            MIOFILE mf;
            mf.init_file(fin);
            coprocs.parse(mf);
            continue;
        }
        if (parse_bool(buf, "client_cap_plan_class", client_cap_plan_class)) continue;
        if (parse_int(buf, "<sandbox>", sandbox)) continue;

        if (match_tag(buf, "<active_task_set>")) continue;
        if (match_tag(buf, "<app>")) continue;
        if (match_tag(buf, "<app_version>")) continue;
        if (match_tag(buf, "<duration_variability>")) continue;
        if (match_tag(buf, "<new_version_check_time>")) continue;
        if (match_tag(buf, "<newer_version>")) continue;
        if (match_tag(buf, "<project>")) continue;
        if (match_tag(buf, "<project_files>")) continue;
        if (match_tag(buf, "<proxy_info>")) continue;
        if (match_tag(buf, "<user_network_request>")) continue;
        if (match_tag(buf, "<user_run_request>")) continue;
        if (match_tag(buf, "<master_url>")) continue;
        if (match_tag(buf, "<project_name>")) continue;
        if (match_tag(buf, "<user_name>")) continue;
        if (match_tag(buf, "<team_name>")) continue;
        if (match_tag(buf, "<email_hash>")) continue;
        if (match_tag(buf, "<user_total_credit>")) continue;
        if (match_tag(buf, "<user_expavg_credit>")) continue;
        if (match_tag(buf, "<user_create_time>")) continue;
        if (match_tag(buf, "<host_total_credit>")) continue;
        if (match_tag(buf, "<host_expavg_credit>")) continue;
        if (match_tag(buf, "<host_create_time>")) continue;
        if (match_tag(buf, "<nrpc_failures>")) continue;
        if (match_tag(buf, "<master_fetch_failures>")) continue;
        if (match_tag(buf, "<min_rpc_time>")) continue;
        if (match_tag(buf, "<short_term_debt>")) continue;
        if (match_tag(buf, "<long_term_debt>")) continue;
        if (match_tag(buf, "<resource_share>")) continue;
        if (match_tag(buf, "<scheduler_url>")) continue;
        if (match_tag(buf, "</project>")) continue;
        if (match_tag(buf, "<?xml")) continue;
        strip_whitespace(buf);
        if (!strlen(buf)) continue;

        log_messages.printf(MSG_NORMAL,
            "SCHEDULER_REQUEST::parse(): unrecognized: %s\n", buf
        );
        MIOFILE mf;
        mf.init_file(fin);
        retval = skip_unrecognized(buf, mf);
        if (retval) return "unterminated unrecognized XML";
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
        anonymous(platforms.list[0])?"true":"false"
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

int MSG_FROM_HOST_DESC::parse(FILE* fin) {
    char buf[256];

    msg_text = "";
    while (fgets(buf, sizeof(buf), fin)) {
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

SCHEDULER_REPLY::~SCHEDULER_REPLY() {
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
        std::string msg;
        std::string pri = "low";
        for (i=0; i<messages.size(); i++) {
            USER_MESSAGE& um = messages[i];
            msg += um.message + std::string(" ");
            if (um.priority == "high") {
                pri = "high";
            }
        }
        if (messages.size()>0) {
            // any newlines will break message printing under 4.19 and under!
            // replace them with spaces.
            //
            while (1) {
                std::string::size_type pos = msg.find("\n", 0);
                if (pos == std::string::npos) break;
                msg.replace(pos, 1, " ");
            }
            fprintf(fout,
                "<message priority=\"%s\">%s</message>\n",
                pri.c_str(), msg.c_str()
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
        fprintf(fout, "<send_full_workload\n");
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
            "<user_name>%s</user_name>\n"
            "<user_total_credit>%f</user_total_credit>\n"
            "<user_expavg_credit>%f</user_expavg_credit>\n"
            "<user_create_time>%d</user_create_time>\n",
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
            "<team_name>%s</team_name>\n",
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
    }

    for (i=0; i<results.size(); i++) {
        results[i].write_to_client(fout);
    }

    if (strlen(code_sign_key)) {
        fputs("<code_sign_key>\n", fout);
        fputs(code_sign_key, fout);
        fputs("</code_sign_key>\n", fout);
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

void SCHEDULER_REPLY::insert_result(RESULT& result) {
    results.push_back(result);
}

void SCHEDULER_REPLY::insert_message(const char* msg, const char* prio) {
    messages.push_back(USER_MESSAGE(msg, prio));
}

void SCHEDULER_REPLY::insert_message(USER_MESSAGE& um) {
    messages.push_back(um);
}

USER_MESSAGE::USER_MESSAGE(const char* m, const char* p) {
    message = m;
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
    if (bavp->host_usage.ncudas) {
        fprintf(fout,
            "    <coproc>\n"
            "        <type>CUDA</type>\n"
            "        <count>%f</count>\n"
            "    </coproc>\n",
            bavp->host_usage.ncudas
        );
    }
    if (bavp->host_usage.natis) {
        fprintf(fout,
            "    <coproc>\n"
            "        <type>ATI</type>\n"
            "        <count>%f</count>\n"
            "    </coproc>\n",
            bavp->host_usage.natis
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

int RESULT::write_to_client(FILE* fout) {
    char buf[BLOB_SIZE];

    strcpy(buf, xml_doc_in);
    char* p = strstr(buf, "</result>");
    if (!p) {
        fprintf(stderr, "ERROR: result %d XML has no end tag!\n", id);
        return -1;
    }
    *p = 0;
    fputs(buf, fout);

    APP_VERSION* avp = bavp->avp;
    CLIENT_APP_VERSION* cavp = bavp->cavp;
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

int RESULT::parse_from_client(FILE* fin) {
    char buf[256];

    // should be non-zero if exit_status is not found
    exit_status = ERR_NO_EXIT_STATUS;
    memset(this, 0, sizeof(RESULT));
    while (fgets(buf, sizeof(buf), fin)) {
        if (match_tag(buf, "</result>")) {
            return 0;
        }
        if (parse_str(buf, "<name>", name, sizeof(name))) continue;
        if (parse_int(buf, "<state>", client_state)) continue;
        if (parse_double(buf, "<final_cpu_time>", cpu_time)) continue;
        if (parse_double(buf, "<final_elapsed_time>", elapsed_time)) continue;
        if (parse_int(buf, "<exit_status>", exit_status)) continue;
        if (parse_int(buf, "<app_version_num>", app_version_num)) continue;
        if (parse_double(buf, "<fpops_per_cpu_sec>", fpops_per_cpu_sec)) continue;
        if (parse_double(buf, "<fpops_cumulative>", fpops_cumulative)) continue;
        if (parse_double(buf, "<intops_per_cpu_sec>", intops_per_cpu_sec)) continue;
        if (parse_double(buf, "<intops_cumulative>", intops_cumulative)) continue;
        if (match_tag(buf, "<file_info>")) {
            safe_strcat(xml_doc_out, buf);
            while (fgets(buf, sizeof(buf), fin)) {
                safe_strcat(xml_doc_out, buf);
                if (match_tag(buf, "</file_info>")) break;
            }
            continue;
        }
        if (match_tag(buf, "<stderr_out>" )) {
            while (fgets(buf, sizeof(buf), fin)) {
                if (match_tag(buf, "</stderr_out>")) break;
                safe_strcat(stderr_out, buf);
            }
            continue;
        }
        if (match_tag(buf, "<platform>")) continue;
        if (match_tag(buf, "<version_num>")) continue;
        if (match_tag(buf, "<plan_class>")) continue;
        if (match_tag(buf, "<completed_time>")) continue;
        if (match_tag(buf, "<file_name>")) continue;
        if (match_tag(buf, "<file_ref>")) continue;
        if (match_tag(buf, "</file_ref>")) continue;
        if (match_tag(buf, "<open_name>")) continue;
        if (match_tag(buf, "<ready_to_report>")) continue;
        if (match_tag(buf, "<ready_to_report/>")) continue;
        if (match_tag(buf, "<report_deadline>")) continue;
        if (match_tag(buf, "<wu_name>")) continue;

        log_messages.printf(MSG_NORMAL,
            "RESULT::parse_from_client(): unrecognized: %s\n",
            buf
        );
    }
    return ERR_XML_PARSE;
}

int HOST::parse(FILE* fin) {
    char buf[1024];

    p_ncpus = 1;
    while (fgets(buf, sizeof(buf), fin)) {
        if (match_tag(buf, "</host_info>")) return 0;
        if (parse_int(buf, "<timezone>", timezone)) continue;
        if (parse_str(buf, "<domain_name>", domain_name, sizeof(domain_name))) continue;
        if (parse_str(buf, "<ip_addr>", last_ip_addr, sizeof(last_ip_addr))) continue;
        if (parse_str(buf, "<host_cpid>", host_cpid, sizeof(host_cpid))) continue;
        if (parse_int(buf, "<p_ncpus>", p_ncpus)) continue;
        if (parse_str(buf, "<p_vendor>", p_vendor, sizeof(p_vendor))) continue;
        if (parse_str(buf, "<p_model>", p_model, sizeof(p_model))) continue;
        if (parse_double(buf, "<p_fpops>", p_fpops)) continue;
        if (parse_double(buf, "<p_iops>", p_iops)) continue;
        if (parse_double(buf, "<p_membw>", p_membw)) continue;
        if (parse_str(buf, "<os_name>", os_name, sizeof(os_name))) continue;
        if (parse_str(buf, "<os_version>", os_version, sizeof(os_version))) continue;
        if (parse_double(buf, "<m_nbytes>", m_nbytes)) continue;
        if (parse_double(buf, "<m_cache>", m_cache)) continue;
        if (parse_double(buf, "<m_swap>", m_swap)) continue;
        if (parse_double(buf, "<d_total>", d_total)) continue;
        if (parse_double(buf, "<d_free>", d_free)) continue;
        if (parse_double(buf, "<n_bwup>", n_bwup)) continue;
        if (parse_double(buf, "<n_bwdown>", n_bwdown)) continue;
        if (parse_str(buf, "<p_features>", p_features, sizeof(p_features))) continue;

        // parse deprecated fields to avoid error messages
        //
        if (match_tag(buf, "<p_calculated>")) continue;
        if (match_tag(buf, "<p_fpop_err>")) continue;
        if (match_tag(buf, "<p_iop_err>")) continue;
        if (match_tag(buf, "<p_membw_err>")) continue;

        // fields reported by 5.5+ clients, not currently used
        //
        if (match_tag(buf, "<p_capabilities>")) continue;
        if (match_tag(buf, "<accelerators>")) continue;

#if 1
        // not sure where these fields belong in the above categories
        //
        if (match_tag(buf, "<cpu_caps>")) continue;
        if (match_tag(buf, "<cache_l1>")) continue;
        if (match_tag(buf, "<cache_l2>")) continue;
        if (match_tag(buf, "<cache_l3>")) continue;
#endif

        log_messages.printf(MSG_NORMAL,
            "HOST::parse(): unrecognized: %s\n", buf
        );
    }
    return ERR_XML_PARSE;
}


int HOST::parse_time_stats(FILE* fin) {
    char buf[256];

    while (fgets(buf, sizeof(buf), fin)) {
        if (match_tag(buf, "</time_stats>")) return 0;
        if (parse_double(buf, "<on_frac>", on_frac)) continue;
        if (parse_double(buf, "<connected_frac>", connected_frac)) continue;
        if (parse_double(buf, "<active_frac>", active_frac)) continue;
#if 0
        if (match_tag(buf, "<outages>")) continue;
        if (match_tag(buf, "<outage>")) continue;
        if (match_tag(buf, "<start>")) continue;
        if (match_tag(buf, "<end>")) continue;
        log_messages.printf(MSG_NORMAL,
            "HOST::parse_time_stats(): unrecognized: %s\n",
            buf
        );
#endif
    }
    return ERR_XML_PARSE;
}

int HOST::parse_net_stats(FILE* fin) {
    char buf[256];

    while (fgets(buf, sizeof(buf), fin)) {
        if (match_tag(buf, "</net_stats>")) return 0;
        if (parse_double(buf, "<bwup>", n_bwup)) continue;
        if (parse_double(buf, "<bwdown>", n_bwdown)) continue;

        // items reported by 5.10+ clients, not currently used
        //
        if (match_tag(buf, "<avg_time_up>")) continue;
        if (match_tag(buf, "<avg_up>")) continue;
        if (match_tag(buf, "<avg_time_down>")) continue;
        if (match_tag(buf, "<avg_down>")) continue;

        log_messages.printf(MSG_NORMAL,
            "HOST::parse_net_stats(): unrecognized: %s\n",
            buf
        );
    }
    return ERR_XML_PARSE;
}

int HOST::parse_disk_usage(FILE* fin) {
    char buf[256];

    while (fgets(buf, sizeof(buf), fin)) {
        if (match_tag(buf, "</disk_usage>")) return 0;
        if (parse_double(buf, "<d_boinc_used_total>", d_boinc_used_total)) continue;
        if (parse_double(buf, "<d_boinc_used_project>", d_boinc_used_project)) continue;
        log_messages.printf(MSG_NORMAL,
            "HOST::parse_disk_usage(): unrecognized: %s\n",
            buf
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
                "CRITICAL: hav.update_sched() returned %d\n", retval
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

const char *BOINC_RCSID_ea659117b3 = "$Id$";
