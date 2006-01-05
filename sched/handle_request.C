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

// Handle a scheduling server RPC

#include "config.h"
#include <cassert>
#include <cstdio>
#include <vector>
#include <string>
#include <ctime>
#include <cmath>
using namespace std;

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

#ifdef _USING_FCGI_
#include "fcgi_stdio.h"
#endif

// Try to lock a file with name based on host ID,
// to prevent 2 schedulers from running at same time for same host.
// Return:
// 0 if successful
//    In this case store file descriptor in reply struct so we can unlock later
//    In other cases store -1 in reply struct
// PID (>0) if another process has lock
// -1 if error (e.g. can't create file)
//
int lock_sched(SCHEDULER_REPLY& reply) {
    char filename[256];
    char pid_string[16];
    int fd, pid, count;

    reply.lockfile_fd=-1;

    sprintf(filename, "%s/CGI_%07d", config.sched_lockfile_dir, reply.host.id);

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

    reply.lockfile_fd = fd;
    return 0;
}

// unlock and delete per-host lockfile
//
void unlock_sched(SCHEDULER_REPLY& reply) {
    char filename[256];

    if (reply.lockfile_fd < 0) return;
    sprintf(filename, "%s/CGI_%07d", config.sched_lockfile_dir, reply.host.id);
    unlink(filename);
    close(reply.lockfile_fd);
}

// Based on the info in the request message,
// look up the host and its user, and make sure the authenticator matches.
// Some special cases:
//  1) If no host ID is supplied, or if RPC seqno mismatch,
//     create a new host record
//  2) If the host record specified by sreq.hostid is a "zombie"
//     (i.e. it was merged with another host via the web site)
//     then follow links to find the proper host
//
// POSTCONDITION:
// If this returns zero, then:
// - reply.host contains a valid host record (possibly new)
// - reply.user contains a valid user record
// - if user belongs to a team, reply.team contains team record
//
int authenticate_user(SCHEDULER_REQUEST& sreq, SCHEDULER_REPLY& reply) {
    int retval;
    char buf[256];
    DB_HOST host;
    DB_USER user;
    DB_TEAM team;

    if (sreq.hostid) {
        retval = host.lookup_id(sreq.hostid);
        while (!retval && host.userid==0) {
            // if host record is zombie, follow link to new host
            //
            retval = host.lookup_id(host.rpc_seqno);
            if (!retval) {
                reply.hostid = host.id;
                log_messages.printf(
                    SCHED_MSG_LOG::MSG_NORMAL,
                    "[HOST#%d] forwarding to new host ID %d\n",
                    sreq.hostid, host.id
                );
            }
        }
        if (retval) {
            USER_MESSAGE um("Can't find host record", "low");
            reply.insert_message(um);
            log_messages.printf(
                SCHED_MSG_LOG::MSG_NORMAL,
                "[HOST#%d?] can't find host\n",
                sreq.hostid
            );
            sreq.hostid = 0;
            goto lookup_user_and_make_new_host;
        }

        reply.host = host;
        log_messages.printf(
            SCHED_MSG_LOG::MSG_DEBUG,
            "Request [HOST#%d] Database [HOST#%d] Request [RPC#%d] Database [RPC#%d]\n",
            sreq.hostid, host.id, sreq.rpc_seqno, host.rpc_seqno
        );

        strlcpy(
            user.authenticator, sreq.authenticator,
            sizeof(user.authenticator)
        );
        sprintf(buf, "where authenticator='%s'", user.authenticator);
        retval = user.lookup(buf);
        if (retval) {
            USER_MESSAGE um("Invalid or missing account key.  "
                "Visit this project's web site to get an account key.",
                "high"
            );
            reply.insert_message(um);
            reply.set_delay(3600);
            reply.nucleus_only = true;
            log_messages.printf(
                SCHED_MSG_LOG::MSG_CRITICAL,
                "[HOST#%d] [USER#%d] Bad authenticator '%s'\n",
                host.id, user.id, sreq.authenticator
            );
            return ERR_AUTHENTICATOR;
        }

        reply.user = user;

        if (host.userid != user.id) {
            // If the request's host ID isn't consistent with the authenticator,
            // create a new host record.
            //
            log_messages.printf(
                SCHED_MSG_LOG::MSG_NORMAL,
                "[HOST#%d] [USER#%d] inconsistent host ID; creating new host\n",
                host.id, user.id
            );
            goto make_new_host;
        }


        // If the seqno from the host is less than what we expect,
        // the user must have copied the state file to a different host.
        // Make a new host record.
        //
        if (sreq.rpc_seqno < reply.host.rpc_seqno) {
            sreq.hostid = 0;
            log_messages.printf(
                SCHED_MSG_LOG::MSG_NORMAL,
                "[HOST#%d] [USER#%d] RPC seqno %d less than expected %d; creating new host\n",
                reply.host.id, user.id, sreq.rpc_seqno, reply.host.rpc_seqno
            );
            goto make_new_host;
        }
    } else {

        // here no hostid was given; we'll have to create a new host record
        //
lookup_user_and_make_new_host:
        strlcpy(
            user.authenticator, sreq.authenticator,
            sizeof(user.authenticator)
        );
        sprintf(buf, "where authenticator='%s'", user.authenticator);
        retval = user.lookup(buf);
        if (retval) {
            USER_MESSAGE um(
                "Invalid or missing account key.  "
                "Visit this project's web site to get an account key.",
                "low"
            );
            reply.insert_message(um);
            reply.set_delay(3600);
            log_messages.printf(
                SCHED_MSG_LOG::MSG_CRITICAL,
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
            USER_MESSAGE um("Couldn't create host record in database", "low");
            reply.insert_message(um);
            boinc_db.print_error("host.insert()");
            log_messages.printf(SCHED_MSG_LOG::MSG_CRITICAL, "host.insert() failed\n");
            return retval;
        }
        host.id = boinc_db.insert_id();

        reply.host = host;
        reply.hostid = reply.host.id;
        // this tells client to updates its host ID
    }

    // have user record in reply.user at this point
    //

    if (reply.user.teamid) {
        retval = team.lookup_id(reply.user.teamid);
        if (!retval) reply.team = team;
    }

    // compute email hash
    //
    md5_block(
        (unsigned char*)reply.user.email_addr,
        strlen(reply.user.email_addr),
        reply.email_hash
    );

    // see if new cross-project ID
    //
    if (strlen(sreq.cross_project_id)) {
        if (strcmp(sreq.cross_project_id, reply.user.cross_project_id)) {
            user.id = reply.user.id;
            escape_string(sreq.cross_project_id, sizeof(sreq.cross_project_id));
            sprintf(buf, "cross_project_id='%s'", sreq.cross_project_id);
            unescape_string(sreq.cross_project_id, sizeof(sreq.cross_project_id));
            user.update_field(buf);
        }
    }
    return 0;
}

#define COBBLESTONE_FACTOR 100.0

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
    host.credit_per_cpu_sec  = x;
}

static double fpops_to_credit(double fpops, double intops) {
    double fpc = (fpops/1e9)*COBBLESTONE_FACTOR/SECONDS_PER_DAY;
    double intc = (intops/1e9)*COBBLESTONE_FACTOR/SECONDS_PER_DAY;
    return std::max(fpc, intc);
}

// modify host struct based on request.
// Copy all fields that are determined by the client.
//
static int modify_host_struct(SCHEDULER_REQUEST& sreq, HOST& host) {
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
    host.cpu_efficiency = sreq.host.cpu_efficiency;
    host.duration_correction_factor = sreq.host.duration_correction_factor;
    host.p_ncpus = sreq.host.p_ncpus;
    strncpy(host.p_vendor, sreq.host.p_vendor, sizeof(host.p_vendor));
        // unlikely this will change
    strncpy(host.p_model, sreq.host.p_model, sizeof(host.p_model));
    host.p_fpops = sreq.host.p_fpops;
    host.p_iops = sreq.host.p_iops;
    host.p_membw = sreq.host.p_membw;
    strncpy(host.os_name, sreq.host.os_name, sizeof(host.os_name));
    strncpy(host.os_version, sreq.host.os_version, sizeof(host.os_version));
    host.m_nbytes = sreq.host.m_nbytes;
    host.m_cache = sreq.host.m_cache;
    host.m_swap = sreq.host.m_swap;
    host.d_total = sreq.host.d_total;
    host.d_free = sreq.host.d_free;
    host.d_boinc_used_total = sreq.host.d_boinc_used_total;
    host.d_boinc_used_project = sreq.host.d_boinc_used_project;
    host.n_bwup = sreq.host.n_bwup;
    host.n_bwdown = sreq.host.n_bwdown;
    if (strlen(sreq.host.host_cpid)) {
        strcpy(host.host_cpid, sreq.host.host_cpid);
    }
    host.fix_nans();

    compute_credit_rating(host);
    return 0;
}

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
        log_messages.printf(SCHED_MSG_LOG::MSG_CRITICAL, "host.update() failed: %d\n", retval);
    }
    return 0;
}

// Decide which global prefs to use,
// (from request msg, or if absent then from user record)
// and parse them into the request message global_prefs field.
// Decide whether to send global prefs in reply msg
//
int handle_global_prefs(SCHEDULER_REQUEST& sreq, SCHEDULER_REPLY& reply) {
    char buf[LARGE_BLOB_SIZE];
    reply.send_global_prefs = false;

    if (strlen(sreq.global_prefs_xml)) {
        unsigned req_mod_time=0, db_mod_time=0;
        bool same_account = !strcmp(
            sreq.global_prefs_source_email_hash, reply.email_hash
        );
        bool update_prefs = false;

        parse_int(sreq.global_prefs_xml, "<mod_time>", (int&)req_mod_time);
        if (strlen(reply.user.global_prefs)) {
            parse_int(reply.user.global_prefs, "<mod_time>", (int&)db_mod_time);

            // if user record has more recent prefs,
            // use them and arrange to return in reply msg
            //
            if (req_mod_time < db_mod_time) {
                strcpy(sreq.global_prefs_xml, reply.user.global_prefs);
                reply.send_global_prefs = true;
            } else {
                if (same_account) update_prefs = true;
            }
        } else {
            if (same_account) update_prefs = true;
        }
        if (update_prefs) {
            strcpy(reply.user.global_prefs, sreq.global_prefs_xml);
            DB_USER user;
            user.id = reply.user.id;
            escape_string(sreq.global_prefs_xml, sizeof(sreq.global_prefs_xml));
            sprintf(buf, "global_prefs='%s'", sreq.global_prefs_xml);
            unescape_string(sreq.global_prefs_xml, sizeof(sreq.global_prefs_xml));
            int retval = user.update_field(buf);
            if (retval) {
                log_messages.printf(SCHED_MSG_LOG::MSG_CRITICAL,
                    "user.update_field() failed: %d\n", retval
                );
            }
        }
    } else {
        // request message has no global prefs;
        // copy from user record, and send them in reply
        //
        if (strlen(reply.user.global_prefs)) {
            strcpy(sreq.global_prefs_xml, reply.user.global_prefs);
            reply.send_global_prefs = true;
        }
    }
    sreq.global_prefs.parse(sreq.global_prefs_xml, reply.host.venue);
    return 0;
}


// handle completed results
//
int handle_results(SCHEDULER_REQUEST& sreq, SCHEDULER_REPLY& reply) {
    DB_SCHED_RESULT_ITEM_SET result_handler;
    SCHED_RESULT_ITEM* srip;
    unsigned int i;
    int retval;
    RESULT* rp;
    
    if (sreq.results.size() == 0) return 0;
    
    // copy reported results to a separate vector, "result_handler",
    // initially with only the "name" field present
    //
    for (i=0; i<sreq.results.size(); i++) {
        result_handler.add_result(sreq.results[i].name);
    }
    
    // read results from database into "result_handler".
    // Quantities that must be read from the DB are those
    // where srip (see below) appears as an rval.
    // These are: id, name, server_state, received_time, hostid.
    // Quantities that must be written to the DB are those for
    // which srip appears as an lval. These are:
    // hostid, teamid, received_time, client_state, cpu_time, exit_status,
    // app_version_num, claimed_credit, server_state, stderr_out,
    // xml_doc_out, outcome, validate_state
    //
    retval = result_handler.enumerate();
    if (retval) {
        log_messages.printf(
            SCHED_MSG_LOG::MSG_CRITICAL,
            "[HOST#%d] Batch query failed\n",
            reply.host.id
        );
    }

    // loop over results reported by client
    //
    // A note about acks: we send an ack for result received if either
    // 1) there's some problem with it (wrong state, host, not in DB) or
    // 2) we update it successfully.
    // In other words, the only time we don't ack a result is when
    // it looks OK but the update failed.
    //
    for (i=0; i<sreq.results.size(); i++) {
        rp = &sreq.results[i];

        retval = result_handler.lookup_result(rp->name, &srip);
        if (retval) {
            log_messages.printf(
                SCHED_MSG_LOG::MSG_CRITICAL,
                "[HOST#%d] [RESULT#? %s] can't find result\n",
                reply.host.id, rp->name
            );

            reply.result_acks.push_back(std::string(rp->name));
            continue;
        }

        log_messages.printf(
            SCHED_MSG_LOG::MSG_NORMAL, "[HOST#%d] [RESULT#%d %s] got result\n",
            reply.host.id, srip->id, srip->name
        );

        // Do various sanity checks.
        // If one of them fails, set srip->id = 0,
        // which suppresses the DB update later on
        //
        if (srip->server_state == RESULT_SERVER_STATE_UNSENT) {
            log_messages.printf(
                SCHED_MSG_LOG::MSG_CRITICAL,
                "[HOST#%d] [RESULT#%d %s] got unexpected result: server state is %d\n",
                reply.host.id, srip->id, srip->name, srip->server_state
            );
            srip->id=0; // mark to skip when updating DB
            reply.result_acks.push_back(std::string(rp->name));
            continue;
        }

        if (srip->received_time) {
            log_messages.printf(
                SCHED_MSG_LOG::MSG_CRITICAL,
                "[HOST#%d] [RESULT#%d %s] got result twice\n",
                reply.host.id, srip->id, srip->name
            );
            srip->id=0;  // mark to skip when updating DB
            reply.result_acks.push_back(std::string(rp->name));
            continue;
        }

        if (srip->hostid != reply.host.id) {
            log_messages.printf(
                SCHED_MSG_LOG::MSG_CRITICAL,
                "[HOST#%d] [RESULT#%d %s] got result from wrong host; expected [HOST#%d]\n",
                reply.host.id, srip->id, srip->name, srip->hostid
            );
            DB_HOST result_host;
            retval = result_host.lookup_id(srip->hostid);

            if (retval) {
                log_messages.printf(
                    SCHED_MSG_LOG::MSG_CRITICAL,
                    "[RESULT#%d %s] Can't lookup [HOST#%d]\n",
                    srip->id, srip->name, srip->hostid
                );
                srip->id=0; // mark to skip when updating DB
                reply.result_acks.push_back(std::string(rp->name));
                continue;
            } else if (result_host.userid != reply.host.userid) {
                log_messages.printf(
                    SCHED_MSG_LOG::MSG_CRITICAL,
                    "[USER#%d] [HOST#%d] [RESULT#%d %s] Not even the same user; expected [USER#%d]\n",
                    reply.host.userid, reply.host.id, srip->id, srip->name, result_host.userid
                );
                srip->id=0; // mark to skip when updating DB
                reply.result_acks.push_back(std::string(rp->name));
                continue;
            } else {
                log_messages.printf(
                    SCHED_MSG_LOG::MSG_CRITICAL,
                    "[HOST#%d] [RESULT#%d %s] Allowing result because same USER#%d\n",
                    reply.host.id, srip->id, srip->name, reply.host.userid
                );
            }
        } // hostids do not match

        // Modify the in-memory copy obtained from the DB earlier.
        // If we found a problem above,
        // we have continued and skipped this modify
        //
        srip->hostid = reply.host.id;
        srip->teamid = reply.user.teamid;
        srip->received_time = time(0);
        srip->client_state = rp->client_state;
        srip->cpu_time = rp->cpu_time;

        // check for impossible CPU time
        //
        double elapsed_time = srip->received_time - srip->sent_time;
        if (elapsed_time < 0) {
            log_messages.printf(SCHED_MSG_LOG::MSG_NORMAL,
                "[RESULT#%d] inconsistent sent/received times\n", srip->id
            );
        } else {
            if (srip->cpu_time > elapsed_time) {
                log_messages.printf(SCHED_MSG_LOG::MSG_NORMAL,
                    "[RESULT#%d] excessive CPU time reported: %f\n",
                    srip->id, srip->cpu_time
                );
                srip->cpu_time = elapsed_time;
            }
        }

        srip->exit_status = rp->exit_status;
        srip->app_version_num = rp->app_version_num;
        if (rp->fpops_cumulative || rp->intops_cumulative) {
            srip->claimed_credit = fpops_to_credit(rp->fpops_cumulative, rp->intops_cumulative);
        } else if (rp->fpops_per_cpu_sec || rp->intops_per_cpu_sec) {
            srip->claimed_credit = fpops_to_credit(
                rp->fpops_per_cpu_sec*srip->cpu_time,
                rp->intops_per_cpu_sec*srip->cpu_time
            );
        } else {
            srip->claimed_credit = srip->cpu_time * reply.host.credit_per_cpu_sec;
        }
#ifdef EINSTEIN_AT_HOME
        log_messages.printf(SCHED_MSG_LOG::MSG_DEBUG,
            "cpu %f cpcs %f, cc %f\n", srip->cpu_time, reply.host.credit_per_cpu_sec, srip->claimed_credit
        );
#endif
        srip->server_state = RESULT_SERVER_STATE_OVER;

        strlcpy(srip->stderr_out, rp->stderr_out, sizeof(srip->stderr_out));
        strlcpy(srip->xml_doc_out, rp->xml_doc_out, sizeof(srip->xml_doc_out));

        // look for exit status and app version in stderr_out
        // (historical - can be deleted at some point)
        //
        parse_int(srip->stderr_out, "<exit_status>", srip->exit_status);
        parse_int(srip->stderr_out, "<app_version>", srip->app_version_num);

        if ((srip->client_state == RESULT_FILES_UPLOADED) && (srip->exit_status == 0)) {
            srip->outcome = RESULT_OUTCOME_SUCCESS;
            log_messages.printf(SCHED_MSG_LOG::MSG_DEBUG,
                "[RESULT#%d %s]: setting outcome SUCCESS\n",
                srip->id, srip->name
            );
            reply.got_good_result();
        } else {
            log_messages.printf(SCHED_MSG_LOG::MSG_DEBUG,
                "[RESULT#%d %s]: client_state %d exit_status %d; setting outcome ERROR\n",
                srip->id, srip->name, srip->client_state, srip->exit_status
            );
            srip->outcome = RESULT_OUTCOME_CLIENT_ERROR;
            srip->validate_state = VALIDATE_STATE_INVALID;
            reply.got_bad_result();
        }
    } // loop over all incoming results


#if 0
    if (config.use_transactions) {
        retval = boinc_db.start_transaction();
        if (retval) {
            log_messages.printf(SCHED_MSG_LOG::MSG_CRITICAL,
                "[HOST#%d] result_handler.start_transaction() == %d\n",
                reply.host.id, retval
            );
        }
    }
#endif

    // Update the result records
    // (skip items that we previously marked to skip)
    //
    for (i=0; i<result_handler.results.size(); i++) {
        SCHED_RESULT_ITEM& sri = result_handler.results[i];
        if (sri.id == 0) continue;
        retval = result_handler.update_result(sri);
        if (retval) {
            log_messages.printf(
                SCHED_MSG_LOG::MSG_CRITICAL,
                "[HOST#%d] [RESULT#%d %s] can't update result: %s\n",
                reply.host.id, sri.id, sri.name, boinc_db.error_string()
            );
        } else {
            reply.result_acks.push_back(std::string(sri.name));
        }
    }

    // set transition_time for the results' WUs
    //
    retval = result_handler.update_workunits();
    if (retval) {
        log_messages.printf(
            SCHED_MSG_LOG::MSG_CRITICAL,
            "[HOST#%d] can't update WUs: %d\n",
            reply.host.id, retval
        );
    }

#if 0
    if (config.use_transactions) {
        retval = boinc_db.commit_transaction();
        if (retval) {
            log_messages.printf(
                SCHED_MSG_LOG::MSG_CRITICAL,
                "[HOST#%d] result_handler.commit_transaction() == %d\n",
                reply.host.id, retval
            );
        }
    }
#endif
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
            log_messages.printf(SCHED_MSG_LOG::MSG_NORMAL, "received old code sign key\n");

            // look for a signature file
            //
            for (i=0; ; i++) {
                sprintf(path, "%s/old_key_%d", config.key_dir, i);
                retval = read_file_malloc(path, oldkey);
                if (retval) {
                    USER_MESSAGE um(
                       "You may have an outdated code verification key.  "
                       "This may prevent you from accepting new executables.  "
                       "If the problem persists, detach/attach the project. ",
                       "high"
                    );
                    reply.insert_message(um);
                    return;
                }
                if (!strcmp(oldkey, sreq.code_sign_key)) {
                    sprintf(path, "%s/signature_%d", config.key_dir, i);
                    retval = read_file_malloc(path, signature);
                    if (retval) {
                        USER_MESSAGE um(
                           "You may have an outdated code verification key.  "
                           "This may prevent you from accepting new executables.  "
                           "If the problem persists, detach/attach the project. ",
                           "high"
                        );
                        reply.insert_message(um);
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

// This routine examines the <min_core_client_version_announced> value
// from config.xml.  If set, and the core client version is less than
// this version, send a warning to users to upgrade before deadline
// given in <min_core_client_upgrade_deadline> in Unix time(2) format
// expires.
//
void warn_user_if_core_client_upgrade_scheduled(
    SCHEDULER_REQUEST& sreq, SCHEDULER_REPLY& reply
) {

    int core_ver;
    
    core_ver  = sreq.core_client_major_version*100;
    core_ver += sreq.core_client_minor_version;
    
    if (core_ver < config.min_core_client_version_announced) {

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
                sreq.core_client_major_version,
                sreq.core_client_minor_version,
                sreq.core_client_release
            );
            // make this low priority until three days are left.  Then
            // bump to high.
            //
            if (days<3) {
                USER_MESSAGE um(msg, "high");
                reply.insert_message(um);
            } else {
                USER_MESSAGE um(msg, "low");
                reply.insert_message(um);
            }
            log_messages.printf(
                SCHED_MSG_LOG::MSG_DEBUG,
                "Sending warning: upgrade client %d.%d.%d within %d days %d hours\n",
                sreq.core_client_major_version,
                sreq.core_client_minor_version,
                sreq.core_client_release,
                days, hours
            );
        }
    }
    return;
}

#ifdef EINSTEIN_AT_HOME
bool unacceptable_os(
        SCHEDULER_REQUEST& sreq, SCHEDULER_REPLY& reply
) {
    log_messages.printf(
        SCHED_MSG_LOG::MSG_NORMAL,
        "OS version %s %s\n",
        sreq.host.os_name, sreq.host.os_version
    );

    if (!strcmp(sreq.host.os_name, "Darwin") && 
           (!strncmp(sreq.host.os_version, "5.", 2) || 
            !strncmp(sreq.host.os_version, "6.", 2)
           ) 
        ) {
        log_messages.printf(
            SCHED_MSG_LOG::MSG_NORMAL,
            "Unacceptable OS %s %s\n",
            sreq.host.os_name, sreq.host.os_version
        );
        USER_MESSAGE um("Project only supports MacOS Darwin versions 7.X and above",
                        "low");
        reply.insert_message(um);
        reply.set_delay(3600*24);
        return true;
    }
    return false;
}
#else
bool unacceptable_os(
        SCHEDULER_REQUEST& , SCHEDULER_REPLY& 
) {
    return false;
}
#endif // EINSTEIN_AT_HOME

bool wrong_core_client_version(
    SCHEDULER_REQUEST& sreq, SCHEDULER_REPLY& reply
) {
    char msg[256];
    bool wrong_version = false;
    if (config.min_core_client_version) {
        int major = config.min_core_client_version/100;
        int minor = config.min_core_client_version % 100;
        if (sreq.core_client_major_version < major ||
            ((sreq.core_client_major_version == major) && (sreq.core_client_minor_version < minor))) {
            wrong_version = true;
            sprintf(msg,
                "Need version %d.%d or higher of the BOINC core client. You have %d.%d.",
                major, minor,
                sreq.core_client_major_version, sreq.core_client_minor_version
            );
            log_messages.printf(
                SCHED_MSG_LOG::MSG_NORMAL,
                "[HOST#%d] [auth %s] Wrong minor version from user: wanted %d, got %d\n",
                sreq.hostid, sreq.authenticator,
                minor, sreq.core_client_minor_version
            );
        }
    }
    if (wrong_version) {
        USER_MESSAGE um(msg, "low");
        reply.insert_message(um);
        reply.set_delay(3600*24);
        return true;
    }
    return false;
}

inline static const char* get_remote_addr() {
    const char * r = getenv("REMOTE_ADDR");
    return r ? r : "?.?.?.?";
}

void handle_msgs_from_host(SCHEDULER_REQUEST& sreq, SCHEDULER_REPLY& reply) {
    unsigned int i;
    DB_MSG_FROM_HOST mfh;
    int retval;

    for (i=0; i<sreq.msgs_from_host.size(); i++) {
        reply.send_msg_ack = true;
        MSG_FROM_HOST_DESC& md = sreq.msgs_from_host[i];
        mfh.clear();
        mfh.create_time = time(0);
        safe_strcpy(mfh.variety, md.variety);
        mfh.hostid = reply.host.id;
        mfh.handled = false;
        safe_strcpy(mfh.xml, md.msg_text.c_str());
        log_messages.printf(
            SCHED_MSG_LOG::MSG_NORMAL,
            "got msg from host; variety %s text %s\n",
            mfh.variety, mfh.xml
        );
        retval = mfh.insert();
        if (retval) {
            log_messages.printf(SCHED_MSG_LOG::MSG_CRITICAL,
                "[HOST#%d] message insert failed: %d\n",
                reply.host.id, retval
            );
            reply.send_msg_ack = false;

            // may as well return; if one insert failed, others will too
            //
            return;
        }
    }
}

void handle_msgs_to_host(SCHEDULER_REPLY& reply) {
    DB_MSG_TO_HOST mth;
    char buf[256];
    sprintf(buf, "where hostid = %d and handled = %d", reply.host.id, 0);
    while (!mth.enumerate(buf)) {
        reply.msgs_to_host.push_back(mth);
        mth.handled = true;
        mth.update();
    }
}

void process_request(
    SCHEDULER_REQUEST& sreq, SCHEDULER_REPLY& reply, SCHED_SHMEM& ss,
    char* code_sign_key
) {
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

    // if different major version of BOINC, just send a message
    //
    if (wrong_core_client_version(sreq, reply) || unacceptable_os(sreq, reply)) {
        ok_to_send_work = false;

        // if no results, return without accessing DB
        //
        if (sreq.results.size() == 0) {
            return;
        }
    } else {
        warn_user_if_core_client_upgrade_scheduled(sreq, reply);
    }

    if (config.locality_scheduling) {
        have_no_work = false;
    } else {
        lock_sema();
        have_no_work = ss.no_work(g_pid);
        unlock_sema();
    }

    // if there's no work, and client isn't returning results,
    // this isn't an initial RPC,
    // and client is requesting work, return without accessing DB
    //
    if (
        config.nowork_skip
        && (sreq.work_req_seconds > 0)
        && have_no_work
        && (sreq.results.size() == 0)
        && (sreq.hostid != 0)
    ) {
        USER_MESSAGE um("No work available", "low");
        reply.insert_message(um);
        reply.set_delay(3600);
        if (!config.msg_to_host) {
            log_messages.printf(
                SCHED_MSG_LOG::MSG_NORMAL, "No work - skipping DB access\n"
            );
            return;
        }
    }

    // FROM HERE ON DON'T RETURN; goto leave instead
    // because we've tagged an entry in the work array with our process ID

    // now open the database
    //
    retval = open_database();
    if (retval) {
        send_message("Server can't open database", 3600, false);
        goto leave;
    }

    retval = authenticate_user(sreq, reply);
    if (retval) goto leave;
    if (reply.user.id == 0) {
        log_messages.printf(SCHED_MSG_LOG::MSG_CRITICAL, "No user ID!\n");
    }
    initial_host = reply.host;
    reply.host.rpc_seqno = sreq.rpc_seqno;

    log_messages.printf(
        SCHED_MSG_LOG::MSG_NORMAL,
        "Processing request from [USER#%d] [HOST#%d] [IP %s] [RPC#%d] core client version %d.%d.%d\n",
        reply.user.id, reply.host.id, get_remote_addr(), sreq.rpc_seqno,
        sreq.core_client_major_version, sreq.core_client_minor_version,
        sreq.core_client_release
    );
    ++log_messages;

    if (strlen(config.sched_lockfile_dir)) {
        int pid_with_lock = lock_sched(reply);
        if (pid_with_lock > 0) {
            log_messages.printf(SCHED_MSG_LOG::MSG_CRITICAL,
                "Another scheduler instance [PID=%d] is running for this host\n",
                pid_with_lock
            );
        } else if (pid_with_lock) {
            log_messages.printf(SCHED_MSG_LOG::MSG_CRITICAL,
                "Error acquiring lock for [HOST#%d]\n", reply.host.id
            );
        }
        if (pid_with_lock) {
            send_message(
                "Another scheduler instance is running for this host",
                60, false
            );
            goto leave;
        }
    }

    last_rpc_time = reply.host.rpc_time;
    rpc_time_tm = localtime((const time_t*)&reply.host.rpc_time);
    last_rpc_dayofyear = rpc_time_tm->tm_yday;

    reply.host.rpc_time = time(0);
    rpc_time_tm = localtime((const time_t*)&reply.host.rpc_time);
    current_rpc_dayofyear = rpc_time_tm->tm_yday;

    if (config.daily_result_quota) {
        if (reply.host.max_results_day <= 0 || reply.host.max_results_day > config.daily_result_quota) {
            reply.host.max_results_day = config.daily_result_quota;
            log_messages.printf(SCHED_MSG_LOG::MSG_DEBUG,
                "[HOST#%d] Initializing max_results_day to %d\n",
                reply.host.id, config.daily_result_quota
            );
        }
    }

    if (last_rpc_dayofyear != current_rpc_dayofyear) {
        log_messages.printf(SCHED_MSG_LOG::MSG_DEBUG,
            "[HOST#%d] Resetting nresults_today\n", reply.host.id
        );
        reply.host.nresults_today = 0;
    }
    retval = modify_host_struct(sreq, reply.host);

    // look up the client's platform in the DB
    //
    platform = ss.lookup_platform(sreq.platform_name);
    if (!platform) {
        sprintf(buf, "platform '%s' not found", sreq.platform_name);
        USER_MESSAGE um(buf, "low");
        reply.insert_message(um);
        log_messages.printf(
            SCHED_MSG_LOG::MSG_CRITICAL, "[HOST#%d] platform '%s' not found\n",
            reply.host.id, sreq.platform_name
        );
        reply.set_delay(3600*24);
        goto leave;
    }
    
    handle_global_prefs(sreq, reply);

#if 0
    reply.deletion_policy_priority = config.deletion_policy_priority;
    reply.deletion_policy_expire = config.deletion_policy_expire;
#endif

    handle_results(sreq, reply);

    if (config.resend_lost_results && sreq.have_other_results_list) {
        if (resend_lost_work(sreq, reply, *platform, ss)) {
            ok_to_send_work = false;
        }
    }

    // if last RPC was within config.min_sendwork_interval, don't send work
    //
    if (!have_no_work && ok_to_send_work && sreq.work_req_seconds > 0) {
        if (config.min_sendwork_interval) {
            double diff = dtime() - last_rpc_time;
            if (diff < config.min_sendwork_interval) {
                ok_to_send_work = false;
                log_messages.printf(
                    SCHED_MSG_LOG::MSG_NORMAL,
                    "Not sending work - last RPC too recent: %f\n", diff
                );
                sprintf(buf,
                    "Not sending work - last RPC too recent: %d sec", (int)diff
                );
                USER_MESSAGE um(buf, "low");
                reply.insert_message(um);

                // the 1.01 is in case client's clock
                // is slightly faster than ours
                //
                reply.set_delay(1.01*config.min_sendwork_interval);
            }
        }
        if (ok_to_send_work) {
            send_work(sreq, reply, *platform, ss);
        }
    }

    send_code_sign_key(sreq, reply, code_sign_key);

    handle_msgs_from_host(sreq, reply);
    if (config.msg_to_host) {
        handle_msgs_to_host(reply);
    }

    update_host_record(initial_host, reply.host, reply.user);

leave:
    if (!have_no_work) {
        ss.restore_work(g_pid);
    }
}

void debug_sched(
    SCHEDULER_REQUEST& sreq, SCHEDULER_REPLY& sreply, const char *trigger
) {
    char tmpfilename[256];
    FILE *fp;

    if (!boinc_file_exists(trigger)) {
        return;
    }

    sprintf(tmpfilename, "sched_reply_%06d_%06d", sreq.hostid, sreq.rpc_seqno);
    // use _XXXXXX if you want random filenames rather than
    // deterministic mkstemp(tmpfilename);
    
    fp=fopen(tmpfilename, "w");
    
    if (!fp) {
        log_messages.printf(
            SCHED_MSG_LOG::MSG_CRITICAL,
            "Found %s, but can't open %s\n", trigger, tmpfilename
        );
        return;
    }
    
    log_messages.printf(
        SCHED_MSG_LOG::MSG_DEBUG,
        "Found %s, so writing %s\n", trigger, tmpfilename
    );
    
    sreply.write(fp);
    fclose(fp);
    
    sprintf(tmpfilename, "sched_request_%06d_%06d", sreq.hostid, sreq.rpc_seqno);
    fp=fopen(tmpfilename, "w");

    if (!fp) {
        log_messages.printf(
            SCHED_MSG_LOG::MSG_CRITICAL,
            "Found %s, but can't open %s\n", trigger, tmpfilename
        );
        return;
    }

    log_messages.printf(
        SCHED_MSG_LOG::MSG_DEBUG,
        "Found %s, so writing %s\n", trigger, tmpfilename
    );

    sreq.write(fp);
    fclose(fp);

    return;
}

void handle_request(
    FILE* fin, FILE* fout, SCHED_SHMEM& ss, char* code_sign_key
) {
    SCHEDULER_REQUEST sreq;
    SCHEDULER_REPLY sreply;

    memset(&sreq, 0, sizeof(sreq));

    if (sreq.parse(fin) == 0){
        log_messages.printf(
             SCHED_MSG_LOG::MSG_NORMAL,
             "Handling request: IP %s, auth %s, host %d, platform %s, version %d.%d.%d, RSF %f\n",
             get_remote_addr(), sreq.authenticator, sreq.hostid, sreq.platform_name,
             sreq.core_client_major_version, sreq.core_client_minor_version,
             sreq.core_client_release,
             sreq.resource_share_fraction
        );
        process_request(sreq, sreply, ss, code_sign_key);
    } else {
        // BOINC scheduler requests use method POST.
        // So method GET means that someone is trying a browser.
        //
        char *rm=getenv("REQUEST_METHOD");
        if (rm && !strcmp(rm, "GET")) {
            sreply.probable_user_browser=true;
        }
        
        log_messages.printf(
            SCHED_MSG_LOG::MSG_NORMAL,
            "Incomplete request received %sfrom IP %s, auth %s, platform %s, version %d.%d.%d\n",
            sreply.probable_user_browser?"(probably a browser) ":"",
            get_remote_addr(), sreq.authenticator, sreq.platform_name,
            sreq.core_client_major_version, sreq.core_client_minor_version,
            sreq.core_client_release
        );
        
        USER_MESSAGE um("Incomplete request received.", "low");
        sreply.insert_message(um);
        sreply.nucleus_only = true;
    }

#ifdef EINSTEIN_AT_HOME
    // for testing
    if (sreply.user.id==3) {
        USER_MESSAGE um("THIS IS A SHORT MESSAGE. \n AND ANOTHER", "high");
        // USER_MESSAGE um("THIS IS A VERY LONG TEST MESSAGE. THIS IS A VERY LONG TEST MESSAGE. \n"
    //        "THIS IS A VERY LONG TEST MESSAGE. THIS IS A VERY LONG TEST MESSAGE.", "low");
        sreply.insert_message(um);
        // USER_MESSAGE um2("THIS IS A VERY LONG TEST MESSAGE2. THIS IS A VERY LONG TEST MESSAGE. \n"
    //        "THIS IS A VERY LONG TEST MESSAGE. THIS IS A VERY LONG TEST MESSAGE.", "high");
        // sreply.insert_message(um2);
    }
#endif
    
    // if we got no work, and we have no file space, delete some files
    //
    if (sreply.results.size()==0 && (sreply.wreq.insufficient_disk || sreply.wreq.disk_available<0)) {
        // try to delete a file to make more space.
        // Also give some hints to the user about what's going wrong
        // (lack of disk space).
        //
        delete_file_from_host(sreq, sreply);
    }
    
    // write all messages to log file
    for (unsigned int i=0; i<sreply.messages.size(); i++) {
        USER_MESSAGE um = sreply.messages[i];
        log_messages.printf(SCHED_MSG_LOG::MSG_DEBUG,
            "[HOST#%d] MSG(%4s) %s \n", sreply.host.id, um.priority.c_str(), um.message.c_str()
        );
    }

    debug_sched(sreq, sreply, "../debug_sched");

#ifdef EINSTEIN_AT_HOME
    // You can call debug_sched() for whatever situation is of
    // interest to you.  It won't do anything unless you create
    // (touch) the file 'debug_sched' in the project root directory.
    //
    if (sreply.results.size()==0 && sreply.hostid && sreq.work_req_seconds>1.0) {
        debug_sched(sreq, sreply, "../debug_sched");
    } else if (max_allowable_disk(sreq, sreply)<0 || (sreply.wreq.insufficient_disk || sreply.wreq.disk_available<0)) {
        debug_sched(sreq, sreply, "../debug_sched");
    }
#endif
    
    sreply.write(fout);

    if (strlen(config.sched_lockfile_dir)) {
        unlock_sched(sreply);
    }
}

const char *BOINC_RCSID_2ac231f9de = "$Id$";
