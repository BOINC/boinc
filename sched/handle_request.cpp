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

// Handle a scheduling server RPC

#include "config.h"
#ifdef _USING_FCGI_
#include "boinc_fcgi.h"
#else
#include <cstdio>
#endif
#include <cassert>
#include <cstdlib>
#include <vector>
#include <string>
#include <cstring>
#include <ctime>
#include <cmath>

#include <unistd.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/stat.h>

#include "boinc_db.h"
#include "backend_lib.h"
#include "error_numbers.h"
#include "parse.h"
#include "str_util.h"
#include "str_replace.h"
#include "util.h"
#include "filesys.h"

#include "sched_vda.h"

#include "credit.h"
#include "sched_main.h"
#include "sched_types.h"
#include "sched_util.h"
#include "handle_request.h"
#include "sched_msgs.h"
#include "sched_resend.h"
#include "sched_send.h"
#include "sched_config.h"
#include "sched_locality.h"
#include "sched_result.h"
#include "sched_customize.h"
#include "time_stats_log.h"


// find the user's most recently-created host with given various characteristics
//
static bool find_host_by_other(DB_USER& user, HOST req_host, DB_HOST& host) {
    char buf[2048];
    char dn[512], ip[512], os[512], pm[512];

    // don't dig through hosts of these users
    // prevents flooding the DB with slow queries from users with many hosts
    for(unsigned int i=0; i < config.dont_search_host_for_userid.size(); i++)
      if (user.id == config.dont_search_host_for_userid[i])
        return false;

    // Only check if the fields are populated
    if (strlen(req_host.domain_name) && strlen(req_host.last_ip_addr) && strlen(req_host.os_name) && strlen(req_host.p_model)) {
        strcpy(dn, req_host.domain_name);
        escape_string(dn, 512);
        strcpy(ip, req_host.last_ip_addr);
        escape_string(ip, 512);
        strcpy(os, req_host.os_name);
        escape_string(os, 512);
        strcpy(pm, req_host.p_model);
        escape_string(pm, 512);

        sprintf(buf,
            "where userid=%d and id>%d and domain_name='%s' and last_ip_addr = '%s' and os_name = '%s' and p_model = '%s'"
               " and m_nbytes = %lf order by id desc", user.id, req_host.id, dn, ip, os, pm, req_host.m_nbytes
        );
        if (!host.enumerate(buf)) {
            host.end_enumerate();
            return true;
        }
    }
    return false;
}

static void send_error_message(const char* msg, int delay) {
    g_reply->insert_message(msg, "low");
    g_reply->set_delay(delay);
    g_reply->nucleus_only = true;
}

// Try to lock a file with name based on host ID,
// to prevent 2 schedulers from running at same time for same host.
// Return:
// 0 if successful
//    In this case store file descriptor in reply struct so we can unlock later
//    In other cases store -1 in reply struct
// PID (>0) if another process has lock
// -1 if error (e.g. can't create file)
//
int lock_sched() {
    char filename[256];
    char pid_string[16];
    int fd, pid, count;

    g_reply->lockfile_fd=-1;

    sprintf(filename, "%s/CGI_%07d",
        config.sched_lockfile_dir, g_reply->host.id
    );

    fd = open(filename, O_WRONLY|O_CREAT, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
    if (fd < 0) return -1;

    // if we can't get an advisory write lock on the file,
    // return the PID of the process that DOES hold the lock.
    // (or -1 if failure)
    //
    pid = mylockf(fd);
    if (pid) {
        close(fd);
        return pid;
    }

    // write PID into the CGI_<HOSTID> file and flush to disk
    //
    count = sprintf(pid_string, "%d\n", getpid());
    ssize_t n = write(fd, pid_string, count);
    if (n < 0) {
        close(fd);
        return -1;
    }
    fsync(fd);

    g_reply->lockfile_fd = fd;
    return 0;
}

// unlock and delete per-host lockfile
//
void unlock_sched() {
    char filename[256];

    if (g_reply->lockfile_fd < 0) return;
    sprintf(filename, "%s/CGI_%07d", config.sched_lockfile_dir, g_reply->host.id);
    unlink(filename);
    close(g_reply->lockfile_fd);
}


// find the user's most recently-created host with given host CPID
//
static bool find_host_by_cpid(DB_USER& user, char* host_cpid, DB_HOST& host) {
    char buf[256], buf2[256];
    sprintf(buf, "%s%s", host_cpid, user.email_addr);
    md5_block((const unsigned char*)buf, strlen(buf), buf2);

    sprintf(buf,
        "where userid=%d and host_cpid='%s' order by id desc", user.id, buf2
    );
    if (!host.enumerate(buf)) {
        host.end_enumerate();
        return true;
    }
    return false;
}

// Called when there's evidence that the host has detached.
// Mark in-progress results for the given host
// as server state OVER, outcome CLIENT_DETACHED.
// This serves two purposes:
// 1) make sure we don't resend these results to the host
//    (they may be the reason the user detached)
// 2) trigger the generation of new results for these WUs
//
static void mark_results_over(DB_HOST& host) {
    char buf[256], buf2[256];
    DB_RESULT result;
    sprintf(buf, "where hostid=%d and server_state=%d",
        host.id,
        RESULT_SERVER_STATE_IN_PROGRESS
    );
    while (!result.enumerate(buf)) {
        sprintf(buf2,
            "server_state=%d, outcome=%d, received_time = %ld",
            RESULT_SERVER_STATE_OVER,
            RESULT_OUTCOME_CLIENT_DETACHED,
            time(0)
        );
        result.update_field(buf2);

        // and trigger WU transition
        //
        DB_WORKUNIT wu;
        wu.id = result.workunitid;
        sprintf(buf2, "transition_time=%d", (int)time(0));
        wu.update_field(buf2);

        log_messages.printf(MSG_CRITICAL,
            "[HOST#%d] [RESULT#%d] [WU#%d] changed CPID: marking in-progress result %s as client error!\n",
            host.id, result.id, result.workunitid, result.name
        );
    }
}

// Based on the info in the request message,
// look up the host and its user, and make sure the authenticator matches.
// Some special cases:
//  1) If no host ID is supplied, or if RPC seqno mismatch,
//     create a new host record
//  2) If the host record specified by g_request->hostid is a "zombie"
//     (i.e. it was merged with another host via the web site)
//     then follow links to find the proper host
//
// POSTCONDITION:
// If this function returns zero, then:
// - reply.host contains a valid host record (possibly new)
// - reply.user contains a valid user record
// - if user belongs to a team, reply.team contains team record
//
int authenticate_user() {
    int retval;
    char buf[256];
    DB_HOST host;
    DB_USER user;
    DB_TEAM team;

    if (g_request->hostid) {
        retval = host.lookup_id(g_request->hostid);
        while (!retval && host.userid==0) {
            // if host record is zombie, follow link to new host
            //
            retval = host.lookup_id(host.rpc_seqno);
            if (!retval) {
                g_reply->hostid = host.id;
                log_messages.printf(MSG_NORMAL,
                    "[HOST#%d] forwarding to new host ID %d\n",
                    g_request->hostid, host.id
                );
            }
        }
        if (retval) {
            g_reply->insert_message("Can't find host record", "low");
            log_messages.printf(MSG_NORMAL,
                "[HOST#%d?] can't find host\n",
                g_request->hostid
            );
            g_request->hostid = 0;
            goto lookup_user_and_make_new_host;
        }

        g_reply->host = host;

        // look up user based on the ID in host record,
        // and see if the authenticator matches (regular or weak)
        //
        g_request->using_weak_auth = false;
        sprintf(buf, "where id=%d", host.userid);
        retval = user.lookup(buf);
        if (!retval && !strcmp(user.authenticator, g_request->authenticator)) {
            // req auth matches user auth - go on
        } else {
            if (!retval) {
                // user for host.userid exists - check weak auth
                //
                get_weak_auth(user, buf);
                if (!strcmp(buf, g_request->authenticator)) {
                    g_request->using_weak_auth = true;
                    log_messages.printf(MSG_DEBUG,
                        "[HOST#%d] accepting weak authenticator\n",
                        host.id
                    );
                }
            }
            if (!g_request->using_weak_auth) {
                // weak auth failed - look up user based on authenticator
                //
                strlcpy(
                    user.authenticator, g_request->authenticator, sizeof(user.authenticator)
                );
                sprintf(buf, "where authenticator='%s'", user.authenticator);
                retval = user.lookup(buf);
                if (retval) {
                    g_reply->insert_message(
                        _("Invalid or missing account key.  To fix, remove and add this project."),
                        "notice"
                    );
                    g_reply->set_delay(DELAY_MISSING_KEY);
                    g_reply->nucleus_only = true;
                    log_messages.printf(MSG_CRITICAL,
                        "[HOST#%d] [USER#%d] Bad authenticator '%s'\n",
                        host.id, user.id, g_request->authenticator
                    );
                    return ERR_AUTHENTICATOR;
                }
            }
        }

        g_reply->user = user;

        if (host.userid != user.id) {
            // If the request's host ID isn't consistent with the authenticator,
            // create a new host record.
            //
            log_messages.printf(MSG_NORMAL,
                "[HOST#%d] [USER#%d] inconsistent host ID; creating new host\n",
                host.id, user.id
            );
            goto make_new_host;
        }


        // If the seqno from the host is less than what we expect,
        // the user must have copied the state file to a different host.
        // Make a new host record.
        //
        if (!batch && g_request->rpc_seqno < g_reply->host.rpc_seqno) {
            g_request->hostid = 0;
            log_messages.printf(MSG_NORMAL,
                "[HOST#%d] [USER#%d] RPC seqno %d less than expected %d; creating new host\n",
                g_reply->host.id, user.id, g_request->rpc_seqno, g_reply->host.rpc_seqno
            );
            goto make_new_host;
        }

    } else {
        // Here no hostid was given, or the ID was bad.
        // Look up the user, then create a new host record
        //
lookup_user_and_make_new_host:
        // if authenticator contains _, it's a weak auth
        //
        if (strchr(g_request->authenticator, '_')) {
            int userid = atoi(g_request->authenticator);
            retval = user.lookup_id(userid);
            if (!retval) {
                get_weak_auth(user, buf);
                if (strcmp(buf, g_request->authenticator)) {
                    retval = ERR_AUTHENTICATOR;
                }
            }
        } else {
            strlcpy(
                user.authenticator, g_request->authenticator,
                sizeof(user.authenticator)
            );
            sprintf(buf, "where authenticator='%s'", user.authenticator);
            retval = user.lookup(buf);
        }
        if (retval) {
            g_reply->insert_message(
                "Invalid or missing account key.  To fix, remove and add this project .",
                "low"
            );
            g_reply->set_delay(DELAY_MISSING_KEY);
            log_messages.printf(MSG_CRITICAL,
                "[HOST#<none>] Bad authenticator '%s': %s\n",
                g_request->authenticator, boincerror(retval)
            );
            return ERR_AUTHENTICATOR;
        }
        g_reply->user = user;

        // If host CPID is present,
        // scan backwards through this user's hosts,
        // looking for one with the same host CPID.
        // If we find one, it means the user detached and reattached.
        // Use the existing host record,
        // and mark in-progress results as over.
        //
        if (strlen(g_request->host.host_cpid)) {
            if (find_host_by_cpid(user, g_request->host.host_cpid, host)) {
                log_messages.printf(MSG_CRITICAL,
                    "[HOST#%d] [USER#%d] User has another host with same CPID.\n",
                    host.id, host.userid
                );
                if ((g_request->allow_multiple_clients != 1)
                    && (g_request->other_results.size() == 0)
                ) {
                    mark_results_over(host);
                }
                goto got_host;
            }
        }

make_new_host:
        // One final attempt to locate an existing host record:
        // scan backwards through this user's hosts,
        // looking for one with the same host name,
        // IP address, processor and amount of RAM.
        // If found, use the existing host record,
        // and mark in-progress results as over.
        //
        // NOTE: If the client was run with --allow_multiple_clients, skip this.
        //
        if ((g_request->allow_multiple_clients != 1)
            && find_host_by_other(user, g_request->host, host)
        ) {
            log_messages.printf(MSG_NORMAL,
                "[HOST#%d] [USER#%d] Found similar existing host for this user - assigned.\n",
                host.id, host.userid
            );
            mark_results_over(host);
            goto got_host;
        }
        // either of the above cases,
        // or host ID didn't match user ID,
        // or RPC seqno was too low.
        //
        // Create a new host.
        // g_reply->user is filled in and valid at this point
        //
        host = g_request->host;
        host.id = 0;
        host.create_time = time(0);
        host.userid = g_reply->user.id;
        host.rpc_seqno = 0;
        host.expavg_time = time(0);
        strcpy(host.venue, g_reply->user.venue);
        host.fix_nans();
        retval = host.insert();
        if (retval) {
            g_reply->insert_message(
                "Couldn't create host record in database", "low"
            );
            boinc_db.print_error("host.insert()");
            log_messages.printf(MSG_CRITICAL, "host.insert() failed\n");
            return retval;
        }
        host.id = boinc_db.insert_id();

got_host:
        g_reply->host = host;
        g_reply->hostid = g_reply->host.id;
        // this tells client to updates its host ID
        g_request->rpc_seqno = 0;
            // this value eventually gets written to host DB record;
            // for new hosts it must be zero.
            // This kludge forces this.
    }

    // have user record in g_reply->user at this point
    //

    if (g_reply->user.teamid) {
        retval = team.lookup_id(g_reply->user.teamid);
        if (!retval) g_reply->team = team;
    }

    // compute email hash
    //
    md5_block(
        (unsigned char*)g_reply->user.email_addr,
        strlen(g_reply->user.email_addr),
        g_reply->email_hash
    );

    // if new user CPID, update user record
    //
    if (!g_request->using_weak_auth && strlen(g_request->cross_project_id)) {
        if (strcmp(g_request->cross_project_id, g_reply->user.cross_project_id)) {
            user.id = g_reply->user.id;
            escape_string(g_request->cross_project_id, sizeof(g_request->cross_project_id));
            sprintf(buf, "cross_project_id='%s'", g_request->cross_project_id);
            unescape_string(g_request->cross_project_id, sizeof(g_request->cross_project_id));
            user.update_field(buf);
        }
    }

    return 0;
}

// modify host struct based on request.
// Copy all fields that are determined by the client.
//
static int modify_host_struct(HOST& host) {
    host.timezone = g_request->host.timezone;
    strncpy(host.domain_name, g_request->host.domain_name, sizeof(host.domain_name));
    char buf[256], buf2[256];
    sprintf(buf, "[BOINC|%d.%d.%d]",
        g_request->core_client_major_version,
        g_request->core_client_minor_version,
        g_request->core_client_release
    );
    g_request->coprocs.summary_string(buf2, sizeof(buf2));
    strlcpy(host.serialnum, buf, sizeof(host.serialnum));
    strlcat(host.serialnum, buf2, sizeof(host.serialnum));
    if (strlen(g_request->host.virtualbox_version)) {
        sprintf(buf2, "[vbox|%s]", g_request->host.virtualbox_version);
        strlcat(host.serialnum, buf2, sizeof(host.serialnum));
    }
    if (strcmp(host.last_ip_addr, g_request->host.last_ip_addr)) {
        strncpy(
            host.last_ip_addr, g_request->host.last_ip_addr,
            sizeof(host.last_ip_addr)
        );
        host.nsame_ip_addr = 0;
    } else {
        host.nsame_ip_addr++;
    }
    host.on_frac = g_request->host.on_frac;
    host.connected_frac = g_request->host.connected_frac;
    host.active_frac = g_request->host.active_frac;
    host.duration_correction_factor = g_request->host.duration_correction_factor;
    host.p_ncpus = g_request->host.p_ncpus;
    strncpy(host.p_vendor, g_request->host.p_vendor, sizeof(host.p_vendor));
        // unlikely this will change
    strncpy(host.p_model, g_request->host.p_model, sizeof(host.p_model));
    host.p_fpops = g_request->host.p_fpops;
    host.p_iops = g_request->host.p_iops;
    host.p_membw = g_request->host.p_membw;
    strncpy(host.os_name, g_request->host.os_name, sizeof(host.os_name));
    strncpy(host.os_version, g_request->host.os_version, sizeof(host.os_version));
    host.m_nbytes = g_request->host.m_nbytes;
    host.m_cache = g_request->host.m_cache;
    host.m_swap = g_request->host.m_swap;
    host.d_total = g_request->host.d_total;
    host.d_free = g_request->host.d_free;
    host.d_boinc_used_total = g_request->host.d_boinc_used_total;
    host.d_boinc_used_project = g_request->host.d_boinc_used_project;
    host.n_bwup = g_request->host.n_bwup;
    host.n_bwdown = g_request->host.n_bwdown;
    if (strlen(g_request->host.host_cpid)) {
        strcpy(host.host_cpid, g_request->host.host_cpid);
    }
    host.fix_nans();

    return 0;
}

// update the DB record to the values in "xhost"
// "initial_host" stores the current DB values;
// update only those fields that have changed
//
static int update_host_record(HOST& initial_host, HOST& xhost, USER& user) {
    DB_HOST host;
    int retval;
    char buf[1024];

    host = xhost;

    // hash the CPID reported by the host with the user's email address.
    // This prevents one user from spoofing another one's host.
    //
    if (strlen(host.host_cpid)) {
        sprintf(buf, "%s%s", host.host_cpid, user.email_addr);
        md5_block((const unsigned char*)buf, strlen(buf), host.host_cpid);
    }

    char* p = getenv("REMOTE_ADDR");
    if (p) {
        strlcpy(host.external_ip_addr, p, sizeof(host.external_ip_addr));
    }
    retval = host.update_diff_sched(initial_host);
    if (retval) {
        log_messages.printf(MSG_CRITICAL,
            "host.update() failed: %s\n", boincerror(retval)
        );
    }
    return 0;
}

inline const char* reason_str(int n) {
    switch (n) {
    case ABORT_REASON_NOT_FOUND: return "result not in request";
    case ABORT_REASON_WU_CANCELLED: return "WU cancelled";
    case ABORT_REASON_ASSIMILATED: return "WU assimilated";
    case ABORT_REASON_TIMED_OUT: return "result timed out";
    }
    return "Unknown";
}

// Figure out which of the results the host currently has
// should be aborted outright, or aborted if not started yet
//
int send_result_abort() {
    int aborts_sent = 0;
    int retval = 0;
    DB_IN_PROGRESS_RESULT result;
    std::string result_names;
    unsigned int i;

    if (g_request->other_results.size() == 0) {
        return 0;
    }

    // build list of result names
    //
    for (i=0; i<g_request->other_results.size(); i++) {
        OTHER_RESULT& orp=g_request->other_results[i];
        orp.abort = true;
            // if the host has a result not in the DB, abort it
        orp.abort_if_not_started = false;
        orp.reason = ABORT_REASON_NOT_FOUND;
        if (i > 0) result_names.append(", ");
        result_names.append("'");
        char buf[1024];
        strcpy(buf, orp.name);
        escape_string(buf, 1024);
        result_names.append(buf);
        result_names.append("'");
    }

    // look up selected fields from the results and their WUs,
    // and decide if they should be aborted
    //
    while (!(retval = result.enumerate(g_reply->host.id, result_names.c_str()))) {
        for (i=0; i<g_request->other_results.size(); i++) {
            OTHER_RESULT& orp = g_request->other_results[i];
            if (!strcmp(orp.name, result.result_name)) {
                if (result.error_mask&WU_ERROR_CANCELLED ) {
                    // if the WU has been canceled, abort the result
                    //
                    orp.abort = true;
                    orp.abort_if_not_started = false;
                    orp.reason = ABORT_REASON_WU_CANCELLED;
                } else if (result.assimilate_state == ASSIMILATE_DONE) {
                    // if the WU has been assimilated, abort if not started
                    //
                    orp.abort = false;
                    orp.abort_if_not_started = true;
                    orp.reason = ABORT_REASON_ASSIMILATED;
                } else if (result.server_state == RESULT_SERVER_STATE_OVER
                    && result.outcome == RESULT_OUTCOME_NO_REPLY
                ) {
                    // if timed out, abort if not started
                    //
                    orp.abort = false;
                    orp.abort_if_not_started = true;
                    orp.reason = ABORT_REASON_TIMED_OUT;
                } else {
                    // all is good with the result - let it process
                    orp.abort = false;
                    orp.abort_if_not_started = false;
                }
                break;
            }
        }
    }

    // If enumeration returned an error, don't send any aborts
    //
    if (retval && (retval != ERR_DB_NOT_FOUND)) {
        return retval;
    }

    // loop through the results and send the appropriate message (if any)
    //
    for (i=0; i<g_request->other_results.size(); i++) {
        OTHER_RESULT& orp = g_request->other_results[i];
        if (orp.abort) {
            g_reply->result_aborts.push_back(orp.name);
            log_messages.printf(MSG_NORMAL,
                "[HOST#%d]: Send result_abort for result %s; reason: %s\n",
                g_reply->host.id, orp.name, reason_str(orp.reason)
            );
            // send user message
            char buf[256];
            sprintf(buf, "Result %s is no longer usable", orp.name);
            g_reply->insert_message(buf, "low");
        } else if (orp.abort_if_not_started) {
            g_reply->result_abort_if_not_starteds.push_back(orp.name);
            log_messages.printf(MSG_NORMAL,
                "[HOST#%d]: Send result_abort_if_unstarted for result %s; reason %d\n",
                g_reply->host.id, orp.name, orp.reason
            );
        }
    }

    return aborts_sent;
}

// 1) Decide which global prefs to use for sched decisions: either
// - <working_global_prefs> from request msg
// - <global_prefs> from request message
// - prefs from user DB record
// and parse them into g_request->global_prefs.
// 2) update prefs in user record if needed
// 2) send global prefs in reply msg if needed
//
int handle_global_prefs() {
    char buf[BLOB_SIZE];
    g_reply->send_global_prefs = false;
    bool have_working_prefs = (strlen(g_request->working_global_prefs_xml)>0);
    bool have_master_prefs = (strlen(g_request->global_prefs_xml)>0);
        // absent if the host has host-specific prefs
    bool have_db_prefs = (strlen(g_reply->user.global_prefs)>0);
    bool same_account = !strcmp(
        g_request->global_prefs_source_email_hash, g_reply->email_hash
    );
    double master_mod_time=0, db_mod_time=0, working_mod_time=0;
    if (have_master_prefs) {
        parse_double(g_request->global_prefs_xml, "<mod_time>", master_mod_time);
        if (master_mod_time > dtime()) master_mod_time = dtime();
    }
    if (have_working_prefs) {
        parse_double(g_request->working_global_prefs_xml, "<mod_time>", working_mod_time);
        if (working_mod_time > dtime()) working_mod_time = dtime();
    }
    if (have_db_prefs) {
        parse_double(g_reply->user.global_prefs, "<mod_time>", db_mod_time);
        if (db_mod_time > dtime()) db_mod_time = dtime();
    }

    if (config.debug_prefs) {
        log_messages.printf(MSG_NORMAL,
            "[prefs] have_master:%d have_working: %d have_db: %d\n",
            have_master_prefs, have_working_prefs, have_db_prefs
        );
    }

    // decide which prefs to use for sched decisions,
    // and parse them into g_request->global_prefs
    //
    if (have_working_prefs) {
        g_request->global_prefs.parse(g_request->working_global_prefs_xml, "");
        if (config.debug_prefs) {
            log_messages.printf(MSG_NORMAL, "[prefs] using working prefs\n");
        }
    } else {
        if (have_master_prefs) {
            if (have_db_prefs && db_mod_time > master_mod_time) {
                g_request->global_prefs.parse(g_reply->user.global_prefs, g_reply->host.venue);
                if (config.debug_prefs) {
                    log_messages.printf(MSG_NORMAL,
                        "[prefs] using db prefs - more recent\n"
                    );
                }
            } else {
                g_request->global_prefs.parse(g_request->global_prefs_xml, g_reply->host.venue);
                if (config.debug_prefs) {
                    log_messages.printf(MSG_NORMAL,
                        "[prefs] using master prefs\n"
                    );
                }
            }
        } else {
            if (have_db_prefs) {
                g_request->global_prefs.parse(g_reply->user.global_prefs, g_reply->host.venue);
                if (config.debug_prefs) {
                    log_messages.printf(MSG_NORMAL, "[prefs] using db prefs\n");
                }
            } else {
                g_request->global_prefs.defaults();
                if (config.debug_prefs) {
                    log_messages.printf(MSG_NORMAL, "[prefs] using default prefs\n");
                }
            }
        }
    }

    // decide whether to update DB
    //
    if (!g_request->using_weak_auth && have_master_prefs) {
        bool update_user_record = false;
        if (have_db_prefs) {
            if (master_mod_time > db_mod_time && same_account) {
                update_user_record = true;
            }
        } else {
            if (same_account) update_user_record = true;
        }
        if (update_user_record) {
            if (config.debug_prefs) {
                log_messages.printf(MSG_NORMAL, "[prefs] updating db prefs\n");
            }
            strcpy(g_reply->user.global_prefs, g_request->global_prefs_xml);
            DB_USER user;
            user.id = g_reply->user.id;
            escape_string(g_request->global_prefs_xml, sizeof(g_request->global_prefs_xml));
            sprintf(buf, "global_prefs='%s'", g_request->global_prefs_xml);
            unescape_string(g_request->global_prefs_xml, sizeof(g_request->global_prefs_xml));
            int retval = user.update_field(buf);
            if (retval) {
                log_messages.printf(MSG_CRITICAL,
                    "user.update_field() failed: %s\n", boincerror(retval)
                );
            }
        }
    }

    // decide whether to send DB prefs in reply msg
    //
    if (config.debug_prefs) {
        log_messages.printf(MSG_NORMAL,
            "[prefs] have DB prefs: %d; dbmod %f; global mod %f; working mod %f\n",
            have_db_prefs, db_mod_time, g_request->global_prefs.mod_time, working_mod_time
        );
    }
    if (have_db_prefs && db_mod_time > master_mod_time && db_mod_time > working_mod_time) {
        if (config.debug_prefs) {
            log_messages.printf(MSG_DEBUG,
                "[prefs] sending DB prefs in reply\n"
            );
        }
        g_reply->send_global_prefs = true;
    }
    return 0;
}

// if the client has an old code sign public key,
// send it the new one, with a signature based on the old one.
// If they don't have a code sign key, send them one
//
bool send_code_sign_key(char* code_sign_key) {
    char* oldkey, *signature;
    int i, retval;
    char path[256];

    if (strlen(g_request->code_sign_key)) {
        if (strcmp(g_request->code_sign_key, code_sign_key)) {
            log_messages.printf(MSG_NORMAL, "received old code sign key\n");

            // look for a signature file
            //
            for (i=0; ; i++) {
                sprintf(path, "%s/old_key_%d", config.key_dir, i);
                retval = read_file_malloc(path, oldkey);
                if (retval) {
                    g_reply->insert_message(
                       _("Invalid code signing key.  To fix, remove and add this project."),
                       "notice"
                    );
                    return false;
                }
                if (!strcmp(oldkey, g_request->code_sign_key)) {
                    sprintf(path, "%s/signature_%d", config.key_dir, i);
                    retval = read_file_malloc(path, signature);
                    if (retval) {
                        g_reply->insert_message(
                           _("The project has changed its security key.  Please remove and add this project."),
                           "notice"
                        );
                    } else {
                        safe_strcpy(g_reply->code_sign_key, code_sign_key);
                        safe_strcpy(g_reply->code_sign_key_signature, signature);
                        free(signature);
                    }
                }
                free(oldkey);
                return false;
            }
        }
    } else {
        safe_strcpy(g_reply->code_sign_key, code_sign_key);
    }
    return true;
}

// If <min_core_client_version_announced> is set,
// and the core client version is less than this version,
// send a warning to users to upgrade before deadline
// <min_core_client_upgrade_deadline>
//
void warn_user_if_core_client_upgrade_scheduled() {
    if (g_request->core_client_version < config.min_core_client_version_announced) {

        // time remaining in hours, before upgrade required
        int remaining = config.min_core_client_upgrade_deadline-time(0);
        remaining /= 3600;

        if (0 < remaining) {

            char msg[512];
            int days  = remaining / 24;
            int hours = remaining % 24;

            sprintf(msg,
                "In %d days and %d hours, this project will require a minimum "
                "BOINC version of %d.%d.%d.  You are currently using "
                "version %d.%d.%d; please upgrade before this time.",
                days, hours,
                config.min_core_client_version_announced / 10000,
                (config.min_core_client_version_announced / 100)%100,
                config.min_core_client_version_announced % 100,
                g_request->core_client_major_version,
                g_request->core_client_minor_version,
                g_request->core_client_release
            );
            // make this low priority until three days are left.  Then
            // bump to high.
            //
            if (days<3) {
                g_reply->insert_message(msg, "notice");
            } else {
                g_reply->insert_message(msg, "low");
            }
            log_messages.printf(MSG_DEBUG,
                "Sending warning: upgrade client %d.%d.%d within %d days %d hours\n",
                g_request->core_client_major_version,
                g_request->core_client_minor_version,
                g_request->core_client_release,
                days, hours
            );
        }
    }
    return;
}

bool unacceptable_os() {
    unsigned int i;
    char buf[1024];

    for (i=0; i<config.ban_os->size(); i++) {
        regex_t& re = (*config.ban_os)[i];
        strcpy(buf, g_request->host.os_name);
        strcat(buf, "\t");
        strcat(buf, g_request->host.os_version);
        if (!regexec(&re, buf, 0, NULL, 0)) {
            log_messages.printf(MSG_NORMAL,
                "Unacceptable OS %s %s\n",
                g_request->host.os_name, g_request->host.os_version
            );
            sprintf(buf, "%s %s %s",
                _("This project doesn't support operating system"),
                g_request->host.os_name, g_request->host.os_version
            );
            g_reply->insert_message(buf, "notice");
            g_reply->set_delay(DELAY_UNACCEPTABLE_OS);
            return true;
        }
    }
    return false;
}

bool unacceptable_cpu() {
    unsigned int i;
    char buf[1024];

    for (i=0; i<config.ban_cpu->size(); i++) {
        regex_t& re = (*config.ban_cpu)[i];
        strcpy(buf, g_request->host.p_vendor);
        strcat(buf, "\t");
        strcat(buf, g_request->host.p_model);
        if (!regexec(&re, buf, 0, NULL, 0)) {
            log_messages.printf(MSG_NORMAL,
                "Unacceptable CPU %s %s\n",
                g_request->host.p_vendor, g_request->host.p_model
            );
            sprintf(buf, "%s %s %s",
                _("This project doesn't support CPU type"),
                g_request->host.p_vendor, g_request->host.p_model
            );
            g_reply->insert_message(buf, "notice");
            g_reply->set_delay(DELAY_UNACCEPTABLE_OS);
            return true;
        }
    }
    return false;
}

bool wrong_core_client_version() {
    if (!config.min_core_client_version) {
        return false;
    }
    if (g_request->core_client_version >= config.min_core_client_version) {
        return false;
    }
    log_messages.printf(MSG_NORMAL,
        "[HOST#%d] Wrong client version from user: wanted %d, got %d\n",
        g_request->hostid,
        config.min_core_client_version, g_request->core_client_minor_version
    );
    g_reply->insert_message(
        _("Your BOINC client software is too old.  Please install the current version."),
        "notice"
    );
    g_reply->set_delay(DELAY_BAD_CLIENT_VERSION);
    return true;
}

inline static const char* get_remote_addr() {
    const char * r = getenv("REMOTE_ADDR");
    return r ? r : "?.?.?.?";
}

void handle_msgs_from_host() {
    unsigned int i;
    DB_MSG_FROM_HOST mfh;
    int retval;

    for (i=0; i<g_request->msgs_from_host.size(); i++) {
        g_reply->send_msg_ack = true;
        MSG_FROM_HOST_DESC& md = g_request->msgs_from_host[i];
        mfh.clear();
        mfh.create_time = time(0);
        safe_strcpy(mfh.variety, md.variety);
        mfh.hostid = g_reply->host.id;
        mfh.handled = false;
        safe_strcpy(mfh.xml, md.msg_text.c_str());
        log_messages.printf(MSG_NORMAL,
            "got msg from host; variety %s \n",
            mfh.variety
        );
        retval = mfh.insert();
        if (retval) {
            log_messages.printf(MSG_CRITICAL,
                "[HOST#%d] message insert failed: %s\n",
                g_reply->host.id, boincerror(retval)
            );
            g_reply->send_msg_ack = false;

            // may as well return; if one insert failed, others will too
            //
            return;
        }
    }
}

void handle_msgs_to_host() {
    DB_MSG_TO_HOST mth;
    char buf[256];
    sprintf(buf, "where hostid = %d and handled = %d", g_reply->host.id, 0);
    while (!mth.enumerate(buf)) {
        g_reply->msgs_to_host.push_back(mth);
        mth.handled = true;
        mth.update();
    }
}

static void log_request() {
    log_messages.printf(MSG_NORMAL,
        "Request: [USER#%d] [HOST#%d] [IP %s] client %d.%d.%d\n",
        g_reply->user.id, g_reply->host.id, get_remote_addr(),
        g_request->core_client_major_version,
        g_request->core_client_minor_version,
        g_request->core_client_release
    );
    if (config.debug_request_details) {
        log_messages.printf(MSG_DEBUG,
             "Request details: auth %s, RPC seqno %d, platform %s\n",
             g_request->authenticator,
             g_request->rpc_seqno,
             g_request->platform.name
        );
    }
    log_messages.set_indent_level(2);
}

bool bad_install_type() {
    if (config.no_vista_sandbox) {
        if (!strcmp(g_request->host.os_name, "Microsoft Windows Vista")) {
            if (g_request->sandbox == 1) {
                log_messages.printf(MSG_NORMAL,
                    "Vista secure install - not sending work\n"
                );
                g_reply->insert_message(
                    "Unable to send work to Vista with BOINC installed in protected mode.  Please reinstall BOINC and uncheck 'Protected application execution'",
                    "notice"
                );
            }
        }
    }
    return false;
}

static inline bool requesting_work() {
    if (g_request->work_req_seconds > 0) return true;
    if (g_request->cpu_req_secs > 0) return true;
    if (g_request->coprocs.nvidia.count && g_request->coprocs.nvidia.req_secs) return true;
    if (g_request->coprocs.ati.count && g_request->coprocs.ati.req_secs) return true;
    return false;
}

void process_request(char* code_sign_key) {
    PLATFORM* platform;
    int retval;
    double last_rpc_time, x;
    struct tm *rpc_time_tm;
    bool ok_to_send_work = !config.dont_send_jobs;
    bool have_no_work = false;
    char buf[256];
    HOST initial_host;
    unsigned int i;
    time_t t;

    memset(&g_reply->wreq, 0, sizeof(g_reply->wreq));

    // if different major version of BOINC, just send a message
    //
    if (wrong_core_client_version()
        || unacceptable_os()
        || unacceptable_cpu()
    ) {
        ok_to_send_work = false;
    }

    // if no jobs reported and none to send, return without accessing DB
    //
    if (!ok_to_send_work && !g_request->results.size()) {
        return;
    }

    warn_user_if_core_client_upgrade_scheduled();

    if (requesting_work()) {
        if (config.locality_scheduling || config.locality_scheduler_fraction || config.enable_assignment) {
            have_no_work = false;
        } else {
            lock_sema();
            have_no_work = ssp->no_work(g_pid);
            if (have_no_work) {
                g_wreq->no_jobs_available = true;
                if (config.debug_send) {
                    log_messages.printf(MSG_NORMAL,
                        "[send] No jobs in shmem\n"
                    );
                }
            }
            unlock_sema();
        }
    }

    // If:
    // - there's no work,
    // - a config flag is set,
    // - client isn't returning results,
    // - this isn't an initial RPC,
    // - client is requesting work
    // then return without accessing the DB.
    // This is an efficiency hack for when servers are overloaded
    //
    if (
        have_no_work
        && config.nowork_skip
        && requesting_work()
        && (g_request->results.size() == 0)
        && (g_request->hostid != 0)
    ) {
        g_reply->insert_message("No work available", "low");
        g_reply->set_delay(DELAY_NO_WORK_SKIP);
        if (!config.msg_to_host && !config.enable_vda) {
            log_messages.printf(MSG_NORMAL, "No work - skipping DB access\n");
            return;
        }
    }

    // FROM HERE ON DON'T RETURN; "goto leave" instead
    // (because ssp->no_work() may have tagged an entry in the work array
    // with our process ID)

    retval = open_database();
    if (retval) {
        send_error_message("Server can't open database", 3600);
        g_reply->project_is_down = true;
        goto leave;
    }

    retval = authenticate_user();
    if (retval) goto leave;
    if (g_reply->user.id == 0) {
        log_messages.printf(MSG_CRITICAL, "No user ID!\n");
    }
    initial_host = g_reply->host;
    g_reply->host.rpc_seqno = g_request->rpc_seqno;

    g_reply->nucleus_only = false;

    log_request();

    // is host blacklisted?
    //
    if (g_reply->host._max_results_day == -1) {
        send_error_message("Not accepting requests from this host", 86400);
        goto leave;
    }

    if (strlen(config.sched_lockfile_dir)) {
        int pid_with_lock = lock_sched();
        if (pid_with_lock > 0) {
            log_messages.printf(MSG_CRITICAL,
                "Another scheduler instance [PID=%d] is running for this host\n",
                pid_with_lock
            );
        } else if (pid_with_lock) {
            log_messages.printf(MSG_CRITICAL,
                "Error acquiring lock for [HOST#%d]\n", g_reply->host.id
            );
        }
        if (pid_with_lock) {
            send_error_message(
                "Another scheduler instance is running for this host", 60
            );
            goto leave;
        }
    }

    // in deciding whether it's a new day,
    // add a random factor (based on host ID)
    // to smooth out network traffic over the day
    //
    retval = rand();
    srand(g_reply->host.id);
    x = drand()*86400;
    srand(retval);
    last_rpc_time = g_reply->host.rpc_time;
    t = (time_t)(g_reply->host.rpc_time + x);
    rpc_time_tm = localtime(&t);
    g_request->last_rpc_dayofyear = rpc_time_tm->tm_yday;

    t = time(0);
    g_reply->host.rpc_time = t;
    t += (time_t)x;
    rpc_time_tm = localtime(&t);
    g_request->current_rpc_dayofyear = rpc_time_tm->tm_yday;

    retval = modify_host_struct(g_reply->host);

    // write time stats to disk if present
    //
    if (g_request->have_time_stats_log) {
        write_time_stats_log();
    }

    // look up the client's platform(s) in the DB
    //
    platform = ssp->lookup_platform(g_request->platform.name);
    if (platform) g_request->platforms.list.push_back(platform);
    for (i=0; i<g_request->alt_platforms.size(); i++) {
        platform = ssp->lookup_platform(g_request->alt_platforms[i].name);
        if (platform) g_request->platforms.list.push_back(platform);
    }
    if (g_request->platforms.list.size() == 0) {
        sprintf(buf, "%s %s",
            _("This project doesn't support computers of type"),
            g_request->platform.name
        );
        g_reply->insert_message(buf, "notice");
        log_messages.printf(MSG_CRITICAL,
            "[HOST#%d] platform '%s' not found\n",
            g_reply->host.id, g_request->platform.name
        );
        g_reply->set_delay(DELAY_PLATFORM_UNSUPPORTED);
        goto leave;
    }

    handle_global_prefs();

    read_host_app_versions();
    update_n_jobs_today();

    handle_results();
    handle_file_xfer_results();
    if (config.enable_vda) {
        handle_vda();
    }

    // Do this before resending lost jobs
    //
    if (bad_install_type()) {
        ok_to_send_work = false;
    }
    if (!requesting_work()) {
        ok_to_send_work = false;
    }
    send_work_setup();

    if (g_request->have_other_results_list) {
        if (ok_to_send_work
            && (config.resend_lost_results || g_wreq->resend_lost_results)
        ) {
            if (resend_lost_work()) {
                ok_to_send_work = false;
            }
        }
        if (config.send_result_abort) {
            send_result_abort();
        }
    }

    if (requesting_work()) {
        if (!send_code_sign_key(code_sign_key)) {
            ok_to_send_work = false;
        }

        // if last RPC was within config.min_sendwork_interval, don't send work
        //
        if (!have_no_work && ok_to_send_work) {
            if (config.min_sendwork_interval) {
                double diff = dtime() - last_rpc_time;
                if (diff < config.min_sendwork_interval) {
                    ok_to_send_work = false;
                    log_messages.printf(MSG_NORMAL,
                        "Not sending work - last request too recent: %f\n", diff
                    );
                    sprintf(buf,
                        "Not sending work - last request too recent: %d sec", (int)diff
                    );
                    g_reply->insert_message(buf, "low");

                    // the 1.01 is in case client's clock
                    // is slightly faster than ours
                    //
                    g_reply->set_delay(1.01*config.min_sendwork_interval);
                }
            }
            if (ok_to_send_work) {
                send_work();
            }
        }
        if (g_wreq->no_jobs_available) {
            g_reply->insert_message("Project has no tasks available", "low");
        }
    }


    handle_msgs_from_host();
    if (config.msg_to_host) {
        handle_msgs_to_host();
    }

    update_host_record(initial_host, g_reply->host, g_reply->user);
    write_host_app_versions();

leave:
    if (!have_no_work) {
        ssp->restore_work(g_pid);
    }
}

static void log_incomplete_request() {
    // BOINC scheduler requests use method POST.
    // So method GET means that someone is trying a browser.
    //
    char *rm=getenv("REQUEST_METHOD");
    bool used_get = false;
    if (rm && !strcmp(rm, "GET")) {
        used_get = true;
    }
    log_messages.printf(MSG_NORMAL,
        "Incomplete request received %sfrom IP %s, auth %s, platform %s, version %d.%d.%d\n",
        used_get?"(used GET method - probably a browser) ":"",
        get_remote_addr(), g_request->authenticator, g_request->platform.name,
        g_request->core_client_major_version, g_request->core_client_minor_version,
        g_request->core_client_release
    );
}

static void log_user_messages() {
    for (unsigned int i=0; i<g_reply->messages.size(); i++) {
        USER_MESSAGE um = g_reply->messages[i];
        log_messages.printf(MSG_NORMAL,
            "[user_messages] [HOST#%d] MSG(%s) %s\n",
            g_reply->host.id, um.priority.c_str(), um.message.c_str()
        );
    }
}

void handle_request(FILE* fin, FILE* fout, char* code_sign_key) {
    SCHEDULER_REQUEST sreq;
    SCHEDULER_REPLY sreply;
    char buf[1024];

    g_request = &sreq;
    g_reply = &sreply;
    g_wreq = &sreply.wreq;

    sreply.nucleus_only = true;

    log_messages.set_indent_level(1);

    MIOFILE mf;
    XML_PARSER xp(&mf);
    mf.init_file(fin);
    const char* p = sreq.parse(xp);
    double start_time = dtime();
    if (!p){
        process_request(code_sign_key);
    } else {
        sprintf(buf, "Error in request message: %s", p);
        log_incomplete_request();
        sreply.insert_message(buf, "low");
    }

    if ((config.locality_scheduling || config.locality_scheduler_fraction) && !sreply.nucleus_only) {
        send_file_deletes();
    }

    if (config.debug_user_messages) {
        log_user_messages();
    }

    sreply.write(fout, sreq);
    log_messages.printf(MSG_NORMAL,
        "Scheduler ran %.3f seconds\n", dtime()-start_time
    );

    if (strlen(config.sched_lockfile_dir)) {
        unlock_sched();
    }
}

const char *BOINC_RCSID_2ac231f9de = "$Id$";
