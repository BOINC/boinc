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

#include <vector>
#ifdef _WIN32
#include "diagnostics.h"
#ifdef __STDWX_H__
#include "stdwx.h"
#else
#include "boinc_win.h"
#include "win_util.h"
#endif
#else
#include "config.h"
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#if HAVE_CSIGNAL
#include <csignal>
#elif HAVE_SYS_SIGNAL_H
#include <sys/signal.h>
#elif HAVE_SIGNAL_H
#include <signal.h>
#endif
#endif

#include "procinfo.h"
#include "str_util.h"
#include "util.h"

#include "proc_control.h"

using std::vector;

//#define DEBUG

static void get_descendants_aux(PROC_MAP& pm, int pid, vector<int>& pids) {
    PROC_MAP::iterator i = pm.find(pid);
    if (i == pm.end()) return;
    PROCINFO& p = i->second;
    if (p.scanned) return;  // avoid infinite recursion
    p.scanned = true;
    for (unsigned int j=0; j<p.children.size(); j++) {
        int child_pid = p.children[j];
        pids.push_back(child_pid);
        get_descendants_aux(pm, child_pid, pids);
    }
}

// return a list of all descendants of the given process
//
void get_descendants(int pid, vector<int>& pids) {
    int retval;
    PROC_MAP pm;
    pids.clear();
    retval = procinfo_setup(pm);
    if (retval) return;
    get_descendants_aux(pm, pid, pids);
#ifdef DEBUG
    fprintf(stderr, "descendants of %d:\n", pid);
    for (unsigned int i=0; i<pids.size(); i++) {
        fprintf(stderr, "   %d\n", pids[i]);
    }
#endif
}

#ifdef _WIN32

// Suspend or resume the threads in a set of processes,
// but don't suspend 'calling_thread'.
//
// The only way to do this on Windows is to enumerate
// all the threads in the entire system,
// and find those belonging to one of the process (ugh!!)
//

int suspend_or_resume_threads(
    vector<int>pids, DWORD calling_thread_id, bool resume, bool check_exempt
) { 
    HANDLE threads, thread;
    THREADENTRY32 te = {0}; 

#ifdef DEBUG
    fprintf(stderr, "start: check_exempt %d %s\n", check_exempt, precision_time_to_string(dtime()));
    fprintf(stderr, "%s processes", resume?"resume":"suspend");
    for (unsigned int i=0; i<pids.size(); i++) {
        fprintf(stderr, " %d", pids[i]);
    }
    fprintf(stderr, "\n");
#endif

    threads = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0); 
    if (threads == INVALID_HANDLE_VALUE) {
        fprintf(stderr, "CreateToolhelp32Snapshot failed\n");
        return -1;
    }
 
    te.dwSize = sizeof(THREADENTRY32); 
    if (!Thread32First(threads, &te)) { 
        fprintf(stderr, "Thread32First failed\n");
        CloseHandle(threads); 
        return -1;
    }

    do { 
        if (check_exempt && !diagnostics_is_thread_exempt_suspend(te.th32ThreadID)) {
#ifdef DEBUG
            fprintf(stderr, "thread is exempt\n");
#endif
            continue;
        }
        //fprintf(stderr, "thread %d PID %d %s\n", te.th32ThreadID, te.th32OwnerProcessID, precision_time_to_string(dtime()));
        if (te.th32ThreadID == calling_thread_id) continue;
        if (!in_vector(te.th32OwnerProcessID, pids)) continue;
        thread = OpenThread(THREAD_SUSPEND_RESUME, FALSE, te.th32ThreadID);
        if (resume) {
            DWORD n = ResumeThread(thread);
#ifdef DEBUG
            fprintf(stderr, "ResumeThread returns %d\n", n);
#endif
        } else {
            DWORD n = SuspendThread(thread);
#ifdef DEBUG
            fprintf(stderr, "SuspendThread returns %d\n", n);
#endif
        }
        CloseHandle(thread);
    } while (Thread32Next(threads, &te)); 

    CloseHandle (threads); 
#ifdef DEBUG
    fprintf(stderr, "end: %s\n", precision_time_to_string(dtime()));
#endif
    return 0;
} 

#else

bool any_process_exists(vector<int>& pids) {
    int status;
    for (unsigned int i=0; i<pids.size(); i++) {
        if (waitpid(pids[i], &status, WNOHANG) >= 0) {
            return true;
        }
    }
    return false;
}

#endif

void kill_all(vector<int>& pids) {
    for (unsigned int i=0; i<pids.size(); i++) {
#ifdef _WIN32
        HANDLE h = OpenProcess(READ_CONTROL | PROCESS_TERMINATE, false, pids[i]);
        if (h == NULL) continue;
        TerminateProcess(h, 0);
        CloseHandle(h);
#else
        kill(pids[i], SIGTERM);
#endif
    }
}

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
// Same, but if child_pid is nonzero,
// give it a chance to exit gracefully on Unix
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

// suspend/resume the descendants of the calling process
// (or if pid==0, the calling process)
//
void suspend_or_resume_descendants(bool resume) {
    vector<int> descendants;
#ifdef _WIN32
    int pid = GetCurrentProcessId();
    get_descendants(pid, descendants);
    suspend_or_resume_threads(descendants, 0, resume, false);
#else
    int pid = getpid();
    get_descendants(pid, descendants);
    for (unsigned int i=0; i<descendants.size(); i++) {
        kill(descendants[i], resume?SIGCONT:SIGSTOP);
    }
#endif
}

// used by the wrapper
//
void suspend_or_resume_process(int pid, bool resume) {
#ifdef _WIN32
    vector<int> pids;
    pids.push_back(pid);
    suspend_or_resume_threads(pids, 0, resume, false);
#else
    ::kill(pid, resume?SIGCONT:SIGSTOP);
#endif
}
