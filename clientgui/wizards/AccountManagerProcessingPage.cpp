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
#pragma implementation "AccountManagerProcessingPage.h"
#endif

#include "stdwx.h"
#include "wizardex.h"
#include "error_numbers.h"
#include "BOINCGUIApp.h"
#include "BOINCWizards.h"
#include "BOINCBaseWizard.h"
#include "WizardAccountManager.h"
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
    m_ProgressIndicator = NULL;
////@end CAccountManagerProcessingPage member initialisation
 
    m_bProjectCommunitcationsSucceeded = false;
    m_bProjectUnavailable = false;
    m_bProjectAccountAlreadyExists = false;
    m_iBitmapIndex = 0;
    m_iCurrentState = ATTACHACCTMGR_INIT;
 
////@begin CAccountManagerProcessingPage creation
    wxBitmap wizardBitmap(wxNullBitmap);
    wxWizardPageEx::Create( parent, wizardBitmap );

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
    CAccountManagerProcessingPage* itemWizardPage51 = this;

    wxBoxSizer* itemBoxSizer52 = new wxBoxSizer(wxVERTICAL);
    itemWizardPage51->SetSizer(itemBoxSizer52);

    wxStaticText* itemStaticText53 = new wxStaticText;
    itemStaticText53->Create( itemWizardPage51, wxID_STATIC, _("Communicating with account manager\nPlease wait..."), wxDefaultPosition, wxDefaultSize, 0 );
    itemStaticText53->SetFont(wxFont(10, wxSWISS, wxNORMAL, wxBOLD, FALSE, _T("Verdana")));
    itemBoxSizer52->Add(itemStaticText53, 0, wxALIGN_LEFT|wxALL, 5);

    itemBoxSizer52->Add(5, 80, 0, wxALIGN_LEFT|wxALL, 5);

    wxFlexGridSizer* itemFlexGridSizer55 = new wxFlexGridSizer(1, 3, 0, 0);
    itemFlexGridSizer55->AddGrowableRow(0);
    itemFlexGridSizer55->AddGrowableCol(0);
    itemFlexGridSizer55->AddGrowableCol(1);
    itemFlexGridSizer55->AddGrowableCol(2);
    itemBoxSizer52->Add(itemFlexGridSizer55, 0, wxGROW|wxALL, 5);

    itemFlexGridSizer55->Add(5, 5, 0, wxGROW|wxGROW|wxALL, 5);

    wxBitmap m_AttachProjectProgressBitmap(itemWizardPage51->GetBitmapResource(wxT("res/wizprogress01.xpm")));
    m_ProgressIndicator = new wxStaticBitmap;
    m_ProgressIndicator->Create( itemWizardPage51, ID_PROGRESSCTRL, m_AttachProjectProgressBitmap, wxDefaultPosition, wxSize(184, 48), 0 );
    itemFlexGridSizer55->Add(m_ProgressIndicator, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    itemFlexGridSizer55->Add(5, 5, 0, wxGROW|wxGROW|wxALL, 5);

////@end CAccountManagerProcessingPage content construction
}

/*!
 * wxEVT_WIZARD_PAGE_CHANGED event handler for ID_ATTACHPROJECTPAGE
 */

void CAccountManagerProcessingPage::OnPageChanged( wxWizardExEvent& event )
{
    if (event.GetDirection() == false) return;

    SetProjectCommunitcationsSucceeded(false);
    SetProjectUnavailable(false);
    SetProjectAccountAlreadyExists(false);
    SetNextState(ATTACHACCTMGR_INIT);
 
    CAccountManagerProcessingPageEvent TransitionEvent(wxEVT_ACCOUNTMANAGERPROCESSING_STATECHANGE, this);
    AddPendingEvent(TransitionEvent);
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
 
void CAccountManagerProcessingPage::OnStateChange( CAccountManagerProcessingPageEvent& event )
{
    CMainDocument* pDoc      = wxGetApp().GetDocument();
    wxDateTime dtStartExecutionTime;
    wxDateTime dtCurrentExecutionTime;
    wxTimeSpan tsExecutionTime;
    ACCT_MGR_RPC_REPLY reply;
    wxString strBuffer = wxEmptyString;
    std::string url = "";
    std::string username = "";
    std::string password = "";
    bool bPostNewEvent = true;
    bool bSuccessfulCondition = false;
    int iReturnValue = 0;
    unsigned int i;
 
    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));
 
    switch(GetCurrentState()) {
        case ATTACHACCTMGR_INIT:
            ((CWizardAccountManager*)GetParent())->DisableNextButton();
            ((CWizardAccountManager*)GetParent())->DisableBackButton();

            StartProgress(m_ProgressIndicator);
            SetNextState(ATTACHACCTMGR_ATTACHACCTMGR_BEGIN);
            break;
        case ATTACHACCTMGR_ATTACHACCTMGR_BEGIN:
            SetNextState(ATTACHACCTMGR_ATTACHACCTMGR_EXECUTE);
            break;
        case ATTACHACCTMGR_ATTACHACCTMGR_EXECUTE:
            // Attempt to attach to the accout manager.
            url = ((CWizardAccountManager*)GetParent())->m_AccountManagerInfoPage->GetProjectURL().c_str();
            username = ((CWizardAccountManager*)GetParent())->m_AccountInfoPage->GetAccountEmailAddress().c_str();
            password = ((CWizardAccountManager*)GetParent())->m_AccountInfoPage->GetAccountPassword().c_str();
            pDoc->rpc.acct_mgr_rpc(
                url.c_str(),
                username.c_str(),
                password.c_str(),
                ((CWizardAccountManager*)GetParent())->m_bCredentialsCached
            );
    
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
                iReturnValue = pDoc->rpc.acct_mgr_rpc_poll(reply);

                IncrementProgress(m_ProgressIndicator);

                ::wxMilliSleep(500);
                ::wxSafeYield(GetParent());
            }
    
            if (!iReturnValue && !reply.error_num && !CHECK_DEBUG_FLAG(WIZDEBUG_ERRPROJECTATTACH)) {
                SetProjectAttachSucceeded(true);
            } else {
                SetProjectAttachSucceeded(false);
                if ((HTTP_STATUS_INTERNAL_SERVER_ERROR == reply.error_num) || CHECK_DEBUG_FLAG(WIZDEBUG_ERRPROJECTPROPERTIESURL)) {
                    strBuffer = ((CWizardAccountManager*)GetParent())->m_CompletionErrorPage->m_ServerMessages->GetLabel();
                    strBuffer += _T("An internal server error has occurred.\n");
                    ((CWizardAccountManager*)GetParent())->m_CompletionErrorPage->m_ServerMessages->SetLabel(strBuffer);
                } else {
                    strBuffer = ((CWizardAccountManager*)GetParent())->m_CompletionErrorPage->m_ServerMessages->GetLabel();
                    for (i=0; i<reply.messages.size(); i++) {
                        strBuffer += wxString(reply.messages[i].c_str()) + wxString(wxT("\n"));
                    }
                    ((CWizardAccountManager*)GetParent())->m_CompletionErrorPage->m_ServerMessages->SetLabel(strBuffer);
                }
            }
            SetNextState(ATTACHACCTMGR_CLEANUP);
            break;
        case ATTACHACCTMGR_CLEANUP:
            FinishProgress(m_ProgressIndicator);
            SetNextState(ATTACHACCTMGR_END);
            break;
        default:
            // Allow a glimps of what the result was before advancing to the next page.
            wxSleep(1);
            ((CWizardAccountManager*)GetParent())->EnableNextButton();
            ((CWizardAccountManager*)GetParent())->EnableBackButton();
            ((CWizardAccountManager*)GetParent())->SimulateNextButton();
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
    } else {
        // The project much be down for maintenance
        return PAGE_TRANSITION_NEXT(ID_COMPLETIONERRORPAGE);
    } 
    return NULL;
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

wxIcon CAccountManagerProcessingPage::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CAccountManagerProcessingPage icon retrieval
    return wxNullIcon;
////@end CAccountManagerProcessingPage icon retrieval
}

