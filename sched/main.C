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

// The BOINC scheduling server.
//
// command-line options:

#include <iostream>
#include <vector>
#include <string>
using namespace std;

#include <stdio.h>
#include <unistd.h>
#include <assert.h>

#include "boinc_db.h"
#include "parse.h"
#include "shmem.h"
#include "util.h"

#include "sched_config.h"
#include "server_types.h"
#include "handle_request.h"
#include "main.h"
#include "sched_util.h"

#define DEBUG_LEVEL  999

//#define STDERR_FILENAME "cgi_out"
static const char* STDERR_FILENAME = "../log/cgi.log";

#define REQ_FILE_PREFIX "boinc_req_"
#define REPLY_FILE_PREFIX "boinc_reply_"
bool use_files = false;     // use disk files for req/reply msgs (for debugging)

DB_PROJECT gproject;
SCHED_CONFIG config;

int main() {
    FILE* fin, *fout;
    int i, retval, pid;
    char req_path[256], reply_path[256], path[256];
    SCHED_SHMEM* ssp;
    void* p;
    unsigned int counter=0;
    char* code_sign_key;
    bool found;

    if (!freopen(STDERR_FILENAME, "a", stderr)) {
        fprintf(stderr, "Can't redirect stderr\n");
        exit(1);
    }

    log_messages.set_debug_level(DEBUG_LEVEL);

    retval = config.parse_file("..");
    if (retval) {
        log_messages.printf(SchedMessages::CRITICAL, "Can't parse config file\n");
        exit(1);
    }

    sprintf(path, "%s/code_sign_public", config.key_dir);
    retval = read_file_malloc(path, code_sign_key);
    if (retval) {
        log_messages.printf(SchedMessages::CRITICAL, "Can't read code sign key file (%s)\n", path);
        exit(1);
    }

    retval = attach_shmem(config.shmem_key, &p);
    if (retval) {
        log_messages.printf(SchedMessages::CRITICAL, "Can't attach shmem (feeder not running?)\n");
        exit(1);
    }
    ssp = (SCHED_SHMEM*)p;
    retval = ssp->verify();
    if (retval) {
        log_messages.printf(SchedMessages::CRITICAL, "shmem has wrong struct sizes - recompile\n");
        exit(1);
    }

    for (i=0; i<10; i++) {
        if (ssp->ready) break;
        log_messages.printf(SchedMessages::DEBUG, "waiting for ready flag\n");
        sleep(1);
    }
    if (!ssp->ready) {
        log_messages.printf(SchedMessages::CRITICAL, "feeder doesn't seem to be running\n");
        exit(1);
    }

    retval = boinc_db_open(config.db_name, config.db_passwd);
    if (retval) {
        log_messages.printf(SchedMessages::CRITICAL, "can't open database\n");
        exit(1);
    }

    found = false;
    while (!gproject.enumerate("")) {
        found = true;
    }
    if (!found) {
        log_messages.printf(SchedMessages::CRITICAL, "can't find project\n");
        exit(1);
    }

    pid = getpid();
#ifdef _USING_FCGI_
    while(FCGI_Accept() >= 0) {
    counter++;
#endif
    printf("Content-type: text/plain\n\n");
    if (use_files) {
        // the code below is convoluted because,
        // instead of going from stdin to stdout directly,
        // we go via a pair of disk files
        // (this makes it easy to save the input,
        // and to know the length of the output).
        //
        sprintf(req_path, "%s%d_%u", REQ_FILE_PREFIX, pid, counter);
        sprintf(reply_path, "%s%d_%u", REPLY_FILE_PREFIX, pid, counter);
        fout = fopen(req_path, "w");
        if (!fout) {
            log_messages.printf(SchedMessages::CRITICAL, "can't write request file\n");
            exit(1);
        }
        copy_stream(stdin, fout);
        fclose(fout);
        fin = fopen(req_path, "r");
        if (!fin) {
            log_messages.printf(SchedMessages::CRITICAL, "can't read request file\n");
            exit(1);
        }
        fout = fopen(reply_path, "w");
        if (!fout) {
            log_messages.printf(SchedMessages::CRITICAL, "can't write reply file\n");
            exit(1);
        }
        handle_request(fin, fout, *ssp, code_sign_key);
        fclose(fin);
        fclose(fout);
        fin = fopen(reply_path, "r");
        if (!fin) {
            log_messages.printf(SchedMessages::CRITICAL, "can't read reply file\n");
            exit(1);
        }
        copy_stream(fin, stdout);
        fclose(fin);
        //unlink(req_path);
        //unlink(reply_path);
    } else {
        handle_request(stdin, stdout, *ssp, code_sign_key);
    }
#ifdef _USING_FCGI_
    }
#endif
    boinc_db_close();
}
