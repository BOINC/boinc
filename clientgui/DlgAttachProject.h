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
#ifndef _DLGATTACHPROJECT_H_
#define _DLGATTACHPROJECT_H_

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma interface "DlgAttachProject.cpp"
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
#define SYMBOL_CDLGATTACHPROJECT_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU
#define SYMBOL_CDLGATTACHPROJECT_TITLE _("Attach to Project")
#define SYMBOL_CDLGATTACHPROJECT_IDNAME ID_DIALOG
#define SYMBOL_CDLGATTACHPROJECT_SIZE wxSize(400, 300)
#define SYMBOL_CDLGATTACHPROJECT_POSITION wxDefaultPosition
#define ID_PROJECTADDRESS 10001
#define ID_PROJECTACCOUNTKEY 10002
#define ID_FOREIGN 10005
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
 * CDlgAttachProject class declaration
 */

class CDlgAttachProject: public wxDialog
{    
    DECLARE_DYNAMIC_CLASS( CDlgAttachProject )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CDlgAttachProject( );
    CDlgAttachProject( wxWindow* parent, wxWindowID id = SYMBOL_CDLGATTACHPROJECT_IDNAME, const wxString& caption = SYMBOL_CDLGATTACHPROJECT_TITLE, const wxPoint& pos = SYMBOL_CDLGATTACHPROJECT_POSITION, const wxSize& size = SYMBOL_CDLGATTACHPROJECT_SIZE, long style = SYMBOL_CDLGATTACHPROJECT_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CDLGATTACHPROJECT_IDNAME, const wxString& caption = SYMBOL_CDLGATTACHPROJECT_TITLE, const wxPoint& pos = SYMBOL_CDLGATTACHPROJECT_POSITION, const wxSize& size = SYMBOL_CDLGATTACHPROJECT_SIZE, long style = SYMBOL_CDLGATTACHPROJECT_STYLE );

    /// Creates the controls and sizers
    void CreateControls();

////@begin CDlgAttachProject event handler declarations

////@end CDlgAttachProject event handler declarations

////@begin CDlgAttachProject member function declarations

    wxString GetProjectAddress() const { return m_strProjectAddress ; }
    void SetProjectAddress(wxString value) { m_strProjectAddress = value ; }

    wxString GetProjectAccountKey() const { return m_strProjectAccountKey ; }
    void SetProjectAccountKey(wxString value) { m_strProjectAccountKey = value ; }

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CDlgAttachProject member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CDlgAttachProject member variables
    wxTextCtrl* m_ProjectAddressCtrl;
    wxTextCtrl* m_ProjectAccountKeyCtrl;
    wxString m_strProjectAddress;
    wxString m_strProjectAccountKey;
////@end CDlgAttachProject member variables
};

#endif
    // _DLGATTACHPROJECT_H_
