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

// transitioner - handle transitions in the state of a WU
//    - a result has become DONE (via timeout or client reply)
//    - the WU error mask is set (e.g. by validater)
//    - assimilation is finished
//
// cmdline:
//   [ -asynch ]            be asynchronous
//   [ -one_pass ]          do one pass, then exit
//   [ -d x ]               debug level x

using namespace std;

#include <vector>
#include <unistd.h>
#include <limits.h>
#include <sys/time.h>

#include "boinc_db.h"
#include "util.h"
#include "backend_lib.h"
#include "sched_config.h"
#include "sched_util.h"

#define LOCKFILE                "transitioner.out"
#define PIDFILE                 "transitioner.pid"

int startup_time;
SCHED_CONFIG config;
R_RSA_PRIVATE_KEY key;

void handle_wu(DB_WORKUNIT& wu) {
    vector<DB_RESULT> results;
    DB_RESULT* p_canonical_result = NULL;
    int nerrors, retval, ninprogress, nsuccess;
    int nunsent, ncouldnt_send, nover;
    char suffix[256], result_template[MEDIUM_BLOB_SIZE];
    time_t now = time(0), x;
    bool all_over, have_result_to_validate, do_delete;

    {
        char buf[256];
        // scan the results for the WU
        //
        DB_RESULT result;
        sprintf(buf, "where workunitid=%d", wu.id);
        while (!result.enumerate(buf)) {
            results.push_back(result);
        }
    }

    ScopeMessages scope_messages(log_messages, SchedMessages::NORMAL);

    // count up the number of results in various states,
    // and check for timed-out results
    //
    nunsent = 0;
    ninprogress = 0;
    nover = 0;
    nerrors = 0;
    nsuccess = 0;
    ncouldnt_send = 0;
    have_result_to_validate = false;
    for (unsigned int i=0; i<results.size(); i++) {
        DB_RESULT& result = results[i];

        switch (result.server_state) {
        case RESULT_SERVER_STATE_UNSENT:
            nunsent++;
            break;
        case RESULT_SERVER_STATE_IN_PROGRESS:
            if (result.report_deadline < now) {
                log_messages.printf(
                    SchedMessages::NORMAL,
                    "[WU#%d %s] [RESULT#%d %s] result timed out (%d < %d) server_state:IN_PROGRESS=>OVER; outcome:NO_REPLY\n",
                    wu.id, wu.name, result.id, result.name,
                    result.report_deadline, (int)now
                );
                result.server_state = RESULT_SERVER_STATE_OVER;
                result.outcome = RESULT_OUTCOME_NO_REPLY;
                retval = result.update();
                if (retval) {
                    log_messages.printf(
                        SchedMessages::CRITICAL,
                        "[WU#%d %s] [RESULT#%d %s] result.update() == %d\n",
                        wu.id, wu.name, result.id, result.name, retval
                        );
                }
                nover++;
            } else {
                ninprogress++;
            }
            break;
        case RESULT_SERVER_STATE_OVER:
            nover++;
            switch (result.outcome) {
            case RESULT_OUTCOME_COULDNT_SEND:
                log_messages.printf(
                    SchedMessages::NORMAL,
                    "[WU#%d %s] [RESULT#%d %s] result couldn't be sent\n",
                    wu.id, wu.name, result.id, result.name
                );
                ncouldnt_send++;
                break;
            case RESULT_OUTCOME_SUCCESS:
                if (result.validate_state == VALIDATE_STATE_INIT) {
                    have_result_to_validate = true;
                }
                nsuccess++;
                break;
            case RESULT_OUTCOME_CLIENT_ERROR:
                nerrors++;
                break;
            }
            break;
        }
    }

    log_messages.printf(
        SchedMessages::DEBUG,
        "[WU#%d %s] %d results: unsent %d, in_progress %d, over %d (success %d, error %d, couldnt_send %d)\n",
        wu.id, wu.name,
        (int)results.size(),
        nunsent, ninprogress, nover, nsuccess, nerrors, ncouldnt_send
    );

    // trigger validation if we have a quorum
    // and some result hasn't been validated
    //
    if (nsuccess >= wu.min_quorum && have_result_to_validate) {
        wu.need_validate = true;
        log_messages.printf(
            SchedMessages::NORMAL,
            "[WU#%d %s] need_validate:=>true [nsuccess=%d >= min_quorum=%d]\n",
            wu.id, wu.name, nsuccess, wu.min_quorum
        );
    }

    // check for WU error conditions
    // NOTE: check on max # of success results is done in validater
    //
    if (ncouldnt_send > 0) {
        wu.error_mask |= WU_ERROR_COULDNT_SEND_RESULT;
    }

    if (nerrors > wu.max_error_results) {
        log_messages.printf(
            SchedMessages::NORMAL,
            "[WU#%d %s] WU has too many errors (%d errors for %d results)\n",
            wu.id, wu.name, nerrors, (int)results.size()
        );
        wu.error_mask |= WU_ERROR_TOO_MANY_ERROR_RESULTS;
    }
    if ((int)results.size() > wu.max_total_results) {
        log_messages.printf(
            SchedMessages::NORMAL,
            "[WU#%d %s] WU has too many total results (%d)\n",
            wu.id, wu.name, (int)results.size()
        );
        wu.error_mask |= WU_ERROR_TOO_MANY_TOTAL_RESULTS;
    }

    // if this WU had an error, don't send any unsent results,
    // and trigger assimilation if needed
    //
    if (wu.error_mask) {
        for (unsigned int i=0; i<results.size(); i++) {
            DB_RESULT& result = results[i];
            bool update_result = false;
            if (result.server_state == RESULT_SERVER_STATE_UNSENT) {
                log_messages.printf(
                    SchedMessages::NORMAL,
                    "[WU#%d %s] [RESULT#%d %s] server_state:UNSENT=>OVER; outcome:=>DIDNT_NEED\n",
                    wu.id, wu.name, result.id, result.name
                );
                result.server_state = RESULT_SERVER_STATE_OVER;
                result.outcome = RESULT_OUTCOME_DIDNT_NEED;
                update_result = true;
            }
            if (result.validate_state == VALIDATE_STATE_INIT) {
                result.validate_state = VALIDATE_STATE_NO_CHECK;
                update_result = true;
            }
            if (update_result) {
                retval = result.update();
                if (retval) {
                    log_messages.printf(
                        SchedMessages::CRITICAL,
                        "[WU#%d %s] [RESULT#%d %s] result.update() == %d\n",
                        wu.id, wu.name, result.id, result.name, retval
                    );
                }
            }
        }
        if (wu.assimilate_state == ASSIMILATE_INIT) {
            wu.assimilate_state = ASSIMILATE_READY;
            log_messages.printf(
                SchedMessages::NORMAL,
                "[WU#%d %s] error_mask:%d assimilate_state:INIT=>READY\n",
                wu.id, wu.name, wu.error_mask
            );
        }
    } else if (wu.assimilate_state == ASSIMILATE_INIT) {
        // If no error, generate new results if needed.
        // NOTE!! `n' must be a SIGNED integer!
        int n = wu.target_nresults - nunsent - ninprogress - nsuccess;
        if (n > 0) {
            log_messages.printf(
                SchedMessages::NORMAL,
                "[WU#%d %s] Generating %d more results (%d target - %d unsent - %d in progress - %d success)\n",
                wu.id, wu.name, n, wu.target_nresults, nunsent, ninprogress, nsuccess
            );
            for (int i=0; i<n; i++) {
                sprintf(suffix, "%d", results.size()+i);
                strcpy(result_template, wu.result_template);
                retval = create_result(wu, result_template, suffix, key, "");
                if (retval) {
                    log_messages.printf(
                        SchedMessages::CRITICAL,
                        "[WU#%d %s] create_result() %d\n",
                        wu.id, wu.name, retval
                    );
                    break;
                }
            }
        }
    }

    // scan results, see if all over, look for canonical result
    //
    all_over = true;
    for (unsigned int i=0; i<results.size(); i++) {
        DB_RESULT& result = results[i];
        if (result.server_state != RESULT_SERVER_STATE_OVER) {
            all_over = false;
        }
        if (result.id == wu.canonical_resultid) {
            p_canonical_result = &result;
        }
    }
    if (wu.canonical_resultid && p_canonical_result == 0) {
        log_messages.printf(
            SchedMessages::CRITICAL,
            "[WU#%d %s] can't find canonical result\n",
            wu.id, wu.name
        );
    }

    // if WU is assimilated, trigger file deletion
    //
    if (wu.assimilate_state == ASSIMILATE_DONE) {
        // can delete input files if all results OVER
        //
        if (all_over && wu.file_delete_state == FILE_DELETE_INIT) {
            wu.file_delete_state = FILE_DELETE_READY;
            log_messages.printf(
                SchedMessages::DEBUG,
                "[WU#%d %s] ASSIMILATE_DONE: file_delete_state:=>READY\n",
                wu.id, wu.name
            );
        }

        // output of error results can be deleted immediately;
        // output of success results can be deleted if validated
        //
        for (unsigned int i=0; i<results.size(); i++) {
            DB_RESULT& result = results[i];

            // can delete canonical result outputs only if all successful
            // results have been validated
            //
            if (&result == p_canonical_result && !all_over) continue;

            do_delete = false;
            switch(result.outcome) {
            case RESULT_OUTCOME_CLIENT_ERROR:
                do_delete = true;
                break;
            case RESULT_OUTCOME_SUCCESS:
                do_delete = (result.validate_state != VALIDATE_STATE_INIT);
                break;
            }
            if (do_delete && result.file_delete_state == FILE_DELETE_INIT) {
                log_messages.printf(
                    SchedMessages::NORMAL,
                    "[WU#%d %s] [RESULT#%d %s] file_delete_state:=>READY\n",
                    wu.id, wu.name, result.id, result.name
                );
                result.file_delete_state = FILE_DELETE_READY;
                retval = result.update();
                if (retval) {
                    log_messages.printf(
                        SchedMessages::CRITICAL,
                        "[WU#%d %s] [RESULT#%d %s] result.update() == %d\n",
                        wu.id, wu.name, result.id, result.name, retval
                        );
                }
            }
        }
    }

    wu.transition_time = INT_MAX;
    for (unsigned int i=0; i<results.size(); i++) {
        DB_RESULT& result = results[i];
        if (result.server_state == RESULT_SERVER_STATE_IN_PROGRESS) {
            x = result.sent_time + wu.delay_bound;
            if (x < wu.transition_time) {
                wu.transition_time = x;
            }
        }
    }
    retval = wu.update();
    if (retval) {
        log_messages.printf(
            SchedMessages::CRITICAL,
            "[WU#%d %s] workunit.update() == %d\n", wu.id, wu.name, retval
        );
    }
}

bool do_pass() {
    DB_WORKUNIT wu;
    char buf[256];
    bool did_something = false;

    check_stop_trigger();
    // loop over WUs that are due to be checked
    //
    sprintf(buf, "where transition_time<%d", (int)time(0));
    while (!wu.enumerate(buf)) {
        did_something = true;
        handle_wu(wu);
        check_stop_trigger();
    }
    return did_something;
}

void main_loop(bool one_pass) {
    int retval;

    retval = boinc_db.open(config.db_name, config.db_host, config.db_passwd);
    if (retval) {
        log_messages.printf(SchedMessages::CRITICAL, "boinc_db.open: %d\n", retval);
        exit(1);
    }

    if (one_pass) {
        do_pass();
    } else {
        while (1) {
            if (!do_pass()) sleep(1);
        }
    }
}

int main(int argc, char** argv) {
    int i, retval;
    bool asynch = false, one_pass=false;
    char path[256];

    check_stop_trigger();
    startup_time = time(0);
    for (i=1; i<argc; i++) {
        if (!strcmp(argv[i], "-asynch")) {
            asynch = true;
        } else if (!strcmp(argv[i], "-one_pass")) {
            one_pass = true;
        } else if (!strcmp(argv[i], "-d")) {
            log_messages.set_debug_level(atoi(argv[++i]));
        }
    }

    retval = config.parse_file("..");
    if (retval) {
        log_messages.printf(SchedMessages::CRITICAL, "can't read config file\n");
        exit(1);
    }

    sprintf(path, "%s/upload_private", config.key_dir);
    retval = read_key_file(path, key);
    if (retval) {
        log_messages.printf(SchedMessages::CRITICAL, "can't read key\n");
        exit(1);
    }

    if (asynch) {
        if (fork()) {
            exit(0);
        }
    }

    // // Call lock_file after fork(), because file locks are not always inherited
    // if (lock_file(LOCKFILE)) {
    //     log_messages.printf(SchedMessages::NORMAL, "Another copy of transitioner is already running\n");
    //     exit(1);
    // }
    // write_pid_file(PIDFILE);
    log_messages.printf(SchedMessages::NORMAL, "Starting\n");

    install_sigint_handler();

    main_loop(one_pass);
}
