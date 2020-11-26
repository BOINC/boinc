// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2019 University of California
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

#include <map>
#include <string>
#include <vector>

#include "diagnostics.h"
#include "error_numbers.h"
#include "md5_file.h"
#include "miofile.h"
#include "network.h"
#include "parse.h"
#include "str_util.h"
#include "util.h"
#include "pretty_printer.h"

#include "gui_rpc_client.h"

using std::string;
using std::map;
using std::pair;
using std::vector;

// Helper function to generate strings with a given format
template<typename T>
string format(const string& format_string, T arguments) {
    const auto size = snprintf(nullptr, 0, format_string.c_str(), arguments) + 1;
    auto buf = vector<char>(size);
    sprintf(buf.data(), format_string.c_str(), arguments);

    return buf.data();
}

// Helper function to print groups of structs
template<typename T>
void print_group(const string& group_name, vector<T>& items, const bool& console_print, pretty_printer& printer, const bool& print_printer = true) {
    if (console_print) {
        printf("\n======== %s ========\n", group_name.c_str());
        for (unsigned int i = 0; i < items.size(); i++) {
            pretty_printer item_printer = items.at(i)->get();
            item_printer.change_spacing(1);

            printf("%d) -----------\n", i + 1);
            printf("%s\n", item_printer.prettify(true).c_str());
        }

    } else {
        if (items.empty()) return;
        vector<pretty_printer> printers;
        printers.reserve(items.size());
        for (const auto item : items) {
            printers.push_back(item->get());
        }

        printer.insert(group_name, printers);
        if (print_printer) {
            printf("%s\n", printer.prettify(console_print).c_str());
        }
    }
}

void DAILY_XFER_HISTORY::print(const bool& console_print) {
    if (daily_xfers.empty()) {
        printf("\n");
        return;
    }

    pretty_printer printer(0);
    for (const auto& xfer : daily_xfers) {
        char buf[256];
        time_t t = xfer.when * 86400;
        struct tm* tm = localtime(&t);
        strftime(buf, sizeof(buf) - 1, "%d-%b-%Y", tm);

        map<string, int> info;
        info["bytes uploaded"] = xfer.up;
        info["bytes downloaded"] = xfer.down;;
        printer.insert(buf, info);
    }

    printf("%s\n", printer.prettify(console_print).c_str());
}

pair<string, map<string, string>> GUI_URL::get() {
    map<string, string> result;
    result["description"] = description;
    result["URL"] = url;

    return pair<string, map<string, string>>(name, result);
}

pretty_printer PROJECT::get_disk_usage() {
    pretty_printer result;
    result.insert("master URL", master_url);
    result.insert("disk usage", format("%.2fMB", disk_usage / MEGA));
    return result;
}

pretty_printer PROJECT::get() {
    pretty_printer result;

    result.insert("name", project_name.c_str());
    result.insert("master URL", master_url);
    result.insert("user_name", user_name.c_str());
    result.insert("team_name", team_name.c_str());
    result.insert("resource share", resource_share);
    result.insert("user_total_credit", user_total_credit);
    result.insert("user_expavg_credit", user_expavg_credit);
    result.insert("host_expavg_credit", host_total_credit);
    result.insert("nrpc_failures", nrpc_failures);
    result.insert("master_fetch_failures", master_fetch_failures);
    result.insert("master fetch pending", master_url_fetch_pending);
    result.insert("scheduler RPC pending", sched_rpc_pending ? true : false);
    result.insert("trickle upload pending", trickle_up_pending);
    result.insert("attached via Account Manager", attached_via_acct_mgr);
    result.insert("ended", ended);
    result.insert("suspended via GUI", suspended_via_gui);
    result.insert("don't request more work", dont_request_more_work);

    result.insert("disk usage", format("%.2fMB", disk_usage / MEGA));

    auto const foo = (time_t)last_rpc_time;
    auto time_string = string(ctime(&foo));
    if (time_string.back() == '\n') {
        time_string.pop_back();
    }
    result.insert("last RPC", time_string);
    result.insert("project files downloaded", project_files_downloaded_time);

    map<string, map<string, string>> urls;
    for (auto url: gui_urls) {
        urls[url.get().first] = url.get().second;
    }
    result.insert("GUI URLs", urls);

    result.insert("jobs succeeded", njobs_success);
    result.insert("jobs failed", njobs_error);
    result.insert("elapsed time", elapsed_time);
    result.insert("cross-project ID", external_cpid);

    return result;
}

pretty_printer APP::get() const {
    pretty_printer result;

    result.insert("name", name);
    result.insert("Project", project->project_name.c_str());

    return result;
}

pretty_printer APP_VERSION::get() const {
    pretty_printer result(3);

    result.insert("project", project->project_name.c_str());
    result.insert("application", app->name);
    result.insert("platform", platform);
    if (strlen(plan_class)) {
        result.insert("plan class", plan_class);
    }
    result.insert("version", version_num / 100.0);
    if (avg_ncpus != 1) {
        result.insert("avg #CPUS", avg_ncpus);
    }
    if (gpu_type != PROC_TYPE_CPU) {
        result.insert("coprocessor type", proc_type_name(gpu_type));
        result.insert("coprocessor usage", gpu_usage);
    }
    result.change_decimal(2);
    result.insert("estimated GFLOPS", flops/1e9);
    result.insert("filename", exec_filename);

    return result;
}

pretty_printer WORKUNIT::get() {
    pretty_printer result;

    result.insert("name", name);
    result.insert("FP estimate", format("%e", rsc_fpops_est));
    result.insert("FP bound", format("%e", rsc_fpops_bound));
    result.insert("memory bound", format("%.2f MB", rsc_memory_bound / MEGA));
    result.insert("disk bound", format("%.2f MB", rsc_disk_bound / MEGA));
    if (!job_keywords.empty()) {
        vector<string> keywords;
        for (const auto& i : job_keywords.keywords) {
            keywords.emplace_back(i.name);
        }
        result.insert("keywords", keywords);
    }

    return result;
}

pretty_printer RESULT::get() {
    pretty_printer result;

    result.insert("name", name);
    result.insert("WU name", wu_name);
    result.insert("project URL", project_url);

    time_t foo = (time_t) received_time;
    auto time_string = string(ctime(&foo));
    if (time_string.back() == '\n') {
        time_string.pop_back();
    }
    result.insert("received", time_string);

    foo = (time_t)report_deadline;
    time_string = string(ctime(&foo));
    if (time_string.back() == '\n') {
        time_string.pop_back();
    }
    result.insert("report deadline", time_string);

    result.insert("ready to report", ready_to_report);
    result.insert("state", result_client_state_string(state));
    result.insert("scheduler state", result_scheduler_state_string(scheduler_state));
    result.insert("active_task_state", active_task_state_string(active_task_state));
    //result.insert("stderr_out", stderr_out.c_str());
    result.insert("app version num", version_num);
    result.insert("resources", strlen(resources) ? resources : "1 CPU");

    // stuff for jobs that are not yet completed
    //
    if (state <= RESULT_FILES_DOWNLOADED) {
        if (suspended_via_gui) {
            result.insert("suspended via GUI", true);
        }
        result.insert("estimated CPU time remaining", estimated_cpu_time_remaining);
    }

    // stuff for jobs that are running or have run
    //
    if (scheduler_state > CPU_SCHED_UNINITIALIZED) {
        result.insert("slot", slot);
        result.insert("PID", pid);
        result.insert("CPU time at last checkpoint", checkpoint_cpu_time);
        result.insert("current CPU time", current_cpu_time);
        result.insert("fraction done", fraction_done);
        result.insert("swap size", format("%.0f MB", swap_size / MEGA));
        result.insert("working set size", format("%.0f MB", working_set_size_smoothed / MEGA));
        if (bytes_sent || bytes_received) {
            result.change_decimal(0);
            result.insert("bytes sent", bytes_sent);
            result.insert("bytes received", bytes_received);
        }
    }

    // stuff for completed jobs
    //
    if (state > RESULT_FILES_DOWNLOADED) {
        result.insert("final CPU time", final_cpu_time);
        result.insert("final elapsed time", final_elapsed_time);
        result.insert("exit_status", exit_status);
        result.insert("signal", signal);
    }

    return result;
}

pretty_printer FILE_TRANSFER::get() {
    pretty_printer result;

    result.insert("name", name.c_str());
    result.insert("direction", is_upload?"upload":"download");
    result.insert("sticky", sticky);
    result.insert("xfer active", xfer_active);
    result.insert("time_so_far", time_so_far);
    result.insert("bytes_xferred", bytes_xferred);
    result.insert("xfer_speed", xfer_speed);

    return result;
}

pretty_printer MESSAGE::get() const {
    pretty_printer result;
    result.insert("Project", project.c_str());
    result.insert("Priority", priority);
    result.insert("Timestamp", timestamp);
    result.insert("Body", body.c_str());

    return result;
}

void GR_PROXY_INFO::print(const bool& console_print) {
    pretty_printer result;

    result.insert("HTTP server name",this->http_server_name.c_str());
    result.insert("HTTP server port",this->http_server_port);
    result.insert("HTTP user name",this->http_user_name.c_str());
    //result.insert("HTTP user password",this->http_user_passwd.c_str());
    result.insert("SOCKS server name",this->socks_server_name.c_str());
    result.insert("SOCKS server port",this->socks_server_port);
    result.insert("SOCKS5 user name",this->socks5_user_name.c_str());
    //result.insert("SOCKS5 user password",this->socks5_user_passwd.c_str());
    result.insert("no proxy hosts",this->noproxy_hosts.c_str());

    printf("%s\n", result.prettify(console_print).c_str());
}

void HOST_INFO::print(const bool& console_print) {
    pretty_printer result;

    result.insert("timezone", timezone);
    result.insert("domain name", domain_name);
    result.insert("IP addr", ip_addr);
    result.insert("#CPUS", p_ncpus);
    result.insert("CPU vendor", p_vendor);
    result.insert("CPU model", p_model);
    result.insert("CPU FP OPS", p_fpops);
    result.insert("CPU int OPS", p_iops);
    //result.insert("CPU mem BW", p_membw);
    result.insert("OS name", os_name);
    result.insert("OS version", os_version);
    result.insert("mem size", m_nbytes);
    result.insert("cache size", m_cache);
    result.insert("swap size", m_swap);
    result.insert("disk size", d_total);
    result.insert("disk free", d_free);

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
        result.insert("NVIDIA GPU", buf);
        if (cn.count > 1) {
            result.insert("Count", cn.count);
        }
        if (cn.have_opencl) {
            cn.opencl_prop.is_used = COPROC_USED;
            cn.opencl_prop.peak_flops = cn.peak_flops;
            cn.opencl_prop.opencl_available_ram = cn.available_ram;
            cn.opencl_prop.description(buf, sizeof(buf), "NVIDIA");
            const string& buf_string = buf;
            result.insert("OpenCL", buf_string.substr(8, buf_string.size()));
        }
    }
    COPROC_ATI &ca = coprocs.ati;
    if (ca.count) {
        ca.description(buf, sizeof(buf));
        result.insert("AMD GPU", buf);
        if (ca.count > 1) {
            result.insert("Count", ca.count);
        }
        if (ca.have_opencl) {
            ca.opencl_prop.peak_flops = ca.peak_flops;
            ca.opencl_prop.opencl_available_ram = ca.available_ram;
            ca.opencl_prop.is_used = COPROC_USED;
            ca.opencl_prop.description(buf, sizeof(buf), "AMD");
            const string& buf_string = buf;
            result.insert("OpenCL", buf_string.substr(8, buf_string.size()));
        }
    }
    COPROC_INTEL &ci = coprocs.intel_gpu;
    if (ci.count) {
        result.insert("Intel GPU", buf);
        if (ci.count > 1) {
            result.insert("Count", ci.count);
        }
        if (ci.have_opencl) {
            ci.opencl_prop.peak_flops = ci.peak_flops;
            ci.opencl_prop.opencl_available_ram = ci.opencl_prop.global_mem_size;
            ci.opencl_prop.is_used = COPROC_USED;
            ci.opencl_prop.description(buf, sizeof(buf), "Intel GPU");
            const string& buf_string = buf;
            result.insert("OpenCL", buf_string.substr(8, buf_string.size()));
        }
    }

    if (console_print) {
        result.change_spacing(1);
    }
    printf("%s\n", result.prettify(console_print).c_str());
}

void SIMPLE_GUI_INFO::print(const bool& console_print) {
    pretty_printer printer;
    print_group("Projects", projects, console_print, printer, false);
    print_group("Tasks", results, console_print, printer);
}

pretty_printer TIME_STATS::get() {
    pretty_printer result;
    result.insert("now", now);
    result.insert("on_frac", on_frac);
    result.insert("connected_frac", connected_frac);
    result.insert("cpu_and_network_available_frac", cpu_and_network_available_frac);
    result.insert("active_frac", active_frac);
    result.insert("gpu_active_frac", gpu_active_frac);

    time_t foo = (time_t)client_start_time;
    auto time_string = string(ctime(&foo));
    if (time_string.back() == '\n') {
        time_string.pop_back();
    }
    result.insert("client_start_time", time_string);

    result.insert("previous_uptime", previous_uptime);
    result.insert("session_active_duration", session_active_duration);
    result.insert("session_gpu_active_duration", session_gpu_active_duration);

    foo = (time_t)total_start_time;
    time_string = string(ctime(&foo));
    if (time_string.back() == '\n') {
        time_string.pop_back();
    }
    result.insert("total_start_time", time_string);

    result.insert("total_duration", total_duration);
    result.insert("total_active_duration", total_active_duration);
    result.insert("total_gpu_active_duration", total_gpu_active_duration);

    return result;
}

void CC_STATE::print(const bool& console_print) {
    pretty_printer status_printer;

    print_group("Projects", projects, console_print, status_printer, false);
    print_group("Applications", apps, console_print, status_printer, false);
    print_group("Application versions", app_versions, console_print, status_printer, false);
    print_group("Workunits", wus, console_print, status_printer, false);
    print_group("Tasks", results, console_print, status_printer, false);

    if (console_print) {
        printf("\n======== Time stats ========\n");
        auto time_stats_printer = time_stats.get();
        time_stats_printer.change_spacing(1);

        printf("%s\n", time_stats_printer.prettify(true).c_str());
    } else {
        status_printer.insert("Time stats", time_stats.get());
        printf("%s\n", status_printer.prettify().c_str());
    }
}

pretty_printer print_status (int reason, int mode, int mode_perm, double delay) {
    pretty_printer printer;

    if (reason) {
        printer.insert("status", "suspended");
        printer.insert("reason", suspend_reason_string(reason));
    } else {
        printer.insert("status", "not suspended");
    }
    printer.insert("current mode", run_mode_string(mode));
    printer.insert("perm mode", run_mode_string(mode_perm));
    printer.insert("perm becomes current in", format("%.0f sec", delay));

    return printer;
}

void CC_STATUS::print(const bool& console_print) const {
    pretty_printer result;
    result.insert("network connection status", network_status_string(network_status));

    result.insert("CPU Status", print_status(
        task_suspend_reason,
        task_mode,
        task_mode_perm,
        task_mode_delay
    ));
    result.insert("GPU Status", print_status(
        gpu_suspend_reason,
        gpu_mode,
        gpu_mode_perm,
        gpu_mode_delay
    ));
    result.insert("Network Status", print_status(
        network_suspend_reason,
        network_mode,
        network_mode_perm,
        network_mode_delay
    ));

    printf("%s\n", result.prettify(console_print).c_str());
}

void PROJECTS::print(const bool& console_print) {
    pretty_printer printer;
    print_group("Projects", projects, console_print, printer);
}

void PROJECTS::print_urls(const bool& console_print) {
    pretty_printer printer;
    for (const auto& project : projects) {
        printer.insert(project->project_name, project->master_url);
    }
    printf("%s\n", printer.prettify(console_print).c_str());
}

void DISK_USAGE::print(const bool& console_print) {
    if (console_print) {
        printf("======== Disk usage ========\n");
        printf("total: %.2fMB\n", d_total / MEGA);
        printf("free: %.2fMB\n", d_free / MEGA);

        for (unsigned int i = 0; i < projects.size(); i++) {
            printf("%d) -----------\n", i + 1);
            printf("%s\n", projects.at(i)->get_disk_usage().prettify(true).c_str());
        }

    } else {
        pretty_printer printer;

        map<string, string> disk_usage;
        disk_usage["total"] = format("%.2fMB", d_total / MEGA);
        disk_usage["free"] = format("%.2fMB", d_free / MEGA);
        printer.insert("Disk usage", disk_usage);

        vector<pretty_printer> printers;
        for (const auto& project : projects) {
            printers.push_back(project->get_disk_usage());
        }
        printer.insert("Projects", printers);

        printf("%s\n", printer.prettify(console_print).c_str());
    }
}

void RESULTS::print(const bool& console_print) {
    pretty_printer printer;
    print_group("Tasks", results, console_print, printer);
}

void FILE_TRANSFERS::print(const bool& console_print) {
    pretty_printer printer;
    print_group("File transfers", file_transfers, console_print, printer);
}

void MESSAGES::print(const bool& console_print) const {
    vector<MESSAGE*> message_vector;
    for (const auto& message : messages) {
        message_vector.push_back(message);
    }

    pretty_printer printer;
    print_group("Messages", message_vector, console_print, printer);
}

void PROJECT_CONFIG::print(const bool& console_print) const {
    pretty_printer printer;
    printer.insert("use_username", uses_username);
    printer.insert("name", name.c_str());
    printer.insert("min_passwd_length", min_passwd_length);

    printf("%s\n", printer.prettify(console_print).c_str());
}

void ACCOUNT_OUT::print(const bool& console_print) const {
    pretty_printer printer;
    if (error_num) {
        printer.insert("error in account lookup", boincerror(error_num));
    } else {
        printer.insert("account key", authenticator.c_str());
    }

    printf("%s\n", printer.prettify(console_print).c_str());
}

pair<string, pretty_printer> OLD_RESULT::get() {
    pretty_printer printer;
    printer.insert("project_url", project_url);
    printer.insert("app name", app_name);
    printer.insert("exit status", exit_status);
    printer.insert("elapsed time", elapsed_time);
    printer.insert("completed time", time_to_string(completed_time));
    printer.insert("reported time", time_to_string(create_time));

    return pair<string, pretty_printer>(result_name, printer);
}

void ACCT_MGR_INFO::print(const bool& console_print) const {
    map<string, string> info;
    info["Name"] = acct_mgr_name;
    info["URL"] = acct_mgr_url;

    pretty_printer printer;
    printer.insert("Account manager info", info);
    printf("%s\n", printer.prettify(console_print).c_str());
}
