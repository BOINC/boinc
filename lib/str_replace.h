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

#ifdef __APPLE__
#include <ctype.h>
#endif

#if !HAVE_STRLCPY
extern size_t strlcpy(char*, const char*, size_t);
#endif

#if !HAVE_STRLCAT
extern size_t strlcat(char *dst, const char *src, size_t size);
#endif

#if !HAVE_STRCASESTR
extern const char *strcasestr(const char *s1, const char *s2);
#endif

#if !HAVE_STRCASECMP
inline int strcasecmp(const char* s1, const char* s2) {
    while (*s1 && *s2) {
        char c1 = tolower(*s1++);
        char c2 = tolower(*s2++);
        if (c1 != c2) return 1;     // don't worry about +/-
    }
    if (*s1 || *s2) return 1;
    return 0;
}
#endif

#endif
