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
#pragma implementation "AccountManagerPropertiesPage.h"
#endif

#include "stdwx.h"
#include "wizardex.h"
#include "error_numbers.h"
#include "network.h"
#include "BOINCGUIApp.h"
#include "BOINCWizards.h"
#include "BOINCBaseWizard.h"
#include "WizardAccountManager.h"
#include "AccountManagerPropertiesPage.h"
#include "AccountManagerInfoPage.h"


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
 * CAccountManagerPropertiesPage custom event definition
 */
 
DEFINE_EVENT_TYPE(wxEVT_ACCOUNTMANAGERPROPERTIES_STATECHANGE)
  
/*!
 * CAccountManagerPropertiesPage type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CAccountManagerPropertiesPage, wxWizardPageEx )

/*!
 * CAccountManagerPropertiesPage event table definition
 */

BEGIN_EVENT_TABLE( CAccountManagerPropertiesPage, wxWizardPageEx )
 
    EVT_ACCOUNTMANAGERPROPERTIES_STATECHANGE( CAccountManagerPropertiesPage::OnStateChange )
 
////@begin CAccountManagerPropertiesPage event table entries
    EVT_WIZARDEX_PAGE_CHANGED( -1, CAccountManagerPropertiesPage::OnPageChanged )
    EVT_WIZARDEX_CANCEL( -1, CAccountManagerPropertiesPage::OnCancel )

////@end CAccountManagerPropertiesPage event table entries

END_EVENT_TABLE()

/*!
 * CAccountManagerPropertiesPage constructors
 */

CAccountManagerPropertiesPage::CAccountManagerPropertiesPage( )
{
}

CAccountManagerPropertiesPage::CAccountManagerPropertiesPage( CBOINCBaseWizard* parent )
{
    Create( parent );
}

/*!
 * CProjectPropertiesPage creator
 */

bool CAccountManagerPropertiesPage::Create( CBOINCBaseWizard* parent )
{
////@begin CAccountManagerPropertiesPage member initialisation
    m_pTitleStaticCtrl = NULL;
    m_pProgressIndicator = NULL;
////@end CAccountManagerPropertiesPage member initialisation
 
    m_bProjectPropertiesSucceeded = false;
    m_bProjectPropertiesURLFailure = false;
    m_bProjectAccountCreationDisabled = false;
    m_bProjectClientAccountCreationDisabled = false;
    m_bCommunicateYahooSucceeded = false;
    m_bCommunicateGoogleSucceeded = false;
    m_bDeterminingConnectionStatusSucceeded = false;
    m_iBitmapIndex = 0;
    m_iCurrentState = ACCTMGRPROP_INIT;
 
////@begin CAccountManagerPropertiesPage creation
    wxBitmap wizardBitmap(wxNullBitmap);
    wxWizardPageEx::Create( parent, wizardBitmap );

    CreateControls();
    GetSizer()->Fit(this);
////@end CAccountManagerPropertiesPage creation
    return TRUE;
}

/*!
 * Control creation for CProjectPropertiesPage
 */

void CAccountManagerPropertiesPage::CreateControls()
{    
////@begin CAccountManagerPropertiesPage content construction
    CAccountManagerPropertiesPage* itemWizardPage36 = this;

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
////@end CAccountManagerPropertiesPage content construction
}

/*!
 * wxEVT_WIZARD_PAGE_CHANGED event handler for ID_PROJECTPROPERTIESPAGE
 */

void CAccountManagerPropertiesPage::OnPageChanged( wxWizardExEvent& event )
{
    if (event.GetDirection() == false) return;
 
    wxASSERT(m_pTitleStaticCtrl);
    wxASSERT(m_pProgressIndicator);

    m_pTitleStaticCtrl->SetLabel(
        _("Communicating with server -\nplease wait.")
    );

    SetProjectPropertiesSucceeded(false);
    SetProjectPropertiesURLFailure(false);
    SetProjectAccountCreationDisabled(false);
    SetProjectClientAccountCreationDisabled(false);
    SetCommunicateYahooSucceeded(false);
    SetCommunicateGoogleSucceeded(false);
    SetDeterminingConnectionStatusSucceeded(false);
    SetNextState(ACCTMGRPROP_INIT);

    CAccountManagerPropertiesPageEvent TransitionEvent(wxEVT_ACCOUNTMANAGERPROPERTIES_STATECHANGE, this);
    AddPendingEvent(TransitionEvent);

    Fit();
}

/*!
 * wxEVT_WIZARD_CANCEL event handler for ID_PROJECTPROPERTIESPAGE
 */

void CAccountManagerPropertiesPage::OnCancel( wxWizardExEvent& event ) {
    PROCESS_CANCELEVENT(event);
}

/*!
 * wxEVT_PROJECTPROPERTIES_STATECHANGE event handler for ID_PROJECTPROPERTIESPAGE
 */
 
void CAccountManagerPropertiesPage::OnStateChange( CAccountManagerPropertiesPageEvent& event )
{
    CMainDocument* pDoc      = wxGetApp().GetDocument();
    PROJECT_CONFIG* pc       = &((CWizardAccountManager*)GetParent())->project_config;
    wxDateTime dtStartExecutionTime;
    wxDateTime dtCurrentExecutionTime;
    wxTimeSpan tsExecutionTime;
    bool bPostNewEvent = true;
    bool bSuccessfulCondition = false;
    int  iReturnValue = 0;
 
    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));
 
    switch(GetCurrentState()) {
        case ACCTMGRPROP_INIT:
            ((CWizardAccountManager*)GetParent())->DisableNextButton();
            ((CWizardAccountManager*)GetParent())->DisableBackButton();
            StartProgress(m_pProgressIndicator);
            SetNextState(ACCTMGRPROP_RETRPROJECTPROPERTIES_BEGIN);
            break;
        case ACCTMGRPROP_RETRPROJECTPROPERTIES_BEGIN:
            SetNextState(ACCTMGRPROP_RETRPROJECTPROPERTIES_EXECUTE);
            break;
        case ACCTMGRPROP_RETRPROJECTPROPERTIES_EXECUTE:
            // Attempt to retrieve the project's account creation policies
            pDoc->rpc.get_project_config(
                ((CWizardAccountManager*)GetParent())->m_AccountManagerInfoPage->GetProjectURL().c_str()
            );
 
            // Wait until we are done processing the request.
            dtStartExecutionTime = wxDateTime::Now();
            dtCurrentExecutionTime = wxDateTime::Now();
            tsExecutionTime = dtCurrentExecutionTime - dtStartExecutionTime;
            iReturnValue = 0;
            pc->error_num = ERR_IN_PROGRESS;
            while ((!iReturnValue && (ERR_IN_PROGRESS == pc->error_num)) &&
                   tsExecutionTime.GetSeconds() <= 60 &&
                   !CHECK_CLOSINGINPROGRESS()
                  )
            {
                dtCurrentExecutionTime = wxDateTime::Now();
                tsExecutionTime = dtCurrentExecutionTime - dtStartExecutionTime;
                iReturnValue = pDoc->rpc.get_project_config_poll(*pc);
                IncrementProgress(m_pProgressIndicator);

                ::wxMilliSleep(500);
                ::wxSafeYield(GetParent());
            }
 
            // We either successfully retrieved the project's account creation 
            //   policies or we were able to talk to the web server and found out
            //   they do not support account creation through the wizard.  In either
            //   case we should claim success and set the correct flags to show the
            //   correct 'next' page.
            bSuccessfulCondition = 
                (!iReturnValue) && (!pc->error_num) ||
                (!iReturnValue) && (ERR_ACCT_CREATION_DISABLED == pc->error_num);
            if (bSuccessfulCondition && !CHECK_DEBUG_FLAG(WIZDEBUG_ERRPROJECTPROPERTIES)) {
                SetProjectPropertiesSucceeded(true);

                bSuccessfulCondition = pc->account_creation_disabled;
                if (bSuccessfulCondition || CHECK_DEBUG_FLAG(WIZDEBUG_ERRACCOUNTCREATIONDISABLED)) {
                    SetProjectAccountCreationDisabled(true);
                } else {
                    SetProjectAccountCreationDisabled(false);
                }

                bSuccessfulCondition = pc->client_account_creation_disabled;
                if (bSuccessfulCondition || CHECK_DEBUG_FLAG(WIZDEBUG_ERRCLIENTACCOUNTCREATIONDISABLED)) {
                    SetProjectClientAccountCreationDisabled(true);
                } else {
                    SetProjectClientAccountCreationDisabled(false);
                }
 
                SetNextState(ACCTMGRPROP_CLEANUP);
            } else {
                SetProjectPropertiesSucceeded(false);
                bSuccessfulCondition = 
                    (!iReturnValue) && (HTTP_STATUS_NOT_FOUND == pc->error_num) ||
                    (!iReturnValue) && (ERR_GETHOSTBYNAME == pc->error_num) ||
                    (!iReturnValue) && (ERR_XML_PARSE == pc->error_num);
                if (bSuccessfulCondition || CHECK_DEBUG_FLAG(WIZDEBUG_ERRPROJECTPROPERTIESURL)) {
                    SetProjectPropertiesURLFailure(true);
                } else {
                    SetProjectPropertiesURLFailure(false);
                }
                SetNextState(ACCTMGRPROP_COMMUNICATEYAHOO_BEGIN);
            }
            break;
        case ACCTMGRPROP_COMMUNICATEYAHOO_BEGIN:
            SetNextState(ACCTMGRPROP_COMMUNICATEYAHOO_EXECUTE);
            break;
        case ACCTMGRPROP_COMMUNICATEYAHOO_EXECUTE:
            // Attempt to successfully download the Yahoo homepage
            pDoc->rpc.lookup_website(LOOKUP_YAHOO);
 
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
                iReturnValue = pDoc->rpc.lookup_website_poll();
                IncrementProgress(m_pProgressIndicator);

                ::wxMilliSleep(500);
                ::wxSafeYield(GetParent());
            }
 
            if ((BOINC_SUCCESS == iReturnValue) && !CHECK_DEBUG_FLAG(WIZDEBUG_ERRYAHOOCOMM)) {
                SetCommunicateYahooSucceeded(true);
            } else {
                SetCommunicateYahooSucceeded(false);
            }

            SetNextState(ACCTMGRPROP_COMMUNICATEGOOGLE_BEGIN);
            break;
        case ACCTMGRPROP_COMMUNICATEGOOGLE_BEGIN:
            SetNextState(ACCTMGRPROP_COMMUNICATEGOOGLE_EXECUTE);
            break;
        case ACCTMGRPROP_COMMUNICATEGOOGLE_EXECUTE:
            // Attempt to successfully download the Google homepage
            pDoc->rpc.lookup_website(LOOKUP_GOOGLE);
 
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
                iReturnValue = pDoc->rpc.lookup_website_poll();
                IncrementProgress(m_pProgressIndicator);

                ::wxMilliSleep(500);
                ::wxSafeYield(GetParent());
            }
 
            if ((BOINC_SUCCESS == iReturnValue) && !CHECK_DEBUG_FLAG(WIZDEBUG_ERRGOOGLECOMM)) {
                SetCommunicateGoogleSucceeded(true);
            } else {
                SetCommunicateGoogleSucceeded(false);
            }

            SetNextState(ACCTMGRPROP_DETERMINENETWORKSTATUS_BEGIN);
            break;
        case ACCTMGRPROP_DETERMINENETWORKSTATUS_BEGIN:
            SetNextState(ACCTMGRPROP_DETERMINENETWORKSTATUS_EXECUTE);
            break;
        case ACCTMGRPROP_DETERMINENETWORKSTATUS_EXECUTE:
            // Attempt to determine if we are even connected to a network
            bSuccessfulCondition = CONNECTED_STATE_CONNECTED == get_connected_state();
            if (bSuccessfulCondition && !CHECK_DEBUG_FLAG(WIZDEBUG_ERRNETDETECTION)) {
                SetDeterminingConnectionStatusSucceeded(true);
            } else {
                SetDeterminingConnectionStatusSucceeded(false);
            }
            SetNextState(ACCTMGRPROP_CLEANUP);
            break;
        case ACCTMGRPROP_CLEANUP:
            FinishProgress(m_pProgressIndicator);
            SetNextState(ACCTMGRPROP_END);
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
        CAccountManagerPropertiesPageEvent TransitionEvent(wxEVT_ACCOUNTMANAGERPROPERTIES_STATECHANGE, this);
        AddPendingEvent(TransitionEvent);
    }
}
   
/*!
 * Gets the previous page.
 */

wxWizardPageEx* CAccountManagerPropertiesPage::GetPrev() const
{
    return PAGE_TRANSITION_BACK;
}

/*!
 * Gets the next page.
 */

wxWizardPageEx* CAccountManagerPropertiesPage::GetNext() const
{
    if (CHECK_CLOSINGINPROGRESS()) {
        // Cancel Event Detected
        return PAGE_TRANSITION_NEXT(ID_COMPLETIONERRORPAGE);
    } else if (GetProjectPropertiesSucceeded()) {
        // We were successful in retrieving the project properties
        return PAGE_TRANSITION_NEXT(ID_ACCOUNTINFOPAGE);
    } else if (!GetProjectPropertiesSucceeded() && GetProjectPropertiesURLFailure()) {
        // Not a BOINC based project
        return PAGE_TRANSITION_NEXT(ID_ERRNOTDETECTEDPAGE);
    } else if ((!GetCommunicateYahooSucceeded() && !GetCommunicateGoogleSucceeded()) && GetDeterminingConnectionStatusSucceeded()) {
        // Possible proxy problem
        return PAGE_TRANSITION_NEXT(ID_ERRPROXYINFOPAGE);
    } else if ((!GetCommunicateYahooSucceeded() && !GetCommunicateGoogleSucceeded()) && !GetDeterminingConnectionStatusSucceeded()) {
        // No Internet Connection
        return PAGE_TRANSITION_NEXT(ID_ERRNOINTERNETCONNECTIONPAGE);
    } else {
        // The project much be down for maintenance
        return PAGE_TRANSITION_NEXT(ID_ERRUNAVAILABLEPAGE);
    }
    return NULL;
}

/*!
 * Should we show tooltips?
 */

bool CAccountManagerPropertiesPage::ShowToolTips()
{
    return TRUE;
}

 
void CAccountManagerPropertiesPage::StartProgress(wxStaticBitmap* pBitmap) {
    m_iBitmapIndex = 1;
    pBitmap->SetBitmap(GetBitmapResource(wxT("res/wizprogress01.xpm")));
}
 
void CAccountManagerPropertiesPage::IncrementProgress(wxStaticBitmap* pBitmap) {
    m_iBitmapIndex += 1;
    if (12 < m_iBitmapIndex) m_iBitmapIndex = 1;
 
    wxString str;
    str.Printf(wxT("res/wizprogress%02d.xpm"), m_iBitmapIndex);
 
    pBitmap->SetBitmap(GetBitmapResource(str));
    Update();
}
 
void CAccountManagerPropertiesPage::FinishProgress(wxStaticBitmap* pBitmap) {
    m_iBitmapIndex = 12;
    pBitmap->SetBitmap(GetBitmapResource(wxT("res/wizprogress12.xpm")));
}
 
/*!
 * Get bitmap resources
 */

wxBitmap CAccountManagerPropertiesPage::GetBitmapResource( const wxString& name )
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

wxIcon CAccountManagerPropertiesPage::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CAccountManagerPropertiesPage icon retrieval
    return wxNullIcon;
////@end CAccountManagerPropertiesPage icon retrieval
}

