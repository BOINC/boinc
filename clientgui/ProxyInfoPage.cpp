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
#include "ProxyPage.h"
#include "ProxyInfoPage.h"

IMPLEMENT_DYNAMIC_CLASS(CErrProxyInfoPage, CBOINCWizardPage)

BEGIN_EVENT_TABLE(CErrProxyInfoPage, CBOINCWizardPage)
EVT_WIZARD_PAGE_CHANGED(wxID_ANY, CErrProxyInfoPage::OnPageChanged)
EVT_WIZARD_PAGE_CHANGING(wxID_ANY, CErrProxyInfoPage::OnPageChanging)
EVT_WIZARD_CANCEL(wxID_ANY, CErrProxyInfoPage::OnCancel)

END_EVENT_TABLE()

CErrProxyInfoPage::CErrProxyInfoPage() {
}

CErrProxyInfoPage::CErrProxyInfoPage(CWizardAttach* parent) {
    Create(parent);
}

bool CErrProxyInfoPage::Create(CWizardAttach* parent) {
    m_pParent = parent;
    m_pPrev = nullptr;
    m_pTitleStaticCtrl = nullptr;
    m_pDescriptionStaticCtrl = nullptr;
    m_pDirectionsStaticCtrl = nullptr;

    wxWizardPage::Create(parent);

    CreateControls();
    GetSizer()->Fit(this);

    return true;
}

void CErrProxyInfoPage::CreateControls() {
    CErrProxyInfoPage* itemWizardPage126 = this;

    wxBoxSizer* itemBoxSizer127 = new wxBoxSizer(wxVERTICAL);
    itemWizardPage126->SetSizer(itemBoxSizer127);

    m_pTitleStaticCtrl = new wxStaticText;
    m_pTitleStaticCtrl->Create( itemWizardPage126, wxID_STATIC, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    m_pTitleStaticCtrl->SetFont(wxFont(10, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD, FALSE, _T("Verdana")));
    itemBoxSizer127->Add(m_pTitleStaticCtrl, 0, wxALIGN_LEFT|wxALL, 5);

    itemBoxSizer127->Add(5, 5, 0, wxALIGN_LEFT|wxALL, 5);

    m_pDescriptionStaticCtrl = new wxStaticText;
    m_pDescriptionStaticCtrl->Create( itemWizardPage126, wxID_STATIC, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer127->Add(m_pDescriptionStaticCtrl, 0, wxALIGN_LEFT|wxALL, 5);

    itemBoxSizer127->Add(5, 5, 0, wxALIGN_LEFT|wxALL, 5);

    m_pDirectionsStaticCtrl = new wxStaticText;
    m_pDirectionsStaticCtrl->Create( itemWizardPage126, wxID_STATIC, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer127->Add(m_pDirectionsStaticCtrl, 0, wxALIGN_LEFT|wxALL, 5);
}

wxWizardPage* CErrProxyInfoPage::GetPrev() const {
    return m_pPrev;
}

wxWizardPage* CErrProxyInfoPage::GetNext() const {
    if (m_pParent->IsCancelInProgress()) {
        // Cancel Event Detected
        return m_pParent->GetCompletionErrorPage();
    }
    return m_pParent->GetErrProxyPage();
}

void CErrProxyInfoPage::SetPrev(CBOINCWizardPage *prev) {
    m_pPrev = prev;
}

bool CErrProxyInfoPage::HasNextPage() const {
    return true;
}

bool CErrProxyInfoPage::HasPrevPage() const {
    return m_pPrev != nullptr;
}

void CErrProxyInfoPage::OnPageChanged( wxWizardEvent& event ) {
    if (event.GetDirection() == false) return;

    wxASSERT(m_pTitleStaticCtrl);
    wxASSERT(m_pDescriptionStaticCtrl);
    wxASSERT(m_pDirectionsStaticCtrl);

    m_pTitleStaticCtrl->SetLabel(
        _("Network communication failure")
    );
#if defined (_WCG)
    m_pDescriptionStaticCtrl->SetLabel(
        _("The World Community Grid - BOINC software failed to communicate\nover the Internet. The most likely reasons are:\n\n1) Connectivity problem.  Check your network or modem connection\nand then click Back to try again.\n\n2) Personal firewall software is blocking the World Community\nGrid - BOINC software.  Configure your personal firewall to let\nBOINC and BOINC Manager communicate on port 80 and port 443,\nthen click Back to try again.\n\n3) You are using a proxy server.\nClick Next to configure BOINC's proxy settings.")
    );
#else
    m_pDescriptionStaticCtrl->SetLabel(
        _("BOINC failed to communicate on the Internet.\nThe most likely reasons are:\n\n1) Connectivity problem.  Check your network\nor modem connection and then click Back to try again.\n\n2) Personal firewall software is blocking BOINC.\nConfigure your personal firewall to let BOINC and\nBOINC Manager communicate on port 80,\nthen click Back to try again.\n\n3) You are using a proxy server.\nClick Next to configure BOINC's proxy settings.")
    );
#endif

    Layout();
}

void CErrProxyInfoPage::OnCancel(wxWizardEvent& event) {
    m_pParent->ProcessCancelEvent(event);
}

void CErrProxyInfoPage::OnPageChanging(wxWizardEvent& event) {
    if (event.GetDirection() == false ) return;

    CBOINCWizardPage *pageNext = dynamic_cast<CBOINCWizardPage*>(GetNext());
    if (pageNext != nullptr) {
        pageNext->SetPrev(const_cast<CErrProxyInfoPage*>(this));
    }
}
