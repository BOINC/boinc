// The contents of this file are subject to the Mozilla Public License
// Version 1.0 (the "License"); you may not use this file except in
// compliance with the License. You may obtain a copy of the License at
// http://www.mozilla.org/MPL/
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

#include <iostream>
#include <vector>
#include <string>
using namespace std;

#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <time.h>
#include <assert.h>

#include "db.h"
#include "backend_lib.h"
#include "parse.h"
#include "server_types.h"
#include "handle_request.h"

#define MIN_SECONDS_TO_SEND 0
#define MAX_SECONDS_TO_SEND 2419200 //4 weeks
#define MAX_WUS_TO_SEND     2

// return true if the WU can be executed on the host
//
bool wu_is_feasible(WORKUNIT& wu, HOST& host) {
    return ((wu.rsc_disk <= host.d_free) && (wu.rsc_memory <= host.m_nbytes));
}

// estimate the time that a WU will take on a host
//
double estimate_duration(WORKUNIT& wu, HOST& host) {
    return wu.rsc_fpops/host.p_fpops + wu.rsc_iops/host.p_iops;
}

// insert an element in xml_doc with an estimation of how many seconds
// a workunit will take to complete
//
int insert_time_tag(WORKUNIT& wu, double seconds) {
    char *location;

    location = strstr(wu.xml_doc, "</workunit>");
    if ((location - wu.xml_doc) > (MAX_BLOB_SIZE - 64)) {
        return -1; //not enough space to include time info
    }
    sprintf(location,
        "    <seconds_to_complete>%f</seconds_to_complete>\n"
        "</workunit>\n",
        seconds
    );
    return 0;
}

// add the given workunit to a reply.
// look up its app, and make sure there's a version for this platform.
// Add the app and app_version to the reply also.
//
int add_wu_to_reply(
    WORKUNIT& wu, SCHEDULER_REPLY& reply, PLATFORM& platform, SCHED_SHMEM& ss,
    double seconds_to_complete
) {
    APP* app;
    APP_VERSION* app_version;
    int retval;

    app = ss.lookup_app(wu.appid);
    if (!app) return -1;
    app_version = ss.lookup_app_version(app->id, platform.id, app->min_version);
    if (!app_version) return -1;

    // add the app, app_version, and workunit to the reply,
    // but only if they aren't already there

    reply.insert_app_unique(*app);
    reply.insert_app_version_unique(*app_version);

    // add time estimate to reply
    //
    retval = insert_time_tag(wu, seconds_to_complete);
    if (retval) return retval;
    reply.insert_workunit_unique(wu);
    return 0;
}

// Look up the host and its user, and make sure the authenticator matches.
// If no host ID is supplied, or if RPC seqno mismatch,
// create a new host record and return its ID
//
int authenticate_user(SCHEDULER_REQUEST& sreq, SCHEDULER_REPLY& reply) {
    int retval;

    if (sreq.hostid) {
        retval = db_host(sreq.hostid, reply.host);
        if (retval) {
            strcpy(reply.message, "Can't find host record");
            strcpy(reply.message_priority, "low");
            return -1;
        }
        retval = db_user(reply.host.userid, reply.user);
        if (retval) {
            strcpy(reply.message, "Can't find user record");
            strcpy(reply.message_priority, "low");
            return -1;
        }
        if (strcmp(sreq.authenticator, reply.user.authenticator)) {
            strcpy(reply.message,
               "Invalid or missing authenticator.  "
               "Visit this project's web site to get an authenticator."
            );
            strcpy(reply.message_priority, "low");
            return -1;
        }

        // If the seqno from the host is less than what we expect,
        // the user must have copied the state file to a different host.
        // Make a new host record.
        if (sreq.rpc_seqno < reply.host.rpc_seqno) {
            sreq.hostid = 0;
            goto new_host;
        }
        reply.host.rpc_seqno = sreq.rpc_seqno;
        reply.host.rpc_time = time(0);
    } else {
        strncpy(
            reply.user.authenticator, sreq.authenticator,
            sizeof(reply.user.authenticator)
        );
        retval = db_user_lookup_auth(reply.user);
        if (retval) {
            strcpy(reply.message, "Invalid or missing authenticator");
            strcpy(reply.message_priority, "low");
            return -1;
        }
new_host:
        // reply.user is filled in and valid at this point
        //
        reply.host = sreq.host;
        reply.host.id = 0;
        reply.host.create_time = time(0);
        reply.host.userid = reply.user.id;
        reply.host.rpc_seqno = 0;
        reply.host.rpc_time = time(0);
        retval = db_host_new(reply.host);
        if (retval) {
            strcpy(reply.message, "server database error");
            strcpy(reply.message_priority, "low");
            db_print_error("db_host_new");
            return -1;
        }
        reply.host.id = db_insert_id();
        reply.hostid = reply.host.id;
        // this tells client to updates its host ID
    }
    return 0;
}

// somewhat arbitrary formula for credit as a function of CPU time.
// Could also include terms for RAM size, network speed etc.
//
static void compute_credit_rating(HOST& host) {
    host.credit_per_cpu_sec = 
        host.p_fpops
        + host.p_iops
        + 0.1*host.p_membw;
}

// Update host record based on request.
// Copy all fields that are determined by the client.
//
int update_host_record(SCHEDULER_REQUEST& sreq, HOST& host) {
    int retval;

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
    strncpy(host.p_vendor, sreq.host.p_vendor, sizeof(host.p_vendor));    // unlikely this will change
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

    compute_credit_rating(host);

    retval = db_host_update(host);
    if (retval) {
        fprintf(stderr, "sched (%s): db_host_update: %d\n", BOINC_USER, retval);
    }
    return 0;
}

// If the client sent prefs, and they're more recent than ours,
// update user record in DB.
// If we our DB has more recent prefs than client's, send them.
//
int handle_prefs(SCHEDULER_REQUEST& sreq, SCHEDULER_REPLY& reply) {
    if (sreq.prefs_mod_time > reply.user.prefs_mod_time && strlen(sreq.prefs_xml)) {
        strncpy(reply.user.prefs, sreq.prefs_xml, sizeof(reply.user.prefs));
        reply.user.prefs_mod_time = sreq.prefs_mod_time;
        if (reply.user.prefs_mod_time > (unsigned)time(0)) {
            reply.user.prefs_mod_time = (unsigned)time(0);
        }
        db_user_update(reply.user);
    }
    if (reply.user.prefs_mod_time > sreq.prefs_mod_time) {
        reply.send_prefs = true;
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
    RESULT result, *rp;
    WORKUNIT wu;

    for (i=0; i<sreq.results.size(); i++) {
        rp = &sreq.results[i];
        strncpy(result.name, rp->name, sizeof(result.name));
        retval = db_result_lookup_name(result);
        if (retval) {
            printf("can't find result %s\n", rp->name);
        } else {
            if (result.state != RESULT_STATE_IN_PROGRESS) {
                fprintf(stderr,
                    "got unexpected result for %s: state is %d\n",
                    rp->name, result.state
                );
                continue;
            }

            if (result.hostid != sreq.hostid) {
                fprintf(stderr,
                    "got result from wrong host: %d %d\n",
                    result.hostid, sreq.hostid
                );
                continue;
            }

            // TODO: handle error returns
            //
            result.hostid = reply.host.id;
            result.received_time = time(0);
            result.exit_status = rp->exit_status;
            result.cpu_time = rp->cpu_time;
            result.state = RESULT_STATE_DONE;
            result.claimed_credit = result.cpu_time * host.credit_per_cpu_sec;
            result.validate_state = VALIDATE_STATE_NEED_CHECK;
            strncpy(result.stderr_out, rp->stderr_out, sizeof(result.stderr_out));
            strncpy(result.xml_doc_out, rp->xml_doc_out, sizeof(result.xml_doc_out));
            db_result_update(result);

            retval = db_workunit(result.workunitid, wu);
            if (retval) {
                printf(
                    "can't find WU %d for result %d\n",
                    result.workunitid, result.id
                );
            } else {
                wu.nresults_done++;
                if (result.exit_status) wu.nresults_fail++;
                wu.need_validate = 1;
                retval = db_workunit_update(wu);
            }
        }

        // acknowledge the result even if we couldn't find it --
        // don't want it to keep coming back
        //
        reply.result_acks.push_back(*rp);
    }
    return 0;
}

int send_work(
    SCHEDULER_REQUEST& sreq, SCHEDULER_REPLY& reply, PLATFORM& platform,
    SCHED_SHMEM& ss
) {
    int i, retval, nresults = 0, seconds_to_fill;
    WORKUNIT wu;
    RESULT result;
#if 0
    APP* app;
    char prefix [256];
#endif

    seconds_to_fill = sreq.work_req_seconds;
    if (seconds_to_fill > MAX_SECONDS_TO_SEND) {
        seconds_to_fill = MAX_SECONDS_TO_SEND;
    }
    if (seconds_to_fill < MIN_SECONDS_TO_SEND) {
        seconds_to_fill = MIN_SECONDS_TO_SEND;
    }

    for (i=0; i<ss.nwu_results && seconds_to_fill>0; i++) {

        // the following should be a critical section
        //
        if (!ss.wu_results[i].present || 
            !wu_is_feasible(ss.wu_results[i].workunit, reply.host) 
        ) {
            continue;
        }
        wu = ss.wu_results[i].workunit;
        result = ss.wu_results[i].result;
        ss.wu_results[i].present = false;
        
        retval = add_wu_to_reply(wu, reply, platform, ss,
            estimate_duration(wu, reply.host)
        );
        if (retval) continue;
        reply.insert_result(result);
        seconds_to_fill -= (int)estimate_duration(wu, reply.host);

        result.state = RESULT_STATE_IN_PROGRESS;
        result.hostid = reply.host.id;
        result.sent_time = time(0);
        db_result_update(result);

        wu.nresults_unsent--;
        db_workunit_update(wu);

        nresults++;
        if (nresults == MAX_WUS_TO_SEND) break;
    }

#if 0
    while (!db_workunit_enum_dynamic_to_send(wu, 1)) {
        retval = add_wu_to_reply(wu, reply, platform, db);
        if (retval) continue;

        // here we have to create a new result record
        //
        memset(&result, 0, sizeof(result));
        db_result_new(result);
        result.id = db_insert_id();
        result.create_time = time(0);
        result.workunitid = wu.id;
        result.state = RESULT_STATE_IN_PROGRESS;
        result.hostid = reply.host.id;
        result.sent_time = time(0);
        sprintf(result.name, "result_%d", result.id);
        app = db.lookup_app(wu.appid);
        strncpy(result.xml_doc_in, app->result_xml_template, sizeof(result.xml_doc_in));
        sprintf(prefix, "%s_", result.name);
        process_result_template(
            result.xml_doc_in, prefix, wu.name, result.name
        );
        db_result_update(result);

        wu.nresults++;
        db_workunit_update(wu);

        reply.insert_result(result);
        nresults++;
    }
#endif

    if (nresults == 0) {
        strcpy(reply.message, "no work available");
        strcpy(reply.message_priority, "low");
        reply.request_delay = 10;
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

    if (sreq.code_sign_key) {
        if (strcmp(sreq.code_sign_key, code_sign_key)) {
            fprintf(stderr, "received old code sign key\n");
            // look for a signature file
            //
            for (i=0; ; i++) {
                sprintf(path, "%s/old_key_%d", "BOINC_KEY_DIR", i);
                retval = read_file_malloc(path, oldkey);
                if (retval) {
                    strcpy(reply.message,
                        "Can't update code signing key.  "
                        "Please report this to project."
                    );
                    return;
                }
                if (!strcmp(oldkey, sreq.code_sign_key)) {
                    sprintf(path, "%s/signature_%d", "BOINC_KEY_DIR", i);
                    retval = read_file_malloc(path, signature);
                    if (retval) {
                        strcpy(reply.message,
                            "Can't update code signing key.  "
                            "Please report this to project."
                        );
                    } else {
                        reply.code_sign_key = strdup(code_sign_key);
                        reply.code_sign_key_signature = signature;
                    }
                }
                free(oldkey);
                return;
            }
        }
    } else {
        fprintf(stderr, "%d: didn't get code sign key, sending one\n", getpid());
        reply.code_sign_key = strdup(code_sign_key);
    }
}

void process_request(
    SCHEDULER_REQUEST& sreq, SCHEDULER_REPLY& reply, SCHED_SHMEM& ss,
    char* code_sign_key
) {
    PLATFORM* platform;
    int retval;
    char buf[256];

    retval = authenticate_user(sreq, reply);
    if (retval) return;

    retval = update_host_record(sreq, reply.host);

    // look up the client's platform in the DB
    //
    platform = ss.lookup_platform(sreq.platform_name);
    if (!platform) {
        sprintf(buf, "platform %s not found", sreq.platform_name);
        strcpy(reply.message, buf);
        strcpy(reply.message_priority, "low");
        return;
    }

    handle_prefs(sreq, reply);

    handle_results(sreq, reply, reply.host);

    send_work(sreq, reply, *platform, ss);

    send_code_sign_key(sreq, reply, code_sign_key);
}

void handle_request(
    FILE* fin, FILE* fout, SCHED_SHMEM& ss, char* code_sign_key
) {
    SCHEDULER_REQUEST sreq;
    SCHEDULER_REPLY sreply;

    memset(&sreq, 0, sizeof(sreq));
    sreq.parse(fin);
    process_request(sreq, sreply, ss, code_sign_key);
    sreply.write(fout);
}
