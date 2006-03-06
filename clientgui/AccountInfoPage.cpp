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
#include "wx/valtext.h"
#include "ValidateEmailAddress.h"
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
    m_pTitleStaticCtrl = NULL;
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

    m_pTitleStaticCtrl = new wxStaticText;
    m_pTitleStaticCtrl->Create( itemWizardPage56, wxID_STATIC, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    m_pTitleStaticCtrl->SetFont(wxFont(10, wxSWISS, wxNORMAL, wxBOLD, FALSE, _T("Verdana")));
    itemBoxSizer57->Add(m_pTitleStaticCtrl, 0, wxALIGN_LEFT|wxGROW|wxALL, 5);

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
    itemBoxSizer57->Add(itemFlexGridSizer64, 0, wxGROW|wxALL, 0);

    m_pAccountEmailAddressStaticCtrl = new wxStaticText;
    m_pAccountEmailAddressStaticCtrl->Create( itemWizardPage56, ID_ACCOUNTEMAILADDRESSSTATICCTRL, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer64->Add(m_pAccountEmailAddressStaticCtrl, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_pAccountEmailAddressCtrl = new wxTextCtrl;
    m_pAccountEmailAddressCtrl->Create( itemWizardPage56, ID_ACCOUNTEMAILADDRESSCTRL, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer64->Add(m_pAccountEmailAddressCtrl, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_pAccountPasswordStaticCtrl = new wxStaticText;
    m_pAccountPasswordStaticCtrl->Create( itemWizardPage56, ID_ACCOUNTPASSWORDSTATICCTRL, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer64->Add(m_pAccountPasswordStaticCtrl, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_pAccountPasswordCtrl = new wxTextCtrl;
    m_pAccountPasswordCtrl->Create( itemWizardPage56, ID_ACCOUNTPASSWORDCTRL, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_PASSWORD );
    itemFlexGridSizer64->Add(m_pAccountPasswordCtrl, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_pAccountConfirmPasswordStaticCtrl = new wxStaticText;
    m_pAccountConfirmPasswordStaticCtrl->Create( itemWizardPage56, ID_ACCOUNTCONFIRMPASSWORDSTATICCTRL, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer64->Add(m_pAccountConfirmPasswordStaticCtrl, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_pAccountConfirmPasswordCtrl = new wxTextCtrl;
    m_pAccountConfirmPasswordCtrl->Create( itemWizardPage56, ID_ACCOUNTCONFIRMPASSWORDCTRL, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_PASSWORD );
    itemFlexGridSizer64->Add(m_pAccountConfirmPasswordCtrl, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    itemFlexGridSizer64->Add( 0, 0 );

    m_pAccountPasswordRequirmentsStaticCtrl = new wxStaticText;
    m_pAccountPasswordRequirmentsStaticCtrl->Create( itemWizardPage56, ID_ACCOUNTREQUIREMENTSSTATICCTRL, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    m_pAccountPasswordRequirmentsStaticCtrl->SetFont(wxFont(7, wxDEFAULT, wxNORMAL, wxNORMAL, FALSE));
    itemFlexGridSizer64->Add(m_pAccountPasswordRequirmentsStaticCtrl, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    // Set validators
    // m_pAccountEmailAddressCtrl is setup when the OnPageChange event is fired since
    //   it can be a username or an email address.
    m_pAccountPasswordCtrl->SetValidator( wxTextValidator(wxFILTER_ASCII, &m_strAccountPassword) );
    m_pAccountConfirmPasswordCtrl->SetValidator( wxTextValidator(wxFILTER_ASCII, &m_strAccountConfirmPassword) );
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
 
void CAccountInfoPage::OnPageChanged( wxWizardExEvent& event ) {
    if (event.GetDirection() == false) return;

    wxASSERT(m_pTitleStaticCtrl);
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


    static bool bRunOnce = true;
    if (bRunOnce) {
        bRunOnce = false;
        if (!IS_ACCOUNTMANAGERWIZARD()) {
            m_pAccountCreateCtrl->SetValue(TRUE);
            m_pAccountUseExistingCtrl->SetValue(FALSE);
        }
    }

    m_pTitleStaticCtrl->SetLabel(
        _("User information")
    );

    if (!IS_ACCOUNTMANAGERWIZARD() && !IS_ACCOUNTMANAGERUPDATEWIZARD()) {
        m_pAccountQuestionStaticCtrl->SetLabel(
            _("Are you already running this project?")
        );
        m_pAccountCreateCtrl->SetLabel(
            _("&No, new user")
        );
        m_pAccountUseExistingCtrl->SetLabel(
            _("&Yes, existing user")
        );
        if (wxGetApp().GetBrand()->IsBranded() && 
            !wxGetApp().GetBrand()->GetAPWizardAccountInfoText().IsEmpty()) {
            m_pAccountInformationStaticCtrl->SetLabel(
                wxGetApp().GetBrand()->GetAPWizardAccountInfoText()
            );
        }
    } else {
        m_pAccountQuestionStaticCtrl->Hide();
        m_pAccountCreateCtrl->Hide();
        m_pAccountUseExistingCtrl->Hide();
        if (wxGetApp().GetBrand()->IsBranded() && 
            !wxGetApp().GetBrand()->GetAMWizardAccountInfoText().IsEmpty()) {
            m_pAccountInformationStaticCtrl->SetLabel(
                wxGetApp().GetBrand()->GetAMWizardAccountInfoText()
            );
        }
        m_pAccountConfirmPasswordStaticCtrl->Hide();
        m_pAccountConfirmPasswordCtrl->Hide();
    }

    if (m_pAccountUseExistingCtrl->GetValue()) {
        m_pAccountPasswordStaticCtrl->SetLabel(
            _("&Password:")
        );
    } else {
        m_pAccountPasswordStaticCtrl->SetLabel(
            _("Choose a &password:")
        );
    }

    m_pAccountConfirmPasswordStaticCtrl->SetLabel(
        _("C&onfirm password:")
    );

    if (((CBOINCBaseWizard*)GetParent())->project_config.account_creation_disabled) {
        if (!IS_ACCOUNTMANAGERWIZARD()) {
            m_pAccountCreateCtrl->SetValue(false);
            m_pAccountUseExistingCtrl->SetValue(true);
            m_pAccountConfirmPasswordStaticCtrl->Hide();
            m_pAccountConfirmPasswordCtrl->Hide();

            m_pAccountCreateCtrl->Disable();
        }
    }

    if (!((CBOINCBaseWizard*)GetParent())->project_name.IsEmpty()) {
        wxString strQuestion;
        strQuestion.Printf(
            _T("Are you already running %s?"),
            ((CBOINCBaseWizard*)GetParent())->project_name.c_str()
        );
        m_pAccountQuestionStaticCtrl->SetLabel(strQuestion);
    }

    if (((CBOINCBaseWizard*)GetParent())->project_config.uses_username) {
        if (!IS_ACCOUNTMANAGERWIZARD()) {
            if (wxGetApp().GetBrand()->IsBranded() && 
                !wxGetApp().GetBrand()->GetAPWizardAccountInfoText().IsEmpty()) {
                m_pAccountInformationStaticCtrl->SetLabel(
                    wxGetApp().GetBrand()->GetAPWizardAccountInfoText()
                );
            }
        } else {
            if (wxGetApp().GetBrand()->IsBranded() && 
                !wxGetApp().GetBrand()->GetAMWizardAccountInfoText().IsEmpty()) {
                m_pAccountInformationStaticCtrl->SetLabel(
                    wxGetApp().GetBrand()->GetAMWizardAccountInfoText()
                );
            }
        }
        if (m_pAccountInformationStaticCtrl) {
            if (m_pAccountInformationStaticCtrl->GetLabel().IsEmpty()) {
                m_pAccountInformationStaticCtrl->SetLabel(
                    _("Enter the username and password you used on\n"
                    "the web site.")
                );
            }
        }
        m_pAccountEmailAddressStaticCtrl->SetLabel(
            _("&Username:")
        );
        m_pAccountEmailAddressCtrl->SetValidator( wxTextValidator(wxFILTER_ASCII, &m_strAccountEmailAddress) );
    } else {
        if (!IS_ACCOUNTMANAGERWIZARD()) {
            if (wxGetApp().GetBrand()->IsBranded() && 
                !wxGetApp().GetBrand()->GetAPWizardAccountInfoText().IsEmpty()) {
                m_pAccountInformationStaticCtrl->SetLabel(
                    wxGetApp().GetBrand()->GetAPWizardAccountInfoText()
                );
            }
        } else {
            if (wxGetApp().GetBrand()->IsBranded() && 
                !wxGetApp().GetBrand()->GetAMWizardAccountInfoText().IsEmpty()) {
                m_pAccountInformationStaticCtrl->SetLabel(
                    wxGetApp().GetBrand()->GetAMWizardAccountInfoText()
                );
            }
        }
        if (m_pAccountInformationStaticCtrl) {
            if (m_pAccountInformationStaticCtrl->GetLabel().IsEmpty()) {
                m_pAccountInformationStaticCtrl->SetLabel(
                    _("Enter the email address and password you used on\n"
                    "the web site.")
                );
            }
        }
        m_pAccountEmailAddressStaticCtrl->SetLabel(
            _("&Email address:")
        );
        m_pAccountEmailAddressCtrl->SetValidator( CValidateEmailAddress(& m_strAccountEmailAddress) );
    }

    if (((CBOINCBaseWizard*)GetParent())->project_config.min_passwd_length) {
        wxString str;
        str.Printf(_("minimum length %d"), ((CBOINCBaseWizard*)GetParent())->project_config.min_passwd_length);
        m_pAccountPasswordRequirmentsStaticCtrl->SetLabel( str );
    }
 
    Fit();
    m_pAccountEmailAddressCtrl->SetFocus();
}
  
/*!
 * wxEVT_WIZARD_PAGE_CHANGING event handler for ID_ACCOUNTINFOPAGE
 */
 
void CAccountInfoPage::OnPageChanging( wxWizardExEvent& event )
{
    if (event.GetDirection() == false) return;
 
    if (!CHECK_CLOSINGINPROGRESS()) {
        wxString strTitle;
        if (IS_ATTACHTOPROJECTWIZARD()) {
            strTitle = _("Attach to project");
        } else if (IS_ACCOUNTMANAGERWIZARD() && IS_ACCOUNTMANAGERUPDATEWIZARD()) {
            strTitle = _("Update account manager");
        } else if (IS_ACCOUNTMANAGERWIZARD()) {
            strTitle = _("Attach to account manager");
        }
        wxString strMessage = wxT("");
        bool     bDisplayError = false;
 
        // Verify minimum password length
        unsigned int iMinLength = ((CBOINCBaseWizard*)GetParent())->project_config.min_passwd_length;
        wxString strPassword = m_pAccountPasswordCtrl->GetValue();
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

        if (!IS_ACCOUNTMANAGERWIZARD() && m_pAccountCreateCtrl->GetValue()) {
            // Verify that the password and confirmation password math.
            if (m_pAccountPasswordCtrl->GetValue() != m_pAccountConfirmPasswordCtrl->GetValue()) {
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
 
void CAccountInfoPage::OnAccountCreateCtrlSelected( wxCommandEvent& event ) {
    m_pAccountPasswordStaticCtrl->SetLabel(
        _("Choose a &password:")
    );
    m_pAccountConfirmPasswordStaticCtrl->Show();
    m_pAccountConfirmPasswordCtrl->Show();
    m_pAccountPasswordRequirmentsStaticCtrl->Show();
    m_pAccountEmailAddressCtrl->SetFocus();
    Fit();
}

