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
//   [ -mod n i ]           process only WUs with (id mod n) == i

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
#include "sched_msgs.h"

#define LOCKFILE                "transitioner.out"
#define PIDFILE                 "transitioner.pid"

#define SELECT_LIMIT    100

int startup_time;
SCHED_CONFIG config;
R_RSA_PRIVATE_KEY key;
int mod_n, mod_i;
bool do_mod = false;

void handle_wu(DB_TRANSITIONER_ITEM_SET& transitioner, std::vector<TRANSITIONER_ITEM>& items) {
    int nerrors, retval, ninprogress, nsuccess;
    int nunsent, ncouldnt_send, nover;
    int canonical_result_index;
    char suffix[256];
    time_t now = time(0), x;
    bool all_over_and_validated, have_result_to_validate, do_delete;

    SCOPE_MSG_LOG scope_messages(log_messages, SCHED_MSG_LOG::NORMAL);

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
    for (unsigned int i=0; i<items.size(); i++) {
        switch (items[i].res_server_state) {
        case RESULT_SERVER_STATE_UNSENT:
            if (items[i].res_id) { nunsent++ };
            break;
        case RESULT_SERVER_STATE_IN_PROGRESS:
            if (items[i].res_id) { 
                if (items[i].res_report_deadline < now) {
                    log_messages.printf(
                        SCHED_MSG_LOG::NORMAL,
                        "[WU#%d %s] [RESULT#%d %s] result timed out (%d < %d) server_state:IN_PROGRESS=>OVER; outcome:NO_REPLY\n",
                        items[0].id, items[0].name, items[i].res_id, items[i].res_name,
                        items[i].res_report_deadline, (int)now
                    );
                    items[i].res_server_state = RESULT_SERVER_STATE_OVER;
                    items[i].res_outcome = RESULT_OUTCOME_NO_REPLY;
                    retval = transitioner.update_result(items[i]);
                    if (retval) {
                        log_messages.printf(
                            SCHED_MSG_LOG::CRITICAL,
                            "[WU#%d %s] [RESULT#%d %s] result.update() == %d\n",
                            items[0].id, items[0].name, items[i].res_id, items[i].res_name, retval
                            );
                    }
                    nover++;
                } else {
                    ninprogress++;
                }
            }
            break;
        case RESULT_SERVER_STATE_OVER:
            if (items[i].res_id) { nover++; }
            switch (items[i].res_outcome) {
            case RESULT_OUTCOME_COULDNT_SEND:
                if (items[i].res_id) { 
                    log_messages.printf(
                        SCHED_MSG_LOG::NORMAL,
                        "[WU#%d %s] [RESULT#%d %s] result couldn't be sent\n",
                        items[0].id, items[0].name, items[i].res_id, items[i].res_name
                    );
                    ncouldnt_send++;
                }
                break;
            case RESULT_OUTCOME_SUCCESS:
                if (items[i].res_id) { 
                    if (items[i].res_validate_state == VALIDATE_STATE_INIT) {
                        have_result_to_validate = true;
                    }
                    nsuccess++;
                }
                break;
            case RESULT_OUTCOME_CLIENT_ERROR:
                if (items[i].res_id) { nerrors++ };
                break;
            }
            break;
        }
    }

    log_messages.printf(
        SCHED_MSG_LOG::DEBUG,
        "[WU#%d %s] %d results: unsent %d, in_progress %d, over %d (success %d, error %d, couldnt_send %d)\n",
        items[0].id, items[0].name, (int)items.size(),
        nunsent, ninprogress, nover, nsuccess, nerrors, ncouldnt_send
    );

    // trigger validation if we have a quorum
    // and some result hasn't been validated
    //
    if (nsuccess >= items[0].min_quorum && have_result_to_validate) {
        items[0].need_validate = true;
        log_messages.printf(
            SCHED_MSG_LOG::NORMAL,
            "[WU#%d %s] need_validate:=>true [nsuccess=%d >= min_quorum=%d]\n",
            items[0].id, items[0].name, nsuccess, items[0].min_quorum
        );
    }

    // check for WU error conditions
    // NOTE: check on max # of success results is done in validater
    //
    if (ncouldnt_send > 0) {
        items[0].error_mask |= WU_ERROR_COULDNT_SEND_RESULT;
    }

    if (nerrors > items[0].max_error_results) {
        log_messages.printf(
            SCHED_MSG_LOG::NORMAL,
            "[WU#%d %s] WU has too many errors (%d errors for %d results)\n",
            items[0].id, items[0].name, nerrors, (int)items.size()
        );
        items[0].error_mask |= WU_ERROR_TOO_MANY_ERROR_RESULTS;
    }
    if ((int)items.size() > items[0].max_total_results) {
        log_messages.printf(
            SCHED_MSG_LOG::NORMAL,
            "[WU#%d %s] WU has too many total results (%d)\n",
            items[0].id, items[0].name, (int)items.size()
        );
        items[0].error_mask |= WU_ERROR_TOO_MANY_TOTAL_RESULTS;
    }

    // if this WU had an error, don't send any unsent results,
    // and trigger assimilation if needed
    //
    if (items[0].error_mask) {
        for (unsigned int i=0; i<items.size(); i++) {
            bool update_result = false;
            if (items[i].res_server_state == RESULT_SERVER_STATE_UNSENT) {
                log_messages.printf(
                    SCHED_MSG_LOG::NORMAL,
                    "[WU#%d %s] [RESULT#%d %s] server_state:UNSENT=>OVER; outcome:=>DIDNT_NEED\n",
                    items[0].id, items[0].name, items[i].res_id, items[i].res_name
                );
                items[i].res_server_state = RESULT_SERVER_STATE_OVER;
                items[i].res_outcome = RESULT_OUTCOME_DIDNT_NEED;
                update_result = true;
            }
            if (items[i].res_validate_state == VALIDATE_STATE_INIT) {
                items[i].res_validate_state = VALIDATE_STATE_NO_CHECK;
                update_result = true;
            }
            if (update_result) {
                retval = transitioner.update_result(items[i]);
                if (retval) {
                    log_messages.printf(
                        SCHED_MSG_LOG::CRITICAL,
                        "[WU#%d %s] [RESULT#%d %s] result.update() == %d\n",
                        items[0].id, items[0].name, items[i].res_id, items[i].res_name, retval
                    );
                }
            }
        }
        if (items[0].assimilate_state == ASSIMILATE_INIT) {
            items[0].assimilate_state = ASSIMILATE_READY;
            log_messages.printf(
                SCHED_MSG_LOG::NORMAL,
                "[WU#%d %s] error_mask:%d assimilate_state:INIT=>READY\n",
                items[0].id, items[0].name, items[0].error_mask
            );
        }
    } else if (items[0].assimilate_state == ASSIMILATE_INIT) {
        // If no error, generate new results if needed.
        // NOTE!! `n' must be a SIGNED integer!
        int n = items[0].target_nresults - nunsent - ninprogress - nsuccess;
        if (n > 0) {
            log_messages.printf(
                SCHED_MSG_LOG::NORMAL,
                "[WU#%d %s] Generating %d more results (%d target - %d unsent - %d in progress - %d success)\n",
                items[0].id, items[0].name, n, items[0].target_nresults, nunsent, ninprogress, nsuccess
            );
            for (int i=0; i<n; i++) {
                sprintf(suffix, "%d", items.size()+i);
                char rtfpath[256];
                sprintf(rtfpath, "../%s", items[0].result_template_file);
                retval = create_result(
                    items[0].id, items[0].appid, items[0].name,
                    rtfpath, suffix, key, ""
                );
                if (retval) {
                    log_messages.printf(
                        SCHED_MSG_LOG::CRITICAL,
                        "[WU#%d %s] create_result() %d\n",
                        items[0].id, items[0].name, retval
                    );
                    break;
                }
            }
        }
    }

    // scan results:
    //  - see if all over and validated
    //  - look for canonical result
    //
    canonical_result_index = -1;
    all_over_and_validated = true;
    for (unsigned int i=0; i<items.size(); i++) {
        if (items[i].res_id) { 
            if (items[i].res_server_state == RESULT_SERVER_STATE_OVER) {
                if (items[i].res_outcome == RESULT_OUTCOME_SUCCESS) {
                    if (items[i].res_validate_state == VALIDATE_STATE_INIT) {
                        all_over_and_validated = false;
                    }
                }
            } else {
                all_over_and_validated = false;
            }
            if (items[i].res_id == items[0].canonical_resultid) {
                canonical_result_index = i;
            }
        }
    }
    if (items[0].canonical_resultid && (canonical_result_index == -1)) {
        log_messages.printf(
            SCHED_MSG_LOG::CRITICAL,
            "[WU#%d %s] can't find canonical result\n",
            items[0].id, items[0].name
        );
    }

    // if WU is assimilated, trigger file deletion
    //
    if (items[0].assimilate_state == ASSIMILATE_DONE) {
        // can delete input files if all results OVER
        //
        if (all_over_and_validated && items[0].file_delete_state == FILE_DELETE_INIT) {
            items[0].file_delete_state = FILE_DELETE_READY;
            log_messages.printf(
                SCHED_MSG_LOG::DEBUG,
                "[WU#%d %s] ASSIMILATE_DONE: file_delete_state:=>READY\n",
                items[0].id, items[0].name
            );
        }

        // output of error results can be deleted immediately;
        // output of success results can be deleted if validated
        //
        for (unsigned int i=0; i<items.size(); i++) {

            // can delete canonical result outputs only if all successful
            // results have been validated
            //
            if (((int)i == canonical_result_index) && !all_over_and_validated) {
                continue;
            }

            do_delete = false;
            switch(items[i].res_outcome) {
            case RESULT_OUTCOME_CLIENT_ERROR:
                do_delete = true;
                break;
            case RESULT_OUTCOME_SUCCESS:
                do_delete = (items[i].res_validate_state != VALIDATE_STATE_INIT);
                break;
            }
            if (do_delete && items[i].res_file_delete_state == FILE_DELETE_INIT) {
                log_messages.printf(
                    SCHED_MSG_LOG::NORMAL,
                    "[WU#%d %s] [RESULT#%d %s] file_delete_state:=>READY\n",
                    items[0].id, items[0].name, items[i].res_id, items[i].res_name
                );
                items[i].res_file_delete_state = FILE_DELETE_READY;

                retval = transitioner.update_result(items[i]);
                if (retval) {
                    log_messages.printf(
                        SCHED_MSG_LOG::CRITICAL,
                        "[WU#%d %s] [RESULT#%d %s] result.update() == %d\n",
                        items[0].id, items[0].name, items[i].res_id, items[i].res_name, retval
                    );
                }
            }
        }
    }

    items[0].transition_time = INT_MAX;
    for (unsigned int i=0; i<items.size(); i++) {
        if (items[i].res_server_state == RESULT_SERVER_STATE_IN_PROGRESS) {
            x = items[i].res_sent_time + items[0].delay_bound;
            if (x < items[0].transition_time) {
                items[0].transition_time = x;
            }
        }
    }

    retval = transitioner.update_workunit(items[0]);
    if (retval) {
        log_messages.printf(
            SCHED_MSG_LOG::CRITICAL,
            "[WU#%d %s] workunit.update() == %d\n", items[0].id, items[0].name, retval
        );
    }
}

bool do_pass() {
    DB_TRANSITIONER_ITEM_SET transitioner;
    std::vector<TRANSITIONER_ITEM> items;
    bool did_something = false;

    check_stop_daemons();

    // loop over entries that are due to be checked
    //
    while (!transitioner.enumerate((int)time(0), mod_n, mod_i, SELECT_LIMIT, items)) {
        did_something = true;
        handle_wu(transitioner, items);
        check_stop_daemons();
    }
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
            if (!do_pass()) sleep(1);
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
