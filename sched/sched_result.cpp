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

// Logic for handling completed jobs being reported in scheduler requests

#include "boinc_db.h"
#include "str_util.h"
#include "str_replace.h"
#include "parse.h"

#include "credit.h"
#include "sched_types.h"
#include "sched_msgs.h"
#include "sched_util.h"
#include "sched_main.h"
#include "sched_config.h"

#include "sched_result.h"
#include <ctime>

// got a SUCCESS result; double max jobs per day.
// TODO: shouldn't we do this only for valid results?
//
static inline void got_good_result(SCHED_RESULT_ITEM& sri) {
    DB_ID_TYPE gavid = generalized_app_version_id(sri.app_version_id, sri.appid);
    DB_HOST_APP_VERSION* havp = gavid_to_havp(gavid);
    if (!havp) {
        if (config.debug_handle_results) {
            log_messages.printf(MSG_NORMAL,
                "[handle] No HOST_APP_VERSION for app version %ld\n", gavid
            );
        }
        return;
    }
    if (havp->max_jobs_per_day < config.daily_result_quota) {
        int n = havp->max_jobs_per_day*2;
        if (n > config.daily_result_quota) {
            n = config.daily_result_quota;
        }
        if (config.debug_quota) {
            log_messages.printf(MSG_NORMAL,
                "[quota] increasing max_jobs_per_day for %ld: %d->%d\n",
                gavid, havp->max_jobs_per_day, n
            );
        }
        havp->max_jobs_per_day = n;
    }
}

// This is called then a job crashed or exceeded limits on a host.
// Enforce
// - mechanism that reduces jobs per day to that host
// - mechanism that categorizes hosts as "reliable"
//
static inline void got_bad_result(SCHED_RESULT_ITEM& sri) {
    DB_ID_TYPE gavid = generalized_app_version_id(sri.app_version_id, sri.appid);
    DB_HOST_APP_VERSION* havp = gavid_to_havp(gavid);
    if (!havp) {
        if (config.debug_handle_results) {
            log_messages.printf(MSG_NORMAL,
                "[handle] No HOST_APP_VERSION version for app version %ld\n", gavid
            );
        }
        return;
    }

    int n = havp->max_jobs_per_day;
    if (n > config.daily_result_quota) {
        n = config.daily_result_quota;
    }
    n -= 1;
    if (n < 1) {
        n = 1;
    }
    if (config.debug_quota) {
        log_messages.printf(MSG_NORMAL,
            "[quota] decreasing max_jobs_per_day for %ld: %d->%d\n",
            gavid, havp->max_jobs_per_day, n
        );
    }
    havp->max_jobs_per_day = n;

    havp->consecutive_valid = 0;
}

// handle completed results
//
int handle_results() {
    DB_SCHED_RESULT_ITEM_SET result_handler;
    SCHED_RESULT_ITEM* srip;
    unsigned int i;
    int retval;
    RESULT* rp;

    if (g_request->results.size() == 0) return 0;

    // allow projects to limit the # of results handled
    // (in case of server memory limits)
    //
    if (config.report_max
        && (int)g_request->results.size() > config.report_max
    ) {
        g_request->results.resize(config.report_max);
    }

    // copy reported results to a separate vector, "result_handler",
    // initially with only the "name" field present
    //
    for (i=0; i<g_request->results.size(); i++) {
        result_handler.add_result(g_request->results[i].name);
    }

    // read results from database into "result_handler".
    //
    // Quantities that must be read from the DB are those
    // where srip (see below) appears as an rval.
    // These are: id, name, server_state, received_time, hostid, validate_state.
    //
    // Quantities that must be written to the DB are those for
    // which srip appears as an lval. These are:
    // hostid, teamid, received_time, client_state, cpu_time, exit_status,
    // app_version_num, claimed_credit, server_state, stderr_out,
    // xml_doc_out, outcome, validate_state, elapsed_time,
    // and peak_*
    //
    retval = result_handler.enumerate();
    if (retval) {
        log_messages.printf(MSG_CRITICAL,
            "[HOST#%lu] Batch query failed\n",
            g_reply->host.id
        );
    }

    // loop over results reported by client
    //
    // A note about acks: we send an ack for result received if either
    // 1) there's some problem with it (wrong state, host, not in DB) or
    // 2) we update it successfully.
    // In other words, the only time we don't ack a result is when
    // it looks OK but the update failed.
    //
    for (i=0; i<g_request->results.size(); i++) {
        rp = &g_request->results[i];

        retval = result_handler.lookup_result(rp->name, &srip);
        if (retval) {
            log_messages.printf(MSG_CRITICAL,
                "[HOST#%lu] [RESULT#? %s] reported result not in DB\n",
                g_reply->host.id, rp->name
            );

            g_reply->result_acks.push_back(std::string(rp->name));
            continue;
        }

        if (config.debug_handle_results) {
            log_messages.printf(MSG_NORMAL,
                "[handle] [HOST#%lu] [RESULT#%lu] [WU#%lu] got result (DB: server_state=%d outcome=%d client_state=%d validate_state=%d delete_state=%d)\n",
                g_reply->host.id, srip->id, srip->workunitid, srip->server_state,
                srip->outcome, srip->client_state, srip->validate_state,
                srip->file_delete_state
            );
        }

        // Do various sanity checks.
        // If one of them fails, set srip->id = 0,
        // which suppresses the DB update later on
        //

        // If result has server_state OVER
        //   if outcome NO_REPLY accept it (it's just late).
        //   else ignore it
        //
        if (srip->server_state == RESULT_SERVER_STATE_OVER) {
            const char *msg = NULL;
            switch (srip->outcome) {
                case RESULT_OUTCOME_INIT:
                    // should never happen!
                    msg = "this result was never sent";
                    break;
                case RESULT_OUTCOME_SUCCESS:
                    // don't replace a successful result!
                    msg = "result already reported as success";

                    // Client is reporting a result twice.
                    // That could mean it didn't get the first reply.
                    // That reply may have contained new jobs.
                    // So make sure we resend lost jobs
                    //
                    g_wreq->resend_lost_results = true;
                    break;
                case RESULT_OUTCOME_COULDNT_SEND:
                    // should never happen!
                    msg = "this result couldn't be sent";
                    break;
                case RESULT_OUTCOME_CLIENT_ERROR:
                    // should never happen!
                    msg = "result already reported as error";
                    break;
                case RESULT_OUTCOME_CLIENT_DETACHED:
                case RESULT_OUTCOME_NO_REPLY:
                    // result is late in arriving, but keep it anyhow
                    break;
                case RESULT_OUTCOME_DIDNT_NEED:
                    // should never happen
                    msg = "this result wasn't sent (not needed)";
                    break;
                case RESULT_OUTCOME_VALIDATE_ERROR:
                    // we already passed through the validator, so
                    // don't keep the new result
                    msg = "result already reported, validate error";
                    break;
                default:
                    msg = "server logic bug; please alert BOINC developers";
                    break;
            }
            if (msg) {
                if (config.debug_handle_results) {
                    log_messages.printf(MSG_NORMAL,
                        "[handle][HOST#%lu][RESULT#%lu][WU#%lu] result already over [outcome=%d validate_state=%d]: %s\n",
                        g_reply->host.id, srip->id, srip->workunitid,
                        srip->outcome, srip->validate_state, msg
                    );
                }
                srip->id = 0;
                g_reply->result_acks.push_back(std::string(rp->name));
                continue;
            }
        }

        if (srip->server_state == RESULT_SERVER_STATE_UNSENT) {
            log_messages.printf(MSG_CRITICAL,
                "[HOST#%lu] [RESULT#%lu] [WU#%lu] got unexpected result: server state is %d\n",
                g_reply->host.id, srip->id, srip->workunitid, srip->server_state
            );
            srip->id = 0;
            g_reply->result_acks.push_back(std::string(rp->name));
            continue;
        }

        if (srip->received_time) {
            log_messages.printf(MSG_CRITICAL,
                "[HOST#%lu] [RESULT#%lu] [WU#%lu] already got result, at %s \n",
                g_reply->host.id, srip->id, srip->workunitid,
                time_to_string(srip->received_time)
            );
            srip->id = 0;
            g_reply->result_acks.push_back(std::string(rp->name));
            continue;
        }

        if (srip->hostid != g_reply->host.id) {
            log_messages.printf(MSG_CRITICAL,
                "[HOST#%lu] [RESULT#%lu] [WU#%lu] got result from wrong host; expected [HOST#%lu]\n",
                g_reply->host.id, srip->id, srip->workunitid, srip->hostid
            );
            DB_HOST result_host;
            retval = result_host.lookup_id(srip->hostid);

            if (retval) {
                log_messages.printf(MSG_CRITICAL,
                    "[RESULT#%lu] [WU#%lu] Can't lookup [HOST#%lu]\n",
                    srip->id, srip->workunitid, srip->hostid
                );
                srip->id = 0;
                g_reply->result_acks.push_back(std::string(rp->name));
                continue;
            } else if (result_host.userid != g_reply->host.userid) {
                log_messages.printf(MSG_CRITICAL,
                    "[USER#%lu] [HOST#%lu] [RESULT#%lu] [WU#%lu] Not even the same user; expected [USER#%lu]\n",
                    g_reply->host.userid, g_reply->host.id, srip->id, srip->workunitid, result_host.userid
                );
                srip->id = 0;
                g_reply->result_acks.push_back(std::string(rp->name));
                continue;
            } else {
                log_messages.printf(MSG_CRITICAL,
                    "[HOST#%lu] [RESULT#%lu] [WU#%lu] Allowing result because same USER#%lu\n",
                    g_reply->host.id, srip->id, srip->workunitid, g_reply->host.userid
                );
            }
        } // hostids do not match

        // Modify the in-memory copy obtained from the DB earlier.
        // If we found a problem above,
        // we have continued and skipped this modify
        //
        srip->hostid = g_reply->host.id;
        srip->teamid = g_reply->user.teamid;
        srip->received_time = time(0);
        srip->client_state = rp->client_state;
        srip->cpu_time = rp->cpu_time;
        srip->elapsed_time = rp->elapsed_time;
        srip->peak_working_set_size = rp->peak_working_set_size;
        srip->peak_swap_size = rp->peak_swap_size;
        srip->peak_disk_usage = rp->peak_disk_usage;

        // elapsed time is used to compute credit.
        // do various sanity checks on it.

        // 1) Some buggy clients report very low elapsed time
        // but actual CPU time;
        // if it's a single-thread app, set ET = CPU
        //
        if (srip->elapsed_time < srip->cpu_time) {
            DB_ID_TYPE avid = srip->app_version_id;
            if (avid > 0) {
                APP_VERSION* avp = ssp->lookup_app_version(avid);
                if (avp && !avp->is_multithread()) {
                    log_messages.printf(MSG_NORMAL,
                        "[HOST#%lu] [RESULT#%lu] elapsed time %f < CPU %f for seq app; setting to CPU\n",
                        srip->hostid, srip->id, srip->elapsed_time, srip->cpu_time
                    );
                    srip->elapsed_time = srip->cpu_time;
                }
            }
        }

        // 2) If it's negative, set to zero
        //
        if (srip->elapsed_time < 0) {
            log_messages.printf(MSG_NORMAL,
                "[HOST#%lu] [RESULT#%lu] [WU#%lu] negative elapsed time: %f\n",
                srip->hostid, srip->id, srip->workunitid,
                srip->elapsed_time
            );
            srip->elapsed_time = 0;
        }

        // 3) If it's zero, set to CPU time.
        //
        if (srip->elapsed_time == 0 && srip->cpu_time > 0) {
            srip->elapsed_time = srip->cpu_time;
            log_messages.printf(MSG_NORMAL,
                "[HOST#%lu] [RESULT#%lu] elapsed time is zero; setting to CPU time %f\n",
                srip->hostid, srip->id, srip->cpu_time
            );
        }

        // 4) If it's greater than turnaround time, set to turnaround time
        //
        double turnaround_time = srip->received_time - srip->sent_time;
        if (turnaround_time < 0) {
            log_messages.printf(MSG_CRITICAL,
                "[HOST#%lu] [RESULT#%lu] [WU#%lu] inconsistent sent/received times\n",
                srip->hostid, srip->id, srip->workunitid
            );
        } else {
            if (srip->elapsed_time > turnaround_time) {
                log_messages.printf(MSG_NORMAL,
                    "[HOST#%lu] [RESULT#%lu] [WU#%lu] impossible elapsed time: reported %f > turnaround %f\n",
                    srip->hostid, srip->id, srip->workunitid,
                    srip->elapsed_time, turnaround_time
                );
                srip->elapsed_time = turnaround_time;
            }
        }

        // Now do sanity check on CPU time
        //
        if (srip->cpu_time > srip->elapsed_time*g_reply->host.p_ncpus) {
            log_messages.printf(MSG_NORMAL,
                "[HOST#%lu] [RESULT#%lu] [WU#%lu] impossible CPU time: %f > %f * %d\n",
                srip->hostid, srip->id, srip->workunitid,
                srip->cpu_time, srip->elapsed_time, g_reply->host.p_ncpus
            );
            srip->cpu_time = srip->elapsed_time*g_reply->host.p_ncpus;
        }

        srip->exit_status = rp->exit_status;
        srip->app_version_num = rp->app_version_num;
        srip->server_state = RESULT_SERVER_STATE_OVER;

        strlcpy(srip->stderr_out, rp->stderr_out, sizeof(srip->stderr_out));
        strlcpy(srip->xml_doc_out, rp->xml_doc_out, sizeof(srip->xml_doc_out));

        if ((srip->client_state == RESULT_FILES_UPLOADED) && (srip->exit_status == 0)) {
            srip->outcome = RESULT_OUTCOME_SUCCESS;
            if (config.debug_handle_results) {
                log_messages.printf(MSG_NORMAL,
                    "[handle] [RESULT#%lu] [WU#%lu]: setting outcome SUCCESS\n",
                    srip->id, srip->workunitid
                );
            }
            got_good_result(*srip);

            if (config.dont_store_success_stderr) {
                strcpy(srip->stderr_out, "");
            }
        } else {
            if (config.debug_handle_results) {
                log_messages.printf(MSG_NORMAL,
                    "[handle] [RESULT#%lu] [WU#%lu]: client_state %d exit_status %d; setting outcome ERROR\n",
                    srip->id, srip->workunitid, srip->client_state, srip->exit_status
                );
            }
            srip->outcome = RESULT_OUTCOME_CLIENT_ERROR;
            srip->validate_state = VALIDATE_STATE_INVALID;

            // adjust quota and reset consecutive valid
            // (but not if aborted by project)
            //
            if (srip->exit_status != EXIT_ABORTED_BY_PROJECT) {
                got_bad_result(*srip);
            }
        }
    } // loop over all incoming results

    // Update the result records
    // (skip items that we previously marked to skip)
    //
    for (i=0; i<result_handler.results.size(); i++) {
        SCHED_RESULT_ITEM& sri = result_handler.results[i];
        if (sri.id == 0) continue;
        retval = result_handler.update_result(sri);
        if (retval) {
            log_messages.printf(MSG_CRITICAL,
                "[HOST#%lu] [RESULT#%lu] [WU#%lu] can't update result: %s\n",
                g_reply->host.id, sri.id, sri.workunitid, boinc_db.error_string()
            );
        }
        if (retval == 0 || retval == ERR_DB_NOT_FOUND) {
            g_reply->result_acks.push_back(std::string(sri.name));
        }
    }

    // set transition_time for the results' WUs
    //
    retval = result_handler.update_workunits();
    if (retval) {
        log_messages.printf(MSG_CRITICAL,
            "[HOST#%lu] can't update WUs: %s\n",
            g_reply->host.id, boincerror(retval)
        );
    }
    return 0;
}
