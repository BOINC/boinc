// Berkeley Open Infrastructure for Network Computing
// http://boinc.berkeley.edu
// Copyright (C) 2008 University of California
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

