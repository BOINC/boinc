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

#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <vector>

#include "db.h"
#include "parse.h"
#include "util.h"
#include "config.h"
#include "assimilate_handler.h"

#define ASSIMILATOR_LOCKFILE "assimilator.out"

CONFIG config;

void write_log(char* p) {
    fprintf(stderr, "%s: %s", timestamp(), p);
}

// assimilate all WUs that need it
// return nonzero if did anything
//
bool do_pass(APP& app) {
    WORKUNIT wu;
    RESULT canonical_result, result;
    vector<RESULT> results;
    bool did_something = false, delete_inputs, delete_outputs;
    char buf[MAX_BLOB_SIZE];
    unsigned int i;

    wu.appid = app.id;
    wu.assimilate_state = ASSIMILATE_READY;
    while (!db_workunit_enum_app_assimilate_state(wu)) {
        did_something = true;

        sprintf(buf,
            "Assimilating WU %s, assim state %d\n",
            wu.name, wu.assimilate_state
        );
        write_log(buf);

        result.workunitid = wu.id;
        while (!db_result_enum_wuid(result)) {
            results.push_back(result);
            if (result.id == wu.canonical_resultid) {
                canonical_result = result;
            }
        }

        assimilate_handler(wu, results, canonical_result);

        delete_outputs = true;
        delete_inputs = true;
        for (i=0; i<results.size(); i++) {
            result = results[i];
            if (result.server_state != RESULT_SERVER_STATE_OVER
                || (result.outcome != RESULT_OUTCOME_SUCCESS && result.outcome != RESULT_OUTCOME_CLIENT_ERROR)
            ) {
                delete_outputs = false;
            }
            if (result.server_state != RESULT_SERVER_STATE_OVER) {
                delete_inputs = false;
            }
        }

        if (delete_outputs) {
            for (i=0; i<results.size(); i++) {
                result = results[i];
                result.file_delete_state = FILE_DELETE_READY;
                db_result_update(result);
            }
        } else {
            for (i=0; i<results.size(); i++) {
                result = results[i];
                if (result.server_state == RESULT_SERVER_STATE_OVER
                    && result.id != wu.canonical_resultid
                    && (result.outcome == RESULT_OUTCOME_SUCCESS || result.outcome == RESULT_OUTCOME_CLIENT_ERROR)
                ) {
                    result.file_delete_state = FILE_DELETE_READY;
                    db_result_update(result);
                }
            }
        }

        wu.assimilate_state = ASSIMILATE_DONE;
        if (delete_inputs) {
            wu.file_delete_state = FILE_DELETE_READY;
        }
        db_workunit_update(wu);
    }
    return did_something;
}

int main(int argc, char** argv) {
    int retval;
    bool asynch = false, one_pass = false;
    APP app;
    int i;
    char buf[256];

    for (i=1; i<argc; i++) {
        if (!strcmp(argv[i], "-asynch")) {
            asynch = true;
        } else if (!strcmp(argv[i], "-one_pass")) {
            one_pass = true;
        } else if (!strcmp(argv[i], "-app")) {
            strcpy(app.name, argv[++i]);
        } else {
            sprintf(buf, "Unrecognized arg: %s\n", argv[i]);
            write_log(buf);
        }
    }

    retval = config.parse_file();
    if (retval) {
        write_log("Can't parse config file\n");
        exit(1);
    }

    if (asynch) {
        if (fork()) {
            exit(0);
        }
    }

    if (lock_file(ASSIMILATOR_LOCKFILE)) {
        fprintf(stderr, "Another copy of assimilator is already running\n");
        exit(1);
    }

    retval = boinc_db_open(config.db_name, config.db_passwd);
    if (retval) {
        write_log("Can't open DB\n");
        exit(1);
    }
    retval = db_app_lookup_name(app);
    if (retval) {
        write_log("Can't find app\n");
        exit(1);
    }
    if (one_pass) {
        do_pass(app);
    } else {
        while (1) {
            if (!do_pass(app)) sleep(10);
        }
    }
}
