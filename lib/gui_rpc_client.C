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

#ifdef _WIN32
#include "boinc_win.h"
#endif

#ifndef _WIN32
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <cstdio>
#include <unistd.h>
#include <cstdlib>
#include <cstring>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <netdb.h>
#endif

#include "parse.h"
#include "error_numbers.h"
#include "miofile.h"
#include "gui_rpc_client.h"

using std::string;
using std::vector;

int RPC_CLIENT::init(char* host) {
    int retval;
    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(GUI_RPC_PORT);

    if (host) {
        hostent* hep = gethostbyname(host);
        if (!hep) {
            perror("gethostbyname");
            return ERR_GETHOSTBYNAME;
        }
        addr.sin_addr.s_addr = *(int*)hep->h_addr_list[0];
    } else {
#ifdef _WIN32
        addr.sin_addr.s_addr = inet_addr("127.0.0.1");
#else
        addr.sin_addr.s_addr = htonl(INADDR_ANY);
#endif
    }

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock <= 0) {
        perror("socket");
        return ERR_SOCKET;
    }
    retval = connect(sock, (const sockaddr*)(&addr), sizeof(addr));
    if (retval) {
#ifdef _WIN32
        printf( "Windows Socket Error '%d'\n", WSAGetLastError() );
#endif
        perror("connect");
        return ERR_CONNECT;
    }
    return 0;
}

RPC_CLIENT::~RPC_CLIENT() {
}

int RPC_CLIENT::send_request(char* p) {
    int n = send(sock, p, strlen(p), 0);
    if (n < 0) return ERR_WRITE;
    return 0;
}

// get reply from server.  Caller must free buf
//
int RPC_CLIENT::get_reply(char*& mbuf) {
    char buf[1025];
    MFILE mf;
    int n;

    while (1) {
        n = recv(sock, buf, 1024, 0);
        if (n <= 0) return ERR_READ;
        buf[n]=0;
        mf.puts(buf);
        if (strchr(buf, '\003')) break;
    }
    mf.get_buf(mbuf, n);
    return 0;
}

int RPC_CLIENT::get_state(CC_STATE& state) {
    char buf[256];
    PROJECT* project;
    char* mbuf=0;
    int retval;

    retval = send_request("<get_state/>\n");
    if (retval) return retval;
    retval = get_reply(mbuf);
    if (retval) return retval;
    MIOFILE fin;
    fin.init_buf(mbuf);

    while (fin.fgets(buf, 256)) {
        if (match_tag(buf, "</client_state>")) break;
        else if (match_tag(buf, "<project>")) {
            project = new PROJECT;
            project->parse(fin);
            state.projects.push_back(project);
            continue;
        }
        else if (match_tag(buf, "<app>")) {
            APP* app = new APP;
            app->parse(fin);
            app->project = project;
            state.apps.push_back(app);
            continue;
        }
        else if (match_tag(buf, "<app_version>")) {
            APP_VERSION* app_version = new APP_VERSION;
            app_version->parse(fin);
            app_version->project = project;
            app_version->app = state.lookup_app(app_version->app_name);
            state.app_versions.push_back(app_version);
            continue;
        }
        else if (match_tag(buf, "<workunit>")) {
            WORKUNIT* wu = new WORKUNIT;
            wu->parse(fin);
            wu->project = project;
            wu->app = state.lookup_app(wu->app_name);
            wu->avp = state.lookup_app_version(wu->app_name, wu->version_num);
            state.wus.push_back(wu);
            continue;
        }
        else if (match_tag(buf, "<result>")) {
            RESULT* result = new RESULT;
            result->parse(fin);
            result->project = project;
            result->wup = state.lookup_wu(result->wu_name);
            result->app = result->wup->app;
            state.results.push_back(result);
            continue;
        }
    }
    if (mbuf) free(mbuf);
    return 0;
}

int RPC_CLIENT::get_results(RESULTS& t) {
    char buf[256];
    char* mbuf=0;
    int retval;

    retval = send_request("<get_results/>\n");
    if (retval) return retval;
    retval = get_reply(mbuf);
    if (retval) return retval;
    MIOFILE fin;
    fin.init_buf(mbuf);

    while (fin.fgets(buf, 256)) {
        if (match_tag(buf, "</results>")) break;
        else if (match_tag(buf, "<result>")) {
            RESULT* rp = new RESULT;
            rp->parse(fin);
            t.results.push_back(rp);
            continue;
        }
    }
    if (mbuf) free(mbuf);
    return 0;
}

int RPC_CLIENT::get_file_transfers(FILE_TRANSFERS& t) {
    char buf[256];
    char* mbuf=0;
    int retval;

    retval = send_request("<get_file_transfers/>\n");
    if (retval) return retval;
    retval = get_reply(mbuf);
    if (retval) return retval;
    MIOFILE fin;
    fin.init_buf(mbuf);

    while (fin.fgets(buf, 256)) {
        if (match_tag(buf, "</file_transfers>")) break;
        else if (match_tag(buf, "<file_transfer>")) {
            FILE_TRANSFER* fip = new FILE_TRANSFER;
            fip->parse(fin);
            t.file_transfers.push_back(fip);
            continue;
        }
    }
    if (mbuf) free(mbuf);
    return 0;
}

int RPC_CLIENT::get_project_status(std::vector<PROJECT>& projects) {
    char buf[256];
    char* mbuf=0;
    int retval;

    retval = send_request("<get_project_status/>\n");
    if (retval) return retval;
    retval = get_reply(mbuf);
    if (retval) return retval;
    MIOFILE fin;
    fin.init_buf(mbuf);

    while (fin.fgets(buf, 256)) {
        if (match_tag(buf, "</projects>")) break;
        else if (match_tag(buf, "<project>")) {
            PROJECT project;
            project.parse(fin);
            projects.push_back(project);
            continue;
        }
    }
    if (mbuf) free(mbuf);
    return 0;
}

int RPC_CLIENT::get_disk_usage(std::vector<PROJECT>& projects) {
    char buf[256];
    char* mbuf=0;
    int retval;

    retval = send_request("<get_file_transfers/>\n");
    if (retval) return retval;
    retval = get_reply(mbuf);
    if (retval) return retval;
    MIOFILE fin;
    fin.init_buf(mbuf);

    while (fin.fgets(buf, 256)) {
        if (match_tag(buf, "</projects>")) break;
        else if (match_tag(buf, "<project>")) {
            PROJECT project;
            project.parse(fin);
            projects.push_back(project);
            continue;
        }
    }
    if (mbuf) free(mbuf);
    return 0;
}

int RPC_CLIENT::show_graphics(char* project, char* result_name, bool full_screen) {
    char buf[256];
    char* mbuf=0;
    int retval;

    if (project) {
        sprintf(buf, "<result_show_graphics>\n"
            "<project_url>%s</project_url>\n"
            "<result_name>%s</result_name>\n"
            "%s"
            "</result_show_graphics>\n",
            result_name,
            full_screen?"<full_screen/>\n":""
        );
    } else {
        sprintf(buf,
            "<result_show_graphics>\n%s</result_show_graphics>\n",
            full_screen?"<full_screen/>\n":""
        );
    }
    retval = send_request(buf);
    if (retval) return retval;
    retval = get_reply(mbuf);
    if (retval) return retval;
    if (mbuf) free(mbuf);
    return 0;
}

int RPC_CLIENT::project_reset(char* url) {
    char buf[256];
    char* mbuf=0;
    int retval;

    sprintf(buf,
        "<project_reset>\n"
        "  <project_url>%s</project_url>\n"
        "</project_reset>\n",
        url
    );
    retval = send_request(buf);
    if (retval) return retval;
    retval = get_reply(mbuf);
    if (retval) return retval;
    if (mbuf) free(mbuf);
    return 0;
}

int RPC_CLIENT::project_attach(char* url, char* auth) {
    char buf[256];
    char* mbuf=0;
    int retval;

    sprintf(buf,
        "<project_attach>\n"
        "  <project_url>%s</project_url>\n"
        "  <authenticator>%s</authenticator>\n"
        "</project_attach>\n",
        url, auth
    );
    retval = send_request(buf);
    if (retval) return retval;
    retval = get_reply(mbuf);
    if (retval) return retval;
    if (mbuf) free(mbuf);
    return 0;
}

int RPC_CLIENT::project_detach(char* url) {
    char buf[256];
    char* mbuf=0;
    int retval;

    sprintf(buf,
        "<project_detach>\n"
        "  <project_url>%s</project_url>\n"
        "</project_detach>\n",
        url
    );
    retval = send_request(buf);
    if (retval) return retval;
    retval = get_reply(mbuf);
    if (retval) return retval;
    if (mbuf) free(mbuf);
    return 0;
}

int RPC_CLIENT::project_update(char* url) {
    char buf[256];
    char* mbuf=0;
    int retval;

    sprintf(buf,
        "<project_update>\n"
        "  <project_url>%s</project_url>\n"
        "</project_update>\n",
        url
    );
    retval = send_request(buf);
    if (retval) return retval;
    retval = get_reply(mbuf);
    if (retval) return retval;
    if (mbuf) free(mbuf);
    return 0;
}

char* mode_name(int mode) {
    char* p;
    switch (mode) {
    case RUN_MODE_ALWAYS: p="<always/>"; break;
    case RUN_MODE_NEVER: p="<never/>"; break;
    case RUN_MODE_AUTO: p="<auto/>"; break;
    }
    return p;
}

int RPC_CLIENT::set_run_mode(int mode) {
    char *p, buf[256];
    char* mbuf=0;
    int retval;

    sprintf(buf, "<set_run_mode>\n%s\n</set_run_mode>\n", mode_name(mode));
    retval = send_request(buf);
    if (retval) return retval;
    retval = get_reply(mbuf);
    if (retval) return retval;
    if (mbuf) free(mbuf);
    return 0;
}

int RPC_CLIENT::get_run_mode(int& mode) {
    char buf[256];
    char* mbuf=0;
    int retval;

    retval = send_request("<get_run_mode/>\n");
    if (retval) return retval;
    retval = get_reply(mbuf);
    if (retval) return retval;
    MIOFILE fin;
    fin.init_buf(mbuf);

    while (fin.fgets(buf, 256)) {
        if (match_tag(buf, mode_name(RUN_MODE_ALWAYS))) return RUN_MODE_ALWAYS;
        if (match_tag(buf, mode_name(RUN_MODE_NEVER))) return RUN_MODE_NEVER;
        if (match_tag(buf, mode_name(RUN_MODE_AUTO))) return RUN_MODE_AUTO;
    }
}

int RPC_CLIENT::set_network_mode(int mode) {
    char buf[256];
    char* mbuf=0;
    int retval;

    sprintf(buf,
        "<set_network_mode>\n"
        "%s"
        "</set_network_mode>\n",
        mode_name(mode)
    );
    retval = send_request(buf);
    if (retval) return retval;
    retval = get_reply(mbuf);
    if (retval) return retval;
    if (mbuf) free(mbuf);
    return 0;
}

int RPC_CLIENT::run_benchmarks() {
    char buf[256];
    char* mbuf=0;
    int retval;

    retval = send_request("<run_benchmarks/>\n");
    if (retval) return retval;
    retval = get_reply(mbuf);
    if (retval) return retval;
    if (mbuf) free(mbuf);
    return 0;
}

int RPC_CLIENT::set_proxy_settings(PROXY_INFO& pi) {
    char buf[256];
    char* mbuf=0;
    int retval;

    sprintf(buf,
        "<set_proxy_settings>\n%s%s"
        "    <socks_server_name>%s</socks_server_name>\n"
        "    <socks_server_port>%d</socks_server_port>\n"
        "    <http_server_name>%s</http_server_name>\n"
        "    <http_server_port>%d</http_server_port>\n"
        "    <http_user_name>%d</http_user_name>\n"
        "    <http_user_passwd>%d</http_user_passwd>\n"
        "    <socks5_user_name>%d</socks5_user_name>\n"
        "    <socks5_user_passwd>%d</socks5_user_passwd>\n",
        pi.use_http_proxy?"   <use_http_proxy/>\n":"",
        pi.use_socks_proxy?"   <use_socks_proxy/>\n":"",
        pi.socks_server_name,
        pi.socks_server_port,
        pi.http_server_name,
        pi.http_server_port,
        pi.http_user_name,
        pi.http_user_passwd,
        pi.socks5_user_name,
        pi.socks5_user_passwd
    );
    retval = send_request(buf);
    if (retval) return retval;
    retval = get_reply(mbuf);
    if (retval) return retval;
    if (mbuf) free(mbuf);
    return 0;
}

int RPC_CLIENT::get_messages(
    int nmessages, int seqno, vector<MESSAGE_DESC>& msgs
) {
    char buf[256];
    char* mbuf;
    int retval;

    sprintf(buf,
        "<get_messages>\n"
        "  <nmessages>%d</nmessages>\n"
        "  <seqno>%d</seqno>\n"
        "</get_messages>\n",
        nmessages, seqno
    );
    retval = send_request(buf);
    if (retval) return retval;

    retval = get_reply(mbuf);
    if (retval) return retval;
    MIOFILE fin;
    fin.init_buf(mbuf);

    while (fin.fgets(buf, 256)) {
        puts(buf);
        if (match_tag(buf, "<msgs>")) continue;
        if (match_tag(buf, "</msgs>")) break;
        if (match_tag(buf, "<msg>")) {
            MESSAGE_DESC md;
            while (fin.fgets(buf, 256)) {
                puts(buf);
                if (match_tag(buf, "</msg>")) break;
                if (parse_str(buf, "<project>", md.project)) continue;
                if (match_tag(buf, "<body>" )) {
                    copy_element_contents(fin, "</body>", md.body);
                    continue;
                }
                if (parse_int(buf, "<pri>", md.priority)) continue;
                if (parse_int(buf, "<time>", md.timestamp)) continue;
            }
            msgs.push_back(md);
        }
    }
    if (mbuf) free(mbuf);
    return 0;
}

int RPC_CLIENT::retry_file_transfer(FILE_TRANSFER& ft) {
    char buf[256];
    char* mbuf=0;
    int retval;

    sprintf(buf,
        "<retry_file_transfer>\n"
        "   <project_url>%s</project_url>\n"
        "   <filename>%s</filename>\n"
        "</retry_file_transfer>\n",
        ft.project->master_url.c_str(),
        ft.name.c_str()
    );
    retval = send_request(buf);
    if (retval) return retval;
    retval = get_reply(mbuf);
    if (retval) return retval;
    return 0;
}

void CC_STATE::print() {
    unsigned int i;

    printf("======== Projects ========\n");
    for (i=0; i<projects.size(); i++) {
        printf("%d) -----------\n", i+1);
        projects[i]->print();
    }
    printf("\n======== Applications ========\n");
    for (i=0; i<apps.size(); i++) {
        printf("%d) -----------\n", i+1);
        apps[i]->print();
    }
    printf("\n======== Application versions ========\n");
    for (i=0; i<app_versions.size(); i++) {
        printf("%d) -----------\n", i+1);
        app_versions[i]->print();
    }
    printf("\n======== Workunits ========\n");
    for (i=0; i<wus.size(); i++) {
        printf("%d) -----------\n", i+1);
        wus[i]->print();
    }
    printf("\n======== Results ========\n");
    for (i=0; i<results.size(); i++) {
        printf("%d) -----------\n", i+1);
        results[i]->print();
    }
}

void RESULTS::print() {
    unsigned int i;
    printf("\n======== Results ========\n");
    for (i=0; i<results.size(); i++) {
        printf("%d) -----------\n", i+1);
        results[i]->print();
    }
}

void FILE_TRANSFERS::print() {
    unsigned int i;
    printf("\n======== Files ========\n");
    for (i=0; i<file_transfers.size(); i++) {
        printf("%d) -----------\n", i+1);
        file_transfers[i]->print();
    }
}

int FILE_TRANSFER::parse(MIOFILE& in) {
    char buf[256];
    memset(this, 0, sizeof(*this));
    while (in.fgets(buf, 256)) {
        if (match_tag(buf, "</file_transfer>")) return 0;
        else if (parse_str(buf, "<name>", name)) continue;
        else if (match_tag(buf, "<generated_locally/>")) {
            generated_locally = true;
            continue;
        }
        else if (match_tag(buf, "<uploaded/>")) {
            uploaded = true;
            continue;
        }
        else if (match_tag(buf, "<upload_when_present/>")) {
            upload_when_present = true;
            continue;
        }
        else if (match_tag(buf, "<sticky/>")) {
            sticky = true;
            continue;
        }
        else if (match_tag(buf, "<persistent_file_xfer>")) {
            pers_xfer_active = true;
            continue;
        }
        else if (match_tag(buf, "<file_xfer>")) {
            xfer_active = true;
            continue;
        }
        else if (parse_int(buf, "<num_retries>", num_retries)) continue;
        else if (parse_double(buf, "<bytes_xferred>", bytes_xferred)) continue;
        else if (parse_double(buf, "<file_offset>", file_offset)) continue;
        else if (parse_double(buf, "<xfer_speed>", xfer_speed)) continue;
        else if (parse_str(buf, "<hostname>", hostname)) continue;
    }
    return ERR_XML_PARSE;
}

void FILE_TRANSFER::print() {
    printf("   name: %s\n", name.c_str());
    printf("   generated locally: %s\n", generated_locally?"yes":"no");
    printf("   uploaded: %s\n", uploaded?"yes":"no");
    printf("   upload when present: %s\n", upload_when_present?"yes":"no");
    printf("   sticky: %s\n", sticky?"yes":"no");
    printf("   generated locally: %s\n", generated_locally?"yes":"no");
}

int PROJECT::parse(MIOFILE& in) {
    char buf[256];
    memset(this, 0, sizeof(*this));
    while (in.fgets(buf, 256)) {
        if (match_tag(buf, "</project>")) return 0;
        else if (parse_str(buf, "<master_url>", master_url)) continue;
        else if (parse_double(buf, "<resource_share>", resource_share)) continue;
        else if (parse_str(buf, "<project_name>", project_name)) continue;
        else if (parse_str(buf, "<user_name>", user_name)) continue;
        else if (parse_str(buf, "<team_name>", team_name)) continue;
        else if (parse_double(buf, "<user_total_credit>", user_total_credit)) continue;
        else if (parse_double(buf, "<user_expavg_credit>", user_expavg_credit)) continue;
        else if (parse_double(buf, "<host_total_credit>", host_total_credit)) continue;
        else if (parse_double(buf, "<host_expavg_credit>", host_expavg_credit)) continue;
        else if (parse_int(buf, "<nrpc_failures>", nrpc_failures)) continue;
        else if (parse_int(buf, "<master_fetch_failures>", master_fetch_failures)) continue;
        else if (parse_int(buf, "<min_rpc_time>", min_rpc_time)) continue;
        else if (match_tag(buf, "<master_url_fetch_pending>")) {
            master_url_fetch_pending = true;
            continue;
        }
        else if (match_tag(buf, "<sched_rpc_pending>")) {
            sched_rpc_pending = true;
            continue;
        }
        else if (match_tag(buf, "<tentative>")) {
            tentative = true;
            continue;
        }
    }
    return ERR_XML_PARSE;
}

void PROJECT::print() {
    printf("   name: %s\n", project_name.c_str());
    printf("   master URL: %s\n", master_url.c_str());
    printf("   user_name: %s\n", user_name.c_str());
    printf("   team_name: %s\n", team_name.c_str());
    printf("   resource share: %f\n", resource_share);
    printf("   user_total_credit: %f\n", user_total_credit);
    printf("   user_expavg_credit: %f\n", user_expavg_credit);
    printf("   host_total_credit: %f\n", host_total_credit);
    printf("   host_expavg_credit: %f\n", host_expavg_credit);
    printf("   nrpc_failures: %d\n", nrpc_failures);
    printf("   master_fetch_failures: %d\n", master_fetch_failures);
    printf("   master fetch pending: %s\n", master_url_fetch_pending?"yes":"no");
    printf("   scheduler RPC pending: %s\n", sched_rpc_pending?"yes":"no");
    printf("   tentative: %s\n", tentative?"yes":"no");
}

int APP::parse(MIOFILE& in) {
    char buf[256];
    memset(this, 0, sizeof(*this));
    while (in.fgets(buf, 256)) {
        if (match_tag(buf, "</app>")) return 0;
        else if (parse_str(buf, "<name>", name)) continue;
    }
    return ERR_XML_PARSE;
}

void APP::print() {
    printf("   name: %s\n", name.c_str());
    printf("   Project: %s\n", project->project_name.c_str());
}

int APP_VERSION::parse(MIOFILE& in) {
    char buf[256];
    memset(this, 0, sizeof(*this));
    while (in.fgets(buf, 256)) {
        if (match_tag(buf, "</app_version>")) return 0;
        else if (parse_str(buf, "<app_name>", app_name)) continue;
        else if (parse_int(buf, "<version_num>", version_num)) continue;
    }
    return ERR_XML_PARSE;
}

void APP_VERSION::print() {
    printf("   application: %s\n", app->name.c_str());
    printf("   version: %.2f\n", version_num/100.0);
    printf("   project: %s\n", project->project_name.c_str());
}

int WORKUNIT::parse(MIOFILE& in) {
    char buf[256];
    memset(this, 0, sizeof(*this));
    while (in.fgets(buf, 256)) {
        if (match_tag(buf, "</workunit>")) return 0;
        else if (parse_str(buf, "<name>", name)) continue;
        else if (parse_str(buf, "<app_name>", app_name)) continue;
        else if (parse_int(buf, "<version_num>", version_num)) continue;
        else if (parse_double(buf, "<rsc_fpops_est>", rsc_fpops_est)) continue;
        else if (parse_double(buf, "<rsc_fpops_bound>", rsc_fpops_bound)) continue;
        else if (parse_double(buf, "<rsc_memory_bound>", rsc_memory_bound)) continue;
        else if (parse_double(buf, "<rsc_disk_bound>", rsc_disk_bound)) continue;
    }
    return ERR_XML_PARSE;
}

void WORKUNIT::print() {
    printf("   name: %s\n", name.c_str());
    printf("   FP estimate: %f\n", rsc_fpops_est);
    printf("   FP bound: %f\n", rsc_fpops_bound);
    printf("   memory bound: %f\n", rsc_memory_bound);
    printf("   disk bound: %f\n", rsc_disk_bound);
}

int RESULT::parse(MIOFILE& in) {
    char buf[256];
    memset(this, 0, sizeof(*this));
    while (in.fgets(buf, 256)) {
        if (match_tag(buf, "</result>")) return 0;
        else if (parse_str(buf, "<name>", name)) continue;
        else if (parse_str(buf, "<wu_name>", wu_name)) continue;
        else if (parse_int(buf, "<report_deadline>", report_deadline)) continue;
        else if (match_tag(buf, "<ready_to_report>")) {
            ready_to_report = true;
            continue;
        }
        else if (match_tag(buf, "<got_server_ack>")) {
            got_server_ack = true;
            continue;
        }
        else if (parse_double(buf, "<final_cpu_time>", final_cpu_time)) continue;
        else if (parse_int(buf, "<state>", state)) continue;
        else if (parse_int(buf, "<exit_status>", exit_status)) continue;
        else if (parse_int(buf, "<signal>", signal)) continue;
        else if (parse_int(buf, "<active_task_state>", active_task_state)) continue;
        else if (match_tag(buf, "<stderr_out>")) {
            copy_element_contents(in, "</stderr_out>", stderr_out);
        }
        else if (parse_int(buf, "<app_version_num>", app_version_num)) continue;
        else if (parse_double(buf, "<checkpoint_cpu_time>", checkpoint_cpu_time)) continue;
        else if (parse_double(buf, "<current_cpu_time>", current_cpu_time)) continue;
        else if (parse_double(buf, "<fraction_done>", fraction_done)) continue;
    }
    return ERR_XML_PARSE;
}

void RESULT::print() {
    printf("   name: %s\n", name.c_str());
    printf("   WU name: %s\n", wup->name.c_str());
    printf("   ready to report: %s\n", ready_to_report?"yes":"no");
    printf("   got server ack: %s\n", got_server_ack?"yes":"no");
    printf("   final CPU time: %f\n", final_cpu_time);
    printf("   state: %d\n", state);
    printf("   exit_status: %d\n", exit_status);
    printf("   signal: %d\n", signal);
    printf("   active_task_state: %d\n", active_task_state);
    printf("   stderr_out: %s\n", stderr_out.c_str());
    printf("   app version num: %d\n", app_version_num);
    printf("   checkpoint CPU time: %f\n", checkpoint_cpu_time);
    printf("   current CPU time: %f\n", current_cpu_time);
    printf("   fraction done: %f\n", fraction_done);
}

APP* CC_STATE::lookup_app(string& str) {
    unsigned int i;
    for (i=0; i<apps.size(); i++) {
        if (apps[i]->name == str) return apps[i];
    }
    printf("CAN'T FIND APP %s\n", str.c_str());
    return 0;
}

WORKUNIT* CC_STATE::lookup_wu(string& str) {
    unsigned int i;
    for (i=0; i<wus.size(); i++) {
        if (wus[i]->name == str) return wus[i];
    }
    printf("CAN'T FIND WU %s\n", str.c_str());
    return 0;
}

RESULT* CC_STATE::lookup_result(string& str) {
    unsigned int i;
    for (i=0; i<results.size(); i++) {
        if (results[i]->name == str) return results[i];
    }
    printf("CAN'T FIND RESULT %s\n", str.c_str());
    return 0;
}


APP_VERSION* CC_STATE::lookup_app_version(string& str, int version_num) {
    unsigned int i;
    for (i=0; i<app_versions.size(); i++) {
        if (app_versions[i]->app_name == str && app_versions[i]->version_num == version_num) return app_versions[i];
    }
    return 0;
}

