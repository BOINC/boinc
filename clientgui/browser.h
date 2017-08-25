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

#ifndef BOINC_BROWSER_H
#define BOINC_BROWSER_H

//
// The BOINC client now supports the ability to lookup a users
//   authenticator during automatic attachments via a browser
//   cookie.
//

bool detect_setup_authenticator(std::string& project_url, std::string& authenticator);

bool detect_simple_account_credentials(
    std::string& project_name, std::string& project_url, std::string& authenticator, 
    std::string& project_institution, std::string& project_description, std::string& known
);

bool detect_account_manager_credentials(
    std::string& project_url, std::string& login, std::string& password_hash,
    std::string& return_url
);

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
bool detect_cookie_firefox_3(std::string& project_url, std::string& name, std::string& value);
bool detect_cookie_chrome(std::string& project_url, std::string& name, std::string& value);

#endif
