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

// BOINC client simulator.
//
// usage:
// sim [--duration x] [--delta x] [--dirs dir ...]
//  duration = simulation duration (default 86400)
//  delta = simulation time step (default 10)
//
// If no dirs are specified:
// reads input files
//    sim_projects.xml, sim_host.xml, sim_prefs.xml, cc_config.xml
// and does simulation, generating output files
//    sim_log.txt, sim_out.html
//
// If dirs are specified, chdir into each directory in sequence,
// do the above for each one, and write summary info to stdout

#ifdef _MSC_VER
#define finite _finite
#endif

#include <cmath>

#include "error_numbers.h"
#include "filesys.h"
#include "str_replace.h"
#include "str_util.h"
#include "util.h"

#include "client_msgs.h"
#include "client_state.h"
#include "log_flags.h"
#include "result.h"
#include "sim.h"

//////////////// FUNCTIONS MODIFIED OR STUBBED OUT /////////////

void HOST_INFO::generate_host_cpid() {}

//int get_connected_state() {return 1;}

char* sim_time_string(int t) {
    static char buf[256];
    char buf2[256];
    if (t > 86400) {
        int n = t/86400;
        t %= 86400;
        if (n == 1) {
            snprintf(buf2, sizeof(buf2), "1 day ");
        } else {
            snprintf(buf2, sizeof(buf2), "%d days ", n);
        }
    } else {
        safe_strcpy(buf2, "");
    }
    int hours = t / 3600;
    t %= 3600;
    int mins = t/60;
    int secs = t%60;
    snprintf(buf, sizeof(buf), "%s%02d:%02d:%02d", buf2, hours, mins, secs);
    return buf;
}

void show_message(PROJ_AM *p, char* msg, int priority, bool, const char*) {
    const char* x;
    char message[1024];

    if (priority == MSG_INTERNAL_ERROR) {
        strlcpy(message, "[error] ", sizeof(message));
        strlcpy(message+8, msg, sizeof(message)-8);
    } else {
        strlcpy(message, msg, sizeof(message));
    }
    while (strlen(message)&&message[strlen(message)-1] == '\n') {
        message[strlen(message)-1] = 0;
    }

    if (p) {
        x = p->get_project_name();
    } else {
        x = "---";
    }

    fprintf(logfile, "%s [%s] %s\n", sim_time_string(gstate.now), x, message);
}

APP_CLIENT_SHM::APP_CLIENT_SHM() {}
int FILE_INFO::verify_file(bool, bool, bool) {return 0;}


//////////////// FUNCTIONS WE NEED TO IMPLEMENT /////////////

int ACTIVE_TASK::suspend() {
    if (task_state() != PROCESS_EXECUTING) {
        msg_printf(0, MSG_INFO, "Internal error: expected process to be executing");
    }
    set_task_state(PROCESS_SUSPENDED, "suspend");
    return 0;
}

int ACTIVE_TASK::request_exit() {
    set_task_state(PROCESS_UNINITIALIZED, "request_exit");
    return 0;
}

int ACTIVE_TASK::resume_or_start(bool first_time) {
    if (log_flags.task) {
        msg_printf(result->project, MSG_INFO,
            "[task] %s task %s: FLOPS left %.2fG",
            first_time?"Starting":"Resuming",
            result->name, result->sim_flops_left/1e9
        );
    }
    set_task_state(PROCESS_EXECUTING, "start");
    char buf[1024];
    snprintf(buf, sizeof(buf), "Starting %s<br>&nbsp;&nbsp;%s<br>&nbsp;&nbsp;deadline %s<br>",
        result->name, result->project->get_project_name(),
        sim_time_string(result->report_deadline)
    );
    html_msg += buf;
    return 0;
}

int ACTIVE_TASK::init(RESULT* rp) {
    result = rp;
    wup = rp->wup;
    app_version = rp->avp;
    max_elapsed_time = rp->wup->rsc_fpops_bound/result->resource_usage.flops;
    max_disk_usage = rp->wup->rsc_disk_bound;
    max_mem_usage = rp->wup->rsc_memory_bound;
    _task_state = PROCESS_UNINITIALIZED;
    scheduler_state = CPU_SCHED_UNINITIALIZED;
    return 0;
}

//////////////// OTHER

double NORMAL_DIST::sample() {
    if (!std_dev) return mean;
    return (mean + std_dev * rand_normal());
}

inline double exponential(double mean) {
    return -mean*log(1-drand());
}

bool RANDOM_PROCESS::sample(double diff) {
    if (frac==1) return true;
    time_left -= diff;
    if (time_left < 0) {
        if (value) {
            time_left += exponential(off_lambda);
            value = false;
        } else {
            time_left += exponential(lambda);
            value = true;
        }
    }
#if 0
    msg_printf(0, MSG_INFO,
        "value: %d lambda: %f time_left %f", value, lambda, time_left
    );
#endif
    return value;
}

RANDOM_PROCESS::RANDOM_PROCESS() {
    frac = 1;
    last_time = 0;
}

void RANDOM_PROCESS::init(double f, double l) {
    frac = f;
    lambda = l;
    last_time = 0;
    off_lambda = lambda/frac - lambda;
    if (drand() > frac) {
        value = false;
        time_left = exponential(off_lambda);
    } else {
        value = true;
        time_left = exponential(lambda);
    }
}

int RANDOM_PROCESS::parse(XML_PARSER& xp, const char* end_tag) {
    while (!xp.get_tag()) {
        if (!xp.is_tag) return ERR_XML_PARSE;
        if (xp.parse_double("lambda", lambda)) continue;
        else if (xp.parse_double("frac", frac)) continue;
        else if (xp.match_tag(end_tag)) {
            init(frac, lambda);
            return 0;
        } else {
            printf("unrecognized: %s\n", xp.parsed_tag);
            return ERR_XML_PARSE;
        }
    }
    return ERR_XML_PARSE;
}

int UNIFORM_DIST::parse(XML_PARSER& xp, const char* end_tag) {
    while (!xp.get_tag()) {
        if (!xp.is_tag) return ERR_XML_PARSE;
        if (xp.parse_double("lo", lo)) continue;
        else if (xp.parse_double("hi", hi)) continue;
        else if (xp.match_tag(end_tag)) return 0;
        else {
            printf("unrecognized: %s\n", xp.parsed_tag);
            return ERR_XML_PARSE;
        }
    }
    return ERR_XML_PARSE;
}

int NORMAL_DIST::parse(XML_PARSER& xp, const char* end_tag) {
    while(!xp.get_tag()) {
        if (!xp.is_tag) return ERR_XML_PARSE;
        if (xp.parse_double("mean", mean)) continue;
        else if (xp.parse_double("std_dev", std_dev)) continue;
        else if (xp.match_tag(end_tag)) return 0;
        else {
            printf("unrecognized: %s\n", xp.parsed_tag);
            return ERR_XML_PARSE;
        }
    }
    return ERR_XML_PARSE;
}
