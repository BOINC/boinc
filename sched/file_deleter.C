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


// file_deleter: deletes files that are no longer needed

#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#include "db.h"
#include "parse.h"
#include "util.h"
#include "config.h"

#define LOCKFILE "file_deleter.out"

CONFIG config;

void write_log(char* p) {
    time_t now = time(0);
    char* timestr = ctime(&now);
    *(strchr(timestr, '\n')) = 0;
    fprintf(stderr, "%s: %s", timestr, p);
}

int wu_delete_files(WORKUNIT& wu) {
    char* p;
    char filename[256], pathname[256], buf[MAX_BLOB_SIZE], logbuf[256];
    bool no_delete;

    strcpy(buf,wu.xml_doc);

    p = strtok(buf, "\n");
    strcpy(filename, "");
    while (p) {
        if (parse_str(p, "<name>", filename, sizeof(filename))) {
        } else if (match_tag(p, "<file_info>")) {
            no_delete = false;
            strcpy(filename, "");
        } else if (match_tag(p, "<no_delete/>")) {
            no_delete = true;
        } else if (match_tag(p, "</file_info>")) {
            if (!no_delete) {
                sprintf(pathname, "%s/%s", config.download_dir, filename);
                sprintf(logbuf, "deleting %s\n", pathname);
                write_log(logbuf);
                unlink(pathname);
            }
        }
        p = strtok(0, "\n");
    }
    return 0;
}

int result_delete_files(RESULT& result) {
    char* p;
    char filename[256], pathname[256], buf[MAX_BLOB_SIZE], logbuf[256];
    bool no_delete;

    strcpy(buf,result.xml_doc_in);
    p = strtok(buf,"\n");
    while (p) {
        if (parse_str(p, "<name>", filename, sizeof(filename))) {
        } else if (match_tag(p, "<file_info>")) {
            no_delete = false;
            strcpy(filename, "");
        } else if (match_tag(p, "<no_delete/>")) {
            no_delete = true;
        } else if (match_tag(p, "</file_info>")) {
            if (!no_delete) {
                sprintf(pathname, "%s/%s", config.upload_dir, filename);
                sprintf(logbuf, "deleting %s\n", pathname);
                write_log(logbuf);
                unlink(pathname);
            }
        }
        p = strtok(0, "\n");
    }
    return 0;
}

// return nonzero if did anything
//
bool do_pass() {
    WORKUNIT wu;
    RESULT result;
    bool did_something = false;

    wu.file_delete_state = FILE_DELETE_READY;
    while (!db_workunit_enum_file_delete_state(wu)) {
        did_something = true;
        wu_delete_files(wu);
        wu.file_delete_state = FILE_DELETE_DONE;
        db_workunit_update(wu);
    }

    result.file_delete_state = FILE_DELETE_READY;
    while (!db_result_enum_file_delete_state(result)) {
        did_something = true;
        result_delete_files(result);
        result.file_delete_state = FILE_DELETE_DONE;
        db_result_update(result);
    }
    return did_something;
}

int main(int argc, char** argv) {
    int retval;
    bool asynch = false, one_pass = false;
    int i;
    char buf[256];

    for (i=1; i<argc; i++) {
        if (!strcmp(argv[i], "-asynch")) {
            asynch = true;
        } else if (!strcmp(argv[i], "-one_pass")) {
            one_pass = true;
        } else {
            sprintf(buf, "Unrecognized arg: %s\n", argv[i]);
            write_log(buf);
        }
    }

    if (lock_file(LOCKFILE)) {
        fprintf(stderr, "Another copy of file deleter is running\n");
        exit(1);
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

    retval = boinc_db_open(config.db_name, config.db_passwd);
    if (retval) {
        write_log("can't open DB\n");
        exit(1);
    }
    if (one_pass) {
        do_pass();
    } else {
        while (1) {
            if (!do_pass()) sleep(10);
        }
    }
}
