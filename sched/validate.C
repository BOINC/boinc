// validate - check and validate new results
//
// validate appname min_quorum
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

#include <unistd.h>
#include <math.h>
#include <vector>

#include "db.h"

extern int check_set(vector<RESULT>, int& canonical, double& credit);
extern int check_pair(RESULT&, RESULT&, bool&);

#define SECONDS_IN_DAY (3600*24)
#define EXP_DECAY_RATE  (1./(SECONDS_IN_DAY*7))

// update an exponential average of credit per second
//
void update_average(
    double credit, double& avg, double& avg_time
) {
    time_t now = time(0);

    if (avg_time) {
        double deltat = now - avg_time;
        avg *= exp(-deltat*EXP_DECAY_RATE);
    }
    avg += credit/EXP_DECAY_RATE;
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
                    } else {
                        result.validate_state = VALIDATE_STATE_INVALID;
                    }
                }
                retval = db_result_update(result);
                retval = grant_credit(result, wu.canonical_credit);
            }
        } else {
            // Here if WU doesn't have a canonical result yet.
            // Try to get one

            vector<RESULT> results;
            result.workunitid = wu.id;
            while (!db_result_enum_wuid(result)) {
                if (result.state == RESULT_STATE_DONE) {
                    results.push_back(result);
                }
            }
            if (results.size() >= (unsigned int)min_quorum) {
                retval = check_set(results, canonicalid, credit);
                if (!retval && canonicalid) {
                    wu.canonical_resultid = canonicalid;
                    wu.canonical_credit = credit;
                    for (i=0; i<results.size(); i++) {
                        if (results[i].validate_state == VALIDATE_STATE_VALID) {
                            retval = grant_credit(results[i], credit);
                        }
                        retval = db_result_update(results[i]);
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

int main(int argc, char** argv) {
    int retval, min_quorum;
    APP app;
    bool did_something;

    retval = db_open(getenv("BOINC_DB_NAME"), getenv("BOINC_DB_PASSWD"));
    if (retval) {
        fprintf(stderr, "validate: db_open: %d\n", retval);
        exit(1);
    }

    strcpy(app.name, argv[1]);
    retval = db_app_lookup_name(app);
    if (retval) {
        fprintf(stderr, "can't find app %s\n", app.name);
        exit(1);
    }

    min_quorum = atoi(argv[2]);
    if (min_quorum < 1 || min_quorum > 10) {
        fprintf(stderr, "bad min_quorum: %d\n", min_quorum);
        exit(1);
    }

    while (1) {
        did_something = do_validate_scan(app, min_quorum);
        if (!did_something) sleep(1);
    }
}
