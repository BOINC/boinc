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

// transitioner - handle transitions in the state of a WU
//    - a result has become DONE (via timeout or client reply)
//    - the WU error mask is set (e.g. by validater)
//    - assimilation is finished
//
// cmdline:
//   [ -asynch ]            be asynchronous
//   [ -one_pass ]          do one pass, then exit
//   [ -d x ]               debug level x
//   [ -mod n i ]           process only WUs with (id mod n) == i

using namespace std;

#include <vector>
#include <unistd.h>
#include <climits>
#include <sys/time.h>

#include "boinc_db.h"
#include "util.h"
#include "backend_lib.h"
#include "sched_config.h"
#include "sched_util.h"
#include "sched_msgs.h"

#define LOCKFILE                "transitioner.out"
#define PIDFILE                 "transitioner.pid"

#define SELECT_LIMIT    1000
#ifdef EINSTEIN_AT_HOME
#define SLEEP_INTERVAL  1
#else
#define SLEEP_INTERVAL  5
#endif
#define BATCH_INSERT    1

int startup_time;
SCHED_CONFIG config;
R_RSA_PRIVATE_KEY key;
int mod_n, mod_i;
bool do_mod = false;

int result_suffix(char* name) {
    char* p = strrchr(name, '_');
    if (p) return atoi(p+1);
    return 0;
}

// The given result just timed out.
// Update the host's avg_turnaround and max_results_day.
//
int penalize_host(int hostid, double delay_bound) {
    DB_HOST host;
    char buf[256];
    int retval = host.lookup_id(hostid);
    if (retval) return retval;
    compute_avg_turnaround(host, delay_bound);
    if (host.max_results_day <= 0 || host.max_results_day > config.daily_result_quota) {
        host.max_results_day = config.daily_result_quota;
    }
    host.max_results_day -= 1;
    if (host.max_results_day < 1) {
        host.max_results_day = 1;
    }
    sprintf(buf,
        "avg_turnaround=%f, max_results_day=%d",
        host.avg_turnaround, host.max_results_day
    );
    return host.update_field(buf);
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

    // count up the number of results in various states,
    // and check for timed-out results
    //
    ntotal = 0;
    nunsent = 0;
    ninprogress = 0;
    nover = 0;
    nerrors = 0;
    nsuccess = 0;
    ncouldnt_send = 0;
    nno_reply = 0;
    ndidnt_need = 0;
    have_new_result_to_validate = false;
    int rs, max_result_suffix = -1;

    TRANSITIONER_ITEM& wu_item = items[0];
    TRANSITIONER_ITEM wu_item_original = wu_item;

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
        log_messages.printf(
            SCHED_MSG_LOG::CRITICAL,
            "[WU#%d %s] can't find canonical result\n",
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
    // 2) identify time-out results and update their server state and outcome
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
        case RESULT_SERVER_STATE_UNSENT:
            nunsent++;
            break;
        case RESULT_SERVER_STATE_IN_PROGRESS:
            if (res_item.res_report_deadline < now) {
                log_messages.printf(
                    SCHED_MSG_LOG::NORMAL,
                    "[WU#%d %s] [RESULT#%d %s] result timed out (%d < %d) server_state:IN_PROGRESS=>OVER; outcome:NO_REPLY\n",
                    wu_item.id, wu_item.name, res_item.res_id, res_item.res_name,
                    res_item.res_report_deadline, (int)now
                );
                res_item.res_server_state = RESULT_SERVER_STATE_OVER;
                res_item.res_outcome = RESULT_OUTCOME_NO_REPLY;
                retval = transitioner.update_result(res_item);
                if (retval) {
                    log_messages.printf(
                        SCHED_MSG_LOG::CRITICAL,
                        "[WU#%d %s] [RESULT#%d %s] update_result(): %d\n",
                        wu_item.id, wu_item.name, res_item.res_id,
                        res_item.res_name, retval
                    );
                }
                penalize_host(res_item.res_hostid, (double)wu_item.delay_bound);
                nover++;
            } else {
                ninprogress++;
            }
            break;
        case RESULT_SERVER_STATE_OVER:
            nover++;
            switch (res_item.res_outcome) {
            case RESULT_OUTCOME_COULDNT_SEND:
                log_messages.printf(
                    SCHED_MSG_LOG::NORMAL,
                    "[WU#%d %s] [RESULT#%d %s] result couldn't be sent\n",
                    wu_item.id, wu_item.name, res_item.res_id, res_item.res_name
                );
                ncouldnt_send++;
                break;
            case RESULT_OUTCOME_SUCCESS:
                if (res_item.res_validate_state == VALIDATE_STATE_INIT) {
                    if (canonical_result_files_deleted) {
                        res_item.res_validate_state = VALIDATE_STATE_TOO_LATE;
                        retval = transitioner.update_result(res_item);
                        log_messages.printf(
                            SCHED_MSG_LOG::NORMAL,
                            "[WU#%d %s] [RESULT#%d %s] validate_state:INIT=>TOO_LATE retval %d\n",
                            wu_item.id, wu_item.name, res_item.id,
                            res_item.name, retval
                        );
                    } else {
                        have_new_result_to_validate = true;
                    }
                }
                nsuccess++;
                break;
            case RESULT_OUTCOME_CLIENT_ERROR:
            case RESULT_OUTCOME_VALIDATE_ERROR:
                nerrors++;
                break;
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

    log_messages.printf(
        SCHED_MSG_LOG::DEBUG,
        "[WU#%d %s] %d results: unsent %d, in_progress %d, over %d (success %d, error %d, couldnt_send %d, no_reply %d, didnt_need %d)\n",
        wu_item.id, wu_item.name, ntotal, nunsent, ninprogress, nover,
        nsuccess, nerrors, ncouldnt_send, nno_reply, ndidnt_need
    );

    // if there's a new result to validate, trigger validation
    //
    if (have_new_result_to_validate && (nsuccess >= wu_item.min_quorum)) {
        wu_item.need_validate = true;
        log_messages.printf(
            SCHED_MSG_LOG::NORMAL,
            "[WU#%d %s] need_validate:=>true\n", wu_item.id, wu_item.name
        );
    }

    // check for WU error conditions
    // NOTE: check on max # of success results is done in validater
    //
    if (ncouldnt_send > 0) {
        wu_item.error_mask |= WU_ERROR_COULDNT_SEND_RESULT;
    }

    if (nerrors > wu_item.max_error_results) {
        log_messages.printf(
            SCHED_MSG_LOG::NORMAL,
            "[WU#%d %s] WU has too many errors (%d errors for %d results)\n",
            wu_item.id, wu_item.name, nerrors, (int)items.size()
        );
        wu_item.error_mask |= WU_ERROR_TOO_MANY_ERROR_RESULTS;
    }
    if ((int)items.size() > wu_item.max_total_results) {
        log_messages.printf(
            SCHED_MSG_LOG::NORMAL,
            "[WU#%d %s] WU has too many total results (%d)\n",
            wu_item.id, wu_item.name, (int)items.size()
        );
        wu_item.error_mask |= WU_ERROR_TOO_MANY_TOTAL_RESULTS;
    }

    // if this WU had an error, don't send any unsent results,
    // and trigger assimilation if needed
    //
    if (wu_item.error_mask) {
        for (i=0; i<items.size(); i++) {
            TRANSITIONER_ITEM& res_item = items[i];
            if (res_item.res_id) {
                bool update_result = false;
                switch(res_item.res_server_state) {
                case RESULT_SERVER_STATE_UNSENT:
                    log_messages.printf(
                        SCHED_MSG_LOG::NORMAL,
                        "[WU#%d %s] [RESULT#%d %s] server_state:UNSENT=>OVER; outcome:=>DIDNT_NEED\n",
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
                        log_messages.printf(
                            SCHED_MSG_LOG::CRITICAL,
                            "[WU#%d %s] [RESULT#%d %s] result.update() == %d\n",
                            wu_item.id, wu_item.name, res_item.res_id, res_item.res_name, retval
                        );
                    }
                }
            }
        }
        if (wu_item.assimilate_state == ASSIMILATE_INIT) {
            wu_item.assimilate_state = ASSIMILATE_READY;
            log_messages.printf(
                SCHED_MSG_LOG::NORMAL,
                "[WU#%d %s] error_mask:%d assimilate_state:INIT=>READY\n",
                wu_item.id, wu_item.name, wu_item.error_mask
            );
        }
    } else if (wu_item.assimilate_state == ASSIMILATE_INIT) {
        // Here if no WU-level error.
        // Generate new results if needed.
        // NOTE: n must be signed
        //
        int n = wu_item.target_nresults - nunsent - ninprogress - nsuccess;
        string values;
        char value_buf[MAX_QUERY_LEN];
        if (n > 0) {
            log_messages.printf(
                SCHED_MSG_LOG::NORMAL,
                "[WU#%d %s] Generating %d more results (%d target - %d unsent - %d in progress - %d success)\n",
                wu_item.id, wu_item.name, n, wu_item.target_nresults, nunsent, ninprogress, nsuccess
            );
            for (j=0; j<n; j++) {
                sprintf(suffix, "%d", max_result_suffix+j+1);
                char rtfpath[256];
                sprintf(rtfpath, "../%s", wu_item.result_template_file);
#ifdef BATCH_INSERT
                retval = create_result(
                    wu_item, rtfpath, suffix, key, config, value_buf
                );
                if (retval) {
                    log_messages.printf(
                        SCHED_MSG_LOG::CRITICAL,
                        "[WU#%d %s] create_result() %d\n",
                        wu_item.id, wu_item.name, retval
                    );
                    return retval;
                }
                if (j==0) {
                    values = value_buf;
                } else {
                    values += ",";
                    values += value_buf;
                }
#else
                retval = create_result(
                    wu_item, rtfpath, suffix, key, config, 0
                );
                if (retval) {
                    log_messages.printf(
                        SCHED_MSG_LOG::CRITICAL,
                        "[WU#%d %s] create_result() %d\n",
                        wu_item.id, wu_item.name, retval
                    );
                    return retval;
                }
#endif
            }
#ifdef BATCH_INSERT
            DB_RESULT r;
            retval = r.insert_batch(values);
            if (retval) {
                log_messages.printf(
                    SCHED_MSG_LOG::CRITICAL,
                    "[WU#%d %s] insert_batch() %d\n",
                    wu_item.id, wu_item.name, retval
                );
                return retval;
            }
#endif
        }
    }

    // scan results:
    //  - see if all over and validated
    //
    all_over_and_validated = true;
    for (i=0; i<items.size(); i++) {
        TRANSITIONER_ITEM& res_item = items[i];
        if (res_item.res_id) {
            if (res_item.res_server_state == RESULT_SERVER_STATE_OVER) {
                if (res_item.res_outcome == RESULT_OUTCOME_SUCCESS) {
                    if (res_item.res_validate_state == VALIDATE_STATE_INIT) {
                        all_over_and_validated = false;
                    }
                }
            } else {
                all_over_and_validated = false;
            }
        }
    }

    // if WU is assimilated, trigger file deletion
    //
    if (wu_item.assimilate_state == ASSIMILATE_DONE) {
        // can delete input files if all results OVER
        //
        if (all_over_and_validated && wu_item.file_delete_state == FILE_DELETE_INIT) {
            wu_item.file_delete_state = FILE_DELETE_READY;
            log_messages.printf(
                SCHED_MSG_LOG::DEBUG,
                "[WU#%d %s] ASSIMILATE_DONE: file_delete_state:=>READY\n",
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

            if (res_item.res_id) {
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
                    log_messages.printf(
                        SCHED_MSG_LOG::NORMAL,
                        "[WU#%d %s] [RESULT#%d %s] file_delete_state:=>READY\n",
                        wu_item.id, wu_item.name, res_item.res_id, res_item.res_name
                    );
                    res_item.res_file_delete_state = FILE_DELETE_READY;

                    retval = transitioner.update_result(res_item);
                    if (retval) {
                        log_messages.printf(
                            SCHED_MSG_LOG::CRITICAL,
                            "[WU#%d %s] [RESULT#%d %s] result.update() == %d\n",
                            wu_item.id, wu_item.name, res_item.res_id, res_item.res_name, retval
                        );
                    }
                }
            }
        }
    }

    // compute next transition time = minimum timeout of in-progress results
    //
    wu_item.transition_time = INT_MAX;
    for (i=0; i<items.size(); i++) {
        TRANSITIONER_ITEM& res_item = items[i];
        if (res_item.res_id) {
            if (res_item.res_server_state == RESULT_SERVER_STATE_IN_PROGRESS) {
                x = res_item.res_sent_time + wu_item.delay_bound;
                if (x < wu_item.transition_time) {
                    wu_item.transition_time = x;
                }
            }
        }
    }

    // If transition time is in the past,
    // the system is bogged down and behind schedule.
    // Delay processing of the WU by an amount DOUBLE the amount
    // we are behind, but not less than 60 secs or more than
    // one day.
    if (wu_item.transition_time < now) {
        int extra_delay = 2*(now - wu_item.transition_time);
        if (extra_delay < 60) extra_delay = 60;
        if (extra_delay > 86400) extra_delay = 86400;
        log_messages.printf(
            SCHED_MSG_LOG::DEBUG,
            "[WU#%d %s] transition time in past: adding extra delay %d sec\n",
            wu_item.id, wu_item.name, extra_delay
        );
        wu_item.transition_time = now + extra_delay;
    }

    log_messages.printf(
        SCHED_MSG_LOG::DEBUG,
        "[WU#%d %s] setting transition_time to %d\n",
        wu_item.id, wu_item.name, wu_item.transition_time
    );

    retval = transitioner.update_workunit(wu_item, wu_item_original);
    if (retval) {
        log_messages.printf(
            SCHED_MSG_LOG::CRITICAL,
            "[WU#%d %s] workunit.update() == %d\n",
            wu_item.id, wu_item.name, retval
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

    check_stop_daemons();

#if 0
    if (config.use_transactions) {
        retval = boinc_db.start_transaction();
        if (retval) {
            log_messages.printf(
                SCHED_MSG_LOG::CRITICAL,
                "transitioner.start_transaction() == %d\n", retval
            );
        }
    }
#endif

    // loop over entries that are due to be checked
    //
    while (!transitioner.enumerate((int)time(0), SELECT_LIMIT, items)) {
        did_something = true;

        TRANSITIONER_ITEM& wu_item = items[0];

        // if we are assigned a transitioner number,
        // limit which records we should looked at.
        // It'll be less expensive to do the check here than in the DB.
        // ??? why ???
        //
        if ((mod_n == 0) || ((mod_n != 0) && (mod_i == (wu_item.id % mod_n)))) {

            retval = handle_wu(transitioner, items);
            if (retval) {
                log_messages.printf(
                    SCHED_MSG_LOG::CRITICAL,
                    "[WU#%d %s] handle_wu: %d; quitting\n",
                    wu_item.id, wu_item.name, retval
                );
                exit(1);
            }

            check_stop_daemons();
        }
    }

#if 0
    if (config.use_transactions) {
        retval = boinc_db.commit_transaction();
        if (retval) {
            log_messages.printf(
                SCHED_MSG_LOG::CRITICAL,
                "[WU#%d %s] transitioner.commit_transaction() == %d\n",
                wu_item.id, wu_item.name, retval
            );
        }
    }
#endif

    return did_something;
}

void main_loop(bool one_pass) {
    int retval;

    retval = boinc_db.open(config.db_name, config.db_host, config.db_user, config.db_passwd);
    if (retval) {
        log_messages.printf(SCHED_MSG_LOG::CRITICAL, "boinc_db.open: %d\n", retval);
        exit(1);
    }

    if (one_pass) {
        do_pass();
    } else {
        while (1) {
            if (!do_pass()) sleep(SLEEP_INTERVAL);
        }
    }
}

int main(int argc, char** argv) {
    int i, retval;
    bool asynch = false, one_pass=false;
    char path[256];

    check_stop_daemons();
    startup_time = time(0);
    for (i=1; i<argc; i++) {
        if (!strcmp(argv[i], "-asynch")) {
            asynch = true;
        } else if (!strcmp(argv[i], "-one_pass")) {
            one_pass = true;
        } else if (!strcmp(argv[i], "-d")) {
            log_messages.set_debug_level(atoi(argv[++i]));
        } else if (!strcmp(argv[i], "-mod")) {
            mod_n = atoi(argv[++i]);
            mod_i = atoi(argv[++i]);
            do_mod = true;
        }
    }

    retval = config.parse_file("..");
    if (retval) {
        log_messages.printf(SCHED_MSG_LOG::CRITICAL, "can't read config file\n");
        exit(1);
    }

    sprintf(path, "%s/upload_private", config.key_dir);
    retval = read_key_file(path, key);
    if (retval) {
        log_messages.printf(SCHED_MSG_LOG::CRITICAL, "can't read key\n");
        exit(1);
    }

    if (asynch) {
        if (fork()) {
            exit(0);
        }
    }

    // // Call lock_file after fork(), because file locks are not always inherited
    // if (lock_file(LOCKFILE)) {
    //     log_messages.printf(SCHED_MSG_LOG::NORMAL, "Another copy of transitioner is already running\n");
    //     exit(1);
    // }
    // write_pid_file(PIDFILE);
    log_messages.printf(SCHED_MSG_LOG::NORMAL, "Starting\n");

    install_stop_signal_handler();

    main_loop(one_pass);
}

const char *BOINC_RCSID_be98c91511 = "$Id$";
