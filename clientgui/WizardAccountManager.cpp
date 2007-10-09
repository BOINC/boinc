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
// 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
//
#if defined(__GNUG__) && !defined(__APPLE__)
#pragma implementation "WizardAccountManager.h"
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
#include "hyperlink.h"
#include "BOINCGUIApp.h"
#include "SkinManager.h"
#include "MainDocument.h"
#include "BOINCWizards.h"
#include "BOINCBaseWizard.h"
#include "BOINCBaseFrame.h"
#include "WizardAccountManager.h"
#include "WelcomePage.h"
#include "AccountManagerInfoPage.h"
#include "AccountManagerPropertiesPage.h"
#include "AccountManagerProcessingPage.h"
#include "AccountInfoPage.h"
#include "CompletionPage.h"
#include "CompletionErrorPage.h"
#include "NotDetectedPage.h"
#include "UnavailablePage.h"
#include "NoInternetConnectionPage.h"
#include "NotFoundPage.h"
#include "ProxyInfoPage.h"
#include "ProxyPage.h"

  
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
    m_AccountManagerPropertiesPage = NULL;
    m_AccountManagerProcessingPage = NULL;
    m_AccountInfoPage = NULL;
    m_CompletionPage = NULL;
    m_CompletionErrorPage = NULL;
    m_ErrNotDetectedPage = NULL;
    m_ErrUnavailablePage = NULL;
    m_ErrNoInternetConnectionPage = NULL;
    m_ErrNotFoundPage = NULL;
    m_ErrProxyInfoPage = NULL;
    m_ErrProxyPage = NULL;
////@end CWizardAccountManager member initialisation
  
    // Wizard support
    m_ulDiagFlags = 0;

    // Cancel Checking
    m_bCancelInProgress = false;

    // Wizard Detection
    IsAttachToProjectWizard = false;
    IsAccountManagerWizard = true;
    IsAccountManagerUpdateWizard = false;
    IsAccountManagerRemoveWizard = false;
 
    // Global wizard status
    project_config.clear();
    attached_to_project_successfully = false;
    m_strProjectName.Empty();
    m_bCredentialsCached = false;


    CSkinAdvanced*   pSkinAdvanced = wxGetApp().GetSkinManager()->GetAdvanced();
    CSkinWizardATAM* pSkinWizardATAM = wxGetApp().GetSkinManager()->GetWizards()->GetWizardATAM();

    wxASSERT(pSkinAdvanced);
    wxASSERT(pSkinWizardATAM);
    wxASSERT(wxDynamicCast(pSkinAdvanced, CSkinAdvanced));
    wxASSERT(wxDynamicCast(pSkinWizardATAM, CSkinWizardATAM));


    wxString strTitle;
    if (!pSkinWizardATAM->GetWizardTitle().IsEmpty()) {
        strTitle = pSkinWizardATAM->GetWizardTitle();
    } else {
        strTitle = pSkinAdvanced->GetApplicationName();
    }

    wxBitmap wizardBitmap = wxBitmap(*(pSkinWizardATAM->GetWizardBitmap()));

////@begin CWizardAccountManager creation
    CBOINCBaseWizard::Create( parent, id, strTitle, wizardBitmap, pos );

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
    GetPageAreaSizer()->Add(m_WelcomePage);

    m_AccountManagerInfoPage = new CAccountManagerInfoPage;
    m_AccountManagerInfoPage->Create( itemWizard1 );
    GetPageAreaSizer()->Add(m_AccountManagerInfoPage);

    m_AccountManagerPropertiesPage = new CAccountManagerPropertiesPage;
    m_AccountManagerPropertiesPage->Create( itemWizard1 );
    GetPageAreaSizer()->Add(m_AccountManagerPropertiesPage);

    m_AccountInfoPage = new CAccountInfoPage;
    m_AccountInfoPage->Create( itemWizard1 );
    GetPageAreaSizer()->Add(m_AccountInfoPage);

    m_AccountManagerProcessingPage = new CAccountManagerProcessingPage;
    m_AccountManagerProcessingPage->Create( itemWizard1 );
    GetPageAreaSizer()->Add(m_AccountManagerProcessingPage);

    m_CompletionPage = new CCompletionPage;
    m_CompletionPage->Create( itemWizard1 );
    GetPageAreaSizer()->Add(m_CompletionPage);

    m_CompletionErrorPage = new CCompletionErrorPage;
    m_CompletionErrorPage->Create( itemWizard1 );
    GetPageAreaSizer()->Add(m_CompletionErrorPage);

    m_ErrNotDetectedPage = new CErrNotDetectedPage;
    m_ErrNotDetectedPage->Create( itemWizard1 );
    GetPageAreaSizer()->Add(m_ErrNotDetectedPage);

    m_ErrUnavailablePage = new CErrUnavailablePage;
    m_ErrUnavailablePage->Create( itemWizard1 );
    GetPageAreaSizer()->Add(m_ErrUnavailablePage);

    m_ErrNoInternetConnectionPage = new CErrNoInternetConnectionPage;
    m_ErrNoInternetConnectionPage->Create( itemWizard1 );
    GetPageAreaSizer()->Add(m_ErrNoInternetConnectionPage);

    m_ErrNotFoundPage = new CErrNotFoundPage;
    m_ErrNotFoundPage->Create( itemWizard1 );
    GetPageAreaSizer()->Add(m_ErrNotFoundPage);

    m_ErrProxyInfoPage = new CErrProxyInfoPage;
    m_ErrProxyInfoPage->Create( itemWizard1 );
    GetPageAreaSizer()->Add(m_ErrProxyInfoPage);

    m_ErrProxyPage = new CErrProxyPage;
    m_ErrProxyPage->Create( itemWizard1 );
    GetPageAreaSizer()->Add(m_ErrProxyPage);

////@end CWizardAccountManager content construction
 
    wxLogTrace(wxT("Function Status"), wxT("CWizardAccountManager::CreateControls - Begin Page Map"));
    wxLogTrace(wxT("Function Status"), wxT("CWizardAccountManager::CreateControls -     m_WelcomePage = id: '%d', location: '%p'"), m_WelcomePage->GetId(), m_WelcomePage);
    wxLogTrace(wxT("Function Status"), wxT("CWizardAccountManager::CreateControls -     m_AccountManagerInfoPage = id: '%d', location: '%p'"), m_AccountManagerInfoPage->GetId(), m_AccountManagerInfoPage);
    wxLogTrace(wxT("Function Status"), wxT("CWizardAccountManager::CreateControls -     m_AccountManagerPropertiesPage = id: '%d', location: '%p'"), m_AccountManagerPropertiesPage->GetId(), m_AccountManagerPropertiesPage);
    wxLogTrace(wxT("Function Status"), wxT("CWizardAccountManager::CreateControls -     m_AccountInfoPage = id: '%d', location: '%p'"), m_AccountInfoPage->GetId(), m_AccountInfoPage);
    wxLogTrace(wxT("Function Status"), wxT("CWizardAccountManager::CreateControls -     m_AccountManagerProcessingPage = id: '%d', location: '%p'"), m_AccountManagerProcessingPage->GetId(), m_AccountManagerProcessingPage);
    wxLogTrace(wxT("Function Status"), wxT("CWizardAccountManager::CreateControls -     m_CompletionPage = id: '%d', location: '%p'"), m_CompletionPage->GetId(), m_CompletionPage);
    wxLogTrace(wxT("Function Status"), wxT("CWizardAccountManager::CreateControls -     m_CompletionErrorPage = id: '%d', location: '%p'"), m_CompletionErrorPage->GetId(), m_CompletionErrorPage);
    wxLogTrace(wxT("Function Status"), wxT("CWizardAccountManager::CreateControls -     m_ErrNotDetectedPage = id: '%d', location: '%p'"), m_ErrNotDetectedPage->GetId(), m_ErrNotDetectedPage);
    wxLogTrace(wxT("Function Status"), wxT("CWizardAccountManager::CreateControls -     m_ErrUnavailablePage = id: '%d', location: '%p'"), m_ErrUnavailablePage->GetId(), m_ErrUnavailablePage);
    wxLogTrace(wxT("Function Status"), wxT("CWizardAccountManager::CreateControls -     m_ErrNoInternetConnectionPage = id: '%d', location: '%p'"), m_ErrNoInternetConnectionPage->GetId(), m_ErrNoInternetConnectionPage);
    wxLogTrace(wxT("Function Status"), wxT("CWizardAccountManager::CreateControls -     m_ErrNotFoundPage = id: '%d', location: '%p'"), m_ErrNotFoundPage->GetId(), m_ErrNotFoundPage);
    wxLogTrace(wxT("Function Status"), wxT("CWizardAccountManager::CreateControls -     m_ErrProxyInfoPage = id: '%d', location: '%p'"), m_ErrProxyInfoPage->GetId(), m_ErrProxyInfoPage);
    wxLogTrace(wxT("Function Status"), wxT("CWizardAccountManager::CreateControls -     m_ErrProxyPage = id: '%d', location: '%p'"), m_ErrProxyPage->GetId(), m_ErrProxyPage);
    wxLogTrace(wxT("Function Status"), wxT("CWizardAccountManager::CreateControls - End Page Map"));
    wxLogTrace(wxT("Function Start/End"), wxT("CWizardAccountManager::CreateControls - Function End"));
}

/*!
 * Runs the wizard.
 */

bool CWizardAccountManager::Run(int action) {
    ACCT_MGR_INFO ami;
    CMainDocument*            pDoc = wxGetApp().GetDocument();

    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));

    pDoc->rpc.acct_mgr_info(ami);

    if (ami.acct_mgr_url.size()) {
        m_AccountManagerInfoPage->SetProjectURL( wxString(ami.acct_mgr_url.c_str(), wxConvUTF8) );
        m_strProjectName = wxString(ami.acct_mgr_name.c_str(), wxConvUTF8);
        m_bCredentialsCached = ami.have_credentials;
    }

    if ( ami.acct_mgr_url.size() && !ami.have_credentials) {
        return RunWizard(m_AccountManagerPropertiesPage);
    } else if ( ami.acct_mgr_url.size() && ami.have_credentials && (action == ACCOUNTMANAGER_UPDATE)) {
        IsAccountManagerUpdateWizard = true;
        IsAccountManagerRemoveWizard = false;
        return RunWizard(m_AccountManagerProcessingPage);
    } else if ( ami.acct_mgr_url.size() && ami.have_credentials && (action == ACCOUNTMANAGER_DETACH)) {
        IsAccountManagerUpdateWizard = false;
        IsAccountManagerRemoveWizard = true;
        m_AccountManagerInfoPage->SetProjectURL(wxEmptyString);
        m_AccountInfoPage->SetAccountEmailAddress(wxEmptyString);
        m_AccountInfoPage->SetAccountPassword(wxEmptyString);
        m_bCredentialsCached = false;
        return RunWizard(m_WelcomePage);
    } else if (m_WelcomePage) {
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

wxBitmap CWizardAccountManager::GetBitmapResource( const wxString& WXUNUSED(name) )
{
    // Bitmap retrieval
////@begin CWizardAccountManager bitmap retrieval
    return wxNullBitmap;
////@end CWizardAccountManager bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CWizardAccountManager::GetIconResource( const wxString& WXUNUSED(name) )
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
 
        if (ID_ACCOUNTMANAGERPROPERTIESPAGE == ulPageID)
            pPage = m_AccountManagerPropertiesPage;
 
        if (ID_ACCOUNTINFOPAGE == ulPageID)
            pPage = m_AccountInfoPage;
 
        if (ID_ACCOUNTMANAGERPROCESSINGPAGE == ulPageID)
            pPage = m_AccountManagerProcessingPage;
 
        if (ID_COMPLETIONPAGE == ulPageID)
            pPage = m_CompletionPage;
 
        if (ID_COMPLETIONERRORPAGE == ulPageID)
            pPage = m_CompletionErrorPage;
 
        if (ID_ERRNOTDETECTEDPAGE == ulPageID)
            pPage = m_ErrNotDetectedPage;
 
        if (ID_ERRUNAVAILABLEPAGE == ulPageID)
            pPage = m_ErrUnavailablePage;
 
        if (ID_ERRNOINTERNETCONNECTIONPAGE == ulPageID)
            pPage = m_ErrNoInternetConnectionPage;
 
        if (ID_ERRNOTFOUNDPAGE == ulPageID)
            pPage = m_ErrNotFoundPage;
 
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
            m_AccountManagerInfoPage->m_pProjectUrlCtrl->SetValidator(wxDefaultValidator);
        } else if (page == m_AccountInfoPage) {
            m_AccountInfoPage->m_pAccountEmailAddressCtrl->SetValidator(wxDefaultValidator);
            m_AccountInfoPage->m_pAccountPasswordCtrl->SetValidator(wxDefaultValidator);
        } else if (page == m_ErrProxyPage) {
            m_ErrProxyPage->m_pProxyHTTPServerCtrl->SetValidator(wxDefaultValidator);
            m_ErrProxyPage->m_pProxyHTTPPortCtrl->SetValidator(wxDefaultValidator);
            m_ErrProxyPage->m_pProxyHTTPUsernameCtrl->SetValidator(wxDefaultValidator);
            m_ErrProxyPage->m_pProxyHTTPPasswordCtrl->SetValidator(wxDefaultValidator);
            m_ErrProxyPage->m_pProxySOCKSServerCtrl->SetValidator(wxDefaultValidator);
            m_ErrProxyPage->m_pProxySOCKSPortCtrl->SetValidator(wxDefaultValidator);
            m_ErrProxyPage->m_pProxySOCKSUsernameCtrl->SetValidator(wxDefaultValidator);
            m_ErrProxyPage->m_pProxySOCKSPasswordCtrl->SetValidator(wxDefaultValidator);
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

