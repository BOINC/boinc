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
#ifndef _DLGACCOUNTMANAGER_H_
#define _DLGACCOUNTMANAGER_H_

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma interface "DlgAccountManager.cpp"
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
#define SYMBOL_CDLGACCOUNTMANAGER_STYLE wxCAPTION|wxSYSTEM_MENU
#define SYMBOL_CDLGACCOUNTMANAGER_TITLE _("Account Manager Credentials")
#define SYMBOL_CDLGACCOUNTMANAGER_IDNAME ID_DIALOG
#define SYMBOL_CDLGACCOUNTMANAGER_SIZE wxSize(400, 300)
#define SYMBOL_CDLGACCOUNTMANAGER_POSITION wxDefaultPosition
#define ID_ACCTMANAGERUSERNAME 10001
#define ID_ACCTMANAGERPASSWORD 10002
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
 * CDlgAccountManager class declaration
 */

class CDlgAccountManager: public wxDialog
{    
    DECLARE_DYNAMIC_CLASS( CDlgAccountManager )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CDlgAccountManager( );
    CDlgAccountManager( wxWindow* parent, wxWindowID id = SYMBOL_CDLGACCOUNTMANAGER_IDNAME, const wxString& caption = SYMBOL_CDLGACCOUNTMANAGER_TITLE, const wxPoint& pos = SYMBOL_CDLGACCOUNTMANAGER_POSITION, const wxSize& size = SYMBOL_CDLGACCOUNTMANAGER_SIZE, long style = SYMBOL_CDLGACCOUNTMANAGER_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CDLGACCOUNTMANAGER_IDNAME, const wxString& caption = SYMBOL_CDLGACCOUNTMANAGER_TITLE, const wxPoint& pos = SYMBOL_CDLGACCOUNTMANAGER_POSITION, const wxSize& size = SYMBOL_CDLGACCOUNTMANAGER_SIZE, long style = SYMBOL_CDLGACCOUNTMANAGER_STYLE );

    /// Creates the controls and sizers
    void CreateControls();

////@begin CDlgAccountManager event handler declarations
////@end CDlgAccountManager event handler declarations

////@begin CDlgAccountManager member function declarations
    wxString GetAcctManagerUsername() const { return m_strAcctManagerUsername ; }
    void SetAcctManagerUsername(wxString value) { m_strAcctManagerUsername = value ; }

    wxString GetAcctManagerPassword() const { return m_strAcctManagerPassword ; }
    void SetAcctManagerPassword(wxString value) { m_strAcctManagerPassword = value ; }

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CDlgAccountManager member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CDlgAccountManager member variables
    wxTextCtrl* m_AcctManagerUsernameCtrl;
    wxTextCtrl* m_AcctManagerPasswordCtrl;
    wxString m_strAcctManagerUsername;
    wxString m_strAcctManagerPassword;
////@end CDlgAccountManager member variables
};

#endif
    // _DLGACCOUNTMANAGER_H_
