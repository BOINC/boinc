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

// gui_test: test program for BOINC GUI RPCs.
//
// usage: gUi_test [-host hostname] command
//
// commands:
// -state       show state
// -msgs        show messages
// -show_graphics_window result_name    show graphics for result in a window
// -show_graphics_window                show graphics for all results
// -show_graphics_full result_name      show full-screen graphics for result


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

int main(int argc, char** argv) {
    RPC_CLIENT rpc;
    unsigned int i;
    vector<MESSAGE_DESC> message_descs;
    int retval;
    char* hostname=0;

#ifdef _WIN32
    WSADATA wsdata;
    retval = WSAStartup( MAKEWORD( 1, 1 ), &wsdata);
    if (retval) {
        fprintf(stderr, "WinsockInitialize: %d\n", retval);
        exit(1);
    }
#endif
    if (argc < 2) {
        fprintf(stderr, "usage\n");
        exit(1);
    }
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

    if (!strcmp(argv[i], "-state")) {
        CC_STATE state;
        retval = rpc.get_state(state);
        if (!retval) state.print();
    } else if (!strcmp(argv[i], "-msgs")) {
        retval = rpc.get_messages(20, 0, message_descs);
        if (!retval) {
            for (i=0; i<message_descs.size(); i++) {
                MESSAGE_DESC& md = message_descs[i];
                printf("%s %d %d %s\n",
                    md.project.c_str(), md.priority,
                    md.timestamp, md.body.c_str()
                );
            }
        }
    } else if (!strcmp(argv[i], "-suspend")) {
        retval = rpc.set_run_mode(RUN_MODE_NEVER);
    } else if (!strcmp(argv[i], "-resume")) {
        retval = rpc.set_run_mode(RUN_MODE_ALWAYS);
    } else if (!strcmp(argv[i], "-show_graphics_window")) {
        if (i = argc-1) {
            retval = rpc.show_graphics(0, false);
        } else {
            retval = rpc.show_graphics(argv[++i], false);
        }
    } else if (!strcmp(argv[i], "-show_graphics_full")) {
        retval = rpc.show_graphics(argv[++i], true);
    }
    if (retval) {
        fprintf(stderr, "Operation failed: %d\n", retval);
    }

#ifdef _WIN32
    WSACleanup();
#endif
    return 0;
}
