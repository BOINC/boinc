// $Id$
//
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
// Revision History:
//
// $Log$
// Revision 1.4  2004/05/17 22:15:10  rwalton
// *** empty log message ***
//
//

#ifndef __STDWX_H__
#define __STDWX_H__

// For compilers that support precompilation, includes "wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#ifndef WX_PRECOMP
    #include "wx/wx.h"
#endif

#include <wx/debug.h>    // diagnostics support
#include <wx/accel.h>    // accelerator support
#include <wx/cmdline.h>  // command line support
#include <wx/config.h>   // configuration support
#include <wx/intl.h>     // internationalization
#include <wx/listctrl.h> // list control support
#include <wx/msgdlg.h>   // messagebox dialog
#include <wx/notebook.h> // notebook support
#include <wx/regex.h>    // regular expression support
#include <wx/settings.h> // system settings
#include <wx/statline.h> // static line
#include <wx/toolbar.h>  // toolbars support


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

#endif //__WIN32__ && __VISUALC && !__AFX_H__

#endif //__STDWX_H__

