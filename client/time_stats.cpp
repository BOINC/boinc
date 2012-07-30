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

#include "cpp.h"

#ifdef _WIN32
#include "boinc_win.h"
#else
#include "config.h"
#include <cstdio>
#include <ctime>
#include <cmath>
#include <cstring>
#if HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif
#endif

#include "error_numbers.h"
#include "filesys.h"
#include "parse.h"
#include "util.h"

#include "client_msgs.h"
#include "client_state.h"
#include "file_names.h"
#include "log_flags.h"
#include "network.h"
#ifdef SIM
#include "sim.h"
#endif

#include "time_stats.h"

#define CONNECTED_STATE_UNINITIALIZED   -1
#define CONNECTED_STATE_NOT_CONNECTED   0
#define CONNECTED_STATE_CONNECTED       1
#define CONNECTED_STATE_UNKNOWN         2

#ifndef SIM
#ifdef _WIN32
#include <sensapi.h>

int get_connected_state() {
    DWORD flags;
    return IsNetworkAlive(&flags)?CONNECTED_STATE_CONNECTED:CONNECTED_STATE_NOT_CONNECTED;
}
#else

// anyone know how to see if this host has physical network connection?
//
int get_connected_state() {
    return CONNECTED_STATE_UNKNOWN;
}
#endif
#endif

// exponential decay constant.
// The last 10 days have a weight of 1/e;
// everything before that has a weight of (1-1/e)

const float ALPHA = (SECONDS_PER_DAY*10);
//const float ALPHA = 60;   // for testing

void TIME_STATS::init() {
    last_update = 0;
    first = true;
    on_frac = 1;
    connected_frac = 1;
    active_frac = 1;
    gpu_active_frac = 1;
    cpu_and_network_available_frac = 1;
    previous_connected_state = CONNECTED_STATE_UNINITIALIZED;
    inactive_start = 0;
    trim_stats_log();
    time_stats_log = NULL;
}

// if log file is over a meg, discard everything older than a year
//
void TIME_STATS::trim_stats_log() {
#ifndef SIM
    double size;
    char buf[256];
    int retval;
    double x;

    retval = file_size(TIME_STATS_LOG, size);
    if (retval) return;
    if (size < 1e6) return;
    FILE* f = fopen(TIME_STATS_LOG, "r");
    if (!f) return;
    FILE* f2 = fopen(TEMP_TIME_STATS_FILE_NAME, "w");
    if (!f2) {
        fclose(f);
        return;
    }
    while (fgets(buf, 256, f)) {
        int n = sscanf(buf, "%lf", &x);
        if (n != 1) continue;
        if (x < gstate.now-86400*365) continue;
        fputs(buf, f2);
    }
    fclose(f);
    fclose(f2);
#endif
}

void send_log_after(const char* filename, double t, MIOFILE& mf) {
    char buf[256];
    double x;

    FILE* f = fopen(filename, "r");
    if (!f) return;
    while (fgets(buf, 256, f)) {
        int n = sscanf(buf, "%lf", &x);
        if (n != 1) continue;
        if (x < t) continue;
        mf.printf("%s", buf);
    }
    fclose(f);
}

// copy the log file after a given time
//
void TIME_STATS::get_log_after(double t, MIOFILE& mf) {
    if (time_stats_log) {
        fclose(time_stats_log);     // win: can't open twice
    }
    send_log_after(TIME_STATS_LOG, t, mf);
    time_stats_log = fopen(TIME_STATS_LOG, "a");
}

// Update time statistics based on current activities
// NOTE: we don't set the state-file dirty flag here,
// so these get written to disk only when other activities
// cause this to happen.  Maybe should change this.
//
void TIME_STATS::update(int suspend_reason, int _gpu_suspend_reason) {
    double dt, w1, w2;

    bool is_active = !(suspend_reason & ~SUSPEND_REASON_CPU_THROTTLE);
    bool is_gpu_active = is_active && !_gpu_suspend_reason;
    if (last_update == 0) {
        // this is the first time this client has executed.
        // Assume that everything is active

        on_frac = 1;
        connected_frac = 1;
        active_frac = 1;
        gpu_active_frac = 1;
        cpu_and_network_available_frac = 1;
        first = false;
        last_update = gstate.now;
        log_append("power_on", gstate.now);
    } else {
        dt = gstate.now - last_update;
        if (dt <= 10) return;

        if (dt > 14*86400) {
            // If dt is large it could be because user is upgrading
            // from a client version that wasn't updating due to bug.
            // Or it could be because user wasn't running for a while
            // and is starting up again.
            // In either case, don't decay on_frac.
            //
            dt = 0;
        }

        w1 = 1 - exp(-dt/ALPHA);    // weight for recent period
        w2 = 1 - w1;                // weight for everything before that
                                    // (close to zero if long gap)

        int connected_state;
#ifdef SIM
        connected_state = CONNECTED_STATE_NOT_CONNECTED;
#else
        connected_state = get_connected_state();
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
            char buf[256];
#ifndef SIM
            sprintf(buf, "platform %s", gstate.get_primary_platform());
            log_append(buf, gstate.now);
#endif
            sprintf(buf, "version %d.%d.%d",
                BOINC_MAJOR_VERSION, BOINC_MINOR_VERSION, BOINC_RELEASE
            );
            log_append(buf, gstate.now);
            log_append("power_on", gstate.now);
        } else if (dt > 100) {
            // large dt - the client or host must have been suspended
            on_frac *= w2;
            log_append("proc_stop", last_update);
            if (is_active) {
                log_append("proc_start", gstate.now);
            }
        } else {
            on_frac = w1 + w2*on_frac;
            cpu_and_network_available_frac *= w2;
            if (connected_frac < 0) connected_frac = 0;
            switch (connected_state) {
            case CONNECTED_STATE_NOT_CONNECTED:
                connected_frac *= w2;
                break;
            case CONNECTED_STATE_CONNECTED:
                connected_frac *= w2;
                connected_frac += w1;
                if (!gstate.network_suspended && !gstate.tasks_suspended) {
                    cpu_and_network_available_frac += w1;
                }
                break;
            case CONNECTED_STATE_UNKNOWN:
                connected_frac = -1;
                if (!gstate.network_suspended && !gstate.tasks_suspended) {
                    cpu_and_network_available_frac += w1;
                }
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

            gpu_active_frac *= w2;
            if (is_gpu_active) {
                gpu_active_frac += w1;
            }

            //msg_printf(NULL, MSG_INFO, "is_active %d, active_frac %f", is_active, active_frac);
        }
        last_update = gstate.now;
        if (log_flags.time_debug) {
            msg_printf(0, MSG_INFO,
                "[time] dt %f w2 %f on %f; active %f; gpu_active %f; conn %f, cpu_and_net_avail %f",
                dt, w2, on_frac, active_frac, gpu_active_frac, connected_frac,
                cpu_and_network_available_frac
            );
        }
    }
}

// Write XML based time statistics
//
int TIME_STATS::write(MIOFILE& out, bool to_server) {
    out.printf(
        "<time_stats>\n"
        "    <on_frac>%f</on_frac>\n"
        "    <connected_frac>%f</connected_frac>\n"
        "    <cpu_and_network_available_frac>%f</cpu_and_network_available_frac>\n"
        "    <active_frac>%f</active_frac>\n"
        "    <gpu_active_frac>%f</gpu_active_frac>\n",
        on_frac,
        connected_frac,
        cpu_and_network_available_frac,
        active_frac,
        gpu_active_frac
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
int TIME_STATS::parse(XML_PARSER& xp) {
    double x;
#ifdef SIM
    double on_lambda = 3600, connected_lambda = 3600;
    double active_lambda = 3600, gpu_active_lambda = 3600;
#endif

    while (!xp.get_tag()) {
        if (xp.match_tag("/time_stats")) {
#ifdef SIM
            on_proc.init(on_frac, on_lambda);
            connected_proc.init(connected_frac, connected_lambda);
            active_proc.init(active_frac, active_lambda);
            gpu_active_proc.init(gpu_active_frac, gpu_active_lambda);
#endif
            return 0;
        }
#ifdef SIM
        else if (xp.parse_double("on_lambda", on_lambda)) continue;
        else if (xp.parse_double("connected_lambda", connected_lambda)) continue;
        else if (xp.parse_double("active_lambda", active_lambda)) continue;
        else if (xp.parse_double("gpu_active_lambda", gpu_active_lambda)) continue;
#endif
        else if (xp.parse_double("last_update", x)) {
            if (x < 0 || x > gstate.now) {
#ifndef SIM
                msg_printf(0, MSG_INTERNAL_ERROR,
                    "bad value %f of time stats last update; ignoring", x
                );
#endif
            } else {
                last_update = x;
            }
            continue;
        } else if (xp.parse_double("on_frac", x)) {
            if (x <= 0 || x > 1) {
                msg_printf(0, MSG_INTERNAL_ERROR,
                    "bad value %f of time stats on_frac; ignoring", x
                );
            } else {
                on_frac = x;
            }
            continue;
        } else if (xp.parse_double("connected_frac", x)) {
            // -1 means undefined; skip check
            connected_frac = x;
            continue;
        } else if (xp.parse_double("cpu_and_network_available_frac", x)) {
            if (x <= 0 || x > 1) {
                msg_printf(0, MSG_INTERNAL_ERROR,
                    "bad value %f of time stats cpu_and_network_available_frac; ignoring", x
                );
            } else {
                cpu_and_network_available_frac = x;
            }
        } else if (xp.parse_double("active_frac", x)) {
            if (x <= 0 || x > 1) {
                msg_printf(0, MSG_INTERNAL_ERROR,
                    "bad value %f of time stats active_frac; ignoring", x
                );
            } else {
                active_frac = x;
            }
            continue;
        } else if (xp.parse_double("gpu_active_frac", x)) {
            if (x <= 0 || x > 1) {
                msg_printf(0, MSG_INTERNAL_ERROR,
                    "bad value %f of time stats gpu_active_frac; ignoring", x
                );
            } else {
                gpu_active_frac = x;
            }
            continue;
        } else {
            if (log_flags.unparsed_xml) {
                msg_printf(0, MSG_INFO,
                    "[unparsed_xml] TIME_STATS::parse(): unrecognized: %s\n",
                    xp.parsed_tag
                );
            }
        }
    }
    return ERR_XML_PARSE;
}

void TIME_STATS::start() {
    time_stats_log = fopen(TIME_STATS_LOG, "a");
    if (time_stats_log) {
        setbuf(time_stats_log, 0);
    }
}

void TIME_STATS::quit() {
    log_append("power_off", gstate.now);
}

#ifdef SIM
void TIME_STATS::log_append(const char* , double ) {}
#else
void TIME_STATS::log_append(const char* msg, double t) {
    if (!time_stats_log) return;
    fprintf(time_stats_log, "%f %s\n", t, msg);
}
#endif

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

