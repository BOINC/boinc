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

#include "boinc_db.h"
#include "str_util.h"
#include "str_replace.h"
#include "parse.h"

#include "credit.h"
#include "sched_types.h"
#include "sched_msgs.h"
#include "sched_util.h"
#include "sched_config.h"

#include "sched_result.h"

static inline void got_good_result(SCHED_RESULT_ITEM& sri) {
    int gavid = generalized_app_version_id(sri.app_version_id, sri.appid);
    DB_HOST_APP_VERSION* havp = gavid_to_havp(gavid);
    if (!havp) {
        if (config.debug_handle_results) {
            log_messages.printf(MSG_NORMAL,
                "[handle] No app version for %d\n", gavid
            );
        }
        return;
    }
    havp->max_jobs_per_day *= 2;
    if (havp->max_jobs_per_day > config.daily_result_quota) {
        havp->max_jobs_per_day = config.daily_result_quota;
    }
}

static inline void got_bad_result(SCHED_RESULT_ITEM& sri, double delay_bound) {
    int gavid = generalized_app_version_id(sri.app_version_id, sri.appid);
    DB_HOST_APP_VERSION* havp = gavid_to_havp(gavid);
    if (!havp) {
        if (config.debug_handle_results) {
            log_messages.printf(MSG_NORMAL,
                "[handle] No app version for %d\n", gavid
            );
        }
        return;
    }

    // if job was aborted (possibly by client scheduler) don't penalize
    //
    if (sri.client_state != RESULT_ABORTED) {
        havp->max_jobs_per_day -= 1;
        if (havp->max_jobs_per_day < 1) {
            havp->max_jobs_per_day = 1;
        }
    }

    // but put on host scale probation regardless
    //
    host_scale_probation(*havp, delay_bound);
}

// handle completed results
//
int handle_results() {
    DB_SCHED_RESULT_ITEM_SET result_handler;
    SCHED_RESULT_ITEM* srip;
    unsigned int i;
    int retval;
    RESULT* rp;
    bool changed_host=false;

    if (g_request->results.size() == 0) return 0;

    // allow projects to limit the # of results handled
    // (in case of server memory limits)
    //
    if (config.report_max && g_request->results.size() > config.report_max) {
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
    // xml_doc_out, outcome, validate_state, elapsed_time
    //
    retval = result_handler.enumerate();
    if (retval) {
        log_messages.printf(MSG_CRITICAL,
            "[HOST#%d] Batch query failed\n",
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
                "[HOST#%d] [RESULT#? %s] can't find result\n",
                g_reply->host.id, rp->name
            );

            g_reply->result_acks.push_back(std::string(rp->name));
            continue;
        }

        if (config.debug_handle_results) {
            log_messages.printf(MSG_NORMAL,
                "[handle] [HOST#%d] [RESULT#%d] [WU#%d] got result (DB: server_state=%d outcome=%d client_state=%d validate_state=%d delete_state=%d)\n",
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
            const char *dont_replace_result = NULL;
            switch (srip->outcome) {
                case RESULT_OUTCOME_INIT:
                    // should never happen!
                    dont_replace_result = "this result was never sent";
                    break;
                case RESULT_OUTCOME_SUCCESS:
                    // don't replace a successful result!
                    dont_replace_result = "result already reported as success";
                    break;
                case RESULT_OUTCOME_COULDNT_SEND:
                    // should never happen!
                    dont_replace_result = "this result couldn't be sent";
                    break;
                case RESULT_OUTCOME_CLIENT_ERROR:
                    // should never happen!
                    dont_replace_result = "result already reported as error";
                    break;
                case RESULT_OUTCOME_CLIENT_DETACHED:
                case RESULT_OUTCOME_NO_REPLY:
                    // result is late in arriving, but keep it anyhow
                    break;
                case RESULT_OUTCOME_DIDNT_NEED:
                    // should never happen
                    dont_replace_result = "this result wasn't sent (not needed)";
                    break;
                case RESULT_OUTCOME_VALIDATE_ERROR:
                    // we already passed through the validator, so
                    // don't keep the new result
                    dont_replace_result = "result already reported, validate error";
                    break;
                default:
                    dont_replace_result = "server logic bug; please alert BOINC developers";
                    break;
            }
            if (dont_replace_result) {
                char buf[256];
                log_messages.printf(MSG_CRITICAL,
                    "[HOST#%d] [RESULT#%d] [WU#%d] result already over [outcome=%d validate_state=%d]: %s\n",
                    g_reply->host.id, srip->id, srip->workunitid, srip->outcome,
                    srip->validate_state, dont_replace_result
                );
                sprintf(buf, "Completed result %s refused: %s", srip->name, dont_replace_result);
                g_reply->insert_message(buf, "high");
                srip->id = 0;
                g_reply->result_acks.push_back(std::string(rp->name));
                continue;
            }
        }

        if (srip->server_state == RESULT_SERVER_STATE_UNSENT) {
            log_messages.printf(MSG_CRITICAL,
                "[HOST#%d] [RESULT#%d] [WU#%d] got unexpected result: server state is %d\n",
                g_reply->host.id, srip->id, srip->workunitid, srip->server_state
            );
            srip->id = 0;
            g_reply->result_acks.push_back(std::string(rp->name));
            continue;
        }

        if (srip->received_time) {
            log_messages.printf(MSG_CRITICAL,
                "[HOST#%d] [RESULT#%d] [WU#%d] already got result, at %s \n",
                g_reply->host.id, srip->id, srip->workunitid,
                time_to_string(srip->received_time)
            );
            srip->id = 0;
            g_reply->result_acks.push_back(std::string(rp->name));
            continue;
        }

        if (srip->hostid != g_reply->host.id) {
            log_messages.printf(MSG_CRITICAL,
                "[HOST#%d] [RESULT#%d] [WU#%d] got result from wrong host; expected [HOST#%d]\n",
                g_reply->host.id, srip->id, srip->workunitid, srip->hostid
            );
            DB_HOST result_host;
            retval = result_host.lookup_id(srip->hostid);

            if (retval) {
                log_messages.printf(MSG_CRITICAL,
                    "[RESULT#%d] [WU#%d] Can't lookup [HOST#%d]\n",
                    srip->id, srip->workunitid, srip->hostid
                );
                srip->id = 0;
                g_reply->result_acks.push_back(std::string(rp->name));
                continue;
            } else if (result_host.userid != g_reply->host.userid) {
                log_messages.printf(MSG_CRITICAL,
                    "[USER#%d] [HOST#%d] [RESULT#%d] [WU#%d] Not even the same user; expected [USER#%d]\n",
                    g_reply->host.userid, g_reply->host.id, srip->id, srip->workunitid, result_host.userid
                );
                srip->id = 0;
                g_reply->result_acks.push_back(std::string(rp->name));
                continue;
            } else {
                log_messages.printf(MSG_CRITICAL,
                    "[HOST#%d] [RESULT#%d] [WU#%d] Allowing result because same USER#%d\n",
                    g_reply->host.id, srip->id, srip->workunitid, g_reply->host.userid
                );
                changed_host = true;
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

        // check for impossible CPU time
        //
        double turnaround_time = srip->received_time - srip->sent_time;
        if (turnaround_time < 0) {
            log_messages.printf(MSG_NORMAL,
                "[HOST#%d] [RESULT#%d] [WU#%d] inconsistent sent/received times\n", srip->hostid, srip->id, srip->workunitid
            );
        } else {
            if (srip->elapsed_time > turnaround_time) {
                log_messages.printf(MSG_NORMAL,
                    "[HOST#%d] [RESULT#%d] [WU#%d] excessive elapsed time: reported %f > elapsed %f%s\n",
                    srip->hostid, srip->id, srip->workunitid,
                    srip->elapsed_time, turnaround_time,
                    changed_host?" [OK: HOST changed]":""
                );
            }
        }

        srip->exit_status = rp->exit_status;
        srip->app_version_num = rp->app_version_num;

        // TODO: this is outdated, and doesn't belong here

        if (rp->fpops_cumulative || rp->intops_cumulative) {
            srip->claimed_credit = fpops_to_credit(rp->fpops_cumulative, rp->intops_cumulative);
        } else if (rp->fpops_per_cpu_sec || rp->intops_per_cpu_sec) {
            srip->claimed_credit = fpops_to_credit(
                rp->fpops_per_cpu_sec*srip->cpu_time,
                rp->intops_per_cpu_sec*srip->cpu_time
            );
        } else {
            srip->claimed_credit = srip->cpu_time * g_reply->host.claimed_credit_per_cpu_sec;
        }

        if (config.use_credit_multiplier) {
            // Regardless of the method of claiming credit,
            // multiply by the application's credit multiplier
            // at the time of result creation.
            //
            srip->claimed_credit *= credit_multiplier(srip->appid,srip->sent_time);
        }

        if (config.debug_handle_results) {
            log_messages.printf(MSG_NORMAL,
                "[handle] cpu time %f credit/sec %f, claimed credit %f\n", srip->cpu_time, g_reply->host.claimed_credit_per_cpu_sec, srip->claimed_credit
            );
        }
        srip->server_state = RESULT_SERVER_STATE_OVER;

        strlcpy(srip->stderr_out, rp->stderr_out, sizeof(srip->stderr_out));
        strlcpy(srip->xml_doc_out, rp->xml_doc_out, sizeof(srip->xml_doc_out));

        // look for exit status and app version in stderr_out
        // (historical - can be deleted at some point)
        //
        parse_int(srip->stderr_out, "<exit_status>", srip->exit_status);
        parse_int(srip->stderr_out, "<app_version>", srip->app_version_num);

        if ((srip->client_state == RESULT_FILES_UPLOADED) && (srip->exit_status == 0)) {
            srip->outcome = RESULT_OUTCOME_SUCCESS;
            if (config.debug_handle_results) {
                log_messages.printf(MSG_NORMAL,
                    "[handle] [RESULT#%d] [WU#%d]: setting outcome SUCCESS\n",
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
                    "[handle] [RESULT#%d] [WU#%d]: client_state %d exit_status %d; setting outcome ERROR\n",
                    srip->id, srip->workunitid, srip->client_state, srip->exit_status
                );
            }
            srip->outcome = RESULT_OUTCOME_CLIENT_ERROR;
            srip->validate_state = VALIDATE_STATE_INVALID;

            // penalize result quota
            //
            DB_WORKUNIT wu;
            int delay_bound;
            wu.id = srip->workunitid;
            retval = wu.get_field_int("delay_bound", delay_bound);
            if (!retval) {
                got_bad_result(*srip, (double) delay_bound);
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
                "[HOST#%d] [RESULT#%d] [WU#%d] can't update result: %s\n",
                g_reply->host.id, sri.id, sri.workunitid, boinc_db.error_string()
            );
        } else {
            g_reply->result_acks.push_back(std::string(sri.name));
        }
    }

    // set transition_time for the results' WUs
    //
    retval = result_handler.update_workunits();
    if (retval) {
        log_messages.printf(MSG_CRITICAL,
            "[HOST#%d] can't update WUs: %d\n",
            g_reply->host.id, retval
        );
    }
    return 0;
}
