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
#include "config.h"

#ifdef _USING_FCGI_
#include "boinc_fcgi.h"
#else
#include <cstdio>
#endif

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

static int send_assigned_job(ASSIGNMENT& asg) {
    int retval;
    DB_WORKUNIT wu;
    char suffix[256], path[256], buf[256];
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
    retval = create_result(wu, (char *)rtfpath, suffix, key, config, 0, 0);
    if (retval) {
        log_messages.printf(MSG_CRITICAL,
            "[WU#%d %s] create_result() %d\n", wu.id, wu.name, retval
        );
        return retval;
    }
    int result_id = boinc_db.insert_id();
    DB_RESULT result;
    retval = result.lookup_id(result_id);
    add_result_to_reply(result, wu, bavp, false);

    // if this is a one-job assignment, fill in assignment.resultid
    // so that it doesn't get sent again
    //
    if (!asg.multi && asg.target_type!=ASSIGN_NONE) {
        DB_ASSIGNMENT db_asg;
        db_asg.id = asg.id;
        sprintf(buf, "resultid=%d", result_id);
        retval = db_asg.update_field(buf);
        if (retval) {
            log_messages.printf(MSG_CRITICAL,
                "assign update failed: %d\n", retval
            );
            return retval;
        }
        asg.resultid = result_id;
    }
    if (config.debug_assignment) {
        log_messages.printf(MSG_NORMAL,
            "[assign] [WU#%d] [RESULT#%d] [HOST#%d] send assignment %d\n",
            wu.id, result_id, g_reply->host.id, asg.id
        );
    }
    return 0;
}

// Send this host any jobs assigned to it, or to its user/team
// Return true iff we sent anything
//
bool send_assigned_jobs() {
    DB_RESULT result;
    int retval;
    char buf[256];
    bool sent_something = false;

    for (int i=0; i<ssp->nassignments; i++) {
        ASSIGNMENT& asg = ssp->assignments[i];

        if (config.debug_assignment) {
            log_messages.printf(MSG_NORMAL,
                "[assign] processing assignment type %d\n", asg.target_type
            );
        }
        // see if this assignment applies to this host
        //
        if (asg.resultid) continue;
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
        case ASSIGN_HOST:
            if (g_reply->host.id != asg.target_id) continue;
            sprintf(buf, "where workunitid=%d", asg.workunitid);
            retval = result.lookup(buf);
            if (retval == ERR_DB_NOT_FOUND) {
                retval = send_assigned_job(asg);
                if (!retval) sent_something = true;
            }
            break;
        case ASSIGN_USER:
            if (g_reply->user.id != asg.target_id) continue;
            if (asg.multi) {
                sprintf(buf, "where workunitid=%d and hostid=%d", asg.workunitid, g_reply->host.id);
            } else {
                sprintf(buf, "where workunitid=%d", asg.workunitid);
            }
            retval = result.lookup(buf);
            if (retval == ERR_DB_NOT_FOUND) {
                retval = send_assigned_job(asg);
                if (!retval) sent_something = true;
            }
            break;
        case ASSIGN_TEAM:
            if (g_reply->team.id != asg.target_id) continue;
            if (asg.multi) {
                sprintf(buf, "where workunitid=%d and hostid=%d", asg.workunitid, g_reply->host.id);
            } else {
                sprintf(buf, "where workunitid=%d", asg.workunitid);
            }
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
