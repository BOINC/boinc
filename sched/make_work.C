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

// make_work
//      -wu_name name
//      -result_template filename
//      [ -redundancy n ]
//      [ -cushion n ]
//
// Create WU and result records as needed to maintain a pool of work
// (for testing purposes).
// Makes a new WU for every "redundancy" results.
// Clones the WU of the given name.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

#include "db.h"
#include "crypt.h"
#include "backend_lib.h"
#include "config.h"

#define TRIGGER_FILENAME    "stop_server"

int cushion = 10;
int redundancy = 10;
char wu_name[256], result_template_file[256];

void check_trigger() {
    FILE* f = fopen(TRIGGER_FILENAME, "r");
    if (!f) return;
    exit(0);
}

void make_work() {
    CONFIG config;
    int retval, i, start_time=time(0), n, nresults_left;
    char keypath[256], suffix[256], result_template[MAX_BLOB_SIZE];
    R_RSA_PRIVATE_KEY key;
    WORKUNIT wu;

    retval = config.parse_file();
    if (retval) {
        fprintf(stderr, "make_work: can't read config file\n");
        exit(1);
    }

    retval = db_open(config.db_name, config.db_passwd);
    if (retval) {
        fprintf(stderr, "make_work: can't open db\n");
        exit(1);
    }

    strcpy(wu.name, wu_name);
    retval = db_workunit_lookup_name(wu);
    if (retval) {
        fprintf(stderr, "make_work: can't find wu %s\n", wu_name);
        exit(1);
    }

    sprintf(keypath, "%s/upload_private", config.key_dir);
    retval = read_key_file(keypath, key);
    if (retval) {
        fprintf(stderr, "make_work: can't read key\n");
        exit(1);
    }

    retval = read_filename(result_template_file, result_template);
    if (retval) {
        fprintf(stderr, "make_work: can't open result template\n");
        exit(1);
    }

    nresults_left = 0;
    while (true) {
        fflush(stdout);
        retval = db_result_count_state(RESULT_STATE_UNSENT, n);
        if (retval) {
            fprintf(stderr, "make_work: can't counts results\n");
            exit(1);
        }
        printf("make_work: %d results\n", n);
        if (n > cushion) {
            sleep(1);
            continue;
        }

        if (nresults_left == 0) {
            nresults_left = redundancy;
            sprintf(wu.name, "wu_%d_%d", start_time, i++);
            wu.id = 0;
            wu.create_time = time(0);
            retval = db_workunit_new(wu);
            wu.id = db_insert_id();
        }
        sprintf(suffix, "%d_%d", start_time, i++);
        create_result(
            wu, result_template, suffix, key,
            config.upload_url, config.download_url
        );
        printf("make_work: added a result\n");
        nresults_left--;
    }
}

int main(int argc, char** argv) {
    bool asynch = false;
    int i;

    unlink(TRIGGER_FILENAME);
    for (i=1; i<argc; i++) {
        if (!strcmp(argv[i], "-asynch")) {
            asynch = true;
        } else if (!strcmp(argv[i], "-cushion")) {
            cushion = atoi(argv[++i]);
        } else if (!strcmp(argv[i], "-result_template")) {
            strcpy(result_template_file, argv[++i]);
        } else if (!strcmp(argv[i], "-wu_name")) {
            strcpy(wu_name, argv[++i]);
        }
    }

    if (!strlen(result_template_file)) {
        fprintf(stderr, "make_work: missing -result_template\n");
        exit(1);
    }
    if (!strlen(wu_name)) {
        fprintf(stderr, "make_work: missing -wu_name\n");
        exit(1);
    }

    if (asynch) {
        if (!fork()) {
            make_work();
        }
    } else {
        make_work();
    }
}
