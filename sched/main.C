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
// Portions created by the SETI@home project are Copyright (C) 2002, 2003
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

#include "config.h"
#include "server_types.h"
#include "handle_request.h"
#include "main.h"
#include "sched_util.h"

#define DEBUG_LEVEL  1

#define STDERR_FILENAME "cgi_out"
#define REQ_FILE_PREFIX "boinc_req_"
#define REPLY_FILE_PREFIX "boinc_reply_"
bool use_files = false;     // use disk files for req/reply msgs (for debugging)

DB_PROJECT gproject;
CONFIG config;

int main() {
    FILE* fin, *fout;
    int i, retval, pid;
    char buf[256], req_path[256], reply_path[256], path[256];
    SCHED_SHMEM* ssp;
    void* p;
    unsigned int counter=0;
    char* code_sign_key;
    bool found;

    if (!freopen(STDERR_FILENAME, "a", stderr)) {
        fprintf(stderr, "Can't redirect stderr\n");
        exit(1);
    }

    set_debug_level(DEBUG_LEVEL);

    retval = config.parse_file();
    if (retval) {
        write_log("Can't parse config file\n", MSG_CRITICAL);
        exit(1);
    }

    sprintf(path, "%s/code_sign_public", config.key_dir);
    retval = read_file_malloc(path, code_sign_key);
    if (retval) {
        sprintf(buf, "Can't read code sign key file (%s)\n", path);
        write_log(buf, MSG_CRITICAL);
        exit(1);
    }

    retval = attach_shmem(config.shmem_key, &p);
    if (retval) {
        write_log("Can't attach shmem (feeder not running?)\n", MSG_CRITICAL);
        exit(1);
    }
    ssp = (SCHED_SHMEM*)p;
    retval = ssp->verify();
    if (retval) {
        write_log("shmem has wrong struct sizes - recompile\n", MSG_CRITICAL);
        exit(1);
    }

    for (i=0; i<10; i++) {
        if (ssp->ready) break;
        write_log("waiting for ready flag\n", MSG_DEBUG);
        sleep(1);
    }
    if (!ssp->ready) {
        write_log("feeder doesn't seem to be running\n", MSG_CRITICAL);
        exit(1);
    }

    retval = boinc_db_open(config.db_name, config.db_passwd);
    if (retval) {
        write_log("can't open database\n", MSG_CRITICAL);
        exit(1);
    }

    found = false;
    while (!gproject.enumerate("")) {
        found = true;
    }
    if (!found) {
        write_log("can't find project\n", MSG_CRITICAL);
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
            write_log("can't write request file\n", MSG_CRITICAL);
            exit(1);
        }
        copy_stream(stdin, fout);
        fclose(fout);
        fin = fopen(req_path, "r");
        if (!fin) {
            write_log("can't read request file\n", MSG_CRITICAL);
            exit(1);
        }
        fout = fopen(reply_path, "w");
        if (!fout) {
            write_log("can't write reply file\n", MSG_CRITICAL);
            exit(1);
        }
        handle_request(fin, fout, *ssp, code_sign_key);
        fclose(fin);
        fclose(fout);
        fin = fopen(reply_path, "r");
        if (!fin) {
            write_log("can't read reply file\n", MSG_CRITICAL);
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
