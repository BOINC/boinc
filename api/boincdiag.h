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

///////////////////////////////////////////////////////////////////////////////
//
// This include file should define the basic debugging infrastructure for
//   various boinc applications.
//
// BOINCASSERT should be used to evaluate expressions and spew results to
//   sdterr
//
// BOINCTRACE should spew debug information to stderr
//
///////////////////////////////////////////////////////////////////////////////


//
// Diagnostic Function Prototypes
//
int		boinc_diag_init();
int		boinc_diag_cleanup();


// Diagnostic functions only available in debug builds
#ifdef _DEBUG

void	boinc_trace(const char *pszFormat, ...);

#endif


///////////////////////////////////////////////////////////////////////////////
//
// Windows 32-bit Platforms
//
///////////////////////////////////////////////////////////////////////////////
#ifdef _WIN32

// Check to see if the MFC/ATL frameworks are included, if so we have to use a
//   different set of diagnostics functions.
#ifdef __AFXWIN_H__


//
// TODO: Determine what needs to happen between the two frameworks and implement it
//


// Well if we are not using the MFC/ATL frameworks, then we are using the CRT.
#else __AFXWIN_H__


//
// Define macros for both debug and release builds.
//
// We are using the native debugging technology built into the Microsoft
//   C Runtime Libraries to trap and report the asserts and traces.
//
#include <crtdbg.h>


#define BOINCASSERT(expr)	_ASSERT_BASE((expr), #expr)

#ifdef _DEBUG

#define BOINCTRACE			boinc_trace

#else _DEBUG

#define BOINCTRACE			((void)0)

#endif _DEBUG

#endif __AFXWIN_H__

#endif _WIN32




///////////////////////////////////////////////////////////////////////////////
//
// Undefined Platform/Diagnostic Functions.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef BOINCASSERT

#include <assert.h>
#define BOINCASSERT		assert

#endif

#ifndef BOINCTRACE

#define BOINCTRACE		((int)0)

#endif


#endif _BOINC_DIAGNOSTICS_