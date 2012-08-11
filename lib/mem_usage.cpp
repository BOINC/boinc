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

#if   defined(_WIN32) && !defined(__STDWX_H__)
#include "boinc_win.h"
#elif defined(_WIN32) && defined(__STDWX_H__)
#include "stdwx.h"
#else
#include "config.h"
#if HAVE_PROCFS_H
// Can't use large file calls with solaris procfs.
#if defined(_FILE_OFFSET_BITS) && ( _FILE_OFFSET_BITS == 64 )
#undef _FILE_OFFSET_BITS
#undef _LARGE_FILES
#undef _LARGEFILE_SOURCE
#undef _LARGEFILE64_SOURCE
#endif
#endif
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#if HAVE_PROCFS_H
#include <procfs.h> // definitions for solaris /proc structs
#endif
#endif

#include "error_numbers.h"

#include "mem_usage.h"

using std::FILE;
using std::fread;
using std::fopen;
using std::fclose;

int mem_usage(double& vm_usage, double& resident_set) {

#ifdef _WIN32

    // Figure out if we're on WinNT
    OSVERSIONINFO osvi; 
    osvi.dwOSVersionInfoSize = sizeof(osvi);
    GetVersionEx( &osvi );
    if (osvi.dwPlatformId == VER_PLATFORM_WIN32_NT) {
        SIZE_T lpMinimumWorkingSetSize;
        SIZE_T lpMaximumWorkingSetSize;

        GetProcessWorkingSetSize(
            GetCurrentProcess(),
            &lpMinimumWorkingSetSize,
            &lpMaximumWorkingSetSize
        );

        vm_usage = (double)lpMinimumWorkingSetSize;
        resident_set = (double)lpMaximumWorkingSetSize;
    } else {
        return ERR_NOT_IMPLEMENTED;
    }
    return 0;
#else


#if HAVE_PROCFS_H && HAVE__PROC_SELF_PSINFO
    FILE* f;

    // guess that this is solaris
    // need psinfo_t from procfs.h
    //
    if ((f = fopen("/proc/self/psinfo", "r")) != 0) {
        psinfo_t psinfo;

        if (fread(&psinfo, sizeof(psinfo_t), 1, f) == 1) {
            vm_usage = psinfo.pr_size*1024.;
            resident_set = psinfo.pr_rssize*1024.;
            fclose(f);
            return 0;
        } else {
            fclose(f);
            return ERR_FREAD;
        }
    }
#endif

#if HAVE__PROC_SELF_STAT
    FILE* f;
    // guess that this is linux
    //
    if ((f = fopen("/proc/self/stat", "r")) != 0) {
        char buf[256];
        char* p;
        int i;
        unsigned long tmp;

        i = fread(buf, sizeof(char), 255, f);
        buf[i] = '\0'; // terminate string
        p = &buf[0];

        // skip over first 22 fields
        //
        for (i = 0; i < 22; ++i) {
            p = strchr(p, ' ');
            if (!p) break;
            ++p; // move past space
        }
        if (!p) {
            return ERR_NOT_IMPLEMENTED;
        }

        // read virtual memory size in bytes.
        //
        vm_usage = atof(p);
        p = strchr(p, ' ');

        // read resident set size: number of pages the process has in
        // real memory, minus 3 for administrative purposes.
        //
        tmp = strtol(p, 0, 0);
        resident_set = (double)(tmp + 3)*getpagesize();

        fclose(f);
        return 0;
    }
#endif

    return ERR_NOT_IMPLEMENTED;
#endif
}

