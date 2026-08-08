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

#ifndef BOINC_DLGEXITMESSAGE_H
#define BOINC_DLGEXITMESSAGE_H

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma interface "DlgExitMessage.cpp"
#endif

/*!
 * Includes
 */

////@begin includes
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
#ifdef __WXMAC__
#define SYMBOL_CDLGEXITMESSAGE_STYLE wxCAPTION|wxSYSTEM_MENU|wxCLOSE_BOX
#else
#define SYMBOL_CDLGEXITMESSAGE_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX
#endif
#define SYMBOL_CDLGEXITMESSAGE_TITLE _T("")
#define SYMBOL_CDLGEXITMESSAGE_IDNAME ID_DIALOG
#define SYMBOL_CDLGEXITMESSAGE_SIZE wxSize(400, 300)
#define SYMBOL_CDLGEXITMESSAGE_POSITION wxDefaultPosition
#define ID_CDLGEXITMESSAGE_SHUTDOWNCORECLIENT 10017
#define ID_CDLGEXITMESSAGE_DISPLAY 10018
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
 * CDlgExitMessage class declaration
 */

class CDlgExitMessage: public wxDialog
{
    DECLARE_DYNAMIC_CLASS( CDlgExitMessage )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CDlgExitMessage( );
    CDlgExitMessage( wxWindow* parent, wxWindowID id = SYMBOL_CDLGEXITMESSAGE_IDNAME, const wxString& caption = SYMBOL_CDLGEXITMESSAGE_TITLE, const wxPoint& pos = SYMBOL_CDLGEXITMESSAGE_POSITION, const wxSize& size = SYMBOL_CDLGEXITMESSAGE_SIZE, long style = SYMBOL_CDLGEXITMESSAGE_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CDLGEXITMESSAGE_IDNAME, const wxString& caption = SYMBOL_CDLGEXITMESSAGE_TITLE, const wxPoint& pos = SYMBOL_CDLGEXITMESSAGE_POSITION, const wxSize& size = SYMBOL_CDLGEXITMESSAGE_SIZE, long style = SYMBOL_CDLGEXITMESSAGE_STYLE );

    /// Creates the controls and sizers
    void CreateControls();

////@begin CDlgExitMessage event handler declarations

////@end CDlgExitMessage event handler declarations

////@begin CDlgExitMessage member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CDlgExitMessage member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CDlgExitMessage member variables
    wxStaticText* m_DialogExitMessage;
    wxCheckBox*   m_DialogShutdownCoreClient;
    wxCheckBox*   m_DialogDisplay;
////@end CDlgExitMessage member variables
};

#endif
