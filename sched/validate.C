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
//   -app appname
//   -quorum n      // example WUs only with this many done results
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
#include "config.h"
#include "sched_util.h"

#define LOCKFILE "validate.out"
#define PIDFILE  "validate.pid"

extern int check_set(vector<RESULT>&, int& canonical, double& credit);
extern int check_pair(RESULT&, RESULT&, bool&);

CONFIG config;
char app_name[256];
int min_quorum;

// here when a result has been validated;
// grant credit to host and user
//
int grant_credit(DB_RESULT& result, double credit) {
    DB_USER user;
    DB_HOST host;
    int retval;

    retval = host.lookup_id(result.hostid);
    if (retval) return retval;
    retval = user.lookup_id(host.userid);
    if (retval) return retval;

    user.total_credit += credit;
    update_average(result.sent_time, credit, user.expavg_credit, user.expavg_time);
    retval = user.update();
    if (retval) return retval;

    host.total_credit += credit;
    update_average(result.sent_time, credit, host.expavg_credit, host.expavg_time);
    retval = host.update();
    if (retval) return retval;

    return 0;
}

void handle_wu(DB_WORKUNIT& wu) {
    DB_RESULT result, canonical_result;
    bool match, update_result;
    int retval, canonicalid = 0;
    double credit;
    unsigned int i;
    char buf[256];

    if (wu.canonical_resultid) {
        log_messages.printf(SchedMessages::NORMAL,
                  "validating WU %s; already have canonical result\n", wu.name
            );

        // Here if WU already has a canonical result.
        // Get unchecked results and see if they match the canonical result
        //
        retval = canonical_result.lookup_id(wu.canonical_resultid);
        if (retval) {
            log_messages.printf(SchedMessages::NORMAL, "can't read canonical result\n");
            // Mark this WU as validated, otherwise we'll keep checking it
            goto mark_validated;
        }

        // scan this WU's results, and check the unchecked ones
        //
        sprintf(buf, "where workunitid=%d", wu.id);
        while (!result.enumerate(buf)) {
            if (result.validate_state == VALIDATE_STATE_INIT
                && result.server_state == RESULT_SERVER_STATE_OVER
                && result.outcome == RESULT_OUTCOME_SUCCESS
            ) {
                retval = check_pair(result, canonical_result, match);
                if (retval) {
                    log_messages.printf(SchedMessages::DEBUG,
                              "validate: pair_check failed for result %d\n", result.id);
                    continue;
                } else {
                    if (match) {
                        result.validate_state = VALIDATE_STATE_VALID;
                        result.granted_credit = wu.canonical_credit;
                        printf("setting result %d to valid; credit %f\n", result.id, result.granted_credit);
                    } else {
                        result.validate_state = VALIDATE_STATE_INVALID;
                        printf("setting result %d to invalid\n", result.id);
                    }
                }
                retval = result.update();
                if (retval) {
                    log_messages.printf(SchedMessages::CRITICAL, "Can't update result\n");
                    continue;
                }
                retval = grant_credit(result, result.granted_credit);
                if (retval) {
                    log_messages.printf(SchedMessages::NORMAL, "Can't grant credit\n");
                    continue;
                }
            }
        }
    } else {
        vector<RESULT> results;

        // Here if WU doesn't have a canonical result yet.
        // Try to get one

        log_messages.printf(SchedMessages::DEBUG, "validating WU %s; no canonical result\n", wu.name);

        sprintf(buf, "where workunitid=%d", wu.id);
        while (!result.enumerate(buf)) {
            if (result.server_state == RESULT_SERVER_STATE_OVER
                && result.outcome == RESULT_OUTCOME_SUCCESS
            ) {
                results.push_back(result);
            }
        }
        log_messages.printf(SchedMessages::DEBUG, "found %d successful results\n", results.size());
        if (results.size() >= (unsigned int)min_quorum) {
            retval = check_set(results, canonicalid, credit);
            if (!retval && canonicalid) {
                log_messages.printf(SchedMessages::DEBUG, "found a canonical result\n");
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
                            log_messages.printf(SchedMessages::DEBUG, "validate: grant_credit %d\n", retval );
                        }
                        result.granted_credit = credit;
                        log_messages.printf(SchedMessages::NORMAL,
                                  "updating result %d to %d; credit %f\n",
                                  result.id, result.validate_state, credit );
                    }

                    // don't send any unsent results
                    //
                    if (result.server_state == RESULT_SERVER_STATE_UNSENT) {
                        update_result = true;
                        result.server_state = RESULT_SERVER_STATE_OVER;
                        result.received_time = time(0);
                        result.outcome = RESULT_OUTCOME_DIDNT_NEED;
                    }

                    if (update_result) {
                        retval = result.update();
                        if (retval) {
                            log_messages.printf(SchedMessages::CRITICAL, "validate: boinc_db_result_update %d\n", retval );
                        }
                    }
                }
            }
        }
    }

    mark_validated:
    // we've checked all results for this WU, so turn off flag
    //
    wu.need_validate = 0;
    retval = wu.update();
    if (retval) {
        log_messages.printf(SchedMessages::CRITICAL, "db_workunit_update: %d\n", retval);
    }
}

// make one pass through the workunits with need_validate set.
// return true if there were any
//
bool do_validate_scan(APP& app, int min_quorum) {
    DB_WORKUNIT wu;
    char buf[256];
    bool found=false;

    sprintf(buf, "where appid=%d and need_validate<>0", app.id);
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

    retval = boinc_db_open(config.db_name, config.db_passwd);
    if (retval) {
        log_messages.printf(SchedMessages::CRITICAL, "boinc_db_open: %d\n", retval);
        exit(1);
    }

    sprintf(buf, "where name='%s'", app_name);
    retval = app.lookup(buf);
    if (retval) {
        log_messages.printf(SchedMessages::CRITICAL, "can't find app %s\n", app.name);
        exit(1);
    }

    while (1) {
        check_stop_trigger();
        did_something = do_validate_scan(app, min_quorum);
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
        } else if (!strcmp(argv[i], "-quorum")) {
            min_quorum = atoi(argv[++i]);
        } else {
            log_messages.printf(SchedMessages::CRITICAL, "unrecognized arg: %s\n", argv[i]);
        }
    }

    if (min_quorum < 1 || min_quorum > 10) {
        log_messages.printf(SchedMessages::CRITICAL, "bad min_quorum: %d\n", min_quorum);
        exit(1);
    }

    retval = config.parse_file();
    if (retval) {
        log_messages.printf(SchedMessages::CRITICAL, "Can't parse config file\n");
        exit(1);
    }

    if (asynch) {
        if (fork()) {
            exit(0);
        }
    }

    // Call lock_file after fork(), because file locks are not always inherited
    if (lock_file(LOCKFILE)) {
        log_messages.printf(SchedMessages::NORMAL, "Another copy of validate is already running\n");
        exit(1);
    }
    write_pid_file(PIDFILE);
    log_messages.printf(SchedMessages::NORMAL, "Starting validator; min_quorum %d\n", min_quorum);

    install_sigint_handler();

    main_loop(one_pass);
}
