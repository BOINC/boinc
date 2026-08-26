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
#include "CompletionErrorPage.h"
#include "ProjectPropertiesPage.h"
#include "AccountManagerPropertiesPage.h"
#include "ProxyPage.h"


IMPLEMENT_DYNAMIC_CLASS(CErrProxyPage, CBOINCWizardPage)

BEGIN_EVENT_TABLE(CErrProxyPage, CBOINCWizardPage)

EVT_WIZARD_PAGE_CHANGED(wxID_ANY, CErrProxyPage::OnPageChanged)
EVT_WIZARD_PAGE_CHANGING(wxID_ANY, CErrProxyPage::OnPageChanging)
EVT_WIZARD_CANCEL(wxID_ANY, CErrProxyPage::OnCancel)

END_EVENT_TABLE()

CErrProxyPage::CErrProxyPage() {
}

CErrProxyPage::CErrProxyPage(CWizardAttach* parent) {
    Create(parent);
}

bool CErrProxyPage::Create(CWizardAttach* parent) {
    m_pParent = parent;
    m_pPrev = nullptr;
    m_pTitleStaticCtrl = nullptr;
    m_pProxyHTTPDescriptionCtrl = nullptr;
    m_pProxyHTTPServerStaticCtrl = nullptr;
    m_pProxyHTTPServerCtrl = nullptr;
    m_pProxyHTTPPortStaticCtrl = nullptr;
    m_pProxyHTTPPortCtrl = nullptr;
    m_pProxyHTTPUsernameStaticCtrl = nullptr;
    m_pProxyHTTPUsernameCtrl = nullptr;
    m_pProxyHTTPPasswordStaticCtrl = nullptr;
    m_pProxyHTTPPasswordCtrl = nullptr;
    m_pProxySOCKSDescriptionCtrl = nullptr;
    m_pProxySOCKSServerStaticCtrl = nullptr;
    m_pProxySOCKSServerCtrl = nullptr;
    m_pProxySOCKSPortStaticCtrl = nullptr;
    m_pProxySOCKSPortCtrl = nullptr;
    m_pProxySOCKSUsernameStaticCtrl = nullptr;
    m_pProxySOCKSUsernameCtrl = nullptr;
    m_pProxySOCKSPasswordStaticCtrl = nullptr;
    m_pProxySOCKSPasswordCtrl = nullptr;

    wxWizardPage::Create(parent);

    CreateControls();
    GetSizer()->Fit(this);

    return true;
}

void CErrProxyPage::CreateControls() {
    CErrProxyPage* itemWizardPage121 = this;

    wxBoxSizer* itemBoxSizer122 = new wxBoxSizer(wxVERTICAL);
    itemWizardPage121->SetSizer(itemBoxSizer122);

    m_pTitleStaticCtrl = new wxStaticText;
    m_pTitleStaticCtrl->Create( itemWizardPage121, wxID_STATIC, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    m_pTitleStaticCtrl->SetFont(wxFont(10, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD, FALSE, _T("Verdana")));
    itemBoxSizer122->Add(m_pTitleStaticCtrl, 0, wxALIGN_LEFT|wxALL, 5);

    itemBoxSizer122->Add(5, 5, 0, wxALIGN_LEFT|wxALL, 5);

    m_pProxyHTTPDescriptionCtrl = new wxStaticBox(itemWizardPage121, wxID_ANY, wxEmptyString);
    wxStaticBoxSizer* itemStaticBoxSizer125 = new wxStaticBoxSizer(m_pProxyHTTPDescriptionCtrl, wxVERTICAL);
    itemBoxSizer122->Add(itemStaticBoxSizer125, 0, wxGROW|wxALL, 0);

    wxFlexGridSizer* itemFlexGridSizer126 = new wxFlexGridSizer(3, 2, 0, 0);
    itemFlexGridSizer126->AddGrowableCol(1);
    itemStaticBoxSizer125->Add(itemFlexGridSizer126, 0, wxGROW|wxALL, 2);

    m_pProxyHTTPServerStaticCtrl = new wxStaticText;
    m_pProxyHTTPServerStaticCtrl->Create( itemWizardPage121, ID_PROXYHTTPSERVERSTATICCTRL, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer126->Add(m_pProxyHTTPServerStaticCtrl, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 2);

    wxFlexGridSizer* itemFlexGridSizer128 = new wxFlexGridSizer(1, 3, 0, 0);
    itemFlexGridSizer128->AddGrowableCol(0);
    itemFlexGridSizer126->Add(itemFlexGridSizer128, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 0);

    m_pProxyHTTPServerCtrl = new wxTextCtrl;
    m_pProxyHTTPServerCtrl->Create( itemWizardPage121, ID_PROXYHTTPSERVERCTRL, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer128->Add(m_pProxyHTTPServerCtrl, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 2);

    m_pProxyHTTPPortStaticCtrl = new wxStaticText;
    m_pProxyHTTPPortStaticCtrl->Create( itemWizardPage121, ID_PROXYHTTPPORTSTATICCTRL, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer128->Add(m_pProxyHTTPPortStaticCtrl, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 2);

    m_pProxyHTTPPortCtrl = new wxTextCtrl;
    m_pProxyHTTPPortCtrl->Create( itemWizardPage121, ID_PROXYHTTPPORTCTRL, wxEmptyString, wxDefaultPosition, wxSize(50, -1), 0 );
    itemFlexGridSizer128->Add(m_pProxyHTTPPortCtrl, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 2);

    m_pProxyHTTPUsernameStaticCtrl = new wxStaticText;
    m_pProxyHTTPUsernameStaticCtrl->Create( itemWizardPage121, ID_PROXYHTTPUSERNAMESTATICCTRL, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer126->Add(m_pProxyHTTPUsernameStaticCtrl, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 2);

    m_pProxyHTTPUsernameCtrl = new wxTextCtrl;
    m_pProxyHTTPUsernameCtrl->Create( itemWizardPage121, ID_PROXYHTTPUSERNAMECTRL, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer126->Add(m_pProxyHTTPUsernameCtrl, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 2);

    m_pProxyHTTPPasswordStaticCtrl = new wxStaticText;
    m_pProxyHTTPPasswordStaticCtrl->Create( itemWizardPage121, ID_PROXYHTTPPASSWORDSTATICCTRL, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer126->Add(m_pProxyHTTPPasswordStaticCtrl, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 2);

    m_pProxyHTTPPasswordCtrl = new wxTextCtrl;
    m_pProxyHTTPPasswordCtrl->Create( itemWizardPage121, ID_PROXYHTTPPASSWORDCTRL, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_PASSWORD );
    itemFlexGridSizer126->Add(m_pProxyHTTPPasswordCtrl, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 2);

    m_pProxySOCKSDescriptionCtrl = new wxStaticBox(itemWizardPage121, wxID_ANY, wxEmptyString);
    wxStaticBoxSizer* itemStaticBoxSizer137 = new wxStaticBoxSizer(m_pProxySOCKSDescriptionCtrl, wxVERTICAL);
    itemBoxSizer122->Add(itemStaticBoxSizer137, 0, wxGROW|wxALL, 0);

    wxFlexGridSizer* itemFlexGridSizer138 = new wxFlexGridSizer(3, 2, 0, 0);
    itemFlexGridSizer138->AddGrowableCol(1);
    itemStaticBoxSizer137->Add(itemFlexGridSizer138, 0, wxGROW|wxALL, 2);

    m_pProxySOCKSServerStaticCtrl = new wxStaticText;
    m_pProxySOCKSServerStaticCtrl->Create( itemWizardPage121, ID_PROXYSOCKSSERVERSTATICCTRL, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer138->Add(m_pProxySOCKSServerStaticCtrl, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 2);

    wxFlexGridSizer* itemFlexGridSizer140 = new wxFlexGridSizer(1, 3, 0, 0);
    itemFlexGridSizer140->AddGrowableCol(0);
    itemFlexGridSizer138->Add(itemFlexGridSizer140, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 0);

    m_pProxySOCKSServerCtrl = new wxTextCtrl;
    m_pProxySOCKSServerCtrl->Create( itemWizardPage121, ID_PROXYSOCKSSERVERCTRL, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer140->Add(m_pProxySOCKSServerCtrl, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 2);

    m_pProxySOCKSPortStaticCtrl = new wxStaticText;
    m_pProxySOCKSPortStaticCtrl->Create( itemWizardPage121, ID_PROXYSOCKSPORTSTATICCTRL, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer140->Add(m_pProxySOCKSPortStaticCtrl, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 2);

    m_pProxySOCKSPortCtrl = new wxTextCtrl;
    m_pProxySOCKSPortCtrl->Create( itemWizardPage121, ID_PROXYSOCKSPORTCTRL, wxEmptyString, wxDefaultPosition, wxSize(50, -1), 0 );
    itemFlexGridSizer140->Add(m_pProxySOCKSPortCtrl, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 2);

    m_pProxySOCKSUsernameStaticCtrl = new wxStaticText;
    m_pProxySOCKSUsernameStaticCtrl->Create( itemWizardPage121, ID_PROXYSOCKSUSERNAMESTATICCTRL, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer138->Add(m_pProxySOCKSUsernameStaticCtrl, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 2);

    m_pProxySOCKSUsernameCtrl = new wxTextCtrl;
    m_pProxySOCKSUsernameCtrl->Create( itemWizardPage121, ID_PROXYSOCKSUSERNAMECTRL, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer138->Add(m_pProxySOCKSUsernameCtrl, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 2);

    m_pProxySOCKSPasswordStaticCtrl = new wxStaticText;
    m_pProxySOCKSPasswordStaticCtrl->Create( itemWizardPage121, ID_PROXYSOCKSPASSWORDSTATICCTRL, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer138->Add(m_pProxySOCKSPasswordStaticCtrl, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 2);

    m_pProxySOCKSPasswordCtrl = new wxTextCtrl;
    m_pProxySOCKSPasswordCtrl->Create( itemWizardPage121, ID_PROXYSOCKSPASSWORDCTRL, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_PASSWORD );
    itemFlexGridSizer138->Add(m_pProxySOCKSPasswordCtrl, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 2);

    // Set validators
    m_pProxyHTTPServerCtrl->SetValidator( wxTextValidator(wxFILTER_NONE, & m_strProxyHTTPServer) );
    m_pProxyHTTPPortCtrl->SetValidator( wxTextValidator(wxFILTER_NUMERIC, & m_strProxyHTTPPort) );
    m_pProxyHTTPUsernameCtrl->SetValidator( wxTextValidator(wxFILTER_NONE, & m_strProxyHTTPUsername) );
    m_pProxyHTTPPasswordCtrl->SetValidator( wxTextValidator(wxFILTER_NONE, & m_strProxyHTTPPassword) );
    m_pProxySOCKSServerCtrl->SetValidator( wxTextValidator(wxFILTER_NONE, & m_strProxySOCKSServer) );
    m_pProxySOCKSPortCtrl->SetValidator( wxTextValidator(wxFILTER_NUMERIC, & m_strProxySOCKSPort) );
    m_pProxySOCKSUsernameCtrl->SetValidator( wxTextValidator(wxFILTER_NONE, & m_strProxySOCKSUsername) );
    m_pProxySOCKSPasswordCtrl->SetValidator( wxTextValidator(wxFILTER_NONE, & m_strProxySOCKSPassword) );
}

wxWizardPage* CErrProxyPage::GetPrev() const {
    return m_pPrev;
}

wxWizardPage* CErrProxyPage::GetNext() const {
    if (m_pParent->IsCancelInProgress()) {
        // Cancel Event Detected
        return m_pParent->GetCompletionErrorPage();
    } else if (m_pParent->GetIsAttachToProjectWizard()) {
        return m_pParent->GetProjectPropertiesPage();
    } else if (m_pParent->GetIsAccountManagerWizard()) {
        return m_pParent->GetAccountManagerPropertiesPage();
    }
    return nullptr;
}

void CErrProxyPage::SetPrev(CBOINCWizardPage *prev) {
    m_pPrev = prev;
}

bool CErrProxyPage::HasNextPage() const {
    return true;
}

bool CErrProxyPage::HasPrevPage() const {
    return m_pPrev != nullptr;
}

void CErrProxyPage::OnPageChanged(wxWizardEvent& WXUNUSED(event)) {
    CMainDocument* pDoc = wxGetApp().GetDocument();
    wxString       strBuffer = wxEmptyString;

    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));
    wxASSERT(m_pTitleStaticCtrl);
    wxASSERT(m_pProxyHTTPDescriptionCtrl);
    wxASSERT(m_pProxyHTTPServerStaticCtrl);
    wxASSERT(m_pProxyHTTPServerCtrl);
    wxASSERT(m_pProxyHTTPPortStaticCtrl);
    wxASSERT(m_pProxyHTTPPortCtrl);
    wxASSERT(m_pProxyHTTPUsernameStaticCtrl);
    wxASSERT(m_pProxyHTTPUsernameCtrl);
    wxASSERT(m_pProxyHTTPPasswordStaticCtrl);
    wxASSERT(m_pProxyHTTPPasswordCtrl);
    wxASSERT(m_pProxySOCKSDescriptionCtrl);
    wxASSERT(m_pProxySOCKSServerStaticCtrl);
    wxASSERT(m_pProxySOCKSServerCtrl);
    wxASSERT(m_pProxySOCKSPortStaticCtrl);
    wxASSERT(m_pProxySOCKSPortCtrl);
    wxASSERT(m_pProxySOCKSUsernameStaticCtrl);
    wxASSERT(m_pProxySOCKSUsernameCtrl);
    wxASSERT(m_pProxySOCKSPasswordStaticCtrl);
    wxASSERT(m_pProxySOCKSPasswordCtrl);

    // Moving from the previous page, update text and get state

    m_pTitleStaticCtrl->SetLabel(
        _("Proxy configuration")
    );
    m_pProxyHTTPDescriptionCtrl->SetLabel(
        _("HTTP proxy")
    );
    m_pProxyHTTPServerStaticCtrl->SetLabel(
        _("Server:")
    );
    m_pProxyHTTPPortStaticCtrl->SetLabel(
        _("Port:")
    );
    m_pProxyHTTPUsernameStaticCtrl->SetLabel(
        _("User Name:")
    );
    m_pProxyHTTPPasswordStaticCtrl->SetLabel(
        _("Password:")
    );
#if 0
    m_pProxyHTTPAutodetectCtrl->SetLabel(
        _("Autodetect")
    );
#endif
    m_pProxySOCKSDescriptionCtrl->SetLabel(
        _("SOCKS proxy")
    );
    m_pProxySOCKSServerStaticCtrl->SetLabel(
        _("Server:")
    );
    m_pProxySOCKSPortStaticCtrl->SetLabel(
        _("Port:")
    );
    m_pProxySOCKSUsernameStaticCtrl->SetLabel(
        _("User Name:")
    );
    m_pProxySOCKSPasswordStaticCtrl->SetLabel(
        _("Password:")
    );

    pDoc->GetProxyConfiguration();
    m_pProxyHTTPServerCtrl->SetValue(wxString(pDoc->proxy_info.http_server_name.c_str(), wxConvUTF8));
    m_pProxyHTTPUsernameCtrl->SetValue(wxString(pDoc->proxy_info.http_user_name.c_str(), wxConvUTF8));
    m_pProxyHTTPPasswordCtrl->SetValue(wxString(pDoc->proxy_info.http_user_passwd.c_str(), wxConvUTF8));

    strBuffer.Printf(wxT("%d"), pDoc->proxy_info.http_server_port);
    m_pProxyHTTPPortCtrl->SetValue(strBuffer);

    m_pProxySOCKSServerCtrl->SetValue(wxString(pDoc->proxy_info.socks_server_name.c_str(), wxConvUTF8));
    m_pProxySOCKSUsernameCtrl->SetValue(wxString(pDoc->proxy_info.socks5_user_name.c_str(), wxConvUTF8));
    m_pProxySOCKSPasswordCtrl->SetValue(wxString(pDoc->proxy_info.socks5_user_passwd.c_str(), wxConvUTF8));

    strBuffer.Printf(wxT("%d"), pDoc->proxy_info.socks_server_port);
    m_pProxySOCKSPortCtrl->SetValue(strBuffer);

    Fit();
    m_pProxyHTTPServerCtrl->SetFocus();
}

void CErrProxyPage::OnPageChanging(wxWizardEvent& event) {
    CMainDocument* pDoc = wxGetApp().GetDocument();
    wxString       strBuffer = wxEmptyString;
    long           lBuffer = 0;

    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));

    if (event.GetDirection() == true) {
        CBOINCWizardPage *pageNext = dynamic_cast<CBOINCWizardPage*>(GetNext());
        if (pageNext != nullptr) {
            pageNext->SetPrev(const_cast<CErrProxyPage*>(this));
        }
// Moving to the next page, save state
        pDoc->proxy_info.use_http_proxy = (m_pProxyHTTPServerCtrl->GetValue().Length() > 0);
        pDoc->proxy_info.http_server_name = (const char*)m_pProxyHTTPServerCtrl->GetValue().mb_str();
        pDoc->proxy_info.http_user_name = (const char*)m_pProxyHTTPUsernameCtrl->GetValue().mb_str();
        pDoc->proxy_info.http_user_passwd = (const char*)m_pProxyHTTPPasswordCtrl->GetValue().mb_str();

        strBuffer = m_pProxyHTTPPortCtrl->GetValue();
        strBuffer.ToLong((long*)&lBuffer);
        pDoc->proxy_info.http_server_port = lBuffer;

        pDoc->proxy_info.use_socks_proxy = (m_pProxySOCKSServerCtrl->GetValue().Length() > 0);
        pDoc->proxy_info.socks_server_name = (const char*)m_pProxySOCKSServerCtrl->GetValue().mb_str();
        pDoc->proxy_info.socks5_user_name = (const char*)m_pProxySOCKSUsernameCtrl->GetValue().mb_str();
        pDoc->proxy_info.socks5_user_passwd = (const char*)m_pProxySOCKSPasswordCtrl->GetValue().mb_str();

        strBuffer = m_pProxySOCKSPortCtrl->GetValue();
        strBuffer.ToLong((long*)&lBuffer);
        pDoc->proxy_info.socks_server_port = lBuffer;

        pDoc->SetProxyConfiguration();
    }
}

void CErrProxyPage::OnCancel(wxWizardEvent& event) {
    m_pParent->ProcessCancelEvent(event);
}
