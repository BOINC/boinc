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

#ifdef _USING_FCGI_
#include "/usr/local/include/fcgi_stdio.h"
#else
#include <stdio.h>
#endif

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
#include "sched_msgs.h"
#include "sched_send.h"

const double COBBLESTONE_FACTOR = 300.0;

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
                SCHED_MSG_LOG::NORMAL,
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
                SCHED_MSG_LOG::NORMAL,
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
                SCHED_MSG_LOG::CRITICAL,
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
                SCHED_MSG_LOG::NORMAL,
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
                SCHED_MSG_LOG::CRITICAL,
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
            log_messages.printf(SCHED_MSG_LOG::CRITICAL, "host.insert() failed\n");
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
        log_messages.printf(SCHED_MSG_LOG::CRITICAL, "host.update() failed: %d\n", retval);
    }
    return 0;
}

// Decide which global prefs to use,
// (from request msg, or if absent then from user record)
// and parse them into the request message global_prefs field.
// Decide whether to send global prefs in reply msg
//
int handle_global_prefs(SCHEDULER_REQUEST& sreq, SCHEDULER_REPLY& reply) {
    reply.send_global_prefs = false;

    if (strlen(sreq.global_prefs_xml)) {
        unsigned req_mod_time=0, db_mod_time=0;
        parse_int(sreq.global_prefs_xml, "<mod_time>", (int&)req_mod_time);
        if (strlen(reply.user.global_prefs)) {
            parse_int(reply.user.global_prefs, "<mod_time>", (int&)db_mod_time);

            // if user record has more recent prefs,
            // use them and arrange to return in reply msg
            //
            if (req_mod_time < db_mod_time) {
                strcpy(sreq.global_prefs_xml, reply.user.global_prefs);
                reply.send_global_prefs = true;
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
    sreq.global_prefs.parse(sreq.global_prefs_xml, sreq.host.venue);
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
                SCHED_MSG_LOG::CRITICAL,
                "[HOST#%d] [RESULT#? %s] can't find result\n",
                host.id, rp->name
            );
            continue;
        }

        log_messages.printf(
            SCHED_MSG_LOG::NORMAL, "[HOST#%d] [RESULT#%d %s] got result\n",
            host.id, result.id, result.name
        );

        if (result.server_state == RESULT_SERVER_STATE_UNSENT) {
            log_messages.printf(
                SCHED_MSG_LOG::CRITICAL,
                "[HOST#%d] [RESULT#%d %s] got unexpected result: server state is %d\n",
                host.id, result.id, result.name, result.server_state
            );
            continue;
        }

        if (result.received_time) {
            log_messages.printf(
                SCHED_MSG_LOG::CRITICAL,
                "[HOST#%d] [RESULT#%d %s] got result twice\n",
                host.id, result.id, result.name
            );
            continue;
        }

        if (result.hostid != sreq.hostid) {
            log_messages.printf(
                SCHED_MSG_LOG::CRITICAL,
                "[HOST#%d] [RESULT#%d %s] got result from wrong host; expected [HOST#%d]\n",
                host.id, result.id, result.name, result.hostid
            );
            DB_HOST result_host;
            int retval = result_host.lookup_id(result.hostid);

            if (retval) {
                log_messages.printf(
                    SCHED_MSG_LOG::CRITICAL,
                    "[RESULT#%d %s] Can't lookup [HOST#%d]\n",
                    result.id, result.name, result.hostid
                );
                continue;
            } else if (result_host.userid != host.userid) {
                log_messages.printf(
                    SCHED_MSG_LOG::CRITICAL,
                    "[USER#%d] [HOST#%d] [RESULT#%d %s] Not even the same user; expected [USER#%d]\n",
                    host.userid, host.id, result.id, result.name, result_host.userid
                );
                continue;
            } else {
                log_messages.printf(
                    SCHED_MSG_LOG::CRITICAL,
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
        result.exit_status = rp->exit_status;
        result.app_version_num = rp->app_version_num;
        result.claimed_credit = result.cpu_time * host.credit_per_cpu_sec;
        result.server_state = RESULT_SERVER_STATE_OVER;

        strncpy(result.stderr_out, rp->stderr_out, sizeof(result.stderr_out));
        strncpy(result.xml_doc_out, rp->xml_doc_out, sizeof(result.xml_doc_out));

        // look for exit status and app version in stderr_out
        // (historical - can be deleted at some point)
        //
        parse_int(result.stderr_out, "<exit_status>", result.exit_status);
        parse_int(result.stderr_out, "<app_version>", result.app_version_num);

        if ((result.client_state == RESULT_FILES_UPLOADED) && (result.exit_status == 0)) {
            result.outcome = RESULT_OUTCOME_SUCCESS;
            log_messages.printf(SCHED_MSG_LOG::DEBUG,
                "[RESULT#%d %s]: setting outcome SUCCESS\n",
                result.id, result.name
            );
        } else {
            log_messages.printf(SCHED_MSG_LOG::DEBUG,
                "[RESULT#%d %s]: client_state %d exit_status %d; setting outcome ERROR\n",
                result.id, result.name, result.client_state, result.exit_status
            );
            result.outcome = RESULT_OUTCOME_CLIENT_ERROR;
            result.validate_state = VALIDATE_STATE_INVALID;
        }

        result.teamid = reply.user.teamid;
        retval = result.update();
        if (retval) {
            log_messages.printf(
                SCHED_MSG_LOG::NORMAL,
                "[HOST#%d] [RESULT#%d %s] can't update result: %s\n",
                host.id, result.id, result.name, boinc_db.error_string()
            );
        }

        // trigger the transition handle for the result's WU
        //
        retval = wu.lookup_id(result.workunitid);
        if (retval) {
            log_messages.printf(
                SCHED_MSG_LOG::CRITICAL,
                "[HOST#%d] [RESULT#%d %s] Can't find [WU#%d] for result\n",
                host.id, result.id, result.name, result.workunitid
            );
        } else {
            wu.transition_time = time(0);
            retval = wu.update();
            if (retval) {
                log_messages.printf(
                    SCHED_MSG_LOG::CRITICAL,
                    "[HOST#%d] [RESULT#%d %s] Can't update [WU#%d %s]\n",
                    host.id, result.id, result.name, wu.id, wu.name
                );
            }
        }
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
            log_messages.printf(SCHED_MSG_LOG::NORMAL, "received old code sign key\n");

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
            SCHED_MSG_LOG::NORMAL,
            "[HOST#%d] [auth %s] Wrong major version from user: wanted %d, got %d\n",
            sreq.hostid, sreq.authenticator,
            MAJOR_VERSION, sreq.core_client_major_version
        );
        reply.request_delay = 3600*24;
        return true;
    }
    return false;
}

inline static const char* get_remote_addr() {
    const char * r = getenv("REMOTE_ADDR");
    return r ? r : "?.?.?.?";
}

void handle_trickle_ups(SCHEDULER_REQUEST& sreq, SCHEDULER_REPLY& reply) {
    unsigned int i;
    DB_RESULT result;
    DB_TRICKLE_UP trickle;
    int retval;
    char buf[256];

    for (i=0; i<sreq.trickles.size(); i++) {
        reply.send_trickle_up_ack = true;
        TRICKLE_UP_DESC& td = sreq.trickles[i];
        sprintf(buf, "where name='%s'", td.result_name);
        retval = result.lookup(buf);
        if (retval) continue;
        if (reply.user.id != result.userid) {
            log_messages.printf(SCHED_MSG_LOG::NORMAL,
                "[HOST#%d] trickle up: wrong user ID %d, %d\n", 
                sreq.host.id, reply.user.id, result.userid
            );
            continue;
        }
        if (sreq.host.id != result.hostid) {
            log_messages.printf(SCHED_MSG_LOG::NORMAL,
                "[HOST#%d] trickle up: wrong host ID %d\n", 
                sreq.host.id, result.hostid
            );
            continue;
        }
        memset(&trickle, 0, sizeof(trickle));
        trickle.send_time = td.send_time;
        trickle.resultid = result.id;
        trickle.appid = result.appid;
        trickle.handled = false;
        safe_strcpy(trickle.xml, td.trickle_text.c_str());
        retval = trickle.insert();
        if (retval) {
            log_messages.printf(SCHED_MSG_LOG::CRITICAL,
                "[HOST#%d] trickle insert failed: %d\n", 
                sreq.host.id, retval
            );
        }
    }
}

void handle_trickle_downs(SCHEDULER_REQUEST& sreq, SCHEDULER_REPLY& reply) {
    DB_TRICKLE_DOWN td;
    char buf[256];

    sprintf(buf, "where hostid = %d", sreq.host.id);
    while (!td.enumerate(buf)) {
        reply.trickle_downs.push_back(td);
        td.handled = true;
        td.update();
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

    // now open the database
    //
    retval = open_database();
    if (retval) {
        send_shut_message();
        return;
    }

    retval = authenticate_user(sreq, reply);
    if (retval) return;
    if (reply.user.id == 0) {
        log_messages.printf(SCHED_MSG_LOG::CRITICAL, "No user ID!\n");
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
            SCHED_MSG_LOG::CRITICAL, "[HOST#%d] platform '%s' not found\n",
            reply.host.id, sreq.platform_name
        );
        return;
    }

    log_messages.printf(
        SCHED_MSG_LOG::NORMAL, "Processing request from [USER#%d] [HOST#%d] [IP %s] [RPC#%d] core client version %d.%02d\n",
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
                SCHED_MSG_LOG::NORMAL,
                "Not sending work - last RPC too recent: %f\n", diff
            );
            sprintf(reply.message,
                "Not sending work - last RPC too recent: %d sec", (int)diff
            );
            strcpy(reply.message_priority, "low");
            reply.request_delay = config.min_sendwork_interval;
        }
    }
    if (ok_to_send) {
        send_work(sreq, reply, *platform, ss);
    }

    send_code_sign_key(sreq, reply, code_sign_key);

    handle_trickle_ups(sreq, reply);
    if (config.trickle_down) {
        handle_trickle_downs(sreq, reply);
    }
}

void handle_request(
    FILE* fin, FILE* fout, SCHED_SHMEM& ss, char* code_sign_key
) {
    SCHEDULER_REQUEST sreq;
    SCHEDULER_REPLY sreply;

    memset(&sreq, 0, sizeof(sreq));

    if (sreq.parse(fin) == 0){
        log_messages.printf(
             SCHED_MSG_LOG::NORMAL, "Handling request: IP %s, auth %s, platform %s, version %d.%d\n",
             get_remote_addr(), sreq.authenticator, sreq.platform_name,
             sreq.core_client_major_version, sreq.core_client_minor_version
        );
        process_request(sreq, sreply, ss, code_sign_key);
    } else {
        log_messages.printf(
            SCHED_MSG_LOG::NORMAL, "Incomplete request received from IP %s, auth %s, platform %s, version %d.%d\n",
             get_remote_addr(), sreq.authenticator, sreq.platform_name,
             sreq.core_client_major_version, sreq.core_client_minor_version
        );
        strcpy(sreply.message, "Incomplete request received.");
        strcpy(sreply.message_priority, "low");
        return;
    }
    
    sreply.write(fout);
}
