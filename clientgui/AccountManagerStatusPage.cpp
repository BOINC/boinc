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
    m_pTitleStaticCtrl = NULL;
    m_pAcctManagerDescriptionStaticCtrl = NULL;
    m_pAcctManagerNameStaticCtrl = NULL;
    m_pAcctManagerNameCtrl = NULL;
    m_pAcctManagerURLStaticCtrl = NULL;
    m_pAcctManagerURLCtrl = NULL;
    m_pAcctManagerUpdateCtrl = NULL;
    m_pAcctManagerUpdateDescriptionStaticCtrl = NULL;
    m_pAcctManagerRemoveCtrl = NULL;
    m_pAcctManagerRemoveDescriptionStaticCtrl = NULL;
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

    m_pTitleStaticCtrl = new wxStaticText;
    m_pTitleStaticCtrl->Create( itemWizardPage111, wxID_STATIC, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    m_pTitleStaticCtrl->SetFont(wxFont(10, wxSWISS, wxNORMAL, wxBOLD, FALSE, _T("Verdana")));
    itemBoxSizer112->Add(m_pTitleStaticCtrl, 0, wxALIGN_LEFT|wxALL, 5);

    itemBoxSizer112->Add(5, 5, 0, wxALIGN_LEFT|wxALL, 5);

    m_pAcctManagerDescriptionStaticCtrl = new wxStaticText;
    m_pAcctManagerDescriptionStaticCtrl->Create( itemWizardPage111, wxID_STATIC, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer112->Add(m_pAcctManagerDescriptionStaticCtrl, 0, wxALIGN_LEFT|wxALL|wxADJUST_MINSIZE, 5);

    wxFlexGridSizer* itemFlexGridSizer5 = new wxFlexGridSizer(2, 2, 0, 0);
    itemBoxSizer112->Add(itemFlexGridSizer5, 0, wxALIGN_LEFT|wxALL|wxADJUST_MINSIZE, 5);

    m_pAcctManagerNameStaticCtrl = new wxStaticText;
    m_pAcctManagerNameStaticCtrl->Create( itemWizardPage111, wxID_STATIC, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer5->Add(m_pAcctManagerNameStaticCtrl, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 5);

    m_pAcctManagerNameCtrl = new wxStaticText;
    m_pAcctManagerNameCtrl->Create( itemWizardPage111, ID_ACCTMANAGERNAMECTRL, wxEmptyString, wxDefaultPosition, wxSize(200, -1), 0 );
    itemFlexGridSizer5->Add(m_pAcctManagerNameCtrl, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_pAcctManagerURLStaticCtrl = new wxStaticText;
    m_pAcctManagerURLStaticCtrl->Create( itemWizardPage111, wxID_STATIC, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer5->Add(m_pAcctManagerURLStaticCtrl, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 5);

    m_pAcctManagerURLCtrl = new wxHyperLink;
    m_pAcctManagerURLCtrl->Create( itemWizardPage111, ID_ACCTMANAGERLINKCTRL, wxEmptyString, wxDefaultPosition, wxSize(200, -1), 0 );
    itemFlexGridSizer5->Add(m_pAcctManagerURLCtrl, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    itemBoxSizer112->Add(5, 5, 0, wxALIGN_LEFT|wxALL, 5);

    m_pAcctManagerUpdateCtrl = new wxRadioButton;
    m_pAcctManagerUpdateCtrl->Create( itemWizardPage111, ID_ACCTMANAGERUPDATECTRL, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    m_pAcctManagerUpdateCtrl->SetValue(FALSE);
    itemBoxSizer112->Add(m_pAcctManagerUpdateCtrl, 0, wxALIGN_LEFT|wxALL, 5);

    wxBoxSizer* itemBoxSizer12 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer112->Add(itemBoxSizer12, 0, wxALIGN_LEFT|wxALL, 0);

    itemBoxSizer12->Add(5, 5, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_pAcctManagerUpdateDescriptionStaticCtrl = new wxStaticText;
    m_pAcctManagerUpdateDescriptionStaticCtrl->Create( itemWizardPage111, wxID_STATIC, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer12->Add(m_pAcctManagerUpdateDescriptionStaticCtrl, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_pAcctManagerRemoveCtrl = new wxRadioButton;
    m_pAcctManagerRemoveCtrl->Create( itemWizardPage111, ID_ACCTMANAGERREMOVECTRL, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    m_pAcctManagerRemoveCtrl->SetValue(FALSE);
    itemBoxSizer112->Add(m_pAcctManagerRemoveCtrl, 0, wxALIGN_LEFT|wxALL, 5);

    wxBoxSizer* itemBoxSizer16 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer112->Add(itemBoxSizer16, 0, wxALIGN_LEFT|wxALL, 0);

    itemBoxSizer16->Add(5, 5, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_pAcctManagerRemoveDescriptionStaticCtrl = new wxStaticText;
    m_pAcctManagerRemoveDescriptionStaticCtrl->Create( itemWizardPage111, wxID_STATIC, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer16->Add(m_pAcctManagerRemoveDescriptionStaticCtrl, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    // Set validators
    m_pAcctManagerNameCtrl->SetValidator( wxGenericValidator(& m_strAcctManagerName) );
    m_pAcctManagerURLCtrl->SetValidator( wxGenericValidator(& m_strAcctManagerURL) );

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
    } else if (m_pAcctManagerUpdateCtrl->GetValue() && ((CWizardAccountManager*)GetParent())->m_bCredentialsCached) {
        // We are supposed to update and we already have credentials to the
        //   account manager
        return PAGE_TRANSITION_NEXT(ID_ACCOUNTMANAGERPROCESSINGPAGE);
    } else if (m_pAcctManagerUpdateCtrl->GetValue()) {
        // We are supposed to update and we do not have credentials to the
        //   account manager
        ((CWizardAccountManager*)GetParent())->IsAccountManagerUpdateWizard = true;
        ((CWizardAccountManager*)GetParent())->IsAccountManagerRemoveWizard = false;
        return PAGE_TRANSITION_NEXT(ID_ACCOUNTINFOPAGE);
    } else if (m_pAcctManagerRemoveCtrl->GetValue()) {
        // We are supposed to the account manager
        ((CWizardAccountManager*)GetParent())->IsAccountManagerUpdateWizard = false;
        ((CWizardAccountManager*)GetParent())->IsAccountManagerRemoveWizard = true;
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

    wxASSERT(m_pTitleStaticCtrl);
    wxASSERT(m_pAcctManagerDescriptionStaticCtrl);
    wxASSERT(m_pAcctManagerNameStaticCtrl);
    wxASSERT(m_pAcctManagerNameCtrl);
    wxASSERT(m_pAcctManagerURLStaticCtrl);
    wxASSERT(m_pAcctManagerURLCtrl);
    wxASSERT(m_pAcctManagerUpdateCtrl);
    wxASSERT(m_pAcctManagerUpdateDescriptionStaticCtrl);
    wxASSERT(m_pAcctManagerRemoveCtrl);
    wxASSERT(m_pAcctManagerRemoveDescriptionStaticCtrl);


    m_pTitleStaticCtrl->SetLabel(
        _("Account Manager")
    );
    m_pAcctManagerDescriptionStaticCtrl->SetLabel(
        _("Your current account manager is:")
    );
    m_pAcctManagerNameStaticCtrl->SetLabel(
        _("Name:")
    );
    m_pAcctManagerNameCtrl->SetLabel(
        ((CWizardAccountManager*)GetParent())->m_strProjectName
    );
    m_pAcctManagerNameStaticCtrl->SetLabel(
        _("URL:")
    );
    m_pAcctManagerURLCtrl->SetLabel(
        ((CWizardAccountManager*)GetParent())->m_AccountManagerInfoPage->GetProjectURL()
    );
    m_pAcctManagerUpdateCtrl->SetLabel(
        _("Update")
    );
    m_pAcctManagerUpdateDescriptionStaticCtrl->SetLabel(
        _("Get latest settings from account manager.")
    );
    m_pAcctManagerRemoveCtrl->SetLabel(
        _("Remove")
    );
    m_pAcctManagerRemoveDescriptionStaticCtrl->SetLabel(
        _("Stop using account manager.")
    );

    Fit();
}

/*!
 * wxEVT_WIZARD_CANCEL event handler for ID_ERRNACCOUNTDOESNOTEXISTPAGE
 */

void CAccountManagerStatusPage::OnCancel( wxWizardExEvent& event ) {
    PROCESS_CANCELEVENT(event);
}

