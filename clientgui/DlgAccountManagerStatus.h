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
#ifndef _DLGACCOUNTMANAGERSTATUS_H_
#define _DLGACCOUNTMANAGERSTATUS_H_

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma interface "DlgAccountManagerStatus.cpp"
#endif

/*!
 * Includes
 */

////@begin includes
#include "wx/valgen.h"
#include "hyperlink.h"
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
#define SYMBOL_CDLGACCOUNTMANAGERSTATUS_STYLE wxDEFAULT_DIALOG_STYLE
#define SYMBOL_CDLGACCOUNTMANAGERSTATUS_TITLE _("Account Manager")
#define SYMBOL_CDLGACCOUNTMANAGERSTATUS_IDNAME ID_DIALOG
#define SYMBOL_CDLGACCOUNTMANAGERSTATUS_SIZE wxSize(400, 300)
#define SYMBOL_CDLGACCOUNTMANAGERSTATUS_POSITION wxDefaultPosition
#define ID_ACCTMANAGERNAME 10001
#define ID_ACCTMANAGERLINK 10002
#define ID_UPDATE 10005
#define ID_CHANGE 10017
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
 * CDlgAccountManagerStatus class declaration
 */

class CDlgAccountManagerStatus: public wxDialog
{    
    DECLARE_DYNAMIC_CLASS( CDlgAccountManagerStatus )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CDlgAccountManagerStatus( );
    CDlgAccountManagerStatus( wxWindow* parent, wxWindowID id = SYMBOL_CDLGACCOUNTMANAGERSTATUS_IDNAME, const wxString& caption = SYMBOL_CDLGACCOUNTMANAGERSTATUS_TITLE, const wxPoint& pos = SYMBOL_CDLGACCOUNTMANAGERSTATUS_POSITION, const wxSize& size = SYMBOL_CDLGACCOUNTMANAGERSTATUS_SIZE, long style = SYMBOL_CDLGACCOUNTMANAGERSTATUS_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CDLGACCOUNTMANAGERSTATUS_IDNAME, const wxString& caption = SYMBOL_CDLGACCOUNTMANAGERSTATUS_TITLE, const wxPoint& pos = SYMBOL_CDLGACCOUNTMANAGERSTATUS_POSITION, const wxSize& size = SYMBOL_CDLGACCOUNTMANAGERSTATUS_SIZE, long style = SYMBOL_CDLGACCOUNTMANAGERSTATUS_STYLE );

    /// Creates the controls and sizers
    void CreateControls();

////@begin CDlgAccountManagerStatus event handler declarations

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_UPDATE
    void OnUpdateClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_CHANGE
    void OnChangeClick( wxCommandEvent& event );

////@end CDlgAccountManagerStatus event handler declarations

////@begin CDlgAccountManagerStatus member function declarations

    wxString GetAcctManagerName() const { return m_strAcctManagerName ; }
    void SetAcctManagerName(wxString value) { m_strAcctManagerName = value ; }

    wxString GetAcctManagerURL() const { return m_strAcctManagerURL ; }
    void SetAcctManagerURL(wxString value) { m_strAcctManagerURL = value ; }

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CDlgAccountManagerStatus member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CDlgAccountManagerStatus member variables
    wxStaticText* m_AcctManagerNameCtrl;
    wxString m_strAcctManagerName;
    wxString m_strAcctManagerURL;
////@end CDlgAccountManagerStatus member variables
};

#endif
    // _DLGACCOUNTMANAGERSTATUS_H_
