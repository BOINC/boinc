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

// Handle a scheduling server RPC

#include <iostream>
#include <vector>
#include <string>
using namespace std;

#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <time.h>
#include <assert.h>
#include <math.h>

#include "boinc_db.h"
#include "backend_lib.h"
#include "error_numbers.h"
#include "parse.h"
#include "util.h"
#include "server_types.h"
#include "sched_util.h"
#include "main.h"
#include "handle_request.h"

const int MIN_SECONDS_TO_SEND = 0;
const int MAX_SECONDS_TO_SEND = (28*SECONDS_PER_DAY);

const double COBBLESTONE_FACTOR = 300.0;

const double MIN_POSSIBLE_RAM = 64000000;

struct WORK_REQ {
    bool infeasible_only;
    double seconds_to_fill;
    double disk_available;
    int nresults;

    // the following flags are set whenever a result is infeasible;
    // used to construct explanatory message to user
    //
    bool insufficient_disk;
    bool insufficient_mem;
    bool insufficient_speed;
    bool no_app_version;
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
double max_allowable_disk(USER& user, SCHEDULER_REQUEST& req) {
    GLOBAL_PREFS prefs;
    HOST host = req.host;
    double x1, x2, x3, x;

    prefs.parse(user.global_prefs);

    // fill in default values for missing prefs
    //
    if (prefs.disk_max_used_gb == 0) prefs.disk_max_used_gb = 0.1;   // 100 MB
    if (prefs.disk_max_used_pct == 0) prefs.disk_max_used_pct = 10;
    // min_free_gb can be zero

    // default values for BOINC disk usage (project and total) is zero
    //

    // no defaults for total/free disk space (host.d_total, d_free)
    // if they're zero, project will get no work.
    //

    x1 = prefs.disk_max_used_gb*1e9 - req.total_disk_usage;
    x2 = host.d_total*prefs.disk_max_used_pct/100.;
    x3 = host.d_free - prefs.disk_min_free_gb*1e9;      // may be negative

    x = min(x1, min(x2, x3));
    if (x < 0) {
        log_messages.printf(
            SchedMessages::NORMAL,
            "disk_max_used_gb %f disk_max_used_pct %f disk_min_free_gb %f\n",
            prefs.disk_max_used_gb, prefs.disk_max_used_pct,
            prefs.disk_min_free_gb
        );
        log_messages.printf(
            SchedMessages::NORMAL,
            "req.total_disk_usage %f host.d_total %f host.d_free %f\n",
            req.total_disk_usage, host.d_total, host.d_free
        );
        log_messages.printf(
            SchedMessages::NORMAL,
            "x1 %f x2 %f x3 %f x %f\n",
            x1, x2, x3, x
        );
    }
    return x;
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

// estimate the amount of real time for this WU based on active_frac
//
inline double estimate_wallclock_duration(WORKUNIT& wu, HOST& host) {
    return estimate_cpu_duration(wu, host)
        / max(HOST_ACTIVE_FRAC_MIN, host.active_frac)
    ;

}

// return true if the WU can be executed on the host
//
bool wu_is_feasible(WORKUNIT& wu, HOST& host, WORK_REQ& wreq) {
    double m_nbytes = host.m_nbytes;
    if (m_nbytes < MIN_POSSIBLE_RAM) m_nbytes = MIN_POSSIBLE_RAM;

    if (wu.rsc_memory_bound > m_nbytes) {
        log_messages.printf(
            SchedMessages::DEBUG, "[WU#%d %s] needs %f mem; [HOST#%d] has %f\n",
            wu.id, wu.name, wu.rsc_memory_bound, host.id, m_nbytes
        );
        wreq.insufficient_mem = true;
        return false;
    }

    double wu_wallclock_time = estimate_wallclock_duration(wu, host);
    int host_remaining_time = 0; // TODO

    if (host_remaining_time + wu_wallclock_time > wu.delay_bound) {
        log_messages.printf(
            SchedMessages::DEBUG, "[WU#%d %s] needs requires %d seconds on [HOST#%d]; delay_bound is %d\n",
            wu.id, wu.name, (int)wu_wallclock_time, host.id, wu.delay_bound
        );
        wreq.insufficient_speed = true;
        return false;
    }

    return true;
}

// insert "text" right after "after" in the given buffer
//
int insert_after(char* buffer, char* after, char* text) {
    char* p;
    char temp[MEDIUM_BLOB_SIZE];

    if (strlen(buffer) + strlen(text) > MEDIUM_BLOB_SIZE-1) {
        log_messages.printf(SchedMessages::NORMAL, "insert_after: overflow\n");
        return ERR_BUFFER_OVERFLOW;
    }
    p = strstr(buffer, after);
    if (!p) {
        log_messages.printf(SchedMessages::CRITICAL, "insert_after: %s not found in %s\n", after, buffer);
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
    char buf[MEDIUM_BLOB_SIZE];

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
            SchedMessages::CRITICAL, "Can't find APP#%d\n", wu.appid
        );
        return false;
    }
    avp = ss.lookup_app_version(app->id, platform.id, app->min_version);
    if (!avp) {
        log_messages.printf(
            SchedMessages::DEBUG,
            "no app version available: APP#%d PLATFORM#%d min_version %d\n",
            app->id, platform.id, app->min_version
        );
        wreq.no_app_version = true;
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
            SchedMessages::DEBUG,
            "[HOST#%d] Sending app_version %s %s %d\n",
            reply.host.id, app->name, platform.name, avp->version_num
        );
    }

    // add time estimate to reply
    //
    wu2 = wu;       // make copy since we're going to modify its XML field
    retval = insert_wu_tags(wu2, *app);
    if (retval) {
        log_messages.printf(SchedMessages::NORMAL, "insert_wu_tags failed\n");
        return retval;
    }
    reply.insert_workunit_unique(wu2);
    return 0;
}

// Look up the host and its user, and make sure the authenticator matches.
// If no host ID is supplied, or if RPC seqno mismatch,
// create a new host record and return its ID
//
int authenticate_user(SCHEDULER_REQUEST& sreq, SCHEDULER_REPLY& reply) {
    int retval;
    char buf[256];
    DB_HOST host;
    DB_USER user;
    DB_TEAM team;

    if (sreq.hostid) {
        retval = host.lookup_id(sreq.hostid);
        if (retval) {
            strcpy(reply.message, "Can't find host record");
            strcpy(reply.message_priority, "low");
            log_messages.printf(
                SchedMessages::NORMAL,
                "[HOST#%d?] can't find host\n",
                sreq.hostid
            );
            sreq.hostid = 0;
            goto lookup_user_and_make_new_host;
        }
        reply.host = host;

        retval = user.lookup_id(reply.host.userid);
        if (retval) {
            // this should never happen - means inconsistent DB
            //
            strcpy(reply.message, "Can't find user record");
            strcpy(reply.message_priority, "low");
            reply.request_delay = 120;
            reply.nucleus_only = true;
            log_messages.printf(
                SchedMessages::NORMAL,
                "[HOST#%d] [USER#%d?] can't find user record\n",
                host.id, reply.host.userid
            );
            return retval;
        }
        reply.user = user;
        if (strcmp(sreq.authenticator, reply.user.authenticator)) {
            strcpy(reply.message,
               "Invalid or missing authenticator.  "
               "Visit this project's web site to get an authenticator."
            );
            strcpy(reply.message_priority, "low");
            reply.request_delay = 120;
            reply.nucleus_only = true;
            log_messages.printf(
                SchedMessages::CRITICAL,
                "[HOST#%d] [USER#%d] Bad authenticator '%s'\n",
                host.id, user.id, sreq.authenticator
            );
            return ERR_AUTHENTICATOR;
        }

        // If the seqno from the host is less than what we expect,
        // the user must have copied the state file to a different host.
        // Make a new host record.
        if (sreq.rpc_seqno < reply.host.rpc_seqno) {
            sreq.hostid = 0;
            log_messages.printf(
                SchedMessages::NORMAL,
                "[HOST#%d] [USER#%d] RPC seqno %d less than expected %d; creating new host\n",
                reply.host.id, user.id, sreq.rpc_seqno, reply.host.rpc_seqno
            );
            goto make_new_host;
        }
        reply.host.rpc_seqno = sreq.rpc_seqno;
    } else {

        // here no hostid was given; we'll have to create a new host record
        //
lookup_user_and_make_new_host:
        strncpy(
            user.authenticator, sreq.authenticator,
            sizeof(user.authenticator)
        );
        sprintf(buf, "where authenticator='%s'", user.authenticator);
        retval = user.lookup(buf);
        if (retval) {
            strcpy(reply.message,
                "Invalid or missing account ID.  "
                "Visit this project's web site to get an account ID."
            );
            strcpy(reply.message_priority, "low");
            reply.request_delay = 120;
            log_messages.printf(
                SchedMessages::CRITICAL,
                "[HOST#<none>] Bad authenticator '%s'\n",
                sreq.authenticator
            );
            return ERR_AUTHENTICATOR;
        }
        reply.user = user;
make_new_host:
        // reply.user is filled in and valid at this point
        //
        host = sreq.host;
        host.id = 0;
        host.create_time = time(0);
        host.userid = reply.user.id;
        host.rpc_seqno = 0;
        strcpy(host.venue, reply.user.venue);
        host.fix_nans();
        retval = host.insert();
        if (retval) {
            strcpy(reply.message, "Couldn't create host record in database");
            strcpy(reply.message_priority, "low");
            boinc_db.print_error("host.insert()");
            log_messages.printf(SchedMessages::CRITICAL, "host.insert() failed\n");
            return retval;
        }
        host.id = boinc_db.insert_id();

        reply.host = host;
        reply.hostid = reply.host.id;
        // this tells client to updates its host ID
    }
    if (reply.user.teamid) {
        retval = team.lookup_id(reply.user.teamid);
        if (!retval) reply.team = team;
    }
    return 0;
}

// somewhat arbitrary formula for credit as a function of CPU time.
// Could also include terms for RAM size, network speed etc.
//
static void compute_credit_rating(HOST& host) {
    host.credit_per_cpu_sec =
        (fabs(host.p_fpops)/1e9 + fabs(host.p_iops)/1e9 + fabs(host.p_membw)/4e9)
        * COBBLESTONE_FACTOR / (3 * SECONDS_PER_DAY);
}

// Update host record based on request.
// Copy all fields that are determined by the client.
//
int update_host_record(SCHEDULER_REQUEST& sreq, HOST& xhost) {
    int retval;
    DB_HOST host;
    host = xhost;

    host.timezone = sreq.host.timezone;
    strncpy(host.domain_name, sreq.host.domain_name, sizeof(host.domain_name));
    strncpy(host.serialnum, sreq.host.serialnum, sizeof(host.serialnum));
    if (strcmp(host.last_ip_addr, sreq.host.last_ip_addr)) {
        strncpy(host.last_ip_addr, sreq.host.last_ip_addr, sizeof(host.last_ip_addr));
    } else {
        host.nsame_ip_addr++;
    }
    host.on_frac = sreq.host.on_frac;
    host.connected_frac = sreq.host.connected_frac;
    host.active_frac = sreq.host.active_frac;
    host.p_ncpus = sreq.host.p_ncpus;
    strncpy(host.p_vendor, sreq.host.p_vendor, sizeof(host.p_vendor));
        // unlikely this will change
    strncpy(host.p_model, sreq.host.p_model, sizeof(host.p_model));
    host.p_fpops = sreq.host.p_fpops;
    host.p_iops = sreq.host.p_iops;
    host.p_membw = sreq.host.p_membw;
    host.p_calculated = sreq.host.p_calculated;
    strncpy(host.os_name, sreq.host.os_name, sizeof(host.os_name));
    strncpy(host.os_version, sreq.host.os_version, sizeof(host.os_version));
    host.m_nbytes = sreq.host.m_nbytes;
    host.m_cache = sreq.host.m_cache;
    host.m_swap = sreq.host.m_swap;
    host.d_total = sreq.host.d_total;
    host.d_free = sreq.host.d_free;
    host.n_bwup = sreq.host.n_bwup;
    host.n_bwdown = sreq.host.n_bwdown;
    host.fix_nans();

    compute_credit_rating(host);

    retval = host.update();
    if (retval) {
        log_messages.printf(SchedMessages::CRITICAL, "host.update() failed: %d\n", retval);
    }
    return 0;
}

// Deal with global preferences.
// If the client sent global prefs, and they're more recent than ours,
// update user record in DB.
// If DB has more recent global prefs than client's, send them.
//
int handle_global_prefs(SCHEDULER_REQUEST& sreq, SCHEDULER_REPLY& reply) {
    reply.send_global_prefs = false;
    if (strlen(sreq.global_prefs_xml)) {
        bool need_update = false;
        unsigned req_mod_time=0, db_mod_time=0;
        parse_int(sreq.global_prefs_xml, "<mod_time>", (int&)req_mod_time);
        if (strlen(reply.user.global_prefs)) {
            parse_int(reply.user.global_prefs, "<mod_time>", (int&)db_mod_time);
            if (req_mod_time > db_mod_time) {
                need_update = true;
            } else if (req_mod_time < db_mod_time) {
                reply.send_global_prefs = true;
            }
        } else {
            need_update = true;
        }
        if (need_update) {
            safe_strcpy(reply.user.global_prefs, sreq.global_prefs_xml);
            DB_USER user;
            user = reply.user;
            user.update();
        }
    } else {
        if (strlen(reply.user.global_prefs)) {
            reply.send_global_prefs = true;
        }
    }
    return 0;
}

// handle completed results
//
int handle_results(
    SCHEDULER_REQUEST& sreq, SCHEDULER_REPLY& reply, HOST& host
) {
    unsigned int i;
    int retval;
    DB_RESULT result;
    RESULT* rp;
    DB_WORKUNIT wu;
    char buf[256];

    for (i=0; i<sreq.results.size(); i++) {
        rp = &sreq.results[i];

        // acknowledge the result even if we couldn't find it --
        // don't want it to keep coming back
        //
        reply.result_acks.push_back(*rp);

        strncpy(result.name, rp->name, sizeof(result.name));
        sprintf(buf, "where name='%s'", result.name);
        retval = result.lookup(buf);
        if (retval) {
            log_messages.printf(
                SchedMessages::CRITICAL,
                "[HOST#%d] [RESULT#? %s] can't find result\n",
                host.id, rp->name
            );
            continue;
        }

        log_messages.printf(
            SchedMessages::NORMAL, "[HOST#%d] [RESULT#%d %s] got result\n",
            host.id, result.id, result.name
        );

        if (result.server_state == RESULT_SERVER_STATE_UNSENT) {
            log_messages.printf(
                SchedMessages::CRITICAL,
                "[HOST#%d] [RESULT#%d %s] got unexpected result: server state is %d\n",
                host.id, result.id, result.name, result.server_state
            );
            continue;
        }

        if (result.received_time) {
            log_messages.printf(
                SchedMessages::CRITICAL,
                "[HOST#%d] [RESULT#%d %s] got result twice\n",
                host.id, result.id, result.name
            );
            continue;
        }

        if (result.hostid != sreq.hostid) {
            log_messages.printf(
                SchedMessages::CRITICAL,
                "[HOST#%d] [RESULT#%d %s] got result from wrong host; expected [HOST#%d]\n",
                host.id, result.id, result.name, result.hostid
            );
            DB_HOST result_host;
            int retval = result_host.lookup_id(result.hostid);

            if (retval) {
                log_messages.printf(
                    SchedMessages::CRITICAL,
                    "[RESULT#%d %s] Can't lookup [HOST#%d]\n",
                    result.id, result.name, result.hostid
                );
                continue;
            } else if (result_host.userid != host.userid) {
                log_messages.printf(
                    SchedMessages::CRITICAL,
                    "[USER#%d] [HOST#%d] [RESULT#%d %s] Not even the same user; expected [USER#%d]\n",
                    host.userid, host.id, result.id, result.name, result_host.userid
                );
                continue;
            } else {
                log_messages.printf(
                    SchedMessages::CRITICAL,
                    "[HOST#%d] [RESULT#%d %s] Allowing result because same USER#%d\n",
                    host.id, result.id, result.name, host.userid
                );
            }
        }

        // update the result record in DB
        //
        result.hostid = reply.host.id;
        result.received_time = time(0);
        result.client_state = rp->client_state;
        result.cpu_time = rp->cpu_time;
        result.claimed_credit = result.cpu_time * host.credit_per_cpu_sec;
        result.server_state = RESULT_SERVER_STATE_OVER;

		strncpy(result.stderr_out, rp->stderr_out, sizeof(result.stderr_out));
        strncpy(result.xml_doc_out, rp->xml_doc_out, sizeof(result.xml_doc_out));
        parse_int(result.stderr_out, "<app_version>", result.app_version_num);

		// look for exit status in stderr_out
        // TODO: return separately
        //
        parse_int(result.stderr_out, "<exit_status>", result.exit_status);

		// Success can only be declared if all the result files have been successfully uploaded
		// and the client exit status returns 0
        if ( (result.client_state == RESULT_FILES_UPLOADED) && (0 == result.exit_status) ) {
            result.outcome = RESULT_OUTCOME_SUCCESS;
        } else {
            result.outcome = RESULT_OUTCOME_CLIENT_ERROR;
            result.validate_state = VALIDATE_STATE_INVALID;
        }

        result.teamid = reply.user.teamid;
        retval = result.update();
        if (retval) {
            log_messages.printf(
                SchedMessages::NORMAL,
                "[HOST#%d] [RESULT#%d %s] can't update result: %s\n",
                host.id, result.id, result.name, boinc_db.error_string()
            );
        }

        // trigger the transition handle for the result's WU
        //
        retval = wu.lookup_id(result.workunitid);
        if (retval) {
            log_messages.printf(
                SchedMessages::CRITICAL,
                "[HOST#%d] [RESULT#%d %s] Can't find [WU#%d] for result\n",
                host.id, result.id, result.name, result.workunitid
            );
        } else {
            wu.transition_time = time(0);
            retval = wu.update();
            if (retval) {
                log_messages.printf(
                    SchedMessages::CRITICAL,
                    "[HOST#%d] [RESULT#%d %s] Can't update [WU#%d %s]\n",
                    host.id, result.id, result.name, wu.id, wu.name
                );
            }
        }
    }
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

    for (i=0; i<ss.nwu_results; i++) {
        if (wreq.seconds_to_fill <= 0) break;
        if (wreq.disk_available <= 0) break;
        if (wreq.nresults >= config.max_wus_to_send) break;

        WU_RESULT& wu_result = ss.wu_results[i];

        // the following should be a critical section
        //
        if (!wu_result.present) {
            continue;
        }

        if (wu_result.workunit.rsc_disk_bound > wreq.disk_available) {
            wreq.insufficient_disk = true;
            wu_result.infeasible_count++;
            continue;
        }

        if (wreq.infeasible_only && (wu_result.infeasible_count==0)) {
            continue;
        }

        // don't send if we're already sending a result for same WU
        //
        if (already_in_reply(wu_result, reply)) {
            continue;
        }

        // don't send if we've already sent a result of this WU to this user
        //
        if (config.one_result_per_user_per_wu) {
            sprintf(buf,
                "where workunitid=%d and userid=%d",
                wu_result.workunit.id, reply.user.id
            );
            retval = result.count(n, buf);
            if (retval) {
                log_messages.printf(
                    SchedMessages::CRITICAL,
                    "send_work: can't get result count (%d)\n", retval
                );
                continue;
            } else {
                if (n>0) {
#if 0
                    log_messages.printf(
                        SchedMessages::NORMAL,
                        "send_work: user %d already has %d result(s) for WU %d\n",
                        reply.user.id, n, wu_result.workunit.id
                    );
#endif
                    continue;
                }
            }
        }

        // don't send if host can't handle it
        //
        wu = wu_result.workunit;
        if (!wu_is_feasible(wu, reply.host, wreq)) {
            log_messages.printf(
                SchedMessages::DEBUG, "[HOST#%d] [WU#%d %s] WU is infeasible\n",
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
        }

        result = wu_result.result;

        // mark slot as empty AFTER we've copied out of it
        // (since otherwise feeder might overwrite it)
        //
        wu_result.present = false;

        // reread result from DB, make sure it's still unsent
        // TODO: from here to update() should be a transaction
        //
        retval = result.lookup_id(result.id);
        if (retval) continue;
        if (result.server_state != RESULT_SERVER_STATE_UNSENT) continue;

        // ****** HERE WE'VE COMMITTED TO SENDING THIS RESULT TO HOST ******
        //

        retval = add_wu_to_reply(wu, reply, platform, ss, wreq, app, avp);
        if (retval) continue;

        wreq.disk_available -= wu.rsc_disk_bound;

        // update the result in DB
        //
        result.server_state = RESULT_SERVER_STATE_IN_PROGRESS;
        result.hostid = reply.host.id;
        result.userid = reply.user.id;
        result.sent_time = time(0);
        result.report_deadline = result.sent_time + wu.delay_bound;
        result.update();

        wu_seconds_filled = estimate_cpu_duration(wu, reply.host);
        log_messages.printf(
            SchedMessages::NORMAL,
            "[HOST#%d] Sending [RESULT#%d %s] (fills %d seconds)\n",
            reply.host.id, result.id, result.name, int(wu_seconds_filled)
        );

        retval = update_wu_transition_time(wu, result.report_deadline);
        if (retval) {
            log_messages.printf(
                SchedMessages::CRITICAL,
                "send_work: can't update WU transition time\n"
            );
        }

        // The following overwrites the result's xml_doc field.
        // But that's OK cuz we're done with DB updates
        //
        retval = insert_name_tags(result, wu);
        if (retval) {
            log_messages.printf(
                SchedMessages::CRITICAL, "send_work: can't insert name tags\n"
            );
        }
        retval = insert_deadline_tag(result);
        if (retval) {
            log_messages.printf(
                SchedMessages::CRITICAL,
                "send_work: can't insert deadline tag\n"
            );
        }
        reply.insert_result(result);

        wreq.seconds_to_fill -= wu_seconds_filled;

        wreq.nresults++;
    }
}

int send_work(
    SCHEDULER_REQUEST& sreq, SCHEDULER_REPLY& reply, PLATFORM& platform,
    SCHED_SHMEM& ss
) {
    WORK_REQ wreq;

    memset(&wreq, 0, sizeof(wreq));
    wreq.disk_available = max_allowable_disk(reply.user, sreq);
    wreq.insufficient_disk = false;
    wreq.insufficient_mem = false;
    wreq.insufficient_speed = false;
    wreq.no_app_version = false;
    wreq.nresults = 0;

    log_messages.printf(
        SchedMessages::NORMAL,
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

    log_messages.printf(
        SchedMessages::NORMAL, "[HOST#%d] Sent %d results\n",
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
        if (wreq.insufficient_mem) {
            strcat(reply.message,
                " (there was work but your computer would not finish it before it is due"
            );
        }
        strcpy(reply.message_priority, "low");
        reply.request_delay = 10;

        log_messages.printf(
            SchedMessages::NORMAL, "[HOST#%d] %s\n",
            reply.host.id, reply.message
        );
    }
    return 0;
}

// if the client has an old code sign public key,
// send it the new one, with a signature based on the old one.
// If they don't have a code sign key, send them one
//
void send_code_sign_key(
    SCHEDULER_REQUEST& sreq, SCHEDULER_REPLY& reply, char* code_sign_key
) {
    char* oldkey, *signature;
    int i, retval;
    char path[256];

    if (strlen(sreq.code_sign_key)) {
        if (strcmp(sreq.code_sign_key, code_sign_key)) {
            log_messages.printf(SchedMessages::NORMAL, "received old code sign key\n");

            // look for a signature file
            //
            for (i=0; ; i++) {
                sprintf(path, "%s/old_key_%d", config.key_dir, i);
                retval = read_file_malloc(path, oldkey);
                if (retval) {
                    strcpy(reply.message,
                           "Can't update code signing key.  "
                           "Please report this to the project administrators."
                    );
                    return;
                }
                if (!strcmp(oldkey, sreq.code_sign_key)) {
                    sprintf(path, "%s/signature_%d", config.key_dir, i);
                    retval = read_file_malloc(path, signature);
                    if (retval) {
                        strcpy(reply.message,
                               "Can't update code signing key.  "
                               "Please report this to the project administrators."
                        );
                    } else {
                        safe_strcpy(reply.code_sign_key, code_sign_key);
                        safe_strcpy(reply.code_sign_key_signature, signature);
                        free(signature);
                    }
                }
                free(oldkey);
                return;
            }
        }
    } else {
        safe_strcpy(reply.code_sign_key, code_sign_key);
    }
}

bool wrong_major_version(SCHEDULER_REQUEST& sreq, SCHEDULER_REPLY& reply) {
    // char buf[256];
    if (sreq.core_client_major_version != MAJOR_VERSION) {
        reply.nucleus_only = true;
        // TODO: check for user-agent not empty and not BOINC
        reply.probable_user_browser = true;
        sprintf(reply.message,
            "To participate in this project, "
            "you must use major version %d of the BOINC core client. "
            "Your core client is major version %d.",
            MAJOR_VERSION,
            sreq.core_client_major_version
        );
        strcpy(reply.message_priority, "low");
        log_messages.printf(
            SchedMessages::NORMAL,
            "[HOST#%d] [auth %s] Wrong major version from user: wanted %d, got %d\n",
            sreq.hostid, sreq.authenticator,
            MAJOR_VERSION, sreq.core_client_major_version
        );
        return true;
    }
    return false;
}

inline static const char* get_remote_addr() {
    const char * r = getenv("REMOTE_ADDR");
    return r ? r : "?.?.?.?";
}

void handle_trickles(SCHEDULER_REQUEST& sreq, SCHEDULER_REPLY& reply) {
    unsigned int i;
    DB_RESULT result;
    DB_TRICKLE trickle;
    int retval;
    char buf[256];

    for (i=0; i<sreq.trickles.size(); i++) {
        reply.send_trickle_ack = true;
        TRICKLE_DESC& td = sreq.trickles[i];
        sprintf(buf, "where name='%s'", td.result_name);
        retval = result.lookup(buf);
        if (retval) continue;
        if (reply.user.id != result.userid) continue;
        memset(&trickle, 0, sizeof(trickle));
        trickle.create_time = td.create_time;
        trickle.resultid = result.id;
        trickle.hostid = sreq.host.id;
        trickle.userid = reply.user.id;
        trickle.appid = result.appid;
        safe_strcpy(trickle.xml, td.trickle_text.c_str());
        retval = trickle.insert();
    }
}

void process_request(
    SCHEDULER_REQUEST& sreq, SCHEDULER_REPLY& reply, SCHED_SHMEM& ss,
    char* code_sign_key
) {
    PLATFORM* platform;
    int retval;
    double last_rpc_time;

    // if different major version of BOINC, just send a message
    //
    if (wrong_major_version(sreq, reply)) return;

    retval = authenticate_user(sreq, reply);
    if (retval) return;
    if (reply.user.id == 0) {
        log_messages.printf(SchedMessages::CRITICAL, "No user ID!\n");
    }

    last_rpc_time = reply.host.rpc_time;
    reply.host.rpc_time = time(0);
    retval = update_host_record(sreq, reply.host);

    // look up the client's platform in the DB
    //
    platform = ss.lookup_platform(sreq.platform_name);
    if (!platform) {
        sprintf(reply.message, "platform '%s' not found", sreq.platform_name);
        strcpy(reply.message_priority, "low");
        log_messages.printf(
            SchedMessages::CRITICAL, "[HOST#%d] platform '%s' not found\n",
            reply.host.id, sreq.platform_name
        );
        return;
    }

    log_messages.printf(
        SchedMessages::NORMAL, "Processing request from [USER#%d] [HOST#%d] [IP %s] [RPC#%d] core client version %d.%02d\n",
        reply.user.id, reply.host.id,
        get_remote_addr(),
        sreq.rpc_seqno,
        sreq.core_client_major_version, sreq.core_client_minor_version
    );
    ++log_messages;
    handle_global_prefs(sreq, reply);

    handle_results(sreq, reply, reply.host);

    // if last RPC was within config.min_sendwork_interval, don't send work
    //
    bool ok_to_send = true;
    if (config.min_sendwork_interval) {
        double diff = dtime() - last_rpc_time;
        if (diff < config.min_sendwork_interval) {
            ok_to_send = false;
            log_messages.printf(
                SchedMessages::NORMAL,
                "Not sending work - last RPC too recent: %f\n", diff
            );
            sprintf(reply.message,
                "Not sending work - last RPC too recent: %d sec", (int)diff
            );
            strcpy(reply.message_priority, "low");
        }
    }
    if (ok_to_send) {
        send_work(sreq, reply, *platform, ss);
    }

    send_code_sign_key(sreq, reply, code_sign_key);

    handle_trickles(sreq, reply);
}

void handle_request(
    FILE* fin, FILE* fout, SCHED_SHMEM& ss, char* code_sign_key
) {
    SCHEDULER_REQUEST sreq;
    SCHEDULER_REPLY sreply;

    memset(&sreq, 0, sizeof(sreq));
    sreq.parse(fin);
    log_messages.printf(
        SchedMessages::NORMAL, "Handling request: IP %s, auth %s, platform %s, version %d.%d\n",
        get_remote_addr(), sreq.authenticator, sreq.platform_name,
        sreq.core_client_major_version, sreq.core_client_minor_version
    );
    process_request(sreq, sreply, ss, code_sign_key);
    sreply.write(fout);
}
