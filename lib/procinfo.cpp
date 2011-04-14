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

#if   defined(_WIN32) && !defined(__STDWX_H__)
#include "boinc_win.h"
#include "win_util.h"
#elif defined(_WIN32) && defined(__STDWX_H__)
#include "stdwx.h"
#else
#include "config.h"
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <signal.h>
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

// Kill the descendants of the calling process.
//
#ifdef _WIN32
void kill_descendants() {
    vector<int> descendants;
    // on Win, kill descendants directly
    //
    get_descendants(GetCurrentProcessId(), descendants);
    kill_all(descendants);
}
#else
// Same, but if child_pid is nonzero, give it a chance to exit gracefully on Unix
//
void kill_descendants(int child_pid) {
    vector<int> descendants;
    // on Unix, ask main process nicely.
    // it descendants still exist after 10 sec, use the nuclear option
    //
    get_descendants(getpid(), descendants);
    if (child_pid) {
        ::kill(child_pid, SIGTERM);
        for (int i=0; i<10; i++) {
            if (!any_process_exists(descendants)) {
                return;
            }
            sleep(1);
        }
        kill_all(descendants);
        // kill any processes that might have been created
        // in the last 10 secs
        get_descendants(getpid(), descendants);
    }
    kill_all(descendants);
}
#endif

void suspend_or_resume_all(vector<int>& pids, bool resume) {
    for (unsigned int i=0; i<pids.size(); i++) {
#ifdef _WIN32
        suspend_or_resume_threads(pids[i], 0, resume);
#else
        kill(pids[i], resume?SIGCONT:SIGSTOP);
#endif
    }
}

// suspend/resume the descendants of the given process
// (or if pid==0, the calling process)
//
void suspend_or_resume_descendants(int pid, bool resume) {
    vector<int> descendants;
    if (!pid) {
#ifdef _WIN32
        pid = GetCurrentProcessId();
#else
        pid = getpid();
#endif
    }
    get_descendants(pid, descendants);
    suspend_or_resume_all(descendants, resume);
}

void suspend_or_resume_process(int pid, bool resume) {
#ifdef _WIN32
    suspend_or_resume_threads(pid, 0, resume);
#else
    ::kill(pid, resume?SIGCONT:SIGSTOP);
#endif

}
