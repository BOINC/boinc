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

#ifndef _BOINC_DIAGNOSTICS_
#define _BOINC_DIAGNOSTICS_


// NOTE: No other includes should be included from this file.
//       Whatever is defined in this file should fit into
//       any existing frameworks that might be used by BOINC.
//
#ifdef __cplusplus
#include <cassert>
#else
#include <assert.h>
#endif

#ifndef _WIN32
#include <signal.h>
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


// filenames
//
#define BOINC_DIAG_STDERR                  "stderr"
#define BOINC_DIAG_STDOUT                  "stdout"



#ifdef __cplusplus
extern "C" {
#endif


// These are functions common to all platforms
extern int boinc_init_diagnostics(int flags);
extern int boinc_finish_diag();
extern int boinc_install_signal_handlers();

extern int diagnostics_init(
    int flags, const char* stdout_prefix, const char* stderr_prefix
);
extern int diagnostics_cycle_logs();


// These are functions that are specific to Unix
#ifndef _WIN32

extern void boinc_set_signal_handler(int sig, void(*handler)(int));
extern void boinc_set_signal_handler_force(int sig, void(*handler)(int));

#endif


// These functions are used to log the various messages that are
//   defined in the BOINC Diagnostics Library
extern void boinc_trace(const char *pszFormat, ...);
extern void boinc_info_debug(const char *pszFormat, ...);
extern void boinc_info_release(const char *pszFormat, ...);


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

#define BOINCASSERT(expr)   __noop
#define BOINCTRACE          __noop
#define BOINCINFO           boinc_info_release

#elif defined(_CONSOLE)

// Microsoft CRT
//

#define BOINCASSERT(expr)   _ASSERT_BASE((expr), #expr)
#define BOINCTRACE          boinc_trace
#define BOINCINFO           boinc_info_debug

#endif // _CONSOLE

#else  // _DEBUG

#define BOINCASSERT(expr)   __noop
#define BOINCTRACE          __noop
#define BOINCINFO           boinc_info_release

#endif // _DEBUG

#else

#ifdef _DEBUG

// Standard Frameworks
//

#define BOINCASSERT         assert
#define BOINCTRACE          boinc_trace
#define BOINCINFO           boinc_info_debug

#else  // _DEBUG

#define BOINCASSERT(expr)         
#define BOINCTRACE(...)          
#define BOINCINFO           boinc_info_release

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
#define BOINCINFO			
#endif

#endif
