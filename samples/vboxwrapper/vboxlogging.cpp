// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2010-2015 University of California
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

#ifdef _WIN32
#include "boinc_win.h"
#include "win_util.h"
#else
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdarg.h>
#include <cmath>
#include <ctime>
#include <string>
#include <unistd.h>
#endif

#include "vboxlogging.h"


int vboxlog_msg(const char *fmt, ...) {
    int retval = 0;
    char buf[256];
    int pid;
    struct tm tm;
    va_list ap;

    if (fmt == NULL) return 0;

    time_t x = time(0);

#ifdef _WIN32
    pid = GetCurrentProcessId();
    localtime_s(&tm, &x);
#else
    pid = getpid();
    localtime_r(&x, &tm);
#endif

    strftime(buf, sizeof(buf)-1, "%Y-%m-%d %H:%M:%S", &tm);
    fprintf(stderr, "%s (%d): ", buf, pid);

    va_start(ap, fmt);
    retval = vfprintf(stderr, fmt, ap);
    va_end(ap);

    fprintf(stderr, "\n");

    return retval;
}

