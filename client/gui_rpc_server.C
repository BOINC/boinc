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

#include <stdio.h>
#ifndef _WIN32
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <vector>
#endif
#include <string.h>

#include "parse.h"
#include "client_state.h"

GUI_RPC_CONN::GUI_RPC_CONN(int s) {
    sock = s;
#ifndef _WIN32
    fout = fdopen(dup(sock), "w");
#endif
}

GUI_RPC_CONN::~GUI_RPC_CONN() {
#ifndef _WIN32
    close(sock);
    fclose(fout);
#endif
}

int GUI_RPC_CONN::handle_rpc() {
#ifndef _WIN32
    char buf[256];
    int n;
    unsigned int i;

	// read the request message in one read()
	// so that the core client won't hang because
	// of malformed request msgs
	//
    n = read(sock, buf, 256);
    if (n <= 0) return -1;
    buf[n] = 0;
    printf("got %s\n", buf);
    if (match_tag(buf, "<get_state")) {
        gstate.write_state(fout);
	} else if (match_tag(buf, "<result_show_graphics>")) {
	} else if (match_tag(buf, "<project_reset>")) {
	} else if (match_tag(buf, "<project_attach>")) {
	} else if (match_tag(buf, "<project_detach>")) {
	} else if (match_tag(buf, "<project_update>")) {
	} else if (match_tag(buf, "<set_run_mode>")) {
	} else if (match_tag(buf, "<run_benchmarks>")) {
	} else if (match_tag(buf, "<set_proxy_settings>")) {
    } else {
        fprintf(fout, "<unrecognized/>\n");
    }
    fflush(fout);
#endif
   return 0;
}

int GUI_RPC_CONN_SET::insert(GUI_RPC_CONN* p) {
    gui_rpcs.push_back(p);
    return 0;
}

void GUI_RPC_CONN_SET::init(char* path) {
#ifndef _WIN32
    sockaddr_un addr;
    int retval;

    unlink(path);
    addr.sun_family = AF_UNIX;
    strcpy(addr.sun_path, path);
    lsock = socket(AF_UNIX, SOCK_STREAM, 0);
    if (lsock < 0) {
        perror("socket");
        exit(1);
    }
    retval = bind(lsock, (const sockaddr*)(&addr), sizeof(addr));
    if (retval) {
        perror("bind");
        exit(1);
    }
    retval = listen(lsock, 999);
    if (retval) {
        perror("listen");
        exit(1);
    }
#endif
}

bool GUI_RPC_CONN_SET::poll() {
#ifdef _WIN32
    return false;
#else
    unsigned int i;
    fd_set read_fds, error_fds;
    int sock, n, retval;
    vector<GUI_RPC_CONN*>::iterator iter;
    GUI_RPC_CONN* gr;
    struct timeval tv;

    FD_ZERO(&read_fds);
    FD_ZERO(&error_fds);
    FD_SET(lsock, &read_fds);
    for (i=0; i<gui_rpcs.size(); i++) {
        gr = gui_rpcs[i];
        FD_SET(gr->sock, &read_fds);
        FD_SET(gr->sock, &error_fds);
    }

    memset(&tv, 0, sizeof(tv));
    n = select(FD_SETSIZE, &read_fds, 0, &error_fds, &tv);
    if (FD_ISSET(lsock, &read_fds)) {
        sock = accept(lsock, 0, 0);
        GUI_RPC_CONN* gr = new GUI_RPC_CONN(sock);
        insert(gr);
    }
    iter = gui_rpcs.begin();
    while (iter != gui_rpcs.end()) {
        gr = *iter;
        if (FD_ISSET(gr->sock, &error_fds)) {
            delete gr;
            gui_rpcs.erase(iter);
        } else {
            iter++;
        }
    }
    iter = gui_rpcs.begin();
    while (iter != gui_rpcs.end()) {
        gr = *iter;
        if (FD_ISSET(gr->sock, &read_fds)) {
            retval = gr->handle_rpc();
            if (retval) {
                delete gr;
                gui_rpcs.erase(iter);
                continue;
            }
        }
        iter++;
    }
    return (n != 0);
#endif
}
