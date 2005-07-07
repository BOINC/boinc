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
#pragma implementation "DlgAccountManagerSignup.h"
#endif

#include "stdwx.h"
#include "BOINCGUIApp.h"

////@begin includes
////@end includes

#include "DlgAccountManagerSignup.h"

////@begin XPM images
////@end XPM images

/*!
 * CDlgAccountManagerSignup type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CDlgAccountManagerSignup, wxDialog )

/*!
 * CDlgAccountManagerSignup event table definition
 */

BEGIN_EVENT_TABLE( CDlgAccountManagerSignup, wxDialog )

////@begin CDlgAccountManagerSignup event table entries
////@end CDlgAccountManagerSignup event table entries

END_EVENT_TABLE()

/*!
 * CDlgAccountManagerSignup constructors
 */

CDlgAccountManagerSignup::CDlgAccountManagerSignup( )
{
}

CDlgAccountManagerSignup::CDlgAccountManagerSignup( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
    Create(parent, id, caption, pos, size, style);
}

/*!
 * CDlgAccountManagerSignup creator
 */

bool CDlgAccountManagerSignup::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CDlgAccountManagerSignup member initialisation
    m_AcctManagerURLCtrl = NULL;
    m_AcctManagerUsernameCtrl = NULL;
    m_AcctManagerPasswordCtrl = NULL;
////@end CDlgAccountManagerSignup member initialisation

////@begin CDlgAccountManagerSignup creation
    SetExtraStyle(GetExtraStyle()|wxWS_EX_BLOCK_EVENTS);
    wxDialog::Create( parent, id, caption, pos, size, style );

    CreateControls();
    GetSizer()->Fit(this);
    GetSizer()->SetSizeHints(this);
    Centre();
////@end CDlgAccountManagerSignup creation
    return TRUE;
}

/*!
 * Control creation for CDlgAccountManagerSignup
 */

void CDlgAccountManagerSignup::CreateControls()
{    
////@begin CDlgAccountManagerSignup content construction
    CDlgAccountManagerSignup* itemDialog1 = this;

    wxFlexGridSizer* itemFlexGridSizer2 = new wxFlexGridSizer(1, 2, 0, 0);
    itemDialog1->SetSizer(itemFlexGridSizer2);

    wxBoxSizer* itemBoxSizer3 = new wxBoxSizer(wxVERTICAL);
    itemFlexGridSizer2->Add(itemBoxSizer3, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText4 = new wxStaticText;
    itemStaticText4->Create( itemDialog1, ID_ACCTMANAGERTEXT, _("Account managers make it easy to find and join BOINC projects.\nFor more information, click on the link below."), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer3->Add(itemStaticText4, 0, wxALIGN_LEFT|wxALL, 5);

    wxHyperLink* itemHyperLink5 = new wxHyperLink;
    itemHyperLink5->Create( itemDialog1, ID_ACCTMANAGERDOCLINK, wxT("http://boinc.berkeley.edu/acct_mgrs.php"), wxDefaultPosition, wxDefaultSize, wxNO_BORDER );
    itemBoxSizer3->Add(itemHyperLink5, 0, wxALIGN_LEFT|wxALL, 5);

    wxFlexGridSizer* itemFlexGridSizer6 = new wxFlexGridSizer(3, 2, 0, 0);
    itemBoxSizer3->Add(itemFlexGridSizer6, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxStaticText* itemStaticText7 = new wxStaticText;
    itemStaticText7->Create( itemDialog1, wxID_STATIC, _("Account Manager URL:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer6->Add(itemStaticText7, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_AcctManagerURLCtrl = new wxTextCtrl;
    m_AcctManagerURLCtrl->Create( itemDialog1, ID_ACCTMANAGERURL, _T(""), wxDefaultPosition, wxSize(200, -1), 0 );
    itemFlexGridSizer6->Add(m_AcctManagerURLCtrl, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText9 = new wxStaticText;
    itemStaticText9->Create( itemDialog1, wxID_STATIC, _("Login:"), wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT );
    itemFlexGridSizer6->Add(itemStaticText9, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_AcctManagerUsernameCtrl = new wxTextCtrl;
    m_AcctManagerUsernameCtrl->Create( itemDialog1, ID_ACCTMANAGERUSERNAME, _T(""), wxDefaultPosition, wxSize(200, -1), 0 );
    itemFlexGridSizer6->Add(m_AcctManagerUsernameCtrl, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText11 = new wxStaticText;
    itemStaticText11->Create( itemDialog1, wxID_STATIC, _("Password:"), wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT );
    itemFlexGridSizer6->Add(itemStaticText11, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_AcctManagerPasswordCtrl = new wxTextCtrl;
    m_AcctManagerPasswordCtrl->Create( itemDialog1, ID_ACCTMANAGERPASSWORD, _T(""), wxDefaultPosition, wxSize(200, -1), wxTE_PASSWORD );
    itemFlexGridSizer6->Add(m_AcctManagerPasswordCtrl, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxFlexGridSizer* itemFlexGridSizer13 = new wxFlexGridSizer(2, 1, 0, 0);
    itemFlexGridSizer2->Add(itemFlexGridSizer13, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_TOP|wxALL, 5);

    wxButton* itemButton14 = new wxButton;
    itemButton14->Create( itemDialog1, wxID_OK, _("&OK"), wxDefaultPosition, wxDefaultSize, 0 );
    itemButton14->SetDefault();
    itemFlexGridSizer13->Add(itemButton14, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxButton* itemButton15 = new wxButton;
    itemButton15->Create( itemDialog1, wxID_CANCEL, _("&Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer13->Add(itemButton15, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    // Set validators
    m_AcctManagerURLCtrl->SetValidator( wxGenericValidator(& m_strAcctManagerURL) );
    m_AcctManagerUsernameCtrl->SetValidator( wxGenericValidator(& m_strAcctManagerUsername) );
    m_AcctManagerPasswordCtrl->SetValidator( wxGenericValidator(& m_strAcctManagerPassword) );
////@end CDlgAccountManagerSignup content construction
}

/*!
 * Should we show tooltips?
 */

bool CDlgAccountManagerSignup::ShowToolTips()
{
    return TRUE;
}

/*!
 * Get bitmap resources
 */

wxBitmap CDlgAccountManagerSignup::GetBitmapResource( const wxString& WXUNUSED(name) )
{
    // Bitmap retrieval
////@begin CDlgAccountManagerSignup bitmap retrieval
    return wxNullBitmap;
////@end CDlgAccountManagerSignup bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CDlgAccountManagerSignup::GetIconResource( const wxString& WXUNUSED(name) )
{
    // Icon retrieval
////@begin CDlgAccountManagerSignup icon retrieval
    return wxNullIcon;
////@end CDlgAccountManagerSignup icon retrieval
}
