// The contents of this file are subject to the Mozilla Public License
// Version 1.0 (the "License"); you may not use this file except in
// compliance with the License. You may obtain a copy of the License at
// http://www.mozilla.org/MPL/ 
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

#include <strings.h>
#include <assert.h>

#include "parse.h"
#include "server_types.h"

SCHEDULER_REQUEST::SCHEDULER_REQUEST() {
    prefs_xml = 0;
    code_sign_key = 0;
}

SCHEDULER_REQUEST::~SCHEDULER_REQUEST() {
    if (prefs_xml) free(prefs_xml);
    if (code_sign_key) free(code_sign_key);
}

int SCHEDULER_REQUEST::parse(FILE* fin) {
    char buf[256];
    RESULT result;
    assert(fin!=NULL);
    prefs_mod_time = 0;
    strcpy(authenticator, "");
    hostid = 0;
    work_req_seconds = 0;
    prefs_mod_time = 0;
    prefs_xml = strdup("");

    fgets(buf, 256, fin);
    if (!match_tag(buf, "<scheduler_request>")) return 1;
    while (fgets(buf, 256, fin)) {
        if (match_tag(buf, "</scheduler_request>")) return 0;
        else if (parse_str(buf, "<authenticator>", authenticator)) continue;
        else if (parse_int(buf, "<hostid>", hostid)) continue;
        else if (parse_int(buf, "<rpc_seqno>", rpc_seqno)) continue;
        else if (parse_str(buf, "<platform_name>", platform_name)) continue;
        else if (parse_int(buf, "<core_client_version>", core_client_version)) continue;
        else if (parse_int(buf, "<work_req_seconds>", work_req_seconds)) continue;
        else if (parse_int(buf, "<prefs_mod_time>", (int)prefs_mod_time)) {
            continue;
        }
        else if (match_tag(buf, "<preferences>")) {
            while (fgets(buf, 256, fin)) {
                if (strstr(buf, "</preferences>")) break;
                strcatdup(prefs_xml, buf);
            }
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
            dup_element_contents(fin, "</code_sign_key>", &code_sign_key);
        }
        else fprintf(stderr, "SCHEDULER_REQUEST::parse(): unrecognized: %s\n", buf);
    }
    return 1;
}


SCHEDULER_REPLY::SCHEDULER_REPLY() {
    request_delay = 0;
    hostid = 0;
    strcpy(message, "");
    strcpy(message_priority, "");
    send_prefs = false;
    code_sign_key = 0;
    code_sign_key_signature = 0;
}

SCHEDULER_REPLY::~SCHEDULER_REPLY() {
    if (code_sign_key) free(code_sign_key);
    if (code_sign_key_signature) free(code_sign_key_signature);
}

int SCHEDULER_REPLY::write(FILE* fout) {
    unsigned int i, j;
    assert(fout!=NULL);
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

    if (hostid) {
        fprintf(fout, "<hostid>%d</hostid>\n", hostid);
    }
    
    if (send_prefs) {
        fprintf(fout,
            "<prefs_mod_time>%d</prefs_mod_time>\n",
            user.prefs_mod_time
        );
        fputs(user.prefs, fout);
    }

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
    if (code_sign_key) {
        fputs("<code_sign_key>\n", fout);
        fputs(code_sign_key, fout);
        fputs("</code_sign_key>\n", fout);
    }
    if (code_sign_key_signature) {
        fputs("<code_sign_key_signature>\n", fout);
        fputs(code_sign_key_signature, fout);
        fputs("</code_sign_key_signature>\n", fout);
    }
    fprintf(fout,
        "</scheduler_reply>\n"
    );
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
    assert(fin!=NULL);
    memset(this, 0, sizeof(RESULT));
    while (fgets(buf, 256, fin)) {
        if (match_tag(buf, "</result>")) return 0;
        else if (parse_str(buf, "<name>", name)) continue;
        else if (parse_int(buf, "<exit_status>", exit_status)) continue;
        else if (parse_double(buf, "<final_cpu_time>", cpu_time)) continue;
        else if (match_tag(buf, "<file_info>")) {
            strcat(xml_doc_out, buf);
            while (fgets(buf, 256, fin)) {
                // protect againt buffer overrun
                if (strlen(buf) + strlen(xml_doc_out) <= MAX_BLOB_SIZE) {
                    strcat(xml_doc_out, buf);
                }
                if (match_tag(buf, "</file_info>")) break;
            }
            continue;
        }
        else if (match_tag(buf, "<stderr_out>" )) {
            while (fgets(buf, 256, fin)) {
                if (match_tag(buf, "</stderr_out>")) break;
                if (strlen(stderr_out) + strlen(buf) < MAX_BLOB_SIZE) {
                    strcat(stderr_out, buf);
                }
            }
            continue;
        }
        else fprintf(stderr, "RESULT::parse_from_client(): unrecognized: %s\n", buf);
    }
    return 1;
}

int HOST::parse(FILE* fin) {
    char buf[256];
    assert(fin!=NULL);
    while (fgets(buf, 256, fin)) {
        if (match_tag(buf, "</host_info>")) return 0;
        else if (parse_int(buf, "<timezone>", timezone)) continue;
        else if (parse_str(buf, "<domain_name>", domain_name)) continue;
        else if (parse_str(buf, "<serialnum>", serialnum)) continue;
        else if (parse_str(buf, "<ip_addr>", last_ip_addr)) continue;
        else if (parse_int(buf, "<p_ncpus>", p_ncpus)) continue;
        else if (parse_str(buf, "<p_vendor>", p_vendor)) continue;
        else if (parse_str(buf, "<p_model>", p_model)) continue;
        else if (parse_double(buf, "<p_fpops>", p_fpops)) continue;
        else if (parse_double(buf, "<p_iops>", p_iops)) continue;
        else if (parse_double(buf, "<p_membw>", p_membw)) continue;
        else if (parse_double(buf, "<p_calculated>", p_calculated)) continue;
        else if (parse_str(buf, "<os_name>", os_name)) continue;
        else if (parse_str(buf, "<os_version>", os_version)) continue;
        else if (parse_double(buf, "<m_nbytes>", m_nbytes)) continue;
        else if (parse_double(buf, "<m_cache>", m_cache)) continue;
        else if (parse_double(buf, "<m_swap>", m_swap)) continue;
        else if (parse_double(buf, "<d_total>", d_total)) continue;
        else if (parse_double(buf, "<d_free>", d_free)) continue;
        else if (parse_double(buf, "<n_bwup>", n_bwup)) continue;
        else if (parse_double(buf, "<n_bwdown>", n_bwdown)) continue;
        else fprintf(stderr, "HOST::parse(): unrecognized: %s\n", buf);
    }
    return 1;
}


int HOST::parse_time_stats(FILE* fin) {
    char buf[256];
    assert(fin!=NULL);
    while (fgets(buf, 256, fin)) {
        if (match_tag(buf, "</time_stats>")) return 0;
        else if (parse_double(buf, "<on_frac>", on_frac)) continue;
        else if (parse_double(buf, "<connected_frac>", connected_frac)) continue;
        else if (parse_double(buf, "<active_frac>", active_frac)) continue;
        else fprintf(stderr, "HOST::parse_time_stats(): unrecognized: %s\n", buf);
    }
    return 1;
}

int HOST::parse_net_stats(FILE* fin) {
    char buf[256];

    while (fgets(buf, 256, fin)) {
        if (match_tag(buf, "</net_stats>")) return 0;
        else if (parse_double(buf, "<bwup>", n_bwup)) continue;
        else if (parse_double(buf, "<bwdown>", n_bwdown)) continue;
        else fprintf(stderr, "HOST::parse_net_stats(): unrecognized: %s\n", buf);
    }
    return 1;
}

#if 0
DB_CACHE::DB_CACHE() {
}

int DB_CACHE::read_db() {
    PLATFORM platform;
    APP app;
    APP_VERSION app_version;

    while (!db_platform_enum(platform)) {
        platforms.push_back(platform);
    }
    while (!db_app_enum(app)) {
        apps.push_back(app);
    }
    while (!db_app_version_enum(app_version)) {
        app_versions.push_back(app_version);
    }
    return 0;
}

PLATFORM* DB_CACHE::lookup_platform(char* name) {
    unsigned int i;
    assert(name!=NULL);
    for (i=0; i<platforms.size(); i++) {
        if (!strcmp(platforms[i].name, name)) {
            return &platforms[i];
        }
    }
    return 0;
}

APP* DB_CACHE::lookup_app(int id) {
    unsigned int i;

    for (i=0; i<apps.size(); i++) {
        if (apps[i].id == id) {
            return &apps[i];
        }
    }
    return 0;
}

APP_VERSION* DB_CACHE::lookup_app_version(
    int appid, int platformid, int version
) {
    unsigned int i;
    APP_VERSION* avp;
    assert(version>=0);
    for (i=0; i<app_versions.size(); i++) {
        avp = &app_versions[i];
        if (avp->appid == appid && avp->platformid == platformid && avp->version_num == version) {
            return avp;
        }
    }
    return 0;
}
#endif
