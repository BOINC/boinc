// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2023 University of California
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

// Logic related to general (also known as global) preferences:
// when to compute, how much disk to use, etc.
//

#include "cpp.h"

#ifdef _WIN32
#include "boinc_win.h"
#else
#include "config.h"
#if HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif
#if HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#endif

#include "common_defs.h"
#include "filesys.h"
#include "parse.h"
#include "str_util.h"
#include "str_replace.h"
#include "util.h"

#include "client_msgs.h"
#include "client_state.h"
#include "cpu_benchmark.h"
#include "file_names.h"
#include "project.h"

using std::min;
using std::max;
using std::string;

#define MAX_PROJ_PREFS_LEN  65536
    // max length of project-specific prefs

// Return the maximum allowed disk usage by projects as determined by user preferences.
// There are three different settings in the prefs;
// return the least of the three.
//
double CLIENT_STATE::allowed_disk_usage(double boinc_total) {
    double limit = boinc_total + host_info.d_free - global_prefs.disk_min_free_gb*GIGA;
    if (global_prefs.disk_max_used_pct) {
        double limit_pct = host_info.d_total*global_prefs.disk_max_used_pct/100.0;
        limit = min(limit, limit_pct);
    }

    if (global_prefs.disk_max_used_gb) {
        double limit_abs = global_prefs.disk_max_used_gb*(GIGA);
        limit = min(limit, limit_abs);
    }
    return max(limit, 0.);
}

#ifndef SIM

// populate:
// PROJECT::disk_usage for all projects
// GLOBAL_STATE::client_disk_usage
// GLOBAL_STATE::total_disk_usage
//
int CLIENT_STATE::get_disk_usages() {
    unsigned int i;
    double size;
    PROJECT* p;
    int retval;
    char buf[MAXPATHLEN];

    client_disk_usage = 0;
    total_disk_usage = 0;
    for (i=0; i<projects.size(); i++) {
        p = projects[i];
        p->disk_usage = 0;
        retval = dir_size_alloc(p->project_dir(), size);
        if (!retval) p->disk_usage = size;
    }

    for (i=0; i<active_tasks.active_tasks.size(); i++) {
        ACTIVE_TASK* atp = active_tasks.active_tasks[i];
        get_slot_dir(atp->slot, buf, sizeof(buf));
        retval = dir_size_alloc(buf, size);
        if (retval) continue;
        atp->wup->project->disk_usage += size;
    }
    for (i=0; i<projects.size(); i++) {
        p = projects[i];
        total_disk_usage += p->disk_usage;
    }
    retval = dir_size_alloc(".", size, false);
    if (!retval) {
        client_disk_usage = size;
        total_disk_usage += size;
    }
    return 0;
}

// populate PROJECT::disk_share for all projects,
// i.e. the max space we should allocate to the project.
// This is calculated as follows:
// - each project has a "disk_resource_share" (DRS)
//   This is the resource share plus .1*(max resource share).
//   This ensures that backup projects get some disk.
// - each project has a "desired_disk_usage (DDU)",
//   which is either its current usage
//   or an amount sent from the scheduler.
// - each project has a "quota": (available space)*(drs/total_drs).
// - a project is "greedy" if DDU > quota.
// - if a project is non-greedy, share = quota
// - X = available space - space used by non-greedy projects
// - if a project is greedy, share = quota
//   + X*drs/(total drs of greedy projects)
//
void CLIENT_STATE::get_disk_shares() {
    PROJECT* p;
    unsigned int i;

    // compute disk resource shares
    //
    double trs = 0;
    double max_rs = 0;
    for (i=0; i<projects.size(); i++) {
        p = projects[i];
        p->ddu = std::max(p->disk_usage, p->desired_disk_usage);
        double rs = p->resource_share;
        trs += rs;
        if (rs > max_rs) max_rs = rs;
    }
    if (trs) {
        max_rs /= 10;
        for (i=0; i<projects.size(); i++) {
            p = projects[i];
            p->disk_resource_share = p->resource_share + max_rs;
        }
    } else {
        for (i=0; i<projects.size(); i++) {
            p = projects[i];
            p->disk_resource_share = 1;
        }
    }

    // Compute:
    // greedy_drs: total disk resource share of greedy projects
    // non_greedy_ddu: total desired disk usage of non-greedy projects
    //
    double greedy_drs = 0;
    double non_greedy_ddu = 0;
    double allowed = allowed_disk_usage(total_disk_usage);
    for (i=0; i<projects.size(); i++) {
        p = projects[i];
        p->disk_quota = allowed*p->disk_resource_share/trs;
        if (p->ddu > p->disk_quota) {
            greedy_drs += p->disk_resource_share;
        } else {
            non_greedy_ddu += p->ddu;
        }
    }

    double greedy_allowed = allowed - non_greedy_ddu;
    if (log_flags.disk_usage_debug) {
        msg_printf(0, MSG_INFO,
            "[disk_usage] allowed %.2fGB used %.2fGB",
            allowed/GIGA, total_disk_usage/GIGA
        );
    }
    for (i=0; i<projects.size(); i++) {
        p = projects[i];
        double rs = p->disk_resource_share/trs;
        if (p->ddu > allowed*rs) {
            p->disk_share = greedy_allowed*p->disk_resource_share/greedy_drs;
        } else {
            p->disk_share = p->disk_quota;
        }
        if (log_flags.disk_usage_debug) {
            msg_printf(p, MSG_INFO,
                "[disk_usage] usage %.2fGB share %.2fGB",
                p->disk_usage/GIGA, p->disk_share/GIGA
            );
        }
    }
}

// See if we should suspend CPU and/or GPU processing;
// return the CPU suspend_reason,
// and if it's zero set gpu_suspend_reason
//
int CLIENT_STATE::check_suspend_processing() {
    static double last_cpu_usage_suspend=0;

    if (benchmarks_running) {
        return SUSPEND_REASON_BENCHMARKS;
    }

    if (cc_config.start_delay && now < time_stats.client_start_time + cc_config.start_delay) {
        return SUSPEND_REASON_INITIAL_DELAY;
    }

    if (os_requested_suspend) {
        return SUSPEND_REASON_OS;
    }

    switch (cpu_run_mode.get_current()) {
    case RUN_MODE_ALWAYS: break;
    case RUN_MODE_NEVER:
        return SUSPEND_REASON_USER_REQ;
    default:
        // "run according to prefs" checks:
        //
        if (!global_prefs.run_on_batteries
            && host_info.host_is_running_on_batteries()
        ) {
            return SUSPEND_REASON_BATTERIES;
        }
        if (!global_prefs.run_if_user_active && user_active) {
            return SUSPEND_REASON_USER_ACTIVE;
        }
        if (global_prefs.cpu_times.suspended(now)) {
            return SUSPEND_REASON_TIME_OF_DAY;
        }
        if (global_prefs.suspend_if_no_recent_input) {
            long idle_time = host_info.user_idle_time(check_all_logins);
            if (idle_time != USER_IDLE_TIME_INF
                && idle_time > global_prefs.suspend_if_no_recent_input*60
            ) {
                return SUSPEND_REASON_NO_RECENT_INPUT;
            }
        }
        if (now - exclusive_app_running < MEMORY_USAGE_PERIOD + EXCLUSIVE_APP_WAIT) {
            return SUSPEND_REASON_EXCLUSIVE_APP_RUNNING;
        }

        // if we suspended because of CPU usage,
        // don't unsuspend for at least 2*MEMORY_USAGE_PERIOD
        //
        if (current_suspend_cpu_usage()) {
            if (now < last_cpu_usage_suspend+2*MEMORY_USAGE_PERIOD) {
                return SUSPEND_REASON_CPU_USAGE;
            }
            if (non_boinc_cpu_usage*100 > current_suspend_cpu_usage()) {
                last_cpu_usage_suspend = now;
                return SUSPEND_REASON_CPU_USAGE;
            }
        }
    }

#ifdef ANDROID
    // Battery checks.
    // Do these only if we've received an RPC from the GUI
    // (which is where we get battery info)

    if (device_status_time) {
        // exit if we haven't heard from the GUI in 30 sec
        // (we rely on it for battery info)
        //
        if (now > device_status_time + ANDROID_KEEPALIVE_TIMEOUT) {
            msg_printf(NULL, MSG_INTERNAL_ERROR,
                "No RPC from GUI in last %d sec - exiting",
                ANDROID_KEEPALIVE_TIMEOUT
            );
            requested_exit = true;
            return SUSPEND_REASON_NO_GUI_KEEPALIVE;
        }

        // check for hot battery
        // If suspend because of hot battery, don't resume for at least 5 min
        // (crude hysteresis)
        //
        static double battery_heat_resume_time=0;
        if (now < battery_heat_resume_time) {
            return SUSPEND_REASON_BATTERY_OVERHEATED;
        }
        if (device_status.battery_state == BATTERY_STATE_OVERHEATED) {
            battery_heat_resume_time = now + ANDROID_BATTERY_BACKOFF;
            return SUSPEND_REASON_BATTERY_OVERHEATED;
        }
        if (device_status.battery_temperature_celsius > global_prefs.battery_max_temperature) {
            battery_heat_resume_time = now + ANDROID_BATTERY_BACKOFF;
            return SUSPEND_REASON_BATTERY_OVERHEATED;
        }

        // check for sufficient battery charge.
        // If suspend, don't resume for at least 5 min
        //
        static double battery_charge_resume_time=0;
        if (now < battery_charge_resume_time) {
            return SUSPEND_REASON_BATTERY_CHARGING;
        }
        int cp = device_status.battery_charge_pct;
        if (cp >= 0) {
            if (cp < global_prefs.battery_charge_min_pct) {
                battery_charge_resume_time = now + ANDROID_BATTERY_BACKOFF;
                return SUSPEND_REASON_BATTERY_CHARGING;
            }
        }
    }
#endif

    // CPU is not suspended.  See if GPUs are
    //
    if (!coprocs.none()) {
        int old_gpu_suspend_reason = gpu_suspend_reason;
        gpu_suspend_reason = 0;
        switch (gpu_run_mode.get_current()) {
        case RUN_MODE_ALWAYS:
            break;
        case RUN_MODE_NEVER:
            gpu_suspend_reason = SUSPEND_REASON_USER_REQ;
            break;
        default:
            if (now - exclusive_gpu_app_running < MEMORY_USAGE_PERIOD + EXCLUSIVE_APP_WAIT) {
                gpu_suspend_reason = SUSPEND_REASON_EXCLUSIVE_APP_RUNNING;
                break;
            }
            if (user_active && !global_prefs.run_gpu_if_user_active) {
                gpu_suspend_reason = SUSPEND_REASON_USER_ACTIVE;
                break;
            }
        }

        if (old_gpu_suspend_reason && !gpu_suspend_reason) {
            if (log_flags.task) {
                msg_printf(NULL, MSG_INFO, "Resuming GPU computation");
            }
            request_schedule_cpus("GPU resumption");
        } else if (!old_gpu_suspend_reason && gpu_suspend_reason) {
            if (log_flags.task) {
                msg_printf(NULL, MSG_INFO, "Suspending GPU computation - %s",
                    suspend_reason_string(gpu_suspend_reason)
                );
            }
            request_schedule_cpus("GPU suspension");
        }
    }

    return 0;
}

void CLIENT_STATE::show_suspend_tasks_message(int reason) {
    if (reason != SUSPEND_REASON_CPU_THROTTLE) {
        if (log_flags.task) {
            msg_printf(NULL, MSG_INFO,
                "Suspending computation - %s",
                suspend_reason_string(reason)
            );
        }
#ifdef ANDROID
        switch (reason) {
        case SUSPEND_REASON_BATTERY_OVERHEATED:
            if (log_flags.task) {
                msg_printf(NULL, MSG_INFO,
                    "(battery temperature %.1f > limit %.1f Celsius)",
                    device_status.battery_temperature_celsius,
                    global_prefs.battery_max_temperature
                );
            }
            break;
        case SUSPEND_REASON_BATTERY_CHARGING:
            if (log_flags.task) {
                msg_printf(NULL, MSG_INFO,
                    "(battery charge level %.1f%% < threshold %.1f%%",
                    device_status.battery_charge_pct,
                    global_prefs.battery_charge_min_pct
                );
            }
            break;
        }
#endif
    }
}

int CLIENT_STATE::resume_tasks(int reason) {
    if (reason == SUSPEND_REASON_CPU_THROTTLE) {
        active_tasks.unsuspend_all(SUSPEND_REASON_CPU_THROTTLE);
    } else {
        active_tasks.unsuspend_all();
        request_schedule_cpus("Resuming computation");
    }
    return 0;
}

// Check whether to set network_suspended and file_xfers_suspended.
//
void CLIENT_STATE::check_suspend_network() {
    network_suspended = false;
    file_xfers_suspended = false;
    network_suspend_reason = 0;
    bool recent_rpc;

    // don't start network ops if system is shutting down
    //
    if (os_requested_suspend) {
        network_suspend_reason = SUSPEND_REASON_OS;
        network_suspended = true;
        goto done;
    }

    // no network traffic if we're allowing unsigned apps
    //
    if (cc_config.unsigned_apps_ok) {
        network_suspended = true;
        file_xfers_suspended = true;
        network_suspend_reason = SUSPEND_REASON_USER_REQ;
        goto done;
    }

    // was there a recent GUI RPC that needs network?
    //
    recent_rpc = gui_rpcs.recent_rpc_needs_network(
        ALLOW_NETWORK_IF_RECENT_RPC_PERIOD
    );

    switch(network_run_mode.get_current()) {
    case RUN_MODE_ALWAYS:
        goto done;
    case RUN_MODE_NEVER:
        file_xfers_suspended = true;
        if (!recent_rpc) network_suspended = true;
        network_suspend_reason = SUSPEND_REASON_USER_REQ;
        goto done;
    }

#ifdef ANDROID
    if (now > device_status_time + ANDROID_KEEPALIVE_TIMEOUT) {
        requested_exit = true;
        file_xfers_suspended = true;
        if (!recent_rpc) network_suspended = true;
        network_suspend_reason = SUSPEND_REASON_NO_GUI_KEEPALIVE;
    }
    // use only WiFi
    //
    if (global_prefs.network_wifi_only && !device_status.wifi_online) {
        file_xfers_suspended = true;
        if (!recent_rpc) network_suspended = true;
        network_suspend_reason = SUSPEND_REASON_WIFI_STATE;
    }
#endif

    if (global_prefs.daily_xfer_limit_mb && global_prefs.daily_xfer_period_days) {
        double up, down;
        daily_xfer_history.totals(
            global_prefs.daily_xfer_period_days, up, down
        );
        if (up+down > global_prefs.daily_xfer_limit_mb*MEGA) {
            file_xfers_suspended = true;
            if (!recent_rpc) network_suspended = true;
            network_suspend_reason = SUSPEND_REASON_NETWORK_QUOTA_EXCEEDED;
        }
    }

#ifndef ANDROID
    // allow network transfers while user active, i.e. screen on.
    // otherwise nothing (visible to the user) happens after initial attach
    //
    if (!global_prefs.run_if_user_active && user_active) {
        file_xfers_suspended = true;
        if (!recent_rpc) network_suspended = true;
        network_suspend_reason = SUSPEND_REASON_USER_ACTIVE;
    }
#endif
    if (global_prefs.net_times.suspended(now)) {
        file_xfers_suspended = true;
        if (!recent_rpc) network_suspended = true;
        network_suspend_reason = SUSPEND_REASON_TIME_OF_DAY;
    }
    if (now - exclusive_app_running < MEMORY_USAGE_PERIOD + EXCLUSIVE_APP_WAIT) {
        file_xfers_suspended = true;
        if (!recent_rpc) network_suspended = true;
        network_suspend_reason = SUSPEND_REASON_EXCLUSIVE_APP_RUNNING;
    }

done:
    if (log_flags.suspend_debug) {
        msg_printf(0, MSG_INFO, "[suspend] net_susp: %s; file_xfer_susp: %s; reason: %s",
            network_suspended?"yes":"no",
            file_xfers_suspended?"yes":"no",
            suspend_reason_string(network_suspend_reason)
        );
    }
}

#endif // ifndef SIM

// call this only after parsing global prefs
//
PROJECT* CLIENT_STATE::global_prefs_source_project() {
    return lookup_project(global_prefs.source_project);
}

void CLIENT_STATE::show_global_prefs_source(bool found_venue) {
    PROJECT* pp = global_prefs_source_project();
    if (pp) {
        msg_printf(pp, MSG_INFO,
            "Computing prefs: from %s (last modified %s)",
            pp->get_project_name(), time_to_string(global_prefs.mod_time)
        );
    } else {
        msg_printf(NULL, MSG_INFO,
            "Computing prefs: from %s (last modified %s)",
            global_prefs.source_project,
            time_to_string(global_prefs.mod_time)
        );
    }
    if (strlen(main_host_venue)) {
        msg_printf(pp, MSG_INFO, "Computer location: %s", main_host_venue);
        if (found_venue) {
            msg_printf(NULL, MSG_INFO,
                "Computing prefs: using separate prefs for %s", main_host_venue
            );
        } else {
            msg_printf(pp, MSG_INFO,
                "Computing prefs: no separate prefs for %s; using default location",
                main_host_venue
            );
        }
    } else {
        msg_printf(pp, MSG_INFO, "Computing prefs: computer location unspecified; using default");
    }
}

// parse user's project preferences,
// generating FILE_REF and FILE_INFO objects for each <app_file> element.
//
int PROJECT::parse_preferences_for_user_files() {
    char buf[1024];
    string timestamp, open_name, url, filename;
    FILE_INFO* fip;
    FILE_REF fr;

    user_files.clear();
    size_t n=0, start, end;
    const size_t app_file_open_tag_len = strlen("<app_file>");
    const size_t app_file_close_tag_len = strlen("</app_file>");
    while (1) {
        start = project_specific_prefs.find("<app_file>", n);
        if (start == string::npos) break;
        end = project_specific_prefs.find("</app_file>", n);
        if (end == string::npos) break;
        start += app_file_open_tag_len;
        string x = project_specific_prefs.substr(start, end);
        n = end + app_file_close_tag_len;

        strlcpy(buf, x.c_str(), sizeof(buf));
        if (!parse_str(buf, "<timestamp>", timestamp)) break;
        if (!parse_str(buf, "<open_name>", open_name)) break;
        if (!parse_str(buf, "<url>", url)) break;

        filename = open_name + "_" + timestamp;
        fip = gstate.lookup_file_info(this, filename.c_str());
        if (!fip) {
            fip = new FILE_INFO;
            fip->project = this;
            fip->download_urls.add(url);
            safe_strcpy(fip->name, filename.c_str());
            fip->is_user_file = true;
            gstate.file_infos.push_back(fip);
        }

        fr.file_info = fip;
        safe_strcpy(fr.open_name, open_name.c_str());
        user_files.push_back(fr);
    }
    return 0;
}

// Read global preferences into the global_prefs structure.
// 1) read the override file to get venue in case it's there
// 2) read global_prefs.xml
// 3) read the override file again
//
// This is called:
// - on startup
// - on completion of a scheduler or AMS RPC, if they sent prefs
// - in response to read_global_prefs_override GUI RPC
//
void CLIENT_STATE::read_global_prefs(
    const char* fname, const char* override_fname
) {
    bool found_venue;
    bool venue_specified_in_override = false;
    int retval;
    FILE* f;
    string foo;

#ifdef USE_NET_PREFS
    if (override_fname) {
        retval = read_file_string(override_fname, foo);
        if (!retval) {
            parse_str(foo.c_str(), "<host_venue>", main_host_venue, sizeof(main_host_venue));
            if (strlen(main_host_venue)) {
                venue_specified_in_override = true;
            }
        }
    }

    retval = global_prefs.parse_file(
        fname, main_host_venue, found_venue
    );
    if (retval) {
        if (retval == ERR_FOPEN) {
            msg_printf(NULL, MSG_INFO,
                "No general preferences found - using defaults"
            );
        } else {
            msg_printf(NULL, MSG_INFO,
                "Couldn't parse preferences file - using defaults"
            );
            boinc_delete_file(fname);
        }
        global_prefs.init();
    } else {
        if (!venue_specified_in_override) {
            // check that the source project's venue matches main_host_venue.
            // If not, read file again.
            // This is a fix for cases where main_host_venue is out of synch
            //
            PROJECT* p = global_prefs_source_project();
            if (p && strcmp(main_host_venue, p->host_venue)) {
                safe_strcpy(main_host_venue, p->host_venue);
                global_prefs.parse_file(fname, main_host_venue, found_venue);
            }
        }
        show_global_prefs_source(found_venue);
    }
#endif

    // read the override file
    //
    global_prefs.override_file_present = false;
    if (override_fname) {
        f = fopen(override_fname, "r");
        if (f) {
            MIOFILE mf;
            GLOBAL_PREFS_MASK mask;
            mf.init_file(f);
            XML_PARSER xp(&mf);
            global_prefs.parse_override(xp, "", found_venue, mask);
            msg_printf(NULL, MSG_INFO, "Reading preferences override file");
            fclose(f);
            global_prefs.override_file_present = true;
        }
    }

#ifndef SIM
    get_disk_usages();
#endif
    set_n_usable_cpus();

#ifdef ANDROID
    global_prefs.run_if_user_active = false;
#endif
#ifndef SIM
    file_xfers->set_bandwidth_limits(true);
    file_xfers->set_bandwidth_limits(false);
#endif

    bool have_gpu = coprocs.n_rsc > 1;
    global_prefs.need_idle_state = global_prefs.get_need_idle_state(have_gpu);
    print_global_prefs();
    request_schedule_cpus("Prefs update");
    request_work_fetch("Prefs update");
#ifndef SIM
    active_tasks.request_reread_app_info();
#endif
}

void CLIENT_STATE::print_global_prefs() {
    msg_printf(NULL, MSG_INFO, "Computing preferences:");

    // in-use prefs
    //
    msg_printf(NULL, MSG_INFO, "-  When computer is in use");
    msg_printf(NULL, MSG_INFO,
        "-     'In use' means mouse/keyboard input in last %.2f minutes",
        global_prefs.idle_time_to_run
    );
    if (!global_prefs.run_if_user_active) {
        msg_printf(NULL, MSG_INFO, "-     don't compute");
    }
    if (!global_prefs.run_gpu_if_user_active) {
        msg_printf(NULL, MSG_INFO, "-     don't use GPU");
    }
    double p = global_prefs.max_ncpus_pct;
    if (p) {
        int n = (int)((host_info.p_ncpus * p)/100);
        msg_printf(NULL, MSG_INFO,
            "-     max CPUs used: %d", n
        );
    }
    if (global_prefs.cpu_usage_limit) {
        msg_printf(NULL, MSG_INFO,
            "-     Use at most %.0f%% of the CPU time",
            global_prefs.cpu_usage_limit
        );
    }
    if (global_prefs.suspend_cpu_usage) {
        msg_printf(NULL, MSG_INFO,
            "-     suspend if non-BOINC CPU load exceeds %.0f%%",
            global_prefs.suspend_cpu_usage
        );
    }
    msg_printf(NULL, MSG_INFO,
        "-     max memory usage: %.2f GB",
        (host_info.m_nbytes*global_prefs.ram_max_used_busy_frac)/GIGA
    );

    // not-in-use prefs
    //
    msg_printf(NULL, MSG_INFO,
        "-  When computer is not in use"
    );
    p = global_prefs.niu_max_ncpus_pct;
    int n = (int)((host_info.p_ncpus * p)/100);
    msg_printf(NULL, MSG_INFO,
        "-     max CPUs used: %d", n
    );

    msg_printf(NULL, MSG_INFO,
        "-     Use at most %.0f%% of the CPU time",
        global_prefs.niu_cpu_usage_limit
    );

    if (global_prefs.niu_suspend_cpu_usage > 0) {
        msg_printf(NULL, MSG_INFO,
            "-     suspend if non-BOINC CPU load exceeds %.0f%%",
            global_prefs.niu_suspend_cpu_usage
        );
    }
    msg_printf(NULL, MSG_INFO,
        "-     max memory usage: %.2f GB",
        (host_info.m_nbytes*global_prefs.ram_max_used_idle_frac)/GIGA
    );
    if (global_prefs.suspend_if_no_recent_input > 0) {
        msg_printf(NULL, MSG_INFO,
            "-     Suspend if no input in last %.2f minutes",
            global_prefs.suspend_if_no_recent_input
        );
    }

    // Computing (CPU or GPU) could be suspended indefinitely
    // if the idle time required before continuing computing
    // is longer than the time required to suspend computing
    // when the computer is idle.
    // In this case show an alert message.
    //
    if ((!global_prefs.run_if_user_active || !global_prefs.run_gpu_if_user_active) && (global_prefs.suspend_if_no_recent_input > 0) &&
        ((global_prefs.idle_time_to_run - global_prefs.suspend_if_no_recent_input) >= 0)) {
        msg_printf(0, MSG_USER_ALERT,
            "Preference settings don't allow computing (%.2f > %.2f). Please review.",
            global_prefs.idle_time_to_run, global_prefs.suspend_if_no_recent_input
        );
    }

    // other prefs
    //

    if (!global_prefs.run_on_batteries) {
        msg_printf(NULL, MSG_INFO,
            "-  Suspend if running on batteries"
        );
    }
    if (global_prefs.leave_apps_in_memory) {
        msg_printf(NULL, MSG_INFO,
            "-  Leave apps in memory if not running"
        );
    }
    msg_printf(NULL, MSG_INFO,
        "-  Store at least %.2f days of work",
        global_prefs.work_buf_min_days
    );
    msg_printf(NULL, MSG_INFO,
        "-  Store up to an additional %.2f days of work",
        global_prefs.work_buf_additional_days
    );

    // network
    //
    if (global_prefs.max_bytes_sec_down) {
        msg_printf(NULL, MSG_INFO,
            "-  max download rate: %.0f bytes/sec",
            global_prefs.max_bytes_sec_down
        );
    }
    if (global_prefs.max_bytes_sec_up) {
        msg_printf(NULL, MSG_INFO,
            "-  max upload rate: %.0f bytes/sec",
            global_prefs.max_bytes_sec_up
        );
    }

    // disk
    //
#ifndef SIM
    msg_printf(NULL, MSG_INFO,
        "-  max disk usage: %.2f GB",
        allowed_disk_usage(total_disk_usage)/GIGA
    );
#endif
    if (!global_prefs.need_idle_state) {
        msg_printf(NULL, MSG_INFO,
            "-  Preferences don't depend on whether computer is in use"
        );
    }
    msg_printf(NULL, MSG_INFO,
        "-  (to change preferences, visit a project web site or select 'Options / Computing preferences...' in the Manager)"
    );
}

int CLIENT_STATE::save_global_prefs(
    const char* global_prefs_xml, char* master_url, char* scheduler_url
) {
    FILE* f = boinc_fopen(GLOBAL_PREFS_FILE_NAME, "w");
    if (!f) return ERR_FOPEN;
    fprintf(f,
        "<global_preferences>\n"
    );

    // tag with the project and scheduler URL,
    // but only if not already tagged
    //
    if (!strstr(global_prefs_xml, "<source_project>")) {
        fprintf(f,
            "    <source_project>%s</source_project>\n"
            "    <source_scheduler>%s</source_scheduler>\n",
            master_url,
            scheduler_url
        );
    }
    fprintf(f,
        "%s"
        "</global_preferences>\n",
        global_prefs_xml
    );
    fclose(f);
    return 0;
}

// amount of RAM usable now
//
double CLIENT_STATE::available_ram() {
    if (user_active) {
        return host_info.m_nbytes * global_prefs.ram_max_used_busy_frac;
    } else {
        return host_info.m_nbytes * global_prefs.ram_max_used_idle_frac;
    }
}

// max amount that will ever be usable
//
double CLIENT_STATE::max_available_ram() {
    return host_info.m_nbytes*std::max(
        global_prefs.ram_max_used_busy_frac, global_prefs.ram_max_used_idle_frac
    );
}
