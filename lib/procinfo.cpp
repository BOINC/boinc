// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2011 University of California
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

// platform-independent process-enumeration functions

#ifndef _WIN32
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#endif

#include "procinfo.h"

using std::vector;

static void get_descendants_aux(vector<PROCINFO>& piv, int pid, vector<int>& pids) {
    for (unsigned int i=0; i<pids.size(); i++) {
        PROCINFO& p = piv[i];
        if (p.parentid == pid) {
            pids.push_back(p.id);
            get_descendants_aux(piv, p.id, pids);
        }
    }
}

// return a list of all descendants of the given process
//
void get_descendants(int pid, vector<int>& pids) {
    int retval;
    vector<PROCINFO> piv;
    retval = procinfo_setup(piv);
    if (retval) return;
    get_descendants_aux(piv, pid, pids);
}


#ifndef _WIN32

// get resource usage of non-BOINC apps
//
void procinfo_other(PROCINFO& pi, vector<PROCINFO>& piv) {
    unsigned int i;

    memset(&pi, 0, sizeof(pi));
    for (i=0; i<piv.size(); i++) {
        PROCINFO& p = piv[i];
        if (p.is_boinc_app) continue;
        if (p.is_low_priority) continue;

        pi.kernel_time += p.kernel_time;
        pi.user_time += p.user_time;
        pi.swap_size += p.swap_size;
        pi.working_set_size += p.working_set_size;
    }
}

bool any_process_exists(vector<int>& pids) {
    int status;
    for (unsigned int i=0; i<pids.size(); i++) {
        if (waitpid(pids[i], &status, WNOHANG) >= 0) {
            return true;
        }
    }
    return false;
}

void kill_all(vector<int>& pids) {
    for (unsigned int i=0; i<pids.size(); i++) {
        kill(pids[i], SIGTERM);
    }
}

#endif
