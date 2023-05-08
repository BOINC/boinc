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

// boinc_log: show stream of event-log messages from a client
//
// usage: boinc_log [--host hostname] [--passwd passwd]

#ifdef _WIN32
#include "boinc_win.h"
#include "win_util.h"
#else
#include "config.h"
#include <cstdio>
#include <cstring>
#include <unistd.h>
#endif

#include <vector>
#include <string>
using std::vector;
using std::string;

#include "gui_rpc_client.h"
#include "error_numbers.h"
#include "util.h"
#include "str_util.h"
#include "str_replace.h"
#include "url.h"
#include "version.h"
#include "common_defs.h"


#define ARGX2(s1,s2) (!strcmp(argv[i], s1)||!strcmp(argv[i], s2))
#define ARG(S) ARGX2("-"#S, "--"#S)


// Global variables
char  g_log_filename[256];
int   g_message_sequence;

void version(){
    printf("boinclog,  built from %s \n", PACKAGE_STRING );
    exit(0);
}

void usage() {
    fprintf(stderr, "\n\
usage: boinclog [--host hostname] [--passwd passwd] [commands]\n\n\
Commands:\n\
 --version, -V                      show version of the logging tool\n\
 --datadir <directory>              where the data directory is located\n\
"
);
    exit(1);
}

void show_error(int retval) {
    fprintf(stderr, "Error %d: %s\n", retval, boincerror(retval));
}

const char* prio_name(int prio) {
    switch (prio) {
    case MSG_INFO: return "low";
    case MSG_USER_ALERT: return "user notification";
    case MSG_INTERNAL_ERROR: return "internal error";
    }
    return "unknown";
}


void update_display() {
    system("cls");
    printf("BOINC Log Conversion Client %s\n", PACKAGE_VERSION);
    printf("Log file: %s\n", g_log_filename);
    printf("%d message(s) processed.\n\n", g_message_sequence);
    printf("Press CTRL-C to exit application.\n");
}


int main(int argc, char** argv) {
    unsigned int i;
    int retval, port=0;
    RPC_CLIENT rpc;
    MESSAGES msgs;
    char buf[256];
    char datadir[256];
    char hostname_buf[256], passwd_buf[256];
    char *hostname = 0, *passwd = passwd_buf, *p;
    struct tm* ptm;
    time_t timestamp;
    FILE* f = NULL;
    std::string msg_datetime;
    std::string msg_project;
    std::string msg_priority;
    std::string msg_type;
    std::string msg_body;
    std::string msg_tmp;

    strlcpy(buf, "", sizeof(buf));
    strlcpy(datadir, "", sizeof(datadir));
    strlcpy(hostname_buf, "", sizeof(hostname_buf));
    strlcpy(passwd_buf, "", sizeof(passwd_buf));
    strlcpy(g_log_filename, "", sizeof(g_log_filename));
    g_message_sequence = 0;

#if defined(_WIN32) && defined(USE_WINSOCK)
    WSADATA wsdata;
    retval = WSAStartup( MAKEWORD( 1, 1 ), &wsdata);
    if (retval) {
        fprintf(stderr, "WinsockInitialize: %d\n", retval);
        exit(1);
    }
#endif

    for (i=1; i<(unsigned int)argc; i++) {
        if (0) {
        } else if (ARG(h)) {
            usage();
        } else if (ARG(help)) {
            usage();
        } else if (ARG(V)) {
            version();
        } else if (ARG(version)) {
            version();
        } else if (ARG(host)) {
            if ((i+1) == (unsigned int)argc) usage();
            hostname = hostname_buf;
            safe_strcpy(hostname_buf, argv[++i]);
            p = strchr(hostname, ':');
            if (p) {
                port = atoi(p+1);
                *p=0;
            }
        } else if (ARG(passwd)) {
            if ((i+1) == (unsigned int)argc) usage();
            safe_strcpy(passwd_buf, argv[++i]);
        } else if (ARG(datadir)) {
            if ((i+1) == (unsigned int)argc) usage();
            safe_strcpy(datadir, argv[++i]);
        } else {
            printf("Unknown option: %s\n", argv[i]);
            usage();
        }
    }

    if (strlen(datadir)) {
        chdir(datadir);
    } else {
#ifdef _WIN32
        chdir_to_data_dir();
#endif
    }

    std::string msg;
    read_gui_rpc_password(passwd_buf, msg);

    retval = rpc.init(hostname, port);
    if (retval) {
        fprintf(stderr, "can't connect to %s\n", hostname?hostname:"local host");
        show_error(retval);
        exit(1);
    }

    if (passwd) {
        retval = rpc.authorize(passwd);
        if (retval) {
            fprintf(stderr, "authorization failure: %d\n", retval);
            show_error(retval);
            exit(1);
        }
    }


    // Construct a unique filename for the output.
    time(&timestamp);
    ptm = localtime(&timestamp);
    strftime(g_log_filename, sizeof(g_log_filename), "%Y%m%d%H%M.log", ptm);

    // Open the new log file for output
    f = fopen(g_log_filename, "w");
    setbuf(f, NULL);

    while(true) {
        update_display();

        msgs.clear();

        rpc.get_messages(g_message_sequence, msgs);

        for (i=0; i<msgs.messages.size(); i++) {
            MESSAGE* pMsg = msgs.messages[i];

            msg_datetime.clear();
            msg_project.clear();
            msg_priority.clear();
            msg_type.clear();
            msg_body.clear();

            msg_datetime = time_to_string(double(pMsg->timestamp));
            msg_project = pMsg->project;
            msg_priority = prio_name(pMsg->priority);
            msg_body = pMsg->body;
            if (pMsg->body[0] == '[') {
                msg_type = pMsg->body.substr(1, pMsg->body.find(']') - 1);
            }

            // If a message type is found in the message body, remove it from
            // the message body
            if (!msg_type.empty()) {
                msg_tmp = std::string("[") + msg_type + std::string("] ");
                msg_body.replace(0, msg_tmp.size(), "");
            }

            // If the last character if the message body is a newline character,
            // remove it before continueing on.  Standard BOINC messages contain
            // a newline character at the end.
            if (msg_body[msg_body.size() - 1] == '\n') {
                msg_body[msg_body.size() - 1] = ' ';
            }

            // If line feeds are detected in the message body, replace them with
            // the pipe symbol.
            for (unsigned int j = 0; j < msg_body.size(); j++) {
                if (msg_body[j] == '\n') {
                    msg_body[j] = '^';
                }
            }

            // Dump to tab delimited file
            fprintf(f,
                "%s\t%s\t%s\t%s\t%s\n",
                msg_datetime.c_str(),
                msg_priority.c_str(),
                msg_project.c_str(),
                msg_type.c_str(),
                msg_body.c_str()
            );

            g_message_sequence = pMsg->seqno;
        }

        boinc_sleep(1.0);
    }

#if defined(_WIN32) && defined(USE_WINSOCK)
    WSACleanup();
#endif
    exit(retval);
}
