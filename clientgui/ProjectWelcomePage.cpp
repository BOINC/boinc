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

// show a "welcome" dialog showing the user what project they're about to run,
// in the case where a project_init.xml was present on startup
//
// AFAIK no one uses this mechanism, so this may not be needed

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
#include "CompletionErrorPage.h"
#include "AccountInfoPage.h"
#include "ProjectPropertiesPage.h"
#include "ProjectWelcomePage.h"

IMPLEMENT_DYNAMIC_CLASS(CProjectWelcomePage, CBOINCWizardPage)

BEGIN_EVENT_TABLE(CProjectWelcomePage, CBOINCWizardPage)

EVT_WIZARD_PAGE_CHANGED(wxID_ANY, CProjectWelcomePage::OnPageChanged)
EVT_WIZARD_PAGE_CHANGING(wxID_ANY, CProjectWelcomePage::OnPageChanging)
EVT_WIZARD_CANCEL(wxID_ANY, CProjectWelcomePage::OnCancel)

END_EVENT_TABLE()

CProjectWelcomePage::CProjectWelcomePage() {
}

CProjectWelcomePage::CProjectWelcomePage(CWizardAttach* parent) {
    Create(parent);
}

bool CProjectWelcomePage::Create(CWizardAttach* parent) {
    m_pParent = parent;
    m_pPrev = nullptr;

    wxWizardPage::Create(parent);

    CreateControls();
    GetSizer()->Fit(this);

	return true;
}

void CProjectWelcomePage::CreateControls() {
    CProjectWelcomePage* itemWizardPage2 = this;

    wxBoxSizer* itemBoxSizer3 = new wxBoxSizer(wxVERTICAL);
    itemWizardPage2->SetSizer(itemBoxSizer3);

    title_ctrl = new wxStaticText;
    title_ctrl->Create( itemWizardPage2, wxID_STATIC, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    title_ctrl->SetFont(wxFont(12, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD, FALSE, _T("Verdana")));
    itemBoxSizer3->Add(title_ctrl, 0, wxALIGN_LEFT|wxALL, 5);

    intro_ctrl = new wxStaticText;
    intro_ctrl->Create( itemWizardPage2, wxID_STATIC, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer3->Add(intro_ctrl, 0, wxALIGN_LEFT|wxALL, 5);

    itemBoxSizer3->Add(5, 5, 0, wxALIGN_LEFT|wxALL, 5);

    wxFlexGridSizer* grid = new wxFlexGridSizer(5, 2, 0, 0);
    grid->AddGrowableCol(1);
    grid->SetFlexibleDirection(wxBOTH);
    itemBoxSizer3->Add(grid, 0, wxEXPAND|wxALL, 5);

    project_name1_ctrl = new wxStaticText;
    project_name1_ctrl->Create( itemWizardPage2, wxID_STATIC, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    grid->Add(project_name1_ctrl, 0, wxALIGN_LEFT|wxALL, 5);

    project_name2_ctrl = new wxStaticText;
    project_name2_ctrl->Create( itemWizardPage2, wxID_STATIC, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    grid->Add(project_name2_ctrl, 0, wxALIGN_LEFT|wxALL|wxEXPAND, 5);

    project_url1_ctrl = new wxStaticText;
    project_url1_ctrl->Create( itemWizardPage2, wxID_STATIC, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    grid->Add(project_url1_ctrl, 0, wxALIGN_LEFT|wxALL, 5);

    project_url2_ctrl = new wxStaticText;
    project_url2_ctrl->Create( itemWizardPage2, wxID_STATIC, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    grid->Add(project_url2_ctrl, 0, wxALIGN_LEFT|wxALL|wxEXPAND, 5);

    continue_ctrl = new wxStaticText;
    continue_ctrl->Create( itemWizardPage2, wxID_STATIC, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer3->Add(continue_ctrl, 0, wxALIGN_LEFT|wxALL, 5);

    itemWizardPage2->SetSizer(itemBoxSizer3);
}

wxWizardPage* CProjectWelcomePage::GetPrev() const {
    return nullptr;
}

wxWizardPage* CProjectWelcomePage::GetNext() const {
    if (m_pParent->IsCancelInProgress()) {
        // Cancel Event Detected
        return m_pParent->GetCompletionErrorPage();
    } else if (m_pParent->GetProjectAuthenticator().IsEmpty()) {
        return m_pParent->GetAccountInfoPage();
    } else {
        return m_pParent->GetProjectPropertiesPage();
    }
}

void CProjectWelcomePage::SetPrev(CBOINCWizardPage *prev) {
    m_pPrev = prev;
}

bool CProjectWelcomePage::HasNextPage() const {
    return true;
}

bool CProjectWelcomePage::HasPrevPage() const {
    return m_pPrev != nullptr;
}

void CProjectWelcomePage::OnPageChanged(wxWizardEvent& event) {
    if (event.GetDirection() == false) return;
    wxLogTrace(wxT("Function Start/End"), wxT("CProjectWelcomePage::OnPageChanged - Function Begin"));

    wxString buf;
    buf.Printf(_("Welcome to %s."), m_pParent->GetProjectName().c_str());
    title_ctrl->SetLabel(buf);

    intro_ctrl->SetLabel(_("You have volunteered to compute for this project:"));
    project_name1_ctrl->SetLabel(_("Name:"));
    project_name2_ctrl->SetLabel(m_pParent->GetProjectName());
    project_url1_ctrl->SetLabel(_("URL:"));
    project_url2_ctrl->SetLabel(m_pParent->GetProjectURL());

    continue_ctrl->SetLabel(
        _("To continue, click Next.")
    );

    Layout();

    wxLogTrace(wxT("Function Start/End"), wxT("CProjectWelcomePage::OnPageChanged - Function End"));
}

void CProjectWelcomePage::OnCancel(wxWizardEvent& event) {
    m_pParent->ProcessCancelEvent(event);
}

void CProjectWelcomePage::OnPageChanging(wxWizardEvent& event) {
    if (event.GetDirection() == false) return;
    CBOINCWizardPage *pageNext = dynamic_cast<CBOINCWizardPage*>(GetNext());
    if (pageNext != nullptr) {
        pageNext->SetPrev(const_cast<CProjectWelcomePage*>(this));
    }
}
