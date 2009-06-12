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

#ifndef _BROWSER_
#define _BROWSER_

#include <sqlite3.h>

//
// The BOINC client now supports the ability to lookup a users
//   authenticator during automatic attachments via a browser
//   cookie.
//

bool detect_setup_authenticator(std::string& project_url, std::string& authenticator);
bool detect_account_manager_credentials(std::string& project_url, std::string& login, std::string& password_hash, std::string& return_url);
bool is_authenticator_valid(const std::string authenticator);

// platform specific browsers
//
#ifdef _WIN32
bool detect_cookie_ie(std::string& project_url, std::string& name, std::string& value);
#endif
#ifdef __APPLE__
bool detect_cookie_safari(std::string& project_url, std::string& name, std::string& value);
#endif

// Cross-platform browsers
//
bool detect_cookie_firefox_2(std::string& project_url, std::string& name, std::string& value);
bool detect_cookie_firefox_3(std::string& project_url, std::string& name, std::string& value);

#if defined(__APPLE__)
    // sqlite3 is not av ailable on Mac OS 10.3.9
    extern int sqlite3_open(const char *filename, sqlite3 **ppDb) __attribute__((weak_import));
    extern int sqlite3_close(sqlite3 *) __attribute__((weak_import));
    extern int sqlite3_exec(sqlite3*,  const char *sql, sqlite3_callback, void *, char **errmsg) __attribute__((weak_import));
    extern void sqlite3_free(char *z) __attribute__((weak_import));
#endif

#endif
