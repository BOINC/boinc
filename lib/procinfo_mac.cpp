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
#endif

#include "procinfo.h"
#include "client_msgs.h"
#include "client_state.h"

using std::vector;

// Possible values of iBrandId:
#define BOINC_BRAND_ID 0
#define GRIDREPUBLIC_BRAND_ID 1
#define PROGRESSTHRUPROCESSORS_BRAND_ID 2


// build table of all processes in system
//
int procinfo_setup(vector<PROCINFO>& pi) {
    int pid = getpid();
    FILE* fd;
    PROCINFO p;
    int c, real_mem, virtual_mem, hours;
    char* lf;
    static long iBrandID = -1;
    
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
// rss        resident set size in Kbytes
// time       accumulated cpu time, user + system
// vsz        virtual size in Kbytes
//
// Unfortunately, the selectors majflt, minflt, pagein do not work on OS X, 
// and ps does not return kernel time separately from user time.  
//
// This code doesn't use PROCINFO.is_boinc_app, but we set it to be 
// consistent with the code for other platforms.
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


    fd = popen("ps -axcopid,ppid,rss,vsz,pagein,time,command", "r");
    if (!fd) return 0;

    // Skip over the header line
    do {
        c = fgetc(fd);
        if (c == EOF) {
            pclose(fd);
            return 0;
        }
    } while (c != '\n');

    while (1) {
        memset(&p, 0, sizeof(p));
        c = fscanf(fd, "%d%d%d%d%ld%d:%lf ", &p.id, &p.parentid, &real_mem, 
                    &virtual_mem, &p.page_fault_count, &hours, &p.user_time);
        if (c < 7) break;
        if (fgets(p.command, sizeof(p.command) , fd) == NULL) break;
        lf = strchr(p.command, '\n');
        if (lf) *lf = '\0';         // Strip trailing linefeed
        p.working_set_size = (double)real_mem * 1024.;
        p.swap_size = (double)virtual_mem * 1024.;
        p.user_time += 60. * (float)hours;
        p.is_boinc_app = (p.id == pid || strcasestr(p.command, "boinc"));

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
        pi.push_back(p);
    }
    
    pclose(fd);

#if SHOW_TIMING
    end = UpTime();
    elapsed = AbsoluteToNanoseconds(SubAbsoluteFromAbsolute(end, start));
    msg_printf(NULL, MSG_INFO, "elapsed time = %llu, m_swap = %lf\n", elapsed, gstate.host_info.m_swap);
#endif
    
    return 0;
}

// Scan the process table adding in CPU time and mem usage. Loop
// thru entire table as the entries aren't in order.  Recurse at
// most 4 times to get additional child processes 
//
void add_proc_totals(PROCINFO& pi, vector<PROCINFO>& piv, int pid, char* graphics_exec_file, int start, int rlvl) {
    unsigned int i;

    if (rlvl > 4) {
        return;
    }
    for (i=0; i<piv.size(); i++) {
        PROCINFO& p = piv[i];
		if (p.id == pid || p.parentid == pid) {
//            pi.kernel_time += p.kernel_time;
            pi.user_time += p.user_time;
            pi.swap_size += p.swap_size;
            pi.working_set_size += p.working_set_size;
            p.is_boinc_app = true;
        }
        if (!strcmp(p.command, graphics_exec_file)) {
            p.is_boinc_app = true;
        }
        // look for child process of this one
		if (p.parentid == pid) {
			add_proc_totals(pi, piv, p.id, graphics_exec_file, i+1, rlvl+1);    // recursion - woo hoo!
		}
    }
}

// fill in the given PROCINFO (which initially is zero except for id)
// with totals from that process and all its descendants
//
void procinfo_app(PROCINFO& pi, vector<PROCINFO>& piv, char* graphics_exec_file) {
	add_proc_totals(pi, piv, pi.id, graphics_exec_file, 0, 0);
}

void procinfo_other(PROCINFO& pi, vector<PROCINFO>& piv) {
    unsigned int i;

    memset(&pi, 0, sizeof(pi));
    for (i=0; i<piv.size(); i++) {
        PROCINFO& p = piv[i];
        if (!p.is_boinc_app) {
//            pi.kernel_time += p.kernel_time;
            pi.user_time += p.user_time;
            pi.swap_size += p.swap_size;
            pi.working_set_size += p.working_set_size;
            p.is_boinc_app = true;
        }
    }
}
