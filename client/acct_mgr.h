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
#include "http.h"

// represents info stored on files
//
struct ACCT_MGR_INFO {
    std::string acct_mgr_name;
    std::string acct_mgr_url;
    std::string login_name;
    std::string password;

    int parse_url(MIOFILE&);
    int parse_login(MIOFILE&);
    int write_info();
    int init();
};

// stuff after here related to RPCs to account managers

struct ACCOUNT {
    std::string url;
    std::string authenticator;

    int parse(MIOFILE&);
    ACCOUNT();
    ~ACCOUNT();
};

#define ACCT_MGR_STATE_IDLE     0
#define ACCT_MGR_STATE_BUSY     1

struct ACCT_MGR {
    int state;
    ACCT_MGR_INFO ami;
    std::string error_str;
    HTTP_OP http_op;
    std::vector<ACCOUNT> accounts;
    int do_rpc(std::string url, std::string name, std::string password);
    int parse(MIOFILE&);
    bool poll();
    void handle_reply();

    ACCT_MGR();
    ~ACCT_MGR();
};

#endif
