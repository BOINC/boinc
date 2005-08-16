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

// The plumbing of GUI RPC, server side
// (but not the actual RPCs)

#ifdef _WIN32
#include "boinc_win.h"
#endif

#ifndef _WIN32
#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <vector>
#include <string.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#endif

#include "util.h"
#include "error_numbers.h"
#include "parse.h"
#include "network.h"
#include "filesys.h"

#include "file_names.h"
#include "client_msgs.h"
#include "client_state.h"

using std::string;
using std::vector;

GUI_RPC_CONN::GUI_RPC_CONN(int s) {
    sock = s;
    auth_needed = false;
}

GUI_RPC_CONN::~GUI_RPC_CONN() {
    boinc_close_socket(sock);
}

GUI_RPC_CONN_SET::GUI_RPC_CONN_SET() {
    lsock = -1;
}

int GUI_RPC_CONN_SET::get_password() {
    strcpy(password, "");
    if (boinc_file_exists(GUI_RPC_PASSWD_FILE)) {
        FILE* f = fopen(GUI_RPC_PASSWD_FILE, "r");
        if (f) {
            fgets(password, 256, f);
            strip_whitespace(password);
            fclose(f);
        }
    }
    return 0;
}

int GUI_RPC_CONN_SET::get_allowed_hosts() {
    int ipaddr, retval;
    char buf[256], msg[256];

    SCOPE_MSG_LOG scope_messages(log_messages, CLIENT_MSG_LOG::DEBUG_STATE);
 
    // open file remote_hosts.cfg and read in the
    // allowed host list and resolve them to an ip address
    //
    FILE* f = fopen(REMOTEHOST_FILE_NAME, "r");
    if (f != NULL) {    
         scope_messages.printf(
            "GUI_RPC_CONN_SET::get_allowed_hosts(): found allowed hosts list\n"
        );
 
        // read in each line, if it is not a comment
        // then resolve the address and add to our allowed list
         //
        memset(buf,0,sizeof(buf));
        while (fgets(buf, 256, f) != NULL) {
            strip_whitespace(buf);
            if (!(buf[0] =='#' || buf[0] == ';') && strlen(buf) > 0 ) {
                retval = resolve_hostname(buf, ipaddr, msg);
                if (!retval) {
                    allowed_remote_ip_addresses.push_back((int)ntohl(ipaddr));
                }
            }
        }
        fclose(f);
    }
    return 0;
}

int GUI_RPC_CONN_SET::insert(GUI_RPC_CONN* p) {
    gui_rpcs.push_back(p);
    return 0;
}

int GUI_RPC_CONN_SET::init() {
    sockaddr_in addr;
    int retval;

    get_allowed_hosts();
    get_password();

    lsock = socket(AF_INET, SOCK_STREAM, 0);
    if (lsock < 0) {
        msg_printf(NULL, MSG_ERROR,
            "GUI RPC failed to create socket: %d\n", lsock
        );
        return ERR_SOCKET;
    }

    addr.sin_family = AF_INET;
    addr.sin_port = htons(GUI_RPC_PORT);
#ifdef __APPLE__
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
#else
    if (gstate.allow_remote_gui_rpc || allowed_remote_ip_addresses.size() > 0) {
        addr.sin_addr.s_addr = htonl(INADDR_ANY);
        msg_printf(NULL, MSG_INFO, "Remote control allowed\n");
    } else {
        addr.sin_addr.s_addr = inet_addr("127.0.0.1");
        msg_printf(NULL, MSG_INFO, "Remote control not allowed; using loopback address\n");
    }
#endif

    int one = 1;
    setsockopt(lsock, SOL_SOCKET, SO_REUSEADDR, (char*)&one, 4);

    retval = bind(lsock, (const sockaddr*)(&addr), (boinc_socklen_t)sizeof(addr));
    if (retval) {
        addr.sin_port = htons(GUI_RPC_PORT_ALT);
        retval = bind(lsock, (const sockaddr*)(&addr), (boinc_socklen_t)sizeof(addr));
        if (retval) {
            msg_printf(NULL, MSG_ERROR, "GUI RPC bind failed: %d\n", retval);
            boinc_close_socket(lsock);
            lsock = -1;
            return ERR_BIND;
        }
    }
    retval = listen(lsock, 999);
    if (retval) {
        msg_printf(NULL, MSG_ERROR, "GUI RPC listen failed: %d\n", retval);
        boinc_close_socket(lsock);
        lsock = -1;
        return ERR_LISTEN;
    }
    return 0;
}

static void show_connect_error(in_addr ia) {
    static double last_time=0;
    static int count=0;

    if (last_time == 0) {
        last_time = gstate.now;
        count = 1;
    } else {
        if (gstate.now - last_time < 600) {
            count++;
            return;
        }
        last_time = gstate.now;
    }
    msg_printf(
        NULL, MSG_ERROR,
        "GUI RPC request from non-allowed address %s\n",
        inet_ntoa(ia)
    );
    if (count > 1) {
        msg_printf(
            NULL, MSG_ERROR,
            "%d connections rejected in last 10 minutes\n",
            count
        );
    }
    count = 0;
}

void GUI_RPC_CONN_SET::get_fdset(FDSET_GROUP& fg) {
    unsigned int i;
    GUI_RPC_CONN* gr;

    if (lsock < 0) return;
    for (i=0; i<gui_rpcs.size(); i++) {
        gr = gui_rpcs[i];
        FD_SET(gr->sock, &fg.read_fds);
        FD_SET(gr->sock, &fg.exc_fds);
    }
    FD_SET(lsock, &fg.read_fds);
}

void GUI_RPC_CONN_SET::got_select(FDSET_GROUP& fg) {
    int n = 0;
    unsigned int i;
    fd_set read_fds, error_fds;
    int sock, retval;
    vector<GUI_RPC_CONN*>::iterator iter;
    GUI_RPC_CONN* gr;
    bool is_local = false;

    if (lsock < 0) return;

    if (FD_ISSET(lsock, &fg.read_fds)) {
        struct sockaddr_in addr;

        boinc_socklen_t addr_len = sizeof(addr);
        sock = accept(lsock, (struct sockaddr*)&addr, &addr_len);

        int peer_ip = (int) ntohl(addr.sin_addr.s_addr);

        // check list of allowed remote hosts
        bool allowed = false;
        vector<int>::iterator remote_iter;

        remote_iter = allowed_remote_ip_addresses.begin();
        while (remote_iter != allowed_remote_ip_addresses.end() ) {
            int remote_host = *remote_iter;
            if (peer_ip == remote_host) allowed = true;
            remote_iter++;
        }
         
        // accept the connection if:
        // 1) allow_remote_gui_rpc is set or
        // 2) client host is included in "remote_hosts" file or
        // 3) client is on localhost
        //
        if (peer_ip == 0x7f000001) {
            allowed = true;
            is_local = true;
        }

        if (!(gstate.allow_remote_gui_rpc) && !(allowed)) {
            in_addr ia;
            ia.s_addr = htonl(peer_ip);
            show_connect_error(ia);
            boinc_close_socket(sock);
        } else {
            gr = new GUI_RPC_CONN(sock);
            if (strlen(password)) {
                gr->auth_needed = true;
            }
            gr->is_local = is_local;
            insert(gr);
        }
    }
    iter = gui_rpcs.begin();
    while (iter != gui_rpcs.end()) {
        gr = *iter;
        if (FD_ISSET(gr->sock, &fg.exc_fds)) {
            delete gr;
            gui_rpcs.erase(iter);
        } else {
            iter++;
        }
    }
    iter = gui_rpcs.begin();
    while (iter != gui_rpcs.end()) {
        gr = *iter;
        if (FD_ISSET(gr->sock, &fg.read_fds)) {
            retval = gr->handle_rpc();
            if (retval) {
                delete gr;
                gui_rpcs.erase(iter);
                continue;
            }
        }
        iter++;
    }
}

const char *BOINC_RCSID_88dd75dd85 = "$Id$";
