#ifdef _WIN32
#include "boinc_win.h"
#include "win_config.h"
#else
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#endif

#ifdef HAVE_PROCFS_H
#include <procfs.h> // definitions for solaris /proc structs
#endif

#include "error_numbers.h"
#include "mem_usage.h"

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


#if defined(HAVE_PROCFS_H) && defined(HAVE__PROC_SELF_PSINFO)
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

#if defined(HAVE__PROC_SELF_STAT)
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

#ifdef __GNUC__
static volatile const char  __attribute__((unused)) *BOINCrcsid="$Id$";
#else
static volatile const char *BOINCrcsid="$Id$";
#endif
