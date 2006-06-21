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
#include "error_numbers.h"
#include "client_msgs.h"
#include "client_state.h"
#include "network.h"
#include "log_flags.h"

#include "time_stats.h"

// exponential decay constant.
// The last 30 days have a weight of 1/e;
// everything before that has a weight of (1-1/e)

const float ALPHA = (SECONDS_PER_DAY*30);
//const float ALPHA = 60;   // for testing

TIME_STATS::TIME_STATS() {
    last_update = 0;
    first = true;
    on_frac = 1;
    connected_frac = 1;
    active_frac = 1;
    cpu_efficiency = 1;
}

// Update time statistics based on current activities
// NOTE: we don't set the state-file dirty flag here,
// so these get written to disk only when other activities
// cause this to happen.  Maybe should change this.
//
void TIME_STATS::update(bool is_active) {
    double dt, w1, w2;

    if (last_update == 0) {
        // this is the first time this client has executed.
        // Assume that everything is active

        on_frac = 1;
        connected_frac = 1;
        active_frac = 1;
        first = false;
        last_update = gstate.now;
    } else {
        dt = gstate.now - last_update;
        if (dt <= 10) return;
        w1 = 1 - exp(-dt/ALPHA);    // weight for recent period
        w2 = 1 - w1;                // weight for everything before that
                                    // (close to zero if long gap)
        int connected_state = get_connected_state();
        if (first) {
            // the client has just started; this is the first call.
            //
            on_frac *= w2;
            first = false;
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
            active_frac *= w2;
            if (is_active) active_frac += w1;
        }
        last_update = gstate.now;
    }
#if 0
    msg_printf(0, MSG_INFO, "on %f; active %f; conn %f",
        on_frac, active_frac, connected_frac
    );
#endif
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
            "CPU efficiency old %f new %f wall %f CPU %f w %f e %f",
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
    out.printf("</time_stats>\n");
    return 0;
}

// Parse XML based time statistics, usually from client_state.xml
//
int TIME_STATS::parse(MIOFILE& in) {
    char buf[256];

    SCOPE_MSG_LOG scope_messages(log_messages, CLIENT_MSG_LOG::DEBUG_STATE);

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
        }
        else scope_messages.printf("TIME_STATS::parse(): unrecognized: %s\n", buf);
    }
    return ERR_XML_PARSE;
}

const char *BOINC_RCSID_472504d8c2 = "$Id$";
