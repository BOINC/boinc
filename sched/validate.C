// The contents of this file are subject to the Mozilla Public License
// Version 1.0 (the "License"); you may not use this file except in
// compliance with the License. You may obtain a copy of the License at
// http://www.mozilla.org/MPL/ 
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

#include "db.h"
#include "config.h"

extern int check_set(vector<RESULT>&, int& canonical, double& credit);
extern int check_pair(RESULT&, RESULT&, bool&);

CONFIG config;
char app_name[256];
int min_quorum;

// "average credit" uses an exponential decay so that recent
// activity is weighted more heavily.
// H is the "half-life" period: the average goes down by 1/2
// if idle for this period.
// Specifically, the weighting function W(t) is
// W(t) = exp(t/(H*log(2))*H*log(2).
// The average credit is the sum of X*W(t(X))
// over units of credit X that were granted t(X) time ago.

#define LOG2 M_LN2
    // log(2)
#define SECONDS_IN_DAY (3600*24)
#define AVG_HALF_LIFE  (SECONDS_IN_DAY*7)
#define ALPHA (1./(AVG_HALF_LIFE*LOG2))

// update an exponential average of credit per second.
//
void update_average(double credit, double& avg, double& avg_time) {
    time_t now = time(0);

    // decrease existing average according to how long it's been
    // since it was computed
    //
    if (avg_time) {
        double deltat = now - avg_time;
        avg *= exp(-deltat/ALPHA);
    }
    avg += credit*ALPHA;
    avg_time = now;
}

// here when a result has been validated;
// grant credit to host and user
//
int grant_credit(RESULT& result, double credit) {
    USER user;
    HOST host;
    int retval;

    retval = db_host(result.hostid, host);
    if (retval) return retval;
    retval = db_user(host.userid, user);
    if (retval) return retval;

    user.total_credit += credit;
    update_average(credit, user.expavg_credit, user.expavg_time);
    retval = db_user_update(user);
    if (retval) return retval;

    host.total_credit += credit;
    update_average(credit, host.expavg_credit, host.expavg_time);
    retval = db_host_update(host);
    if (retval) return retval;

    return 0;
}

// make one pass through the workunits with need_validate set.
// return true if there were any
//
bool do_validate_scan(APP& app, int min_quorum) {
    WORKUNIT wu;
    RESULT result, canonical_result;
    bool found=false, match;
    int retval, canonicalid;
    double credit;
    unsigned int i;

    wu.appid = app.id;
    while(!db_workunit_enum_app_need_validate(wu)) {
        found = true;
        if (wu.canonical_resultid) {
            printf("validating WU %s; already have canonical result\n", wu.name);

            // Here if WU already has a canonical result.
            // Get unchecked results and see if they match the canonical result
            //
            retval = db_result(wu.canonical_resultid, canonical_result);
            if (retval) {
                fprintf(stderr, "validate: can't read canonical result\n");
                continue;
            }

            // scan this WU's results, and check any that need checking
            //
            result.workunitid = wu.id;
            while (!db_result_enum_wuid(result)) {
                if (result.validate_state != VALIDATE_STATE_NEED_CHECK) {
                    continue;
                }
                retval = check_pair(result, canonical_result, match);
                if (retval) {
                    fprintf(stderr,
                        "validate: pair_check failed for result %d\n",
                        result.id
                    );
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
                retval = db_result_update(result);
                if (retval) {
                    fprintf(stderr, "Can't update result\n");
                    continue;
                }
                retval = grant_credit(result, result.granted_credit);
                if (retval) {
                    fprintf(stderr, "Can't grant credit\n");
                    continue;
                }
            }
        } else {
            // Here if WU doesn't have a canonical result yet.
            // Try to get one

            printf("validating WU %s; no canonical result\n", wu.name);

            vector<RESULT> results;
            result.workunitid = wu.id;
            while (!db_result_enum_wuid(result)) {
                if (result.state == RESULT_STATE_DONE) {
                    results.push_back(result);
                }
            }
            printf("found %d results\n", results.size());
            if (results.size() >= (unsigned int)min_quorum) {
                retval = check_set(results, canonicalid, credit);
                if (!retval && canonicalid) {
                    printf("found a canonical result\n");
                    wu.canonical_resultid = canonicalid;
                    wu.canonical_credit = credit;
                    for (i=0; i<results.size(); i++) {
                        if (results[i].validate_state == VALIDATE_STATE_VALID) {
                            retval = grant_credit(results[i], credit);
                            if (retval) {
                                fprintf(stderr,
                                    "validate: grant_credit %d\n", retval
                                );
                            }
                            results[i].granted_credit = credit;
                        }
                        printf("updating result %d to %d; credit %f\n", results[i].id, results[i].validate_state, credit);
                        retval = db_result_update(results[i]);
                        if (retval) {
                            fprintf(stderr,
                                "validate: db_result_update %d\n", retval
                            );
                        }
                    }
                }
            }
        }

        // we've checked all results for this WU, so turn off flag
        //
        wu.need_validate = 0;
        retval = db_workunit_update(wu);
        if (retval) {
            fprintf(stderr, "db_workunit_update: %d\n", retval);
        }
    }
    return found;
}

int main_loop(bool one_pass) {
    int retval;
    APP app;
    bool did_something;

    retval = db_open(config.db_name, config.db_passwd);
    if (retval) {
        fprintf(stderr, "validate: db_open: %d\n", retval);
        exit(1);
    }

    strcpy(app.name, app_name);
    retval = db_app_lookup_name(app);
    if (retval) {
        fprintf(stderr, "can't find app %s\n", app.name);
        exit(1);
    }

    while (1) {
        did_something = do_validate_scan(app, min_quorum);
        if (one_pass) break;
        if (!did_something) {
            printf("sleeping\n");
            fflush(stdout);
            sleep(1);
        }
    }
    return 0;
}


int main(int argc, char** argv) {
    int i, retval;
    bool asynch = false, one_pass = false;

    for (i=1; i<argc; i++) {
        if (!strcmp(argv[i], "-asynch")) {
            asynch = true;
        } else if (!strcmp(argv[i], "-one_pass")) {
            one_pass = true;
        } else if (!strcmp(argv[i], "-app")) {
            strcpy(app_name, argv[++i]);
        } else if (!strcmp(argv[i], "-quorum")) {
            min_quorum = atoi(argv[++i]);
        }
    }

    if (min_quorum < 1 || min_quorum > 10) {
        fprintf(stderr, "bad min_quorum: %d\n", min_quorum);
        exit(1);
    }

    retval = config.parse_file();
    if (retval) {
        fprintf(stderr, "Can't parse config file\n");
        exit(1);
    }

    if (asynch) {
        if (fork()) {
            exit(0);
        }
    }
    main_loop(one_pass);
}
