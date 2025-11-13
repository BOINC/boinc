// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2019 University of California
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

// transitioner - handle transitions in the state of a WU
//    - a result has become DONE (via timeout or client reply)
//    - the WU error mask is set (e.g. by validater)
//    - assimilation is finished
//
// cmdline:
//   [ --one_pass ]          do one pass, then exit
//   [ --d x ]               debug level x
//   [ --mod n i ]           process only WUs with (id mod n) == i
//   [ --sleep_interval x ]  sleep x seconds if nothing to do
//   [ --wu_id n ]           transition WU n (debugging)

#include "config.h"
#include <vector>
#include <unistd.h>
#include <cstring>
#include <climits>
#include <cstdlib>
#include <string>
#include <signal.h>
#include <sys/time.h>
#include <sys/param.h>

#include "backend_lib.h"
#include "boinc_db.h"
#include "common_defs.h"
#include "error_numbers.h"
#include "filesys.h"
#include "str_util.h"
#include "svn_version.h"
#include "util.h"

#include "sched_config.h"
#include "credit.h"
#include "sched_util.h"
#include "sched_msgs.h"
#ifdef GCL_SIMULATOR
#include "gcl_simulator.h"
#endif

#define LOCKFILE                "transitioner.out"
#define PIDFILE                 "transitioner.pid"

#define SELECT_LIMIT    1000

#define DEFAULT_SLEEP_INTERVAL  5

int startup_time;
R_RSA_PRIVATE_KEY key;
int mod_n, mod_i;
bool do_mod = false;
bool one_pass = false;
int sleep_interval = DEFAULT_SLEEP_INTERVAL;
int wu_id = 0;

void signal_handler(int) {
    log_messages.printf(MSG_NORMAL, "Signaled by simulator\n");
}

int result_suffix(char* name) {
    char* p = strrchr(name, '_');
    if (p) return atoi(p+1);
    return 0;
}

// A result timed out; penalize the corresponding host_app_version
//
static int result_timed_out(
    TRANSITIONER_ITEM &res_item, TRANSITIONER_ITEM& wu_item
) {
    DB_HOST_APP_VERSION hav;
    char query[512], clause[512];

    DB_ID_TYPE gavid = generalized_app_version_id(
        res_item.res_app_version_id, wu_item.appid
    );
    int retval = hav_lookup(hav, res_item.res_hostid, gavid);
    if (retval) {
        log_messages.printf(MSG_NORMAL,
            "result_timed_out(): hav_lookup failed: %s\n", boincerror(retval)
        );
        return 0;
    }
    hav.turnaround.update_var(
        (double)wu_item.delay_bound,
        HAV_AVG_THRESH, HAV_AVG_WEIGHT, HAV_AVG_LIMIT
    );
    int n = hav.max_jobs_per_day;
    if (n == 0) {
        n = config.daily_result_quota;
    }
    if (n > config.daily_result_quota) {
        n = config.daily_result_quota;
    }
    n -= 1;
    if (n < 1) {
        n = 1;
    }
    if (config.debug_quota) {
        log_messages.printf(MSG_NORMAL,
            "[quota] max_jobs_per_day for %ld; %d->%d\n",
            gavid, hav.max_jobs_per_day, n
        );
    }
    hav.max_jobs_per_day = n;

    hav.consecutive_valid = 0;

    sprintf(query,
        "turnaround_n=%.15e, turnaround_avg=%.15e, turnaround_var=%.15e, turnaround_q=%.15e, max_jobs_per_day=%d, consecutive_valid=%d",
        hav.turnaround.n,
        hav.turnaround.avg,
        hav.turnaround.var,
        hav.turnaround.q,
        hav.max_jobs_per_day,
        hav.consecutive_valid
    );
    sprintf(clause,
        "host_id=%lu and app_version_id=%lu",
        hav.host_id, hav.app_version_id
    );
    retval = hav.update_fields_noid(query, clause);
    if (retval) {
        log_messages.printf(MSG_CRITICAL,
            "CRITICAL result_timed_out(): hav updated failed: %s\n",
            boincerror(retval)
        );
    }
    return 0;
}

int handle_wu(
    DB_TRANSITIONER_ITEM_SET& transitioner,
    std::vector<TRANSITIONER_ITEM>& items
) {
    int ntotal, nerrors, retval, ninprogress, nsuccess;
    int nunsent, ncouldnt_send, nover, ndidnt_need, nno_reply;
    int canonical_result_index, j;
    char suffix[256];
    time_t now = time(0), x;
    bool all_over_and_validated, have_new_result_to_validate, do_delete;
    unsigned int i;

    TRANSITIONER_ITEM& wu_item = items[0];
    TRANSITIONER_ITEM wu_item_original = wu_item;

    // count up the number of results in various states,
    // and check for timed-out results
    //
    ntotal = 0;
    nunsent = 0;    // including INACTIVE
    ninprogress = 0;
    nover = 0;
    nerrors = 0;
    nsuccess = 0;
        // not counting invalid results!!!!
    ncouldnt_send = 0;
    nno_reply = 0;
    ndidnt_need = 0;
    have_new_result_to_validate = false;
    int rs, max_result_suffix = -1;

    // Scan the WU's results, and find the canonical result if there is one
    //
    canonical_result_index = -1;
    if (wu_item.canonical_resultid) {
        for (i=0; i<items.size(); i++) {
            TRANSITIONER_ITEM& res_item = items[i];
            if (!res_item.res_id) continue;
            if (res_item.res_id == wu_item.canonical_resultid) {
                canonical_result_index = i;
            }
        }
    }

    if (wu_item.canonical_resultid && (canonical_result_index == -1)) {
        log_messages.printf(MSG_CRITICAL,
            "[WU#%lu %s] can't find canonical result\n",
            wu_item.id, wu_item.name
        );
    }

    // if there is a canonical result, see if its file are deleted
    //
    bool canonical_result_files_deleted = false;
    if (canonical_result_index >= 0) {
        TRANSITIONER_ITEM& cr = items[canonical_result_index];
        if (cr.res_file_delete_state == FILE_DELETE_DONE) {
            canonical_result_files_deleted = true;
        }
    }

    // Scan this WU's results, and
    // 1) count those in various server states;
    // 2) identify timed-out results and update their server state and outcome
    // 3) find the max result suffix (in case need to generate new ones)
    // 4) see if we have a new result to validate
    //    (outcome SUCCESS and validate_state INIT)
    //
    for (i=0; i<items.size(); i++) {
        TRANSITIONER_ITEM& res_item = items[i];

        if (!res_item.res_id) continue;
        ntotal++;

        rs = result_suffix(res_item.res_name);
        if (rs > max_result_suffix) max_result_suffix = rs;

        switch (res_item.res_server_state) {
        case RESULT_SERVER_STATE_INACTIVE:
        case RESULT_SERVER_STATE_UNSENT:
            nunsent++;
            break;
        case RESULT_SERVER_STATE_IN_PROGRESS:
            if (res_item.res_report_deadline < now) {
                log_messages.printf(MSG_NORMAL,
                    "[WU#%lu %s] [RESULT#%lu %s] result timed out (%d < %d) server_state:IN_PROGRESS=>OVER; outcome:NO_REPLY\n",
                    wu_item.id, wu_item.name, res_item.res_id,
                    res_item.res_name,
                    res_item.res_report_deadline, (int)now
                );
                res_item.res_server_state = RESULT_SERVER_STATE_OVER;
                res_item.res_outcome = RESULT_OUTCOME_NO_REPLY;
                retval = transitioner.update_result(res_item);
                if (retval) {
                    log_messages.printf(MSG_CRITICAL,
                        "[WU#%lu %s] [RESULT#%lu %s] update_result(): %s\n",
                        wu_item.id, wu_item.name, res_item.res_id,
                        res_item.res_name, boincerror(retval)
                    );
                }
                retval = result_timed_out(res_item, wu_item);
                if (retval) {
                    log_messages.printf(MSG_CRITICAL,
                        "result_timed_out() error: %s\n", boincerror(retval)
                    );
                    exit(1);
                }
                nover++;
                nno_reply++;
            } else {
                ninprogress++;
            }
            break;
        case RESULT_SERVER_STATE_OVER:
            nover++;
            switch (res_item.res_outcome) {
            case RESULT_OUTCOME_COULDNT_SEND:
                log_messages.printf(MSG_NORMAL,
                    "[WU#%lu %s] [RESULT#%lu %s] result couldn't be sent\n",
                    wu_item.id, wu_item.name, res_item.res_id, res_item.res_name
                );
                ncouldnt_send++;
                break;
            case RESULT_OUTCOME_SUCCESS:
                if (res_item.res_validate_state == VALIDATE_STATE_INIT) {
                    if (canonical_result_files_deleted) {
                        res_item.res_validate_state = VALIDATE_STATE_TOO_LATE;
                        retval = transitioner.update_result(res_item);
                        if (retval) {
                            log_messages.printf(MSG_CRITICAL,
                                "[WU#%lu %s] [RESULT#%lu %s] update_result(): %s\n",
                                wu_item.id, wu_item.name, res_item.res_id,
                                res_item.res_name, boincerror(retval)
                            );
                        } else {
                            log_messages.printf(MSG_NORMAL,
                                "[WU#%lu %s] [RESULT#%lu %s] validate_state:INIT=>TOO_LATE\n",
                                wu_item.id, wu_item.name, res_item.res_id,
                                res_item.res_name
                            );
                        }
                    } else {
                        have_new_result_to_validate = true;
                    }
                }
                // don't count invalid results as successful
                //
                if (res_item.res_validate_state != VALIDATE_STATE_INVALID) {
                    nsuccess++;
                }
                break;
            case RESULT_OUTCOME_CLIENT_ERROR:
                // is user aborted job, don't count it as an error
                //
                if (res_item.res_exit_status == EXIT_ABORTED_VIA_GUI) {
                    nno_reply++;
                } else {
                    nerrors++;
                }
                break;
            case RESULT_OUTCOME_VALIDATE_ERROR:
                nerrors++;
                break;
            case RESULT_OUTCOME_CLIENT_DETACHED:
            case RESULT_OUTCOME_NO_REPLY:
                nno_reply++;
                break;
            case RESULT_OUTCOME_DIDNT_NEED:
                ndidnt_need++;
                break;
            }
            break;
        }
    }

    log_messages.printf(MSG_DEBUG,
        "[WU#%lu %s] %d results: unsent %d, in_progress %d, over %d (success %d, error %d, couldnt_send %d, no_reply %d, didnt_need %d)\n",
        wu_item.id, wu_item.name, ntotal, nunsent, ninprogress, nover,
        nsuccess, nerrors, ncouldnt_send, nno_reply, ndidnt_need
    );

    // if there's a new result to validate, trigger validation
    //
    if (have_new_result_to_validate && (nsuccess >= wu_item.min_quorum)) {
        wu_item.need_validate = true;
        log_messages.printf(MSG_NORMAL,
            "[WU#%lu %s] need_validate:=>true\n", wu_item.id, wu_item.name
        );
    }

    // check for WU error conditions
    // NOTE: check on max # of success results is done in validater
    //
    if (ncouldnt_send > 0) {
        wu_item.error_mask |= WU_ERROR_COULDNT_SEND_RESULT;
    }

    // if WU has results with errors and there are no results that are
    // - successful
    // - in progress
    // - timed out (but could still be returned)
    // reset homogeneous redundancy class to give other platforms a try;
    // also reset app version ID if using HAV
    //
    if (nerrors && !(nsuccess || ninprogress || nno_reply)) {
        if (!config.hr_class_static) {
            wu_item.hr_class = 0;
            wu_item.app_version_id = 0;
        }
    }

    if (nerrors > wu_item.max_error_results) {
        log_messages.printf(MSG_NORMAL,
            "[WU#%lu %s] WU has too many errors (%d errors for %d results)\n",
            wu_item.id, wu_item.name, nerrors, ntotal
        );
        wu_item.error_mask |= WU_ERROR_TOO_MANY_ERROR_RESULTS;
    }

    // see how many new results we need to make
    //
    int n_new_results_needed = wu_item.target_nresults - nunsent - ninprogress - nsuccess;
    if (n_new_results_needed < 0) n_new_results_needed = 0;
    int n_new_results_allowed = wu_item.max_total_results - ntotal;

    // if we're already at the limit and need more, error out the WU
    //
    bool too_many = false;
    if (n_new_results_allowed < 0) {
        too_many = true;
    } else if (n_new_results_allowed == 0) {
        if (n_new_results_needed > 0) {
            too_many = true;
        }
    } else {
        if (n_new_results_needed > n_new_results_allowed) {
            n_new_results_needed = n_new_results_allowed;
        }
    }
    if (too_many) {
        log_messages.printf(MSG_NORMAL,
            "[WU#%lu %s] WU has too many total results (%d)\n",
            wu_item.id, wu_item.name, ntotal
        );
        wu_item.error_mask |= WU_ERROR_TOO_MANY_TOTAL_RESULTS;
    }

    // if this WU had an error, don't send any unsent results,
    // and trigger assimilation if needed
    //
    if (wu_item.error_mask) {
        for (i=0; i<items.size(); i++) {
            TRANSITIONER_ITEM& res_item = items[i];
            if (!res_item.res_id) continue;
            bool update_result = false;
            switch(res_item.res_server_state) {
            case RESULT_SERVER_STATE_INACTIVE:
            case RESULT_SERVER_STATE_UNSENT:
                log_messages.printf(MSG_NORMAL,
                    "[WU#%lu %s] [RESULT#%lu %s] server_state:UNSENT=>OVER; outcome:=>DIDNT_NEED\n",
                    wu_item.id, wu_item.name, res_item.res_id, res_item.res_name
                );
                res_item.res_server_state = RESULT_SERVER_STATE_OVER;
                res_item.res_outcome = RESULT_OUTCOME_DIDNT_NEED;
                update_result = true;
                break;
            case RESULT_SERVER_STATE_OVER:
                switch (res_item.res_outcome) {
                case RESULT_OUTCOME_SUCCESS:
                    switch(res_item.res_validate_state) {
                    case VALIDATE_STATE_INIT:
                    case VALIDATE_STATE_INCONCLUSIVE:
                        res_item.res_validate_state = VALIDATE_STATE_NO_CHECK;
                        update_result = true;
                        break;
                    }
                }
            }
            if (update_result) {
                retval = transitioner.update_result(res_item);
                if (retval) {
                    log_messages.printf(MSG_CRITICAL,
                        "[WU#%lu %s] [RESULT#%lu %s] result.update(): %s\n",
                        wu_item.id, wu_item.name, res_item.res_id,
                        res_item.res_name, boincerror(retval)
                    );
                }
            }
        }
        if (wu_item.assimilate_state == ASSIMILATE_INIT) {
            wu_item.assimilate_state = ASSIMILATE_READY;
            log_messages.printf(MSG_NORMAL,
                "[WU#%lu %s] error_mask:%d assimilate_state:INIT=>READY\n",
                wu_item.id, wu_item.name, wu_item.error_mask
            );
        }
    } else if (wu_item.canonical_resultid == 0) {
        // Here if no WU-level error.
        // Generate new results if needed.
        //
        std::string values;
        char value_buf[MAX_QUERY_LEN];
        if (wu_item.transitioner_flags != TRANSITION_NO_NEW_RESULTS
            && n_new_results_needed > 0
        ) {
            log_messages.printf(
                MSG_NORMAL,
                "[WU#%lu %s] Generating %d more results (%d target - %d unsent - %d in progress - %d success)\n",
                wu_item.id, wu_item.name, n_new_results_needed,
                wu_item.target_nresults, nunsent, ninprogress, nsuccess
            );
            for (j=0; j<n_new_results_needed; j++) {
                sprintf(suffix, "%d", max_result_suffix+j+1);
                const char *rtfpath = config.project_path("%s", wu_item.result_template_file);
                int priority_increase = 0;
                if (nover && config.reliable_priority_on_over) {
                    priority_increase += config.reliable_priority_on_over;
                } else if (nover && !nerrors && config.reliable_priority_on_over_except_error) {
                    priority_increase += config.reliable_priority_on_over_except_error;
                }
                retval = create_result_ti(
                    wu_item, (char *)rtfpath, suffix, key, config, value_buf, priority_increase
                );
                if (retval) {
                    log_messages.printf(MSG_CRITICAL,
                        "[WU#%lu %s] create_result_ti(): %s\n",
                        wu_item.id, wu_item.name, boincerror(retval)
                    );
                    return retval;
                }
                if (j==0) {
                    values = value_buf;
                } else {
                    values += ",";
                    values += value_buf;
                }
            }
            DB_RESULT r;
            retval = r.insert_batch(values);
            if (retval) {
                log_messages.printf(MSG_CRITICAL,
                    "[WU#%lu %s] insert_batch(): %s\n",
                    wu_item.id, wu_item.name, boincerror(retval)
                );
                return retval;
            }
        }
    }

    // scan results:
    //  - see if all over and validated
    //
    all_over_and_validated = true;
    bool all_over_and_ready_to_assimilate = true;
        // used for the defer assimilation
    double most_recently_returned = 0;
    for (i=0; i<items.size(); i++) {
        TRANSITIONER_ITEM& res_item = items[i];
        if (!res_item.res_id) continue;
        if (res_item.res_server_state == RESULT_SERVER_STATE_OVER) {
            if (res_item.res_received_time > most_recently_returned) {
                most_recently_returned = res_item.res_received_time;
            }
            if (res_item.res_outcome == RESULT_OUTCOME_SUCCESS) {
                if (res_item.res_validate_state == VALIDATE_STATE_INIT) {
                    all_over_and_validated = false;
                    all_over_and_ready_to_assimilate = false;
                }
            } else if (res_item.res_outcome == RESULT_OUTCOME_NO_REPLY) {
                if (now < res_item.res_report_deadline) {
                    all_over_and_validated = false;
                }
            }
        } else {
            all_over_and_validated = false;
            all_over_and_ready_to_assimilate = false;
        }
    }

    // If we're deferring assimilation until all results are over and validated,
    // when that happens make sure that WU state is advanced to assimilate ready
    // the items.size is a kludge
    //
    if (all_over_and_ready_to_assimilate
        && wu_item.assimilate_state == ASSIMILATE_INIT
        && items.size() > 0
        && wu_item.canonical_resultid > 0
    ) {
        wu_item.assimilate_state = ASSIMILATE_READY;
        log_messages.printf(MSG_NORMAL,
            "[WU#%lu %s] Deferred assimilation now set to ASSIMILATE_STATE_READY\n",
            wu_item.id, wu_item.name
        );
    }

    // if WU is assimilated, trigger file deletion
    //
    double deferred_file_delete_time = 0;
    if (wu_item.assimilate_state == ASSIMILATE_DONE) {
        if (now >= (most_recently_returned + config.delete_delay)) {
            // can delete input files if all results OVER
            //
            if (all_over_and_validated && wu_item.file_delete_state == FILE_DELETE_INIT) {
                wu_item.file_delete_state = FILE_DELETE_READY;
                log_messages.printf(MSG_DEBUG,
                    "[WU#%lu %s] ASSIMILATE_DONE: file_delete_state:=>READY\n",
                    wu_item.id, wu_item.name
                );
            }

            // output of error results can be deleted immediately;
            // output of success results can be deleted if validated
            //
            for (i=0; i<items.size(); i++) {
                TRANSITIONER_ITEM& res_item = items[i];

                // can delete canonical result outputs only if all successful
                // results have been validated
                //
                if (((int)i == canonical_result_index) && !all_over_and_validated) {
                    continue;
                }

                if (!res_item.res_id) continue;
                do_delete = false;
                switch(res_item.res_outcome) {
                case RESULT_OUTCOME_CLIENT_ERROR:
                    do_delete = true;
                    break;
                case RESULT_OUTCOME_SUCCESS:
                    do_delete = (res_item.res_validate_state != VALIDATE_STATE_INIT);
                    break;
                }
                if (do_delete && res_item.res_file_delete_state == FILE_DELETE_INIT) {
                    log_messages.printf(MSG_NORMAL,
                        "[WU#%lu %s] [RESULT#%lu %s] file_delete_state:=>READY\n",
                        wu_item.id, wu_item.name, res_item.res_id, res_item.res_name
                    );
                    res_item.res_file_delete_state = FILE_DELETE_READY;

                    retval = transitioner.update_result(res_item);
                    if (retval) {
                        log_messages.printf(MSG_CRITICAL,
                            "[WU#%lu %s] [RESULT#%lu %s] result.update(): %s\n",
                            wu_item.id, wu_item.name, res_item.res_id,
                            res_item.res_name, boincerror(retval)
                        );
                    }
                }
            }
        } else {
            deferred_file_delete_time = most_recently_returned + config.delete_delay;
            log_messages.printf(MSG_DEBUG,
                "[WU#%lu %s] deferring file deletion for %.0f seconds\n",
                wu_item.id,
                wu_item.name,
                deferred_file_delete_time - now
            );
        }
    }

    // Compute next transition time.
    // This is the min of
    // - timeouts of in-progress results
    // - deferred file deletion time
    // - safety net
    //
    // It is then adjusted to deal with transitioner congestion
    //
    if (wu_item.canonical_resultid || wu_item.error_mask) {
        wu_item.transition_time = INT_MAX;
    } else {
        // Safety net: if there is no canonical result and no WU-level error,
        // make sure that the transitioner will process this WU again.
        // In principle this is not needed,
        // but it makes the BOINC back-end more robust.
        //
        const int ten_days = 10*86400;
        int long_delay = (int)(1.5*wu_item.delay_bound);
        wu_item.transition_time = (long_delay > ten_days) ? long_delay : ten_days;
        wu_item.transition_time += time(0);
    }

    // handle timeout of in-progress results
    //
    for (i=0; i<items.size(); i++) {
        TRANSITIONER_ITEM& res_item = items[i];
        if (!res_item.res_id) continue;
        if (res_item.res_server_state == RESULT_SERVER_STATE_IN_PROGRESS) {
            x = res_item.res_report_deadline;
            if (x < wu_item.transition_time) {
                wu_item.transition_time = x;
            }
        }
    }

    // handle deferred file deletion
    //
    if (deferred_file_delete_time
        && deferred_file_delete_time < wu_item.transition_time
    ) {
        wu_item.transition_time = (int)deferred_file_delete_time;
    }

    // Handle transitioner overload.
    // If transition time is in the past,
    // the system is bogged down and behind schedule.
    // Delay processing of the WU by an amount DOUBLE the amount we are behind,
    // but not less than 60 secs or more than one day.
    //
    if (wu_item.transition_time < now) {
        int extra_delay = 2*(now - wu_item.transition_time);
        if (extra_delay < 60) extra_delay = 60;
        if (extra_delay > 86400) extra_delay = 86400;
        log_messages.printf(MSG_DEBUG,
            "[WU#%lu %s] transition time in past: adding extra delay %d sec\n",
            wu_item.id, wu_item.name, extra_delay
        );
        wu_item.transition_time = now + extra_delay;
    }

    log_messages.printf(MSG_DEBUG,
        "[WU#%lu %s] setting transition_time to %d\n",
        wu_item.id, wu_item.name, wu_item.transition_time
    );

    retval = transitioner.update_workunit(wu_item, wu_item_original);
    if (retval) {
        log_messages.printf(MSG_CRITICAL,
            "[WU#%lu %s] workunit.update(): %s\n",
            wu_item.id, wu_item.name, boincerror(retval)
        );
        return retval;
    }
    return 0;
}

bool do_pass() {
    int retval;
    DB_TRANSITIONER_ITEM_SET transitioner;
    std::vector<TRANSITIONER_ITEM> items;
    bool did_something = false;

    if (!one_pass) check_stop_daemons();

    // loop over entries that are due to be checked
    //
    while (1) {
        if (wu_id) {
            // kludge to tell enumerate to return a given WU
            mod_n = 1;
            mod_i = wu_id;
        }
        retval = transitioner.enumerate(
            (int)time(0), SELECT_LIMIT, mod_n, mod_i, items
        );
        if (retval) {
            if (retval != ERR_DB_NOT_FOUND) {
                log_messages.printf(MSG_CRITICAL,
                    "WU enum error: %s; exiting\n", boincerror(retval)
                );
                exit(1);
            }
            break;
        }
        did_something = true;
        TRANSITIONER_ITEM& wu_item = items[0];
        retval = handle_wu(transitioner, items);
        if (retval) {
            log_messages.printf(MSG_CRITICAL,
                "[WU#%lu %s] handle_wu: %s; quitting\n",
                wu_item.id, wu_item.name, boincerror(retval)
            );
            // probably better to exit here.
            // Whatever cause this WU to fail (and it could be temporary)
            // might cause ALL WUs to fail
            //
            exit(1);
        }

        if (!one_pass) check_stop_daemons();
        if (wu_id) break;
    }
    return did_something;
}

void main_loop() {
    int retval;

    retval = boinc_db.open(config.db_name, config.db_host, config.db_user, config.db_passwd);
    if (retval) {
        log_messages.printf(MSG_CRITICAL,
            "boinc_db.open failed: %s\n", boinc_db.error_string()
        );
        exit(1);
    }

    while (1) {
        log_messages.printf(MSG_DEBUG, "doing a pass\n");
        if (1) {
            bool did_something = do_pass();
            if (one_pass) break;
            if (did_something) continue;
#ifdef GCL_SIMULATOR
            continue_simulation("transitioner");
            signal(SIGUSR2, simulator_signal_handler);
            pause();
#else
            log_messages.printf(MSG_DEBUG, "sleeping %d\n", sleep_interval);
            daemon_sleep(sleep_interval);
#endif
        }
    }
}

void usage(char *name) {
    fprintf(stderr,
        "Handles transitions in the state of a WU\n"
        " - a result has become DONE (via timeout or client reply)\n"
        " - the WU error mask is set (e.g. by validater)\n"
        " - assimilation is finished\n\n"
        "Usage: %s [OPTION]...\n\n"
        "Options: \n"
        "  [ --one_pass ]                  do one pass, then exit\n"
        "  [ --d x ]                       debug level x\n"
        "  [ --mod n i ]                   process only WUs with (id mod n) == i\n"
        "  [ --sleep_interval x ]          sleep x seconds if nothing to do\n"
        "  [ -h | --help ]                 Show this help text.\n"
        "  [ -v | --version ]              Shows version information.\n",
        name
    );
}

int main(int argc, char** argv) {
    int i, retval;
    char path[MAXPATHLEN];

    startup_time = time(0);
    for (i=1; i<argc; i++) {
        if (is_arg(argv[i], "one_pass")) {
            one_pass = true;
        } else if (is_arg(argv[i], "d")) {
            if (!argv[++i]) {
                log_messages.printf(MSG_CRITICAL, "%s requires an argument\n\n", argv[--i]);
                usage(argv[0]);
                exit(1);
            }
            int dl = atoi(argv[i]);
            log_messages.set_debug_level(dl);
            if (dl == 4) g_print_queries = true;
        } else if (is_arg(argv[i], "mod")) {
            if (!argv[i+1] || !argv[i+2]) {
                log_messages.printf(MSG_CRITICAL, "%s requires two arguments\n\n", argv[i]);
                usage(argv[0]);
                exit(1);
            }
            mod_n = atoi(argv[++i]);
            mod_i = atoi(argv[++i]);
            do_mod = true;
        } else if (is_arg(argv[i], "sleep_interval")) {
            if (!argv[++i]) {
                log_messages.printf(MSG_CRITICAL, "%s requires an argument\n\n", argv[--i]);
                usage(argv[0]);
                exit(1);
            }
            sleep_interval = atoi(argv[i]);
        } else if (is_arg(argv[i], "h") || is_arg(argv[i], "help")) {
            usage(argv[0]);
            exit(0);
        } else if (is_arg(argv[i], "v") || is_arg(argv[i], "version")) {
            printf("%s\n", SVN_VERSION);
            exit(0);
        } else if (is_arg(argv[i], "wu_id")) {
            wu_id = atoi(argv[++i]);
            one_pass = true;
        } else {
            log_messages.printf(MSG_CRITICAL, "unknown command line argument: %s\n\n", argv[i]);
            usage(argv[0]);
            exit(1);
        }
    }
    if (!one_pass) check_stop_daemons();

    retval = config.parse_file();
    if (retval) {
        log_messages.printf(MSG_CRITICAL, "Can't parse config.xml: %s\n", boincerror(retval));
        exit(1);
    }

    sprintf(path, "%s/upload_private", config.key_dir);
    retval = read_key_file(path, key);
    if (retval) {
        log_messages.printf(MSG_CRITICAL, "can't read key\n");
        exit(1);
    }

    log_messages.printf(MSG_NORMAL, "Starting\n");

    install_stop_signal_handler();

    main_loop();
}
