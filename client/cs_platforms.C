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
// 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

// Determine which platforms are supported and provide a way
//   of exposing that information to the rest of the client.

#include "cpp.h"

#ifdef _WIN32
#include "boinc_win.h"
#endif

#ifndef _WIN32
#include "config.h"
#include <stdio.h>
#endif

#include "client_types.h"
#include "client_state.h"
#include "error_numbers.h"
#include "log_flags.h"
#include "str_util.h"
#include "util.h"


// return the primary platform id.
//
const char* CLIENT_STATE::get_primary_platform() {
    return platforms[0].name.c_str();
}


// add a platform to the vector.
//
void CLIENT_STATE::add_platform(const char* platform) {
    PLATFORM pp;
    pp.name = platform;
    platforms.push_back(pp);
}


// determine the list of supported platforms.
//
void CLIENT_STATE::detect_platforms() {

#if defined(_WIN32) && !defined(__CYGWIN32__)
#if defined(_WIN64) && defined(_M_X64)

    add_platform("windows_x86_64");
    add_platform("windows_intelx86");

#else

    add_platform("windows_intelx86");

#endif

#elif defined(__APPLE__)
#if defined(__x86_64__)

    add_platform("x86_64-apple-darwin");

#endif

#if defined(__i386__) || defined(__x86_64__)

    // Supported on both Mac Intel architectures
    add_platform("i686-apple-darwin");

#endif

    // Supported on all 3 Mac architectures
    add_platform("powerpc-apple-darwin");

#else

    // Any other platform, fall back to the previous method
    add_platform(HOSTTYPE);
#ifdef HOSTTYPEALT
    add_platform(HOSTTYPEALT);
#endif

#endif

    if (config.no_alt_platform) {
        PLATFORM p = platforms[0];
        platforms.clear();
        platforms.push_back(p);
    }

    // add platforms listed in cc_config.xml AFTER the above.
    //
    for (unsigned int i=0; i<config.alt_platforms.size(); i++) {
        add_platform(config.alt_platforms[i].c_str());
    }
}


// write XML list of supported platforms
//
void CLIENT_STATE::write_platforms(PROJECT* p, MIOFILE& mf) {

    mf.printf(
        "    <platform_name>%s</platform_name>\n",
        p->anonymous_platform ? "anonymous" : get_primary_platform()
    );

    for (unsigned int i=1; i<platforms.size(); i++) {
        PLATFORM& platform = platforms[i];
        mf.printf(
            "    <alt_platform>\n"
            "        <name>%s</name>\n"
            "    </alt_platform>\n",
            platform.name.c_str()
        );
    }
}

bool CLIENT_STATE::is_supported_platform(const char* p) {
    for (unsigned int i=0; i<platforms.size(); i++) {
        PLATFORM& platform = platforms[i];
        if (!strcmp(p, platform.name.c_str())) {
            return true;
        }
    }
    return false;
}
