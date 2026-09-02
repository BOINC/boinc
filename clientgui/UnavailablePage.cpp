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
#include "UnavailablePage.h"

IMPLEMENT_DYNAMIC_CLASS(CErrUnavailablePage, CBOINCWizardPage)

BEGIN_EVENT_TABLE(CErrUnavailablePage, CBOINCWizardPage)

EVT_WIZARD_PAGE_CHANGED(wxID_ANY, CErrUnavailablePage::OnPageChanged)
EVT_WIZARD_CANCEL(wxID_ANY, CErrUnavailablePage::OnCancel)

END_EVENT_TABLE()

CErrUnavailablePage::CErrUnavailablePage() {
}

CErrUnavailablePage::CErrUnavailablePage(CWizardAttach* parent) {
    Create(parent);
}

bool CErrUnavailablePage::Create(CWizardAttach* parent) {
    m_pParent = parent;
    m_pPrev = nullptr;
    m_pTitleStaticCtrl = nullptr;
    m_pDirectionsStaticCtrl = nullptr;

    wxWizardPage::Create(parent);

    CreateControls();
    GetSizer()->Fit(this);

    return true;
}

void CErrUnavailablePage::CreateControls() {
    CErrUnavailablePage* itemWizardPage96 = this;

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

wxWizardPage* CErrUnavailablePage::GetPrev() const {
    return m_pPrev;
}

wxWizardPage* CErrUnavailablePage::GetNext() const {
    return nullptr;
}

void CErrUnavailablePage::SetPrev(CBOINCWizardPage *prev) {
    m_pPrev = prev;
}

bool CErrUnavailablePage::HasNextPage() const {
    return false;
}

bool CErrUnavailablePage::HasPrevPage() const {
    return m_pPrev != nullptr;
}

void CErrUnavailablePage::OnPageChanged(wxWizardEvent& event) {
    if (event.GetDirection() == false) return;

    wxASSERT(m_pTitleStaticCtrl);
    wxASSERT(m_pDirectionsStaticCtrl);

    if (m_pParent->GetIsAttachToProjectWizard()) {
        m_pTitleStaticCtrl->SetLabel(
            _("Project temporarily unavailable")
        );
        m_pDirectionsStaticCtrl->SetLabel(
            _("The project is temporarily unavailable.\n\nPlease try again later.")
        );
    } else if (m_pParent->GetIsAccountManagerWizard()) {
        m_pTitleStaticCtrl->SetLabel(
            _("Account manager temporarily unavailable")
        );
        m_pDirectionsStaticCtrl->SetLabel(
            _("The account manager is temporarily unavailable.\n\nPlease try again later.")
        );
    } else {
        wxASSERT(FALSE);
    }

    Layout();
}

void CErrUnavailablePage::OnCancel( wxWizardEvent& event ) {
    m_pParent->ProcessCancelEvent(event);
}
