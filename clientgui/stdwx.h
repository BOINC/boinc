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

#ifndef __STDWX_H__
#define __STDWX_H__


#ifdef _WIN32

// Visual Studio 2005 has extended the C Run-Time Library by including "secure"
// runtime functions and deprecating the previous function prototypes.  Since
// we need to use the previous prototypes to maintain compatibility with other
// platforms we are going to disable the deprecation warnings if we are compiling
// on Visual Studio 2005
#if _MSC_VER >= 1400
#pragma warning(disable: 4996) // function deprecation
#endif

// Modify the following defines if you have to target a platform prior to the ones specified below.
// Refer to MSDN for the latest info on corresponding values for different platforms.
#ifndef WINVER                  // Allow use of features specific to Windows 95 and Windows NT 4 or later.
#define WINVER 0x0400           // Change this to the appropriate value to target Windows 98 and Windows 2000 or later.
#endif

#ifndef _WIN32_WINNT            // Allow use of features specific to Windows NT 4 or later.
#define _WIN32_WINNT 0x0400		// Change this to the appropriate value to target Windows 98 and Windows 2000 or later.
#endif

#ifndef _WIN32_WINDOWS          // Allow use of features specific to Windows 98 or later.
#define _WIN32_WINDOWS 0x0400   // Change this to the appropriate value to target Windows Me or later.
#endif

#ifndef _WIN32_IE               // Allow use of features specific to IE 5.01 or later.
#define _WIN32_IE 0x0501        // Change this to the appropriate value to target IE 6.0 or later.
#endif

#endif

#include <wx/config.h>          // configuration support

// For compilers that support precompilation, includes "wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#ifndef WX_PRECOMP
    #include "wx/wx.h"
#endif

#include <wx/debug.h>           // diagnostics support
#include <wx/log.h>             // logging support
#include <wx/accel.h>           // accelerator support
#include <wx/regex.h>           // regular expression support
#include <wx/cmdline.h>         // command line support
#include <wx/settings.h>        // system settings support
#include <wx/intl.h>            // internationalization support
#include <wx/timer.h>           // timer support
#include <wx/image.h>
#include <wx/url.h>
#include <wx/filename.h>        // filesystem support
#include <wx/bitmap.h>          // bitmap modification support
#include <wx/toolbar.h>         // toolbars support
#include <wx/listctrl.h>        // list control support
#include <wx/msgdlg.h>          // messagebox dialog support
#include <wx/panel.h>           // panel support
#include <wx/notebook.h>        // notebook support
#include <wx/statline.h>        // static line support
#include <wx/statbmp.h>         // static bitmap support
#include <wx/stattext.h>        // static text support
#include <wx/clipbrd.h>         // clipboard support
#include <wx/datetime.h>        // date/time support
#include <wx/textdlg.h>
#include <wx/mimetype.h>
#include <wx/event.h>
#include <wx/list.h>
#include <wx/icon.h>
#include <wx/utils.h>
#include <wx/process.h>
#include <wx/dynlib.h>
#include <wx/dialup.h>

#ifndef NOTASKBAR
#include <wx/taskbar.h>         // taskbar support
#endif


#ifdef __WXMSW__

// Windows Headers
//
#include <crtdbg.h>

#endif

// Standard Libraries
//

// C headers
#include <sys/stat.h>
#include <sys/types.h>
#include <cassert>
#include <cctype>
#include <cerrno>
#include <cerrno>
#include <cmath>
#include <csetjmp>
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <fcntl.h>


// C++ headers
#ifdef __WXMSW__
#include <xdebug>
#endif
#include <algorithm>
#include <stdexcept>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <list>
#include <locale>


#ifdef __WXMSW__
#define vsnprintf               _vsnprintf
#define snprintf                _snprintf
#define fdopen                  _fdopen
#define dup                     _dup
#define unlink                  _unlink
#define strdup                  _strdup
#define read                    _read
#define stat                    _stat
#endif


// On the Win32 platform include file and line number information for each
//   memory allocation/deallocation
#if (defined(__WIN32__) && defined(__VISUALC__) && !defined(__AFX_H__))

#ifdef _DEBUG

#define malloc(s)                             _malloc_dbg(s, _NORMAL_BLOCK, __FILE__, __LINE__)
#define calloc(c, s)                          _calloc_dbg(c, s, _NORMAL_BLOCK, __FILE__, __LINE__)
#define realloc(p, s)                         _realloc_dbg(p, s, _NORMAL_BLOCK, __FILE__, __LINE__)
#define _expand(p, s)                         _expand_dbg(p, s, _NORMAL_BLOCK, __FILE__, __LINE__)
#define free(p)                               _free_dbg(p, _NORMAL_BLOCK)
#define _msize(p)                             _msize_dbg(p, _NORMAL_BLOCK)
#define _aligned_malloc(s, a)                 _aligned_malloc_dbg(s, a, __FILE__, __LINE__)
#define _aligned_realloc(p, s, a)             _aligned_realloc_dbg(p, s, a, __FILE__, __LINE__)
#define _aligned_offset_malloc(s, a, o)       _aligned_offset_malloc_dbg(s, a, o, __FILE__, __LINE__)
#define _aligned_offset_realloc(p, s, a, o)   _aligned_offset_realloc_dbg(p, s, a, o, __FILE__, __LINE__)
#define _aligned_free(p)                      _aligned_free_dbg(p)

#define DEBUG_NEW                             new(_NORMAL_BLOCK, __FILE__, __LINE__)

// The following macros set and clear, respectively, given bits 
// of the C runtime library debug flag, as specified by a bitmask. 
#define SET_CRT_DEBUG_FIELD(a)                _CrtSetDbgFlag((a) | _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG)) 
#define CLEAR_CRT_DEBUG_FIELD(a)              _CrtSetDbgFlag(~(a) & _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG)) 

#else //_DEBUG

#define DEBUG_NEW                             new

#define SET_CRT_DEBUG_FIELD(a)                ((void) 0) 
#define CLEAR_CRT_DEBUG_FIELD(a)              ((void) 0) 

#endif //_DEBUG

#define new DEBUG_NEW

#endif //__WIN32__ && __VISUALC && !__AFX_H__

#endif //__STDWX_H__

