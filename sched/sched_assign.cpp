// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2023 University of California
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
//

// scheduler functions to send assigned jobs.

#include "config.h"

#include "boinc_stdio.h"

#include <sys/param.h>
#include <unistd.h>

#include "backend_lib.h"
#include "boinc_db.h"
#include "crypt.h"
#include "error_numbers.h"
#include "filesys.h"

#include "sched_check.h"
#include "sched_main.h"
#include "sched_msgs.h"
#include "sched_send.h"
#include "sched_version.h"
#include "sched_types.h"

#include "sched_assign.h"

// The workunit is targeted to the host (or user or team).
// Decide if we should actually send an instance
//
bool need_targeted_instance(WORKUNIT& wu, int hostid) {

    // don't send if WU had error or was canceled
    // (db_purge will eventually delete WU and assignment records)
    //
    if (wu.error_mask) {
        return false;
    }

    // don't send if WU is validation pending or completed,
    // or has transition pending
    //
    if (wu.need_validate) return false;
    if (wu.canonical_resultid) return false;
    if (wu.transition_time < time(0)) return false;

    // See if this WU needs another instance.
    // This replicates logic in the transitioner
    //
    char buf[256];
    DB_RESULT result;
    int nunsent=0, ninprogress=0, nsuccess=0;
    sprintf(buf, "where workunitid=%lu", wu.id);
    while (!result.enumerate(buf)) {
        // send at most 1 instance to a given host
        //
        if (result.hostid == hostid) {
            return false;
        }
        switch (result.server_state) {
        case RESULT_SERVER_STATE_INACTIVE:
        case RESULT_SERVER_STATE_UNSENT:
            nunsent++;
            break;
        case RESULT_SERVER_STATE_IN_PROGRESS:
            ninprogress++;
            break;
        case RESULT_SERVER_STATE_OVER:
            if (result.outcome == RESULT_OUTCOME_SUCCESS
                && result.validate_state != VALIDATE_STATE_INVALID
            ) {
                nsuccess++;
            }
            break;
        }
    }
    int needed = wu.target_nresults - nunsent - ninprogress - nsuccess;
    if (needed <= 0) return false;

    return true;
}

// send a job for the given assignment
//
static int send_assigned_job(ASSIGNMENT& asg) {
    int retval;
    DB_WORKUNIT wu;
    char suffix[256], path[MAXPATHLEN];
    const char *rtfpath;
    static bool first=true;
    static int seqno=0;
    static R_RSA_PRIVATE_KEY key;
    BEST_APP_VERSION* bavp;

    if (first) {
        first = false;
        sprintf(path, "%s/upload_private", config.key_dir);
        retval = read_key_file(path, key);
        if (retval) {
            log_messages.printf(MSG_CRITICAL, "can't read key\n");
            return -1;
        }

    }
    retval = wu.lookup_id(asg.workunitid);
    if (retval) {
        log_messages.printf(MSG_CRITICAL,
            "assigned WU %lu not found\n", asg.workunitid
        );
        return retval;
    }

    if (app_not_selected(wu.appid)) {
        log_messages.printf(MSG_CRITICAL,
            "Assigned WU %s is for app not selected by user\n", wu.name
        );
        return -1;
    }

    bavp = get_app_version(wu, false, false);
    if (!bavp) {
        log_messages.printf(MSG_CRITICAL,
            "App version for assigned WU not found\n"
        );
        return ERR_NOT_FOUND;
    }

    rtfpath = config.project_path("%s", wu.result_template_file);
    sprintf(suffix, "%d_%d_%d", getpid(), (int)time(0), seqno++);
    retval = create_result(
        wu, const_cast<char*>(rtfpath), suffix, key, config, 0, 0
    );
    if (retval) {
        log_messages.printf(MSG_CRITICAL,
            "[WU#%lu %s] create_result(): %s\n", wu.id, wu.name, boincerror(retval)
        );
        return retval;
    }
    DB_ID_TYPE result_id = boinc_db.insert_id();
    SCHED_DB_RESULT result;
    retval = result.lookup_id(result_id);
    HOST_USAGE hu;
    BUDA_VARIANT *bvp = NULL;
    if (is_buda(wu)) {
        if (!choose_buda_variant(wu, -1, &bvp, hu)) {
            return -1;
        }
    } else {
        hu = bavp->host_usage;
    }
    add_result_to_reply(result, wu, bavp, hu, bvp, false);

    if (config.debug_assignment) {
        log_messages.printf(MSG_NORMAL,
            "[assign] [WU#%lu] [RESULT#%lu] [HOST#%lu] send assignment %lu\n",
            wu.id, result_id, g_reply->host.id, asg.id
        );
    }
    return 0;
}

// Send this host any broadcast jobs.
// Return true iff we sent anything
//
bool send_broadcast_jobs() {
    DB_RESULT result;
    int retval;
    char buf[256];
    bool sent_something = false;

    for (int i=0; i<ssp->nassignments; i++) {
        ASSIGNMENT& asg = ssp->assignments[i];

        if (config.debug_assignment) {
            log_messages.printf(MSG_NORMAL,
                "[assign] processing broadcast type %d\n",
                asg.target_type
            );
        }
        // see if this assignment applies to this host
        //
        switch (asg.target_type) {
        case ASSIGN_NONE:
            sprintf(buf, "where hostid=%lu and workunitid=%lu",
                g_reply->host.id, asg.workunitid
            );
            retval = result.lookup(buf);
            if (retval == ERR_DB_NOT_FOUND) {
                retval = send_assigned_job(asg);
                if (!retval) sent_something = true;
            }
            break;
        case ASSIGN_USER:
            if (g_reply->user.id != asg.target_id) continue;
            sprintf(buf, "where workunitid=%lu and hostid=%lu",
                asg.workunitid, g_reply->host.id
            );
            retval = result.lookup(buf);
            if (retval == ERR_DB_NOT_FOUND) {
                retval = send_assigned_job(asg);
                if (!retval) sent_something = true;
            }
            break;
        case ASSIGN_TEAM:
            if (g_reply->team.id != asg.target_id) continue;
            sprintf(buf, "where workunitid=%lu and hostid=%lu",
                asg.workunitid, g_reply->host.id
            );
            retval = result.lookup(buf);
            if (retval == ERR_DB_NOT_FOUND) {
                retval = send_assigned_job(asg);
                if (!retval) sent_something = true;
            }
            break;
        }
    }
    return sent_something;
}

// Send targeted jobs of a given type.
// NOTE: there may be an atomicity problem in the following.
// Ideally it should be in a transaction.
//
bool send_jobs(int assign_type) {
    DB_ASSIGNMENT asg;
    DB_RESULT result;
    DB_WORKUNIT wu;
    int retval;
    bool sent_something = false;
    char query[256];

    switch (assign_type) {
    case ASSIGN_USER:
        sprintf(query, "where target_type=%d and target_id=%lu and multi=0",
            ASSIGN_USER, g_reply->user.id
        );
        break;
    case ASSIGN_HOST:
        sprintf(query, "where target_type=%d and target_id=%lu and multi=0",
            ASSIGN_HOST, g_reply->host.id
        );
        break;
    case ASSIGN_TEAM:
        sprintf(query, "where target_type=%d and target_id=%lu and multi=0",
            ASSIGN_TEAM, g_reply->team.id
        );
        break;
    }

    while (!asg.enumerate(query)) {
        if (!work_needed(false)) {
            asg.end_enumerate();
            break;
        }

        // if the WU doesn't exist, delete the assignment record.
        //
        retval = wu.lookup_id(asg.workunitid);
        if (retval) {
            asg.delete_from_db();
            continue;
        }

        if (!need_targeted_instance(wu, g_reply->host.id)) {
            continue;
        }

        // OK, send the job
        //
        if (config.debug_send) {
            log_messages.printf(MSG_NORMAL,
                "sending targeted job: %s\n", wu.name
            );
        }
        retval = send_assigned_job(asg);
        if (retval) {
            log_messages.printf(MSG_NORMAL,
                "failed to send targeted job: %s\n", boincerror(retval)
            );
            continue;
        }

        sent_something = true;

        // update the WU's transition time to time out this job
        //
        retval = wu.lookup_id(asg.workunitid);
        if (retval) continue;
        int new_tt = time(0) + wu.delay_bound;
        if (new_tt < wu.transition_time) {
            char buf2[256];
            sprintf(buf2, "transition_time=%d", new_tt);
            wu.update_field(buf2);
        }
    }
    return sent_something;
}

// send targeted jobs
//
bool send_targeted_jobs() {
    bool sent_something = false;
    if (config.debug_send) {
        log_messages.printf(MSG_NORMAL, "checking for targeted jobs\n");
    }
    sent_something |= send_jobs(ASSIGN_USER);
    sent_something |= send_jobs(ASSIGN_HOST);
    sent_something |= send_jobs(ASSIGN_TEAM);
    return sent_something;
}
