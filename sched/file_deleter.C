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


// file_deleter: deletes files that are no longer needed

#include <cstring>
#include <cstdlib>
#include <ctime>
#include <unistd.h>

#include "boinc_db.h"
#include "parse.h"
#include "util.h"
#include "sched_config.h"
#include "sched_util.h"
#include "sched_msgs.h"

#define LOCKFILE "file_deleter.out"
#define PIDFILE  "file_deleter.pid"

SCHED_CONFIG config;

int wu_delete_files(WORKUNIT& wu) {
    char* p;
    char filename[256], pathname[256], buf[LARGE_BLOB_SIZE];
    bool no_delete=false;
    int count_deleted = 0;

    safe_strcpy(buf, wu.xml_doc);

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
                log_messages.printf(SCHED_MSG_LOG::NORMAL, "[%s] deleting download/%s\n", wu.name, filename);
                unlink(pathname);
                ++count_deleted;
            }
        }
        p = strtok(0, "\n");
    }
    log_messages.printf(SCHED_MSG_LOG::DEBUG, "[%s] deleted %d file(s)\n", wu.name, count_deleted);
    return 0;
}

int result_delete_files(RESULT& result) {
    char* p;
    char filename[256], pathname[256], buf[LARGE_BLOB_SIZE];
    bool no_delete=false;
    int count_deleted = 0, retval;

    safe_strcpy(buf, result.xml_doc_in);
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
                retval = unlink(pathname);
                ++count_deleted;
                log_messages.printf(SCHED_MSG_LOG::NORMAL,
                    "[%s] unlinked %s; retval %d\n",
                    result.name, filename, retval
                );
            }
        }
        p = strtok(0, "\n");
    }

    log_messages.printf(SCHED_MSG_LOG::DEBUG,
        "[%s] deleted %d file(s)\n", result.name, count_deleted
    );
    return 0;
}

// return nonzero if did anything
//
bool do_pass() {
    DB_WORKUNIT wu;
    DB_RESULT result;
    bool did_something = false;
    char buf[256];

    check_stop_daemons();

    sprintf(buf, "where file_delete_state=%d", FILE_DELETE_READY);
    while (!wu.enumerate(buf)) {
        did_something = true;
        wu_delete_files(wu);
        wu.file_delete_state = FILE_DELETE_DONE;
        wu.update();
    }

    sprintf(buf, "where file_delete_state=%d", FILE_DELETE_READY);
    while (!result.enumerate(buf)) {
        did_something = true;
        result_delete_files(result);
        result.file_delete_state = FILE_DELETE_DONE;
        result.update();
    }
    return did_something;
}

int main(int argc, char** argv) {
    int retval;
    bool asynch = false, one_pass = false;
    int i;

    check_stop_daemons();
    for (i=1; i<argc; i++) {
        if (!strcmp(argv[i], "-asynch")) {
            asynch = true;
        } else if (!strcmp(argv[i], "-one_pass")) {
            one_pass = true;
        } else if (!strcmp(argv[i], "-d")) {
            log_messages.set_debug_level(atoi(argv[++i]));
        } else {
            log_messages.printf(SCHED_MSG_LOG::CRITICAL, "Unrecognized arg: %s\n", argv[i]);
        }
    }

    retval = config.parse_file("..");
    if (retval) {
        log_messages.printf(SCHED_MSG_LOG::CRITICAL, "Can't parse config file\n");
        exit(1);
    }

    if (asynch) {
        if (fork()) {
            exit(0);
        }
    }

    // // Call lock_file after fork(), because file locks are not always inherited
    // if (lock_file(LOCKFILE)) {
    //     log_messages.printf(SCHED_MSG_LOG::NORMAL, "Another copy of file deleter is running\n");
    //     exit(1);
    // }
    // write_pid_file(PIDFILE);
    log_messages.printf(SCHED_MSG_LOG::NORMAL, "Starting\n");

    retval = boinc_db.open(config.db_name, config.db_host, config.db_user, config.db_passwd);
    if (retval) {
        log_messages.printf(SCHED_MSG_LOG::CRITICAL, "can't open DB\n");
        exit(1);
    }
    install_stop_signal_handler();
    if (one_pass) {
        do_pass();
    } else {
        while (1) {
            if (!do_pass()) sleep(10);
        }
    }
}
