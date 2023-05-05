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
#include <cstddef>
#if HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <cerrno>
#if HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif
#if HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif
#include <sys/un.h>
#include <vector>
#include <cstring>
#if HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif
#if HAVE_NETINET_TCP_H
#include <netinet/tcp.h>
#endif
#if HAVE_ARPA_INET_H
#include <arpa/inet.h>
#endif
#if HAVE_FCNTL_H
#include <fcntl.h>
#endif
#endif

#include "error_numbers.h"
#include "filesys.h"
#include "md5_file.h"
#include "network.h"
#include "str_replace.h"
#include "str_util.h"
#include "thread.h"
#include "util.h"

#include "file_names.h"
#include "client_msgs.h"
#include "client_state.h"
#include "sandbox.h"

using std::string;
using std::vector;

GUI_RPC_CONN::GUI_RPC_CONN(int s) :
    xp(&mfin),
    get_project_config_op(&gui_http),
    lookup_account_op(&gui_http),
    create_account_op(&gui_http)
{
    sock = s;
    mfout.init_mfile(&mout);
    safe_strcpy(request_msg,"");
    request_nbytes = 0;
    safe_strcpy(nonce,"");
    auth_needed = false;
    got_auth1 = false;
    got_auth2 = false;
    sent_unauthorized = false;
    is_local = false;
    quit_flag = false;
    au_ss_state = AU_SS_INIT;
    au_mgr_state = AU_MGR_INIT;

    notice_refresh = false;
}

GUI_RPC_CONN::~GUI_RPC_CONN() {
    boinc_close_socket(sock);
}

GUI_RPC_CONN_SET::GUI_RPC_CONN_SET() {
    remote_hosts_configured = false;
    lsock = -1;
    time_of_last_rpc_needing_network = 0;
    safe_strcpy(password,"");
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

// read the GUI RPC password from gui_rpc_auth.cfg;
// create one if missing
//
void GUI_RPC_CONN_SET::get_password() {
    int retval;

    safe_strcpy(password, "");
    FILE* f = fopen(GUI_RPC_PASSWD_FILE, "r");
    if (f) {
        if (fgets(password, 256, f)) {
            strip_whitespace(password);
        }
        fclose(f);

        // if password is empty, allow it but issue a warning
        //
        if (!strlen(password)) {
            msg_printf(NULL, MSG_USER_ALERT,
                "Warning: GUI RPC password is empty.  BOINC can be controlled by any user on this computer.  See https://boinc.berkeley.edu/gui_rpc_passwd.php for more information."
            );
        }
        return;
    }

    // make a random password
    //
    make_secure_random_string(password);

    // try to write it to the file.
    // if fail, just return; we're still password-protected
    //
    f = fopen(GUI_RPC_PASSWD_FILE, "w");
    if (!f) {
        msg_printf(NULL, MSG_USER_ALERT,
            "Can't open %s - fix permissions", GUI_RPC_PASSWD_FILE
        );
    } else {
        retval = fputs(password, f);
        fclose(f);
        if (retval == EOF) {
            msg_printf(NULL, MSG_USER_ALERT,
                "Can't write %s - fix permissions", GUI_RPC_PASSWD_FILE
            );
        }
    }
#ifdef _WIN32
#elif defined(__APPLE__)
    // Mac: Make sure the password file is not world-read or write.
    // If someone can read or set the password,
    // they can cause code to execute as this user.
    //
    if (g_use_sandbox) {
        // Allow group access so authorized administrator can modify it
        chmod(GUI_RPC_PASSWD_FILE, S_IRUSR|S_IWUSR | S_IRGRP | S_IWGRP);
    } else {
        chmod(GUI_RPC_PASSWD_FILE, S_IRUSR|S_IWUSR);
    }
#else
    // general case: allow group read if group is "boinc"
    //
    gid_t gid = getgid();
    struct group *g = getgrgid(gid);
    if (g && !strcmp(g->gr_name, "boinc")) {
        chmod(GUI_RPC_PASSWD_FILE, S_IRUSR|S_IWUSR | S_IRGRP);
    } else {
        chmod(GUI_RPC_PASSWD_FILE, S_IRUSR|S_IWUSR);
    }
#endif
}

int GUI_RPC_CONN_SET::get_allowed_hosts() {
    int retval;
    sockaddr_storage ip_addr;
    char buf[256];

    allowed_remote_ip_addresses.clear();

    // scan remote_hosts.cfg, convert names to IP addresses
    //
    FILE* f = fopen(REMOTEHOST_FILE_NAME, "r");
    if (f) {
        if (log_flags.gui_rpc_debug) {
            msg_printf(0, MSG_INFO,
                "[gui_rpc] found allowed hosts list"
            );
        }

        // read in each line, if it is not a comment
        // then resolve the address and add to our allowed list
        //
        while (fgets(buf, 256, f)) {
            strip_whitespace(buf);
            if (!(buf[0] =='#' || buf[0] == ';') && strlen(buf) > 0 ) {
                retval = resolve_hostname_or_ip_addr(buf, ip_addr);
                if (retval) {
                    msg_printf(NULL, MSG_INFO,
                        "Can't resolve hostname in remote_hosts.cfg: %s",
                        buf
                    );
                } else {
                    allowed_remote_ip_addresses.push_back(ip_addr);
                }
            }
        }
        fclose(f);
    }

    remote_hosts_configured = !allowed_remote_ip_addresses.empty();

    return 0;
}

int GUI_RPC_CONN_SET::insert(GUI_RPC_CONN* p) {
    gui_rpcs.push_back(p);
    return 0;
}

int GUI_RPC_CONN_SET::init_unix_domain() {
#if !defined(_WIN32)
    struct sockaddr_un addr;
    get_password();
    int retval = boinc_socket(lsock, AF_UNIX);
    if (retval) {
        msg_printf(NULL, MSG_INTERNAL_ERROR,
            "Failed to create Unix domain socket: %s", boincerror(retval)
        );
        return retval;
    }
    addr.sun_family = AF_UNIX;
#ifdef ANDROID
    // bind socket in abstract address space, i.e. start with 0 byte
    addr.sun_path[0] = '\0';
    // using app specific socket name instead of GUI_RPC_FILE defintion
    // to avoid interference with other BOINC based Android apps.
    strcpy(&addr.sun_path[1], "edu_berkeley_boinc_client_socket");
    socklen_t len = offsetof(struct sockaddr_un, sun_path) + 1 + strlen(&addr.sun_path[1]);
#else
    // NOTE: if we ever add Mac OS X support, need to change this
    //
#ifdef __APPLE__
    addr.sun_len = sizeof(addr);
#endif
    strcpy(addr.sun_path, GUI_RPC_FILE);
    socklen_t len = offsetof(sockaddr_un, sun_path) + strlen(GUI_RPC_FILE);
#endif
    unlink(GUI_RPC_FILE);
    if (bind(lsock, (struct sockaddr*)&addr, len) < 0) {
        msg_printf(NULL, MSG_INTERNAL_ERROR,
            "Failed to bind Unix domain socket"
        );
        boinc_close_socket(lsock);
        return ERR_BIND;
    }
    retval = listen(lsock, 999);
    if (retval < 0) {
        msg_printf(NULL, MSG_INTERNAL_ERROR,
            "Failed to listen on Unix domain socket"
        );
        boinc_close_socket(lsock);
        return ERR_LISTEN;
    }
#endif
    return 0;
}

// If the client runs at boot time,
// it may be a while (~10 sec) before the DNS system is working.
// If this returns an error,
// it will get called once a second for up to 30 seconds.
// On the last call, "last_time" is set; print error messages then.
//
int GUI_RPC_CONN_SET::init_tcp(bool last_time) {
    sockaddr_in addr;
    int retval;
    bool first = true;

    if (first) {
        get_password();
        get_allowed_hosts();
        first = false;
    }

    retval = boinc_socket(lsock, AF_INET);
    if (retval) {
        if (last_time) {
            msg_printf(NULL, MSG_INTERNAL_ERROR,
                "Failed to create TCP socket: %s", boincerror(retval)
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
    if (cc_config.allow_remote_gui_rpc || remote_hosts_configured) {
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

static void show_connect_error(sockaddr_storage& s) {
    static double last_time=0;
    static int count=0;

    if (last_time == 0) {
        last_time = gstate.now;
        count = 1;
    } else {
        if (!gstate.clock_change && gstate.now - last_time < CONNECT_ERROR_PERIOD) {
            count++;
            return;
        }
        last_time = gstate.now;
    }
    char buf[256];
#ifdef _WIN32
    sockaddr_in* sin = (sockaddr_in*)&s;
    safe_strcpy(buf, inet_ntoa(sin->sin_addr));
#else
    if (s.ss_family == AF_INET) {
        sockaddr_in* sin = (sockaddr_in*)&s;
        inet_ntop(AF_INET, (void*)(&sin->sin_addr), buf, 256);
    } else if (s.ss_family == AF_INET6) {
        sockaddr_in6* sin = (sockaddr_in6*)&s;
        inet_ntop(AF_INET6, (void*)(&sin->sin6_addr), buf, 256);
    } else {
        snprintf(buf, sizeof(buf), "Unknown address family %d", s.ss_family);
    }
#endif
    msg_printf(NULL, MSG_INFO,
        "GUI RPC request from non-allowed address %s",
        buf
    );
    if (count > 1) {
        msg_printf(
            NULL, MSG_INFO,
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

bool GUI_RPC_CONN_SET::check_allowed_list(sockaddr_storage& peer_ip) {
    vector<sockaddr_storage>::iterator remote_iter = allowed_remote_ip_addresses.begin();
    while (remote_iter != allowed_remote_ip_addresses.end() ) {
        if (same_ip_addr(peer_ip, *remote_iter)) {
            return true;
        }
        ++remote_iter;
    }
    return false;
}

void GUI_RPC_CONN_SET::got_select(FDSET_GROUP& fg) {
    int sock, retval;
    vector<GUI_RPC_CONN*>::iterator iter;
    GUI_RPC_CONN* gr;

    if (lsock < 0) return;

    if (FD_ISSET(lsock, &fg.read_fds)) {
        struct sockaddr_storage addr;

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

        bool host_allowed;

        // accept the connection if:
        // 1) allow_remote_gui_rpc is set or
        // 2) client host is included in "remote_hosts" file or
        // 3) client is on localhost
        //
        if (gstate.gui_rpc_unix_domain) {
            host_allowed = true;
        } else if (cc_config.allow_remote_gui_rpc) {
            host_allowed = true;
        } else if (is_localhost(addr)) {
            host_allowed = true;
        } else {
            // reread host file because IP addresses might have changed
            //
            get_allowed_hosts();
            host_allowed = check_allowed_list(addr);
        }

        if (!host_allowed) {
            show_connect_error(addr);
            boinc_close_socket(sock);
        } else {
            gr = new GUI_RPC_CONN(sock);
            if (strlen(password)) {
                gr->auth_needed = true;
            }
            if (gstate.gui_rpc_unix_domain) {
                gr->is_local = true;
            } else {
                gr->is_local = is_localhost(addr);
            }
            if (log_flags.gui_rpc_debug) {
                msg_printf(0, MSG_INFO,
                    "[gui_rpc] got new GUI RPC connection"
                );
            }
            insert(gr);
        }
    }

    // delete connections with failed sockets
    //
    iter = gui_rpcs.begin();
    while (iter != gui_rpcs.end()) {
        gr = *iter;
        if (FD_ISSET(gr->sock, &fg.exc_fds)) {
            delete gr;
            iter = gui_rpcs.erase(iter);
            continue;
        }
        ++iter;
    }

    // handle RPCs on connections with pending requests
    //
    iter = gui_rpcs.begin();
    while (iter != gui_rpcs.end()) {
        gr = *iter;
        if (FD_ISSET(gr->sock, &fg.read_fds)) {
            retval = gr->handle_rpc();
            if (retval) {
                if (log_flags.gui_rpc_debug) {
                    msg_printf(NULL, MSG_INFO,
                        "[gui_rpc] handler returned %d, closing socket\n",
                        retval
                    );
                }
                delete gr;
                iter = gui_rpcs.erase(iter);
                continue;
            }
        }
        ++iter;
    }
}

// called when client is shutting down
//
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
    for (unsigned int i=0; i<gui_rpcs.size(); i++) {
        delete gui_rpcs[i];
    }
    gui_rpcs.clear();
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

void* gui_rpc_handler(void* p) {
    THREAD& thread = *((THREAD*)p);
    GUI_RPC_CONN& grc = *((GUI_RPC_CONN*)thread.arg);
    while (1) {
        if(grc.handle_rpc()){
            break;
        }
        if (grc.quit_flag) {
            break;
        }
    }
    return NULL;
}
