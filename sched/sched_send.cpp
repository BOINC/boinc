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

// scheduler code related to sending jobs

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
#include "boinc_fcgi.h"
#endif


void send_work_matchmaker(SCHEDULER_REQUEST& sreq, SCHEDULER_REPLY& reply);

int preferred_app_message_index=0;

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

// return a number that
// - is the # of CPUs in EDF simulation
// - scales the daily result quota
// - scales max_wus_in_progress

inline int effective_ncpus(SCHEDULER_REQUEST& sreq, SCHEDULER_REPLY& reply) {
    int ncpus = reply.host.p_ncpus;
    if (ncpus > config.max_ncpus) ncpus = config.max_ncpus;
    if (ncpus < 1) ncpus = 1;
    if (config.have_cuda_apps) {
        COPROC* cp = sreq.coprocs.lookup("cuda");
        if (cp && cp->count > ncpus) {
            ncpus = cp->count;
        }
    }
    return ncpus;
}

const double DEFAULT_RAM_SIZE = 64000000;
    // if host sends us an impossible RAM size, use this instead

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
    unsigned int i;
    int j;
    BEST_APP_VERSION* bavp;
    char message[256];

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
        log_messages.printf(MSG_CRITICAL, "WU refers to nonexistent app: %d\n", wu.appid);
        return NULL;
    }

    bavp = new BEST_APP_VERSION;
    bavp->appid = wu.appid;
    if (anonymous(sreq.platforms.list[0])) {
        found = sreq.has_version(*app);
        if (!found) {
            if (config.debug_send) {
                log_messages.printf(MSG_DEBUG,
                    "Didn't find anonymous platform app for %s\n", app->name
                );
                sprintf(message,
                    "Your app_info.xml file doesn't have a version of %s.",
                    app->user_friendly_name
                );
                USER_MESSAGE um(message, "high");
                reply.wreq.insert_no_work_message(um);
                reply.wreq.no_app_version = true;
            }
            bavp->avp = 0;
        } else {
            if (config.debug_send) {
                log_messages.printf(MSG_DEBUG,
                    "Found anonymous platform app for %s\n", app->name
                );
            }
            // TODO: anonymous platform apps should be able to tell us
            // how fast they are and how many CPUs and coprocs they use.
            // For now, assume they use 1 CPU
            //
            bavp->host_usage.sequential_app(reply.host.p_fpops);
            bavp->avp = (APP_VERSION*)1;    // arbitrary nonzero value;
                // means the client already has the app version
        }
        reply.wreq.best_app_versions.push_back(bavp);
        if (!bavp->avp) return NULL;
        return bavp;
    }

    // Go through the client's platforms.
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
            if (sreq.core_client_version < av.min_core_version) {
                log_messages.printf(MSG_NORMAL,
                    "outdated client version %d < min core version %d\n",
                    sreq.core_client_version, av.min_core_version
                );
                reply.wreq.outdated_core = true;
                continue;
            }
            if (strlen(av.plan_class)) {
                if (!sreq.client_cap_plan_class) continue;
                if (!app_plan(sreq, av.plan_class, host_usage)) {
                    continue;
                }
            } else {
                host_usage.sequential_app(reply.host.p_fpops);
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
                "Best version of app %s is %d (%.2f GFLOPS)\n",
                app->name, bavp->avp->id, bavp->host_usage.flops/1e9
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
        sprintf(message,
            "%s is not available for your type of computer.",
            app->user_friendly_name
        );
        USER_MESSAGE um(message, "high");
        reply.wreq.insert_no_work_message(um);
        reply.wreq.no_app_version = true;
        return NULL;
    }
    return bavp;
}

static const char* find_user_friendly_name(int appid) {
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
        x1 = prefs.disk_max_used_gb*GIGA - host.d_boinc_used_total;
        x2 = host.d_total*prefs.disk_max_used_pct/100.
            - host.d_boinc_used_total;
        x3 = host.d_free - prefs.disk_min_free_gb*GIGA;      // may be negative
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
        x = host.d_free - prefs.disk_min_free_gb*GIGA;      // may be negative
        reply.disk_limits.min_free = x;
        x1 = x2 = x3 = 0;
    }

    if (x < 0) {
        if (config.debug_send) {
            log_messages.printf(MSG_DEBUG,
                "No disk space available: disk_max_used_gb %f disk_max_used_pct %f disk_min_free_gb %f\n",
                prefs.disk_max_used_gb, prefs.disk_max_used_pct,
                prefs.disk_min_free_gb
            );
            log_messages.printf(MSG_DEBUG,
                "No disk space available: host.d_total %f host.d_free %f host.d_boinc_used_total %f\n",
                host.d_total, host.d_free, host.d_boinc_used_total
            );
            log_messages.printf(MSG_DEBUG,
                "No disk space available: x1 %f x2 %f x3 %f x %f\n",
                x1, x2, x3, x
            );
        }
        reply.wreq.disk.set_insufficient(-x);
        x = 0;
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
    WORKUNIT& wu, SCHEDULER_REQUEST&, SCHEDULER_REPLY& reply
) {
    double ecd = estimate_cpu_duration(wu, reply);
    double ewd = ecd/reply.wreq.running_frac;
    if (reply.host.duration_correction_factor) {
        ewd *= reply.host.duration_correction_factor;
    }
    if (reply.host.cpu_efficiency) {
        ewd /= reply.host.cpu_efficiency;
    }
    if (config.debug_send) {
        log_messages.printf(MSG_DEBUG,
            "est cpu dur %f;  est wall dur %f\n", ecd, ewd
        );
    }
    return ewd;
}

// Find or compute various info about the host;
// this info affects which jobs are sent to the host.
//
static int get_host_info(SCHEDULER_REPLY& reply) {
    char buf[8096];
    string str;
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
	if (parse_bool(buf,"allow_non_preferred_apps", flag)) {
	    reply.wreq.host_info.allow_non_preferred_apps = flag;
    }
	if (parse_bool(buf,"allow_beta_work", flag)) {
        reply.wreq.host_info.allow_beta_work = flag;
	}
 
    // Decide whether or not this computer is 'reliable'
    // A computer is reliable if the following conditions are true
    // (for those that are set in the config file)
    // 1) The host average turnaround is less than the config
    // max average turnaround
    // 2) The host error rate is less then the config max error rate
    // 3) The host results per day is equal to the config file value
    //
    double expavg_credit = reply.host.expavg_credit;
    double expavg_time = reply.host.expavg_time;
    update_average(0, 0, CREDIT_HALF_LIFE, expavg_credit, expavg_time);

	// Platforms other then Windows, Linux and Intel Macs need a
    // larger set of computers to be marked reliable
    //
    double multiplier = 1.0;
    if (strstr(reply.host.os_name,"Windows")
        || strstr(reply.host.os_name,"Linux")
        || (strstr(reply.host.os_name,"Darwin")
            && !(strstr(reply.host.p_vendor,"Power Macintosh"))
    )) {
    	multiplier = 1.0;
    } else {
    	multiplier = 1.8;
    }

    if ((config.reliable_max_avg_turnaround == 0 || reply.host.avg_turnaround < config.reliable_max_avg_turnaround*multiplier)
        && (config.reliable_max_error_rate == 0 || reply.host.error_rate < config.reliable_max_error_rate*multiplier)
        && (config.daily_result_quota == 0 || reply.host.max_results_day >= config.daily_result_quota)
     ) {
        reply.wreq.host_info.reliable = true;
    }
    if (config.debug_send) {
        log_messages.printf(MSG_DEBUG,
            "[HOST#%d] is%s reliable (OS = %s) error_rate = %.6f avg_turn_hrs = %.3f \n",
            reply.host.id,
            reply.wreq.host_info.reliable?"":" not",
            reply.host.os_name, reply.host.error_rate,
            reply.host.avg_turnaround/3600
        );
    }
    return 0;
}

// Return true if the user has set application preferences,
// and this job is not for a selected app
//
bool app_not_selected(
    WORKUNIT& wu, SCHEDULER_REQUEST& , SCHEDULER_REPLY& reply
) {
    unsigned int i;

    if (reply.wreq.host_info.preferred_apps.size() == 0) return false;
    for (i=0; i<reply.wreq.host_info.preferred_apps.size(); i++) {
        if (wu.appid == reply.wreq.host_info.preferred_apps[i].appid) {
    	    reply.wreq.host_info.preferred_apps[i].work_available = true;
            return false;
        }
    }
    return true;
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
        
        if (config.debug_send) {
            log_messages.printf(MSG_DEBUG,
                "[WU#%d %s] needs %0.2fMB RAM; [HOST#%d] has %0.2fMB, %0.2fMB usable\n",
                wu.id, wu.name, wu.rsc_memory_bound/MEGA,
                reply.host.id, ram/MEGA, usable_ram/MEGA
            );
        }
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
    
    // if n_bwdown is zero, the host has never downloaded anything,
    // so skip this check
    //
    if (reply.host.n_bwdown == 0) return 0;

    double diff = wu.rsc_bandwidth_bound - reply.host.n_bwdown;
    if (diff > 0) {
        char message[256];
        sprintf(message,
            "%s requires %0.2f KB/sec download bandwidth.  Your computer has been measured at %0.2f KB/sec.",
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

// Determine if the app is "hard",
// and we should send it only to high-end hosts.
// Currently this is specified by setting weight=-1;
// this is a kludge for SETI@home/Astropulse.
//
static inline bool hard_app(APP& app) {
    return (app.weight == -1);
}

static inline int check_deadline(
    WORKUNIT& wu, APP& app, SCHEDULER_REQUEST& request, SCHEDULER_REPLY& reply
) {
    if (config.ignore_delay_bound) return 0;

    // skip delay check if host currently doesn't have any work
    // and it's not a hard app.
    // (i.e. everyone gets one result, no matter how slow they are)
    //
    if (request.estimated_delay == 0 && !hard_app(app)) return 0;

    // if it's a hard app, don't send it to a host with no credit
    //
    if (hard_app(app) && reply.host.total_credit == 0) {
        return INFEASIBLE_CPU;
    }

    double ewd = estimate_wallclock_duration(wu, request, reply);
    if (hard_app(app)) ewd *= 1.3;
    double est_completion_delay = request.estimated_delay + ewd;
    double est_report_delay = max(est_completion_delay, request.global_prefs.work_buf_min());
    double diff = est_report_delay - wu.delay_bound;
    if (diff > 0) {
        if (config.debug_send) {
            log_messages.printf(MSG_DEBUG,
                "[WU#%d %s] est report delay %d on [HOST#%d]; delay_bound is %d\n",
                wu.id, wu.name, (int)est_report_delay,
                reply.host.id, wu.delay_bound
            );
        }
        reply.wreq.speed.set_insufficient(diff);
        return INFEASIBLE_CPU;
    }
    return 0;
}

// Fast checks (no DB access) to see if the job can be sent to the host.
// Reasons why not include:
// 1) the host doesn't have enough memory;
// 2) the host doesn't have enough disk space;
// 3) based on CPU speed, resource share and estimated delay,
//    the host probably won't get the result done within the delay bound
// 4) app isn't in user's "approved apps" list
//
int wu_is_infeasible_fast(
    WORKUNIT& wu, SCHEDULER_REQUEST& request, SCHEDULER_REPLY& reply, APP& app
) {
    int retval;

    // homogeneous redundancy, quick check
    //
    if (app_hr_type(app)) {
        if (hr_unknown_platform_type(reply.host, app_hr_type(app))) {
            if (config.debug_send) {
                log_messages.printf(MSG_DEBUG,
                    "[HOST#%d] [WU#%d %s] host is of unknown class in HR type %d\n",
                    reply.host.id, wu.id, wu.name, app_hr_type(app)
                );
            }
            return INFEASIBLE_HR;
        }
        if (already_sent_to_different_platform_quick(request, wu, app)) {
            if (config.debug_send) {
                log_messages.printf(MSG_DEBUG,
                    "[HOST#%d] [WU#%d %s] failed quick HR check: WU is class %d, host is class %d\n",
                    reply.host.id, wu.id, wu.name, wu.hr_class, hr_class(request.host, app_hr_type(app))
                );
            }
            return INFEASIBLE_HR;
        }
    }
    
    if (config.one_result_per_user_per_wu || config.one_result_per_host_per_wu) {
        if (wu_already_in_reply(wu, reply)) {
            return INFEASIBLE_DUP;
        }
    }

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
        if (reply.wreq.edf_reject_test(est_cpu, wu.delay_bound)) {
            return INFEASIBLE_WORKLOAD;
        }
        IP_RESULT candidate("", wu.delay_bound, est_cpu);
        strcpy(candidate.name, wu.name);
        if (check_candidate(candidate, effective_ncpus(request, reply), request.ip_results)) {
            // it passed the feasibility test,
            // but don't add it the the workload yet;
            // wait until we commit to sending it
        } else {
            reply.wreq.edf_reject(est_cpu, wu.delay_bound);
            reply.wreq.speed.set_insufficient(0);
            return INFEASIBLE_WORKLOAD;
        }
    } else {
        retval = check_deadline(wu, app, request, reply);
        if (retval) return INFEASIBLE_WORKLOAD;
    }

    return 0;
}

// insert "text" right after "after" in the given buffer
//
int insert_after(char* buffer, const char* after, const char* text) {
    char* p;
    char temp[BLOB_SIZE];

    if (strlen(buffer) + strlen(text) > BLOB_SIZE-1) {
        log_messages.printf(MSG_CRITICAL,
            "insert_after: overflow: %d %d\n", strlen(buffer), strlen(text)
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
    char buf[BLOB_SIZE];
    
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
        av2.bavp = bavp;
        reply.insert_app_version_unique(*avp2);
        if (config.debug_send) {
            log_messages.printf(MSG_DEBUG,
                "[HOST#%d] Sending app_version %s %d %d\n",
                reply.host.id, app->name, avp2->platformid, avp2->version_num
            );
        }
    }

    // add time estimate to reply
    //
    wu2 = wu;       // make copy since we're going to modify its XML field
    retval = insert_wu_tags(wu2, *app);
    if (retval) {
        log_messages.printf(MSG_CRITICAL, "insert_wu_tags failed %d\n", retval);
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
bool work_needed(
    SCHEDULER_REQUEST& sreq, SCHEDULER_REPLY& reply, bool locality_sched
) {
    if (locality_sched) {
        // if we've failed to send a result because of a transient condition,
        // return false to preserve invariant
        //
        if (reply.wreq.disk.insufficient || reply.wreq.speed.insufficient || reply.wreq.mem.insufficient || reply.wreq.no_allowed_apps_available) {
            return false;
        }
    }
    if (reply.wreq.seconds_to_fill <= 0) return false;
    if (reply.wreq.nresults >= config.max_wus_to_send) return false;

    int ncpus = effective_ncpus(sreq, reply);

    // host.max_results_day is between 1 and config.daily_result_quota inclusive
    // wreq.daily_result_quota is between ncpus
    // and ncpus*host.max_results_day inclusive
    //
    if (config.daily_result_quota) {
        if (reply.host.max_results_day == 0 || reply.host.max_results_day>config.daily_result_quota) {
            reply.host.max_results_day = config.daily_result_quota;
        }
        reply.wreq.daily_result_quota = ncpus*reply.host.max_results_day;
        if (reply.host.nresults_today >= reply.wreq.daily_result_quota) {
            reply.wreq.daily_result_quota_exceeded = true;
            return false;
        }
    }

    if (config.max_wus_in_progress) {
        if (reply.wreq.nresults_on_host >= config.max_wus_in_progress*ncpus) {
            if (config.debug_send) {
                log_messages.printf(MSG_DEBUG,
                    "in-progress job limit exceeded; %d > %d*%d\n",
                    reply.wreq.nresults_on_host, config.max_wus_in_progress, ncpus
                );
            }
            reply.wreq.cache_size_exceeded = true;
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

        if (config.debug_send) {
            log_messages.printf(MSG_DEBUG,
                "[RESULT#%d] [HOST#%d] (resend lost work)\n",
                result.id, reply.host.id
            );
        }
    }
    retval = result.mark_as_sent(old_server_state);
    if (retval == ERR_DB_NOT_FOUND) {
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
    if (config.debug_send) {
        log_messages.printf(MSG_NORMAL,
            "[HOST#%d] Sending [RESULT#%d %s] (fills %.2f seconds)\n",
            reply.host.id, result.id, result.name, wu_seconds_filled
        );
    }

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
    request.estimated_delay += wu_seconds_filled/effective_ncpus(request, reply);
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

    // mark job as done if debugging flag is set;
    // this is used by sched_driver.C (performance testing)
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
        sprintf(buf, "transition_time=%ld", time(0));
        dbwu.update_field(buf);

    }

    // If we're sending an unreplicated job to an untrusted host,
    // mark it as replicated
    //
    if (wu.target_nresults == 1 && app->target_nresults > 1) {
        if (reply.wreq.trust) {
            if (config.debug_send) {
                log_messages.printf(MSG_DEBUG,
                    "[WU#%d] sending to trusted host, not replicating\n", wu.id
                );
            }
        } else {
            DB_WORKUNIT dbwu;
            char buf[256];
            sprintf(buf,
                "target_nresults=%d, min_quorum=%d, transition_time=%ld",
                app->target_nresults, app->target_nresults, time(0)
            );
            dbwu.id = wu.id;
            if (config.debug_send) {
                log_messages.printf(MSG_DEBUG,
                    "[WU#%d] sending to untrusted host, replicating\n", wu.id
                );
            }
            retval = dbwu.update_field(buf);
            if (retval) {
                log_messages.printf(MSG_CRITICAL,
                    "WU update failed: %d", retval
                );
            }
        }
    }

    return 0;
}

// send messages to user about why jobs were or weren't sent
//
static void explain_to_user(SCHEDULER_REQUEST& sreq, SCHEDULER_REPLY& reply) {
    char helpful[512];
    unsigned int i;
    int j;

    // If work was sent from apps the user did not select, explain.
    // NOTE: this will have to be done differently with matchmaker scheduling
    //
    if (!config.locality_scheduling && !config.matchmaker) {
        if (reply.wreq.nresults && !reply.wreq.user_apps_only) {
            USER_MESSAGE um(
                "No work can be sent for the applications you have selected",
                "high"
            );
            reply.insert_message(um);

            // Inform the user about applications with no work
            //
            for (i=0; i<reply.wreq.host_info.preferred_apps.size(); i++) {
                if (!reply.wreq.host_info.preferred_apps[i].work_available) {
                    APP* app = ssp->lookup_app(reply.wreq.host_info.preferred_apps[i].appid);
                    // don't write message if the app is deprecated
                    //
                    if (app) {
                        char explanation[256];
                        sprintf(explanation,
                            "No work is available for %s",
                            find_user_friendly_name(reply.wreq.host_info.preferred_apps[i].appid)
                        );
                        USER_MESSAGE um2(explanation, "high");
                        reply.insert_message(um2);
                    }
                }
            }

            // Tell the user about applications they didn't qualify for
            //
            for (j=0; j<preferred_app_message_index; j++){
                reply.insert_message(reply.wreq.no_work_messages.at(j));
            }
            USER_MESSAGE um1(
                "You have selected to receive work from other applications if no work is available for the applications you selected",
                "high"
            );
            reply.insert_message(um1);
            USER_MESSAGE um2("Sending work from other applications", "high");
            reply.insert_message(um2);
        }
    }

    // if client asked for work and we're not sending any, explain why
    //
    if (reply.wreq.nresults == 0) {
        reply.set_delay(DELAY_NO_WORK_TEMP);
        USER_MESSAGE um2("No work sent", "high");
        reply.insert_message(um2);
        // Inform the user about applications with no work
        for (i=0; i<reply.wreq.host_info.preferred_apps.size(); i++) {
         	if (!reply.wreq.host_info.preferred_apps[i].work_available) {
         		APP* app = ssp->lookup_app(reply.wreq.host_info.preferred_apps[i].appid);
         		// don't write message if the app is deprecated
         		if (app != NULL) {
           			char explanation[256];
           			sprintf(explanation, "No work is available for %s",
                        find_user_friendly_name(reply.wreq.host_info.preferred_apps[i].appid)
                    );
        			USER_MESSAGE um(explanation, "high");
           			reply.insert_message(um);
         		}
           	}
        }
        // Inform the user about applications they didn't qualify for
        for (i=0; i<reply.wreq.no_work_messages.size(); i++){
        	reply.insert_message(reply.wreq.no_work_messages.at(i));
        }
        if (reply.wreq.no_app_version) {
            reply.set_delay(DELAY_NO_WORK_PERM);
        }
        if (reply.wreq.no_allowed_apps_available) {
            USER_MESSAGE um(
                "No work available for the applications you have selected.  Please check your settings on the web site.",
                "high"
            );
            reply.insert_message(um);
        }
        if (reply.wreq.speed.insufficient) {
            if (sreq.core_client_version>419) {
                sprintf(helpful,
                    "(won't finish in time) "
                    "BOINC runs %.1f%% of time, computation enabled %.1f%% of that",
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
        if (reply.wreq.no_jobs_available) {
            USER_MESSAGE um(
                "(Project has no jobs available)",
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
            // the first hour of the next day.
            // This is to prevent a lot of hosts from flooding the scheduler
            // with requests at the same time of day.
            //
            time_t t = reply.host.rpc_time;
            rpc_time_tm = localtime(&t);
            delay_time  = (23 - rpc_time_tm->tm_hour) * 3600
                + (59 - rpc_time_tm->tm_min) * 60
                + (60 - rpc_time_tm->tm_sec)
                + (int)(3600*(double)rand()/(double)RAND_MAX);
            reply.set_delay(delay_time);
        }
        if (reply.wreq.cache_size_exceeded) {
            sprintf(helpful, "(reached per-CPU limit of %d tasks)",
                config.max_wus_in_progress
            );
            USER_MESSAGE um(helpful, "high");
            reply.insert_message(um);
            reply.set_delay(DELAY_NO_WORK_CACHE);
            log_messages.printf(MSG_NORMAL,
                "host %d already has %d result(s) in progress\n",
                reply.host.id, reply.wreq.nresults_on_host
            );
        }        
    }
}

static void get_running_frac(SCHEDULER_REQUEST& sreq, SCHEDULER_REPLY& reply) {
    if (sreq.core_client_version<=419) {
        reply.wreq.running_frac = reply.host.on_frac;
    } else {
        reply.wreq.running_frac = reply.host.active_frac * reply.host.on_frac;
    }
    if (reply.wreq.running_frac < HOST_ACTIVE_FRAC_MIN) {
        reply.wreq.running_frac = HOST_ACTIVE_FRAC_MIN;
    }
    if (reply.wreq.running_frac > 1) reply.wreq.running_frac = 1;
}

static void send_work_old(SCHEDULER_REQUEST& sreq, SCHEDULER_REPLY& reply) {
    reply.wreq.beta_only = false;
    reply.wreq.user_apps_only = true;

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
        if (config.debug_send) {
            log_messages.printf(MSG_DEBUG,
                "[HOST#%d] will accept beta work.  Scanning for beta work.\n",
                reply.host.id
            );
        }
        scan_work_array(sreq, reply);
    }
    reply.wreq.beta_only = false;

    // give next priority to results that were infeasible for some other host
    //
    reply.wreq.infeasible_only = true;
    scan_work_array(sreq, reply);

    reply.wreq.infeasible_only = false;
    scan_work_array(sreq, reply);
    
    // If user has selected apps but will accept any,
    // and we haven't found any jobs for selected apps, try others
    //
    if (!reply.wreq.nresults && reply.wreq.host_info.allow_non_preferred_apps ) {
        reply.wreq.user_apps_only = false;
        preferred_app_message_index = reply.wreq.no_work_messages.size();
        if (config.debug_send) {
            log_messages.printf(MSG_DEBUG,
                "[HOST#%d] is looking for work from a non-preferred application\n",
                reply.host.id
            );
        }
        scan_work_array(sreq, reply);
    }
}

#define ER_MAX  0.05
// decide whether to unreplicated jobs to this host
//
void set_trust(SCHEDULER_REPLY& reply) {
    reply.wreq.trust = false;
    if (reply.host.error_rate > ER_MAX) {
        if (config.debug_send) {
            log_messages.printf(MSG_DEBUG,
                "set_trust: error rate %f > %f, don't trust\n",
                reply.host.error_rate, ER_MAX
            );
        }
        return;
    }
    double x = sqrt(reply.host.error_rate/ER_MAX);
    if (drand() > x) reply.wreq.trust = true;
    if (config.debug_send) {
        log_messages.printf(MSG_DEBUG,
            "set_trust: random choice for error rate %f: %s\n",
            reply.host.error_rate, reply.wreq.trust?"yes":"no"
        );
    }
}

void send_work(SCHEDULER_REQUEST& sreq, SCHEDULER_REPLY& reply) {
    if (sreq.work_req_seconds <= 0) return;

    reply.wreq.disk_available = max_allowable_disk(sreq, reply);

    if (hr_unknown_platform(sreq.host)) {
        reply.wreq.hr_reject_perm = true;
        return;
    }

    get_host_info(reply); // parse project prefs for app details

    set_trust(reply);

    get_running_frac(sreq, reply);

    if (config.debug_send) {
        log_messages.printf(MSG_DEBUG,
            "%s matchmaker scheduling; %s EDF sim\n",
            config.matchmaker?"Using":"Not using",
            config.workload_sim?"Using":"Not using"
        );
        log_messages.printf(MSG_DEBUG,
            "available disk %f GB, work_buf_min %d\n",
            reply.wreq.disk_available/GIGA,
            (int)sreq.global_prefs.work_buf_min()
        );
        log_messages.printf(MSG_DEBUG,
            "running frac %f DCF %f CPU effic %f est delay %d\n",
            reply.wreq.running_frac,
            reply.host.duration_correction_factor,
            reply.host.cpu_efficiency,
            (int)sreq.estimated_delay
        );
    }

    reply.wreq.seconds_to_fill = sreq.work_req_seconds;
    if (reply.wreq.seconds_to_fill > MAX_SECONDS_TO_SEND) {
        reply.wreq.seconds_to_fill = MAX_SECONDS_TO_SEND;
    }
    if (reply.wreq.seconds_to_fill < MIN_SECONDS_TO_SEND) {
        reply.wreq.seconds_to_fill = MIN_SECONDS_TO_SEND;
    }

    if (config.enable_assignment) {
        if (send_assigned_jobs(sreq, reply)) {
            if (config.debug_assignment) {
                log_messages.printf(MSG_DEBUG,
                    "[HOST#%d] sent assigned jobs\n", reply.host.id
                );
            }
            return;
        }
    }

    if (config.workload_sim && sreq.have_other_results_list) {
        init_ip_results(
            sreq.global_prefs.work_buf_min(), effective_ncpus(sreq, reply), sreq.ip_results
        );
    }

    if (config.locality_scheduling) {
        reply.wreq.infeasible_only = false;
        send_work_locality(sreq, reply);
    } else if (config.matchmaker) {
        send_work_matchmaker(sreq, reply);
    } else {
        send_work_old(sreq, reply);
    }

    explain_to_user(sreq, reply);
}

// Matchmaker scheduling code follows

struct JOB {
    int index;
    double score;
    double est_time;
    double disk_usage;
    APP* app;
    BEST_APP_VERSION* bavp;

    bool get_score(SCHEDULER_REQUEST&, SCHEDULER_REPLY&);
};

struct JOB_SET {
    double work_req;
    double est_time;
    double disk_usage;
    double disk_limit;
    int max_jobs;
    std::list<JOB> jobs;     // sorted high to low

    JOB_SET(SCHEDULER_REQUEST& sreq, SCHEDULER_REPLY& reply) {
        work_req = sreq.work_req_seconds;
        est_time = 0;
        disk_usage = 0;
        disk_limit = reply.wreq.disk_available;
        max_jobs = config.max_wus_to_send;
        int ncpus = effective_ncpus(sreq, reply), n;

        if (config.daily_result_quota) {
            if (reply.host.max_results_day == 0 || reply.host.max_results_day>config.daily_result_quota) {
                reply.host.max_results_day = config.daily_result_quota;
            }
            reply.wreq.daily_result_quota = ncpus*reply.host.max_results_day;
            n = reply.wreq.daily_result_quota - reply.host.nresults_today;
            if (n < 0) n = 0;
            if (n < max_jobs) max_jobs = n;
        }

        if (config.max_wus_in_progress) {
            n = config.max_wus_in_progress*ncpus - reply.wreq.nresults_on_host;
            if (n < 0) n = 0;
            if (n < max_jobs) max_jobs = n;
        }
    }
    void add_job(JOB&);
    double higher_score_disk_usage(double);
    double lowest_score();
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
        log_messages.printf(MSG_NORMAL,
            "[RESULT#%d] expected to be unsent; instead, state is %d\n",
            result.id, result.server_state
        );
        return ERR_BAD_RESULT_STATE;
    }
    return 0;
}

// compute a "score" for sending this job to this host.
// Return false if the WU is infeasible.
// Otherwise set est_time and disk_usage.
//
bool JOB::get_score(SCHEDULER_REQUEST& sreq, SCHEDULER_REPLY& reply) {
    WORKUNIT wu;
    int retval;

    WU_RESULT& wu_result = ssp->wu_results[index];
    wu = wu_result.workunit;
    app = ssp->lookup_app(wu.appid);

    score = 0;

    // Find the app_version for the client's platform.
    //
    bavp = get_app_version(sreq, reply, wu);
    if (!bavp) return false;

    retval = wu_is_infeasible_fast(wu, sreq, reply, *app);
    if (retval) {
        if (config.debug_send) {
            log_messages.printf(MSG_DEBUG,
                "[HOST#%d] [WU#%d %s] WU is infeasible: %s\n",
                reply.host.id, wu.id, wu.name, infeasible_string(retval)
            );
        }
        return false;
    }

    score = 1;

    // check if user has selected apps,
    // and send beta work to beta users
    //
    if (app->beta && !config.distinct_beta_apps) {
        if (reply.wreq.host_info.allow_beta_work) {
            score += 1;
        } else {
            return false;
        }
    } else {
        if (app_not_selected(wu, sreq, reply)) {
            if (!reply.wreq.host_info.allow_non_preferred_apps) {
                return false;
            } else {
            // Allow work to be sent, but it will not get a bump in its score
            }
        } else {
            score += 1;
        }
    }
            
    // if job needs to get done fast, send to fast/reliable host
    //
    if (reply.wreq.host_info.reliable && (wu_result.need_reliable)) {
        score += 1;
    }
    
    // if job already committed to an HR class,
    // try to send to host in that class
    //
    if (wu_result.infeasible_count) {
        score += 1;
    }

    // Favor jobs that will run fast
    //
    score += bavp->host_usage.flops/1e9;

    // match large jobs to fast hosts
    //
    if (config.job_size_matching) {
        double host_stdev = (reply.host.p_fpops - ssp->perf_info.host_fpops_mean)/ ssp->perf_info.host_fpops_stdev;
        double diff = host_stdev - wu_result.fpops_size;
        score -= diff*diff;
    }

    // TODO: If user has selected some apps but will accept jobs from others,
    // try to send them jobs from the selected apps
    //

    est_time = estimate_wallclock_duration(wu, sreq, reply);
    disk_usage = wu.rsc_disk_bound;
    return true;
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
                if (config.debug_send) {
                    log_messages.printf(MSG_DEBUG,
                        "send_work: user %d already has %d result(s) for WU %d\n",
                        reply.user.id, n, wu_result.workunit.id
                    );
                }
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
                if (config.debug_send) {
                    log_messages.printf(MSG_DEBUG,
                        "send_work: host %d already has %d result(s) for WU %d\n",
                        reply.host.id, n, wu_result.workunit.id
                    );
                }
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
            if (config.debug_send) {
                log_messages.printf(MSG_DEBUG,
                    "[HOST#%d] [WU#%d %s] WU is infeasible (assigned to different platform)\n",
                    reply.host.id, wu.id, wu.name
                );
            }
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

double JOB_SET::lowest_score() {
    if (jobs.empty()) return 0;
    return jobs.back().score;
}

// add the given job, and remove lowest-score jobs that
// - are in excess of work request
// - are in excess of per-request or per-day limits
// - cause the disk limit to be exceeded
//
void JOB_SET::add_job(JOB& job) {
    while (!jobs.empty()) {
        JOB& worst_job = jobs.back();
        if (est_time + job.est_time - worst_job.est_time > work_req) {
            est_time -= worst_job.est_time;
            disk_usage -= worst_job.disk_usage;
            jobs.pop_back();
            ssp->wu_results[worst_job.index].state = WR_STATE_PRESENT;
        } else {
            break;
        }
    }
    while (!jobs.empty()) {
        JOB& worst_job = jobs.back();
        if (disk_usage + job.disk_usage > disk_limit) {
            est_time -= worst_job.est_time;
            disk_usage -= worst_job.disk_usage;
            jobs.pop_back();
            ssp->wu_results[worst_job.index].state = WR_STATE_PRESENT;
        } else {
            break;
        }
    }

    if (jobs.size() == max_jobs) {
        JOB& worst_job = jobs.back();
        jobs.pop_back();
        ssp->wu_results[worst_job.index].state = WR_STATE_PRESENT;
    }

    list<JOB>::iterator i = jobs.begin();
    while (i != jobs.end()) {
        if (i->score < job.score) {
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
    if (config.debug_send) {
        log_messages.printf(MSG_DEBUG,
            "added job to set.  est_time %f disk_usage %f\n",
            est_time, disk_usage
        );
    }
}

// return the disk usage of jobs above the given score
//
double JOB_SET::higher_score_disk_usage(double v) {
    double sum = 0;
    list<JOB>::iterator i = jobs.begin();
    while (i != jobs.end()) {
        if (i->score < v) break;
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
        JOB& job = *(i++);
        WU_RESULT wu_result = ssp->wu_results[job.index];
        ssp->wu_results[job.index].state = WR_STATE_EMPTY;
        wu = wu_result.workunit;
        result.id = wu_result.resultid;
        retval = read_sendable_result(result);
        if (!retval) {
            add_result_to_reply(result, wu, sreq, reply, job.bavp);
        }
    }
}

void send_work_matchmaker(SCHEDULER_REQUEST& sreq, SCHEDULER_REPLY& reply) {
    int i, slots_locked=0, slots_nonempty=0;
    JOB_SET jobs (sreq, reply);
    int min_slots = config.mm_min_slots;
    if (!min_slots) min_slots = ssp->max_wu_results/2;
    int max_slots = config.mm_max_slots;
    if (!max_slots) max_slots = ssp->max_wu_results;
    int max_locked = 10;

    lock_sema();
    i = rand() % ssp->max_wu_results;

    // scan through the job cache, maintaining a JOB_SET of jobs
    // that we can send to this client, ordered by score.
    //
    for (int slots_scanned=0; slots_scanned<max_slots; slots_scanned++) {
        i = (i+1) % ssp->max_wu_results;
        WU_RESULT& wu_result = ssp->wu_results[i];
        switch (wu_result.state) {
        case WR_STATE_EMPTY:
            continue;
        case WR_STATE_PRESENT:
            slots_nonempty++;
            break;
        default:
            slots_nonempty++;
            if (wu_result.state == g_pid) break;
            slots_locked++;
            continue;
        }

        JOB job;
        job.index = i;

        // get score for this job, and skip it if it fails quick check.
        // NOTE: the EDF check done in get_score()
        // includes only in-progress jobs.
        //
        if (!job.get_score(sreq, reply)) {
            continue;
        }
        if (config.debug_send) {
            log_messages.printf(MSG_DEBUG,
                "score for %s: %f\n", wu_result.workunit.name, job.score
            );
        }

        if (job.score > jobs.lowest_score() || !jobs.request_satisfied()) {
            ssp->wu_results[i].state = g_pid;
            unlock_sema();
            if (wu_is_infeasible_slow(wu_result, sreq, reply)) {
                // if we can't use this job, put it back in pool
                //
                lock_sema();
                ssp->wu_results[i].state = WR_STATE_PRESENT;
                continue;
            }
            lock_sema();
            jobs.add_job(job);
        }

        if (jobs.request_satisfied() && slots_scanned>=min_slots) break;
    }

    if (!slots_nonempty) {
        log_messages.printf(MSG_CRITICAL,
            "Job cache is empty - check feeder\n"
        );
        reply.wreq.no_jobs_available = true;
    }

    // TODO: trim jobs from tail of list until we pass the EDF check
    //
    jobs.send(sreq, reply);
    unlock_sema();
    if (slots_locked > max_locked) {
        log_messages.printf(MSG_CRITICAL,
            "Found too many locked slots (%d>%d) - increase array size",
            slots_locked, max_locked
        );
    }
}

const char *BOINC_RCSID_32dcd335e7 = "$Id$";
