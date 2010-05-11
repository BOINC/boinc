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

// The plumbing of GUI RPC, server side
// (but not the actual RPCs)

#include "cpp.h"

#ifdef _WIN32
#include "boinc_win.h"
#else
#include "config.h"
#include <cstdio>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <cerrno>
#ifdef HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif
#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif
#ifdef HAVE_SYS_UN_H
#include <sys/un.h>
#endif
#include <vector>
#include <cstring>
#ifdef HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif
#ifdef HAVE_NETINET_TCP_H
#include <netinet/tcp.h>
#endif
#ifdef HAVE_ARPA_INET_H
#include <arpa/inet.h>
#endif
#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif
#endif

#include "str_util.h"
#include "util.h"
#include "error_numbers.h"
#include "network.h"
#include "filesys.h"
#include "md5_file.h"

#include "file_names.h"
#include "client_msgs.h"
#include "client_state.h"
#include "sandbox.h"

using std::string;
using std::vector;

GUI_RPC_CONN::GUI_RPC_CONN(int s):
    get_project_config_op(&gui_http),
    lookup_account_op(&gui_http),
    create_account_op(&gui_http)
{
    sock = s;
    auth_needed = false;
    au_ss_state = AU_SS_INIT;
    au_mgr_state = AU_MGR_INIT;
    got_auth1 = false;
    got_auth2 = false;
    sent_unauthorized = false;
    notice_refresh = false;
    request_nbytes = 0;
}

GUI_RPC_CONN::~GUI_RPC_CONN() {
    boinc_close_socket(sock);
}

GUI_RPC_CONN_SET::GUI_RPC_CONN_SET() {
    lsock = -1;
    time_of_last_rpc_needing_network = 0;
}

bool GUI_RPC_CONN_SET::poll() {
    unsigned int i;
    bool action = false;
    for (i=0; i<gui_rpcs.size(); i++) {
        action |= gui_rpcs[i]->gui_http.poll();
    }
    return action;
}

bool GUI_RPC_CONN_SET::recent_rpc_needs_network(double interval) {
    if (!time_of_last_rpc_needing_network) return false;
    if (gstate.now < time_of_last_rpc_needing_network + interval) return true;
    return false;
}

int GUI_RPC_CONN_SET::get_password() {
    FILE* f;
    int retval;

    strcpy(password, "");
    if (boinc_file_exists(GUI_RPC_PASSWD_FILE)) {
        f = fopen(GUI_RPC_PASSWD_FILE, "r");
        if (f) {
            if (fgets(password, 256, f)) {
                strip_whitespace(password);
            }
            fclose(f);
        }
    } else {
        // if no password file, make a random password
        //
        retval = make_random_string(password);
        if (retval) {
            if (config.os_random_only) {
                msg_printf(
                    NULL, MSG_INTERNAL_ERROR,
                    "OS random string generation failed, exiting"
                );
                exit(1);
            }
            gstate.host_info.make_random_string("guirpc", password);
        }
        f = fopen(GUI_RPC_PASSWD_FILE, "w");
        if (f) {
            fputs(password, f);
            fclose(f);
#ifndef _WIN32
            // if someone can read the password,
            // they can cause code to execute as this user.
            // So better protect it.
            //
            if (g_use_sandbox) {
                // Allow group access so authorized administrator can modify it
                chmod(GUI_RPC_PASSWD_FILE, S_IRUSR|S_IWUSR | S_IRGRP | S_IWGRP);
            } else {
                chmod(GUI_RPC_PASSWD_FILE, S_IRUSR|S_IWUSR);
            }
#endif
        }
    }
    return 0;
}

int GUI_RPC_CONN_SET::get_allowed_hosts() {
    int ipaddr, retval;
    char buf[256];

    allowed_remote_ip_addresses.clear();
    remote_hosts_file_exists = false;

    // scan remote_hosts.cfg, convert names to IP addresses
    //
    FILE* f = fopen(REMOTEHOST_FILE_NAME, "r");
    if (f) {
        remote_hosts_file_exists = true;
        if (log_flags.gui_rpc_debug) {
            msg_printf(0, MSG_INFO,
                "[gui_rpc] found allowed hosts list"
            );
        }
 
        // read in each line, if it is not a comment
        // then resolve the address and add to our allowed list
        //
        memset(buf,0,sizeof(buf));
        while (fgets(buf, 256, f)) {
            strip_whitespace(buf);
            if (!(buf[0] =='#' || buf[0] == ';') && strlen(buf) > 0 ) {
                retval = resolve_hostname(buf, ipaddr);
                if (retval) {
                    msg_printf(0, MSG_USER_ALERT,
                        "Can't resolve hostname %s in %s",
                        buf, REMOTEHOST_FILE_NAME
                    );
                } else {
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

// If the core client runs at boot time,
// it may be a while (~10 sec) before the DNS system is working.
// If this returns an error, it will get called once a second
// for up to 30 seconds.
// On the last call, "last_time" is set; print error messages then.
//
int GUI_RPC_CONN_SET::init(bool last_time) {
    sockaddr_in addr;
    int retval;

    get_password();
    get_allowed_hosts();

    retval = boinc_socket(lsock);
    if (retval) {
        if (last_time) {
            msg_printf(NULL, MSG_INTERNAL_ERROR,
                "GUI RPC failed to create socket: %d", lsock
            );
        }
        return retval;
    }

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;

    if (gstate.cmdline_gui_rpc_port) {
        addr.sin_port = htons(gstate.cmdline_gui_rpc_port);
    } else {
        addr.sin_port = htons(GUI_RPC_PORT);
    }

#ifdef __APPLE__
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
#else
    if (config.allow_remote_gui_rpc || remote_hosts_file_exists) {
        addr.sin_addr.s_addr = htonl(INADDR_ANY);
        if (log_flags.gui_rpc_debug) {
            msg_printf(NULL, MSG_INFO, "[gui_rpc] Remote control allowed");
        }
    } else {
        addr.sin_addr.s_addr = inet_addr("127.0.0.1");
        if (log_flags.gui_rpc_debug) {
            msg_printf(NULL, MSG_INFO, "[gui_rpc] Local control only allowed");
        }
    }
#endif

    int one = 1;
    setsockopt(lsock, SOL_SOCKET, SO_REUSEADDR, (char*)&one, 4);

    retval = bind(lsock, (const sockaddr*)(&addr), (BOINC_SOCKLEN_T)sizeof(addr));
    if (retval) {
#ifndef _WIN32
        retval = errno;     // Display the real error code
#endif
        if (last_time) {
            msg_printf(NULL, MSG_INTERNAL_ERROR,
                "GUI RPC bind to port %d failed: %d", htons(addr.sin_port), retval
            );
        }
        boinc_close_socket(lsock);
        lsock = -1;
        return ERR_BIND;
    }
    if (log_flags.gui_rpc_debug) {
        msg_printf(NULL, MSG_INFO, "[gui_rpc] Listening on port %d", htons(addr.sin_port));
    }

    retval = listen(lsock, 999);
    if (retval) {
        if (last_time) {
            msg_printf(NULL, MSG_INTERNAL_ERROR,
                "GUI RPC listen failed: %d", retval
            );
        }
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
        if (gstate.now - last_time < CONNECT_ERROR_PERIOD) {
            count++;
            return;
        }
        last_time = gstate.now;
    }
    msg_printf(
        NULL, MSG_USER_ALERT,
        "GUI RPC request from non-allowed address %s",
        inet_ntoa(ia)
    );
    if (count > 1) {
        msg_printf(
            NULL, MSG_USER_ALERT,
            "%d connections rejected in last 10 minutes",
            count
        );
    }
    count = 0;
}

void GUI_RPC_CONN_SET::get_fdset(FDSET_GROUP& fg, FDSET_GROUP& all) {
    unsigned int i;
    GUI_RPC_CONN* gr;

    if (lsock < 0) return;
    for (i=0; i<gui_rpcs.size(); i++) {
        gr = gui_rpcs[i];
        int s = gr->sock;
        FD_SET(s, &fg.read_fds);
        FD_SET(s, &fg.exc_fds);
        if (s > fg.max_fd) fg.max_fd = s;

        FD_SET(s, &all.read_fds);
        FD_SET(s, &all.exc_fds);
        if (s > all.max_fd) all.max_fd = s;
    }
    FD_SET(lsock, &fg.read_fds);
    if (lsock > fg.max_fd) fg.max_fd = lsock;
    FD_SET(lsock, &all.read_fds);
    if (lsock > all.max_fd) all.max_fd = lsock;
}

bool GUI_RPC_CONN_SET::check_allowed_list(int peer_ip) {
    vector<int>::iterator remote_iter = allowed_remote_ip_addresses.begin();
    while (remote_iter != allowed_remote_ip_addresses.end() ) {
        int remote_host = *remote_iter;
        if (peer_ip == remote_host) {
            return true;
        }
        remote_iter++;
    }
    return false;
}

void GUI_RPC_CONN_SET::got_select(FDSET_GROUP& fg) {
    int sock, retval;
    vector<GUI_RPC_CONN*>::iterator iter;
    GUI_RPC_CONN* gr;
    bool is_local = false;

    if (lsock < 0) return;

    if (FD_ISSET(lsock, &fg.read_fds)) {
        struct sockaddr_in addr;

        // For unknown reasons, the FD_ISSET() above succeeds
        // after a SIGTERM, SIGHUP, SIGINT or SIGQUIT is received,
        // even if there is no data available on the socket.
        // This causes the accept() call to block, preventing the main 
        // loop from processing the exit request.
        // This is a workaround for that problem.
        //
        if (gstate.requested_exit) {
            return;
        }

        BOINC_SOCKLEN_T addr_len = sizeof(addr);
        sock = accept(lsock, (struct sockaddr*)&addr, (BOINC_SOCKLEN_T*)&addr_len);
        if (sock == -1) {
            return;
        }

        // apps shouldn't inherit the socket!
        //
#ifndef _WIN32
        fcntl(sock, F_SETFD, FD_CLOEXEC);
#endif

        int peer_ip = (int) ntohl(addr.sin_addr.s_addr);
        bool allowed;
         
        // accept the connection if:
        // 1) allow_remote_gui_rpc is set or
        // 2) client host is included in "remote_hosts" file or
        // 3) client is on localhost
        //
        if (peer_ip == 0x7f000001) {
            allowed = true;
            is_local = true;
        } else {
            // reread host file because IP addresses might have changed
            //
            get_allowed_hosts();
            allowed = check_allowed_list(peer_ip);
        }

        if (!(config.allow_remote_gui_rpc) && !(allowed)) {
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
            if (log_flags.gui_rpc_debug) {
                msg_printf(0, MSG_INFO,
                    "[gui_rpc] got new GUI RPC connection"
                );
            }
            insert(gr);
        }
    }
    iter = gui_rpcs.begin();
    while (iter != gui_rpcs.end()) {
        gr = *iter;
        if (FD_ISSET(gr->sock, &fg.exc_fds)) {
            delete gr;
            iter = gui_rpcs.erase(iter);
            continue;
        }
        iter++;
    }
    iter = gui_rpcs.begin();
    while (iter != gui_rpcs.end()) {
        gr = *iter;
        if (FD_ISSET(gr->sock, &fg.read_fds)) {
            retval = gr->handle_rpc();
            if (retval) {
                if (log_flags.gui_rpc_debug) {
                    msg_printf(NULL, MSG_INFO,
                        "[gui_rpc] error %d from handler, closing socket\n",
                        retval
                    );
                }
                delete gr;
                iter = gui_rpcs.erase(iter);
                continue;
            }
        }
        iter++;
    }
}

void GUI_RPC_CONN_SET::close() {
    if (log_flags.gui_rpc_debug) {
        msg_printf(NULL, MSG_INFO,
            "[gui_rpc] closing GUI RPC listening socket %d\n", lsock
        );
    }
    if (lsock >= 0) {
        boinc_close_socket(lsock);
        lsock = -1;
    }
}

// this is called when we're ready to auto-update;
// set flags to send quit messages to screensaver and local manager
//
void GUI_RPC_CONN_SET::send_quits() {
    for (unsigned int i=0; i<gui_rpcs.size(); i++) {
        GUI_RPC_CONN* gr = gui_rpcs[i];
        if (gr->au_ss_state == AU_SS_GOT) {
            gr->au_ss_state = AU_SS_QUIT_REQ;
        }
        if (gr->au_mgr_state == AU_MGR_GOT && gr->is_local) {
            gr->au_mgr_state = AU_MGR_QUIT_REQ;
        }
    }
}

// check whether the quit messages have actually been sent
//
bool GUI_RPC_CONN_SET::quits_sent() {
    for (unsigned int i=0; i<gui_rpcs.size(); i++) {
        GUI_RPC_CONN* gr = gui_rpcs[i];
        if (gr->au_ss_state == AU_SS_QUIT_REQ) return false;
        if (gr->au_mgr_state == AU_MGR_QUIT_REQ) return false;
    }
    return true;
}

