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

// gui_test: test program for BOINC GUI RPCs.
//
// usage: gui_test [-host hostname] command
//
// commands:
// -get_state               show state
// -get_results             show active results
// -get_file_transfers
// -get_messages    nmsgs seqno
// -set_run_mode {always | auto | never}
// -show_graphics {window | fullscreen} url result_name
// -project_reset url
// -project_attach url auth
// -project_detach url
// -project_update url
// -project_nomorework url
// -project_allowmorework url
// -run_benchmarks
// -set_proxy_settings


#ifdef _WIN32
#include "boinc_win.h"
#include "win_net.h"
#endif
#include <vector>
using std::vector;

#ifndef _WIN32
#include <cstdio>
#endif

#include "gui_rpc_client.h"
#include "error_numbers.h"

void usage() {
    fprintf(stderr, "bad usage\n");
    exit(1);
}

int main(int argc, char** argv) {
    RPC_CLIENT rpc;
    unsigned int i;
    MESSAGES messages;
    int retval;
    char* hostname=0;
    PROJECT project;

#ifdef _WIN32
    WSADATA wsdata;
    retval = WSAStartup( MAKEWORD( 1, 1 ), &wsdata);
    if (retval) {
        fprintf(stderr, "WinsockInitialize: %d\n", retval);
        exit(1);
    }
#endif
    if (argc < 2) usage();
    i = 1;
    if (!strcmp(argv[1], "-host")) {
        hostname = argv[2];
        i = 3;
    }
    retval = rpc.init(hostname);
    if (retval) {
        fprintf(stderr, "can't connect\n");
        exit(1);
    }

    if (!strcmp(argv[i], "-get_state")) {
        CC_STATE state;
        retval = rpc.get_state(state);
        if (!retval) state.print();
    } else if (!strcmp(argv[i], "-get_results")) {
        RESULTS results;
        retval = rpc.get_results(results);
    } else if (!strcmp(argv[i], "-get_file_transfers")) {
        FILE_TRANSFERS ft;
        retval = rpc.get_file_transfers(ft);
    } else if (!strcmp(argv[i], "-get_messages")) {
        if (i != argc-2) usage();
        int seqno = atoi(argv[++i]);
        retval = rpc.get_messages(seqno, messages);
        if (!retval) {
            for (i=0; i<messages.messages.size(); i++) {
                MESSAGE& md = *messages.messages[i];
                printf("%s %d %d %s\n",
                    md.project.c_str(), md.priority,
                    md.timestamp, md.body.c_str()
                );
            }
        }
    } else if (!strcmp(argv[i], "-set_run_mode")) {
        if (i != argc-2) usage();
        i++;
        if (!strcmp(argv[i], "always")) {
            retval = rpc.set_run_mode(RUN_MODE_ALWAYS);
        } else if (!strcmp(argv[i], "auto")) {
            retval = rpc.set_run_mode(RUN_MODE_AUTO);
        } else if (!strcmp(argv[i], "never")) {
            retval = rpc.set_run_mode(RUN_MODE_NEVER);
        }
    } else if (!strcmp(argv[i], "-show_graphics")) {
        if (argc != 5) {
            printf("usage: gui_test -show_graphics window proj result\n");
            exit(1);
        }
        bool fullscreen = !strcmp(argv[++i], "fullscreen");
        char* url = argv[++i];
        char* result = argv[++i];
        retval = rpc.show_graphics(url, result, fullscreen, "winsta0", "default");
    } else if (!strcmp(argv[i], "-project_reset")) {
        project.master_url = argv[++i];
        retval = rpc.project_op(project, "reset");
    } else if (!strcmp(argv[i], "-project_attach")) {
        retval = rpc.project_attach(argv[++i], argv[++i]);
    } else if (!strcmp(argv[i], "-project_detach")) {
        project.master_url = argv[++i];
        retval = rpc.project_op(project, "detach");
    } else if (!strcmp(argv[i], "-project_update")) {
        project.master_url = argv[++i];
        retval = rpc.project_op(project, "update");
    } else if (!strcmp(argv[i], "-project_nomorework")) {
        project.master_url = argv[++i];
        retval = rpc.project_op(project, "nomorework");
     } else if (!strcmp(argv[i], "-project_allowmorework")) {
        project.master_url = argv[++i];
        retval = rpc.project_op(project, "allowmorework");
    } else if (!strcmp(argv[i], "-run_benchmarks")) {
        retval = rpc.run_benchmarks();
    }
    if (retval) {
        fprintf(stderr, "Operation failed: %d\n", retval);
    }

#ifdef _WIN32
    WSACleanup();
#endif
    return 0;
}

const char *BOINC_RCSID_77f00010ab = "$Id$";
