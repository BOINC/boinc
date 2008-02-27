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

#if defined(_WIN32) && !defined(__CYGWIN32__)

#if defined(_WIN64) && defined(_M_X64)
#define HOSTTYPE    "windows_x86_64"
#define HOSTTYPEALT "windows_intelx86"
#else
#define HOSTTYPE "windows_intelx86"
#endif

#include "version.h"         // version numbers from autoconf
#endif

#if !defined(_WIN32) || defined(__CYGWIN32__)
#include "config.h"
#include <iostream>
#include <vector>
#include <string>
#include <cstring>
#endif
