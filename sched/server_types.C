// The contents of this file are subject to the BOINC Public License
// Version 1.0 (the "License"); you may not use this file except in
// compliance with the License. You may obtain a copy of the License at
// http://boinc.berkeley.edu/license_1.0.txt
//
// Software distributed under the License is distributed on an "AS IS"
// basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the
// License for the specific language governing rights and limitations
// under the License.
//
// The Original Code is the Berkeley Open Infrastructure for Network Computing.
//
// The Initial Developer of the Original Code is the SETI@home project.
// Portions created by the SETI@home project are Copyright (C) 2002
// University of California at Berkeley. All Rights Reserved.
//
// Contributor(s):
//

#include <iostream>
#include <vector>
#include <string>
using namespace std;

#include <strings.h>
#include <assert.h>

#include "parse.h"
#include "error_numbers.h"
#include "util.h"
#include "main.h"
#include "sched_util.h"
#include "sched_msgs.h"
#include "server_types.h"

int CLIENT_APP_VERSION::parse(FILE* f) {
    char buf[256];

    while (fgets(buf, 256, f)) {
        if (match_tag(buf, "</app_version>")) return 0;
        if (parse_str(buf, "<app_name>", app_name, 256)) continue;
        if (parse_int(buf, "<version_num>", version_num)) continue;
    }
    return ERR_XML_PARSE;
}

SCHEDULER_REQUEST::SCHEDULER_REQUEST() {
}

SCHEDULER_REQUEST::~SCHEDULER_REQUEST() {
}

int SCHEDULER_REQUEST::parse(FILE* fin) {
    char buf[256];
    RESULT result;
    int retval;

    strcpy(authenticator, "");
    hostid = 0;
    work_req_seconds = 0;
    strcpy(global_prefs_xml, "");
    strcpy(projects_xml, "");
    strcpy(code_sign_key, "");

    fgets(buf, 256, fin);
    if (!match_tag(buf, "<scheduler_request>")) return ERR_XML_PARSE;
    while (fgets(buf, 256, fin)) {
        if (match_tag(buf, "</scheduler_request>")) return 0;
        else if (parse_str(buf, "<authenticator>", authenticator, sizeof(authenticator))) continue;
        else if (parse_int(buf, "<hostid>", hostid)) continue;
        else if (parse_int(buf, "<rpc_seqno>", rpc_seqno)) continue;
        else if (parse_str(buf, "<platform_name>", platform_name, sizeof(platform_name))) continue;
        else if (match_tag(buf, "<app_versions>")) {
            while (fgets(buf, 256, fin)) {
                if (match_tag(buf, "</app_versions>")) break;
                if (match_tag(buf, "<app_version>")) {
                    CLIENT_APP_VERSION cav;
                    cav.parse(fin);
                    client_app_versions.push_back(cav);
                }
            }
            continue;
        }
        else if (parse_int(buf, "<core_client_major_version>", core_client_major_version)) continue;
        else if (parse_int(buf, "<core_client_minor_version>", core_client_minor_version)) continue;
        else if (parse_int(buf, "<work_req_seconds>", work_req_seconds)) continue;
        else if (parse_double(buf, "<project_disk_usage>", project_disk_usage)) continue;
        else if (parse_double(buf, "<total_disk_usage>", total_disk_usage)) continue;
        else if (match_tag(buf, "<global_preferences>")) {
            strcpy(global_prefs_xml, "<global_preferences>\n");
            while (fgets(buf, 256, fin)) {
                if (strstr(buf, "</global_preferences>")) break;
                safe_strcat(global_prefs_xml, buf);
            }
            safe_strcat(global_prefs_xml, "</global_preferences>\n");
        }
        else if (match_tag(buf, "<projects>")) {
            strcpy(projects_xml, "<projects>\n");
            while (fgets(buf, 256, fin)) {
                if (strstr(buf, "</projects>")) break;
                safe_strcat(projects_xml, buf);
            }
            safe_strcat(projects_xml, "</projects>\n");
        }
        else if (match_tag(buf, "<host_info>")) {
            host.parse(fin);
            continue;
        }
        else if (match_tag(buf, "<time_stats>")) {
            host.parse_time_stats(fin);
            continue;
        }
        else if (match_tag(buf, "<net_stats>")) {
            host.parse_net_stats(fin);
            continue;
        }
        else if (match_tag(buf, "<result>")) {
            result.parse_from_client(fin);
            results.push_back(result);
            continue;
        }
        else if (match_tag(buf, "<code_sign_key>")) {
            copy_element_contents(fin, "</code_sign_key>", code_sign_key, sizeof(code_sign_key));
        }
        else if (match_tag(buf, "<trickle>")) {
            TRICKLE_UP_DESC td;
            retval = td.parse(fin);
            if (!retval) {
                trickles.push_back(td);
            }
        } else {
            log_messages.printf(SCHED_MSG_LOG::NORMAL, "SCHEDULER_REQUEST::parse(): unrecognized: %s\n", buf);
        }
    }
    return ERR_XML_PARSE;
}

int TRICKLE_UP_DESC::parse(FILE* fin) {
    char buf[256];

    trickle_text = "";
    while (fgets(buf, 256, fin)) {
        if (match_tag(buf, "</trickle>")) return 0;
        if (parse_int(buf, "<time>", send_time)) continue;
        if (match_tag(buf, "<text>")) {
            while (fgets(buf, 256, fin)) {
                if (match_tag(buf, "</text>")) break;
                trickle_text += buf;
            }
        }
    }
    return ERR_XML_PARSE;
}

SCHEDULER_REPLY::SCHEDULER_REPLY() {
    request_delay = 0;
    hostid = 0;
    strcpy(message, "");
    strcpy(message_priority, "");
    send_global_prefs = false;
    strcpy(code_sign_key, "");
    strcpy(code_sign_key_signature, "");
    memset(&user, 0, sizeof(user));
    memset(&host, 0, sizeof(host));
    memset(&team, 0, sizeof(team));
    nucleus_only = false;
    probable_user_browser = false;
    send_trickle_up_ack = false;
}

SCHEDULER_REPLY::~SCHEDULER_REPLY() {
}

int SCHEDULER_REPLY::write(FILE* fout) {
    unsigned int i, j;
    string u1, u2, t1, t2;
    int retval;

    fprintf(fout,
        "<scheduler_reply>\n"
    );

    if (request_delay) {
        fprintf(fout, "<request_delay>%d</request_delay>\n", request_delay);
    }
    if (strlen(message)) {
        fprintf(fout,
            "<message priority=\"%s\">%s</message>\n",
            message_priority,
            message
        );
    }
    if (nucleus_only) goto end;

    fprintf(fout,
        "<project_name>%s</project_name>\n",
        gproject.long_name
    );

    u1 = user.name;
    xml_escape(u1, u2);
    fprintf(fout,
        "<user_name>%s</user_name>\n"
        "<user_total_credit>%f</user_total_credit>\n"
        "<user_expavg_credit>%f</user_expavg_credit>\n"
        "<user_create_time>%d</user_create_time>\n"
        "<host_total_credit>%f</host_total_credit>\n"
        "<host_expavg_credit>%f</host_expavg_credit>\n"
        "<host_venue>%s</host_venue>\n",
        u2.c_str(),
        user.total_credit,
        user.expavg_credit,
        user.create_time,
        host.total_credit,
        host.expavg_credit,
        host.venue
    );

    // might want to send team credit too.
    //
    if (team.id) {
        t1 = team.name;
        xml_escape(t1, t2);
        fprintf(fout,
            "<team_name>%s</team_name>\n",
            t2.c_str()
        );
    }

    if (hostid) {
        fprintf(fout,
            "<hostid>%d</hostid>\n"
            "<host_create_time>%d</host_create_time>\n",
            hostid,
            host.create_time
        );
    }

    if (send_global_prefs) {
        fputs(user.global_prefs, fout);
    }

    // always send project prefs
    //
    fputs(user.project_prefs, fout);

    // acknowledge results
    //
    for (i=0; i<result_acks.size(); i++) {
        fprintf(fout,
            "<result_ack>\n"
            "    <name>%s</name>\n"
            "</result_ack>\n",
            result_acks[i].name
        );
    }

    for (i=0; i<apps.size(); i++) {
        apps[i].write(fout);
    }

    for (i=0; i<app_versions.size(); i++) {
        for (j=0; j<apps.size(); j++) {
            if (apps[j].id == app_versions[i].appid) {
                app_versions[i].write(fout, apps[j]);
                break;
            }
        }
    }

    for (i=0; i<wus.size(); i++) {
        fputs(wus[i].xml_doc, fout);
    }

    for (i=0; i<results.size(); i++) {
        fputs(results[i].xml_doc_in, fout);
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
    if (send_trickle_up_ack) {
        fputs("<trickle_up_ack/>\n", fout);
    }
    for (i=0; i<trickle_downs.size(); i++) {
        TRICKLE_DOWN& td = trickle_downs[i];
        DB_RESULT result;
        retval = result.lookup_id(td.resultid);
        if (retval) {
            continue;
        }
        fprintf(fout,
            "<trickle_down>\n"
            "    <result_name>%s</result_name>\n"
            "    <send_time>%d</send_time>\n"
            "    <text>\n"
            "%s\n"
            "    </text>\n"
            "</trickle_down>\n",
            result.name,
            td.create_time,
            td.xml
        );
    }
end:
    fprintf(fout,
        "</scheduler_reply>\n"
    );
    if (probable_user_browser) {
        // User is probably trying to look at cgi output with a browser.
        // Redirect them to the project home page.

        fprintf(fout,
            "<HTML><HEAD><META HTTP-EQUIV=Refresh CONTENT=\"0;URL=%s\"></HEAD><BODY>\n\n"
            "You seem to be viewing this page in a WWW browser.  Visit the <a href=\"%s\">main page</a>.\n\n"
            "</BODY></HTML>\n",
            "../", "../"
        );
    }
    return 0;
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

int APP::write(FILE* fout) {
    fprintf(fout,
        "<app>\n"
        "    <name>%s</name>\n"
        "</app>\n",
        name
    );
    return 0;
}

int APP_VERSION::write(FILE* fout, APP& app) {
    fputs(xml_doc, fout);
    return 0;
}

int RESULT::parse_from_client(FILE* fin) {
    char buf[256];

    // should be non-zero if exit_status is not found
    exit_status = ERR_NO_EXIT_STATUS; 
    memset(this, 0, sizeof(RESULT));
    while (fgets(buf, 256, fin)) {
        if (match_tag(buf, "</result>")) return 0;
        else if (parse_str(buf, "<name>", name, sizeof(name))) continue;
        else if (parse_int(buf, "<state>", client_state)) continue;
        else if (parse_double(buf, "<final_cpu_time>", cpu_time)) continue;
        else if (parse_int(buf, "<exit_status>", exit_status)) continue;
        else if (parse_int(buf, "<app_version_num>", app_version_num)) continue;
        else if (match_tag(buf, "<file_info>")) {
            safe_strcat(xml_doc_out, buf);
            while (fgets(buf, 256, fin)) {
                safe_strcat(xml_doc_out, buf);
                if (match_tag(buf, "</file_info>")) break;
            }
            continue;
        } else if (match_tag(buf, "<stderr_out>" )) {
            while (fgets(buf, 256, fin)) {
                if (match_tag(buf, "</stderr_out>")) break;
                safe_strcat(stderr_out, buf);
            }
            continue;
        } else {
            log_messages.printf(
                SCHED_MSG_LOG::NORMAL,
                "RESULT::parse_from_client(): unrecognized: %s\n",
                buf
            );
        }
    }
    return ERR_XML_PARSE;
}

// TODO: put the benchmark errors into the DB
//
int HOST::parse(FILE* fin) {
    char buf[256];
    int p_fpop_err, p_iop_err, p_membw_err;

    while (fgets(buf, 256, fin)) {
        if (match_tag(buf, "</host_info>")) return 0;
        else if (parse_int(buf, "<timezone>", timezone)) continue;
        else if (parse_str(buf, "<domain_name>", domain_name, sizeof(domain_name))) continue;
        else if (parse_str(buf, "<serialnum>", serialnum, sizeof(serialnum))) continue;
        else if (parse_str(buf, "<ip_addr>", last_ip_addr, sizeof(last_ip_addr))) continue;
        else if (parse_int(buf, "<p_ncpus>", p_ncpus)) continue;
        else if (parse_str(buf, "<p_vendor>", p_vendor, sizeof(p_vendor))) continue;
        else if (parse_str(buf, "<p_model>", p_model, sizeof(p_model))) continue;
        else if (parse_double(buf, "<p_fpops>", p_fpops)) continue;
        else if (parse_double(buf, "<p_iops>", p_iops)) continue;
        else if (parse_double(buf, "<p_membw>", p_membw)) continue;
        else if (parse_int(buf, "<p_fpop_err>", p_fpop_err)) continue;
        else if (parse_int(buf, "<p_iop_err>", p_iop_err)) continue;
        else if (parse_int(buf, "<p_membw_err>", p_membw_err)) continue;
        else if (parse_double(buf, "<p_calculated>", p_calculated)) continue;
        else if (parse_str(buf, "<os_name>", os_name, sizeof(os_name))) continue;
        else if (parse_str(buf, "<os_version>", os_version, sizeof(os_version))) continue;
        else if (parse_double(buf, "<m_nbytes>", m_nbytes)) continue;
        else if (parse_double(buf, "<m_cache>", m_cache)) continue;
        else if (parse_double(buf, "<m_swap>", m_swap)) continue;
        else if (parse_double(buf, "<d_total>", d_total)) continue;
        else if (parse_double(buf, "<d_free>", d_free)) continue;
        else if (parse_double(buf, "<n_bwup>", n_bwup)) continue;
        else if (parse_double(buf, "<n_bwdown>", n_bwdown)) continue;
        else {
            log_messages.printf(SCHED_MSG_LOG::NORMAL, "HOST::parse(): unrecognized: %s\n", buf);
        }
    }
    return ERR_XML_PARSE;
}


int HOST::parse_time_stats(FILE* fin) {
    char buf[256];

    while (fgets(buf, 256, fin)) {
        if (match_tag(buf, "</time_stats>")) return 0;
        else if (parse_double(buf, "<on_frac>", on_frac)) continue;
        else if (parse_double(buf, "<connected_frac>", connected_frac)) continue;
        else if (parse_double(buf, "<active_frac>", active_frac)) continue;
        else {
            log_messages.printf(
                SCHED_MSG_LOG::NORMAL,
                "HOST::parse_time_stats(): unrecognized: %s\n",
                buf
            );
        }
    }
    return ERR_XML_PARSE;
}

int HOST::parse_net_stats(FILE* fin) {
    char buf[256];

    while (fgets(buf, 256, fin)) {
        if (match_tag(buf, "</net_stats>")) return 0;
        else if (parse_double(buf, "<bwup>", n_bwup)) continue;
        else if (parse_double(buf, "<bwdown>", n_bwdown)) continue;
        else {
            log_messages.printf(
                SCHED_MSG_LOG::NORMAL,
                "HOST::parse_net_stats(): unrecognized: %s\n",
                buf
            );
        }
    }
    return ERR_XML_PARSE;
}

void GLOBAL_PREFS::parse(char* buf, char* venue) {
    disk_max_used_gb = 0;
    disk_max_used_pct = 0;
    disk_min_free_gb = 0;
    char buf2[LARGE_BLOB_SIZE];

    extract_venue(buf, venue, buf2);
    parse_double(buf2, "<disk_max_used_gb>", disk_max_used_gb);
    parse_double(buf2, "<disk_max_used_pct>", disk_max_used_pct);
    parse_double(buf2, "<disk_min_free_gb>", disk_min_free_gb);
}
