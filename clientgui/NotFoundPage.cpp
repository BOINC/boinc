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
#include "util.h"
#include "mfile.h"
#include "miofile.h"
#include "parse.h"
#include "error_numbers.h"
#include "error_numbers.h"
#include "BOINCGUIApp.h"
#include "SkinManager.h"
#include "MainDocument.h"
#include "BOINCBaseWizard.h"
#include "WizardAttach.h"
#include "AccountInfoPage.h"
#include "NotFoundPage.h"

IMPLEMENT_DYNAMIC_CLASS(CErrNotFoundPage, CBOINCWizardPage)

BEGIN_EVENT_TABLE(CErrNotFoundPage, CBOINCWizardPage)

EVT_WIZARD_PAGE_CHANGED(wxID_ANY, CErrNotFoundPage::OnPageChanged)
EVT_WIZARD_PAGE_CHANGING(wxID_ANY, CErrNotFoundPage::OnPageChanging)
EVT_WIZARD_CANCEL(wxID_ANY, CErrNotFoundPage::OnCancel)

END_EVENT_TABLE()

CErrNotFoundPage::CErrNotFoundPage() {
}

CErrNotFoundPage::CErrNotFoundPage(CWizardAttach* parent) {
    Create(parent);
}

bool CErrNotFoundPage::Create(CWizardAttach* parent) {
    m_pParent = parent;
    m_pPrev = nullptr;
    m_pTitleStaticCtrl = nullptr;
    m_pDirectionsStaticCtrl = nullptr;

    wxWizardPage::Create(parent);

    CreateControls();
    GetSizer()->Fit(this);

    return true;
}

void CErrNotFoundPage::CreateControls() {
    CErrNotFoundPage* itemWizardPage96 = this;

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
}

wxWizardPage* CErrNotFoundPage::GetPrev() const {
    return m_pPrev;
}

wxWizardPage* CErrNotFoundPage::GetNext() const {
    return m_pParent->GetAccountInfoPage();
}

void CErrNotFoundPage::SetPrev(CBOINCWizardPage *prev) {
    m_pPrev = prev;
}

bool CErrNotFoundPage::HasNextPage() const {
    return true;
}

bool CErrNotFoundPage::HasPrevPage() const {
    return m_pPrev != nullptr;
}

void CErrNotFoundPage::OnPageChanged(wxWizardEvent& event) {
    if (event.GetDirection() == false) return;

    wxASSERT(m_pTitleStaticCtrl);
    wxASSERT(m_pDirectionsStaticCtrl);

    m_pTitleStaticCtrl->SetLabel(
        _("Login Failed.")
    );
    if (m_pParent->GetProjectConfig().uses_username) {
        m_pDirectionsStaticCtrl->SetLabel(
            _("Check the username and password, and try again.")
        );
    } else {
        m_pDirectionsStaticCtrl->SetLabel(
            _("Check the email address and password, and try again.")
        );
    }

    m_pParent->DisableNextButton();
    m_pParent->GetBackButton()->SetDefault();

    Fit();
}

void CErrNotFoundPage::OnCancel(wxWizardEvent& event) {
    m_pParent->ProcessCancelEvent(event);
}

void CErrNotFoundPage::OnPageChanging(wxWizardEvent& event) {
    if (event.GetDirection() == false) return;

    CBOINCWizardPage *pageNext = dynamic_cast<CBOINCWizardPage*>(GetNext());
    if (pageNext != nullptr) {
        pageNext->SetPrev(const_cast<CErrNotFoundPage*>(this));
    }
}
