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

// a C++ interface to BOINC Account Manager Interface

#ifndef _WIN32
#include <stdio.h>
#include <string>
#include <vector>
#endif

#include "miofile.h"


class ACCT_MGR {
public:
    std::string name;
    std::string url;

    ACCT_MGR();
    ~ACCT_MGR();

    int parse(MIOFILE&);
    void print();
    void clear();
};


class ACCT_MGR_LOGIN {
public:
    std::string login;
    std::string password;

    ACCT_MGR_LOGIN();
    ~ACCT_MGR_LOGIN();

    int parse(MIOFILE&);
    void print();
    void clear();
};


class ACCT_MGR_CLIENT {
public:
    bool acct_mgr_found;
    ACCT_MGR acct_mgr;
    bool acct_mgr_login_found;
    bool acct_mgr_login_initialized;
    ACCT_MGR_LOGIN acct_mgr_login;

    ACCT_MGR_CLIENT();
    ~ACCT_MGR_CLIENT();

    int init();
    void close();
};
