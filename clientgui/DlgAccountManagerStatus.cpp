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
#pragma implementation "DlgAccountManagerStatus.h"
#endif

#include "stdwx.h"
#include "BOINCGUIApp.h"

////@begin includes
////@end includes

#include "DlgAccountManagerStatus.h"

////@begin XPM images
////@end XPM images

/*!
 * CDlgAccountManagerStatus type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CDlgAccountManagerStatus, wxDialog )

/*!
 * CDlgAccountManagerStatus event table definition
 */

BEGIN_EVENT_TABLE( CDlgAccountManagerStatus, wxDialog )

////@begin CDlgAccountManagerStatus event table entries
    EVT_BUTTON( ID_UPDATE, CDlgAccountManagerStatus::OnUpdateClick )

    EVT_BUTTON( ID_CHANGE, CDlgAccountManagerStatus::OnChangeClick )

////@end CDlgAccountManagerStatus event table entries

END_EVENT_TABLE()

/*!
 * CDlgAccountManagerStatus constructors
 */

CDlgAccountManagerStatus::CDlgAccountManagerStatus( )
{
}

CDlgAccountManagerStatus::CDlgAccountManagerStatus( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
    Create(parent, id, caption, pos, size, style);
}

/*!
 * CDlgAccountManagerStatus creator
 */

bool CDlgAccountManagerStatus::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CDlgAccountManagerStatus member initialisation
    m_AcctManagerNameCtrl = NULL;
////@end CDlgAccountManagerStatus member initialisation

////@begin CDlgAccountManagerStatus creation
    SetExtraStyle(GetExtraStyle()|wxWS_EX_BLOCK_EVENTS);
    wxDialog::Create( parent, id, caption, pos, size, style );

    CreateControls();
    GetSizer()->Fit(this);
    GetSizer()->SetSizeHints(this);
    Centre();
////@end CDlgAccountManagerStatus creation
    return TRUE;
}

/*!
 * Control creation for CDlgAccountManagerStatus
 */

void CDlgAccountManagerStatus::CreateControls()
{    
////@begin CDlgAccountManagerStatus content construction
    CDlgAccountManagerStatus* itemDialog1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemDialog1->SetSizer(itemBoxSizer2);

    wxBoxSizer* itemBoxSizer3 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer3, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxBoxSizer* itemBoxSizer4 = new wxBoxSizer(wxVERTICAL);
    itemBoxSizer3->Add(itemBoxSizer4, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText5 = new wxStaticText;
    itemStaticText5->Create( itemDialog1, wxID_STATIC, _("Your current account manager is:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer4->Add(itemStaticText5, 0, wxALIGN_LEFT|wxALL, 5);

    wxFlexGridSizer* itemFlexGridSizer6 = new wxFlexGridSizer(2, 2, 0, 0);
    itemBoxSizer4->Add(itemFlexGridSizer6, 0, wxALIGN_LEFT|wxALL, 5);

    wxStaticText* itemStaticText7 = new wxStaticText;
    itemStaticText7->Create( itemDialog1, wxID_STATIC, _("Name:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer6->Add(itemStaticText7, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_AcctManagerNameCtrl = new wxStaticText;
    m_AcctManagerNameCtrl->Create( itemDialog1, ID_ACCTMANAGERNAME, _("foo"), wxDefaultPosition, wxSize(200, -1), 0 );
    itemFlexGridSizer6->Add(m_AcctManagerNameCtrl, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText9 = new wxStaticText;
    itemStaticText9->Create( itemDialog1, wxID_STATIC, _("URL:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer6->Add(itemStaticText9, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxHyperLink* itemHyperLink10 = new wxHyperLink;
    itemHyperLink10->Create( itemDialog1, ID_ACCTMANAGERLINK, wxT("http://a/b/c"), wxDefaultPosition, wxSize(200, -1), wxNO_BORDER );
    itemFlexGridSizer6->Add(itemHyperLink10, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxBoxSizer* itemBoxSizer11 = new wxBoxSizer(wxVERTICAL);
    itemBoxSizer3->Add(itemBoxSizer11, 0, wxALIGN_TOP|wxALL, 5);

    wxButton* itemButton12 = new wxButton;
    itemButton12->Create( itemDialog1, ID_UPDATE, _("&Update"), wxDefaultPosition, wxDefaultSize, 0 );
    itemButton12->SetDefault();
    itemBoxSizer11->Add(itemButton12, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxButton* itemButton13 = new wxButton;
    itemButton13->Create( itemDialog1, ID_CHANGE, _("Change"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer11->Add(itemButton13, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxButton* itemButton14 = new wxButton;
    itemButton14->Create( itemDialog1, wxID_CANCEL, _("&Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer11->Add(itemButton14, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    // Set validators
    m_AcctManagerNameCtrl->SetValidator( wxGenericValidator(& m_strAcctManagerName) );
    itemHyperLink10->SetValidator( wxGenericValidator(& m_strAcctManagerURL) );
////@end CDlgAccountManagerStatus content construction
}

/*!
 * Should we show tooltips?
 */

bool CDlgAccountManagerStatus::ShowToolTips()
{
    return TRUE;
}

/*!
 * Get bitmap resources
 */

wxBitmap CDlgAccountManagerStatus::GetBitmapResource( const wxString& WXUNUSED(name) )
{
    // Bitmap retrieval
////@begin CDlgAccountManagerStatus bitmap retrieval
    return wxNullBitmap;
////@end CDlgAccountManagerStatus bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CDlgAccountManagerStatus::GetIconResource( const wxString& WXUNUSED(name) )
{
    // Icon retrieval
////@begin CDlgAccountManagerStatus icon retrieval
    return wxNullIcon;
////@end CDlgAccountManagerStatus icon retrieval
}

/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_UPDATE
 */

void CDlgAccountManagerStatus::OnUpdateClick( wxCommandEvent& WXUNUSED(event) )
{
    if ( Validate() && TransferDataFromWindow() )
    {
        EndModal(ID_UPDATE);
    }
}


/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_CHANGE
 */

void CDlgAccountManagerStatus::OnChangeClick( wxCommandEvent& WXUNUSED(event) )
{
    if ( Validate() && TransferDataFromWindow() )
    {
        EndModal(ID_CHANGE);
    }
}


const char *BOINC_RCSID_3c7bab9af5="$Id$";
