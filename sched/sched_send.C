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
// 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

// scheduler code related to sending work


#include "config.h"
#include <vector>
#include <list>
#include <string>
#include <ctime>
#include <cstdio>
#include <cstring>
#include <stdlib.h>

using namespace std;

#include <unistd.h>

#include "error_numbers.h"
#include "parse.h"
#include "util.h"
#include "str_util.h"

#include "server_types.h"
#include "sched_shmem.h"
#include "sched_config.h"
#include "sched_util.h"
#include "main.h"
#include "sched_array.h"
#include "sched_msgs.h"
#include "sched_hr.h"
#include "hr.h"
#include "sched_locality.h"
#include "sched_timezone.h"
#include "sched_assign.h"
#include "sched_plan.h"

#include "sched_send.h"

#ifdef _USING_FCGI_
#include "fcgi_stdio.h"
#else
#define FCGI_ToFILE(x) (x)
#endif

//#define MATCHMAKER

#ifdef MATCHMAKER
void send_work_matchmaker(SCHEDULER_REQUEST& sreq, SCHEDULER_REPLY& reply);
#endif

const char* infeasible_string(int code) {
    switch (code) {
    case INFEASIBLE_MEM: return "Not enough memory";
    case INFEASIBLE_DISK: return "Not enough disk";
    case INFEASIBLE_CPU: return "CPU too slow";
    case INFEASIBLE_APP_SETTING: return "App not selected";
    case INFEASIBLE_WORKLOAD: return "Existing workload";
    case INFEASIBLE_DUP: return "Already in reply";
    case INFEASIBLE_HR: return "Homogeneous redundancy";
    case INFEASIBLE_BANDWIDTH: return "Download bandwidth too low";
    }
    return "Unknown";
}

const int MIN_SECONDS_TO_SEND = 0;
const int MAX_SECONDS_TO_SEND = (28*SECONDS_IN_DAY);

inline int effective_ncpus(HOST& host) {
    int ncpus = host.p_ncpus;
    if (ncpus > config.max_ncpus) ncpus = config.max_ncpus;
    if (ncpus < 1) ncpus = 1;
    return ncpus;
}

const double DEFAULT_RAM_SIZE = 64000000;
    // if host sends us an impossible RAM size, use this instead

bool anonymous(PLATFORM* platform) {
    return (!strcmp(platform->name, "anonymous"));
}

bool SCHEDULER_REQUEST::has_version(APP& app) {
    unsigned int i;

    for (i=0; i<client_app_versions.size(); i++) {
        CLIENT_APP_VERSION& cav = client_app_versions[i];
        if (!strcmp(cav.app_name, app.name) && cav.version_num >= app.min_version) {
            return true;
        }
    }
    return false;
}

// return BEST_APP_VERSION for the given host, or NULL if none
//
//
BEST_APP_VERSION* get_app_version(
    SCHEDULER_REQUEST& sreq, SCHEDULER_REPLY& reply, WORKUNIT& wu
) {
    bool found;
    double flops;
    unsigned int i;
    int j;
    BEST_APP_VERSION* bavp;

    //
    // see if app is already in memoized array
    //
    for (i=0; i<reply.wreq.best_app_versions.size(); i++) {
        bavp = reply.wreq.best_app_versions[i];
        if (bavp->appid == wu.appid) {
            if (!bavp->avp) return NULL;
            return bavp;
        }
    }

    APP* app = ssp->lookup_app(wu.appid);
    if (!app) {
        log_messages.printf(MSG_CRITICAL, "app not found: %d\n", wu.appid);
        return NULL;
    }

    bavp = new BEST_APP_VERSION;
    bavp->appid = wu.appid;
    if (anonymous(sreq.platforms.list[0])) {
        found = sreq.has_version(*app);
        if (!found) {
            log_messages.printf(MSG_DEBUG, "Didn't find anonymous app\n");
            bavp->avp = 0;
        } else {
            bavp->avp = (APP_VERSION*)1;    // arbitrary nonzero value
        }
        reply.wreq.best_app_versions.push_back(bavp);
        return bavp;
    }


    // go through the client's platforms.
    // Scan the app versions for each platform.
    // Find the one with highest expected FLOPS
    //
    bavp->host_usage.flops = 0;
    bavp->avp = NULL;
    for (i=0; i<sreq.platforms.list.size(); i++) {
        PLATFORM* p = sreq.platforms.list[i];
        for (j=0; j<ssp->napp_versions; j++) {
            HOST_USAGE host_usage;
            APP_VERSION& av = ssp->app_versions[j];
            if (av.appid != wu.appid) continue;
            if (av.platformid != p->id) continue;
            if (reply.wreq.core_client_version < av.min_core_version) {
                reply.wreq.outdated_core = true;
                continue;
            }
            if (strlen(av.plan_class)) {
                if (!app_plan(sreq, av.plan_class, host_usage)) {
                    continue;
                }
            } else {
                host_usage.init_seq(reply.host.p_fpops);
            }
            if (host_usage.flops > bavp->host_usage.flops) {
                bavp->host_usage = host_usage;
                bavp->avp = &av;
            }
        }
    }
    reply.wreq.best_app_versions.push_back(bavp);
    if (bavp->avp) {
        if (config.debug_version_select) {
            log_messages.printf(MSG_DEBUG,
                "Best version of app %s is %d (%f FLOPS)\n",
                app->name, bavp->avp->id, bavp->host_usage.flops
            );
        }
    } else {
        // here if no app version exists
        //
        if (config.debug_version_select) {
            log_messages.printf(MSG_DEBUG,
                "no app version available: APP#%d PLATFORM#%d min_version %d\n",
                app->id, sreq.platforms.list[0]->id, app->min_version
            );
        }
        char message[256];
        sprintf(message,
            "%s is not available for your type of computer.",
            app->user_friendly_name
        );
        USER_MESSAGE um(message, "high");
        reply.wreq.insert_no_work_message(um);
        reply.wreq.no_app_version = true;
    }
    return bavp;
}

static char* find_user_friendly_name(int appid) {
	APP* app = ssp->lookup_app(appid);
	if (app) return app->user_friendly_name;
    return "deprecated application";
}


// Compute the max additional disk usage we can impose on the host.
// Depending on the client version, it can either send us
// - d_total and d_free (pre 4 oct 2005)
// - the above plus d_boinc_used_total and d_boinc_used_project
//
double max_allowable_disk(SCHEDULER_REQUEST& req, SCHEDULER_REPLY& reply) {
    HOST host = req.host;
    GLOBAL_PREFS prefs = req.global_prefs;
    double x1, x2, x3, x;

    // defaults are from config.xml
    // if not there these are used:
    // -default_max_used_gb= 100
    // -default_max_used_pct = 50
    // -default_min_free_gb = .001
    //
    if (prefs.disk_max_used_gb == 0) {
       prefs.disk_max_used_gb = config.default_disk_max_used_gb;
    }
    if (prefs.disk_max_used_pct == 0) {
       prefs.disk_max_used_pct = config.default_disk_max_used_pct;
    }
    // Always leave some disk space free
    if (prefs.disk_min_free_gb < config.default_disk_min_free_gb) {
       prefs.disk_min_free_gb = config.default_disk_min_free_gb;
    }

    // no defaults for total/free disk space (host.d_total, d_free)
    // if they're zero, client will get no work.
    //

    if (host.d_boinc_used_total) {
        // The post 4 oct 2005 case.
        // Compute the max allowable additional disk usage based on prefs
        //
        x1 = prefs.disk_max_used_gb*1e9 - host.d_boinc_used_total;
        x2 = host.d_total*prefs.disk_max_used_pct/100.
            - host.d_boinc_used_total;
        x3 = host.d_free - prefs.disk_min_free_gb*1e9;      // may be negative
        x = min(x1, min(x2, x3));

        // see which bound is the most stringent
        //
        if (x==x1) {
            reply.disk_limits.max_used = x;
        } else if (x==x2) {
            reply.disk_limits.max_frac = x;
        } else {
            reply.disk_limits.min_free = x;
        }
    } else {
        // here we don't know how much space BOINC is using.
        // so we're kinda screwed.
        // All we can do is assume that BOINC is using zero space.
        // We can't honor the max_used for max_used_pct preferences.
        // We can only honor the min_free pref.
        //
        x = host.d_free - prefs.disk_min_free_gb*1e9;      // may be negative
        reply.disk_limits.min_free = x;
        x1 = x2 = x3 = 0;
    }

    if (x < 0) {
        log_messages.printf(MSG_DEBUG,
            "Insufficient disk: disk_max_used_gb %f disk_max_used_pct %f disk_min_free_gb %f\n",
            prefs.disk_max_used_gb, prefs.disk_max_used_pct,
            prefs.disk_min_free_gb
        );
        log_messages.printf(MSG_DEBUG,
            "Insufficient disk: host.d_total %f host.d_free %f host.d_boinc_used_total %f\n",
            host.d_total, host.d_free, host.d_boinc_used_total
        );
        log_messages.printf(MSG_DEBUG,
            "Insufficient disk: x1 %f x2 %f x3 %f x %f\n",
            x1, x2, x3, x
        );
        reply.wreq.disk.set_insufficient(-x);
    }
    return x;
}

// if a host has active_frac < 0.1, assume 0.1 so we don't deprive it of work.
//
const double HOST_ACTIVE_FRAC_MIN = 0.1;

// estimate the number of CPU seconds that a workunit requires
// running on this host.
//
double estimate_cpu_duration(WORKUNIT& wu, SCHEDULER_REPLY& reply) {
    double p_fpops = reply.host.p_fpops;
    if (p_fpops <= 0) p_fpops = 1e9;
    double rsc_fpops_est = wu.rsc_fpops_est;
    if (rsc_fpops_est <= 0) rsc_fpops_est = 1e12;
    return rsc_fpops_est/p_fpops;
}

// estimate the amount of real time to complete this WU,
// taking into account active_frac etc.
// Note: don't factor in resource_share_fraction.
// The core client no longer necessarily does round-robin
// across all projects.
//
static double estimate_wallclock_duration(
    WORKUNIT& wu, SCHEDULER_REQUEST& request, SCHEDULER_REPLY& reply
) {
    double running_frac;
    if (reply.wreq.core_client_version<=419) {
        running_frac = reply.host.on_frac;
    } else {
        running_frac = reply.host.active_frac * reply.host.on_frac;
    }
    if (running_frac < HOST_ACTIVE_FRAC_MIN) {
        running_frac = HOST_ACTIVE_FRAC_MIN;
    }
    if (running_frac > 1) running_frac = 1;
    double ecd = estimate_cpu_duration(wu, reply);
    double ewd = ecd/running_frac;
    if (reply.host.duration_correction_factor) {
        ewd *= reply.host.duration_correction_factor;
    }
    if (reply.host.cpu_efficiency) {
        ewd /= reply.host.cpu_efficiency;
    }
    log_messages.printf(MSG_DEBUG,
        "est cpu dur %f; running_frac %f; est %f\n",
        ecd, running_frac, ewd
    );
    return ewd;
}

// Find or compute various info about the host.
// These parameters affect how work is sent to the host
//
static int get_host_info(SCHEDULER_REPLY& reply) {
    char buf[8096];
    std::string str;
    unsigned int pos = 0;
    int temp_int;
    bool flag;

    extract_venue(reply.user.project_prefs, reply.host.venue, buf);
    str = buf;

    // scan user's project prefs for elements of the form <app_id>N</app_id>,
    // indicating the apps they want to run.
    //
    reply.wreq.host_info.preferred_apps.clear();
    while (parse_int(str.substr(pos,str.length()-pos).c_str(), "<app_id>", temp_int)) {
        APP_INFO ai;
        ai.appid = temp_int;
        ai.work_available = false;
        reply.wreq.host_info.preferred_apps.push_back(ai);

        pos = str.find("<app_id>", pos) + 1;
    }
	if (parse_bool(buf,"<allow_non_preferred_apps>", flag)) {
	    reply.wreq.host_info.allow_non_preferred_apps = flag;
    }
	if (parse_bool(buf,"<allow_beta_work>", flag)) {
        reply.wreq.host_info.allow_beta_work = flag;
	}
 
    // Decide whether or not this computer is a 'reliable' computer
    //
    double expavg_credit = reply.host.expavg_credit;
    double expavg_time = reply.host.expavg_time;
    double avg_turnaround = reply.host.avg_turnaround;
    update_average(0, 0, CREDIT_HALF_LIFE, expavg_credit, expavg_time);
    // A computer is reliable if the following conditions are true
    // (for those that are set in the config file)
    // 1) The host average turnaround is less than the config
    // max average turnaround
    // 2) The host error rate is less then the config max error rate
    // 3) The host results per day is equal to the config file value
    //
    log_messages.printf(MSG_DEBUG, "[HOST#%d] Checking if it is reliable (OS = %s) error_rate = %.3f avg_turnaround(hours) = %.0f \n",
        reply.host.id, reply.host.os_name, reply.host.error_rate,
        reply.host.avg_turnaround/3600
    );

	// Platforms other then Windows, Linux and Intel Macs need a
    // larger set of computers to be marked reliable
    //
    double multiplier = 1.0;
    if (strstr(reply.host.os_name,"Windows") || strstr(reply.host.os_name,"Linux") || (strstr(reply.host.os_name,"Darwin") && !(strstr(reply.host.p_vendor,"Power Macintosh")))) {
    	multiplier = 1.0;
    } else {
    	multiplier = 1.8;
    }

    if ((config.reliable_max_avg_turnaround == 0 || reply.host.avg_turnaround < config.reliable_max_avg_turnaround*multiplier)
        && (config.reliable_max_error_rate == 0 || reply.host.error_rate < config.reliable_max_error_rate*multiplier)
        && (config.daily_result_quota == 0 || reply.host.max_results_day >= config.daily_result_quota)
     ) {
        reply.wreq.host_info.reliable = true;
        log_messages.printf(MSG_DEBUG,
            "[HOST#%d] is reliable (OS = %s) error_rate = %.3f avg_turnaround(hours) = %.0f \n",
            reply.host.id, reply.host.os_name, reply.host.error_rate,
            reply.host.avg_turnaround/3600
        );
    }
    return 0;
}

// Check to see if the user has set application preferences.
// If they have, then only send work for the allowed applications
//

static inline int check_app_filter(
    WORKUNIT& wu, SCHEDULER_REQUEST& , SCHEDULER_REPLY& reply
) {
    unsigned int i;

    if (reply.wreq.host_info.preferred_apps.size() == 0) return 0;
    bool app_allowed = false;
    for (i=0; i<reply.wreq.host_info.preferred_apps.size(); i++) {
        if (wu.appid==reply.wreq.host_info.preferred_apps[i].appid) {
            app_allowed = true;
    	    reply.wreq.host_info.preferred_apps[i].work_available = true;
            break;
        }
    }
    // TODO: select between the following via config
    //if (!app_allowed) {
    if (!app_allowed && reply.wreq.user_apps_only && !reply.wreq.beta_only) {
        reply.wreq.no_allowed_apps_available = true;
        log_messages.printf(MSG_DEBUG,
            "[USER#%d] [WU#%d] user doesn't want work for this application\n",
            reply.user.id, wu.id
        );
        return INFEASIBLE_APP_SETTING;
    }
    return 0;
}

// see how much RAM we can use on this machine
// TODO: compute this once, not once per job
//
static inline void get_mem_sizes(
    SCHEDULER_REQUEST& request, SCHEDULER_REPLY& reply,
    double& ram, double& usable_ram
) {
    ram = reply.host.m_nbytes;
    if (ram <= 0) ram = DEFAULT_RAM_SIZE;
    usable_ram = ram;
    double busy_frac = request.global_prefs.ram_max_used_busy_frac;
    double idle_frac = request.global_prefs.ram_max_used_idle_frac;
    double frac = 1;
    if (busy_frac>0 && idle_frac>0) {
        frac = std::max(busy_frac, idle_frac);
        if (frac > 1) frac = 1;
        usable_ram *= frac;
    }
}

static inline int check_memory(
    WORKUNIT& wu, SCHEDULER_REQUEST& request, SCHEDULER_REPLY& reply
) {
    double ram, usable_ram;
    get_mem_sizes(request, reply, ram, usable_ram);

    double diff = wu.rsc_memory_bound - usable_ram;
    if (diff > 0) {
        char message[256];
        sprintf(message,
            "%s needs %0.2f MB RAM but only %0.2f MB is available for use.",
            find_user_friendly_name(wu.appid),
            wu.rsc_memory_bound/MEGA, usable_ram/MEGA
        );
        USER_MESSAGE um(message,"high");
        reply.wreq.insert_no_work_message(um);
        
        log_messages.printf(MSG_DEBUG,
            "[WU#%d %s] needs %0.2fMB RAM; [HOST#%d] has %0.2fMB, %0.2fMB usable\n",
            wu.id, wu.name, wu.rsc_memory_bound/MEGA,
            reply.host.id, ram/MEGA, usable_ram/MEGA
        );
        reply.wreq.mem.set_insufficient(wu.rsc_memory_bound);
        reply.set_delay(DELAY_NO_WORK_TEMP);
        return INFEASIBLE_MEM;
    }
    return 0;
}

static inline int check_disk(
    WORKUNIT& wu, SCHEDULER_REQUEST& , SCHEDULER_REPLY& reply
) {
    double diff = wu.rsc_disk_bound - reply.wreq.disk_available;
    if (diff > 0) {
        char message[256];
        sprintf(message,
            "%s needs %0.2fMB more disk space.  You currently have %0.2f MB available and it needs %0.2f MB.",
            find_user_friendly_name(wu.appid),
            diff/MEGA, reply.wreq.disk_available/MEGA, wu.rsc_disk_bound/MEGA
        );
        USER_MESSAGE um(message,"high");
        reply.wreq.insert_no_work_message(um);

        reply.wreq.disk.set_insufficient(diff);
        return INFEASIBLE_DISK;
    }
    return 0;
}

static inline int check_bandwidth(
    WORKUNIT& wu, SCHEDULER_REQUEST& , SCHEDULER_REPLY& reply
) {
    if (wu.rsc_bandwidth_bound == 0) return 0;
    double diff = wu.rsc_bandwidth_bound - reply.host.n_bwdown;
    if (diff > 0) {
        char message[256];
        sprintf(message,
            "%s requires %0.2f kbps download bandwidth.  Your computer has been measured at %0.2f kbps.",
            find_user_friendly_name(wu.appid),
            wu.rsc_bandwidth_bound/KILO, reply.host.n_bwdown/KILO
        );
        USER_MESSAGE um(message,"high");
        reply.wreq.insert_no_work_message(um);

        reply.wreq.bandwidth.set_insufficient(diff);
        return INFEASIBLE_BANDWIDTH;
    }
    return 0;
}

static inline int check_deadline(
    WORKUNIT& wu, SCHEDULER_REQUEST& request, SCHEDULER_REPLY& reply
) {
    // skip delay check if host currently doesn't have any work
    // (i.e. everyone gets one result, no matter how slow they are)
    //
    if (!config.ignore_delay_bound && request.estimated_delay>0) {
        double ewd = estimate_wallclock_duration(wu, request, reply);
        double est_completion_delay = request.estimated_delay + ewd;
        double est_report_delay = max(est_completion_delay, request.global_prefs.work_buf_min());
        double diff = est_report_delay - wu.delay_bound;
        if (diff > 0) {
            log_messages.printf(MSG_DEBUG,
                "[WU#%d %s] needs %d seconds on [HOST#%d]; delay_bound is %d (estimated_delay is %f)\n",
                wu.id, wu.name, (int)ewd, reply.host.id, wu.delay_bound,
                request.estimated_delay
            );
            reply.wreq.speed.set_insufficient(diff);
            return INFEASIBLE_CPU;
        }
    }
    return 0;
}

// Quick checks (no DB access) to see if the WU can be sent on the host.
// Reasons why not include:
// 1) the host doesn't have enough memory;
// 2) the host doesn't have enough disk space;
// 3) based on CPU speed, resource share and estimated delay,
//    the host probably won't get the result done within the delay bound
// 4) app isn't in user's "approved apps" list
//
// TODO: this should be used in locality scheduling case too.
// Should move a few other checks from sched_array.C
//
int wu_is_infeasible(
    WORKUNIT& wu, SCHEDULER_REQUEST& request, SCHEDULER_REPLY& reply, APP& app
) {
    int retval;

    // homogeneous redundancy, quick check
    //
    if (app_hr_type(app)) {
        if (hr_unknown_platform_type(reply.host, app_hr_type(app))) {
            log_messages.printf(MSG_DEBUG,
                "[HOST#%d] [WU#%d %s] host is of unknown class in HR type %d\n",
                reply.host.id, wu.id, app_hr_type(app)
            );
            return INFEASIBLE_HR;
        }
        if (already_sent_to_different_platform_quick(request, wu, app)) {
            log_messages.printf(MSG_DEBUG,
                "[HOST#%d] [WU#%d %s] failed quick HR check: WU is class %d, host is class %d\n",
                reply.host.id, wu.id, wu.name, wu.hr_class, hr_class(request.host, app_hr_type(app))
            );
            return INFEASIBLE_HR;
        }
    }
    
    if (config.one_result_per_user_per_wu || config.one_result_per_host_per_wu) {
        if (wu_already_in_reply(wu, reply)) {
            return INFEASIBLE_DUP;
        }
    }

    retval = check_app_filter(wu, request, reply);
    if (retval) return retval;
    retval = check_memory(wu, request, reply);
    if (retval) return retval;
    retval = check_disk(wu, request, reply);
    if (retval) return retval;
    retval = check_bandwidth(wu, request, reply);
    if (retval) return retval;

    // do this last because EDF sim uses some CPU
    //
    if (config.workload_sim && request.have_other_results_list) {
        double est_cpu = estimate_cpu_duration(wu, reply);
        IP_RESULT candidate("", wu.delay_bound, est_cpu);
        strcpy(candidate.name, wu.name);
        if (check_candidate(candidate, effective_ncpus(reply.host), request.ip_results)) {
            // it passed the feasibility test,
            // but don't add it the the workload yet;
            // wait until we commit to sending it
        } else {
            reply.wreq.speed.set_insufficient(0);
            return INFEASIBLE_WORKLOAD;
        }
    } else {
        retval = check_deadline(wu, request, reply);
        if (retval) return INFEASIBLE_WORKLOAD;
    }

    return 0;
}

// insert "text" right after "after" in the given buffer
//
int insert_after(char* buffer, const char* after, const char* text) {
    char* p;
    char temp[LARGE_BLOB_SIZE];

    if (strlen(buffer) + strlen(text) > LARGE_BLOB_SIZE-1) {
        log_messages.printf(MSG_CRITICAL,
            "insert_after: overflow: %d %d\n",
            strlen(buffer), strlen(text)
        );
        return ERR_BUFFER_OVERFLOW;
    }
    p = strstr(buffer, after);
    if (!p) {
        log_messages.printf(MSG_CRITICAL,
            "insert_after: %s not found in %s\n", after, buffer
        );
        return ERR_NULL;
    }
    p += strlen(after);
    strcpy(temp, p);
    strcpy(p, text);
    strcat(p, temp);
    return 0;
}

// add elements to WU's xml_doc,
// in preparation for sending it to a client
//
int insert_wu_tags(WORKUNIT& wu, APP& app) {
    char buf[LARGE_BLOB_SIZE];
    
    sprintf(buf,
        "    <rsc_fpops_est>%f</rsc_fpops_est>\n"
        "    <rsc_fpops_bound>%f</rsc_fpops_bound>\n"
        "    <rsc_memory_bound>%f</rsc_memory_bound>\n"
        "    <rsc_disk_bound>%f</rsc_disk_bound>\n"
        "    <name>%s</name>\n"
        "    <app_name>%s</app_name>\n",
        wu.rsc_fpops_est,
        wu.rsc_fpops_bound,
        wu.rsc_memory_bound,
        wu.rsc_disk_bound,
        wu.name,
        app.name
    );
    return insert_after(wu.xml_doc, "<workunit>\n", buf);
}

// verify that the given APP_VERSION will work with the core client
//
bool app_core_compatible(WORK_REQ& wreq, APP_VERSION& av) {
    if (wreq.core_client_version < av.min_core_version) {
        wreq.outdated_core = true;
        return false;
    }
    return true;
}

// add the given workunit to a reply.
// Add the app and app_version to the reply also.
//
int add_wu_to_reply(
    WORKUNIT& wu, SCHEDULER_REPLY& reply, APP* app, BEST_APP_VERSION* bavp
) {
    int retval;
    WORKUNIT wu2, wu3;
    
    APP_VERSION* avp = bavp->avp;
    if (avp == (APP_VERSION*)1) avp = NULL;

    // add the app, app_version, and workunit to the reply,
    // but only if they aren't already there
    //
    if (avp) {
        APP_VERSION av2=*avp, *avp2=&av2;
        
        if (config.choose_download_url_by_timezone) {
            process_av_timezone(reply, avp, av2);
        }
        
        reply.insert_app_unique(*app);
        reply.insert_app_version_unique(*avp2);
        log_messages.printf(MSG_DEBUG,
            "[HOST#%d] Sending app_version %s %d %d\n",
            reply.host.id, app->name, avp2->platformid, avp2->version_num
        );
    }

    // add time estimate to reply
    //
    wu2 = wu;       // make copy since we're going to modify its XML field
    retval = insert_wu_tags(wu2, *app);
    if (retval) {
        log_messages.printf(MSG_NORMAL,
            "insert_wu_tags failed %d\n", retval
        );
        return retval;
    }
    wu3=wu2;
    if (config.choose_download_url_by_timezone) {
        process_wu_timezone(reply, wu2, wu3);
    }
    
    reply.insert_workunit_unique(wu3);

    // switch to tighter policy for estimating delay
    //
    return 0;
}

int insert_name_tags(RESULT& result, WORKUNIT const& wu) {
    char buf[256];
    int retval;

    sprintf(buf, "<name>%s</name>\n", result.name);
    retval = insert_after(result.xml_doc_in, "<result>\n", buf);
    if (retval) return retval;
    sprintf(buf, "<wu_name>%s</wu_name>\n", wu.name);
    retval = insert_after(result.xml_doc_in, "<result>\n", buf);
    if (retval) return retval;
    return 0;
}

int insert_deadline_tag(RESULT& result) {
    char buf[256];
    sprintf(buf, "<report_deadline>%d</report_deadline>\n", result.report_deadline);
    int retval = insert_after(result.xml_doc_in, "<result>\n", buf);
    if (retval) return retval;
    return 0;
}

int update_wu_transition_time(WORKUNIT wu, time_t x) {
    DB_WORKUNIT dbwu;
    char buf[256];

    dbwu.id = wu.id;
    
    // SQL note: can't use min() here
    //
    sprintf(buf,
        "transition_time=if(transition_time<%d, transition_time, %d)",
        (int)x, (int)x
    );
    return dbwu.update_field(buf);
}

// return true iff a result for same WU is already being sent
//
bool wu_already_in_reply(WORKUNIT& wu, SCHEDULER_REPLY& reply) {
    unsigned int i;
    for (i=0; i<reply.results.size(); i++) {
        if (wu.id == reply.results[i].workunitid) {
            return true;
        }
    }
    return false;
}

void lock_sema() {
    lock_semaphore(sema_key);
}

void unlock_sema() {
    unlock_semaphore(sema_key);
}

// return true if additional work is needed,
// and there's disk space left,
// and we haven't exceeded result per RPC limit,
// and we haven't exceeded results per day limit
//
bool SCHEDULER_REPLY::work_needed(bool locality_sched) {
    if (locality_sched) {
        // if we've failed to send a result because of a transient condition,
        // return false to preserve invariant
        //
        if (wreq.disk.insufficient || wreq.speed.insufficient || wreq.mem.insufficient || wreq.no_allowed_apps_available) {
            return false;
        }
    }
    if (wreq.seconds_to_fill <= 0) return false;
    if (wreq.disk_available <= 0) {
        return false;
    }
    if (wreq.nresults >= config.max_wus_to_send) return false;

    int ncpus = effective_ncpus(host);

    // host.max_results_day is between 1 and config.daily_result_quota inclusive
    // wreq.daily_result_quota is between ncpus
    // and ncpus*host.max_results_day inclusive
    //
    if (config.daily_result_quota) {
        if (host.max_results_day == 0 || host.max_results_day>config.daily_result_quota) {
            host.max_results_day = config.daily_result_quota;
        }
        wreq.daily_result_quota = ncpus*host.max_results_day;
        if (host.nresults_today >= wreq.daily_result_quota) {
            wreq.daily_result_quota_exceeded = true;
            return false;
        }
    }

    if (config.max_wus_in_progress) {
        if (wreq.nresults_on_host >= config.max_wus_in_progress*ncpus) {
            log_messages.printf(MSG_DEBUG,
                "cache limit exceeded; %d > %d*%d\n",
                wreq.nresults_on_host, config.max_wus_in_progress, ncpus
            );
            wreq.cache_size_exceeded = true;
            return false;
        }
    }
    return true;
}

void SCHEDULER_REPLY::got_good_result() {
    host.max_results_day *= 2;
    if (host.max_results_day > config.daily_result_quota) {
        host.max_results_day = config.daily_result_quota;
    }
}

void SCHEDULER_REPLY::got_bad_result() {
    host.max_results_day -= 1;
    if (host.max_results_day < 1) {
        host.max_results_day = 1;
    }
}

int add_result_to_reply(
    DB_RESULT& result, WORKUNIT& wu, SCHEDULER_REQUEST& request,
    SCHEDULER_REPLY& reply, BEST_APP_VERSION* bavp
) {
    int retval;
    double wu_seconds_filled;
    bool resent_result = false;
    APP* app = ssp->lookup_app(wu.appid);

    retval = add_wu_to_reply(wu, reply, app, bavp);
    if (retval) return retval;

    // in the scheduling locality case,
    // reduce the available space by LESS than the workunit rsc_disk_bound,
    // IF the host already has the file OR the file was not already sent.
    //
    if (!config.locality_scheduling ||
        decrement_disk_space_locality(wu, request, reply)
    ) {
        reply.wreq.disk_available -= wu.rsc_disk_bound;
    }

    // update the result in DB
    //
    result.hostid = reply.host.id;
    result.userid = reply.user.id;
    result.sent_time = time(0);
    int old_server_state = result.server_state;

    int delay_bound = wu.delay_bound;
    if (result.server_state != RESULT_SERVER_STATE_IN_PROGRESS) {
        // We are sending this result for the first time
        //
        // If the workunit needs reliable and is being sent to a reliable host,
        // then shorten the delay bound by the percent specified
        //
        if (config.reliable_on_priority && result.priority >= config.reliable_on_priority && config.reliable_reduced_delay_bound > 0.01
        ) {
			double reduced_delay_bound = delay_bound*config.reliable_reduced_delay_bound;
			double est_wallclock_duration = estimate_wallclock_duration(wu, request, reply);
            // Check to see how reasonable this reduced time is.
            // Increase it to twice the estimated delay bound
            // if all the following apply:
            //
			// 1) Twice the estimate is longer then the reduced delay bound
			// 2) Twice the estimate is less then the original delay bound
			// 3) Twice the estimate is less then the twice the reduced delay bound
			if (est_wallclock_duration*2 > reduced_delay_bound && est_wallclock_duration*2 < delay_bound && est_wallclock_duration*2 < delay_bound*config.reliable_reduced_delay_bound*2 ) {
        		reduced_delay_bound = est_wallclock_duration*2;
            }
			delay_bound = (int) reduced_delay_bound;
        }

        result.report_deadline = result.sent_time + delay_bound;
        result.server_state = RESULT_SERVER_STATE_IN_PROGRESS;
    } else {
        // Result was already sent to this host but was lost,
        // so we are resending it.
        //
        resent_result = true;
 
        // TODO: explain the following
        //
        if (result.report_deadline < result.sent_time) {
            result.report_deadline = result.sent_time + 10;
        }
        if (result.report_deadline > result.sent_time + delay_bound) {
            result.report_deadline = result.sent_time + delay_bound;
        }

        log_messages.printf(MSG_DEBUG,
            "[RESULT#%d] [HOST#%d] (resend lost work)\n",
            result.id, reply.host.id
        );
    }
    retval = result.mark_as_sent(old_server_state);
    if (retval==ERR_DB_NOT_FOUND) {
        log_messages.printf(MSG_CRITICAL,
            "[RESULT#%d] [HOST#%d]: CAN'T SEND, already sent to another host\n",
            result.id, reply.host.id
        );
    } else if (retval) {
        log_messages.printf(MSG_CRITICAL,
            "add_result_to_reply: can't update result: %d\n", retval
        );
    }
    if (retval) return retval;

    wu_seconds_filled = estimate_wallclock_duration(wu, request, reply);
    log_messages.printf(MSG_NORMAL,
        "[HOST#%d] Sending [RESULT#%d %s] (fills %.2f seconds)\n",
        reply.host.id, result.id, result.name, wu_seconds_filled
    );

    retval = update_wu_transition_time(wu, result.report_deadline);
    if (retval) {
        log_messages.printf(MSG_CRITICAL,
            "add_result_to_reply: can't update WU transition time: %d\n",
            retval
        );
        return retval;
    }

    // The following overwrites the result's xml_doc field.
    // But that's OK cuz we're done with DB updates
    //
    retval = insert_name_tags(result, wu);
    if (retval) {
        log_messages.printf(MSG_CRITICAL,
            "add_result_to_reply: can't insert name tags: %d\n",
            retval
        );
        return retval;
    }
    retval = insert_deadline_tag(result);
    if (retval) {
        log_messages.printf(MSG_CRITICAL,
            "add_result_to_reply: can't insert deadline tag: %d\n", retval
        );
        return retval;
    }
    result.bavp = bavp;
    reply.insert_result(result);
    reply.wreq.seconds_to_fill -= wu_seconds_filled;
    request.estimated_delay += wu_seconds_filled/effective_ncpus(reply.host);
    reply.wreq.nresults++;
    reply.wreq.nresults_on_host++;
    if (!resent_result) reply.host.nresults_today++;

    // add this result to workload for simulation
    //
    if (config.workload_sim && request.have_other_results_list) {
        double est_cpu = estimate_cpu_duration(wu, reply);
        IP_RESULT ipr ("", time(0)+wu.delay_bound, est_cpu);
        request.ip_results.push_back(ipr);
    }

    // mark job as done if debugging flag is set
    //
    if (mark_jobs_done) {
        DB_WORKUNIT dbwu;
        char buf[256];
        sprintf(buf,
            "server_state=%d outcome=%d",
            RESULT_SERVER_STATE_OVER, RESULT_OUTCOME_SUCCESS
        );
        result.update_field(buf);

        dbwu.id = wu.id;
        sprintf(buf, "transition_time=%d", time(0));
        dbwu.update_field(buf);

    }
    return 0;
}

void send_work(SCHEDULER_REQUEST& sreq, SCHEDULER_REPLY& reply) {
    char helpful[512];
    int preferred_app_message_index=0;

    reply.wreq.core_client_version =
        sreq.core_client_major_version*100 + sreq.core_client_minor_version;
    reply.wreq.disk_available = max_allowable_disk(sreq, reply);
    reply.wreq.core_client_version = sreq.core_client_major_version*100
        + sreq.core_client_minor_version;

    if (hr_unknown_platform(sreq.host)) {
        reply.wreq.hr_reject_perm = true;
        return;
    }

    get_host_info(reply); // parse project prefs for app details
    reply.wreq.beta_only = false;
    reply.wreq.user_apps_only=true;

    log_messages.printf(MSG_DEBUG,
        "[HOST#%d] got request for %f seconds of work; available disk %f GB\n",
        reply.host.id, sreq.work_req_seconds, reply.wreq.disk_available/1e9
    );

    if (sreq.work_req_seconds <= 0) return;

    reply.wreq.seconds_to_fill = sreq.work_req_seconds;
    if (reply.wreq.seconds_to_fill > MAX_SECONDS_TO_SEND) {
        reply.wreq.seconds_to_fill = MAX_SECONDS_TO_SEND;
    }
    if (reply.wreq.seconds_to_fill < MIN_SECONDS_TO_SEND) {
        reply.wreq.seconds_to_fill = MIN_SECONDS_TO_SEND;
    }

    if (config.enable_assignment) {
        if (send_assigned_jobs(sreq, reply)) {
            log_messages.printf(MSG_DEBUG,
                "[HOST#%d] sent assigned jobs\n", reply.host.id
            );
            return;
        }
    }

    if (config.workload_sim && sreq.have_other_results_list) {
        init_ip_results(
            sreq.global_prefs.work_buf_min(), effective_ncpus(reply.host), sreq.ip_results
        );
    }

    if (config.locality_scheduling) {
        reply.wreq.infeasible_only = false;
        send_work_locality(sreq, reply);
    } else {
#ifdef MATCHMAKER
        send_work_matchmaker(sreq, reply);
#else
        // give top priority to results that require a 'reliable host'
        //
        if (reply.wreq.host_info.reliable) {
            reply.wreq.reliable_only = true;
            reply.wreq.infeasible_only = false;
            scan_work_array(sreq, reply);
        }
        reply.wreq.reliable_only = false;

        // give 2nd priority to results for a beta app
        // (projects should load beta work with care,
        // otherwise your users won't get production work done!
        //
        if (reply.wreq.host_info.allow_beta_work) {
            reply.wreq.beta_only = true;
            log_messages.printf(MSG_DEBUG,
                "[HOST#%d] will accept beta work.  Scanning for beta work.\n",
                reply.host.id
            );
            scan_work_array(sreq, reply);
        }
        reply.wreq.beta_only = false;

        // give next priority to results that were infeasible for some other host
        //
        reply.wreq.infeasible_only = true;
        scan_work_array(sreq, reply);

        reply.wreq.infeasible_only = false;
        scan_work_array(sreq, reply);
        
       	// If the user has said they prefer to only receive work
       	// from certain apps
       	//
       	if (!reply.wreq.nresults && reply.wreq.host_info.allow_non_preferred_apps ) {
       		reply.wreq.user_apps_only = false;
       		preferred_app_message_index = reply.wreq.no_work_messages.size();
           	log_messages.printf(MSG_DEBUG,
                "[HOST#%d] is looking for work from a non-preferred application\n",
                reply.host.id
            );
       		scan_work_array(sreq, reply);
       	}
#endif
    }

    log_messages.printf(MSG_NORMAL,
        "[HOST#%d] Sent %d results [scheduler ran %f seconds]\n",
        reply.host.id, reply.wreq.nresults, elapsed_wallclock_time() 
    );

    // Send messages explaining why work was sent from apps
    // the user did not check on the website
    //
    if (reply.wreq.nresults && !reply.wreq.user_apps_only) {
        USER_MESSAGE um("No work can be sent for the applications you have selected", "high");
        reply.insert_message(um);

        // Inform the user about applications with no work
        for (int i=0; i<reply.wreq.host_info.preferred_apps.size(); i++) {
         	if (!reply.wreq.host_info.preferred_apps[i].work_available) {
         		APP* app = ssp->lookup_app(reply.wreq.host_info.preferred_apps[i].appid);
         		// don't write message if the app is deprecated
         		if (app) {
           			char explanation[256];
           			sprintf(explanation,
                        "No work is available for %s",
                        find_user_friendly_name(reply.wreq.host_info.preferred_apps[i].appid)
                    );
        			USER_MESSAGE um(explanation, "high");
           			reply.insert_message(um);
         		}
           	}
        }

        // Inform the user about applications they didn't qualify for
        //
        for(int i=0;i<preferred_app_message_index;i++){
            reply.insert_message(reply.wreq.no_work_messages.at(i));
        }
        USER_MESSAGE um1("You have selected to receive work from other applications if no work is available for the applications you selected", "high");
        reply.insert_message(um1);
        USER_MESSAGE um2("Sending work from other applications", "high");
        reply.insert_message(um2);
     }

    // if client asked for work and we're not sending any, explain why
    //
    if (reply.wreq.nresults == 0) {
        reply.set_delay(DELAY_NO_WORK_TEMP);
        USER_MESSAGE um2("No work sent", "high");
        reply.insert_message(um2);
        // Inform the user about applications with no work
        for(int i=0; i<reply.wreq.host_info.preferred_apps.size(); i++) {
         	if ( !reply.wreq.host_info.preferred_apps[i].work_available ) {
         		APP* app = ssp->lookup_app(reply.wreq.host_info.preferred_apps[i].appid);
         		// don't write message if the app is deprecated
         		if ( app != NULL ) {
           			char explanation[256];
           			sprintf(explanation,"No work is available for %s",find_user_friendly_name(reply.wreq.host_info.preferred_apps[i].appid));
        			USER_MESSAGE um(explanation, "high");
           			reply.insert_message(um);
         		}
           	}
        }
        // Inform the user about applications they didn't qualify for
        for(int i=0;i<reply.wreq.no_work_messages.size();i++){
        	reply.insert_message(reply.wreq.no_work_messages.at(i));
        }
        if (reply.wreq.no_app_version) {
            reply.set_delay(DELAY_NO_WORK_PERM);
        }
        if (reply.wreq.no_allowed_apps_available) {
            USER_MESSAGE um(
                "No work available for the applications you have selected.  Please check your settings on the website.",
                "high"
            );
            reply.insert_message(um);
        }
        if (reply.wreq.speed.insufficient) {
            if (reply.wreq.core_client_version>419) {
                sprintf(helpful,
                    "(won't finish in time) "
                    "Computer on %.1f%% of time, BOINC on %.1f%% of that",
                    100.0*reply.host.on_frac, 100.0*reply.host.active_frac
                );
            } else {
                sprintf(helpful,
                    "(won't finish in time) "
                    "Computer available %.1f%% of time",
                    100.0*reply.host.on_frac
                );
            }
            USER_MESSAGE um(helpful, "high");
            reply.insert_message(um);
        }
        if (reply.wreq.hr_reject_temp) {
            USER_MESSAGE um(
                "(there was work but it was committed to other platforms)",
                "high"
            );
            reply.insert_message(um);
        }
        if (reply.wreq.hr_reject_perm) {
            USER_MESSAGE um(
                "(your platform is not supported by this project)",
                "high"
            );
            reply.insert_message(um);
        }
        if (reply.wreq.outdated_core) {
            USER_MESSAGE um(
                " (your BOINC client is old - please install current version)",
                "high"
            );
            reply.insert_message(um);
            reply.set_delay(DELAY_NO_WORK_PERM);
            log_messages.printf(MSG_NORMAL,
                "Not sending work because client is outdated\n"
            );
        }
        if (reply.wreq.excessive_work_buf) {
            USER_MESSAGE um(
                "(Your network connection interval is longer than WU deadline)",
                "high"
            );
            reply.insert_message(um);
        }
        if (reply.wreq.daily_result_quota_exceeded) {
            struct tm *rpc_time_tm;
            int delay_time;

            sprintf(helpful, "(reached daily quota of %d results)", reply.wreq.daily_result_quota);
            USER_MESSAGE um(helpful, "high");
            reply.insert_message(um);
            log_messages.printf(MSG_NORMAL,
                "Daily result quota exceeded for host %d\n",
                reply.host.id
            );

            // set delay so host won't return until a random time in
            // the first hour of 'the next day'.  This is to prevent a
            // lot of hosts from flooding the scheduler with requests
            // at the same time of day.
            rpc_time_tm = localtime((const time_t*)&reply.host.rpc_time);
            delay_time  = (23 - rpc_time_tm->tm_hour) * 3600 +
                          (59 - rpc_time_tm->tm_min) * 60 +
                          (60 - rpc_time_tm->tm_sec) + 
                          (int)(3600*(double)rand()/(double)RAND_MAX);
            reply.set_delay(delay_time);
        }
        if (reply.wreq.cache_size_exceeded) {
            sprintf(helpful, "(reached per-host limit of %d tasks)",
                config.max_wus_in_progress
            );
            USER_MESSAGE um(helpful, "high");
            reply.insert_message(um);
            reply.set_delay(DELAY_NO_WORK_CACHE);
            log_messages.printf(MSG_NORMAL,
                "host %d already has %d result(s) on cache\n",
                reply.host.id, reply.wreq.nresults_on_host
            );
        }        
    }
}

#ifdef MATCHMAKER

struct JOB{
    int index;
    double value;
    double est_time;
    double disk_usage;
    APP* app;
    APP_VERSION* avp;

    void get_value(SCHEDULER_REQUEST&, SCHEDULER_REPLY&);
};

struct JOB_SET {
    double work_req;
    double est_time;
    double disk_usage;
    double disk_limit;
    std::list<JOB> jobs;     // sorted high to low

    void add_job(JOB&);
    double higher_value_disk_usage(double);
    double lowest_value();
    inline bool request_satisfied() {
        return est_time >= work_req;
    }
    void send(SCHEDULER_REQUEST&, SCHEDULER_REPLY&);
};

// reread result from DB, make sure it's still unsent
// TODO: from here to add_result_to_reply()
// (which updates the DB record) should be a transaction
//
int read_sendable_result(DB_RESULT& result) {
    int retval = result.lookup_id(result.id);
    if (retval) {
        log_messages.printf(MSG_CRITICAL,
            "[RESULT#%d] result.lookup_id() failed %d\n",
            result.id, retval
        );
        return ERR_NOT_FOUND;
    }
    if (result.server_state != RESULT_SERVER_STATE_UNSENT) {
        log_messages.printf(MSG_DEBUG,
            "[RESULT#%d] expected to be unsent; instead, state is %d\n",
            result.id, result.server_state
        );
        return ERR_BAD_RESULT_STATE;
    }
    return 0;
}

// compute a "value" for sending this WU to this host.
// return 0 if the WU is infeasible
//
void JOB::get_value(SCHEDULER_REQUEST& sreq, SCHEDULER_REPLY& reply) {
    bool found;
    WORKUNIT wu;
    int retval;

    WU_RESULT& wu_result = ssp->wu_results[index];
    wu = wu_result.workunit;

    value = 0;

    // Find the app and app_version for the client's platform.
    //
    if (anonymous(sreq.platforms.list[0])) {
        app = ssp->lookup_app(wu.appid);
        found = sreq.has_version(*app);
        if (!found) return;
        avp = NULL;
    } else {
        found = get_app_version(sreq, reply, wu, app, avp);
        if (!found) return;

        // see if the core client is too old.
        // don't bump the infeasible count because this
        // isn't the result's fault
        //
        if (!app_core_compatible(reply.wreq, *avp)) {
            return;
        }
    }
    if (app == NULL) return; // this should never happen

    retval = wu_is_infeasible(wu, sreq, reply, *app);
    if (retval) {
        log_messages.printf(MSG_DEBUG,
            "[HOST#%d] [WU#%d %s] WU is infeasible: %s\n",
            reply.host.id, wu.id, wu.name, infeasible_string(retval)
        );
        return;
    }

    value = 1;
    double val = 1;
    if (app->beta) {
        if (reply.wreq.host_info.allow_beta_work) {
            value += 1;
        } else {
            value = 0;
            return;
        }
    } else {
        if (reply.wreq.host_info.reliable && (wu_result.need_reliable)) {
            value += 1;
        }
    }
    
    if (wu_result.infeasible_count) {
        value += 1;
    }
}

bool wu_is_infeasible_slow(
    WU_RESULT& wu_result, SCHEDULER_REQUEST& sreq, SCHEDULER_REPLY& reply
) {
    char buf[256];
    int retval;
    int n;
    DB_RESULT result;

    // Don't send if we've already sent a result of this WU to this user.
    //
    if (config.one_result_per_user_per_wu) {
        sprintf(buf,
            "where workunitid=%d and userid=%d",
            wu_result.workunit.id, reply.user.id
        );
        retval = result.count(n, buf);
        if (retval) {
            log_messages.printf(MSG_CRITICAL,
                "send_work: can't get result count (%d)\n", retval
            );
            return true;
        } else {
            if (n>0) {
                log_messages.printf(MSG_DEBUG,
                    "send_work: user %d already has %d result(s) for WU %d\n",
                    reply.user.id, n, wu_result.workunit.id
                );
                return true;
            }
        }
    } else if (config.one_result_per_host_per_wu) {
        // Don't send if we've already sent a result
        // of this WU to this host.
        // We only have to check this
        // if we don't send one result per user.
        //
        sprintf(buf,
            "where workunitid=%d and hostid=%d",
            wu_result.workunit.id, reply.host.id
        );
        retval = result.count(n, buf);
        if (retval) {
            log_messages.printf(MSG_CRITICAL,
                "send_work: can't get result count (%d)\n", retval
            );
            return true;
        } else {
            if (n>0) {
                log_messages.printf(MSG_DEBUG,
                    "send_work: host %d already has %d result(s) for WU %d\n",
                    reply.host.id, n, wu_result.workunit.id
                );
            return true;
            }
        }
    }

    APP* app = ssp->lookup_app(wu_result.workunit.appid);
    WORKUNIT wu = wu_result.workunit;
    if (app_hr_type(*app)) {
        if (already_sent_to_different_platform_careful(
            sreq, reply.wreq, wu, *app
        )) {
             log_messages.printf(MSG_DEBUG,
                "[HOST#%d] [WU#%d %s] WU is infeasible (assigned to different platform)\n",
                reply.host.id, wu.id, wu.name
            );
            // Mark the workunit as infeasible.
            // This ensures that jobs already assigned to a platform
            // are processed first.
            //
            wu_result.infeasible_count++;
            return true;
        }
    }
    return false;
}

double JOB_SET::lowest_value() {
    if (jobs.empty()) return 0;
    return jobs.back().value;
}

// add the given job, and remove lowest-value jobs
// that are in excess of work request
// or that cause the disk limit to be exceeded
//
void JOB_SET::add_job(JOB& job) {
    while (!jobs.empty()) {
        JOB& worst_job = jobs.back();
        if (est_time + job.est_time - worst_job.est_time > work_req) {
            est_time -= worst_job.est_time;
            disk_usage -= worst_job.disk_usage;
            jobs.pop_back();
        }
    }
    while (!jobs.empty()) {
        JOB& worst_job = jobs.back();
        if (disk_usage + job.disk_usage > disk_limit) {
            est_time -= worst_job.est_time;
            disk_usage -= worst_job.disk_usage;
            jobs.pop_back();
        }
    }
    list<JOB>::iterator i = jobs.begin();
    while (i != jobs.end()) {
        if (i->value < job.value) {
            jobs.insert(i, job);
            break;
        }
        i++;
    }
    if (i == jobs.end()) {
        jobs.push_back(job);
    }
    est_time += job.est_time;
    disk_usage += job.disk_usage;
}

// return the disk usage of jobs above the given value
//
double JOB_SET::higher_value_disk_usage(double v) {
    double sum = 0;
    list<JOB>::iterator i = jobs.begin();
    while (i != jobs.end()) {
        if (i->value < v) break;
        sum += i->disk_usage;
        i++;
    }
    return sum;
}

void JOB_SET::send(SCHEDULER_REQUEST& sreq, SCHEDULER_REPLY& reply) {
    WORKUNIT wu;
    DB_RESULT result;
    int retval;

    list<JOB>::iterator i = jobs.begin();
    while (i != jobs.end()) {
        JOB& job = *i;
        WU_RESULT wu_result = ssp->wu_results[job.index];
        ssp->wu_results[job.index].state = WR_STATE_EMPTY;
        wu = wu_result.workunit;
        result.id = wu_result.resultid;
        retval = read_sendable_result(result);
        if (retval) continue;
        add_result_to_reply(result, wu, sreq, reply, job.app, job.avp);
        i++;
    }
}

void send_work_matchmaker(SCHEDULER_REQUEST& sreq, SCHEDULER_REPLY& reply) {
    int i, slots_scanned=0, slots_locked=0;
    JOB_SET jobs;
    int min_slots = 20;
    int max_slots = 50;
    int max_locked = 10;
    int pid = getpid();

    lock_sema();
    i = rand() % ssp->max_wu_results;
    while (1) {
        i = (i+1) % ssp->max_wu_results;
        slots_scanned++;
        if (slots_scanned >= max_slots) break;
        WU_RESULT& wu_result = ssp->wu_results[i];
        switch (wu_result.state) {
        case WR_STATE_EMPTY:
            continue;
        case WR_STATE_PRESENT:
            break;
        default:
            slots_locked++;
            continue;
        }

        JOB job;
        job.index = i;
        job.get_value(sreq, reply);
        if (job.value > jobs.lowest_value()) {
            ssp->wu_results[i].state = pid;
            unlock_sema();
            if (wu_is_infeasible_slow(wu_result, sreq, reply)) {
                lock_sema();
                ssp->wu_results[i].state = WR_STATE_EMPTY;
                continue;
            }
            lock_sema();
            jobs.add_job(job);
        }

        if (jobs.request_satisfied() && slots_scanned>=min_slots) break;
    }

    jobs.send(sreq, reply);
    unlock_sema();
    if (slots_locked > max_locked) {
        log_messages.printf(MSG_CRITICAL,
            "Found too many locked slots (%d>%d) - increase array size",
            slots_locked, max_locked
        );
    }
}
#endif

const char *BOINC_RCSID_32dcd335e7 = "$Id$";
