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
#include "sched_msgs.h"
#include "sched_send.h"

#ifdef _USING_FCGI_
#include "fcgi_stdio.h"
#endif

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
            reply.request_delay = 3600;
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
            reply.request_delay = 3600;
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
        //
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
            reply.request_delay = 3600;
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
            strcpy(reply.user.cross_project_id, sreq.cross_project_id);
            reply.update_user_record = true;
        }
    }
    return 0;
}

// somewhat arbitrary formula for credit as a function of CPU time.
// Could also include terms for RAM size, network speed etc.
//
static void compute_credit_rating(HOST& host, SCHEDULER_REQUEST& sreq) {
    double cobblestone_factor = 300;
    if (sreq.core_client_major_version > 3) cobblestone_factor = 100;
    if (sreq.core_client_minor_version > 5) cobblestone_factor = 100;
    host.credit_per_cpu_sec =
        (fabs(host.p_fpops)/1e9 + fabs(host.p_iops)/1e9)
        * cobblestone_factor / (2 * SECONDS_PER_DAY);
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

    compute_credit_rating(host, sreq);
    return 0;
}

static int update_host_record(HOST& xhost) {
    DB_HOST host;
    int retval;

    host = xhost;
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
            reply.update_user_record = true;
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
    SCHEDULER_REQUEST& sreq, SCHEDULER_REPLY& reply
) {
    DB_SCHED_RESULT_ITEM_SET result_handler;
    SCHED_RESULT_ITEM result;
    unsigned int i;
    int retval;
    RESULT* rp;

    // lets request the status of all results the user is reporting in a batch
    //
    if (0 < sreq.results.size()) {
        for (i=0; i<sreq.results.size(); i++) {
            result_handler.add_result(sreq.results[i].name);
        }

        retval = result_handler.enumerate();
        if (retval) {
            log_messages.printf(
                SCHED_MSG_LOG::CRITICAL,
                "[HOST#%d] Batch query failed\n"
            );
        }
    }

    for (i=0; i<sreq.results.size(); i++) {
        rp = &sreq.results[i];

        // acknowledge the result even if we couldn't find it --
        // don't want it to keep coming back
        //
        reply.result_acks.push_back(*rp);

        retval = result_handler.lookup_result(rp->name, result);
        if (retval) {
            log_messages.printf(
                SCHED_MSG_LOG::CRITICAL,
                "[HOST#%d] [RESULT#? %s] can't find result\n",
                reply.host.id, rp->name
            );
            continue;
        }

        log_messages.printf(
            SCHED_MSG_LOG::NORMAL, "[HOST#%d] [RESULT#%d %s] got result\n",
            reply.host.id, result.id, result.name
        );

        if (result.server_state == RESULT_SERVER_STATE_UNSENT) {
            log_messages.printf(
                SCHED_MSG_LOG::CRITICAL,
                "[HOST#%d] [RESULT#%d %s] got unexpected result: server state is %d\n",
                reply.host.id, result.id, result.name, result.server_state
            );
            continue;
        }

        if (result.received_time) {
            log_messages.printf(
                SCHED_MSG_LOG::CRITICAL,
                "[HOST#%d] [RESULT#%d %s] got result twice\n",
                reply.host.id, result.id, result.name
            );
            continue;
        }

        if (result.hostid != sreq.hostid) {
            log_messages.printf(
                SCHED_MSG_LOG::CRITICAL,
                "[HOST#%d] [RESULT#%d %s] got result from wrong host; expected [HOST#%d]\n",
                reply.host.id, result.id, result.name, result.hostid
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
            } else if (result_host.userid != reply.host.userid) {
                log_messages.printf(
                    SCHED_MSG_LOG::CRITICAL,
                    "[USER#%d] [HOST#%d] [RESULT#%d %s] Not even the same user; expected [USER#%d]\n",
                    reply.host.userid, reply.host.id, result.id, result.name, result_host.userid
                );
                continue;
            } else {
                log_messages.printf(
                    SCHED_MSG_LOG::CRITICAL,
                    "[HOST#%d] [RESULT#%d %s] Allowing result because same USER#%d\n",
                    reply.host.id, result.id, result.name, reply.host.userid
                );
            }
        }

        // update the result record in DB
        //
        result.hostid = reply.host.id;
        result.teamid = reply.user.teamid;
        result.received_time = time(0);
        result.client_state = rp->client_state;
        result.cpu_time = rp->cpu_time;
        result.exit_status = rp->exit_status;
        result.app_version_num = rp->app_version_num;
        result.claimed_credit = result.cpu_time * reply.host.credit_per_cpu_sec;
#if 1
        log_messages.printf(SCHED_MSG_LOG::DEBUG,
            "cpu %f cpcs %f, cc %f\n", result.cpu_time, reply.host.credit_per_cpu_sec, result.claimed_credit
        );
#endif
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
    }


    // Lets update all the results we have
    //
    log_messages.printf(
        SCHED_MSG_LOG::DEBUG,
        "[HOST#%d] Starting Result Update Transaction...\n",
        reply.host.id
        );
    retval = result_handler.start_transaction();
    if (retval) {
        log_messages.printf(SCHED_MSG_LOG::CRITICAL,
            "[HOST#%d] result_handler.start_transaction() == %d\n",
            reply.host.id, retval
        );
    }
    
    for (i=0; i<result_handler.results.size(); i++) {
        if (0 < result_handler.results[i].id) {
            retval = result_handler.update_result(result_handler.results[i]);
            if (retval) {
                log_messages.printf(
                    SCHED_MSG_LOG::CRITICAL,
                    "[HOST#%d] [RESULT#%d %s] can't update result: %s\n",
                    reply.host.id, result_handler.results[i].id, result_handler.results[i].name, 
                    boinc_db.error_string()
                );
            }

            // trigger the transition handle for the result's WU
            //
            retval = result_handler.update_workunit(result_handler.results[i]);
            if (retval) {
                log_messages.printf(
                    SCHED_MSG_LOG::CRITICAL,
                    "[HOST#%d] [RESULT#%d %s] can't update [WU#%d]\n",
                    reply.host.id, result_handler.results[i].id, result_handler.results[i].name, 
                    result_handler.results[i].workunitid
                );
            }
        }
    }

    log_messages.printf(
        SCHED_MSG_LOG::DEBUG,
        "[HOST#%d] Committing Transaction...\n",
        reply.host.id
        );
    retval = result_handler.commit_transaction();
    if (retval) {
        log_messages.printf(
            SCHED_MSG_LOG::CRITICAL,
            "[HOST#%d] result_handler.commit_transaction() == %d\n",
            reply.host.id, retval
        );
    } else {
        log_messages.printf(
            SCHED_MSG_LOG::DEBUG,
            "[HOST#%d] Committed Transaction Successfully...\n",
            reply.host.id
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
        //reply.nucleus_only = true;
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

void handle_msgs_from_host(SCHEDULER_REQUEST& sreq, SCHEDULER_REPLY& reply) {
    unsigned int i;
    DB_MSG_FROM_HOST mfh;
    int retval;
    char buf[256];

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
            SCHED_MSG_LOG::NORMAL,
            "got msg from host; variety %s text %s\n",
            mfh.variety, mfh.xml
        );
        retval = mfh.insert();
        if (retval) {
            log_messages.printf(SCHED_MSG_LOG::CRITICAL,
                "[HOST#%d] message insert failed: %d\n",
                reply.host.id, retval
            );
        }
    }
}

void handle_msgs_to_host(SCHEDULER_REQUEST& sreq, SCHEDULER_REPLY& reply) {
    DB_MSG_TO_HOST mth;
    char buf[256];
    sprintf(buf, "where hostid = %d and handled=0", reply.host.id);
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


    // if different major version of BOINC, just send a message
    //
    if (wrong_major_version(sreq, reply)) {
        ok_to_send_work = false;

        // if no results, return without accessing DB
        //
        if (sreq.results.size() == 0) {
            return;
        }
    }

    // if there's no work, and client isn't returning results,
    // and client is requesting work, return without accessing DB
    //
    if ((sreq.work_req_seconds > 0)
        && ss.no_work()
        && (sreq.results.size() == 0)
    ) {
        strcat(reply.message, "No work available");
        strcpy(reply.message_priority, "low");
        reply.request_delay = 3600;
        log_messages.printf(
            SCHED_MSG_LOG::NORMAL, "No work - skipping DB access\n"
        );
        return;
    }

    // now open the database
    //
    retval = open_database();
    if (retval) {
        send_message("Server can't open database", 3600);
        return;
    }

    retval = authenticate_user(sreq, reply);
    if (retval) return;
    if (reply.user.id == 0) {
        log_messages.printf(SCHED_MSG_LOG::CRITICAL, "No user ID!\n");
    }

    log_messages.printf(
        SCHED_MSG_LOG::NORMAL,
        "Processing request from [USER#%d] [HOST#%d] [IP %s] [RPC#%d] core client version %d.%02d\n",
        reply.user.id, reply.host.id, get_remote_addr(), sreq.rpc_seqno,
        sreq.core_client_major_version, sreq.core_client_minor_version
    );
    ++log_messages;

    last_rpc_time = reply.host.rpc_time;
    rpc_time_tm = localtime((const time_t*)&reply.host.rpc_time);
    last_rpc_dayofyear = rpc_time_tm->tm_yday;

    reply.host.rpc_time = time(0);
    rpc_time_tm = localtime((const time_t*)&reply.host.rpc_time);
    current_rpc_dayofyear = rpc_time_tm->tm_yday;

    if (last_rpc_dayofyear != current_rpc_dayofyear) {
        log_messages.printf(SCHED_MSG_LOG::DEBUG, "[HOST#%d] Resetting nresults_today\n", reply.host.id);
        reply.host.nresults_today = 0;
    }
    retval = modify_host_struct(sreq, reply.host);

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

    handle_global_prefs(sreq, reply);

    if (reply.update_user_record) {
        DB_USER user;
        user = reply.user;
        user.update();
    }

    handle_results(sreq, reply);

    // if last RPC was within config.min_sendwork_interval, don't send work
    //
    if (ok_to_send_work && sreq.work_req_seconds > 0) {
        if (config.min_sendwork_interval) {
            double diff = dtime() - last_rpc_time;
            if (diff < config.min_sendwork_interval) {
                ok_to_send_work = false;
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
        if (ok_to_send_work) {
            send_work(sreq, reply, *platform, ss);
        }
    }

    send_code_sign_key(sreq, reply, code_sign_key);

    handle_msgs_from_host(sreq, reply);
    if (config.msg_to_host) {
        handle_msgs_to_host(sreq, reply);
    }

    update_host_record(reply.host);
}

void handle_request(
    FILE* fin, FILE* fout, SCHED_SHMEM& ss, char* code_sign_key
) {
    SCHEDULER_REQUEST sreq;
    SCHEDULER_REPLY sreply;

    memset(&sreq, 0, sizeof(sreq));

    if (sreq.parse(fin) == 0){
        log_messages.printf(
             SCHED_MSG_LOG::NORMAL, "Handling request: IP %s, auth %s, platform %s, version %d.%02d\n",
             get_remote_addr(), sreq.authenticator, sreq.platform_name,
             sreq.core_client_major_version, sreq.core_client_minor_version
        );
        process_request(sreq, sreply, ss, code_sign_key);
    } else {
        log_messages.printf(
            SCHED_MSG_LOG::NORMAL,
            "Incomplete request received from IP %s, auth %s, platform %s, version %d.%02d\n",
            get_remote_addr(), sreq.authenticator, sreq.platform_name,
            sreq.core_client_major_version, sreq.core_client_minor_version
        );
        strcpy(sreply.message, "Incomplete request received.");
        strcpy(sreply.message_priority, "low");
        sreply.nucleus_only = true;
    }

    sreply.write(fout);
}
