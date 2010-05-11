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

// scheduler code related to sending "lost" work
// (i.e. results we sent to the host, but which they're not reporting)
//
// TODO:
// - make sure result is still needed (no canonical result yet)
// - don't send if project has been reset since first send;
//   this result may have been the cause of reset
//   (need to pass last reset time from client)

#include "config.h"

#include <cstdlib>
#include <cstring>
#include <string>

#include "error_numbers.h"

#include "sched_main.h"
#include "sched_config.h"
#include "sched_locality.h"
#include "sched_msgs.h"
#include "sched_send.h"
#include "sched_shmem.h"
#include "sched_util.h"
#include "sched_version.h"
#include "sched_types.h"

#include "sched_resend.h"


#ifdef _USING_FCGI_
#include "boinc_fcgi.h"
#endif

// Assign a new deadline for the result;
// if it's not likely to complete by this time, return nonzero.
// TODO: EXPLAIN THE FORMULA FOR NEW DEADLINE
//
static int possibly_give_result_new_deadline(
    DB_RESULT& result, WORKUNIT& wu, BEST_APP_VERSION& bav
) {
    const double resend_frac = 0.5;  // range [0, 1)
    int now = time(0);
    int result_report_deadline = now + (int)(resend_frac*(result.report_deadline - result.sent_time));

    if (result_report_deadline < result.report_deadline) {
        result_report_deadline = result.report_deadline;
    }
    if (result_report_deadline > now + wu.delay_bound) {
        result_report_deadline = now + wu.delay_bound;
    }

    // If infeasible, return without modifying result
    //
    if (estimate_duration(wu, bav) > result_report_deadline-now) {
        if (config.debug_resend) {
            log_messages.printf(MSG_NORMAL,
                "[resend] [RESULT#%d] [HOST#%d] not resending lost result: can't complete in time\n",
                result.id, g_reply->host.id
            );
        }
        return 1;
    }
    
    // update result with new report time and sent time
    //
    if (config.debug_resend) {
        log_messages.printf(MSG_NORMAL,
            "[resend] [RESULT#%d] [HOST#%d] %s report_deadline (resend lost work)\n",
            result.id, g_reply->host.id,
            result_report_deadline==result.report_deadline?"NO update to":"Updated"
        );
    }
    result.sent_time = now;
    result.report_deadline = result_report_deadline;
    return 0;
}

// resend any jobs that:
// 1) we already sent to this host;
// 2) are still in progress (i.e. haven't timed out) and
// 3) aren't present on the host
// Return true if there were any such jobs
//
bool resend_lost_work() {
    DB_RESULT result;
    std::vector<DB_RESULT>results;
    unsigned int i;
    char buf[256];
    char warning_msg[256];
    bool did_any = false;
    int num_eligible_to_resend=0;
    int num_resent=0;
    BEST_APP_VERSION* bavp;
    int retval;

    sprintf(buf, " where hostid=%d and server_state=%d ",
        g_reply->host.id, RESULT_SERVER_STATE_IN_PROGRESS
    );
    while (!result.enumerate(buf)) {
        if (!work_needed(false)) {
            result.end_enumerate();
            break;
        }

        bool found = false;
        for (i=0; i<g_request->other_results.size(); i++) {
            OTHER_RESULT& orp = g_request->other_results[i];
            if (!strcmp(orp.name, result.name)) {
                found = true;
                break;
            }
        }
        if (found) continue;

        num_eligible_to_resend++;
        if (config.debug_resend) {
            log_messages.printf(MSG_NORMAL,
                "[resend] [HOST#%d] found lost [RESULT#%d]: %s\n",
                g_reply->host.id, result.id, result.name
            );
        }

        DB_WORKUNIT wu;
        retval = wu.lookup_id(result.workunitid);
        if (retval) {
            log_messages.printf(MSG_CRITICAL,
                "[HOST#%d] WU not found for [RESULT#%d]\n",
                g_reply->host.id, result.id
            );
            continue;
        }

        bavp = get_app_version(wu, false, false);
        if (!bavp) {
            APP* app = ssp->lookup_app(wu.appid);
            log_messages.printf(MSG_CRITICAL,
                "[HOST#%d] can't resend [RESULT#%d]: no app version for %s\n",
                g_reply->host.id, result.id, app->name
            );
            continue;
        }

        // If time is too close to the deadline,
        // or we already have a canonical result,
        // or WU error flag is set,
        // then don't bother to resend this result.
        // Instead make it time out right away
        // so that the transitioner does 'the right thing'.
        //
        if (
            wu.error_mask ||
            wu.canonical_resultid ||
            possibly_give_result_new_deadline(result, wu, *bavp)
        ) {
            if (config.debug_resend) {
                log_messages.printf(MSG_NORMAL,
                    "[resend] [HOST#%d][RESULT#%d] not needed or too close to deadline, expiring\n",
                    g_reply->host.id, result.id
                );
            }
            result.report_deadline = time(0)-1;
            retval = result.mark_as_sent(result.server_state);
            if (retval) {
                log_messages.printf(MSG_CRITICAL,
                    "resend_lost_work: can't update result deadline: %d\n", retval
                );
                continue;
            }

            retval = update_wu_transition_time(wu, result.report_deadline);
            if (retval) {
                log_messages.printf(MSG_CRITICAL,
                    "resend_lost_result: can't update WU transition time: %d\n", retval
                );
                continue;
            }
            sprintf(warning_msg,
                "Didn't resend lost task %s (expired)", result.name
            );
            g_reply->insert_message(warning_msg, "high");
        } else {
            retval = add_result_to_reply(result, wu, bavp, false);
            if (retval) {
                log_messages.printf(MSG_CRITICAL,
                    "[HOST#%d] failed to send [RESULT#%d]\n",
                    g_reply->host.id, result.id
                );
                continue;
            }
            sprintf(warning_msg, "Resent lost task %s", result.name);
            g_reply->insert_message(warning_msg, "high");
            num_resent++;
            did_any = true;

            if (g_wreq->njobs_sent >= config.max_wus_to_send) {
                result.end_enumerate();
                break;
            }
        }
    }

    if (num_eligible_to_resend && config.debug_resend) {
        log_messages.printf(MSG_NORMAL,
            "[resend] [HOST#%d] %d lost results, resent %d\n",
            g_reply->host.id, num_eligible_to_resend, num_resent 
        );
    }

    return did_any;
}

const char *BOINC_RCSID_3be23838b4="$Id$";
