// validate - check and validate new results
//
// validate appname min_quorum
//
// This program must be linked with two project-specific functions:
//
// int check_set(vector<RESULT>, int& canonical)
//    Compare the set of results.  If a canonical result is found,
//    return its ID, and set the "validate_state" field of all the results
//    according to whether they match the canonical result.
//
// int pair_check(RESULT& new_result, RESULT& canonical, bool& valid);
//    return valid=true iff the new result matches the canonical one
//
// Both functions return nonzero if an error occurred,
// in which case other outputs are undefined

#include <vector>

#include "db.h"

extern check_set(vector<RESULT>, int& canonical);
extern check_pair(RESULT&, RESULT&);

// make one pass through the workunits with need_validate set.
// return true if there were any
//
bool do_validate_scan(APP& app) {
    WORKUNIT wu;
    RESULT result, canonical_result;
    bool found=false, match;

    wu.appid = app.id;
    while(!db_workunit_enum_app_need_validate(wu)) {
        found = true;
        if (wu.canonical_resultid) {

            // Here if WU already has a canonical result.
            // Get unchecked results and see if they match the canonical result
            //
            result.validate_state = UNCHECKED;
            retval = db_result(wu.canonical_resultid, canonical_result);
            if (retval) {
                fprintf(stderr, "validate: can't read canonical result\n");
                continue;
            }
            while(!db_result_enum_validate_state(result)) {
                retval = pair_check(result, canonical_result, match);
                if (retval) {
                    fprintf(stderr,
                        "validate: pair_check failed for result %d\n",
                        result.id
                    );
                    continue;
                } else {
                    if (match) {
                        result.validate_state = VALID;
                    } else {
                    }
                }
                result.validate
            }
        } else {
            // Here if WU doesn't have a canonical result yet.
            // Try to get one

            vector<RESULT> results;
            result.workunitid = wu.id;
            while (!db_result_enum_workunit(result)) {
                if (result.state == RESULT_STATE_DONE) {
                    results.push_back(result);
                }
            }
            if (results.size() >= MIN_QUORUM) {
            }
        }
    }
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
