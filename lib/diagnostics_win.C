// Berkeley Open Infrastructure for Network Computing
// http://boinc.berkeley.edu
// Copyright (C) 2005 University of California
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

// Stuff related to catching SEH exceptions, monitoring threads, and trapping
// debugger messages; used by both core client and by apps.

#if !defined(__STDWX_H__) && !defined(_BOINC_WIN_) && !defined(_AFX_STDAFX_H_)
#include "boinc_win.h"
#endif

#ifndef __CYGWIN32__
#include "stackwalker_win.h"
#endif

#include "version.h"
#include "diagnostics.h"
#include "diagnostics_win.h"
#include "error_numbers.h"
#include "util.h"


// NtQuerySystemInformation
typedef NTSTATUS (WINAPI *tNTQSI)(
    ULONG SystemInformationClass,
    PVOID SystemInformation,
    ULONG SystemInformationLength,
    PULONG ReturnLength
);

// CreateToolhelp32Snapshot
typedef HANDLE (WINAPI *tCT32S)(DWORD dwFlags, DWORD dwProcessID);
// Thread32First
typedef BOOL (WINAPI *tT32F)(HANDLE hSnapshot, LPTHREADENTRY32 lpte);
// Thread32Next
typedef BOOL (WINAPI *tT32N)(HANDLE hSnapshot, LPTHREADENTRY32 lpte);
// OpenThread
typedef HANDLE (WINAPI *tOT)(DWORD dwDesiredAccess, BOOL bInheritHandle, DWORD dwThreadId);


// Provide a set of API's which can be used to display more friendly
//   information about each thread.  These should also be used to
//   dump the callstacks for each executing thread when an unhandled
//   SEH exception is thrown.
//

// This structure is used to keep track of stuff nessassary
//   to dump backtraces for all threads during an abort or
//   crash.  This is platform specific in nature since it
//   depends on the OS datatypes.
typedef struct _BOINC_THREADLISTENTRY {
    std::string         name;
    DWORD               thread_id;
    HANDLE              thread_handle;
    BOOL                graphics_thread;
    BOOL                worker_thread;
    BOOL                crash_suspend_exempt;
    FLOAT               crash_kernel_time;
    FLOAT               crash_user_time;
    FLOAT               crash_wait_time;
    INT                 crash_priority;
    INT                 crash_base_priority;
    INT                 crash_state;
    INT                 crash_wait_reason;
    PEXCEPTION_POINTERS crash_exception_record;
} BOINC_THREADLISTENTRY, *PBOINC_THREADLISTENTRY;

static std::vector<PBOINC_THREADLISTENTRY> diagnostics_threads;
static HANDLE hThreadListSync;


// Initialize the thread list entry.
int diagnostics_init_thread_entry(PBOINC_THREADLISTENTRY entry) {
    entry->name.clear();
    entry->thread_id = 0;
    entry->thread_handle = 0;
    entry->crash_suspend_exempt = FALSE;
    entry->crash_kernel_time = 0.0;
    entry->crash_user_time = 0.0;
    entry->crash_wait_time = 0.0;
    entry->crash_priority = 0;
    entry->crash_base_priority = 0;
    entry->crash_state = 0;
    entry->crash_wait_reason = 0;
    entry->crash_exception_record = NULL;
    return 0;
}


// Initialize the thread list, which means empty it if anything is
//   in it.
int diagnostics_init_thread_list() {
    unsigned int i;
    PBOINC_THREADLISTENTRY pThreadEntry = NULL;
    std::vector<PBOINC_THREADLISTENTRY>::iterator thread_iter;


    // Create a Mutex that can be used to syncronize data access
    //   to the global thread list.
    hThreadListSync = CreateMutex(NULL, TRUE, NULL);

    for (i=0; i<diagnostics_threads.size(); i++) {
        pThreadEntry = diagnostics_threads[0];

        thread_iter = diagnostics_threads.begin();
        diagnostics_threads.erase(thread_iter);

        if (pThreadEntry) delete pThreadEntry;
    }

    // Release the Mutex
    ReleaseMutex(hThreadListSync);

    return 0;
}


// Finish the thread list, which means empty it if anything is
//   in it.
int diagnostics_finish_thread_list() {
    unsigned int i;
    PBOINC_THREADLISTENTRY pThreadEntry = NULL;
    std::vector<PBOINC_THREADLISTENTRY>::iterator thread_iter;


    // Wait for the ThreadListSync mutex before writing updates
    WaitForSingleObject(hThreadListSync, INFINITE);

    for (i=0; i<diagnostics_threads.size(); i++) {
        pThreadEntry = diagnostics_threads[0];

        thread_iter = diagnostics_threads.begin();
        diagnostics_threads.erase(thread_iter);

        if (pThreadEntry) delete pThreadEntry;
    }

    // Release the Mutex
    ReleaseMutex(hThreadListSync);
    CloseHandle(hThreadListSync);

    return 0;
}


// Return a pointer to the thread entry.
//
PBOINC_THREADLISTENTRY diagnostics_find_thread_entry(DWORD dwThreadId) {
    PBOINC_THREADLISTENTRY pThread = NULL;
    UINT                   uiIndex = 0;
    size_t                 size = 0;

    size = diagnostics_threads.size();
    for (uiIndex = 0; uiIndex < size; uiIndex++) {
        if (diagnostics_threads[uiIndex]) {
            if (dwThreadId == diagnostics_threads[uiIndex]->thread_id) {
                pThread = diagnostics_threads[uiIndex];
            }
        }
    }

    return pThread;
}


// Enumerate the running threads in the process space and add them to
//   the list.  This is the most compatible implementation.
int diagnostics_update_thread_list_9X() {
    HANDLE  hThreadSnap = INVALID_HANDLE_VALUE; 
    HANDLE  hThread = NULL;
    HMODULE hKernel32 = NULL;
    PBOINC_THREADLISTENTRY pThreadEntry = NULL;
    tCT32S  pCT32S = NULL;
    tT32F   pT32F = NULL;
    tT32N   pT32N = NULL;
    tOT     pOT = NULL;
    THREADENTRY32 te32; 

    // Which version of the data structure are we using.
    te32.dwSize = sizeof(te32); 

    // Dynamically link to the proper function pointers.
    hKernel32 = LoadLibrary("kernel32.dll");

    pCT32S = (tCT32S) GetProcAddress( hKernel32, "CreateToolhelp32Snapshot" );
    pT32F = (tT32F) GetProcAddress( hKernel32, "Thread32First" );
    pT32N = (tT32N) GetProcAddress( hKernel32, "Thread32Next" );
    pOT = (tOT) GetProcAddress( hKernel32, "OpenThread" );

    if (!pCT32S || !pT32F || !pT32N) {
        CloseHandle( hKernel32 );
        return ERROR_NOT_SUPPORTED; 
    }

    // Take a snapshot of all running threads  
    hThreadSnap = pCT32S(TH32CS_SNAPTHREAD, 0); 
    if( hThreadSnap == INVALID_HANDLE_VALUE ) {
        CloseHandle( hKernel32 );
        return GetLastError(); 
    }

    // Retrieve information about the first thread,
    // and exit if unsuccessful
    if( !pT32F( hThreadSnap, &te32 ) ) {
        CloseHandle( hKernel32 );
        CloseHandle( hThreadSnap );
        return GetLastError();
    }

    // Wait for the ThreadListSync mutex before writing updates
    WaitForSingleObject(hThreadListSync, INFINITE);

    // Now walk the thread list of the system,
    // and display information about each thread
    // associated with the specified process
    do { 
        if( te32.th32OwnerProcessID == GetCurrentProcessId() ) {
            pThreadEntry = diagnostics_find_thread_entry(te32.th32ThreadID);
            if (!pThreadEntry) {
                pThreadEntry = new BOINC_THREADLISTENTRY;
                diagnostics_init_thread_entry(pThreadEntry);
                pThreadEntry->thread_id = te32.th32ThreadID;
                if (pOT) {
                    hThread = pOT(
                        THREAD_ALL_ACCESS,
                        FALSE,
                        te32.th32ThreadID
                    );
                    pThreadEntry->thread_handle = hThread;
                }
                diagnostics_threads.push_back(pThreadEntry);
            }
        }
    } 
    while( pT32N(hThreadSnap, &te32 ) ); 

    // Release the Mutex
    ReleaseMutex(hThreadListSync);

    CloseHandle(hThreadSnap);
    CloseHandle(hKernel32);

    return 0;
}


// Use the native NT API to get all the process and thread information
//   about the current process.  This isn't a fully documented API but
//   enough information exists that we can rely on it for the known
//   Windows OS versions.  For each new Windows version check the
//   _SYSTEM_PROCESS and _SYSTEM_THREAD structures in the DDK to make
//   sure it is compatible with the existing stuff.
int diagnostics_get_process_information(PVOID* ppBuffer, PULONG pcbBuffer) {
    int      retval = 0;
    NTSTATUS Status = STATUS_INFO_LENGTH_MISMATCH;
    HANDLE   hHeap  = GetProcessHeap();
    HMODULE  hNTDll = NULL;
    tNTQSI   pNTQSI = NULL;

    hNTDll = LoadLibrary("ntdll.dll");

    pNTQSI = (tNTQSI)GetProcAddress(hNTDll, "NtQuerySystemInformation");

    do {
        *ppBuffer = HeapAlloc(hHeap, HEAP_ZERO_MEMORY, *pcbBuffer);
        if (ppBuffer == NULL) {
            retval = ERROR_NOT_ENOUGH_MEMORY;
        }

        Status = pNTQSI(
            SystemProcessAndThreadInformation,
            *ppBuffer,
            *pcbBuffer,
            pcbBuffer
        );

        if (Status == STATUS_INFO_LENGTH_MISMATCH) {
            HeapFree(hHeap, NULL, *ppBuffer);
            *pcbBuffer *= 2;
        } else if (!NT_SUCCESS(Status)) {
            HeapFree(hHeap, NULL, *ppBuffer);
            retval = Status;
        }
    } while (Status == STATUS_INFO_LENGTH_MISMATCH);

    if (hNTDll) {
        FreeLibrary(hNTDll);
    }

    return retval;
}


// Enumerate the running threads in the process space and add them to
//   the list.  This only works on NT 4.0 based machines.  This also
//   includes additional information which can be logged during a crash
//   event.
int diagnostics_update_thread_list_NT() {
    DWORD                   dwCurrentProcessId = GetCurrentProcessId();
    HANDLE                  hThread = NULL;
    PBOINC_THREADLISTENTRY  pThreadEntry = NULL;
    ULONG                   cbBuffer = 32*1024;    // 32k initial buffer
    PVOID                   pBuffer = NULL;
    PSYSTEM_PROCESSES_NT4   pProcesses = NULL;
    PSYSTEM_THREADS         pThread = NULL;
    UINT                    uiSystemIndex = 0;
    UINT                    uiInternalIndex = 0;
    UINT                    uiInternalCount = 0;
    HMODULE                 hKernel32;
    tOT                     pOT = NULL;


    // Dynamically link to the proper function pointers.
    hKernel32 = LoadLibrary("kernel32.dll");
    pOT = (tOT) GetProcAddress( hKernel32, "OpenThread" );

    // Get a snapshot of the process and thread information.
    diagnostics_get_process_information(&pBuffer, &cbBuffer);

    // Wait for the ThreadListSync mutex before writing updates
    WaitForSingleObject(hThreadListSync, INFINITE);

    // Lets start walking the structures to find the good stuff.
    pProcesses = (PSYSTEM_PROCESSES_NT4)pBuffer;
    do {
        // Okay, found the current procceses entry now we just need to
        //   update the thread data.
        if (pProcesses->ProcessId == dwCurrentProcessId) {

            // Enumerate the threads
            for(uiSystemIndex = 0; uiSystemIndex < pProcesses->ThreadCount; uiSystemIndex++) {
                pThread = &pProcesses->Threads[uiSystemIndex];
                pThreadEntry = diagnostics_find_thread_entry(pThread->ClientId.UniqueThread);

                if (pThreadEntry) {
                    pThreadEntry->crash_kernel_time = (FLOAT)pThread->KernelTime.QuadPart;
                    pThreadEntry->crash_user_time = (FLOAT)pThread->UserTime.QuadPart;
                    pThreadEntry->crash_wait_time = (FLOAT)pThread->WaitTime;
                    pThreadEntry->crash_priority = pThread->Priority;
                    pThreadEntry->crash_base_priority = pThread->BasePriority;
                    pThreadEntry->crash_state = pThread->State;
                    pThreadEntry->crash_wait_reason = pThread->WaitReason;
                } else {
                    if (pOT) {
                        hThread = pOT(
                            THREAD_ALL_ACCESS,
                            FALSE,
                            pThread->ClientId.UniqueThread
                        );
                    }

                    pThreadEntry = new BOINC_THREADLISTENTRY;
                    diagnostics_init_thread_entry(pThreadEntry);
                    pThreadEntry->name = "";
                    pThreadEntry->thread_id = pThread->ClientId.UniqueThread;
                    pThreadEntry->thread_handle = hThread;
                    pThreadEntry->crash_kernel_time = (FLOAT)pThread->KernelTime.QuadPart;
                    pThreadEntry->crash_user_time = (FLOAT)pThread->UserTime.QuadPart;
                    pThreadEntry->crash_wait_time = (FLOAT)pThread->WaitTime;
                    pThreadEntry->crash_priority = pThread->Priority;
                    pThreadEntry->crash_base_priority = pThread->BasePriority;
                    pThreadEntry->crash_state = pThread->State;
                    pThreadEntry->crash_wait_reason = pThread->WaitReason;
                    diagnostics_threads.push_back(pThreadEntry);
                }
            }
        }

        // Move to the next structure if one exists
        if (!pProcesses->NextEntryDelta) {
            break;
        }
        pProcesses = (PSYSTEM_PROCESSES_NT4)(((LPBYTE)pProcesses) + pProcesses->NextEntryDelta);
    } while (pProcesses);

    // Release resources
    if (hThreadListSync) ReleaseMutex(hThreadListSync);
    if (hKernel32) CloseHandle(hKernel32);
    if (pBuffer) HeapFree(GetProcessHeap(), NULL, pBuffer);

    return 0;
}


// Enumerate the running threads in the process space and add them to
//   the list.  This only works on XP or better based machines.  This also
//   includes additional information which can be logged during a crash
//   event.
int diagnostics_update_thread_list_XP() {
    DWORD                   dwCurrentProcessId = GetCurrentProcessId();
    HANDLE                  hThread = NULL;
    PBOINC_THREADLISTENTRY  pThreadEntry = NULL;
    ULONG                   cbBuffer = 32*1024;    // 32k initial buffer
    PVOID                   pBuffer = NULL;
    PSYSTEM_PROCESSES       pProcesses = NULL;
    PSYSTEM_THREADS         pThread = NULL;
    UINT                    uiSystemIndex = 0;
    UINT                    uiInternalIndex = 0;
    UINT                    uiInternalCount = 0;
    HMODULE                 hKernel32;
    tOT                     pOT = NULL;


    // Dynamically link to the proper function pointers.
    hKernel32 = LoadLibrary("kernel32.dll");
    pOT = (tOT) GetProcAddress( hKernel32, "OpenThread" );

    // Get a snapshot of the process and thread information.
    diagnostics_get_process_information(&pBuffer, &cbBuffer);

    // Wait for the ThreadListSync mutex before writing updates
    WaitForSingleObject(hThreadListSync, INFINITE);

    // Lets start walking the structures to find the good stuff.
    pProcesses = (PSYSTEM_PROCESSES)pBuffer;
    do {
        // Okay, found the current procceses entry now we just need to
        //   update the thread data.
        if (pProcesses->ProcessId == dwCurrentProcessId) {
            // Enumerate the threads
            for(uiSystemIndex = 0; uiSystemIndex < pProcesses->ThreadCount; uiSystemIndex++) {
                pThread = &pProcesses->Threads[uiSystemIndex];
                pThreadEntry = diagnostics_find_thread_entry(pThread->ClientId.UniqueThread);

                if (pThreadEntry) {
                    pThreadEntry->crash_kernel_time = (FLOAT)pThread->KernelTime.QuadPart;
                    pThreadEntry->crash_user_time = (FLOAT)pThread->UserTime.QuadPart;
                    pThreadEntry->crash_wait_time = (FLOAT)pThread->WaitTime;
                    pThreadEntry->crash_priority = pThread->Priority;
                    pThreadEntry->crash_base_priority = pThread->BasePriority;
                    pThreadEntry->crash_state = pThread->State;
                    pThreadEntry->crash_wait_reason = pThread->WaitReason;
                } else {
                    if (pOT) {
                        hThread = pOT(
                            THREAD_ALL_ACCESS,
                            FALSE,
                            pThread->ClientId.UniqueThread
                        );
                    }

                    pThreadEntry = new BOINC_THREADLISTENTRY;
                    diagnostics_init_thread_entry(pThreadEntry);
                    pThreadEntry->name = "";
                    pThreadEntry->thread_id = pThread->ClientId.UniqueThread;
                    pThreadEntry->thread_handle = hThread;
                    pThreadEntry->crash_kernel_time = (FLOAT)pThread->KernelTime.QuadPart;
                    pThreadEntry->crash_user_time = (FLOAT)pThread->UserTime.QuadPart;
                    pThreadEntry->crash_wait_time = (FLOAT)pThread->WaitTime;
                    pThreadEntry->crash_priority = pThread->Priority;
                    pThreadEntry->crash_base_priority = pThread->BasePriority;
                    pThreadEntry->crash_state = pThread->State;
                    pThreadEntry->crash_wait_reason = pThread->WaitReason;
                    diagnostics_threads.push_back(pThreadEntry);
                }
            }
        }

        // Move to the next structure if one exists
        if (!pProcesses->NextEntryDelta) {
            break;
        }
        pProcesses = (PSYSTEM_PROCESSES)(((LPBYTE)pProcesses) + pProcesses->NextEntryDelta);
    } while (pProcesses);

    // Release resources
    if (hThreadListSync) ReleaseMutex(hThreadListSync);
    if (hKernel32) CloseHandle(hKernel32);
    if (pBuffer) HeapFree(GetProcessHeap(), NULL, pBuffer);

    return 0;
}


// Determine which update thread list function to call based on OS
//   version.
int diagnostics_update_thread_list() {
    int retval = 0;

    // Detect platform information
    OSVERSIONINFO osvi; 
    osvi.dwOSVersionInfoSize = sizeof(osvi);
    GetVersionEx(&osvi);

    switch(osvi.dwPlatformId) {
        case VER_PLATFORM_WIN32_WINDOWS:
            // Win95, Win98, WinME
            retval = diagnostics_update_thread_list_9X();
            break;
        case VER_PLATFORM_WIN32_NT:
            switch(osvi.dwMajorVersion) {
                case 4:
                    // WinNT 4.0
                    retval = diagnostics_update_thread_list_NT();
                    break;
                case 5:
                    // Win2k, WinXP, Win2k3
                    retval = diagnostics_update_thread_list_XP();
                    break;
                case 6:
                    if (osvi.dwMinorVersion == 0) {
                        // WinVista
                        retval = diagnostics_update_thread_list_XP();
                    } else {
                        // In cases where we do not know if the interfaces have
                        //   changed from the ones we know about, just default to
                        //   the most compatible implementation.
                        retval = diagnostics_update_thread_list_9X();
                    }
                    break;
                default:
                    // In cases where we do not know if the interfaces have
                    //   changed from the ones we know about, just default to
                    //   the most compatible implementation.
                    retval = diagnostics_update_thread_list_9X();
                    break;
            }
            break;
    }

    return retval;
}


// Set the current threads name to make it easy to know what the
//   thread is supposed to be doing.
int diagnostics_set_thread_name( char* name ) {
    HANDLE hThread;
    PBOINC_THREADLISTENTRY pThreadEntry = NULL;


    // Wait for the ThreadListSync mutex before writing updates
    WaitForSingleObject(hThreadListSync, INFINITE);

    pThreadEntry = diagnostics_find_thread_entry(GetCurrentThreadId());

    // If we already know about the thread, just set its name.  Otherwise
    //   create a new entry and then set the name to the new entry.
    if (pThreadEntry) {
        pThreadEntry->name = name;
    } else {
        DuplicateHandle(
            GetCurrentProcess(),
            GetCurrentThread(),
            GetCurrentProcess(),
            &hThread,
            0,
            FALSE,
            DUPLICATE_SAME_ACCESS
        );

        pThreadEntry = new BOINC_THREADLISTENTRY;
        diagnostics_init_thread_entry(pThreadEntry);
        pThreadEntry->name = name;
        pThreadEntry->thread_id = GetCurrentThreadId();
        pThreadEntry->thread_handle = hThread;
        diagnostics_threads.push_back(pThreadEntry);
    }

    // Release the Mutex
    ReleaseMutex(hThreadListSync);

    return 0;
}


// Set the current threads name to make it easy to know what the
//   thread is supposed to be doing.
int diagnostics_set_thread_exception_record(PEXCEPTION_POINTERS pExPtrs) {
    PBOINC_THREADLISTENTRY pThreadEntry = NULL;

    // Wait for the ThreadListSync mutex before writing updates
    WaitForSingleObject(hThreadListSync, INFINITE);

    pThreadEntry = diagnostics_find_thread_entry(GetCurrentThreadId());
    if (pThreadEntry) {
        pThreadEntry->crash_exception_record = pExPtrs;
    }

    // Release the Mutex
    ReleaseMutex(hThreadListSync);

    return 0;
}


// Set the current threads name to make it easy to know what the
//   thread is supposed to be doing.
int diagnostics_set_thread_exempt_suspend() {
    PBOINC_THREADLISTENTRY pThreadEntry = NULL;

    // Wait for the ThreadListSync mutex before writing updates
    WaitForSingleObject(hThreadListSync, INFINITE);

    pThreadEntry = diagnostics_find_thread_entry(GetCurrentThreadId());
    if (pThreadEntry) {
        pThreadEntry->crash_suspend_exempt = TRUE;
    }

    // Release the Mutex
    ReleaseMutex(hThreadListSync);

    return 0;
}


// Set the current thread as the graphics thread.
//
int diagnostics_set_thread_graphics() {
    PBOINC_THREADLISTENTRY pThreadEntry = NULL;

    // Wait for the ThreadListSync mutex before writing updates
    WaitForSingleObject(hThreadListSync, INFINITE);

    pThreadEntry = diagnostics_find_thread_entry(GetCurrentThreadId());
    if (pThreadEntry) {
        pThreadEntry->graphics_thread = TRUE;
    }

    // Release the Mutex
    ReleaseMutex(hThreadListSync);

    return 0;
}


// Set the current thread as the worker thread.
//
int diagnostics_set_thread_worker() {
    PBOINC_THREADLISTENTRY pThreadEntry = NULL;

    // Wait for the ThreadListSync mutex before writing updates
    WaitForSingleObject(hThreadListSync, INFINITE);

    pThreadEntry = diagnostics_find_thread_entry(GetCurrentThreadId());
    if (pThreadEntry) {
        pThreadEntry->worker_thread = TRUE;
    }

    // Release the Mutex
    ReleaseMutex(hThreadListSync);

    return 0;
}


// Translate the thread state into a human readable form.
//
// See: http://support.microsoft.com/?kbid=837372
//
char* diagnostics_format_thread_state(int thread_state) {
    switch(thread_state) {
        case ThreadStateInitialized:
            return "Initialized";
            break;
        case ThreadStateReady:
            return "Ready";
            break;
        case ThreadStateRunning:
            return "Running";
            break;
        case ThreadStateStandby:
            return "Standby";
            break;
        case ThreadStateTerminated:
            return "Terminated";
            break;
        case ThreadStateWaiting:
            return "Waiting";
            break;
        case ThreadStateTransition:
            return "Transition";
            break;
        default: 
            return "Unknown";
            break;
    }
    return "";
}


// Translate the thread wait reason into a human readable form.
//
// See: http://support.microsoft.com/?kbid=837372
//
char* diagnostics_format_thread_wait_reason(int thread_wait_reason) {
    switch(thread_wait_reason) {
        case ThreadWaitReasonExecutive:
            return "Executive";
            break;
        case ThreadWaitReasonFreePage:
            return "FreePage";
            break;
        case ThreadWaitReasonPageIn:
            return "PageIn";
            break;
        case ThreadWaitReasonPoolAllocation:
            return "PoolAllocation";
            break;
        case ThreadWaitReasonDelayExecution:
            return "ExecutionDelay";
            break;
        case ThreadWaitReasonSuspended:
            return "Suspended";
            break;
        case ThreadWaitReasonUserRequest:
            return "UserRequest";
            break;
        case ThreadWaitReasonWrExecutive:
            return "Executive";
            break;
        case ThreadWaitReasonWrFreePage:
            return "FreePage";
            break;
        case ThreadWaitReasonWrPageIn:
            return "PageIn";
            break;
        case ThreadWaitReasonWrPoolAllocation:
            return "PoolAllocation";
            break;
        case ThreadWaitReasonWrDelayExecution:
            return "ExecutionDelay";
            break;
        case ThreadWaitReasonWrSuspended:
            return "Suspended";
            break;
        case ThreadWaitReasonWrUserRequest:
            return "UserRequest";
            break;
        case ThreadWaitReasonWrEventPairHigh:
            return "EventPairHigh";
            break;
        case ThreadWaitReasonWrEventPairLow:
            return "EventPairLow";
            break;
        case ThreadWaitReasonWrLpcReceive:
            return "LPCReceive";
            break;
        case ThreadWaitReasonWrLpcReply:
            return "LPCReply";
            break;
        case ThreadWaitReasonWrVirtualMemory:
            return "VirtualMemory";
            break;
        case ThreadWaitReasonWrPageOut:
            return "PageOut";
            break;
        default:
            return "Unknown";
            break;
    }
    return "";
}


// Provide a mechinism to trap and report messages sent to the debugger's
//   viewport.  This should only been enabled if a debugger isn't running
//   against the current process already.
//
// Documentation about the protocol can be found here:
//   http://www.unixwiz.net/techtips/outputdebugstring.html
//

typedef struct _DEBUGGERMESSAGE {
        DWORD   dwProcessId;
        char    data[4096 - sizeof(DWORD)];
} DEBUGGERMESSAGE, *PDEBUGGERMESSAGE;

typedef struct _BOINC_MESSAGEMONITORENTRY {
    double      timestamp;
    std::string message;
} BOINC_MESSAGEMONITORENTRY, *PBOINC_MESSAGEMONITORENTRY;

static std::vector<PBOINC_MESSAGEMONITORENTRY> diagnostics_monitor_messages;
static PDEBUGGERMESSAGE pMessageBuffer;
static HANDLE hMessageMonitorThread;
static HANDLE hMessageMonitorSync;
static HANDLE hMessageSharedMap;
static HANDLE hMessageAckEvent;
static HANDLE hMessageReadyEvent;
static HANDLE hMessageQuitEvent;
static HANDLE hMessageQuitFinishedEvent;


// Initialize the needed structures and startup the message processing thread.
//
int diagnostics_init_message_monitor() {
    int retval = 0;
    unsigned int i;
    DWORD  dwThreadId;
    PBOINC_MESSAGEMONITORENTRY pMessageEntry = NULL;
    std::vector<PBOINC_MESSAGEMONITORENTRY>::iterator message_iter;

    SECURITY_ATTRIBUTES sa;
    SECURITY_DESCRIPTOR sd;

    sa.nLength = sizeof(SECURITY_ATTRIBUTES);
    sa.bInheritHandle = TRUE;
    sa.lpSecurityDescriptor = &sd;


    // Create a mutex that can be used to syncronize data access
    //   to the global thread list.
    hMessageMonitorSync = CreateMutex(NULL, TRUE, NULL);

    // Clear out any previous messages.
    for (i=0; i<diagnostics_monitor_messages.size(); i++) {
        pMessageEntry = diagnostics_monitor_messages[0];

        message_iter = diagnostics_monitor_messages.begin();
        diagnostics_monitor_messages.erase(message_iter);

        if (pMessageEntry) delete pMessageEntry;
    }


    if (!IsDebuggerPresent()) {

        InitializeSecurityDescriptor(&sd, SECURITY_DESCRIPTOR_REVISION);
        SetSecurityDescriptorDacl(&sd, TRUE, (PACL)NULL, FALSE);

        hMessageAckEvent = CreateEvent(&sa, FALSE, FALSE, "DBWIN_BUFFER_READY");
        hMessageReadyEvent = CreateEvent(&sa, FALSE, FALSE, "DBWIN_DATA_READY");
        hMessageQuitEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
        hMessageQuitFinishedEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

        hMessageSharedMap = CreateFileMapping(
            INVALID_HANDLE_VALUE,    // use paging file
            &sa,                     // default security 
            PAGE_READWRITE,          // read/write access
            0,                       // max. object size 
            sizeof(DEBUGGERMESSAGE), // buffer size  
            "DBWIN_BUFFER"           // name of mapping object
        );

        pMessageBuffer = (PDEBUGGERMESSAGE)MapViewOfFile(
            hMessageSharedMap,
            FILE_MAP_READ | FILE_MAP_WRITE,
            0,                       // file offset high
            0,                       // file offset low
            sizeof(DEBUGGERMESSAGE)  // # of bytes to map (entire file)
        );

        hMessageMonitorThread = CreateThread(
            NULL,
            0,
            diagnostics_message_monitor,
            0,
            0,
            &dwThreadId
        );
    } else {
        retval = ERROR_NOT_SUPPORTED;
    }


    // Release the Mutex
    ReleaseMutex(hMessageMonitorSync);

    return retval;
}


// Shutdown the message monitoring thread and cleanup any of the in memory
//   structures.
int diagnostics_finish_message_monitor() {
    unsigned int i;
    PBOINC_MESSAGEMONITORENTRY pMessageEntry = NULL;
    std::vector<PBOINC_MESSAGEMONITORENTRY>::iterator message_iter;

    // Begin the cleanup process by means of shutting down the
    //   message monitoring thread.
    SetEvent(hMessageQuitEvent);

    // Wait until it is message monitoring thread is shutdown before
    //   cleaning up the structure since we'll need to aquire the
    //   MessageMonitorSync mutex.
    WaitForSingleObject(hMessageQuitFinishedEvent, INFINITE);
    WaitForSingleObject(hMessageMonitorSync, INFINITE);
  

    // Now clean up everything we can.
    //

    // Clear out any previous messages.
    for (i=0; i<diagnostics_monitor_messages.size(); i++) {
        pMessageEntry = diagnostics_monitor_messages[0];

        message_iter = diagnostics_monitor_messages.begin();
        diagnostics_monitor_messages.erase(message_iter);

        if (pMessageEntry) delete pMessageEntry;
    }


    // Cleanup the handles
    if (pMessageBuffer) UnmapViewOfFile(pMessageBuffer);
    if (hMessageSharedMap) CloseHandle(hMessageSharedMap);
    if (hMessageAckEvent) CloseHandle(hMessageAckEvent);
    if (hMessageReadyEvent) CloseHandle(hMessageReadyEvent);
    if (hMessageQuitEvent) CloseHandle(hMessageQuitEvent);
    if (hMessageQuitFinishedEvent) CloseHandle(hMessageQuitFinishedEvent);
    if (hMessageMonitorThread) CloseHandle(hMessageMonitorThread);

    // Release and close the mutex.
    if (hMessageMonitorSync) CloseHandle(hMessageMonitorSync);

    return 0;
}


int diagnostics_message_monitor_dump() {
    unsigned int i;
    PBOINC_MESSAGEMONITORENTRY pMessageEntry = NULL;

    // Wait for the MessageMonitorSync mutex before writing updates
    WaitForSingleObject(hMessageMonitorSync, INFINITE);

    fprintf(stderr, "\n*** Debug Message Dump ****\n");
    
    // Clear out any previous messages.
    for (i=0; i<diagnostics_monitor_messages.size(); i++) {
        pMessageEntry = diagnostics_monitor_messages[i];
        fprintf(
            stderr, 
            "[%s] %s",
            time_to_string(pMessageEntry->timestamp),
            pMessageEntry->message.c_str()
        );
    }

    fprintf(stderr, "\n\n");

    // Release the Mutex
    ReleaseMutex(hMessageMonitorSync);

    return 0;
}


// This thread monitors the shared memory buffer used to pass debug messages
//   around.  due to an anomaly in the Windows debug environment it is
//   suggested that a sleep(0) be introduced before any 
//   SetEvent/ResetEvent/PulseEvent function is called.
//
// See: http://support.microsoft.com/kb/q173260/
//
DWORD WINAPI diagnostics_message_monitor(LPVOID lpParameter) {
    DWORD       dwEvent = NULL;
    DWORD       dwCurrentProcessId = NULL;
    BOOL        bContinue = TRUE;
    DWORD       dwRepeatMessageCounter = 0;
    DWORD       dwRepeatMessageProcessId = 0;
    std::string strRepeatMessage;
    PBOINC_MESSAGEMONITORENTRY pMessageEntry = NULL;
    std::vector<PBOINC_MESSAGEMONITORENTRY>::iterator message_iter;
    HANDLE      hEvents[2];

    // Set our friendly name
    diagnostics_set_thread_name("Debug Message Monitor");
    diagnostics_set_thread_exempt_suspend();


    // Which events do we want to wait for?
    hEvents[0] = hMessageQuitEvent;
    hEvents[1] = hMessageReadyEvent;

    // Cache the current process id
    dwCurrentProcessId = GetCurrentProcessId();

    // Signal that the buffer is ready for action.
    Sleep(0);
    SetEvent(hMessageAckEvent);

    while (bContinue) {
        dwEvent = WaitForMultipleObjects( 
            2,           // number of objects in array
            hEvents,     // array of objects
            FALSE,       // wait for any
            INFINITE     // wait
        );
        switch(dwEvent) {
            // hMessageQuitEvent was signaled.
            case WAIT_OBJECT_0 + 0:

                // We are shutting down so lets cleanup and exit.
                bContinue = false;
                
                break;

            // hMessageReadyEvent was signaled.
            case WAIT_OBJECT_0 + 1:
                // The debugger protocol assumes that only one debugger is going
                //   to exist on the system, but we are only interested in messages
                //   from the current process id.  Since we are here we can assume
                //   that no debugger was present when the application was launched
                //   so we can safely ignore messages that didn't come from us
                //   because that means they are from another application.
                //
                // If we detect a message from a different process just ignore it
                //   and re-signal the event.  We'll go to sleep for 100 milliseconds
                //   and let the other BOINC based applications have a shot at it.
                //
                // If we see the same message four times that means it is from an
                //   application that doesn't understand our modificatons, so we'll
                //   process the message just like we were a regular debugger and
                //   signal that the buffer is available again.
                if (dwCurrentProcessId != pMessageBuffer->dwProcessId) {
                    // Message from a different process.

                    // Is this the same message as before?
                    if ((dwRepeatMessageProcessId != pMessageBuffer->dwProcessId) || 
                        (strRepeatMessage != pMessageBuffer->data)
                    ) {
                        dwRepeatMessageCounter = 0;

                        // Cache the data for future checks.
                        dwRepeatMessageProcessId = pMessageBuffer->dwProcessId;
                        strRepeatMessage = pMessageBuffer->data;
                    } else {
                        dwRepeatMessageCounter++;
                    }

                    if (dwRepeatMessageCounter > 4) {
                        // Buffer is ready to receive a new message.
                        Sleep(0);
                        SetEvent(hMessageAckEvent);

                        dwRepeatMessageCounter = 0;
                        dwRepeatMessageProcessId = 0;
                        strRepeatMessage = "";
                    } else {
                        // Let another application have a go at the message.
                        Sleep(0);
                        SetEvent(hMessageReadyEvent);
                        Sleep(100);
                    }
                } else {
                    // A message for us to process
                    pMessageEntry = new BOINC_MESSAGEMONITORENTRY;
                    pMessageEntry->timestamp = dtime();
                    pMessageEntry->message = pMessageBuffer->data;

                    // Wait for the MessageMonitorSync mutex before writing updates
                    WaitForSingleObject(hMessageMonitorSync, INFINITE);

                    diagnostics_monitor_messages.push_back(pMessageEntry);

                    // Trim back the number of messages in memory
                    if (diagnostics_monitor_messages.size() > 50) {
                        pMessageEntry = diagnostics_monitor_messages[0];

                        message_iter = diagnostics_monitor_messages.begin();
                        diagnostics_monitor_messages.erase(message_iter);

                        delete pMessageEntry;
                    }

                    // Release the Mutex
                    ReleaseMutex(hMessageMonitorSync);

                    // Clear out the old message
                    ZeroMemory(pMessageBuffer, sizeof(DEBUGGERMESSAGE));

                    // Buffer is ready to receive a new message.
                    Sleep(0);
                    SetEvent(hMessageAckEvent);
                }
                break;
        }
    }

    // Notify the calling thread that the message monitoring thread is
    //   finished.
    SetEvent(hMessageQuitFinishedEvent);
    return 0;
}


// Structured Exceptions are Windows primary mechanism for dealing with
//   badly behaved applications or applications where something bad has
//   happened underneath them and they need to clean up after themselves.
//
// Applications can define an unhandled exception filter to handle any
//   exception event that Windows will throw.  If you leave things to
//   the OS defaults, you'll end up with the annoying Windows Error
//   Reporting dialog and they user will be asked if they want to report
//   the crash to Microsoft.  Most of the time this is okay for regular
//   applications, but for BOINC based applications this is really bad.
//
// BOINC based applications need to be completely autonomous.  Unhandled
//   exceptions are caught and we dump as much information, about what
//   has happened, to stderr so that project administrators can look at
//   it and fix whatever bug might have caused the event.
//
// To accomplish this BOINC starts up a thread that will handle any
//   unhandled exceptions when one is detected.  By using a separate
//   thread the runtime debugger can avoid stack corruption issues and
//   multiple unhandled exceptions.  In a multi-processor system it is
//   possible that both the graphics thread and the worker threads would
//   be referencing the same corrupted area of memory.  Previous
//   implementations of the runtime debugger would have just terminated
//   the process believing it was a nested unhandled exception instead
//   of believing it to be two seperate exceptions thrown from different
//   threads.
//

// This structure is used to keep track of stuff nessassary
//   to dump information about the top most window during
//   a crash event.
typedef struct _BOINC_WINDOWCAPTURE {
    HWND         hwnd;
    char         window_name[256];
    char         window_class[256];
    DWORD        window_process_id;
    DWORD        window_thread_id;
} BOINC_WINDOWCAPTURE, *PBOINC_WINDOWCAPTURE;

static HANDLE hExceptionMonitorThread;
static HANDLE hExceptionMonitorHalt;
static HANDLE hExceptionDetectedEvent;
static HANDLE hExceptionQuitEvent;
static HANDLE hExceptionQuitFinishedEvent;

// Initialize the needed structures and startup the unhandled exception
//   monitor thread.
int diagnostics_init_unhandled_exception_monitor() {
    int retval = 0;
    DWORD  dwThreadId;

    // Create a mutex that can be used to put any thread that has thrown
    //   an unhandled exception to sleep.
    hExceptionMonitorHalt = CreateMutex(NULL, FALSE, NULL);

    // The following event is thrown by a thread that has experienced an
    //   unhandled exception after storing its exception record but before
    //   it attempts to aquire the halt mutex.
    hExceptionDetectedEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

    // Create an event that we can use to shutdown the unhandled exception
    //   monitoring thread.
    hExceptionQuitEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    hExceptionQuitFinishedEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

    // Create the thread that is going to monitor any unhandled exceptions
    hExceptionMonitorThread = CreateThread(
        NULL,
        0,
        diagnostics_unhandled_exception_monitor,
        0,
        0,
        &dwThreadId
    );

    return retval;
}


// Shutdown the unhandled exception monitoring thread and cleanup any
//   of the in memory structures.
int diagnostics_finish_unhandled_exception_monitor() {

    // Begin the cleanup process by means of shutting down the
    //   message monitoring thread.
    SetEvent(hExceptionQuitEvent);

    // Wait until it is message monitoring thread is shutdown before
    //   cleaning up the structure since we'll need to aquire the
    //   MessageMonitorSync mutex.
    WaitForSingleObject(hExceptionQuitFinishedEvent, INFINITE);

    // Cleanup the handles
    if (hExceptionDetectedEvent) CloseHandle(hExceptionDetectedEvent);
    if (hExceptionQuitEvent) CloseHandle(hExceptionQuitEvent);
    if (hExceptionQuitFinishedEvent) CloseHandle(hExceptionQuitFinishedEvent);
    if (hExceptionMonitorHalt) CloseHandle(hExceptionMonitorHalt);
    if (hExceptionMonitorThread) CloseHandle(hExceptionMonitorThread);

    return 0;
}


// Dump crash header information
//
int diagnostics_unhandled_exception_dump_banner() {
    char pszTemp[11];
    char pszTemp2[11];

    _strdate(pszTemp);
    _strtime(pszTemp2);

    fprintf( stderr, "\n\n");
    fprintf( stderr, "**********\n");
    fprintf( stderr, "**********\n");
    fprintf( stderr, "\n");
    fprintf( stderr, "BOINC Windows Runtime Debugger Version %s\n", BOINC_VERSION_STRING);
    fprintf( stderr, "\n\n");
    fprintf( stderr, "Dump Timestamp    : ");
    fprintf( stderr, "%s %s", pszTemp, pszTemp2 );
    fprintf( stderr, "\n");

    return 0;
}

// Capture the foreground window details for future use.
//
int diagnostics_capture_foreground_window(PBOINC_WINDOWCAPTURE window_info) {

    window_info->hwnd = GetForegroundWindow();

    GetWindowText(
        window_info->hwnd, 
        window_info->window_name,
        sizeof(window_info->window_name)
    );

    GetClassName(
        window_info->hwnd,
        window_info->window_class,
        sizeof(window_info->window_class)
    );

    window_info->window_thread_id = GetWindowThreadProcessId(
        window_info->hwnd,
        &window_info->window_process_id
    );

    return 0;
}


// Dump the foreground window details to stderr.
//
int diagnostics_foreground_window_dump(PBOINC_WINDOWCAPTURE window_info) {

    fprintf(
        stderr,
        "*** Foreground Window Data ***\n"
        "    Window Name      : %s\n"
        "    Window Class     : %s\n"
        "    Window Process ID: %x\n"
        "    Window Thread ID : %x\n\n",
        window_info->window_name,
        window_info->window_class,
        window_info->window_process_id,
        window_info->window_thread_id
    );

    return 0;
}


// Dump the captured information for a given thread.
//
int diagnostics_dump_thread_information(PBOINC_THREADLISTENTRY pThreadEntry) {
    std::string strThreadStatus;
    std::string strThreadWaitReason;

    strThreadStatus = diagnostics_format_thread_state(pThreadEntry->crash_state);

    if (pThreadEntry->crash_state == ThreadStateWaiting) {
        strThreadWaitReason += "Wait Reason: ";
        strThreadWaitReason += diagnostics_format_thread_state(pThreadEntry->crash_wait_reason);
        strThreadWaitReason += ", ";

    }

    fprintf(
        stderr, 
        "*** Dump of the %s thread (%x): ***\n\n"
        "- Information -\n"
        "Status: %s, "
        "%s"
        "Kernel Time: %f, "
        "User Time: %f, "
        "Wait Time: %f\n"
        "\n",
        pThreadEntry->name.c_str(),
        pThreadEntry->thread_id,
        strThreadStatus.c_str(),
        strThreadWaitReason.c_str(),
        pThreadEntry->crash_kernel_time,
        pThreadEntry->crash_user_time,
        pThreadEntry->crash_wait_time
    );

    return 0;
}


// Dump the exception code record to stderr in a human readable form.
//
int diagnostics_dump_exception_record(PEXCEPTION_POINTERS pExPtrs) {
    char  status[256];
    char  substatus[256];
    PVOID exceptionAddr = pExPtrs->ExceptionRecord->ExceptionAddress;
    DWORD exceptionCode = pExPtrs->ExceptionRecord->ExceptionCode;

    switch (exceptionCode) {
        case EXCEPTION_ACCESS_VIOLATION:
            strcpy(status, "Access Violation");
            if (pExPtrs->ExceptionRecord->NumberParameters == 2) {
                switch(pExPtrs->ExceptionRecord->ExceptionInformation[0]) {
                case 0: // read attempt
                    sprintf(substatus, "read attempt to address 0x%8.8X", pExPtrs->ExceptionRecord->ExceptionInformation[1]);
                    break;
                case 1: // write attempt
                    sprintf(substatus, "write attempt to address 0x%8.8X", pExPtrs->ExceptionRecord->ExceptionInformation[1]);
                    break;
                }
            }
            break;
        case EXCEPTION_DATATYPE_MISALIGNMENT:
            strcpy(status, "Data Type Misalignment");
            break;
        case EXCEPTION_BREAKPOINT:
            strcpy(status, "Breakpoint Encountered");
            break;
        case EXCEPTION_SINGLE_STEP:
            strcpy(status, "Single Instruction Executed");
            break;
        case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:
            strcpy(status, "Array Bounds Exceeded");
            break;
        case EXCEPTION_FLT_DENORMAL_OPERAND:
            strcpy(status, "Float Denormal Operand");
            break;
        case EXCEPTION_FLT_DIVIDE_BY_ZERO:
            strcpy(status, "Divide by Zero");
            break;
        case EXCEPTION_FLT_INEXACT_RESULT:
            strcpy(status, "Float Inexact Result");
            break;
        case EXCEPTION_FLT_INVALID_OPERATION:
            strcpy(status, "Float Invalid Operation");
            break;
        case EXCEPTION_FLT_OVERFLOW:
            strcpy(status, "Float Overflow");
            break;
        case EXCEPTION_FLT_STACK_CHECK:
            strcpy(status, "Float Stack Check");
            break;
        case EXCEPTION_FLT_UNDERFLOW:
            strcpy(status, "Float Underflow");
            break;
        case EXCEPTION_INT_DIVIDE_BY_ZERO:
            strcpy(status, "Integer Divide by Zero");
            break;
        case EXCEPTION_INT_OVERFLOW:
            strcpy(status, "Integer Overflow");
            break;
        case EXCEPTION_PRIV_INSTRUCTION:
            strcpy(status, "Privileged Instruction");
            break;
        case EXCEPTION_IN_PAGE_ERROR:
            strcpy(status, "In Page Error");
            break;
        case EXCEPTION_ILLEGAL_INSTRUCTION:
            strcpy(status, "Illegal Instruction");
            break;
        case EXCEPTION_NONCONTINUABLE_EXCEPTION:
            strcpy(status, "Noncontinuable Exception");
            break;
        case EXCEPTION_STACK_OVERFLOW:
            strcpy(status, "Stack Overflow");
            break;
        case EXCEPTION_INVALID_DISPOSITION:
            strcpy(status, "Invalid Disposition");
            break;
        case EXCEPTION_GUARD_PAGE:
            strcpy(status, "Guard Page Violation");
            break;
        case EXCEPTION_INVALID_HANDLE:
            strcpy(status, "Invalid Handle");
            break;
        case CONTROL_C_EXIT:
            strcpy(status, "Ctrl+C Exit");
            break;
        default:
            strcpy(status, "Unknown exception");
            break;
    }

    fprintf(stderr, "- Unhandled Exception Record -\n");
    if (EXCEPTION_ACCESS_VIOLATION == exceptionCode) {
        fprintf(stderr, "Reason: %s (0x%x) at address 0x%p %s\n\n", status, exceptionCode, exceptionAddr, substatus);
    } else {
        fprintf(stderr, "Reason: %s (0x%x) at address 0x%p\n\n", status, exceptionCode, exceptionAddr);
    }

    return 0;
}


// Priority is given to the worker threads exception code, and then the
//   graphics thread.  If neither of those two threw the exception grab
//   the exception code of the thread that did.
UINT diagnostics_determine_exit_code() {
    UINT   uiReturn = 0;
    UINT   uiIndex = 0;
    size_t size = 0;

    // Worker thread.
    size = diagnostics_threads.size();
    for (uiIndex = 0; uiIndex < size; uiIndex++) {
        if (diagnostics_threads[uiIndex]) {
            if (diagnostics_threads[uiIndex]->worker_thread && diagnostics_threads[uiIndex]->crash_exception_record) {
                uiReturn = 
                    diagnostics_threads[uiIndex]->crash_exception_record->ExceptionRecord->ExceptionCode;
            }
        }
    }

    // Graphics thread
    if (!uiReturn) {
        size = diagnostics_threads.size();
        for (uiIndex = 0; uiIndex < size; uiIndex++) {
            if (diagnostics_threads[uiIndex]) {
                if (diagnostics_threads[uiIndex]->graphics_thread && diagnostics_threads[uiIndex]->crash_exception_record) {
                    uiReturn = 
                        diagnostics_threads[uiIndex]->crash_exception_record->ExceptionRecord->ExceptionCode;
                }
            }
        }

        // Any thread will do at this point
        if (!uiReturn) {
            size = diagnostics_threads.size();
            for (uiIndex = 0; uiIndex < size; uiIndex++) {
                if (diagnostics_threads[uiIndex]->crash_exception_record) {
                    uiReturn = 
                        diagnostics_threads[uiIndex]->crash_exception_record->ExceptionRecord->ExceptionCode;
                }
            }
        }
    }

    return uiReturn;
}


DWORD WINAPI diagnostics_unhandled_exception_monitor(LPVOID lpParameter) {
    DWORD        dwEvent = NULL;
    BOOL         bContinue = TRUE;
    HANDLE       hEvents[2];
    unsigned int i;
    CONTEXT      c;
    BOINC_WINDOWCAPTURE window_info;
    PBOINC_THREADLISTENTRY pThreadEntry = NULL;

    // Set our friendly name
    diagnostics_set_thread_name("Debug Exception Monitor");
    diagnostics_set_thread_exempt_suspend();

    // Aquire the mutex that will keep all the threads that throw an exception
    //   at bay until we are ready to deal with them.
    WaitForSingleObject(hExceptionMonitorHalt, INFINITE);


    // Which events do we want to wait for?
    hEvents[0] = hExceptionQuitEvent;
    hEvents[1] = hExceptionDetectedEvent;

    while (bContinue) {
        dwEvent = WaitForMultipleObjects( 
            2,           // number of objects in array
            hEvents,     // array of objects
            FALSE,       // wait for any
            INFINITE     // wait
        );
        switch(dwEvent) {
            // hExceptionQuitEvent was signaled.
            case WAIT_OBJECT_0 + 0:

                // We are shutting down so lets cleanup and exit.
                bContinue = false;
                
                break;

            // hExceptionDetectedEvent was signaled.
            case WAIT_OBJECT_0 + 1:
#ifdef _DEBUG
                if (diagnostics_is_flag_set(BOINC_DIAG_MEMORYLEAKCHECKENABLED)) {
                    CLEAR_CRT_DEBUG_FIELD(_CRTDBG_LEAK_CHECK_DF);
                }
                if (diagnostics_is_flag_set(BOINC_DIAG_HEAPCHECKENABLED)) {
                    CLEAR_CRT_DEBUG_FIELD(_CRTDBG_CHECK_ALWAYS_DF);
                    CLEAR_CRT_DEBUG_FIELD(_CRTDBG_CHECK_EVERY_1024_DF);
                }
#endif // _DEBUG


                // Enumerate through all the threads so we have a complete list of what we need to dump.
                diagnostics_update_thread_list();

                // Get any data that will be needed later but will cause a deadlock if called after
                //   the other threads are suspended.
                diagnostics_capture_foreground_window(&window_info);

                // Wait for the ThreadListSync mutex before writing updates
                WaitForSingleObject(hThreadListSync, INFINITE);

                // Suspend the other threads.
                for (i=0; i<diagnostics_threads.size(); i++) {
                    pThreadEntry = diagnostics_threads[i];
			        // Suspend the thread before getting the threads context, otherwise
                    //   it'll be junk.
                    if (!pThreadEntry->crash_suspend_exempt && pThreadEntry->thread_handle) {
                        SuspendThread(pThreadEntry->thread_handle);
                    }
                }

                // Dump some basic stuff about runtime debugger version and date
                diagnostics_unhandled_exception_dump_banner();

#ifndef __CYGWIN__
                // Kickstart the debugger extensions
 	            DebuggerInitialize(
                    diagnostics_get_boinc_dir(),
                    diagnostics_get_symstore(),
                    diagnostics_is_proxy_enabled(),
                    diagnostics_get_proxy()
                );

                // Dump any useful information
                DebuggerDisplayDiagnostics();
#endif

                // Dump the other threads stack.
                for (i=0; i<diagnostics_threads.size(); i++) {
                    pThreadEntry = diagnostics_threads[i];
                    if (pThreadEntry->thread_id && !pThreadEntry->crash_suspend_exempt) {

                        diagnostics_dump_thread_information(pThreadEntry);

                        // Dump the exception record
                        if (pThreadEntry->crash_exception_record) {
                            diagnostics_dump_exception_record(
                                pThreadEntry->crash_exception_record
                            );
                        }

                        if (diagnostics_is_flag_set(BOINC_DIAG_DUMPCALLSTACKENABLED)) {
#ifndef __CYGWIN__
                            if (pThreadEntry->crash_exception_record) {
                                StackwalkFilter(
                                    pThreadEntry->crash_exception_record,
                                    EXCEPTION_EXECUTE_HANDLER
                                );
                            } else {
                                // Get the thread context
                                memset(&c, 0, sizeof(CONTEXT));
                                c.ContextFlags = CONTEXT_FULL;
				                GetThreadContext(pThreadEntry->thread_handle, &c);

                                StackwalkThread(
                                    pThreadEntry->thread_handle,
                                    &c
                                );
                            }
#else
                            fprintf(stderr, "Warning: Callstack dumps are not supported on CYGWIN\n");
#endif
                        }
                        fprintf(stderr, "\n");
                    }
                }

                diagnostics_message_monitor_dump();

                diagnostics_foreground_window_dump(&window_info);

                fprintf(stderr, "Exiting...\n");

                // Release the Mutex
                ReleaseMutex(hThreadListSync);

                // Force terminate the app letting BOINC know an exception has occurred.
                if (diagnostics_is_aborted_via_gui()) {
                    TerminateProcess(GetCurrentProcess(), ERR_ABORTED_VIA_GUI);
                } else {
                    TerminateProcess(GetCurrentProcess(), diagnostics_determine_exit_code());
                }

                break;
        }
    }

    // Notify the calling thread that the message monitoring thread is
    //   finished.
    SetEvent(hMessageQuitFinishedEvent);
    return 0;
}


// Let the unhandled exception monitor take care of logging the exception data.
//   Store the exception pointers and then singal the exception monitor to start
//   partying on the data.
LONG CALLBACK boinc_catch_signal(PEXCEPTION_POINTERS pExPtrs) {

    // Store the exception record pointers.
    diagnostics_set_thread_exception_record(pExPtrs);

    // Wake the unhandled exception monitor up to process the exception.
    SetEvent(hExceptionDetectedEvent);

    // Go to sleep waiting for something this thread will never see.
    WaitForSingleObject(hExceptionMonitorHalt, INFINITE);

    // We won't make it to this point, but make the compiler happy anyway.
    return 0;
}


const char *BOINC_RCSID_5967ad204d = "$Id$";
