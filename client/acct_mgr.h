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

// represents info stored on files
//
struct ACCT_MGR_INFO {
	// the following used to be std::string but there
	// were mysterious bugs where setting it to "" didn't work
	//
    char acct_mgr_name[256];
    char acct_mgr_url[256];
    char login_name[256];
    char password[256];

    ACCT_MGR_INFO();
    int parse_url(MIOFILE&);
    int parse_login(MIOFILE&);
    int write_info();
    int init();
    void clear();
};

// stuff after here related to RPCs to account managers

struct ACCOUNT {
    std::string url;
    std::string authenticator;

    int parse(MIOFILE&);
    ACCOUNT() {}
    ~ACCOUNT() {}
};

struct ACCT_MGR_OP: public GUI_HTTP_OP {
    int error_num;
    ACCT_MGR_INFO ami;
        // a temporary copy while doing RPC.
        // CLIENT_STATE::acct_mgr_info is authoratative
    std::string error_str;
    std::vector<ACCOUNT> accounts;
    int do_rpc(std::string url, std::string name, std::string password);
    int parse(MIOFILE&);
    virtual void handle_reply(int http_op_retval);

    ACCT_MGR_OP(){}
    virtual ~ACCT_MGR_OP(){}
};

#endif
