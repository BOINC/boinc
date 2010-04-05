// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2008 University of California
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

// The BOINC scheduling server.

// Note: use_files is a compile setting that records everything in files.
// Also, You can call debug_sched() for whatever situation is of
// interest to you.  It won't do anything unless you create
// (touch) the file 'debug_sched' in the project root directory.
//

#include "config.h"
#include <cassert>
#ifdef _USING_FCGI_
#include "boinc_fcgi.h"
#else
#include <cstdio>
#endif
#include <cstdlib>
#include <vector>
#include <string>
#include <cstring>

#include <unistd.h>
#include <csignal>
#include <cerrno>
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
#include "str_util.h"
#include "synch.h"
#include "svn_version.h"

#include "sched_config.h"
#include "sched_types.h"
#include "handle_request.h"
#include "sched_util.h"
#include "sched_msgs.h"
#include "sched_main.h"


// Useful for debugging, if your cgi script keeps crashing.  This
// makes it dump a core file that you can load into a debugger to see
// where the problem is.
#define DUMP_CORE_ON_SEGV 0

#define DEBUG_LEVEL  999
#define MAX_FCGI_COUNT  20

#define REQ_FILE_PREFIX "boinc_req/"
#define REPLY_FILE_PREFIX "boinc_reply/"
bool use_files = false;     // use disk files for req/reply msgs (for debugging)

GUI_URLS gui_urls;
PROJECT_FILES project_files;
key_t sema_key;
int g_pid;
static bool db_opened=false;
SCHED_SHMEM* ssp = 0;
bool batch = false;
bool mark_jobs_done = false;
bool all_apps_use_hr;

static void usage(char* p) {
    fprintf(stderr,
        "Usage: %s [OPTION]...\n\n"
        "Options:\n"
        "  --batch            stdin contains a sequence of request messages.\n"
        "                     Do them all, and ignore rpc_seqno.\n"
        "  --mark_jobs_done   When send a job, also mark it as done.\n"
        "                     (for performance testing)\n"
        "  --debug_log        Write messages to the file 'debug_log'\n"
        "  --simulator X      Start with simulated time X\n"
        "                     (only if compiled with GCL_SIMULATOR)\n"
        "  -h | --help        Show this help text\n"
        "  -v | --version     Show version information\n",
        p
    );
}

void debug_sched(const char *trigger) {
    char tmpfilename[256];
#ifndef _USING_FCGI_
    FILE *fp;
#else
    FCGI_FILE *fp;
#endif

    if (!boinc_file_exists(config.project_path("%s", trigger))) {
        return;
    }

    sprintf(tmpfilename,
        "sched_reply_%06d_%06d", g_request->hostid, g_request->rpc_seqno
    );
    // use _XXXXXX if you want random filenames rather than
    // deterministic mkstemp(tmpfilename);

#ifndef _USING_FCGI_
    fp=fopen(tmpfilename, "w");
#else
    fp=FCGI::fopen(tmpfilename,"w");
#endif

    if (!fp) {
        log_messages.printf(MSG_CRITICAL,
            "Found %s, but can't open %s\n", trigger, tmpfilename
        );
        return;
    }

    log_messages.printf(MSG_DEBUG,
        "Found %s, so writing %s\n", trigger, tmpfilename
    );

    g_reply->write(fp, *g_request);
    fclose(fp);

    sprintf(tmpfilename,
        "sched_request_%06d_%06d", g_request->hostid, g_request->rpc_seqno
    );
#ifndef _USING_FCGI_
    fp=fopen(tmpfilename, "w");
#else
    fp=FCGI::fopen(tmpfilename,"w");
#endif

    if (!fp) {
        log_messages.printf(MSG_CRITICAL,
            "Found %s, but can't open %s\n", trigger, tmpfilename
        );
        return;
    }

    log_messages.printf(MSG_DEBUG,
        "Found %s, so writing %s\n", trigger, tmpfilename
    );

    g_request->write(fp);
    fclose(fp);

    return;
}

// call this only if we're not going to call handle_request()
//
static void send_message(const char* msg, int delay) {
    fprintf(stdout,
        "Content-type: text/plain\n\n"
        "<scheduler_reply>\n"
        "    <message priority=\"low\">%s</message>\n"
        "    <request_delay>%d</request_delay>\n"
        "    <project_is_down/>\n"
        "%s</scheduler_reply>\n",
        msg, delay,
        config.ended?"    <ended>1</ended>\n":""
    );
}

int open_database() {
    int retval;

    if (db_opened) {
        retval = boinc_db.ping();
        if (retval) {
            log_messages.printf(MSG_CRITICAL,
                "lost connection to database - trying to reconnect\n"
            );
        } else {
            return 0;
        }
    }

    retval = boinc_db.open(
        config.db_name, config.db_host, config.db_user, config.db_passwd
    );
    if (retval) {
        log_messages.printf(MSG_CRITICAL, "can't open database\n");
        return retval;
    }
    db_opened = true;
    return 0;
}

// If the scheduler 'hangs' (e.g. because DB is slow),
// Apache will send it a SIGTERM.
// Record this in the log file and close the DB conn.
//
void sigterm_handler(int signo) {
    if (db_opened) {
        boinc_db.close();
    }
    log_messages.printf(MSG_CRITICAL,
        "Caught SIGTERM (sent by Apache); exiting\n"
    );
    fflush((FILE*)NULL);
    exit(1);
    return;
}

static void log_request_headers(int& length) {
    char *cl=getenv("CONTENT_LENGTH");
    char *ri=getenv("REMOTE_ADDR");
    char *rm=getenv("REQUEST_METHOD");
    char *ct=getenv("CONTENT_TYPE");
    char *ha=getenv("HTTP_ACCEPT");
    char *hu=getenv("HTTP_USER_AGENT");

    if (config.debug_request_details) {
        log_messages.printf(MSG_INFO,
            "(req details) REQUEST_METHOD=%s CONTENT_TYPE=%s HTTP_ACCEPT=%s HTTP_USER_AGENT=%s\n",
            rm?rm:"" , ct?ct:"", ha?ha:"", hu?hu:""
        );
    }

    if (!cl) {
        log_messages.printf(MSG_CRITICAL,
            "CONTENT_LENGTH environment variable not set\n"
        );
    } else {
        length=atoi(cl);
        if (config.debug_request_details) {
            log_messages.printf(MSG_INFO,
                "CONTENT_LENGTH=%d from %s\n", length, ri?ri:"[Unknown]"
            );
        }
    }
}

#if DUMP_CORE_ON_SEGV
void set_core_dump_size_limit() {
    struct rlimit limit;
    if (getrlimit(RLIMIT_CORE, &limit)) {
        log_messages.printf(MSG_CRITICAL,
            "Unable to read resource limit for core dump size.\n"
        );
    } else {
        char short_string[256], *short_message=short_string;

        short_message += sprintf(short_message,"Default resource limit for core dump size curr=");
        if (limit.rlim_cur == RLIM_INFINITY) {
            short_message += sprintf(short_message,"Inf max=");
        } else {
            short_message += sprintf(short_message,"%d max=", (int)limit.rlim_cur);
        }

        if (limit.rlim_max == RLIM_INFINITY) {
            short_message += sprintf(short_message,"Inf\n");
        } else {
            short_message += sprintf(short_message,"%d\n", (int)limit.rlim_max);
        }
      
        log_messages.printf(MSG_DEBUG, "%s", short_string);
        
        // now set limit to the maximum allowed value
        limit.rlim_cur=limit.rlim_max;
        if (setrlimit(RLIMIT_CORE, &limit)) {
            log_messages.printf(MSG_CRITICAL,
                "Unable to set current resource limit for core dump size to max value.\n"
            );
        } else {
            log_messages.printf(MSG_DEBUG,
                "Set limit for core dump size to max value.\n"
            );
        }   
    }
}
#endif

void attach_to_feeder_shmem() {
    char path[256];
    strncpy(path, config.project_dir, sizeof(path));
    get_key(path, 'a', sema_key);
    int i, retval;
    void* p;

    retval = attach_shmem(config.shmem_key, &p);
    if (retval || p==0) {
        log_messages.printf(MSG_CRITICAL,
            "Can't attach shmem: %d (feeder not running?)\n",
            retval
        );
        log_messages.printf(MSG_CRITICAL,
            "uid %d euid %d gid %d eguid%d\n",
            getuid(), geteuid(), getgid(), getegid()
        );
        send_message(
            "Server error: feeder not running", 3600
        );
        exit(0);
    } else {
        ssp = (SCHED_SHMEM*)p;
        retval = ssp->verify();
        if (retval) {
            log_messages.printf(MSG_CRITICAL,
                "shmem has wrong struct sizes - recompile\n"
            );
            send_message("Server error: recompile needed", 3600);
            exit(0);
        }

        for (i=0; i<10; i++) {
            if (ssp->ready) break;
            log_messages.printf(MSG_DEBUG,
                "waiting for ready flag\n"
            );
            sleep(1);
        }
        if (!ssp->ready) {
            log_messages.printf(MSG_CRITICAL,
                "feeder doesn't seem to be running\n"
            );
            send_message(
                "Server error: feeder not running", 3600
            );
            exit(0);
        }
    }

    all_apps_use_hr = true;
    for (int i=0; i<ssp->napps; i++) {
        if (!ssp->apps[i].homogeneous_redundancy) {
            all_apps_use_hr = false;
            break;
        }
    }
}

int main(int argc, char** argv) {
#ifndef _USING_FCGI_
    FILE* fin, *fout;
#else
    FCGI_FILE *fin, *fout;
#endif
    int i, retval;
    char req_path[256], reply_path[256], path[256];
    unsigned int counter=0;
    char* code_sign_key;
    int length=-1;
    log_messages.pid = getpid();
    bool debug_log = false;

    for (i=1; i<argc; i++) {
        if (!strcmp(argv[i], "--batch")) {
            batch = true;
            continue;
        } else if (!strcmp(argv[i], "--mark_jobs_done")) {
            mark_jobs_done = true;
        } else if (!strcmp(argv[i], "--debug_log")) {
            debug_log = true;
#ifdef GCL_SIMULATOR
        } else if (!strcmp(argv[i], "--simulator")) {
            if(!argv[++i]) {
                log_messages.printf(MSG_CRITICAL, "%s requires an argument\n\n", argv[--i]);
                usage(argv[0]);
                exit(1);
            }
            simtime = atof(argv[i]);
#endif 
        } else if(!strcmp(argv[i], "-h") || !strcmp(argv[i], "--help")) {
            usage(argv[0]);
            exit(0);
        } else if(!strcmp(argv[i], "-v") || !strcmp(argv[i], "--version")) {
            printf("%s\n", SVN_VERSION);
            exit(0);
        } else if (strlen(argv[i])){
            log_messages.printf(MSG_CRITICAL, "unknown command line argument: %s\n\n", argv[i]);
            usage(argv[0]);
            exit(1);
        }
    }

    // install a signal handler that catches SIGTERMS sent by Apache if the cgi
    // times out.
    //
    signal(SIGTERM, sigterm_handler);

    if (debug_log) {
        freopen("debug_log", "w", stderr);
    } else {
        char *stderr_buffer, buf[256];
        get_log_path(path, "scheduler.log");
#ifndef _USING_FCGI_
        if (!freopen(path, "a", stderr)) {
            fprintf(stderr, "Can't redirect stderr\n");
            sprintf(buf, "Server can't open log file (%s)", path);
            send_message(buf, 3600);
            exit(1);
        }
        // install a larger buffer for stderr.  This ensures that
        // log information from different scheduler requests running
        // in parallel don't collide in the log file and appear intermingled.
        //
        if (!(stderr_buffer=(char *)malloc(32768)) || setvbuf(stderr, stderr_buffer, _IOFBF, 32768)) {
            log_messages.printf(MSG_CRITICAL,
                "Unable to change stderr buffering preferences\n"
            );
        }
#else
        FCGI_FILE* f = FCGI::fopen(path, "a");
        if (f) {
            log_messages.redirect(f);
        } else {
            char buf[256];
            fprintf(stderr, "Can't redirect FCGI log messages\n");
            sprintf(buf, "Server can't open log file for FCGI (%s)", path);
            send_message(buf, 3600);
            exit(1);
        }
        // set buffer as above, note that f is really a struct from fcgi_stdio.h
        if (!(stderr_buffer=(char *)malloc(32768)) || setvbuf(f->stdio_stream, stderr_buffer, _IOFBF, 32768)) {
            log_messages.printf(MSG_CRITICAL,
                "Unable to change stderr FCGI buffering preferences\n"
            );
        }
#endif
    }

    srand(time(0)+getpid());
    log_messages.set_debug_level(DEBUG_LEVEL);

#if DUMP_CORE_ON_SEGV
    set_core_dump_size_limit();
#endif

    retval = config.parse_file();
    if (retval) {
        log_messages.printf(MSG_CRITICAL,
            "Can't parse config.xml: %s\n", boincerror(retval)
        );
        send_message("Server can't parse configuration file", 3600);
        exit(0);
    }

    log_messages.set_debug_level(config.sched_debug_level);
    if (config.sched_debug_level == 4) g_print_queries = true;

    gui_urls.init();
    project_files.init();

    sprintf(path, "%s/code_sign_public", config.key_dir);
    retval = read_file_malloc(path, code_sign_key);
    if (retval) {
        log_messages.printf(MSG_CRITICAL,
            "Can't read code sign key file (%s)\n", path
        );
        send_message("Server can't find key file", 3600);
        exit(0);
    }


    g_pid = getpid();
#ifdef _USING_FCGI_
    //while(FCGI_Accept() >= 0 && counter < MAX_FCGI_COUNT) {
    while(FCGI_Accept() >= 0) {
        counter++;
        log_messages.set_indent_level(0);
#endif
    if (config.debug_request_headers) {
        log_request_headers(length);
    }

    if (check_stop_sched()) {
        send_message("Project is temporarily shut down for maintenance", 3600);
        goto done;
    }

    if (!ssp) {
        attach_to_feeder_shmem();
    }
    if (!ssp) {
        send_message("Server error: can't attach shared memory", 3600);
        goto done;
    }

    if (use_files) {
        struct stat statbuf;
        // the code below is convoluted because,
        // instead of going from stdin to stdout directly,
        // we go via a pair of disk files
        // (this makes it easy to save the input,
        // and to know the length of the output).
        // NOTE: to use this, you must create group-writeable dirs
        // boinc_req and boinc_reply in the project dir
        //
        sprintf(req_path, "%s%d_%u", config.project_path(REQ_FILE_PREFIX), g_pid, counter);
        sprintf(reply_path, "%s%d_%u", config.project_path(REPLY_FILE_PREFIX), g_pid, counter);
#ifndef _USING_FCGI_
        fout = fopen(req_path, "w");
#else
        fout = FCGI::fopen(req_path,"w");
#endif
        if (!fout) {
            log_messages.printf(MSG_CRITICAL,
                "can't write request file\n"
            );
            exit(1);
        }
        copy_stream(stdin, fout);
        fclose(fout);
        stat(req_path, &statbuf);
        if (length>=0 && (statbuf.st_size != length)) {
            log_messages.printf(MSG_CRITICAL,
                "Request length %d != CONTENT_LENGTH %d\n",
                (int)statbuf.st_size, length
            );
        }

#ifndef _USING_FCGI_
        fin = fopen(req_path, "r");
#else
        fin = FCGI::fopen(req_path,"r");
#endif
        if (!fin) {
            log_messages.printf(MSG_CRITICAL,
                "can't read request file\n"
            );
            exit(1);
        }
#ifndef _USING_FCGI_
        fout = fopen(reply_path, "w");
#else
        fout = FCGI::fopen(reply_path, "w");
#endif
        if (!fout) {
            log_messages.printf(MSG_CRITICAL,
                "can't write reply file\n"
            );
            exit(1);
        }

        handle_request(fin, fout, code_sign_key);
        fclose(fin);
        fclose(fout);
#ifndef _USING_FCGI_
        fin = fopen(reply_path, "r");
#else
        fin = FCGI::fopen(reply_path, "r");
#endif
        if (!fin) {
            log_messages.printf(MSG_CRITICAL,
                "can't read reply file\n"
            );
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
#ifndef _USING_FCGI_
    } else if (batch) {
        while (!feof(stdin)) {
            handle_request(stdin, stdout, code_sign_key);
            fflush(stdout);
        }
#endif
    } else {
        handle_request(stdin, stdout, code_sign_key);
    }
done:
#ifdef _USING_FCGI_
        log_messages.printf(MSG_DEBUG,
            "FCGI: counter: %d\n", counter
        );
        log_messages.flush();
    }   // do()
    if (counter == MAX_FCGI_COUNT) {
        fprintf(stderr, "FCGI: counter passed MAX_FCGI_COUNT - exiting..\n");
    } else {
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
