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

#include <unistd.h>
#include <sys/wait.h>
#include <time.h>

#include "db.h"
#include "backend_lib.h"
#include "parse.h"
#include "server_types.h"
#include "handle_request.h"

// return true if the WU can be executed on the host
//
bool wu_is_feasible(WORKUNIT& wu, HOST& host) {
    if (wu.rsc_disk > host.d_free) return false;
    if (wu.rsc_memory > host.m_nbytes) return false;
    return true;
}

// estimate the time that a WU will take on a host
//
double estimate_duration(WORKUNIT& wu, HOST& host) {
    return wu.rsc_fpops/host.p_fpops + wu.rsc_iops/host.p_iops;
}

// add the given workunit to a reply.
// look up its app, and make sure there's a version for this platform.
// Add the app and app_version to the reply also.
//
int add_wu_to_reply(
    WORKUNIT& wu, SCHEDULER_REPLY& reply, PLATFORM& platform, DB_CACHE& db
) {
    APP* app;
    APP_VERSION* app_version;

    app = db.lookup_app(wu.appid);
    if (!app) return -1;
    app_version = db.lookup_app_version(app->id, platform.id, app->prod_vers);
    if (!app_version) return -1;

    // add the app, app_version, and workunit to the reply,
    // but only if they aren't already there

    reply.insert_app_unique(*app);
    reply.insert_app_version_unique(*app_version);
    reply.insert_workunit_unique(wu);
    return 0;
}

// Look up the host and its user, and make sure the authenticator matches.
// If no host ID is supplied, or if RPC seqno mismatch,
// create a new host record and return its ID
//
int authenticate_user(
    SCHEDULER_REQUEST& sreq, SCHEDULER_REPLY& reply, USER& user, HOST& host
) {
    int retval;

    if (sreq.hostid) {
        retval = db_host(sreq.hostid, host);
        if (retval) {
            strcpy(reply.error_message, "invalid hostid");
            return -1;
        }
        retval = db_user(host.userid, user);
        if (retval) {
            strcpy(reply.error_message, "internal error - no such user");
            return -1;
        }
        if (strcmp(sreq.authenticator, user.authenticator)) {
            strcpy(reply.error_message,
               "Invalid or missing authenticator.  "
               "Visit this project's web site to request your authenticator, "
               "then cut and paste it into your accounts file."
            );
            return -1;
        }

        // If the seqno from the host is less than what we expect,
        // the user must have copied the state file to a different host.
        // Make a new host record.
        if (sreq.rpc_seqno < host.rpc_seqno) {
            sreq.hostid = 0;
            goto new_host;
        }
        host.rpc_seqno = sreq.rpc_seqno;
        host.rpc_time = time(0);
    } else {
        strcpy(user.authenticator, sreq.authenticator);
        retval = db_user_lookup_auth(user);
        if (retval) {
            strcpy(reply.error_message, "invalid or missing authenticator");
            return -1;
        }
new_host:
        host = sreq.host;
        host.id = 0;
        host.create_time = time(0);
        host.userid = user.id;
        host.prefsid = user.default_prefsid;
        host.rpc_seqno = 0;
        host.rpc_time = time(0);
        retval = db_host_new(host);
        if (retval) {
            strcpy(reply.error_message, "DB operation failed");
            db_print_error("db_host_new");
            return -1;
        }
        host.id = db_insert_id();
        reply.hostid = host.id;
    }
    return 0;
}

// Update host record based on request.
// Copy all fields that are determined by the client.
//
int update_host_record(SCHEDULER_REQUEST& sreq, HOST& host) {
    int retval;

    host.timezone = sreq.host.timezone;
    strcpy(host.domain_name, sreq.host.domain_name);
    strcpy(host.serialnum, sreq.host.serialnum);
    if (strcmp(host.last_ip_addr, sreq.host.last_ip_addr)) {
        strcpy(host.last_ip_addr, sreq.host.last_ip_addr);
    } else {
        host.nsame_ip_addr++;
    }
    host.on_frac = sreq.host.on_frac;
    host.connected_frac = sreq.host.connected_frac;
    host.active_frac = sreq.host.active_frac;
    host.p_ncpus = sreq.host.p_ncpus;
    strcpy(host.p_vendor, sreq.host.p_vendor);    // unlikely this will change
    strcpy(host.p_model, sreq.host.p_model);
    host.p_fpops = sreq.host.p_fpops;
    host.p_iops = sreq.host.p_iops;
    host.p_membw = sreq.host.p_membw;
    strcpy(host.os_name, sreq.host.os_name);
    strcpy(host.os_version, sreq.host.os_version);
    host.m_nbytes = sreq.host.m_nbytes;
    host.m_cache = sreq.host.m_cache;
    host.m_swap = sreq.host.m_swap;
    host.d_total = sreq.host.d_total;
    host.d_free = sreq.host.d_free;
    host.n_bwup = sreq.host.n_bwup;
    host.n_bwdown = sreq.host.n_bwdown;
    retval = db_host_update(host);
    if (retval) {
        fprintf(stderr, "db_host_update: %d\n", retval);
    }
    return 0;
}

// If the client asked for preferences,
// see if they have changed, and if so send.
//
int send_prefs(SCHEDULER_REQUEST& sreq, SCHEDULER_REPLY& reply, HOST& host) {
    int retval;

    if (sreq.want_prefs) {
        if (host.prefsid) {
            retval = db_prefs(host.prefsid, reply.prefs);
            if (retval) {
                fprintf(stderr, "can't find prefs for host %d\n", host.id);
            } else {
                if (reply.prefs.modified_time > sreq.prefs_mod_time) {
                    reply.send_prefs = true;
                }
            }
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
    RESULT result, *rp;
    WORKUNIT wu;

    for (i=0; i<sreq.results.size(); i++) {
        rp = &sreq.results[i];
        strcpy(result.name, rp->name);
        retval = db_result_lookup_name(result);
        if (retval) {
            printf("can't find result %s\n", rp->name);
        } else {
            if (result.state != RESULT_STATE_IN_PROGRESS) {
                printf("got unexpected result for %s\n", rp->name);
                continue;
            }

            // TODO: handle error returns
            //
            result.hostid = host.id;
            result.received_time = time(0);
            result.exit_status = rp->exit_status;
            result.cpu_time = rp->cpu_time;
            result.state = RESULT_STATE_DONE;
            strcpy(result.stderr_out, rp->stderr_out);
            strcpy(result.xml_doc_out, rp->xml_doc_out);
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

// KLUDGE - query database for WUs.
// this must replaced for efficiency.
//
int send_work(
    SCHEDULER_REQUEST& sreq, SCHEDULER_REPLY& reply, PLATFORM& platform,
    HOST& host, DB_CACHE& db
) {
    int retval, nresults = 0;
    WORKUNIT wu;
    RESULT result;
    APP* app;
    char prefix [256];

    while (!db_result_enum_to_send(result, 1)) {
        db_workunit(result.workunitid, wu);
        retval = add_wu_to_reply(wu, reply, platform, db);
        if (retval) continue;
        reply.insert_result(result);
        nresults++;

        result.state = RESULT_STATE_IN_PROGRESS;
        result.hostid = host.id;
        result.sent_time = time(0);
        db_result_update(result);

        wu.nresults_unsent--;
        db_workunit_update(wu);
    }

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
        result.hostid = host.id;
        result.sent_time = time(0);
        sprintf(result.name, "result_%d", result.id);
        app = db.lookup_app(wu.appid);
        strcpy(result.xml_doc_in, app->result_xml_template);
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

    if (nresults == 0) {
        strcpy(reply.error_message, "no work available");
        reply.request_delay = 10;
    }
    return 0;
}

void process_request(
    SCHEDULER_REQUEST& sreq, SCHEDULER_REPLY& reply, DB_CACHE& db
) {
    HOST host;
    USER user;
    PLATFORM* platform;
    int retval;
    char buf[256];

    retval = authenticate_user(sreq, reply, user, host);
    if (retval) return;

    retval = update_host_record(sreq, host);

    // look up the client's platform in the DB
    // the following should be replaced with in-memory equivalents
    //
    platform = db.lookup_platform(sreq.platform_name);
    if (!platform) {
        sprintf(buf, "platform %s not found", sreq.platform_name);
        strcpy(reply.error_message, buf);
        return;
    }

    send_prefs(sreq, reply, host);

    handle_results(sreq, reply, host);

    send_work(sreq, reply, *platform, host, db);
}

void handle_request(FILE* fin, FILE* fout, DB_CACHE& db) {
    SCHEDULER_REQUEST sreq;
    SCHEDULER_REPLY sreply;

    memset(&sreq, 0, sizeof(sreq));
    sreq.parse(fin);

    process_request(sreq, sreply, db);

    sreply.write(fout);
}

