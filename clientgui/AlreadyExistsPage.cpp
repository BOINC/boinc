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
#include "AlreadyExistsPage.h"


IMPLEMENT_DYNAMIC_CLASS(CErrAlreadyExistsPage, CBOINCWizardPage)

BEGIN_EVENT_TABLE(CErrAlreadyExistsPage, CBOINCWizardPage)

EVT_WIZARD_PAGE_CHANGED(wxID_ANY, CErrAlreadyExistsPage::OnPageChanged)
EVT_WIZARD_CANCEL(wxID_ANY, CErrAlreadyExistsPage::OnCancel)

END_EVENT_TABLE()

CErrAlreadyExistsPage::CErrAlreadyExistsPage() {
}

CErrAlreadyExistsPage::CErrAlreadyExistsPage(CWizardAttach* parent) {
    Create(parent);
}

bool CErrAlreadyExistsPage::Create(CWizardAttach* parent) {
    m_pParent = parent;
    m_pPrev = nullptr;
    m_pTitleStaticCtrl = nullptr;
    m_pDirectionsStaticCtrl = nullptr;

    wxWizardPage::Create(parent);

    CreateControls();
    GetSizer()->Fit(this);

    return true;
}

void CErrAlreadyExistsPage::CreateControls() {
    CErrAlreadyExistsPage* itemWizardPage96 = this;

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

wxWizardPage* CErrAlreadyExistsPage::GetPrev() const {
    return m_pPrev;
}

wxWizardPage* CErrAlreadyExistsPage::GetNext() const {
    return nullptr;
}

void CErrAlreadyExistsPage::SetPrev(CBOINCWizardPage *prev) {
    m_pPrev = prev;
}

bool CErrAlreadyExistsPage::HasNextPage() const {
    return false;
}

bool CErrAlreadyExistsPage::HasPrevPage() const {
    return m_pPrev != nullptr;
}

void CErrAlreadyExistsPage::OnPageChanged(wxWizardEvent& event) {
    if (event.GetDirection() == false) return;

    wxASSERT(m_pTitleStaticCtrl);
    wxASSERT(m_pDirectionsStaticCtrl);
    wxASSERT(m_pParent);

    if (m_pParent->GetProjectConfig().uses_username) {
        m_pTitleStaticCtrl->SetLabel(
            _("Username already in use")
        );
        m_pDirectionsStaticCtrl->SetLabel(
            _("An account with that username already exists and has a\ndifferent password than the one you entered.\n\nPlease visit the project's web site and follow the instructions there.")
        );
    } else {
        m_pTitleStaticCtrl->SetLabel(
            _("Email address already in use")
        );
        m_pDirectionsStaticCtrl->SetLabel(
            _("An account with that email address already exists and has a\ndifferent password than the one you entered.\n\nPlease visit the project's web site and follow the instructions there.")
        );
    }

    Fit();
}

void CErrAlreadyExistsPage::OnCancel(wxWizardEvent& event) {
    m_pParent->ProcessCancelEvent(event);
}
