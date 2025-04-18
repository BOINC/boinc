// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2022 University of California
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

// This file is code to print (in ASCII) the stuff returned by GUI RPC.
// Used only by boinccmd.

#if defined(_WIN32)
#include "boinc_win.h"
#include "../version.h"
#else
#include "config.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <cstdio>
#include <unistd.h>
#include <time.h>
#include <cstdlib>
#include <cstring>
#endif

#include "diagnostics.h"
#include "error_numbers.h"
#include "md5_file.h"
#include "miofile.h"
#include "network.h"
#include "parse.h"
#include "str_util.h"
#include "util.h"

#include "gui_rpc_client.h"

using std::string;
using std::vector;

void DAILY_XFER_HISTORY::print() {
    for (unsigned int i=0; i<daily_xfers.size(); i++) {
        DAILY_XFER& dx = daily_xfers[i];
        char buf[256];
        time_t t = dx.when*86400;
        struct tm* tm = localtime(&t);
        strftime(buf, sizeof(buf)-1, "%d-%b-%Y", tm);
        printf("%s: %.0f bytes uploaded, %.0f bytes downloaded\n",
            buf, dx.up, dx.down
        );
    }
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

void PROJECT::print_disk_usage() {
    printf("   master URL: %s\n", master_url);
    printf("   disk usage: %.2fGB\n", disk_usage/GIGA);
}

void PROJECT::print() {
    unsigned int i;

    printf("   name: %s\n", project_name.c_str());
    printf("   master URL: %s\n", master_url);
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
    printf("   trickle upload pending: %s\n", trickle_up_pending?"yes":"no");
    printf("   attached via Account Manager: %s\n", attached_via_acct_mgr?"yes":"no");
    printf("   ended: %s\n", ended?"yes":"no");
    printf("   suspended via GUI: %s\n", suspended_via_gui?"yes":"no");
    printf("   don't request more work: %s\n", dont_request_more_work?"yes":"no");
    printf("   disk usage: %.2fGB\n", disk_usage/GIGA);
    time_t foo = (time_t)last_rpc_time;
    printf("   last RPC: %s\n", ctime(&foo));
    printf("   project files downloaded: %f\n", project_files_downloaded_time);
    for (i=0; i<gui_urls.size(); i++) {
        gui_urls[i].print();
    }
    printf("   jobs succeeded: %d\n", njobs_success);
    printf("   jobs failed: %d\n", njobs_error);
    printf("   elapsed time: %f\n", elapsed_time);
    printf("   cross-project ID: %s\n", external_cpid);
}

void APP::print() {
    printf("   name: %s\n", name);
    printf("   Project: %s\n", project->project_name.c_str());
}

void APP_VERSION::print() {
    printf("   project: %s\n", project->project_name.c_str());
    printf("   application: %s\n", app->name);
    printf("   platform: %s\n", platform);
    if (strlen(plan_class)) {
        printf("   plan class: %s\n", plan_class);
    }
    printf("   version: %.2f\n", version_num/100.0);
    if (avg_ncpus != 1) {
        printf("   avg #CPUS: %.3f\n", avg_ncpus);
    }
    if (gpu_type != PROC_TYPE_CPU) {
        printf("   coprocessor type: %s\n", proc_type_name(gpu_type));
        printf("   coprocessor usage: %.3f\n", gpu_usage);
    }
    printf("   estimated GFLOPS: %.2f\n", flops/1e9);
    printf("   filename: %s\n", exec_filename);
}

void WORKUNIT::print() {
    printf("   project: %s\n", project->project_name.c_str());
    printf("   name: %s\n", name);
    printf("   FP estimate: %e\n", rsc_fpops_est);
    printf("   FP bound: %e\n", rsc_fpops_bound);
    printf("   memory bound: %.2f MB\n", rsc_memory_bound/MEGA);
    printf("   disk bound: %.2f MB\n", rsc_disk_bound/MEGA);
    if (!job_keywords.empty()) {
        printf("   keywords:\n");
        for (unsigned int i=0; i<job_keywords.keywords.size(); i++) {
            KEYWORD &kw = job_keywords.keywords[i];
            printf("      %s\n", kw.name.c_str());
        }
    }
}

void RESULT::print() {
    printf("   name: %s\n", name);
    printf("   WU name: %s\n", wu_name);
    if (project) {
        printf("   project: %s\n", project->project_name.c_str());
    } else {
        printf("   project URL: %s\n", project_url);
    }
    time_t foo = (time_t)received_time;
    printf("   received: %s", ctime(&foo));
    foo = (time_t)report_deadline;
    printf("   report deadline: %s", ctime(&foo));
    printf("   ready to report: %s\n", ready_to_report?"yes":"no");
    printf("   state: %s\n", result_client_state_string(state));
    printf("   scheduler state: %s\n", result_scheduler_state_string(scheduler_state));
    printf("   active_task_state: %s\n", active_task_state_string(active_task_state));
    //printf("   stderr_out: %s\n", stderr_out.c_str());
    printf("   app version num: %d\n", version_num);
    printf("   resources: %s\n", strlen(resources)?resources:"1 CPU");

    // stuff for jobs that are not yet completed
    //
    if (state <= RESULT_FILES_DOWNLOADED) {
        if (suspended_via_gui) {
            printf("   suspended via GUI: yes\n");
        }
        printf("   estimated CPU time remaining: %f\n", estimated_cpu_time_remaining);
        printf("   elapsed task time: %f\n", elapsed_time);
    }

    // stuff for jobs that are running or have run
    //
    if (scheduler_state > CPU_SCHED_UNINITIALIZED) {
        printf("   slot: %d\n", slot);
        printf("   PID: %d\n", pid);
        printf("   CPU time at last checkpoint: %f\n", checkpoint_cpu_time);
        printf("   current CPU time: %f\n", current_cpu_time);
        printf("   fraction done: %f\n", fraction_done);
        printf("   swap size: %.0f MB\n", swap_size/MEGA);
        printf("   working set size: %.0f MB\n", working_set_size_smoothed/MEGA);
        if (bytes_sent || bytes_received) {
            printf("   bytes sent: %.0f received: %.0f\n",
                bytes_sent, bytes_received
            );
        }
    }

    // stuff for completed jobs
    //
    if (state > RESULT_FILES_DOWNLOADED) {
        printf("   final CPU time: %f\n", final_cpu_time);
        printf("   final elapsed time: %f\n", final_elapsed_time);
        printf("   exit_status: %d\n", exit_status);
        printf("   signal: %d\n", signal);
    }
}

void FILE_TRANSFER::print() {
    printf("   name: %s\n", name.c_str());
    printf("   direction: %s\n", is_upload?"upload":"download");
    printf("   sticky: %s\n", sticky?"yes":"no");
    printf("   xfer active: %s\n", xfer_active?"yes":"no");
    printf("   time_so_far: %f\n", time_so_far);
    if (xfer_active) printf("   estimated_xfer_time_remaining: %f\n", estimated_xfer_time_remaining);
    printf("   bytes_xferred: %f\n", bytes_xferred);
    printf("   xfer_speed: %f\n", xfer_speed);
}

void MESSAGE::print() {
    printf("%s %d %d %s\n",
        project.c_str(), priority, timestamp, body.c_str()
    );
}

void GR_PROXY_INFO::print() {
    printf("HTTP server name: %s\n",this->http_server_name.c_str());
    printf("HTTP server port: %d\n",this->http_server_port);
    printf("HTTP user name: %s\n",this->http_user_name.c_str());
    //printf("HTTP user password: %s\n",this->http_user_passwd.c_str());
    printf("SOCKS server name: %s\n",this->socks_server_name.c_str());
    printf("SOCKS server port: %d\n",this->socks_server_port);
    printf("SOCKS5 user name: %s\n",this->socks5_user_name.c_str());
    //printf("SOCKS5 user password: %s\n",this->socks5_user_passwd.c_str());
    printf("no proxy hosts: %s\n",this->noproxy_hosts.c_str());
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
    //printf("  CPU mem BW: %f\n", p_membw);
    printf("  OS name: %s\n", os_name);
    printf("  OS version: %s\n", os_version);
    printf("  mem size: %f\n", m_nbytes);
    printf("  cache size: %f\n", m_cache);
    printf("  swap size: %f\n", m_swap);
    printf("  disk size: %f\n", d_total);
    printf("  disk free: %f\n", d_free);

    // Show GPU info.
    // This is harder than it should be,
    // because the structures aren't populated like they were
    // at GPU detection time.
    // Would be better for the client to export the description strings.
    //
    char buf[256];
    COPROC_NVIDIA& cn = coprocs.nvidia;
    if (cn.count) {
        cn.description(buf, sizeof(buf));
        printf("  NVIDIA GPU: %s\n", buf);
        if (cn.count > 1) {
            printf("    Count: %d\n", cn.count);
        }
        if (cn.have_opencl) {
            cn.opencl_prop.is_used = COPROC_USED;
            cn.opencl_prop.peak_flops = cn.peak_flops;
            cn.opencl_prop.opencl_available_ram = cn.available_ram;
            cn.opencl_prop.description(buf, sizeof(buf), "NVIDIA");
            printf("    %s\n", buf);
        }
    }
    COPROC_ATI &ca = coprocs.ati;
    if (ca.count) {
        ca.description(buf, sizeof(buf));
        printf("  AMD GPU: %s\n", buf);
        if (ca.count > 1) {
            printf("    Count: %d\n", ca.count);
        }
        if (ca.have_opencl) {
            ca.opencl_prop.peak_flops = ca.peak_flops;
            ca.opencl_prop.opencl_available_ram = ca.available_ram;
            ca.opencl_prop.is_used = COPROC_USED;
            ca.opencl_prop.description(buf, sizeof(buf), "AMD");
            printf("    %s\n", buf);
        }
    }
    COPROC_INTEL &ci = coprocs.intel_gpu;
    if (ci.count) {
        printf("  Intel GPU\n");
        if (ci.count > 1) {
            printf("    Count: %d\n", ci.count);
        }
        if (ci.have_opencl) {
            ci.opencl_prop.peak_flops = ci.peak_flops;
            ci.opencl_prop.opencl_available_ram = (double)ci.opencl_prop.global_mem_size;
            ci.opencl_prop.is_used = COPROC_USED;
            ci.opencl_prop.description(buf, sizeof(buf), "Intel GPU");
            printf("    %s\n", buf);
        }
    }
    COPROC_APPLE &cap = coprocs.apple_gpu;
    if (cap.count) {
        printf("  Apple GPU\n");
        if (cap.count > 1) {
            printf("    Count: %d\n", cap.count);
        }
        if (cap.have_opencl) {
            cap.opencl_prop.peak_flops = cap.peak_flops;
            cap.opencl_prop.opencl_available_ram = (double)cap.opencl_prop.global_mem_size;
            cap.opencl_prop.is_used = COPROC_USED;
            cap.opencl_prop.description(buf, sizeof(buf), "Apple GPU");
            printf("    %s\n", buf);
        }
    }
}

void SIMPLE_GUI_INFO::print() {
    unsigned int i;
    printf("======== Projects ========\n");
    for (i=0; i<projects.size(); i++) {
        printf("%d) -----------\n", i+1);
        projects[i]->print();
    }
    printf("\n======== Tasks ========\n");
    for (i=0; i<results.size(); i++) {
        printf("%d) -----------\n", i+1);
        results[i]->print();
    }
}

void TIME_STATS::print() {
    printf("  now: %f\n", now);
    printf("  on_frac: %f\n", on_frac);
    printf("  connected_frac: %f\n", connected_frac);
    printf("  cpu_and_network_available_frac: %f\n", cpu_and_network_available_frac);
    printf("  active_frac: %f\n", active_frac);
    printf("  gpu_active_frac: %f\n", gpu_active_frac);
    time_t foo = (time_t)client_start_time;
    printf("  client_start_time: %s\n", ctime(&foo));
    printf("  previous_uptime: %f\n", previous_uptime);
    printf("  session_active_duration: %f\n", session_active_duration);
    printf("  session_gpu_active_duration: %f\n", session_gpu_active_duration);
    foo = (time_t)total_start_time;
    printf("  total_start_time: %s\n", ctime(&foo));
    printf("  total_duration: %f\n", total_duration);
    printf("  total_active_duration: %f\n", total_active_duration);
    printf("  total_gpu_active_duration: %f\n", total_gpu_active_duration);
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
    printf("\n======== Tasks ========\n");
    for (i=0; i<results.size(); i++) {
        printf("%d) -----------\n", i+1);
        results[i]->print();
    }
    printf("\n======== Time stats ========\n");
    time_stats.print();
}

void print_status(
    const char* name, int reason, int mode, int mode_perm, double delay
) {
    printf("%s status\n", name);
    if (reason) {
        printf("    suspended: %s\n", suspend_reason_string(reason));
    } else {
        printf("    not suspended\n");
    }
    printf(
        "    current mode: %s\n"
        "    perm mode: %s\n"
        "    perm becomes current in %.0f sec\n",
        run_mode_string(mode),
        run_mode_string(mode_perm),
        delay
    );
}

void CC_STATUS::print() {
    printf("network connection status: %s\n",
        network_status_string(network_status)
    );
    print_status("CPU",
        task_suspend_reason,
        task_mode,
        task_mode_perm,
        task_mode_delay
    );
    print_status("GPU",
        gpu_suspend_reason,
        gpu_mode,
        gpu_mode_perm,
        gpu_mode_delay
    );
    print_status("Network",
        network_suspend_reason,
        network_mode,
        network_mode_perm,
        network_mode_delay
    );
}

void PROJECTS::print() {
    unsigned int i;
    printf("======== Projects ========\n");
    for (i=0; i<projects.size(); i++) {
        printf("%d) -----------\n", i+1);
        projects[i]->print();
    }
}

void PROJECTS::print_urls() {
    unsigned int i;
    for (i=0; i<projects.size(); i++) {
        printf("%s\n", projects[i]->master_url);
    }
}

void DISK_USAGE::print() {
    unsigned int i;
    printf("======== Disk usage ========\n");
    printf("total: %.2fGB\n", d_total/GIGA);
    printf("free: %.2fGB\n", d_free/GIGA);
    for (i=0; i<projects.size(); i++) {
        printf("%d) -----------\n", i+1);
        projects[i]->print_disk_usage();
    }
}

void RESULTS::print() {
    unsigned int i;
    printf("\n======== Tasks ========\n");
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
    if (error_num) {
        printf("error in account lookup: %s\n", boincerror(error_num));
    } else {
        printf("account key: %s\n", authenticator.c_str());
    }
}

void OLD_RESULT::print() {
    printf(
        "task %s:\n"
        "   project URL: %s\n"
        "   app name: %s\n"
        "   exit status: %d\n"
        "   elapsed time: %f sec\n"
        "   task completed: %s\n"
        "   acked by project: %s\n",
        result_name,
        project_url,
        app_name,
        exit_status,
        elapsed_time,
        time_to_string(completed_time),
        time_to_string(create_time)
    );
}

void ACCT_MGR_INFO::print() {
    printf(
        "Account manager info:\n"
        "   Name: %s\n"
        "   URL: %s\n",
        acct_mgr_name.c_str(),
        acct_mgr_url.c_str()
    );
}
