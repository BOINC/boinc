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

#ifndef UNIX_UTIL_H
#define UNIX_UTIL_H

// Nothing in this file is needed on WIN32
#ifndef _WIN32

#include "config.h"

#if 0
#ifndef HAVE_SETENV
extern "C" int setenv(const char *name, const char *value, int overwrite);
#endif
#endif


#ifndef HAVE_DAEMON

extern "C" int daemon(int nochdir, int noclose);

#endif /* HAVE_DAEMON */

#endif /* _WIN32 */

#endif /* UNIX_UTIL_H */

