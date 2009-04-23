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

//
// The BOINC client now supports the ability to lookup a users
//   authenticator during automatic attachments via a browser
//   cookie.
//

bool detect_setup_authenticator(std::string& project_url, std::string& authenticator);

// is_authenticator_valid() is used by detect_setup_authenticator_safari()
// in mac_bowser.mm
bool is_authenticator_valid(const std::string authenticator);

// These functions are browser specific functions
//
#ifdef __APPLE__
bool detect_setup_authenticator_safari(std::string& project_url, std::string& authenticator);
#endif
#ifdef _WIN32
bool detect_setup_authenticator_ie(std::string& project_url, std::string& authenticator);
#endif
bool detect_setup_authenticator_firefox(std::string& project_url, std::string& authenticator);

#endif
