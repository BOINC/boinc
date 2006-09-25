// Berkeley Open Infrastructure for Network Computing
// http://boinc.berkeley.edu
// Copyright (C) 2006 University of California
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

// procinfo_mac.C
//

#include "config.h"
#include <stdio.h>

#include <ctype.h>
#include <sys/types.h>
#include <dirent.h>

#include "procinfo.h"
#include "client_msgs.h"
#include "file_names.h"
#include "client_state.h"
#include "hostinfo.h"

using std::vector;

// Routines to determine cpu time, memory usage and page fault count for 
// BOINC Client's child processes (including all their descendants) and also 
// totals for all other processes.  This also sets host_info.m_swap.
//
// On the Mac, most of this information is only accessible by the super-user, 
// so this code calls a helper application AppStats which is run setuid root 
// to do most of the work.  
//
// Since AppStats is called from the BOINC Client, it gets the BOINC Client's 
// pid using getppid().  It then searches the process list for all other 
// processes with the same parent.  
//
// For each child of BOINC Client, AppStats totals the cpu times, memory usage 
// and page fault counts for the child and all its descendants, and prints the 
// totals to stdout.
//
// Finally, AppStats computes these totals for all remaining processes and 
// prints them to stdout (with a value of 0 for the pid).
//
// This code doesn't use PROCINFO.is_boinc_app, but we set it to be consistent 
// with the code for other platforms.


int procinfo_setup(vector<PROCINFO>& pi) {
    char appstats_path[100];
    FILE* fd;
    PROCINFO p;
    int c;
    unsigned int i;
    double m_swap;

    sprintf(appstats_path, "%s/%s", SWITCHER_DIR, APP_STATS_FILE_NAME);
    fd = popen(appstats_path, "r");
    if (!fd) return 0;

    while (1) {
        memset(&p, 0, sizeof(p));
        c = fscanf(fd, "%d %d %lf %lf %lu %lf %lf\n", 
                        &p.id, &p.parentid, &p.working_set_size, &p.swap_size, 
                        &p.page_fault_count, &p.user_time, &p.kernel_time);
        if (c < 7) break;
        p.is_boinc_app = false;
        pi.push_back(p);
        
        if (log_flags.mem_usage_debug) {
            msg_printf(
                    NULL, MSG_INFO,
                    "[mem_usage_debug] pid=%d, ppid=%d, rm=%.0lf, vm=%.0lf, pageins=%lu, usertime=%lf, systime=%lf\n",
                    p.id, p.parentid, p.working_set_size, p.swap_size, 
                    p.page_fault_count, p.user_time, p.kernel_time  
            );
        }
    }
    
    pclose(fd);
    
    // The sysctl(vm.vmmeter) function doesn't work on OS X, so hostinfo_unix.C
    // function HOST_INFO::get_host_info() can't get the total swap space. 
    // It is easily calculated here, so fill in the value of host_info.m_swap.
    m_swap = 0;
    for (i=0; i<pi.size(); i++) {
        m_swap += pi[i].swap_size;
    }
    gstate.host_info.m_swap = m_swap;
    
    return 0;
}

// fill in the given PROCINFO (which initially is zero except for id)
// with totals from that process and all its descendants
//
void procinfo_app(PROCINFO& pi, vector<PROCINFO>& piv) {
    unsigned int i;
    // AppStat returned totals for each of BOINC's children and its descendants
    // as a single PROCINFO struct with the id field set to the pid of BOINC's 
    // direct child. 
    for (i=0; i<piv.size(); i++) {
        PROCINFO& p = piv[i];
        if (p.id == pi.id) {
            pi = p;
            p.is_boinc_app = true;
            return;
        }
    }
}

void procinfo_other(PROCINFO& pi, vector<PROCINFO>& piv) {
    // AppStat returned total for all other processes as a single PROCINFO 
    // struct with id field set to zero.
    memset(&pi, 0, sizeof(pi));
    procinfo_app(pi, piv);
}
