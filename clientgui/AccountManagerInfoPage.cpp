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
#include "str_util.h"
#include "error_numbers.h"
#include "error_numbers.h"
#include "BOINCGUIApp.h"
#include "SkinManager.h"
#include "MainDocument.h"
#include "ValidateURL.h"
#include "BOINCBaseWizard.h"
#include "WizardAttach.h"
#include "CompletionErrorPage.h"
#include "AccountManagerPropertiesPage.h"
#include "AccountManagerInfoPage.h"

IMPLEMENT_DYNAMIC_CLASS(CAcctMgrListItem, wxObject)

IMPLEMENT_DYNAMIC_CLASS(CAccountManagerInfoPage, CBOINCWizardPage)

BEGIN_EVENT_TABLE(CAccountManagerInfoPage, CBOINCWizardPage)

EVT_WIZARD_PAGE_CHANGED(wxID_ANY, CAccountManagerInfoPage::OnPageChanged)
EVT_WIZARD_PAGE_CHANGING(wxID_ANY, CAccountManagerInfoPage::OnPageChanging)
EVT_WIZARD_CANCEL(wxID_ANY, CAccountManagerInfoPage::OnCancel)
EVT_LISTBOX(ID_PROJECTS, CAccountManagerInfoPage::OnProjectSelected)
EVT_BUTTON(ID_PROJECTWEBPAGECTRL, CAccountManagerInfoPage::OnProjectItemDisplay)
EVT_TEXT(ID_PROJECTURLCTRL, CAccountManagerInfoPage::OnURLChanged)

END_EVENT_TABLE()

CAccountManagerInfoPage::CAccountManagerInfoPage() {
}

CAccountManagerInfoPage::CAccountManagerInfoPage(CWizardAttach* parent) {
    Create(parent);
}

bool CAccountManagerInfoPage::Create(CWizardAttach* parent) {
    m_pParent = parent;
    m_pPrev = nullptr;
    m_pTitleStaticCtrl = nullptr;
    m_pDescriptionStaticCtrl = nullptr;
    m_pProjectListCtrl = nullptr;
    m_pProjectUrlStaticCtrl = nullptr;
    m_pProjectUrlCtrl = nullptr;
    m_bAccountManagerListPopulated = false;

    wxWizardPage::Create(parent);

    CreateControls();
    GetSizer()->Fit(this);
    return true;
}

void CAccountManagerInfoPage::CreateControls() {
#ifdef __WXMAC__
    const int listboxWidth = 225;
    const int descriptionWidth = 350;
#else
    const int listboxWidth = 150;
    const int descriptionWidth = 310;
#endif

    CAccountManagerInfoPage* itemWizardPage23 = this;

    wxBoxSizer* itemBoxSizer24 = new wxBoxSizer(wxVERTICAL);
    itemWizardPage23->SetSizer(itemBoxSizer24);

    m_pTitleStaticCtrl = new wxStaticText;
    m_pTitleStaticCtrl->Create( itemWizardPage23, wxID_STATIC, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    m_pTitleStaticCtrl->SetFont(wxFont(10, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD, FALSE, _T("Verdana")));
    itemBoxSizer24->Add(m_pTitleStaticCtrl, 0, wxALIGN_LEFT|wxALL, 5);

    m_pDescriptionStaticCtrl = new wxStaticText;
    m_pDescriptionStaticCtrl->Create( itemWizardPage23, wxID_STATIC, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer24->Add(m_pDescriptionStaticCtrl, 0, wxALIGN_LEFT|wxALL, 5);

    wxFlexGridSizer* itemFlexGridSizer3 = new wxFlexGridSizer(1, 2, 0, 0);
    itemFlexGridSizer3->AddGrowableRow(0);
    itemFlexGridSizer3->AddGrowableCol(0);
    itemFlexGridSizer3->AddGrowableCol(1);
    itemBoxSizer24->Add(itemFlexGridSizer3, 1, wxGROW|wxALL, 5);

    wxArrayString m_pProjectsCtrlStrings;
    m_pProjectListCtrl = new wxListBox( itemWizardPage23, ID_PROJECTS, wxDefaultPosition, wxSize(listboxWidth, 175), m_pProjectsCtrlStrings, wxLB_SINGLE|wxLB_SORT );
    itemFlexGridSizer3->Add(m_pProjectListCtrl, 0, wxGROW|wxRIGHT, 10);

    wxFlexGridSizer* itemFlexGridSizer4 = new wxFlexGridSizer(3, 1, 0, 0);
    itemFlexGridSizer4->AddGrowableRow(1);
    itemFlexGridSizer3->Add(itemFlexGridSizer4, 0, wxGROW|wxLEFT, 10);

    m_pProjectDetailsStaticCtrl = new wxStaticText;
    m_pProjectDetailsStaticCtrl->Create( itemWizardPage23, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer4->Add(m_pProjectDetailsStaticCtrl, 0, wxBOTTOM, 5);

    m_pProjectDetailsDescriptionCtrl = new wxTextCtrl( itemWizardPage23, ID_PROJECTDESCRIPTION, wxT(""), wxDefaultPosition, wxSize(descriptionWidth, 100), wxTE_MULTILINE|wxTE_READONLY );
    itemFlexGridSizer4->Add(m_pProjectDetailsDescriptionCtrl, 0, wxGROW);

    m_pOpenWebSiteButton = new wxButton( this, ID_PROJECTWEBPAGECTRL, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer4->Add(m_pOpenWebSiteButton, 0, wxALIGN_CENTER|wxTOP|wxBOTTOM, 5);

    wxFlexGridSizer* itemFlexGridSizer11 = new wxFlexGridSizer(2, 1, 0, 0);
    itemFlexGridSizer11->AddGrowableRow(0);
    itemFlexGridSizer11->AddGrowableCol(0);
    itemBoxSizer24->Add(itemFlexGridSizer11, 0, wxGROW|wxALL, 0);

    wxBoxSizer* itemBoxSizer22 = new wxBoxSizer(wxVERTICAL);
    itemFlexGridSizer11->Add(itemBoxSizer22, 0, wxGROW|wxALL, 0);

    wxFlexGridSizer* itemFlexGridSizer14 = new wxFlexGridSizer(1, 2, 0, 0);
    itemFlexGridSizer14->AddGrowableCol(1);
    itemBoxSizer24->Add(itemFlexGridSizer14, 0, wxGROW|wxRIGHT, 10);

    m_pProjectUrlStaticCtrl = new wxStaticText;
    m_pProjectUrlStaticCtrl->Create( itemWizardPage23, ID_PROJECTURLSTATICCTRL, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer14->Add(m_pProjectUrlStaticCtrl, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_pProjectUrlCtrl = new wxTextCtrl;
    m_pProjectUrlCtrl->Create( itemWizardPage23, ID_PROJECTURLCTRL, wxEmptyString, wxDefaultPosition, wxSize(200, -1), 0 );
    itemFlexGridSizer14->Add(m_pProjectUrlCtrl, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);
}

wxWizardPage* CAccountManagerInfoPage::GetPrev() const {
    return m_pPrev;
}

wxWizardPage* CAccountManagerInfoPage::GetNext() const {
    if (m_pParent->IsCancelInProgress()) {
        // Cancel Event Detected
        return m_pParent->GetCompletionErrorPage();
    } else {
        return m_pParent->GetAccountManagerPropertiesPage();
    }
}

void CAccountManagerInfoPage::SetPrev(CBOINCWizardPage *prev) {
    m_pPrev = prev;
}

bool CAccountManagerInfoPage::HasNextPage() const {
    return true;
}

bool CAccountManagerInfoPage::HasPrevPage() const {
    return m_pPrev != nullptr;
}

void CAccountManagerInfoPage::OnPageChanged(wxWizardEvent& event) {
    if (event.GetDirection() == false) return;
    wxLogTrace(wxT("Function Start/End"), wxT("CAccountManagerInfoPage::OnPageChanged - Function Begin"));

    unsigned int      i;
    ALL_PROJECTS_LIST pl;
    CMainDocument* pDoc = wxGetApp().GetDocument();

    wxASSERT(m_pTitleStaticCtrl);
    wxASSERT(m_pDescriptionStaticCtrl);
    wxASSERT(m_pProjectDetailsStaticCtrl);
    wxASSERT(m_pProjectDetailsDescriptionCtrl);
    wxASSERT(m_pOpenWebSiteButton);
    wxASSERT(m_pProjectUrlStaticCtrl);
    wxASSERT(m_pProjectUrlCtrl);


    m_pTitleStaticCtrl->SetLabel(
        _("Choose an account manager")
    );
    m_pDescriptionStaticCtrl->SetLabel(
        _("To choose an account manager, click its name or \ntype its URL below.")
    );

    m_pProjectDetailsStaticCtrl->SetLabel(
        _("Account manager details:")
    );

    m_pProjectUrlStaticCtrl->SetLabel(
        _("Account manager &URL:")
    );

    m_pOpenWebSiteButton->SetLabel(
        _("Open web page")
    );

    m_pOpenWebSiteButton->SetToolTip( _("Visit this account manager's web site"));

    // Populate the list box with project information
    //
    if (!m_bAccountManagerListPopulated) {
        pDoc->rpc.get_all_projects_list(pl);
        for (i=0; i<pl.account_managers.size(); i++) {
            wxLogTrace(
                wxT("Function Status"),
                wxT("CAccountManagerInfoPage::OnPageChanged - Name: '%s', URL: '%s', Supported: '%d'"),
                wxString(pl.account_managers[i]->name.c_str(), wxConvUTF8).c_str(),
                wxString(pl.account_managers[i]->url.c_str(), wxConvUTF8).c_str(),
                true
            );

            CAcctMgrListItem* pItem = new CAcctMgrListItem();

            pItem->SetURL( wxString(pl.account_managers[i]->url.c_str(), wxConvUTF8) );
            pItem->SetName( wxString(pl.account_managers[i]->name.c_str(), wxConvUTF8) );
            pItem->SetImage( wxString(pl.account_managers[i]->image.c_str(), wxConvUTF8) );
            pItem->SetDescription( wxString(pl.account_managers[i]->description.c_str(), wxConvUTF8) );


            m_pProjectListCtrl->Append(
                wxString(pl.account_managers[i]->name.c_str(), wxConvUTF8),
                pItem
            );
        }

        // Pre select the first element
        if (m_pProjectListCtrl->GetCount()) {
            m_pProjectListCtrl->SetSelection(0);
            CAcctMgrListItem* pItem = (CAcctMgrListItem*)(m_pProjectListCtrl->GetClientData(0));

            m_pProjectUrlCtrl->SetValue(pItem->GetURL());
            m_pProjectDetailsDescriptionCtrl->SetValue(pItem->GetDescription());
        }
        m_bAccountManagerListPopulated = true;
    }

    Layout();
    FitInside();
    m_pProjectListCtrl->SetFocus();

    wxLogTrace(wxT("Function Start/End"), wxT("CAccountManagerInfoPage::OnPageChanged - Function End"));
}

void CAccountManagerInfoPage::OnPageChanging(wxWizardEvent& event) {
    if (event.GetDirection() == false) return;

    CBOINCWizardPage *pageNext = dynamic_cast<CBOINCWizardPage*>(GetNext());
    if (pageNext != nullptr) {
        pageNext->SetPrev(const_cast<CAccountManagerInfoPage*>(this));
    }

    wxString url = m_pProjectUrlCtrl->GetValue();
    wxString name = url;
    int sel = m_pProjectListCtrl->GetSelection();
    if (sel != wxNOT_FOUND) {
        CAcctMgrListItem* pItem = (CAcctMgrListItem*)(m_pProjectListCtrl->GetClientData(sel));
        // Update authoritative data in CWizardAttach
        if (m_pProjectUrlCtrl->GetValue() == pItem->GetURL()) {
            url = pItem->GetURL();
            name = pItem->GetName();
        }
    }
    m_pParent->SetProjectURL(url);
    m_pParent->SetProjectName(name);
}

void CAccountManagerInfoPage::OnProjectSelected(wxCommandEvent&) {
    int sel = m_pProjectListCtrl->GetSelection();
    if (sel == wxNOT_FOUND) {
        m_pProjectUrlCtrl->SetValue(wxEmptyString);
        m_pProjectDetailsDescriptionCtrl->SetValue(wxEmptyString);
    } else {
        CAcctMgrListItem* pItem = (CAcctMgrListItem*)(m_pProjectListCtrl->GetClientData(sel));
        m_pProjectUrlCtrl->SetValue(pItem->GetURL());
        m_pProjectDetailsDescriptionCtrl->SetValue(pItem->GetDescription());
    }
}

void CAccountManagerInfoPage::OnURLChanged(wxCommandEvent&) {
    m_pOpenWebSiteButton->Enable(!m_pProjectUrlCtrl->GetValue().IsEmpty());
}

void CAccountManagerInfoPage::OnProjectItemDisplay(wxCommandEvent&) {
    wxString url = m_pProjectUrlCtrl->GetValue();
    if (!url.IsEmpty()) {
        wxLaunchDefaultBrowser(url);
    }
}

void CAccountManagerInfoPage::OnCancel(wxWizardEvent& event) {
    m_pParent->ProcessCancelEvent(event);
}
