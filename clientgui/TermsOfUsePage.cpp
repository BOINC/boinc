// This file is part of BOINC.
// https://boinc.berkeley.edu
// Copyright (C) 2026 University of California
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

#include "stdwx.h"
#include "diagnostics.h"
#include "BOINCGUIApp.h"
#include "SkinManager.h"
#include "MainDocument.h"
#include "BOINCBaseWizard.h"
#include "WizardAttach.h"
#include "CompletionErrorPage.h"
#include "AccountInfoPage.h"
#include "AccountManagerProcessingPage.h"
#include "ProjectProcessingPage.h"
#include "TermsOfUsePage.h"

IMPLEMENT_DYNAMIC_CLASS(CTermsOfUsePage, CBOINCWizardPage)

BEGIN_EVENT_TABLE(CTermsOfUsePage, CBOINCWizardPage)

EVT_WIZARD_PAGE_CHANGED(wxID_ANY, CTermsOfUsePage::OnPageChanged)
EVT_WIZARD_PAGE_CHANGING(wxID_ANY, CTermsOfUsePage::OnPageChanging)
EVT_WIZARD_CANCEL(wxID_ANY, CTermsOfUsePage::OnCancel)
EVT_RADIOBUTTON(ID_TERMSOFUSEAGREECTRL, CTermsOfUsePage::OnTermsOfUseStatusChange)
EVT_RADIOBUTTON(ID_TERMSOFUSEDISAGREECTRL, CTermsOfUsePage::OnTermsOfUseStatusChange)
EVT_HTML_LINK_CLICKED(ID_TERMSOFUSECTRL, CTermsOfUsePage::OnLinkClicked)

END_EVENT_TABLE()

CTermsOfUsePage::CTermsOfUsePage() {
}

CTermsOfUsePage::CTermsOfUsePage(CWizardAttach* parent) {
    Create(parent);
}

bool CTermsOfUsePage::Create(CWizardAttach* parent) {
    m_pParent = parent;
    m_pPrev = nullptr;
    m_pTitleStaticCtrl = nullptr;
    m_pDirectionsStaticCtrl = nullptr;
    m_pTermsOfUseCtrl = nullptr;
    m_pAgreeCtrl = nullptr;
    m_pDisagreeCtrl = nullptr;
    m_bUserAgrees = false;
    m_bCredentialsAlreadyAvailable = false;

    wxWizardPage::Create(parent);

    CreateControls();
    GetSizer()->Fit(this);

    return true;
}

void CTermsOfUsePage::CreateControls() {
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
}

void CTermsOfUsePage::OnLinkClicked(wxHtmlLinkEvent& event) {
    wxString url = event.GetLinkInfo().GetHref();
    if (url.StartsWith(wxT("http://")) || url.StartsWith(wxT("https://"))) {
        // wxHtmlLinkEvent doesn't have Veto(), but only loads the page if you
          // call Skip().
            wxLaunchDefaultBrowser(url);
    } else {
        event.Skip();
    }
 }

wxWizardPage* CTermsOfUsePage::GetPrev() const {
    return m_pPrev;
}

wxWizardPage* CTermsOfUsePage::GetNext() const {
    if (m_pParent->IsCancelInProgress()) {
        // Cancel Event Detected
        return m_pParent->GetCompletionErrorPage();
    } else if (m_pParent->GetIsAttachToProjectWizard() && GetUserAgrees() && GetCredentialsAlreadyAvailable()) {
        return m_pParent->GetProjectProcessingPage();
    } else if (m_pParent->GetIsAccountManagerWizard() && GetUserAgrees() && GetCredentialsAlreadyAvailable()) {
        return m_pParent->GetAccountManagerProcessingPage();
    } else if (GetUserAgrees()) {
        return m_pParent->GetAccountInfoPage();
    } else {
        return m_pParent->GetCompletionErrorPage();
    }
}

void CTermsOfUsePage::SetPrev(CBOINCWizardPage *prev) {
    m_pPrev = prev;
}

bool CTermsOfUsePage::HasNextPage() const {
    return true;
}

bool CTermsOfUsePage::HasPrevPage() const {
    return m_pPrev != nullptr;
}

void CTermsOfUsePage::OnPageChanged(wxWizardEvent& event) {
    if (event.GetDirection() == false) return;

    PROJECT_CONFIG& pc = m_pParent->GetProjectConfig();

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
    m_pParent->DisableNextButton();

    Layout();
}

void CTermsOfUsePage::OnPageChanging(wxWizardEvent& event) {
    wxASSERT(m_pParent);
    wxASSERT(wxDynamicCast(m_pParent, CWizardAttach));

    // If the user has left the terms of use disagree radio button
    // selected, then the next button is disabled and needs to be
    // re-enabled if the back button is pressed.
    m_pParent->EnableNextButton();

    if (event.GetDirection() == false) {
        m_pParent->SetConsentedToTerms(false);
        return;
    }

    if (!m_pParent->IsCancelInProgress()) {
        // We are leaving this page.
        // Determine if the account settings are already pre-populated.
        //   If so, advance to the Account Manager Processing page or the
        //   Project Processing page.
        if ( m_pParent->IsCredentialsCached() || m_pParent->IsCredentialsDetected()) {
            SetCredentialsAlreadyAvailable(true);
        } else {
            SetCredentialsAlreadyAvailable(false);
        }
        m_pParent->SetConsentedToTerms(GetUserAgrees());

        CBOINCWizardPage *pageNext = dynamic_cast<CBOINCWizardPage*>(GetNext());
        if (pageNext != nullptr) {
            pageNext->SetPrev(const_cast<CTermsOfUsePage*>(this));
        }

   }
}

void CTermsOfUsePage::OnCancel(wxWizardEvent& event) {
    m_pParent->ProcessCancelEvent(event);
}

void CTermsOfUsePage::OnTermsOfUseStatusChange(wxCommandEvent& event) {
    wxLogTrace(wxT("Function Start/End"), wxT("CTermsOfUsePage::OnTermsOfUseStatusChange - Function Begin"));

    if ((ID_TERMSOFUSEAGREECTRL == event.GetId()) && event.IsChecked()){
        wxLogTrace(wxT("Function Status"), wxT("CTermsOfUsePage::OnTermsOfUseStatusChange - SetUserAgrees(true)"));
        SetUserAgrees(true);
        m_pParent->EnableNextButton();
    } else {
        wxLogTrace(wxT("Function Status"), wxT("CTermsOfUsePage::OnTermsOfUseStatusChange - SetUserAgrees(false)"));
        SetUserAgrees(false);
        m_pParent->DisableNextButton();
    }

    wxLogTrace(wxT("Function Start/End"), wxT("CTermsOfUsePage::OnTermsOfUseStatusChange - Function End"));
}
