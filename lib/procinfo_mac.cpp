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

#define SHOW_TIMING 0

#include "config.h"
#include <cstdio>

#include <ctype.h>
#include <sys/types.h>
#include <dirent.h>

#if SHOW_TIMING
#include <Carbon/Carbon.h>
#include "client_msgs.h"
#endif

#include "error_numbers.h"

#include "procinfo.h"

using std::vector;

// Possible values of iBrandId:
#define BOINC_BRAND_ID 0
#define GRIDREPUBLIC_BRAND_ID 1
#define PROGRESSTHRUPROCESSORS_BRAND_ID 2


// build table of all processes in system
//
int procinfo_setup(PROC_MAP& pm) {
    int pid = getpid();
    FILE* fd;
    PROCINFO p;
    int c, real_mem, virtual_mem, hours;
    char* lf;
    static long iBrandID = -1;
    int priority;
    
    if (iBrandID < 0) {
        iBrandID = BOINC_BRAND_ID;

        // For GridRepublic or ProgressThruProcessors, the Mac 
        // installer put a branding file in our data directory
        FILE *f = fopen("/Library/Application Support/BOINC Data/Branding", "r");
        if (f) {
            fscanf(f, "BrandId=%ld\n", &iBrandID);
            fclose(f);
        }
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


    fd = popen("ps -axcopid,ppid,rss,vsz,pagein,pri,time,command", "r");
    if (!fd) return ERR_FOPEN;

    // Skip over the header line
    do {
        c = fgetc(fd);
        if (c == EOF) {
            pclose(fd);
            return ERR_GETS;
        }
    } while (c != '\n');

    while (1) {
        p.clear();
        c = fscanf(fd, "%d%d%d%d%ld%d%d:%lf ",
            &p.id,
            &p.parentid,
            &real_mem, 
            &virtual_mem,
            &p.page_fault_count,
            &priority,
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
        p.is_low_priority = (priority <= 12);

        switch (iBrandID) {
        case GRIDREPUBLIC_BRAND_ID:
            if (!strcmp(p.command, "GridRepublic Desktop")) {
                p.is_boinc_app = true;
            }
            break;
        case PROGRESSTHRUPROCESSORS_BRAND_ID:
            if (!strcmp(p.command, "Progress Thru Processors Desktop")) {
                p.is_boinc_app = true;
            }
            break;
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
    return 0;
}
