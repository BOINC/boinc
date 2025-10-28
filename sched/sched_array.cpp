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

// The "old style" scheduler,
// where we make multiple scans through the job cache.
// DEPRECATED - replaced by score-based scheduling.

#include <cstdlib>
#include <string>
#include <cstring>

#include "config.h"

#include "sched_check.h"
#include "sched_config.h"
#include "sched_hr.h"
#include "sched_main.h"
#include "sched_msgs.h"
#include "sched_send.h"
#include "sched_shmem.h"
#include "sched_types.h"
#include "sched_util.h"
#include "sched_version.h"
#include "boinc_stdio.h"
#include "sched_array.h"

// do fast checks on this job, i.e. ones that don't require DB access
// if any check fails, return false
//
static bool quick_check(
    WU_RESULT& wu_result,
    WORKUNIT& wu,       // a mutable copy of wu_result.workunit.
        // We may modify its delay_bound, rsc_fpops_est, and rsc_fpops_bound
    BEST_APP_VERSION* &bavp,
    APP* app,
    int& last_retval
) {
    int retval;

    // If we're looking for beta jobs and this isn't one, skip it
    //
    if (g_wreq->beta_only) {
        if (!app->beta) {
            if (config.debug_send_job) {
                log_messages.printf(MSG_NORMAL,
                    "[send_job] job is not from beta app; skipping\n"
                );
            }
            return false;
        }
        if (config.debug_send_job) {
            log_messages.printf(MSG_NORMAL,
                "[send_job] [HOST#%lu] beta work found: [RESULT#%lu]\n",
                g_reply->host.id, wu_result.resultid
            );
        }
    } else {
        if (app->beta) {
            if (config.debug_send_job) {
                log_messages.printf(MSG_NORMAL,
                    "[send_job] job is from beta app; skipping\n"
                );
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
            if (config.debug_send_job) {
                log_messages.printf(MSG_NORMAL,
                    "[send_job] job doesn't need reliable host; skipping\n"
                );
            }
            return false;
        } else if (!g_wreq->reliable_only && wu_result.need_reliable) {
            if (config.debug_send_job) {
                log_messages.printf(MSG_NORMAL,
                    "[send_job] job needs reliable host; skipping\n"
                );
            }
            return false;
        }
    }

    // don't send if we are looking for infeasible results
    // and the result is not infeasible
    //
    if (g_wreq->infeasible_only && (wu_result.infeasible_count==0)) {
        if (config.debug_send_job) {
            log_messages.printf(MSG_NORMAL,
                "[send_job] job is not infeasible; skipping\n"
            );
        }
        return false;
    }

    // locality sched lite check.
    // Allow non-LSL jobs; otherwise we could starve them
    // NOTE: THIS NEGATES THE OTHER SCHED POLICIES (reliable, etc.).
    // Need to think of some way of combining them.
    //
    if (g_wreq->locality_sched_lite) {
        // skip this job if host has sticky files
        // but none of them is used by this job.
        // TODO: it should really be "host has sticky files for this app".
        // However, we don't have a way of making that association.
        // Could add something based on filename
        //
        if (app->locality_scheduling == LOCALITY_SCHED_LITE
            && g_request->file_infos.size()
        ) {
            int n = nfiles_on_host(wu_result.workunit);
            if (config.debug_locality_lite) {
                log_messages.printf(MSG_NORMAL,
                    "[loc_lite] job %s has %d files on this host\n",
                    wu_result.workunit.name, n
                );
            }
            if (n == 0) {
                return false;
            }
        }
    }

    // Find the best app_version for this host.
    //
    bavp = get_app_version(wu, true, g_wreq->reliable_only);
    if (!bavp) {
        if (config.debug_send_job) {
            log_messages.printf(MSG_NORMAL,
                "[send_job] No app version for job; skipping\n"
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
        if (app_not_selected(app->id)) {
            g_wreq->no_allowed_apps_available = true;
            if (config.debug_send_job) {
                log_messages.printf(MSG_NORMAL,
                    "[send_job] [USER#%lu] [WU#%lu] user doesn't want work for app %s\n",
                    g_reply->user.id, wu.id, app->name
                );
            }
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
        if (retval != last_retval && config.debug_send_job) {
            log_messages.printf(MSG_NORMAL,
                "[send_job] [HOST#%lu] [WU#%lu %s] WU is infeasible: %s\n",
                g_reply->host.id, wu.id, wu.name, infeasible_string(retval)
            );
        }
        last_retval = retval;
        if (config.debug_send_job) {
            log_messages.printf(MSG_NORMAL,
                "[send_job] is_infeasible_fast() failed; skipping\n"
            );
        }
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

    // To minimize the amount of time we lock the array,
    // we initially scan without holding the lock.
    // If we find a job that passes quick_check(),
    // we acquire the lock and then check the job again.
    //
    bool sema_locked = false;

    rnd_off = rand() % ssp->max_wu_results;
    for (j=0; j<ssp->max_wu_results; j++) {
        i = (j+rnd_off) % ssp->max_wu_results;

        WU_RESULT& wu_result = ssp->wu_results[i];

        if (config.debug_send_job) {
            log_messages.printf(MSG_NORMAL,
                "[send_job] scanning slot %d\n", i
            );
        }

recheck:
        if (wu_result.state != WR_STATE_PRESENT && wu_result.state != g_pid) {
            continue;
        }

        // make a copy of the WORKUNIT part,
        // which we can modify without affecting the cache
        //
        WORKUNIT wu = wu_result.workunit;

        app = ssp->lookup_app(wu_result.workunit.appid);
        if (app == NULL) {
            log_messages.printf(MSG_CRITICAL,
                "[WU#%lu] no app\n",
                wu_result.workunit.id
            );
            continue; // this should never happen
        }

        if (app->non_cpu_intensive) continue;

        // do fast (non-DB) checks.
        // This may modify wu.rsc_fpops_est
        //
        if (!quick_check(wu_result, wu, bavp, app, last_retval)) {
            if (config.debug_send_job) {
                log_messages.printf(MSG_NORMAL,
                    "[send_job] slot %d failed quick check\n", i
                );
            }
            continue;
        }

        if (!sema_locked) {
            lock_sema();
            sema_locked = true;
            goto recheck;
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
        sema_locked = false;

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
                HOST_USAGE hu;
                BUDA_VARIANT *bvp = NULL;
                if (is_buda(wu)) {
                    if (!choose_buda_variant(wu, -1, &bvp, hu)) {
                        continue;
                    }
                } else {
                    hu = bavp->host_usage;
                }
                add_result_to_reply(result, wu, bavp, hu, bvp, false);

                // add_result_to_reply() fails only in pathological cases -
                // e.g. we couldn't update the DB record or modify XML fields.
                // If this happens, don't replace the record in the array
                // (we can't anyway, since we marked the entry as "empty").
                // The feeder will eventually pick it up again,
                // and hopefully the problem won't happen twice.
            }
            break;
        }
        if (!work_needed(false)) {
            no_more_needed = true;
            break;
        }
    }
    if (sema_locked) {
        unlock_sema();
    }
    return no_more_needed;
}

// Send work by scanning the job array multiple times,
// with different selection criteria on each scan.
//
void send_work_old() {
    g_wreq->beta_only = false;
    g_wreq->user_apps_only = true;
    g_wreq->infeasible_only = false;

    // give top priority to results that require a 'reliable host'
    //
    if (g_wreq->has_reliable_version) {
        g_wreq->reliable_only = true;
        if (config.debug_send_scan) {
            log_messages.printf(MSG_NORMAL,
                "[send_scan] scanning for jobs that need reliable host\n"
            );
        }
        if (scan_work_array()) return;
        g_wreq->reliable_only = false;
        g_wreq->best_app_versions.clear();
    } else {
        if (config.debug_send_scan) {
            log_messages.printf(MSG_NORMAL,
                "[send_scan] host has no reliable app versions; skipping scan\n"
            );
        }
    }

    // give 2nd priority to results for a beta app
    // (projects should load beta work with care,
    // otherwise your users won't get production work done!
    //
    if (g_wreq->project_prefs.allow_beta_work) {
        g_wreq->beta_only = true;
        if (config.debug_send_scan) {
            log_messages.printf(MSG_NORMAL,
                "[send_scan] host will accept beta jobs.  Scanning for them.\n"
            );
        }
        if (scan_work_array()) return;
        g_wreq->beta_only = false;
    }

    // give next priority to results that were infeasible for some other host
    //
    g_wreq->infeasible_only = true;
    if (config.debug_send_scan) {
        log_messages.printf(MSG_NORMAL,
            "[send_scan] Scanning for jobs that were infeasible for another host.\n"
        );
    }
    if (scan_work_array()) return;
    g_wreq->infeasible_only = false;

    // if some app uses locality sched lite,
    // make a pass accepting only jobs for which the client has a file
    //
    if (ssp->locality_sched_lite) {
        if (config.debug_send_scan) {
            log_messages.printf(MSG_NORMAL,
                "[send_scan] Scanning for locality sched Lite jobs.\n"
            );
        }
        g_wreq->locality_sched_lite = true;
        if (scan_work_array()) return;
        g_wreq->locality_sched_lite = false;
    }

    // end of high-priority cases.  Now do general scan.
    //
    if (config.debug_send_scan) {
        log_messages.printf(MSG_NORMAL,
            "[send_scan] Scanning: general case.\n"
        );
    }
    if (scan_work_array()) return;

    // If user has selected apps but will accept any,
    // and we haven't found any jobs for selected apps, try others
    //
    if (!g_wreq->njobs_sent && g_wreq->project_prefs.allow_non_preferred_apps ) {
        g_wreq->user_apps_only = false;
        selected_app_message_index = g_wreq->no_work_messages.size();
        if (config.debug_send_scan) {
            log_messages.printf(MSG_NORMAL,
                "[send_scan] scanning for jobs from non-selected applications\n"
            );
        }
        if (scan_work_array()) return;
    }
}
