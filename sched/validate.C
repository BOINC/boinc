// The contents of this file are subject to the BOINC Public License
// Version 1.0 (the "License"); you may not use this file except in
// compliance with the License. You may obtain a copy of the License at
// http://boinc.berkeley.edu/license_1.0.txt
//
// Software distributed under the License is distributed on an "AS IS"
// basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the
// License for the specific language governing rights and limitations
// under the License.
//
// The Original Code is the Berkeley Open Infrastructure for Network Computing.
//
// The Initial Developer of the Original Code is the SETI@home project.
// Portions created by the SETI@home project are Copyright (C) 2002
// University of California at Berkeley. All Rights Reserved.
//
// Contributor(s):
//

//
// validate - check and validate new results, and grant credit
//  -app appname
//  [-d debug_level]
//  [-one_pass]     // make one pass through WU table, then exit
//  [-asynch]       // fork, run in separate process
//
// This program must be linked with two project-specific functions:
//
// int check_set(vector<RESULT>, int& canonical, double& credit)
//    Compare a set of results.
//    If a canonical result is found, return its ID,
//    and set the "validate_state" field of all the results
//    according to whether they match the canonical result.
//    Also return the "canonical credit" (e.g. the average or median)
//
// int pair_check(RESULT& new_result, RESULT& canonical, bool& valid);
//    return valid=true iff the new result matches the canonical one
//
// Both functions return nonzero if an error occurred,
// in which case other outputs are undefined

using namespace std;

#include <unistd.h>
#include <math.h>
#include <vector>

#include "boinc_db.h"
#include "util.h"
#include "sched_config.h"
#include "sched_util.h"
#include "sched_msgs.h"

#define LOCKFILE "validate.out"
#define PIDFILE  "validate.pid"

extern int check_set(vector<RESULT>&, int& canonical, double& credit);
extern int check_pair(RESULT const&, RESULT const&, bool&);

SCHED_CONFIG config;
char app_name[256];

// here when a result has been validated;
// grant credit to host, user and team
//
int grant_credit(DB_RESULT& result, double credit) {
    DB_USER user;
    DB_HOST host;
    DB_TEAM team;
    int retval;

    retval = host.lookup_id(result.hostid);
    if (retval) return retval;
    retval = user.lookup_id(host.userid);
    if (retval) return retval;

    user.total_credit += credit;
    update_average(result.sent_time, credit, CREDIT_HALF_LIFE, user.expavg_credit, user.expavg_time);
    retval = user.update();
    if (retval) return retval;

    host.total_credit += credit;
    update_average(result.sent_time, credit, CREDIT_HALF_LIFE, host.expavg_credit, host.expavg_time);
    retval = host.update();
    if (retval) return retval;

    if (user.teamid) {
        retval = team.lookup_id(user.teamid);
        if (retval) return retval;
        team.total_credit += credit;
        update_average(result.sent_time, credit, CREDIT_HALF_LIFE, team.expavg_credit, team.expavg_time);
        retval = team.update();
        if (retval) return retval;
    }

    return 0;
}

void handle_wu(DB_WORKUNIT& wu) {
    DB_RESULT result, canonical_result;
    bool match, update_result, need_transition = false;
    int retval, canonicalid = 0;
    double credit;
    unsigned int i;
    char buf[256];

    if (wu.canonical_resultid) {
        log_messages.printf(
            SCHED_MSG_LOG::NORMAL,
            "[WU#%d %s] handle_wu(): Already has canonical result\n",
            wu.id, wu.name
        );
        ++log_messages;

        // Here if WU already has a canonical result.
        // Get unchecked results and see if they match the canonical result
        //
        retval = canonical_result.lookup_id(wu.canonical_resultid);
        if (retval) {
            log_messages.printf(
                SCHED_MSG_LOG::CRITICAL,
                "[WU#%d %s] Can't read canonical result; marking as validated: %d\n",
                wu.id, wu.name, retval
            );
            // Mark this WU as validated, otherwise we'll keep checking it
            goto mark_validated;
        }

        // scan this WU's results, and check the unchecked ones
        // TODO: do we have an index on these fields?
        // maybe better just to enum on workunitid
        //
        sprintf(buf, "where workunitid=%d and validate_state=%d and server_state=%d and outcome=%d",
            wu.id, VALIDATE_STATE_INIT, RESULT_SERVER_STATE_OVER, RESULT_OUTCOME_SUCCESS
        );
        while (!result.enumerate(buf)) {
            need_transition = true;

            // it's possible that we've deleted canonical result outputs
            //
            if (canonical_result.file_delete_state == FILE_DELETE_DONE) {
                log_messages.printf(
                    SCHED_MSG_LOG::DEBUG,
                    "[WU#%d]: Canonical result (%d) has been deleted\n",
                    wu.id, canonical_result.id
                );
                match = false;
                retval = 0;
            } else {
                retval = check_pair(result, canonical_result, match);
            }
            if (retval) {
                log_messages.printf(
                    SCHED_MSG_LOG::DEBUG,
                    "[RESULT#%d %s]: pair_check() failed for result: %d\n",
                    result.id, result.name, retval
                );
                continue;
            } else {
                if (match) {
                    result.validate_state = VALIDATE_STATE_VALID;
                    result.granted_credit = wu.canonical_credit;
                    log_messages.printf(
                        SCHED_MSG_LOG::NORMAL,
                        "[RESULT#%d %s] pair_check() matched: setting result to valid; credit %f\n",
                        result.id, result.name, result.granted_credit
                    );
                } else {
                    result.validate_state = VALIDATE_STATE_INVALID;
                    log_messages.printf(
                        SCHED_MSG_LOG::NORMAL,
                        "[RESULT#%d %s] pair_check() didn't match: setting result to invalid\n",
                        result.id, result.name
                    );
                }
            }
            retval = result.update();
            if (retval) {
                log_messages.printf(
                    SCHED_MSG_LOG::CRITICAL,
                    "[RESULT#%d %s] Can't update result: %d\n",
                    result.id, result.name, retval
                );
                continue;
            }
            retval = grant_credit(result, result.granted_credit);
            if (retval) {
                log_messages.printf(
                    SCHED_MSG_LOG::NORMAL,
                    "[RESULT#%d %s] Can't grant credit: %d\n",
                    result.id, result.name, retval
                );
                continue;
            }
        }
    } else {
        vector<RESULT> results;

        // Here if WU doesn't have a canonical result yet.
        // Try to get one

        log_messages.printf(
            SCHED_MSG_LOG::NORMAL,
            "[WU#%d %s] handle_wu(): No canonical result yet\n", wu.id, wu.name
        );
        ++log_messages;

        // sprintf(buf, "where workunitid=%d", wu.id);
        // TODO: do we have an index on these fields?
        // maybe better to enum on workunitid
        // while (!result.enumerate(buf)) {
        //     if (result.server_state == RESULT_SERVER_STATE_OVER
        //         && result.outcome == RESULT_OUTCOME_SUCCESS
        //     ) {
        sprintf(
            buf, "where workunitid=%d and server_state=%d and outcome=%d",
            wu.id, RESULT_SERVER_STATE_OVER, RESULT_OUTCOME_SUCCESS
        );
        while (!result.enumerate(buf)) {
            results.push_back(result);
        }
        log_messages.printf(
            SCHED_MSG_LOG::DEBUG, "[WU#%d %s] Found %d successful results\n",
            wu.id, wu.name, (int)results.size()
        );
        if (results.size() >= (unsigned int)wu.min_quorum) {
            log_messages.printf(
                SCHED_MSG_LOG::DEBUG,
                "[WU#%d %s] Enough for quorum, checking set.\n", wu.id, wu.name
            );
            retval = check_set(results, canonicalid, credit);
            if (!retval && canonicalid) {
                need_transition = true;
                log_messages.printf(
                    SCHED_MSG_LOG::DEBUG,
                    "[WU#%d %s] Found a canonical result: id=%d\n",
                    wu.id, wu.name, canonicalid
                );
                wu.canonical_resultid = canonicalid;
                wu.canonical_credit = credit;
                wu.assimilate_state = ASSIMILATE_READY;
                for (i=0; i<results.size(); i++) {
                    result = results[i];
                    update_result = false;

                    // grant credit for valid results
                    //
                    if (result.validate_state == VALIDATE_STATE_VALID) {
                        update_result = true;
                        retval = grant_credit(result, credit);
                        if (retval) {
                            log_messages.printf(
                                SCHED_MSG_LOG::DEBUG,
                                "[RESULT#%d %s] grant_credit() failed: %d\n",
                                result.id, result.name, retval
                            );
                        }
                        result.granted_credit = credit;
                        log_messages.printf(
                            SCHED_MSG_LOG::NORMAL,
                            "[RESULT#%d %s] Granted %f credit to valid result [HOST#%d]\n",
                            result.id, result.name, result.granted_credit, result.hostid
                        );
                    }

                    if (update_result) {
                        retval = result.update();
                        if (retval) {
                            log_messages.printf(
                                SCHED_MSG_LOG::CRITICAL,
                                "[RESULT#%d %s] result.update() failed: %d\n",
                                result.id, result.name, retval
                            );
                        }
                    }
                }

                // don't send any unsent results
                sprintf(buf, "where workunitid=%d and server_state=%d",
                    wu.id, RESULT_SERVER_STATE_UNSENT
                );
                while (!result.enumerate(buf)) {
                    result.server_state = RESULT_SERVER_STATE_OVER;
                    result.outcome = RESULT_OUTCOME_DIDNT_NEED;
                    retval = result.update();
                    if (retval) {
                        log_messages.printf(
                            SCHED_MSG_LOG::CRITICAL,
                            "[RESULT#%d %s] result.update() failed: %d\n",
                            result.id, result.name, retval
                        );
                    }
                }
            } else {
                // here if no consensus; check if #success results is too large
                //
                if ((int)results.size() > wu.max_success_results) {
                    wu.error_mask |= WU_ERROR_TOO_MANY_SUCCESS_RESULTS;
                    need_transition = true;
                }
            }
        }
    }

    --log_messages;

mark_validated:

    if (need_transition) {
        wu.transition_time = time(0);
    }

    // we've checked all results for this WU, so turn off flag
    //
    wu.need_validate = 0;
    retval = wu.update();
    if (retval) {
        log_messages.printf(
            SCHED_MSG_LOG::CRITICAL,
            "[WU#%d %s] wu.update() failed: %d\n", wu.id, wu.name, retval
        );
    }
}

// make one pass through the workunits with need_validate set.
// return true if there were any
//
bool do_validate_scan(APP& app) {
    DB_WORKUNIT wu;
    char buf[256];
    bool found=false;

    sprintf(buf, "where appid=%d and need_validate > 0", app.id);
    while (!wu.enumerate(buf)) {
        handle_wu(wu);
        found = true;
    }
    return found;
}

int main_loop(bool one_pass) {
    int retval;
    DB_APP app;
    bool did_something;
    char buf[256];

    retval = boinc_db.open(config.db_name, config.db_host, config.db_user, config.db_passwd);
    if (retval) {
        log_messages.printf(SCHED_MSG_LOG::CRITICAL, "boinc_db.open failed: %d\n", retval);
        exit(1);
    }

    sprintf(buf, "where name='%s'", app_name);
    retval = app.lookup(buf);
    if (retval) {
        log_messages.printf(SCHED_MSG_LOG::CRITICAL, "can't find app %s\n", app.name);
        exit(1);
    }

    while (1) {
        check_stop_trigger();
        did_something = do_validate_scan(app);
        if (one_pass) break;
        if (!did_something) {
            sleep(5);
        }
    }
    return 0;
}


int main(int argc, char** argv) {
    int i, retval;
    bool asynch = false, one_pass = false;

    check_stop_trigger();

    for (i=1; i<argc; i++) {
        if (!strcmp(argv[i], "-asynch")) {
            asynch = true;
        } else if (!strcmp(argv[i], "-one_pass")) {
            one_pass = true;
        } else if (!strcmp(argv[i], "-app")) {
            strcpy(app_name, argv[++i]);
        } else if (!strcmp(argv[i], "-d")) {
            log_messages.set_debug_level(atoi(argv[++i]));
        } else {
            log_messages.printf(SCHED_MSG_LOG::CRITICAL, "unrecognized arg: %s\n", argv[i]);
        }
    }

    retval = config.parse_file("..");
    if (retval) {
        log_messages.printf(SCHED_MSG_LOG::CRITICAL,
            "Can't parse config file: %d\n", retval
        );
        exit(1);
    }

    if (asynch) {
        if (fork()) {
            exit(0);
        }
    }

    // // Call lock_file after fork(), because file locks are not always inherited
    // if (lock_file(LOCKFILE)) {
    //     log_messages.printf(SCHED_MSG_LOG::NORMAL, "Another copy of validate is already running\n");
    //     exit(1);
    // }
    // write_pid_file(PIDFILE);
    log_messages.printf(SCHED_MSG_LOG::NORMAL, "Starting validator\n");

    install_stop_signal_handler();

    main_loop(one_pass);
}
