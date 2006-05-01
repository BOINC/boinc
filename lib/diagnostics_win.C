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

#if defined(_WIN32) && !defined(__STDWX_H__) && !defined(_BOINC_WIN_) && !defined(_AFX_STDAFX_H_)
#include "boinc_win.h"
#endif

#if defined(_WIN32) && !defined(__CYGWIN32__)
#include "stackwalker_win.h"
#endif

#include "diagnostics.h"
#include "error_numbers.h"
#include "util.h"


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
    std::string name;
    DWORD       thread_id;
    HANDLE      thread_handle;
} BOINC_THREADLISTENTRY, *PBOINC_THREADLISTENTRY;

static std::vector<PBOINC_THREADLISTENTRY> diagnostics_threads;
static HANDLE hThreadListSync;


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


// Enumerate the running threads in the process space and add them to
//   the list.

// CreateToolhelp32Snapshot
typedef HANDLE (WINAPI *tCT32S)(DWORD dwFlags, DWORD dwProcessID);
// Thread32First
typedef BOOL (WINAPI *tT32F)(HANDLE hSnapshot, LPTHREADENTRY32 lpte);
// Thread32Next
typedef BOOL (WINAPI *tT32N)(HANDLE hSnapshot, LPTHREADENTRY32 lpte);
// OpenThread
typedef HANDLE (WINAPI *tOT)(DWORD dwDesiredAccess, BOOL bInheritHandle, DWORD dwThreadId);

int diagnostics_enum_thread_list() {
    HANDLE  hThreadSnap = INVALID_HANDLE_VALUE; 
    HANDLE  hThread = NULL;
    HMODULE hKernel32 = NULL;
    PBOINC_THREADLISTENTRY pThreadEntry = NULL;
    tCT32S  pCT32S = NULL;
    tT32F   pT32F = NULL;
    tT32N   pT32N = NULL;
    tOT     pOT = NULL;
    THREADENTRY32 te32; 
    unsigned int i;

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
            pThreadEntry = NULL;
            for (i=0; i<diagnostics_threads.size(); i++) {
                if (diagnostics_threads[i]) {
                    if (te32.th32ThreadID == diagnostics_threads[i]->thread_id) {
                        pThreadEntry = diagnostics_threads[i];
                    }
                }
            }

            if (!pThreadEntry) {
                pThreadEntry = new BOINC_THREADLISTENTRY;
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


// Set the current threads name to make it easy to know what the
//   thread is supposed to be doing.
int diagnostics_set_thread_name( char* name ) {
    HANDLE hThread;
    PBOINC_THREADLISTENTRY pThreadEntry = NULL;
    unsigned int i;


    // Wait for the ThreadListSync mutex before writing updates
    WaitForSingleObject(hThreadListSync, INFINITE);

    for (i=0; i<diagnostics_threads.size(); i++) {
        if (diagnostics_threads[i]) {
            if (GetCurrentThreadId() == diagnostics_threads[i]->thread_id) {
                pThreadEntry = diagnostics_threads[i];
            }
        }
    }

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
        pThreadEntry->name = name;
        pThreadEntry->thread_id = GetCurrentThreadId();
        pThreadEntry->thread_handle = hThread;
        diagnostics_threads.push_back(pThreadEntry);
    }

    // Release the Mutex
    ReleaseMutex(hThreadListSync);

    return 0;
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
static HANDLE hMessageMonitorSync;
static HANDLE hMessageSharedMap;
static HANDLE hMessageAckEvent;
static HANDLE hMessageReadyEvent;
static HANDLE hMessageQuitEvent;


int diagnostics_init_message_monitor() {
    int retval = 0;
    unsigned int i;
    HANDLE hThread;
    DWORD  dwThreadId;
    PBOINC_MESSAGEMONITORENTRY pMessageEntry = NULL;
    std::vector<PBOINC_MESSAGEMONITORENTRY>::iterator message_iter;

    SECURITY_ATTRIBUTES sa;
    SECURITY_DESCRIPTOR sd;

    sa.nLength = sizeof(SECURITY_ATTRIBUTES);
    sa.bInheritHandle = TRUE;
    sa.lpSecurityDescriptor = &sd;


    // Create a Mutex that can be used to syncronize data access
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

        hMessageAckEvent = CreateEvent(&sa, TRUE, FALSE, "DBWIN_BUFFER_READY");
        hMessageReadyEvent = CreateEvent(&sa, TRUE, FALSE, "DBWIN_DATA_READY");
        hMessageQuitEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

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
            FILE_MAP_READ,
            0,                       // file offset high
            0,                       // file offset low
            sizeof(DEBUGGERMESSAGE)  // # of bytes to map (entire file)
        );

        hThread = CreateThread(
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
            "[%s] %s\n",
            time_to_string(pMessageEntry->timestamp),
            pMessageEntry->message.c_str()
        );
    }

    fprintf(stderr, "\n\n");

    // Release the Mutex
    ReleaseMutex(hMessageMonitorSync);

    return 0;
}


DWORD WINAPI diagnostics_message_monitor(LPVOID lpParameter) {
    DWORD       dwEvent = NULL;
    DWORD       dwCurrentProcessId = NULL;
    BOOL        bContinue = TRUE;
    DWORD       dwRepeatMessageCounter = 0;
    DWORD       dwRepeatMessageProcessId = 0;
    std::string strRepeatMessage;
    PBOINC_MESSAGEMONITORENTRY pMessageEntry = NULL;
    std::vector<PBOINC_MESSAGEMONITORENTRY>::iterator message_iter;
    HANDLE hEvents[2];

    // Set our friendly name
    diagnostics_set_thread_name("Debug Message Monitor");

    // Which events do we want to wait for?
    hEvents[0] = hMessageQuitEvent;
    hEvents[1] = hMessageReadyEvent;

    // Cache the current process id
    dwCurrentProcessId = GetCurrentProcessId();

    // Signal that the buffer is ready for action.
    SetEvent(hMessageAckEvent);

    while (bContinue) {
        dwEvent = WaitForMultipleObjects( 
            2,           // number of objects in array
            hEvents,     // array of objects
            FALSE,       // wait for any
            INFINITE     // indefinite wait
        );

        // Keep anything happening to the buffer until we figure out what to
        //   do with it.
        ResetEvent(hMessageAckEvent);
        ResetEvent(hMessageReadyEvent);

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
                //   and re-signal the event.  We'll go to sleep for 250 milliseconds
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
                        SetEvent(hMessageAckEvent);

                        dwRepeatMessageCounter = 0;
                        dwRepeatMessageProcessId = 0;
                        strRepeatMessage = "";
                    } else {
                        // Let another application have a go at the message.
                        SetEvent(hMessageReadyEvent);
                        Sleep(250);
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

                    // Buffer is ready to receive a new message.
                    SetEvent(hMessageAckEvent);
                }
                break;
        }
    }
    return 0;
}


// Used to unwind the stack and spew the callstack to stderr. Terminate the
//   process afterwards and return the exception code as the exit code.
//
LONG CALLBACK boinc_catch_signal(EXCEPTION_POINTERS *pExPtrs) {

// Removed due to a nested exception problem
// RTW: 09/30/2004
/*
    // Snagged from the latest stackwalker code base.  This allows us to grab
    //   callstacks even in a stack overflow scenario
    if (pExPtrs->ExceptionRecord->ExceptionCode == EXCEPTION_STACK_OVERFLOW)
    {
        static char MyStack[1024*128];  // be sure that we have enought space...
        // it assumes that DS and SS are the same!!! (this is the case for Win32)
        // change the stack only if the selectors are the same (this is the case for Win32)
        //__asm push offset MyStack[1024*128];
        //__asm pop esp;
        __asm mov eax,offset MyStack[1024*128];
        __asm mov esp,eax;
    }
*/

#ifdef _DEBUG

    if (diagnostics_is_flag_set(BOINC_DIAG_MEMORYLEAKCHECKENABLED)) {
        CLEAR_CRT_DEBUG_FIELD(_CRTDBG_LEAK_CHECK_DF);
    }

    if (diagnostics_is_flag_set(BOINC_DIAG_HEAPCHECKENABLED)) {
        CLEAR_CRT_DEBUG_FIELD(_CRTDBG_CHECK_EVERY_1024_DF);
    }

#endif // _DEBUG


    PVOID                  exceptionAddr = pExPtrs->ExceptionRecord->ExceptionAddress;
    DWORD                  exceptionCode = pExPtrs->ExceptionRecord->ExceptionCode;
    PBOINC_THREADLISTENTRY pThreadEntry = NULL;

    char                   status[256];
    char                   substatus[256];
    bool                   bDumpedException;
    unsigned int           i;
    CONTEXT                c;
    HWND                   hwndForeWindow;
    char                   window_name[256];
    char                   window_class[256];
    DWORD                  window_process_id;
    DWORD                  window_thread_id;

    static long   lDetectNestedException = 0;

    // If we've been in this procedure before, something went wrong so we immediately exit
    if (InterlockedIncrement(&lDetectNestedException) > 1) {
        TerminateProcess(GetCurrentProcess(), (UINT)ERR_NESTED_UNHANDLED_EXCEPTION_DETECTED);
    }

    // Enumerate through all the threads so we have a complete list of what we need to dump.
    diagnostics_enum_thread_list();


    // Get any data that will be needed later but will cause a deadlock if called after
    //   the other threads are suspended.
    hwndForeWindow = GetForegroundWindow();
    GetWindowText(hwndForeWindow, window_name, sizeof(window_name));
    GetClassName(hwndForeWindow, window_class, sizeof(window_class));
    window_thread_id = GetWindowThreadProcessId(hwndForeWindow, &window_process_id);


    // Wait for the ThreadListSync mutex before writing updates
    WaitForSingleObject(hThreadListSync, INFINITE);


    // Suspend the other threads.
    for (i=0; i<diagnostics_threads.size(); i++) {
        pThreadEntry = diagnostics_threads[i];
        if ((GetCurrentThreadId() != pThreadEntry->thread_id) && pThreadEntry->thread_id) {
			// Suspend the thread before getting the threads context, otherwise
            //   it'll be junk.
            if (pThreadEntry->thread_handle) {
                SuspendThread(pThreadEntry->thread_handle);
            }
        }
    }

#if !defined(__CYGWIN__)
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

    fprintf(stderr, "\n*** UNHANDLED EXCEPTION ****\n");
    if (EXCEPTION_ACCESS_VIOLATION == exceptionCode) {
        fprintf(stderr, "Reason: %s (0x%x) at address 0x%p %s\n\n", status, exceptionCode, exceptionAddr, substatus);
    } else {
        fprintf(stderr, "Reason: %s (0x%x) at address 0x%p\n\n", status, exceptionCode, exceptionAddr);
    }

#if !defined(__MINGW32__) && !defined(__CYGWIN32__)
    // Unwind the stack and spew it to stderr
    if (diagnostics_is_flag_set(BOINC_DIAG_DUMPCALLSTACKENABLED)) {
        // Dump the offending thread's stack first.
        bDumpedException = false;
        for (i=0; i<diagnostics_threads.size(); i++) {
            pThreadEntry = diagnostics_threads[i];
            if (GetCurrentThreadId() == pThreadEntry->thread_id) {
                fprintf(
                    stderr, 
                    "*** Dump of the %s(offending) thread (%x): ***\n",
                    pThreadEntry->name.c_str(),
                    pThreadEntry->thread_id
                );
                StackwalkFilter(pExPtrs, EXCEPTION_EXECUTE_HANDLER);
                fprintf(stderr, "\n");
                bDumpedException = true;
            }
        }
        if (!bDumpedException) {
            fprintf(
                stderr,
                "*** Dump of the (offending) thread (%x): ***\n",
                pThreadEntry->thread_id
            );
            StackwalkFilter(pExPtrs, EXCEPTION_EXECUTE_HANDLER);
            fprintf(stderr, "\n");
        }

        // Dump the other threads stack.
        for (i=0; i<diagnostics_threads.size(); i++) {
            pThreadEntry = diagnostics_threads[i];
            if ((GetCurrentThreadId() != pThreadEntry->thread_id) && pThreadEntry->thread_id) {
                // Get the thread context
                memset(&c, 0, sizeof(CONTEXT));
                c.ContextFlags = CONTEXT_FULL;
				GetThreadContext(pThreadEntry->thread_handle, &c);

				// Dump the thread's stack.
                fprintf(
                    stderr,
                    "*** Dump of the %s thread (%x): ***\n",
                    pThreadEntry->name.c_str(),
                    pThreadEntry->thread_id
                );
                StackwalkThread(pThreadEntry->thread_handle, &c);
				fprintf(stderr, "\n");
            }
        }
    }
#endif

    diagnostics_message_monitor_dump();

    fprintf(
        stderr,
        "*** Foreground Window Data ***\n"
        "    Window Name      : %s\n"
        "    Window Class     : %s\n"
        "    Window Process ID: %x\n"
        "    Window Thread ID : %x\n\n",
        window_name,
        window_class,
        window_process_id,
        window_thread_id
    );

    fprintf(stderr, "Exiting...\n");

    // Release the Mutex
    ReleaseMutex(hThreadListSync);

    // Force terminate the app letting BOINC know an exception has occurred.
    if (diagnostics_is_aborted_via_gui()) {
        TerminateProcess(GetCurrentProcess(), ERR_ABORTED_VIA_GUI);
    } else {
        TerminateProcess(GetCurrentProcess(), pExPtrs->ExceptionRecord->ExceptionCode);
    }

    // We won't make it to this point, but make the compiler happy anyway.
    return 1;
}


const char *BOINC_RCSID_5967ad204d = "$Id$";
