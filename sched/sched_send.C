// The contents of this file are subject to the BOINC Public License
// Version 1.0 (the "License"); you may not use this file except in
// compliance with the License. You may obtain a copy of the License at
// http://boinc.berkeley.edu/license_1.0.txt
//
// Software distributed under the License is distributed on an "AS IS"
// basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the
// License for the specific language governing rights and limitations
// under the License.
//
// The Original Code is the Berkeley Open Infrastructure for Network Computing.
//
// The Initial Developer of the Original Code is the SETI@home project.
// Portions created by the SETI@home project are Copyright (C) 2002
// University of California at Berkeley. All Rights Reserved.
//
// Contributor(s):
//

// scheduler code related to sending work

#include <vector>
#include <string>
#include <ctime>
#include <cstdio>
using namespace std;

#include "error_numbers.h"
#include "server_types.h"
#include "sched_shmem.h"
#include "sched_config.h"
#include "sched_util.h"
#include "main.h"
#include "sched_msgs.h"
#include "sched_send.h"

#ifdef _USING_FCGI_
#include "fcgi_stdio.h"
#endif

const int MIN_SECONDS_TO_SEND = 0;
const int MAX_SECONDS_TO_SEND = (28*SECONDS_IN_DAY);

const double MIN_POSSIBLE_RAM = 64000000;

struct WORK_REQ {
    bool infeasible_only;
    double seconds_to_fill;
    double disk_available;
    int nresults;
    int core_client_version;

    // the following flags are set whenever a result is infeasible;
    // used to construct explanatory message to user
    //
    bool insufficient_disk;
    bool insufficient_mem;
    bool insufficient_speed;
    bool no_app_version;
    bool homogeneous_redundancy_reject;
    bool outdated_core;
    bool daily_result_quota_exceeded;
};

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

// compute the max disk usage we can request of the host
//
double max_allowable_disk(SCHEDULER_REQUEST& req) {
    double x1, x2, x3;

    HOST host = req.host;

    x1 = req.project_disk_free;
    x2 = req.potentially_free_offender;
    x3 = req.potentially_free_self;

    if (x1 < 0) {
        log_messages.printf(
            SCHED_MSG_LOG::NORMAL,
            "req.project_disk_free_gb %f\n",
            x1
        );
    }
    return max(max(x1,x2), x3);
}

// if a host has active_frac < 0.1, assume 0.1 so we don't deprive it of work.
//
const double HOST_ACTIVE_FRAC_MIN = 0.1;

// estimate the number of seconds that a workunit requires running 100% on a
// single CPU of this host.
//
// TODO: improve this.  take memory bandwidth into account
//
inline double estimate_cpu_duration(WORKUNIT& wu, HOST& host) {
    if (host.p_fpops <= 0) host.p_fpops = 1e9;
    if (wu.rsc_fpops_est <= 0) wu.rsc_fpops_est = 1e12;
    return wu.rsc_fpops_est/host.p_fpops;
}

// estimate the amount of real time for this WU based on active_frac,
// and resource_share_fraction
inline double estimate_wallclock_duration(
    WORKUNIT& wu, HOST& host, double resource_share_fraction
) {
    return estimate_cpu_duration(wu, host)
        / max(HOST_ACTIVE_FRAC_MIN, host.active_frac * resource_share_fraction)
    ;
}

// return false if the WU can't be executed on the host
// because of insufficient memory, CPU speed, or resource share
//
bool wu_is_feasible(
    WORKUNIT& wu, HOST& host, WORK_REQ& wreq,
    double resource_share_fraction, double estimated_delay
) {
    double m_nbytes = host.m_nbytes;
    if (m_nbytes < MIN_POSSIBLE_RAM) m_nbytes = MIN_POSSIBLE_RAM;

    if (wu.rsc_memory_bound > m_nbytes) {
        log_messages.printf(
            SCHED_MSG_LOG::DEBUG, "[WU#%d %s] needs %f mem; [HOST#%d] has %f\n",
            wu.id, wu.name, wu.rsc_memory_bound, host.id, m_nbytes
        );
        wreq.insufficient_mem = true;
        return false;
    }

    if (config.enforce_delay_bound) {
        double wu_wallclock_time =
            estimate_wallclock_duration(wu, host, resource_share_fraction);
        double host_remaining_time = estimated_delay;

        if (host_remaining_time + wu_wallclock_time > wu.delay_bound) {
            log_messages.printf(
                SCHED_MSG_LOG::DEBUG, "[WU#%d %s] needs requires %d seconds on [HOST#%d]; delay_bound is %d\n",
                wu.id, wu.name, (int)wu_wallclock_time, host.id, wu.delay_bound
            );
            wreq.insufficient_speed = true;
            return false;
        }
    }

    return true;
}

// insert "text" right after "after" in the given buffer
//
int insert_after(char* buffer, char* after, char* text) {
    char* p;
    char temp[LARGE_BLOB_SIZE];

    if (strlen(buffer) + strlen(text) > LARGE_BLOB_SIZE-1) {
        log_messages.printf(SCHED_MSG_LOG::NORMAL, "insert_after: overflow\n");
        return ERR_BUFFER_OVERFLOW;
    }
    p = strstr(buffer, after);
    if (!p) {
        log_messages.printf(SCHED_MSG_LOG::CRITICAL, "insert_after: %s not found in %s\n", after, buffer);
        return ERR_NULL;
    }
    p += strlen(after);
    strcpy(temp, p);
    strcpy(p, text);
    strcat(p, temp);
    return 0;
}

// add elements to WU's xml_doc, in preparation for sending
// it to a client
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
            SCHED_MSG_LOG::CRITICAL, "Can't find APP#%d\n", wu.appid
        );
        return false;
    }
    avp = ss.lookup_app_version(app->id, platform.id, app->min_version);
    if (!avp) {
        log_messages.printf(
            SCHED_MSG_LOG::DEBUG,
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
            SCHED_MSG_LOG::DEBUG,
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
    WORKUNIT& wu, SCHEDULER_REPLY& reply, PLATFORM& platform, SCHED_SHMEM& ss,
    WORK_REQ& wreq, APP* app, APP_VERSION* avp
) {
    int retval;
    WORKUNIT wu2;

    // add the app, app_version, and workunit to the reply,
    // but only if they aren't already there
    //
    if (avp) {
        reply.insert_app_unique(*app);
        reply.insert_app_version_unique(*avp);
        log_messages.printf(
            SCHED_MSG_LOG::DEBUG,
            "[HOST#%d] Sending app_version %s %s %d\n",
            reply.host.id, app->name, platform.name, avp->version_num
        );
    }

    // add time estimate to reply
    //
    wu2 = wu;       // make copy since we're going to modify its XML field
    retval = insert_wu_tags(wu2, *app);
    if (retval) {
        log_messages.printf(SCHED_MSG_LOG::NORMAL, "insert_wu_tags failed\n");
        return retval;
    }
    reply.insert_workunit_unique(wu2);
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

static int update_wu_transition_time(WORKUNIT wu, time_t x) {
    // TODO: this might be better: a mysql statement such as "update set
    // transition_time=X where id=ID and transition_time<X".  this avoids
    // concurrency problems altogether.
    DB_WORKUNIT dbwu;
    int retval;

    retval = dbwu.lookup_id(wu.id);
    if (retval) return retval;
    if (x < dbwu.transition_time) {
        dbwu.transition_time = x;
        retval = dbwu.update();
        if (retval) return retval;
    }
    return 0;
}

// return true iff a result for same WU is already being sent
//
static bool already_in_reply(WU_RESULT& wu_result, SCHEDULER_REPLY& reply) {
    unsigned int i;
    for (i=0; i<reply.results.size(); i++) {
        if (wu_result.workunit.id == reply.results[i].workunitid) {
            return true;
        }
    }
    return false;
}

// modified by Pietro Cicotti
// Check that the two platform has the same architecture and operating system
// Architectures: AMD, Intel, Macintosh
// OS: Linux, Windows, Darwin, SunOS

const int nocpu = 0;
const int Intel = 1;
const int AMD = 2;
const int Macintosh = 3;

const int noos = 10;
const int Linux = 11;
const int Windows = 12;
const int Darwin = 13;
const int SunOS = 14;

bool same_platform(DB_HOST& host, SCHEDULER_REQUEST& sreq) {
    int h_processor = nocpu;
    int h_system = noos;
    int r_processor = nocpu;
    int r_system = noos;

    if ( strstr(host.p_vendor, "Intel") != NULL ) h_processor = Intel;
    else if( strstr(host.p_vendor, "AMD") != NULL ) h_processor = AMD;
    else if( strstr(host.p_vendor, "Macintosh") != NULL ) h_processor = Macintosh;
    if ( strstr(sreq.host.p_vendor, "Intel") != NULL ) r_processor = Intel;
    else if( strstr(sreq.host.p_vendor, "AMD") != NULL ) r_processor = AMD;
    else if( strstr(sreq.host.p_vendor, "Macintosh") != NULL ) r_processor = Macintosh;

    if ( strstr(host.os_name, "Linux") != NULL ) h_system = Linux;
    else if( strstr(host.os_name, "Windows") != NULL ) h_system = Windows;
    else if( strstr(host.os_name, "Darwin") != NULL ) h_system = Darwin;
    else if( strstr(host.os_name, "SunOS") != NULL ) h_system = SunOS;
    if ( strstr(sreq.host.os_name, "Linux") != NULL ) r_system = Linux;
    else if( strstr(sreq.host.os_name, "Windows") != NULL ) r_system = Windows;
    else if( strstr(sreq.host.os_name, "Darwin") != NULL ) r_system = Darwin;
    else if( strstr(sreq.host.os_name, "SunOS") != NULL ) r_system = SunOS;

    return ( h_processor == r_processor && h_system == r_system );
}

#if 0
// old version, just in case
bool same_platform(DB_HOST& host, SCHEDULER_REQUEST& sreq) {
    return !strcmp(host.os_name, sreq.host.os_name)
        && !strcmp(host.p_vendor, sreq.host.p_vendor);
}
#endif

// return true if we've already sent a result of this WU to a different platform
// (where "platform" is os_name + p_vendor;
// may want to sharpen this for Unix)
//
static bool already_sent_to_different_platform(
    WORK_REQ& wreq, SCHEDULER_REQUEST& sreq, WORKUNIT& workunit
) {
    DB_RESULT result;
    DB_HOST host;
    char buf[256];
    bool found = false;
    int retval;

    sprintf(buf, "where workunitid=%d", workunit.id);
    while (!result.enumerate(buf)) {
        if (result.hostid) {
            sprintf(buf, "where id=%d", result.hostid);
            retval = host.lookup(buf);
            if (retval) {
                log_messages.printf(
                    SCHED_MSG_LOG::CRITICAL,
                    "send_work: host lookup failed (%d)\n", retval
                );
                found = true;
                break;
            }
            if (same_platform(host, sreq)) {
                // already sent to same platform - don't need to keep looking
                //
                break;
            }
            wreq.homogeneous_redundancy_reject = true;
            found = true;
            break;
        }
    }
    result.end_enumerate();
    return found;
}

void lock_sema() {
    lock_semaphore(sema_key);
}

void unlock_sema() {
    unlock_semaphore(sema_key);
}

// Make a pass through the wu/results array, sending work.
// If "infeasible_only" is true, send only results that were
// previously infeasible for some host
//
static void scan_work_array(
    WORK_REQ& wreq,
    SCHEDULER_REQUEST& sreq, SCHEDULER_REPLY& reply, PLATFORM& platform,
    SCHED_SHMEM& ss
) {
    int i, retval, n;
    WORKUNIT wu;
    DB_RESULT result;
    double wu_seconds_filled;
    char buf[256];
    APP* app;
    APP_VERSION* avp;
    bool found;

    if (wreq.disk_available < 0) wreq.insufficient_disk = true;

    lock_sema();
    for (i=0; i<ss.nwu_results; i++) {
        if (wreq.seconds_to_fill <= 0) break;
        if (wreq.disk_available <= 0) break;
        if (wreq.nresults >= config.max_wus_to_send) break;
        if (config.daily_result_quota) {
            if (reply.host.nresults_today >= config.daily_result_quota) {
                wreq.daily_result_quota_exceeded = true;
                break;
            }
        }

        WU_RESULT& wu_result = ss.wu_results[i];

        // do fast checks on this wu_result;
        // i.e. ones that don't require DB access
        // if any check fails, continue

        if (wu_result.state != WR_STATE_PRESENT && wu_result.state != g_pid) {
            continue;
        }

        if (wreq.infeasible_only && (wu_result.infeasible_count==0)) {
            continue;
        }

        if (wu_result.workunit.rsc_disk_bound > wreq.disk_available) {
            wreq.insufficient_disk = true;
            wu_result.infeasible_count++;
            continue;
        }

        // don't send if we're already sending a result for same WU
        //
        if (already_in_reply(wu_result, reply)) {
            continue;
        }

        // don't send if host can't handle it
        //
        wu = wu_result.workunit;
        if (!wu_is_feasible(
            wu, reply.host, wreq, sreq.resource_share_fraction,
            sreq.estimated_delay
        )) {
            log_messages.printf(
                SCHED_MSG_LOG::DEBUG, "[HOST#%d] [WU#%d %s] WU is infeasible\n",
                reply.host.id, wu.id, wu.name
            );
            wu_result.infeasible_count++;
            continue;
        }

        // Find the app and app_version for the client's platform.
        // If none, treat the WU as infeasible
        //
        if (anonymous(platform)) {
            app = ss.lookup_app(wu.appid);
            found = sreq.has_version(*app);
            if (!found) {
                continue;
            }
            avp = NULL;
        } else {
            found = find_app_version(wreq, wu, platform, ss, app, avp);
            if (!found) {
                wu_result.infeasible_count++;
                continue;
            }

            // see if the core client is too old.
            // don't bump the infeasible count because this
            // isn't the result's fault
            //
            if (!app_core_compatible(wreq, *avp)) {
                continue;
            }
        }

        // end of fast checks - mark wu_result as checked out and release sema.
        // from here on in this loop, don't continue on failure;
        // instead, goto dont_send (so that we reacquire semaphore)

        wu_result.state = WR_STATE_CHECKED_OUT;
        unlock_sema();

        // Don't send if we've already sent a result of this WU to this user.
        //
        if (config.one_result_per_user_per_wu) {
            sprintf(buf,
                "where workunitid=%d and userid=%d",
                wu_result.workunit.id, reply.user.id
            );
            retval = result.count(n, buf);
            if (retval) {
                log_messages.printf(
                    SCHED_MSG_LOG::CRITICAL,
                    "send_work: can't get result count (%d)\n", retval
                );
                goto dont_send;
            } else {
                if (n>0) {
#if 0
                    log_messages.printf(
                        SCHED_MSG_LOG::NORMAL,
                        "send_work: user %d already has %d result(s) for WU %d\n",
                        reply.user.id, n, wu_result.workunit.id
                    );
#endif
                    goto dont_send;
                }
            }
        }

        // if desired, make sure redundancy is homogeneous
        //
        if (config.homogeneous_redundancy) {
            if (already_sent_to_different_platform(
                wreq, sreq, wu_result.workunit
            )) {
                goto dont_send;
            }
        }

        result.id = wu_result.resultid;

        // mark slot as empty AFTER we've copied out of it
        // (since otherwise feeder might overwrite it)
        //
        wu_result.state = WR_STATE_EMPTY;

        // reread result from DB, make sure it's still unsent
        // TODO: from here to update() should be a transaction
        //
        retval = result.lookup_id(result.id);
        if (retval) {
            log_messages.printf(SCHED_MSG_LOG::CRITICAL,
                "[RESULT#%d] result.lookup_id() failed %d\n",
                result.id, retval
            );
            goto done;
        }
        if (result.server_state != RESULT_SERVER_STATE_UNSENT) {
            log_messages.printf(SCHED_MSG_LOG::DEBUG,
                "[RESULT#%d] expected to be unsent; instead, state is %d\n",
                result.id, result.server_state
            );
            goto done;
        }

        // ****** HERE WE'VE COMMITTED TO SENDING THIS RESULT TO HOST ******
        //

        retval = add_wu_to_reply(wu, reply, platform, ss, wreq, app, avp);
        if (retval) goto done;

        wreq.disk_available -= wu.rsc_disk_bound;

        // update the result in DB
        //
        result.server_state = RESULT_SERVER_STATE_IN_PROGRESS;
        result.hostid = reply.host.id;
        result.userid = reply.user.id;
        result.sent_time = time(0);
        result.report_deadline = result.sent_time + wu.delay_bound;
        result.update_subset();

        wu_seconds_filled = estimate_cpu_duration(wu, reply.host);
        log_messages.printf(
            SCHED_MSG_LOG::NORMAL,
            "[HOST#%d] Sending [RESULT#%d %s] (fills %d seconds)\n",
            reply.host.id, result.id, result.name, int(wu_seconds_filled)
        );

        retval = update_wu_transition_time(wu, result.report_deadline);
        if (retval) {
            log_messages.printf(
                SCHED_MSG_LOG::CRITICAL,
                "send_work: can't update WU transition time\n"
            );
        }

        // The following overwrites the result's xml_doc field.
        // But that's OK cuz we're done with DB updates
        //
        retval = insert_name_tags(result, wu);
        if (retval) {
            log_messages.printf(
                SCHED_MSG_LOG::CRITICAL, "send_work: can't insert name tags\n"
            );
        }
        retval = insert_deadline_tag(result);
        if (retval) {
            log_messages.printf(
                SCHED_MSG_LOG::CRITICAL,
                "send_work: can't insert deadline tag\n"
            );
        }
        reply.insert_result(result);

        wreq.seconds_to_fill -= wu_seconds_filled;
        wreq.nresults++;
        reply.host.nresults_today++;
        goto done;

dont_send:
        // here we couldn't send the result for some reason --
        // set its state back to PRESENT
        //
        wu_result.state = WR_STATE_PRESENT;
done:
        lock_sema();
    }
    unlock_sema();
}

int send_work(
    SCHEDULER_REQUEST& sreq, SCHEDULER_REPLY& reply, PLATFORM& platform,
    SCHED_SHMEM& ss
) {
    WORK_REQ wreq;

    memset(&wreq, 0, sizeof(wreq));
    wreq.disk_available = sreq.project_disk_free;
    wreq.insufficient_disk = false;
    wreq.insufficient_mem = false;
    wreq.insufficient_speed = false;
    wreq.no_app_version = false;
    wreq.homogeneous_redundancy_reject = false;
    wreq.daily_result_quota_exceeded = false;
    wreq.core_client_version = sreq.core_client_major_version*100
        + sreq.core_client_minor_version;
    wreq.nresults = 0;

    log_messages.printf(
        SCHED_MSG_LOG::NORMAL,
        "[HOST#%d] got request for %d seconds of work; available disk %f GB\n",
        reply.host.id, sreq.work_req_seconds, wreq.disk_available/1e9
    );

    if (sreq.work_req_seconds <= 0) return 0;

    wreq.seconds_to_fill = sreq.work_req_seconds;
    if (wreq.seconds_to_fill > MAX_SECONDS_TO_SEND) {
        wreq.seconds_to_fill = MAX_SECONDS_TO_SEND;
    }
    if (wreq.seconds_to_fill < MIN_SECONDS_TO_SEND) {
        wreq.seconds_to_fill = MIN_SECONDS_TO_SEND;
    }

    // give priority to results that were infeasible for some other host
    //
    wreq.infeasible_only = true;
    scan_work_array(wreq, sreq, reply, platform, ss);

    wreq.infeasible_only = false;
    scan_work_array(wreq, sreq, reply, platform, ss);

    if(wreq.nresults == 0) {
        wreq.disk_available = sreq.potentially_free_offender;
        scan_work_array(wreq, sreq, reply, platform, ss);
    }
    if(wreq.nresults == 0 && config.delete_from_self) {
        wreq.disk_available = sreq.potentially_free_self;
        scan_work_array(wreq, sreq, reply, platform, ss);
    }

    log_messages.printf(
        SCHED_MSG_LOG::NORMAL, "[HOST#%d] Sent %d results\n",
        reply.host.id, wreq.nresults
    );

    if (wreq.nresults == 0) {
        strcpy(reply.message, "No work available");
        if (wreq.no_app_version) {
            strcat(reply.message,
                " (there was work for other platforms)"
            );
        }
        if (wreq.insufficient_disk) {
            strcat(reply.message,
                " (there was work but you don't have enough disk space allocated)"
            );
        }
        if (wreq.insufficient_mem) {
            strcat(reply.message,
                " (there was work but your computer doesn't have enough memory)"
            );
        }
        if (wreq.insufficient_speed) {
            strcat(reply.message,
                " (there was work but your computer would not finish it before it is due"
            );
        }
        if (wreq.homogeneous_redundancy_reject) {
            strcat(reply.message,
                " (there was work but it was committed to other platforms"
            );
        }
        if (wreq.outdated_core) {
            strcat(reply.message,
                " (your core client is out of date - please upgrade)"
            );
            log_messages.printf(
                SCHED_MSG_LOG::NORMAL,
                "Not sending work because core client is outdated\n"
            );
        }
        if (wreq.daily_result_quota_exceeded) {
            strcat(reply.message, " (daily quota exceeded)");
            log_messages.printf(
                SCHED_MSG_LOG::NORMAL,
                "Daily result quota exceeded for host %d\n",
                reply.host.id
            );
        }

        strcpy(reply.message_priority, "high");
        reply.request_delay = 3600;

        log_messages.printf(
            SCHED_MSG_LOG::NORMAL, "[HOST#%d] %s\n",
            reply.host.id, reply.message
        );
    }
    return 0;
}

