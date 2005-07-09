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

// Stuff related to stderr/stdout direction and exception handling;
// used by both core client and by apps

#ifndef _WIN32
#include <cstdio>
#include <cstdarg>
#include <cstdlib>
#include "config.h"
#endif

#ifdef _WIN32
#include "stackwalker_win.h"
#endif

#ifdef __GLIBC__
#include <execinfo.h>
#endif

#include "diagnostics.h"
#include "error_numbers.h"
#include "filesys.h"
#include "util.h"


#define MAX_STDERR_FILE_SIZE        2048*1024
#define MAX_STDOUT_FILE_SIZE        2048*1024


static int   flags;
static char  stdout_log[256];
static char  stdout_archive[256];
static FILE* stdout_file;
static char  stderr_log[256];
static char  stderr_archive[256];
static FILE* stderr_file;


#ifdef _WIN32

LONG CALLBACK boinc_catch_signal(EXCEPTION_POINTERS *ExceptionInfo);
int __cdecl boinc_message_reporting( int reportType, char *szMsg, int *retVal );

#else

static void boinc_catch_signal(int signal);

#endif


// stub function for initializing the diagnostics environment.
//
int boinc_init_diagnostics(int _flags) {
    return diagnostics_init( _flags, BOINC_DIAG_STDOUT, BOINC_DIAG_STDERR );
}


// Used to cleanup the diagnostics environment.
//
int boinc_finish_diag() {
    return BOINC_SUCCESS;
}


// to setup an unhandled exception filter on Windows
//
int boinc_install_signal_handlers() {
#ifdef _WIN32
    SetUnhandledExceptionFilter( boinc_catch_signal );
#else  //_WIN32

    // register handlers for fatal internal signals
    // so that they get reported in stderr.txt
    // Do NOT catch SIGQUIT because core client uses that to kill app
    //
    boinc_set_signal_handler(SIGILL, boinc_catch_signal);
    boinc_set_signal_handler(SIGABRT, boinc_catch_signal);
    boinc_set_signal_handler(SIGBUS, boinc_catch_signal);
    boinc_set_signal_handler(SIGSEGV, boinc_catch_signal);
    boinc_set_signal_handler(SIGSYS, boinc_catch_signal);
    boinc_set_signal_handler(SIGPIPE, boinc_catch_signal);
#endif //_WIN32
    return 0;
}


// initialize the diagnostics environment.
//
int diagnostics_init(
    int _flags, const char* stdout_prefix, const char* stderr_prefix
) {

    flags = _flags;
    snprintf(stdout_log, sizeof(stdout_log), "%s.txt", stdout_prefix);
    snprintf(stdout_archive, sizeof(stdout_archive), "%s.old", stdout_prefix);
    snprintf(stderr_log, sizeof(stderr_log), "%s.txt", stderr_prefix);
    snprintf(stderr_archive, sizeof(stderr_archive), "%s.old", stderr_prefix);


    // Check for invalid parameter combinations
    if ( ( flags & BOINC_DIAG_REDIRECTSTDERR ) && ( flags & BOINC_DIAG_REDIRECTSTDERROVERWRITE ) ) {
        return ERR_INVALID_PARAM;
    }

    if ( ( flags & BOINC_DIAG_REDIRECTSTDOUT ) && ( flags & BOINC_DIAG_REDIRECTSTDOUTOVERWRITE ) ) {
        return ERR_INVALID_PARAM;
    }


    // Archive any old stderr.txt and stdout.txt files, if requested
    if ( flags & BOINC_DIAG_ARCHIVESTDERR ) {
        boinc_copy(stderr_log, stderr_archive);
    }

    if ( flags & BOINC_DIAG_ARCHIVESTDOUT) {
        boinc_copy( stdout_log, stdout_archive );
    }


    // Redirect stderr and/or stdout streams, if requested
    if (flags & BOINC_DIAG_REDIRECTSTDERR ) {
        stderr_file = freopen(stderr_log, "a", stderr);
        if ( NULL == stderr_file ) {
            return ERR_FOPEN;
        }
    }

    if (flags & BOINC_DIAG_REDIRECTSTDERROVERWRITE ) {
        stderr_file = freopen(stderr_log, "w", stderr);
        if ( NULL == stderr_file ) {
            return ERR_FOPEN;
        }
    }

    if (flags & BOINC_DIAG_REDIRECTSTDOUT ) {
        stdout_file = freopen(stdout_log, "a", stdout);
        if ( NULL == stdout_file ) {
            return ERR_FOPEN;
        }
    }

    if (flags & BOINC_DIAG_REDIRECTSTDOUTOVERWRITE ) {
        stdout_file = freopen(stdout_log, "w", stdout);
        if ( NULL == stdout_file ) {
            return ERR_FOPEN;
        }
    }


#if defined(_WIN32) && defined(_DEBUG)

    _CrtSetReportHook( boinc_message_reporting );

    if (flags & BOINC_DIAG_MEMORYLEAKCHECKENABLED )
        SET_CRT_DEBUG_FIELD( _CRTDBG_LEAK_CHECK_DF );

    if (flags & BOINC_DIAG_HEAPCHECKENABLED )
        if (flags & BOINC_DIAG_HEAPCHECKEVERYALLOC )
            SET_CRT_DEBUG_FIELD( _CRTDBG_CHECK_ALWAYS_DF );
        else
            SET_CRT_DEBUG_FIELD( _CRTDBG_CHECK_EVERY_1024_DF );

#endif // defined(_WIN32) && defined(_DEBUG)


    // Install unhandled exception filters and signal traps.
    if ( BOINC_SUCCESS != boinc_install_signal_handlers() ) {
        return ERR_SIGNAL_OP;
    }

    return BOINC_SUCCESS;
}


// Cycle the log files at regular events.
//
int diagnostics_cycle_logs() {
    double f_size;

    fflush(stdout);
    fflush(stderr);

    // If the stderr.txt or stdout.txt files are too big, cycle them
    //
    if (flags & BOINC_DIAG_REDIRECTSTDERR) {
        file_size(stderr_log, f_size);
        if (MAX_STDERR_FILE_SIZE < f_size) {
            fclose( stderr_file );
            boinc_copy(stderr_log, stderr_archive);
            stderr_file = freopen( stderr_log, "w", stderr );
            if ( NULL == stderr_file ) return ERR_FOPEN;
        }
    }

    if (flags & BOINC_DIAG_REDIRECTSTDOUT) {
        file_size(stdout_log, f_size);
        if (MAX_STDOUT_FILE_SIZE < f_size) {
            fclose( stdout_file );
            boinc_copy( stdout_log, stdout_archive );
            stdout_file = freopen( stdout_log, "w", stdout );
            if ( NULL == stdout_file ) return ERR_FOPEN;
        }
    }

    return BOINC_SUCCESS;
}


#ifdef _WIN32

// Used to unwind the stack and spew the callstack to stderr. Terminate the
//   process afterwards and return the exception code as the exit code.
//
LONG CALLBACK boinc_catch_signal(EXCEPTION_POINTERS *pExPtrs) {

// Removed due to a nested exception problem
// RTW: 09/30/2004
/*
    // Snagged from the latest stackwalker code base.  This allows us to grab
    //   callstacks even in a stack overflow scenario
    if ( pExPtrs->ExceptionRecord->ExceptionCode == EXCEPTION_STACK_OVERFLOW )
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

    if (flags & BOINC_DIAG_MEMORYLEAKCHECKENABLED )
        CLEAR_CRT_DEBUG_FIELD( _CRTDBG_LEAK_CHECK_DF );

    if (flags & BOINC_DIAG_HEAPCHECKENABLED )
        CLEAR_CRT_DEBUG_FIELD( _CRTDBG_CHECK_EVERY_1024_DF );

#endif // _DEBUG


    PVOID exceptionAddr = pExPtrs->ExceptionRecord->ExceptionAddress;
    DWORD exceptionCode = pExPtrs->ExceptionRecord->ExceptionCode;

    char  status[256];
    char  substatus[256];

    static long   lDetectNestedException = 0;

    // If we've been in this procedure before, something went wrong so we immediately exit
    if ( InterlockedIncrement(&lDetectNestedException) > 1 ) {
        TerminateProcess( GetCurrentProcess(), (UINT)ERR_NESTED_UNHANDLED_EXCEPTION_DETECTED );
    }

    switch ( exceptionCode ) {
        case EXCEPTION_ACCESS_VIOLATION:
            safe_strncpy( status, "Access Violation", sizeof(status) );
            if ( pExPtrs->ExceptionRecord->NumberParameters == 2 ) {
                switch( pExPtrs->ExceptionRecord->ExceptionInformation[0] ) {
                case 0: // read attempt
                    sprintf( substatus, "read attempt to address 0x%8.8X", pExPtrs->ExceptionRecord->ExceptionInformation[1] );
                    break;
                case 1: // write attempt
                    sprintf( substatus, "write attempt to address 0x%8.8X", pExPtrs->ExceptionRecord->ExceptionInformation[1] );
                    break;
                }
            }
            break;
        case EXCEPTION_DATATYPE_MISALIGNMENT:
            safe_strncpy( status, "Data Type Misalignment", sizeof(status) );
            break;
        case EXCEPTION_BREAKPOINT:
            safe_strncpy( status, "Breakpoint Encountered", sizeof(status) );
            break;
        case EXCEPTION_SINGLE_STEP:
            safe_strncpy( status, "Single Instruction Executed", sizeof(status) );
            break;
        case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:
            safe_strncpy( status, "Array Bounds Exceeded", sizeof(status) );
            break;
        case EXCEPTION_FLT_DENORMAL_OPERAND:
            safe_strncpy( status, "Float Denormal Operand", sizeof(status) );
            break;
        case EXCEPTION_FLT_DIVIDE_BY_ZERO:
            safe_strncpy( status, "Divide by Zero", sizeof(status) );
            break;
        case EXCEPTION_FLT_INEXACT_RESULT:
            safe_strncpy( status, "Float Inexact Result", sizeof(status) );
            break;
        case EXCEPTION_FLT_INVALID_OPERATION:
            safe_strncpy( status, "Float Invalid Operation", sizeof(status) );
            break;
        case EXCEPTION_FLT_OVERFLOW:
            safe_strncpy( status, "Float Overflow", sizeof(status) );
            break;
        case EXCEPTION_FLT_STACK_CHECK:
            safe_strncpy( status, "Float Stack Check", sizeof(status) );
            break;
        case EXCEPTION_FLT_UNDERFLOW:
            safe_strncpy( status, "Float Underflow", sizeof(status) );
            break;
        case EXCEPTION_INT_DIVIDE_BY_ZERO:
            safe_strncpy( status, "Integer Divide by Zero", sizeof(status) );
            break;
        case EXCEPTION_INT_OVERFLOW:
            safe_strncpy( status, "Integer Overflow", sizeof(status) );
            break;
        case EXCEPTION_PRIV_INSTRUCTION:
            safe_strncpy( status, "Privileged Instruction", sizeof(status) );
            break;
        case EXCEPTION_IN_PAGE_ERROR:
            safe_strncpy( status, "In Page Error", sizeof(status) );
            break;
        case EXCEPTION_ILLEGAL_INSTRUCTION:
            safe_strncpy( status, "Illegal Instruction", sizeof(status) );
            break;
        case EXCEPTION_NONCONTINUABLE_EXCEPTION:
            safe_strncpy( status, "Noncontinuable Exception", sizeof(status) );
            break;
        case EXCEPTION_STACK_OVERFLOW:
            safe_strncpy( status, "Stack Overflow", sizeof(status) );
            break;
        case EXCEPTION_INVALID_DISPOSITION:
            safe_strncpy( status, "Invalid Disposition", sizeof(status) );
            break;
        case EXCEPTION_GUARD_PAGE:
            safe_strncpy( status, "Guard Page Violation", sizeof(status) );
            break;
        case EXCEPTION_INVALID_HANDLE:
            safe_strncpy( status, "Invalid Handle", sizeof(status) );
            break;
        case CONTROL_C_EXIT:
            safe_strncpy( status, "Ctrl+C Exit", sizeof(status) );
            break;
        default:
            safe_strncpy( status, "Unknown exception", sizeof(status) );
            break;
    }

    fprintf( stderr, "\n***UNHANDLED EXCEPTION****\n" );
    if ( EXCEPTION_ACCESS_VIOLATION == exceptionCode ) {
        fprintf( stderr, "Reason: %s (0x%x) at address 0x%p %s\n\n", status, exceptionCode, exceptionAddr, substatus );
    } else {
        fprintf( stderr, "Reason: %s (0x%x) at address 0x%p\n\n", status, exceptionCode, exceptionAddr );
    }
    fflush( stderr );

    // Unwind the stack and spew it to stderr
    if (flags & BOINC_DIAG_DUMPCALLSTACKENABLED )
        StackwalkFilter( pExPtrs, EXCEPTION_EXECUTE_HANDLER, NULL );

    fprintf( stderr, "Exiting...\n" );
    fflush( stderr );

    // Force terminate the app letting BOINC know an exception has occurred.
    TerminateProcess( GetCurrentProcess(), pExPtrs->ExceptionRecord->ExceptionCode );

    // We won't make it to this point, but make the compiler happy anyway.
    return 1;
}


#ifdef _DEBUG


// Trap ASSERTs and TRACEs from the CRT and spew them to stderr.
//
int __cdecl boinc_message_reporting( int reportType, char *szMsg, int *retVal ){
    (*retVal) = 0;

    switch(reportType){

        case _CRT_WARN:
        case _CRT_ERROR:

            OutputDebugString(szMsg);            // Reports string to the debugger output window

            if (flags & BOINC_DIAG_TRACETOSTDERR ) {
                fprintf( stderr, szMsg );
                fflush( stderr );
            }

            if (flags & BOINC_DIAG_TRACETOSTDOUT ) {
                fprintf( stdout, szMsg );
                fflush( stdout );
            }

            break;
        case _CRT_ASSERT:

            OutputDebugString("ASSERT: ");      // Reports string to the debugger output window
            OutputDebugString(szMsg);           // Reports string to the debugger output window
            OutputDebugString("\n");            // Reports string to the debugger output window

            fprintf( stderr, "ASSERT: %s\n", szMsg );
            fflush( stderr );

            (*retVal) = 1;
            break;

    }

    return(TRUE);
}


// Converts the BOINCTRACE macro into a single string and report it
//   to the CRT so it can be reported via the normal means.
//
void boinc_trace(const char *pszFormat, ...) {
    static char szBuffer[4096];

    // Trace messages should only be reported if running as a standalone
    //   application or told too.
    if ((flags & BOINC_DIAG_TRACETOSTDERR) ||
         (flags & BOINC_DIAG_TRACETOSTDOUT) ) {

        memset(szBuffer, 0, sizeof(szBuffer));

        va_list ptr;
        va_start(ptr, pszFormat);

        BOINCASSERT( -1 != _vsnprintf(szBuffer, sizeof(szBuffer), pszFormat, ptr) );

        va_end(ptr);

        _CrtDbgReport(_CRT_WARN, NULL, NULL, NULL, "TRACE: %s", szBuffer);
    }
}


// Converts the BOINCINFO macro into a single string and report it
//   to stderr so it can be reported via the normal means.
//
void boinc_info_debug(const char *pszFormat, ...){
    static char szBuffer[4096];

    memset(szBuffer, 0, sizeof(szBuffer));

    va_list ptr;
    va_start(ptr, pszFormat);

    BOINCASSERT( -1 != _vsnprintf(szBuffer, sizeof(szBuffer), pszFormat, ptr) );

    va_end(ptr);

    _CrtDbgReport(_CRT_WARN, NULL, NULL, NULL, "%s", szBuffer);
}


#else // _DEBUG


// Converts the BOINCINFO macro into a single string and report it
//   to stderr so it can be reported via the normal means.
//
void boinc_info_release(const char *pszFormat, ...){
    static char szBuffer[4096];

    memset(szBuffer, 0, sizeof(szBuffer));

    va_list ptr;
    va_start(ptr, pszFormat);

    _vsnprintf(szBuffer, sizeof(szBuffer), pszFormat, ptr);

    va_end(ptr);

    fprintf( stderr, "%s", szBuffer );
    fflush( stderr );
}

#endif // _DEBUG


#else // _WIN32


#ifdef _DEBUG


// Converts the BOINCTRACE macro into a single string and report it
//   to the CRT so it can be reported via the normal means.
//
void boinc_trace(const char *pszFormat, ...) {
    static char szBuffer[4096];

    // Trace messages should only be reported if running as a standalone
    //   application or told too.
    if ((flags & BOINC_DIAG_TRACETOSTDERR) ||
         (flags & BOINC_DIAG_TRACETOSTDOUT) ) {

        memset(szBuffer, 0, sizeof(szBuffer));

        va_list ptr;
        va_start(ptr, pszFormat);

        BOINCASSERT( -1 != vsnprintf(szBuffer, sizeof(szBuffer), pszFormat, ptr) );

        va_end(ptr);

        if (flags & BOINC_DIAG_TRACETOSTDERR ) {
            fprintf( stderr, "TRACE: %s", szBuffer );
            fflush( stderr );
        }

        if (flags & BOINC_DIAG_TRACETOSTDOUT ) {
            fprintf( stdout, "TRACE: %s", szBuffer );
            fflush( stdout );
        }
    }
}


// Converts the BOINCINFO macro into a single string and report it
//   to stderr so it can be reported via the normal means.
//
void boinc_info_debug(const char *pszFormat, ...){
    static char szBuffer[4096];

    memset(szBuffer, 0, sizeof(szBuffer));

    va_list ptr;
    va_start(ptr, pszFormat);

    BOINCASSERT( -1 != vsnprintf(szBuffer, sizeof(szBuffer), pszFormat, ptr) );

    va_end(ptr);

    fprintf( stderr, "%s", szBuffer );
    fflush( stderr );
}


#else // _DEBUG


// Converts the BOINCINFO macro into a single string and report it
//   to stderr so it can be reported via the normal means.
//
void boinc_info_release(const char *pszFormat, ...){
    static char szBuffer[4096];

    memset(szBuffer, 0, sizeof(szBuffer));

    va_list ptr;
    va_start(ptr, pszFormat);

    vsnprintf(szBuffer, sizeof(szBuffer), pszFormat, ptr);

    va_end(ptr);

    fprintf( stderr, "%s", szBuffer );
    fflush( stderr );
}

#endif // _DEBUG


#endif // _WIN32


// Diagnostics for POSIX Compatible systems.
//

#ifdef HAVE_SIGNAL_H


// Set a signal handler only if it is not currently ignored
//
void boinc_set_signal_handler(int sig, void(*handler)(int)) {
#ifdef HAVE_SIGACTION
    struct sigaction temp;
    sigaction(sig, NULL, &temp);
    if (temp.sa_handler != SIG_IGN) {
        temp.sa_handler = handler;
    //        sigemptyset(&temp.sa_mask);
        sigaction(sig, &temp, NULL);
    }
#else
    void (*temp)(int);
    temp = signal(sig, boinc_catch_signal);
    if (temp == SIG_IGN) {
        signal(sig, SIG_IGN);
    }
#endif /* HAVE_SIGACTION */
}


// Set a signal handler even if it is currently ignored
//
void boinc_set_signal_handler_force(int sig, void(*handler)(int)) {
#ifdef HAVE_SIGACTION
    struct sigaction temp;
    sigaction(sig, NULL, &temp);
    temp.sa_handler = handler;
    //    sigemptyset(&temp.sa_mask);
    sigaction(sig, &temp, NULL);
#else
    void (*temp)(int);
    temp = signal(sig, boinc_catch_signal);
    signal(sig, SIG_IGN);
#endif /* HAVE_SIGACTION */
}


void boinc_catch_signal(int signal) {
    switch(signal) {
        case SIGHUP: fprintf(stderr, "SIGHUP: terminal line hangup");
             return;
        case SIGINT: fprintf(stderr, "SIGINT: interrupt program"); break;
        case SIGILL: fprintf(stderr, "SIGILL: illegal instruction"); break;
        case SIGABRT: fprintf(stderr, "SIGABRT: abort called"); break;
        case SIGBUS: fprintf(stderr, "SIGBUS: bus error"); break;
        case SIGSEGV: fprintf(stderr, "SIGSEGV: segmentation violation"); break;
        case SIGSYS: fprintf(stderr, "SIGSYS: system call given invalid argument"); break;
        case SIGPIPE: fprintf(stderr, "SIGPIPE: write on a pipe with no reader");
            return;
        default: fprintf(stderr, "unknown signal %d", signal); break;
    }

#ifdef __GLIBC__
    void *array[64];
    size_t size;
    size = backtrace (array, 64);
    fprintf(stderr, "Stack trace (%d frames):\n", size);
    backtrace_symbols_fd(array, size, fileno(stderr));
#endif

    fprintf(stderr, "\nExiting...\n");
    exit(ERR_SIGNAL_CATCH);
}


#endif

const char *BOINC_RCSID_4967ad204c = "$Id$";
