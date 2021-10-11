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

// Don't include config.h or C++ header files from here.
// there are #defines that alter the behaviour of the standard C and C++ headers.
// In this case we need to use the "small files" environment on some unix
// systems.  That can't be done if we include "cpp.h"

#if defined(_WIN32) && !defined(__CYGWIN32__)

#if defined(_WIN64) && (defined(_M_X64) || defined(__x86_64__))
#define HOSTTYPE    "windows_x86_64"
#define HOSTTYPEALT "windows_intelx86"
#elif defined(_M_ARM)
#define HOSTTYPE    "windows_arm"
#elif defined(_M_ARM64)
#define HOSTTYPE    "windows_arm64"
#define HOSTTYPEALT "windows_arm"
#else
#define HOSTTYPE "windows_intelx86"
#endif

#include "version.h"         // version numbers from autoconf
#endif
