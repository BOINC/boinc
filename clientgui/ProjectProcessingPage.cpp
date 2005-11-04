// Berkeley Open Infrastructure for Network Computing
// http://boinc.berkeley.edu
// Copyright (C) 2005 University of California
//
// This is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation;
// either version 2.1 of the License, or (at your option) any later version.
//
// This software is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//
// To view the GNU Lesser General Public License visit
// http://www.gnu.org/copyleft/lesser.html
// or write to the Free Software Foundation, Inc.,
// 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
#if defined(__GNUG__) && !defined(__APPLE__)
#pragma implementation "ProjectProcessingPage.h"
#endif

#include "stdwx.h"
#include "wizardex.h"
#include "error_numbers.h"
#include "BOINCGUIApp.h"
#include "BOINCWizards.h"
#include "BOINCBaseWizard.h"
#include "WizardAttachProject.h"
#include "ProjectProcessingPage.h"
#include "ProjectInfoPage.h"
#include "AccountKeyPage.h"
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
    m_ProgressIndicator = NULL;
////@end CProjectProcessingPage member initialisation
 
    m_bProjectCommunitcationsSucceeded = false;
    m_bProjectUnavailable = false;
    m_bProjectAccountNotFound = false;
    m_bProjectAccountAlreadyExists = false;
    m_iBitmapIndex = 0;
    m_iCurrentState = ATTACHPROJECT_INIT;
 
////@begin CProjectProcessingPage creation
    wxBitmap wizardBitmap(wxNullBitmap);
    wxWizardPageEx::Create( parent, wizardBitmap );

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
    CProjectProcessingPage* itemWizardPage71 = this;

    wxBoxSizer* itemBoxSizer72 = new wxBoxSizer(wxVERTICAL);
    itemWizardPage71->SetSizer(itemBoxSizer72);

    wxStaticText* itemStaticText73 = new wxStaticText;
    itemStaticText73->Create( itemWizardPage71, wxID_STATIC, _("Communicating with project \nPlease wait..."), wxDefaultPosition, wxDefaultSize, 0 );
    itemStaticText73->SetFont(wxFont(10, wxSWISS, wxNORMAL, wxBOLD, FALSE, _T("Verdana")));
    itemBoxSizer72->Add(itemStaticText73, 0, wxALIGN_LEFT|wxALL, 5);

    itemBoxSizer72->Add(5, 80, 0, wxALIGN_LEFT|wxALL, 5);

    wxFlexGridSizer* itemFlexGridSizer75 = new wxFlexGridSizer(1, 3, 0, 0);
    itemFlexGridSizer75->AddGrowableRow(0);
    itemFlexGridSizer75->AddGrowableCol(0);
    itemFlexGridSizer75->AddGrowableCol(1);
    itemFlexGridSizer75->AddGrowableCol(2);
    itemBoxSizer72->Add(itemFlexGridSizer75, 0, wxGROW|wxALL, 5);

    itemFlexGridSizer75->Add(5, 5, 0, wxGROW|wxGROW|wxALL, 5);

    wxBitmap m_AttachProjectProgressBitmap(itemWizardPage71->GetBitmapResource(wxT("res/wizprogress01.xpm")));
    m_ProgressIndicator = new wxStaticBitmap;
    m_ProgressIndicator->Create( itemWizardPage71, ID_PROGRESSCTRL, m_AttachProjectProgressBitmap, wxDefaultPosition, wxSize(184, 48), 0 );
    itemFlexGridSizer75->Add(m_ProgressIndicator, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    itemFlexGridSizer75->Add(5, 5, 0, wxGROW|wxGROW|wxALL, 5);

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
    } else if (!GetProjectCommunitcationsSucceeded() && GetProjectAccountAlreadyExists()) {
        // The requested account already exists
        return PAGE_TRANSITION_NEXT(ID_ERRALREADYEXISTSPAGE);
    } else if (!GetProjectCommunitcationsSucceeded() && GetProjectAccountNotFound()) {
        // The requested account does not exist or the password is bad
        return PAGE_TRANSITION_NEXT(ID_ERRNOTFOUNDPAGE);
    } else {
        // Ann error must have occurred
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
 
wxIcon CProjectProcessingPage::GetIconResource( const wxString& name )
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

    SetProjectCommunitcationsSucceeded(false);
    SetProjectUnavailable(false);
    SetProjectAccountAlreadyExists(false);
    SetNextState(ATTACHPROJECT_INIT);
 
    CProjectProcessingPageEvent TransitionEvent(wxEVT_PROJECTPROCESSING_STATECHANGE, this);
    AddPendingEvent(TransitionEvent);
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
 
void CProjectProcessingPage::OnStateChange( CProjectProcessingPageEvent& event )
{
    CMainDocument* pDoc      = wxGetApp().GetDocument();
    ACCOUNT_IN* ai           = &((CWizardAttachProject*)GetParent())->account_in;
    ACCOUNT_OUT* ao          = &((CWizardAttachProject*)GetParent())->account_out;
    unsigned int i;
    PROJECT_ATTACH_REPLY reply;
    wxString strBuffer = wxEmptyString;
    wxDateTime dtStartExecutionTime;
    wxDateTime dtCurrentExecutionTime;
    wxTimeSpan tsExecutionTime;
    bool bPostNewEvent = true;
    int iReturnValue = 0;
 
    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));
 
    switch(GetCurrentState()) {
        case ATTACHPROJECT_INIT:
            ((CWizardAttachProject*)GetParent())->DisableNextButton();
            ((CWizardAttachProject*)GetParent())->DisableBackButton();

            StartProgress(m_ProgressIndicator);
            SetNextState(ATTACHPROJECT_ACCOUNTQUERY_BEGIN);
            break;
        case ATTACHPROJECT_ACCOUNTQUERY_BEGIN:
            SetNextState(ATTACHPROJECT_ACCOUNTQUERY_EXECUTE);
            break;
        case ATTACHPROJECT_ACCOUNTQUERY_EXECUTE:
            // Attempt to create the account or reterieve the authenticator.
            ai->clear();
            ao->clear();

            ai->url = ((CWizardAttachProject*)GetParent())->m_ProjectInfoPage->GetProjectURL().c_str();

            if (!((CWizardAttachProject*)GetParent())->m_AccountKeyPage->m_strAccountKey.IsEmpty() ||
                ((CWizardAttachProject*)GetParent())->m_bCredentialsCached
            ) {
                if (!((CWizardAttachProject*)GetParent())->m_bCredentialsCached) {
                    ao->authenticator = ((CWizardAttachProject*)GetParent())->m_AccountKeyPage->m_strAccountKey.c_str();
                }
                SetProjectCommunitcationsSucceeded(true);
            } else {
                if (((CWizardAttachProject*)GetParent())->m_AccountInfoPage->m_AccountCreateCtrl->GetValue()) {
                    if (!((CWizardAttachProject*)GetParent())->project_config.uses_username) {
                        ai->email_addr = ((CWizardAttachProject*)GetParent())->m_AccountInfoPage->GetAccountEmailAddress().c_str();
                        ai->user_name = ::wxGetUserName().c_str();
                    } else {
                        ai->email_addr = wxT("");
                        ai->user_name = ((CWizardAttachProject*)GetParent())->m_AccountInfoPage->GetAccountEmailAddress().c_str();
                    }
                    ai->passwd = ((CWizardAttachProject*)GetParent())->m_AccountInfoPage->GetAccountPassword().c_str();
                    pDoc->rpc.create_account(*ai);

                    // Wait until we are done processing the request.
                    dtStartExecutionTime = wxDateTime::Now();
                    dtCurrentExecutionTime = wxDateTime::Now();
                    tsExecutionTime = dtCurrentExecutionTime - dtStartExecutionTime;
                    iReturnValue = ERR_IN_PROGRESS;
                    while (ERR_IN_PROGRESS == iReturnValue &&
                        tsExecutionTime.GetSeconds() <= 60 &&
                        !((CWizardAttachProject*)GetParent())->IsCancelInProgress()
                        )
                    {
                        dtCurrentExecutionTime = wxDateTime::Now();
                        tsExecutionTime = dtCurrentExecutionTime - dtStartExecutionTime;
                        iReturnValue = pDoc->rpc.create_account_poll(*ao);

                        IncrementProgress(m_ProgressIndicator);

                        ::wxMilliSleep(500);
                        ::wxSafeYield(GetParent());
                    }

                    if ((!iReturnValue) && !ao->error_num && !CHECK_DEBUG_FLAG(WIZDEBUG_ERRPROJECTCOMM)) {
                        ((CWizardAttachProject*)GetParent())->SetAccountCreatedSuccessfully(true);
                    }
                } else {
                    if (!((CWizardAttachProject*)GetParent())->project_config.uses_username) {
                        ai->email_addr = ((CWizardAttachProject*)GetParent())->m_AccountInfoPage->GetAccountEmailAddress().c_str();
                    } else {
                        ai->user_name = ((CWizardAttachProject*)GetParent())->m_AccountInfoPage->GetAccountEmailAddress().c_str();
                    }
                    ai->passwd = ((CWizardAttachProject*)GetParent())->m_AccountInfoPage->GetAccountPassword().c_str();

                    pDoc->rpc.lookup_account(*ai);
 
                    // Wait until we are done processing the request.
                    dtStartExecutionTime = wxDateTime::Now();
                    dtCurrentExecutionTime = wxDateTime::Now();
                    tsExecutionTime = dtCurrentExecutionTime - dtStartExecutionTime;
                    iReturnValue = ERR_IN_PROGRESS;
                    while (ERR_IN_PROGRESS == iReturnValue &&
                        tsExecutionTime.GetSeconds() <= 60 &&
                        !CHECK_CLOSINGINPROGRESS()
                        )
                    {
                        dtCurrentExecutionTime = wxDateTime::Now();
                        tsExecutionTime = dtCurrentExecutionTime - dtStartExecutionTime;
                        iReturnValue = pDoc->rpc.lookup_account_poll(*ao);

                        IncrementProgress(m_ProgressIndicator);

                        ::wxMilliSleep(500);
                        ::wxSafeYield(GetParent());
                    }
                }
 
                if ((!iReturnValue) && !ao->error_num && !CHECK_DEBUG_FLAG(WIZDEBUG_ERRPROJECTCOMM)) {
                    SetProjectCommunitcationsSucceeded(true);
                } else {
                    SetProjectCommunitcationsSucceeded(false);
                    if ((ERR_NONUNIQUE_EMAIL == ao->error_num) || CHECK_DEBUG_FLAG(WIZDEBUG_ERRACCOUNTALREADYEXISTS)) {
                        SetProjectAccountAlreadyExists(true);
                    } else {
                        SetProjectAccountAlreadyExists(false);
                    }
                    if ((ERR_NOT_FOUND == ao->error_num) || CHECK_DEBUG_FLAG(WIZDEBUG_ERRACCOUNTNOTFOUND)) {
                        SetProjectAccountNotFound(true);
                    } else {
                        SetProjectAccountNotFound(false);
                    }
                    if ((HTTP_STATUS_NOT_FOUND == ao->error_num) || CHECK_DEBUG_FLAG(WIZDEBUG_ERRPROJECTPROPERTIESURL)) {
                        wxString strBuffer = ((CWizardAttachProject*)GetParent())->m_CompletionErrorPage->m_ServerMessages->GetLabel();
                        strBuffer += _T("Required wizard file(s) are missing from the target server.\n(lookup_account.php/create_account.php)\n");
                        ((CWizardAttachProject*)GetParent())->m_CompletionErrorPage->m_ServerMessages->SetLabel(strBuffer);
                    }
                    if ((HTTP_STATUS_INTERNAL_SERVER_ERROR == ao->error_num) || CHECK_DEBUG_FLAG(WIZDEBUG_ERRPROJECTPROPERTIESURL)) {
                        wxString strBuffer = ((CWizardAttachProject*)GetParent())->m_CompletionErrorPage->m_ServerMessages->GetLabel();
                        strBuffer += _T("An internal server error has occurred.\n");
                        ((CWizardAttachProject*)GetParent())->m_CompletionErrorPage->m_ServerMessages->SetLabel(strBuffer);
                    }
                }
            }
            SetNextState(ATTACHPROJECT_ATTACHPROJECT_BEGIN);
            break;
        case ATTACHPROJECT_ATTACHPROJECT_BEGIN:
            SetNextState(ATTACHPROJECT_ATTACHPROJECT_EXECUTE);
            break;
        case ATTACHPROJECT_ATTACHPROJECT_EXECUTE:
            if (GetProjectCommunitcationsSucceeded()) {
                // Attempt to attach to the project.
                if (((CWizardAttachProject*)GetParent())->m_bCredentialsCached) {
                    pDoc->rpc.project_attach(
                        ai->url.c_str(),
                        ao->authenticator.c_str(),
                        true
                    );
                } else {
                    pDoc->rpc.project_attach(
                        ai->url.c_str(),
                        ao->authenticator.c_str()
                    );
                }
     
                // Wait until we are done processing the request.
                dtStartExecutionTime = wxDateTime::Now();
                dtCurrentExecutionTime = wxDateTime::Now();
                tsExecutionTime = dtCurrentExecutionTime - dtStartExecutionTime;
                iReturnValue = 0;
                reply.error_num = ERR_IN_PROGRESS;
                while ((!iReturnValue && (ERR_IN_PROGRESS == reply.error_num)) &&
                    tsExecutionTime.GetSeconds() <= 60 &&
                    !CHECK_CLOSINGINPROGRESS()
                    )
                {
                    dtCurrentExecutionTime = wxDateTime::Now();
                    tsExecutionTime = dtCurrentExecutionTime - dtStartExecutionTime;
                    iReturnValue = pDoc->rpc.project_attach_poll(reply);

                    IncrementProgress(m_ProgressIndicator);

                    ::wxMilliSleep(500);
                    ::wxSafeYield(GetParent());
                }
     
                if (!iReturnValue && !reply.error_num && !CHECK_DEBUG_FLAG(WIZDEBUG_ERRPROJECTATTACH)) {
                    SetProjectAttachSucceeded(true);
                    ((CWizardAttachProject*)GetParent())->SetAttachedToProjectSuccessfully(true);
                    ((CWizardAttachProject*)GetParent())->SetProjectURL(ai->url.c_str());
                    ((CWizardAttachProject*)GetParent())->SetProjectAuthenticator(ao->authenticator.c_str());
                } else {
                    SetProjectAttachSucceeded(false);
                    if ((HTTP_STATUS_INTERNAL_SERVER_ERROR == reply.error_num) || CHECK_DEBUG_FLAG(WIZDEBUG_ERRPROJECTPROPERTIESURL)) {
                        strBuffer = ((CWizardAttachProject*)GetParent())->m_CompletionErrorPage->m_ServerMessages->GetLabel();
                        strBuffer += _T("An internal server error has occurred.\n");
                        ((CWizardAttachProject*)GetParent())->m_CompletionErrorPage->m_ServerMessages->SetLabel(strBuffer);
                    } else {
                        strBuffer = ((CWizardAttachProject*)GetParent())->m_CompletionErrorPage->m_ServerMessages->GetLabel();
                        for (i=0; i<reply.messages.size(); i++) {
                            strBuffer += wxString(reply.messages[i].c_str()) + wxString(wxT("\n"));
                        }
                        ((CWizardAttachProject*)GetParent())->m_CompletionErrorPage->m_ServerMessages->SetLabel(strBuffer);
                    }
                }
            } else {
                SetProjectAttachSucceeded(false);
            }
            SetNextState(ATTACHPROJECT_CLEANUP);
            break;
        case ATTACHPROJECT_CLEANUP:
            FinishProgress(m_ProgressIndicator);
            SetNextState(ATTACHPROJECT_END);
            break;
        default:
            // Allow a glimps of what the result was before advancing to the next page.
            wxSleep(1);
            ((CWizardAttachProject*)GetParent())->EnableNextButton();
            ((CWizardAttachProject*)GetParent())->EnableBackButton();
            ((CWizardAttachProject*)GetParent())->SimulateNextButton();
            bPostNewEvent = false;
            break;
    }
 
    Update();
 
    if (bPostNewEvent && !CHECK_CLOSINGINPROGRESS()) {
        CProjectProcessingPageEvent TransitionEvent(wxEVT_PROJECTPROCESSING_STATECHANGE, this);
        AddPendingEvent(TransitionEvent);
    }
}

