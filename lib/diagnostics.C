// The contents of this file are subject to the BOINC Public License
// Version 1.0 (the "License"); you may not use this file except in
// compliance with the License. You may obtain a copy of the License at
// http://boinc.berkeley.edu/license_1.0.txt
//
// Software distributed under the License is distributed on an "AS IS"
// basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the
// License for the specific language governing rights and limitations
// under the License.
//
// The Original Code is the Berkeley Open Infrastructure for Network Computing.
//
// The Initial Developer of the Original Code is the SETI@home project.
// Portions created by the SETI@home project are Copyright (C) 2002
// University of California at Berkeley. All Rights Reserved.
//
// Contributor(s):
//

#ifdef _WIN32
#include "stdafx.h"
#endif

#ifndef _WIN32
#include "config.h"
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#ifdef HAVE_SIGNAL_H
#include <signal.h>
#endif
#endif

#include "diagnostics.h"
#include "error_numbers.h"
#include "filesys.h"
#include "util.h"

// Diagnostics Global State
//

static int g_BOINCDIAG_dwDiagnosticsFlags;


#ifdef _WIN32

// Diagnostics Support for Windows 95/98/ME/2000/XP/2003
//

// Forward declare implementation specific functions - Windows Platform Only.
LONG CALLBACK     boinc_catch_signal(EXCEPTION_POINTERS *ExceptionInfo);
int __cdecl       boinc_message_reporting( int reportType, char *szMsg, int *retVal );

#else

// Diagnostics for POSIX Compatible systems.
//

// Forward declare implementation functions - POSIX Platform Only.
extern RETSIGTYPE boinc_catch_signal(int signal);
extern void       boinc_quit(int sig);

#endif


// Used to initialize the diagnostics environment.
//
int boinc_init_diag( int dwDiagnosticsFlags ) {

	void* lpRetVal;

    // Store Diagnostics Flags globally for use.
    g_BOINCDIAG_dwDiagnosticsFlags = dwDiagnosticsFlags;


    // Archive any old stderr.txt and stdout.txt files, if requested
    if ( g_BOINCDIAG_dwDiagnosticsFlags & BOINC_DIAG_ARCHIVESTDERR ) {
	    if ( 0 != boinc_copy( BOINC_DIAG_STDERR, BOINC_DIAG_STDERROLD ) )
		    BOINCFILEERROR( "Failed to archive existing stderr.txt file" );
    }

    if ( g_BOINCDIAG_dwDiagnosticsFlags & BOINC_DIAG_ARCHIVESTDOUT ) {
	    if ( 0 != boinc_copy( BOINC_DIAG_STDOUT, BOINC_DIAG_STDOUTOLD ) )
		    BOINCFILEERROR( "Failed to archive existing stdout.txt file" );
    }

    
    // Redirect stderr and/or stdout streams, if requested
    if ( g_BOINCDIAG_dwDiagnosticsFlags & BOINC_DIAG_REDIRECTSTDERR ) {
        lpRetVal = (void*) freopen(BOINC_DIAG_STDERR, "a", stderr);
	    if ( NULL == lpRetVal )
		    BOINCFILEERROR( "Failed to reopen stderr for diagnostics redirection" );
    }

    if ( g_BOINCDIAG_dwDiagnosticsFlags & BOINC_DIAG_REDIRECTSTDERROVERWRITE ) {
        lpRetVal = (void*) freopen(BOINC_DIAG_STDERR, "w", stderr);
	    if ( NULL == lpRetVal )
		    BOINCFILEERROR( "Failed to reopen stderr for diagnostics redirection (overwrite)" );
    }

    if ( g_BOINCDIAG_dwDiagnosticsFlags & BOINC_DIAG_REDIRECTSTDOUT ) {
	    lpRetVal = (void*) freopen(BOINC_DIAG_STDOUT, "a", stdout);
	    if ( NULL == lpRetVal )
		    BOINCFILEERROR( "Failed to reopen stdout for diagnostics redirection" );
    }

    if ( g_BOINCDIAG_dwDiagnosticsFlags & BOINC_DIAG_REDIRECTSTDOUTOVERWRITE ) {
	    lpRetVal = (void*) freopen(BOINC_DIAG_STDOUT, "w", stdout);
	    if ( NULL == lpRetVal )
		    BOINCFILEERROR( "Failed to reopen stdout for diagnostics redirection (overwrite)" );
    }


#if defined(_WIN32) && defined(_DEBUG)

#ifndef _CONSOLE

    // MFC by default, configures itself for the memory leak detection on exit
    //

    //if ( g_BOINCDIAG_dwDiagnosticsFlags & BOINC_DIAG_MEMORYLEAKCHECKENABLED )
    //    SET_CRT_DEBUG_FIELD( _CRTDBG_LEAK_CHECK_DF );

    if ( g_BOINCDIAG_dwDiagnosticsFlags & BOINC_DIAG_HEAPCHECKENABLED ) {
        AfxEnableMemoryTracking(TRUE);
        afxMemDF = allocMemDF | checkAlwaysMemDF;
    }

#else

    _CrtSetReportHook( boinc_message_reporting );

    if ( g_BOINCDIAG_dwDiagnosticsFlags & BOINC_DIAG_MEMORYLEAKCHECKENABLED )
        SET_CRT_DEBUG_FIELD( _CRTDBG_LEAK_CHECK_DF );

    if ( g_BOINCDIAG_dwDiagnosticsFlags & BOINC_DIAG_HEAPCHECKENABLED )
        SET_CRT_DEBUG_FIELD( _CRTDBG_CHECK_EVERY_1024_DF );

#endif // _CONSOLE

#endif // defined(_WIN32) && defined(_DEBUG)


    // Install unhandled exception filters and signal traps.
    if ( BOINC_SUCCESS != boinc_install_signal_handlers() )
		BOINCSIGNALERROR( "Failed to install signal handlers" );


	return BOINC_SUCCESS;
}


// Used to cleanup the diagnostics environment.
//
int boinc_finish_diag() {
	return BOINC_SUCCESS;
}


// Used to setup an unhandled exception filter on Windows
//
int boinc_install_signal_handlers() {

#ifdef _WIN32

    SetUnhandledExceptionFilter( boinc_catch_signal );

#else  //_WIN32

    boinc_set_signal_handler(SIGHUP, boinc_catch_signal);
    boinc_set_signal_handler(SIGINT, boinc_catch_signal);
    boinc_set_signal_handler(SIGQUIT, boinc_catch_signal);
    boinc_set_signal_handler(SIGILL, boinc_catch_signal);
    boinc_set_signal_handler(SIGABRT, boinc_catch_signal);
    boinc_set_signal_handler(SIGBUS, boinc_catch_signal);
    boinc_set_signal_handler(SIGSEGV, boinc_catch_signal);
    boinc_set_signal_handler(SIGSYS, boinc_catch_signal);
    boinc_set_signal_handler(SIGPIPE, boinc_catch_signal);

#endif //_WIN32

	return BOINC_SUCCESS;
}


#ifdef _WIN32

// Used to unwind the stack and spew the callstack to stderr. Terminate the
//   process afterwards and return the exception code as the exit code.
//
LONG CALLBACK boinc_catch_signal(EXCEPTION_POINTERS *pExPtrs) {

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

	PVOID exceptionAddr = pExPtrs->ExceptionRecord->ExceptionAddress;
    DWORD exceptionCode = pExPtrs->ExceptionRecord->ExceptionCode;

	LONG  lReturnValue = NULL;
	char  status[256];
	char  substatus[256];
    
	static long   lDetectNestedException = 0;

    // If we've been in this procedure before, something went wrong so we immediately exit
	if ( InterlockedIncrement(&lDetectNestedException) > 1 ) {
		TerminateProcess( GetCurrentProcess(), ERR_NESTED_UNHANDLED_EXCEPTION_DETECTED );
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
    if ( g_BOINCDIAG_dwDiagnosticsFlags & BOINC_DIAG_DUMPCALLSTACKENABLED )
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

			OutputDebugString(szMsg);			// Reports string to the debugger output window
			
            if ( g_BOINCDIAG_dwDiagnosticsFlags & BOINC_DIAG_TRACETOSTDERR ) {
                fprintf( stderr, "%s", szMsg );
			    fflush( stderr );
            }
			
            if ( g_BOINCDIAG_dwDiagnosticsFlags & BOINC_DIAG_TRACETOSTDOUT ) {
                fprintf( stdout, "%s", szMsg );
			    fflush( stdout );
            }

            break;
		case _CRT_ASSERT:
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
void boinc_trace(const char *pszFormat, ...)
{
	static char szBuffer[1024];

	// Trace messages should only be reported if running as a standalone 
	//   application or told too.
	if ( (g_BOINCDIAG_dwDiagnosticsFlags & BOINC_DIAG_TRACETOSTDERR) ||
         (g_BOINCDIAG_dwDiagnosticsFlags & BOINC_DIAG_TRACETOSTDOUT) ) {

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
void boinc_info_debug(const char *pszFormat, ...)
{
	static char szBuffer[1024];

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
void boinc_info_release(const char *pszFormat, ...)
{
	static char szBuffer[1024];

	memset(szBuffer, 0, sizeof(szBuffer));

	va_list ptr;
	va_start(ptr, pszFormat);

	_vsnprintf(szBuffer, sizeof(szBuffer), pszFormat, ptr);

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
void boinc_set_signal_handler(int sig, RETSIGTYPE (*handler)(int)) {
#ifdef HAVE_SIGACTION
    struct sigaction temp;
    sigaction(sig, NULL, &temp);
    if (temp.sa_handler != SIG_IGN) {
        temp.sa_handler = handler;
        sigemptyset(&temp.sa_mask);
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
void boinc_set_signal_handler_force(int sig, RETSIGTYPE (*handler)(int)) {
#ifdef HAVE_SIGACTION
    struct sigaction temp;
    sigaction(sig, NULL, &temp);
    temp.sa_handler = handler;
    sigemptyset(&temp.sa_mask);
    sigaction(sig, &temp, NULL);
#else
    void (*temp)(int);
    temp = signal(sig, handler);
    signal(sig, SIG_IGN);
#endif /* HAVE_SIGACTION */
}


RETSIGTYPE boinc_catch_signal(int signal) {
    switch(signal) {
        case SIGHUP: fprintf(stderr, "SIGHUP: terminal line hangup"); break;
        case SIGINT: fprintf(stderr, "SIGINT: interrupt program"); break;
        case SIGILL: fprintf(stderr, "SIGILL: illegal instruction"); break;
        case SIGABRT: fprintf(stderr, "SIGABRT: abort called"); break;
        case SIGBUS: fprintf(stderr, "SIGBUS: bus error"); break;
        case SIGSEGV: fprintf(stderr, "SIGSEGV: segmentation violation"); break;
        case SIGSYS: fprintf(stderr, "SIGSYS: system call given invalid argument"); break;
        case SIGPIPE: fprintf(stderr, "SIGPIPE: write on a pipe with no reader"); break;
        default: fprintf(stderr, "unknown signal %d", signal); break;
    }
    fprintf(stderr, "\nExiting...\n");
    exit(ERR_SIGNAL_CATCH);
}


/*
void boinc_quit(int sig) {
    signal(SIGQUIT, boinc_quit);    // reset signal
    time_to_quit = true;
}
*/

#endif
