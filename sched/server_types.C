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

#include <cassert>
#include <vector>
#include <string>
using namespace std;

#include "parse.h"
#include "error_numbers.h"
#include "util.h"
#include "main.h"
#include "sched_util.h"
#include "sched_msgs.h"
#include "server_types.h"

#ifdef _USING_FCGI_
#include "fcgi_stdio.h"
#endif

int CLIENT_APP_VERSION::parse(FILE* f) {
    char buf[256];

    while (fgets(buf, 256, f)) {
        if (match_tag(buf, "</app_version>")) return 0;
        if (parse_str(buf, "<app_name>", app_name, 256)) continue;
        if (parse_int(buf, "<version_num>", version_num)) continue;
    }
    return ERR_XML_PARSE;
}

int FILE_INFO::parse(FILE* f) {
    char buf[256];

    memset(this, 0, sizeof(FILE_INFO));
    while (fgets(buf, 256, f)) {
        if (match_tag(buf, "</file_info>")) {
            if (!strlen(name)) return ERR_XML_PARSE;
            return 0;
        }
        if (parse_str(buf, "<name>", name, 256)) continue;
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
    resource_share_fraction = 1.0;
    estimated_delay = 0;
    strcpy(global_prefs_xml, "");
    strcpy(code_sign_key, "");
    strcpy(cross_project_id, "");

    fgets(buf, 256, fin);
    if (!match_tag(buf, "<scheduler_request>")) return ERR_XML_PARSE;
    while (fgets(buf, 256, fin)) {
        if (match_tag(buf, "</scheduler_request>")) return 0;
        else if (parse_str(buf, "<authenticator>", authenticator, sizeof(authenticator))) continue;
        else if (parse_str(buf, "<cross_project_id>", cross_project_id, sizeof(cross_project_id))) continue;
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
        else if (parse_double(buf, "<work_req_seconds>", work_req_seconds)) continue;
        else if (parse_double(buf, "<resource_share_fraction>", resource_share_fraction)) continue;
        else if (parse_double(buf, "<estimated_delay>", estimated_delay)) continue;

// ROMW: Added these back in since we have 3.x clients who still want
//       want to send us the older style for determining disk usage.
// TODO: Remove the two lines below when the 4.x way of doing things
//       is completely implemented.
        else if (parse_double(buf, "<project_disk_usage>", project_disk_usage)) continue;
        else if (parse_double(buf, "<total_disk_usage>", total_disk_usage)) continue;

#if 0
        else if (parse_double(buf, "<project_disk_free>", project_disk_free)) continue;
        else if (parse_double(buf, "<potentially_free_offender>", potentially_free_offender)) continue;
        else if (parse_double(buf, "<potentially_free_self>", potentially_free_self)) continue;
#endif
        else if (match_tag(buf, "<global_preferences>")) {
            strcpy(global_prefs_xml, "<global_preferences>\n");
            while (fgets(buf, 256, fin)) {
                if (strstr(buf, "</global_preferences>")) break;
                safe_strcat(global_prefs_xml, buf);
            }
            safe_strcat(global_prefs_xml, "</global_preferences>\n");
        }
        else if (parse_str(buf, "<global_prefs_source_email_hash>", global_prefs_source_email_hash, sizeof(global_prefs_source_email_hash))) continue;
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
        else if (match_tag(buf, "<msg_from_host>")) {
            MSG_FROM_HOST_DESC md;
            retval = md.parse(fin);
            if (!retval) {
                msgs_from_host.push_back(md);
            }
        }
        else if (match_tag(buf, "<file_info>")) {
            FILE_INFO fi;
            retval = fi.parse(fin);
            if (!retval) {
                file_infos.push_back(fi);
            }
        } else if (match_tag(buf, "<host_venue>")) {
            // do NOTHING here
        } else {
            log_messages.printf(SCHED_MSG_LOG::NORMAL, "SCHEDULER_REQUEST::parse(): unrecognized: %s\n", buf);
        }
    }
    return ERR_XML_PARSE;
}

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
          "  <rpc_seqno>%d</rpc_seqno>\n"
          "  <work_req_seconds>%.15f</work_req_seconds>\n"
          "  <resource_share_fraction>%.15f</resource_share_fraction>\n"
          "  <estimated_delay>%.15f</estimated_delay>\n"
          "  <code_sign_key>%s</code_sign_key>\n"
          "  <total_disk_usage>%.15f</total_disk_usage>\n"
	  "  <project_disk_usage>%.15f</project_disk_usage>\n"
          "  <anonymous_platform>%s</anonymous_platform>\n",
	  authenticator,
          platform_name,
          cross_project_id,
          hostid,
          core_client_major_version,
          core_client_minor_version,
          rpc_seqno,
          work_req_seconds,
          resource_share_fraction,
          estimated_delay,
          code_sign_key,
          total_disk_usage,
          project_disk_usage,
          anonymous_platform?"true":"false"
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
    while (fgets(buf, 256, fin)) {
        if (match_tag(buf, "</msg_from_host>")) return 0;
        if (parse_str(buf, "<variety>", variety, sizeof(variety))) continue;
        msg_text += buf;
    }
    return ERR_XML_PARSE;
}

SCHEDULER_REPLY::SCHEDULER_REPLY() {
    memset(&wreq, 0, sizeof(wreq));
    request_delay = 0;
    hostid = 0;
    send_global_prefs = false;
    strcpy(code_sign_key, "");
    strcpy(code_sign_key_signature, "");
    memset(&user, 0, sizeof(user));
    memset(&host, 0, sizeof(host));
    memset(&team, 0, sizeof(team));
    nucleus_only = false;
    probable_user_browser = false;
    send_msg_ack = false;
    strcpy(email_hash, "");
    update_user_record = false;
}

SCHEDULER_REPLY::~SCHEDULER_REPLY() {
}

int SCHEDULER_REPLY::write(FILE* fout) {
    unsigned int i, j;
    string u1, u2, t1, t2;
    char buf[LARGE_BLOB_SIZE];

    fprintf(fout,
        "<scheduler_reply>\n"
        "<scheduler_version>%d</scheduler_version>\n",
        BOINC_MAJOR_VERSION*100+BOINC_MINOR_VERSION
    );

    if (request_delay) {
        fprintf(fout, "<request_delay>%d</request_delay>\n", request_delay);
        log_messages.printf(SCHED_MSG_LOG::NORMAL, "sending delay request %d\n", request_delay);
    }
    if (wreq.core_client_version < 462) {
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
    if (nucleus_only) goto end;

    fprintf(fout,
        "<project_name>%s</project_name>\n",
        config.long_name
    );

    if (user.id) {
        u1 = user.name;
        xml_escape(u1, u2);
        fprintf(fout,
            "<user_name>%s</user_name>\n"
            "<user_total_credit>%f</user_total_credit>\n"
            "<user_expavg_credit>%f</user_expavg_credit>\n"
            "<user_create_time>%d</user_create_time>\n",
            u2.c_str(),
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
        }

        // always send project prefs
        //
        fputs(user.project_prefs, fout);

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
        t1 = team.name;
        xml_escape(t1, t2);
        fprintf(fout,
            "<team_name>%s</team_name>\n",
            t2.c_str()
        );
    }

#if 0
    if (deletion_policy_priority) fprintf(fout, "<deletion_policy_priority/>\n");
    if (deletion_policy_expire) fprintf(fout, "<deletion_policy_expire/>\n");
#endif

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
    if (send_msg_ack) {
        fputs("<message_ack/>\n", fout);
    }

    // changed implimentation so that messages have no flags
    // that say they are messages unless specified in the xml
    // portion in the MSG_TO_HOST object.

    for (i=0; i<msgs_to_host.size(); i++) {
        MSG_TO_HOST& md = msgs_to_host[i];
        fprintf(fout, "%s\n", md.xml);
    }

    if (config.non_cpu_intensive) {
        fprintf(fout, "<non_cpu_intensive/>\n");
    }

    for (i=0; i<file_deletes.size(); i++) {
        fprintf(fout,
            "<delete_file_info>%s</delete_file_info>\n",
            file_deletes[i].name
        );
    }

    gui_urls.get_gui_urls(user, host, team, buf);
    fputs(buf, fout);

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

void SCHEDULER_REPLY::set_delay(int delay) {
    // set delay to the MAX of the existing value or the requested value
    // never send a delay request longer than two days.
    if (request_delay < delay) {
        request_delay = delay;
    }
    if (request_delay > 2*24*3600) {
        request_delay = 2*24*3600;
    }
    return;
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

void SCHEDULER_REPLY::insert_message(USER_MESSAGE& um) {
    messages.push_back(um);
}

USER_MESSAGE::USER_MESSAGE(char* m, char* p) {
    message = m;
    priority = p;
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

    p_ncpus = 1;
    while (fgets(buf, 256, fin)) {
        if (match_tag(buf, "</host_info>")) return 0;
        else if (parse_int(buf, "<timezone>", timezone)) continue;
        else if (parse_str(buf, "<domain_name>", domain_name, sizeof(domain_name))) continue;
        else if (parse_str(buf, "<serialnum>", serialnum, sizeof(serialnum))) continue;
        else if (parse_str(buf, "<ip_addr>", last_ip_addr, sizeof(last_ip_addr))) continue;
        else if (parse_str(buf, "<host_cpid>", host_cpid, sizeof(host_cpid))) continue;
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

void GUI_URLS::init() {
    text = 0;
    read_file_malloc("../gui_urls.xml", text);
}

void GUI_URLS::get_gui_urls(USER& user, HOST& host, TEAM& team, char* buf) {
    bool found;
    char userid[256], teamid[256], hostid[256];
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
    while (1) {
        found = false;
        found |= str_replace(buf, "<userid/>", userid);
        found |= str_replace(buf, "<user_name/>", user.name);
        found |= str_replace(buf, "<hostid/>", hostid);
        found |= str_replace(buf, "<teamid/>", teamid);
        found |= str_replace(buf, "<team_name/>", team.name);
        found |= str_replace(buf, "<authenticator/>", user.authenticator);
        if (!found) break;
    }
}

const char *BOINC_RCSID_ea659117b3 = "$Id$";
