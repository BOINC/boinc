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

#include <cassert>
#include <cstdio>
#include <vector>
#include <string>
using namespace std;

#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "boinc_db.h"
#include "parse.h"
#include "filesys.h"
#include "error_numbers.h"
#include "shmem.h"
#include "util.h"

#include "sched_config.h"
#include "server_types.h"
#include "handle_request.h"
#include "sched_util.h"
#include "sched_msgs.h"
#include "main.h"

#ifdef _USING_FCGI_
#include "fcgi_stdio.h"
#endif

#define DEBUG_LEVEL  999
#define MAX_FCGI_COUNT  20

#define REQ_FILE_PREFIX "boinc_req_"
#define REPLY_FILE_PREFIX "boinc_reply_"
bool use_files = false;     // use disk files for req/reply msgs (for debugging)

SCHED_CONFIG config;
GUI_URLS gui_urls;
key_t sema_key;
int g_pid;

void send_message(char* msg, int delay) {
    printf(
        "Content-type: text/plain\n\n"
        "<scheduler_reply>\n"
        "    <message priority=\"low\">%s</message>\n"
        "    <request_delay>%d</request_delay>\n"
        "    <project_is_down/>\n"
        "</scheduler_reply>\n",
        msg, delay
    );
}

static bool db_opened=false;
int open_database() {
    int retval;

    if (db_opened) return 0;

    retval = boinc_db.open(config.db_name, config.db_host, config.db_user, config.db_passwd);
    if (retval) {
        log_messages.printf(SCHED_MSG_LOG::CRITICAL, "can't open database\n");
        return retval;
    }
    db_opened = true;
    return 0;
}

int main() {
    FILE* fin, *fout;
    int i, retval;
    char req_path[256], reply_path[256], path[256];
    SCHED_SHMEM* ssp=0;
    void* p;
    unsigned int counter=0;
    char* code_sign_key;
    bool project_stopped = false;

#ifndef _USING_FCGI_
    char *stderr_buffer, buf[256];
    get_log_path(path, "cgi.log");
    if (!freopen(path, "a", stderr)) {
        fprintf(stderr, "Can't redirect stderr\n");
        sprintf(buf, "Server can't open log file (%s)", path);
        send_message(buf, 3600);
        exit(0);
    }
    // install a larger buffer for stderr.  This ensures that
    // log information from different scheduler requests running
    // in parallel don't collide in the log file and appear
    // intermingled.
    //
    if (!(stderr_buffer=(char *)malloc(32768)) || setvbuf(stderr, stderr_buffer, _IOFBF, 32768)) {
        log_messages.printf(SCHED_MSG_LOG::CRITICAL,
            "Unable to change stderr buffering preferences\n"
        );
    }
#endif

    srand(time(0)+getpid());
    log_messages.set_debug_level(DEBUG_LEVEL);

    if (check_stop_sched()) {
        send_message("Project is temporarily shut down for maintenance", 3600);
        goto done;
    }

    retval = config.parse_file("..");
    if (retval) {
        log_messages.printf(SCHED_MSG_LOG::CRITICAL, "Can't parse config file\n");
        send_message("Server can't parse configuration file", 3600);
        exit(0);
    }

    gui_urls.init();

    sprintf(path, "%s/code_sign_public", config.key_dir);
    retval = read_file_malloc(path, code_sign_key);
    if (retval) {
        log_messages.printf(SCHED_MSG_LOG::CRITICAL,
            "Can't read code sign key file (%s)\n", path
        );
        send_message("Server can't find key file", 3600);
        exit(0);
    }

    get_project_dir(path, sizeof(path));
    get_key(path, 'a', sema_key);

    retval = attach_shmem(config.shmem_key, &p);
    if (retval || p==0) {
        log_messages.printf(SCHED_MSG_LOG::CRITICAL,
            "Can't attach shmem (feeder not running?)\n"
        );
        project_stopped = true;
    } else {
        ssp = (SCHED_SHMEM*)p;
        retval = ssp->verify();
        if (retval) {
            log_messages.printf(SCHED_MSG_LOG::CRITICAL,
                "shmem has wrong struct sizes - recompile\n"
            );
            send_message("Server has software problem", 3600);
            exit(0);
        }

        for (i=0; i<10; i++) {
            if (ssp->ready) break;
            log_messages.printf(SCHED_MSG_LOG::DEBUG, "waiting for ready flag\n");
            sleep(1);
        }
        if (!ssp->ready) {
            log_messages.printf(SCHED_MSG_LOG::CRITICAL, "feeder doesn't seem to be running\n");
            send_message("Server has software problem", 3600);
            exit(0);
        }
    }

    g_pid = getpid();
#ifdef _USING_FCGI_
    //while(FCGI_Accept() >= 0 && counter < MAX_FCGI_COUNT) {
    while(FCGI_Accept() >= 0) {
    counter++;
#endif
    if (project_stopped) {
        send_message("Project is temporarily shut down for maintenance", 3600);
        goto done;
    }
    fprintf(stdout,"Content-type: text/plain\n\n");
    if (use_files) {
        // the code below is convoluted because,
        // instead of going from stdin to stdout directly,
        // we go via a pair of disk files
        // (this makes it easy to save the input,
        // and to know the length of the output).
        //
        sprintf(req_path, "%s%d_%u", REQ_FILE_PREFIX, g_pid, counter);
        sprintf(reply_path, "%s%d_%u", REPLY_FILE_PREFIX, g_pid, counter);
        fout = fopen(req_path, "w");
        if (!fout) {
            log_messages.printf(SCHED_MSG_LOG::CRITICAL, "can't write request file\n");
            exit(1);
        }
        copy_stream(stdin, fout);
        fclose(fout);
        fin = fopen(req_path, "r");
        if (!fin) {
            log_messages.printf(SCHED_MSG_LOG::CRITICAL, "can't read request file\n");
            exit(1);
        }
        fout = fopen(reply_path, "w");
        if (!fout) {
            log_messages.printf(SCHED_MSG_LOG::CRITICAL, "can't write reply file\n");
            exit(1);
        }
        handle_request(fin, fout, *ssp, code_sign_key);
        fclose(fin);
        fclose(fout);
        fin = fopen(reply_path, "r");
        if (!fin) {
            log_messages.printf(SCHED_MSG_LOG::CRITICAL, "can't read reply file\n");
            exit(1);
        }
        copy_stream(fin, stdout);
        fclose(fin);
        //unlink(req_path);
        //unlink(reply_path);
    } else {
        handle_request(stdin, stdout, *ssp, code_sign_key);
    }
done:
#ifdef _USING_FCGI_
    fprintf(stderr, "FCGI: counter: %d\n", counter);
    continue;
    }
    if (counter == MAX_FCGI_COUNT) {
      fprintf(stderr, "FCGI: counter passed MAX_FCGI_COUNT - exiting..\n");
    }
    else {
      fprintf(stderr, "FCGI: FCGI_Accept failed - exiting..\n");
    }
    // when exiting, write headers back to apache so it won't complain
    // about "incomplete headers"
    fprintf(stdout,"Content-type: text/plain\n\n");
#endif
    if (db_opened) {
        boinc_db.close();
    }
}

const char *BOINC_RCSID_0ebdf5d770 = "$Id$";
