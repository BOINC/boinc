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
        /// if true, don't allow operations other than authentication
    bool auth_needed;
    bool got_auth1;
        /// keep track of whether we've got the 2 authentication msgs;
        /// don't accept more than one of each (to prevent DoS)
    bool got_auth2;
        /// we've send one <unauthorized>.
        /// On next auth failure, disconnect
    bool sent_unauthorized;
        /// connection is from local host
    bool is_local;
    int au_ss_state;
    int au_mgr_state;
    GUI_HTTP gui_http;
    GET_PROJECT_CONFIG_OP get_project_config_op;
    LOOKUP_ACCOUNT_OP lookup_account_op;
    CREATE_ACCOUNT_OP create_account_op;
    bool notice_refresh;
        // next time we get a get_notices RPC,
        // send a -1 seqno, then the whole list

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
        /// time of the last RPC that needs network access to handle
    double time_of_last_rpc_needing_network;

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
    void set_notice_refresh() {
        for (unsigned int i=0; i<gui_rpcs.size(); i++) {
            gui_rpcs[i]->notice_refresh = true;
        }
    }
};
