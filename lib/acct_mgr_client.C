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

#ifdef _WIN32
#include "boinc_win.h"
#include "version.h"
#else
#include "config.h"
#include <sys/types.h>
#include <sys/un.h>
#include <cstdio>
#include <unistd.h>
#include <cstdlib>
#include <cstring>
#endif

#include "diagnostics.h"
#include "parse.h"
#include "error_numbers.h"
#include "miofile.h"
#include "md5_file.h"
#include "acct_mgr_client.h"

using std::string;
using std::vector;


ACCT_MGR::ACCT_MGR() {
    clear();
}

ACCT_MGR::~ACCT_MGR() {
    clear();
}

int ACCT_MGR::parse(MIOFILE& in) {
    char buf[256];
    while (in.fgets(buf, 256)) {
        if (match_tag(buf, "</acct_mgr>")) return 0;
        else if (parse_str(buf, "<name>", name)) continue;
        else if (parse_str(buf, "<url>", url)) continue;
    }
    return ERR_XML_PARSE;
}

void ACCT_MGR::print() {
    printf("   name: %s\n", name.c_str());
    printf("   url: %s\n", url.c_str());
}

void ACCT_MGR::clear() {
    name.clear();
    url.clear();
}

ACCT_MGR_LOGIN::ACCT_MGR_LOGIN() {
    clear();
}

ACCT_MGR_LOGIN::~ACCT_MGR_LOGIN() {
    clear();
}

int ACCT_MGR_LOGIN::parse(MIOFILE& in) {
    char buf[256];
    while (in.fgets(buf, 256)) {
        if (match_tag(buf, "</acct_mgr_login>")) return 0;
        else if (parse_str(buf, "<login>", login)) continue;
        else if (parse_str(buf, "<password>", password)) continue;
    }
    return ERR_XML_PARSE;
}

void ACCT_MGR_LOGIN::print() {
    printf("   login: %s\n", login.c_str());
    printf("   password: %s\n", password.c_str());
}

void ACCT_MGR_LOGIN::clear() {
    login.clear();
    password.clear();
}


ACCT_MGR_CLIENT::ACCT_MGR_CLIENT()
{
    acct_mgr_found = false;
    acct_mgr_login_found = false;
    acct_mgr_login_initialized = false;
    acct_mgr.clear();
    acct_mgr_login.clear();
}


ACCT_MGR_CLIENT::~ACCT_MGR_CLIENT()
{
    acct_mgr_found = false;
    acct_mgr_login_found = false;
    acct_mgr_login_initialized = false;
    acct_mgr.clear();
    acct_mgr_login.clear();
}


int ACCT_MGR_CLIENT::init()
{
    char    buf[256];
    int     retval;
    MIOFILE mf;
    FILE*   acct_mgr_file;
    FILE*   acct_mgr_login_file;

    acct_mgr_file = fopen("acct_mgr_url.xml", "r");
    if ( NULL != acct_mgr_file ) {
        acct_mgr_found = true;

        mf.init_file(acct_mgr_file);

        while(mf.fgets(buf, sizeof(buf))) {
            if (match_tag(buf, "<acct_mgr>")) {
                retval = acct_mgr.parse(mf);
                if (retval) {
                    fprintf( stderr, "Can't parse acct_mgr in ACCT_MGR_CLIENT: %d", retval );
                }
            }
        }
    }

    acct_mgr_login_file = fopen("acct_mgr_login.xml", "r");
    if ( (NULL != acct_mgr_login_file) && (0 == retval) ) {
        acct_mgr_login_found = true;

        mf.init_file(acct_mgr_login_file);

        while(mf.fgets(buf, sizeof(buf))) {
            if (match_tag(buf, "<acct_mgr_login>")) {
                retval = acct_mgr_login.parse(mf);
                if (retval) {
                    fprintf( stderr, "Can't parse acct_mgr_login in ACCT_MGR_CLIENT: %d", retval );
                }
            }
        }
    }

    if (acct_mgr_file) fclose(acct_mgr_file);
    if (acct_mgr_login_file) fclose(acct_mgr_login_file);

    return 0;
}


void ACCT_MGR_CLIENT::close() {
    if ( (!acct_mgr_login_found ) && ( acct_mgr_login_initialized ) ) {
        FILE* acct_mgr_login_file;
        acct_mgr_login_file = fopen("acct_mgr_login.xml", "w");
        if ( NULL != acct_mgr_login_file ) {
            fprintf(
                acct_mgr_login_file, 
                "<acct_mgr_login>\n"
                "    <login>%s</login>\n"
                "    <password>%s</password>\n"
                "</acct_mgr_login>\n",
                acct_mgr_login.login.c_str(),
                acct_mgr_login.password.c_str()
            );

            fclose(acct_mgr_login_file);
        }
    }
}

