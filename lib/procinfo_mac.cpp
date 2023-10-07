// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2022 University of California
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

#define SHOW_TIMING 0

#include "config.h"
#include <cstdio>

#include <ctype.h>
#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>
#include <string>
#include <locale.h>
#include <sys/sysctl.h>
#include <mach/mach.h>

#if SHOW_TIMING
#include <Carbon/Carbon.h>
#include "client_msgs.h"
#endif

#include "error_numbers.h"
#include "procinfo.h"

#include "mac_branding.h"

using std::vector;

// build table of all processes in system
//
int procinfo_setup(PROC_MAP& pm) {
    int pid = getpid();
    FILE* fd;
    PROCINFO p;
    int c, real_mem, virtual_mem, hours;
    char* lf;
    static long iBrandID = -1;
    std::string old_locale;

    // For branded installs, the Mac installer put a branding file in our data directory
    FILE *f = fopen("/Library/Application Support/BOINC Data/Branding", "r");
    if (f) {
        fscanf(f, "BrandId=%ld\n", &iBrandID);
        fclose(f);
    }
    if ((iBrandID < 0) || (iBrandID > (NUMBRANDS-1))) {
        iBrandID = 0;
    }

#if SHOW_TIMING
    UnsignedWide start, end, elapsed;

    start = UpTime();
#endif

// Some values of possible interest available from 'ps' command:
// %cpu       percentage cpu usage (alias pcpu)
// %mem       percentage memory usage (alias pmem)
// command    command (executable name)
// majflt     total page faults
// minflt     total page reclaims
// nswap      total swaps in/out
// pagein     pageins (same as majflt)
// pid        process ID
// ppid       parent process ID
// poip       pageouts in progress
// pri        scheduling priority
// rss        resident set size in Kbytes
// time       accumulated cpu time, user + system
// vsz        virtual size in Kbytes
//
// Unfortunately, the selectors majflt, minflt, pagein do not work on OS X,
// and ps does not return kernel time separately from user time.
//
// Earlier versions of procinf_mac.C launched a small helper application
// AppStats using a bi-directional pipe.  AppStats used mach ports to get
// all the information, including page fault counts, kernel and user times.
// In order to use mach ports, AppStats must run setuid root.
//
// But these three items are not actually used (page fault counts aren't
// available from Windows either) so we have reverted to using the ps
// utility, even though it takes more cpu time than AppStats did.
// This eliminates the need to install our own application which runs setuid
// root; this was perceived by some users as a security risk.

// Under OS 10.8.x (only) ps writes a spurious warning to stderr if called
// from a process that has the DYLD_LIBRARY_PATH environment variable set.
// "env -i command" prevents the command from inheriting the caller's
// environment, which avoids the spurious warning.
    fd = popen("env -i ps -axcopid,ppid,rss,vsz,pagein,time,command", "r");
    if (!fd) return ERR_FOPEN;

    // Skip over the header line
    do {
        c = fgetc(fd);
        if (c == EOF) {
            pclose(fd);
            return ERR_GETS;
        }
    } while (c != '\n');

    // Ensure %lf works correctly if called from non-English Manager
    old_locale = setlocale(LC_ALL, NULL);
    setlocale(LC_ALL, "C");

    while (1) {
        p.clear();
        c = fscanf(fd, "%d%d%d%d%lu%d:%lf ",
            &p.id,
            &p.parentid,
            &real_mem,
            &virtual_mem,
            &p.page_fault_count,
            &hours,
            &p.user_time
        );
        if (c < 7) break;
        if (fgets(p.command, sizeof(p.command) , fd) == NULL) break;
        lf = strchr(p.command, '\n');
        if (lf) *lf = '\0';         // Strip trailing linefeed
        p.working_set_size = (double)real_mem * 1024.;
        p.swap_size = (double)virtual_mem * 1024.;
        p.user_time += 60. * (float)hours;
        p.is_boinc_app = (p.id == pid || strcasestr(p.command, "boinc"));
        // Ideally, we should count ScreenSaverEngine.app as a BOINC process
        // only if BOINC is set as the screensaver.  We could set a flag in
        // the client when the get_screensaver_tasks rpc is called, but that
        // would not be 100% reliable for several reasons.
        if (strcasestr(p.command, "screensaverengine")) p.is_boinc_app = true;

        // We do not mark Mac processes as low priority because some processes
        // (e.g., Finder) change priority frequently, which would cause
        // procinfo_non_boinc() and ACTIVE_TASK_SET::get_memory_usage() to get
        // incorrect results for the % CPU used.
        p.is_low_priority = false;

        if (strcasestr(p.command, brandName[iBrandID])) {
            p.is_boinc_app = true;
        }

        pm.insert(std::pair<int, PROCINFO>(p.id, p));
    }

    pclose(fd);

#if SHOW_TIMING
    end = UpTime();
    elapsed = AbsoluteToNanoseconds(SubAbsoluteFromAbsolute(end, start));
    msg_printf(NULL, MSG_INFO, "elapsed time = %llu, m_swap = %lf\n", elapsed, gstate.host_info.m_swap);
#endif

    find_children(pm);

    setlocale(LC_ALL, old_locale.c_str());
    return 0;
}

// get total user-mode CPU time
//
// From usr/include/mach/processor_info.h:
// struct processor_cpu_load_info {             /* number of ticks while running... */
//	 unsigned int    cpu_ticks[CPU_STATE_MAX]; /* ... in the given mode */
// };
//
double total_cpu_time() {
    static bool first = true;
    natural_t processorCount = 0;
    processor_cpu_load_info_t cpuLoad;
    mach_msg_type_number_t processorMsgCount;
    static double scale;
    uint64_t totalUserTime = 0;

    if (first) {
        first = false;
        long hz = sysconf(_SC_CLK_TCK);
        scale = 1./hz;
    }

    kern_return_t err = host_processor_info(mach_host_self(), PROCESSOR_CPU_LOAD_INFO, &processorCount, (processor_info_array_t *)&cpuLoad, &processorMsgCount);

    if (err != KERN_SUCCESS) {
        return 0.0;
    }

    for (natural_t i = 0; i < processorCount; i++) {
        // Calc user and nice CPU usage, with guards against 32-bit overflow
        // (values are natural_t)
        uint64_t user = 0, nice = 0;

        user = cpuLoad[i].cpu_ticks[CPU_STATE_USER];
        nice = cpuLoad[i].cpu_ticks[CPU_STATE_NICE];

        totalUserTime = totalUserTime + user + nice;
    }

    return totalUserTime * scale;
}
