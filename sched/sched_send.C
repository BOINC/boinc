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

#include "server_types.h"
#include "sched_shmem.h"
#include "sched_config.h"
#include "sched_util.h"
#include "main.h"
#include "sched_array.h"
#include "sched_msgs.h"
#include "sched_send.h"
#include "sched_locality.h"
#include "sched_timezone.h"


#ifdef _USING_FCGI_
#include "fcgi_stdio.h"
#else
#define FCGI_ToFILE(x) (x)
#endif

const int MIN_SECONDS_TO_SEND = 0;
const int MAX_SECONDS_TO_SEND = (28*SECONDS_IN_DAY);

const double MIN_POSSIBLE_RAM = 64000000;

bool anonymous(PLATFORM& platform) {
    return (!strcmp(platform.name, "anonymous"));
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

// Find the app and app_version for the client's platform.
//
int get_app_version(
    WORKUNIT& wu, APP* &app, APP_VERSION* &avp,
    SCHEDULER_REQUEST& sreq, SCHEDULER_REPLY& reply, PLATFORM& platform,
    SCHED_SHMEM& ss
) {
    bool found;
    if (anonymous(platform)) {
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
        found = find_app_version(reply.wreq, wu, platform, ss, app, avp);
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

    // fill in default values for missing prefs
    //
    if (prefs.disk_max_used_gb == 0) prefs.disk_max_used_gb = 100.0; // 100 GB
    if (prefs.disk_max_used_pct == 0) prefs.disk_max_used_pct = 50;  // 50%
    // Always leave at least 1MB free!
    if (prefs.disk_min_free_gb < 0.001) prefs.disk_min_free_gb = 0.001; // 1MB

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


    //if (true) {
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
#ifdef EINSTEIN_AT_HOME
    log_messages.printf(
        SCHED_MSG_LOG::MSG_DEBUG,
        "est cpu dur %f; running_frac %f; rsf %f; est %f\n",
        ecd, running_frac, request.resource_share_fraction, ewd
    );
#endif

    return ewd;
}

// if the WU can't be executed on the host, return a bitmap of reasons why.
// Reasons include:
// 1) the host doesn't have enough memory;
// 2) the host doesn't have enough disk space;
// 3) based on CPU speed, resource share and estimated delay,
//    the host probably won't get the result done within the delay bound
//
// NOTE: This is a "fast" check; no DB access allowed.
// In particular it doesn't enforce the one-result-per-user-per-wu rule
//
int wu_is_infeasible(
    WORKUNIT& wu, SCHEDULER_REQUEST& request, SCHEDULER_REPLY& reply
) {
    int reason = 0;

    double m_nbytes = reply.host.m_nbytes;
    if (m_nbytes < MIN_POSSIBLE_RAM) m_nbytes = MIN_POSSIBLE_RAM;

    if (wu.rsc_memory_bound > m_nbytes) {
        log_messages.printf(
            SCHED_MSG_LOG::MSG_DEBUG,
            "[WU#%d %s] needs %f mem; [HOST#%d] has %f\n",
            wu.id, wu.name, wu.rsc_memory_bound, reply.host.id, m_nbytes
        );
        char explanation[256];
        if (!reply.wreq.insufficient_mem) {
            // only add message once
            //
            sprintf(explanation,
                "Your computer has only %.0f bytes of memory; workunit requires %.0f more bytes",
                m_nbytes, wu.rsc_memory_bound-m_nbytes
            );
            USER_MESSAGE um(explanation, "high");
            reply.insert_message(um);
        }
        reply.wreq.insufficient_mem = true;
        reason |= INFEASIBLE_MEM;
        reply.set_delay(24*3600);
    }

    if (wu.rsc_disk_bound > reply.wreq.disk_available) {
        reply.wreq.insufficient_disk = true;
        reason |= INFEASIBLE_DISK;
    }

    // skip delay check if host currently doesn't have any work
    // (i.e. everyone gets one result, no matter how slow they are)
    //
    if (!config.ignore_delay_bound && request.estimated_delay>0) {
        double ewd = estimate_wallclock_duration(wu, request, reply);
        if (request.estimated_delay + ewd > wu.delay_bound) {
            log_messages.printf(
                SCHED_MSG_LOG::MSG_DEBUG,
                "[WU#%d %s] needs %d seconds on [HOST#%d]; delay_bound is %d (request.estimated_delay is %f)\n",
                wu.id, wu.name, (int)ewd, reply.host.id, wu.delay_bound, request.estimated_delay
            );
            reply.set_delay(0.2*request.estimated_delay);
            reply.wreq.insufficient_speed = true;
            reason |= INFEASIBLE_CPU;
        }
    }

    return reason;
}

// insert "text" right after "after" in the given buffer
//
int insert_after(char* buffer, const char* after, const char* text) {
    char* p;
    char temp[LARGE_BLOB_SIZE];

    if (strlen(buffer) + strlen(text) > LARGE_BLOB_SIZE-1) {
        log_messages.printf(SCHED_MSG_LOG::MSG_NORMAL,
            "insert_after: overflow\n"
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
    WORK_REQ& wreq, WORKUNIT& wu, PLATFORM& platform, SCHED_SHMEM& ss,
    APP*& app, APP_VERSION*& avp
) {
    app = ss.lookup_app(wu.appid);
    if (!app) {
        log_messages.printf(
            SCHED_MSG_LOG::MSG_CRITICAL, "Can't find APP#%d\n", wu.appid
        );
        return false;
    }
    avp = ss.lookup_app_version(app->id, platform.id, app->min_version);
    if (!avp) {
        log_messages.printf(
            SCHED_MSG_LOG::MSG_DEBUG,
            "no app version available: APP#%d PLATFORM#%d min_version %d\n",
            app->id, platform.id, app->min_version
        );
        wreq.no_app_version = true;
        return false;
    }
    return true;
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
    WORKUNIT& wu, SCHEDULER_REPLY& reply, PLATFORM& platform,
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
            "[HOST#%d] Sending app_version %s %s %d\n",
            reply.host.id, app->name, platform.name, avp2->version_num
        );
    }

    // add time estimate to reply
    //
    wu2 = wu;       // make copy since we're going to modify its XML field
    retval = insert_wu_tags(wu2, *app);
    if (retval) {
        log_messages.printf(SCHED_MSG_LOG::MSG_NORMAL, "insert_wu_tags failed\n");
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
        if (wreq.insufficient_disk || wreq.insufficient_speed || wreq.insufficient_mem) {
            return false;
        }
    }
    if (wreq.seconds_to_fill <= 0) return false;
    if (wreq.disk_available <= 0) {
        wreq.insufficient_disk = true;
        return false;
    }
    if (wreq.nresults >= config.max_wus_to_send) return false;

    // config.daily_result_quota is PER CPU (up to max of four CPUs)
    // host.max_results_day is between 1 and config.daily_result_quota inclusive
    // wreq.daily_result_quota is between ncpus and ncpus*host.max_results_day inclusive
    if (config.daily_result_quota) {
        if (host.max_results_day <= 0 || host.max_results_day>config.daily_result_quota) {
            host.max_results_day = config.daily_result_quota;
        }
        // scale daily quota by #CPUs, up to a limit of 4
        //
        int ncpus = host.p_ncpus;
        if (ncpus > 4) ncpus = 4;
        if (ncpus < 1) ncpus = 1;
        wreq.daily_result_quota = ncpus*host.max_results_day;
        if (host.nresults_today >= wreq.daily_result_quota) {
            wreq.daily_result_quota_exceeded = true;
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
    SCHEDULER_REPLY& reply, PLATFORM& platform,
    APP* app, APP_VERSION* avp
) {
    int retval;
    double wu_seconds_filled;
    bool resent_result = false;

    retval = add_wu_to_reply(wu, reply, platform, app, avp);
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

    if (result.server_state != RESULT_SERVER_STATE_IN_PROGRESS) {
        // We are sending this result for the first time
        //
        result.report_deadline = result.sent_time + wu.delay_bound;
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
        if (result.report_deadline > result.sent_time + wu.delay_bound) {
            result.report_deadline = result.sent_time + wu.delay_bound;
        }

        log_messages.printf(
            SCHED_MSG_LOG::MSG_DEBUG,
            "[RESULT#%d] [HOST#%d] (resend lost work)\n",
            result.id, reply.host.id
        );
    }
    retval = result.update_subset();
    if (retval) {
        log_messages.printf(
            SCHED_MSG_LOG::MSG_CRITICAL,
            "add_result_to_reply: can't update result: %d\n", retval
        );
        return retval;
    }

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
            "send_work: can't insert name tags: %d\n",
            retval
        );
        return retval;
    }
    retval = insert_deadline_tag(result);
    if (retval) {
        log_messages.printf(
            SCHED_MSG_LOG::MSG_CRITICAL,
            "send_work: can't insert deadline tag: %d\n", retval
        );
        return retval;
    }
    reply.insert_result(result);
    reply.wreq.seconds_to_fill -= wu_seconds_filled;
    request.estimated_delay += wu_seconds_filled/reply.host.p_ncpus;
    reply.wreq.nresults++;
    if (!resent_result) reply.host.nresults_today++;
    return 0;
}

int send_work(
    SCHEDULER_REQUEST& sreq, SCHEDULER_REPLY& reply, PLATFORM& platform,
    SCHED_SHMEM& ss
) {
#if 1
    reply.wreq.disk_available = max_allowable_disk(sreq, reply);
#else
    reply.wreq.disk_available = sreq.project_disk_free;
#endif
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

    log_messages.printf(
        SCHED_MSG_LOG::MSG_NORMAL,
        "[HOST#%d] got request for %f seconds of work; available disk %f GB\n",
        reply.host.id, sreq.work_req_seconds, reply.wreq.disk_available/1e9
    );

    if (sreq.work_req_seconds <= 0) return 0;

    reply.wreq.seconds_to_fill = sreq.work_req_seconds;
    if (reply.wreq.seconds_to_fill > MAX_SECONDS_TO_SEND) {
        reply.wreq.seconds_to_fill = MAX_SECONDS_TO_SEND;
    }
    if (reply.wreq.seconds_to_fill < MIN_SECONDS_TO_SEND) {
        reply.wreq.seconds_to_fill = MIN_SECONDS_TO_SEND;
    }

    // TODO: add code to send results that were sent earlier but not reported.
    // Cautions (from John McLeod):
    // - make sure the result is still needed
    // - don't send if the project has been reset since first send,
    //   since result may have been cause of the reset
    //   (need to pass reset time?)
    // - make sure can complete by deadline
    // - don't send if project is suspended or "no more work" on client
    //   (need to pass these)

    if (config.locality_scheduling) {
        reply.wreq.infeasible_only = false;
        send_work_locality(sreq, reply, platform, ss);
    } else {
        // give priority to results that were infeasible for some other host
        //
        reply.wreq.infeasible_only = true;
        scan_work_array(sreq, reply, platform, ss);

        reply.wreq.infeasible_only = false;
        scan_work_array(sreq, reply, platform, ss);
    }

    log_messages.printf(
        SCHED_MSG_LOG::MSG_NORMAL,
        "[HOST#%d] Sent %d results [scheduler ran %f seconds]\n",
        reply.host.id, reply.wreq.nresults, elapsed_wallclock_time() 
    );

    if (reply.wreq.nresults == 0) {
        reply.set_delay(3600);
        USER_MESSAGE um2("No work sent", "high");
        reply.insert_message(um2);
        if (reply.wreq.no_app_version) {
            USER_MESSAGE um("(there was work for other platforms)", "high");
            reply.insert_message(um);
            reply.set_delay(3600*24);
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
            reply.set_delay(24*3600);
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
            reply.set_delay(3600*24);
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
    }
    return 0;
}

const char *BOINC_RCSID_32dcd335e7 = "$Id$";
