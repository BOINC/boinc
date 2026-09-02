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
#include "ProjectInfoPage.h"
#include "AccountInfoPage.h"
#include "CompletionErrorPage.h"
#include "CompletionPage.h"
#include "NotFoundPage.h"
#include "AlreadyExistsPage.h"
#include "ProjectProcessingPage.h"

#include "res/wizprogress01.xpm"
#include "res/wizprogress02.xpm"
#include "res/wizprogress03.xpm"
#include "res/wizprogress04.xpm"
#include "res/wizprogress05.xpm"
#include "res/wizprogress06.xpm"
#include "res/wizprogress07.xpm"
#include "res/wizprogress08.xpm"
#include "res/wizprogress09.xpm"
#include "res/wizprogress10.xpm"
#include "res/wizprogress11.xpm"
#include "res/wizprogress12.xpm"

DEFINE_EVENT_TYPE(wxEVT_PROJECTPROCESSING_STATECHANGE)

IMPLEMENT_DYNAMIC_CLASS(CProjectProcessingPage, CBOINCWizardPage)

BEGIN_EVENT_TABLE(CProjectProcessingPage, CBOINCWizardPage)

EVT_PROJECTPROCESSING_STATECHANGE(CProjectProcessingPage::OnStateChange)
EVT_WIZARD_PAGE_CHANGED(wxID_ANY, CProjectProcessingPage::OnPageChanged )
EVT_WIZARD_PAGE_CHANGING(wxID_ANY, CProjectProcessingPage::OnPageChanging)
EVT_WIZARD_CANCEL(wxID_ANY, CProjectProcessingPage::OnCancel)

END_EVENT_TABLE()

CProjectProcessingPage::CProjectProcessingPage() {
}

CProjectProcessingPage::CProjectProcessingPage(CWizardAttach* parent) {
    Create(parent);
}

bool CProjectProcessingPage::Create(CWizardAttach* parent) {
    m_pParent = parent;
    m_pPrev = nullptr;
    m_pTitleStaticCtrl = nullptr;
    m_pProgressIndicator = nullptr;

    m_bProjectCommunicationsSucceeded = false;
    m_bProjectUnavailable = false;
    m_bProjectAccountNotFound = false;
    m_bProjectAccountAlreadyExists = false;
    m_iBitmapIndex = 0;
    m_iCurrentState = ATTACHPROJECT_INIT;

    wxWizardPage::Create(parent);

    CreateControls();
    GetSizer()->Fit(this);

    return true;
}

void CProjectProcessingPage::CreateControls() {
    CProjectProcessingPage* itemWizardPage36 = this;

    wxBoxSizer* itemBoxSizer37 = new wxBoxSizer(wxVERTICAL);
    itemWizardPage36->SetSizer(itemBoxSizer37);

    m_pTitleStaticCtrl = new wxStaticText;
    m_pTitleStaticCtrl->Create( itemWizardPage36, wxID_STATIC, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    m_pTitleStaticCtrl->SetFont(wxFont(10, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD, FALSE, _T("Verdana")));
    itemBoxSizer37->Add(m_pTitleStaticCtrl, 0, wxALIGN_LEFT|wxALL, 5);

    itemBoxSizer37->Add(5, 80, 0, wxALIGN_LEFT|wxALL, 5);

    wxFlexGridSizer* itemFlexGridSizer40 = new wxFlexGridSizer(1, 3, 0, 0);
    itemFlexGridSizer40->AddGrowableRow(0);
    itemFlexGridSizer40->AddGrowableCol(0);
    itemFlexGridSizer40->AddGrowableCol(1);
    itemFlexGridSizer40->AddGrowableCol(2);
    itemBoxSizer37->Add(itemFlexGridSizer40, 0, wxGROW|wxALL, 5);

    itemFlexGridSizer40->Add(5, 5, 0, wxGROW|wxALL, 5);

    wxBitmap itemBitmap41(GetBitmapResource(wxT("res/wizprogress01.xpm")));
    m_pProgressIndicator = new wxStaticBitmap;
    m_pProgressIndicator->Create( itemWizardPage36, ID_PROGRESSCTRL, itemBitmap41, wxDefaultPosition, wxSize(184, 48), 0 );
    itemFlexGridSizer40->Add(m_pProgressIndicator, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    itemFlexGridSizer40->Add(5, 5, 0, wxGROW|wxALL, 5);
}

wxWizardPage* CProjectProcessingPage::GetPrev() const {
    return m_pPrev;
}

wxWizardPage* CProjectProcessingPage::GetNext() const {
    if (m_pParent->IsCancelInProgress()) {
        // Cancel Event Detected
        return m_pParent->GetCompletionErrorPage();
    } else if (GetProjectAttachSucceeded()) {
        // We were successful in creating or retrieving an account
        return m_pParent->GetCompletionPage();
    } else if (!GetProjectCommunicationsSucceeded() && GetProjectAccountAlreadyExists()) {
        // The requested account already exists
        return m_pParent->GetErrAlreadyExistsPage();
    } else if (!GetProjectCommunicationsSucceeded() && GetProjectAccountNotFound()) {
        // The requested account does not exist or the password is bad
        return m_pParent->GetErrNotFoundPage();
    } else {
        // An error must have occurred
        return m_pParent->GetCompletionErrorPage();
    }
}

void CProjectProcessingPage::SetPrev(CBOINCWizardPage *prev) {
    m_pPrev = prev;
}

bool CProjectProcessingPage::HasNextPage() const {
    return true;
}

bool CProjectProcessingPage::HasPrevPage() const {
    return m_pPrev != nullptr;
}

void CProjectProcessingPage::StartProgress(wxStaticBitmap* pBitmap) {
    m_iBitmapIndex = 1;
    pBitmap->SetBitmap(GetBitmapResource(wxT("res/wizprogress01.xpm")));
}

void CProjectProcessingPage::IncrementProgress(wxStaticBitmap* pBitmap) {
    m_iBitmapIndex += 1;
    if (12 < m_iBitmapIndex) m_iBitmapIndex = 1;

    wxString str;
    str.Printf(wxT("res/wizprogress%02d.xpm"), m_iBitmapIndex);

    pBitmap->SetBitmap(GetBitmapResource(str));
    Update();
}

void CProjectProcessingPage::FinishProgress(wxStaticBitmap* pBitmap) {
    m_iBitmapIndex = 12;
    pBitmap->SetBitmap(GetBitmapResource(wxT("res/wizprogress12.xpm")));
}

wxBitmap CProjectProcessingPage::GetBitmapResource(const wxString& name) {
// TODO: Choose from multiple size images if provided, else resize the closest one
    // Bitmap retrieval
    if (name == wxT("res/wizprogress01.xpm"))
    {
        wxBitmap bitmap(wizprogress01_xpm);
        return bitmap;
    }
    else if (name == wxT("res/wizprogress02.xpm"))
    {
        wxBitmap  bitmap(wizprogress02_xpm);
        return bitmap;
    }
    else if (name == wxT("res/wizprogress03.xpm"))
    {
        wxBitmap  bitmap(wizprogress03_xpm);
        return bitmap;
    }
    else if (name == wxT("res/wizprogress04.xpm"))
    {
        wxBitmap  bitmap(wizprogress04_xpm);
        return bitmap;
    }
    else if (name == wxT("res/wizprogress05.xpm"))
    {
        wxBitmap  bitmap(wizprogress05_xpm);
        return bitmap;
    }
    else if (name == wxT("res/wizprogress06.xpm"))
    {
        wxBitmap  bitmap(wizprogress06_xpm);
        return bitmap;
    }
    else if (name == wxT("res/wizprogress07.xpm"))
    {
        wxBitmap  bitmap(wizprogress07_xpm);
        return bitmap;
    }
    else if (name == wxT("res/wizprogress08.xpm"))
    {
        wxBitmap  bitmap(wizprogress08_xpm);
        return bitmap;
    }
    else if (name == wxT("res/wizprogress09.xpm"))
    {
        wxBitmap  bitmap(wizprogress09_xpm);
        return bitmap;
    }
    else if (name == wxT("res/wizprogress10.xpm"))
    {
        wxBitmap  bitmap(wizprogress10_xpm);
        return bitmap;
    }
    else if (name == wxT("res/wizprogress11.xpm"))
    {
        wxBitmap  bitmap(wizprogress11_xpm);
        return bitmap;
    }
    else if (name == wxT("res/wizprogress12.xpm"))
    {
        wxBitmap  bitmap(wizprogress12_xpm);
        return bitmap;
    }
    return wxNullBitmap;
}

void CProjectProcessingPage::OnPageChanged(wxWizardEvent& event) {
    if (event.GetDirection() == false) return;

    wxASSERT(m_pTitleStaticCtrl);
    wxASSERT(m_pProgressIndicator);

    m_pTitleStaticCtrl->SetLabel(
        _("Communicating with project\nPlease wait...")
    );

    SetProjectCommunicationsSucceeded(false);
    SetProjectUnavailable(false);
    SetProjectAccountAlreadyExists(false);
    SetNextState(ATTACHPROJECT_INIT);

    CProjectProcessingPageEvent TransitionEvent(wxEVT_PROJECTPROCESSING_STATECHANGE, this);
    AddPendingEvent(TransitionEvent);

    Layout();
}

void CProjectProcessingPage::OnCancel(wxWizardEvent& event) {
    m_pParent->ProcessCancelEvent(event);
}

void CProjectProcessingPage::OnStateChange(CProjectProcessingPageEvent& WXUNUSED(event)) {
    CMainDocument* pDoc = wxGetApp().GetDocument();
    ACCOUNT_IN& ai      = m_pParent->GetAccountIn();
    ACCOUNT_OUT& ao     = m_pParent->GetAccountOut();
    unsigned int i;
    PROJECT_ATTACH_REPLY reply;
    wxString strBuffer = wxEmptyString;
    wxDateTime dtStartExecutionTime;
    wxDateTime dtCurrentExecutionTime;
    wxTimeSpan tsExecutionTime;
    bool bPostNewEvent = true;
    int retval = 0;
    bool creating_account = false;

    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));

    switch(GetCurrentState()) {
        case ATTACHPROJECT_INIT:
            m_pParent->DisableNextButton();
            m_pParent->DisableBackButton();

            StartProgress(m_pProgressIndicator);
            SetNextState(ATTACHPROJECT_ACCOUNTQUERY_BEGIN);
            break;
        case ATTACHPROJECT_ACCOUNTQUERY_BEGIN:
            SetNextState(ATTACHPROJECT_ACCOUNTQUERY_EXECUTE);
            break;
        case ATTACHPROJECT_ACCOUNTQUERY_EXECUTE:
            // Attempt to create the account or retrieve the authenticator.
            ai.clear();
            ao.clear();

            // use the web RPC URL in the get_project_config response
            // if present, otherwise use what the user typed
            //
            if (!m_pParent->GetProjectConfig().web_rpc_url_base.empty()) {
                ai.url = m_pParent->GetProjectConfig().web_rpc_url_base;
            } else if (!m_pParent->GetProjectConfig().master_url.empty()) {
                ai.url = m_pParent->GetProjectConfig().master_url;
            } else {
                ai.url = (const char*)m_pParent->GetProjectURL().mb_str();
            }

            if (!m_pParent->GetProjectAuthenticator().IsEmpty() ||
                m_pParent->IsCredentialsCached() || m_pParent->IsCredentialsDetected()
            ) {
                if (!m_pParent->IsCredentialsCached() || m_pParent->IsCredentialsDetected()) {
                    ao.authenticator = (const char*)m_pParent->GetProjectAuthenticator().mb_str();
                }
                SetProjectCommunicationsSucceeded(true);
            } else {
                // Setup initial values for both the create and lookup API

                if (m_pParent->GetProjectConfig().uses_username) {
                    ai.email_addr = (const char*)m_pParent->GetAccountUsername().utf8_str();
                } else {
                    ai.email_addr = (const char*)m_pParent->GetAccountEmailAddress().mb_str();
                }
                ai.passwd = (const char*)m_pParent->GetAccountPassword().mb_str();
                ai.user_name = (const char*)::wxGetUserName().utf8_str();
                if (ai.user_name.empty()) {
                    ai.user_name = (const char*)::wxGetUserId().mb_str();
                }

                // Configure for LDAP use
                //
                ai.ldap_auth = m_pParent->GetProjectConfig().ldap_auth;

                // Configure for project assigned hash lookup

                if (m_pParent->GetAccountInfoPage()->GetAccountCreateCtrlValue()) {
                    creating_account = true;
                    ai.consented_to_terms = m_pParent->GetConsentedToTerms();

                    // Wait until we are done processing the request.
                    dtStartExecutionTime = wxDateTime::Now();
                    dtCurrentExecutionTime = wxDateTime::Now();
                    tsExecutionTime = dtCurrentExecutionTime - dtStartExecutionTime;
                    retval = 0;
                    ao.error_num = ERR_RETRY;
                    while (
                        !retval &&
                        ((ERR_IN_PROGRESS == ao.error_num) || (ERR_RETRY == ao.error_num)) &&
                        tsExecutionTime.GetSeconds() <= 60 &&
                        !m_pParent->IsCancelInProgress()
                    ) {
                        if (ERR_RETRY == ao.error_num) {
                            retval = pDoc->rpc.create_account(ai);
                            if (retval) break;
                        }

                        dtCurrentExecutionTime = wxDateTime::Now();
                        tsExecutionTime = dtCurrentExecutionTime - dtStartExecutionTime;
                        retval = pDoc->rpc.create_account_poll(ao);

                        IncrementProgress(m_pProgressIndicator);

                        ::wxMilliSleep(500);
                        wxEventLoopBase::GetActive()->YieldFor(wxEVT_CATEGORY_USER_INPUT);
                    }

                    if ((!retval) && !ao.error_num) {
                        m_pParent->SetAccountCreatedSuccessfully(true);
                    }
                } else {
                    creating_account = false;

                    // Wait until we are done processing the request.
                    dtStartExecutionTime = wxDateTime::Now();
                    dtCurrentExecutionTime = wxDateTime::Now();
                    tsExecutionTime = dtCurrentExecutionTime - dtStartExecutionTime;
                    retval = 0;
                    ao.error_num = ERR_RETRY;
                    while (
                        !retval &&
                        ((ERR_IN_PROGRESS == ao.error_num) || (ERR_RETRY == ao.error_num)) &&
                        tsExecutionTime.GetSeconds() <= 60 &&
                        !m_pParent->IsCancelInProgress()
                    ) {
                        if (ERR_RETRY == ao.error_num) {
                            pDoc->rpc.lookup_account(ai);
                        }

                        dtCurrentExecutionTime = wxDateTime::Now();
                        tsExecutionTime = dtCurrentExecutionTime - dtStartExecutionTime;
                        retval = pDoc->rpc.lookup_account_poll(ao);

                        IncrementProgress(m_pProgressIndicator);

                        ::wxMilliSleep(500);
                        wxEventLoopBase::GetActive()->YieldFor(wxEVT_CATEGORY_USER_INPUT);
                    }
                }


                if ((!retval) && !ao.error_num) {
                    SetProjectCommunicationsSucceeded(true);
                } else {
                    SetProjectCommunicationsSucceeded(false);

                    if ((ao.error_num == ERR_DB_NOT_UNIQUE)
                        || (ao.error_num == ERR_NONUNIQUE_EMAIL)
                        || (ao.error_num == ERR_BAD_PASSWD && creating_account)
                    ) {
                        SetProjectAccountAlreadyExists(true);
                    } else {
                        SetProjectAccountAlreadyExists(false);
                    }

                    if ((ERR_NOT_FOUND == ao.error_num) ||
                        (ao.error_num == ERR_DB_NOT_FOUND) ||
                        (ERR_BAD_EMAIL_ADDR == ao.error_num) ||
                        (ERR_BAD_PASSWD == ao.error_num)
                    ) {
                        if (!m_pParent->GetProjectAuthenticator().IsEmpty()) {
                            if (!m_pParent->GetProjectAuthenticator().IsEmpty()) {
                                m_pParent->SetProjectAuthenticator(wxEmptyString);
                            }
                            CBOINCWizardPage *old = dynamic_cast<CBOINCWizardPage*>(m_pPrev);
                            m_pPrev = m_pParent->GetAccountInfoPage();
                            if (m_pPrev != nullptr) {
                                dynamic_cast<CBOINCWizardPage*>(m_pPrev)->SetPrev(old);
                            }
                        }
                        SetProjectAccountNotFound(true);
                    } else {
                        SetProjectAccountNotFound(false);
                    }

                    strBuffer = m_pParent->GetCompletionErrorPage()->GetServerMessagesCtrlLabel();
                    if ((HTTP_STATUS_NOT_FOUND == ao.error_num)) {
                        strBuffer +=
                            _("Required files not found on the server.");
                    } else if ((HTTP_STATUS_INTERNAL_SERVER_ERROR == ao.error_num)) {
                        strBuffer +=
                            _("An internal server error has occurred.");
                    } else {
                        if (ao.error_msg.size()) {
                            strBuffer += wxString(ao.error_msg.c_str(), wxConvUTF8) + wxString(wxT("\n"));
                        }
                    }
                    m_pParent->GetCompletionErrorPage()->SetServerMessagesCtrlLabel(strBuffer);
                }
            }
            SetNextState(ATTACHPROJECT_ATTACHPROJECT_BEGIN);
            break;
        case ATTACHPROJECT_ATTACHPROJECT_BEGIN:
            SetNextState(ATTACHPROJECT_ATTACHPROJECT_EXECUTE);
            break;
        case ATTACHPROJECT_ATTACHPROJECT_EXECUTE:
            if (GetProjectCommunicationsSucceeded()) {

                // Wait until we are done processing the request.
                dtStartExecutionTime = wxDateTime::Now();
                dtCurrentExecutionTime = wxDateTime::Now();
                tsExecutionTime = dtCurrentExecutionTime - dtStartExecutionTime;
                retval = 0;
                reply.error_num = ERR_RETRY;
                while (
                    !retval &&
                    ((ERR_IN_PROGRESS == reply.error_num) || (ERR_RETRY == reply.error_num)) &&
                    tsExecutionTime.GetSeconds() <= 60 &&
                    !m_pParent->IsCancelInProgress()
                ) {
                    if (ERR_RETRY == reply.error_num) {
                        if (m_pParent->IsCredentialsCached()) {
                            pDoc->rpc.project_attach_from_file();
                        } else {
                            std::string master_url;
                            if (!m_pParent->GetProjectConfig().master_url.empty()) {
                                master_url = m_pParent->GetProjectConfig().master_url;
                            } else {
                                master_url = (const char*)m_pParent->GetProjectURL().mb_str();
                            }
                            pDoc->rpc.project_attach(
                                master_url.c_str(),
                                ao.authenticator.c_str(),
                                m_pParent->GetProjectConfig().name.c_str(),
                                ai.email_addr.c_str()
                            );
                        }
                    }

                    dtCurrentExecutionTime = wxDateTime::Now();
                    tsExecutionTime = dtCurrentExecutionTime - dtStartExecutionTime;
                    retval = pDoc->rpc.project_attach_poll(reply);

                    IncrementProgress(m_pProgressIndicator);

                    ::wxMilliSleep(500);
#ifdef __WXMAC__
                    wxEventLoopBase * const modalLoop = wxEventLoopBase::GetActive();
                    modalLoop->YieldFor(wxEVT_CATEGORY_USER_INPUT);
#else
                    ::wxSafeYield(GetParent());
#endif
                }

                if (!retval && !reply.error_num) {
                    SetProjectAttachSucceeded(true);
                    m_pParent->SetAttachedToProjectSuccessfully(true);
                    m_pParent->SetProjectURL(wxString(ai.url.c_str(), wxConvUTF8));
                    m_pParent->SetProjectAuthenticator(wxString(ao.authenticator.c_str(), wxConvUTF8));
                } else {
                    SetProjectAttachSucceeded(false);

                    strBuffer = m_pParent->GetCompletionErrorPage()->GetServerMessagesCtrlLabel();
                    if ((HTTP_STATUS_INTERNAL_SERVER_ERROR == reply.error_num)) {
                        strBuffer +=
                            _("An internal server error has occurred.");
                    } else {
                        for (i=0; i<reply.messages.size(); i++) {
                            strBuffer += wxString(reply.messages[i].c_str(), wxConvUTF8) + wxString(wxT("\n"));
                        }
                    }
                    m_pParent->GetCompletionErrorPage()->SetServerMessagesCtrlLabel(strBuffer);
                }
            } else {
                SetProjectAttachSucceeded(false);
            }
            SetNextState(ATTACHPROJECT_CLEANUP);
            break;
        case ATTACHPROJECT_CLEANUP:
            FinishProgress(m_pProgressIndicator);
            SetNextState(ATTACHPROJECT_END);
            break;
        default:
            // Allow a glimpse of what the result was before advancing to the next page.
            wxSleep(1);

            m_pParent->EnableNextButton();
            m_pParent->EnableBackButton();
            m_pParent->SimulateNextButton();
            bPostNewEvent = false;
            break;
    }

    Update();

    if (bPostNewEvent && !m_pParent->IsCancelInProgress()) {
        CProjectProcessingPageEvent TransitionEvent(wxEVT_PROJECTPROCESSING_STATECHANGE, this);
        AddPendingEvent(TransitionEvent);
    }
}

void CProjectProcessingPage::OnPageChanging(wxWizardEvent& event) {
    if (event.GetDirection() == false) return;

    CBOINCWizardPage *pageNext = dynamic_cast<CBOINCWizardPage*>(GetNext());
    if (pageNext != nullptr) {
        pageNext->SetPrev(m_pPrev);
    }
}
