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

#include "main.h"
#include "server_types.h"
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

#ifdef EINSTEIN_AT_HOME
    // This is to prevent GRID hosts that manipulate their hostids from flooding E@H's DB with slow queries
    if ((user.id == 282952) || (user.id == 243543))
      return false;
#endif

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
static void get_weak_auth(USER& user, char* buf) {
    char buf2[256], out[256];
    sprintf(buf2, "%s%s", user.authenticator, user.passwd_hash);
    md5_block((unsigned char*)buf2, strlen(buf2), out);
    sprintf(buf, "%d_%s", user.id, out);
}

static void send_error_message(const char* msg, int delay) {
    g_reply->insert_message(USER_MESSAGE(msg, "low"));
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

    sprintf(filename, "%s/CGI_%07d", config.sched_lockfile_dir, g_reply->host.id);

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
    write(fd, pid_string, count);
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
            g_reply->insert_message(
                USER_MESSAGE("Can't find host record", "low")
            );
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
        sprintf(buf, "where id=%d", host.userid);
        retval = user.lookup(buf);
        if (!retval && !strcmp(user.authenticator, g_request->authenticator)) {
            // req auth matches user auth - go on
        } else {
            bool weak_auth = false;
            if (!retval) {
                // user for host.userid exists - check weak auth
                //
                get_weak_auth(user, buf);
                if (!strcmp(buf, g_request->authenticator)) {
                    weak_auth = true;
                    log_messages.printf(MSG_DEBUG,
                        "[HOST#%d] accepting weak authenticator\n",
                        host.id
                    );
                }
            }
            if (!weak_auth) {
                // weak auth failed - look up user based on authenticator
                //
                strlcpy(
                    user.authenticator, g_request->authenticator, sizeof(user.authenticator)
                );
                sprintf(buf, "where authenticator='%s'", user.authenticator);
                retval = user.lookup(buf);
                if (retval) {
                    g_reply->insert_message(
                        USER_MESSAGE("Invalid or missing account key.  "
                            "Detach and reattach to this project to fix this.",
                            "high"
                        )
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
                USER_MESSAGE(
                    "Invalid or missing account key.  "
                    "Detach and reattach to this project to fix this.",
                    "low"
                )
            );
            g_reply->set_delay(DELAY_MISSING_KEY);
            log_messages.printf(MSG_CRITICAL,
                "[HOST#<none>] Bad authenticator '%s': %d\n",
                g_request->authenticator, retval
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
                if (!config.multiple_clients_per_host
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
        // NOTE: if the project allows multiple clients per host
        // (e.g. those that run on grids), skip this.
        //
        if (!config.multiple_clients_per_host && find_host_by_other(user, g_request->host, host)) {
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
        host.error_rate = 0.1;
        strcpy(host.venue, g_reply->user.venue);
        host.fix_nans();
        retval = host.insert();
        if (retval) {
            g_reply->insert_message(
                USER_MESSAGE("Couldn't create host record in database", "low")
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
    if (strlen(g_request->cross_project_id)) {
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

// Modify claimed credit based on the historical granted credit if
// the project is configured to do this
//
static void modify_credit_rating(HOST& host) {
    double new_claimed_credit = 0;
    double percent_difference = 0;
    // The percent difference between claim and history
    double difference_weight = 1;
    // The weight to be applied based on the difference between claim and 
    // history
    double credit_weight = 1;
    // The weight to be applied based on how much credit the host has earned
    // (hosts that are new do not have accurate histories so they shouldn't
    // have much weight)
    double combined_weight = 1;

    // Only modify if the credit_per_cpu_sec is established
    // and the option is enabled
    if ( host.credit_per_cpu_sec > 0 
        && config.granted_credit_weight > 0.0 
        && config.granted_credit_weight <= 1.0 ) {
    
        // Calculate the difference between claimed credit and the hosts
        // historical granted credit history
        percent_difference=host.claimed_credit_per_cpu_sec-host.credit_per_cpu_sec;
        percent_difference = fabs(percent_difference/host.credit_per_cpu_sec);

        // A study on World Community Grid determined that 50% of hosts
        // claimed within 10% of their historical credit per cpu sec.  
        // These hosts should not have their credit modified.
        if ( percent_difference < 0.1000 ) {
     		log_messages.printf(MSG_DEBUG, "[HOSTID:%d] Claimed credit %.1lf not "
     		    "modified.  Percent Difference %.4lf\n", host.id, 
     		    host.claimed_credit_per_cpu_sec*86400, percent_difference
     		);
            return;
}

        // The study also determined that 95% of hosts claim within 
        // 50% of their historical credit per cpu sec.
        // Computers claiming above 10% but below 50% should have their 
        // credit adjusted based on their history
        // Computers claiming more than 50% above should use their 
        // historical value.
        if ( percent_difference < .5 ) {
            // weight based on variance from historical credit
            difference_weight=1-(0.5-percent_difference)/0.4;
    	    } else { 
            difference_weight=1;
    	    }
    	    
        // A weight also needs to be calculated based upon the amount of
        // credit awarded to a host.  This is becuase hosts without much
        // credit awarded do not yet have an accurate history so the weight
        // should be limited for these hosts.
        if ( config.granted_credit_ramp_up ) {
    	    credit_weight=config.granted_credit_ramp_up - host.total_credit;
    	    credit_weight=credit_weight/config.granted_credit_ramp_up;
    		if ( credit_weight < 0) credit_weight = 0;
    	   	credit_weight = 1 - credit_weight;
        }
    	    
        // Compute the combined weight
        combined_weight=credit_weight*difference_weight*config.granted_credit_weight;    
     	log_messages.printf(MSG_DEBUG, "[HOSTID:%d] Weight details: "
     	     "diff_weight=%.4lf credit_weight=%.4lf config_weight=%.4lf\n",
     	     host.id, difference_weight, credit_weight, 
     	     config.granted_credit_weight
     	);
            
        // Compute the new value for claimed credit
        new_claimed_credit=(1-combined_weight)*host.claimed_credit_per_cpu_sec;
        new_claimed_credit=new_claimed_credit+combined_weight*host.credit_per_cpu_sec;
        
        if ( new_claimed_credit < host.claimed_credit_per_cpu_sec ) {
     	    log_messages.printf(MSG_DEBUG, "[HOSTID:%d] Modified claimed credit "
     	        "(lowered) original: %.1lf new: %.1lf historical: %.1lf "
     	        "combined weight: %.4lf\n", host.id, 
     	        host.claimed_credit_per_cpu_sec*86400, 
     	        new_claimed_credit*86400, host.credit_per_cpu_sec*86400, 
     	        combined_weight
     	    );
        } else {
     	    log_messages.printf(MSG_DEBUG, "[HOSTID:%d] Modified claimed credit "
     	        "(increased) original: %.1lf new: %.1lf historical: %.1lf " 
     	        "combined weight: %.4lf\n", host.id, 
     	        host.claimed_credit_per_cpu_sec*86400, 
     	        new_claimed_credit*86400, host.credit_per_cpu_sec*86400, 
     	        combined_weight
     	    );
    	    }
    	    
        host.claimed_credit_per_cpu_sec=new_claimed_credit;
        }
    }

// somewhat arbitrary formula for credit as a function of CPU time.
// Could also include terms for RAM size, network speed etc.
//
static void compute_credit_rating(HOST& host) {
    double fpw, intw, scale, x;
    if (config.use_benchmark_weights) {

        fpw = config.fp_benchmark_weight;
        intw = 1. - fpw;

        // FP benchmark is 2x int benchmark, on average.
        // Compute a scaling factor the gives the same credit per day
        // no matter how benchmarks are weighted
        //
        scale = 1.5 / (2*intw + fpw);
    } else {
        fpw = .5;
        intw = .5;
        scale = 1;
    }
    x = fpw*fabs(host.p_fpops) + intw*fabs(host.p_iops);
    x /= 1e9;
    x *= COBBLESTONE_FACTOR;
    x /= SECONDS_PER_DAY;
    x *= scale;
    host.claimed_credit_per_cpu_sec  = x;
    
    if (config.granted_credit_weight) {
        modify_credit_rating(host);
    }
}

// modify host struct based on request.
// Copy all fields that are determined by the client.
//
static int modify_host_struct(HOST& host) {
    host.timezone = g_request->host.timezone;
    strncpy(host.domain_name, g_request->host.domain_name, sizeof(host.domain_name));
    g_request->coprocs.summary_string(host.serialnum, sizeof(host.serialnum));
    if (strcmp(host.last_ip_addr, g_request->host.last_ip_addr)) {
        strncpy(host.last_ip_addr, g_request->host.last_ip_addr, sizeof(host.last_ip_addr));
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

    compute_credit_rating(host);
    g_request->host.claimed_credit_per_cpu_sec = host.claimed_credit_per_cpu_sec;
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
    retval = host.update_diff(initial_host);
    if (retval) {
        log_messages.printf(MSG_CRITICAL, "host.update() failed: %d\n", retval);
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
        orp.abort_if_not_started = false;
        orp.reason = ABORT_REASON_NOT_FOUND;
        if (i > 0) result_names.append(", ");
        result_names.append("'");
        result_names.append(orp.name);
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
                } else if ( result.assimilate_state == ASSIMILATE_DONE ) {
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
            g_reply->insert_message(USER_MESSAGE(buf, "high"));
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
    bool have_db_prefs = (strlen(g_reply->user.global_prefs)>0);
    bool same_account = !strcmp(
        g_request->global_prefs_source_email_hash, g_reply->email_hash
    );
    double master_mod_time=0, db_mod_time=0;
    if (have_master_prefs) {
        parse_double(g_request->global_prefs_xml, "<mod_time>", master_mod_time);
        if (master_mod_time > dtime()) master_mod_time = dtime();
    }
    if (have_db_prefs) {
        parse_double(g_reply->user.global_prefs, "<mod_time>", db_mod_time);
        if (db_mod_time > dtime()) db_mod_time = dtime();
    }

    if (config.debug_prefs) {
        log_messages.printf(MSG_DEBUG,
            "have_master:%d have_working: %d have_db: %d\n",
            have_master_prefs, have_working_prefs, have_db_prefs
        );
    }

    // decide which prefs to use for sched decisions,
    // and parse them into g_request->global_prefs
    //
    if (have_working_prefs) {
        g_request->global_prefs.parse(g_request->working_global_prefs_xml, "");
        if (config.debug_prefs) {
            log_messages.printf(MSG_DEBUG, "using working prefs\n");
        }
    } else {
        if (have_master_prefs) {
            if (have_db_prefs && db_mod_time > master_mod_time) {
                g_request->global_prefs.parse(g_reply->user.global_prefs, g_reply->host.venue);
                if (config.debug_prefs) {
                    log_messages.printf(MSG_DEBUG, "using db prefs - more recent\n");
                }
            } else {
                g_request->global_prefs.parse(g_request->global_prefs_xml, g_reply->host.venue);
                if (config.debug_prefs) {
                    log_messages.printf(MSG_DEBUG, "using master prefs\n");
                }
            }
        } else {
            if (have_db_prefs) {
                g_request->global_prefs.parse(g_reply->user.global_prefs, g_reply->host.venue);
                if (config.debug_prefs) {
                    log_messages.printf(MSG_DEBUG, "using db prefs\n");
                }
            } else {
                g_request->global_prefs.defaults();
                if (config.debug_prefs) {
                    log_messages.printf(MSG_DEBUG, "using default prefs\n");
                }
            }
        }
    }

    // decide whether to update DB
    //
    if (have_master_prefs) {
        bool update_user_record = false;
        if (have_db_prefs) {
            if (master_mod_time > db_mod_time && same_account) {
                update_user_record = true;
            }
        } else {
            if (same_account) update_user_record = true;
        }
        if (update_user_record) {
            log_messages.printf(MSG_DEBUG, "updating db prefs\n");
            strcpy(g_reply->user.global_prefs, g_request->global_prefs_xml);
            DB_USER user;
            user.id = g_reply->user.id;
            escape_string(g_request->global_prefs_xml, sizeof(g_request->global_prefs_xml));
            sprintf(buf, "global_prefs='%s'", g_request->global_prefs_xml);
            unescape_string(g_request->global_prefs_xml, sizeof(g_request->global_prefs_xml));
            int retval = user.update_field(buf);
            if (retval) {
                log_messages.printf(MSG_CRITICAL,
                    "user.update_field() failed: %d\n", retval
                );
            }
        }
    }

    // decide whether to send DB prefs in reply msg
    //
    if (config.debug_prefs) {
        log_messages.printf(MSG_DEBUG,
            "have db %d; dbmod %f; global mod %f\n",
            have_db_prefs, db_mod_time, g_request->global_prefs.mod_time
        );
    }
    if (have_db_prefs && db_mod_time > master_mod_time) {
        if (config.debug_prefs) {
            log_messages.printf(MSG_DEBUG, "sending db prefs in reply\n");
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
                        USER_MESSAGE(
                           "You may have an outdated code verification key.  "
                           "This may prevent you from accepting new executables.  "
                           "If the problem persists, detach/attach the project. ",
                           "high"
                        )
                    );
                    return false;
                }
                if (!strcmp(oldkey, g_request->code_sign_key)) {
                    sprintf(path, "%s/signature_%d", config.key_dir, i);
                    retval = read_file_malloc(path, signature);
                    if (retval) {
                        g_reply->insert_message(
                            USER_MESSAGE(
                               "You may have an outdated code verification key.  "
                               "This may prevent you from accepting new executables.  "
                               "If the problem persists, detach/attach the project. ",
                               "high"
                            )
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

// This routine examines the <min_core_client_version_announced> value
// from config.xml.  If set, and the core client version is less than
// this version, send a warning to users to upgrade before deadline
// given in <min_core_client_upgrade_deadline> in Unix time(2) format
// expires.
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
                "Starting in %d days and %d hours, project will require a minimum "
                "BOINC core client version of %d.%d.0.  You are currently using "
                "version %d.%d.%d; please upgrade before this time.",
                days, hours,
                config.min_core_client_version_announced / 100,
                config.min_core_client_version_announced % 100,
                g_request->core_client_major_version,
                g_request->core_client_minor_version,
                g_request->core_client_release
            );
            // make this low priority until three days are left.  Then
            // bump to high.
            //
            if (days<3) {
                g_reply->insert_message(USER_MESSAGE(msg, "high"));
            } else {
                g_reply->insert_message(USER_MESSAGE(msg, "low"));
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
            sprintf(buf, "This project doesn't support OS type %s %s",
                g_request->host.os_name, g_request->host.os_version
            );
            g_reply->insert_message(USER_MESSAGE(buf, "low"));
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
            sprintf(buf, "This project doesn't support CPU type %s %s",
                g_request->host.p_vendor, g_request->host.p_model
            );
            g_reply->insert_message(USER_MESSAGE(buf, "low"));
            g_reply->set_delay(DELAY_UNACCEPTABLE_OS);
            return true;
        }
    }
    return false;
}

bool wrong_core_client_version() {
    char msg[256];
    bool wrong_version = false;
    if (config.min_core_client_version) {
        int major = config.min_core_client_version/100;
        int minor = config.min_core_client_version % 100;
        if (g_request->core_client_major_version < major ||
            ((g_request->core_client_major_version == major) && (g_request->core_client_minor_version < minor))) {
            wrong_version = true;
            sprintf(msg,
                "Need version %d.%d or higher of the BOINC core client. You have %d.%d.",
                major, minor,
                g_request->core_client_major_version, g_request->core_client_minor_version
            );
            log_messages.printf(MSG_NORMAL,
                "[HOST#%d] [auth %s] Wrong minor version from user: wanted %d, got %d\n",
                g_request->hostid, g_request->authenticator,
                minor, g_request->core_client_minor_version
            );
        }
    }
    if (wrong_version) {
        g_reply->insert_message(USER_MESSAGE(msg, "low"));
        g_reply->set_delay(DELAY_BAD_CLIENT_VERSION);
        return true;
    }
    return false;
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
                "[HOST#%d] message insert failed: %d\n",
                g_reply->host.id, retval
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
                    USER_MESSAGE(
                        "Unable to send work to Vista with BOINC installed in protected mode",
                        "high"
                    )
                );
                g_reply->insert_message(
                    USER_MESSAGE(
                        "Please reinstall BOINC and uncheck 'Protected application execution'",
                        "high"
                    )
                );
            }
        }
    }
    return false;
}

static inline bool requesting_work() {
    if (g_request->work_req_seconds > 0) return true;
    if (g_request->cpu_req_secs > 0) return true;
    if (g_request->coproc_cuda && g_request->coproc_cuda->req_secs) return true;
    return false;
}

void process_request(char* code_sign_key) {
    PLATFORM* platform;
    int retval;
    double last_rpc_time;
    struct tm *rpc_time_tm;
    int last_rpc_dayofyear;
    int current_rpc_dayofyear;
    bool ok_to_send_work = true;
    bool have_no_work;
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

        // if no results, return without accessing DB
        //
        if (g_request->results.size() == 0) {
            return;
        }
    } else {
        warn_user_if_core_client_upgrade_scheduled();
    }

    if (requesting_work()) {
        if (config.locality_scheduling || config.locality_scheduler_fraction || config.enable_assignment) {
            have_no_work = false;
        } else {
            lock_sema();
            have_no_work = ssp->no_work(g_pid);
            if (have_no_work) {
                g_wreq->no_jobs_available = true;
                log_messages.printf(MSG_NORMAL, "No jobs in shmem\n");
            }
            unlock_sema();
        }
    }

    // if there's no work, and client isn't returning results,
    // this isn't an initial RPC,
    // and client is requesting work, return without accessing DB
    //
    if (
        config.nowork_skip
        && requesting_work()
        && have_no_work
        && (g_request->results.size() == 0)
        && (g_request->hostid != 0)
    ) {
        g_reply->insert_message(USER_MESSAGE("No work available", "low"));
        g_reply->set_delay(DELAY_NO_WORK_SKIP);
        if (!config.msg_to_host) {
            log_messages.printf(MSG_NORMAL, "No work - skipping DB access\n");
            return;
        }
    }

    // FROM HERE ON DON'T RETURN; goto leave instead
    // because we've tagged an entry in the work array with our process ID

    // now open the database
    //
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
    if (g_reply->host.max_results_day == -1) {
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

    last_rpc_time = g_reply->host.rpc_time;
    t = g_reply->host.rpc_time;
    rpc_time_tm = localtime(&t);
    last_rpc_dayofyear = rpc_time_tm->tm_yday;

    t = time(0);
    g_reply->host.rpc_time = t;
    rpc_time_tm = localtime(&t);
    current_rpc_dayofyear = rpc_time_tm->tm_yday;

    if (config.daily_result_quota) {
        if (g_reply->host.max_results_day == 0 || g_reply->host.max_results_day > config.daily_result_quota) {
            g_reply->host.max_results_day = config.daily_result_quota;
            log_messages.printf(MSG_DEBUG,
                "[HOST#%d] Initializing max_results_day to %d\n",
                g_reply->host.id, config.daily_result_quota
            );
        }
    }

    if (last_rpc_dayofyear != current_rpc_dayofyear) {
        log_messages.printf(MSG_DEBUG,
            "[HOST#%d] Resetting nresults_today\n", g_reply->host.id
        );
        g_reply->host.nresults_today = 0;
    }
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
        sprintf(buf, "platform '%s' not found", g_request->platform.name);
        g_reply->insert_message(USER_MESSAGE(buf, "low"));
        log_messages.printf(MSG_CRITICAL,
            "[HOST#%d] platform '%s' not found\n",
            g_reply->host.id, g_request->platform.name
        );
        g_reply->set_delay(DELAY_PLATFORM_UNSUPPORTED);
        goto leave;
    }

    handle_global_prefs();

    handle_results();

    // Do this before resending lost jobs
    //
    if (bad_install_type()) {
        ok_to_send_work = false;
    }
    send_work_setup();

    g_wreq->njobs_on_host = g_request->other_results.size();
    for (i=0; i<g_request->other_results.size(); i++) {
        OTHER_RESULT& r = g_request->other_results[i];
        if (r.have_plan_class) {
            if (app_plan_uses_gpu(r.plan_class)) {
                g_wreq->njobs_on_host_gpu++;
            } else {
                g_wreq->njobs_on_host_cpu++;
            }
        }
    }
    if (g_request->have_other_results_list) {
        if (config.resend_lost_results && ok_to_send_work) {
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
                    g_reply->insert_message(USER_MESSAGE(buf, "low"));

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
            g_reply->insert_message(
                USER_MESSAGE(
                    "(Project has no jobs available)",
                    "high"
                )
            );
        }
    }


    handle_msgs_from_host();
    if (config.msg_to_host) {
        handle_msgs_to_host();
    }

    update_host_record(initial_host, g_reply->host, g_reply->user);

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
        log_messages.printf(MSG_DEBUG,
            "[HOST#%d] MSG(%4s) %s\n",
            g_reply->host.id, um.priority.c_str(), um.message.c_str()
        );
    }
}

void handle_request(FILE* fin, FILE* fout, char* code_sign_key) {
    SCHEDULER_REQUEST sreq;
    SCHEDULER_REPLY sreply;
    char buf[1024];
    double start_time = dtime();

    g_request = &sreq;
    g_reply = &sreply;
    g_wreq = &sreply.wreq;

    memset(&sreq, 0, sizeof(sreq));
    sreply.nucleus_only = true;

    log_messages.set_indent_level(1);

    const char* p = sreq.parse(fin);
    if (!p){
        process_request(code_sign_key);
    } else {
        sprintf(buf, "Error in request message: %s", p);
        log_incomplete_request();
        sreply.insert_message(USER_MESSAGE(buf, "low"));
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
