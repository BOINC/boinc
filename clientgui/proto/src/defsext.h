/////////////////////////////////////////////////////////////////////////////
// Name:        defsext.h extensions
// Purpose:     Common declarations
// Maintainer:  Wyo
// Created:     2003-01-20
// RCS-ID:      $Id$
// Copyright:   (c) wxGuide
// Licence:     wxWindows licence
//////////////////////////////////////////////////////////////////////////////

#ifndef _WX_DEFSEXT_H_
#define _WX_DEFSEXT_H_

//----------------------------------------------------------------------------
// headers
//----------------------------------------------------------------------------

//! wxWindows headers
#include <wx/html/helpctrl.h> // html help support
#include <wx/print.h>    // printing support
#include <wx/printdlg.h> // printing dialog

#include "setup.h"       // Configuration


//============================================================================
// declarations
//============================================================================

const wxString PAGE_COMMON = _T("Common");


// ----------------------------------------------------------------------------
// standard IDs
// ----------------------------------------------------------------------------

enum {
    // menu IDs
    myID_PROPERTIES = wxID_HIGHEST,
    myID_FINDNEXT,
    myID_NEWFRAME,
    myID_OPENFRAME,
    myID_REPLACE,
    myID_REPLACENEXT,
    myID_REPLACEALL,
    myID_GOTO,
    myID_PREFS,
    myID_TOOLBARS,
    myID_PAGEACTIVE,
    myID_PAGEPREV,
    myID_PAGENEXT,
    myID_WINDOW1,
    myID_WINDOW2,
    myID_WINDOW3,
    myID_WINDOW4,
    myID_WINDOW5,
    myID_WINDOW6,
    myID_WINDOW7,
    myID_WINDOW8,
    myID_WINDOW9,
    myID_WINDOWS,
    myID_FRAMELAYOUT,

    // other IDs
    myID_STATUSBAR,
    myID_PAGESELECT,
    myID_ABOUTTIMER,
    myID_UPDATETIMER,

    // dialog find IDs
    myID_DLG_FIND_TEXT,

    // preferences IDs
    myID_PREFS_LANGUAGE,
    myID_PREFS_STYLETYPE,
    myID_PREFS_KEYWORDS,
};

// ----------------------------------------------------------------------------
// global items
// ----------------------------------------------------------------------------

//! global application name
extern wxString g_appname;

//! global help provider
extern wxHtmlHelpController *g_help;

//! global status text
extern wxString g_statustext;

#endif // _WX_DEFSEXT_H_

