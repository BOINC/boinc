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
//
#ifndef _DLGABOUT_H_
#define _DLGABOUT_H_

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma interface "DlgAbout.cpp"
#endif

/*!
 * Includes
 */

////@begin includes
#include "wx/valgen.h"
#include "wx/statline.h"
////@end includes

/*!
 * Forward declarations
 */

////@begin forward declarations
////@end forward declarations

/*!
 * Control identifiers
 */

////@begin control identifiers
#define ID_DIALOG 10000
#define SYMBOL_CDLGABOUT_STYLE wxDEFAULT_DIALOG_STYLE
#define SYMBOL_CDLGABOUT_TITLE wxT("")
#define SYMBOL_CDLGABOUT_IDNAME ID_DIALOG
#define SYMBOL_CDLGABOUT_SIZE wxSize(-1, -1)
#define SYMBOL_CDLGABOUT_POSITION wxDefaultPosition
#define ID_ABOUTBOINCLINK 10031
////@end control identifiers

/*!
 * Compatibility
 */

#ifndef wxCLOSE_BOX
#define wxCLOSE_BOX 0x1000
#endif
#ifndef wxFIXED_MINSIZE
#define wxFIXED_MINSIZE 0
#endif

/*!
 * CDlgAbout class declaration
 */

class CDlgAbout: public wxDialog
{    
    DECLARE_DYNAMIC_CLASS( CDlgAbout )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CDlgAbout( );
    CDlgAbout( wxWindow* parent, wxWindowID id = SYMBOL_CDLGABOUT_IDNAME, const wxString& caption = SYMBOL_CDLGABOUT_TITLE, const wxPoint& pos = SYMBOL_CDLGABOUT_POSITION, const wxSize& size = SYMBOL_CDLGABOUT_SIZE, long style = SYMBOL_CDLGABOUT_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CDLGABOUT_IDNAME, const wxString& caption = SYMBOL_CDLGABOUT_TITLE, const wxPoint& pos = SYMBOL_CDLGABOUT_POSITION, const wxSize& size = SYMBOL_CDLGABOUT_SIZE, long style = SYMBOL_CDLGABOUT_STYLE );

    /// Creates the controls and sizers
    void CreateControls();

////@begin CDlgAbout event handler declarations

////@end CDlgAbout event handler declarations

////@begin CDlgAbout member function declarations

    wxString GetVersion() const { return m_strVersion ; }
    void SetVersion(wxString value) { m_strVersion = value ; }

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CDlgAbout member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CDlgAbout member variables
    wxStaticText* m_AboutBOINCTitleCtrl;
    wxStaticBitmap* m_AboutBOINCLogoCtrl;
    wxStaticText* m_AboutBOINCSloganCtrl;
    wxHyperlinkCtrl* m_AboutBOINCURLCtrl;
    wxString m_strVersion;
    wxString m_strWidgetsVersion;
////@end CDlgAbout member variables
};

#endif
    // _DLGABOUT_H_
