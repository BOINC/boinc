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

// scheduler code related to sending work


#include "config.h"
#include <vector>
#include <string>
#include <ctime>
#include <cstdio>
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

#include "sched_send.h"

#ifdef _USING_FCGI_
#include "fcgi_stdio.h"
#else
#define FCGI_ToFILE(x) (x)
#endif

const int MIN_SECONDS_TO_SEND = 0;
const int MAX_SECONDS_TO_SEND = (28*SECONDS_IN_DAY);
const int MAX_CPUS = 8;
    // max multiplier for daily_result_quota;
    // need to change as multicore processors expand

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

// Find an app and app_version for the client's platform(s).
//
int get_app_version(
    WORKUNIT& wu, APP* &app, APP_VERSION* &avp,
    SCHEDULER_REQUEST& sreq, SCHEDULER_REPLY& reply, PLATFORM_LIST& platforms,
    SCHED_SHMEM& ss
) {
    bool found;
    if (anonymous(platforms.list[0])) {
        app = ss.lookup_app(wu.appid);
        found = sreq.has_version(*app);
        if (!found) {
            log_messages.printf(SCHED_MSG_LOG::MSG_DEBUG,
                "Didn't find anonymous app\n"
            );
            return ERR_NO_APP_VERSION;
        }
        avp = NULL;
    } else {
        found = find_app_version(reply.wreq, wu, platforms, ss, app, avp);
        if (!found) {
            log_messages.printf(SCHED_MSG_LOG::MSG_DEBUG, "Didn't find app version\n");
            return ERR_NO_APP_VERSION;
        }

        // see if the core client is too old.
        //
        if (!app_core_compatible(reply.wreq, *avp)) {
            log_messages.printf(SCHED_MSG_LOG::MSG_DEBUG, "Didn't find app version: core client too old\n");
            return ERR_NO_APP_VERSION;
        }
    }
    return 0;
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
        log_messages.printf(
            SCHED_MSG_LOG::MSG_NORMAL,
            "disk_max_used_gb %f disk_max_used_pct %f disk_min_free_gb %f\n",
            prefs.disk_max_used_gb, prefs.disk_max_used_pct,
            prefs.disk_min_free_gb
        );
        log_messages.printf(
            SCHED_MSG_LOG::MSG_NORMAL,
            "host.d_total %f host.d_free %f host.d_boinc_used_total %f\n",
            host.d_total, host.d_free, host.d_boinc_used_total
        );
        log_messages.printf(
            SCHED_MSG_LOG::MSG_NORMAL,
            "x1 %f x2 %f x3 %f x %f\n",
            x1, x2, x3, x
        );
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
    log_messages.printf(
        SCHED_MSG_LOG::MSG_DEBUG,
        "est cpu dur %f; running_frac %f; rsf %f; est %f\n",
        ecd, running_frac, request.resource_share_fraction, ewd
    );
    return ewd;
}

// Find or compute various info about the host.
// These parameters affect how work is sent to the host
//
static int get_host_info(SCHEDULER_REPLY& reply) {
    char buf[8096];
   	std::string str;
   	extract_venue(reply.user.project_prefs, reply.host.venue, buf);
   	str = buf;
	unsigned int pos = 0;
	int temp_int;

    // scan user's project prefs for elements of the form <app_id>N</app_id>,
    // indicating the apps they want to run.
    //
    reply.wreq.host_info.preferred_apps.clear();
	while (parse_int(str.substr(pos,str.length()-pos).c_str(), "<app_id>", temp_int)) {
        APP_INFO ai;
        ai.appid = temp_int;
        reply.wreq.host_info.preferred_apps.push_back(ai);

		pos = str.find("<app_id>", pos) + 1;
	}
    temp_int = parse_int(buf,"<allow_beta_work>", temp_int);
    reply.wreq.host_info.allow_beta_work = temp_int;
 
    // Decide whether or not this computer is a 'reliable' computer
    //
    double expavg_credit = reply.host.expavg_credit;
    double expavg_time = reply.host.expavg_time;
    double avg_turnaround = reply.host.avg_turnaround;
    update_average(0, 0, CREDIT_HALF_LIFE, expavg_credit, expavg_time);
    double credit_scale, turnaround_scale;
    if (strstr(reply.host.os_name,"Windows") || strstr(reply.host.os_name,"Linux")
    ) {
        credit_scale = 1;
        turnaround_scale = 1;
    } else {
        credit_scale = .75;
        turnaround_scale = 1.25;
    }

    if (((expavg_credit/reply.host.p_ncpus) > config.reliable_min_avg_credit*credit_scale || config.reliable_min_avg_credit == 0)
            && (avg_turnaround < config.reliable_max_avg_turnaround*turnaround_scale || config.reliable_max_avg_turnaround == 0)
    ){
        reply.wreq.host_info.reliable = true;
        log_messages.printf(SCHED_MSG_LOG::MSG_DEBUG,
            "[HOST#%d] is reliable (OS = %s) expavg_credit = %.0f avg_turnaround(hours) = %.0f \n",
            reply.host.id, reply.host.os_name, expavg_credit,
            avg_turnaround/3600
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
            break;
        }
    }
    if (!app_allowed && !reply.wreq.beta_only) {
        reply.wreq.no_allowed_apps_available = true;
        log_messages.printf(SCHED_MSG_LOG::MSG_DEBUG,
            "[USER#%d] [WU#%d] user doesn't want work for this application\n",
            reply.user.id, wu.id
        );
        return INFEASIBLE_APP_SETTING;
    }
    return 0;
}

static inline int check_memory(
    WORKUNIT& wu, SCHEDULER_REQUEST& request, SCHEDULER_REPLY& reply
) {
    // see how much RAM we can use on this machine
    //
    double ram = reply.host.m_nbytes;
    if (ram <= 0) ram = DEFAULT_RAM_SIZE;
    double usable_ram = ram;
    double busy_frac = request.global_prefs.ram_max_used_busy_frac;
    double idle_frac = request.global_prefs.ram_max_used_idle_frac;
    double frac = 1;
    if (busy_frac>0 && idle_frac>0) {
        frac = std::max(busy_frac, idle_frac);
        if (frac > 1) frac = 1;
        usable_ram *= frac;
    }

    if (wu.rsc_memory_bound > usable_ram) {
        log_messages.printf(
            SCHED_MSG_LOG::MSG_DEBUG,
            "[WU#%d %s] needs %0.2fMB RAM; [HOST#%d] has %0.2fMB, %0.2fMB usable\n",
            wu.id, wu.name, wu.rsc_memory_bound/MEGA,
            reply.host.id, ram/MEGA, usable_ram/MEGA
        );
        // only add message once
        //
        if (!reply.wreq.insufficient_mem) {
            char explanation[256];
            if (wu.rsc_memory_bound > ram) {
                sprintf(explanation,
                    "Your computer has %0.2fMB of memory, and a job requires %0.2fMB",
                    ram/MEGA, wu.rsc_memory_bound/MEGA
                );
            } else {
                sprintf(explanation,
                    "Your preferences limit memory usage to %0.2fMB, and a job requires %0.2fMB",
                    usable_ram/MEGA, wu.rsc_memory_bound/MEGA
                );
            }
            USER_MESSAGE um(explanation, "high");
            reply.insert_message(um);
        }
        reply.wreq.insufficient_mem = true;
        reply.set_delay(DELAY_NO_WORK_TEMP);
        return INFEASIBLE_MEM;
    }
    return 0;
}

static inline int check_disk(
    WORKUNIT& wu, SCHEDULER_REQUEST& , SCHEDULER_REPLY& reply
) {
    if (wu.rsc_disk_bound > reply.wreq.disk_available) {
        reply.wreq.insufficient_disk = true;
        return INFEASIBLE_DISK;
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
        if (est_report_delay> wu.delay_bound) {
            log_messages.printf(
                SCHED_MSG_LOG::MSG_DEBUG,
                "[WU#%d %s] needs %d seconds on [HOST#%d]; delay_bound is %d (request.estimated_delay is %f)\n",
                wu.id, wu.name, (int)ewd, reply.host.id, wu.delay_bound, request.estimated_delay
            );
            reply.wreq.insufficient_speed = true;
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
    WORKUNIT& wu, SCHEDULER_REQUEST& request, SCHEDULER_REPLY& reply,
    APP* app
) {
    int retval;

    // homogeneous redundancy, quick check
    //
    if (config.homogeneous_redundancy || app->homogeneous_redundancy) {
        if (already_sent_to_different_platform_quick(request, wu, *app)) {
            log_messages.printf(
                SCHED_MSG_LOG::MSG_DEBUG,
                "[HOST#%d] [WU#%d %s] failed quick HR check: WU is class %d, host is class %d\n",
                reply.host.id, wu.id, wu.name, wu.hr_class, hr_class(request.host, app_hr_type(*app))
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

    // do this last because EDF sim uses some CPU
    //
    if (config.workload_sim && request.have_other_results_list) {
        double est_cpu = estimate_cpu_duration(wu, reply);
        IP_RESULT candidate("", wu.delay_bound, est_cpu);
        strcpy(candidate.name, wu.name);
        if (check_candidate(candidate, reply.host.p_ncpus, request.ip_results)) {
            // it passed the feasibility test,
            // but don't add it the the workload yet;
            // wait until we commit to sending it
        } else {
            reply.wreq.insufficient_speed = true;
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
        log_messages.printf(SCHED_MSG_LOG::MSG_CRITICAL,
            "insert_after: overflow: %d %d\n",
            strlen(buffer), strlen(text)
        );
        return ERR_BUFFER_OVERFLOW;
    }
    p = strstr(buffer, after);
    if (!p) {
        log_messages.printf(SCHED_MSG_LOG::MSG_CRITICAL,
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

// return the APP and APP_VERSION for the given WU, for the given platform.
// return false if none
//
bool find_app_version(
    WORK_REQ& wreq, WORKUNIT& wu, PLATFORM_LIST& platforms, SCHED_SHMEM& ss,
    APP*& app, APP_VERSION*& avp
) {
    app = ss.lookup_app(wu.appid);
    if (!app) {
        log_messages.printf(
            SCHED_MSG_LOG::MSG_CRITICAL, "Can't find APP#%d\n", wu.appid
        );
        return false;
    }
    unsigned int i;
    for (i=0; i<platforms.list.size(); i++) {
        PLATFORM* p = platforms.list[i];
        avp = ss.lookup_app_version(app->id, p->id, app->min_version);
        if (avp) return true;
    }
    log_messages.printf(
        SCHED_MSG_LOG::MSG_DEBUG,
        "no app version available: APP#%d PLATFORM#%d min_version %d\n",
        app->id, platforms.list[0]->id, app->min_version
    );
    wreq.no_app_version = true;
    return false;
}

// verify that the given APP_VERSION will work with the core client
//
bool app_core_compatible(WORK_REQ& wreq, APP_VERSION& av) {
    if (wreq.core_client_version < av.min_core_version) {
#if 0
        log_messages.printf(
            SCHED_MSG_LOG::MSG_DEBUG,
            "Outdated core version: wanted %d, got %d\n",
            av.min_core_version, wreq.core_client_version
        );
#endif
        wreq.outdated_core = true;
        return false;
    }
    return true;
}

// add the given workunit to a reply.
// look up its app, and make sure there's a version for this platform.
// Add the app and app_version to the reply also.
//
int add_wu_to_reply(
    WORKUNIT& wu, SCHEDULER_REPLY& reply, PLATFORM_LIST& ,
    APP* app, APP_VERSION* avp
) {
    int retval;
    WORKUNIT wu2, wu3;
    
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
        log_messages.printf(
            SCHED_MSG_LOG::MSG_DEBUG,
            "[HOST#%d] Sending app_version %s %d %d\n",
            reply.host.id, app->name, avp2->platformid, avp2->version_num
        );
    }

    // add time estimate to reply
    //
    wu2 = wu;       // make copy since we're going to modify its XML field
    retval = insert_wu_tags(wu2, *app);
    if (retval) {
        log_messages.printf(SCHED_MSG_LOG::MSG_NORMAL,
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
        if (wreq.insufficient_disk || wreq.insufficient_speed || wreq.insufficient_mem || wreq.no_allowed_apps_available) {
            return false;
        }
    }
    if (wreq.seconds_to_fill <= 0) return false;
    if (wreq.disk_available <= 0) {
        wreq.insufficient_disk = true;
        return false;
    }
    if (wreq.nresults >= config.max_wus_to_send) return false;

    // config.daily_result_quota is PER CPU (up to max of MAX_CPUS CPUs)
    // host.max_results_day is between 1 and config.daily_result_quota inclusive
    // wreq.daily_result_quota is between ncpus and ncpus*host.max_results_day inclusive
    if (config.daily_result_quota) {
        if (host.max_results_day == 0 || host.max_results_day>config.daily_result_quota) {
            host.max_results_day = config.daily_result_quota;
        }
        // scale daily quota by #CPUs, up to a limit of MAX_CPUS 4
        //
        int ncpus = host.p_ncpus;
        if (ncpus > MAX_CPUS) ncpus = MAX_CPUS;
        if (ncpus < 1) ncpus = 1;
        wreq.daily_result_quota = ncpus*host.max_results_day;
        if (host.nresults_today >= wreq.daily_result_quota) {
            wreq.daily_result_quota_exceeded = true;
            return false;
        }
    }

    if (config.max_wus_in_progress) {
        if (wreq.nresults_on_host >= config.max_wus_in_progress) {
            log_messages.printf(
                SCHED_MSG_LOG::MSG_DEBUG,
                "cache limit exceeded; %d > %d\n",
                wreq.nresults_on_host, config.max_wus_in_progress
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
    SCHEDULER_REPLY& reply, PLATFORM_LIST& platforms,
    APP* app, APP_VERSION* avp
) {
    int retval;
    double wu_seconds_filled;
    bool resent_result = false;

    retval = add_wu_to_reply(wu, reply, platforms, app, avp);
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
     	// If the workunit needs reliable and is being sent to a reliable host,
    	// then shorten the delay bound by the percent specified
    	//
    	if (config.reliable_time && reply.wreq.host_info.reliable && config.reliable_reduced_delay_bound > 0.01) {
        	if ((wu.create_time + config.reliable_time) <= time(0)) {
            	delay_bound = (int) (delay_bound * config.reliable_reduced_delay_bound);
        	}
    	}
    	
        // We are sending this result for the first time
        //
        result.report_deadline = result.sent_time + delay_bound;
        result.server_state = RESULT_SERVER_STATE_IN_PROGRESS;
    } else {
        // Result was ALREADY sent to this host but never arrived.
        // So we are resending it.
        // result.report_deadline and time_sent
        // have already been updated before this function was called.
        //
        resent_result = true;
 
        if (result.report_deadline < result.sent_time) {
            result.report_deadline = result.sent_time + 10;
        }
        if (result.report_deadline > result.sent_time + delay_bound) {
            result.report_deadline = result.sent_time + delay_bound;
        }

        log_messages.printf(
            SCHED_MSG_LOG::MSG_DEBUG,
            "[RESULT#%d] [HOST#%d] (resend lost work)\n",
            result.id, reply.host.id
        );
    }
    retval = result.mark_as_sent(old_server_state);
    if (retval==ERR_DB_NOT_FOUND) {
        log_messages.printf(
            SCHED_MSG_LOG::MSG_CRITICAL,
            "[RESULT#%d] [HOST#%d]: CAN'T SEND, already sent to another host\n",
            result.id, reply.host.id
        );
    } else if (retval) {
        log_messages.printf(
            SCHED_MSG_LOG::MSG_CRITICAL,
            "add_result_to_reply: can't update result: %d\n", retval
        );
    }
    if (retval) return retval;

    wu_seconds_filled = estimate_wallclock_duration(wu, request, reply);
    log_messages.printf(
        SCHED_MSG_LOG::MSG_NORMAL,
        "[HOST#%d] Sending [RESULT#%d %s] (fills %.2f seconds)\n",
        reply.host.id, result.id, result.name, wu_seconds_filled
    );

    retval = update_wu_transition_time(wu, result.report_deadline);
    if (retval) {
        log_messages.printf(
            SCHED_MSG_LOG::MSG_CRITICAL,
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
        log_messages.printf(
            SCHED_MSG_LOG::MSG_CRITICAL,
            "add_result_to_reply: can't insert name tags: %d\n",
            retval
        );
        return retval;
    }
    retval = insert_deadline_tag(result);
    if (retval) {
        log_messages.printf(
            SCHED_MSG_LOG::MSG_CRITICAL,
            "add_result_to_reply: can't insert deadline tag: %d\n", retval
        );
        return retval;
    }
    if (avp) {
        PLATFORM* pp = ssp->lookup_platform_id(avp->platformid);
        strcpy(result.platform_name, pp->name);
        result.version_num = avp->version_num;
    }
    reply.insert_result(result);
    reply.wreq.seconds_to_fill -= wu_seconds_filled;
    request.estimated_delay += wu_seconds_filled/reply.host.p_ncpus;
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

void send_work(
    SCHEDULER_REQUEST& sreq, SCHEDULER_REPLY& reply, PLATFORM_LIST& platforms,
    SCHED_SHMEM& ss
) {
    reply.wreq.disk_available = max_allowable_disk(sreq, reply);
    reply.wreq.insufficient_disk = false;
    reply.wreq.insufficient_mem = false;
    reply.wreq.insufficient_speed = false;
    reply.wreq.excessive_work_buf = false;
    reply.wreq.no_app_version = false;
    reply.wreq.hr_reject_temp = false;
    reply.wreq.hr_reject_perm = false;
    reply.wreq.daily_result_quota_exceeded = false;
    reply.wreq.core_client_version = sreq.core_client_major_version*100
        + sreq.core_client_minor_version;
    reply.wreq.nresults = 0;

    if (hr_unknown_platform(sreq.host)) {
        reply.wreq.hr_reject_perm = true;
        return;
    }

    get_host_info(reply); // parse project prefs for app details
    reply.wreq.beta_only = false;

    log_messages.printf(
        SCHED_MSG_LOG::MSG_NORMAL,
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

    if (config.workload_sim && sreq.have_other_results_list) {
        init_ip_results(
            sreq.global_prefs.work_buf_min(), reply.host.p_ncpus, sreq.ip_results
        );
    }

    if (config.locality_scheduling) {
        reply.wreq.infeasible_only = false;
        send_work_locality(sreq, reply, platforms, ss);
    } else {
    	// give top priority to results that require a 'reliable host'
        //
        if (reply.wreq.host_info.reliable) {
        	reply.wreq.reliable_only = true;
        	reply.wreq.infeasible_only = false;
        	scan_work_array(sreq, reply, platforms, ss);
        }
    	reply.wreq.reliable_only = false;

        // give 2nd priority to results for a beta app
        // (projects should load beta work with care,
        // otherwise your users won't get production work done!
        //
        if (reply.wreq.host_info.allow_beta_work) {
            reply.wreq.beta_only = true;
            log_messages.printf(
                SCHED_MSG_LOG::MSG_DEBUG,
                "[HOST#%d] will accept beta work.  Scanning for beta work.\n",
                reply.host.id
            );
            scan_work_array(sreq, reply, platforms, ss);
        }
        reply.wreq.beta_only = false;
    	
        // give next priority to results that were infeasible for some other host
        //
        reply.wreq.infeasible_only = true;
        scan_work_array(sreq, reply, platforms, ss);

        reply.wreq.infeasible_only = false;
        scan_work_array(sreq, reply, platforms, ss);
    }

    log_messages.printf(
        SCHED_MSG_LOG::MSG_NORMAL,
        "[HOST#%d] Sent %d results [scheduler ran %f seconds]\n",
        reply.host.id, reply.wreq.nresults, elapsed_wallclock_time() 
    );

    if (reply.wreq.nresults == 0) {
        reply.set_delay(DELAY_NO_WORK_TEMP);
        USER_MESSAGE um2("No work sent", "high");
        reply.insert_message(um2);
        if (reply.wreq.no_app_version) {
            USER_MESSAGE um("(there was work for other platforms)", "high");
            reply.insert_message(um);
            reply.set_delay(DELAY_NO_WORK_PERM);
        }
        if (reply.wreq.no_allowed_apps_available) {
            USER_MESSAGE um(
                "(There was work but not for the applications you have allowed.  Please check your settings on the website.)",
                "high"
            );
            reply.insert_message(um);
        }
        if (reply.wreq.insufficient_disk) {
            USER_MESSAGE um(
                "(there was work but you don't have enough disk space allocated)",
                "high"
            );
            reply.insert_message(um);
        }
        if (reply.wreq.insufficient_mem) {
            USER_MESSAGE um(
                "(there was work but your computer doesn't have enough memory)",
                "high"
            );
            reply.insert_message(um);
        }
        if (reply.wreq.insufficient_speed) {
            char helpful[512];
            if (reply.wreq.core_client_version>419) {
                sprintf(helpful,
                    "(won't finish in time) "
                    "Computer on %.1f%% of time, BOINC on %.1f%% of that, this project gets %.1f%% of that",
                    100.0*reply.host.on_frac, 100.0*reply.host.active_frac, 100.0*sreq.resource_share_fraction
                );
            }
            else {
                sprintf(helpful,
                    "(won't finish in time) "
                    "Computer available %.1f%% of time, this project gets %.1f%% of that",
                    100.0*reply.host.on_frac, 100.0*sreq.resource_share_fraction
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
                " (your core client is out of date - please upgrade)",
                "high"
            );
            reply.insert_message(um);
            reply.set_delay(DELAY_NO_WORK_PERM);
            log_messages.printf(
                SCHED_MSG_LOG::MSG_NORMAL,
                "Not sending work because core client is outdated\n"
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
            char helpful[256];
            struct tm *rpc_time_tm;
            int delay_time;

            sprintf(helpful, "(reached daily quota of %d results)", reply.wreq.daily_result_quota);
            USER_MESSAGE um(helpful, "high");
            reply.insert_message(um);
            log_messages.printf(
                SCHED_MSG_LOG::MSG_NORMAL,
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
            char helpful[256];
            sprintf(helpful, "(reached per-host limit of %d tasks)",
                config.max_wus_in_progress
            );
            USER_MESSAGE um(helpful, "high");
            reply.insert_message(um);
            reply.set_delay(DELAY_NO_WORK_CACHE);
            log_messages.printf(
                SCHED_MSG_LOG::MSG_NORMAL,
                "host %d already has %d result(s) on cache\n",
                reply.host.id, reply.wreq.nresults_on_host
            );
        }        
    }
}

const char *BOINC_RCSID_32dcd335e7 = "$Id$";
