// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2020 University of California
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
#include "boinc_win.h"
#include "win_util.h"
#else
#include "config.h"
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <unistd.h>

#if HAVE_CSIGNAL
#include <csignal>
#elif HAVE_SYS_SIGNAL_H
#include <sys/signal.h>
#elif HAVE_SIGNAL_H
#include <signal.h>
#endif
#endif

#include "common_defs.h"
#include "procinfo.h"
#include "str_util.h"
#include "util.h"

#include "proc_control.h"

using std::vector;

//#define DEBUG_PROC_CONTROL

#ifdef DEBUG_PROC_CONTROL
#include <stdio.h>
#endif

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
#ifdef DEBUG_PROC_CONTROL
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
// Called from:
//  API (processes = self)
//      This handles a) throttling; b) suspend/resume by user
//  wrapper (via suspend_or_resume_process()); process = child
//  wrapper, MP case (via suspend_or_resume_decendants());
//      processes = descendants
//
// The only way to do this on Windows is to enumerate
// all the threads in the entire system,
// and identify those belonging to one of the processes (ugh!!)
//
int suspend_or_resume_threads(
    vector<int>pids, DWORD calling_thread_id, bool resume, bool check_exempt
) {
    HANDLE threads, thread;
    THREADENTRY32 te = {0};
    int retval = 0;
    DWORD n;
    static vector<DWORD> suspended_threads;

#ifdef DEBUG_PROC_CONTROL
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

    if (!resume) {
        suspended_threads.clear();
    }

    do {
        if (check_exempt && !diagnostics_is_thread_exempt_suspend(te.th32ThreadID)) {
#ifdef DEBUG_PROC_CONTROL
            fprintf(stderr, "thread is exempt\n");
#endif
            continue;
        }
#ifdef DEBUG_PROC_CONTROL
        fprintf(stderr, "thread %d PID %d %s\n",
            te.th32ThreadID, te.th32OwnerProcessID,
            precision_time_to_string(dtime())
        );
#endif
        if (te.th32ThreadID == calling_thread_id) continue;
        if (!in_vector(te.th32OwnerProcessID, pids)) continue;
        thread = OpenThread(THREAD_SUSPEND_RESUME, FALSE, te.th32ThreadID);
        if (resume) {
            // check whether we suspended this thread earlier
            //
            if (std::find(
                suspended_threads.begin(), suspended_threads.end(),
                te.th32ThreadID
            ) != suspended_threads.end()) {
                n = ResumeThread(thread);
#ifdef DEBUG_PROC_CONTROL
                fprintf(stderr, "ResumeThread returns %d\n", n);
#endif
            } else {
                n = 0;
            }
        } else {
            n = SuspendThread(thread);
            suspended_threads.push_back(te.th32ThreadID);
#ifdef DEBUG_PROC_CONTROL
            fprintf(stderr, "SuspendThread returns %d\n", n);
#endif
        }
        if (n == -1) retval = -1;
        CloseHandle(thread);
    } while (Thread32Next(threads, &te));

    CloseHandle (threads);
#ifdef DEBUG_PROC_CONTROL
    fprintf(stderr, "end: %s\n", precision_time_to_string(dtime()));
#endif
    return retval;
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
            boinc_sleep(1);
        }
        kill_all(descendants);
        // kill any processes that might have been created
        // in the last 10 secs
        get_descendants(getpid(), descendants);
    }
    kill_all(descendants);
}
#endif

// suspend/resume the descendants of the calling process.
// Unix version lets you choose the stop signal:
// SIGSTOP (the default) can't be caught.
// SIGTSTP can be caught, but it has no effect for processes without a TTY.
// So it's useful only for programs that are wrappers of some sort;
// they must catch and handle it.
//
#ifdef _WIN32
void suspend_or_resume_descendants(bool resume) {
    vector<int> descendants;
    int pid = GetCurrentProcessId();
    get_descendants(pid, descendants);
    suspend_or_resume_threads(descendants, 0, resume, false);
}
#else
void suspend_or_resume_descendants(bool resume, bool use_tstp) {
    vector<int> descendants;
    int pid = getpid();
    get_descendants(pid, descendants);
    for (unsigned int i=0; i<descendants.size(); i++) {
        kill(descendants[i], resume?SIGCONT:(use_tstp?SIGTSTP:SIGSTOP));
    }
}
#endif

// Suspend/resume the given process; used by the wrapper.
// See signal comment above.
//
#ifdef _WIN32
void suspend_or_resume_process(int pid, bool resume) {
    vector<int> pids;
    pids.push_back(pid);
    suspend_or_resume_threads(pids, 0, resume, false);
}
#else
void suspend_or_resume_process(int pid, bool resume, bool use_tstp) {
    ::kill(pid, resume?SIGCONT:(use_tstp?SIGTSTP:SIGSTOP));
}
#endif

// return OS-specific value associated with priority code
//
int process_priority_value(int priority) {
#ifdef _WIN32
    switch (priority) {
    case PROCESS_PRIORITY_LOWEST: return IDLE_PRIORITY_CLASS;
    case PROCESS_PRIORITY_LOW: return BELOW_NORMAL_PRIORITY_CLASS;
    case PROCESS_PRIORITY_NORMAL: return NORMAL_PRIORITY_CLASS;
    case PROCESS_PRIORITY_HIGH: return ABOVE_NORMAL_PRIORITY_CLASS;
    case PROCESS_PRIORITY_HIGHEST: return HIGH_PRIORITY_CLASS;
    }
    return 0;
#else
    switch (priority) {
    case PROCESS_PRIORITY_LOWEST: return PROCESS_IDLE_PRIORITY;
    case PROCESS_PRIORITY_LOW: return PROCESS_MEDIUM_PRIORITY;
    case PROCESS_PRIORITY_NORMAL: return PROCESS_NORMAL_PRIORITY;
    case PROCESS_PRIORITY_HIGH: return PROCESS_ABOVE_NORMAL_PRIORITY;
    case PROCESS_PRIORITY_HIGHEST: return PROCESS_HIGH_PRIORITY;
    }
    return 0;
#endif
}
