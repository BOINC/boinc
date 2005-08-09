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

// This file is code to print (in ASCII) the stuff returned by GUI RPC.
// Used only by boinc_cmd.

#if defined(_WIN32) && !defined(__STDWX_H__) && !defined(_BOINC_WIN_) && !defined(_AFX_STDAFX_H_)
#include "boinc_win.h"
#endif

#ifdef _WIN32
#include "version.h"
#else
#include "config.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <cstdio>
#include <unistd.h>
#include <cstdlib>
#include <cstring>
#endif

#include "diagnostics.h"
#include "parse.h"
#include "util.h"
#include "error_numbers.h"
#include "miofile.h"
#include "md5_file.h"
#include "network.h"
#include "gui_rpc_client.h"

using std::string;
using std::vector;

void GUI_URL::print() {
    printf(
        "GUI URL:\n"
        "   name: %s\n"
        "   description: %s\n"
        "   URL: %s\n",
        name.c_str(), description.c_str(), url.c_str()
    );
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

void APP::print() {
    printf("   name: %s\n", name.c_str());
    printf("   Project: %s\n", project->project_name.c_str());
}

void APP_VERSION::print() {
    printf("   application: %s\n", app->name.c_str());
    printf("   version: %.2f\n", version_num/100.0);
    printf("   project: %s\n", project->project_name.c_str());
}

void WORKUNIT::print() {
    printf("   name: %s\n", name.c_str());
    printf("   FP estimate: %f\n", rsc_fpops_est);
    printf("   FP bound: %f\n", rsc_fpops_bound);
    printf("   memory bound: %f\n", rsc_memory_bound);
    printf("   disk bound: %f\n", rsc_disk_bound);
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

void FILE_TRANSFER::print() {
    printf("   name: %s\n", name.c_str());
    printf("   generated locally: %s\n", generated_locally?"yes":"no");
    printf("   uploaded: %s\n", uploaded?"yes":"no");
    printf("   upload when present: %s\n", upload_when_present?"yes":"no");
    printf("   sticky: %s\n", sticky?"yes":"no");
    printf("   generated locally: %s\n", generated_locally?"yes":"no");
}

void MESSAGE::print() {
    printf("%s %d %d %s\n",
        project.c_str(), priority, timestamp, body.c_str()
    );
}

void GR_PROXY_INFO::print() {      // anyone need this?
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

void PROJECTS::print() {
    unsigned int i;
    printf("======== Projects ========\n");
    for (i=0; i<projects.size(); i++) {
        printf("%d) -----------\n", i+1);
        projects[i]->print();
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
    printf("\n======== File transfers ========\n");
    for (i=0; i<file_transfers.size(); i++) {
        printf("%d) -----------\n", i+1);
        file_transfers[i]->print();
    }
}

void MESSAGES::print() {
    unsigned int i;
    printf("\n======== Messages ========\n");
    for (i=0; i<messages.size(); i++) {
        printf("%d) -----------\n", i+1);
        messages[i]->print();
    }
}

void PROJECT_CONFIG::print() {
    printf(
        "uses_username: %d\n"
        "name: %s\n"
        "min_passwd_length: %d\n",
        uses_username,
        name.c_str(),
        min_passwd_length
    );
}

void ACCOUNT_OUT::print() {
    printf(
        "error_num: %d\n"
        "authenticator: %s\n",
        error_num,
        authenticator.c_str()
    );
}
