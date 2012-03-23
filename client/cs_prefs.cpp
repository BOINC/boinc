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

#include "str_util.h"
#include "str_replace.h"
#include "util.h"
#include "filesys.h"
#include "parse.h"
#include "file_names.h"
#include "cpu_benchmark.h"
#include "client_msgs.h"
#include "client_state.h"

using std::min;
using std::string;

#define MAX_PROJ_PREFS_LEN  65536
    // max length of project-specific prefs

// Return the maximum allowed disk usage as determined by user preferences.
// There are three different settings in the prefs;
// return the least of the three.
//
double CLIENT_STATE::allowed_disk_usage(double boinc_total) {
    double limit_pct, limit_min_free, limit_abs;

    limit_pct = host_info.d_total*global_prefs.disk_max_used_pct/100.0;
    limit_min_free = boinc_total + host_info.d_free - global_prefs.disk_min_free_gb*GIGA;
    limit_abs = global_prefs.disk_max_used_gb*(GIGA);

    double size = min(min(limit_abs, limit_pct), limit_min_free);
    if (size < 0) size = 0;
    return size;
}

#ifndef SIM

// populate:
// PROJECT::disk_usage for all projects
// GLOBAL_STATE::client_disk_usage
// GLOBAL_STATE::total_disk_usage
//
int CLIENT_STATE::get_disk_usages() {
    char buf[256];
    unsigned int i;
    double size;
    PROJECT* p;
    int retval;

    client_disk_usage = 0;
    total_disk_usage = 0;
    for (i=0; i<projects.size(); i++) {
        p = projects[i];
        p->disk_usage = 0;
        get_project_dir(p, buf, sizeof(buf));
        retval = dir_size(buf, size);
        if (!retval) p->disk_usage = size;
    }

    for (i=0; i<active_tasks.active_tasks.size(); i++) {
        ACTIVE_TASK* atp = active_tasks.active_tasks[i];
        get_slot_dir(atp->slot, buf, sizeof(buf));
        retval = dir_size(buf, size);
        if (retval) continue;
        atp->wup->project->disk_usage += size;
    }
    for (i=0; i<projects.size(); i++) {
        p = projects[i];
        total_disk_usage += p->disk_usage;
    }
    retval = dir_size(".", size, false);
    if (!retval) {
        client_disk_usage = size;
        total_disk_usage += size;
    }
    return 0;
}

// populate PROJECT::disk_share for all projects
//
void CLIENT_STATE::get_disk_shares() {
    PROJECT* p;
    unsigned int i;

    double rss = 0;
    for (i=0; i<projects.size(); i++) {
        p = projects[i];
        rss += p->resource_share;
        p->disk_share = p->disk_usage;
    }
    if (!rss) return;

    // a project is "greedy" if it's using more than its share of disk
    //
    double greedy_rs = 0;
    double non_greedy_usage = 0;
    double allowed = allowed_disk_usage(total_disk_usage);
    for (i=0; i<projects.size(); i++) {
        p = projects[i];
        double rs = p->resource_share/rss;
        if (p->disk_usage > allowed*rs) {
            greedy_rs += p->resource_share;
        } else {
            non_greedy_usage += p->disk_usage;
        }
    }
    if (!greedy_rs) greedy_rs = 1;      // handle projects w/ zero resource share

    double greedy_allowed = allowed - non_greedy_usage;
    if (log_flags.disk_usage_debug) {
        msg_printf(0, MSG_INFO,
            "[disk_usage] allowed %.2fMB used %.2fMB",
            allowed, total_disk_usage
        );
    }
    for (i=0; i<projects.size(); i++) {
        p = projects[i];
        double rs = p->resource_share/rss;
        if (p->disk_usage > allowed*rs) {
            p->disk_share = greedy_allowed*p->resource_share/greedy_rs;
        }
        if (log_flags.disk_usage_debug) {
            msg_printf(p, MSG_INFO,
                "[disk_usage] usage %.2fMB share %.2fMB",
                p->disk_usage/MEGA, p->disk_share/MEGA
            );
        }
    }
}

// See if we should suspend processing
//
int CLIENT_STATE::check_suspend_processing() {
    if (are_cpu_benchmarks_running()) {
        return SUSPEND_REASON_BENCHMARKS;
    }

    if (config.start_delay && now < client_start_time + config.start_delay) {
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
            bool idle = host_info.users_idle(
                check_all_logins, global_prefs.suspend_if_no_recent_input
            );
            if (idle) {
                return SUSPEND_REASON_NO_RECENT_INPUT;
            }
        }
        if (now - exclusive_app_running < EXCLUSIVE_APP_WAIT) {
            return SUSPEND_REASON_EXCLUSIVE_APP_RUNNING;
        }
        if (global_prefs.suspend_cpu_usage && non_boinc_cpu_usage*100 > global_prefs.suspend_cpu_usage) {
            return SUSPEND_REASON_CPU_USAGE;
        }
    }

    if (global_prefs.cpu_usage_limit < 99) {        // round-off?
        static double last_time=0, debt=0;
        double diff = now - last_time;
        last_time = now;
        if (diff >= POLL_INTERVAL/2. && diff < POLL_INTERVAL*10.) {
            debt += diff*global_prefs.cpu_usage_limit/100;
            if (debt < 0) {
                return SUSPEND_REASON_CPU_THROTTLE;
            } else {
                debt -= diff;
            }
        }
    }

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
            if (now - exclusive_gpu_app_running < EXCLUSIVE_APP_WAIT) {
                gpu_suspend_reason = SUSPEND_REASON_EXCLUSIVE_APP_RUNNING;
                break;
            }
            if (user_active && !global_prefs.run_gpu_if_user_active) {
                gpu_suspend_reason = SUSPEND_REASON_USER_ACTIVE;
                break;
            }
        }

        if (log_flags.cpu_sched) {
            if (old_gpu_suspend_reason && !gpu_suspend_reason) {
                msg_printf(NULL, MSG_INFO, "[cpu_sched] resuming GPU activity");
                request_schedule_cpus("GPU resumption");
            } else if (!old_gpu_suspend_reason && gpu_suspend_reason) {
                msg_printf(NULL, MSG_INFO, "[cpu_sched] suspending GPU activity");
                request_schedule_cpus("GPU suspension");
            }
        }
    }

    return 0;
}


void print_suspend_tasks_message(int reason) {
    msg_printf(NULL, MSG_INFO, "Suspending computation - %s", suspend_reason_string(reason));
}


int CLIENT_STATE::suspend_tasks(int reason) {
    if (reason == SUSPEND_REASON_CPU_THROTTLE) {
        if (log_flags.cpu_sched) {
            msg_printf(NULL, MSG_INFO, "[cpu_sched] Suspending - CPU throttle");
        }
    } else {
        print_suspend_tasks_message(reason);
    }
    active_tasks.suspend_all(reason);
    return 0;
}

int CLIENT_STATE::resume_tasks(int reason) {
    if (reason == SUSPEND_REASON_CPU_THROTTLE) {
        if (log_flags.cpu_sched) {
            msg_printf(NULL, MSG_INFO, "[cpu_sched] Resuming - CPU throttle");
        }
        active_tasks.unsuspend_all();
    } else {
        msg_printf(NULL, MSG_INFO, "Resuming computation");
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

    if (os_requested_suspend) {
        network_suspend_reason = SUSPEND_REASON_OS;
        network_suspended = true;
        return;
    }

    // no network traffic if we're allowing unsigned apps
    //
    if (config.unsigned_apps_ok) {
        network_suspended = true;
        file_xfers_suspended = true;
        network_suspend_reason = SUSPEND_REASON_USER_REQ;
        return;
    }

    // was there a recent GUI RPC that needs network?
    //
    bool recent_rpc = gui_rpcs.recent_rpc_needs_network(
        ALLOW_NETWORK_IF_RECENT_RPC_PERIOD
    );

    switch(network_run_mode.get_current()) {
    case RUN_MODE_ALWAYS: 
        return;
    case RUN_MODE_NEVER:
        file_xfers_suspended = true;
        if (!recent_rpc) network_suspended = true;
        network_suspend_reason = SUSPEND_REASON_USER_REQ;
    }

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

    if (!global_prefs.run_if_user_active && user_active) {
        file_xfers_suspended = true;
        if (!recent_rpc) network_suspended = true;
        network_suspend_reason = SUSPEND_REASON_USER_ACTIVE;
    }
    if (global_prefs.net_times.suspended(now)) {
        file_xfers_suspended = true;
        if (!recent_rpc) network_suspended = true;
        network_suspend_reason = SUSPEND_REASON_TIME_OF_DAY;
    }
    if (now - exclusive_app_running < EXCLUSIVE_APP_WAIT) {
        file_xfers_suspended = true;
        if (!recent_rpc) network_suspended = true;
        network_suspend_reason = SUSPEND_REASON_EXCLUSIVE_APP_RUNNING;
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
            "General prefs: from %s (last modified %s)",
            pp->get_project_name(), time_to_string(global_prefs.mod_time)
        );
    } else {
        msg_printf(NULL, MSG_INFO,
            "General prefs: from %s (last modified %s)",
            global_prefs.source_project,
            time_to_string(global_prefs.mod_time)
        );
    }
    if (strlen(main_host_venue)) {
        msg_printf(pp, MSG_INFO, "Computer location: %s", main_host_venue);
        if (found_venue) {
            msg_printf(NULL, MSG_INFO,
                "General prefs: using separate prefs for %s", main_host_venue
            );
        } else {
            msg_printf(pp, MSG_INFO,
                "General prefs: no separate prefs for %s; using your defaults",
                main_host_venue
            );
        }
    } else {
        msg_printf(pp, MSG_INFO, "Host location: none");
        msg_printf(pp, MSG_INFO, "General prefs: using your defaults");
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
    while (1) {
        start = project_specific_prefs.find("<app_file>", n);
        if (start == string::npos) break;
        end = project_specific_prefs.find("</app_file>", n);
        if (end == string::npos) break;
        start += strlen("<app_file>");
        string x = project_specific_prefs.substr(start, end);
        n = end + strlen("</app_file>");

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
            strcpy(fip->name, filename.c_str());
            fip->is_user_file = true;
            gstate.file_infos.push_back(fip);
        }

        fr.file_info = fip;
        strcpy(fr.open_name, open_name.c_str());
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
                strcpy(main_host_venue, p->host_venue);
                global_prefs.parse_file(fname, main_host_venue, found_venue);
            }
        }
        show_global_prefs_source(found_venue);
    }

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

    msg_printf(NULL, MSG_INFO, "Preferences:");
    msg_printf(NULL, MSG_INFO,
        "   max memory usage when active: %.2fMB",
        (host_info.m_nbytes*global_prefs.ram_max_used_busy_frac)/MEGA
    );
    msg_printf(NULL, MSG_INFO,
        "   max memory usage when idle: %.2fMB",
        (host_info.m_nbytes*global_prefs.ram_max_used_idle_frac)/MEGA
    );
#ifndef SIM
    get_disk_usages();
    msg_printf(NULL, MSG_INFO,
        "   max disk usage: %.2fGB",
        allowed_disk_usage(total_disk_usage)/GIGA
    );
#endif
    // max_cpus, bandwidth limits may have changed
    //
    set_ncpus();
    if (ncpus != host_info.p_ncpus) {
        msg_printf(NULL, MSG_INFO,
            "   max CPUs used: %d", ncpus
        );
    }
    if (!global_prefs.run_if_user_active) {
        msg_printf(NULL, MSG_INFO, "   don't compute while active");
    }
    if (!global_prefs.run_gpu_if_user_active) {
        msg_printf(NULL, MSG_INFO, "   don't use GPU while active");
    }
    if (global_prefs.suspend_cpu_usage) {
        msg_printf(NULL, MSG_INFO,
            "   suspend work if non-BOINC CPU load exceeds %.0f %%",
            global_prefs.suspend_cpu_usage
        );
    }
    if (global_prefs.max_bytes_sec_down) {
        msg_printf(NULL, MSG_INFO,
            "   max download rate: %.0f bytes/sec",
            global_prefs.max_bytes_sec_down
        );
    }
    if (global_prefs.max_bytes_sec_up) {
        msg_printf(NULL, MSG_INFO,
            "   max upload rate: %.0f bytes/sec",
            global_prefs.max_bytes_sec_up
        );
    }
#ifndef SIM
    file_xfers->set_bandwidth_limits(true);
    file_xfers->set_bandwidth_limits(false);
#endif
    msg_printf(NULL, MSG_INFO,
        "   (to change preferences, visit the web site of an attached project, or select Preferences in the Manager)"
    );
    request_schedule_cpus("Prefs update");
    request_work_fetch("Prefs update");
#ifndef SIM
    active_tasks.request_reread_app_info();
#endif
}

int CLIENT_STATE::save_global_prefs(
    char* global_prefs_xml, char* master_url, char* scheduler_url
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

