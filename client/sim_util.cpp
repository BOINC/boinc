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
#include "str_replace.h"
#include "log_flags.h"
#include "filesys.h"
#include "network.h"
#include "client_msgs.h"
#include "sim.h"

//////////////// FUNCTIONS MODIFIED OR STUBBED OUT /////////////

double dtime() {
    return 0;
}

CLIENT_STATE::CLIENT_STATE() {
    initialized = false;
    retry_shmem_time = 0;
}

FILE* boinc_fopen(const char* path, const char* mode) {
    return fopen(path, mode);
}

void CLIENT_STATE::set_client_state_dirty(char const*) {
}

void HOST_INFO::generate_host_cpid() {}

//int get_connected_state() {return 1;}

int CLIENT_STATE::report_result_error(RESULT& , const char* , ...) {return 0;}

void show_message(PROJECT *p, char* msg, int priority) {
    const char* x;
    char message[1024];
    char* time_string = time_to_string(gstate.now);

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

    fprintf(logfile, "%s [%s] %s\n", time_string, x, message);
}
bool RESULT::some_download_stalled() {
    return false;
}
bool PROJECT::some_download_stalled() {
    return false;
}

APP_CLIENT_SHM::APP_CLIENT_SHM() {}
GRAPHICS_MSG::GRAPHICS_MSG() {}
HOST_INFO::HOST_INFO() {}

//////////////// FUNCTIONS COPIED /////////////

void SIM_PROJECT::init() {
    strcpy(master_url, "");
    strcpy(authenticator, "");
    project_specific_prefs = "";
    gui_urls = "";
    resource_share = 100;
    strcpy(host_venue, "");
    using_venue_specific_prefs = false;
    scheduler_urls.clear();
    strcpy(project_name, "");
    strcpy(symstore, "");
    strcpy(user_name, "");
    strcpy(team_name, "");
    strcpy(email_hash, "");
    strcpy(cross_project_id, "");
    user_total_credit = 0;
    user_expavg_credit = 0;
    user_create_time = 0;
    ams_resource_share = 0;
    rpc_seqno = 0;
    hostid = 0;
    host_total_credit = 0;
    host_expavg_credit = 0;
    host_create_time = 0;
    nrpc_failures = 0;
    master_fetch_failures = 0;
    min_rpc_time = 0;
    possibly_backed_off = true;
    master_url_fetch_pending = false;
    sched_rpc_pending = 0;
    next_rpc_time = 0;
    last_rpc_time = 0;
    trickle_up_pending = false;
    anonymous_platform = false;
    non_cpu_intensive = false;
    verify_files_on_app_start = false;
    send_file_list = false;
    suspended_via_gui = false;
    dont_request_more_work = false;
    detach_when_done = false;
    attached_via_acct_mgr = false;
    strcpy(code_sign_key, "");
    user_files.clear();
    project_files.clear();
    next_runnable_result = NULL;
    duration_correction_factor = 1;
    project_files_downloaded_time = 0;

    // Initialize scratch variables.
    rr_sim_status.clear();

    // sim-specific:
    idle_time = 0;
    idle_time_sumsq = 0;
    completed_task_count = 0;
    completions_ratio_mean = 0.0;
    completions_ratio_s = 0.0;
    completions_ratio_stdev = 0.1;  // for the first couple of completions - guess.
    completions_required_stdevs = 3.0;
}

void RESULT::clear() {
    strcpy(name, "");
    strcpy(wu_name, "");
    report_deadline = 0;
    output_files.clear();
    _state = RESULT_NEW;
    ready_to_report = false;
    completed_time = 0;
    got_server_ack = false;
    final_cpu_time = 0;
    exit_status = 0;
    stderr_out = "";
    suspended_via_gui = false;
    rr_sim_misses_deadline = false;
    fpops_per_cpu_sec = 0;
    fpops_cumulative = 0;
    intops_per_cpu_sec = 0;
    intops_cumulative = 0;
    app = NULL;
    wup = NULL;
    project = NULL;
}

static const char* task_state_name(int val) {
    switch (val) {
    case PROCESS_UNINITIALIZED: return "UNINITIALIZED";
    case PROCESS_EXECUTING: return "EXECUTING";
    case PROCESS_SUSPENDED: return "SUSPENDED";
    case PROCESS_ABORT_PENDING: return "ABORT_PENDING";
    case PROCESS_EXITED: return "EXITED";
    case PROCESS_WAS_SIGNALED: return "WAS_SIGNALED";
    case PROCESS_EXIT_UNKNOWN: return "EXIT_UNKNOWN";
    case PROCESS_ABORTED: return "ABORTED";
    case PROCESS_COULDNT_START: return "COULDNT_START";
    case PROCESS_QUIT_PENDING: return "QUIT_PENDING";
    }
    return "Unknown";
}

void ACTIVE_TASK::set_task_state(int val, const char* where) {
    _task_state = val;
    if (log_flags.task_debug) {
        msg_printf(result->project, MSG_INFO,
            "[task] task_state=%s for %s from %s",
            task_state_name(val), result->name, where
        );
    }
}

char* PROJECT::get_project_name() {
    if (strlen(project_name)) {
        return project_name;
    } else {
        return master_url;
    }
}

inline double drand() {
    return (double)rand()/(double)RAND_MAX;
}

// return a random double in the range [rmin,rmax)

inline double rand_range(double rmin, double rmax) {
    if (rmin < rmax) {
        return drand() * (rmax-rmin) + rmin;
    } else {
        return rmin;
    }
}

// return a random double in the range [MIN,min(e^n,MAX))
//
double calculate_exponential_backoff( int n, double MIN, double MAX) {
    double rmax = std::min(MAX, exp((double)n));
    return rand_range(MIN, rmax);
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

RESULT* CLIENT_STATE::lookup_result(PROJECT* p, const char* name) {
    for (unsigned int i=0; i<results.size(); i++) {
        RESULT* rp = results[i];
        if (rp->project == p && !strcmp(name, rp->name)) return rp;
    }
    return 0;
}

//////////////// FUNCTIONS WE NEED TO IMPLEMENT /////////////

ACTIVE_TASK::ACTIVE_TASK() {
    memset(this, 0, sizeof(*this));
    result = NULL;
    wup = NULL;
    app_version = NULL;
    pid = 0;
    slot = 0;
    _task_state = PROCESS_UNINITIALIZED;
    scheduler_state = CPU_SCHED_UNINITIALIZED;
    signal = 0;
    strcpy(slot_dir, "");
    graphics_mode_acked = MODE_UNSUPPORTED;
    graphics_mode_ack_timeout = 0;
    quit_time = 0;
    fraction_done = 0;
    run_interval_start_wall_time = gstate.now;
    checkpoint_cpu_time = 0;
    checkpoint_wall_time = 0;
    current_cpu_time = 0;
    have_trickle_down = false;
    send_upload_file_status = false;
    too_large = false;
    want_network = 0;
}

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
            "[task] %s task %s",
            first_time?"Starting":"Resuming", result->name
        );
    }
    set_task_state(PROCESS_EXECUTING, "start");
    char buf[256];
    sprintf(buf, "Starting %s: %f<br>", result->name, cpu_time_left);
    gstate.html_msg += buf;
    return 0;
}

void ACTIVE_TASK::get_free_slot(RESULT*){
}
int ACTIVE_TASK::init(RESULT* rp) {
    result = rp;
    wup = rp->wup;
    app_version = rp->avp;
    max_elapsed_time = rp->wup->rsc_fpops_bound/result->avp->flops;
    max_disk_usage = rp->wup->rsc_disk_bound;
    max_mem_usage = rp->wup->rsc_memory_bound;
    cpu_time_left = rp->final_cpu_time;
    _task_state = PROCESS_UNINITIALIZED;
    scheduler_state = CPU_SCHED_UNINITIALIZED;
    return 0;
}

void CLIENT_STATE::print_project_results(FILE* f) {
    for (unsigned int i=0; i<projects.size(); i++) {
        SIM_PROJECT* p = (SIM_PROJECT*) projects[i];
        p->print_results(f, sim_results);
    }
}


//////////////// OTHER

PROJECT::PROJECT() {
}

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

int NORMAL_DIST::parse(XML_PARSER& xp, char* end_tag) {
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

int UNIFORM_DIST::parse(XML_PARSER& xp, char* end_tag) {
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

int RANDOM_PROCESS::parse(XML_PARSER& xp, char* end_tag) {
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

int SIM_PROJECT::parse(XML_PARSER& xp) {
    char tag[256];
    bool is_tag;
    int retval;

    max_infeasible_count = 0;
    while(!xp.get(tag, sizeof(tag), is_tag)) {
        if (!is_tag) return ERR_XML_PARSE;
        if (!strcmp(tag, "/project")) return 0;
        else if (xp.parse_str(tag, "project_name", project_name, sizeof(project_name))) continue;
        else if (!strcmp(tag, "app")) {
            SIM_APP* sap = new SIM_APP;
            retval = sap->parse(xp);
            if (retval) return retval;
            sap->project = this;
            gstate.apps.push_back(sap);

            // for the time being, assume that there's a CPU app version
            // for each app
            //
            APP_VERSION* avp = new APP_VERSION;
            avp->app = sap;
            avp->avg_ncpus = 1;
            avp->flops = gstate.host_info.p_fpops;
            gstate.app_versions.push_back(avp);

        } else if (!strcmp(tag, "available")) {
            retval = available.parse(xp, "/available");
            if (retval) return retval;
        } else if (xp.parse_double(tag, "resource_share", resource_share)) {
            continue;
        } else if (xp.parse_int(tag, "max_infeasible_count", max_infeasible_count)) {
            continue;
        } else {
            printf("unrecognized: %s\n", tag);
            return ERR_XML_PARSE;
        }
    }
    return ERR_XML_PARSE;
}

int SIM_GPU::parse(XML_PARSER& xp, const char* end_tag) {
    char tag[256];
    bool is_tag;
    while(!xp.get(tag, sizeof(tag), is_tag)) {
        if (!is_tag) return ERR_XML_PARSE;
        if (!strcmp(tag, end_tag)) return 0;
        else if (xp.parse_int(tag, "count", count)) continue;
        else if (xp.parse_double(tag, "flops", flops)) continue;
        else if (xp.parse_str(tag, "type", type, sizeof(type))) continue;
        else return ERR_XML_PARSE;
    }
    return ERR_XML_PARSE;
}

int SIM_HOST::parse(XML_PARSER& xp) {
    char tag[256];
    bool is_tag;
    int retval;

    connection_interval = 0;
    p_ncpus = 1;
    while(!xp.get(tag, sizeof(tag), is_tag)) {
        if (!is_tag) return ERR_XML_PARSE;
        if (!strcmp(tag, "/host")) return 0;
        else if (xp.parse_double(tag, "p_fpops", p_fpops)) continue;
        else if (xp.parse_double(tag, "m_nbytes", m_nbytes)) continue;
        else if (xp.parse_int(tag, "p_ncpus", p_ncpus)) continue;
        else if (xp.parse_double(tag, "connection_interval", connection_interval)) continue;
        else if (!strcmp(tag, "available")) {
            retval = available.parse(xp, "/available");
            if (retval) return retval;
            available.init(START_TIME);
        } else if (!strcmp(tag, "idle")) {
            retval = idle.parse(xp, "/idle");
            if (retval) return retval;
            idle.init(START_TIME);
        } else if (!strcmp(tag, "gpu")) {
            SIM_GPU* sgp = new SIM_GPU;
            retval = sgp->parse(xp, "/gpu");
            if (retval) {
                delete sgp;
                return retval;
            }
            coprocs.coprocs.push_back(sgp);
        } else {
            printf("unrecognized: %s\n", tag);
            return ERR_XML_PARSE;
        }
    }       
    return ERR_XML_PARSE;
}

int CLIENT_STATE::parse_projects(char* name) {
    char tag[256];
    bool is_tag;
    MIOFILE mf;
    int retval, index=0;

    FILE* f = fopen(name, "r");
    if (!f) return ERR_FOPEN;
    mf.init_file(f);
    XML_PARSER xp(&mf);
    if (!xp.parse_start("projects")) return ERR_XML_PARSE;
    while(!xp.get(tag, sizeof(tag), is_tag)) {
        if (!is_tag) return ERR_XML_PARSE;
        if (!strcmp(tag, "project")) {
            SIM_PROJECT *p = new SIM_PROJECT;
            p->init();
            retval = p->parse(xp);
            if (retval) return retval;
            p->index = index++;
            p->result_index = 0;
            projects.push_back(p);
        } else if (!strcmp(tag, "/projects")) {
            return 0;
        } else {
            printf("unrecognized: %s\n", tag);
            return ERR_XML_PARSE;
        }
    }
    return ERR_XML_PARSE;
}

int CLIENT_STATE::parse_host(char* name) {
    MIOFILE mf;

    FILE* f = fopen(name, "r");
    if (!f) return ERR_FOPEN;
    mf.init_file(f);
    XML_PARSER xp(&mf);
    if (!xp.parse_start("host")) return ERR_XML_PARSE;
    return host_info.parse(xp);
}

int IP_RESULT::parse(FILE*) {
    return 0;
}

bool boinc_is_finite(double x) {
#if defined (HPUX_SOURCE)
    return _Isfinite(x);
    return false;
#else
    return (finite(x) != 0);
#endif
}
