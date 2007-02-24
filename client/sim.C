// usage:
// sim [--projects X] [--host X] [--prefs X] [--duration X]
//   defaults: projects.xml, host.xml, prefs.xml, 86400

#include "error_numbers.h"
#include "str_util.h"
#include "filesys.h"
#include "network.h"
#include "sim.h"

CLIENT_STATE gstate;
NET_STATUS net_status;
bool g_use_sandbox;
bool user_active;

//////////////// FUNCTIONS MODIFIED OR STUBBED OUT /////////////

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

APP_CLIENT_SHM::APP_CLIENT_SHM() {}
GRAPHICS_MSG::GRAPHICS_MSG() {}
HOST_INFO::HOST_INFO() {}

//////////////// FUNCTIONS COPIED /////////////

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

bool CLIENT_STATE::scheduler_rpc_poll() {
    PROJECT *p;

    p = next_project_sched_rpc_pending();
    if (p) {
        // simulate RPC
        return true;
    }
    
    p = find_project_with_overdue_results();
    if (p) {
        // simulate RPC
        return true;
    }
    p = next_project_need_work();
    if (p) {
        // simulate RPC
        return true;
    }
    return false;
}

bool ACTIVE_TASK_SET::poll() {
    return false;
}

//////////////// FUNCTIONS WE NEED TO IMPLEMENT /////////////

ACTIVE_TASK::ACTIVE_TASK() {
}
int ACTIVE_TASK::resume_or_start(bool first_time) {
    return 0;
}

int ACTIVE_TASK::preempt(bool quit_task) {
    return 0;
}
int ACTIVE_TASK_SET::get_free_slot(){
    return 0;
}
int ACTIVE_TASK::init(RESULT*){
    return 0;
}

//////////////// OTHER

PROJECT::PROJECT() {}

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
        } else if (xp.parse_double(tag, "latency_bound", latency_bound)) continue;
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
        } else return ERR_XML_PARSE;
    }
    return ERR_XML_PARSE;
}

int SIM_HOST::parse(XML_PARSER& xp) {
    char tag[256];
    bool is_tag;
    int retval;

    while(!xp.get(tag, sizeof(tag), is_tag)) {
        if (!is_tag) return ERR_XML_PARSE;
        if (!strcmp(tag, "/host")) return 0;
        else if (xp.parse_double(tag, "p_fpops", p_fpops)) continue;
        else if (xp.parse_double(tag, "m_nbytes", m_nbytes)) continue;
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
            retval = p->parse(xp);
            if (retval) return retval;
            projects.push_back(p);
        } else if (!strcmp(tag, "/projects")) {
            return 0;
        }
    }
    return ERR_XML_PARSE;
}

int CLIENT_STATE::parse_host(char*) {
    return 0;
}

void CLIENT_STATE::simulate(double duration) {
    bool action;
    now = 0;
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

int main(int argc, char** argv) {
    char projects[256], host[256], prefs[256];
    double duration = 86400;
    bool flag;
    int retval; 

    strcpy(projects, "projects.xml");
    strcpy(host, "host.xml");
    strcpy(prefs, "prefs.xml");

    retval = gstate.parse_projects(projects);
    if (retval) {
        printf("can't parse projects\n");
        exit(1);
    }
    retval = gstate.parse_host(host);
    if (retval) {
        printf("can't parse host\n");
        exit(1);
    }
    retval = gstate.global_prefs.parse_file(prefs, "", flag);
    if (retval) {
        printf("can't parse prefs\n");
        exit(1);
    }
    gstate.simulate(duration);
}
