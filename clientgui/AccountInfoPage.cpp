// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2008 University of California
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

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma implementation "AccountInfoPage.h"
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
#include "wx/valgen.h"
#include "wx/valtext.h"
#include "ValidateEmailAddress.h"
#include "BOINCWizards.h"
#include "BOINCBaseWizard.h"
#include "WizardAttachProject.h"
#include "ProjectInfoPage.h"
#include "AccountManagerInfoPage.h"
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
    m_pTitleStaticCtrl = NULL;
    m_pCookieDetectionFailedStaticCtrl = NULL;
    m_pCookieDetectionFailedCtrl = NULL;
    m_pAccountQuestionStaticCtrl = NULL;
    m_pAccountInformationStaticCtrl = NULL;
    m_pAccountCreateCtrl = NULL;
    m_pAccountUseExistingCtrl = NULL;
    m_pAccountEmailAddressStaticCtrl = NULL;
    m_pAccountEmailAddressCtrl = NULL;
    m_pAccountPasswordStaticCtrl = NULL;
    m_pAccountPasswordCtrl = NULL;
    m_pAccountConfirmPasswordStaticCtrl = NULL;
    m_pAccountConfirmPasswordCtrl = NULL;
    m_pAccountPasswordRequirmentsStaticCtrl = NULL;
    m_pAccountManagerLinkLabelStaticCtrl = NULL;
    m_pAccountForgotPasswordCtrl = NULL;
////@end CAccountInfoPage member initialisation
 
////@begin CAccountInfoPage creation
    wxWizardPageEx::Create( parent, ID_ACCOUNTINFOPAGE );

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

    m_pTitleStaticCtrl = new wxStaticText;
    m_pTitleStaticCtrl->Create( itemWizardPage56, wxID_STATIC, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    m_pTitleStaticCtrl->SetFont(wxFont(10, wxSWISS, wxNORMAL, wxBOLD, FALSE, _T("Verdana")));
    itemBoxSizer57->Add(m_pTitleStaticCtrl, 0, wxALIGN_LEFT|wxGROW|wxALL, 5);

    m_pCookieDetectionFailedStaticCtrl = new wxStaticText;
    m_pCookieDetectionFailedStaticCtrl->Create( itemWizardPage56, wxID_STATIC, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    m_pCookieDetectionFailedStaticCtrl->Hide();
    itemBoxSizer57->Add(m_pCookieDetectionFailedStaticCtrl, 0, wxALIGN_LEFT|wxALL, 5);

    m_pCookieDetectionFailedCtrl = new wxHyperlinkCtrl;
    m_pCookieDetectionFailedCtrl->Create( itemWizardPage56, ID_ACCOUNTCOOKIEDETECTIONFAILEDCTRL, wxT(" "), wxT(" "), wxDefaultPosition, wxDefaultSize, wxNO_BORDER | wxHL_ALIGN_LEFT | wxHL_CONTEXTMENU );
    m_pCookieDetectionFailedCtrl->Hide();
    itemBoxSizer57->Add(m_pCookieDetectionFailedCtrl, 0, wxALIGN_LEFT|wxALL, 5);

    m_pAccountQuestionStaticCtrl = new wxStaticText;
    m_pAccountQuestionStaticCtrl->Create( itemWizardPage56, wxID_STATIC, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer57->Add(m_pAccountQuestionStaticCtrl, 0, wxALIGN_LEFT|wxALL, 5);

    wxFlexGridSizer* itemFlexGridSizer61 = new wxFlexGridSizer(1, 2, 0, 0);
    itemFlexGridSizer61->AddGrowableCol(1);
    itemBoxSizer57->Add(itemFlexGridSizer61, 0, wxGROW|wxALL, 5);

    m_pAccountCreateCtrl = new wxRadioButton;
    m_pAccountCreateCtrl->Create( itemWizardPage56, ID_ACCOUNTCREATECTRL, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxRB_GROUP );
    m_pAccountCreateCtrl->SetValue(TRUE);
    itemFlexGridSizer61->Add(m_pAccountCreateCtrl, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_pAccountUseExistingCtrl = new wxRadioButton;
    m_pAccountUseExistingCtrl->Create( itemWizardPage56, ID_ACCOUNTUSEEXISTINGCTRL, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    m_pAccountUseExistingCtrl->SetValue(FALSE);
    itemFlexGridSizer61->Add(m_pAccountUseExistingCtrl, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_pAccountInformationStaticCtrl = new wxStaticText;
    m_pAccountInformationStaticCtrl->Create( itemWizardPage56, wxID_STATIC, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer57->Add(m_pAccountInformationStaticCtrl, 0, wxALIGN_LEFT|wxALL, 5);

    wxFlexGridSizer* itemFlexGridSizer64 = new wxFlexGridSizer(4, 2, 0, 0);
    itemFlexGridSizer64->AddGrowableCol(1);
    itemBoxSizer57->Add(itemFlexGridSizer64, 0, wxEXPAND|wxALL, 0);

    m_pAccountEmailAddressStaticCtrl = new wxStaticText;
    m_pAccountEmailAddressStaticCtrl->Create( itemWizardPage56, ID_ACCOUNTEMAILADDRESSSTATICCTRL, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer64->Add(m_pAccountEmailAddressStaticCtrl, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_pAccountEmailAddressCtrl = new wxTextCtrl;
    m_pAccountEmailAddressCtrl->Create( itemWizardPage56, ID_ACCOUNTEMAILADDRESSCTRL, wxEmptyString, wxDefaultPosition, wxSize(200, 22), 0 );
    itemFlexGridSizer64->Add(m_pAccountEmailAddressCtrl, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_pAccountPasswordStaticCtrl = new wxStaticText;
    m_pAccountPasswordStaticCtrl->Create( itemWizardPage56, ID_ACCOUNTPASSWORDSTATICCTRL, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer64->Add(m_pAccountPasswordStaticCtrl, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_pAccountPasswordCtrl = new wxTextCtrl;
    m_pAccountPasswordCtrl->Create( itemWizardPage56, ID_ACCOUNTPASSWORDCTRL, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_PASSWORD );
    itemFlexGridSizer64->Add(m_pAccountPasswordCtrl, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_pAccountConfirmPasswordStaticCtrl = new wxStaticText;
    m_pAccountConfirmPasswordStaticCtrl->Create( itemWizardPage56, ID_ACCOUNTCONFIRMPASSWORDSTATICCTRL, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer64->Add(m_pAccountConfirmPasswordStaticCtrl, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_pAccountConfirmPasswordCtrl = new wxTextCtrl;
    m_pAccountConfirmPasswordCtrl->Create( itemWizardPage56, ID_ACCOUNTCONFIRMPASSWORDCTRL, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_PASSWORD );
    itemFlexGridSizer64->Add(m_pAccountConfirmPasswordCtrl, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    itemFlexGridSizer64->Add( 0, 0 );

    m_pAccountPasswordRequirmentsStaticCtrl = new wxStaticText;
    m_pAccountPasswordRequirmentsStaticCtrl->Create( itemWizardPage56, ID_ACCOUNTREQUIREMENTSSTATICCTRL, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    m_pAccountPasswordRequirmentsStaticCtrl->SetFont(wxFont(7, wxDEFAULT, wxNORMAL, wxNORMAL, FALSE));
    itemFlexGridSizer64->Add(m_pAccountPasswordRequirmentsStaticCtrl, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_pAccountManagerLinkLabelStaticCtrl = new wxStaticText;
    m_pAccountManagerLinkLabelStaticCtrl->Create( itemWizardPage56, ID_ACCOUNTLINKLABELSTATICCTRL, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer57->Add(m_pAccountManagerLinkLabelStaticCtrl, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_pAccountForgotPasswordCtrl = new wxHyperlinkCtrl;
    m_pAccountForgotPasswordCtrl->Create( itemWizardPage56, ID_ACCOUNTFORGOTPASSWORDCTRL, wxT(" "), wxT(" "), wxDefaultPosition, wxDefaultSize, wxNO_BORDER | wxHL_ALIGN_LEFT | wxHL_CONTEXTMENU );
    itemBoxSizer57->Add(m_pAccountForgotPasswordCtrl, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    // Set validators
    // m_pAccountEmailAddressCtrl is setup when the OnPageChange event is fired since
    //   it can be a username or an email address.
    m_pAccountPasswordCtrl->SetValidator( wxTextValidator(wxFILTER_NONE, &m_strAccountPassword) );
    m_pAccountConfirmPasswordCtrl->SetValidator( wxTextValidator(wxFILTER_NONE, &m_strAccountConfirmPassword) );
    
#ifdef __WXMAC__
    //Accessibility
    HIViewRef buttonView = (HIViewRef)m_pAccountCreateCtrl->GetHandle();
    HIObjectRef   theObject = (HIObjectRef)HIViewGetSuperview(buttonView);
    HIObjectSetAccessibilityIgnored(theObject, true);
#endif
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
 
wxBitmap CAccountInfoPage::GetBitmapResource( const wxString& WXUNUSED(name) )
{
    // Bitmap retrieval
////@begin CAccountInfoPage bitmap retrieval
    return wxNullBitmap;
////@end CAccountInfoPage bitmap retrieval
}
 
/*!
 * Get icon resources
 */
 
wxIcon CAccountInfoPage::GetIconResource( const wxString& WXUNUSED(name) )
{
    // Icon retrieval
////@begin CAccountInfoPage icon retrieval
    return wxNullIcon;
////@end CAccountInfoPage icon retrieval
}
  
/*!
 * wxEVT_WIZARD_PAGE_CHANGED event handler for ID_ACCOUNTINFOPAGE
 */
 
void CAccountInfoPage::OnPageChanged( wxWizardExEvent& event ) {
    if (event.GetDirection() == false) return;

    PROJECT_CONFIG&        pc = ((CBOINCBaseWizard*)GetParent())->project_config;
    CSkinAdvanced*         pSkinAdvanced = wxGetApp().GetSkinManager()->GetAdvanced();
    CSkinWizardATAM*       pSkinWizardATAM = wxGetApp().GetSkinManager()->GetWizards()->GetWizardATAM();
    CWizardAttachProject*  pWAP = ((CWizardAttachProject*)GetParent());
    wxString               strBaseConfigLocation = wxString(wxT("/Wizards"));
    wxConfigBase*          pConfig = wxConfigBase::Get(FALSE);
 
    wxASSERT(pSkinAdvanced);
    wxASSERT(pSkinWizardATAM);
    wxASSERT(pWAP);
    wxASSERT(m_pTitleStaticCtrl);
    wxASSERT(m_pCookieDetectionFailedStaticCtrl);
    wxASSERT(m_pCookieDetectionFailedCtrl);
    wxASSERT(m_pAccountQuestionStaticCtrl);
    wxASSERT(m_pAccountInformationStaticCtrl);
    wxASSERT(m_pAccountCreateCtrl);
    wxASSERT(m_pAccountUseExistingCtrl);
    wxASSERT(m_pAccountEmailAddressStaticCtrl);
    wxASSERT(m_pAccountEmailAddressCtrl);
    wxASSERT(m_pAccountPasswordStaticCtrl);
    wxASSERT(m_pAccountPasswordCtrl);
    wxASSERT(m_pAccountConfirmPasswordStaticCtrl);
    wxASSERT(m_pAccountConfirmPasswordCtrl);
    wxASSERT(m_pAccountPasswordRequirmentsStaticCtrl);
    wxASSERT(m_pAccountManagerLinkLabelStaticCtrl);
    wxASSERT(m_pAccountForgotPasswordCtrl);
    wxASSERT(wxDynamicCast(pSkinAdvanced, CSkinAdvanced));
    wxASSERT(wxDynamicCast(pSkinWizardATAM, CSkinWizardATAM));


    // We are entering this page, so reterieve the previously used email
    // address.
    pConfig->SetPath(strBaseConfigLocation);
    m_strAccountEmailAddress = pConfig->Read(wxT("DefaultEmailAddress"));

    // Setup the desired controls and default values.
    static bool bRunOnce = true;
    if (bRunOnce) {
        bRunOnce = false;
        if (!IS_ACCOUNTMANAGERWIZARD()) {
            m_pAccountCreateCtrl->SetValue(true);
            m_pAccountUseExistingCtrl->SetValue(false);
        }
    }

    if (IS_ACCOUNTMANAGERWIZARD()) {
        if (!(pWAP->m_bCookieRequired && !pWAP->m_bCredentialsDetected)) {
            m_pCookieDetectionFailedStaticCtrl->Hide();
            m_pCookieDetectionFailedCtrl->Hide();
        } else {
            m_pCookieDetectionFailedStaticCtrl->Show();
            m_pCookieDetectionFailedCtrl->Show();
        }

        m_pAccountQuestionStaticCtrl->Hide();
        m_pAccountCreateCtrl->SetValue(false);
        m_pAccountCreateCtrl->Hide();
        m_pAccountUseExistingCtrl->SetValue(true);
        m_pAccountUseExistingCtrl->Hide();
        m_pAccountConfirmPasswordStaticCtrl->Hide();
        m_pAccountConfirmPasswordCtrl->Hide();
        m_pAccountPasswordRequirmentsStaticCtrl->Hide();

        if (pWAP->m_bCookieRequired && !pWAP->m_bCredentialsDetected) {
            m_pAccountManagerLinkLabelStaticCtrl->Hide();
            m_pAccountForgotPasswordCtrl->Hide();
        }
    } else {
        m_pCookieDetectionFailedStaticCtrl->Hide();
        m_pCookieDetectionFailedCtrl->Hide();
        if (pc.account_creation_disabled || pc.client_account_creation_disabled) {
            m_pAccountCreateCtrl->SetValue(false);
            m_pAccountCreateCtrl->Hide();
            m_pAccountUseExistingCtrl->SetValue(true);
            m_pAccountUseExistingCtrl->Hide();
        } else {
            m_pAccountCreateCtrl->Show();
            m_pAccountCreateCtrl->Enable();
            m_pAccountUseExistingCtrl->Show();
        }
        m_pAccountManagerLinkLabelStaticCtrl->Hide();
    }

    m_pTitleStaticCtrl->SetLabel(
        _("Identify your account ")
    );

    if (!IS_ACCOUNTMANAGERWIZARD() && !IS_ACCOUNTMANAGERUPDATEWIZARD()) {
		if (pc.client_account_creation_disabled) {
			m_pAccountQuestionStaticCtrl->SetLabel(
				_("Please enter your account information\n(to create an account, visit the project's web site)")
			);
		} else if (pc.account_creation_disabled) {
			m_pAccountQuestionStaticCtrl->SetLabel(
				_("This project is not currently accepting new accounts.\nYou can attach only if you already have an account.")
			);
		} else {
			m_pAccountQuestionStaticCtrl->SetLabel(
				_("Are you already running this project?")
			);
		}
        m_pAccountCreateCtrl->SetLabel(
            _("&No, new user")
        );
        m_pAccountUseExistingCtrl->SetLabel(
            _("&Yes, existing user")
        );
    } else {
        if (pWAP->m_bCookieRequired && !pWAP->m_bCredentialsDetected) {
            m_pCookieDetectionFailedStaticCtrl->SetLabel(
                _("We were not able to set up your account information\nautomatically.\n\nPlease click on the 'Find logon information' link\nbelow to find out what to put in the email address and\npassword fields.")
            );
            m_pCookieDetectionFailedCtrl->SetLabel(
                _("Find logon information")
            );
            m_pCookieDetectionFailedCtrl->SetURL(
                pWAP->m_strCookieFailureURL
            );
        }

        if (pSkinAdvanced->IsBranded() && 
            !pSkinWizardATAM->GetAccountInfoMessage().IsEmpty()) {
            m_pAccountInformationStaticCtrl->SetLabel(
                pSkinWizardATAM->GetAccountInfoMessage()
            );
        }
    }

    if (m_pAccountUseExistingCtrl->GetValue()) {
        m_pAccountConfirmPasswordStaticCtrl->Hide();
        m_pAccountConfirmPasswordCtrl->Hide();
        m_pAccountPasswordRequirmentsStaticCtrl->Hide();
        m_pAccountPasswordStaticCtrl->SetLabel(
            _("&Password:")
        );
    } else {
        m_pAccountConfirmPasswordStaticCtrl->Show();
        m_pAccountConfirmPasswordCtrl->Show();
        m_pAccountPasswordRequirmentsStaticCtrl->Show();
        m_pAccountPasswordStaticCtrl->SetLabel(
            _("Choose a &password:")
        );
        m_pAccountConfirmPasswordStaticCtrl->SetLabel(
            _("C&onfirm password:")
        );
    }

    if (!((CBOINCBaseWizard*)GetParent())->project_name.IsEmpty()) {
        wxString strQuestion;
        strQuestion.Printf(
            _("Are you already running %s?"),
            ((CBOINCBaseWizard*)GetParent())->project_name.c_str()
        );
        m_pAccountQuestionStaticCtrl->SetLabel(strQuestion);
    }

    if (((CBOINCBaseWizard*)GetParent())->project_config.uses_username) {
        if (IS_ACCOUNTMANAGERWIZARD()) {
            if (pSkinAdvanced->IsBranded() && 
                !pSkinWizardATAM->GetAccountInfoMessage().IsEmpty()) {
                m_pAccountInformationStaticCtrl->SetLabel(
                    pSkinWizardATAM->GetAccountInfoMessage()
                );
            }
        }

        m_pAccountEmailAddressStaticCtrl->SetLabel(
            _("&Username:")
        );
        m_pAccountEmailAddressCtrl->SetValidator( wxTextValidator(wxFILTER_ASCII, &m_strAccountEmailAddress) );
    } else {
        if (IS_ACCOUNTMANAGERWIZARD()) {
            if (pSkinAdvanced->IsBranded() && 
                !pSkinWizardATAM->GetAccountInfoMessage().IsEmpty()) {
                m_pAccountInformationStaticCtrl->SetLabel(
                    pSkinWizardATAM->GetAccountInfoMessage()
                );
            }
        }

        m_pAccountEmailAddressStaticCtrl->SetLabel(
            _("&Email address:")
        );
        m_pAccountEmailAddressCtrl->SetValidator( CValidateEmailAddress(& m_strAccountEmailAddress) );
        m_pAccountEmailAddressCtrl->SetValue(m_strAccountEmailAddress);
    }

    if (pc.min_passwd_length) {
        wxString str;
        str.Printf(_("minimum length %d"), pc.min_passwd_length);
        m_pAccountPasswordRequirmentsStaticCtrl->SetLabel( str );
    }


    if (!IS_ACCOUNTMANAGERWIZARD()) {
        m_pAccountForgotPasswordCtrl->SetLabel(
            _("Forgot your password?")
        );
        m_pAccountForgotPasswordCtrl->SetURL(
            wxString(pWAP->m_ProjectInfoPage->GetProjectURL() + _T("get_passwd.php"))
        );
    } else {
        m_pAccountManagerLinkLabelStaticCtrl->SetLabel(
            _("If you have not yet registered with this account manager,\nplease do so before proceeding.  Click on the link below\nto register or to retrieve a forgotten password." )
        );
        m_pAccountForgotPasswordCtrl->SetLabel(
            _("Account manager website")
        );
        m_pAccountForgotPasswordCtrl->SetURL(
            wxString(pWAP->m_AccountManagerInfoPage->GetProjectURL())
        );
    }

    Fit();
    m_pAccountEmailAddressCtrl->SetFocus();
}
  
/*!
 * wxEVT_WIZARD_PAGE_CHANGING event handler for ID_ACCOUNTINFOPAGE
 */
 
void CAccountInfoPage::OnPageChanging( wxWizardExEvent& event ) {
    if (event.GetDirection() == false) return;

    wxString               strTitle;
    wxString               strMessage = wxT("");
    bool                   bDisplayError = false;
    wxString               strBaseConfigLocation = wxString(wxT("/Wizards"));
    wxConfigBase*          pConfig = wxConfigBase::Get(FALSE);

    if (!CHECK_CLOSINGINPROGRESS()) {
        // We are leaving this page, so store the email address for future
        // use.
        pConfig->SetPath(strBaseConfigLocation);
        pConfig->Write(wxT("DefaultEmailAddress"), m_strAccountEmailAddress);

        // Construct potiental dialog title
        if (IS_ATTACHTOPROJECTWIZARD()) {
            strTitle = _("Attach to project");
        } else if (IS_ACCOUNTMANAGERWIZARD() && IS_ACCOUNTMANAGERUPDATEWIZARD()) {
            strTitle = _("Update account manager");
        } else if (IS_ACCOUNTMANAGERWIZARD()) {
            strTitle = _("Attach to account manager");
        }
 
        // Verify minimum password length
        unsigned int iMinLength = ((CBOINCBaseWizard*)GetParent())->project_config.min_passwd_length;
        wxString strPassword = m_pAccountPasswordCtrl->GetValue();
        if (strPassword.Length() < iMinLength) {
            if (IS_ATTACHTOPROJECTWIZARD()) {
                strMessage.Printf(
                    _("The minimum password length for this project is %d. Please enter a different password."),
                    iMinLength
                );
            }
            if (IS_ACCOUNTMANAGERWIZARD()) {
                strMessage.Printf(
                    _("The minimum password length for this account manager is %d. Please enter a different password."),
                    iMinLength
                );
            }

            bDisplayError = true;
        }

        if (!IS_ACCOUNTMANAGERWIZARD() && m_pAccountCreateCtrl->GetValue()) {
            // Verify that the password and confirmation password math.
            if (m_pAccountPasswordCtrl->GetValue() != m_pAccountConfirmPasswordCtrl->GetValue()) {
                strMessage = _("The password and confirmation password do not match. Please type them again.");
                bDisplayError = true;
            }
        }
 
        if (bDisplayError) {
            wxGetApp().SafeMessageBox(
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
 
void CAccountInfoPage::OnAccountUseExistingCtrlSelected( wxCommandEvent& WXUNUSED(event) ) {
    m_pAccountPasswordStaticCtrl->SetLabel(
        _("&Password:")
    );
    m_pAccountConfirmPasswordStaticCtrl->Hide();
    m_pAccountConfirmPasswordCtrl->Hide();
    m_pAccountPasswordRequirmentsStaticCtrl->Hide();
    m_pAccountEmailAddressCtrl->SetFocus();
    Fit();
}
  
/*!
 * wxEVT_COMMAND_RADIOBUTTON_SELECTED event handler for ID_ACCOUNTCREATEBUTTON
 */
 
void CAccountInfoPage::OnAccountCreateCtrlSelected( wxCommandEvent& WXUNUSED(event) ) {
    m_pAccountPasswordStaticCtrl->SetLabel(
        _("Choose a &password:")
    );
    m_pAccountConfirmPasswordStaticCtrl->Show();
    m_pAccountConfirmPasswordCtrl->Show();
    m_pAccountPasswordRequirmentsStaticCtrl->Show();
    m_pAccountEmailAddressCtrl->SetFocus();
    Fit();
}

