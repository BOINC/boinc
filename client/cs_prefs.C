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

// Logic related to general (also known as global) preferences:
// when to compute, how much disk to use, etc.
//

#ifdef _WIN32
#include "boinc_win.h"
#endif

#ifndef _WIN32
#include "config.h"
#if HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif
#if HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#endif

#include "str_util.h"
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
double CLIENT_STATE::allowed_disk_usage() {
    double percent_space, min_val;

    percent_space = host_info.d_total*global_prefs.disk_max_used_pct/100.0;

    min_val = host_info.d_free - global_prefs.disk_min_free_gb*1e9;

    double size = min(min(global_prefs.disk_max_used_gb*(1e9), percent_space), min_val);
    if (size < 0) size = 0;
    return size;
}

int CLIENT_STATE::project_disk_usage(PROJECT* p, double& size) {
    char buf[256];
    unsigned int i;
    double s;

    get_project_dir(p, buf, sizeof(buf));
    dir_size(buf, size);

    for (i=0; i<active_tasks.active_tasks.size(); i++) {
        ACTIVE_TASK* atp = active_tasks.active_tasks[i];
        if (atp->wup->project != p) continue;
        get_slot_dir(atp->slot, buf, sizeof(buf));
        dir_size(buf, s);
        size += s;
    }

    return 0;
}

int CLIENT_STATE::total_disk_usage(double& size) {
    return dir_size(".", size);
}

#if 0
int CLIENT_STATE::allowed_project_disk_usage(double& size) {
    double other_disk_used;
    double total_disk_available;
    double project_disk_used;
    total_disk_usage(other_disk_used);
    allowed_disk_usage(total_disk_available);
    for(unsigned int i=0; i<projects.size(); i++) {
        project_disk_usage(projects[i], project_disk_used);
        other_disk_used -= project_disk_used;
    }
    size = total_disk_available - other_disk_used;
    return 0;
}
#endif

// See if (on the basis of user run request and prefs)
// we should suspend activities.
//
void CLIENT_STATE::check_suspend_activities(int& reason) {
    reason = 0;
#ifdef __APPLE__
    double idletime;
#endif
    // Don't work while we're running CPU benchmarks
    //
    if (are_cpu_benchmarks_running()) {
        reason = SUSPEND_REASON_BENCHMARKS;
        return;
    }

	bool old_user_active = user_active;
    user_active = !host_info.users_idle(
        check_all_logins, global_prefs.idle_time_to_run
#ifdef __APPLE__
         , &idletime
#endif
    );

	if (user_active != old_user_active) {
		request_schedule_cpus("Idle state change");
	}
#ifdef __APPLE__
    // Mac screensaver launches client if not already running.
    // OS X quits screensaver when energy saver puts display to sleep,
    // but we want to keep crunching.
    // Also, user can start Mac screensaver by putting cursor in "hot corner"
    // so idletime may be very small initially.
    // If screensaver started client, this code tells client 
    // to exit when user becomes active, accounting for all these factors.
    //
    if (started_by_screensaver && (idletime < 30) && (getppid() == 1)) {
        // pid is 1 if parent has exited
        requested_exit = true;
    }
    
    // Exit if we were launched by Manager and it crashed.
    if (launched_by_manager) {
        // pid is 1 if parent no longer running
        if (getppid() == 1)
            gstate.requested_exit = true;
    }
#endif

    switch(run_mode.get_current()) {
    case RUN_MODE_ALWAYS: break;
    case RUN_MODE_NEVER:
        reason = SUSPEND_REASON_USER_REQ;
        return;
    default:
        if (!global_prefs.run_on_batteries
            && host_info.host_is_running_on_batteries()
        ) {
            reason = SUSPEND_REASON_BATTERIES;
            return;
        }

        if (!global_prefs.run_if_user_active && user_active) {
            reason = SUSPEND_REASON_USER_ACTIVE;
            return;
        }

        if (global_prefs.suspended_time_of_day(PREFS_CPU)) {
            reason = SUSPEND_REASON_TIME_OF_DAY;
            return;
        }
    }

    if (global_prefs.cpu_usage_limit != 100) {
        static double last_time=0, debt=0;
        if (last_time) {
            double diff = now - last_time;
            if (diff >= POLL_INTERVAL/2. && diff < POLL_INTERVAL*10.) {
                debt += diff*global_prefs.cpu_usage_limit/100;
                if (debt < 0) {
                    reason = SUSPEND_REASON_CPU_USAGE_LIMIT;
                } else {
                    debt -= diff;
                }
            }
        }
        last_time = now;
    }
}

static string reason_string(int reason) {
    string s_reason;
    if (reason & SUSPEND_REASON_BATTERIES) {
        s_reason += " - on batteries";
    }
    if (reason & SUSPEND_REASON_USER_ACTIVE) {
        s_reason += " - user is active";
    }
    if (reason & SUSPEND_REASON_USER_REQ) {
        s_reason += " - user request";
    }
    if (reason & SUSPEND_REASON_TIME_OF_DAY) {
        s_reason += " - time of day";
    }
    if (reason & SUSPEND_REASON_BENCHMARKS) {
        s_reason += " - running CPU benchmarks";
    }
    if (reason & SUSPEND_REASON_DISK_SIZE) {
        s_reason += " - out of disk space - change global prefs";
    }
    return s_reason;
}

int CLIENT_STATE::suspend_tasks(int reason) {
    if (reason == SUSPEND_REASON_CPU_USAGE_LIMIT) {
        if (log_flags.cpu_sched) {
            msg_printf(NULL, MSG_INFO, "[cpu_sched] Suspending - CPU throttle");
        }
        active_tasks.suspend_all(true);
    } else {
        string s_reason;
        s_reason = "Suspending computation" + reason_string(reason);
        msg_printf(NULL, MSG_INFO, s_reason.c_str());
        active_tasks.suspend_all(global_prefs.leave_apps_in_memory);
    }
    return 0;
}

int CLIENT_STATE::resume_tasks(int reason) {
    if (reason == SUSPEND_REASON_CPU_USAGE_LIMIT) {
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

void CLIENT_STATE::check_suspend_network(int& reason) {
    reason = 0;

    switch(network_mode.get_current()) {
    case RUN_MODE_ALWAYS: return;
    case RUN_MODE_NEVER:
        reason |= SUSPEND_REASON_USER_REQ;
        return;
    }
    if (global_prefs.suspended_time_of_day(PREFS_NETWORK)) {
        reason |= SUSPEND_REASON_TIME_OF_DAY;
    }
    return;
}

int CLIENT_STATE::suspend_network(int reason) {
    string s_reason;
    s_reason = "Suspending network activity" + reason_string(reason);
    msg_printf(NULL, MSG_INFO, s_reason.c_str());
    pers_file_xfers->suspend();
    return 0;
}

int CLIENT_STATE::resume_network() {
    msg_printf(NULL, MSG_INFO, "Resuming network activity");
    return 0;
}

// call this only after parsing global prefs
//
PROJECT* CLIENT_STATE::global_prefs_source_project() {
    return lookup_project(global_prefs.source_project);
}

void CLIENT_STATE::show_global_prefs_source(bool found_venue) {
    PROJECT* pp = global_prefs_source_project();
    if (pp) {
        msg_printf(NULL, MSG_INFO,
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
		msg_printf(NULL, MSG_INFO, "Host location: %s", main_host_venue);
        if (found_venue) {
            msg_printf(NULL, MSG_INFO,
                "General prefs: using separate prefs for %s", main_host_venue
            );
        } else {
            msg_printf(NULL, MSG_INFO,
                "General prefs: no separate prefs for %s; using your defaults",
                main_host_venue
            );
        }
    } else {
		msg_printf(NULL, MSG_INFO, "Host location: none");
        msg_printf(NULL, MSG_INFO, "General prefs: using your defaults");
    }
}

// parse user's project preferences,
// generating FILE_REF and FILE_INFO objects for each <app_file> element.
//
int PROJECT::parse_preferences_for_user_files() {
    char* p, *q, *q2;
    char buf[1024];
    string timestamp, open_name, url, filename;
    FILE_INFO* fip;
    FILE_REF fr;
    char prefs_buf[MAX_PROJ_PREFS_LEN];
    strcpy(prefs_buf, project_specific_prefs.c_str());
    p = prefs_buf;

    user_files.clear();
    while (1) {
        q = strstr(p, "<app_file>");
        if (!q) break;
        q2 = strstr(q, "</app_file>");
        if (!q2) break;
        *q2 = 0;
        strcpy(buf, q);
        if (!parse_str(buf, "<timestamp>", timestamp)) break;
        if (!parse_str(buf, "<open_name>", open_name)) break;
        if (!parse_str(buf, "<url>", url)) break;

        filename = open_name + "_" + timestamp;
        fip = gstate.lookup_file_info(this, filename.c_str());
        if (!fip) {
            fip = new FILE_INFO;
            fip->project = this;
            fip->urls.push_back(url);
            strcpy(fip->name, filename.c_str());
            fip->is_user_file = true;
            gstate.file_infos.push_back(fip);
        }

        fr.file_info = fip;
        strcpy(fr.open_name, open_name.c_str());
        user_files.push_back(fr);

        p = q2+strlen("</app_file>");
    }

    return 0;
}

// Get global preferences.
// 1) read the override file to get venue in case it's there
// 2) read the prefs file
// 3) read the override file again
//
void CLIENT_STATE::read_global_prefs() {
    bool found_venue;
    int retval;
	FILE* f;
    string foo;

    retval = read_file_string(GLOBAL_PREFS_OVERRIDE_FILE, foo);
    if (!retval) {
		parse_str(foo.c_str(), "<host_venue>", main_host_venue, sizeof(main_host_venue));
	}

    retval = global_prefs.parse_file(
        GLOBAL_PREFS_FILE_NAME, main_host_venue, found_venue
    );
    if (retval) {
        msg_printf(NULL, MSG_INFO,
            "No general preferences found - using BOINC defaults"
        );
    } else {
		// check that the source project's venue matches main_host_venue.
		// If not, read file again.
		// This is a fix for cases where main_host_venue is out of synch
		//
		PROJECT* p = global_prefs_source_project();
		if (p && strcmp(main_host_venue, p->host_venue)) {
			strcpy(main_host_venue, p->host_venue);
			global_prefs.parse_file(GLOBAL_PREFS_FILE_NAME, main_host_venue, found_venue);
		}
        show_global_prefs_source(found_venue);
    }

    // read the override file
    //
    f = fopen(GLOBAL_PREFS_OVERRIDE_FILE, "r");
    if (f) {
        MIOFILE mf;
        GLOBAL_PREFS_MASK mask;
        mf.init_file(f);
        XML_PARSER xp(&mf);
        global_prefs.parse_override(xp, "", found_venue, mask);
        msg_printf(NULL, MSG_INFO, "Reading preferences override file");
        fclose(f);
    }

    msg_printf(NULL, MSG_INFO,
		"Preferences limit memory usage when active to %.2fMB",
        (host_info.m_nbytes*global_prefs.ram_max_used_busy_frac)/MEGA
    );
    msg_printf(NULL, MSG_INFO,
		"Preferences limit memory usage when idle to %.2fMB",
		(host_info.m_nbytes*global_prefs.ram_max_used_idle_frac)/MEGA
    );
    msg_printf(NULL, MSG_INFO,
		"Preferences limit disk usage to %.2fGB",
        allowed_disk_usage()/GIGA
    );
    // max_cpus, bandwidth limits may have changed
    //
    set_ncpus();
	file_xfers->set_bandwidth_limits(true);
	file_xfers->set_bandwidth_limits(false);
	request_schedule_cpus("Prefs update");
	request_work_fetch("Prefs update");
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

const char *BOINC_RCSID_92ad99cddf = "$Id$";
