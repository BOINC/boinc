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
#ifdef _USING_FCGI_
#include "boinc_fcgi.h"
#endif

#include "sched_array.h"

// do fast checks on this job, i.e. ones that don't require DB access
// if any check fails, return false
//
static bool quick_check(
    WU_RESULT& wu_result,
    WORKUNIT& wu,       // a mutable copy of wu_result.workunit.
        // We may modify its delay_bound and rsc_fpops_est
    BEST_APP_VERSION* &bavp,
    APP* &app, int& last_retval
) {
    int retval;

    app = ssp->lookup_app(wu_result.workunit.appid);
    if (app == NULL) {
        log_messages.printf(MSG_CRITICAL,
            "[WU#%d] no app\n",
            wu_result.workunit.id
        );
        return false; // this should never happen
    }

    g_wreq->no_jobs_available = false;

    // If we're looking for beta jobs and this isn't one, skip it
    //
    if (g_wreq->beta_only) {
        if (!app->beta) {
            if (config.debug_array) {
                log_messages.printf(MSG_NORMAL, "[array] not beta\n");
            }
            return false;
        }
        if (config.debug_send) {
            log_messages.printf(MSG_NORMAL,
                "[send] [HOST#%d] beta work found: [RESULT#%d]\n",
                g_reply->host.id, wu_result.resultid
            );
        }
    } else {
        if (app->beta) {
            if (config.debug_array) {
                log_messages.printf(MSG_NORMAL, "[array] is beta\n");
            }
            return false;
        }
    }

    // Are we scanning for need_reliable results?
    // skip this check the app is beta
    // (beta apps don't use the reliable mechanism)
    //
    if (!app->beta) {
        if (g_wreq->reliable_only && (!wu_result.need_reliable)) {
            if (config.debug_array) {
                log_messages.printf(MSG_NORMAL, "[array] don't need reliable\n");
            }
            return false;
        } else if (!g_wreq->reliable_only && wu_result.need_reliable) {
            if (config.debug_array) {
                log_messages.printf(MSG_NORMAL, "[array] need reliable\n");
            }
            return false;
        }
    }

    // don't send if we are looking for infeasible results
    // and the result is not infeasible
    //
    if (g_wreq->infeasible_only && (wu_result.infeasible_count==0)) {
        if (config.debug_array) {
            log_messages.printf(MSG_NORMAL, "[array] not infeasible\n");
        }
        return false;
    }

    // locality sched lite check.
    // Note: allow non-LSL jobs; otherwise we could starve them
    //
    if (g_wreq->locality_sched_lite) {
        if (app->locality_scheduling == LOCALITY_SCHED_LITE) {
            int n = nfiles_on_host(wu_result.workunit);
            if (config.debug_array) {
                log_messages.printf(MSG_NORMAL,
                    "[array] job %s: %d files on host\n",
                    wu_result.workunit.name, n
                );
            }
            if (n == 0) {
                return false;
            }
        }
    }

    // Find the app and best app_version for this host.
    //
    bavp = get_app_version(wu, true, g_wreq->reliable_only);
    if (!bavp) {
        if (config.debug_array) {
            log_messages.printf(MSG_NORMAL,
                "[array] No app version\n"
            );
        }
        return false;
    }

    // Check app filter if needed.
    // Do this AFTER get_app_version(), otherwise we could send
    // a misleading message to user
    //
    if (g_wreq->user_apps_only &&
        (!g_wreq->beta_only || config.distinct_beta_apps)
    ) {
        if (app_not_selected(wu)) {
            g_wreq->no_allowed_apps_available = true;
#if 1
            if (config.debug_array) {
                log_messages.printf(MSG_NORMAL,
                    "[array] [USER#%d] [WU#%d] user doesn't want work for app %s\n",
                    g_reply->user.id, wu.id, app->name
                );
            }
#endif
            return false;
        }
    }

    // Check whether we can send this job.
    // This may modify wu.delay_bound and wu.rsc_fpops_est
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
        return false;
    }
    return true;
}

// Do checks that require DB access for whether we can send this job,
// and return:
// 0 if OK to send
// 1 if can't send to this host
// 2 if can't send to ANY host
//
static int slow_check(
    WU_RESULT& wu_result,       // the job cache entry.
        // We may refresh its hr_class and app_version_id fields.
    APP* app,
    BEST_APP_VERSION* bavp      // the app version to be used
) {
    int n, retval;
    DB_RESULT result;
    char buf[256];
    WORKUNIT& wu = wu_result.workunit;

    // Don't send if we've already sent a result of this WU to this user.
    //
    if (config.one_result_per_user_per_wu) {
        sprintf(buf,
            "where workunitid=%d and userid=%d", wu.id, g_reply->user.id
        );
        retval = result.count(n, buf);
        if (retval) {
            log_messages.printf(MSG_CRITICAL,
                "send_work: can't get result count (%s)\n", boincerror(retval)
            );
            return 1;
        } else {
            if (n>0) {
                if (config.debug_send) {
                    log_messages.printf(MSG_NORMAL,
                        "[send] [USER#%d] already has %d result(s) for [WU#%d]\n",
                        g_reply->user.id, n, wu.id
                    );
                }
                return 1;
            }
        }
    } else if (config.one_result_per_host_per_wu) {
        // Don't send if we've already sent a result of this WU to this host.
        // We only have to check this if we don't send one result per user.
        //
        sprintf(buf,
            "where workunitid=%d and hostid=%d", wu.id, g_reply->host.id
        );
        retval = result.count(n, buf);
        if (retval) {
            log_messages.printf(MSG_CRITICAL,
                "send_work: can't get result count (%s)\n", boincerror(retval)
            );
            return 1;
        } else {
            if (n>0) {
                if (config.debug_send) {
                    log_messages.printf(MSG_NORMAL,
                        "[send] [HOST#%d] already has %d result(s) for [WU#%d]\n",
                        g_reply->host.id, n, wu.id
                    );
                }
                return 1;
            }
        }
    }

    // Checks that require looking up the WU.
    // Lump these together so we only do 1 lookup
    //
    if (app_hr_type(*app) || app->homogeneous_app_version) {
        DB_WORKUNIT db_wu;
        db_wu.id = wu.id;
        int vals[3];
        retval = db_wu.get_field_ints(
            "hr_class, app_version_id, error_mask", 3, vals
        );
        if (retval) {
            log_messages.printf(MSG_CRITICAL,
                "can't get fields for [WU#%d]: %s\n", db_wu.id, boincerror(retval)
            );
            return 1;
        }

        // check wu.error_mask
        //
        if (vals[2] != 0) {
            return 2;
        }

        if (app_hr_type(*app)) {
            wu.hr_class = vals[0];
            if (already_sent_to_different_hr_class(wu, *app)) {
                if (config.debug_send) {
                    log_messages.printf(MSG_NORMAL,
                        "[send] [HOST#%d] [WU#%d %s] is assigned to different HR class\n",
                        g_reply->host.id, wu.id, wu.name
                    );
                }
                // Mark the workunit as infeasible.
                // This ensures that jobs already assigned to an HR class
                // are processed first.
                //
                wu_result.infeasible_count++;
                return 1;
            }
        }
        if (app->homogeneous_app_version) {
            int wu_avid = vals[1];
            wu.app_version_id = wu_avid;
            if (wu_avid && wu_avid != bavp->avp->id) {
                if (config.debug_send) {
                    log_messages.printf(MSG_NORMAL,
                        "[send] [HOST#%d] [WU#%d %s] is assigned to different app version\n",
                        g_reply->host.id, wu.id, wu.name
                    );
                }
                wu_result.infeasible_count++;
                return 1;
            }
        }
    }
    return 0;
}

// Check for pathological conditions that mean
// result is not sendable at all.
//
static bool result_still_sendable(DB_RESULT& result, WORKUNIT& wu) {
    int retval = result.lookup_id(result.id);
    if (retval) {
        log_messages.printf(MSG_CRITICAL,
            "[RESULT#%d] result.lookup_id() failed: %s\n",
            result.id, boincerror(retval)
        );
        return false;
    }
    if (result.server_state != RESULT_SERVER_STATE_UNSENT) {
        log_messages.printf(MSG_NORMAL,
            "[RESULT#%d] expected to be unsent; instead, state is %d\n",
            result.id, result.server_state
        );
        return false;
    }
    if (result.workunitid != wu.id) {
        log_messages.printf(MSG_CRITICAL,
            "[RESULT#%d] wrong WU ID: wanted %d, got %d\n",
            result.id, wu.id, result.workunitid
        );
        return false;
    }
    return true;
}

// Make a pass through the wu/results array, sending work.
// The choice of jobs is limited by flags in g_wreq, as follows:
// infeasible_only:
//      send only results that were previously infeasible for some host
// reliable_only:
//      send only jobs with "need_reliable" set (e.g. retries)
//      and send them only w/ app versions that are "reliable" for this host
// user_apps_only:
//      Send only jobs for apps selected by user
// beta_only:
//      Send only jobs for beta-test apps
// locality_sched_lite:
//      For apps that use locality sched Lite,
//      send only jobs for which the host already has at least 1 file
//
// Return true if no more work is needed.
//
static bool scan_work_array() {
    int i, j, rnd_off, last_retval=0;;
    APP* app;
    BEST_APP_VERSION* bavp;
    bool no_more_needed = false;
    SCHED_DB_RESULT result;

    lock_sema();

    rnd_off = rand() % ssp->max_wu_results;
    for (j=0; j<ssp->max_wu_results; j++) {
        i = (j+rnd_off) % ssp->max_wu_results;

        WU_RESULT& wu_result = ssp->wu_results[i];

#if 0
        if (config.debug_array) {
            log_messages.printf(MSG_NORMAL,
                "[array] scanning slot %d\n", i
            );
        }
#endif

        // make a copy of the WORKUNIT part,
        // which we can modify without affecting the cache
        //
        WORKUNIT wu = wu_result.workunit;

        if (wu_result.state != WR_STATE_PRESENT && wu_result.state != g_pid) {
            continue;
        }

        // do fast (non-DB) checks.
        // This may modify wu.rsc_fpops_est
        //
        if (!quick_check(wu_result, wu, bavp, app, last_retval)) {
            if (config.debug_array) {
                log_messages.printf(MSG_NORMAL,
                    "[array] slot %d failed quick check\n", i
                );
            }
            continue;
        }

        // mark wu_result as checked out and release semaphore.
        // from here on in this loop, don't continue on failure;
        // instead, goto dont_send (so that we reacquire semaphore)
        //
        // Note: without the semaphore we don't have mutual exclusion;
        // ideally we should use a transaction from now until when
        // we commit to sending the results.

        wu_result.state = g_pid;
        unlock_sema();

        switch (slow_check(wu_result, app, bavp)) {
        case 1:
            // if we couldn't send the result to this host,
            // set its state back to PRESENT
            //
            wu_result.state = WR_STATE_PRESENT;
            break;
        case 2:
            // can't send this job to any host
            //
            wu_result.state = WR_STATE_EMPTY;
            break;
        default:
            // slow_check() refreshes fields of wu_result.workunit;
            // update our copy too
            //
            wu.hr_class = wu_result.workunit.hr_class;
            wu.app_version_id = wu_result.workunit.app_version_id;

            // mark slot as empty AFTER we've copied out of it
            // (since otherwise feeder might overwrite it)
            //
            wu_result.state = WR_STATE_EMPTY;

            // reread result from DB, make sure it's still unsent
            // TODO: from here to end of add_result_to_reply()
            // (which updates the DB record) should be a transaction
            //
            result.id = wu_result.resultid;
            if (result_still_sendable(result, wu)) {
                add_result_to_reply(result, wu, bavp, false);

                // add_result_to_reply() fails only in pathological cases -
                // e.g. we couldn't update the DB record or modify XML fields.
                // If this happens, don't replace the record in the array
                // (we can't anyway, since we marked the entry as "empty").
                // The feeder will eventually pick it up again,
                // and hopefully the problem won't happen twice.
            }
            break;
        }
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
    g_wreq->beta_only = false;
    g_wreq->user_apps_only = true;
    g_wreq->infeasible_only = false;

    // give top priority to results that require a 'reliable host'
    //
    if (g_wreq->has_reliable_version) {
        g_wreq->reliable_only = true;
        if (config.debug_send) {
            log_messages.printf(MSG_NORMAL,
                "[send] scanning for jobs that need reliable host\n"
            );
        }
        if (scan_work_array()) return;
        g_wreq->reliable_only = false;
        g_wreq->best_app_versions.clear();
    }

    // give 2nd priority to results for a beta app
    // (projects should load beta work with care,
    // otherwise your users won't get production work done!
    //
    if (g_wreq->allow_beta_work) {
        g_wreq->beta_only = true;
        if (config.debug_send) {
            log_messages.printf(MSG_NORMAL,
                "[send] host will accept beta jobs.  Scanning for them.\n"
            );
        }
        if (scan_work_array()) return;
        g_wreq->beta_only = false;
    }

    // give next priority to results that were infeasible for some other host
    //
    g_wreq->infeasible_only = true;
    if (config.debug_send) {
        log_messages.printf(MSG_NORMAL,
            "[send] Scanning for jobs that were infeasible for another host.\n"
        );
    }
    if (scan_work_array()) return;
    g_wreq->infeasible_only = false;

    // if some app uses locality sched lite,
    // make a pass accepting only jobs for which the client has a file
    //
    if (ssp->locality_sched_lite) {
        if (config.debug_send) {
            log_messages.printf(MSG_NORMAL,
                "[send] Scanning for locality sched Lite jobs.\n"
            );
        }
        g_wreq->locality_sched_lite = true;
        if (scan_work_array()) return;
        g_wreq->locality_sched_lite = false;
    }

    // end of high-priority cases.  Now do general scan.
    //
    if (config.debug_send) {
        log_messages.printf(MSG_NORMAL,
            "[send] doing general job scan.\n"
        );
    }
    if (scan_work_array()) return;

    // If user has selected apps but will accept any,
    // and we haven't found any jobs for selected apps, try others
    //
    if (!g_wreq->njobs_sent && g_wreq->allow_non_preferred_apps ) {
        g_wreq->user_apps_only = false;
        preferred_app_message_index = g_wreq->no_work_messages.size();
        if (config.debug_send) {
            log_messages.printf(MSG_NORMAL,
                "[send] scanning for jobs from non-preferred applications\n"
            );
        }
        scan_work_array();
    }
}

const char *BOINC_RCSID_d9f764fd14="$Id$";
