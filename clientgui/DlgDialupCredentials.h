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
//
#ifndef _DLGDIALUPCREDENTIALS_H_
#define _DLGDIALUPCREDENTIALS_H_

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma interface "DlgDialupCredentials.cpp"
#endif

/*!
 * Includes
 */

////@begin includes
#include "wx/valgen.h"
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
#define SYMBOL_CDLGDIALUPCREDENTIALS_STYLE wxCAPTION|wxSYSTEM_MENU
#define SYMBOL_CDLGDIALUPCREDENTIALS_TITLE _("Dialup Logon")
#define SYMBOL_CDLGDIALUPCREDENTIALS_IDNAME ID_DIALOG
#define SYMBOL_CDLGDIALUPCREDENTIALS_SIZE wxSize(400, 300)
#define SYMBOL_CDLGDIALUPCREDENTIALS_POSITION wxDefaultPosition
#define ID_USERNAME 10002
#define ID_PASSWORD 10003
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
 * CDlgDialupCredentials class declaration
 */

class CDlgDialupCredentials: public wxDialog
{    
    DECLARE_DYNAMIC_CLASS( CDlgDialupCredentials )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CDlgDialupCredentials( );
    CDlgDialupCredentials( wxWindow* parent, wxWindowID id = SYMBOL_CDLGDIALUPCREDENTIALS_IDNAME, const wxString& caption = SYMBOL_CDLGDIALUPCREDENTIALS_TITLE, const wxPoint& pos = SYMBOL_CDLGDIALUPCREDENTIALS_POSITION, const wxSize& size = SYMBOL_CDLGDIALUPCREDENTIALS_SIZE, long style = SYMBOL_CDLGDIALUPCREDENTIALS_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CDLGDIALUPCREDENTIALS_IDNAME, const wxString& caption = SYMBOL_CDLGDIALUPCREDENTIALS_TITLE, const wxPoint& pos = SYMBOL_CDLGDIALUPCREDENTIALS_POSITION, const wxSize& size = SYMBOL_CDLGDIALUPCREDENTIALS_SIZE, long style = SYMBOL_CDLGDIALUPCREDENTIALS_STYLE );

    /// Creates the controls and sizers
    void CreateControls();

////@begin CDlgDialupCredentials event handler declarations

////@end CDlgDialupCredentials event handler declarations

////@begin CDlgDialupCredentials member function declarations

    wxString GetUsername() const { return m_strUsername ; }
    void SetUsername(wxString value) { m_strUsername = value ; }

    wxString GetPassword() const { return m_strPassword ; }
    void SetPassword(wxString value) { m_strPassword = value ; }

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CDlgDialupCredentials member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CDlgDialupCredentials member variables
#if defined(__WXMSW__)
    wxTextCtrl* m_UsernameCtrl;
#endif
#if defined(__WXMSW__)
    wxTextCtrl* m_PasswordCtrl;
#endif
    wxString m_strUsername;
    wxString m_strPassword;
////@end CDlgDialupCredentials member variables
};

#endif
    // _DLGDIALUPCREDENTIALS_H_
