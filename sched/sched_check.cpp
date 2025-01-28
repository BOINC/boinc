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

// Check whether a job can be sent to a host

#include "credit.h"
#include "sched_config.h"
#include "sched_hr.h"
#include "sched_main.h"
#include "sched_msgs.h"
#include "sched_send.h"
#include "sched_check.h"
#include "boinc_stdio.h"

const char* infeasible_string(int code) {
    switch (code) {
    case INFEASIBLE_MEM: return "Not enough memory";
    case INFEASIBLE_DISK: return "Not enough disk";
    case INFEASIBLE_CPU: return "CPU too slow";
    case INFEASIBLE_APP_SETTING: return "App not selected";
    case INFEASIBLE_WORKLOAD: return "Existing workload";
    case INFEASIBLE_DUP: return "Already in reply";
    case INFEASIBLE_HR: return "Homogeneous redundancy";
    case INFEASIBLE_BANDWIDTH: return "Download bandwidth too low";
    }
    return "Unknown";
}

// Return true if the user has set application preferences,
// and excluded this app
//
bool app_not_selected(int appid) {
    unsigned int i;

    if (g_wreq->project_prefs.selected_apps.size() == 0) return false;
    for (i=0; i<g_wreq->project_prefs.selected_apps.size(); i++) {
        if (appid == g_wreq->project_prefs.selected_apps[i].appid) {
            g_wreq->project_prefs.selected_apps[i].work_available = true;
            return false;
        }
    }
    return true;
}

static inline int check_memory(WORKUNIT& wu) {
    double diff = wu.rsc_memory_bound - g_wreq->usable_ram;
    if (diff > 0) {
        char message[256];
        sprintf(message,
            "%s needs %0.2f MB RAM but only %0.2f MB is available for use.",
            find_user_friendly_name(wu.appid),
            wu.rsc_memory_bound/MEGA, g_wreq->usable_ram/MEGA
        );
        add_no_work_message(message);

        if (config.debug_send_job) {
            log_messages.printf(MSG_NORMAL,
                "[send_job] [WU#%lu %s] needs %0.2fMB RAM; [HOST#%lu] has %0.2fMB, %0.2fMB usable\n",
                wu.id, wu.name, wu.rsc_memory_bound/MEGA,
                g_reply->host.id, g_wreq->ram/MEGA, g_wreq->usable_ram/MEGA
            );
        }
        g_wreq->mem.set_insufficient(wu.rsc_memory_bound);
        g_reply->set_delay(DELAY_NO_WORK_TEMP);
        return INFEASIBLE_MEM;
    }
    return 0;
}

static inline int check_disk(WORKUNIT& wu) {
    double diff = wu.rsc_disk_bound - g_wreq->disk_available;
    if (diff > 0) {
        char message[256];
        sprintf(message,
            "%s needs %0.2fMB more disk space.  You currently have %0.2f MB available and it needs %0.2f MB.",
            find_user_friendly_name(wu.appid),
            diff/MEGA, g_wreq->disk_available/MEGA, wu.rsc_disk_bound/MEGA
        );
        add_no_work_message(message);

        g_wreq->disk.set_insufficient(diff);
        return INFEASIBLE_DISK;
    }
    return 0;
}

static inline int check_bandwidth(WORKUNIT& wu) {
    if (wu.rsc_bandwidth_bound == 0) return 0;

    // if n_bwdown is zero, the host has never downloaded anything,
    // so skip this check
    //
    if (g_reply->host.n_bwdown == 0) return 0;

    double diff = wu.rsc_bandwidth_bound - g_reply->host.n_bwdown;
    if (diff > 0) {
        char message[256];
        sprintf(message,
            "%s requires %0.2f KB/sec download bandwidth.  Your computer has been measured at %0.2f KB/sec.",
            find_user_friendly_name(wu.appid),
            wu.rsc_bandwidth_bound/KILO, g_reply->host.n_bwdown/KILO
        );
        add_no_work_message(message);

        g_wreq->bandwidth.set_insufficient(diff);
        return INFEASIBLE_BANDWIDTH;
    }
    return 0;
}

// Determine if the app is "hard",
// and we should send it only to high-end hosts.
// Currently this is specified by setting weight=-1;
// this is a kludge for SETI@home/Astropulse.
//
static inline bool hard_app(APP& app) {
    return (app.weight == -1);
}

static inline double get_estimated_delay(BEST_APP_VERSION& bav) {
    int pt = bav.host_usage.proc_type;
    if (pt == PROC_TYPE_CPU) {
        return g_request->cpu_estimated_delay;
    }
    COPROC* cp = g_request->coprocs.proc_type_to_coproc(pt);
    return cp->estimated_delay;
}


// return the delay bound to use for this job/host.
// Actually, return two: optimistic (lower) and pessimistic (higher).
// If the deadline check with the optimistic bound fails,
// try the pessimistic bound.
// TODO: clean up this mess
//
static void get_delay_bound_range(
    WORKUNIT& wu,
    int res_server_state, int res_priority, double res_report_deadline,
    BEST_APP_VERSION& bav,
    double& opt, double& pess
) {
    if (res_server_state == RESULT_SERVER_STATE_IN_PROGRESS) {
        double now = dtime();
        if (res_report_deadline < now) {
            // if original deadline has passed, return zeros
            // This will skip deadline check.
            opt = pess = 0;
            return;
        }
        opt = res_report_deadline - now;
        pess = wu.delay_bound;
    } else {
        opt = pess = wu.delay_bound;

        // If the workunit needs reliable and is being sent to a reliable host,
        // then shorten the delay bound by the percent specified
        //
        if (config.reliable_on_priority && res_priority >= config.reliable_on_priority && config.reliable_reduced_delay_bound > 0.01
        ) {
            opt = wu.delay_bound*config.reliable_reduced_delay_bound;
            double est_wallclock_duration = estimate_duration(wu, bav);

            // Check to see how reasonable this reduced time is.
            // Increase it to twice the estimated delay bound
            // if all the following apply:
            //
            // 1) Twice the estimate is longer then the reduced delay bound
            // 2) Twice the estimate is less then the original delay bound
            // 3) Twice the estimate is less then the twice the reduced delay bound
            if (est_wallclock_duration*2 > opt
                && est_wallclock_duration*2 < wu.delay_bound
                && est_wallclock_duration*2 < wu.delay_bound*config.reliable_reduced_delay_bound*2
            ) {
                opt = est_wallclock_duration*2;
            }
        }
    }
}

// return 0 if the job, with the given delay bound,
// will complete by its deadline, and won't cause other jobs to miss deadlines.
//
static inline int check_deadline(
    WORKUNIT& wu, APP& app, BEST_APP_VERSION& bav
) {
    if (config.ignore_delay_bound) return 0;

    // skip delay check if host currently doesn't have any work
    // and it's not a hard app.
    // (i.e. everyone gets one result, no matter how slow they are)
    //
    if (get_estimated_delay(bav) == 0 && !hard_app(app)) {
        if (config.debug_send_job) {
            log_messages.printf(MSG_NORMAL,
                "[send_job] [WU#%lu] est delay 0, skipping deadline check\n",
                wu.id
            );
        }
        return 0;
    }

    // if it's a hard app, don't send it to a host with no credit
    //
    if (hard_app(app) && g_reply->host.total_credit == 0) {
        return INFEASIBLE_CPU;
    }

    // do EDF simulation if possible; else use cruder approximation
    //
    if (config.workload_sim && g_request->have_other_results_list) {
        double est_dur = estimate_duration(wu, bav);
        if (g_reply->wreq.edf_reject_test(est_dur, wu.delay_bound)) {
            return INFEASIBLE_WORKLOAD;
        }
        IP_RESULT candidate("", wu.delay_bound, est_dur);
        safe_strcpy(candidate.name, wu.name);
        if (check_candidate(candidate, g_wreq->effective_ncpus, g_request->ip_results)) {
            // it passed the feasibility test,
            // but don't add it to the workload yet;
            // wait until we commit to sending it
        } else {
            g_reply->wreq.edf_reject(est_dur, wu.delay_bound);
            g_reply->wreq.speed.set_insufficient(0);
            return INFEASIBLE_WORKLOAD;
        }
    } else {
        double ewd = estimate_duration(wu, bav);
        if (hard_app(app)) ewd *= 1.3;
        double est_report_delay = get_estimated_delay(bav) + ewd;
        double diff = est_report_delay - wu.delay_bound;
        if (diff > 0) {
            if (config.debug_send_job) {
                log_messages.printf(MSG_NORMAL,
                    "[send_job] [WU#%lu] deadline miss %d > %d\n",
                    wu.id, (int)est_report_delay, wu.delay_bound
                );
            }
            g_reply->wreq.speed.set_insufficient(diff);
            return INFEASIBLE_CPU;
        } else {
            if (config.debug_send_job) {
                log_messages.printf(MSG_NORMAL,
                    "[send_job] [WU#%lu] meets deadline: %.2f + %.2f < %d\n",
                    wu.id, get_estimated_delay(bav), ewd, wu.delay_bound
                );
            }
        }
    }
    return 0;
}

// Fast checks (no DB access) to see if the job can be sent to the host.
// Reasons why not include:
// 1) the host doesn't have enough memory;
// 2) the host doesn't have enough disk space;
// 3) based on CPU speed, resource share and estimated delay,
//    the host probably won't get the result done within the delay bound
// 4) app isn't in user's "approved apps" list
//
// If the job is feasible, return 0 and fill in wu.delay_bound
// with the delay bound we've decided to use.
//
int wu_is_infeasible_fast(
    WORKUNIT& wu,
    int res_server_state, int res_priority, double res_report_deadline,
    APP& app, BEST_APP_VERSION& bav
) {
    int retval;

    // project-specific check
    //
    if (wu_is_infeasible_custom(wu, app, bav)) {
        return INFEASIBLE_CUSTOM;
    }

    if (config.user_filter) {
        if (wu.batch && wu.batch != g_reply->user.id) {
            return INFEASIBLE_USER_FILTER;
        }
    }

    // homogeneous redundancy: can't send if app uses HR and
    // 1) host is of unknown HR class, or
    // 2) WU is already committed to different HR class
    //
    if (app_hr_type(app)) {
        if (hr_unknown_class(g_reply->host, app_hr_type(app))) {
            if (config.debug_send_job) {
                log_messages.printf(MSG_NORMAL,
                    "[send_job] [HOST#%lu] [WU#%lu %s] host is of unknown class in HR type %d\n",
                    g_reply->host.id, wu.id, wu.name, app_hr_type(app)
                );
            }
            return INFEASIBLE_HR;
        }
        if (already_sent_to_different_hr_class(wu, app)) {
            if (config.debug_send_job) {
                log_messages.printf(MSG_NORMAL,
                    "[send_job] [HOST#%lu] [WU#%lu %s] failed quick HR check: WU is class %d, host is class %d\n",
                    g_reply->host.id, wu.id, wu.name, wu.hr_class, hr_class(g_request->host, app_hr_type(app))
                );
            }
            return INFEASIBLE_HR;
        }
    }

    // homogeneous app version
    //
    if (app.homogeneous_app_version) {
        DB_ID_TYPE avid = wu.app_version_id;
        if (avid && bav.avp->id != avid) {
            if (config.debug_send_job) {
                log_messages.printf(MSG_NORMAL,
                    "[send_job] [HOST#%lu] [WU#%lu %s] failed homogeneous app version check: %lu %lu\n",
                    g_reply->host.id, wu.id, wu.name, avid, bav.avp->id
                );
            }
            return INFEASIBLE_HAV;
        }
    }

    if (config.one_result_per_user_per_wu || config.one_result_per_host_per_wu) {
        if (wu_already_in_reply(wu)) {
            return INFEASIBLE_DUP;
        }
    }

    retval = check_memory(wu);
    if (retval) return retval;
    retval = check_disk(wu);
    if (retval) return retval;
    retval = check_bandwidth(wu);
    if (retval) return retval;

    if (app.non_cpu_intensive) {
        return 0;
    }

    // do deadline check last because EDF sim uses some CPU
    //
    double opt, pess;
    get_delay_bound_range(
        wu, res_server_state, res_priority, res_report_deadline, bav, opt, pess
    );
    wu.delay_bound = (int)opt;
    if (opt == 0) {
        // this is a resend; skip deadline check
        return 0;
    }
    retval = check_deadline(wu, app, bav);
    if (retval && (opt != pess)) {
        wu.delay_bound = (int)pess;
        retval = check_deadline(wu, app, bav);
    }
    return retval;
}

// Do checks that require DB access for whether we can send this job,
// and return:
// CHECK_OK if OK to send
// CHECK_NO_HOST if can't send to this host
// CHECK_NO_ANY if can't send to ANY host
//  e.g. WU error mask is nonzero
//
int slow_check(
    WU_RESULT& wu_result,       // the job cache entry.
        // We may refresh its hr_class and app_version_id fields.
    APP* app,
    BEST_APP_VERSION* bavp      // the app version to be used
) {
    int retval;
    long n;
    DB_RESULT result;
    char buf[256];
    WORKUNIT& wu = wu_result.workunit;

    // Don't send if we've already sent a result of this WU to this user.
    //
    if (config.one_result_per_user_per_wu) {
        sprintf(buf,
            "where workunitid=%lu and userid=%lu", wu.id, g_reply->user.id
        );
        retval = result.count(n, buf);
        if (retval) {
            log_messages.printf(MSG_CRITICAL,
                "send_work: can't get result count (%s)\n", boincerror(retval)
            );
            return CHECK_NO_HOST;
        } else {
            if (n>0) {
                if (config.debug_send_job) {
                    log_messages.printf(MSG_NORMAL,
                        "[send_job] [USER#%lu] already has %ld result(s) for [WU#%lu]\n",
                        g_reply->user.id, n, wu.id
                    );
                }
                return CHECK_NO_HOST;
            }
        }
    } else if (config.one_result_per_host_per_wu) {
        // Don't send if we've already sent a result of this WU to this host.
        // We only have to check this if we don't send one result per user.
        //
        sprintf(buf,
            "where workunitid=%lu and hostid=%lu", wu.id, g_reply->host.id
        );
        retval = result.count(n, buf);
        if (retval) {
            log_messages.printf(MSG_CRITICAL,
                "send_work: can't get result count (%s)\n", boincerror(retval)
            );
            return CHECK_NO_HOST;
        } else {
            if (n>0) {
                if (config.debug_send_job) {
                    log_messages.printf(MSG_NORMAL,
                        "[send_job] [HOST#%lu] already has %ld result(s) for [WU#%lu]\n",
                        g_reply->host.id, n, wu.id
                    );
                }
                return CHECK_NO_HOST;
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
                "can't get fields for [WU#%lu]: %s\n", db_wu.id, boincerror(retval)
            );
            return CHECK_NO_HOST;
        }

        // check wu.error_mask
        //
        if (vals[2] != 0) {
            return CHECK_NO_ANY;
        }

        if (app_hr_type(*app)) {
            wu.hr_class = vals[0];
            if (already_sent_to_different_hr_class(wu, *app)) {
                if (config.debug_send_job) {
                    log_messages.printf(MSG_NORMAL,
                        "[send_job] [HOST#%lu] [WU#%lu %s] is assigned to different HR class\n",
                        g_reply->host.id, wu.id, wu.name
                    );
                }
                // Mark the workunit as infeasible.
                // This ensures that jobs already assigned to an HR class
                // are processed first.
                //
                wu_result.infeasible_count++;
                return CHECK_NO_HOST;
            }
        }
        if (app->homogeneous_app_version) {
            int wu_avid = vals[1];
            wu.app_version_id = wu_avid;
            if (wu_avid && wu_avid != bavp->avp->id) {
                if (config.debug_send_job) {
                    log_messages.printf(MSG_NORMAL,
                        "[send_job] [HOST#%lu] [WU#%lu %s] is assigned to different app version\n",
                        g_reply->host.id, wu.id, wu.name
                    );
                }
                wu_result.infeasible_count++;
                return CHECK_NO_HOST;
            }
        }
    }
    return CHECK_OK;
}

// Check for pathological conditions that mean
// result is not sendable at all.
//
bool result_still_sendable(DB_RESULT& result, WORKUNIT& wu) {
    int retval = result.lookup_id(result.id);
    if (retval) {
        log_messages.printf(MSG_CRITICAL,
            "[RESULT#%lu] result.lookup_id() failed: %s\n",
            result.id, boincerror(retval)
        );
        return false;
    }
    if (result.server_state != RESULT_SERVER_STATE_UNSENT) {
        log_messages.printf(MSG_NORMAL,
            "[RESULT#%lu] expected to be unsent; instead, state is %d\n",
            result.id, result.server_state
        );
        return false;
    }
    if (result.workunitid != wu.id) {
        log_messages.printf(MSG_CRITICAL,
            "[RESULT#%lu] wrong WU ID: wanted %lu, got %lu\n",
            result.id, wu.id, result.workunitid
        );
        return false;
    }
    return true;
}
