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

#ifndef _BOINC_DIAGNOSTICS_
#define _BOINC_DIAGNOSTICS_


#ifdef _WIN32
#include <crtdbg.h>
#endif

#include <assert.h>

#include "boinc_exception.h"


// ****************************************************************************
// ****************************************************************************
//
// Diagnostics Support for Windows 95/98/ME/2000/XP/2003
//
// ****************************************************************************
// ****************************************************************************

#ifdef _WIN32

//
// Define macros for both debug and release builds.
//
// We are using the native debugging technology built into the Microsoft
//   C Runtime Libraries to trap and report the asserts and traces.
//

#ifdef _DEBUG

// Forward declare so we can assign a macro to it.

void	boinc_trace(const char *pszFormat, ...);
void	boinc_info_debug(const char *pszFormat, ...);

#define BOINCASSERT(expr)	_ASSERT_BASE((expr), #expr)
#define BOINCTRACE			boinc_trace
#define BOINCINFO			boinc_info_debug
#define BOINCERROR( err, errmsg ) \
    throw #err( __FILE__, __LINE__, errmsg )
	

#else // _DEBUG

// Forward declare so we can assign a macro to it.
void	boinc_info_release(const char *pszFormat, ...);

#define BOINCASSERT(expr)	((void)0)
#define BOINCTRACE			((void)0)
#define BOINCINFO			boinc_info_release
#define BOINCERROR( err, errmsg ) \
	throw #err( errmsg )

#endif // _DEBUG

#endif // _WIN32


// ****************************************************************************
// ****************************************************************************
//
// Diagnostics Support for Undefined Platform
//
// ****************************************************************************
// ****************************************************************************
#ifndef BOINCASSERT
#define BOINCASSERT			assert
#endif

#ifndef BOINCTRACE
#define BOINCTRACE			((int)0)
#endif

#ifndef BOINCINFO
#define BOINCINFO			((int)0)
#endif

#ifndef BOINCERROR
#define BOINCERROR			((int)0)
#endif


// ****************************************************************************
// ****************************************************************************
//
// Diagnostics Functions
//
// ****************************************************************************
// ****************************************************************************

int boinc_init_diag();
int boinc_finish_diag();

int boinc_install_signal_handlers();


// ****************************************************************************
// ****************************************************************************
//
// Diagnostics Functions
//
// ****************************************************************************
// ****************************************************************************

#define BOINC_SUCCESS								0
#define BOINC_NESTED_UNHANDLED_EXCEPTION_DETECTED   -1000
#define BOINC_OUT_OF_MEMORY                         boinc_out_of_memory_exception
#define BOINC_INVALID_PARAMETER                     boinc_invalid_parameter_exception
#define BOINC_FILE_OPERATION_FAILED                 boinc_file_operation_exception
#define BOINC_SIGNAL_OPERATION_FAILED               boinc_signal_operation_exception


#endif
