// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2008 University of California
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
// along with BOINC.  If not, see <http://www.gnu.org/licenses/>.
//
#if defined(__GNUG__) && !defined(__APPLE__)
#pragma implementation "ProjectProcessingPage.h"
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
#include "ProjectProcessingPage.h"
#include "ProjectInfoPage.h"
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
 * CProjectPropertiesPage custom event definition
 */

DEFINE_EVENT_TYPE(wxEVT_PROJECTPROCESSING_STATECHANGE)
  
/*!
 * CProjectProcessingPage type definition
 */
 
IMPLEMENT_DYNAMIC_CLASS( CProjectProcessingPage, wxWizardPageEx )
  
/*!
 * CProjectProcessingPage event table definition
 */
 
BEGIN_EVENT_TABLE( CProjectProcessingPage, wxWizardPageEx )
 
    EVT_PROJECTPROCESSING_STATECHANGE( CProjectProcessingPage::OnStateChange )
 
////@begin CProjectProcessingPage event table entries
    EVT_WIZARDEX_PAGE_CHANGED( -1, CProjectProcessingPage::OnPageChanged )
    EVT_WIZARDEX_CANCEL( -1, CProjectProcessingPage::OnCancel )

////@end CProjectProcessingPage event table entries
 
END_EVENT_TABLE()
  
/*!
 * CProjectProcessingPage constructors
 */
 
CProjectProcessingPage::CProjectProcessingPage( )
{
}
  
CProjectProcessingPage::CProjectProcessingPage( CBOINCBaseWizard* parent )
{
    Create( parent );
}
  
/*!
 * CProjectPropertiesPage creator
 */
 
bool CProjectProcessingPage::Create( CBOINCBaseWizard* parent )
{
 
////@begin CProjectProcessingPage member initialisation
    m_pTitleStaticCtrl = NULL;
    m_pProgressIndicator = NULL;
////@end CProjectProcessingPage member initialisation
 
    m_bProjectCommunicationsSucceeded = false;
    m_bProjectUnavailable = false;
    m_bProjectAccountNotFound = false;
    m_bProjectAccountAlreadyExists = false;
    m_iBitmapIndex = 0;
    m_iCurrentState = ATTACHPROJECT_INIT;
 
////@begin CProjectProcessingPage creation
    wxWizardPageEx::Create( parent, ID_PROJECTPROCESSINGPAGE );

    CreateControls();
    GetSizer()->Fit(this);
////@end CProjectProcessingPage creation
 
    return TRUE;
}
 
/*!
 * Control creation for CProjectPropertiesPage
 */
 
void CProjectProcessingPage::CreateControls()
{    
////@begin CProjectProcessingPage content construction
    CProjectProcessingPage* itemWizardPage36 = this;

    wxBoxSizer* itemBoxSizer37 = new wxBoxSizer(wxVERTICAL);
    itemWizardPage36->SetSizer(itemBoxSizer37);

    m_pTitleStaticCtrl = new wxStaticText;
    m_pTitleStaticCtrl->Create( itemWizardPage36, wxID_STATIC, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    m_pTitleStaticCtrl->SetFont(wxFont(10, wxSWISS, wxNORMAL, wxBOLD, FALSE, _T("Verdana")));
    itemBoxSizer37->Add(m_pTitleStaticCtrl, 0, wxALIGN_LEFT|wxALL, 5);

    itemBoxSizer37->Add(5, 80, 0, wxALIGN_LEFT|wxALL, 5);

    wxFlexGridSizer* itemFlexGridSizer40 = new wxFlexGridSizer(1, 3, 0, 0);
    itemFlexGridSizer40->AddGrowableRow(0);
    itemFlexGridSizer40->AddGrowableCol(0);
    itemFlexGridSizer40->AddGrowableCol(1);
    itemFlexGridSizer40->AddGrowableCol(2);
    itemBoxSizer37->Add(itemFlexGridSizer40, 0, wxGROW|wxALL, 5);

    itemFlexGridSizer40->Add(5, 5, 0, wxGROW|wxGROW|wxALL, 5);

    wxBitmap itemBitmap41(GetBitmapResource(wxT("res/wizprogress01.xpm")));
    m_pProgressIndicator = new wxStaticBitmap;
    m_pProgressIndicator->Create( itemWizardPage36, ID_PROGRESSCTRL, itemBitmap41, wxDefaultPosition, wxSize(184, 48), 0 );
    itemFlexGridSizer40->Add(m_pProgressIndicator, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    itemFlexGridSizer40->Add(5, 5, 0, wxGROW|wxGROW|wxALL, 5);
////@end CProjectProcessingPage content construction
}
  
/*!
 * Gets the previous page.
 */
 
wxWizardPageEx* CProjectProcessingPage::GetPrev() const
{
    return PAGE_TRANSITION_BACK;
}
  
/*!
 * Gets the next page.
 */
 
wxWizardPageEx* CProjectProcessingPage::GetNext() const
{
    if (CHECK_CLOSINGINPROGRESS()) {
        // Cancel Event Detected
        return PAGE_TRANSITION_NEXT(ID_COMPLETIONERRORPAGE);
    } else if (GetProjectAttachSucceeded()) {
        // We were successful in creating or retrieving an account
        return PAGE_TRANSITION_NEXT(ID_COMPLETIONPAGE);
    } else if (!GetProjectCommunicationsSucceeded() && GetProjectAccountAlreadyExists()) {
        // The requested account already exists
        return PAGE_TRANSITION_NEXT(ID_ERRALREADYEXISTSPAGE);
    } else if (!GetProjectCommunicationsSucceeded() && GetProjectAccountNotFound()) {
        // The requested account does not exist or the password is bad
        return PAGE_TRANSITION_NEXT(ID_ERRNOTFOUNDPAGE);
    } else {
        // An error must have occurred
        return PAGE_TRANSITION_NEXT(ID_COMPLETIONERRORPAGE);
    } 
    return NULL;
}
  
/*!
 * Should we show tooltips?
 */
 
bool CProjectProcessingPage::ShowToolTips()
{
    return TRUE;
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
 
/*!
 * Get bitmap resources
 */
 
wxBitmap CProjectProcessingPage::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
    if (name == wxT("res/wizprogress01.xpm"))
    {
        wxBitmap bitmap(wizprogress01_xpm);
        return bitmap;
    }
    else if (name == wxT("res/wizprogress02.xpm"))
    {
        wxBitmap bitmap(wizprogress02_xpm);
        return bitmap;
    }
    else if (name == wxT("res/wizprogress03.xpm"))
    {
        wxBitmap bitmap(wizprogress03_xpm);
        return bitmap;
    }
    else if (name == wxT("res/wizprogress04.xpm"))
    {
        wxBitmap bitmap(wizprogress04_xpm);
        return bitmap;
    }
    else if (name == wxT("res/wizprogress05.xpm"))
    {
        wxBitmap bitmap(wizprogress05_xpm);
        return bitmap;
    }
    else if (name == wxT("res/wizprogress06.xpm"))
    {
        wxBitmap bitmap(wizprogress06_xpm);
        return bitmap;
    }
    else if (name == wxT("res/wizprogress07.xpm"))
    {
        wxBitmap bitmap(wizprogress07_xpm);
        return bitmap;
    }
    else if (name == wxT("res/wizprogress08.xpm"))
    {
        wxBitmap bitmap(wizprogress08_xpm);
        return bitmap;
    }
    else if (name == wxT("res/wizprogress09.xpm"))
    {
        wxBitmap bitmap(wizprogress09_xpm);
        return bitmap;
    }
    else if (name == wxT("res/wizprogress10.xpm"))
    {
        wxBitmap bitmap(wizprogress10_xpm);
        return bitmap;
    }
    else if (name == wxT("res/wizprogress11.xpm"))
    {
        wxBitmap bitmap(wizprogress11_xpm);
        return bitmap;
    }
    else if (name == wxT("res/wizprogress12.xpm"))
    {
        wxBitmap bitmap(wizprogress12_xpm);
        return bitmap;
    }
    return wxNullBitmap;
}
  
/*!
 * Get icon resources
 */
 
wxIcon CProjectProcessingPage::GetIconResource( const wxString& WXUNUSED(name) )
{
    // Icon retrieval
////@begin CProjectProcessingPage icon retrieval
    return wxNullIcon;
////@end CProjectProcessingPage icon retrieval
}
  
/*!
 * wxEVT_WIZARD_PAGE_CHANGED event handler for ID_ATTACHPROJECTPAGE
 */
 
void CProjectProcessingPage::OnPageChanged( wxWizardExEvent& event ) {
    if (event.GetDirection() == false) return;
 
    wxASSERT(m_pTitleStaticCtrl);
    wxASSERT(m_pProgressIndicator);

    m_pTitleStaticCtrl->SetLabel(
        _("Communicating with project.")
    );

    SetProjectCommunicationsSucceeded(false);
    SetProjectUnavailable(false);
    SetProjectAccountAlreadyExists(false);
    SetNextState(ATTACHPROJECT_INIT);
 
    CProjectProcessingPageEvent TransitionEvent(wxEVT_PROJECTPROCESSING_STATECHANGE, this);
    AddPendingEvent(TransitionEvent);

    Fit();
}
  
/*!
 * wxEVT_WIZARD_CANCEL event handler for ID_ACCOUNTCREATIONPAGE
 */
 
void CProjectProcessingPage::OnCancel( wxWizardExEvent& event ) {
    PROCESS_CANCELEVENT(event);
}
 
/*!
 * wxEVT_ACCOUNTCREATION_STATECHANGE event handler for ID_ACCOUNTCREATIONPAGE
 */
 
void CProjectProcessingPage::OnStateChange( CProjectProcessingPageEvent& WXUNUSED(event) )
{
    CMainDocument* pDoc = wxGetApp().GetDocument();
    CWizardAttach* pWA  = ((CWizardAttach*)GetParent());
    ACCOUNT_IN* ai      = &pWA->account_in;
    ACCOUNT_OUT* ao     = &pWA->account_out;
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
            pWA->DisableNextButton();
            pWA->DisableBackButton();

            StartProgress(m_pProgressIndicator);
            SetNextState(ATTACHPROJECT_ACCOUNTQUERY_BEGIN);
            break;
        case ATTACHPROJECT_ACCOUNTQUERY_BEGIN:
            SetNextState(ATTACHPROJECT_ACCOUNTQUERY_EXECUTE);
            break;
        case ATTACHPROJECT_ACCOUNTQUERY_EXECUTE:
            // Attempt to create the account or reterieve the authenticator.
            ai->clear();
            ao->clear();

            // Newer versions of the server-side software contain the correct
            //   master url in the get_project_config response.  If it is available
            //   use it instead of what the user typed in.
            if (!pWA->project_config.master_url.empty()) {
                ai->url = pWA->project_config.master_url;
            } else {
                ai->url = (const char*)pWA->m_ProjectInfoPage->GetProjectURL().mb_str();
            }

            if (!pWA->GetProjectAuthenticator().IsEmpty() || 
                pWA->m_bCredentialsCached || 
                pWA->m_bCredentialsDetected
            ) {
                if (!pWA->m_bCredentialsCached || pWA->m_bCredentialsDetected) {
                    ao->authenticator = (const char*)pWA->GetProjectAuthenticator().mb_str();
                }
                SetProjectCommunicationsSucceeded(true);
            } else {
                // Setup initial values for both the create and lookup API

                if (pWA->project_config.uses_username) {
                    ai->email_addr = (const char*)pWA->m_AccountInfoPage->GetAccountUsername().mb_str();
                } else {
                    ai->email_addr = (const char*)pWA->m_AccountInfoPage->GetAccountEmailAddress().mb_str();
                }
                ai->passwd = (const char*)pWA->m_AccountInfoPage->GetAccountPassword().mb_str();
                ai->user_name = (const char*)::wxGetUserName().mb_str();
                if (ai->user_name.empty()) {
                    ai->user_name = (const char*)::wxGetUserId().mb_str();
                }
                //ai->team_name = (const char*)pWA->GetTeamName().mb_str();

                if (pWA->m_AccountInfoPage->m_pAccountCreateCtrl->GetValue()) {
					creating_account = true;

                    // Wait until we are done processing the request.
                    dtStartExecutionTime = wxDateTime::Now();
                    dtCurrentExecutionTime = wxDateTime::Now();
                    tsExecutionTime = dtCurrentExecutionTime - dtStartExecutionTime;
                    retval = 0;
                    ao->error_num = ERR_RETRY;
                    while (
                        !retval &&
                        ((ERR_IN_PROGRESS == ao->error_num) || (ERR_RETRY == ao->error_num)) && 
                        tsExecutionTime.GetSeconds() <= 60 &&
                        !CHECK_CLOSINGINPROGRESS()
                    ) {
                        if (ERR_RETRY == ao->error_num) {
                            pDoc->rpc.create_account(*ai);
                        }

                        dtCurrentExecutionTime = wxDateTime::Now();
                        tsExecutionTime = dtCurrentExecutionTime - dtStartExecutionTime;
                        retval = pDoc->rpc.create_account_poll(*ao);

                        IncrementProgress(m_pProgressIndicator);

                        ::wxMilliSleep(500);
                        ::wxSafeYield(GetParent());
                    }

                    if ((!retval) && !ao->error_num) {
                        pWA->SetAccountCreatedSuccessfully(true);
                    }
                } else {
					creating_account = false;
 
                    // Wait until we are done processing the request.
                    dtStartExecutionTime = wxDateTime::Now();
                    dtCurrentExecutionTime = wxDateTime::Now();
                    tsExecutionTime = dtCurrentExecutionTime - dtStartExecutionTime;
                    retval = 0;
                    ao->error_num = ERR_RETRY;
                    while (
                        !retval &&
                        ((ERR_IN_PROGRESS == ao->error_num) || (ERR_RETRY == ao->error_num)) && 
                        tsExecutionTime.GetSeconds() <= 60 &&
                        !CHECK_CLOSINGINPROGRESS()
                    ) {
                        if (ERR_RETRY == ao->error_num) {
                            retval = pDoc->rpc.lookup_account(*ai);
                            if (retval) {
                                // REPORT ERROR
                            }
                        }

                        dtCurrentExecutionTime = wxDateTime::Now();
                        tsExecutionTime = dtCurrentExecutionTime - dtStartExecutionTime;
                        retval = pDoc->rpc.lookup_account_poll(*ao);

                        IncrementProgress(m_pProgressIndicator);

                        ::wxMilliSleep(500);
                        ::wxSafeYield(GetParent());
                    }
                }
 

                if ((!retval) && !ao->error_num) {
                    SetProjectCommunicationsSucceeded(true);
                } else {
                    SetProjectCommunicationsSucceeded(false);

                    if ((ao->error_num == ERR_DB_NOT_UNIQUE)
						|| (ao->error_num == ERR_NONUNIQUE_EMAIL)
						|| (ao->error_num == ERR_BAD_PASSWD && creating_account)
					) {
                        SetProjectAccountAlreadyExists(true);
                    } else {
                        SetProjectAccountAlreadyExists(false);
                    }

                    if ((ERR_NOT_FOUND == ao->error_num) ||
						(ao->error_num == ERR_DB_NOT_FOUND) ||
                        (ERR_BAD_EMAIL_ADDR == ao->error_num) ||
                        (ERR_BAD_PASSWD == ao->error_num)
                    ) {
                        SetProjectAccountNotFound(true);
                    } else {
                        SetProjectAccountNotFound(false);
                    }

                    strBuffer = pWA->m_CompletionErrorPage->m_pServerMessagesCtrl->GetLabel();
                    if ((HTTP_STATUS_NOT_FOUND == ao->error_num)) {
                        strBuffer += 
                            _("Required files not found on the server.");
                    } else if ((HTTP_STATUS_INTERNAL_SERVER_ERROR == ao->error_num)) {
                        strBuffer += 
                            _("An internal server error has occurred.");
                    } else {
						if (ao->error_msg.size()) {
                            strBuffer += wxString(ao->error_msg.c_str(), wxConvUTF8) + wxString(wxT("\n"));
                        }
                    }
                    pWA->m_CompletionErrorPage->m_pServerMessagesCtrl->SetLabel(strBuffer);
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
                    !CHECK_CLOSINGINPROGRESS()
                ) {
                    if (ERR_RETRY == reply.error_num) {
                        if (pWA->m_bCredentialsCached) {
                            pDoc->rpc.project_attach_from_file();
                        } else {
                            pDoc->rpc.project_attach(
                                ai->url.c_str(),
                                ao->authenticator.c_str(),
                                pWA->project_config.name.c_str()
                            );
                        }
                    }

                    dtCurrentExecutionTime = wxDateTime::Now();
                    tsExecutionTime = dtCurrentExecutionTime - dtStartExecutionTime;
                    retval = pDoc->rpc.project_attach_poll(reply);

                    IncrementProgress(m_pProgressIndicator);

                    ::wxMilliSleep(500);
                    ::wxSafeYield(GetParent());
                }
     
                if (!retval && !reply.error_num) {
                    SetProjectAttachSucceeded(true);
                    pWA->SetAttachedToProjectSuccessfully(true);
                    pWA->SetProjectURL(wxString(ai->url.c_str(), wxConvUTF8));
                    pWA->SetProjectAuthenticator(wxString(ao->authenticator.c_str(), wxConvUTF8));
                } else {
                    SetProjectAttachSucceeded(false);

                    strBuffer = pWA->m_CompletionErrorPage->m_pServerMessagesCtrl->GetLabel();
                    if ((HTTP_STATUS_INTERNAL_SERVER_ERROR == reply.error_num)) {
                        strBuffer += 
                            _("An internal server error has occurred.");
                    } else {
                        for (i=0; i<reply.messages.size(); i++) {
                            strBuffer += wxString(reply.messages[i].c_str(), wxConvUTF8) + wxString(wxT("\n"));
                        }
                    }
                    pWA->m_CompletionErrorPage->m_pServerMessagesCtrl->SetLabel(strBuffer);
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
            // Allow a glimps of what the result was before advancing to the next page.
            wxSleep(1);
            pWA->EnableNextButton();
            pWA->EnableBackButton();
            pWA->SimulateNextButton();
            bPostNewEvent = false;
            break;
    }
 
    Update();
 
    if (bPostNewEvent && !CHECK_CLOSINGINPROGRESS()) {
        CProjectProcessingPageEvent TransitionEvent(wxEVT_PROJECTPROCESSING_STATECHANGE, this);
        AddPendingEvent(TransitionEvent);
    }
}

