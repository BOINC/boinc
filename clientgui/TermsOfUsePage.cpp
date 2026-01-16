// This file is part of BOINC.
// https://boinc.berkeley.edu
// Copyright (C) 2025 University of California
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
// along with BOINC.  If not, see <https://www.gnu.org/licenses/>.
//
#if defined(__GNUG__) && !defined(__APPLE__)
#pragma implementation "TermsOfUsePage.h"
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
#include "BOINCBaseWizard.h"
#include "WizardAttach.h"
#include "TermsOfUsePage.h"


/*!
 * CTermsOfUsePage type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CTermsOfUsePage, wxWizardPageEx )

/*!
 * CTermsOfUsePage event table definition
 */

BEGIN_EVENT_TABLE( CTermsOfUsePage, wxWizardPageEx )

////@begin CTermsOfUsePage event table entries
    EVT_WIZARDEX_PAGE_CHANGED( -1, CTermsOfUsePage::OnPageChanged )
    EVT_WIZARDEX_PAGE_CHANGING( -1, CTermsOfUsePage::OnPageChanging )
    EVT_WIZARDEX_CANCEL( -1, CTermsOfUsePage::OnCancel )
    EVT_RADIOBUTTON( ID_TERMSOFUSEAGREECTRL, CTermsOfUsePage::OnTermsOfUseStatusChange )
    EVT_RADIOBUTTON( ID_TERMSOFUSEDISAGREECTRL, CTermsOfUsePage::OnTermsOfUseStatusChange )
    EVT_HTML_LINK_CLICKED(ID_TERMSOFUSECTRL, CTermsOfUsePage::OnLinkClicked)
////@end CTermsOfUsePage event table entries

END_EVENT_TABLE()

/*!
 * CTermsOfUsePage constructors
 */

CTermsOfUsePage::CTermsOfUsePage( )
{
}

CTermsOfUsePage::CTermsOfUsePage( CBOINCBaseWizard* parent )
{
    Create( parent );
}

/*!
 * CTermsOfUsePage creator
 */

bool CTermsOfUsePage::Create( CBOINCBaseWizard* parent )
{
////@begin CTermsOfUsePage member initialisation
    m_pTitleStaticCtrl = NULL;
    m_pDirectionsStaticCtrl = NULL;
    m_pTermsOfUseCtrl = NULL;
    m_pAgreeCtrl = NULL;
    m_pDisagreeCtrl = NULL;
////@end CTermsOfUsePage member initialisation
    m_bUserAgrees = false;
    m_bCredentialsAlreadyAvailable = false;

////@begin CTermsOfUsePage creation
    wxWizardPageEx::Create( parent, ID_TERMSOFUSEPAGE );

    CreateControls();
    GetSizer()->Fit(this);
////@end CTermsOfUsePage creation

    return TRUE;
}

/*!
 * Control creation for CTermsOfUsePage
 */

void CTermsOfUsePage::CreateControls()
{
////@begin CTermsOfUsePage content construction
    CTermsOfUsePage* itemWizardPage96 = this;

    wxBoxSizer* itemBoxSizer97 = new wxBoxSizer(wxVERTICAL);
    itemWizardPage96->SetSizer(itemBoxSizer97);

    m_pTitleStaticCtrl = new wxStaticText;
    m_pTitleStaticCtrl->Create( itemWizardPage96, wxID_STATIC, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    m_pTitleStaticCtrl->SetFont(wxFont(10, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD, FALSE, _T("Verdana")));
    itemBoxSizer97->Add(m_pTitleStaticCtrl, 0, wxALIGN_LEFT|wxALL, 5);

    itemBoxSizer97->Add(5, 5, 0, wxALIGN_LEFT|wxALL, 5);

    m_pDirectionsStaticCtrl = new wxStaticText;
    m_pDirectionsStaticCtrl->Create( itemWizardPage96, wxID_STATIC, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer97->Add(m_pDirectionsStaticCtrl, 0, wxALIGN_LEFT|wxALL, 5);

    m_pTermsOfUseCtrl = new wxHtmlWindow;
    m_pTermsOfUseCtrl->Create( itemWizardPage96, ID_TERMSOFUSECTRL, wxDefaultPosition, wxSize(580, 250), wxHW_SCROLLBAR_AUTO, wxEmptyString);
    itemBoxSizer97->Add(m_pTermsOfUseCtrl, 0, wxGROW|wxALL, 5);

    m_pAgreeCtrl = new wxRadioButton;
    m_pAgreeCtrl->Create( itemWizardPage96, ID_TERMSOFUSEAGREECTRL, _("I agree to the terms of use."), wxDefaultPosition, wxDefaultSize, 0 );
    m_pAgreeCtrl->SetValue(false);
    itemBoxSizer97->Add(m_pAgreeCtrl, 0, wxALIGN_LEFT|wxALL, 5);

    m_pDisagreeCtrl = new wxRadioButton;
    m_pDisagreeCtrl->Create( itemWizardPage96, ID_TERMSOFUSEDISAGREECTRL, _("I do not agree to the terms of use."), wxDefaultPosition, wxDefaultSize, 0 );
    m_pDisagreeCtrl->SetValue(true);
    itemBoxSizer97->Add(m_pDisagreeCtrl, 0, wxALIGN_LEFT|wxALL, 5);

////@end CTermsOfUsePage content construction
}


void CTermsOfUsePage::OnLinkClicked( wxHtmlLinkEvent& event ) {
    wxString url = event.GetLinkInfo().GetHref();
    if (url.StartsWith(wxT("http://")) || url.StartsWith(wxT("https://"))) {
        // wxHtmlLinkEvent doesn't have Veto(), but only loads the page if you
          // call Skip().
            wxLaunchDefaultBrowser(url);
    } else {
        event.Skip();
    }
 }

/*!
 * Gets the previous page.
 */

wxWizardPageEx* CTermsOfUsePage::GetPrev() const
{
    return PAGE_TRANSITION_BACK;
}

/*!
 * Gets the next page.
 */

wxWizardPageEx* CTermsOfUsePage::GetNext() const
{
    if (CHECK_CLOSINGINPROGRESS()) {
        // Cancel Event Detected
        return PAGE_TRANSITION_NEXT(ID_COMPLETIONERRORPAGE);
    } else if (IS_ATTACHTOPROJECTWIZARD() && GetUserAgrees() && GetCredentialsAlreadyAvailable()) {
        return PAGE_TRANSITION_NEXT(ID_PROJECTPROCESSINGPAGE);
    } else if (IS_ACCOUNTMANAGERWIZARD() && GetUserAgrees() && GetCredentialsAlreadyAvailable()) {
        return PAGE_TRANSITION_NEXT(ID_ACCOUNTMANAGERPROCESSINGPAGE);
    } else if (GetUserAgrees()) {
        return PAGE_TRANSITION_NEXT(ID_ACCOUNTINFOPAGE);
    } else {
        return PAGE_TRANSITION_NEXT(ID_COMPLETIONERRORPAGE);
    }
}

/*!
 * Should we show tooltips?
 */

bool CTermsOfUsePage::ShowToolTips()
{
    return TRUE;
}

/*!
 * Get bitmap resources
 */

wxBitmap CTermsOfUsePage::GetBitmapResource( const wxString& WXUNUSED(name) )
{
    // Bitmap retrieval

////@begin CTermsOfUsePage bitmap retrieval
    return wxNullBitmap;
////@end CTermsOfUsePage bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CTermsOfUsePage::GetIconResource( const wxString& WXUNUSED(name) )
{
    // Icon retrieval

////@begin CTermsOfUsePage icon retrieval
    return wxNullIcon;
////@end CTermsOfUsePage icon retrieval
}

/*!
 * wxEVT_WIZARD_PAGE_CHANGED event handler for ID_TERMSOFUSEPAGE
 */

void CTermsOfUsePage::OnPageChanged( wxWizardExEvent& event ) {
    if (event.GetDirection() == false) return;

    PROJECT_CONFIG& pc = ((CWizardAttach*)GetParent())->project_config;

    wxASSERT(m_pTitleStaticCtrl);
    wxASSERT(m_pDirectionsStaticCtrl);

    m_pTitleStaticCtrl->SetLabel(
        _("Terms of Use")
    );

    m_pDirectionsStaticCtrl->SetLabel(
        _("Please read the following terms of use:")
    );

    wxString terms_of_use(pc.terms_of_use.c_str(), wxConvUTF8);
    // We need to replace all line endings in text TOU
    // to make it looks properly in HTML Window
    if (!pc.terms_of_use_is_html) {
        terms_of_use.Replace("\r\n", "<br>");
        terms_of_use.Replace("\r", "<br>");
        terms_of_use.Replace("\n", "<br>");
    }
    m_pTermsOfUseCtrl->SetPage(terms_of_use);

    m_pAgreeCtrl->SetValue(false);

    m_pDisagreeCtrl->SetValue(true);

    SetUserAgrees(false);
    ((CWizardAttach*)GetParent())->DisableNextButton();

    Fit();
}

/*!
 * wxEVT_WIZARD_PAGE_CHANGING event handler for ID_TERMSOFUSEPAGE
 */

void CTermsOfUsePage::OnPageChanging( wxWizardExEvent& event ) {
    CWizardAttach*  pWA = ((CWizardAttach*)GetParent());

    wxASSERT(pWA);
    wxASSERT(wxDynamicCast(pWA, CWizardAttach));

    // If the user has left the terms of use disagree radio button
    // selected, then the next button is disabled and needs to be
    // re-enabled if the back button is pressed.
    pWA->EnableNextButton();

    if (event.GetDirection() == false) {
        pWA->SetConsentedToTerms(false);
        return;
    }

    if (!CHECK_CLOSINGINPROGRESS()) {
        // We are leaving this page.

        // Determine if the account settings are already pre-populated.
        //   If so, advance to the Account Manager Processing page or the
        //   Project Processing page.
        if ( pWA->IsCredentialsCached() || pWA->IsCredentialsDetected()) {
            SetCredentialsAlreadyAvailable(true);
        } else {
            SetCredentialsAlreadyAvailable(false);
        }
        pWA->SetConsentedToTerms(GetUserAgrees());
    }
}

/*!
 * wxEVT_WIZARD_CANCEL event handler for ID_TERMSOFUSEPAGE
 */

void CTermsOfUsePage::OnCancel( wxWizardExEvent& event ) {
    PROCESS_CANCELEVENT(event);
}

/*!
 * wxEVT_COMMAND_RADIOBUTTON_SELECTED event handler for ID_TERMSOFUSEAGREECTRL
 *   or ID_TERMSOFUSEDISAGREECTRL
 */

void CTermsOfUsePage::OnTermsOfUseStatusChange( wxCommandEvent& event ) {
    wxLogTrace(wxT("Function Start/End"), wxT("CTermsOfUsePage::OnTermsOfUseStatusChange - Function Begin"));

    if ((ID_TERMSOFUSEAGREECTRL == event.GetId()) && event.IsChecked()){
        wxLogTrace(wxT("Function Status"), wxT("CTermsOfUsePage::OnTermsOfUseStatusChange - SetUserAgrees(true)"));
        SetUserAgrees(true);
        ((CWizardAttach*)GetParent())->EnableNextButton();
    } else {
        wxLogTrace(wxT("Function Status"), wxT("CTermsOfUsePage::OnTermsOfUseStatusChange - SetUserAgrees(false)"));
        SetUserAgrees(false);
        ((CWizardAttach*)GetParent())->DisableNextButton();
    }

    wxLogTrace(wxT("Function Start/End"), wxT("CTermsOfUsePage::OnTermsOfUseStatusChange - Function End"));
}
