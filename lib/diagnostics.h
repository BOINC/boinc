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

#include "exception.h"

#ifndef _BOINC_DIAGNOSTICS_
#define _BOINC_DIAGNOSTICS_


// Diagnostics Support for Windows 95/98/ME/2000/XP/2003
//
#ifdef _WIN32

// Define macros for both debug and release builds.
//
// We are using the native debugging technology built into the Microsoft
//   C Runtime Libraries to trap and report the asserts and traces.
//

// Forward declare so we can assign a macro to it.
void	boinc_info_debug(const char *pszFormat, ...);
void	boinc_info_release(const char *pszFormat, ...);

#ifdef _DEBUG

#ifndef _CONSOLE

// Microsoft MFC/ATL
//

#define BOINCASSERT         ASSERT
#define BOINCTRACE          TRACE
#define BOINCINFO           boinc_info_debug

#else

// Microsoft CRT
//

#define BOINCASSERT(expr)   _ASSERT_BASE((expr), #expr)
#define BOINCTRACE          boinc_trace
#define BOINCINFO           boinc_info_debug

#endif // _CONSOLE

#else  // _DEBUG

#define BOINCASSERT(expr)   ((void)0)
#define BOINCTRACE          ((void)0)
#define BOINCINFO           boinc_info_release

#endif // _DEBUG

#endif // _WIN32


// Diagnostics Support for All Platforms
//
#ifdef _DEBUG

#define BOINCERROR( errmsg ) \
    throw boinc_runtime_base_exception( __FILE__, __LINE__, errmsg )
#define BOINCMEMORYERROR( errmsg ) \
    throw boinc_out_of_memory_exception( __FILE__, __LINE__, errmsg )
#define BOINCPARAMETERERROR( errmsg ) \
    throw boinc_invalid_parameter_exception( __FILE__, __LINE__, errmsg )
#define BOINCFILEERROR( errmsg ) \
    throw boinc_file_operation_exception( __FILE__, __LINE__, errmsg )
#define BOINCSIGNALERROR( errmsg ) \
    throw boinc_signal_operation_exception( __FILE__, __LINE__, errmsg )

#else // _DEBUG

#define BOINCERROR( errmsg ) \
    throw boinc_runtime_base_exception( errmsg )
#define BOINCMEMORYERROR( errmsg ) \
    throw boinc_out_of_memory_exception( errmsg )
#define BOINCPARAMETERERROR( errmsg ) \
    throw boinc_invalid_parameter_exception( errmsg )
#define BOINCFILEERROR( errmsg ) \
    throw boinc_file_operation_exception( errmsg )
#define BOINCSIGNALERROR( errmsg ) \
    throw boinc_signal_operation_exception( errmsg )

#endif // _DEBUG


// Diagnostics Support for Undefined Platform
//
#ifndef BOINCASSERT
#define BOINCASSERT			assert
#endif

#ifndef BOINCTRACE
#define BOINCTRACE			((int)0)
#endif

#ifndef BOINCINFO
#define BOINCINFO			((int)0)
#endif


// Diagnostics Functions
//
extern int boinc_init_diag(int);
extern int boinc_finish_diag();

extern int boinc_install_signal_handlers();
extern void boinc_set_signal_handler(int sig, RETSIGTYPE (*handler)(int));
extern void boinc_set_signal_handler_force(int sig, RETSIGTYPE (*handler)(int));

// Diagnostic Flags
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


// Diagnostics Contants
//
#define BOINC_DIAG_STDERR                  "stderr.txt"
#define BOINC_DIAG_STDERROLD               "stderr.old"
#define BOINC_DIAG_STDOUT                  "stdout.txt"
#define BOINC_DIAG_STDOUTOLD               "stdout.old"


#endif //_BOINC_DIAGNOSTICS_
