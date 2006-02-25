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

// validator - check and validate results, and grant credit
//  -app appname
//  [-d debug_level]
//  [-one_pass_N_WU N]  // Validate only N WU in one pass, then exit
//  [-one_pass]         // make one pass through WU table, then exit
//  [-asynch]           // fork, run in separate process
//  [-mod n i]          // process only WUs with (id mod n) == i
//
// This program must be linked with two project-specific functions:
// check_set() and check_pair().
// See doc/validate.php for a description.

using namespace std;

#include "config.h"
#include <unistd.h>
#include <cmath>
#include <vector>

#include "boinc_db.h"
#include "util.h"
#include "error_numbers.h"
#include "sched_config.h"
#include "sched_util.h"
#include "sched_msgs.h"

#define LOCKFILE "validate.out"
#define PIDFILE  "validate.pid"

#define SELECT_LIMIT    1000
#define SLEEP_PERIOD    5

typedef enum {
    NEVER,
    DELAYED,
    IMMEDIATE,
    NO_CHANGE
} TRANSITION_TIME;

extern int check_set(
    vector<RESULT>&, WORKUNIT& wu, int& canonical, double& credit,
    bool& retry
);
extern int check_pair(
    RESULT & new_result, RESULT const& canonical_result, bool& retry
);

SCHED_CONFIG config;
char app_name[256];
int wu_id_modulus=0;
int wu_id_remainder=0;
int one_pass_N_WU=0;
bool one_pass = false;


// here when a result has been validated;
// grant credit to host, user and team
//
int grant_credit(RESULT& result, double credit) {
    DB_USER user;
    DB_HOST host;
    DB_TEAM team;
    int retval;
    char buf[256];

    retval = host.lookup_id(result.hostid);
    if (retval) {
        log_messages.printf(
            SCHED_MSG_LOG::MSG_CRITICAL,
            "[RESULT#%d] lookup of host %d failed %d\n",
            result.id, result.hostid, retval
        );
        return retval;
    }
    retval = user.lookup_id(host.userid);
    if (retval) {
        log_messages.printf(
            SCHED_MSG_LOG::MSG_CRITICAL,
            "[RESULT#%d] lookup of user %d failed %d\n",
            result.id, host.userid, retval
        );
        return retval;
    }

    user.total_credit += credit;
    update_average(result.sent_time, credit, CREDIT_HALF_LIFE, user.expavg_credit, user.expavg_time);
    sprintf(
        buf, "total_credit=%f, expavg_credit=%f, expavg_time=%f",
        user.total_credit,  user.expavg_credit,
        user.expavg_time
    ); 
    retval = user.update_field(buf);
    if (retval) {
        log_messages.printf(
            SCHED_MSG_LOG::MSG_CRITICAL,
            "[RESULT#%d] update of user %d failed %d\n",
            result.id, host.userid, retval
        );
    }

    host.total_credit += credit;
    update_average(result.sent_time, credit, CREDIT_HALF_LIFE, host.expavg_credit, host.expavg_time);

    double turnaround = result.received_time - result.sent_time;
    compute_avg_turnaround(host, turnaround);

    sprintf(
        buf,
        "total_credit=%f, expavg_credit=%f, expavg_time=%f, avg_turnaround=%f",
        host.total_credit,  host.expavg_credit, host.expavg_time,
        host.avg_turnaround
    );
    retval = host.update_field(buf);
    if (retval) {
        log_messages.printf(
            SCHED_MSG_LOG::MSG_CRITICAL,
            "[RESULT#%d] update of host %d failed %d\n",
            result.id, result.hostid, retval
        );
    }

    if (user.teamid) {
        retval = team.lookup_id(user.teamid);
        if (retval) {
            log_messages.printf(
                SCHED_MSG_LOG::MSG_CRITICAL,
                "[RESULT#%d] lookup of team %d failed %d\n",
                result.id, user.teamid, retval
            );
            return retval;
        }
        team.total_credit += credit;
        update_average(result.sent_time, credit, CREDIT_HALF_LIFE, team.expavg_credit, team.expavg_time);
        sprintf(
            buf, "total_credit=%f, expavg_credit=%f, expavg_time=%f",
            team.total_credit,  team.expavg_credit,
            team.expavg_time
        );
        retval = team.update_field(buf);
        if (retval) {
            log_messages.printf(
                SCHED_MSG_LOG::MSG_CRITICAL,
                "[RESULT#%d] update of team %d failed %d\n",
                result.id, team.id, retval
            );
        }
    }

    return 0;
}

void handle_wu(
    DB_VALIDATOR_ITEM_SET& validator, std::vector<VALIDATOR_ITEM>& items
) { 
    int canonical_result_index = -1;
    bool update_result, retry;
    TRANSITION_TIME transition_time = NO_CHANGE;
    int retval = 0, canonicalid = 0, x;
    double credit = 0;
    double granted_credit = 0;
    unsigned int i;

    WORKUNIT& wu = items[0].wu;

    if (wu.canonical_resultid) {
        log_messages.printf(
            SCHED_MSG_LOG::MSG_NORMAL,
            "[WU#%d %s] handle_wu(): Already has canonical result %d\n",
            wu.id, wu.name, wu.canonical_resultid
        );
        ++log_messages;

        // Here if WU already has a canonical result.
        // Get unchecked results and see if they match the canonical result
        //
        for (i=0; i<items.size(); i++) {
            RESULT& result = items[i].res;

            log_messages.printf(
                SCHED_MSG_LOG::MSG_NORMAL,
                 "[WU#%d %s] handle_wu(): Analyzing result %d\n",
                 wu.id, wu.name, result.id
             );
            if (result.id == wu.canonical_resultid) {
                canonical_result_index = i; 
            }
        }
        if (canonical_result_index == -1) {
            log_messages.printf(
                SCHED_MSG_LOG::MSG_CRITICAL,
                "[WU#%d %s] Can't find canonical result %d\n",
                wu.id, wu.name, wu.canonical_resultid
            );
            return;
        }

        RESULT& canonical_result = items[canonical_result_index].res;

        // scan this WU's results, and check the unchecked ones
        //
        for (i=0; i<items.size(); i++) {
            RESULT& result = items[i].res;

            if (result.server_state != RESULT_SERVER_STATE_OVER) continue;
            if (result.outcome !=  RESULT_OUTCOME_SUCCESS) continue;
            switch (result.validate_state) {
            case VALIDATE_STATE_INIT:
            case VALIDATE_STATE_INCONCLUSIVE:
                break;
            default:
                continue;
            }

            retval = check_pair(result, canonical_result, retry);
            if (retval) {
                log_messages.printf(
                    SCHED_MSG_LOG::MSG_DEBUG,
                    "[RESULT#%d %s]: pair_check() failed for result: %d\n",
                    result.id, result.name, retval
                );
                exit(retval);
            }
            if (retry) transition_time = DELAYED;
            update_result = false;

            if (result.outcome == RESULT_OUTCOME_VALIDATE_ERROR) {
                update_result = true;
            }

            // this might be last result, so let validator
            // trigger file delete etc. if needed
            //
            transition_time = IMMEDIATE;

            switch (result.validate_state) {
            case VALIDATE_STATE_VALID:
                update_result = true;
                result.granted_credit = (config.grant_claimed_credit) ? result.claimed_credit : wu.canonical_credit;
                log_messages.printf(
                    SCHED_MSG_LOG::MSG_NORMAL,
                    "[RESULT#%d %s] pair_check() matched: setting result to valid; credit %f\n",
                    result.id, result.name, result.granted_credit
                );
                retval = grant_credit(result, result.granted_credit);
                if (retval) {
                    log_messages.printf(
                        SCHED_MSG_LOG::MSG_NORMAL,
                        "[RESULT#%d %s] Can't grant credit: %d\n",
                        result.id, result.name, retval
                    );
                }
                break;
            case VALIDATE_STATE_INVALID:
                update_result = true;
                log_messages.printf(
                    SCHED_MSG_LOG::MSG_NORMAL,
                    "[RESULT#%d %s] pair_check() didn't match: setting result to invalid\n",
                    result.id, result.name
                );
            }
            if (update_result) {
                log_messages.printf(
                    SCHED_MSG_LOG::MSG_NORMAL,
                    "[RESULT#%d %s] granted_credit %f\n", 
                    result.id, result.name, result.granted_credit
                );

                retval = validator.update_result(result);
                if (retval) {
                    log_messages.printf(
                        SCHED_MSG_LOG::MSG_CRITICAL,
                        "[RESULT#%d %s] Can't update result: %d\n",
                        result.id, result.name, retval
                    );
                }
            }
        }
    } else {
        vector<RESULT> results;
        int nsuccess_results;

        // Here if WU doesn't have a canonical result yet.
        // Try to get one

        log_messages.printf(
            SCHED_MSG_LOG::MSG_NORMAL,
            "[WU#%d %s] handle_wu(): No canonical result yet\n",
            wu.id, wu.name
        );
        ++log_messages;

        // make a vector of only successful results
        //
        for (i=0; i<items.size(); i++) {
            RESULT& result = items[i].res;

            if ((result.server_state == RESULT_SERVER_STATE_OVER) &&
                (result.outcome == RESULT_OUTCOME_SUCCESS)
            ) {
                results.push_back(result);
            }

        }

        log_messages.printf(
            SCHED_MSG_LOG::MSG_DEBUG, "[WU#%d %s] Found %d successful results\n",
            wu.id, wu.name, (int)results.size()
        );
        if (results.size() >= (unsigned int)wu.min_quorum) {
            log_messages.printf(
                SCHED_MSG_LOG::MSG_DEBUG,
                "[WU#%d %s] Enough for quorum, checking set.\n",
                wu.id, wu.name
            );
           
            retval = check_set(results, wu, canonicalid, credit, retry);
            if (retval) {
                log_messages.printf(
                    SCHED_MSG_LOG::MSG_CRITICAL,
                    "[WU#%d %s] check_set returned %d, exiting\n",
                    wu.id, wu.name, retval
                );
                exit(retval);
            }
            if (retry) transition_time = DELAYED;

            // scan results.
            // update as needed, and count the # of results
            // that are still outcome=SUCCESS
            // (some may have changed to VALIDATE_ERROR)
            //
            nsuccess_results = 0;
            for (i=0; i<results.size(); i++) {
                update_result = false;
                RESULT& result = results[i];
                if (result.outcome == RESULT_OUTCOME_VALIDATE_ERROR) {
                    transition_time = IMMEDIATE;
                    update_result = true;
                } else {
                    nsuccess_results++;
                }

                switch (result.validate_state) {
                case VALIDATE_STATE_VALID:
                    // grant credit for valid results
                    //
                    update_result = true;
                    granted_credit = (config.grant_claimed_credit) ? result.claimed_credit : credit;
                    retval = grant_credit(result, granted_credit);
                    if (retval) {
                        log_messages.printf(
                            SCHED_MSG_LOG::MSG_DEBUG,
                            "[RESULT#%d %s] grant_credit() failed: %d\n",
                            result.id, result.name, retval
                        );
                    }
                    result.granted_credit = granted_credit;
                    log_messages.printf(
                        SCHED_MSG_LOG::MSG_NORMAL,
                        "[RESULT#%d %s] Granted %f credit to valid result [HOST#%d]\n",
                        result.id, result.name, result.granted_credit, result.hostid
                    );
                    break;
                case VALIDATE_STATE_INVALID:
                    update_result = true;
                    break;
                case VALIDATE_STATE_INIT:
                    result.validate_state = VALIDATE_STATE_INCONCLUSIVE;
                    update_result = true;
                    break;
                }

                if (update_result) {
                    retval = validator.update_result(result);
                    if (retval) {
                        log_messages.printf(
                            SCHED_MSG_LOG::MSG_CRITICAL,
                            "[RESULT#%d %s] result.update() failed: %d\n",
                            result.id, result.name, retval
                        );
                    }
                }
            }

            if (canonicalid) {
                // if we found a canonical result,
                // trigger the assimilator, but do NOT trigger
                // the transitioner - doing so creates a race condition
                //
                transition_time = NEVER;
                log_messages.printf(
                    SCHED_MSG_LOG::MSG_DEBUG,
                    "[WU#%d %s] Found a canonical result: id=%d\n",
                    wu.id, wu.name, canonicalid
                );
                wu.canonical_resultid = canonicalid;
                wu.canonical_credit = credit;
                wu.assimilate_state = ASSIMILATE_READY;

                // If found a canonical result, don't send any unsent results
                //
                for (i=0; i<items.size(); i++) {
                    RESULT& result = items[i].res;

                    if (result.server_state != RESULT_SERVER_STATE_UNSENT) {
                        continue;
                    }

                    result.server_state = RESULT_SERVER_STATE_OVER;
                    result.outcome = RESULT_OUTCOME_DIDNT_NEED;
                    retval = validator.update_result(result);
                    if (retval) {
                        log_messages.printf(
                            SCHED_MSG_LOG::MSG_CRITICAL,
                            "[RESULT#%d %s] result.update() failed: %d\n",
                            result.id, result.name, retval
                        );
                    }
                }
            } else {
                // here if no consensus.

                // check if #success results is too large
                //
                if (nsuccess_results > wu.max_success_results) {
                    wu.error_mask |= WU_ERROR_TOO_MANY_SUCCESS_RESULTS;
                    transition_time = IMMEDIATE;
                }

                // if #success results == than target_nresults,
                // we need more results, so bump target_nresults
                // NOTE: nsuccess_results should never be > target_nresults,
                // but accommodate that if it should happen
                //
                if (nsuccess_results >= wu.target_nresults) {
                    wu.target_nresults = nsuccess_results+1;
                    transition_time = IMMEDIATE;
                }
            }
        }
    }

    --log_messages;

    switch (transition_time) {
    case IMMEDIATE:
        wu.transition_time = time(0);
        break;
    case DELAYED:
        x = time(0) + 6*3600;
        if (x < wu.transition_time) wu.transition_time = x;
        break;
    case NEVER:
        wu.transition_time = INT_MAX;
        break;
    case NO_CHANGE:
        break;
    }

    wu.need_validate = 0;
    
    retval = validator.update_workunit(wu);
    if (retval) {
        log_messages.printf(
            SCHED_MSG_LOG::MSG_CRITICAL,
            "[WU#%d %s] update_workunit() failed: %d; exiting\n",
            wu.id, wu.name, retval
        );
        exit(1);
    }
}

// make one pass through the workunits with need_validate set.
// return true if there were any
//
bool do_validate_scan(APP& app) {
    DB_VALIDATOR_ITEM_SET validator;
    std::vector<VALIDATOR_ITEM> items;
    bool found=false;
    int retval;

    // loop over entries that need to be checked
    //
    while (1) {
        retval = validator.enumerate(
            app.id, one_pass_N_WU?one_pass_N_WU:SELECT_LIMIT,
            wu_id_modulus, wu_id_remainder,
            items
        );
        if (retval) break;
        handle_wu(validator, items);
        found = true;
    }
    return found;
}

int main_loop() {
    int retval;
    DB_APP app;
    bool did_something;
    char buf[256];

    retval = boinc_db.open(config.db_name, config.db_host, config.db_user, config.db_passwd);
    if (retval) {
        log_messages.printf(SCHED_MSG_LOG::MSG_CRITICAL, "boinc_db.open failed: %d\n", retval);
        exit(1);
    }

    sprintf(buf, "where name='%s'", app_name);
    retval = app.lookup(buf);
    if (retval) {
        log_messages.printf(SCHED_MSG_LOG::MSG_CRITICAL, "can't find app %s\n", app_name);
        exit(1);
    }

    while (1) {
        check_stop_daemons();
        did_something = do_validate_scan(app);
        if (!did_something) {
            if (one_pass) break;
            sleep(SLEEP_PERIOD);
        }
    }
    return 0;
}

// For use by user routines check_set() and check_match() that link to
// this code.
int boinc_validator_debuglevel=0;

int main(int argc, char** argv) {
    int i, retval;
    bool asynch = false;

#if 0
    int mypid=getpid();
    char debugcmd[512];
    sprintf(debugcmd, "ddd %s %d &", argv[0], mypid);
    system(debugcmd);
    sleep(30);
#endif

    check_stop_daemons();

    for (i=1; i<argc; i++) {
        if (!strcmp(argv[i], "-asynch")) {
            asynch = true;
        } else if (!strcmp(argv[i], "-one_pass_N_WU")) {
            one_pass_N_WU = atoi(argv[++i]);
            one_pass = true;
        } else if (!strcmp(argv[i], "-one_pass")) {
            one_pass = true;
        } else if (!strcmp(argv[i], "-app")) {
            strcpy(app_name, argv[++i]);
        } else if (!strcmp(argv[i], "-d")) {
            boinc_validator_debuglevel=atoi(argv[++i]);
            log_messages.set_debug_level(boinc_validator_debuglevel);
        } else if (!strcmp(argv[i], "-mod")) {
            wu_id_modulus = atoi(argv[++i]);
            wu_id_remainder = atoi(argv[++i]);
        } else {
            log_messages.printf(SCHED_MSG_LOG::MSG_CRITICAL, "unrecognized arg: %s\n", argv[i]);
            exit(1);
        }
    }

    retval = config.parse_file("..");
    if (retval) {
        log_messages.printf(SCHED_MSG_LOG::MSG_CRITICAL,
            "Can't parse config file: %d\n", retval
        );
        exit(1);
    }

    if (asynch) {
        if (fork()) {
            exit(0);
        }
    }

    log_messages.printf(SCHED_MSG_LOG::MSG_NORMAL, "Starting validator\n");
    if (wu_id_modulus) {
        log_messages.printf(SCHED_MSG_LOG::MSG_NORMAL,
            "Modulus %d, remainder %d\n", wu_id_modulus, wu_id_remainder
        );
    }

    install_stop_signal_handler();

    main_loop();
}

const char *BOINC_RCSID_634dbda0b9 = "$Id$";
