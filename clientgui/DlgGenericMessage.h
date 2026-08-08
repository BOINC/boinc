// This file is part of BOINC.
// https://boinc.berkeley.edu
// Copyright (C) 2025 University of California
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

#ifndef BOINC_DLGGENERICMESSAGE_H
#define BOINC_DLGGENERICMESSAGE_H

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma interface "DlgGenericMessage.cpp"
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
#ifndef wxCLOSE_BOX
#define wxCLOSE_BOX 0x1000
#endif
#ifdef __WXMAC__
#define SYMBOL_CDLGGENERICMESSAGE_STYLE wxCAPTION|wxSYSTEM_MENU|wxCLOSE_BOX
#else
#define SYMBOL_CDLGGENERICMESSAGE_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX
#endif
////@end control identifiers

/*!
 * CDlgGenericMessage class declaration
 */

struct CDlgGenericMessageButton
{
    CDlgGenericMessageButton(bool _show = true, wxWindowID _id = wxID_OK, wxString _label = _T("&OK"))
    {
        this->show = _show;
        this->id = _id;
        this->label = _label;
    }
    bool show = true;
    wxWindowID id = wxID_OK;
    wxString label = _T("&OK");
};

struct CDlgGenericMessageParameters
{
    wxWindowID id = ID_DIALOG;
    wxString caption = _T("");
    wxPoint pos = wxDefaultPosition;
    wxSize size = wxSize(400, 300);
    long style = SYMBOL_CDLGGENERICMESSAGE_STYLE;
    wxString message = _T("");
    bool showDisableMessage = true;
    CDlgGenericMessageButton button1 = CDlgGenericMessageButton(true, wxID_OK, _T("&OK"));
    CDlgGenericMessageButton button2 = CDlgGenericMessageButton(true, wxID_CANCEL, _T("Cancel"));
};

class CDlgGenericMessage: public wxDialog
{
    DECLARE_DYNAMIC_CLASS( CDlgGenericMessage )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CDlgGenericMessage( );
    CDlgGenericMessage( wxWindow* parent, CDlgGenericMessageParameters* parameters = NULL );

////@begin CDlgGenericMessage event handler declarations

////@end CDlgGenericMessage event handler declarations

////@begin CDlgGenericMessage member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CDlgGenericMessage member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

    bool GetDisableMessageValue();

private:
    /// Creation
    bool Create();
    /// Creates the controls and sizers
    void CreateControls();

    ////@begin CDlgGenericMessage member variables
    wxWindow* m_DialogParent;
    CDlgGenericMessageParameters m_DialogParameters;
    wxCheckBox* m_DialogDisableMessage = NULL;
    ////@end CDlgGenericMessage member variables
};

#endif
