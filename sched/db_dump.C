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

// db_dump: dump parts of database in XML form

// files:
//
// team_totalcredit_N.xml       N = 0, 1, ...
// team_avgcredit_N.xml         N = 0, 1, ...
// user_totalcredit_N.xml       N = 0, 1, ...
// user_avgcredit_N.xml         N = 0, 1, ...
// user_team_N.xml              N = team ID
// host_user_N.xml              N = user ID
// host_totalcredit_N.xml       N = 0, 1, ...
// host_avgcredit_N.xml         N = 0, 1, ...

#include <stdio.h>

#include "db.h"
#include "config.h"

void write_team(TEAM& team, FILE* f) {
    fprintf(f,
        "<team>"
    );
}

void team_totalcredit() {
    TEAM team;
    FILE* f;
    int nfile=0, nrec;
    char buf[256];

    while (!db_team_enum_total_credit(team)) {
        if (!f) {
            sprintf(buf, "team_totalcredit_%d", nf);
            f = fopen(buf, "w");
            nf++;
            nrec = 0;
        }
        write_team(team, f);
        nrec++;
        if (nrec == NRECS_PER_FILE) {
            fclose(f);
            f = 0;
        }
    }
}

main() {
    CONFIG config;
    int retval;

    retval = config.parse_file();
    if (retval) {
        fprintf(stderr, "Can't parse config file\n");
        exit(1);
    }
    retval = boinc_db_open(config.db_name, config.db_passwd);
    if (retval) {
        fprintf(stderr, "Can't open DB\n");
        exit(1);
    }
    team_totalcredit();
}
