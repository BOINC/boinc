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
#include <values.h>
#include <sys/time.h>

#include "boinc_db.h"
#include "util.h"
#include "backend_lib.h"
#include "sched_config.h"
#include "sched_util.h"

#define LOCKFILE                "transitioner.out"
#define PIDFILE                 "transitioner.pid"

int startup_time;
CONFIG config;
R_RSA_PRIVATE_KEY key;
// char app_name[256];

// The scheme for generating unique output filenames is as follows.
// If the original filename is of the form x__y,
// then y is replaced with a string of the form time_seqno,
// where "time" is when this program started up.
// NOTE: if you ever need to start up multiple copies of this,
// you'll need to add a PID in there somewhere.
//
// If the original filename doesn't have __, add a string
// of the form __time_seqno

void make_unique_name(char* name) {
    char buf[256], *p;
    static int seqno;

    sprintf(buf, "%d_%d", startup_time, seqno++);
    p = strstr(name, "__");
    if (p) {
        strcpy(p+2, buf);
    } else {
        strcat(name, "__");
        strcat(name, buf);
    }
}

// convert a result's XML document to generate new output filenames.
// The input has the form
// <file_info>
//    <name>xxx</name>
//    ...
// </file_info>
// ...
// <result>
//    <file_ref>
//       <file_name>xxx</file_name>
//       ...
//    </file_ref>
//    ...
// </result>
//
// Look for <name>...</name> elements within <file_info>
// and make a unique name based on it;
// apply the same conversion to the <file_name> element later on.
//
// TODO: this is ad-hoc.  Would be nice to use some generic
// XML parsing routines, or XSLT or something.
//
int assign_new_names(char* in) {
    char *p = in, *n1, *n2, *r;
    char name[256], newname[256], element[256], buf[MAX_BLOB_SIZE];
    int len;

    // notice where the <result> is so we don't try to convert
    // the result name
    //
    r = strstr(in, "<result>");

    while (1) {
        n1 = strstr(p, "<name>");
        if (!n1) break;

        if (n1 > r) break;      // don't go past <result>

        n1 += strlen("<name>");
        n2 = strstr(p, "</name>");
        if (!n2) {
            log_messages.printf(SchedMessages::CRITICAL, "assign_new_names(): malformed XML:\n%s", in);
            return 1;
        }
        len = n2 - n1;
        memcpy(name, n1, len);
        name[len] = 0;
        strcpy(newname, name);
        make_unique_name(newname);
        strcpy(buf, n2);
        strcpy(n1, newname);
        strcat(n1, buf);

        // replace the name in the <file_name> element
        //
        sprintf(element, "<file_name>%s</file_name>", name);
        n2 = strstr(n1, element);
        if (!n2) {
            log_messages.printf(SchedMessages::CRITICAL, "assign_new_names(): no <file_name>:\n%s", in);
            return 1;
        }
        strcpy(buf, n2+strlen(element));
        sprintf(element, "<file_name>%s</file_name>", newname);
        strcpy(n2, element);
        strcat(n2, buf);
        p = n1;
    }
    return 0;
}

void handle_wu(DB_WORKUNIT& wu) {
    vector<RESULT> results;
    DB_RESULT result;
    DB_RESULT canonical_result;
    int nerrors, retval, ninprogress, nsuccess;
    int nunsent, ncouldnt_send, nover;
    unsigned int i, n;
    char buf[256];
    time_t now = time(0), x;
    bool all_over, have_result_to_validate, do_delete;

    log_messages.printf(SchedMessages::DEBUG, "[WU#%d %s] handling WU\n", wu.id, wu.name);
    ScopeMessages scope_messages(log_messages, SchedMessages::NORMAL);

    // scan the results for the WU
    //
    sprintf(buf, "where workunitid=%d", wu.id);
    while (!result.enumerate(buf)) {
        results.push_back(result);
    }

    if (results.size() == 0) {
        log_messages.printf(
            SchedMessages::NORMAL, "[WU#%d %s] No results\n",
            wu.id, wu.name
        );
        return;
    }

    log_messages.printf(SchedMessages::DEBUG,
        "[WU#%d %s] enumerated %d results\n",
        wu.id, wu.name, (int)results.size()
    );

    // count up the number of results in various states,
    // and check for timed-out results
    //
    nunsent = 0;
    ninprogress = 0;
    nerrors = 0;
    nsuccess = 0;
    ncouldnt_send = 0;
    have_result_to_validate = false;
    for (i=0; i<results.size(); i++) {
        result = results[i];

        switch (result.server_state) {
        case RESULT_SERVER_STATE_UNSENT:
            nunsent++;
            break;
        case RESULT_SERVER_STATE_IN_PROGRESS:
            if (result.report_deadline < now) {
                log_messages.printf(
                    SchedMessages::NORMAL,
                    "[WU#%d %s] [RESULT#%d %s] result timed out (%d < %d)\n",
                    wu.id, wu.name, result.id, result.name,
                    result.report_deadline, (int)now
                );
                result.server_state = RESULT_SERVER_STATE_OVER;
                result.outcome = RESULT_OUTCOME_NO_REPLY;
                result.update();
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

    // trigger validation if we have a quorum
    // and some result hasn't been validated
    //
    if (nsuccess >= wu.min_quorum && have_result_to_validate) {
        wu.need_validate = true;
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
        for (i=0; i<results.size(); i++) {
            result = results[i];
            if (result.server_state == RESULT_SERVER_STATE_UNSENT) {
                result.server_state = RESULT_SERVER_STATE_OVER;
                result.outcome = RESULT_OUTCOME_DIDNT_NEED;
                result.update();
            }
        }
        if (wu.assimilate_state == ASSIMILATE_INIT) {
            wu.assimilate_state = ASSIMILATE_READY;
        }
    } else {
        // If no error, generate new results if needed.
        // Munge the XML of an existing result
        // to create unique new output filenames.
        //
        n = wu.target_nresults - nunsent - ninprogress;
        if (n > 0) {
            log_messages.printf(
                SchedMessages::NORMAL,
                "[WU#%d %s] Generating %d more results\n",
                wu.id, wu.name, n
            );
            for (i=0; i<n; i++) {
#if 0
                result = results[0];
                make_unique_name(result.name);
                initialize_result(result, wu);
                remove_signatures(result.xml_doc_in);
                assign_new_names(result.xml_doc_in);
                add_signatures(result.xml_doc_in, key);
                retval = result.insert();
                if (retval) {
                    log_messages.printf(
                        SchedMessages::CRITICAL,
                        "[WU#%d %s] [RESULT#%d %s] result.insert() %d\n",
                        wu.id, wu.name, result.id, result.name, retval
                    );
                    break;
                }
#endif
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
    canonical_result.id = 0;
    for (i=0; i<results.size(); i++) {
        result = results[i];
        if (result.server_state != RESULT_SERVER_STATE_OVER) {
            all_over = false;
        }
        if (result.id == wu.canonical_resultid) {
            canonical_result = result;
        }
    }
    if (wu.canonical_resultid && canonical_result.id == 0) {
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
        if (all_over) {
            wu.file_delete_state = FILE_DELETE_READY;
            log_messages.printf(
                SchedMessages::DEBUG,
                "[WU#%d %s] ASSIMILATE_DONE => setting FILE_DELETE_READY\n",
                wu.id, wu.name
            );

            // can delete canonical result outputs
            // if all successful results have been validated
            //
            if (canonical_result.id && canonical_result.file_delete_state == FILE_DELETE_INIT) {
                canonical_result.file_delete_state = FILE_DELETE_READY;
                canonical_result.update();
            }
        }

        // output of error results can be deleted immediately;
        // output of success results can be deleted if validated
        //
        for (i=0; i<results.size(); i++) {
            result = results[i];
            do_delete = false;
            switch(result.outcome) {
            case RESULT_OUTCOME_CLIENT_ERROR:
                do_delete = true;
                break;
            case RESULT_OUTCOME_SUCCESS:
                do_delete = (result.validate_state != VALIDATE_STATE_INIT);
                break;
            }
            if (do_delete) {
                result.file_delete_state = FILE_DELETE_READY;
                result.update();
            }
        }
    }

    wu.transition_time = MAXINT;
    for (i=0; i<results.size(); i++) {
        result = results[i];
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
            "[WU#%d %s] workunit.update() %d\n", wu.id, wu.name, retval
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
    }
    return did_something;
}

void main_loop(bool one_pass) {
    int retval;

    retval = boinc_db_open(config.db_name, config.db_passwd);
    if (retval) {
        log_messages.printf(SchedMessages::CRITICAL, "boinc_db_open: %d\n", retval);
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

    retval = config.parse_file();
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
