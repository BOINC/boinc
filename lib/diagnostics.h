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

#ifndef _BOINC_DIAGNOSTICS_
#define _BOINC_DIAGNOSTICS_


#ifdef _WIN32
#include "boinc_win.h"
#else 
#include <signal.h>
#ifdef __cplusplus
#include <cassert>
#else
#include <assert.h>
#endif
#endif


// flags for boinc_init_diagnostics()
//
#define BOINC_DIAG_DUMPCALLSTACKENABLED     0x00000001L
#define BOINC_DIAG_HEAPCHECKENABLED         0x00000002L
#define BOINC_DIAG_MEMORYLEAKCHECKENABLED   0x00000004L
#define BOINC_DIAG_ARCHIVESTDERR            0x00000008L
#define BOINC_DIAG_ARCHIVESTDOUT            0x00000010L
#define BOINC_DIAG_REDIRECTSTDERR           0x00000020L
#define BOINC_DIAG_REDIRECTSTDOUT           0x00000040L
#define BOINC_DIAG_REDIRECTSTDERROVERWRITE  0x00000080L
#define BOINC_DIAG_REDIRECTSTDOUTOVERWRITE  0x00000100L
#define BOINC_DIAG_TRACETOSTDERR            0x00000200L
#define BOINC_DIAG_TRACETOSTDOUT            0x00000400L
#define BOINC_DIAG_HEAPCHECKEVERYALLOC      0x00000800L
#define BOINC_DIAG_BOINCAPPLICATION         0x00001000L
#define BOINC_DIAG_DEFAULTS \
    BOINC_DIAG_DUMPCALLSTACKENABLED | \
    BOINC_DIAG_HEAPCHECKENABLED | \
    BOINC_DIAG_MEMORYLEAKCHECKENABLED | \
    BOINC_DIAG_REDIRECTSTDERR | \
    BOINC_DIAG_TRACETOSTDERR


// filenames
//
#define BOINC_DIAG_STDERR                  "stderr"
#define BOINC_DIAG_STDOUT                  "stdout"
#define BOINC_DIAG_GFX_STDERR              "stderrgfx"
#define BOINC_DIAG_GFX_STDOUT              "stdoutgfx"


#ifdef __cplusplus
extern "C" {
#endif


// These are functions common to all platforms
extern int boinc_init_diagnostics( int flags );
extern int boinc_init_graphics_diagnostics( int flags );
extern int boinc_install_signal_handlers();
extern int boinc_finish_diag();

extern int diagnostics_init(
    int flags, const char* stdout_prefix, const char* stderr_prefix
);
extern int diagnostics_finish();
extern int diagnostics_is_initialized();
extern int diagnostics_is_flag_set(int flags);

// Properties
extern char* diagnostics_get_boinc_dir();
extern char* diagnostics_get_boinc_install_dir();
extern char* diagnostics_get_symstore();
extern int diagnostics_set_symstore(char* symstore);
extern int diagnostics_is_proxy_enabled();
extern char* diagnostics_get_proxy();

extern int diagnostics_is_aborted_via_gui();
extern int diagnostics_set_aborted_via_gui();

// Log rotation
extern int diagnostics_cycle_logs();
extern void diagnostics_set_max_file_sizes(int stdout_size, int stderr_size);

// Thread Tracking
extern int diagnostics_init_thread_list();
extern int diagnostics_finish_thread_list();
extern int diagnostics_update_thread_list();
extern int diagnostics_set_thread_exempt_suspend();
extern int diagnostics_is_thread_exempt_suspend(long thread_id);

// Message Monitoring
extern int diagnostics_init_message_monitor();
extern int diagnostics_finish_message_monitor();
#ifdef _WIN32
extern UINT WINAPI diagnostics_message_monitor(LPVOID lpParameter);
#endif

// Unhandled exception monitor
extern int diagnostics_init_unhandled_exception_monitor();
extern int diagnostics_finish_unhandled_exception_monitor();
#ifdef _WIN32
extern UINT WINAPI diagnostics_unhandled_exception_monitor(LPVOID lpParameter);
extern LONG CALLBACK boinc_catch_signal(EXCEPTION_POINTERS *ExceptionInfo);
extern void boinc_catch_signal_invalid_parameter(const wchar_t* expression, const wchar_t* function, const wchar_t* file, unsigned int line, uintptr_t pReserved);
#else
extern void boinc_catch_signal(int signal);
extern void boinc_set_signal_handler(int sig, void(*handler)(int));
extern void boinc_set_signal_handler_force(int sig, void(*handler)(int));
#endif


// These functions are used to log the various messages that are
//   defined in the BOINC Diagnostics Library
extern void boinc_trace(const char *pszFormat, ...);
extern void boinc_info(const char *pszFormat, ...);

extern void set_signal_exit_code(int);

#ifdef __cplusplus
}
#endif


#ifdef _WIN32

// Define macros for both debug and release builds.
//
// We are using the native debugging technology built into the Microsoft
//   C Runtime Libraries to trap and report the asserts and traces.
//

#ifdef _DEBUG

#if defined(WXDEBUG) || defined(WXNDEBUG)

// wxWidgets UI Framework
//

#define BOINCASSERT(expr)   wxASSERT(expr)
#ifdef _UNICODE
#define BOINCTRACE          __noop
#else
#define BOINCTRACE          wxLogDebug
#endif

#elif defined(_CONSOLE) && !(defined(__MINGW32__) || defined(__CYGWIN32__))

// Microsoft CRT
//

#define BOINCASSERT(expr)   _ASSERTE(expr)
#define BOINCTRACE          boinc_trace

#endif // _CONSOLE

#else  // _DEBUG

#if defined(__MINGW32__) || defined(__CYGWIN32__)
#define BOINCASSERT(expr)   assert(expr)
#define BOINCTRACE(...)     boinc_trace
#else  // __MINGW32__
#define BOINCASSERT(expr)   __noop
#define BOINCTRACE          boinc_trace
#endif // __MINGW32__

#endif // _DEBUG

#else

#ifdef _DEBUG

// Standard Frameworks
//

#define BOINCASSERT         assert
#define BOINCTRACE          boinc_trace

#else  // _DEBUG

#define BOINCASSERT(expr)         
#ifndef IRIX
#define BOINCTRACE(...)          
#endif

#endif // _DEBUG

#endif // ! _WIN32


// If none of the TRACE and INFO macros have been defined for
//   any existing framework to plug into, null them out.
// ASSERT is a special case and should point to the CRT version
//   if it hasn't already been redirected.

#ifndef BOINCASSERT
#define BOINCASSERT			assert
#endif

#ifndef BOINCTRACE
#define BOINCTRACE			
#endif

#ifndef BOINCINFO
#define BOINCINFO			boinc_info
#endif

#endif
