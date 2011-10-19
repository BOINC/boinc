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

#include "proc_control.h"

using std::vector;

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
}

#ifdef _WIN32
// signature of OpenThread()
//
typedef HANDLE (WINAPI *tOT)(
    DWORD dwDesiredAccess, BOOL bInheritHandle, DWORD dwThreadId
);

// Suspend or resume the threads in a given process,
// but don't suspend 'calling_thread'.
//
// The only way to do this on Windows is to enumerate
// all the threads in the entire system,
// and find those belonging to the process (ugh!!)
//

int suspend_or_resume_threads(
    DWORD pid, DWORD calling_thread_id, bool resume
) { 
    HANDLE threads, thread;
    static HMODULE hKernel32Lib = NULL;
    THREADENTRY32 te = {0}; 
    static tOT pOT = NULL;
 
    // Dynamically link to the proper function pointers.
    if (!hKernel32Lib) {
        hKernel32Lib = GetModuleHandleA("kernel32.dll");
    }
    if (!pOT) {
        pOT = (tOT) GetProcAddress( hKernel32Lib, "OpenThread" );
    }

    if (!pOT) {
        return -1;
    }

    threads = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0); 
    if (threads == INVALID_HANDLE_VALUE) return -1;
 
    te.dwSize = sizeof(THREADENTRY32); 
    if (!Thread32First(threads, &te)) { 
        CloseHandle(threads); 
        return -1;
    }

    do { 
        if (!diagnostics_is_thread_exempt_suspend(te.th32ThreadID)) continue;
        if (te.th32ThreadID == calling_thread_id) continue;
        if (te.th32OwnerProcessID == pid) {
            thread = pOT(THREAD_SUSPEND_RESUME, FALSE, te.th32ThreadID);
            resume ?  ResumeThread(thread) : SuspendThread(thread);
            CloseHandle(thread);
        } 
    } while (Thread32Next(threads, &te)); 

    CloseHandle (threads); 

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
