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

#ifndef _BOINC_API_
#define _BOINC_API_

#include <string>
#include <assert.h>

#ifdef _WIN32
#include <crtdbg.h>
#endif

#include "app_ipc.h"

using namespace std;

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
void	boinc_error_debug(int iExitCode, const char *pszFormat, ...);

#define BOINCASSERT(expr)	_ASSERT_BASE((expr), #expr)
#define BOINCTRACE			boinc_trace
#define BOINCERROR			boinc_error_debug

#else // _DEBUG

// Forward declare so we can assign a macro to it.
void	boinc_error_release(int iExitCode, const char *pszFormat, ...);

#define BOINCASSERT(expr)	((void)0)
#define BOINCTRACE			((void)0)
#define BOINCERROR			boinc_error_release

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

#ifndef BOINCERROR
#define BOINCERROR			((int)0)
#endif


// MFILE supports a primitive form of checkpointing.
// Write all your output (and restart file) to MFILEs.
// The output is buffered in memory.
// Then close or flush all the MFILEs;
// all the buffers will be flushed to disk, almost atomically.

class MFILE {
    char* buf;
    int len;
    FILE* f;
public:
    int open(const char* path, const char* mode);
    int _putchar(char);
    int puts(const char*);
    int printf(const char* format, ...);
    size_t write(const void *, size_t size, size_t nitems);
    int close();
    int flush();
    long tell() const;
};


/////////// API BEGINS HERE 

extern int	boinc_set_error(int exit_code);

extern int	boinc_init(bool standalone = false);
extern int	boinc_finish(int);

extern bool	boinc_is_standalone();

extern int	boinc_resolve_filename(const char*, char*, int len);
extern int	boinc_resolve_filename(const char*, string&);

extern int	boinc_parse_init_data_file();
extern int	boinc_get_init_data(APP_INIT_DATA&);

extern int	boinc_trickle(char*);

extern bool	boinc_time_to_checkpoint();
extern int	boinc_checkpoint_completed();

extern int	boinc_fraction_done(double);
extern int	boinc_child_start();
extern int	boinc_child_done(double);

extern int	boinc_wu_cpu_time(double&);
extern int	boinc_thread_cpu_time(double&, double&);

/////////// API ENDS HERE

/////////// IMPLEMENTATION STUFF BEGINS HERE

extern APP_CLIENT_SHM *app_client_shm;

/////////// IMPLEMENTATION STUFF ENDS HERE

#endif
