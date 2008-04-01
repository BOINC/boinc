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

#ifndef UNIX_UTIL_H
#define UNIX_UTIL_H

// Nothing in this file is needed on WIN32
#ifndef _WIN32

#include "config.h"


#ifdef __cplusplus
#include <cstdio>
#include <cstdlib>
#else
#include <stdio.h>
#include <stdlib.h>
#endif

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#ifndef HAVE_DAEMON

#ifdef __cplusplus
extern "C" {
#endif

int daemon(int nochdir, int noclose);

#ifdef __cplusplus
}
#endif

#endif /* HAVE_DAEMON */

#endif /* _WIN32 */

#endif /* UNIX_UTIL_H */

