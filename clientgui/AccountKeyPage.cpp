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
#pragma implementation "AccountKeyPage.h"
#endif

#include "stdwx.h"
#include "wizardex.h"
#include "error_numbers.h"
#include "BOINCGUIApp.h"
#include "ValidateAccountKey.h"
#include "BOINCWizards.h"
#include "BOINCBaseWizard.h"
#include "AccountKeyPage.h"


/*!
 * CAccountKeyPage type definition
 */
 
IMPLEMENT_DYNAMIC_CLASS( CAccountKeyPage, wxWizardPageEx )
 
/*!
 * CAccountKeyPage event table definition
 */
 
BEGIN_EVENT_TABLE( CAccountKeyPage, wxWizardPageEx )
 
////@begin CAccountKeyPage event table entries
    EVT_WIZARDEX_PAGE_CHANGED( -1, CAccountKeyPage::OnPageChanged )
    EVT_WIZARDEX_CANCEL( -1, CAccountKeyPage::OnCancel )

////@end CAccountKeyPage event table entries
 
END_EVENT_TABLE()
  
/*!
 * CAccountKeyPage constructors
 */
 
CAccountKeyPage::CAccountKeyPage( )
{
}
  
CAccountKeyPage::CAccountKeyPage( CBOINCBaseWizard* parent )
{
    Create( parent );
}
  
/*!
 * CAuthenticatorPage creator
 */
 
bool CAccountKeyPage::Create( CBOINCBaseWizard* parent )
{
////@begin CAccountKeyPage member initialisation
    m_AccountKeyStaticCtrl = NULL;
    m_AccountKeyCtrl = NULL;
////@end CAccountKeyPage member initialisation
 
////@begin CAccountKeyPage creation
    wxBitmap wizardBitmap(wxNullBitmap);
    wxWizardPageEx::Create( parent, wizardBitmap );

    CreateControls();
    GetSizer()->Fit(this);
////@end CAccountKeyPage creation

    return TRUE;
}
  
/*!
 * Control creation for CAuthenticatorPage
 */
 
void CAccountKeyPage::CreateControls()
{    
 
////@begin CAccountKeyPage content construction
    CAccountKeyPage* itemWizardPage44 = this;

    wxBoxSizer* itemBoxSizer45 = new wxBoxSizer(wxVERTICAL);
    itemWizardPage44->SetSizer(itemBoxSizer45);

    wxStaticText* itemStaticText46 = new wxStaticText;
    itemStaticText46->Create( itemWizardPage44, wxID_STATIC, _("Enter account key"), wxDefaultPosition, wxDefaultSize, 0 );
    itemStaticText46->SetFont(wxFont(10, wxSWISS, wxNORMAL, wxBOLD, FALSE, _T("Verdana")));
    itemBoxSizer45->Add(itemStaticText46, 0, wxALIGN_LEFT|wxALL, 5);

    itemBoxSizer45->Add(5, 5, 0, wxALIGN_LEFT|wxALL, 5);

    wxStaticText* itemStaticText48 = new wxStaticText;
    itemStaticText48->Create( itemWizardPage44, wxID_STATIC, _("This project uses an \"account key\" to identify you."), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer45->Add(itemStaticText48, 0, wxALIGN_LEFT|wxALL, 5);

    wxStaticText* itemStaticText49 = new wxStaticText;
    itemStaticText49->Create( itemWizardPage44, wxID_STATIC, _("Go to the project's web site to create an account. Your account\nkey will be emailed to you."), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer45->Add(itemStaticText49, 0, wxALIGN_LEFT|wxALL, 5);

    wxStaticText* itemStaticText50 = new wxStaticText;
    itemStaticText50->Create( itemWizardPage44, wxID_STATIC, _("An account key looks like:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer45->Add(itemStaticText50, 0, wxALIGN_LEFT|wxALL, 5);

    wxStaticText* itemStaticText51 = new wxStaticText;
    itemStaticText51->Create( itemWizardPage44, wxID_STATIC, _("82412313ac88e9a3638f66ea82186948"), wxDefaultPosition, wxDefaultSize, 0 );
    itemStaticText51->SetFont(wxFont(8, wxSWISS, wxNORMAL, wxNORMAL, FALSE, _T("Courier New")));
    itemBoxSizer45->Add(itemStaticText51, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 0);

    itemBoxSizer45->Add(5, 5, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxFlexGridSizer* itemFlexGridSizer53 = new wxFlexGridSizer(1, 2, 0, 0);
    itemFlexGridSizer53->AddGrowableCol(1);
    itemBoxSizer45->Add(itemFlexGridSizer53, 0, wxGROW|wxALL, 5);

    m_AccountKeyStaticCtrl = new wxStaticText;
    m_AccountKeyStaticCtrl->Create( itemWizardPage44, ID_ACCOUNTKEYSTATICCTRL, _("Account key:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer53->Add(m_AccountKeyStaticCtrl, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_AccountKeyCtrl = new wxTextCtrl;
    m_AccountKeyCtrl->Create( itemWizardPage44, ID_ACCOUNTKEYCTRL, _T(""), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer53->Add(m_AccountKeyCtrl, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    // Set validators
    m_AccountKeyCtrl->SetValidator( CValidateAccountKey( & m_strAccountKey) );
////@end CAccountKeyPage content construction
 
}
  
/*!
 * Gets the previous page.
 */
 
wxWizardPageEx* CAccountKeyPage::GetPrev() const
{
    return PAGE_TRANSITION_BACK;
}
  
/*!
 * Gets the next page.
 */
 
wxWizardPageEx* CAccountKeyPage::GetNext() const
{
    if (CHECK_CLOSINGINPROGRESS()) {
        // Cancel Event Detected
        return PAGE_TRANSITION_NEXT(ID_COMPLETIONERRORPAGE);
    } else {
        return PAGE_TRANSITION_NEXT(ID_PROJECTPROCESSINGPAGE);
    }
    return NULL;
}
 
/*!
 * Should we show tooltips?
 */
 
bool CAccountKeyPage::ShowToolTips()
{
    return TRUE;
}
  
/*!
 * Get bitmap resources
 */
 
wxBitmap CAccountKeyPage::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CAccountKeyPage bitmap retrieval
    return wxNullBitmap;
////@end CAccountKeyPage bitmap retrieval
}
  
/*!
 * Get icon resources
 */

wxIcon CAccountKeyPage::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CAccountKeyPage icon retrieval
    return wxNullIcon;
////@end CAccountKeyPage icon retrieval
}
  
/*!
 * wxEVT_WIZARD_PAGE_CHANGED event handler for ID_ACCOUNTKEYPAGE
 */
 
void CAccountKeyPage::OnPageChanged( wxWizardExEvent& event ) {
    if (m_AccountKeyCtrl) m_AccountKeyCtrl->SetFocus();
}
  
/*!
 * wxEVT_WIZARD_CANCEL event handler for ID_ACCOUNTKEYPAGE
 */
 
void CAccountKeyPage::OnCancel( wxWizardExEvent& event ) {
    PROCESS_CANCELEVENT(event);
}

