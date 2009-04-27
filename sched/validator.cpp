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

// validator - check and validate results, and grant credit
//  -app appname
//  [-d debug_level]
//  [-one_pass_N_WU N]      // Validate only N WU in one pass, then exit
//  [-one_pass]             // make one pass through WU table, then exit
//  [-mod n i]              // process only WUs with (id mod n) == i
//  [-max_granted_credit X] // limit maximum granted credit to X
//  [-max_claimed_credit Y] // invalid if claims more than Y
//  [-grant_claimed_credit] // just grant whatever is claimed 
//  [-update_credited_job]  // add userid/wuid pair to credited_job table
//  [-credit_from_wu]       // get credit from WU XML
//
// This program must be linked with two project-specific functions:
// check_set() and check_pair().
// See doc/validate.php for a description.


#include "config.h"
#include <unistd.h>
#include <climits>
#include <cmath>
#include <vector>
#include <cstdlib>
#include <string>
#include <signal.h>

#include "boinc_db.h"
#include "util.h"
#include "str_util.h"
#include "error_numbers.h"

#include "sched_config.h"
#include "sched_util.h"
#include "sched_msgs.h"
#include "validator.h"
#include "validate_util.h"
#ifdef GCL_SIMULATOR
#include "gcl_simulator.h"
#endif

#define LOCKFILE "validate.out"
#define PIDFILE  "validate.pid"

#define SELECT_LIMIT    1000
#define SLEEP_PERIOD    5

int sleep_interval = SLEEP_PERIOD;

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
    RESULT & new_result, RESULT & canonical_result, bool& retry
);

char app_name[256];
DB_APP app;
int wu_id_modulus=0;
int wu_id_remainder=0;
int one_pass_N_WU=0;
bool one_pass = false;
double max_granted_credit = 0;
double max_claimed_credit = 0;
bool grant_claimed_credit = false;
bool update_credited_job = false;
bool credit_from_wu = false;
WORKUNIT* g_wup;

bool is_unreplicated(WORKUNIT& wu) {
    return (wu.target_nresults == 1 && app.target_nresults > 1);
}

void update_error_rate(DB_HOST& host, bool valid) {
    if (valid) {
        host.error_rate *= 0.95;
    } else {
        host.error_rate += 0.1;
    }
    if (host.error_rate > 1) host.error_rate = 1;
    if (host.error_rate <= 0) host.error_rate = 0.1;
}

// Here when a result has been validated and its granted_credit has been set.
// Grant credit to host, user and team, and update host error rate.
//
int is_valid(RESULT& result, WORKUNIT& wu) {
    DB_USER user;
    DB_HOST host;
    DB_TEAM team;
    DB_CREDITED_JOB credited_job;
    int retval;
    char buf[256];

    retval = host.lookup_id(result.hostid);
    if (retval) {
        log_messages.printf(MSG_CRITICAL,
            "[RESULT#%d] lookup of host %d failed %d\n",
            result.id, result.hostid, retval
        );
        return retval;
    }

    retval = user.lookup_id(host.userid);
    if (retval) {
        log_messages.printf(MSG_CRITICAL,
            "[RESULT#%d] lookup of user %d failed %d\n",
            result.id, host.userid, retval
        );
        return retval;
    }

    update_average(
        result.sent_time, result.granted_credit, CREDIT_HALF_LIFE,
        user.expavg_credit, user.expavg_time
    );
    sprintf(
        buf, "total_credit=total_credit+%f, expavg_credit=%f, expavg_time=%f",
        result.granted_credit,  user.expavg_credit, user.expavg_time
    ); 
    retval = user.update_field(buf);
    if (retval) {
        log_messages.printf(MSG_CRITICAL,
            "[RESULT#%d] update of user %d failed %d\n",
            result.id, host.userid, retval
        );
    }

    update_average(
        result.sent_time, result.granted_credit, CREDIT_HALF_LIFE,
        host.expavg_credit, host.expavg_time
    );

    double turnaround = result.received_time - result.sent_time;
    compute_avg_turnaround(host, turnaround);

        
    // compute new credit per CPU time
    //
    retval = update_credit_per_cpu_sec(
        result.granted_credit, result.cpu_time, host.credit_per_cpu_sec
    );
    if (retval) {
        log_messages.printf(MSG_CRITICAL,
            "[RESULT#%d][HOST#%d] claimed too much credit (%f) in too little CPU time (%f)\n",
            result.id, result.hostid, result.granted_credit, result.cpu_time
        );
    }

    double old_error_rate = host.error_rate;
    if (!is_unreplicated(wu)) {
        update_error_rate(host, true);
    }
    sprintf(
        buf,
        "total_credit=total_credit+%f, expavg_credit=%f, expavg_time=%f, avg_turnaround=%f, credit_per_cpu_sec=%f, error_rate=%f",
        result.granted_credit,  host.expavg_credit, host.expavg_time, host.avg_turnaround, host.credit_per_cpu_sec, host.error_rate
    );
    retval = host.update_field(buf);
    if (retval) {
        log_messages.printf(MSG_CRITICAL,
            "[RESULT#%d] update of host %d failed %d\n",
            result.id, result.hostid, retval
        );
    }
    log_messages.printf(MSG_DEBUG,
        "[HOST#%d] error rate %f->%f\n",
        host.id, old_error_rate, host.error_rate
    );

    if (user.teamid) {
        retval = team.lookup_id(user.teamid);
        if (retval) {
            log_messages.printf(MSG_CRITICAL,
                "[RESULT#%d] lookup of team %d failed %d\n",
                result.id, user.teamid, retval
            );
            return retval;
        }
        update_average(result.sent_time, result.granted_credit, CREDIT_HALF_LIFE, team.expavg_credit, team.expavg_time);
        sprintf(
            buf, "total_credit=total_credit+%f, expavg_credit=%f, expavg_time=%f",
            result.granted_credit,  team.expavg_credit, team.expavg_time
        );
        retval = team.update_field(buf);
        if (retval) {
            log_messages.printf(MSG_CRITICAL,
                "[RESULT#%d] update of team %d failed %d\n",
                result.id, team.id, retval
            );
        }
    }

    if (update_credited_job) {
        credited_job.userid = user.id;
        credited_job.workunitid = long(wu.opaque);
        retval = credited_job.insert();
        if (retval) {
            log_messages.printf(MSG_CRITICAL,
                "[RESULT#%d] Warning: credited_job insert failed (userid: %d workunit: %f err: %d)\n",
                result.id, user.id, wu.opaque, retval
            );
        } else {
            log_messages.printf(MSG_DEBUG,
                "[RESULT#%d %s] added credited_job record [WU#%d OPAQUE#%f USER#%d]\n",
                result.id, result.name, wu.id, wu.opaque, user.id
            );
        }
    }

    return 0;
}

int is_invalid(WORKUNIT& wu, RESULT& result) {
    char buf[256];
    int retval;
    DB_HOST host;

    retval = host.lookup_id(result.hostid);
    if (retval) {
        log_messages.printf(MSG_CRITICAL,
            "[RESULT#%d] lookup of host %d failed %d\n",
            result.id, result.hostid, retval
        );
        return retval;
    }
    double old_error_rate = host.error_rate;
    if (!is_unreplicated(wu)) {
        update_error_rate(host, false);
    }
    sprintf(buf, "error_rate=%f", host.error_rate);
    retval = host.update_field(buf);
    if (retval) {
        log_messages.printf(MSG_CRITICAL,
            "[RESULT#%d] update of host %d failed %d\n",
            result.id, result.hostid, retval
        );
        return retval;
    }
    log_messages.printf(MSG_DEBUG,
        "[HOST#%d] invalid result; error rate %f->%f\n",
        host.id, old_error_rate, host.error_rate
    );
    return 0;
}

// Return zero iff we resolved the WU
//
int handle_wu(
    DB_VALIDATOR_ITEM_SET& validator, std::vector<VALIDATOR_ITEM>& items
) { 
    int canonical_result_index = -1;
    bool update_result, retry;
    TRANSITION_TIME transition_time = NO_CHANGE;
    int retval = 0, canonicalid = 0, x;
    double credit = 0;
    unsigned int i;

    WORKUNIT& wu = items[0].wu;
    g_wup = &wu;

    if (wu.canonical_resultid) {
        log_messages.printf(MSG_NORMAL,
            "[WU#%d %s] Already has canonical result %d\n",
            wu.id, wu.name, wu.canonical_resultid
        );
        ++log_messages;

        // Here if WU already has a canonical result.
        // Get unchecked results and see if they match the canonical result
        //
        for (i=0; i<items.size(); i++) {
            RESULT& result = items[i].res;

            if (result.id == wu.canonical_resultid) {
                canonical_result_index = i; 
            }
        }
        if (canonical_result_index == -1) {
            log_messages.printf(MSG_CRITICAL,
                "[WU#%d %s] Can't find canonical result %d\n",
                wu.id, wu.name, wu.canonical_resultid
            );
            return 0;
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
            log_messages.printf(MSG_NORMAL,
                 "[WU#%d] handle_wu(): testing result %d\n",
                 wu.id, result.id
             );

            check_pair(result, canonical_result, retry);
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
                if (result.granted_credit == 0) {
                    result.granted_credit = grant_claimed_credit ? result.claimed_credit : wu.canonical_credit;
                    if (max_granted_credit && result.granted_credit > max_granted_credit) {
                        result.granted_credit = max_granted_credit;
                    }
                }
                log_messages.printf(MSG_NORMAL,
                    "[RESULT#%d %s] pair_check() matched: setting result to valid; credit %f\n",
                    result.id, result.name, result.granted_credit
                );
                retval = is_valid(result, wu);
                if (retval) {
                    log_messages.printf(MSG_NORMAL,
                        "[RESULT#%d %s] Can't grant credit: %d\n",
                        result.id, result.name, retval
                    );
                }
                break;
            case VALIDATE_STATE_INVALID:
                update_result = true;
                log_messages.printf(MSG_NORMAL,
                    "[RESULT#%d %s] pair_check() didn't match: setting result to invalid\n",
                    result.id, result.name
                );
                is_invalid(wu, result);
            }
            if (update_result) {
                log_messages.printf(MSG_NORMAL,
                    "[RESULT#%d %s] granted_credit %f\n", 
                    result.id, result.name, result.granted_credit
                );

                retval = validator.update_result(result);
                if (retval) {
                    log_messages.printf(MSG_CRITICAL,
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

        log_messages.printf(MSG_NORMAL,
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

        log_messages.printf(MSG_DEBUG,
            "[WU#%d %s] Found %d successful results\n",
            wu.id, wu.name, (int)results.size()
        );
        if (results.size() >= (unsigned int)wu.min_quorum) {
            log_messages.printf(MSG_DEBUG,
                "[WU#%d %s] Enough for quorum, checking set.\n",
                wu.id, wu.name
            );
           
            retval = check_set(results, wu, canonicalid, credit, retry);
            if (retval) {
                log_messages.printf(MSG_CRITICAL,
                    "[WU#%d %s] check_set returned %d, exiting\n",
                    wu.id, wu.name, retval
                );
                return retval;
            }
            if (retry) transition_time = DELAYED;

            if (credit_from_wu) {
                retval = get_credit_from_wu(wu, results, credit);
                if (retval) {
                    log_messages.printf(MSG_CRITICAL,
                        "[WU#%d %s] get_credit_from_wu returned %d\n",
                        wu.id, wu.name, retval
                    );
                    return retval;
                }
            }
            if (max_granted_credit && credit>max_granted_credit) {
                credit = max_granted_credit;
            }

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
                    if (result.granted_credit == 0) {
                        result.granted_credit = grant_claimed_credit ? result.claimed_credit : credit;
                        if (max_granted_credit && result.granted_credit > max_granted_credit) {
                            result.granted_credit = max_granted_credit;
                        }
                    }
                    retval = is_valid(result, wu);
                    if (retval) {
                        log_messages.printf(MSG_DEBUG,
                            "[RESULT#%d %s] is_valid() failed: %d\n",
                            result.id, result.name, retval
                        );
                    }
                    log_messages.printf(MSG_NORMAL,
                        "[RESULT#%d %s] Valid; granted %f credit [HOST#%d]\n",
                        result.id, result.name, result.granted_credit,
                        result.hostid
                    );
                    break;
                case VALIDATE_STATE_INVALID:
                    log_messages.printf(MSG_NORMAL,
                        "[RESULT#%d %s] Invalid [HOST#%d]\n",
                        result.id, result.name, result.hostid
                    );
                    is_invalid(wu, result);
                    update_result = true;
                    break;
                case VALIDATE_STATE_INIT:
                    log_messages.printf(MSG_NORMAL,
                        "[RESULT#%d %s] Inconclusive [HOST#%d]\n",
                        result.id, result.name, result.hostid
                    );
                    result.validate_state = VALIDATE_STATE_INCONCLUSIVE;
                    update_result = true;
                    break;
                }

                if (update_result) {
                    retval = validator.update_result(result);
                    if (retval) {
                        log_messages.printf(MSG_CRITICAL,
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
                log_messages.printf(MSG_DEBUG,
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
                        log_messages.printf(MSG_CRITICAL,
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
        log_messages.printf(MSG_CRITICAL,
            "[WU#%d %s] update_workunit() failed: %d; exiting\n",
            wu.id, wu.name, retval
        );
        return retval;
    }
    return 0;
}

// make one pass through the workunits with need_validate set.
// return true if there were any
//
bool do_validate_scan() {
    DB_VALIDATOR_ITEM_SET validator;
    std::vector<VALIDATOR_ITEM> items;
    bool found=false;
    int retval, i=0;

    // loop over entries that need to be checked
    //
    while (1) {
        retval = validator.enumerate(
            app.id, SELECT_LIMIT, wu_id_modulus, wu_id_remainder, items
        );
        if (retval) {
            if (retval != ERR_DB_NOT_FOUND) {
                log_messages.printf(MSG_DEBUG,
                    "DB connection lost, exiting\n"
                );
                exit(0);
            }
            break;
        }
        retval = handle_wu(validator, items);
        if (!retval) found = true;
        if (++i == one_pass_N_WU) break;
    }
    return found;
}

int main_loop() {
    int retval;
    bool did_something;
    char buf[256];

    retval = boinc_db.open(config.db_name, config.db_host, config.db_user, config.db_passwd);
    if (retval) {
        log_messages.printf(MSG_CRITICAL, "boinc_db.open failed: %d\n", retval);
        exit(1);
    }

    sprintf(buf, "where name='%s'", app_name);
    retval = app.lookup(buf);
    if (retval) {
        log_messages.printf(MSG_CRITICAL, "can't find app %s\n", app_name);
        exit(1);
    }

    while (1) {
        check_stop_daemons();
        did_something = do_validate_scan();
        if (!did_something) {
            if (one_pass) break;
#ifdef GCL_SIMULATOR
             char nameforsim[64];
             sprintf(nameforsim, "validator%i", app.id);
             continue_simulation(nameforsim);
             signal(SIGUSR2, simulator_signal_handler);
             pause();
#else
            sleep(sleep_interval);
#endif
        }
    }
    return 0;
}

// For use by user routines check_set() and check_match() that link to
// this code.
int boinc_validator_debuglevel=0;

int main(int argc, char** argv) {
    int i, retval;

#if 0
    int mypid=getpid();
    char debugcmd[512];
    sprintf(debugcmd, "ddd %s %d &", argv[0], mypid);
    system(debugcmd);
    sleep(30);
#endif

    const char *usage = 
      "\nUsage: %s -app <app-name> [OPTIONS]\n"
      "Start validator for application <app-name>\n\n"
      "Optional arguments:\n"
      "  -one_pass_N_WU N       Validate at most N WUs, then exit\n"
      "  -one_pass              Make one pass through WU table, then exit\n"
      "  -mod n i               Process only WUs with (id mod n) == i\n"
      "  -max_claimed_credit X  If a result claims more credit than this, mark it as invalid\n"
      "  -max_granted_credit X  Grant no more than this amount of credit to a result\n"
      "  -grant_claimed_credit  Grant the claimed credit, regardless of what other results for this workunit claimed\n"
      "  -update_credited_job   Add record to credited_job table after granting credit\n"
      "  -credit_from_wu        Credit is specified in WU XML\n"
      "  -sleep_interval n      Set sleep-interval to n\n"
      "  -d level               Set debug-level\n\n";

    if ((argc > 1) && (!strcmp(argv[1], "-h") || !strcmp(argv[1], "--help"))) {
      printf (usage, argv[0] );
      exit(1);
    }


    check_stop_daemons();

    for (i=1; i<argc; i++) {
        if (!strcmp(argv[i], "-one_pass_N_WU")) {
            one_pass_N_WU = atoi(argv[++i]);
            one_pass = true;
        } else if (!strcmp(argv[i], "-sleep_interval")) {
            sleep_interval = atoi(argv[++i]);
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
        } else if (!strcmp(argv[i], "-max_granted_credit")) {
            max_granted_credit = atof(argv[++i]);
        } else if (!strcmp(argv[i], "-max_claimed_credit")) {
            max_claimed_credit = atof(argv[++i]);
        } else if (!strcmp(argv[i], "-grant_claimed_credit")) {
            grant_claimed_credit = true;
        } else if (!strcmp(argv[i], "-update_credited_job")) {
            update_credited_job = true;
        } else if (!strcmp(argv[i], "-credit_from_wu")) {
            credit_from_wu = true;
        } else {
            fprintf(stderr, "Invalid option '%s'\nTry `%s --help` for more information\n", argv[i], argv[0]);
            log_messages.printf(MSG_CRITICAL, "unrecognized arg: %s\n", argv[i]);
            exit(1);
        }
    }

    // -app is required
    if (app_name[0] == 0) {
      fprintf (stderr, "\nERROR: use '-app' to specify the application to run the validator for.\n");
      printf (usage, argv[0] );
      exit(1);      
    }

    retval = config.parse_file("..");
    if (retval) {
        log_messages.printf(MSG_CRITICAL,
            "Can't parse ../config.xml: %s\n", boincerror(retval)
        );
        exit(1);
    }

    log_messages.printf(MSG_NORMAL,
        "Starting validator, debug level %d\n", log_messages.debug_level
    );
    if (wu_id_modulus) {
        log_messages.printf(MSG_NORMAL,
            "Modulus %d, remainder %d\n", wu_id_modulus, wu_id_remainder
        );
    }

    install_stop_signal_handler();

    main_loop();
}

const char *BOINC_RCSID_634dbda0b9 = "$Id$";
