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
// validator - check and validate new results, and grant credit
//  -app appname
//  [-d debug_level]
//  [-one_pass]     // make one pass through WU table, then exit
//  [-asynch]       // fork, run in separate process
//
// This program must be linked with two project-specific functions:
// check_set() and check_pair().
// See doc/validate.php for a description.

using namespace std;

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


extern int check_set(
    vector<RESULT>&, WORKUNIT& wu, int& canonical, double& credit,
    bool& retry
);
extern int check_pair(
    RESULT & new_result, RESULT const& canonical_result, bool& retry
);

SCHED_CONFIG config;
char app_name[256];

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
            SCHED_MSG_LOG::CRITICAL,
            "[RESULT#%d] lookup of host %d failed %d\n",
            result.id, result.hostid, retval
        );
        return retval;
    }
    retval = user.lookup_id(host.userid);
    if (retval) {
        log_messages.printf(
            SCHED_MSG_LOG::CRITICAL,
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
            SCHED_MSG_LOG::CRITICAL,
            "[RESULT#%d] update of user %d failed %d\n",
            result.id, host.userid, retval
        );
    }

    host.total_credit += credit;
    update_average(result.sent_time, credit, CREDIT_HALF_LIFE, host.expavg_credit, host.expavg_time);
    sprintf(
        buf, "total_credit=%f, expavg_credit=%f, expavg_time=%f",
        host.total_credit,  host.expavg_credit,
        host.expavg_time
    );
    retval = host.update_field(buf);
    if (retval) {
        log_messages.printf(
            SCHED_MSG_LOG::CRITICAL,
            "[RESULT#%d] update of host %d failed %d\n",
            result.id, result.hostid, retval
        );
    }

    if (user.teamid) {
        retval = team.lookup_id(user.teamid);
        if (retval) {
            log_messages.printf(
                SCHED_MSG_LOG::CRITICAL,
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
                SCHED_MSG_LOG::CRITICAL,
                "[RESULT#%d] update of team %d failed %d\n",
                result.id, team.id, retval
            );
        }
    }

    return 0;
}

void handle_wu(DB_VALIDATOR_ITEM_SET& validator,
            std::vector<VALIDATOR_ITEM>& items) { 
    int  canonical_result_index = -1;
    bool update_result, retry;
    bool need_immediate_transition = false, need_delayed_transition = false;
    int retval = 0, canonicalid = 0;
    double credit = 0;
    unsigned int i;
    RESULT result, canonical_result;
    WORKUNIT wu;

    VALIDATOR_ITEM& wu_vi = items[0];

    if (wu_vi.canonical_resultid) {
        log_messages.printf(
            SCHED_MSG_LOG::NORMAL,
            "[WU#%d %s] handle_wu(): Already has canonical result %d\n",
            wu_vi.id, wu_vi.name, wu_vi.canonical_resultid
        );
        ++log_messages;

        // Here if WU already has a canonical result.
        // Get unchecked results and see if they match the canonical result
        //
        for (i=0; i<items.size(); i++) {
            VALIDATOR_ITEM& vi= items[i];

            log_messages.printf(
                SCHED_MSG_LOG::NORMAL,
                 "[WU#%d %s] handle_wu(): Analyzing result %d\n",
                 vi.id, vi.name, vi.res_id
             );
            if (vi.res_id == wu_vi.canonical_resultid)
                canonical_result_index= i; 
        }
        if (canonical_result_index == (-1)) {
            log_messages.printf(
                SCHED_MSG_LOG::CRITICAL,
                "[WU#%d %s] Can't read canonical result %d; exiting\n",
                wu_vi.id, wu_vi.name, wu_vi.canonical_resultid
            );
            exit(1);
        }

        // scan this WU's results, and check the unchecked ones
        for (i=0; i<items.size(); i++) {
        
            VALIDATOR_ITEM& vitem= items[i];

            if (!((vitem.res_validate_state == VALIDATE_STATE_INIT) &&
                (vitem.res_server_state == RESULT_SERVER_STATE_OVER) &&
                (vitem.res_outcome ==  RESULT_OUTCOME_SUCCESS)))
                continue;

            need_immediate_transition = true;
            
            result = validator.create_result(items[i]);
            canonical_result =
            validator.create_result(items[canonical_result_index]);

            retval = check_pair(
                result, canonical_result, retry
            );
            if (retval) {
                log_messages.printf(
                    SCHED_MSG_LOG::DEBUG,
                    "[RESULT#%d %s]: pair_check() failed for result: %d\n",
                    result.id, result.name, retval
                );
                exit(retval);
            }
            if (retry) need_delayed_transition = true;
            update_result = false;

            // if result had nonrecoverable error, make sure it gets updated
            //
            if (result.outcome != RESULT_OUTCOME_SUCCESS) {
                update_result = true;
            }
            switch (result.validate_state) {
            case VALIDATE_STATE_VALID:
                update_result = true;
                result.granted_credit = wu_vi.canonical_credit;
                log_messages.printf(
                    SCHED_MSG_LOG::NORMAL,
                    "[RESULT#%d %s] pair_check() matched: setting result to valid; credit %f\n",
                    result.id, result.name, result.granted_credit
                );
                retval = grant_credit(result, result.granted_credit);
                if (retval) {
                    log_messages.printf(
                        SCHED_MSG_LOG::NORMAL,
                        "[RESULT#%d %s] Can't grant credit: %d\n",
                        result.id, result.name, retval
                    );
                }
                break;
            case VALIDATE_STATE_INVALID:
                update_result = true;
                log_messages.printf(
                    SCHED_MSG_LOG::NORMAL,
                    "[RESULT#%d %s] pair_check() didn't match: setting result to invalid\n",
                    result.id, result.name
                );
            }
            if (update_result) {
		log_messages.printf(
                    SCHED_MSG_LOG::NORMAL,
                    "[RESULT#%d %s] granted_credit %f", 
		    result.id, result.name, result.granted_credit
                );

                retval = validator.update_result(result);
                if (retval) {
                    log_messages.printf(
                        SCHED_MSG_LOG::CRITICAL,
                        "[RESULT#%d %s] Can't update result: %d\n",
                        result.id, result.name, retval
                    );
                }
            }
        }
    } else {
        vector<RESULT> results;

        // Here if WU doesn't have a canonical result yet.
        // Try to get one

        log_messages.printf(
            SCHED_MSG_LOG::NORMAL,
            "[WU#%d %s] handle_wu(): No canonical result yet\n", wu_vi.id,
            wu_vi.name
        );
        ++log_messages;

        for (i=0; i<items.size(); i++) {
            VALIDATOR_ITEM& vitem= items[i];

            if (!((vitem.res_validate_state == VALIDATE_STATE_INIT) &&
                (vitem.res_server_state == RESULT_SERVER_STATE_OVER) &&
                (vitem.res_outcome == RESULT_OUTCOME_SUCCESS)))
                continue;

            RESULT result= validator.create_result(vitem);
            results.push_back(result);
        }

        log_messages.printf(
            SCHED_MSG_LOG::DEBUG, "[WU#%d %s] Found %d successful results\n",
            wu_vi.id, wu_vi.name, (int)results.size()
        );
        if (results.size() >= (unsigned int)wu_vi.min_quorum) {
            log_messages.printf(
                SCHED_MSG_LOG::DEBUG,
                "[WU#%d %s] Enough for quorum, checking set.\n", wu_vi.id,
                wu_vi.name
            );
           
            wu= validator.create_workunit(wu_vi);
            retval = check_set(results, wu, canonicalid, credit, retry);
            if (retval) {
                log_messages.printf(
                    SCHED_MSG_LOG::CRITICAL,
                    "[WU#%d %s] check_set returned %d, exiting\n",
                    wu_vi.id, wu_vi.name, retval
                );
                exit(retval);
            }
            if (retry) need_delayed_transition = true;

            // See if any results had nonrecoverable errors
            //
            for (i=0; i<results.size(); i++) {
                result = results[i];
                if (result.outcome != RESULT_OUTCOME_SUCCESS) {
                    need_immediate_transition = true;
                    retval = validator.update_result(result);
                    if (retval) {
                        log_messages.printf(
                            SCHED_MSG_LOG::CRITICAL,
                            "[RESULT#%d %s] result.update() failed: %d\n",
                            result.id, result.name, retval
                        );
                    }
                }
            }
            if (canonicalid) {
                need_immediate_transition = true;
                log_messages.printf(
                    SCHED_MSG_LOG::DEBUG,
                    "[WU#%d %s] Found a canonical result: id=%d\n",
                    wu_vi.id, wu_vi.name, canonicalid
                );
                wu_vi.canonical_resultid = canonicalid;
                wu_vi.canonical_credit = credit;
                wu_vi.assimilate_state = ASSIMILATE_READY;
                for (i=0; i<results.size(); i++) {
                    result = results[i];

                    // skip results that had file-read errors
                    //
                    if (result.outcome != RESULT_OUTCOME_SUCCESS) continue;
                    if (result.validate_state == VALIDATE_STATE_INIT) continue;

                    // grant credit for valid results
                    //
                    if (result.validate_state == VALIDATE_STATE_VALID) {
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

		    log_messages.printf(
                    	SCHED_MSG_LOG::NORMAL,
                    	"[RESULT#%d %s] granted_credit %f\n",
                    	result.id, result.name, result.granted_credit
	       	    );

                    retval = validator.update_result(result);
                    if (retval) {
                        log_messages.printf(
                            SCHED_MSG_LOG::CRITICAL,
                            "[RESULT#%d %s] result.update() failed: %d\n",
                            result.id, result.name, retval
                        );
                    }
                }

                // If found a canonical result, don't send any unsent results
                for (i=0; i<items.size(); i++) {
                    VALIDATOR_ITEM& vitem= items[i];

                    if (!(vitem.res_server_state ==
                        RESULT_SERVER_STATE_UNSENT)) 
                        continue;

                    result.server_state = RESULT_SERVER_STATE_OVER;
                    result.outcome = RESULT_OUTCOME_DIDNT_NEED;
                    retval = validator.update_result(result);
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
                if ((int)results.size() > wu_vi.max_success_results) {
                    wu_vi.error_mask |= WU_ERROR_TOO_MANY_SUCCESS_RESULTS;
                    need_immediate_transition = true;
                }
            }
        }
    }

    --log_messages;

    if (need_immediate_transition) {
        wu_vi.transition_time = time(0);
    } else if (need_delayed_transition) {
        int x = time(0) + 6*3600;
        if (x < wu_vi.transition_time) wu_vi.transition_time = x;
    }

    // clear WU.need_validate
    //
    wu_vi.need_validate = 0;
    wu= validator.create_workunit(wu_vi);
    retval = validator.update_workunit(wu);
    if (retval) {
        log_messages.printf(
            SCHED_MSG_LOG::CRITICAL,
            "[WU#%d %s] wu_vi.update() failed: %d; exiting\n", wu_vi.id,
            wu_vi.name, retval
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

    // loop over entries that need to be checked
    //
    while (!validator.enumerate(app.id, SELECT_LIMIT, items)) {
        handle_wu(validator, items);
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
        log_messages.printf(SCHED_MSG_LOG::CRITICAL, "can't find app %s\n", app_name);
        exit(1);
    }

    while (1) {
        check_stop_daemons();
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

    check_stop_daemons();

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
