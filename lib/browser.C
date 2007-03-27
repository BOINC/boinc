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


#if defined(_WIN32) && !defined(__STDWX_H__) && !defined(_BOINC_WIN_) && !defined(_AFX_STDAFX_H_)
#include "boinc_win.h"
#endif

#ifndef _WIN32
#include <string>
#endif

#include "browser.h"

//
// Some projects perfer to use there web sites as the primary means of setting up
//   there user accounts, so walk through the various browsers looking up the
//   project cookies until the projects 'Setup' cookie is found.
//
// Give preference to the default platform specific browers first before going
//   to the platform independant browsers since most people don't switch from
//   the default.
// 
bool detect_setup_authenticator(std::string& project_url, std::string& authenticator) {
#ifdef _WIN32
    if (detect_setup_authenticator_ie(project_url, authenticator)) {
        return true;
    }
#endif
    if (detect_setup_authenticator_firefox(project_url, authenticator)) {
        return true;
    }

    return false;
}

