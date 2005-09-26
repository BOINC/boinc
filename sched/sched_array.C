// Berkeley Open Infrastructure for Network Computing
// http://boinc.berkeley.edu
// Copyright (C) 2005 University of California
//
// This is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation;
// either version 2.1 of the License, or (at your option) any later version.
//
// This software is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//
// To view the GNU Lesser General Public License visit
// http://www.gnu.org/copyleft/lesser.html
// or write to the Free Software Foundation, Inc.,
// 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

// scheduler code related to sending work


#include "main.h"
#include "server_types.h"
#include "sched_shmem.h"
#include "sched_hr.h"
#include "sched_config.h"
#include "sched_util.h"
#include "sched_msgs.h"
#include "sched_send.h"

#include "sched_array.h"


#ifdef _USING_FCGI_
#include "fcgi_stdio.h"
#else
#define FCGI_ToFILE(x) (x)
#endif

// Make a pass through the wu/results array, sending work.
// If "infeasible_only" is true, send only results that were
// previously infeasible for some host
//
void scan_work_array(
    SCHEDULER_REQUEST& sreq, SCHEDULER_REPLY& reply, PLATFORM& platform,
    SCHED_SHMEM& ss
) {
    int i, j, retval, n, rnd_off;
    WORKUNIT wu;
    DB_RESULT result;
    char buf[256];
    APP* app;
    APP_VERSION* avp;
    bool found;

    lock_sema();
    
    rnd_off = rand() % ss.nwu_results;
    for (j=0; j<ss.nwu_results; j++) {
        i = (j+rnd_off) % ss.nwu_results;
        if (!reply.work_needed()) break;

        WU_RESULT& wu_result = ss.wu_results[i];

        // do fast checks on this wu_result;
        // i.e. ones that don't require DB access
        // if any check fails, continue

        if (wu_result.state != WR_STATE_PRESENT && wu_result.state != g_pid) {
            continue;
        }

        if (reply.wreq.infeasible_only && (wu_result.infeasible_count==0)) {
            continue;
        }

        // don't send if we're already sending a result for same WU
        //
        if (config.one_result_per_user_per_wu) {
            if (wu_already_in_reply(wu_result.workunit, reply)) {
                continue;
            }
        }

        // don't send if host can't handle it
        //
        wu = wu_result.workunit;
        if (wu_is_infeasible(wu, sreq, reply)) {
            log_messages.printf(
                SCHED_MSG_LOG::MSG_DEBUG, "[HOST#%d] [WU#%d %s] WU is infeasible\n",
                reply.host.id, wu.id, wu.name
            );
            wu_result.infeasible_count++;
            continue;
        }

        // Find the app and app_version for the client's platform.
        // If none, treat the WU as infeasible
        //
        if (anonymous(platform)) {
            app = ss.lookup_app(wu.appid);
            found = sreq.has_version(*app);
            if (!found) {
                continue;
            }
            avp = NULL;
        } else {
            found = find_app_version(reply.wreq, wu, platform, ss, app, avp);
            if (!found) {
                wu_result.infeasible_count++;
                continue;
            }

            // see if the core client is too old.
            // don't bump the infeasible count because this
            // isn't the result's fault
            //
            if (!app_core_compatible(reply.wreq, *avp)) {
                continue;
            }
        }

        // end of fast checks - mark wu_result as checked out and release sema.
        // from here on in this loop, don't continue on failure;
        // instead, goto dont_send (so that we reacquire semaphore)

        wu_result.state = WR_STATE_CHECKED_OUT;
        unlock_sema();

        // Don't send if we've already sent a result of this WU to this user.
        //
        if (config.one_result_per_user_per_wu) {
            sprintf(buf,
                "where workunitid=%d and userid=%d",
                wu_result.workunit.id, reply.user.id
            );
            retval = result.count(n, buf);
            if (retval) {
                log_messages.printf(
                    SCHED_MSG_LOG::MSG_CRITICAL,
                    "send_work: can't get result count (%d)\n", retval
                );
                goto dont_send;
            } else {
                if (n>0) {
                    log_messages.printf(
                        SCHED_MSG_LOG::MSG_DEBUG,
                        "send_work: user %d already has %d result(s) for WU %d\n",
                        reply.user.id, n, wu_result.workunit.id
                    );
                    goto dont_send;
                }
            }
        }

        // if desired, make sure redundancy is homogeneous
        //
        if (config.homogeneous_redundancy || app->homogeneous_redundancy) {
            if (already_sent_to_different_platform(
                sreq, wu_result.workunit, reply.wreq
            )) {
                goto dont_send;
            }
        }

        result.id = wu_result.resultid;

        // mark slot as empty AFTER we've copied out of it
        // (since otherwise feeder might overwrite it)
        //
        wu_result.state = WR_STATE_EMPTY;

        // reread result from DB, make sure it's still unsent
        // TODO: from here to update() should be a transaction
        //
        retval = result.lookup_id(result.id);
        if (retval) {
            log_messages.printf(SCHED_MSG_LOG::MSG_CRITICAL,
                "[RESULT#%d] result.lookup_id() failed %d\n",
                result.id, retval
            );
            goto done;
        }
        if (result.server_state != RESULT_SERVER_STATE_UNSENT) {
            log_messages.printf(SCHED_MSG_LOG::MSG_DEBUG,
                "[RESULT#%d] expected to be unsent; instead, state is %d\n",
                result.id, result.server_state
            );
            goto done;
        }
        if (result.workunitid != wu.id) {
            log_messages.printf(SCHED_MSG_LOG::MSG_CRITICAL,
                "[RESULT#%d] wrong WU ID: wanted %d, got %d\n",
                result.id, wu.id, result.workunitid
            );
            goto done;
        }

        // ****** HERE WE'VE COMMITTED TO SENDING THIS RESULT TO HOST ******
        //

        retval = add_result_to_reply(
            result, wu, sreq, reply, platform, app, avp
        );
        if (!retval) goto done;

dont_send:
        // here we couldn't send the result for some reason --
        // set its state back to PRESENT
        //
        wu_result.state = WR_STATE_PRESENT;
done:
        lock_sema();
    }
    unlock_sema();
}

const char *BOINC_RCSID_d9f764fd14="$Id$";
