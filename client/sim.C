// usage:
// sim [--projects X] [--host X] [--prefs X] [--duration X]
//   defaults: projects.xml, host.xml, prefs.xml, 86400

#include "error_numbers.h"
#include "str_util.h"
#include "log_flags.h"
#include "filesys.h"
#include "network.h"
#include "client_msgs.h"
#include "sim.h"

CLIENT_STATE gstate;
NET_STATUS net_status;
bool g_use_sandbox;
bool user_active;

//////////////// FUNCTIONS MODIFIED OR STUBBED OUT /////////////

CLIENT_STATE::CLIENT_STATE() {
    initialized = false;
}

FILE* boinc_fopen(const char* path, const char* mode) {
    return fopen(path, mode);
}

void CLIENT_STATE::set_client_state_dirty(char const*) {
}

void HOST_INFO::generate_host_cpid() {}

int get_connected_state() {return CONNECTED_STATE_CONNECTED;}

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

    printf("%s [%s] %s\n", time_string, x, message);
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

void PROJECT::init() {
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
    tentative = false;
    anonymous_platform = false;
    non_cpu_intensive = false;
    verify_files_on_app_start = false;
    short_term_debt = 0;
    long_term_debt = 0;
    send_file_list = false;
    suspended_via_gui = false;
    dont_request_more_work = false;
    detach_when_done = false;
    attached_via_acct_mgr = false;
    strcpy(code_sign_key, "");
    user_files.clear();
    project_files.clear();
    anticipated_debt = 0;
    wall_cpu_time_this_debt_interval = 0;
    next_runnable_result = NULL;
    work_request = 0;
    work_request_urgency = WORK_FETCH_DONT_NEED;
    duration_correction_factor = 1;
    project_files_downloaded_time = 0;

    // Initialize scratch variables.
    rrsim_proc_rate = 0.0;
    cpu_shortfall = 0.0;
    rr_sim_deadlines_missed = 0;
    deadlines_missed = 0;
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
    last_rr_sim_missed_deadline = false;
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
            "[task_debug] task_state=%s for %s from %s",
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

static inline double drand() {
    return (double)rand()/(double)RAND_MAX;
}

// return a random double in the range [rmin,rmax)

static inline double rand_range(double rmin, double rmax) {
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

void CLIENT_STATE::simulate_rpc(PROJECT* p) {
    unsigned int i;
    double net_fpops = host_info.p_fpops;
    while (1) {
        // pick a random app
        SIM_APP* ap;
        double x = drand();
        for (i=0; i<apps.size();i++) {
            ap = (SIM_APP*)apps[i];
            if (ap->project != p) continue;
            x -= ap->weight;
            if (x <= 0) break;
        }
        RESULT* rp = new RESULT;
        WORKUNIT* wup = new WORKUNIT;
        rp->clear();
        rp->project = p;
        rp->wup = wup;
        sprintf(rp->name, "result_%d", i++);
        rp->set_state(RESULT_FILES_DOWNLOADED, "simulate_rpc");
        wup->project = p;
        wup->rsc_fpops_est = ap->fpops_est;
        results.push_back(rp);
        rp->rrsim_cpu_left = ap->fpops.sample()/net_fpops;
        p->work_request -= ap->fpops_est/net_fpops;
        if (p->work_request <= 0) break;
    }
    p->work_request = 0;
    request_schedule_cpus("simulate_rpc");
}

bool CLIENT_STATE::scheduler_rpc_poll() {
    PROJECT *p;

    p = next_project_sched_rpc_pending();
    if (p) {
        simulate_rpc(p);
        return true;
    }
    
    p = find_project_with_overdue_results();
    if (p) {
        simulate_rpc(p);
        return true;
    }
    p = next_project_need_work();
    if (p) {
        simulate_rpc(p);
        return true;
    }
    return false;
}

bool ACTIVE_TASK_SET::poll() {
    unsigned int i;
    bool action = false;
    static double last_time = 0;
    double diff = gstate.now - last_time;
    if (diff < 1.0) return false;
    last_time = gstate.now;

    for (i=0; i<active_tasks.size(); i++) {
        ACTIVE_TASK* atp = active_tasks[i];
        switch (atp->task_state()) {
        case PROCESS_EXECUTING:
            atp->cpu_time_left -= diff;
            if (atp->cpu_time_left <= 0) {
                atp->set_task_state(PROCESS_EXITED, "poll");
            }
        }
    }
                     
    return action;
}

//////////////// FUNCTIONS WE NEED TO IMPLEMENT /////////////

ACTIVE_TASK::ACTIVE_TASK() {
}

int ACTIVE_TASK::resume_or_start(bool first_time) {
    set_task_state(PROCESS_EXECUTING, "start");
    return 0;
}

int ACTIVE_TASK::preempt(bool quit_task) {
    if (quit_task) {
        set_task_state(PROCESS_UNINITIALIZED, "start");
    } else {
        set_task_state(PROCESS_SUSPENDED, "start");
    }
    return 0;
}
int ACTIVE_TASK_SET::get_free_slot(){
    return 0;
}
int ACTIVE_TASK::init(RESULT* rp) {
    result = rp;
    wup = rp->wup;
    app_version = wup->avp;
    max_cpu_time = rp->wup->rsc_fpops_bound/gstate.host_info.p_fpops;
    max_disk_usage = rp->wup->rsc_disk_bound;
    max_mem_usage = rp->wup->rsc_memory_bound;
    cpu_time_left = rp->rrsim_cpu_left;
    _task_state = PROCESS_UNINITIALIZED;
    scheduler_state = CPU_SCHED_UNINITIALIZED;
    return 0;
}

//////////////// OTHER

PROJECT::PROJECT() {
}

double NORMAL_DIST::sample() {
    int i;
    double x=0;
    for (i=0; i<9; i++) {
        x += rand();
    }
    x /= 3;
    x *= var;
    x += mean;
    return x;
}

int NORMAL_DIST::parse(XML_PARSER& xp, char* end_tag) {
    char tag[256];
    bool is_tag;
    while(!xp.get(tag, sizeof(tag), is_tag)) {
        if (!is_tag) return ERR_XML_PARSE;
        if (xp.parse_double(tag, "mean", mean)) continue;
        else if (xp.parse_double(tag, "var", var)) continue;
        else if (!strcmp(tag, end_tag)) return 0;
        else return ERR_XML_PARSE;
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
        else return ERR_XML_PARSE;
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
        else return ERR_XML_PARSE;
    }
    return ERR_XML_PARSE;
}

int SIM_APP::parse(XML_PARSER& xp) {
    char tag[256];
    bool is_tag;
    int retval;

    while(!xp.get(tag, sizeof(tag), is_tag)) {
        if (!is_tag) return ERR_XML_PARSE;
        if (!strcmp(tag, "/app")) {
            return 0;
        }
        else if (xp.parse_double(tag, "latency_bound", latency_bound)) continue;
        else if (xp.parse_double(tag, "fpops_est", fpops_est)) continue;
        else if (!strcmp(tag, "fpops")) {
            retval = fpops.parse(xp, "/fpops");
            if (retval) return retval;
        } else if (!strcmp(tag, "checkpoint_period")) {
            retval = checkpoint_period.parse(xp, "/checkpoint_period");
            if (retval) return retval;
        } else if (xp.parse_double(tag, "working_set", working_set)) continue;
        else return ERR_XML_PARSE;
    }
    return ERR_XML_PARSE;
}

int SIM_PROJECT::parse(XML_PARSER& xp) {
    char tag[256];
    bool is_tag;
    int retval;

    while(!xp.get(tag, sizeof(tag), is_tag)) {
        if (!is_tag) return ERR_XML_PARSE;
        if (!strcmp(tag, "/project")) return 0;
        else if (xp.parse_str(tag, "project_name", project_name, sizeof(project_name))) continue;
        else if (!strcmp(tag, "app")) {
            SIM_APP* sap = new SIM_APP;
            retval = sap->parse(xp);
            if (retval) return retval;
            gstate.apps.push_back(sap);
        } else if (!strcmp(tag, "available")) {
            retval = available.parse(xp, "/available");
            if (retval) return retval;
        } else if (xp.parse_double(tag, "resource_share", resource_share)) {
            continue;
        } else return ERR_XML_PARSE;
    }
    return ERR_XML_PARSE;
}

int SIM_HOST::parse(XML_PARSER& xp) {
    char tag[256];
    bool is_tag;
    int retval;

    p_ncpus = 1;
    while(!xp.get(tag, sizeof(tag), is_tag)) {
        if (!is_tag) return ERR_XML_PARSE;
        if (!strcmp(tag, "/host")) return 0;
        else if (xp.parse_double(tag, "p_fpops", p_fpops)) continue;
        else if (xp.parse_double(tag, "m_nbytes", m_nbytes)) continue;
        else if (xp.parse_int(tag, "p_ncpus", p_ncpus)) continue;
        else if (!strcmp(tag, "available")) {
            retval = available.parse(xp, "/available");
            if (retval) return retval;
        } else if (!strcmp(tag, "idle")) {
            retval = idle.parse(xp, "/idle");
            if (retval) return retval;
        } else return ERR_XML_PARSE;
    }       
    return ERR_XML_PARSE;
}

int CLIENT_STATE::parse_projects(char* name) {
    char tag[256];
    bool is_tag;
    MIOFILE mf;
    int retval;

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
            projects.push_back(p);
        } else if (!strcmp(tag, "/projects")) {
            return 0;
        }
    }
    return ERR_XML_PARSE;
}

int CLIENT_STATE::parse_host(char* name) {
    char tag[256];
    bool is_tag;
    MIOFILE mf;
    int retval;

    FILE* f = fopen(name, "r");
    if (!f) return ERR_FOPEN;
    mf.init_file(f);
    XML_PARSER xp(&mf);
    if (!xp.parse_start("host")) return ERR_XML_PARSE;
    retval = host_info.parse(xp);
    return retval;
}

void CLIENT_STATE::simulate(double duration) {
    bool action;
    now = 0;
    printf("n: %d\n", active_tasks.active_tasks.size());
    while (1) {
        while (1) {
            action = active_tasks.poll();
            action |= possibly_schedule_cpus();
            action |= enforce_schedule();
            action |= compute_work_requests();
            action |= scheduler_rpc_poll();
            if (!action) break;
        }
        now += 60;
        if (now > duration) break;
    }
}

void parse_error(char* file) {
    printf("can't parse %s\n", file);
    exit(1);
}

int main(int argc, char** argv) {
    char projects[256], host[256], prefs[256];
    double duration = 100;
    bool flag;
    int retval;

    strcpy(projects, "sim_projects.xml");
    strcpy(host, "sim_host.xml");
    strcpy(prefs, "sim_prefs.xml");

    read_config_file();

    retval = gstate.parse_projects(projects);
    if (retval) parse_error("projects");
    retval = gstate.parse_host(host);
    if (retval) parse_error("host");
    retval = gstate.global_prefs.parse_file(prefs, "", flag);
    if (retval) parse_error("prefs");

    gstate.set_ncpus();
    printf("ncpus: %d\n", gstate.ncpus);
    gstate.request_work_fetch("init");
    gstate.simulate(duration);
}
