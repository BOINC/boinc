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

// scheduler code related to sending work

#include <cstdlib>
#include <string>
#include <cstring>

#include "config.h"
#include "sched_main.h"
#include "sched_types.h"
#include "sched_shmem.h"
#include "sched_hr.h"
#include "sched_config.h"
#include "sched_util.h"
#include "sched_msgs.h"
#include "sched_send.h"
#include "sched_version.h"

#include "sched_array.h"


#ifdef _USING_FCGI_
#include "boinc_fcgi.h"
#endif

// Make a pass through the wu/results array, sending work.
// The choice of jobs is limited by flags in g_wreq, as follows:
// infeasible_only:
//      send only results that were previously infeasible for some host
// reliable_only: 
//      send only retries
// user_apps_only:
//      Send only jobs for apps selected by user
// beta_only:
//      Send only jobs for beta-test apps
//
// Return true if no more work is needed.
//
static bool scan_work_array() {
    int i, j, retval, n, rnd_off, last_retval=0;;
    WORKUNIT wu;
    DB_RESULT result;
    char buf[256];
    APP* app;
    bool no_more_needed = false;

    lock_sema();
    
    rnd_off = rand() % ssp->max_wu_results;
    for (j=0; j<ssp->max_wu_results; j++) {
        i = (j+rnd_off) % ssp->max_wu_results;

        WU_RESULT& wu_result = ssp->wu_results[i];

        // do fast checks on this wu_result;
        // i.e. ones that don't require DB access
        // if any check fails, continue

        if (wu_result.state != WR_STATE_PRESENT && wu_result.state != g_pid) {
            continue;
        }

        // If we are looking for beta results and result is not a beta result
        // then move on
        //
        app = ssp->lookup_app(wu_result.workunit.appid);
        if (app == NULL) continue; // this should never happen
        if (g_wreq->beta_only) {
            if (!app->beta) {
                continue;
            }
            if (config.debug_send) {
                log_messages.printf(MSG_NORMAL,
                    "[send] [HOST#%d] beta work found: [RESULT#%d]\n",
                    g_reply->host.id, wu_result.resultid
                );
            }
        } else {
            if (app->beta) {
                continue;
            }
        }
        
        g_wreq->no_jobs_available = false;
        
        // If this is a reliable host and we are checking for results that
        // need a reliable host, then continue if the result is a normal result
        // skip if the app is beta (beta apps don't use the reliable mechanism)
        //
        if (!app->beta) {
            if (g_wreq->reliable_only && (!wu_result.need_reliable)) {
                continue;
            } else if (!g_wreq->reliable_only && wu_result.need_reliable) {
                continue;
            }
        }
        
        // don't send if we are looking for infeasible results
        // and the result is not infeasible
        //
        if (g_wreq->infeasible_only && (wu_result.infeasible_count==0)) {
            continue;
        }
        
        wu = wu_result.workunit;

        // check app filter if needed
        //
        if (g_wreq->user_apps_only &&
            (!g_wreq->beta_only || config.distinct_beta_apps)
        ) {
            if (app_not_selected(wu)) {
                g_wreq->no_allowed_apps_available = true;
#if 0
                if (config.debug_send) {
                    log_messages.printf(MSG_NORMAL,
                        "[send] [USER#%d] [WU#%d] user doesn't want work for app %s\n",
                        g_reply->user.id, wu.id, app->name
                    );
                }
#endif
                continue;
            }
        }

        // Find the app and best app_version for this host.
        //
        BEST_APP_VERSION* bavp;
        bavp = get_app_version(wu, true);
        if (!bavp) {
            if (config.debug_array) {
                log_messages.printf(MSG_NORMAL,
                    "[array] No app version\n"
                );
            }
            continue;
        }

        // don't send job if host can't handle it
        //
        retval = wu_is_infeasible_fast(
            wu,
            wu_result.res_server_state, wu_result.res_priority,
            wu_result.res_report_deadline,
            *app, *bavp
        );
        if (retval) {
            if (retval != last_retval && config.debug_send) {
                log_messages.printf(MSG_NORMAL,
                    "[send] [HOST#%d] [WU#%d %s] WU is infeasible: %s\n",
                    g_reply->host.id, wu.id, wu.name, infeasible_string(retval)
                );
            }
            last_retval = retval;
            if (config.debug_array) {
                log_messages.printf(MSG_NORMAL, "[array] infeasible\n");
            }
            continue;
        }

        // End of fast checks;
        // mark wu_result as checked out and release semaphore.
        // from here on in this loop, don't continue on failure;
        // instead, goto dont_send (so that we reacquire semaphore)
        //
        // Note: without the semaphore we don't have mutual exclusion;
        // ideally we should use a transaction from now until when
        // we commit to sending the results.

        wu_result.state = g_pid;
        unlock_sema();

        // Don't send if we've already sent a result of this WU to this user.
        //
        if (config.one_result_per_user_per_wu) {
            sprintf(buf,
                "where workunitid=%d and userid=%d",
                wu_result.workunit.id, g_reply->user.id
            );
            retval = result.count(n, buf);
            if (retval) {
                log_messages.printf(MSG_CRITICAL,
                    "send_work: can't get result count (%d)\n", retval
                );
                goto dont_send;
            } else {
                if (n>0) {
                    if (config.debug_send) {
                        log_messages.printf(MSG_NORMAL,
                            "[send] [USER#%d] already has %d result(s) for [WU#%d]\n",
                            g_reply->user.id, n, wu_result.workunit.id
                        );
                    }
                    goto dont_send;
                }
            }
        } else if (config.one_result_per_host_per_wu) {
            // Don't send if we've already sent a result
            // of this WU to this host.
            // We only have to check this
            // if we don't send one result per user.
            //
            sprintf(buf,
                "where workunitid=%d and hostid=%d",
                wu_result.workunit.id, g_reply->host.id
            );
            retval = result.count(n, buf);
            if (retval) {
                log_messages.printf(MSG_CRITICAL,
                    "send_work: can't get result count (%d)\n", retval
                );
                goto dont_send;
            } else {
                if (n>0) {
                    if (config.debug_send) {
                        log_messages.printf(MSG_NORMAL,
                            "[send] [HOST#%d] already has %d result(s) for [WU#%d]\n",
                            g_reply->host.id, n, wu_result.workunit.id
                        );
                    }
                    goto dont_send;
                }
            }
        }

        if (app_hr_type(*app)) {
            if (already_sent_to_different_platform_careful(
                wu_result.workunit, *app
            )) {
                if (config.debug_send) {
                    log_messages.printf(MSG_NORMAL,
                        "[send] [HOST#%d] [WU#%d %s] is assigned to different platform\n",
                        g_reply->host.id, wu.id, wu.name
                    );
                }
                // Mark the workunit as infeasible.
                // This ensures that jobs already assigned to a platform
                // are processed first.
                //
                wu_result.infeasible_count++;
                goto dont_send;
            }
        }

        result.id = wu_result.resultid;

        // mark slot as empty AFTER we've copied out of it
        // (since otherwise feeder might overwrite it)
        //
        wu_result.state = WR_STATE_EMPTY;

        // reread result from DB, make sure it's still unsent
        // TODO: from here to add_result_to_reply()
        // (which updates the DB record) should be a transaction
        //
        retval = result.lookup_id(result.id);
        if (retval) {
            log_messages.printf(MSG_CRITICAL,
                "[RESULT#%d] result.lookup_id() failed %d\n",
                result.id, retval
            );
            goto done;
        }
        if (result.server_state != RESULT_SERVER_STATE_UNSENT) {
            log_messages.printf(MSG_NORMAL,
                "[RESULT#%d] expected to be unsent; instead, state is %d\n",
                result.id, result.server_state
            );
            goto done;
        }
        if (result.workunitid != wu.id) {
            log_messages.printf(MSG_CRITICAL,
                "[RESULT#%d] wrong WU ID: wanted %d, got %d\n",
                result.id, wu.id, result.workunitid
            );
            goto done;
        }

        retval = add_result_to_reply(result, wu, bavp, false);

        // add_result_to_reply() fails only in fairly pathological cases -
        // e.g. we couldn't update the DB record or modify XML fields.
        // If this happens, don't replace the record in the array
        // (we can't anyway, since we marked the entry as "empty").
        // The feeder will eventually pick it up again,
        // and hopefully the problem won't happen twice.
        //
        goto done;

dont_send:
        // here we couldn't send the result for some reason --
        // set its state back to PRESENT
        //
        wu_result.state = WR_STATE_PRESENT;
done:
        lock_sema();
        if (!work_needed(false)) {
            no_more_needed = true;
            break;
        }
    }
    unlock_sema();
    return no_more_needed;
}

// Send work by scanning the job array multiple times,
// with different selection criteria on each scan.
// This has been superceded by send_work_matchmaker()
//
void send_work_old() {
    if (!work_needed(false)) return;
    g_wreq->no_jobs_available = true;
    g_wreq->beta_only = false;
    g_wreq->user_apps_only = true;
    g_wreq->infeasible_only = false;

    // give top priority to results that require a 'reliable host'
    //
    if (g_wreq->reliable) {
        g_wreq->reliable_only = true;
        if (scan_work_array()) return;
    }
    g_wreq->reliable_only = false;

    // give 2nd priority to results for a beta app
    // (projects should load beta work with care,
    // otherwise your users won't get production work done!
    //
    if (g_wreq->allow_beta_work) {
        g_wreq->beta_only = true;
        if (config.debug_send) {
            log_messages.printf(MSG_NORMAL,
                "[send] [HOST#%d] will accept beta work.  Scanning for beta work.\n",
                g_reply->host.id
            );
        }
        if (scan_work_array()) return;
    }
    g_wreq->beta_only = false;

    // give next priority to results that were infeasible for some other host
    //
    g_wreq->infeasible_only = true;
    if (scan_work_array()) return;;

    g_wreq->infeasible_only = false;
    if (scan_work_array()) return;

    // If user has selected apps but will accept any,
    // and we haven't found any jobs for selected apps, try others
    //
    if (!g_wreq->njobs_sent && g_wreq->allow_non_preferred_apps ) {
        g_wreq->user_apps_only = false;
        preferred_app_message_index = g_wreq->no_work_messages.size();
        if (config.debug_send) {
            log_messages.printf(MSG_NORMAL,
                "[send] [HOST#%d] is looking for work from a non-preferred application\n",
                g_reply->host.id
            );
        }
        scan_work_array();
    }
}

const char *BOINC_RCSID_d9f764fd14="$Id$";
