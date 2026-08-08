// This file is part of BOINC.
// https://boinc.berkeley.edu
// Copyright (C) 2025 University of California
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
#if defined(__GNUG__) && !defined(__APPLE__)
#pragma implementation "AccountManagerProcessingPage.h"
#endif

#include "stdwx.h"
#include "diagnostics.h"
#include "util.h"
#include "mfile.h"
#include "miofile.h"
#include "parse.h"
#include "error_numbers.h"
#include "wizardex.h"
#include "error_numbers.h"
#include "BOINCGUIApp.h"
#include "SkinManager.h"
#include "MainDocument.h"
#include "BOINCBaseWizard.h"
#include "WizardAttach.h"
#include "AccountManagerProcessingPage.h"
#include "AccountManagerInfoPage.h"
#include "AccountInfoPage.h"
#include "CompletionErrorPage.h"

////@begin XPM images
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
////@end XPM images

/*!
 * CAccountManagerProcessingPage custom event definition
 */

DEFINE_EVENT_TYPE(wxEVT_ACCOUNTMANAGERPROCESSING_STATECHANGE)

/*!
 * CAccountManagerProcessingPage type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CAccountManagerProcessingPage, wxWizardPageEx )

/*!
 * CAccountManagerProcessingPage event table definition
 */

BEGIN_EVENT_TABLE( CAccountManagerProcessingPage, wxWizardPageEx )

    EVT_ACCOUNTMANAGERPROCESSING_STATECHANGE( CAccountManagerProcessingPage::OnStateChange )

////@begin CAccountManagerProcessingPage event table entries
    EVT_WIZARDEX_PAGE_CHANGED( -1, CAccountManagerProcessingPage::OnPageChanged )
    EVT_WIZARDEX_CANCEL( -1, CAccountManagerProcessingPage::OnCancel )

////@end CAccountManagerProcessingPage event table entries

END_EVENT_TABLE()

/*!
 * CAccountManagerProcessingPage constructors
 */

CAccountManagerProcessingPage::CAccountManagerProcessingPage( )
{
}

CAccountManagerProcessingPage::CAccountManagerProcessingPage( CBOINCBaseWizard* parent )
{
    Create( parent );
}

/*!
 * CAttachProjectPage creator
 */

bool CAccountManagerProcessingPage::Create( CBOINCBaseWizard* parent )
{
////@begin CAccountManagerProcessingPage member initialisation
    m_pTitleStaticCtrl = NULL;
    m_pPleaseWaitStaticCtrl = NULL;
    m_pProgressIndicator = NULL;
////@end CAccountManagerProcessingPage member initialisation

    m_bProjectCommunicationsSucceeded = false;
    m_bProjectUnavailable = false;
    m_bProjectAccountNotFound = false;
    m_bProjectAccountAlreadyExists = false;
    m_iBitmapIndex = 0;
    m_iCurrentState = ATTACHACCTMGR_INIT;

////@begin CAccountManagerProcessingPage creation
    wxWizardPageEx::Create( parent, ID_ACCOUNTMANAGERPROCESSINGPAGE );

    CreateControls();
    GetSizer()->Fit(this);
////@end CAccountManagerProcessingPage creation
    return TRUE;
}

/*!
 * Control creation for CAttachProjectPage
 */

void CAccountManagerProcessingPage::CreateControls()
{
////@begin CAccountManagerProcessingPage content construction
    CAccountManagerProcessingPage* itemWizardPage36 = this;

    wxBoxSizer* itemBoxSizer37 = new wxBoxSizer(wxVERTICAL);
    itemWizardPage36->SetSizer(itemBoxSizer37);

    m_pTitleStaticCtrl = new wxStaticText;
    m_pTitleStaticCtrl->Create( itemWizardPage36, wxID_STATIC, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    m_pTitleStaticCtrl->SetFont(wxFont(10, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD, FALSE, _T("Verdana")));
    itemBoxSizer37->Add(m_pTitleStaticCtrl, 0, wxALIGN_LEFT|wxALL, 5);

    m_pPleaseWaitStaticCtrl = new wxStaticText;
    m_pPleaseWaitStaticCtrl->Create( itemWizardPage36, wxID_STATIC, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer37->Add(m_pPleaseWaitStaticCtrl, 0, wxALIGN_LEFT|wxALL, 5);

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
////@end CAccountManagerProcessingPage content construction
}

/*!
 * wxEVT_WIZARD_PAGE_CHANGED event handler for ID_ATTACHPROJECTPAGE
 */

void CAccountManagerProcessingPage::OnPageChanged( wxWizardExEvent& event )
{
    if (event.GetDirection() == false) return;

    CWizardAttach* pWA = ((CWizardAttach*)GetParent());

    wxASSERT(m_pTitleStaticCtrl);
    wxASSERT(m_pPleaseWaitStaticCtrl);
    wxASSERT(m_pProgressIndicator);
    wxASSERT(pWA);

    if (!pWA->GetProjectName().IsEmpty()) {
        wxString str;

        // %s is the project name
        //    i.e. 'BOINC', 'GridRepublic'
        str.Printf(_("Communicating with %s."), pWA->GetProjectName().c_str());

        m_pTitleStaticCtrl->SetLabel(
            str
        );
    } else {
        m_pTitleStaticCtrl->SetLabel(
            _("Communicating with server.")
        );
    }

    m_pPleaseWaitStaticCtrl->SetLabel(
        _("Please wait...")
    );

    SetProjectCommunicationsSucceeded(false);
    SetProjectUnavailable(false);
    SetProjectAccountAlreadyExists(false);
    SetNextState(ATTACHACCTMGR_INIT);

    CAccountManagerProcessingPageEvent TransitionEvent(wxEVT_ACCOUNTMANAGERPROCESSING_STATECHANGE, this);
    AddPendingEvent(TransitionEvent);

    Fit();
}

/*!
 * wxEVT_WIZARD_CANCEL event handler for ID_ATTACHPROJECTPAGE
 */

void CAccountManagerProcessingPage::OnCancel( wxWizardExEvent& event ) {
    PROCESS_CANCELEVENT(event);
}

/*!
 * wxEVT_ACCOUNTCREATION_STATECHANGE event handler for ID_ACCOUNTCREATIONPAGE
 */

void CAccountManagerProcessingPage::OnStateChange( CAccountManagerProcessingPageEvent& WXUNUSED(event) )
{
    CMainDocument* pDoc = wxGetApp().GetDocument();
    CWizardAttach* pWA = ((CWizardAttach*)GetParent());
    wxDateTime dtStartExecutionTime;
    wxDateTime dtCurrentExecutionTime;
    wxTimeSpan tsExecutionTime;
    ACCT_MGR_RPC_REPLY reply;
    wxString strBuffer = wxEmptyString;
    std::string url = "";
    std::string username = "";
    std::string password = "";
    bool bPostNewEvent = true;
    int iReturnValue = 0;
    unsigned int i;

    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));

    switch(GetCurrentState()) {
        case ATTACHACCTMGR_INIT:
            pWA->DisableNextButton();
            pWA->DisableBackButton();
            StartProgress(m_pProgressIndicator);
            SetNextState(ATTACHACCTMGR_ATTACHACCTMGR_BEGIN);
            break;
        case ATTACHACCTMGR_ATTACHACCTMGR_BEGIN:
            SetNextState(ATTACHACCTMGR_ATTACHACCTMGR_EXECUTE);
            break;
        case ATTACHACCTMGR_ATTACHACCTMGR_EXECUTE:
            // Attempt to attach to the account manager.

            // Newer versions of the server-side software contain the correct
            //   master url in the get_project_config response.  If it is available
            //   use it instead of what the user typed in.
            if (!pWA->project_config.master_url.empty()) {
                url = pWA->project_config.master_url;
            } else {
                url = (const char*)pWA->GetProjectURL().mb_str();
            }

            if (pWA->project_config.uses_username) {
                username = (const char*)pWA->GetAccountUsername().utf8_str();
            } else {
                username = (const char*)pWA->GetAccountEmailAddress().mb_str();
            }
            password = (const char*)pWA->GetAccountPassword().mb_str();

            // Wait until we are done processing the request.
            dtStartExecutionTime = wxDateTime::Now();
            dtCurrentExecutionTime = wxDateTime::Now();
            tsExecutionTime = dtCurrentExecutionTime - dtStartExecutionTime;
            iReturnValue = 0;
            reply.error_num = ERR_RETRY;
            while (
                !iReturnValue &&
                ((ERR_IN_PROGRESS == reply.error_num) || (ERR_RETRY == reply.error_num)) &&
                (tsExecutionTime.GetSeconds() <= 60) &&
                !CHECK_CLOSINGINPROGRESS()
            ) {
                if (ERR_RETRY == reply.error_num) {
                    pDoc->rpc.acct_mgr_rpc(
                        url.c_str(),
                        username.c_str(),
                        password.c_str(),
                        pWA->IsCredentialsCached()
                    );
                }

                dtCurrentExecutionTime = wxDateTime::Now();
                tsExecutionTime = dtCurrentExecutionTime - dtStartExecutionTime;
                iReturnValue = pDoc->rpc.acct_mgr_rpc_poll(reply);

                IncrementProgress(m_pProgressIndicator);

                ::wxMilliSleep(500);
                wxEventLoopBase::GetActive()->YieldFor(wxEVT_CATEGORY_USER_INPUT);
            }

            if (!iReturnValue && (!reply.error_num && reply.messages.size() == 0)) {
                SetProjectAttachSucceeded(true);
                pWA->SetAttachedToProjectSuccessfully(true);
            } else {
                SetProjectAttachSucceeded(false);

                if ((ERR_NOT_FOUND == reply.error_num) ||
                    (ERR_DB_NOT_FOUND == reply.error_num) ||
                    (ERR_BAD_EMAIL_ADDR == reply.error_num) ||
                    (ERR_BAD_PASSWD == reply.error_num)
                ) {
                    // For any logon error, make sure we do not attempt to use cached credentials
                    //   on any follow-ups.
                    pWA->SetCredentialsCached(false);
                    SetProjectAccountNotFound(true);
                } else {
                    SetProjectAccountNotFound(false);
                }

                strBuffer = pWA->m_CompletionErrorPage->m_pServerMessagesCtrl->GetLabel();
                if ((HTTP_STATUS_INTERNAL_SERVER_ERROR == reply.error_num)) {
                    strBuffer +=
                        _("An internal server error has occurred.\n");
                } else {
                    for (i=0; i<reply.messages.size(); i++) {
                        strBuffer += wxString(reply.messages[i].c_str(), wxConvUTF8) + wxString(wxT("\n"));
                    }
                }
                pWA->m_CompletionErrorPage->m_pServerMessagesCtrl->SetLabel(strBuffer);
            }
            SetNextState(ATTACHACCTMGR_CLEANUP);
            break;
        case ATTACHACCTMGR_CLEANUP:
            FinishProgress(m_pProgressIndicator);
            SetNextState(ATTACHACCTMGR_END);
            break;
        default:
            // Allow a glimpse of what the result was before advancing to the next page.
            wxSleep(1);
            pWA->EnableNextButton();
            pWA->EnableBackButton();
            pWA->SimulateNextButton();
            bPostNewEvent = false;
            break;
    }

    Update();

    if (bPostNewEvent && !CHECK_CLOSINGINPROGRESS()) {
        CAccountManagerProcessingPageEvent TransitionEvent(wxEVT_ACCOUNTMANAGERPROCESSING_STATECHANGE, this);
        AddPendingEvent(TransitionEvent);
    }
}

/*!
 * Gets the previous page.
 */

wxWizardPageEx* CAccountManagerProcessingPage::GetPrev() const
{
    return PAGE_TRANSITION_BACK;
}

/*!
 * Gets the next page.
 */

wxWizardPageEx* CAccountManagerProcessingPage::GetNext() const
{
    if (CHECK_CLOSINGINPROGRESS()) {
        // Cancel Event Detected
        return PAGE_TRANSITION_NEXT(ID_COMPLETIONERRORPAGE);
    } else if (GetProjectAttachSucceeded()) {
        // We were successful in creating or retrieving an account
        return PAGE_TRANSITION_NEXT(ID_COMPLETIONPAGE);
    } else if (!GetProjectCommunicationsSucceeded() && GetProjectAccountNotFound()) {
        // The requested account does not exist or the password is bad
        return PAGE_TRANSITION_NEXT(ID_ERRNOTFOUNDPAGE);
    } else {
        // The project much be down for maintenance
        return PAGE_TRANSITION_NEXT(ID_COMPLETIONERRORPAGE);
    }
}

/*!
 * Should we show tooltips?
 */

bool CAccountManagerProcessingPage::ShowToolTips()
{
    return TRUE;
}

void CAccountManagerProcessingPage::StartProgress(wxStaticBitmap* pBitmap) {
    m_iBitmapIndex = 1;
    pBitmap->SetBitmap(GetBitmapResource(wxT("res/wizprogress01.xpm")));
}

void CAccountManagerProcessingPage::IncrementProgress(wxStaticBitmap* pBitmap) {
    m_iBitmapIndex += 1;
    if (12 < m_iBitmapIndex) m_iBitmapIndex = 1;

    wxString str;
    str.Printf(wxT("res/wizprogress%02d.xpm"), m_iBitmapIndex);

    pBitmap->SetBitmap(GetBitmapResource(str));
    Update();
}

void CAccountManagerProcessingPage::FinishProgress(wxStaticBitmap* pBitmap) {
    m_iBitmapIndex = 12;
    pBitmap->SetBitmap(GetBitmapResource(wxT("res/wizprogress12.xpm")));
}

/*!
 * Get bitmap resources
 */

wxBitmap CAccountManagerProcessingPage::GetBitmapResource( const wxString& name )
{
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

/*!
 * Get icon resources
 */

wxIcon CAccountManagerProcessingPage::GetIconResource( const wxString& WXUNUSED(name) )
{
    // Icon retrieval
////@begin CAccountManagerProcessingPage icon retrieval
    return wxNullIcon;
////@end CAccountManagerProcessingPage icon retrieval
}

