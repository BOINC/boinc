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

// Logic related to general (also known as global) preferences:
// when to compute, how much disk to use, etc.
//

#ifdef _WIN32
#include "stdafx.h"
#endif

#ifndef _WIN32
#if HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif
#if HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#endif

#include "util.h"
#include "filesys.h"
#include "file_names.h"
#include "cpu_benchmark.h"
#include "client_msgs.h"
#include "client_state.h"

void CLIENT_STATE::install_global_prefs() {
    net_xfers->max_bytes_sec_up = global_prefs.max_bytes_sec_up;
    net_xfers->max_bytes_sec_down = global_prefs.max_bytes_sec_down;
    net_xfers->bytes_left_up = global_prefs.max_bytes_sec_up;
    net_xfers->bytes_left_down = global_prefs.max_bytes_sec_down;

    // max_cpus may have changed, so update nslots
    //
    set_nslots();
}

// Return the maximum allowed disk usage as determined by user preferences.
// There are three different settings in the prefs;
// return the least of the three.
//
int CLIENT_STATE::allowed_disk_usage(double& size) {
    double percent_space, min_val;

    percent_space = host_info.d_total*global_prefs.disk_max_used_pct;

    min_val = host_info.d_free - global_prefs.disk_min_free_gb*(1024.*1024.*1024.);

    size = min(min(global_prefs.disk_max_used_gb*(1024.*1024.*1024.), percent_space), min_val);
    if(size < 0) size = 0;
    return 0;
}

int CLIENT_STATE::project_disk_usage(PROJECT* p, double& size) {
    char buf[256],buf2[256];

    // TODO: replace the following with a function

    escape_project_url(p->master_url, buf);
    sprintf(buf2, "%s%s%s", PROJECTS_DIR, PATH_SEPARATOR, buf);

    return dir_size(buf2, size);
}

int CLIENT_STATE::total_disk_usage(double& size) {
    return dir_size(".", size);
}

// returns true if start_hour == end_hour or start_hour <= now < end_hour
//
inline bool now_between_two_hours(int start_hour, int end_hour) {
    if (start_hour == end_hour) {
        // always work
        return true;
    }

    time_t now = time(0);
    struct tm *tmp = localtime(&now);
    int hour = tmp->tm_hour;
    if (start_hour < end_hour) {
        return (hour >= start_hour && hour < end_hour);
    } else {
        return !(hour >= end_hour && hour < start_hour);
    }
}

#ifndef _WIN32
// returns true iff device was last accessed before t
// or if an error occurred looking at the device.
inline bool device_idle(time_t t, char *device) {
    struct stat sbuf;
    return stat(device, &sbuf) || (sbuf.st_atime < t);
}

inline bool all_tty_idle(time_t t, char *device, char first_char, int num_tty) {
    struct stat sbuf;
    char *tty_index = device + strlen(device) - 1;
    *tty_index = first_char;
    for (int i = 0; i < num_tty; i++, (*tty_index)++) {
        if (stat(device, &sbuf)) {
            // error looking at device; don't try any more
            return true;
        } else if (sbuf.st_atime >= t) {
            return false;
        }
    }
    return true;
}

void CLIENT_STATE::check_idle() {
    char device_tty[] = "/dev/tty1";
    time_t idle_time =
        time(NULL) - (long) (60 * global_prefs.idle_time_to_run);
    user_idle = true
#ifdef HAVE__DEV_MOUSE
        && device_idle(idle_time, "/dev/mouse") // solaris, linux
#endif
#ifdef HAVE__DEV_KBD
        && device_idle(idle_time, "/dev/kbd") // solaris
#endif
#ifdef HAVE__DEV_TTY1
        && all_tty_idle(idle_time, device_tty, '1', 7) // linux
#endif
        ;
}
#endif

// See if (on the basis of user run request and prefs)
// we should suspend activities.
//
void CLIENT_STATE::check_suspend_activities(int& reason) {
    reason = 0;

    // Don't work while we're running CPU benchmarks
    //
    if (are_cpu_benchmarks_running()) {
        reason |= SUSPEND_REASON_BENCHMARKS;
    }

    if (user_run_request == USER_RUN_REQUEST_ALWAYS) return;

    if (user_run_request == USER_RUN_REQUEST_NEVER) {
        reason |= SUSPEND_REASON_USER_REQ;
        return;
    }

    if (!global_prefs.run_on_batteries
        && host_info.host_is_running_on_batteries()
    ) {
        reason |= SUSPEND_REASON_BATTERIES;
    }

    if (!global_prefs.run_if_user_active && !user_idle) {
        reason |= SUSPEND_REASON_USER_ACTIVE;
    }

    if (!now_between_two_hours(global_prefs.start_hour, global_prefs.end_hour)) {
        reason |= SUSPEND_REASON_TIME_OF_DAY;
    }

    return;
}

int CLIENT_STATE::suspend_activities(int reason) {
    string s_reason;
    s_reason = "Suspending computation and network activity";
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
    msg_printf(NULL, MSG_INFO, const_cast<char*>(s_reason.c_str()));
    active_tasks.suspend_all();
    pers_file_xfers->suspend();
    return 0;
}

// persistent file xfers will resume of their own accord
// since activities_suspended is now true
//
int CLIENT_STATE::resume_activities() {
    msg_printf(NULL, MSG_INFO, "Resuming computation and network activity");
    active_tasks.unsuspend_all();
    return 0;
}

void CLIENT_STATE::check_suspend_network(int& reason) {
    reason = 0;

    if (user_network_request == USER_RUN_REQUEST_ALWAYS) return;
    if (user_network_request == USER_RUN_REQUEST_NEVER) {
        reason |= SUSPEND_REASON_USER_REQ;
        return;
    }
    return;
}

int CLIENT_STATE::suspend_network(int reason) {
    string s_reason;
    s_reason = "Suspending network activity";
    if (reason & SUSPEND_REASON_USER_REQ) {
        s_reason += " - user request";
    }
    msg_printf(NULL, MSG_INFO, const_cast<char*>(s_reason.c_str()));
    pers_file_xfers->suspend();
    return 0;
}

int CLIENT_STATE::resume_network() {
    msg_printf(NULL, MSG_INFO, "Resuming network activity");
    return 0;
}

void CLIENT_STATE::show_global_prefs_source(bool found_venue) {
    PROJECT* pp = lookup_project(global_prefs.source_project.c_str());
    if (pp) {
        msg_printf(NULL, MSG_INFO,
            "General prefs: from %s (last modified %s)\n",
            pp->get_project_name(), time_to_string(global_prefs.mod_time)
        );
    } else {
        msg_printf(NULL, MSG_INFO,
            "General prefs: from unknown project %s (last modified %s)\n",
            global_prefs.source_project.c_str(),
            time_to_string(global_prefs.mod_time)
        );
    }
    if (strlen(host_venue)) {
        if (found_venue) {
            msg_printf(NULL, MSG_INFO,
                "General prefs: using separate prefs for %s\n", host_venue
            );
        } else {
            msg_printf(NULL, MSG_INFO,
                "General prefs: no separate prefs for %s; using your defaults\n",
                host_venue
            );
        }
    } else {
        msg_printf(NULL, MSG_INFO, "General prefs: using your defaults\n");
    }
}
