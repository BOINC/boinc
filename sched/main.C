// Berkeley Open Infrastructure for Network Computing
// http://boinc.berkeley.edu
// Copyright (C) 2005 University of California
//
// This is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation;
// either version 2.1 of the License, or (at your option) any later version.
//
// This software is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//
// To view the GNU Lesser General Public License visit
// http://www.gnu.org/copyleft/lesser.html
// or write to the Free Software Foundation, Inc.,
// 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

// The BOINC scheduling server.
//
// command-line options:

#include <cassert>
#include <cstdio>
#include <vector>
#include <string>
using namespace std;

#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/resource.h>

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

// Useful for debugging, if your cgi script keeps crashing.  This
// makes it dump a core file that you can load into a debugger to see
// where the problem is.
#define DUMP_CORE_ON_SEGV 0

#define DEBUG_LEVEL  999
#define MAX_FCGI_COUNT  20

#define REQ_FILE_PREFIX "boinc_req_"
#define REPLY_FILE_PREFIX "boinc_reply_"
bool use_files = false;     // use disk files for req/reply msgs (for debugging)

SCHED_CONFIG config;
GUI_URLS gui_urls;
key_t sema_key;
int g_pid;

void send_message(const char* msg, int delay) {
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

void debug_sched(SCHEDULER_REQUEST& sreq, SCHEDULER_REPLY& sreply, const char *trigger); 

// If the scheduler 'hangs', which it can do if a request is not fully processed
// or some other process arises, then Apache will send a SIGTERM to the cgi.
// This signal handler ensures that rather than dying silently, the cgi process
// will leave behind some record in the log file.
//
void sigterm_handler(int signo) {
   log_messages.printf(SCHED_MSG_LOG::CRITICAL, 
       "Caught signal %d [scheduler ran %d seconds].  Exit(1)ing\n",
       signo, elapsed_time()
    );
    fflush(NULL);
    exit(1);
    return;
}

void log_request_info(int& length) {
    char *cl=getenv("CONTENT_LENGTH");
    char *ri=getenv("REMOTE_ADDR");
    char *rm=getenv("REQUEST_METHOD");
    char *ct=getenv("CONTENT_TYPE");
    char *ha=getenv("HTTP_ACCEPT");
    char *hu=getenv("HTTP_USER_AGENT");

    log_messages.printf(SCHED_MSG_LOG::DEBUG,
        "REQUEST_METHOD=%s "
        "CONTENT_TYPE=%s "
        "HTTP_ACCEPT=%s "
        "HTTP_USER_AGENT=%s\n",
        rm?rm:"" , ct?ct:"", ha?ha:"", hu?hu:""
    );

    if (!cl) {
        log_messages.printf(SCHED_MSG_LOG::CRITICAL, "CONTENT_LENGTH environment variable not set\n");
    }
    else {
        length=atoi(cl);
        log_messages.printf(SCHED_MSG_LOG::DEBUG, "CONTENT_LENGTH=%d from %s\n", length, ri?ri:"[Unknown]");
    }
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
    int length=-1;
    log_messages.pid = getpid();

    // initialized timer
    elapsed_time();

    // install a signal handler that catches SIGTERMS sent by Apache if the cgi
    // times out.
    //
    signal(SIGTERM, sigterm_handler);

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

#if DUMP_CORE_ON_SEGV
    {
        struct rlimit limit;
        if (getrlimit(RLIMIT_CORE, &limit)) {
            log_messages.printf(SCHED_MSG_LOG::CRITICAL,
                "Unable to read resource limit for core dump size.\n"
            );
        }
        else {
            char short_string[256], *short_message=short_string;

            short_message += sprintf(short_message,"Default resource limit for core dump size curr=");
            if (limit.rlim_cur == RLIM_INFINITY)
                short_message += sprintf(short_message,"Inf max=");
            else
                short_message += sprintf(short_message,"%d max=", (int)limit.rlim_cur);

            if (limit.rlim_max == RLIM_INFINITY)
                short_message += sprintf(short_message,"Inf\n");
            else
                short_message += sprintf(short_message,"%d\n", (int)limit.rlim_max);
          
            log_messages.printf(SCHED_MSG_LOG::DEBUG, "%s", short_string);
            
            // now set limit to the maximum allowed value
            limit.rlim_cur=limit.rlim_max;
            if (setrlimit(RLIMIT_CORE, &limit)) {
                log_messages.printf(SCHED_MSG_LOG::CRITICAL,
                    "Unable to set current resource limit for core dump size to max value.\n"
                );
            }
            else {
                log_messages.printf(SCHED_MSG_LOG::DEBUG,
                    "Set limit for core dump size to max value.\n"
                );
            }   
        }
    }
#endif

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
    log_request_info(length);
    fprintf(stdout,"Content-type: text/plain\n\n");

    if (use_files) {
        struct stat statbuf;
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
        stat(req_path, &statbuf);
        if (length>=0 && (statbuf.st_size != length)) {
            log_messages.printf(SCHED_MSG_LOG::CRITICAL,
                "Request length %d != CONTENT_LENGTH %d\n",
                (int)statbuf.st_size, length
            );
        }

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
#ifdef EINSTEIN_AT_HOME
        if (getenv("CONTENT_LENGTH")) unlink(req_path);
        if (getenv("CONTENT_LENGTH")) unlink(reply_path);
#else
        // unlink(req_path);
        // unlink(reply_path);
#endif
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
