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
// 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

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
    
class GUI_RPC_CONN {
public:
    int sock;
    char nonce[256];
    bool auth_needed;
        // if true, don't allow operations other than authentication
    bool got_auth1;
    bool got_auth2;
        // keep track of whether we've got the 2 authentication msgs;
        // don't accept more than one of each (to prevent DoS)
    bool is_local;
        // connection is from local host
    int au_ss_state;
    int au_mgr_state;
    GUI_HTTP gui_http;
    GET_PROJECT_CONFIG_OP get_project_config_op;
    LOOKUP_ACCOUNT_OP lookup_account_op;
    CREATE_ACCOUNT_OP create_account_op;

    GUI_RPC_CONN(int);
    ~GUI_RPC_CONN();
    int handle_rpc();
    void handle_auth1(MIOFILE&);
    int handle_auth2(char*, MIOFILE&);
    void handle_get_project_config(char* buf, MIOFILE& fout);
    void handle_get_project_config_poll(char*, MIOFILE& fout);
    void handle_lookup_account(char* buf, MIOFILE& fout);
    void handle_lookup_account_poll(char*, MIOFILE& fout);
    void handle_create_account(char* buf, MIOFILE& fout);
    void handle_create_account_poll(char*, MIOFILE& fout);
};

// authentication for GUI RPCs:
// 1) if a IPaddr-list file is found, accept only from those addrs
// 2) if a password file file is found, ALSO demand password auth

class GUI_RPC_CONN_SET {
    std::vector<GUI_RPC_CONN*> gui_rpcs;
    std::vector<int> allowed_remote_ip_addresses;
    int get_allowed_hosts();
    int get_password();
    int insert(GUI_RPC_CONN*);
    bool check_allowed_list(int ip_addr);
    bool remote_hosts_file_exists;
public:
    int lsock;
    double time_of_last_rpc_needing_network;
        // time of the last RPC that needs network access to handle

    GUI_RPC_CONN_SET();
    char password[256];
    void get_fdset(FDSET_GROUP&, FDSET_GROUP&);
    void got_select(FDSET_GROUP&);
    int init(bool last_time);
    void close();
    bool recent_rpc_needs_network(double interval);
    void send_quits();
    bool quits_sent();
    bool poll();
};
