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

#include <stdio.h>

#include "parse.h"
#include "boinc_db.h"
#include "sched_config.h"

// wu_checker
// See whether input files that should be present, are

SCHED_CONFIG config;

// get the path a WU's input file
//
int get_file_path(WORKUNIT& wu, char* path) {
    char buf[256];
    bool flag;
    flag = parse_str(wu.xml_doc, "<name>", buf, sizeof(buf));
    if (!flag) return -1;
    sprintf(path, "%s/%s", config.upload_dir, buf);
    return 0;
}

void handle_result(DB_RESULT& result) {
    DB_WORKUNIT wu;
    int retval;
    char path[256];
    FILE* f;

    retval = wu.lookup_id(result.workunitid);
    if (retval) {
        printf(
            "ERROR: can't find WU %d for result %d\n",
            result.workunitid, result.id
        );
        return;
    }
    get_file_path(wu, path);
    f = fopen(path, "r");
    if (f) {
        fclose(f);
    } else {
        printf("ERROR can't find file %s for result %d\n",
            path, result.id
        );
    }
}

int main() {
    DB_RESULT result;
    char clause[256];

    config.parse_file();

    sprintf(clause, "where server_state=%d", RESULT_SERVER_STATE_UNSENT);
    while (!result.enumerate(clause)) {
        handle_result(result);
    }
    sprintf(clause, "where server_state=%d", RESULT_SERVER_STATE_IN_PROGRESS);
    while (!result.enumerate(clause)) {
        handle_result(result);
    }
}
