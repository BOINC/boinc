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

#include <math.h>

#include "error_numbers.h"
#include "str_util.h"
#include "util.h"
#include "str_replace.h"
#include "log_flags.h"
#include "filesys.h"
#include "client_msgs.h"
#include "client_state.h"
#include "sim.h"

//////////////// FUNCTIONS MODIFIED OR STUBBED OUT /////////////

void HOST_INFO::generate_host_cpid() {}

//int get_connected_state() {return 1;}

void show_message(PROJECT *p, char* msg, int priority, const char*) {
    const char* x;
    char message[1024];

    if (priority == MSG_INTERNAL_ERROR) {
        strcpy(message, "[error] ");
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

    fprintf(logfile, "%.0f [%s] %s\n", gstate.now, x, message);
}

APP_CLIENT_SHM::APP_CLIENT_SHM() {}
GRAPHICS_MSG::GRAPHICS_MSG() {}


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
            first_time?"Starting":"Resuming", result->name, flops_left/1e9
        );
    }
    set_task_state(PROCESS_EXECUTING, "start");
    char buf[256];
    sprintf(buf, "Starting %s: %.2fG<br>", result->name, flops_left/1e9);
    gstate.html_msg += buf;
    return 0;
}

int ACTIVE_TASK::init(RESULT* rp) {
    result = rp;
    wup = rp->wup;
    app_version = rp->avp;
    max_elapsed_time = rp->wup->rsc_fpops_bound/result->avp->flops;
    max_disk_usage = rp->wup->rsc_disk_bound;
    max_mem_usage = rp->wup->rsc_memory_bound;
    flops_left = rp->wup->rsc_fpops_est;
    _task_state = PROCESS_UNINITIALIZED;
    scheduler_state = CPU_SCHED_UNINITIALIZED;
    return 0;
}

void CLIENT_STATE::print_project_results(FILE* f) {
    for (unsigned int i=0; i<projects.size(); i++) {
        PROJECT* p = projects[i];
        p->print_results(f, sim_results);
    }
}


//////////////// OTHER

// http://www.cs.wm.edu/~va/software/park/rvgs.c
double NORMAL_DIST::sample() {
  const double p0 = 0.322232431088;     const double q0 = 0.099348462606;
  const double p1 = 1.0;                const double q1 = 0.588581570495;
  const double p2 = 0.342242088547;     const double q2 = 0.531103462366;
  const double p3 = 0.204231210245e-1;  const double q3 = 0.103537752850;
  const double p4 = 0.453642210148e-4;  const double q4 = 0.385607006340e-2;
  double u, t, p, q, z;

  u   = drand();
  if (u < 0.5)
    t = sqrt(-2.0 * log(u));
  else
    t = sqrt(-2.0 * log(1.0 - u));
  p   = p0 + t * (p1 + t * (p2 + t * (p3 + t * p4)));
  q   = q0 + t * (q1 + t * (q2 + t * (q3 + t * q4)));
  if (u < 0.5)
    z = (p / q) - t;
  else
    z = t - (p / q);
  return (mean + stdev * z);

}

inline double exponential(double mean) {
    return -mean*log(1-drand());
}

bool RANDOM_PROCESS::sample(double t) {
    if (frac==1) return true;
    double diff = t-last_time;
    last_time = t;
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
    msg_printf(0, MSG_INFO,
        "value: %d lambda: %f t %f time_left %f",
        value, lambda, t, time_left
    );
    return value;
}

RANDOM_PROCESS::RANDOM_PROCESS() {
    frac = 1;
}

void RANDOM_PROCESS::init(double st) {
    last_time = st;
    value = true;
    time_left = exponential(lambda);
    off_lambda = lambda/frac - lambda;
}

int NORMAL_DIST::parse(XML_PARSER& xp, const char* end_tag) {
    char tag[256];
    bool is_tag;
    while(!xp.get(tag, sizeof(tag), is_tag)) {
        if (!is_tag) return ERR_XML_PARSE;
        if (xp.parse_double(tag, "mean", mean)) continue;
        else if (xp.parse_double(tag, "stdev", stdev)) continue;
        else if (!strcmp(tag, end_tag)) return 0;
        else {
            printf("unrecognized: %s\n", tag);
            return ERR_XML_PARSE;
        }
    }
    return ERR_XML_PARSE;
}

int UNIFORM_DIST::parse(XML_PARSER& xp, const char* end_tag) {
    char tag[256];
    bool is_tag;
    while(!xp.get(tag, sizeof(tag), is_tag)) {
        if (!is_tag) return ERR_XML_PARSE;
        if (xp.parse_double(tag, "lo", lo)) continue;
        else if (xp.parse_double(tag, "hi", hi)) continue;
        else if (!strcmp(tag, end_tag)) return 0;
        else {
            printf("unrecognized: %s\n", tag);
            return ERR_XML_PARSE;
        }
    }
    return ERR_XML_PARSE;
}

int RANDOM_PROCESS::parse(XML_PARSER& xp, const char* end_tag) {
    char tag[256];
    bool is_tag;
    while(!xp.get(tag, sizeof(tag), is_tag)) {
        if (!is_tag) return ERR_XML_PARSE;
        if (xp.parse_double(tag, "frac", frac)) continue;
        else if (xp.parse_double(tag, "lambda", lambda)) continue;
        else if (!strcmp(tag, end_tag)) return 0;
        else {
            printf("unrecognized: %s\n", tag);
            return ERR_XML_PARSE;
        }
    }
    return ERR_XML_PARSE;
}

