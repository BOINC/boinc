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
#if defined(__GNUG__) && !defined(__APPLE__)
#pragma implementation "DlgDialupCredentials.h"
#endif

#include "stdwx.h"
#include "BOINCGUIApp.h"

////@begin includes
////@end includes

#include "DlgDialupCredentials.h"

////@begin XPM images
////@end XPM images

/*!
 * CDlgDialupCredentials type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CDlgDialupCredentials, wxDialog )

/*!
 * CDlgDialupCredentials event table definition
 */

BEGIN_EVENT_TABLE( CDlgDialupCredentials, wxDialog )

////@begin CDlgDialupCredentials event table entries
////@end CDlgDialupCredentials event table entries

END_EVENT_TABLE()

/*!
 * CDlgDialupCredentials constructors
 */

CDlgDialupCredentials::CDlgDialupCredentials( )
{
}

CDlgDialupCredentials::CDlgDialupCredentials( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
    Create(parent, id, caption, pos, size, style);
}

/*!
 * CDlgDialupCredentials creator
 */

bool CDlgDialupCredentials::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CDlgDialupCredentials member initialisation
#if defined(__WXMSW__)
    m_UsernameCtrl = NULL;
#endif
#if defined(__WXMSW__)
    m_PasswordCtrl = NULL;
#endif
////@end CDlgDialupCredentials member initialisation

////@begin CDlgDialupCredentials creation
    SetExtraStyle(GetExtraStyle()|wxWS_EX_BLOCK_EVENTS);
    wxDialog::Create( parent, id, caption, pos, size, style );

    CreateControls();
    GetSizer()->Fit(this);
    GetSizer()->SetSizeHints(this);
    Centre();
////@end CDlgDialupCredentials creation
    return TRUE;
}

/*!
 * Control creation for CDlgDialupCredentials
 */

void CDlgDialupCredentials::CreateControls()
{    
////@begin CDlgDialupCredentials content construction
    CDlgDialupCredentials* itemDialog1 = this;

    wxFlexGridSizer* itemFlexGridSizer2 = new wxFlexGridSizer(1, 2, 0, 0);
    itemDialog1->SetSizer(itemFlexGridSizer2);

    wxBoxSizer* itemBoxSizer3 = new wxBoxSizer(wxVERTICAL);
    itemFlexGridSizer2->Add(itemBoxSizer3, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxFlexGridSizer* itemFlexGridSizer4 = new wxFlexGridSizer(3, 2, 0, 0);
    itemBoxSizer3->Add(itemFlexGridSizer4, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxStaticText* itemStaticText5 = new wxStaticText;
    itemStaticText5->Create( itemDialog1, wxID_STATIC, _("Username:"), wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT );
    itemFlexGridSizer4->Add(itemStaticText5, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_UsernameCtrl = new wxTextCtrl;
    m_UsernameCtrl->Create( itemDialog1, ID_USERNAME, _T(""), wxDefaultPosition, wxSize(200, -1), 0 );
    itemFlexGridSizer4->Add(m_UsernameCtrl, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText7 = new wxStaticText;
    itemStaticText7->Create( itemDialog1, wxID_STATIC, _("Password:"), wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT );
    itemFlexGridSizer4->Add(itemStaticText7, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_PasswordCtrl = new wxTextCtrl;
    m_PasswordCtrl->Create( itemDialog1, ID_PASSWORD, _T(""), wxDefaultPosition, wxSize(200, -1), wxTE_PASSWORD );
    itemFlexGridSizer4->Add(m_PasswordCtrl, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxFlexGridSizer* itemFlexGridSizer9 = new wxFlexGridSizer(2, 1, 0, 0);
    itemFlexGridSizer2->Add(itemFlexGridSizer9, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_TOP|wxALL, 5);

    wxButton* itemButton10 = new wxButton;
    itemButton10->Create( itemDialog1, wxID_OK, _("&OK"), wxDefaultPosition, wxDefaultSize, 0 );
    itemButton10->SetDefault();
    itemFlexGridSizer9->Add(itemButton10, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxButton* itemButton11 = new wxButton;
    itemButton11->Create( itemDialog1, wxID_CANCEL, _("&Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer9->Add(itemButton11, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    // Set validators
#if defined(__WXMSW__)
    m_UsernameCtrl->SetValidator( wxGenericValidator(& m_strUsername) );
#endif
#if defined(__WXMSW__)
    m_PasswordCtrl->SetValidator( wxGenericValidator(& m_strPassword) );
#endif
////@end CDlgDialupCredentials content construction
}

/*!
 * Should we show tooltips?
 */

bool CDlgDialupCredentials::ShowToolTips()
{
    return TRUE;
}

/*!
 * Get bitmap resources
 */

wxBitmap CDlgDialupCredentials::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CDlgDialupCredentials bitmap retrieval
    return wxNullBitmap;
////@end CDlgDialupCredentials bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CDlgDialupCredentials::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CDlgDialupCredentials icon retrieval
    return wxNullIcon;
////@end CDlgDialupCredentials icon retrieval
}
