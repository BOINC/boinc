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
// 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
//
#if defined(__GNUG__) && !defined(__APPLE__)
#pragma implementation "AccountKeyPage.h"
#endif

#include "stdwx.h"
#include "diagnostics.h"
#include "util.h"
#include "mfile.h"
#include "miofile.h"
#include "parse.h"
#include "error_numbers.h"
#include "wizardex.h"
#include "error_numbers.h"
#include "BOINCGUIApp.h"
#include "SkinManager.h"
#include "MainDocument.h"
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
    m_pTitleStaticCtrl = NULL;
    m_pDirectionsStaticCtrl = NULL;
    m_pAccountKeyExampleDescriptionStaticCtrl = NULL;
    m_pAccountKeyExampleStaticCtrl = NULL;
    m_pAccountKeyStaticCtrl = NULL;
    m_pAccountKeyCtrl = NULL;
////@end CAccountKeyPage member initialisation
 
////@begin CAccountKeyPage creation
    wxBitmap wizardBitmap(wxNullBitmap);
    wxWizardPageEx::Create( parent, ID_ACCOUNTKEYPAGE, wizardBitmap );

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

    m_pTitleStaticCtrl = new wxStaticText;
    m_pTitleStaticCtrl->Create( itemWizardPage44, wxID_STATIC, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    m_pTitleStaticCtrl->SetFont(wxFont(10, wxSWISS, wxNORMAL, wxBOLD, FALSE, _T("Verdana")));
    itemBoxSizer45->Add(m_pTitleStaticCtrl, 0, wxALIGN_LEFT|wxALL, 5);

    itemBoxSizer45->Add(5, 5, 0, wxALIGN_LEFT|wxALL, 5);

    m_pDirectionsStaticCtrl = new wxStaticText;
    m_pDirectionsStaticCtrl->Create( itemWizardPage44, wxID_STATIC, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer45->Add(m_pDirectionsStaticCtrl, 0, wxALIGN_LEFT|wxALL, 5);

    m_pAccountKeyExampleDescriptionStaticCtrl = new wxStaticText;
    m_pAccountKeyExampleDescriptionStaticCtrl->Create( itemWizardPage44, wxID_STATIC, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer45->Add(m_pAccountKeyExampleDescriptionStaticCtrl, 0, wxALIGN_LEFT|wxALL, 5);

    m_pAccountKeyExampleStaticCtrl = new wxStaticText;
    m_pAccountKeyExampleStaticCtrl->Create( itemWizardPage44, wxID_STATIC, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    m_pAccountKeyExampleStaticCtrl->SetFont(wxFont(8, wxSWISS, wxNORMAL, wxNORMAL, FALSE, _T("Courier New")));
    itemBoxSizer45->Add(m_pAccountKeyExampleStaticCtrl, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 0);

    itemBoxSizer45->Add(5, 5, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxFlexGridSizer* itemFlexGridSizer53 = new wxFlexGridSizer(1, 2, 0, 0);
    itemFlexGridSizer53->AddGrowableCol(1);
    itemBoxSizer45->Add(itemFlexGridSizer53, 0, wxGROW|wxALL, 5);

    m_pAccountKeyStaticCtrl = new wxStaticText;
    m_pAccountKeyStaticCtrl->Create( itemWizardPage44, ID_ACCOUNTKEYSTATICCTRL, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer53->Add(m_pAccountKeyStaticCtrl, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_pAccountKeyCtrl = new wxTextCtrl;
    m_pAccountKeyCtrl->Create( itemWizardPage44, ID_ACCOUNTKEYCTRL, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer53->Add(m_pAccountKeyCtrl, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    // Set validators
    m_pAccountKeyCtrl->SetValidator( CValidateAccountKey( & m_strAccountKey) );
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
 
wxBitmap CAccountKeyPage::GetBitmapResource( const wxString& WXUNUSED(name) )
{
    // Bitmap retrieval
////@begin CAccountKeyPage bitmap retrieval
    return wxNullBitmap;
////@end CAccountKeyPage bitmap retrieval
}
  
/*!
 * Get icon resources
 */

wxIcon CAccountKeyPage::GetIconResource( const wxString& WXUNUSED(name) )
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
    if (event.GetDirection() == false) return;

    wxASSERT(m_pTitleStaticCtrl);
    wxASSERT(m_pDirectionsStaticCtrl);
    wxASSERT(m_pAccountKeyExampleDescriptionStaticCtrl);
    wxASSERT(m_pAccountKeyExampleStaticCtrl);
    wxASSERT(m_pAccountKeyStaticCtrl);
    wxASSERT(m_pAccountKeyCtrl);

    m_pTitleStaticCtrl->SetLabel(
        _("Enter account key")
    );
    m_pDirectionsStaticCtrl->SetLabel(
        _("This project uses an \"account key\" to identify you.\n"
          "\n"
          "Go to the project's web site to create an account. Your account\n"
          "key will be emailed to you.")
    );
    m_pAccountKeyExampleDescriptionStaticCtrl->SetLabel(
        _("An account key looks like:")
    );
    m_pAccountKeyExampleStaticCtrl->SetLabel(
        _("82412313ac88e9a3638f66ea82186948")
    );
    m_pAccountKeyStaticCtrl->SetLabel(
        _("Account key:")
    );

    Fit();
    m_pAccountKeyCtrl->SetFocus();
}
  
/*!
 * wxEVT_WIZARD_CANCEL event handler for ID_ACCOUNTKEYPAGE
 */
 
void CAccountKeyPage::OnCancel( wxWizardExEvent& event ) {
    PROCESS_CANCELEVENT(event);
}

