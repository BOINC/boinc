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

#ifndef BOINC_GUI_RPC_SERVER_H
#define BOINC_GUI_RPC_SERVER_H

#include "network.h"
#include "acct_setup.h"

// FSM states for auto-update

#define AU_SS_INIT          0
    // no get_screensaver_mode() yet
#define AU_SS_GOT           1
    // got a get_screensaver_mode()
#define AU_SS_QUIT_REQ      2
    // send a QUIT next time
#define AU_SS_QUIT_SENT     3
    // QUIT sent

#define AU_MGR_INIT         0
#define AU_MGR_GOT          1
#define AU_MGR_QUIT_REQ     2
#define AU_MGR_QUIT_SENT    3

#define GUI_RPC_REQ_MSG_SIZE    100000

class GUI_RPC_CONN {
public:
    int sock;
    MIOFILE mfout;
    MFILE mout;
    MIOFILE mfin;
    XML_PARSER xp;
    char request_msg[GUI_RPC_REQ_MSG_SIZE+1];
    int request_nbytes;
    char nonce[256];
    bool auth_needed;
        // if true, don't allow operations other than authentication
    bool got_auth1;
    bool got_auth2;
        // keep track of whether we've got the 2 authentication msgs;
        // don't accept more than one of each (to prevent DoS)
    bool sent_unauthorized;
        // we've send one <unauthorized>.
        // On next auth failure, disconnect
    bool is_local;
        // connection is from local host
    VERSION_INFO client_api;
    std::string client_name;
    bool quit_flag;
    int au_ss_state;
    int au_mgr_state;
    GUI_HTTP gui_http;
    GET_PROJECT_CONFIG_OP get_project_config_op;
    LOOKUP_ACCOUNT_OP lookup_account_op;
    CREATE_ACCOUNT_OP create_account_op;
private:
    bool notice_refresh;
        // next time we get a get_notices RPC,
        // send a -1 seqno, then the whole list
public:
    void set_notice_refresh() {
        notice_refresh = true;
    }
    void clear_notice_refresh() {
        notice_refresh = false;
    }
    bool get_notice_refresh() {
        return notice_refresh;
    }
    GUI_RPC_CONN(int);
    ~GUI_RPC_CONN();
    int handle_rpc();
    void handle_auth1(MIOFILE&);
    int handle_auth2(char*, MIOFILE&);
    void http_error(const char* msg);
    void handle_get();
};

// authentication for GUI RPCs:
// 1) if a host-list file is found, accept only from those hosts
// 2) if a password file file is found, ALSO demand password auth

class GUI_RPC_CONN_SET {
    std::vector<GUI_RPC_CONN*> gui_rpcs;
    std::vector<sockaddr_storage> allowed_remote_ip_addresses;
    int get_allowed_hosts();
    void get_password();
    int insert(GUI_RPC_CONN*);
    bool check_allowed_list(sockaddr_storage& ip_addr);
    bool remote_hosts_configured;
public:
    int lsock;
    double time_of_last_rpc_needing_network;
        // time of the last RPC that needs network access to handle

    GUI_RPC_CONN_SET();
    char password[256];
    void get_fdset(FDSET_GROUP&, FDSET_GROUP&);
    void got_select(FDSET_GROUP&);
    int init_tcp(bool last_time);
    int init_unix_domain();
    void close();
    bool recent_rpc_needs_network(double interval);
    void send_quits();
    bool quits_sent();
    bool poll();
    void set_notice_refresh() {
        for (unsigned int i=0; i<gui_rpcs.size(); i++) {
            gui_rpcs[i]->set_notice_refresh();
        }
    }
};

#endif
