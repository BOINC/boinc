/////////////////////////////////////////////////////////////////////////////
// Name:        setup.h configuration
// Purpose:     Configuration of optional items
// Maintainer:  Wyo
// Created:     2003-05-07
// RCS-ID:      $Id$
// Copyright:   (c) wxGuide
// Licence:     wxWindows licence
//////////////////////////////////////////////////////////////////////////////

#ifndef _SETUP_H_
#define _SETUP_H_

//============================================================================
// declarations
//============================================================================


// ----------------------------------------------------------------------------
// Optional items
// ----------------------------------------------------------------------------

// Determines if the wxHtmlHelpController::KeywordIndex is available
//
// Default setting: 0 (current stable wxWindows doesn't provide it)
// Recommended setting: 1 if available
#define HELP_INDEX_KEYWORD  0

// Determines if the wxFileConfig allows empty name using GetAppName
//
// Default setting: 0 (current stable wxWindows doesn't provide it)
// Recommended setting: 1 if supported
#define APPNAME_FILECONFIG  0

// Determines if the wxNotebook::HitTest is available
//
// Default setting: 0 (current stable wxWindows doesn't provide it)
// Recommended setting: 1 if available
#define NOTEBOOK_HITTEST  0

#endif // _SETUP_H_

