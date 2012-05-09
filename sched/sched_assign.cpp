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
//

// scheduler functions to send assigned jobs.

#include "config.h"

#ifdef _USING_FCGI_
#include "boinc_fcgi.h"
#else
#include <cstdio>
#endif

#include <sys/param.h>
#include <unistd.h>

#include "boinc_db.h"
#include "crypt.h"
#include "backend_lib.h"
#include "error_numbers.h"

#include "sched_main.h"
#include "sched_msgs.h"
#include "sched_send.h"
#include "sched_version.h"
#include "sched_types.h"

#include "sched_assign.h"

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
            "assigned WU %d not found\n", asg.workunitid
        );
        return retval;
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
            "[WU#%d %s] create_result(): %s\n", wu.id, wu.name, boincerror(retval)
        );
        return retval;
    }
    int result_id = boinc_db.insert_id();
    SCHED_DB_RESULT result;
    retval = result.lookup_id(result_id);
    add_result_to_reply(result, wu, bavp, false);

    if (config.debug_assignment) {
        log_messages.printf(MSG_NORMAL,
            "[assign] [WU#%d] [RESULT#%d] [HOST#%d] send assignment %d\n",
            wu.id, result_id, g_reply->host.id, asg.id
        );
    }
    return 0;
}

// Send this host any "multi" assigned jobs.
// Return true iff we sent anything
//
bool send_assigned_jobs_multi() {
    DB_RESULT result;
    int retval;
    char buf[256];
    bool sent_something = false;

    for (int i=0; i<ssp->nassignments; i++) {
        ASSIGNMENT& asg = ssp->assignments[i];

        if (config.debug_assignment) {
            log_messages.printf(MSG_NORMAL,
                "[assign] processing multi assignment type %d\n",
                asg.target_type
            );
        }
        // see if this assignment applies to this host
        //
        switch (asg.target_type) {
        case ASSIGN_NONE:
            sprintf(buf, "where hostid=%d and workunitid=%d",
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
            sprintf(buf, "where workunitid=%d and hostid=%d",
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
            sprintf(buf, "where workunitid=%d and hostid=%d", asg.workunitid, g_reply->host.id);
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

// send non-multi assigned jobs
//
bool send_assigned_jobs() {
    DB_ASSIGNMENT asg;
    DB_RESULT result;
    DB_WORKUNIT wu;
    bool sent_something = false;
    int retval;

    // for now, only look for user assignments
    //
    char buf[256];
    sprintf(buf, "where target_type=%d and target_id=%d and multi=0",
        ASSIGN_USER, g_reply->user.id
    );
    while (!asg.enumerate(buf)) {
        if (!work_needed(false)) continue; 

        // if the WU doesn't exist, delete the assignment record.
        //
        retval = wu.lookup_id(asg.workunitid);
        if (retval) {
            asg.delete_from_db();
            continue;
        }
        // don't send if WU is validation pending or completed,
        // or has transition pending
        //
        if (wu.need_validate) continue;
        if (wu.canonical_resultid) continue;
        if (wu.transition_time < time(0)) continue;

        // don't send if we already sent one to this host
        //
        sprintf(buf, "where workunitid=%d and hostid=%d",
            asg.workunitid,
            g_reply->host.id
        );
        retval = result.lookup(buf);
        if (retval != ERR_DB_NOT_FOUND) continue;

        // don't send if there's already one in progress to this user
        //
        sprintf(buf,
            "where workunitid=%d and userid=%d and server_state=%d",
            asg.workunitid,
            g_reply->user.id,
            RESULT_SERVER_STATE_IN_PROGRESS
        );
        retval = result.lookup(buf);
        if (retval != ERR_DB_NOT_FOUND) continue;

        // OK, send the job
        //
        retval = send_assigned_job(asg);
        if (retval) continue;

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
