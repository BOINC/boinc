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
#include "version.h"
#endif

#ifndef _WIN32
#include "config.h"
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

#include "diagnostics.h"
#include "parse.h"
#include "error_numbers.h"
#include "miofile.h"
#include "gui_rpc_client.h"

using std::string;
using std::vector;


int GUI_URL::parse(MIOFILE& in) {
    char buf[256];
    while (in.fgets(buf, 256)) {
        if (match_tag(buf, "</gui_url>")) return 0;
        else if (parse_str(buf, "<name>", name)) continue;
        else if (parse_str(buf, "<description>", description)) continue;
        else if (parse_str(buf, "<url>", url)) continue;
    }
    return ERR_XML_PARSE;
}

void GUI_URL::print() {
    printf(
        "GUI URL:\n"
        "   name: %s\n"
        "   description: %s\n"
        "   URL: %s\n",
        name.c_str(), description.c_str(), url.c_str()
    );
}

PROJECT::PROJECT() {
    clear();
}

PROJECT::~PROJECT() {
    clear();
}

int PROJECT::parse(MIOFILE& in) {
    char buf[256];
    int retval;

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
        else if (parse_double(buf, "<disk_usage>", disk_usage)) continue;
        else if (parse_int(buf, "<nrpc_failures>", nrpc_failures)) continue;
        else if (parse_int(buf, "<master_fetch_failures>", master_fetch_failures)) continue;
        else if (parse_double(buf, "<min_rpc_time>", min_rpc_time)) continue;
        else if (match_tag(buf, "<master_url_fetch_pending/>")) {
            master_url_fetch_pending = true;
            continue;
        }
        else if (match_tag(buf, "<sched_rpc_pending/>")) {
            sched_rpc_pending = true;
            continue;
        }
        else if (match_tag(buf, "<non_cpu_intensive/>")) {
            non_cpu_intensive = true;
            continue;
        }
        else if (match_tag(buf, "<suspended_via_gui/>")) {
            suspended_via_gui = true;
            continue;
        }
        else if (match_tag(buf, "<dont_request_more_work/>")) {
            dont_request_more_work = true;
            continue;
        }
        else if (match_tag(buf, "<tentative/>")) {
            tentative = true;
            continue;
        }
        else if (match_tag(buf, "<gui_urls>")) {
            while (in.fgets(buf, 256)) {
                if (match_tag(buf, "</gui_urls>")) break;
                else if (match_tag(buf, "<gui_url>")) {
                    GUI_URL gu;
                    retval = gu.parse(in);
                    if (!retval) {
                        gui_urls.push_back(gu);
                    }
                }
            }
            continue;
        }
    }
    return ERR_XML_PARSE;
}

void PROJECT::print() {
    unsigned int i;

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
    printf("   suspended via GUI: %s\n", suspended_via_gui?"yes":"no");
    printf("   don't request more work: %s\n", dont_request_more_work?"yes":"no");
    printf("   disk usage: %f\n", disk_usage);
    for (i=0; i<gui_urls.size(); i++) {
        gui_urls[i].print();
    }
}

void PROJECT::clear() {
    master_url.clear();
    resource_share = 0.0;
    project_name.clear();
    user_name.clear();
    team_name.clear();
    user_total_credit = 0.0;
    user_expavg_credit = 0.0;
    host_total_credit = 0.0;
    host_expavg_credit = 0.0;
    disk_usage = 0.0;
    nrpc_failures = 0;
    master_fetch_failures = 0;
    min_rpc_time = 0;
    master_url_fetch_pending = false;
    sched_rpc_pending = false;
    tentative = false;
    non_cpu_intensive = false;
    suspended_via_gui = false;
	dont_request_more_work = false;
    gui_urls.clear();
}

APP::APP() {
    clear();
}

APP::~APP() {
    clear();
}

int APP::parse(MIOFILE& in) {
    char buf[256];
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

void APP::clear() {
    name.clear();
    project = NULL;
}

APP_VERSION::APP_VERSION() {
    clear();
}

APP_VERSION::~APP_VERSION() {
    clear();
}

int APP_VERSION::parse(MIOFILE& in) {
    char buf[256];
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

void APP_VERSION::clear() {
    app_name.clear();
    version_num = 0;
    app = NULL;
    project = NULL;
}

WORKUNIT::WORKUNIT() {
    clear();
}

WORKUNIT::~WORKUNIT() {
    clear();
}

int WORKUNIT::parse(MIOFILE& in) {
    char buf[256];
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

void WORKUNIT::clear() {
    name.clear();
    app_name.clear();
    version_num = 0;
    rsc_fpops_est = 0.0;
    rsc_fpops_bound = 0.0;
    rsc_memory_bound = 0.0;
    rsc_disk_bound = 0.0;
    project = NULL;
    app = NULL;
    avp = NULL;
}

RESULT::RESULT() {
    clear();
}

RESULT::~RESULT() {
    clear();
}

int RESULT::parse(MIOFILE& in) {
    char buf[256];
    while (in.fgets(buf, 256)) {
        if (match_tag(buf, "</result>")) return 0;
        else if (parse_str(buf, "<name>", name)) continue;
        else if (parse_str(buf, "<wu_name>", wu_name)) continue;
        else if (parse_str(buf, "<project_url>", project_url)) continue;
        else if (parse_int(buf, "<report_deadline>", report_deadline)) continue;
        else if (match_tag(buf, "<ready_to_report/>")) {
            ready_to_report = true;
            continue;
        }
        else if (match_tag(buf, "<got_server_ack/>")) {
            got_server_ack = true;
            continue;
        }
        else if (match_tag(buf, "<suspended_via_gui/>")) {
            suspended_via_gui = true;
            continue;
        }
        else if (match_tag(buf, "<aborted_via_gui/>")) {
            aborted_via_gui = true;
            continue;
        }
        else if (match_tag(buf, "<active_task>")) {
            active_task = true;
            continue;
        }
        else if (match_tag(buf, "<supports_graphics/>")) {
            supports_graphics = true;
            continue;
        }
        else if (parse_double(buf, "<final_cpu_time>", final_cpu_time)) continue;
        else if (parse_int(buf, "<state>", state)) continue;
        else if (parse_int(buf, "<scheduler_state>", scheduler_state)) continue;
        else if (parse_int(buf, "<exit_status>", exit_status)) continue;
        else if (parse_int(buf, "<signal>", signal)) continue;
        else if (parse_int(buf, "<active_task_state>", active_task_state)) continue;
        else if (match_tag(buf, "<stderr_out>")) {
            copy_element_contents(in, "</stderr_out>", stderr_out);
        }
        else if (parse_int(buf, "<app_version_num>", app_version_num)) continue;
        else if (parse_double(buf, "<checkpoint_cpu_time>", checkpoint_cpu_time)) continue;
        else if (parse_double(buf, "<current_cpu_time>", current_cpu_time)) continue;
        else if (parse_double(buf, "<vm_bytes>", vm_bytes)) continue;
        else if (parse_double(buf, "<rss_bytes>", rss_bytes)) continue;
        else if (parse_double(buf, "<fraction_done>", fraction_done)) continue;
        else if (parse_double(buf, "<estimated_cpu_time_remaining>", estimated_cpu_time_remaining)) continue;
    }
    return ERR_XML_PARSE;
}

void RESULT::print() {
    printf("   name: %s\n", name.c_str());
    printf("   WU name: %s\n", wu_name.c_str());
    printf("   project URL: %s\n", project_url.c_str());
    time_t foo = (time_t)report_deadline;
    printf("   report deadline: %s", ctime(&foo));
    printf("   ready to report: %s\n", ready_to_report?"yes":"no");
    printf("   got server ack: %s\n", got_server_ack?"yes":"no");
    printf("   final CPU time: %f\n", final_cpu_time);
    printf("   state: %d\n", state);
    printf("   scheduler state: %d\n", scheduler_state);
    printf("   exit_status: %d\n", exit_status);
    printf("   signal: %d\n", signal);
    printf("   suspended via GUI: %s\n", suspended_via_gui?"yes":"no");
    printf("   aborted via GUI: %s\n", aborted_via_gui?"yes":"no");
    printf("   active_task_state: %d\n", active_task_state);
    printf("   stderr_out: %s\n", stderr_out.c_str());
    printf("   app version num: %d\n", app_version_num);
    printf("   checkpoint CPU time: %f\n", checkpoint_cpu_time);
    printf("   current CPU time: %f\n", current_cpu_time);
    printf("   fraction done: %f\n", fraction_done);
    printf("   VM usage: %f\n", vm_bytes);
    printf("   resident set size: %f\n", rss_bytes);
    printf("   estimated CPU time remaining: %f\n", estimated_cpu_time_remaining);
    printf("   supports graphics: %s\n", supports_graphics?"yes":"no");
}

void RESULT::clear() {
    name.clear();
    wu_name.clear();
    project_url.clear();
    report_deadline = 0;
    ready_to_report = false;
    got_server_ack = false;
    final_cpu_time = 0.0;
    state = 0;
    scheduler_state = 0;
    exit_status = 0;
    signal = 0;
    active_task_state = 0;
    active_task = false;
    stderr_out.clear();
    app_version_num = 0;
    checkpoint_cpu_time = 0.0;
    current_cpu_time = 0.0;
    fraction_done = 0.0;
    estimated_cpu_time_remaining = 0.0;
    suspended_via_gui = false;
    aborted_via_gui = false;
    supports_graphics = false;
}

FILE_TRANSFER::FILE_TRANSFER() {
    clear();
}

FILE_TRANSFER::~FILE_TRANSFER() {
    clear();
}

int FILE_TRANSFER::parse(MIOFILE& in) {
    char buf[256];
    while (in.fgets(buf, 256)) {
        if (match_tag(buf, "</file_transfer>")) return 0;
        else if (parse_str(buf, "<name>", name)) continue;
        else if (parse_str(buf, "<project_url>", project_url)) continue;
        else if (parse_str(buf, "<project_name>", project_name)) continue;
        else if (parse_double(buf, "<nbytes>", nbytes)) continue;
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
        else if (parse_int(buf, "<first_request_time>", first_request_time)) continue;
        else if (parse_int(buf, "<next_request_time>", next_request_time)) continue;
        else if (parse_int(buf, "<status>", status)) continue;
        else if (parse_double(buf, "<time_so_far>", time_so_far)) continue;
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

void FILE_TRANSFER::clear() {
    name.clear();
    project_url.clear();
    project_name.clear();
    nbytes = 0.0;
    generated_locally = false;
    uploaded = false;
    upload_when_present = false;
    sticky = false;
    pers_xfer_active = false;
    xfer_active = false;
    num_retries = 0;
    first_request_time = 0;
    next_request_time = 0;
    status = 0;
    time_so_far = 0.0;
    bytes_xferred = 0.0;
    file_offset = 0.0;
    xfer_speed = 0.0;
    hostname.clear();
    project = NULL;
}

MESSAGE::MESSAGE() {
    clear();
}

MESSAGE::~MESSAGE() {
    clear();
}

int MESSAGE::parse(MIOFILE& in) {
    char buf[256];
    while (in.fgets(buf, 256)) {
        if (match_tag(buf, "</msg>")) return 0;
        else if (parse_str(buf, "<project>", project)) continue;
        else if (match_tag(buf, "<body>" )) {
            copy_element_contents(in, "</body>", body);
            continue;
        }
        else if (parse_int(buf, "<pri>", priority)) continue;
        else if (parse_int(buf, "<time>", timestamp)) continue;
        else if (parse_int(buf, "<seqno>", seqno)) continue;
    }
    return ERR_XML_PARSE;
}

void MESSAGE::print() {
    printf("%s %d %d %s\n",
        project.c_str(), priority, timestamp, body.c_str()
    );
}

void MESSAGE::clear() {
    project.clear();
    priority = 0;
    timestamp = 0;
    body.clear();
}

PROXY_INFO::PROXY_INFO() {
    clear();
}

PROXY_INFO::~PROXY_INFO() {
    clear();
}

int PROXY_INFO::parse(MIOFILE& in) {
    char buf[4096];
    use_http_proxy = false;
    use_socks_proxy = false;
    use_http_authentication = false;
    while (in.fgets(buf, 256)) {
        if (match_tag(buf, "</proxy_info>")) return 0;
        else if (parse_int(buf, "<socks_version>", socks_version)) continue;
        else if (parse_str(buf, "<socks_server_name>", socks_server_name)) continue;
        else if (parse_int(buf, "<socks_server_port>", socks_server_port)) continue;
        else if (parse_str(buf, "<socks5_user_name>", socks5_user_name)) continue;
        else if (parse_str(buf, "<socks5_user_passwd>", socks5_user_passwd)) continue;
        else if (parse_str(buf, "<http_server_name>", http_server_name)) continue;
        else if (parse_int(buf, "<http_server_port>", http_server_port)) continue;
        else if (parse_str(buf, "<http_user_name>", http_user_name)) continue;
        else if (parse_str(buf, "<http_user_passwd>", http_user_passwd)) continue;
        else if (match_tag(buf, "<use_http_proxy/>")) {
            use_http_proxy = true;
            continue;
        }
        else if (match_tag(buf, "<use_socks_proxy/>")) {
            use_socks_proxy = true;
            continue;
        }
        else if (match_tag(buf, "<use_http_auth/>")) {
            use_http_authentication = true;
            continue;
        }
    }
    return ERR_XML_PARSE;
}

void PROXY_INFO::print() {
}

void PROXY_INFO::clear() {
    use_http_proxy = false;
    use_socks_proxy = false;
    use_http_authentication = false;
    socks_version = 0;
    socks_server_name.clear();
    http_server_name.clear();
    socks_server_port = 0;
    http_server_port = 0;
    http_user_name.clear();
    http_user_passwd.clear();
    socks5_user_name.clear();
    socks5_user_passwd.clear();
}

HOST_INFO::HOST_INFO() {
    clear();
}

HOST_INFO::~HOST_INFO() {
    clear();
}

int HOST_INFO::parse(MIOFILE& in) {
    char buf[256];

    memset(this, 0, sizeof(HOST_INFO));
    while (in.fgets(buf, 256)) {
        if (match_tag(buf, "</host_info>")) return 0;
        else if (parse_int(buf, "<timezone>", timezone)) continue;
        else if (parse_str(buf, "<domain_name>", domain_name, sizeof(domain_name))) continue;
        else if (parse_str(buf, "<ip_addr>", ip_addr, sizeof(ip_addr))) continue;
        else if (parse_int(buf, "<p_ncpus>", p_ncpus)) continue;
        else if (parse_str(buf, "<p_vendor>", p_vendor, sizeof(p_vendor))) continue;
        else if (parse_str(buf, "<p_model>", p_model, sizeof(p_model))) continue;
        else if (parse_double(buf, "<p_fpops>", p_fpops)) {
            // fix foolishness that could result in negative value here
            //
            if (p_fpops < 0) p_fpops = -p_fpops;
            continue;
        }
        else if (parse_double(buf, "<p_iops>", p_iops)) {
            if (p_iops < 0) p_iops = -p_iops;
            continue;
        }
        else if (parse_double(buf, "<p_membw>", p_membw)) {
            if (p_membw < 0) p_membw = -p_membw;
            continue;
        }
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
    }
    return 0;
}

void HOST_INFO::print() {
    printf("  timezone: %d\n", timezone);
    printf("  domain name: %s\n", domain_name);
    printf("  IP addr: %s\n", ip_addr);
    printf("  #CPUS: %d\n", p_ncpus);
    printf("  CPU vendor: %s\n", p_vendor);
    printf("  CPU model: %s\n", p_model);
    printf("  CPU FP OPS: %f\n", p_fpops);
    printf("  CPU int OPS: %f\n", p_iops);
    printf("  CPU mem BW: %f\n", p_membw);
    printf("  OS name: %s\n", os_name);
    printf("  OS version: %s\n", os_version);
    printf("  mem size: %f\n", m_nbytes);
    printf("  cache size: %f\n", m_cache);
    printf("  swap size: %f\n", m_swap);
    printf("  disk size: %f\n", d_total);
    printf("  disk free: %f\n", d_free);
}

void HOST_INFO::clear() {
    timezone = 0;
    strcpy(domain_name, "");
    strcpy(serialnum, "");
    strcpy(ip_addr, "");
    p_ncpus = 0;
    strcpy(p_vendor, "");
    strcpy(p_model, "");
    p_fpops = 0.0;
    p_iops = 0.0;
    p_membw = 0.0;
    p_fpop_err = 0;
    p_iop_err = 0;
    p_membw_err = 0;
    p_calculated = 0.0;
    strcpy(os_name, "");
    strcpy(os_version, "");
    m_nbytes = 0.0;
    m_cache = 0.0;
    m_swap = 0.0;
    d_total = 0.0;
    d_free = 0.0;
}

CC_STATE::CC_STATE() {
    clear();
}

CC_STATE::~CC_STATE() {
    clear();
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

void CC_STATE::clear() {
    unsigned int i;
    for (i=0; i<projects.size(); i++) {
        delete projects[i];
    }
    projects.clear();
    for (i=0; i<apps.size(); i++) {
        delete apps[i];
    }
    apps.clear();
    for (i=0; i<app_versions.size(); i++) {
        delete app_versions[i];
    }
    app_versions.clear();
    for (i=0; i<wus.size(); i++) {
        delete wus[i];
    }
    wus.clear();
    for (i=0; i<results.size(); i++) {
        delete results[i];
    }
    results.clear();
}

PROJECT* CC_STATE::lookup_project(string& str) {
    unsigned int i;
    for (i=0; i<projects.size(); i++) {
        if (projects[i]->master_url == str) return projects[i];
    }
    printf("CAN'T FIND PROJECT %s\n", str.c_str());
    return 0;
}

APP* CC_STATE::lookup_app(string& project_url, string& str) {
    unsigned int i;
    for (i=0; i<apps.size(); i++) {
        if (apps[i]->project->master_url != project_url) continue;
        if (apps[i]->name == str) return apps[i];
    }
    printf("CAN'T FIND APP %s\n", str.c_str());
    return 0;
}

APP* CC_STATE::lookup_app(PROJECT* project, string& str) {
    unsigned int i;
    for (i=0; i<apps.size(); i++) {
        if (apps[i]->project != project) continue;
        if (apps[i]->name == str) return apps[i];
    }
    printf("CAN'T FIND APP %s\n", str.c_str());
    return 0;
}

APP_VERSION* CC_STATE::lookup_app_version(
    string& project_url, string& str, int version_num
) {
    unsigned int i;
    for (i=0; i<app_versions.size(); i++) {
        if (app_versions[i]->project->master_url != project_url) continue;
        if (app_versions[i]->app_name == str && app_versions[i]->version_num == version_num) return app_versions[i];
    }
    return 0;
}

APP_VERSION* CC_STATE::lookup_app_version(
    PROJECT* project, string& str, int version_num
) {
    unsigned int i;
    for (i=0; i<app_versions.size(); i++) {
        if (app_versions[i]->project != project) continue;
        if (app_versions[i]->app_name == str && app_versions[i]->version_num == version_num) return app_versions[i];
    }
    return 0;
}

WORKUNIT* CC_STATE::lookup_wu(string& project_url, string& str) {
    unsigned int i;
    for (i=0; i<wus.size(); i++) {
        if (wus[i]->project->master_url != project_url) continue;
        if (wus[i]->name == str) return wus[i];
    }
    printf("CAN'T FIND WU %s\n", str.c_str());
    return 0;
}

WORKUNIT* CC_STATE::lookup_wu(PROJECT* project, string& str) {
    unsigned int i;
    for (i=0; i<wus.size(); i++) {
        if (wus[i]->project != project) continue;
        if (wus[i]->name == str) return wus[i];
    }
    printf("CAN'T FIND WU %s\n", str.c_str());
    return 0;
}

RESULT* CC_STATE::lookup_result(string& project_url, string& str) {
    unsigned int i;
    for (i=0; i<results.size(); i++) {
        if (results[i]->project->master_url != project_url) continue;
        if (results[i]->name == str) return results[i];
    }
    printf("CAN'T FIND RESULT %s\n", str.c_str());
    return 0;
}

RESULT* CC_STATE::lookup_result(PROJECT* project, string& str) {
    unsigned int i;
    for (i=0; i<results.size(); i++) {
        if (results[i]->project != project) continue;
        if (results[i]->name == str) return results[i];
    }
    printf("CAN'T FIND RESULT %s\n", str.c_str());
    return 0;
}

PROJECTS::PROJECTS() {
    clear();
}

PROJECTS::~PROJECTS() {
    clear();
}

void PROJECTS::print() {
    unsigned int i;
    printf("======== Projects ========\n");
    for (i=0; i<projects.size(); i++) {
        printf("%d) -----------\n", i+1);
        projects[i]->print();
    }
}

void PROJECTS::clear() {
    unsigned int i;
    for (i=0; i<projects.size(); i++) {
        delete projects[i];
    }
    projects.clear();
}

RESULTS::RESULTS() {
    clear();
}

RESULTS::~RESULTS() {
    clear();
}

void RESULTS::print() {
    unsigned int i;
    printf("\n======== Results ========\n");
    for (i=0; i<results.size(); i++) {
        printf("%d) -----------\n", i+1);
        results[i]->print();
    }
}

void RESULTS::clear() {
    unsigned int i;
    for (i=0; i<results.size(); i++) {
        delete results[i];
    }
    results.clear();
}

FILE_TRANSFERS::FILE_TRANSFERS() {
    clear();
}

FILE_TRANSFERS::~FILE_TRANSFERS() {
    clear();
}

void FILE_TRANSFERS::print() {
    unsigned int i;
    printf("\n======== File transfers ========\n");
    for (i=0; i<file_transfers.size(); i++) {
        printf("%d) -----------\n", i+1);
        file_transfers[i]->print();
    }
}

void FILE_TRANSFERS::clear() {
    unsigned int i;
    for (i=0; i<file_transfers.size(); i++) {
        delete file_transfers[i];
    }
    file_transfers.clear();
}

MESSAGES::MESSAGES() {
    clear();
}

MESSAGES::~MESSAGES() {
    clear();
}

void MESSAGES::print() {
    unsigned int i;
    printf("\n======== Messages ========\n");
    for (i=0; i<messages.size(); i++) {
        printf("%d) -----------\n", i+1);
        messages[i]->print();
    }
}

void MESSAGES::clear() {
    unsigned int i;
    for (i=0; i<messages.size(); i++) {
        delete messages[i];
    }
    messages.clear();
}

RPC_CLIENT::RPC_CLIENT() {
}

RPC_CLIENT::~RPC_CLIENT() {
}

// if any RPC returns ERR_READ or ERR_WRITE,
// call this and then call init() again.
//
void RPC_CLIENT::close() {
#ifdef _WIN32
    ::closesocket(sock);
#else
    ::close(sock);
#endif
    sock = 0;
}

int RPC_CLIENT::init(const char* host) {
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
        close();
        return ERR_CONNECT;
    }
    return 0;
}

int RPC_CLIENT::send_request(char* p) {
    char buf[4096];
    sprintf(buf,
        "<boinc_gui_rpc_request>\n"
        "   <version>%d</version>\n"
        "%s"
        "</boinc_gui_rpc_request>\n\003",
        BOINC_MAJOR_VERSION*100 + BOINC_MINOR_VERSION,
        p
    );
    int n = send(sock, buf, strlen(buf), 0);
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

RPC::RPC(RPC_CLIENT* rc) {
    mbuf = 0;
    rpc_client = rc;
}

RPC::~RPC() {
    if (mbuf) free(mbuf);
}

int RPC::do_rpc(char* req) {
    int retval;

    if (rpc_client->sock == 0) return ERR_CONNECT;
    retval = rpc_client->send_request(req);
    if (retval) return retval;
    retval = rpc_client->get_reply(mbuf);
    if (retval) return retval;
    fin.init_buf(mbuf);
    return 0;
}

int RPC_CLIENT::get_state(CC_STATE& state) {
    char buf[256];
    PROJECT* project;
    RPC rpc(this);
    int retval;

    state.clear();

    retval = rpc.do_rpc("<get_state/>\n");
    if (retval) return retval;

    while (rpc.fin.fgets(buf, 256)) {
        if (match_tag(buf, "</client_state>")) break;
        else if (match_tag(buf, "<project>")) {
            project = new PROJECT();
            project->parse(rpc.fin);
            state.projects.push_back(project);
            continue;
        }
        else if (match_tag(buf, "<app>")) {
            APP* app = new APP();
            app->parse(rpc.fin);
            app->project = project;
            state.apps.push_back(app);
            continue;
        }
        else if (match_tag(buf, "<app_version>")) {
            APP_VERSION* app_version = new APP_VERSION();
            app_version->parse(rpc.fin);
            app_version->project = project;
            app_version->app = state.lookup_app(project, app_version->app_name);
            state.app_versions.push_back(app_version);
            continue;
        }
        else if (match_tag(buf, "<workunit>")) {
            WORKUNIT* wu = new WORKUNIT();
            wu->parse(rpc.fin);
            wu->project = project;
            wu->app = state.lookup_app(project, wu->app_name);
            wu->avp = state.lookup_app_version(project, wu->app_name, wu->version_num);
            state.wus.push_back(wu);
            continue;
        }
        else if (match_tag(buf, "<result>")) {
            RESULT* result = new RESULT();
            result->parse(rpc.fin);
            result->project = project;
            result->wup = state.lookup_wu(project, result->wu_name);
            result->app = result->wup->app;
            state.results.push_back(result);
            continue;
        }
    }
    return 0;
}

int RPC_CLIENT::get_results(RESULTS& t) {
    char buf[256];
    int retval;
    RPC rpc(this);

    t.clear();

    retval = rpc.do_rpc("<get_results/>\n");
    if (retval) return retval;

    while (rpc.fin.fgets(buf, 256)) {
        if (match_tag(buf, "</results>")) break;
        else if (match_tag(buf, "<result>")) {
            RESULT* rp = new RESULT();
            rp->parse(rpc.fin);
            t.results.push_back(rp);
            continue;
        }
    }
    return 0;
}

int RPC_CLIENT::get_file_transfers(FILE_TRANSFERS& t) {
    char buf[256];
    int retval;
    RPC rpc(this);

    t.clear();

    retval = rpc.do_rpc("<get_file_transfers/>\n");
    if (retval) return retval;

    while (rpc.fin.fgets(buf, 256)) {
        if (match_tag(buf, "</file_transfers>")) break;
        else if (match_tag(buf, "<file_transfer>")) {
            FILE_TRANSFER* fip = new FILE_TRANSFER();
            fip->parse(rpc.fin);
            t.file_transfers.push_back(fip);
            continue;
        }
    }
    return 0;
}

int RPC_CLIENT::get_project_status(PROJECTS& p) {
    char buf[256];
    RPC rpc(this);
    int retval;

    p.clear();

    retval = rpc.do_rpc("<get_project_status/>\n");
    if (retval) return retval;

    while (rpc.fin.fgets(buf, 256)) {
        if (match_tag(buf, "</projects>")) break;
        else if (match_tag(buf, "<project>")) {
            PROJECT* project = new PROJECT();
            project->parse(rpc.fin);
            p.projects.push_back(project);
            continue;
        }
    }
    return 0;
}

int RPC_CLIENT::get_disk_usage(PROJECTS& p) {
    char buf[256];
    RPC rpc(this);
    int retval;

    p.clear();

    retval = rpc.do_rpc("<get_disk_usage/>\n");
    if (retval) return retval;

    while (rpc.fin.fgets(buf, 256)) {
        if (match_tag(buf, "</projects>")) break;
        else if (match_tag(buf, "<project>")) {
            PROJECT* project = new PROJECT();
            project->parse(rpc.fin);
            p.projects.push_back(project);
            continue;
        }
    }
    return 0;
}

int RPC_CLIENT::show_graphics(
    const char* project_url, const char* result_name, bool full_screen,
    const char* window_station, const char* desktop
) {
    char buf[1024];
    RPC rpc(this);

    sprintf(buf, 
        "<result_show_graphics>\n"
        "   <project_url>%s</project_url>\n"
        "   <result_name>%s</result_name>\n"
        "%s"
        "   <window_station>%s</window_station>\n"
        "   <desktop>%s</desktop>\n"
        "</result_show_graphics>\n",
        project_url,
        result_name,
        full_screen?"   <full_screen/>\n":"",
        window_station,
        desktop
    );
    return rpc.do_rpc(buf);
}

int RPC_CLIENT::project_op(PROJECT& project, char* op) {
    char buf[256], *tag;
    RPC rpc(this);

    if (!strcmp(op, "reset")) {
        tag = "project_reset";
    } else if (!strcmp(op, "detach")) {
        tag = "project_detach";
    } else if (!strcmp(op, "update")) {
        tag = "project_update";
    } else if (!strcmp(op, "suspend")) {
        tag = "project_suspend";
    } else if (!strcmp(op, "resume")) {
        tag = "project_resume";
    } else if (!strcmp(op, "allowmorework")) {
         tag = "project_allowmorework";
    } else if (!strcmp(op, "nomorework")) {
         tag = "project_nomorework";
    } else {
        return -1;
    }
    sprintf(buf,
        "<%s>\n"
        "  <project_url>%s</project_url>\n"
        "</%s>\n",
        tag,
        project.master_url.c_str(),
        tag
    );
    return rpc.do_rpc(buf);
}

int RPC_CLIENT::project_attach(char* url, char* auth) {
    char buf[256];
    RPC rpc(this);

    sprintf(buf,
        "<project_attach>\n"
        "  <project_url>%s</project_url>\n"
        "  <authenticator>%s</authenticator>\n"
        "</project_attach>\n",
        url, auth
    );
    return rpc.do_rpc(buf);
}

char* RPC_CLIENT::mode_name(int mode) {
    char* p = NULL;
    switch (mode) {
    case RUN_MODE_ALWAYS: p="<always/>"; break;
    case RUN_MODE_NEVER: p="<never/>"; break;
    case RUN_MODE_AUTO: p="<auto/>"; break;
    }
    return p;
}

int RPC_CLIENT::set_run_mode(int mode) {
    char buf[256];
    RPC rpc(this);

    sprintf(buf, "<set_run_mode>\n%s\n</set_run_mode>\n", mode_name(mode));
    return rpc.do_rpc(buf);
}

int RPC_CLIENT::get_run_mode(int& mode) {
    char buf[256];
    RPC rpc(this);
    int retval;

    retval = rpc.do_rpc("<get_run_mode/>\n");
    if (retval) return retval;

    mode = -1;
    while (rpc.fin.fgets(buf, 256)) {
        if (match_tag(buf, "</run_mode>")) break;
        if (match_tag(buf, mode_name(RUN_MODE_ALWAYS))) mode = RUN_MODE_ALWAYS;
        if (match_tag(buf, mode_name(RUN_MODE_NEVER))) mode = RUN_MODE_NEVER;
        if (match_tag(buf, mode_name(RUN_MODE_AUTO))) mode = RUN_MODE_AUTO;
    }
    return 0;
}

int RPC_CLIENT::set_network_mode(int mode) {
    char buf[256];
    RPC rpc(this);

    sprintf(buf,
        "<set_network_mode>\n"
        "%s"
        "</set_network_mode>\n",
        mode_name(mode)
    );
    return rpc.do_rpc(buf);
}

int RPC_CLIENT::get_network_mode(int& mode) {
    char buf[256];
    RPC rpc(this);
    int retval;

    retval = rpc.do_rpc("<get_network_mode/>\n");
    if (retval) return retval;

    mode = -1;
    while (rpc.fin.fgets(buf, 256)) {
        if (match_tag(buf, "</network_mode>")) break;
        if (match_tag(buf, mode_name(RUN_MODE_ALWAYS))) mode = RUN_MODE_ALWAYS;
        if (match_tag(buf, mode_name(RUN_MODE_NEVER))) mode = RUN_MODE_NEVER;
        if (match_tag(buf, mode_name(RUN_MODE_AUTO))) mode = RUN_MODE_AUTO;
    }
    return 0;
}

int RPC_CLIENT::get_screensaver_mode(int& status) {
    char buf[256];
    RPC rpc(this);
    int retval;

    retval = rpc.do_rpc("<get_screensaver_mode/>\n");
    if (retval) return retval;

    while (rpc.fin.fgets(buf, 256)) {
        if (match_tag(buf, "</screensaver_mode>")) break;
        else if (parse_int(buf, "<status>", status)) continue;
    }
    //BOINCTRACE(_T("Receiving: get_screensaver_mode\nstatus = %d\n"), status);
    return 0;
}

int RPC_CLIENT::set_screensaver_mode(bool enabled, const char* window_station, const char* desktop, double blank_time) {
    char buf[256];
    RPC rpc(this);

    sprintf(buf,
        "<set_screensaver_mode>\n"
        "     %s\n"
        "     <window_station>%s</window_station>\n"
        "     <desktop>%s</desktop>\n"
        "     <blank_time>%f</blank_time>\n"
        "</set_screensaver_mode>\n",
        enabled ? "<enabled/>" : "",
        window_station ? window_station : "", 
        desktop ? desktop : "", 
        blank_time
    );
    //BOINCTRACE(_T("Sending: set_screensaver_mode\n%s\n"), buf);
    return rpc.do_rpc(buf);
}

int RPC_CLIENT::run_benchmarks() {
    RPC rpc(this);
    return rpc.do_rpc("<run_benchmarks/>\n");
}

int RPC_CLIENT::set_proxy_settings(PROXY_INFO& pi) {
    char buf[1024];
    RPC rpc(this);

    sprintf(buf,
        "<set_proxy_settings>\n%s%s%s"
        "    <proxy_info>\n"
        "        <http_server_name>%s</http_server_name>\n"
        "        <http_server_port>%d</http_server_port>\n"
        "        <http_user_name>%s</http_user_name>\n"
        "        <http_user_passwd>%s</http_user_passwd>\n"
        "        <socks_server_name>%s</socks_server_name>\n"
        "        <socks_server_port>%d</socks_server_port>\n"
        "        <socks_version>%d</socks_version>\n"
        "        <socks5_user_name>%s</socks5_user_name>\n"
        "        <socks5_user_passwd>%s</socks5_user_passwd>\n"
        "    </proxy_info>\n"
        "</set_proxy_settings>\n",
        pi.use_http_proxy?"   <use_http_proxy/>\n":"",
        pi.use_socks_proxy?"   <use_socks_proxy/>\n":"",
        pi.use_http_authentication?"   <use_http_auth/>\n":"",
        pi.http_server_name.c_str(),
        pi.http_server_port,
        pi.http_user_name.c_str(),
        pi.http_user_passwd.c_str(),
        pi.socks_server_name.c_str(),
        pi.socks_server_port,
        pi.socks_version,
        pi.socks5_user_name.c_str(),
        pi.socks5_user_passwd.c_str()
    );
    return rpc.do_rpc(buf);
}

int RPC_CLIENT::get_proxy_settings(PROXY_INFO& p) {
    RPC rpc(this);
    int retval;

    retval = rpc.do_rpc("<get_proxy_settings/>");
    if (retval) return retval;

    return p.parse(rpc.fin);
}

int RPC_CLIENT::get_messages(int seqno, MESSAGES& msgs) {
    char buf[4096];
    RPC rpc(this);
    int retval;

    sprintf(buf,
        "<get_messages>\n"
        "  <seqno>%d</seqno>\n"
        "</get_messages>\n",
        seqno
    );
    retval = rpc.do_rpc(buf);
    if (retval) return retval;

    while (rpc.fin.fgets(buf, 256)) {
        if (match_tag(buf, "</msgs>")) break;
        else if (match_tag(buf, "<msg>")) {
            MESSAGE* message = new MESSAGE();
            message->parse(rpc.fin);
            msgs.messages.push_back(message);
            continue;
        }
    }

    return 0;
}

int RPC_CLIENT::file_transfer_op(FILE_TRANSFER& ft, char* op) {
    char buf[4096], *tag;
    RPC rpc(this);

    if (!strcmp(op, "retry")) {
        tag = "retry_file_transfer";
    } else if (!strcmp(op, "abort")) {
        tag = "abort_file_transfer";
    } else {
        return -1;
    }
    sprintf(buf,
        "<%s>\n"
        "   <project_url>%s</project_url>\n"
        "   <filename>%s</filename>\n"
        "</%s>\n",
        tag,
        ft.project_url.c_str(),
        ft.name.c_str(),
        tag
    );
    return rpc.do_rpc(buf);
}

int RPC_CLIENT::result_op(RESULT& result, char* op) {
    char buf[4096], *tag;
    RPC rpc(this);

    if (!strcmp(op, "abort")) {
        tag = "abort_result";
    } else if (!strcmp(op, "suspend")) {
        tag = "suspend_result";
    } else if (!strcmp(op, "resume")) {
        tag = "resume_result";
    } else {
        return -1;
    }
    sprintf(buf,
        "<%s>\n"
        "   <project_url>%s</project_url>\n"
        "   <name>%s</name>\n"
        "</%s>\n",
        tag,
        result.project_url.c_str(),
        result.name.c_str(),
        tag
    );
    return rpc.do_rpc(buf);
}

int RPC_CLIENT::get_host_info(HOST_INFO& h) {
    RPC rpc(this);
    int retval;

    retval = rpc.do_rpc("<get_host_info/>");
    if (retval) return retval;

    return h.parse(rpc.fin);
}


int RPC_CLIENT::quit() {
    RPC rpc(this);
    return rpc.do_rpc("<quit/>\n");
}

int RPC_CLIENT::acct_mgr_rpc(char* url, char* name, char* passwd) {
    char buf[4096];
    RPC rpc(this);
    sprintf(buf,
        "<acct_mgr_rpc>\n"
        "  <url>%s</url>\n"
        "  <name>%s</name>\n"
        "  <passwd>%s</passwd>\n",
        url, name, passwd
    );
    return rpc.do_rpc(buf);
}

const char *BOINC_RCSID_6802bead97 = "$Id$";
