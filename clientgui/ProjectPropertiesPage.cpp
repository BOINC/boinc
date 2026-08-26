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
#include "network.h"
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
#include "ProjectInfoPage.h"
#include "CompletionErrorPage.h"
#include "TermsOfUsePage.h"
#include "ProjectProcessingPage.h"
#include "AccountInfoPage.h"
#include "ProxyInfoPage.h"
#include "UnavailablePage.h"
#include "NotDetectedPage.h"
#include "ProjectPropertiesPage.h"

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

DEFINE_EVENT_TYPE(wxEVT_PROJECTPROPERTIES_STATECHANGE)

IMPLEMENT_DYNAMIC_CLASS(CProjectPropertiesPage, CBOINCWizardPage)

BEGIN_EVENT_TABLE(CProjectPropertiesPage, CBOINCWizardPage)

EVT_PROJECTPROPERTIES_STATECHANGE(CProjectPropertiesPage::OnStateChange)
EVT_WIZARD_PAGE_CHANGED(wxID_ANY, CProjectPropertiesPage::OnPageChanged)
EVT_WIZARD_PAGE_CHANGING(wxID_ANY, CProjectPropertiesPage::OnPageChanging)
EVT_WIZARD_CANCEL(wxID_ANY, CProjectPropertiesPage::OnCancel)

END_EVENT_TABLE()

CProjectPropertiesPage::CProjectPropertiesPage() {
}

CProjectPropertiesPage::CProjectPropertiesPage(CWizardAttach* parent) {
    Create(parent);
}

bool CProjectPropertiesPage::Create(CWizardAttach* parent) {
    m_pParent = parent;
    m_pPrev = nullptr;
    m_pTitleStaticCtrl = nullptr;
    m_pProgressIndicator = nullptr;

    m_bProjectPropertiesSucceeded = false;
    m_bProjectPropertiesURLFailure = false;
    m_bProjectPropertiesCommunicationFailure = false;
    m_bProjectAccountCreationDisabled = false;
    m_bProjectClientAccountCreationDisabled = false;
    m_bNetworkConnectionNotDetected = false;
    m_bServerReportedError = false;
    m_bTermsOfUseRequired = true;
    m_iBitmapIndex = 0;
    m_iCurrentState = PROJPROP_INIT;

    wxWizardPage::Create(parent);

    CreateControls();
    GetSizer()->Fit(this);

    return true;
}

void CProjectPropertiesPage::CreateControls() {
    CProjectPropertiesPage* itemWizardPage36 = this;

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

wxWizardPage* CProjectPropertiesPage::GetPrev() const {
    return m_pPrev;
}

wxWizardPage* CProjectPropertiesPage::GetNext() const {
    if (m_pParent->IsCancelInProgress()) {
        // Cancel Event Detected
        return m_pParent->GetCompletionErrorPage();
    } else if (GetProjectPropertiesSucceeded() && GetTermsOfUseRequired()) {
        // Terms of Use are required before requesting account information
        return m_pParent->GetTermsOfUsePage();
    } else if (GetProjectPropertiesSucceeded() && GetCredentialsAlreadyAvailable()) {
        // Credentials are already available, do whatever we need to do.
        return m_pParent->GetProjectProcessingPage();
    } else if (GetProjectPropertiesSucceeded()) {
        // We were successful in retrieving the project properties
        return m_pParent->GetAccountInfoPage();
    } else if (GetProjectPropertiesCommunicationFailure() && GetNetworkConnectionNotDetected()) {
        // No Internet Connection
        return m_pParent->GetErrProxyInfoPage();
    } else if (GetProjectPropertiesURLFailure()) {
        // Not a BOINC based project
        return m_pParent->GetErrNotDetectedPage();
    } else if (GetServerReportedError()) {
        // Server reported an error, display the error
        return m_pParent->GetCompletionErrorPage();
    } else {
        // The project must be down for maintenance
        return m_pParent->GetErrUnavailablePage();
    }
}

void CProjectPropertiesPage::SetPrev(CBOINCWizardPage *prev) {
    m_pPrev = prev;
}

bool CProjectPropertiesPage::HasNextPage() const {
    return true;
}

bool CProjectPropertiesPage::HasPrevPage() const {
    return m_pPrev != nullptr;
}

void CProjectPropertiesPage::StartProgress(wxStaticBitmap* pBitmap) {
    m_iBitmapIndex = 1;
    pBitmap->SetBitmap(GetBitmapResource(wxT("res/wizprogress01.xpm")));
}

void CProjectPropertiesPage::IncrementProgress(wxStaticBitmap* pBitmap) {
    m_iBitmapIndex += 1;
    if (12 < m_iBitmapIndex) m_iBitmapIndex = 1;

    wxString str;
    str.Printf(wxT("res/wizprogress%02d.xpm"), m_iBitmapIndex);

    pBitmap->SetBitmap(GetBitmapResource(str));
    Update();
}

void CProjectPropertiesPage::FinishProgress(wxStaticBitmap* pBitmap) {
    m_iBitmapIndex = 12;
    pBitmap->SetBitmap(GetBitmapResource(wxT("res/wizprogress12.xpm")));
}

wxBitmap CProjectPropertiesPage::GetBitmapResource(const wxString& name) {
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

void CProjectPropertiesPage::OnPageChanged(wxWizardEvent& event) {
    if (event.GetDirection() == false) return;

    wxASSERT(m_pTitleStaticCtrl);
    wxASSERT(m_pProgressIndicator);

    m_pTitleStaticCtrl->SetLabel(
        _("Communicating with project\nPlease wait...")
    );

    SetProjectPropertiesSucceeded(false);
    SetProjectPropertiesURLFailure(false);
    SetProjectPropertiesCommunicationFailure(false);
    SetProjectAccountCreationDisabled(false);
    SetProjectClientAccountCreationDisabled(false);
    SetNetworkConnectionNotDetected(false);
    SetNextState(PROJPROP_INIT);

    CProjectPropertiesPageEvent TransitionEvent(wxEVT_PROJECTPROPERTIES_STATECHANGE, this);
    AddPendingEvent(TransitionEvent);

    Fit();
}

void CProjectPropertiesPage::OnCancel(wxWizardEvent& event) {
    m_pParent->ProcessCancelEvent(event);
}

void CProjectPropertiesPage::OnStateChange(CProjectPropertiesPageEvent& WXUNUSED(event)) {
    CMainDocument* pDoc = wxGetApp().GetDocument();
    PROJECT_CONFIG* pc  = &m_pParent->GetProjectConfig();
    CC_STATUS status;
    wxDateTime dtStartExecutionTime;
    wxDateTime dtCurrentExecutionTime;
    wxTimeSpan tsExecutionTime;
    wxString strBuffer = wxEmptyString;
    bool bPostNewEvent = true;
    int  iReturnValue = 0;

    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));

    switch(GetCurrentState()) {
        case PROJPROP_INIT:
            m_pParent->DisableNextButton();
            m_pParent->DisableBackButton();
            StartProgress(m_pProgressIndicator);
            SetNextState(PROJPROP_RETRPROJECTPROPERTIES_BEGIN);
            break;
        case PROJPROP_RETRPROJECTPROPERTIES_BEGIN:
            SetNextState(PROJPROP_RETRPROJECTPROPERTIES_EXECUTE);
            break;
        case PROJPROP_RETRPROJECTPROPERTIES_EXECUTE:
            // Attempt to retrieve the project's account creation policies

            // Wait until we are done processing the request.
            dtStartExecutionTime = wxDateTime::Now();
            dtCurrentExecutionTime = wxDateTime::Now();
            tsExecutionTime = dtCurrentExecutionTime - dtStartExecutionTime;
            iReturnValue = 0;
            pc->clear();
            pc->error_num = ERR_RETRY;
            while (
                !iReturnValue &&
                ((ERR_IN_PROGRESS == pc->error_num) || (ERR_RETRY == pc->error_num)) &&
                tsExecutionTime.GetSeconds() <= 60 &&
                !m_pParent->IsCancelInProgress()
            ) {
                if (ERR_RETRY == pc->error_num) {
                    pDoc->rpc.get_project_config(
                        (const char*)m_pParent->GetProjectURL().mb_str()
                    );
                }

                dtCurrentExecutionTime = wxDateTime::Now();
                tsExecutionTime = dtCurrentExecutionTime - dtStartExecutionTime;
                iReturnValue = pDoc->rpc.get_project_config_poll(*pc);
                IncrementProgress(m_pProgressIndicator);

                ::wxMilliSleep(500);
                wxEventLoopBase::GetActive()->YieldFor(wxEVT_CATEGORY_USER_INPUT);
            }

            if (
                !iReturnValue
                && (!pc->error_num || pc->error_num == ERR_ACCT_CREATION_DISABLED)
                ) {
                if (pc->account_manager) {
                    // user tried to attach an account manager as a project
                    //
                    SetProjectPropertiesSucceeded(false);
                    SetServerReportedError(true);
                    strBuffer = m_pParent->GetCompletionErrorPage()->GetServerMessagesCtrlLabel();
                    strBuffer += wxString(pc->name);
                    strBuffer += wxString(wxT(" is an account manager, not a project.\nSelect Tools / Use account manager.\n"));
                    m_pParent->GetCompletionErrorPage()->SetServerMessagesCtrlLabel(strBuffer);
                } else {
                    // We either successfully retrieved the project's
                    // account creation policies or we were able to talk
                    // to the web server and found out they do not support
                    // account creation through the wizard.
                    // In either case, claim success and set the correct flags
                    // to show the correct 'next' page.
                    //
                    SetProjectPropertiesSucceeded(true);
                    SetProjectAccountCreationDisabled(pc->account_creation_disabled);
                    SetProjectClientAccountCreationDisabled(pc->client_account_creation_disabled);
                    SetTermsOfUseRequired(!pc->terms_of_use.empty());
                }
            } else {

                SetProjectPropertiesSucceeded(false);
                SetProjectPropertiesURLFailure(pc->error_num == ERR_HTTP_PERMANENT);

                bool comm_failure = !iReturnValue && (
                    (ERR_GETHOSTBYNAME == pc->error_num)
                    || (ERR_CONNECT == pc->error_num)
                    || (ERR_XML_PARSE == pc->error_num)
                    || (ERR_PROJECT_DOWN == pc->error_num)
                );
                SetProjectPropertiesCommunicationFailure(comm_failure);

                bool server_reported_error = !iReturnValue && (
                    (ERR_HTTP_PERMANENT != pc->error_num)
                    && (ERR_GETHOSTBYNAME != pc->error_num)
                    && (ERR_CONNECT != pc->error_num)
                    && (ERR_XML_PARSE != pc->error_num)
                    && (ERR_PROJECT_DOWN != pc->error_num)
                );
                SetServerReportedError(server_reported_error);

                if (server_reported_error) {
                    strBuffer = m_pParent->GetCompletionErrorPage()->GetServerMessagesCtrlLabel();
                    if (pc->error_msg.size()) {
                        strBuffer += wxString(pc->error_msg.c_str(), wxConvUTF8) + wxString(wxT("\n"));
                    }
                    m_pParent->GetCompletionErrorPage()->SetServerMessagesCtrlLabel(strBuffer);
                }
            }

            SetNextState(PROJPROP_DETERMINENETWORKSTATUS_BEGIN);
            break;
        case PROJPROP_DETERMINENETWORKSTATUS_BEGIN:
            SetNextState(PROJPROP_DETERMINENETWORKSTATUS_EXECUTE);
            break;
        case PROJPROP_DETERMINENETWORKSTATUS_EXECUTE:
            // Attempt to determine if we are even connected to a network

            // Wait until we are done processing the request.
            dtStartExecutionTime = wxDateTime::Now();
            dtCurrentExecutionTime = wxDateTime::Now();
            tsExecutionTime = dtCurrentExecutionTime - dtStartExecutionTime;
            iReturnValue = 0;
            status.network_status = NETWORK_STATUS_LOOKUP_PENDING;
            while ((!iReturnValue && (NETWORK_STATUS_LOOKUP_PENDING == status.network_status)) &&
                   tsExecutionTime.GetSeconds() <= 60 &&
                   !m_pParent->IsCancelInProgress()
                  )
            {
                dtCurrentExecutionTime = wxDateTime::Now();
                tsExecutionTime = dtCurrentExecutionTime - dtStartExecutionTime;
                iReturnValue = pDoc->GetCoreClientStatus(status);
                IncrementProgress(m_pProgressIndicator);

                ::wxMilliSleep(500);
                wxEventLoopBase::GetActive()->YieldFor(wxEVT_CATEGORY_USER_INPUT);
            }

            SetNetworkConnectionNotDetected(NETWORK_STATUS_WANT_CONNECTION == status.network_status);

            SetNextState(PROJPROP_DETERMINEACCOUNTINFOSTATUS_BEGIN);
            break;
        case PROJPROP_DETERMINEACCOUNTINFOSTATUS_BEGIN:
            SetNextState(PROJPROP_DETERMINEACCOUNTINFOSTATUS_EXECUTE);
            break;
        case PROJPROP_DETERMINEACCOUNTINFOSTATUS_EXECUTE:
            // Determine if the account settings are already pre-populated.
            //   If so, advance to the Project Processing page.
            SetCredentialsAlreadyAvailable(m_pParent->IsCredentialsCached() || m_pParent->IsCredentialsDetected());

            SetNextState(PROJPROP_CLEANUP);
            break;
        case PROJPROP_CLEANUP:
            FinishProgress(m_pProgressIndicator);
            SetNextState(PROJPROP_END);
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
        CProjectPropertiesPageEvent TransitionEvent(wxEVT_PROJECTPROPERTIES_STATECHANGE, this);
        AddPendingEvent(TransitionEvent);
    }
}

void CProjectPropertiesPage::OnPageChanging(wxWizardEvent& event) {
    if (event.GetDirection() == false) return;
    CBOINCWizardPage *pageNext = dynamic_cast<CBOINCWizardPage*>(GetNext());
    if (pageNext != nullptr) {
        // we don't need to return to this page, so return the one before
        pageNext->SetPrev(m_pPrev);
    }

}
