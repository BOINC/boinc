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
#pragma implementation "WizardAttachProject.h"
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
#include "browser.h"
#include "BOINCGUIApp.h"
#include "SkinManager.h"
#include "MainDocument.h"
#include "BOINCWizards.h"
#include "BOINCBaseWizard.h"
#include "BOINCBaseFrame.h"
#include "ProjectListCtrl.h"
#include "WizardAttachProject.h"
#include "WelcomePage.h"
#include "ProjectInfoPage.h"
#include "ProjectPropertiesPage.h"
#include "ProjectProcessingPage.h"
#include "AccountManagerInfoPage.h"
#include "AccountManagerPropertiesPage.h"
#include "AccountManagerProcessingPage.h"
#include "TermsOfUsePage.h"
#include "AccountInfoPage.h"
#include "CompletionPage.h"
#include "CompletionErrorPage.h"
#include "NotDetectedPage.h"
#include "UnavailablePage.h"
#include "NoInternetConnectionPage.h"
#include "NotFoundPage.h"
#include "AlreadyExistsPage.h"
#include "ProxyInfoPage.h"
#include "ProxyPage.h"


/*!
 * CWizardAttachProject type definition
 */
 
IMPLEMENT_DYNAMIC_CLASS( CWizardAttachProject, CBOINCBaseWizard )
 
/*!
 * CWizardAttachProject event table definition
 */
 
BEGIN_EVENT_TABLE( CWizardAttachProject, CBOINCBaseWizard )

////@begin CWizardAttachProject event table entries
    EVT_WIZARDEX_FINISHED( ID_ATTACHPROJECTWIZARD, CWizardAttachProject::OnFinished )

////@end CWizardAttachProject event table entries
 
END_EVENT_TABLE()
 
/*!
 * CWizardAttachProject constructors
 */
 
CWizardAttachProject::CWizardAttachProject()
{
}
 
CWizardAttachProject::CWizardAttachProject( wxWindow* parent, wxWindowID id, const wxPoint& pos )
{
    Create(parent, id, pos);
}
 
/*!
 * CWizardAttachProject creator
 */
 
bool CWizardAttachProject::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos )
{

////@begin CWizardAttachProject member initialisation
    m_WelcomePage = NULL;
    m_ProjectInfoPage = NULL;
    m_ProjectPropertiesPage = NULL;
    m_ProjectProcessingPage = NULL;
    m_AccountManagerInfoPage = NULL;
    m_AccountManagerPropertiesPage = NULL;
    m_AccountManagerProcessingPage = NULL;
    m_TermsOfUsePage = NULL;
    m_AccountInfoPage = NULL;
    m_CompletionPage = NULL;
    m_CompletionErrorPage = NULL;
    m_ErrNotDetectedPage = NULL;
    m_ErrUnavailablePage = NULL;
    m_ErrNoInternetConnectionPage = NULL;
    m_ErrNotFoundPage = NULL;
    m_ErrAlreadyExistsPage = NULL;
    m_ErrProxyInfoPage = NULL;
    m_ErrProxyPage = NULL;
////@end CWizardAttachProject member initialisation
  
    // Cancel Checking
    m_bCancelInProgress = false;
 
    // Wizard Detection
    IsAttachToProjectWizard = true;
    IsAccountManagerWizard = false;
    IsAccountManagerUpdateWizard = false;

    // Global wizard status
    project_config.clear();
    account_in.clear();
    account_out.clear();
    account_created_successfully = false;
    attached_to_project_successfully = false;
    project_url.Empty();
    project_authenticator.Empty();
    project_name.Empty();
    m_strProjectName.Empty();
    m_strReturnURL.Empty();
    m_bCredentialsCached = false;
    m_bCredentialsDetected = false;
    m_bCookieRequired = false;
    m_strCookieFailureURL.Empty();


    CSkinAdvanced*  pSkinAdvanced = wxGetApp().GetSkinManager()->GetAdvanced();
    CSkinWizardATP* pSkinWizardATP = wxGetApp().GetSkinManager()->GetWizards()->GetWizardATP();

    wxASSERT(pSkinAdvanced);
    wxASSERT(pSkinWizardATP);
    wxASSERT(wxDynamicCast(pSkinAdvanced, CSkinAdvanced));
    wxASSERT(wxDynamicCast(pSkinWizardATP, CSkinWizardATP));


    wxString strTitle;
    if (!pSkinWizardATP->GetWizardTitle().IsEmpty()) {
        strTitle = pSkinWizardATP->GetWizardTitle();
    } else {
        strTitle = pSkinAdvanced->GetApplicationName();
    }

////@begin CWizardAttachProject creation
    CBOINCBaseWizard::Create( parent, id, strTitle, pos );

    CreateControls();
////@end CWizardAttachProject creation

    return TRUE;
}

/*!
 * Control creation for CWizardAttachProject
 */

void CWizardAttachProject::CreateControls()
{    
    wxLogTrace(wxT("Function Start/End"), wxT("CWizardAttachProject::CreateControls - Function Begin"));
 
////@begin CWizardAttachProject content construction
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

    m_AccountManagerProcessingPage = new CAccountManagerProcessingPage;
    m_AccountManagerProcessingPage->Create( itemWizard1 );
    GetPageAreaSizer()->Add(m_AccountManagerProcessingPage);

    m_ProjectInfoPage = new CProjectInfoPage;
    m_ProjectInfoPage->Create( itemWizard1 );
    GetPageAreaSizer()->Add(m_ProjectInfoPage);

    m_ProjectPropertiesPage = new CProjectPropertiesPage;
    m_ProjectPropertiesPage->Create( itemWizard1 );
    GetPageAreaSizer()->Add(m_ProjectPropertiesPage);

    m_ProjectProcessingPage = new CProjectProcessingPage;
    m_ProjectProcessingPage->Create( itemWizard1 );
    GetPageAreaSizer()->Add(m_ProjectProcessingPage);

    m_AccountInfoPage = new CAccountInfoPage;
    m_AccountInfoPage->Create( itemWizard1 );
    GetPageAreaSizer()->Add(m_AccountInfoPage);

    m_TermsOfUsePage = new CTermsOfUsePage;
    m_TermsOfUsePage->Create( itemWizard1 );
    GetPageAreaSizer()->Add(m_TermsOfUsePage);

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

    m_ErrAlreadyExistsPage = new CErrAlreadyExistsPage;
    m_ErrAlreadyExistsPage->Create( itemWizard1 );
    GetPageAreaSizer()->Add(m_ErrAlreadyExistsPage);

    m_ErrProxyInfoPage = new CErrProxyInfoPage;
    m_ErrProxyInfoPage->Create( itemWizard1 );
    GetPageAreaSizer()->Add(m_ErrProxyInfoPage);

    m_ErrProxyPage = new CErrProxyPage;
    m_ErrProxyPage->Create( itemWizard1 );
    GetPageAreaSizer()->Add(m_ErrProxyPage);

////@end CWizardAttachProject content construction

    wxLogTrace(wxT("Function Status"), wxT("CWizardAttachProject::CreateControls - Begin Page Map"));

    wxLogTrace(
        wxT("Function Status"),
        wxT("CWizardAttachProject::CreateControls -     m_WelcomePage = id: '%d', location: '%p', height: '%d', width: '%d'"),
        m_WelcomePage->GetId(), m_WelcomePage, m_WelcomePage->GetBestSize().GetHeight(), m_WelcomePage->GetBestSize().GetWidth()
    );
    wxLogTrace(
        wxT("Function Status"), 
        wxT("CWizardAttachProject::CreateControls -     m_ProjectInfoPage = id: '%d', location: '%p', height: '%d', width: '%d'"),
        m_ProjectInfoPage->GetId(), m_ProjectInfoPage, m_ProjectInfoPage->GetBestSize().GetHeight(), m_ProjectInfoPage->GetBestSize().GetWidth()
    );
    wxLogTrace(
        wxT("Function Status"),
        wxT("CWizardAttachProject::CreateControls -     m_ProjectPropertiesPage = id: '%d', location: '%p', height: '%d', width: '%d'"),
        m_ProjectPropertiesPage->GetId(), m_ProjectPropertiesPage, m_ProjectPropertiesPage->GetBestSize().GetHeight(), m_ProjectPropertiesPage->GetBestSize().GetWidth()
    );
    wxLogTrace(
        wxT("Function Status"),
        wxT("CWizardAttachProject::CreateControls -     m_ProjectProcessingPage = id: '%d', location: '%p', height: '%d', width: '%d'"),
        m_ProjectProcessingPage->GetId(), m_ProjectProcessingPage, m_ProjectProcessingPage->GetBestSize().GetHeight(), m_ProjectProcessingPage->GetBestSize().GetWidth()
    );
    wxLogTrace(
        wxT("Function Status"),
        wxT("CWizardAttachProject::CreateControls -     m_AccountManagerInfoPage = id: '%d', location: '%p', height: '%d', width: '%d'"),
        m_AccountManagerInfoPage->GetId(), m_AccountManagerInfoPage, m_AccountManagerInfoPage->GetBestSize().GetHeight(), m_AccountManagerInfoPage->GetBestSize().GetWidth()
    );
    wxLogTrace(
        wxT("Function Status"),
        wxT("CWizardAttachProject::CreateControls -     m_AccountManagerPropertiesPage = id: '%d', location: '%p', height: '%d', width: '%d'"),
        m_AccountManagerPropertiesPage->GetId(), m_AccountManagerPropertiesPage, m_AccountManagerPropertiesPage->GetBestSize().GetHeight(), m_AccountManagerPropertiesPage->GetBestSize().GetWidth()
    );
    wxLogTrace(
        wxT("Function Status"),
        wxT("CWizardAttachProject::CreateControls -     m_AccountManagerProcessingPage = id: '%d', location: '%p', height: '%d', width: '%d'"),
        m_AccountManagerProcessingPage->GetId(), m_AccountManagerProcessingPage, m_AccountManagerProcessingPage->GetBestSize().GetHeight(), m_AccountManagerProcessingPage->GetBestSize().GetWidth()
    );
    wxLogTrace(
        wxT("Function Status"),
        wxT("CWizardAttachProject::CreateControls -     m_TermsOfUsePage = id: '%d', location: '%p', height: '%d', width: '%d'"),
        m_TermsOfUsePage->GetId(), m_TermsOfUsePage, m_TermsOfUsePage->GetBestSize().GetHeight(), m_TermsOfUsePage->GetBestSize().GetWidth()
    );
    wxLogTrace(
        wxT("Function Status"),
        wxT("CWizardAttachProject::CreateControls -     m_AccountInfoPage = id: '%d', location: '%p', height: '%d', width: '%d'"),
        m_AccountInfoPage->GetId(), m_AccountInfoPage, m_AccountInfoPage->GetBestSize().GetHeight(), m_AccountInfoPage->GetBestSize().GetWidth()
    );
    wxLogTrace(
        wxT("Function Status"),
        wxT("CWizardAttachProject::CreateControls -     m_CompletionPage = id: '%d', location: '%p', height: '%d', width: '%d'"),
        m_CompletionPage->GetId(), m_CompletionPage, m_CompletionPage->GetBestSize().GetHeight(), m_CompletionPage->GetBestSize().GetWidth()
    );
    wxLogTrace(
        wxT("Function Status"),
        wxT("CWizardAttachProject::CreateControls -     m_CompletionErrorPage = id: '%d', location: '%p', height: '%d', width: '%d'"),
        m_CompletionErrorPage->GetId(), m_CompletionErrorPage, m_CompletionErrorPage->GetBestSize().GetHeight(), m_CompletionErrorPage->GetBestSize().GetWidth()
    );
    wxLogTrace(
        wxT("Function Status"),
        wxT("CWizardAttachProject::CreateControls -     m_ErrNotDetectedPage = id: '%d', location: '%p', height: '%d', width: '%d'"),
        m_ErrNotDetectedPage->GetId(), m_ErrNotDetectedPage, m_ErrNotDetectedPage->GetBestSize().GetHeight(), m_ErrNotDetectedPage->GetBestSize().GetWidth()
    );
    wxLogTrace(
        wxT("Function Status"),
        wxT("CWizardAttachProject::CreateControls -     m_ErrUnavailablePage = id: '%d', location: '%p', height: '%d', width: '%d'"),
        m_ErrUnavailablePage->GetId(), m_ErrUnavailablePage, m_ErrUnavailablePage->GetBestSize().GetHeight(), m_ErrUnavailablePage->GetBestSize().GetWidth()
    );
    wxLogTrace(
        wxT("Function Status"),
        wxT("CWizardAttachProject::CreateControls -     m_ErrNoInternetConnectionPage = id: '%d', location: '%p', height: '%d', width: '%d'"),
        m_ErrNoInternetConnectionPage->GetId(), m_ErrNoInternetConnectionPage, m_ErrNoInternetConnectionPage->GetBestSize().GetHeight(), m_ErrNoInternetConnectionPage->GetBestSize().GetWidth()
    );
    wxLogTrace(
        wxT("Function Status"),
        wxT("CWizardAttachProject::CreateControls -     m_ErrNotFoundPage = id: '%d', location: '%p', height: '%d', width: '%d'"),
        m_ErrNotFoundPage->GetId(), m_ErrNotFoundPage, m_ErrNotFoundPage->GetBestSize().GetHeight(), m_ErrNotFoundPage->GetBestSize().GetWidth()
    );
    wxLogTrace(
        wxT("Function Status"),
        wxT("CWizardAttachProject::CreateControls -     m_ErrAlreadyExistsPage = id: '%d', location: '%p', height: '%d', width: '%d'"),
        m_ErrAlreadyExistsPage->GetId(), m_ErrAlreadyExistsPage, m_ErrAlreadyExistsPage->GetBestSize().GetHeight(), m_ErrAlreadyExistsPage->GetBestSize().GetWidth()
    );
    wxLogTrace(
        wxT("Function Status"),
        wxT("CWizardAttachProject::CreateControls -     m_ErrProxyInfoPage = id: '%d', location: '%p', height: '%d', width: '%d'"),
        m_ErrProxyInfoPage->GetId(), m_ErrProxyInfoPage, m_ErrProxyInfoPage->GetBestSize().GetHeight(), m_ErrProxyInfoPage->GetBestSize().GetWidth()
    );
    wxLogTrace(
        wxT("Function Status"),
        wxT("CWizardAttachProject::CreateControls -     m_ErrProxyPage = id: '%d', location: '%p', height: '%d', width: '%d'"),
        m_ErrProxyPage->GetId(), m_ErrProxyPage, m_ErrProxyPage->GetBestSize().GetHeight(), m_ErrProxyPage->GetBestSize().GetWidth()
    );

    wxLogTrace(wxT("Function Status"), wxT("CWizardAttachProject::CreateControls - End Page Map"));
    wxLogTrace(wxT("Function Start/End"), wxT("CWizardAttachProject::CreateControls - Function End"));
}
 
/*!
 * Runs the wizard.
 */
bool CWizardAttachProject::Run(
    wxString& WXUNUSED(strName), wxString& strURL, std::string& _team_name,
    bool bCredentialsCached
) {
    team_name = _team_name;
    if (strURL.Length()) {
        m_ProjectInfoPage->SetProjectURL( strURL );
        m_bCredentialsCached = bCredentialsCached;
    }

    // If credentials are not cached, then we should try one last place to look up the
    //   authenticator.  Some projects will set a "Setup" cookie off of their URL with a
    //   pretty short timeout.  Lets take a crack at detecting it.
    //
    if (!strURL.IsEmpty() && !bCredentialsCached) {
        std::string url = std::string(strURL.mb_str());
        std::string authenticator;

        if (detect_setup_authenticator(url, authenticator)) {
            m_bCredentialsDetected = true;
            close_when_completed = true;
            SetProjectAuthenticator(wxString(authenticator.c_str(), wxConvUTF8));
        }
    }

    if (strURL.Length() && m_ProjectPropertiesPage) {
        return RunWizard(m_ProjectPropertiesPage);
    } else if (m_WelcomePage) {
        return RunWizard(m_WelcomePage);
    }

    return FALSE;
}


bool CWizardAttachProject::SyncToAccountManager() {
    ACCT_MGR_INFO ami;
    CMainDocument* pDoc = wxGetApp().GetDocument();

    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));

    IsAttachToProjectWizard = false;
    IsAccountManagerWizard = true;

    pDoc->rpc.acct_mgr_info(ami);

    if (ami.acct_mgr_url.size()) {
        m_AccountManagerInfoPage->SetProjectURL( wxString(ami.acct_mgr_url.c_str(), wxConvUTF8) );
        m_strProjectName = wxString(ami.acct_mgr_name.c_str(), wxConvUTF8);
        m_bCredentialsCached = ami.have_credentials;
        m_bCookieRequired = ami.cookie_required;
        m_strCookieFailureURL = wxString(ami.cookie_failure_url.c_str(), wxConvUTF8);

        if (m_bCredentialsCached) {
            IsAccountManagerUpdateWizard = true;
        }
    }

    if (ami.acct_mgr_url.size() && !m_bCredentialsCached) {
        std::string login;
        std::string password_hash;
        std::string return_url;

        if (detect_account_manager_credentials(ami.acct_mgr_url, login, password_hash, return_url)) {
            wxString strLogin;
            wxString strPasswordHash;
            wxString strReturnURL;

            strLogin = wxURL::Unescape( wxString(login.c_str(), wxConvUTF8) );
            strPasswordHash = wxURL::Unescape( wxString(password_hash.c_str(), wxConvUTF8) );
            strReturnURL = wxURL::Unescape( wxString(return_url.c_str(), wxConvUTF8) );

            m_AccountInfoPage->SetAccountEmailAddress( strLogin );
            m_AccountInfoPage->SetAccountPassword(
                wxString(_T("hash:")) +
                strPasswordHash
            );
            m_strReturnURL = strReturnURL;
            m_bCredentialsDetected = true;
        }
    }

    if (m_AccountManagerPropertiesPage) {
        return RunWizard(m_AccountManagerPropertiesPage);
    }

    return FALSE;
}

/*!
 * Should we show tooltips?
 */
 
bool CWizardAttachProject::ShowToolTips()
{
    return TRUE;
}
 
/*!
 * Get bitmap resources
 */
 
wxBitmap CWizardAttachProject::GetBitmapResource( const wxString& WXUNUSED(name) )
{
    // Bitmap retrieval
////@begin CWizardAttachProject bitmap retrieval
    return wxNullBitmap;
////@end CWizardAttachProject bitmap retrieval
}
 
/*!
 * Get icon resources
 */
 
wxIcon CWizardAttachProject::GetIconResource( const wxString& WXUNUSED(name) )
{
    // Icon retrieval
////@begin CWizardAttachProject icon retrieval
    return wxNullIcon;
////@end CWizardAttachProject icon retrieval
}
 
/*!
 * Determine if the wizard page has a next page
 */

bool CWizardAttachProject::HasNextPage( wxWizardPageEx* page )
{
    bool bNoNextPageDetected = false;

    bNoNextPageDetected |= (page == m_CompletionPage);
    bNoNextPageDetected |= (page == m_CompletionErrorPage);
    bNoNextPageDetected |= (page == m_ErrNotDetectedPage);
    bNoNextPageDetected |= (page == m_ErrUnavailablePage);
    bNoNextPageDetected |= (page == m_ErrNoInternetConnectionPage);
    bNoNextPageDetected |= (page == m_ErrAlreadyExistsPage);
 
    if (bNoNextPageDetected)
        return false;
    return true;
}
  
/*!
 * Determine if the wizard page has a previous page
 */
 
bool CWizardAttachProject::HasPrevPage( wxWizardPageEx* page )
{
    if ((page == m_WelcomePage) || (page == m_CompletionPage) || (page == m_CompletionErrorPage))
        return false;
    return true;
}
 
/*!
 * Remove the page transition from the stack.
 */
wxWizardPageEx* CWizardAttachProject::_PopPageTransition() {
    wxWizardPageEx* pPage = NULL;
    if (GetCurrentPage()) {
        if (m_PageTransition.size() > 0) {
            pPage = m_PageTransition.top();
            m_PageTransition.pop();
            if ((pPage == m_ProjectPropertiesPage) || (pPage == m_ProjectProcessingPage) ||
                (pPage == m_AccountManagerPropertiesPage) || (pPage == m_AccountManagerProcessingPage)) 
            {
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
wxWizardPageEx* CWizardAttachProject::_PushPageTransition( wxWizardPageEx* pCurrentPage, unsigned long ulPageID ) {
    if (GetCurrentPage()) {
        wxWizardPageEx* pPage = NULL;

        if (ID_WELCOMEPAGE == ulPageID)
            pPage = m_WelcomePage;
 
        if (ID_PROJECTINFOPAGE == ulPageID)
            pPage = m_ProjectInfoPage;
 
        if (ID_PROJECTPROPERTIESPAGE == ulPageID)
            pPage = m_ProjectPropertiesPage;
 
        if (ID_PROJECTPROCESSINGPAGE == ulPageID)
            pPage = m_ProjectProcessingPage;
 
        if (ID_ACCOUNTMANAGERINFOPAGE == ulPageID)
            pPage = m_AccountManagerInfoPage;
 
        if (ID_ACCOUNTMANAGERPROPERTIESPAGE == ulPageID)
            pPage = m_AccountManagerPropertiesPage;
 
        if (ID_ACCOUNTMANAGERPROCESSINGPAGE == ulPageID)
            pPage = m_AccountManagerProcessingPage;
 
        if (ID_TERMSOFUSEPAGE == ulPageID)
            pPage = m_TermsOfUsePage;
 
        if (ID_ACCOUNTINFOPAGE == ulPageID)
            pPage = m_AccountInfoPage;
 
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
 
        if (ID_ERRALREADYEXISTSPAGE == ulPageID)
            pPage = m_ErrAlreadyExistsPage;
 
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
  
void CWizardAttachProject::_ProcessCancelEvent( wxWizardExEvent& event ) {

    bool bCancelWithoutNextPage = false;
    wxWizardPageEx* page = GetCurrentPage();

    int iRetVal = wxGetApp().SafeMessageBox(
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
        if ((page == m_ProjectInfoPage) || (page == m_AccountManagerInfoPage)) {
            m_ProjectInfoPage->m_pProjectUrlCtrl->SetValidator(wxDefaultValidator);
        } else if (page == m_AccountInfoPage) {
            m_AccountInfoPage->m_pAccountEmailAddressCtrl->SetValidator(wxDefaultValidator);
            m_AccountInfoPage->m_pAccountPasswordCtrl->SetValidator(wxDefaultValidator);
            if (IsAttachToProjectWizard) {
                m_AccountInfoPage->m_pAccountConfirmPasswordCtrl->SetValidator(wxDefaultValidator);
            }
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
    bCancelWithoutNextPage |= (page == m_ErrNotDetectedPage);
    bCancelWithoutNextPage |= (page == m_ErrUnavailablePage);
    bCancelWithoutNextPage |= (page == m_ErrNoInternetConnectionPage);
    if (IsAttachToProjectWizard) {
        bCancelWithoutNextPage |= (page == m_ErrAlreadyExistsPage);
    } else {
        bCancelWithoutNextPage |= (page == m_WelcomePage);
    }
    if (wxYES != iRetVal) {
        event.Veto();
    }
}

/*!
 * wxEVT_WIZARD_FINISHED event handler for ID_ATTACHPROJECTWIZARD
 */

void CWizardAttachProject::OnFinished( wxWizardExEvent& event ) {

    if (IsAccountManagerWizard) {
        // Attached to an account manager
        if (!m_strReturnURL.empty() && GetAttachedToProjectSuccessfully()) {
            wxLaunchDefaultBrowser(m_strReturnURL);
        }
    } else {
        // Attached to a project
        if (GetAccountCreatedSuccessfully() && GetAttachedToProjectSuccessfully()) {
            wxLaunchDefaultBrowser(GetProjectURL() + wxT("account_finish.php?auth=") + GetProjectAuthenticator());
        }
    }

    // Let the framework clean things up.
    event.Skip();
}

