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

// "Locality scheduling": a scheduling discipline in which
// there are large sticky input files,
// and many WUs share a single input file.
// If a host already has an input file,
// we try to send him another result that uses that file.
//
// see doc/sched_locality.php


#include <stdio.h>
#include <unistd.h>

#include "boinc_db.h"

#include "main.h"
#include "server_types.h"
#include "sched_shmem.h"
#include "sched_send.h"
#include "sched_msgs.h"
#include "sched_locality.h"

// get filename from result name
//
static int extract_filename(char* in, char* out) {
    strcpy(out, in);
    char* p = strstr(out, "__");
    if (!p) return -1;
    *p = 0;
    return 0;
}

// Find the app and app_version for the client's platform.
//
static int get_app_version(
    WORKUNIT& wu, APP* &app, APP_VERSION* &avp,
    SCHEDULER_REQUEST& sreq, SCHEDULER_REPLY& reply, PLATFORM& platform,
    WORK_REQ& wreq, SCHED_SHMEM& ss
) {
    bool found;
    if (anonymous(platform)) {
        app = ss.lookup_app(wu.appid);
        found = sreq.has_version(*app);
        if (!found) {
            return -1;
        }
        avp = NULL;
    } else {
        found = find_app_version(wreq, wu, platform, ss, app, avp);
        if (!found) {
            return -1;
        }

        // see if the core client is too old.
        // don't bump the infeasible count because this
        // isn't the result's fault
        //
        if (!app_core_compatible(wreq, *avp)) {
            return -1;
        }
    }
    return 0;
}

// possibly send the client this result
//
static int possibly_send_result(
    DB_RESULT& result,
    SCHEDULER_REQUEST& sreq, SCHEDULER_REPLY& reply, PLATFORM& platform,
    WORK_REQ& wreq, SCHED_SHMEM& ss
) {
    DB_WORKUNIT wu;
    DB_RESULT result2;
    int retval, count;
    char buf[256];
    APP* app;
    APP_VERSION* avp;

    if (config.one_result_per_user_per_wu) {
        retval = wu.lookup_id(result.workunitid);
        if (retval) return retval;
        sprintf(buf, "where userid=%d and workunitid=%d", reply.user.id, wu.id);
        retval = result2.count(count, buf);
        if (retval) return retval;
        if (count > 0) return -1;
    }

    retval = get_app_version(
        wu, app, avp,
        sreq, reply, platform, wreq, ss
    );
    if (retval) return retval;

    retval = add_result_to_reply(result, wu, reply, platform, wreq, app, avp);
    if (retval) return retval;
    return 0;
}

// Check with the WU generator to see if we can make some
// more WU for this file.
//
void make_more_work_for_file(char* filename) {
    char fullpath[512], buf[256];
    DB_RESULT result;
    int retval;

    sprintf(fullpath, "../locality_scheduling/no_work_available/%s", filename);
    FILE *fp=fopen(fullpath, "r");
    if (fp) {
        // since we found this file, it means that no work
        // remains for this WU.  So give up trying to interact
        // with the WU generator.
        fclose(fp);
        log_messages.printf(
            SCHED_MSG_LOG::DEBUG,
            "found %s indicating no work remaining for file %s\n", fullpath, filename
        );
    } else {
        // open and touch a file in the need_work/
        // directory as a way of indicating that we need work for this file.
        // If this operation fails, don't worry or tarry!
        //
        sprintf(fullpath, "../locality_scheduling/need_work/%s", filename);
        FILE *fp2=fopen(fullpath, "w");
        if (fp2) {
            fclose(fp2);
            log_messages.printf(
                SCHED_MSG_LOG::DEBUG,
                "touching %s: need work for file %s\n", fullpath, filename
            );
            // Finish the transaction, wait for the WU
            // generator to make a new WU, and try again!
            //
            boinc_db.commit_transaction();
            sleep(config.locality_scheduling_wait_period);

            // Now look AGAIN for results which match file
            // 'filename'.
            //
            sprintf(buf,
                "where name like '%s__%%' and server_state=%d limit 1",
                filename, RESULT_SERVER_STATE_UNSENT
            );
            boinc_db.start_transaction();
            retval = result.lookup(buf);
            if (retval) {
                log_messages.printf(
                    SCHED_MSG_LOG::DEBUG,
                    "project didn't make NEW work for file %s in time\n", filename
                );
            } else {
                log_messages.printf(
                    SCHED_MSG_LOG::DEBUG,
                    "success making/finding NEW work for file %s\n", filename
                );
            }
        } else {
            log_messages.printf(
                SCHED_MSG_LOG::CRITICAL,
                "unable to touch %s to indicate need work for file %s\n", fullpath, filename
            );
        }
    }
}

// The client has (or soon will have) the given file.
// Try to send it more results that use the same file
//
static int send_results_for_file(
    char* filename,
    int& nsent,
    SCHEDULER_REQUEST& sreq, SCHEDULER_REPLY& reply, PLATFORM& platform,
    WORK_REQ& wreq, SCHED_SHMEM& ss
) {
    DB_RESULT result;
    int retval=0, lastid=0, i, last_wuid=0;
    char buf[256];

    nsent = 0;
    for (i=0; i<100; i++) {     // avoid infinite loop
        if (!wreq.work_needed(reply)) break;
        boinc_db.start_transaction();
        // Look for results which match file 'filename'

        sprintf(buf,
            "where name like '%s__%%' and server_state=%d and workunitid<>%d limit 1",
            filename, RESULT_SERVER_STATE_UNSENT, last_wuid
        );
        retval = result.lookup(buf);

        // if we see the same result twice, bail (avoid spinning)
        //
        if (!retval && (result.id == lastid)) retval = -1;

        if (retval) {
            if (config.locality_scheduling_wait_period) {
                make_more_work_for_file(filename);
            }
        } else {
            // We found a matching result.
            // Probably we will get one of these,
            // although for example if we already have a
            // result for the same workunit and the administrator has
            // set one_result_per_wu then we won't get one of these.
            //
            lastid = result.id;
            retval = possibly_send_result(
                result,
                sreq, reply, platform, wreq, ss
            );
            if (!retval) {
                if (config.one_result_per_user_per_wu) {
                    last_wuid = result.workunitid;
                }
                nsent++;
            }
        }

        boinc_db.commit_transaction();
        if (retval) break;
    }
    return 0;
}

// The client doesn't have any files for which work is available.
// Pick new file(s) to send.
//
static void send_new_file_work(
    SCHEDULER_REQUEST& sreq, SCHEDULER_REPLY& reply, PLATFORM& platform,
    WORK_REQ& wreq, SCHED_SHMEM& ss
) {
    int lookup_retval, retval, send_retval;
    int lastid=0, nsent, last_wuid=0, i;
    DB_RESULT result;
    char filename[256];
    char buf[256];

    for (i=0; i<100; i++) {     // avoid infinite loop
        if (!wreq.work_needed(reply)) break;
        boinc_db.start_transaction();
        sprintf(buf,
            "where server_state=%d and workunitid<>%d limit 1",
            RESULT_SERVER_STATE_UNSENT, last_wuid
        );
        lookup_retval = result.lookup(buf);

        // if we see the same result twice, bail (avoid spinning)
        //
        if (!lookup_retval && (result.id == lastid)) lookup_retval = -1;

        if (!lookup_retval) {
            lastid = result.id;
            send_retval = possibly_send_result(
                result,
                sreq, reply, platform, wreq, ss
            );
        log_messages.printf(SCHED_MSG_LOG::DEBUG, "possibly_send_result() gives retval=%d\n", send_retval);

            if (config.one_result_per_user_per_wu) {
                last_wuid = result.workunitid;
            }
        }

        boinc_db.commit_transaction();

        log_messages.printf(SCHED_MSG_LOG::DEBUG, "lastid=%d last_wuid=%d\n", lastid, last_wuid);

        if (lookup_retval) break;
        if (send_retval) continue;

        // try to send more result w/ same file
        //
        retval = extract_filename(result.name, filename);
        if (retval) {
            log_messages.printf(
                SCHED_MSG_LOG::DEBUG,
                "result doesn't contain filename: %s\n", result.name
            );
            continue;
        }
        send_results_for_file(
            filename, nsent, sreq, reply, platform, wreq, ss
        );
    }
}

void send_work_locality(
    SCHEDULER_REQUEST& sreq, SCHEDULER_REPLY& reply, PLATFORM& platform,
    WORK_REQ& wreq, SCHED_SHMEM& ss
) {
    unsigned int i;
    int retval, nsent;

    if (sreq.file_infos.size() == 0) {
        send_new_file_work(
            sreq, reply, platform, wreq, ss
        );
    } else {
        for (i=0; i<sreq.file_infos.size(); i++) {
            if (!wreq.work_needed(reply)) break;
            FILE_INFO& fi = sreq.file_infos[i];
            retval = send_results_for_file(
                fi.name, nsent, sreq, reply, platform, wreq, ss
            );

            // if we couldn't send any work for this file, tell client
            // to delete it
            //
            if (nsent == 0) {
                reply.file_deletes.push_back(fi);
            }
        }

        // send new files if needed to satisfy work request
        //
        if (wreq.work_needed(reply)) {
            send_new_file_work(
                sreq, reply, platform, wreq, ss
            );
        }
    }
}

const char *BOINC_RCSID_238cc1aec4 = "$Id$";
