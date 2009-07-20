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

// declare replacement string functions for platforms that lack themn

#ifndef STR_REPLACE_H
#define STR_REPLACE_H

#ifndef _WIN32
#include "config.h"
#endif

#if !defined(HAVE_STRLCPY)
extern size_t strlcpy(char*, const char*, size_t);
#endif

#if !defined(HAVE_STRLCAT)
extern size_t strlcat(char *dst, const char *src, size_t size);
#endif

#if !defined(HAVE_STRCASESTR)
extern const char *strcasestr(const char *s1, const char *s2);
#endif

#endif
