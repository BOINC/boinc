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
#include "parse.h"
#include "util.h"
#include "server_types.h"
#include "sched_util.h"
#include "main.h"
#include "handle_request.h"

#define MIN_SECONDS_TO_SEND 0
#define MAX_SECONDS_TO_SEND (28*SECONDS_PER_DAY)
#define MAX_WUS_TO_SEND     10

// return true if the WU can be executed on the host
//
bool wu_is_feasible(WORKUNIT& wu, HOST& host) {
    if(host.d_free && wu.rsc_disk > host.d_free) {
        log_messages.printf(
            SchedMessages::DEBUG, "[WU#%d] needs %f disk; HOST#%d has %f\n",
            wu.id, wu.rsc_disk, host.id, host.d_free
        );
        return false;
    }
    if (host.m_nbytes && wu.rsc_memory > host.m_nbytes) {
        log_messages.printf(
            SchedMessages::DEBUG, "WU#%d needs %f mem; HOST#%d has %f\n",
            wu.id, wu.rsc_memory, host.id, host.m_nbytes
        );
        return false;
    }
    return true;
}

// estimate the time that a WU will take on a host
// TODO: improve this.  take memory bandwidth into account
//
double estimate_duration(WORKUNIT& wu, HOST& host) {
    if (host.p_fpops <= 0) host.p_fpops = 1e9;
    if (host.p_iops <= 0) host.p_iops = 1e9;
    if (wu.rsc_fpops <= 0) wu.rsc_fpops = 1e12;
    if (wu.rsc_iops <= 0) wu.rsc_iops = 1e12;
    return wu.rsc_fpops/host.p_fpops + wu.rsc_iops/host.p_iops;
}

// insert "text" right after "after" in the given buffer
//
int insert_after(char* buffer, char* after, char* text) {
    char* p;
    char temp[MAX_BLOB_SIZE];

    if (strlen(buffer) + strlen(text) > MAX_BLOB_SIZE-1) {
        log_messages.printf(SchedMessages::NORMAL, "insert_after: overflow\n");
        return -1;
    }
    p = strstr(buffer, after);
    if (!p) {
        log_messages.printf(SchedMessages::CRITICAL, "insert_after: %s not found in %s\n", after, buffer);
        return -1;
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
    char buf[256];

    sprintf(buf,
        "    <rsc_fpops>%f</rsc_fpops>\n"
        "    <rsc_iops>%f</rsc_iops>\n"
        "    <rsc_memory>%f</rsc_memory>\n"
        "    <rsc_disk>%f</rsc_disk>\n"
        "    <name>%s</name>\n"
        "    <app_name>%s</app_name>\n",
        wu.rsc_fpops,
        wu.rsc_iops,
        wu.rsc_memory,
        wu.rsc_disk,
        wu.name,
        app.name
    );
    return insert_after(wu.xml_doc, "<workunit>\n", buf);
}

void parse_project_prefs(char* prefs, vector<APP_FILE>& app_files) {
    char buf[256];
    APP_FILE af;

    while (sgets(buf, 256, prefs)) {
        if (match_tag(buf, "<app_file>")) {
            af.parse(prefs);
            app_files.push_back(af);
        }
    }
}

// Handle user-specified app files, e.g. background graphics:
// if the user's project prefs include elements of the form
// <app_file>
//     <url>X</url>
//     <open_name>Y</open_name>
//     <timestamp>Z</timestamp>
// </app_file>
// then insert corresponding elements in app_version XML doc, namely:
// <file_info>
//     <name>Y_Z</name>
//     <url>X</url>
// </file_info>
// ... (in the <app_version> element)
//     <file_ref>
//         <file_name>Y_Z</file_name>
//         <open_name>Y</open_name>
//         <optional deadline=300/>
//     </file_ref>
// Notes:
// - the timestamp allows you to force a re-download of the file
//   by updating your prefs;
// - the <optional/> allows the app to start after 300 secs
//   even if the file hasn't been successfully downloaded
//
int insert_app_file_tags(APP_VERSION& av, USER& user) {
    vector<APP_FILE> app_files;
    APP_FILE af;
    unsigned int i;
    char buf[256], name[256];
    int retval;

    parse_project_prefs(user.project_prefs, app_files);
    for (i=0; i<app_files.size(); i++) {
        af = app_files[i];
        sprintf(name, "%s_%d", af.open_name, af.timestamp);
        sprintf(buf,
            "<file_info>\n"
            "    <name>%s</name>\n"
            "    <url>%s</url>\n"
            "</file_info>\n",
            name, af.url
        );
        retval = insert_after(av.xml_doc, "", buf);
        if (retval) return retval;
        sprintf(buf,
            "    <file_ref>\n"
            "        <file_name>%s</file_name>\n"
            "        <open_name>%s</open_name>\n"
            "        <optional deadline=300/>\n"
            "    </file_ref>\n",
            name, af.open_name
        );
        retval = insert_after(av.xml_doc, "<app_version>\n", buf);
        if (retval) return retval;
    }
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
    APP_VERSION* avp, app_version;
    int retval;
    WORKUNIT wu2;

    app = ss.lookup_app(wu.appid);
    if (!app) {
        log_messages.printf(
            SchedMessages::CRITICAL, "Can't find APP#%d\n", wu.appid
        );
        return -1;
    }
    avp = ss.lookup_app_version(app->id, platform.id, app->min_version);
    if (!avp) {
        log_messages.printf(
            SchedMessages::CRITICAL,
            "Can't find app version: APP#%d PLATFORM#%d min_version %d\n",
            app->id, platform.id, app->min_version
        );
        return -1;
    }

    // add the app, app_version, and workunit to the reply,
    // but only if they aren't already there
    //
    reply.insert_app_unique(*app);

    // If the user's project prefs include any <app_file> tags,
    // make appropriate modifications to the app_version XML
    // DO THIS IN A COPY OF THE STRUCTURE
    //
    app_version = *avp;
    retval = insert_app_file_tags(app_version, reply.user);
    if (retval) {
        log_messages.printf(SchedMessages::NORMAL, "insert_app_file_tags failed\n");
        return retval;
    }

    reply.insert_app_version_unique(app_version);

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
            goto new_host;
        }
        reply.host = host;

        retval = user.lookup_id(reply.host.userid);
        if (retval) {
            strcpy(reply.message, "Can't find user record");
            strcpy(reply.message_priority, "low");
            reply.request_delay = 120;
            reply.nucleus_only = true;
            log_messages.printf(
                SchedMessages::NORMAL,
                "[HOST#%d] [USER#%d?] can't find user record\n",
                host.id, reply.host.userid
            );
            return -1;
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
        // here no hostid was given; we'll have to create a new host record
        //
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
            return -1;
        }
        reply.user = user;
new_host:
        // reply.user is filled in and valid at this point
        //
        host = sreq.host;
        host.id = 0;
        host.create_time = time(0);
        host.userid = reply.user.id;
        host.rpc_seqno = 0;
        host.rpc_time = time(0);
        strcpy(host.venue, reply.user.venue);
        retval = host.insert();
        if (retval) {
            strcpy(reply.message, "server database error");
            strcpy(reply.message_priority, "low");
            boinc_db_print_error("host.insert()");
            log_messages.printf(SchedMessages::CRITICAL, "host.insert() failed\n");
            return -1;
        }
        host.id = boinc_db_insert_id();

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
        (fabs(host.p_fpops)/1e9 + fabs(host.p_iops)/1e9 + fabs(host.p_membw)/4e9)/(3*SECONDS_PER_DAY);
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
        if (result.server_state == RESULT_SERVER_STATE_OVER) {
            result.file_delete_state = FILE_DELETE_READY;
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

        result.hostid = reply.host.id;
        result.received_time = time(0);
        result.client_state = rp->client_state;
        result.cpu_time = rp->cpu_time;
        result.claimed_credit = result.cpu_time * host.credit_per_cpu_sec;
        result.server_state = RESULT_SERVER_STATE_OVER;
        // TODO: if client application aborted e.g. exceeded resource limits,
        // should client_state be RESULT_OUTCOME_CLIENT_ERROR ?
        if (result.client_state == RESULT_FILES_UPLOADED) {
            result.outcome = RESULT_OUTCOME_SUCCESS;
            retval = wu.lookup_id(result.workunitid);
            if (retval) {
                log_messages.printf(
                    SchedMessages::CRITICAL,
                    "[HOST#%d] [RESULT#%d %s] Can't find [WU#%d] for result\n",
                    host.id, result.id, result.name, result.workunitid
                );
            } else {
                wu.need_validate = 1;
                retval = wu.update();
                if (retval) {
                    log_messages.printf(
                        SchedMessages::CRITICAL,
                        "[HOST#%d] [RESULT#%d %s] Can't update [WU#%d %s]\n",
                        host.id, result.id, result.name, wu.id, wu.name
                    );
                }
            }
        } else {
            result.outcome = RESULT_OUTCOME_CLIENT_ERROR;
            result.validate_state = VALIDATE_STATE_INVALID;
        }

        strncpy(result.stderr_out, rp->stderr_out, sizeof(result.stderr_out));
        strncpy(result.xml_doc_out, rp->xml_doc_out, sizeof(result.xml_doc_out));
        result.client_version_num =
            sreq.core_client_major_version*100 + sreq.core_client_minor_version;
        retval = result.update();
        if (retval) {
            log_messages.printf(
                SchedMessages::NORMAL,
                "[HOST#%d] [RESULT#%d %s] can't update result: %s\n",
                host.id, result.id, result.name, boinc_db_error_string()
            );
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

int send_work(
    SCHEDULER_REQUEST& sreq, SCHEDULER_REPLY& reply, PLATFORM& platform,
    SCHED_SHMEM& ss
) {
    int i, retval, nresults = 0, seconds_to_fill;
    WORKUNIT wu;
    DB_RESULT result, result_copy;

    if (sreq.work_req_seconds <= 0) return 0;

    log_messages.printf(
        SchedMessages::NORMAL,
        "[HOST#%d] got request for %d seconds of work\n",
        reply.host.id, sreq.work_req_seconds
    );

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
        if (!ss.wu_results[i].present) {
            continue;
        }
        wu = ss.wu_results[i].workunit;
        if (!wu_is_feasible(wu, reply.host)) {
            log_messages.printf(
                SchedMessages::DEBUG, "[HOST#%d] [WU#%d %s] WU is infeasible\n",
                reply.host.id, wu.id, wu.name
            );
            continue;
        }

        result = ss.wu_results[i].result;
        ss.wu_results[i].present = false;

        retval = add_wu_to_reply(wu, reply, platform, ss,
            estimate_duration(wu, reply.host)
        );
        if (retval) continue;

        int wu_seconds_filled = (int) estimate_duration(wu, reply.host);

        log_messages.printf(
            SchedMessages::NORMAL,
            "[HOST#%d] Sending [RESULT#%d %s] (fills %d seconds)\n",
            reply.host.id, result.id, result.name, wu_seconds_filled
        );

        result.server_state = RESULT_SERVER_STATE_IN_PROGRESS;
        result.hostid = reply.host.id;
        result.sent_time = time(0);
        result.report_deadline = result.sent_time + wu.delay_bound;
        result.update();

        // copy the result so we don't overwrite its XML fields
        //
        result_copy = result;

        retval = insert_name_tags(result_copy, wu);
        if (retval) {
            log_messages.printf(SchedMessages::CRITICAL, "send_work: can't insert name tags\n");
        }
        retval = insert_deadline_tag(result_copy);
        if (retval) {
            log_messages.printf(SchedMessages::CRITICAL, "send_work: can't insert deadline tag\n");
        }
        reply.insert_result(result_copy);

        seconds_to_fill -= wu_seconds_filled;

        nresults++;
        if (nresults == MAX_WUS_TO_SEND) break;
    }

    log_messages.printf(
        SchedMessages::NORMAL, "[HOST#%d] Sent %d results\n",
        reply.host.id, nresults
    );

    if (nresults == 0) {
        strcpy(reply.message, "no work available");
        strcpy(reply.message_priority, "low");
        reply.request_delay = 10;

        log_messages.printf(
            SchedMessages::NORMAL, "[HOST#%d] No work available\n",
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
            log_messages.printf(SchedMessages::NORMAL, "received old code sign key\n");

            // look for a signature file
            //
            for (i=0; ; i++) {
                sprintf(path, "%s/old_key_%d", config.key_dir, i);
                retval = read_file_malloc(path, oldkey);
                if (retval) {
                    strcpy(reply.message,
                        "Can't update code signing key.  "
                        "Please report this to project."
                    );
                    return;
                }
                if (!strcmp(oldkey, sreq.code_sign_key)) {
                    sprintf(path, "%s/signature_%d", config.key_dir, i);
                    retval = read_file_malloc(path, signature);
                    if (retval) {
                        strcpy(reply.message,
                            "Can't update code signing key.  "
                            "Please report this to project."
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
            "[HOST#%d] Wrong major version '%s' from user: wanted %d, got %d\n",
            reply.host.id,
            sreq.authenticator, MAJOR_VERSION, sreq.core_client_major_version
        );
        return true;
    }
    return false;
}

inline static const char* get_remote_addr()
{
    const char * r = getenv("REMOTE_ADDR");
    return r ? r : "?.?.?.?";
}

void process_request(
    SCHEDULER_REQUEST& sreq, SCHEDULER_REPLY& reply, SCHED_SHMEM& ss,
    char* code_sign_key
) {
    PLATFORM* platform;
    int retval;

    // if different major version of BOINC, just send a message
    //
    if (wrong_major_version(sreq, reply)) return;

    retval = authenticate_user(sreq, reply);
    if (retval) return;

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

    send_work(sreq, reply, *platform, ss);

    send_code_sign_key(sreq, reply, code_sign_key);
}

void handle_request(
    FILE* fin, FILE* fout, SCHED_SHMEM& ss, char* code_sign_key
) {
    SCHEDULER_REQUEST sreq;
    SCHEDULER_REPLY sreply;

    log_messages.printf(
        SchedMessages::NORMAL, "Handling request from %s\n",
        get_remote_addr()
    );
    memset(&sreq, 0, sizeof(sreq));
    sreq.parse(fin);
    process_request(sreq, sreply, ss, code_sign_key);
    sreply.write(fout);
}
