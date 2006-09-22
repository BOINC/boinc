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


#include "config.h"
#include <stdio.h>

#include <ctype.h>
#include <sys/types.h>
#include <dirent.h>

#include "procinfo.h"
#include "client_msgs.h"

using std::vector;


// build table of all processes in system
//
int procinfo_setup(vector<PROCINFO>& pi) {

    FILE* fd;
    PROCINFO p;
    int c, real_mem, virtual_mem, hours;

// Some values of possible interest available from 'ps' command:
// %cpu       percentage cpu usage (alias pcpu)
// %mem       percentage memory usage (alias pmem)
// majflt     total page faults
// minflt     total page reclaims
// nswap      total swaps in/out
// pagein     pageins (same as majflt)
// pid        process ID
// ppid       parent process ID
// poip       pageouts in progress
// rss        resident set size in Kbytes
// rsz        resident set size + (text size / text use count)
// time       accumulated cpu time, user + system
// vsz        virtual size in Kbytes

    fd = popen("ps -axopid,ppid,rsz,vsz,pagein,time", "r");
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
        c = fscanf(fd, "%d%d%d%d%ld%d:%lf\n", &p.id, &p.parentid, &real_mem, &virtual_mem, &p.page_fault_count, &hours, &p.user_time);
        if (c < 7) break;
        p.working_set_size = (double)real_mem * 1024.;
        p.swap_size = (double)virtual_mem * 1024.;
        p.user_time += 60. * (float)hours;
        p.is_boinc_app = false;
        pi.push_back(p);
    }
    
    pclose(fd);
    return 0;
}

// Scan the process table adding in CPU time and mem usage. Loop
// thru entire table as the entries aren't in order.  Recurse at
// most 4 times to get additional child processes 
//
void add_child_totals(PROCINFO& pi, vector<PROCINFO>& piv, int pid, int rlvl) {
    unsigned int i;

    if (rlvl > 3) {
        return;
    }
    for (i=0; i<piv.size(); i++) {
        PROCINFO& p = piv[i];
        if (p.parentid == pid) {
//            pi.kernel_time += p.kernel_time;
            pi.user_time += p.user_time;
            pi.swap_size += p.swap_size;
            pi.working_set_size += p.working_set_size;
            p.is_boinc_app = true;
            // look for child process of this one
            add_child_totals(pi, piv, p.id, rlvl+1); // recursion - woo hoo!
        }
    }
}

// fill in the given PROCINFO (which initially is zero except for id)
// with totals from that process and all its descendants
//
void procinfo_app(PROCINFO& pi, vector<PROCINFO>& piv) {
    unsigned int i;

    for (i=0; i<piv.size(); i++) {
        PROCINFO& p = piv[i];
        if (p.id == pi.id) {
//            pi.kernel_time += p.kernel_time;
            pi.user_time += p.user_time;
            pi.swap_size += p.swap_size;
            pi.working_set_size += p.working_set_size;
            p.is_boinc_app = true;
            // look for child processes
 	    add_child_totals(pi, piv, pi.id, 0);
            return;
        }
    }
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
