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
#pragma implementation "AccountInfoPage.h"
#endif

#include "stdwx.h"
#include "wizardex.h"
#include "error_numbers.h"
#include "BOINCGUIApp.h"
#include "wx/valgen.h"
#include "BOINCWizards.h"
#include "BOINCBaseWizard.h"
#include "AccountInfoPage.h"


/*!
 * CAccountInfoPage type definition
 */
 
IMPLEMENT_DYNAMIC_CLASS( CAccountInfoPage, wxWizardPageEx )
 
/*!
 * CAccountInfoPage event table definition
 */
 
BEGIN_EVENT_TABLE( CAccountInfoPage, wxWizardPageEx )
 
////@begin CAccountInfoPage event table entries
    EVT_WIZARDEX_PAGE_CHANGED( -1, CAccountInfoPage::OnPageChanged )
    EVT_WIZARDEX_PAGE_CHANGING( -1, CAccountInfoPage::OnPageChanging )
    EVT_WIZARDEX_CANCEL( -1, CAccountInfoPage::OnCancel )

    EVT_RADIOBUTTON( ID_ACCOUNTCREATECTRL, CAccountInfoPage::OnAccountCreateCtrlSelected )
    EVT_RADIOBUTTON( ID_ACCOUNTUSEEXISTINGCTRL, CAccountInfoPage::OnAccountUseExistingCtrlSelected )
////@end CAccountInfoPage event table entries
 
END_EVENT_TABLE()
 
/*!
 * CAccountInfoPage constructors
 */
 
CAccountInfoPage::CAccountInfoPage( )
{
}
 
CAccountInfoPage::CAccountInfoPage( CBOINCBaseWizard* parent )
{
    Create( parent );
}
 
/*!
 * AccountInfoPage creator
 */
 
bool CAccountInfoPage::Create( CBOINCBaseWizard* parent )
{

////@begin CAccountInfoPage member initialisation
    m_AccountCreateCtrl = NULL;
    m_AccountUseExistingCtrl = NULL;
    m_AccountEmailAddressStaticCtrl = NULL;
    m_AccountEmailAddressCtrl = NULL;
    m_AccountPasswordStaticCtrl = NULL;
    m_AccountPasswordCtrl = NULL;
    m_AccountConfirmPasswordStaticCtrl = NULL;
    m_AccountConfirmPasswordCtrl = NULL;
////@end CAccountInfoPage member initialisation
 
////@begin CAccountInfoPage creation
    wxBitmap wizardBitmap(wxNullBitmap);
    wxWizardPageEx::Create( parent, wizardBitmap );

    CreateControls();
    GetSizer()->Fit(this);
////@end CAccountInfoPage creation

    return TRUE;

}
 
/*!
 * Control creation for AccountInfoPage
 */
 
void CAccountInfoPage::CreateControls()
{    

////@begin CAccountInfoPage content construction
    CAccountInfoPage* itemWizardPage56 = this;

    wxBoxSizer* itemBoxSizer57 = new wxBoxSizer(wxVERTICAL);
    itemWizardPage56->SetSizer(itemBoxSizer57);

    wxStaticText* itemStaticText58 = new wxStaticText;
    itemStaticText58->Create( itemWizardPage56, wxID_STATIC, _("Account information"), wxDefaultPosition, wxDefaultSize, 0 );
    itemStaticText58->SetFont(wxFont(10, wxSWISS, wxNORMAL, wxBOLD, FALSE, _T("Verdana")));
    itemBoxSizer57->Add(itemStaticText58, 0, wxALIGN_LEFT|wxALL, 5);

    if (!IS_ACCOUNTMANAGERWIZARD()) {
        wxStaticText* itemStaticText59 = new wxStaticText;
        itemStaticText59->Create( itemWizardPage56, wxID_STATIC, _("Do you wish to use an existing account or create a new one?"), wxDefaultPosition, wxDefaultSize, 0 );
        itemBoxSizer57->Add(itemStaticText59, 0, wxALIGN_LEFT|wxALL, 5);

        itemBoxSizer57->Add(5, 5, 0, wxALIGN_LEFT|wxALL, 5);

        wxStaticText* itemStaticText60 = new wxStaticText;
        itemStaticText60->Create( itemWizardPage56, wxID_STATIC, _("Transition Note: Account keys are used as passwords for accounts created\nbefore BOINC migrated to a username and password authentication scheme."), wxDefaultPosition, wxDefaultSize, 0 );
        itemBoxSizer57->Add(itemStaticText60, 0, wxALIGN_LEFT|wxALL, 5);

        itemBoxSizer57->Add(5, 5, 0, wxALIGN_LEFT|wxALL, 5);

        wxFlexGridSizer* itemFlexGridSizer61 = new wxFlexGridSizer(1, 2, 0, 0);
        itemFlexGridSizer61->AddGrowableCol(1);
        itemBoxSizer57->Add(itemFlexGridSizer61, 0, wxGROW|wxALL, 5);

        m_AccountCreateCtrl = new wxRadioButton;
        m_AccountCreateCtrl->Create( itemWizardPage56, ID_ACCOUNTCREATECTRL, _("Create new &account"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP );
        m_AccountCreateCtrl->SetValue(TRUE);
        itemFlexGridSizer61->Add(m_AccountCreateCtrl, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

        m_AccountUseExistingCtrl = new wxRadioButton;
        m_AccountUseExistingCtrl->Create( itemWizardPage56, ID_ACCOUNTUSEEXISTINGCTRL, _("&Use existing account"), wxDefaultPosition, wxDefaultSize, 0 );
        m_AccountUseExistingCtrl->SetValue(FALSE);
        itemFlexGridSizer61->Add(m_AccountUseExistingCtrl, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    }

    wxFlexGridSizer* itemFlexGridSizer64 = new wxFlexGridSizer(3, 2, 0, 0);
    itemFlexGridSizer64->AddGrowableCol(1);
    itemBoxSizer57->Add(itemFlexGridSizer64, 0, wxGROW|wxALL, 5);

    m_AccountEmailAddressStaticCtrl = new wxStaticText;
    m_AccountEmailAddressStaticCtrl->Create( itemWizardPage56, ID_ACCOUNTEMAILADDRESSSTATICCTRL, _("&Email address:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer64->Add(m_AccountEmailAddressStaticCtrl, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_AccountEmailAddressCtrl = new wxTextCtrl;
    m_AccountEmailAddressCtrl->Create( itemWizardPage56, ID_ACCOUNTEMAILADDRESSCTRL, _T(""), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer64->Add(m_AccountEmailAddressCtrl, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_AccountPasswordStaticCtrl = new wxStaticText;
    m_AccountPasswordStaticCtrl->Create( itemWizardPage56, ID_ACCOUNTPASSWORDSTATICCTRL, _("&Password:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer64->Add(m_AccountPasswordStaticCtrl, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_AccountPasswordCtrl = new wxTextCtrl;
    m_AccountPasswordCtrl->Create( itemWizardPage56, ID_ACCOUNTPASSWORDCTRL, _T(""), wxDefaultPosition, wxDefaultSize, wxTE_PASSWORD );
    itemFlexGridSizer64->Add(m_AccountPasswordCtrl, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    if (!IS_ACCOUNTMANAGERWIZARD()) {
        m_AccountConfirmPasswordStaticCtrl = new wxStaticText;
        m_AccountConfirmPasswordStaticCtrl->Create( itemWizardPage56, ID_ACCOUNTCONFIRMPASSWORDSTATICCTRL, _("C&onfirm password:"), wxDefaultPosition, wxDefaultSize, 0 );
        itemFlexGridSizer64->Add(m_AccountConfirmPasswordStaticCtrl, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

        m_AccountConfirmPasswordCtrl = new wxTextCtrl;
        m_AccountConfirmPasswordCtrl->Create( itemWizardPage56, ID_ACCOUNTCONFIRMPASSWORDCTRL, _T(""), wxDefaultPosition, wxDefaultSize, wxTE_PASSWORD );
        itemFlexGridSizer64->Add(m_AccountConfirmPasswordCtrl, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);
    }

    // Set validators
    m_AccountEmailAddressCtrl->SetValidator( wxGenericValidator(& m_strAccountEmailAddress) );
    m_AccountPasswordCtrl->SetValidator( wxGenericValidator(& m_strAccountPassword) );
    if (!IS_ACCOUNTMANAGERWIZARD()) {
        m_AccountConfirmPasswordCtrl->SetValidator( wxGenericValidator(& m_strAccountConfirmPassword) );
    }
////@end CAccountInfoPage content construction

}
 
/*!
 * Gets the previous page.
 */
 
wxWizardPageEx* CAccountInfoPage::GetPrev() const
{
    return PAGE_TRANSITION_BACK;
}
 
/*!
 * Gets the next page.
 */
 
wxWizardPageEx* CAccountInfoPage::GetNext() const
{
    if (CHECK_CLOSINGINPROGRESS()) {
        // Cancel Event Detected
        return PAGE_TRANSITION_NEXT(ID_COMPLETIONERRORPAGE);
    } else if (IS_ATTACHTOPROJECTWIZARD()) {
        return PAGE_TRANSITION_NEXT(ID_PROJECTPROCESSINGPAGE);
    } else if (IS_ACCOUNTMANAGERWIZARD()) {
        return PAGE_TRANSITION_NEXT(ID_ACCOUNTMANAGERPROCESSINGPAGE);
    }
    return NULL;
}
 
/*!
 * Should we show tooltips?
 */
 
bool CAccountInfoPage::ShowToolTips()
{
    return TRUE;
}
 
/*!
 * Get bitmap resources
 */
 
wxBitmap CAccountInfoPage::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CAccountInfoPage bitmap retrieval
    return wxNullBitmap;
////@end CAccountInfoPage bitmap retrieval
}
 
/*!
 * Get icon resources
 */
 
wxIcon CAccountInfoPage::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CAccountInfoPage icon retrieval
    return wxNullIcon;
////@end CAccountInfoPage icon retrieval
}
  
/*!
 * wxEVT_WIZARD_PAGE_CHANGED event handler for ID_ACCOUNTINFOPAGE
 */
 
void CAccountInfoPage::OnPageChanged( wxWizardExEvent& event ) 
{
    if (event.GetDirection() == false) return;

    static bool bRunOnce = true;
    if (bRunOnce) {
        bRunOnce = false;
        if (!IS_ACCOUNTMANAGERWIZARD()) {
            m_AccountCreateCtrl->SetValue(TRUE);
            m_AccountUseExistingCtrl->SetValue(FALSE);
        }
    }

    if (((CBOINCBaseWizard*)GetParent())->project_config.client_account_creation_disabled) {
        if (!IS_ACCOUNTMANAGERWIZARD()) {
            m_AccountCreateCtrl->SetValue(false);
            m_AccountUseExistingCtrl->SetValue(true);
            m_AccountConfirmPasswordStaticCtrl->Hide();
            m_AccountConfirmPasswordCtrl->Hide();

            m_AccountCreateCtrl->Disable();
        }
    }

    if (((CBOINCBaseWizard*)GetParent())->project_config.uses_username) {
        m_AccountEmailAddressStaticCtrl->SetLabel(
            _("Username:")
        );
    } else {
        m_AccountEmailAddressStaticCtrl->SetLabel(
            _("Email address:")
        );
    }
 
    Fit();
}
  
/*!
 * wxEVT_WIZARD_PAGE_CHANGING event handler for ID_ACCOUNTINFOPAGE
 */
 
void CAccountInfoPage::OnPageChanging( wxWizardExEvent& event )
{
    if (event.GetDirection() == false) return;
 
    if (!CHECK_CLOSINGINPROGRESS()) {
        wxString strTitle = wxT("");
        if (IS_ATTACHTOPROJECTWIZARD()) {
            strTitle = _("Attach to project");
        }
        if (IS_ACCOUNTMANAGERWIZARD()) {
            strTitle = _("Attach to account manager");
        }
        wxString strMessage = wxT("");
        bool     bDisplayError = false;
 
        // Verify minimum password length
        unsigned int iMinLength = ((CBOINCBaseWizard*)GetParent())->project_config.min_passwd_length;
        wxString strPassword = m_AccountPasswordCtrl->GetValue();
        if (strPassword.Length() < iMinLength) {
            if (IS_ATTACHTOPROJECTWIZARD()) {
                strMessage.Printf(
                    _("The minimum password length for this project is %d. Please choose a different password."),
                    iMinLength
                );
            }
            if (IS_ACCOUNTMANAGERWIZARD()) {
                strMessage.Printf(
                    _("The minimum password length for this account manager is %d. Please choose a different password."),
                    iMinLength
                );
            }

            bDisplayError = true;
        }

        if (!IS_ACCOUNTMANAGERWIZARD() && m_AccountCreateCtrl->GetValue()) {
            // Verify that the password and confirmation password math.
            if (m_AccountPasswordCtrl->GetValue() != m_AccountConfirmPasswordCtrl->GetValue()) {
                strMessage = _("The password and confirmation password do not match. Please type them again.");
                bDisplayError = true;
            }
        }
 
        if (bDisplayError) {
            ::wxMessageBox(
                strMessage,
                strTitle,
                wxICON_ERROR | wxOK,
                this
            );
            event.Veto();
        }
    }
}
  
/*!
 * wxEVT_WIZARD_CANCEL event handler for ID_ACCOUNTINFOPAGE
 */
 
void CAccountInfoPage::OnCancel( wxWizardExEvent& event ) {
    PROCESS_CANCELEVENT(event);
}
 
/*!
 * wxEVT_COMMAND_RADIOBUTTON_SELECTED event handler for ID_ACCOUNTUSEXISTINGBUTTON
 */
 
void CAccountInfoPage::OnAccountUseExistingCtrlSelected( wxCommandEvent& event ) {
    m_AccountConfirmPasswordStaticCtrl->Hide();
    m_AccountConfirmPasswordCtrl->Hide();
    Fit();
}
  
/*!
 * wxEVT_COMMAND_RADIOBUTTON_SELECTED event handler for ID_ACCOUNTCREATEBUTTON
 */
 
void CAccountInfoPage::OnAccountCreateCtrlSelected( wxCommandEvent& event ) {
    m_AccountConfirmPasswordStaticCtrl->Show();
    m_AccountConfirmPasswordCtrl->Show();
    Fit();
}

