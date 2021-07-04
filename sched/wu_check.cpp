// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2019 University of California
//
// BOINC is free software; you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation,
// either version 3 of the License, or (at your option) any later version.
//
// BOINC is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with BOINC.  If not, see <http://www.gnu.org/licenses/>.

// wu_check
// look for results with missing input files
// --repair      change them to server_state OVER, outcome COULDNT_SEND
//
// NOTE 1: this assumes that jobs have a single input file.
// NOTE 2: should rewrite to enumerate WUs, not results

#include "config.h"
#include <sys/param.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>

#include "boinc_db.h"
#include "svn_version.h"

#include "error_numbers.h"
#include "filesys.h"
#include "parse.h"
#include "util.h"

#include "sched_config.h"
#include "sched_util.h"

bool repair = false;

// wu_check
// See whether input files that should be present, are

// get the path a WU's input file
//
int get_file_path(WORKUNIT& wu, char* path) {
    char buf[256];
    bool flag;
    flag = parse_str(wu.xml_doc, "<name>", buf, sizeof(buf));
    if (!flag) return ERR_XML_PARSE;
    dir_hier_path(buf, config.download_dir, config.uldl_dir_fanout, path);
    return 0;
}

int handle_result(DB_RESULT& result) {
    DB_WORKUNIT wu;
    int retval;
    char path[MAXPATHLEN];
    char buf[256];
    FILE* f;

    retval = wu.lookup_id(result.workunitid);
    if (retval) {
        printf(
            "ERROR: can't find WU %lu for result %lu\n",
            result.workunitid, result.id
        );
        return 1;
    }
    if (get_file_path(wu, path)) { // no input files
        return 0;
    }
    f = fopen(path, "r");
    if (f) {
        fclose(f);
    } else {
        printf("no file %s for result %lu\n",
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
                        "ERROR: can't update result %lu\n",
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

void usage(char *name) {
    fprintf(stderr,
        "Looks for results with missing input files\n\n"
        "Usage: %s [OPTION]\n\n"
        "Options:\n"
        "  [ --repair ]                   change them to server_state OVER,\n"
        "                                 outcome COULDNT_SEND\n"
        "  [ -h | --help ]                Shows this help text\n"
        "  [ -v | --version ]             Shows version information\n",
        name
    );
}

int main(int argc, char** argv) {
    DB_RESULT result;
    char clause[256];
    int retval, n, nerr;

    for(int c = 1; c < argc; c++) {
        std::string option(argv[c]);
        if(option == "-h" || option == "--help") {
            usage(argv[0]);
            exit(0);
        } else if(option == "-v" || option == "--version") {
            printf("%s\n", SVN_VERSION);
            exit(0);
        } else if (option == "--repair") {
            repair = true;
        } else {
            fprintf(stderr, "unknown command line argument: %s\n\n", argv[c]);
            usage(argv[0]);
            exit(1);
        }
    }

    retval = config.parse_file();
    if (retval) exit(1);

    retval = boinc_db.open(
        config.db_name, config.db_host, config.db_user, config.db_passwd
    );
    if (retval) {
        printf("boinc_db.open failed: %s\n", boinc_db.error_string());
        exit(1);
    }

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
