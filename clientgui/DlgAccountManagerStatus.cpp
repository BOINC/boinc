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

    wxFlexGridSizer* itemFlexGridSizer4 = new wxFlexGridSizer(3, 1, 0, 0);
    itemBoxSizer3->Add(itemFlexGridSizer4, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText5 = new wxStaticText;
    itemStaticText5->Create( itemDialog1, wxID_STATIC, _("Your current account manager is:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer4->Add(itemStaticText5, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_AcctManagerNameCtrl = new wxStaticText;
    m_AcctManagerNameCtrl->Create( itemDialog1, ID_ACCTMANAGERNAME, _("foo"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer4->Add(m_AcctManagerNameCtrl, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxHyperLink* itemHyperLink7 = new wxHyperLink;
    itemHyperLink7->Create( itemDialog1, ID_ACCTMANAGERLINK, wxT("http://a/b/c"), wxDefaultPosition, wxDefaultSize, wxNO_BORDER );
    itemFlexGridSizer4->Add(itemHyperLink7, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxBoxSizer* itemBoxSizer8 = new wxBoxSizer(wxVERTICAL);
    itemBoxSizer3->Add(itemBoxSizer8, 0, wxALIGN_TOP|wxALL, 5);

    wxButton* itemButton9 = new wxButton;
    itemButton9->Create( itemDialog1, ID_UPDATE, _("&Update"), wxDefaultPosition, wxDefaultSize, 0 );
    itemButton9->SetDefault();
    itemBoxSizer8->Add(itemButton9, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxButton* itemButton10 = new wxButton;
    itemButton10->Create( itemDialog1, ID_CHANGE, _("Change"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer8->Add(itemButton10, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxButton* itemButton11 = new wxButton;
    itemButton11->Create( itemDialog1, wxID_CANCEL, _("&Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer8->Add(itemButton11, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    // Set validators
    m_AcctManagerNameCtrl->SetValidator( wxGenericValidator(& m_strAcctManagerName) );
    itemHyperLink7->SetValidator( wxGenericValidator(& m_strAcctManagerURL) );
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

wxBitmap CDlgAccountManagerStatus::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CDlgAccountManagerStatus bitmap retrieval
    return wxNullBitmap;
////@end CDlgAccountManagerStatus bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CDlgAccountManagerStatus::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CDlgAccountManagerStatus icon retrieval
    return wxNullIcon;
////@end CDlgAccountManagerStatus icon retrieval
}

/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_UPDATE
 */

void CDlgAccountManagerStatus::OnUpdateClick( wxCommandEvent& event )
{
    if ( Validate() && TransferDataFromWindow() )
    {
        EndModal(ID_UPDATE);
    }
}


/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_CHANGE
 */

void CDlgAccountManagerStatus::OnChangeClick( wxCommandEvent& event )
{
    if ( Validate() && TransferDataFromWindow() )
    {
        EndModal(ID_CHANGE);
    }
}


