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

// Determine which platforms are supported and provide a way
//   of exposing that information to the rest of the client.

#include "cpp.h"

#ifdef _WIN32
#include "boinc_win.h"
#endif

#ifndef _WIN32
#include "config.h"
#include <stdio.h>
#include <math.h>
#include <time.h>
#include <strings.h>
#include <map>
#include <set>
#endif

#include "client_types.h"
#include "client_state.h"
#include "error_numbers.h"
#include "str_util.h"
#include "util.h"


// return the primary platform id.
//
const char* CLIENT_STATE::get_primary_platform() {
    return platforms[0]->name.c_str();
}


// add a supported platform to the platforms vector.
//
void CLIENT_STATE::add_supported_platform(const char* supported_platform) {
    PLATFORM* pp = new PLATFORM;
    pp->name = supported_platform;
    platforms.push_back(pp);
}


// determine the list of supported platforms.
//
void CLIENT_STATE::detect_supported_platforms() {

#if defined(_WIN32) && !defined(__CYGWIN32__)
#if defined(_WIN64) && defined(_M_X64)

    add_supported_platform("windows_x86_64");
    add_supported_platform("windows_intelx86");

#else

    add_supported_platform("windows_intelx86");

#endif

#elif defined(__APPLE__)
#if defined(__i386__)

    add_supported_platform("i686-apple-darwin");
    add_supported_platform("powerpc-apple-darwin");

#else

    add_supported_platform("powerpc-apple-darwin");

#endif

#else

    // Any other platform, fall back to the previous method
    add_supported_platform(HOSTTYPE);
#ifdef HOSTTYPEALT
    add_supported_platform(HOSTTYPEALT);
#endif

#endif

}


// report the list of supported platforms
//
void CLIENT_STATE::report_supported_platforms(PROJECT* p, MIOFILE& mf) {
    PLATFORM* platform = NULL;
    unsigned int i = 0;

    // primary platform
    mf.printf(
        "    <platform_name>%s</platform_name>\n",
        p->anonymous_platform ? "anonymous" : get_primary_platform()
    );

    // alternate platforms
    for (i=1; i<platforms.size(); i++) {
        platform = platforms[i];
        mf.printf(
            "    <alt_platform>\n"
            "        <name>%s</name>\n"
            "    </alt_platform>\n",
            platform->name.c_str()
        );
    }
}

