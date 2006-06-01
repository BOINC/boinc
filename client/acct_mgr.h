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

#ifndef _ACCT_MGR_
#define _ACCT_MGR_

#include <string>
#include <vector>

#include "miofile.h"
#include "gui_http.h"
#include "client_types.h"

// represents info stored in acct_mgr_url.xml and acct_mgr_login.xml
//
struct ACCT_MGR_INFO {
	// the following used to be std::string but there
	// were mysterious bugs where setting it to "" didn't work
	//
    char acct_mgr_name[256];
    char acct_mgr_url[256];
    char login_name[256];
    char password_hash[256];
        // md5 of password.lowercase(login_name)
    char signing_key[MAX_KEY_LEN];
    char previous_host_cpid[64];
        // the host CPID sent in last RPC
    double next_rpc_time;
    bool send_gui_rpc_info;
        // whether to include GUI RPC port and password hash
        // in AM RPCs (used for "farm management")
    bool password_error;

    ACCT_MGR_INFO();
    int parse_url(MIOFILE&);
    int parse_login(MIOFILE&);
    int write_info();
    int init();
    void clear();
    bool poll();
};

// stuff after here related to RPCs to account managers

struct AM_ACCOUNT {
    std::string url;
    std::string authenticator;
    char url_signature[MAX_SIGNATURE_LEN];
    bool detach;
    bool update;

    int parse(FILE*);
    AM_ACCOUNT() {}
    ~AM_ACCOUNT() {}
};

struct ACCT_MGR_OP: public GUI_HTTP_OP {
    bool via_gui;
    int error_num;
    ACCT_MGR_INFO ami;
        // a temporary copy while doing RPC.
        // CLIENT_STATE::acct_mgr_info is authoratative
    std::string error_str;
    std::vector<AM_ACCOUNT> accounts;
    double repeat_sec;
    int do_rpc(
        std::string url, std::string name, std::string password,
        bool via_gui
    );
    int parse(FILE*);
    virtual void handle_reply(int http_op_retval);

    ACCT_MGR_OP(){}
    virtual ~ACCT_MGR_OP(){}
};

#endif
