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
#pragma implementation "AccountManagerStatusPage.h"
#endif

#include "stdwx.h"
#include "wizardex.h"
#include "wx/valgen.h"
#include "error_numbers.h"
#include "BOINCGUIApp.h"
#include "hyperlink.h"
#include "BOINCWizards.h"
#include "BOINCBaseWizard.h"
#include "WizardAccountManager.h"
#include "AccountManagerStatusPage.h"
#include "AccountManagerInfoPage.h"
#include "AccountInfoPage.h"


/*!
 * CAccountManagerStatusPage type definition
 */
 
IMPLEMENT_DYNAMIC_CLASS( CAccountManagerStatusPage, wxWizardPageEx )
 
/*!
 * CAccountManagerStatusPage event table definition
 */
 
BEGIN_EVENT_TABLE( CAccountManagerStatusPage, wxWizardPageEx )

////@begin CAccountManagerStatusPage event table entries
    EVT_WIZARDEX_PAGE_CHANGED( -1, CAccountManagerStatusPage::OnPageChanged )
    EVT_WIZARDEX_CANCEL( -1, CAccountManagerStatusPage::OnCancel )

////@end CAccountManagerStatusPage event table entries
 
END_EVENT_TABLE()
 
/*!
 * CAccountManagerStatusPage constructors
 */
 
CAccountManagerStatusPage::CAccountManagerStatusPage( )
{
}
 
CAccountManagerStatusPage::CAccountManagerStatusPage( CBOINCBaseWizard* parent )
{
    Create( parent );
}
 
/*!
 * CErrNoInternetConnection creator
 */
 
bool CAccountManagerStatusPage::Create( CBOINCBaseWizard* parent )
{
////@begin CAccountManagerStatusPage member initialisation
////@end CAccountManagerStatusPage member initialisation

////@begin CAccountManagerStatusPage creation
    wxBitmap wizardBitmap(wxNullBitmap);
    wxWizardPageEx::Create( parent, wizardBitmap );

    CreateControls();
    GetSizer()->Fit(this);
////@end CAccountManagerStatusPage creation

    return TRUE;
}
 
/*!
 * Control creation for CErrNoInternetConnection
 */
 
void CAccountManagerStatusPage::CreateControls()
{    
////@begin CAccountManagerStatusPage content construction
    CAccountManagerStatusPage* itemWizardPage111 = this;

    wxBoxSizer* itemBoxSizer112 = new wxBoxSizer(wxVERTICAL);
    itemWizardPage111->SetSizer(itemBoxSizer112);

    wxStaticText* itemStaticText113 = new wxStaticText;
    itemStaticText113->Create( itemWizardPage111, wxID_STATIC, _("Account Manager Status"), wxDefaultPosition, wxDefaultSize, 0 );
    itemStaticText113->SetFont(wxFont(10, wxSWISS, wxNORMAL, wxBOLD, FALSE, _T("Verdana")));
    itemBoxSizer112->Add(itemStaticText113, 0, wxALIGN_LEFT|wxALL, 5);

    itemBoxSizer112->Add(5, 5, 0, wxALIGN_LEFT|wxALL, 5);

    wxStaticText* itemStaticText4 = new wxStaticText;
    itemStaticText4->Create( itemWizardPage111, wxID_STATIC, _("Your current account manager is:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer112->Add(itemStaticText4, 0, wxALIGN_LEFT|wxALL|wxADJUST_MINSIZE, 5);

    wxFlexGridSizer* itemFlexGridSizer5 = new wxFlexGridSizer(2, 2, 0, 0);
    itemBoxSizer112->Add(itemFlexGridSizer5, 0, wxALIGN_LEFT|wxALL|wxADJUST_MINSIZE, 5);

    wxStaticText* itemStaticText6 = new wxStaticText;
    itemStaticText6->Create( itemWizardPage111, wxID_STATIC, _("Name:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer5->Add(itemStaticText6, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 5);

    m_AcctManagerNameCtrl = new wxStaticText;
    m_AcctManagerNameCtrl->Create( itemWizardPage111, ID_ACCTMANAGERNAMECTRL, _("foo"), wxDefaultPosition, wxSize(200, -1), 0 );
    itemFlexGridSizer5->Add(m_AcctManagerNameCtrl, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText8 = new wxStaticText;
    itemStaticText8->Create( itemWizardPage111, wxID_STATIC, _("URL:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer5->Add(itemStaticText8, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 5);

    m_AcctManagerURLCtrl = new wxHyperLink;
    m_AcctManagerURLCtrl->Create( itemWizardPage111, ID_ACCTMANAGERLINKCTRL, wxT("http://a/b/c"), wxDefaultPosition, wxSize(200, -1), 0 );
    itemFlexGridSizer5->Add(m_AcctManagerURLCtrl, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    itemBoxSizer112->Add(5, 5, 0, wxALIGN_LEFT|wxALL, 5);

    m_AcctManagerUpdateCtrl = new wxRadioButton;
    m_AcctManagerUpdateCtrl->Create( itemWizardPage111, ID_ACCTMANAGERUPDATECTRL, _("Update"), wxDefaultPosition, wxDefaultSize, 0 );
    m_AcctManagerUpdateCtrl->SetValue(FALSE);
    itemBoxSizer112->Add(m_AcctManagerUpdateCtrl, 0, wxALIGN_LEFT|wxALL, 5);

    wxBoxSizer* itemBoxSizer12 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer112->Add(itemBoxSizer12, 0, wxALIGN_LEFT|wxALL, 0);

    itemBoxSizer12->Add(5, 5, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText14 = new wxStaticText;
    itemStaticText14->Create( itemWizardPage111, wxID_STATIC, _("Updates BOINC with the latest settings from the account manager."), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer12->Add(itemStaticText14, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_AcctManagerRemoveCtrl = new wxRadioButton;
    m_AcctManagerRemoveCtrl->Create( itemWizardPage111, ID_ACCTMANAGERREMOVECTRL, _("Remove"), wxDefaultPosition, wxDefaultSize, 0 );
    m_AcctManagerRemoveCtrl->SetValue(FALSE);
    itemBoxSizer112->Add(m_AcctManagerRemoveCtrl, 0, wxALIGN_LEFT|wxALL, 5);

    wxBoxSizer* itemBoxSizer16 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer112->Add(itemBoxSizer16, 0, wxALIGN_LEFT|wxALL, 0);

    itemBoxSizer16->Add(5, 5, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText18 = new wxStaticText;
    itemStaticText18->Create( itemWizardPage111, wxID_STATIC, _("Removes BOINC from account manager control."), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer16->Add(itemStaticText18, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    // Set validators
    m_AcctManagerNameCtrl->SetValidator( wxGenericValidator(& m_strAcctManagerName) );
    m_AcctManagerURLCtrl->SetValidator( wxGenericValidator(& m_strAcctManagerURL) );

////@end CAccountManagerStatusPage content construction
}
 
/*!
 * Gets the previous page.
 */
 
wxWizardPageEx* CAccountManagerStatusPage::GetPrev() const
{
    return PAGE_TRANSITION_BACK;
}
 
/*!
 * Gets the next page.
 */

wxWizardPageEx* CAccountManagerStatusPage::GetNext() const
{
    if (CHECK_CLOSINGINPROGRESS()) {
        // Cancel Event Detected
        return PAGE_TRANSITION_NEXT(ID_COMPLETIONERRORPAGE);
    } else if (m_AcctManagerUpdateCtrl->GetValue() && ((CWizardAccountManager*)GetParent())->m_bCredentialsCached) {
        // We are supposed to update and we already have credentials to the
        //   account manager
        return PAGE_TRANSITION_NEXT(ID_ACCOUNTMANAGERPROCESSINGPAGE);
    } else if (m_AcctManagerUpdateCtrl->GetValue()) {
        // We are supposed to update and we do not have credentials to the
        //   account manager
        return PAGE_TRANSITION_NEXT(ID_ACCOUNTINFOPAGE);
    } else if (m_AcctManagerRemoveCtrl->GetValue()) {
        // We are supposed to the account manager
        ((CWizardAccountManager*)GetParent())->m_AccountManagerInfoPage->SetProjectURL(wxEmptyString);
        ((CWizardAccountManager*)GetParent())->m_AccountInfoPage->SetAccountEmailAddress(wxEmptyString);
        ((CWizardAccountManager*)GetParent())->m_AccountInfoPage->SetAccountPassword(wxEmptyString);
        ((CWizardAccountManager*)GetParent())->m_bCredentialsCached = false;
        return PAGE_TRANSITION_NEXT(ID_ACCOUNTMANAGERPROCESSINGPAGE);
    }
    return NULL;
}
 
/*!
 * Should we show tooltips?
 */
 
bool CAccountManagerStatusPage::ShowToolTips()
{
    return TRUE;
}
 
/*!
 * Get bitmap resources
 */
 
wxBitmap CAccountManagerStatusPage::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CAccountManagerStatusPage bitmap retrieval
    return wxNullBitmap;
////@end CAccountManagerStatusPage bitmap retrieval
}
 
/*!
 * Get icon resources
 */

wxIcon CAccountManagerStatusPage::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CAccountManagerStatusPage icon retrieval
    return wxNullIcon;
////@end CAccountManagerStatusPage icon retrieval
}
  
/*!
 * wxEVT_WIZARD_PAGE_CHANGED event handler for ID_ERRNACCOUNTDOESNOTEXISTPAGE
 */

void CAccountManagerStatusPage::OnPageChanged( wxWizardExEvent& event ) {
    if (event.GetDirection() == false) return;

    // Update the Acctount Manager URL and Name static text boxes
    m_AcctManagerNameCtrl->SetLabel(((CWizardAccountManager*)GetParent())->m_strProjectName);
    m_AcctManagerURLCtrl->SetLabel(((CWizardAccountManager*)GetParent())->m_AccountManagerInfoPage->GetProjectURL());
    Fit();
}

/*!
 * wxEVT_WIZARD_CANCEL event handler for ID_ERRNACCOUNTDOESNOTEXISTPAGE
 */

void CAccountManagerStatusPage::OnCancel( wxWizardExEvent& event ) {
    PROCESS_CANCELEVENT(event);
}

