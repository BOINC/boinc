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
#pragma implementation "WizardAccountManager.h"
#endif

#include "stdwx.h"
#include "wizardex.h"
#include "error_numbers.h"
#include "BOINCGUIApp.h"
#include "BOINCWizards.h"
#include "BOINCBaseWizard.h"
#include "hyperlink.h"
#include "WizardAccountManager.h"
#include "WelcomePage.h"
#include "AccountManagerInfoPage.h"
#include "AccountManagerStatusPage.h"
#include "AccountManagerPropertiesPage.h"
#include "AccountInfoPage.h"
#include "AccountManagerProcessingPage.h"
#include "CompletionPage.h"
#include "CompletionUpdatePage.h"
#include "CompletionRemovePage.h"
#include "CompletionErrorPage.h"
#include "NotDetectedPage.h"
#include "UnavailablePage.h"
#include "NoInternetConnectionPage.h"
#include "ProxyInfoPage.h"
#include "ProxyPage.h"

////@begin XPM images
#include "res/attachprojectwizard.xpm"
////@end XPM images
  
/*!
 * CWizardAccountManager type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CWizardAccountManager, CBOINCBaseWizard )

/*!
 * CWizardAccountManager event table definition
 */

BEGIN_EVENT_TABLE( CWizardAccountManager, CBOINCBaseWizard )

////@begin CWizardAccountManager event table entries
    EVT_WIZARD_FINISHED( ID_ATTACHACCOUNTMANAGERWIZARD, CWizardAccountManager::OnFinished )

////@end CWizardAccountManager event table entries

END_EVENT_TABLE()

/*!
 * CWizardAccountManager constructors
 */

CWizardAccountManager::CWizardAccountManager()
{
}

CWizardAccountManager::CWizardAccountManager( wxWindow* parent, wxWindowID id, const wxPoint& pos )
{
    Create(parent, id, pos);
}

/*!
 * CWizardAccountManager creator
 */

bool CWizardAccountManager::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos )
{
////@begin CWizardAccountManager member initialisation
    m_WelcomePage = NULL;
    m_AccountManagerInfoPage = NULL;
    m_AccountManagerStatusPage = NULL;
    m_AccountManagerPropertiesPage = NULL;
    m_AccountInfoPage = NULL;
    m_AccountManagerProcessingPage = NULL;
    m_CompletionPage = NULL;
    m_CompletionUpdatePage = NULL;
    m_CompletionRemovePage = NULL;
    m_CompletionErrorPage = NULL;
    m_ErrNotDetectedPage = NULL;
    m_ErrUnavailablePage = NULL;
    m_ErrNoInternetConnectionPage = NULL;
    m_ErrProxyInfoPage = NULL;
    m_ErrProxyPage = NULL;
////@end CWizardAccountManager member initialisation
  
    // Wizard support
    m_ulDiagFlags = 0;

    // Cancel Checking
    m_bCancelInProgress = false;

    // Wizard Detection
    IsAttachToProjectWizard = false;
    IsAccountManagerAttachWizard = true;
    IsAccountManagerUpdateWizard = true;
    IsAccountManagerRemoveWizard = true;
 
    // Global wizard status
    project_config.clear();
    attached_to_project_successfully = false;
    m_strProjectName.Empty();
    m_bCredentialsCached = false;

    wxBitmap wizardBitmap;
    if (wxGetApp().GetBrand()->IsBranded()) {
        wizardBitmap = wxBitmap(*(wxGetApp().GetBrand()->GetAMWizardLogo()));
    } else {
        wizardBitmap = wxBitmap(GetBitmapResource(wxT("res/attachprojectwizard.xpm")));
    }
////@begin CWizardAccountManager creation
    CBOINCBaseWizard::Create( parent, id, _("Attach to Account Manager"), wizardBitmap, pos );

    CreateControls();
////@end CWizardAccountManager creation
    return TRUE;
}

/*!
 * Control creation for CWizardAccountManager
 */

void CWizardAccountManager::CreateControls()
{    
    wxLogTrace(wxT("Function Start/End"), wxT("CWizardAccountManager::CreateControls - Function Begin"));

////@begin CWizardAccountManager content construction
    CBOINCBaseWizard* itemWizard1 = this;

    m_WelcomePage = new CWelcomePage;
    m_WelcomePage->Create( itemWizard1 );

    itemWizard1->FitToPage(m_WelcomePage);
    m_AccountManagerInfoPage = new CAccountManagerInfoPage;
    m_AccountManagerInfoPage->Create( itemWizard1 );

    itemWizard1->FitToPage(m_AccountManagerInfoPage);
    m_AccountManagerStatusPage = new CAccountManagerStatusPage;
    m_AccountManagerStatusPage->Create( itemWizard1 );

    itemWizard1->FitToPage(m_AccountManagerStatusPage);
    m_AccountManagerPropertiesPage = new CAccountManagerPropertiesPage;
    m_AccountManagerPropertiesPage->Create( itemWizard1 );

    itemWizard1->FitToPage(m_AccountManagerPropertiesPage);
    m_AccountInfoPage = new CAccountInfoPage;
    m_AccountInfoPage->Create( itemWizard1 );

    itemWizard1->FitToPage(m_AccountInfoPage);
    m_AccountManagerProcessingPage = new CAccountManagerProcessingPage;
    m_AccountManagerProcessingPage->Create( itemWizard1 );

    itemWizard1->FitToPage(m_AccountManagerProcessingPage);
    m_CompletionPage = new CCompletionPage;
    m_CompletionPage->Create( itemWizard1 );

    itemWizard1->FitToPage(m_CompletionPage);
    m_CompletionUpdatePage = new CCompletionUpdatePage;
    m_CompletionUpdatePage->Create( itemWizard1 );

    itemWizard1->FitToPage(m_CompletionUpdatePage);
    m_CompletionRemovePage = new CCompletionRemovePage;
    m_CompletionRemovePage->Create( itemWizard1 );

    itemWizard1->FitToPage(m_CompletionRemovePage);
    m_CompletionErrorPage = new CCompletionErrorPage;
    m_CompletionErrorPage->Create( itemWizard1 );

    itemWizard1->FitToPage(m_CompletionErrorPage);
    m_ErrNotDetectedPage = new CErrNotDetectedPage;
    m_ErrNotDetectedPage->Create( itemWizard1 );

    itemWizard1->FitToPage(m_ErrNotDetectedPage);
    m_ErrUnavailablePage = new CErrUnavailablePage;
    m_ErrUnavailablePage->Create( itemWizard1 );

    itemWizard1->FitToPage(m_ErrUnavailablePage);
    m_ErrNoInternetConnectionPage = new CErrNoInternetConnectionPage;
    m_ErrNoInternetConnectionPage->Create( itemWizard1 );

    itemWizard1->FitToPage(m_ErrNoInternetConnectionPage);
    m_ErrProxyInfoPage = new CErrProxyInfoPage;
    m_ErrProxyInfoPage->Create( itemWizard1 );

    itemWizard1->FitToPage(m_ErrProxyInfoPage);
    m_ErrProxyPage = new CErrProxyPage;
    m_ErrProxyPage->Create( itemWizard1 );

    itemWizard1->FitToPage(m_ErrProxyPage);

////@end CWizardAccountManager content construction
 
    wxLogTrace(wxT("Function Status"), wxT("CWizardAccountManager::CreateControls - Begin Page Map"));
    wxLogTrace(wxT("Function Status"), wxT("CWizardAccountManager::CreateControls -     m_WelcomePage = id: '%d', location: '%p'"), ID_WELCOMEPAGE, m_WelcomePage);
    wxLogTrace(wxT("Function Status"), wxT("CWizardAccountManager::CreateControls -     m_AccountManagerInfoPage = id: '%d', location: '%p'"), ID_ACCOUNTMANAGERINFOPAGE, m_AccountManagerInfoPage);
    wxLogTrace(wxT("Function Status"), wxT("CWizardAccountManager::CreateControls -     m_AccountManagerStatusPage = id: '%d', location: '%p'"), ID_ACCOUNTMANAGERSTATUSPAGE, m_AccountManagerStatusPage);
    wxLogTrace(wxT("Function Status"), wxT("CWizardAccountManager::CreateControls -     m_AccountManagerPropertiesPage = id: '%d', location: '%p'"), ID_ACCOUNTMANAGERPROPERTIESPAGE, m_AccountManagerPropertiesPage);
    wxLogTrace(wxT("Function Status"), wxT("CWizardAccountManager::CreateControls -     m_AccountInfoPage = id: '%d', location: '%p'"), ID_ACCOUNTINFOPAGE, m_AccountInfoPage);
    wxLogTrace(wxT("Function Status"), wxT("CWizardAccountManager::CreateControls -     m_AccountManagerProcessingPage = id: '%d', location: '%p'"), ID_ACCOUNTMANAGERPROCESSINGPAGE, m_AccountManagerProcessingPage);
    wxLogTrace(wxT("Function Status"), wxT("CWizardAccountManager::CreateControls -     m_CompletionPage = id: '%d', location: '%p'"), ID_COMPLETIONPAGE, m_CompletionPage);
    wxLogTrace(wxT("Function Status"), wxT("CWizardAccountManager::CreateControls -     m_CompletionUpdatePage = id: '%d', location: '%p'"), ID_COMPLETIONUPDATEPAGE, m_CompletionUpdatePage);
    wxLogTrace(wxT("Function Status"), wxT("CWizardAccountManager::CreateControls -     m_CompletionRemovePage = id: '%d', location: '%p'"), ID_COMPLETIONREMOVEPAGE, m_CompletionRemovePage);
    wxLogTrace(wxT("Function Status"), wxT("CWizardAccountManager::CreateControls -     m_CompletionErrorPage = id: '%d', location: '%p'"), ID_COMPLETIONERRORPAGE, m_CompletionErrorPage);
    wxLogTrace(wxT("Function Status"), wxT("CWizardAccountManager::CreateControls -     m_ErrNotDetectedPage = id: '%d', location: '%p'"), ID_ERRNOTDETECTEDPAGE, m_ErrNotDetectedPage);
    wxLogTrace(wxT("Function Status"), wxT("CWizardAccountManager::CreateControls -     m_ErrUnavailablePage = id: '%d', location: '%p'"), ID_ERRUNAVAILABLEPAGE, m_ErrUnavailablePage);
    wxLogTrace(wxT("Function Status"), wxT("CWizardAccountManager::CreateControls -     m_ErrNoInternetConnectionPage = id: '%d', location: '%p'"), ID_ERRNOINTERNETCONNECTIONPAGE, m_ErrNoInternetConnectionPage);
    wxLogTrace(wxT("Function Status"), wxT("CWizardAccountManager::CreateControls -     m_ErrProxyInfoPage = id: '%d', location: '%p'"), ID_ERRPROXYINFOPAGE, m_ErrProxyInfoPage);
    wxLogTrace(wxT("Function Status"), wxT("CWizardAccountManager::CreateControls -     m_ErrProxyPage = id: '%d', location: '%p'"), ID_ERRPROXYPAGE, m_ErrProxyPage);
    wxLogTrace(wxT("Function Status"), wxT("CWizardAccountManager::CreateControls - End Page Map"));
    wxLogTrace(wxT("Function Start/End"), wxT("CWizardAccountManager::CreateControls - Function End"));
}

/*!
 * Runs the wizard.
 */

bool CWizardAccountManager::Run() {
    ACCT_MGR_INFO ami;
    CMainDocument*            pDoc = wxGetApp().GetDocument();

    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));

    pDoc->rpc.acct_mgr_info(ami);

    if (ami.acct_mgr_url.size()) {
        if (wxGetApp().GetBrand()->IsBranded()) {
            wxString strTitle;

            // %s is the project name
            //    i.e. 'GridRepublic'
            strTitle.Printf(
                _("Attach to %s"),
                wxGetApp().GetBrand()->GetProjectName().c_str()
            );

            SetTitle(strTitle);
        } else if (ami.acct_mgr_name.size()) {
            wxString strTitle;
            strTitle = GetTitle();
            strTitle += wxT(" - ");
            strTitle += ami.acct_mgr_name.c_str();
            SetTitle(strTitle);
        }

        m_AccountManagerInfoPage->SetProjectURL( ami.acct_mgr_url.c_str() );
        m_strProjectName = ami.acct_mgr_name.c_str();
        m_bCredentialsCached = ami.have_credentials;
    }

    if ( ami.acct_mgr_url.size() && !ami.have_credentials && m_AccountManagerStatusPage) {
        m_AccountManagerStatusPage->m_AcctManagerUpdateCtrl->SetValue(true);
        m_AccountManagerStatusPage->m_AcctManagerRemoveCtrl->SetValue(false);
        IsAccountManagerAttachWizard = false;
        IsAccountManagerUpdateWizard = true;
        IsAccountManagerRemoveWizard = false;
        return RunWizard(m_AccountInfoPage);
    } else if ( ami.acct_mgr_url.size() && m_AccountManagerStatusPage) {
        IsAccountManagerAttachWizard = false;
        IsAccountManagerUpdateWizard = true;
        IsAccountManagerRemoveWizard = true;
        return RunWizard(m_AccountManagerStatusPage);
    } else if (m_WelcomePage) {
        IsAccountManagerAttachWizard = true;
        IsAccountManagerUpdateWizard = false;
        IsAccountManagerRemoveWizard = false;
        return RunWizard(m_WelcomePage);
    }

    return FALSE;
}

/*!
 * Should we show tooltips?
 */

bool CWizardAccountManager::ShowToolTips()
{
    return TRUE;
}

/*!
 * Get bitmap resources
 */

wxBitmap CWizardAccountManager::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CWizardAccountManager bitmap retrieval
    if (name == wxT("res/attachprojectwizard.xpm"))
    {
        wxBitmap bitmap(attachprojectwizard_xpm);
        return bitmap;
    }
    return wxNullBitmap;
////@end CWizardAccountManager bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CWizardAccountManager::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CWizardAccountManager icon retrieval
    return wxNullIcon;
////@end CWizardAccountManager icon retrieval
}

/*!
 * Determine if the wizard page has a next page
 */

bool CWizardAccountManager::HasNextPage( wxWizardPageEx* page )
{
    bool bNoNextPageDetected = false;

    bNoNextPageDetected |= (page == m_CompletionPage);
    bNoNextPageDetected |= (page == m_CompletionUpdatePage);
    bNoNextPageDetected |= (page == m_CompletionRemovePage);
    bNoNextPageDetected |= (page == m_CompletionErrorPage);
    bNoNextPageDetected |= (page == m_ErrNotDetectedPage);
    bNoNextPageDetected |= (page == m_ErrUnavailablePage);
    bNoNextPageDetected |= (page == m_ErrNoInternetConnectionPage);
 
    if (bNoNextPageDetected)
        return false;
    return true;
}
  
/*!
 * Determine if the wizard page has a previous page
 */
 
bool CWizardAccountManager::HasPrevPage( wxWizardPageEx* page )
{
    bool bNoPrevPageDetected = false;

    bNoPrevPageDetected |= (page == m_WelcomePage);
    bNoPrevPageDetected |= (page == m_CompletionPage);
    bNoPrevPageDetected |= (page == m_CompletionUpdatePage);
    bNoPrevPageDetected |= (page == m_CompletionRemovePage);
    bNoPrevPageDetected |= (page == m_CompletionErrorPage);

    if (bNoPrevPageDetected)
        return false;
    return true;
}
 
/*!
 * Remove the page transition to the stack.
 */
 
wxWizardPageEx* CWizardAccountManager::_PopPageTransition() {
    wxWizardPageEx* pPage = NULL;
    if (GetCurrentPage()) {
        if (m_PageTransition.size() > 0) {
            pPage = m_PageTransition.top();
            m_PageTransition.pop();
            if ((pPage == m_AccountManagerPropertiesPage) || (pPage == m_AccountManagerProcessingPage)) {
                // We want to go back to the page before we attempted to communicate
                //   with any server.
                pPage = m_PageTransition.top();
                m_PageTransition.pop();
            }
            wxASSERT(pPage);
            return pPage;
        }
    }
    return NULL;
}
 
/*!
 * Add the page transition to the stack.
 */

wxWizardPageEx* CWizardAccountManager::_PushPageTransition( wxWizardPageEx* pCurrentPage, unsigned long ulPageID ) {
    if (GetCurrentPage()) {
        wxWizardPageEx* pPage = NULL;

        if (ID_WELCOMEPAGE == ulPageID)
            pPage = m_WelcomePage;
 
        if (ID_ACCOUNTMANAGERINFOPAGE == ulPageID)
            pPage = m_AccountManagerInfoPage;
 
        if (ID_ACCOUNTMANAGERSTATUSPAGE == ulPageID)
            pPage = m_AccountManagerStatusPage;
 
        if (ID_ACCOUNTMANAGERPROPERTIESPAGE == ulPageID)
            pPage = m_AccountManagerPropertiesPage;
 
        if (ID_ACCOUNTINFOPAGE == ulPageID)
            pPage = m_AccountInfoPage;
 
        if (ID_ACCOUNTMANAGERPROCESSINGPAGE == ulPageID)
            pPage = m_AccountManagerProcessingPage;
 
        if (ID_COMPLETIONPAGE == ulPageID)
            pPage = m_CompletionPage;
 
        if (ID_COMPLETIONUPDATEPAGE == ulPageID)
            pPage = m_CompletionUpdatePage;
 
        if (ID_COMPLETIONREMOVEPAGE == ulPageID)
            pPage = m_CompletionRemovePage;
 
        if (ID_COMPLETIONERRORPAGE == ulPageID)
            pPage = m_CompletionErrorPage;
 
        if (ID_ERRNOTDETECTEDPAGE == ulPageID)
            pPage = m_ErrNotDetectedPage;
 
        if (ID_ERRUNAVAILABLEPAGE == ulPageID)
            pPage = m_ErrUnavailablePage;
 
        if (ID_ERRNOINTERNETCONNECTIONPAGE == ulPageID)
            pPage = m_ErrNoInternetConnectionPage;
 
         if (ID_ERRPROXYINFOPAGE == ulPageID)
            pPage = m_ErrProxyInfoPage;
 
        if (ID_ERRPROXYPAGE == ulPageID)
            pPage = m_ErrProxyPage;
 
        if (pPage) {
            if (m_PageTransition.size() == 0) {
                m_PageTransition.push(NULL);
            }
            if (m_PageTransition.top() != pCurrentPage) {
                m_PageTransition.push(pCurrentPage);
            }
            return pPage;
        }
    }
    return NULL;
}
  
void CWizardAccountManager::_ProcessCancelEvent( wxWizardExEvent& event ) {

    bool bCancelWithoutNextPage = false;
    wxWizardPageEx* page = GetCurrentPage();

    int iRetVal = ::wxMessageBox(
        _("Do you really want to cancel?"), 
        _("Question"),
        wxICON_QUESTION | wxYES_NO,
        this
    );

    // Reenable the next and back buttons if they have been disabled
    GetNextButton()->Enable();
    GetBackButton()->Enable();

    // Page specific rules - Disable the validator(s)
    if (wxYES == iRetVal) {
        if (page == m_AccountManagerInfoPage) {
            m_AccountManagerInfoPage->m_AccountManagerUrlCtrl->SetValidator(wxDefaultValidator);
        } else if (page == m_AccountInfoPage) {
            m_AccountInfoPage->m_AccountEmailAddressCtrl->SetValidator(wxDefaultValidator);
            m_AccountInfoPage->m_AccountPasswordCtrl->SetValidator(wxDefaultValidator);
        } else if (page == m_ErrProxyPage) {
            m_ErrProxyPage->m_ProxyHTTPServerCtrl->SetValidator(wxDefaultValidator);
            m_ErrProxyPage->m_ProxyHTTPPortCtrl->SetValidator(wxDefaultValidator);
            m_ErrProxyPage->m_ProxyHTTPUsernameCtrl->SetValidator(wxDefaultValidator);
            m_ErrProxyPage->m_ProxyHTTPPasswordCtrl->SetValidator(wxDefaultValidator);
            m_ErrProxyPage->m_ProxySOCKSServerCtrl->SetValidator(wxDefaultValidator);
            m_ErrProxyPage->m_ProxySOCKSPortCtrl->SetValidator(wxDefaultValidator);
            m_ErrProxyPage->m_ProxySOCKSUsernameCtrl->SetValidator(wxDefaultValidator);
            m_ErrProxyPage->m_ProxySOCKSPasswordCtrl->SetValidator(wxDefaultValidator);
        }
    }

    // Generic rules
    bCancelWithoutNextPage |= (page == m_WelcomePage);
    bCancelWithoutNextPage |= (page == m_ErrNotDetectedPage);
    bCancelWithoutNextPage |= (page == m_ErrUnavailablePage);
    bCancelWithoutNextPage |= (page == m_ErrNoInternetConnectionPage);
    if (wxYES != iRetVal) {
        event.Veto();
    }
/*
    if (!bCancelWithoutNextPage) {
        event.Veto();
        if (wxYES == iRetVal) {
            m_bCancelInProgress = true;
            SimulateNextButton();
        }
    } else {
        if (wxYES != iRetVal) {
            event.Veto();
        }
    }
*/
}
 
/*!
 * wxEVT_WIZARD_FINISHED event handler for ID_ATTACHACCOUNTMANAGERWIZARD
 */

void CWizardAccountManager::OnFinished( wxWizardEvent& event )
{
    event.Skip();
}

