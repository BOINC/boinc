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
#endif

#ifndef _WIN32
#include "config.h"
#include <cstdio>
#include <ctime>
#include <cmath>
#endif

#ifdef HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif

#include "parse.h"
#include "util.h"
#include "filesys.h"
#include "error_numbers.h"
#include "client_msgs.h"
#include "file_names.h"
#ifdef SIM
#include "sim.h"
#else
#include "client_state.h"
#endif
#include "network.h"
#include "log_flags.h"

#include "time_stats.h"

// exponential decay constant.
// The last 10 days have a weight of 1/e;
// everything before that has a weight of (1-1/e)

const float ALPHA = (SECONDS_PER_DAY*10);
//const float ALPHA = 60;   // for testing

TIME_STATS::TIME_STATS() {
    last_update = 0;
    first = true;
    on_frac = 1;
    connected_frac = 1;
    active_frac = 1;
    cpu_efficiency = 1;
    previous_connected_state = CONNECTED_STATE_UNINITIALIZED;
    inactive_start = 0;
    trim_stats_log();
    time_stats_log = fopen(TIME_STATS_LOG, "a");
    if (time_stats_log) {
        setbuf(time_stats_log, 0);
    }
}

// if log file is over a meg, discard everything older than a year
//
void TIME_STATS::trim_stats_log() {
    double size;
    char buf[256];
    int retval;
    double x;

    retval = file_size(TIME_STATS_LOG, size);
    if (retval) return;
    if (size < 1e6) return;
    FILE* f = fopen(TIME_STATS_LOG, "r");
    FILE* f2 = fopen(TEMP_FILE_NAME, "w");
    if (!f || !f2) return;
    while (fgets(buf, 256, f)) {
        int n = sscanf(buf, "%lf", &x);
        if (n != 1) continue;
        if (x < gstate.now-86400*365) continue;
        fputs(buf, f2);
    }
    fclose(f);
    fclose(f2);
}

// copy the log file after a given time
//
void TIME_STATS::get_log_after(double t, MIOFILE& mf) {
    double x;
    char buf[256];

    fclose(time_stats_log);     // win: can't open twice
    FILE* f = fopen(TIME_STATS_LOG, "r");
    if (!f) return;

    while (fgets(buf, 256, f)) {
        int n = sscanf(buf, "%lf", &x);
        if (n != 1) continue;
        if (x < t) continue;
        mf.printf("%s", buf);
    }
    fclose(f);
    time_stats_log = fopen(TIME_STATS_LOG, "a");
}

// Update time statistics based on current activities
// NOTE: we don't set the state-file dirty flag here,
// so these get written to disk only when other activities
// cause this to happen.  Maybe should change this.
//
void TIME_STATS::update(int suspend_reason) {
    double dt, w1, w2;

    bool is_active = !(suspend_reason & ~SUSPEND_REASON_CPU_USAGE_LIMIT);
    if (last_update == 0) {
        // this is the first time this client has executed.
        // Assume that everything is active

        on_frac = 1;
        connected_frac = 1;
        active_frac = 1;
        first = false;
        last_update = gstate.now;
        log_append("power_on", gstate.now);
    } else {
        dt = gstate.now - last_update;
        if (dt <= 10) return;
        w1 = 1 - exp(-dt/ALPHA);    // weight for recent period
        w2 = 1 - w1;                // weight for everything before that
                                    // (close to zero if long gap)

        // the following always returns UNKNOWN,
        // but leave it here for now
        //
        int connected_state = get_connected_state();
#ifndef SIM
        if (gstate.network_suspend_reason) {
            connected_state = CONNECTED_STATE_NOT_CONNECTED;
        }
#endif

        if (first) {
            // the client has just started; this is the first call.
            //
            on_frac *= w2;
            first = false;
            log_append("power_off", last_update);
            log_append("power_on", gstate.now);
        } else {
            on_frac = w1 + w2*on_frac;
            if (connected_frac < 0) connected_frac = 0;
            switch (connected_state) {
            case CONNECTED_STATE_NOT_CONNECTED:
                connected_frac *= w2;
                break;
            case CONNECTED_STATE_CONNECTED:
                connected_frac *= w2;
                connected_frac += w1;
                break;
            case CONNECTED_STATE_UNKNOWN:
                connected_frac = -1;
            }
            if (connected_state != previous_connected_state) {
                log_append_net(connected_state);
                previous_connected_state = connected_state;
            }
            active_frac *= w2;
            if (is_active) {
                active_frac += w1;
                if (inactive_start) {
                    inactive_start = 0;
                    log_append("proc_start", gstate.now);
                }
            } else if (inactive_start == 0){
                inactive_start = gstate.now;
                log_append("proc_stop", gstate.now);
            }
        }
        last_update = gstate.now;
        if (log_flags.time_debug) {
            msg_printf(0, MSG_INFO,
                "[time_debug] dt %f w2 %f on %f; active %f; conn %f",
                dt, w2, on_frac, active_frac, connected_frac
            );
        }
    }
}

void TIME_STATS::update_cpu_efficiency(double cpu_wall_time, double cpu_time) {
    double old_cpu_efficiency = cpu_efficiency;
    if (cpu_wall_time < .01) return;
    double w = exp(-cpu_wall_time/SECONDS_PER_DAY);
    double e = cpu_time/cpu_wall_time;
    if (e<0) {
        return;
    }
    cpu_efficiency = w*cpu_efficiency + (1-w)*e;
    if (log_flags.cpu_sched_debug){
        msg_printf(0, MSG_INFO,
            "[cpu_sched_debug] CPU efficiency old %f new %f wall %f CPU %f w %f e %f",
            old_cpu_efficiency, cpu_efficiency, cpu_wall_time,
            cpu_time, w, e
        );
    }
}

// Write XML based time statistics
//
int TIME_STATS::write(MIOFILE& out, bool to_server) {
    out.printf(
        "<time_stats>\n"
        "    <on_frac>%f</on_frac>\n"
        "    <connected_frac>%f</connected_frac>\n"
        "    <active_frac>%f</active_frac>\n"
        "    <cpu_efficiency>%f</cpu_efficiency>\n",
        on_frac,
        connected_frac,
        active_frac,
        cpu_efficiency
    );
    if (!to_server) {
        out.printf(
            "    <last_update>%f</last_update>\n",
            last_update
        );
    }
#if 0
    // too much text.  Maybe just send the longest outage?
    //
    if (outages.size()) {
        out.printf("    <outages>\n");
        unsigned int i;
        for (i=0; i<outages.size(); i++) {
            if (outages[i].is_recent()) {
                out.printf(
                    "        <outage>\n"
                    "            <start>%f</start>\n"
                    "            <end>%f</end>\n"
                    "        </outage>\n",
                    outages[i].start,
                    outages[i].end
                );
            }
        }
        out.printf("    </outages>\n");
    }
#endif
    out.printf("</time_stats>\n");
    return 0;
}

// Parse XML based time statistics, usually from client_state.xml
//
int TIME_STATS::parse(MIOFILE& in) {
    char buf[256];

    while (in.fgets(buf, 256)) {
        if (match_tag(buf, "</time_stats>")) return 0;
        else if (parse_double(buf, "<last_update>", last_update)) continue;
        else if (parse_double(buf, "<on_frac>", on_frac)) continue;
        else if (parse_double(buf, "<connected_frac>", connected_frac)) continue;
        else if (parse_double(buf, "<active_frac>", active_frac)) continue;
        else if (parse_double(buf, "<cpu_efficiency>", cpu_efficiency)) {
            if (cpu_efficiency < 0) cpu_efficiency = 1;
            if (cpu_efficiency > 1) cpu_efficiency = 1;
            continue;
        } else {
            if (log_flags.unparsed_xml) {
                msg_printf(0, MSG_INFO,
                    "[unparsed_xml] TIME_STATS::parse(): unrecognized: %s\n", buf
                );
            }
        }
    }
    return ERR_XML_PARSE;
}

void TIME_STATS::quit() {
    log_append("power_off", gstate.now);
}

void TIME_STATS::log_append(const char* msg, double t) {
#ifndef SIM
    if (!time_stats_log) return;
    fprintf(time_stats_log, "%f %s\n", t, msg);
#endif
}

void TIME_STATS::log_append_net(int new_state) {
    switch(new_state) {
    case CONNECTED_STATE_NOT_CONNECTED:
        log_append("net_not_connected", gstate.now);
        break;
    case CONNECTED_STATE_CONNECTED:
        log_append("net_connected", gstate.now);
        break;
    case CONNECTED_STATE_UNKNOWN:
        log_append("net_unknown", gstate.now);
        break;
    }
}

const char *BOINC_RCSID_472504d8c2 = "$Id$";
