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

// wu_check [-repair]
// look for results with missing input files
// -repair      change them to server_state OVER, outcome COULDNT_SEND

#include <cstdio>

#include "boinc_db.h"

#include "parse.h"
#include "util.h"
#include "error_numbers.h"

#include "sched_config.h"
#include "sched_util.h"

bool repair = false;

// wu_checker
// See whether input files that should be present, are

SCHED_CONFIG config;

// get the path a WU's input file
//
int get_file_path(WORKUNIT& wu, char* path) {
    char buf[256];
    bool flag;
    flag = parse_str(wu.xml_doc, "<name>", buf, sizeof(buf));
    if (!flag) return ERR_XML_PARSE;
    dir_hier_path(buf, config.download_dir, config.uldl_dir_fanout, true, path);
    return 0;
}

int handle_result(DB_RESULT& result) {
    DB_WORKUNIT wu;
    int retval;
    char path[256];
    char buf[256];
    FILE* f;

    retval = wu.lookup_id(result.workunitid);
    if (retval) {
        printf(
            "ERROR: can't find WU %d for result %d\n",
            result.workunitid, result.id
        );
        return 1;
    }
    get_file_path(wu, path);
    f = fopen(path, "r");
    if (f) {
        fclose(f);
    } else {
        printf("no file %s for result %d\n",
            path, result.id
        );
        if (repair) {
            if (result.server_state == RESULT_SERVER_STATE_UNSENT) {
                result.server_state = RESULT_SERVER_STATE_OVER;
                result.outcome = RESULT_OUTCOME_COULDNT_SEND;
                sprintf(
                    buf,"server_state=%d, outcome=%d",
                    result.server_state, result.outcome
                );
                retval = result.update_field(buf);
                if (retval) {
                    printf(
                        "ERROR: can't update result %d\n",
                        result.id
                    );
                    return 1;
                }
            }
        }
        return 1;
    }
    return 0;
}

int main(int argc, char** argv) {
    DB_RESULT result;
    char clause[256];
    int retval, n, nerr;

    retval = config.parse_file();
    if (retval) exit(1);

    retval = boinc_db.open(config.db_name, config.db_host, config.db_user, config.db_passwd);
    if (retval) {
        printf("boinc_db.open: %d\n", retval);
        exit(1);
    }
    if (argc > 1 && !strcmp(argv[1], "-repair")) repair = true;

    n = nerr = 0;
    printf("Unsent results:\n");
    sprintf(clause, "where server_state=%d", RESULT_SERVER_STATE_UNSENT);
    while (!result.enumerate(clause)) {
        retval = handle_result(result);
        n++;
        if (retval) nerr++;
    }
    printf("%d out of %d errors\n", nerr, n);
    n = nerr = 0;
    printf("In progress results:\n");
    sprintf(clause, "where server_state=%d", RESULT_SERVER_STATE_IN_PROGRESS);
    while (!result.enumerate(clause)) {
        retval = handle_result(result);
        n++;
        if (retval) nerr++;
    }
    printf("%d out of %d errors\n", nerr, n);
}

const char *BOINC_RCSID_8f4e399992 = "$Id$";
