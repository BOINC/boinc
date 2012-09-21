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

// Stuff related to catching SEH exceptions, monitoring threads, and trapping
// debugger messages; used by both core client and by apps.

#if   defined(_WIN32) && !defined(__STDWX_H__)
#include "boinc_win.h"
#elif defined(_WIN32) && defined(__STDWX_H__)
#include "stdwx.h"
#endif

#if defined(_MSC_VER) || defined(__MINGW32__)
#define snprintf    _snprintf
#define strdate     _strdate
#define strtime     _strtime
#endif

#ifndef __CYGWIN32__
#include "stackwalker_win.h"
#endif

#include "diagnostics.h"
#include "error_numbers.h"
#include "str_util.h"
#include "util.h"
#include "version.h"

#include "diagnostics_win.h"

// NtQuerySystemInformation
typedef NTSTATUS (WINAPI *tNTQSI)(
    ULONG SystemInformationClass,
    PVOID SystemInformation,
    ULONG SystemInformationLength,
    PULONG ReturnLength
);

// IsDebuggerPresent
typedef BOOL (WINAPI *tIDP)();

// CreateToolhelp32Snapshot
typedef HANDLE (WINAPI *tCT32S)(DWORD dwFlags, DWORD dwProcessID);
// Thread32First
typedef BOOL (WINAPI *tT32F)(HANDLE hSnapshot, LPTHREADENTRY32 lpte);
// Thread32Next
typedef BOOL (WINAPI *tT32N)(HANDLE hSnapshot, LPTHREADENTRY32 lpte);
// OpenThread
typedef HANDLE (WINAPI *tOT)(DWORD dwDesiredAccess, BOOL bInheritHandle, DWORD dwThreadId);


// Look in the registry for the specified value user the BOINC diagnostics
//   hive.
BOOL diagnostics_get_registry_value(LPCSTR lpName, LPDWORD lpdwType, LPDWORD lpdwSize, LPBYTE lpData) {
	LONG  lRetVal;
	HKEY  hKey;

    // Detect platform information
    OSVERSIONINFO osvi; 
    osvi.dwOSVersionInfoSize = sizeof(osvi);
    GetVersionEx(&osvi);

    if (VER_PLATFORM_WIN32_WINDOWS == osvi.dwPlatformId) {
		lRetVal = RegOpenKeyExA(
            HKEY_LOCAL_MACHINE, 
            "SOFTWARE\\Space Sciences Laboratory, U.C. Berkeley\\BOINC Diagnostics",
			(DWORD)NULL, 
            KEY_READ,
            &hKey
        );
		if (lRetVal != ERROR_SUCCESS) return FALSE;
	} else {
		lRetVal = RegOpenKeyExA(
            HKEY_CURRENT_USER,
            "SOFTWARE\\Space Sciences Laboratory, U.C. Berkeley\\BOINC Diagnostics",
			(DWORD)NULL,
            KEY_READ,
            &hKey
        );
		if (lRetVal != ERROR_SUCCESS) return FALSE;
	}

	lRetVal = RegQueryValueExA(hKey, lpName, NULL, lpdwType, lpData, lpdwSize);

	RegCloseKey(hKey);

	return (lRetVal == ERROR_SUCCESS);
}


// Provide a structure to store process measurements at the time of a
//   crash.
typedef struct _BOINC_PROCESSENTRY {
    DWORD               process_id;
    VM_COUNTERS         vm_counters;
    IO_COUNTERS         io_counters;
} BOINC_PROCESSENTRY, *PBOINC_PROCESSENTRY;

static BOINC_PROCESSENTRY diagnostics_process;


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
    DWORD               thread_id;
    HANDLE              thread_handle;
    BOOL                crash_suspend_exempt;
    FLOAT               crash_kernel_time;
    FLOAT               crash_user_time;
    FLOAT               crash_wait_time;
    INT                 crash_priority;
    INT                 crash_base_priority;
    INT                 crash_state;
    INT                 crash_wait_reason;
    PEXCEPTION_POINTERS crash_exception_record;
    char                crash_message[1024];
} BOINC_THREADLISTENTRY, *PBOINC_THREADLISTENTRY;

static std::vector<PBOINC_THREADLISTENTRY> diagnostics_threads;
static HANDLE hThreadListSync;


// Initialize the thread list entry.
int diagnostics_init_thread_entry(PBOINC_THREADLISTENTRY entry) {
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
    strncpy(entry->crash_message, "", sizeof(entry->crash_message));
    return 0;
}


// Initialize the thread list, which means empty it if anything is
//   in it.
int diagnostics_init_thread_list() {
    int retval = 0;
    size_t i;
    size_t size;

    // Create a Mutex that can be used to syncronize data access
    //   to the global thread list.
    hThreadListSync = CreateMutex(NULL, TRUE, NULL);
    if (!hThreadListSync) {
        fprintf(
            stderr, "diagnostics_init_thread_list(): Creating hThreadListSync failed, GLE %d\n", GetLastError()
        );
        retval = GetLastError();
    } else {

        size = diagnostics_threads.size();
        for (i=0; i<size; i++) {
            delete diagnostics_threads[i];
        }
        diagnostics_threads.clear();

        // Release the Mutex
        ReleaseMutex(hThreadListSync);

    }

    return retval;
}


// Finish the thread list, which means empty it if anything is
//   in it.
int diagnostics_finish_thread_list() {
    size_t i;
    size_t size;

    // Wait for the ThreadListSync mutex before writing updates
    WaitForSingleObject(hThreadListSync, INFINITE);

    size = diagnostics_threads.size();
    for (i=0; i<size; i++) {
        delete diagnostics_threads[i];
    }
    diagnostics_threads.clear();

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
    HMODULE hKernel32Lib = NULL;
    PBOINC_THREADLISTENTRY pThreadEntry = NULL;
    tCT32S  pCT32S = NULL;
    tT32F   pT32F = NULL;
    tT32N   pT32N = NULL;
    tOT     pOT = NULL;
    THREADENTRY32 te32; 

    // Which version of the data structure are we using.
    te32.dwSize = sizeof(te32); 

    // Dynamically link to the proper function pointers.
    hKernel32Lib = GetModuleHandleA("kernel32.dll");

    pCT32S = (tCT32S) GetProcAddress( hKernel32Lib, "CreateToolhelp32Snapshot" );
    pT32F = (tT32F) GetProcAddress( hKernel32Lib, "Thread32First" );
    pT32N = (tT32N) GetProcAddress( hKernel32Lib, "Thread32Next" );
    pOT = (tOT) GetProcAddress( hKernel32Lib, "OpenThread" );

    if (!pCT32S || !pT32F || !pT32N) {
        return ERROR_NOT_SUPPORTED; 
    }

    // Take a snapshot of all running threads  
    hThreadSnap = pCT32S(TH32CS_SNAPTHREAD, 0); 
    if( hThreadSnap == INVALID_HANDLE_VALUE ) {
        return GetLastError(); 
    }

    // Retrieve information about the first thread,
    // and exit if unsuccessful
    if( !pT32F( hThreadSnap, &te32 ) ) {
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
    HMODULE  hNTDllLib = NULL;
    tNTQSI   pNTQSI = NULL;

    hNTDllLib = GetModuleHandleA("ntdll.dll");
    pNTQSI = (tNTQSI)GetProcAddress(hNTDllLib, "NtQuerySystemInformation");

    do {
        *ppBuffer = HeapAlloc(hHeap, HEAP_ZERO_MEMORY, *pcbBuffer);
        if (*ppBuffer == NULL) {
            return ERROR_NOT_ENOUGH_MEMORY;
        }

        Status = pNTQSI(
            SystemProcessAndThreadInformation,
            *ppBuffer,
            *pcbBuffer,
            pcbBuffer
        );

        if (Status == STATUS_INFO_LENGTH_MISMATCH) {
            HeapFree(hHeap, (DWORD)NULL, *ppBuffer);
            *pcbBuffer *= 2;
        } else if (!NT_SUCCESS(Status)) {
            HeapFree(hHeap, (DWORD)NULL, *ppBuffer);
            retval = Status;
        }
    } while (Status == STATUS_INFO_LENGTH_MISMATCH);

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
    HMODULE                 hKernel32Lib;
    tOT                     pOT = NULL;


    // Dynamically link to the proper function pointers.
    hKernel32Lib = GetModuleHandleA("kernel32.dll");
    pOT = (tOT) GetProcAddress( hKernel32Lib, "OpenThread" );

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

            // Store the process information we now know about.
            diagnostics_process.process_id = pProcesses->ProcessId;
            diagnostics_process.vm_counters = pProcesses->VmCounters;

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
    if (pBuffer) HeapFree(GetProcessHeap(), (DWORD)NULL, pBuffer);

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
    HMODULE                 hKernel32Lib;
    tOT                     pOT = NULL;


    // Dynamically link to the proper function pointers.
    hKernel32Lib = GetModuleHandleA("kernel32.dll");
    pOT = (tOT) GetProcAddress( hKernel32Lib, "OpenThread" );

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

            // Store the process information we now know about.
            diagnostics_process.process_id = pProcesses->ProcessId;
            diagnostics_process.vm_counters = pProcesses->VmCounters;
            diagnostics_process.io_counters = pProcesses->IoCounters;

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
    if (pBuffer) HeapFree(GetProcessHeap(), (DWORD)NULL, pBuffer);

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


// Set the cached exception record for the current thread, let the exception monitor
// thread dump the human readable exception information.
int diagnostics_set_thread_exception_record(PEXCEPTION_POINTERS pExPtrs) {
    HANDLE hThread;
    PBOINC_THREADLISTENTRY pThreadEntry = NULL;

    // Wait for the ThreadListSync mutex before writing updates
    WaitForSingleObject(hThreadListSync, INFINITE);

    pThreadEntry = diagnostics_find_thread_entry(GetCurrentThreadId());
    if (pThreadEntry) {
        pThreadEntry->crash_exception_record = pExPtrs;
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
        pThreadEntry->thread_id = GetCurrentThreadId();
        pThreadEntry->thread_handle = hThread;
        pThreadEntry->crash_exception_record = pExPtrs;
        diagnostics_threads.push_back(pThreadEntry);
    }

    // Release the Mutex
    ReleaseMutex(hThreadListSync);

    return 0;
}


// Set the current thread to suspend exempt status.  Prevents deadlocks.
int diagnostics_set_thread_exempt_suspend() {
    HANDLE hThread;
    PBOINC_THREADLISTENTRY pThreadEntry = NULL;

    // Wait for the ThreadListSync mutex before writing updates
    WaitForSingleObject(hThreadListSync, INFINITE);

    pThreadEntry = diagnostics_find_thread_entry(GetCurrentThreadId());
    if (pThreadEntry) {
        pThreadEntry->crash_suspend_exempt = TRUE;
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
        pThreadEntry->thread_id = GetCurrentThreadId();
        pThreadEntry->thread_handle = hThread;
        pThreadEntry->crash_suspend_exempt = TRUE;
        diagnostics_threads.push_back(pThreadEntry);
    }

    // Release the Mutex
    ReleaseMutex(hThreadListSync);

    return 0;
}


// Checks to see if the specified thread id is flagged for suspend exempt status.
// returns 0 on true, 1 on false.  Couldn't use a bool data type since the function
// prototype needs to be compatible with C.
int diagnostics_is_thread_exempt_suspend(long thread_id) {
    int retval = 1;
    PBOINC_THREADLISTENTRY pThreadEntry = NULL;

    // Wait for the ThreadListSync mutex before writing updates
    WaitForSingleObject(hThreadListSync, INFINITE);

    pThreadEntry = diagnostics_find_thread_entry(thread_id);
    if (pThreadEntry) {
        if (pThreadEntry->crash_suspend_exempt) {
            retval = 0;
        }
    }

    // Release the Mutex
    ReleaseMutex(hThreadListSync);

    return retval;
}


// Set the current thread's crash message.
int diagnostics_set_thread_crash_message(char* message) {
    HANDLE hThread;
    PBOINC_THREADLISTENTRY pThreadEntry = NULL;

    // Wait for the ThreadListSync mutex before writing updates
    WaitForSingleObject(hThreadListSync, INFINITE);

    pThreadEntry = diagnostics_find_thread_entry(GetCurrentThreadId());
    if (pThreadEntry) {
        int buffer_used = snprintf(
            pThreadEntry->crash_message, 
            sizeof(pThreadEntry->crash_message),
            "%s",
            message
        );
        if ((sizeof(pThreadEntry->crash_message) == buffer_used) || (-1 == buffer_used)) { 
            pThreadEntry->crash_message[sizeof(pThreadEntry->crash_message)-1] = '\0';
        }
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
        pThreadEntry->thread_id = GetCurrentThreadId();
        pThreadEntry->thread_handle = hThread;
        int buffer_used = snprintf(
            pThreadEntry->crash_message, 
            sizeof(pThreadEntry->crash_message),
            "%s",
            message
        );
        if ((sizeof(pThreadEntry->crash_message) == buffer_used) || (-1 == buffer_used)) { 
            pThreadEntry->crash_message[sizeof(pThreadEntry->crash_message)-1] = '\0';
        }
        diagnostics_threads.push_back(pThreadEntry);
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
        case ThreadStateInitialized: return "Initialized";
        case ThreadStateReady: return "Ready";
        case ThreadStateRunning: return "Running";
        case ThreadStateStandby: return "Standby";
        case ThreadStateTerminated: return "Terminated";
        case ThreadStateWaiting: return "Waiting";
        case ThreadStateTransition: return "Transition";
        default: return "Unknown";
    }
    return "";
}


// Translate the thread wait reason into a human readable form.
//
// See: http://support.microsoft.com/?kbid=837372
//
char* diagnostics_format_thread_wait_reason(int thread_wait_reason) {
    switch(thread_wait_reason) {
        case ThreadWaitReasonExecutive: return "Executive";
        case ThreadWaitReasonFreePage: return "FreePage";
        case ThreadWaitReasonPageIn: return "PageIn";
        case ThreadWaitReasonPoolAllocation: return "PoolAllocation";
        case ThreadWaitReasonDelayExecution: return "ExecutionDelay";
        case ThreadWaitReasonSuspended: return "Suspended";
        case ThreadWaitReasonUserRequest: return "UserRequest";
        case ThreadWaitReasonWrExecutive: return "Executive";
        case ThreadWaitReasonWrFreePage: return "FreePage";
        case ThreadWaitReasonWrPageIn: return "PageIn";
        case ThreadWaitReasonWrPoolAllocation: return "PoolAllocation";
        case ThreadWaitReasonWrDelayExecution: return "ExecutionDelay";
        case ThreadWaitReasonWrSuspended: return "Suspended";
        case ThreadWaitReasonWrUserRequest: return "UserRequest";
        case ThreadWaitReasonWrEventPairHigh: return "EventPairHigh";
        case ThreadWaitReasonWrEventPairLow: return "EventPairLow";
        case ThreadWaitReasonWrLpcReceive: return "LPCReceive";
        case ThreadWaitReasonWrLpcReply: return "LPCReply";
        case ThreadWaitReasonWrVirtualMemory: return "VirtualMemory";
        case ThreadWaitReasonWrPageOut: return "PageOut";
        default: return "Unknown";
    }
    return "";
}


// Translate the process priority class into a human readable form.
//
// See: http://msdn.microsoft.com/library/default.asp?url=/library/en-us/dllproc/base/scheduling_priorities.asp
//
char* diagnostics_format_process_priority(int process_priority) {
    switch(process_priority) {
        case IDLE_PRIORITY_CLASS: return "Idle";
        case BELOW_NORMAL_PRIORITY_CLASS: return "Below Normal";
        case NORMAL_PRIORITY_CLASS: return "Normal";
        case ABOVE_NORMAL_PRIORITY_CLASS: return "Above Normal";
        case HIGH_PRIORITY_CLASS: return "High";
        case REALTIME_PRIORITY_CLASS: return "Realtime";
        default: return "Unknown";
    }
    return "";
}


// Translate the thread priority class into a human readable form.
//
// See: http://msdn.microsoft.com/library/default.asp?url=/library/en-us/dllproc/base/scheduling_priorities.asp
//
char* diagnostics_format_thread_priority(int thread_priority) {
    switch(thread_priority) {
        case THREAD_PRIORITY_IDLE: return "Idle";
        case THREAD_PRIORITY_LOWEST: return "Lowest";
        case THREAD_PRIORITY_BELOW_NORMAL: return "Below Normal";
        case THREAD_PRIORITY_NORMAL: return "Normal";
        case THREAD_PRIORITY_ABOVE_NORMAL: return "Above Normal";
        case THREAD_PRIORITY_HIGHEST: return "Highest";
        case THREAD_PRIORITY_TIME_CRITICAL: return "Time Critical";
        default: return "Unknown";
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
static UINT   uiMessageMonitorThreadId;
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
    DWORD dwType;
    DWORD dwSize;
    DWORD dwCaptureMessages;
    HMODULE hKernel32Lib;
    tIDP pIDP = NULL;

    SECURITY_ATTRIBUTES sa;
    SECURITY_DESCRIPTOR sd;

    sa.nLength = sizeof(SECURITY_ATTRIBUTES);
    sa.bInheritHandle = TRUE;
    sa.lpSecurityDescriptor = &sd;

    InitializeSecurityDescriptor(&sd, SECURITY_DESCRIPTOR_REVISION);
    SetSecurityDescriptorDacl(&sd, TRUE, (PACL)NULL, FALSE);


    // Create a mutex that can be used to syncronize data access
    //   to the global thread list.
    hMessageMonitorSync = CreateMutex(NULL, TRUE, NULL);
    if (!hMessageMonitorSync) {
        fprintf(
            stderr, "diagnostics_init_message_monitor(): Creating hMessageMonitorSync failed, GLE %d\n", GetLastError()
        );
    }

    // Clear out any previous messages.
    for (i=0; i<diagnostics_monitor_messages.size(); i++) {
        delete diagnostics_monitor_messages[i];
    }
    diagnostics_monitor_messages.clear();

    // Check the registry to see if we are aloud to capture debugger messages.
    //   Apparently many audio and visual payback programs dump serious
    //   amounts of data to the debugger viewport even on a release build.
    //   When this feature is enabled it slows down the replay of DVDs and CDs
    //   such that they become jerky and unpleasent to watch or listen too.
    //
    // We'll turn it off by default, but keep it around just in case we need
    //   it.
    //
    dwCaptureMessages = 0;
    dwType = REG_DWORD;
    dwSize = sizeof(dwCaptureMessages);
    diagnostics_get_registry_value(
        "CaptureMessages",
        &dwType,
        &dwSize,
        (LPBYTE)&dwCaptureMessages
    );


    // If a debugger is present then let it capture the debugger messages
    hKernel32Lib = GetModuleHandleA("kernel32.dll");
    pIDP = (tIDP) GetProcAddress(hKernel32Lib, "IsDebuggerPresent");

    if (pIDP) {
        if (!pIDP() && hMessageMonitorSync && dwCaptureMessages) {

            hMessageAckEvent = CreateEventA(&sa, FALSE, FALSE, "DBWIN_BUFFER_READY");
            if (!hMessageAckEvent) {
                fprintf(
                    stderr, "diagnostics_init_message_monitor(): Creating hMessageAckEvent failed, GLE %d\n", GetLastError()
                );
            }

            hMessageReadyEvent = CreateEventA(&sa, FALSE, FALSE, "DBWIN_DATA_READY");
            if (!hMessageReadyEvent) {
                fprintf(
                    stderr, "diagnostics_init_message_monitor(): Creating hMessageReadyEvent failed, GLE %d\n", GetLastError()
                );
            }

            hMessageQuitEvent = CreateEventA(NULL, FALSE, FALSE, NULL);
            if (!hMessageQuitEvent) {
                fprintf(
                    stderr, "diagnostics_init_message_monitor(): Creating hMessageQuitEvent failed, GLE %d\n", GetLastError()
                );
            }

            hMessageQuitFinishedEvent = CreateEventA(NULL, FALSE, FALSE, NULL);
            if (!hMessageQuitFinishedEvent) {
                fprintf(
                    stderr, "diagnostics_init_message_monitor(): Creating hMessageQuitFinishedEvent failed, GLE %d\n", GetLastError()
                );
            }

            hMessageSharedMap = CreateFileMappingA(
                INVALID_HANDLE_VALUE,    // use paging file
                &sa,                     // default security 
                PAGE_READWRITE,          // read/write access
                0,                       // max. object size 
                sizeof(DEBUGGERMESSAGE), // buffer size  
                "DBWIN_BUFFER"           // name of mapping object
            );
            if (!hMessageSharedMap) {
                fprintf(
                    stderr, "diagnostics_init_message_monitor(): CreateFileMapping hMessageSharedMap failed, GLE %d\n", GetLastError()
                );
            }

            pMessageBuffer = (PDEBUGGERMESSAGE)MapViewOfFile(
                hMessageSharedMap,
                FILE_MAP_READ | FILE_MAP_WRITE,
                0,                       // file offset high
                0,                       // file offset low
                sizeof(DEBUGGERMESSAGE)  // # of bytes to map (entire file)
            );
            if (!pMessageBuffer) {
                fprintf(
                    stderr, "diagnostics_init_message_monitor(): MapViewOfFile pMessageBuffer failed, GLE %d\n", GetLastError()
                );
            }

            hMessageMonitorThread = (HANDLE)_beginthreadex(
                NULL,
                0,
                diagnostics_message_monitor,
                0,
                0,
                &uiMessageMonitorThreadId
            );
            if (!hMessageMonitorThread) {
                fprintf(
                    stderr, "diagnostics_init_message_monitor(): _beginthreadex, errno %d\n", errno
                );
            }
        } else {
            retval = ERROR_NOT_SUPPORTED;
        }
    }

    // Release the Mutex
    ReleaseMutex(hMessageMonitorSync);

    return retval;
}


// Shutdown the message monitoring thread and cleanup any of the in memory
//   structures.
int diagnostics_finish_message_monitor() {
    unsigned int i;

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
        delete diagnostics_monitor_messages[i];
    }
    diagnostics_monitor_messages.clear();


    // Cleanup the handles
    if (pMessageBuffer) UnmapViewOfFile(pMessageBuffer);
    if (hMessageSharedMap) CloseHandle(hMessageSharedMap);
    if (hMessageAckEvent) CloseHandle(hMessageAckEvent);
    if (hMessageReadyEvent) CloseHandle(hMessageReadyEvent);
    if (hMessageQuitEvent) CloseHandle(hMessageQuitEvent);
    if (hMessageQuitFinishedEvent) CloseHandle(hMessageQuitFinishedEvent);
    if (hMessageMonitorThread) CloseHandle(hMessageMonitorThread);
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
UINT WINAPI diagnostics_message_monitor(LPVOID /* lpParameter */) {
    DWORD       dwEvent = (DWORD)NULL;
    DWORD       dwCurrentProcessId = (DWORD)NULL;
    BOOL        bContinue = TRUE;
    DWORD       dwRepeatMessageCounter = 0;
    DWORD       dwRepeatMessageProcessId = 0;
    std::string strRepeatMessage;
    PBOINC_MESSAGEMONITORENTRY pMessageEntry = NULL;
    HANDLE      hEvents[2];

    // Make sure this thread doesn't get suspended during
    //   a crash dump, the DBGHELP library is pretty verbose.
    //   Suspending this thread will cause a deadlock.
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
                        delete diagnostics_monitor_messages[0];
                        diagnostics_monitor_messages.erase(diagnostics_monitor_messages.begin());
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

static UINT   uiExceptionMonitorThreadId = (UINT)NULL;
static HANDLE hExceptionMonitorThread = NULL;
static HANDLE hExceptionMonitorHalt = NULL;
static HANDLE hExceptionMonitorStartedEvent = NULL;
static HANDLE hExceptionDetectedEvent = NULL;
static HANDLE hExceptionQuitEvent = NULL;
static HANDLE hExceptionQuitFinishedEvent = NULL;
static CRITICAL_SECTION csExceptionMonitorFallback; 

// Initialize the needed structures and startup the unhandled exception
//   monitor thread.
int diagnostics_init_unhandled_exception_monitor() {
    int retval = 0;

    // Initialize the fallback critical section in case we fail to create the
    //   unhandled exception monitor.
    InitializeCriticalSection(&csExceptionMonitorFallback);


    // Create a mutex that can be used to put any thread that has thrown
    //   an unhandled exception to sleep.
    hExceptionMonitorHalt = CreateMutex(NULL, FALSE, NULL);
    if (!hExceptionMonitorHalt) {
        fprintf(
            stderr, "diagnostics_init_unhandled_exception_monitor(): Creating hExceptionMonitorHalt failed, GLE %d\n", GetLastError()
        );
    }

    // The following event is thrown by the exception monitoring thread
    //   right before it waits for the hExceptionDetectedEvent event.
    hExceptionMonitorStartedEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (!hExceptionMonitorStartedEvent) {
        fprintf(
            stderr, "diagnostics_init_unhandled_exception_monitor(): Creating hExceptionMonitorStartedEvent failed, GLE %d\n", GetLastError()
        );
    }

    // The following event is thrown by a thread that has experienced an
    //   unhandled exception after storing its exception record but before
    //   it attempts to aquire the halt mutex.
    hExceptionDetectedEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (!hExceptionDetectedEvent) {
        fprintf(
            stderr, "diagnostics_init_unhandled_exception_monitor(): Creating hExceptionDetectedEvent failed, GLE %d\n", GetLastError()
        );
    }

    // Create an event that we can use to shutdown the unhandled exception
    //   monitoring thread.
    hExceptionQuitEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (!hExceptionQuitEvent) {
        fprintf(
            stderr, "diagnostics_init_unhandled_exception_monitor(): Creating hExceptionQuitEvent failed, GLE %d\n", GetLastError()
        );
    }
    hExceptionQuitFinishedEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (!hExceptionQuitFinishedEvent) {
        fprintf(
            stderr, "diagnostics_init_unhandled_exception_monitor(): Creating hExceptionQuitFinishedEvent failed, GLE %d\n", GetLastError()
        );
    }

    // Create the thread that is going to monitor any unhandled exceptions
    // NOTE: Only attempt to create the thread if all the thread sync objects
    //   have been created.
    if (hExceptionMonitorHalt && hExceptionDetectedEvent && hExceptionQuitEvent && hExceptionQuitFinishedEvent) {
        hExceptionMonitorThread = (HANDLE)_beginthreadex(
            NULL,
            0,
            diagnostics_unhandled_exception_monitor,
            0,
            0,
            &uiExceptionMonitorThreadId
        );
        if (!hExceptionMonitorThread) {
            fprintf(
                stderr, "diagnostics_init_unhandled_exception_monitor(): Creating hExceptionMonitorThread failed, errno %d\n", errno
            );
        }
    }

    if (!hExceptionMonitorThread) {
        fprintf(
            stderr, "WARNING: BOINC Windows Runtime Debugger has been disabled.\n"
        );
        retval = ERR_THREAD;
    } else {

        // Wait until the exception monitor is ready for business.
        //
        WaitForSingleObject(hExceptionMonitorStartedEvent, INFINITE);

    }

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

    // Cleanup the fallback critical section.
    DeleteCriticalSection(&csExceptionMonitorFallback);

    return 0;
}


// Dump crash header information
//
int diagnostics_unhandled_exception_dump_banner() {
    char szDate[64];
    char szTime[64];

    strdate(szDate);
    strtime(szTime);

    fprintf(stderr, "\n\n");
    fprintf(stderr, "********************\n");
    fprintf(stderr, "\n\n");
    fprintf(stderr, "BOINC Windows Runtime Debugger Version %s\n", BOINC_VERSION_STRING);
    fprintf(stderr, "\n\n");
    fprintf(stderr, "Dump Timestamp    : %s %s\n", szDate, szTime);
    if (diagnostics_is_flag_set(BOINC_DIAG_BOINCAPPLICATION)) {
        fprintf(stderr, "Install Directory : %s\n", diagnostics_get_boinc_install_dir());
        fprintf(stderr, "Data Directory    : %s\n", diagnostics_get_boinc_dir());
        fprintf(stderr, "Project Symstore  : %s\n", diagnostics_get_symstore());
    }

    return 0;
}

// Capture the foreground window details for future use.
//
int diagnostics_capture_foreground_window(PBOINC_WINDOWCAPTURE window_info) {
    DWORD dwType;
    DWORD dwSize;
    DWORD dwCaptureForegroundWindow;


    // Initialize structure variables.
	strcpy(window_info->window_name, "");
	strcpy(window_info->window_class, "");
    window_info->hwnd = 0;
    window_info->window_process_id = 0;
    window_info->window_thread_id = 0;


    // Check the registry to see if we are aloud to capture the foreground
    //   window data. Many people were concerned about privacy issues.
    //
    // We'll turn it off by default, but keep it around just in case we need
    //   it.
    //
    dwCaptureForegroundWindow = 0;
    dwType = REG_DWORD;
    dwSize = sizeof(dwCaptureForegroundWindow);
    diagnostics_get_registry_value(
        "CaptureForegroundWindow",
        &dwType,
        &dwSize,
        (LPBYTE)&dwCaptureForegroundWindow
    );


    if (dwCaptureForegroundWindow) {
        window_info->hwnd = GetForegroundWindow();

        window_info->window_thread_id = GetWindowThreadProcessId(
            window_info->hwnd,
            &window_info->window_process_id
        );

	    // Only query the window text from windows in a different process space.
	    //   All threads that might have windows are suspended in this process
	    //   space and attempting to get the window text will deadlock the exception
	    //   handler.
	    if (window_info->window_process_id != GetCurrentProcessId()) {
		    GetWindowTextA(
			    window_info->hwnd, 
			    window_info->window_name,
			    sizeof(window_info->window_name)
		    );

		    GetClassNameA(
			    window_info->hwnd,
			    window_info->window_class,
			    sizeof(window_info->window_class)
		    );
	    }
    }

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


// Dump the captured information for a the current process.
//
int diagnostics_dump_process_information() {
    // Header
    fprintf(
        stderr, 
        "*** Dump of the Process Statistics: ***\n\n"
    );

    // I/O Counters
    fprintf(
        stderr, 
        "- I/O Operations Counters -\n"
        "Read: %d, Write: %d, Other %d\n"
        "\n"
        "- I/O Transfers Counters -\n"
        "Read: %d, Write: %d, Other %d\n"
        "\n",
        diagnostics_process.io_counters.ReadOperationCount,
        diagnostics_process.io_counters.WriteOperationCount,
        diagnostics_process.io_counters.OtherOperationCount,
        diagnostics_process.io_counters.ReadTransferCount,
        diagnostics_process.io_counters.WriteTransferCount,
        diagnostics_process.io_counters.OtherTransferCount
    );

    // VM Counters
    fprintf(
        stderr, 
        "- Paged Pool Usage -\n"
        "QuotaPagedPoolUsage: %d, QuotaPeakPagedPoolUsage: %d\n"
        "QuotaNonPagedPoolUsage: %d, QuotaPeakNonPagedPoolUsage: %d\n"
        "\n"
        "- Virtual Memory Usage -\n"
        "VirtualSize: %d, PeakVirtualSize: %d\n"
        "\n"
        "- Pagefile Usage -\n"
        "PagefileUsage: %d, PeakPagefileUsage: %d\n"
        "\n"
        "- Working Set Size -\n"
        "WorkingSetSize: %d, PeakWorkingSetSize: %d, PageFaultCount: %d\n"
        "\n",
        diagnostics_process.vm_counters.QuotaPagedPoolUsage,
        diagnostics_process.vm_counters.QuotaPeakPagedPoolUsage,
        diagnostics_process.vm_counters.QuotaNonPagedPoolUsage,
        diagnostics_process.vm_counters.QuotaPeakNonPagedPoolUsage,
        diagnostics_process.vm_counters.VirtualSize,
        diagnostics_process.vm_counters.PeakVirtualSize,
        diagnostics_process.vm_counters.PagefileUsage,
        diagnostics_process.vm_counters.PeakPagefileUsage,
        diagnostics_process.vm_counters.WorkingSetSize,
        diagnostics_process.vm_counters.PeakWorkingSetSize,
        diagnostics_process.vm_counters.PageFaultCount
    );

    return 0;
}


// Dump the captured information for a given thread.
//
int diagnostics_dump_thread_information(PBOINC_THREADLISTENTRY pThreadEntry) {
    std::string strStatusExtra;

    if (pThreadEntry->crash_state == ThreadStateWaiting) {
        strStatusExtra += "Wait Reason: ";
        strStatusExtra += diagnostics_format_thread_wait_reason(pThreadEntry->crash_wait_reason);
        strStatusExtra += ", ";
    } else {
        strStatusExtra += "Base Priority: ";
        strStatusExtra += diagnostics_format_thread_priority(pThreadEntry->crash_base_priority);
        strStatusExtra += ", ";
        strStatusExtra += "Priority: ";
        strStatusExtra += diagnostics_format_thread_priority(pThreadEntry->crash_priority);
        strStatusExtra += ", ";
    }

    fprintf(
        stderr, 
        "*** Dump of thread ID %d (state: %s): ***\n\n"
        "- Information -\n"
        "Status: %s, "
        "Kernel Time: %f, "
        "User Time: %f, "
        "Wait Time: %f\n"
        "\n",
        pThreadEntry->thread_id,
        diagnostics_format_thread_state(pThreadEntry->crash_state),
        strStatusExtra.c_str(),
        pThreadEntry->crash_kernel_time,
        pThreadEntry->crash_user_time,
        pThreadEntry->crash_wait_time
    );

    return 0;
}


// Provide a generic way to format exceptions
//
int diagnostics_dump_generic_exception(char* exception_desc, DWORD exception_code, PVOID exception_address) {
    fprintf(
        stderr, 
        "Reason: %s (0x%x) at address 0x%p\n\n",
        exception_desc,
        exception_code,
        exception_address
    );
    return 0;
}

// Dump the exception code record to stderr in a human readable form.
//
int diagnostics_dump_exception_record(PEXCEPTION_POINTERS pExPtrs) {
    char           status[256];
    char           substatus[256];
    char           message[1024];
    PVOID          exception_address = pExPtrs->ExceptionRecord->ExceptionAddress;
    DWORD          exception_code = pExPtrs->ExceptionRecord->ExceptionCode;
    PDelayLoadInfo delay_load_info = NULL;

    // Print unhandled exception banner
    fprintf(stderr, "- Unhandled Exception Record -\n");

    switch (exception_code) {
        case VcppException(ERROR_SEVERITY_ERROR, ERROR_MOD_NOT_FOUND):
            delay_load_info = (PDelayLoadInfo)pExPtrs->ExceptionRecord->ExceptionInformation[0];
            fprintf(
                stderr,
                "Delay Load Failure: Attempting to load '%s' failed.\n\n",
                delay_load_info->szDll
            );
            break;
        case VcppException(ERROR_SEVERITY_ERROR, ERROR_PROC_NOT_FOUND):
            delay_load_info = (PDelayLoadInfo)pExPtrs->ExceptionRecord->ExceptionInformation[0];
            fprintf(
                stderr,
                "Delay Load Failure: Attempting to find '%s' in '%s' failed.\n\n",
                delay_load_info->dlp.szProcName,
                delay_load_info->szDll
            );
            break;
        case 0xC0000135:                     // STATUS_DLL_NOT_FOUND
        case 0xC0000139:                     // STATUS_ENTRYPOINT_NOT_FOUND
        case 0xC0000142:                     // STATUS_DLL_INIT_FAILED
        case 0xC0000143:                     // STATUS_MISSING_SYSTEMFILE
            fprintf(stderr, "%s\n\n", windows_format_error_string(exception_code, message, sizeof(message)));
            break;
        case 0xE06D7363:
            diagnostics_dump_generic_exception("Out Of Memory (C++ Exception)", exception_code, exception_address);
            break;
        case EXCEPTION_ACCESS_VIOLATION:
            strcpy(status, "Access Violation");
            strcpy(substatus, "");
            if (pExPtrs->ExceptionRecord->NumberParameters == 2) {
                switch(pExPtrs->ExceptionRecord->ExceptionInformation[0]) {
                case 0: // read attempt
                    sprintf(substatus,
                        "read attempt to address 0x%8.8X",
                        pExPtrs->ExceptionRecord->ExceptionInformation[1]
                    );
                    break;
                case 1: // write attempt
                    sprintf(substatus,
                        "write attempt to address 0x%8.8X",
                        pExPtrs->ExceptionRecord->ExceptionInformation[1]
                    );
                    break;
                }
            }
            fprintf(stderr,
                "Reason: %s (0x%x) at address 0x%p %s\n\n",
                status, exception_code, exception_address, substatus
            );
            break;
        case EXCEPTION_DATATYPE_MISALIGNMENT:
            diagnostics_dump_generic_exception("Data Type Misalignment", exception_code, exception_address);
            break;
        case EXCEPTION_BREAKPOINT:
            diagnostics_dump_generic_exception("Breakpoint Encountered", exception_code, exception_address);
            break;
        case EXCEPTION_SINGLE_STEP:
            diagnostics_dump_generic_exception("Single Instruction Executed", exception_code, exception_address);
            break;
        case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:
            diagnostics_dump_generic_exception("Array Bounds Exceeded", exception_code, exception_address);
            break;
        case EXCEPTION_FLT_DENORMAL_OPERAND:
            diagnostics_dump_generic_exception("Float Denormal Operand", exception_code, exception_address);
            break;
        case EXCEPTION_FLT_DIVIDE_BY_ZERO:
            diagnostics_dump_generic_exception("Divide by Zero", exception_code, exception_address);
            break;
        case EXCEPTION_FLT_INEXACT_RESULT:
            diagnostics_dump_generic_exception("Float Inexact Result", exception_code, exception_address);
            break;
        case EXCEPTION_FLT_INVALID_OPERATION:
            diagnostics_dump_generic_exception("Float Invalid Operation", exception_code, exception_address);
            break;
        case EXCEPTION_FLT_OVERFLOW:
            diagnostics_dump_generic_exception("Float Overflow", exception_code, exception_address);
            break;
        case EXCEPTION_FLT_STACK_CHECK:
            diagnostics_dump_generic_exception("Float Stack Check", exception_code, exception_address);
            break;
        case EXCEPTION_FLT_UNDERFLOW:
            diagnostics_dump_generic_exception("Float Underflow", exception_code, exception_address);
            break;
        case EXCEPTION_INT_DIVIDE_BY_ZERO:
            diagnostics_dump_generic_exception("Integer Divide by Zero", exception_code, exception_address);
            break;
        case EXCEPTION_INT_OVERFLOW:
            diagnostics_dump_generic_exception("Integer Overflow", exception_code, exception_address);
            break;
        case EXCEPTION_PRIV_INSTRUCTION:
            diagnostics_dump_generic_exception("Privileged Instruction", exception_code, exception_address);
            break;
        case EXCEPTION_IN_PAGE_ERROR:
            diagnostics_dump_generic_exception("In Page Error", exception_code, exception_address);
            break;
        case EXCEPTION_ILLEGAL_INSTRUCTION:
            diagnostics_dump_generic_exception("Illegal Instruction", exception_code, exception_address);
            break;
        case EXCEPTION_NONCONTINUABLE_EXCEPTION:
            diagnostics_dump_generic_exception("Noncontinuable Exception", exception_code, exception_address);
            break;
        case EXCEPTION_STACK_OVERFLOW:
            diagnostics_dump_generic_exception("Stack Overflow", exception_code, exception_address);
            break;
        case EXCEPTION_INVALID_DISPOSITION:
            diagnostics_dump_generic_exception("Invalid Disposition", exception_code, exception_address);
            break;
        case EXCEPTION_GUARD_PAGE:
            diagnostics_dump_generic_exception("Guard Page Violation", exception_code, exception_address);
            break;
        case EXCEPTION_INVALID_HANDLE:
            diagnostics_dump_generic_exception("Invalid Handle", exception_code, exception_address);
            break;
        case CONTROL_C_EXIT:
            diagnostics_dump_generic_exception("Ctrl+C Exit", exception_code, exception_address);
            break;
        default:
            diagnostics_dump_generic_exception("Unknown exception", exception_code, exception_address);
            break;
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

    // Any thread will do at this point
    size = diagnostics_threads.size();
    for (uiIndex = 0; uiIndex < size; uiIndex++) {
        if (diagnostics_threads[uiIndex]->crash_exception_record) {
            uiReturn = 
                diagnostics_threads[uiIndex]->crash_exception_record->ExceptionRecord->ExceptionCode;
        }
    }

    return uiReturn;
}


UINT WINAPI diagnostics_unhandled_exception_monitor(LPVOID /* lpParameter */) {
    DWORD        dwEvent = (DWORD)NULL;
    BOOL         bContinue = TRUE;
    BOOL         bDebuggerInitialized = FALSE;
    HANDLE       hEvents[2];
    unsigned int i;
    CONTEXT      c;
    BOINC_WINDOWCAPTURE window_info;
    PBOINC_THREADLISTENTRY pThreadEntry = NULL;

    // We should not suspend our crash dump thread.
    diagnostics_set_thread_exempt_suspend();

    // Aquire the mutex that will keep all the threads that throw an exception
    //   at bay until we are ready to deal with them.
    WaitForSingleObject(hExceptionMonitorHalt, INFINITE);

    // Which events do we want to wait for?
    hEvents[0] = hExceptionQuitEvent;
    hEvents[1] = hExceptionDetectedEvent;

    // Notify the initialization thread that initialization is complete and now
    //  we are waiting for an exception event.
    SetEvent(hExceptionMonitorStartedEvent);

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

                // Dump some basic stuff about runtime debugger version and date
                diagnostics_unhandled_exception_dump_banner();

#ifndef __CYGWIN__
                // Kickstart the debugger extensions, look for the debugger files
                //   in the install directory if it is defined, otherwise look
                //   in the data directory.
                if (0 != strlen(diagnostics_get_boinc_install_dir())) {
 	                bDebuggerInitialized = !DebuggerInitialize(
                        diagnostics_get_boinc_install_dir(),
                        diagnostics_get_symstore(),
                        diagnostics_is_proxy_enabled(),
                        diagnostics_get_proxy()
                    );
                } else {
 	                bDebuggerInitialized = !DebuggerInitialize(
                        diagnostics_get_boinc_dir(),
                        diagnostics_get_symstore(),
                        diagnostics_is_proxy_enabled(),
                        diagnostics_get_proxy()
                    );
                }

                // Dump any useful information
                if (bDebuggerInitialized) DebuggerDisplayDiagnostics();
#endif
                // Dump the process statistics
                diagnostics_dump_process_information();

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
                            if (bDebuggerInitialized) {
                                if (pThreadEntry->crash_exception_record ) {
                                    StackwalkFilter(
                                        pThreadEntry->crash_exception_record,
                                        EXCEPTION_EXECUTE_HANDLER
                                    );
                                } else {
                                    // Suspend thread before extracting the contexts, 
                                    //   otherwise it'll be trash.
                                    SuspendThread(pThreadEntry->thread_handle);
                                    
                                    // Get the thread context
                                    memset(&c, 0, sizeof(CONTEXT));
                                    c.ContextFlags = CONTEXT_FULL;
				                    GetThreadContext(pThreadEntry->thread_handle, &c);

                                    StackwalkThread(
                                        pThreadEntry->thread_handle,
                                        &c
                                    );
                                }
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
                    TerminateProcess(GetCurrentProcess(), (UINT)ERR_ABORTED_VIA_GUI);
                } else {
                    TerminateProcess(GetCurrentProcess(), diagnostics_determine_exit_code());
                }

                break;
        }
    }

    // Notify the calling thread that the message monitoring thread is
    //   finished.
    SetEvent(hExceptionQuitFinishedEvent);
    return 0;
}


static int no_reset[SIGRTMAX+1];
static int no_ignore[SIGRTMAX+1];
static int setup_arrays=0;


void setup_no_reset() {
  no_reset[SIGILL]=1;
#ifdef SIGTRAP
  no_reset[SIGTRAP]=1;
#endif
#ifdef SIGPRIV
  no_reset[SIGPRIV]=1;
#endif
  no_reset[SIGINT]=1;
};


void setup_no_ignore() {
#ifdef SIGKILL
  no_ignore[SIGKILL]=1;
#endif
#ifdef SIGSTOP
  no_ignore[SIGSTOP]=1;
#endif
  no_ignore[SIGSEGV]=1;
};


LONG pass_to_signal_handler(int signum) {
    void (*handler)(int);
    
    if (!setup_arrays) {
        setup_arrays=1;
        setup_no_ignore();
        setup_no_reset();
    }

    // Are we using the default signal handler?
    // If so return to the exception handler.
    handler=signal(signum,SIG_DFL);
    if (handler==SIG_DFL) {
       return EXCEPTION_CONTINUE_SEARCH;
    }

    // Should we ignore this signal?
    if (handler==SIG_IGN) {
        signal(signum,handler);
        // Are we allowed to?
        if (!no_ignore[signum]) {
            // Yes? Attempt to ignore the exception.
            return EXCEPTION_CONTINUE_EXECUTION;
        } else {  
            return EXCEPTION_CONTINUE_SEARCH; 
        }
    }

    // Call our signal handler, this probably won't return...
    handler(signum);

    // if it does, reset the signal handler if appropriate.
    if (no_reset[signum]) signal(signum,handler);

    // try to continue execution
    return EXCEPTION_CONTINUE_EXECUTION;
}


// Allow apps to install signal handlers for some exceptions that bypass
// the boinc diagnostics.  This translates the Windows exceptions into
// standard signals.
LONG diagnostics_check_signal_handlers(PEXCEPTION_POINTERS pExPtrs) {
    switch (pExPtrs->ExceptionRecord->ExceptionCode) {
      case CONTROL_C_EXIT:                
                                       return pass_to_signal_handler(SIGINT);
      case EXCEPTION_BREAKPOINT:
      case EXCEPTION_SINGLE_STEP:
#ifdef SIGTRAP
                                       return pass_to_signal_handler(SIGTRAP);
#else
                                       break;
#endif
      case EXCEPTION_FLT_DENORMAL_OPERAND:
      case EXCEPTION_FLT_DIVIDE_BY_ZERO:
      case EXCEPTION_FLT_INEXACT_RESULT:
      case EXCEPTION_FLT_INVALID_OPERATION:
      case EXCEPTION_FLT_OVERFLOW:
      case EXCEPTION_FLT_UNDERFLOW:      
                                       {
                                         LONG rv=pass_to_signal_handler(SIGFPE);
                                         /* MS claims ignoring an FP signal
                                          * results in an unknown FP state.
                                          * Does an _fpreset() help?
                                          */
                                         if (rv != EXCEPTION_CONTINUE_SEARCH) 
                                             _fpreset();
                                         return rv;
                                       }
      case EXCEPTION_INT_DIVIDE_BY_ZERO:
      case EXCEPTION_INT_OVERFLOW:
                                       return pass_to_signal_handler(SIGFPE);
      case EXCEPTION_PRIV_INSTRUCTION:
#ifdef SIGPRIV
                                       return pass_to_signal_handler(SIGPRIV);
                                       // nobreak
#endif
      case EXCEPTION_ILLEGAL_INSTRUCTION:
                                       return pass_to_signal_handler(SIGILL);
                                       // nobreak
      case EXCEPTION_DATATYPE_MISALIGNMENT:
#ifdef SIGBUS
                                       return pass_to_signal_handler(SIGBUS);
                                       // nobreak
#endif
      case EXCEPTION_STACK_OVERFLOW:
      case EXCEPTION_ACCESS_VIOLATION:
      case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:
      case EXCEPTION_IN_PAGE_ERROR:
                                       return pass_to_signal_handler(SIGSEGV);
                                       // nobreak
      default:                         break;
    }
    return EXCEPTION_CONTINUE_SEARCH;
}


// Let the unhandled exception monitor take care of logging the exception data.
//   Store the exception pointers and then singal the exception monitor to start
//   partying on the data.
LONG CALLBACK boinc_catch_signal(PEXCEPTION_POINTERS pExPtrs) {

    // Check whether somone has installed a standard C signal handler to
    // handle this exception. 
    if (diagnostics_check_signal_handlers(pExPtrs) == EXCEPTION_CONTINUE_EXECUTION) {
        return EXCEPTION_CONTINUE_EXECUTION;
    }

    fprintf( stderr, "\n\n");
    fprintf( stderr, "Unhandled Exception Detected...\n\n");

    // Dump what we know about...
    diagnostics_dump_exception_record(pExPtrs);

    if (hExceptionMonitorThread) {

        // Engage the BOINC Windows Runtime Debugger and dump as much diagnostic
        //   data as possible.
        //
        fprintf( stderr, "Engaging BOINC Windows Runtime Debugger...\n\n");

        // Store the exception record pointers.
        diagnostics_set_thread_exception_record(pExPtrs);

        // Wake the unhandled exception monitor up to process the exception.
        SetEvent(hExceptionDetectedEvent);

        // Go to sleep waiting for something this thread will never see.
        WaitForSingleObject(hExceptionMonitorHalt, INFINITE);

    } else {

        // This is a really bad place to be.  The unhandled exception monitor wasn't
        //   created, so we need to bail out as quickly as possible.
        //
        fprintf( stderr, "BOINC Windows Runtime Debugger not configured, terminating application...\n");

        // Enter the critical section in case multiple threads decide to try and blow
        //   chunks at the same time.  Let the OS decide who gets to determine what
        //   error code we return.
        EnterCriticalSection(&csExceptionMonitorFallback);

        TerminateProcess(GetCurrentProcess(), pExPtrs->ExceptionRecord->ExceptionCode);

        LeaveCriticalSection(&csExceptionMonitorFallback);
    }

    // We won't make it to this point, but make the compiler happy anyway.
    return EXCEPTION_CONTINUE_SEARCH;
}


// Starting with Visual Studio 2005 the C Runtime Library has really started to
//   enforce parameter validation. Problem is that the parameter validation code
//   uses its own structured exception handler and terminates without writing
//   any useful output to stderr. Microsoft has created a hook an application
//   developer can use to get more debugging information which is the purpose
//   of this function. When an invalid parameter is passed to the C Runtime
//   library this function will write whatever trace information it can and
//   then throw a breakpoint exception to dump all the rest of the useful
//   information.
void boinc_catch_signal_invalid_parameter(
    const wchar_t* expression, const wchar_t* function, const wchar_t* file, unsigned int line, uintptr_t /* pReserved */
) {
	fprintf(
		stderr,
        "ERROR: Invalid parameter detected in function %s. File: %s Line: %d\n",
		function,
		file,
		line
	);
	fprintf(
		stderr,
		"ERROR: Expression: %s\n",
		expression
	);

	// Cause a Debug Breakpoint.
	DebugBreak();
}

