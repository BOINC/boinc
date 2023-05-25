// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2023 University of California
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
#pragma implementation "WizardAttach.h"
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
#include "BOINCBaseFrame.h"
#include "WizardAttach.h"
#include "ProjectInfoPage.h"
#include "ProjectPropertiesPage.h"
#include "ProjectProcessingPage.h"
#include "ProjectWelcomePage.h"
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
 * CWizardAttach type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CWizardAttach, CBOINCBaseWizard )

/*!
 * CWizardAttach event table definition
 */

BEGIN_EVENT_TABLE( CWizardAttach, CBOINCBaseWizard )
////@begin CWizardAttach event table entries
    EVT_WIZARDEX_FINISHED( ID_ATTACHWIZARD, CWizardAttach::OnFinished )
////@end CWizardAttach event table entries
END_EVENT_TABLE()

/*!
 * CWizardAttach constructors
 */

CWizardAttach::CWizardAttach()
{
}

CWizardAttach::CWizardAttach( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, long style )
{
    Create(parent, id, title, pos, style);
}

/*!
 * CWizardAttach creator
 */

bool CWizardAttach::Create( wxWindow* parent, wxWindowID id, const wxString& /* title */, const wxPoint& pos, long style )
{

////@begin CWizardAttach member initialisation
    m_ProjectInfoPage = NULL;
    m_ProjectPropertiesPage = NULL;
    m_ProjectProcessingPage = NULL;
    m_ProjectWelcomePage = NULL;
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
    m_ErrUserDisagreesPage = NULL;
////@end CWizardAttach member initialisation

    // Cancel Checking
    m_bCancelInProgress = false;

    // Wizard Detection

    IsAttachToProjectWizard = true;
    IsChangeWCGApps = false;
    IsFirstPass = false;
    IsAccountManagerWizard = false;
    IsAccountManagerUpdateWizard = false;

    // Global wizard status
    project_config.clear();
    account_in.clear();
    account_out.clear();
    account_created_successfully = false;
    attached_to_project_successfully = false;
    m_bCloseWhenCompleted = false;
    m_strProjectName.Empty();
    m_strProjectUrl.Empty();
    m_strProjectAuthenticator.Empty();
    m_strReturnURL.Empty();
    m_bCredentialsCached = false;
    m_bCredentialsDetected = false;
    m_bConsentedToTerms = false;


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

////@begin CWizardAttach creation
    CBOINCBaseWizard::Create( parent, id, strTitle, pos, style );

    CreateControls();
////@end CWizardAttach creation

    return TRUE;
}

/*!
 * Control creation for CWizardAttach
 */

void CWizardAttach::CreateControls()
{
    wxLogTrace(wxT("Function Start/End"), wxT("CWizardAttach::CreateControls - Function Begin"));

////@begin CWizardAttach content construction
    CWizardAttach* itemWizard1 = this;

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

    m_ProjectWelcomePage = new CProjectWelcomePage;
    m_ProjectWelcomePage->Create( itemWizard1 );
    GetPageAreaSizer()->Add(m_ProjectWelcomePage);

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

////@end CWizardAttach content construction

    wxLogTrace(wxT("Function Start/End"), wxT("CWizardAttach::CreateControls - Function End"));
}

/*!
 * Runs the wizard.
 */
bool CWizardAttach::Run(
        wxString strProjectName,
        wxString strProjectURL,
        wxString strProjectAuthenticator,
        wxString strProjectInstitution,
        wxString strProjectDescription,
        wxString strProjectKnown,
        bool     bAccountKeyDetected,
        bool     /* bEmbedded */
){
    SetProjectName(strProjectName);
    if (strProjectURL.size()) {
        SetProjectURL(strProjectURL);
        SetCredentialsCached(bAccountKeyDetected);
    }
    SetProjectInstitution(strProjectInstitution);
    SetProjectDescription(strProjectDescription);
    if (strProjectAuthenticator.size()) {
        SetProjectAuthenticator(strProjectAuthenticator);
    }
    SetProjectKnown(strProjectKnown.length() > 0);

    if (strProjectURL.size() && strProjectAuthenticator.size() && !bAccountKeyDetected) {
        SetCredentialsDetected(true);
        SetCloseWhenCompleted(true);
    }

    if (m_ProjectPropertiesPage && m_ProjectInfoPage && m_ProjectWelcomePage) {
        IsAttachToProjectWizard = true;
        IsAccountManagerWizard = false;
        if (strProjectName.size() && strProjectURL.size()) {
            return RunWizard(m_ProjectWelcomePage);
        } else if (strProjectURL.size() && (IsCredentialsCached() || IsCredentialsDetected())) {
            return RunWizard(m_ProjectPropertiesPage);
        } else {
            return RunWizard(m_ProjectInfoPage);
        }
    }

    return FALSE;
}


bool CWizardAttach::SyncToAccountManager() {
    ACCT_MGR_INFO ami;
    CMainDocument* pDoc = wxGetApp().GetDocument();

    CSkinAdvanced*  pSkinAdvanced = wxGetApp().GetSkinManager()->GetAdvanced();
    CSkinWizardATAM* pSkinWizardATAM = wxGetApp().GetSkinManager()->GetWizards()->GetWizardATAM();

   wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));
    wxASSERT(pSkinAdvanced);
    wxASSERT(pSkinWizardATAM);
    wxASSERT(wxDynamicCast(pSkinAdvanced, CSkinAdvanced));
    wxASSERT(wxDynamicCast(pSkinWizardATAM, CSkinWizardATAM));


     if (!pSkinWizardATAM->GetWizardTitle().IsEmpty()) {
        SetTitle(pSkinWizardATAM->GetWizardTitle());
    } else {
        SetTitle(pSkinAdvanced->GetApplicationName());
    }

    IsAttachToProjectWizard = false;
    IsAccountManagerWizard = true;


    pDoc->rpc.acct_mgr_info(ami);

    if (ami.acct_mgr_url.size()) {
        SetProjectName(wxString(ami.acct_mgr_name.c_str(), wxConvUTF8));
        SetProjectURL(wxString(ami.acct_mgr_url.c_str(), wxConvUTF8));

        SetCredentialsCached(ami.have_credentials);
        if (IsCredentialsCached()) {
            IsAccountManagerUpdateWizard = true;
        }
    }

    if (m_AccountManagerInfoPage || m_AccountManagerPropertiesPage) {
        IsAttachToProjectWizard = false;
        IsAccountManagerWizard = true;
        if (!ami.acct_mgr_url.size()) {
            return RunWizard(m_AccountManagerInfoPage);
        } else {
            return RunWizard(m_AccountManagerPropertiesPage);
        }
    }

    return FALSE;
}

/*!
 * Should we show tooltips?
 */

bool CWizardAttach::ShowToolTips()
{
    return TRUE;
}

/*!
 * Get bitmap resources
 */

wxBitmap CWizardAttach::GetBitmapResource( const wxString& WXUNUSED(name) )
{
    // Bitmap retrieval
////@begin CWizardAttach bitmap retrieval
    return wxNullBitmap;
////@end CWizardAttach bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CWizardAttach::GetIconResource( const wxString& WXUNUSED(name) )
{
    // Icon retrieval
////@begin CWizardAttach icon retrieval
    return wxNullIcon;
////@end CWizardAttach icon retrieval
}

/*!
 * Determine if the wizard page has a next page
 */

bool CWizardAttach::HasNextPage( wxWizardPageEx* page )
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

bool CWizardAttach::HasPrevPage( wxWizardPageEx* page )
{
    bool bNoPrevPageDetected = false;

    bNoPrevPageDetected |= (page == m_ProjectWelcomePage);
    bNoPrevPageDetected |= (page == m_ProjectInfoPage);
    bNoPrevPageDetected |= (page == m_AccountManagerInfoPage);
    bNoPrevPageDetected |= (page == m_CompletionPage);
    bNoPrevPageDetected |= (page == m_CompletionErrorPage);

    if (bNoPrevPageDetected)
        return false;
    return true;
}

/*!
 * Translate a Page ID into the wxWizardPageEx instance pointer.
 */

wxWizardPageEx* CWizardAttach::TranslatePage(unsigned long ulPageID) {
    wxWizardPageEx* pPage = NULL;

    if (ID_PROJECTINFOPAGE == ulPageID)
        pPage = m_ProjectInfoPage;

    if (ID_PROJECTPROPERTIESPAGE == ulPageID)
        pPage = m_ProjectPropertiesPage;

    if (ID_PROJECTPROCESSINGPAGE == ulPageID)
        pPage = m_ProjectProcessingPage;

    if (ID_PROJECTWELCOMEPAGE == ulPageID)
        pPage = m_ProjectWelcomePage;

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

    return pPage;
}

/*!
 * Remove the page transition from the stack.
 */
wxWizardPageEx* CWizardAttach::PopPageTransition() {
    wxWizardPageEx* pPage = NULL;
    if (GetCurrentPage()) {
        if (!m_PageTransition.empty()) {
            pPage = m_PageTransition.top();

            wxLogTrace(wxT("Function Status"), wxT("CWizardAttach::PopPageTransition - Popping Page: '%p'"), pPage);
            m_PageTransition.pop();

            // TODO: Figure out the best way to handle the situation where the wizard has been launched with a
            //   project init file and the volunteer hits the back button on the m_ProjectPropertiesPage/
            //   m_AccountManagerPropertiesPage page.  Ideally they go back to the m_ProjectInfoPage/
            //   m_AccountManagerInfoPage page, but since the wizard launched in automatic attach mode
            //   that page isn't on the stack and the manager crashes.
            //
            //   It is probably enough to just push the correct InfoPage on the stack before launching the
            //   wizard in automatic mode.  I need to think about it some more.
            //
            if ((pPage == m_ProjectPropertiesPage) || (pPage == m_ProjectProcessingPage) ||
                (pPage == m_AccountManagerPropertiesPage) || (pPage == m_AccountManagerProcessingPage))
            {
                // We want to go back to the page before we attempted to communicate
                //   with any server.
                pPage = m_PageTransition.top();

                wxLogTrace(wxT("Function Status"), wxT("CWizardAttach::PopPageTransition - Popping Page: '%p'"), pPage);
                m_PageTransition.pop();

            }
            wxASSERT(pPage);
            return pPage;
        }
    }
    return NULL;
}


/*!
 * Push a page onto the stack.
 */

wxWizardPageEx* CWizardAttach::PushPage( unsigned long ulPageID ) {
    if (GetCurrentPage()) {
        wxWizardPageEx* pPage = TranslatePage(ulPageID);
        if (pPage) {
            wxLogTrace(wxT("Function Status"), wxT("CWizardAttach::PushPage - Pushing Page: '%p'"), pPage);
            m_PageTransition.push(pPage);
            return pPage;
        }
    }
    return NULL;
}


/*!
 * Add the page transition to the stack.
 */
wxWizardPageEx* CWizardAttach::PushPageTransition( wxWizardPageEx* pCurrentPage, unsigned long ulPageID ) {
    if (GetCurrentPage()) {
        wxWizardPageEx* pPage = TranslatePage(ulPageID);
        if (pPage) {
            if (m_PageTransition.empty()) {
                wxLogTrace(wxT("Function Status"), wxT("CWizardAttach::PushPageTransition - Pushing Page: '%p'"), pPage);
                m_PageTransition.push(NULL);
            }
            if (m_PageTransition.top() != pCurrentPage) {
                wxLogTrace(wxT("Function Status"), wxT("CWizardAttach::PushPageTransition - Pushing Page: '%p'"), pPage);
                m_PageTransition.push(pCurrentPage);
            }
            return pPage;
        }
    }
    return NULL;
}

void CWizardAttach::_ProcessCancelEvent( wxWizardExEvent& event ) {

    wxWizardPageEx* page = GetCurrentPage();

    m_bCancelInProgress = true;

    int iRetVal = wxGetApp().SafeMessageBox(
        _("Do you really want to cancel?"),
        _("Confirmation"),
        wxICON_QUESTION | wxYES_NO,
        this
    );

    // Reenable the next and back buttons if they have been disabled
    GetNextButton()->Enable(HasNextPage(page));
    GetBackButton()->Enable(HasPrevPage(page));

    if (wxYES != iRetVal) {
        event.Veto();
        m_bCancelInProgress = false;
    } else {
        m_bCancelInProgress = true;
    }
}

/*!
 * wxEVT_WIZARD_FINISHED event handler for ID_ATTACHPROJECTWIZARD
 */

void CWizardAttach::OnFinished( wxWizardExEvent& event ) {

    if (IsAccountManagerWizard) {
        // Attached to an account manager
        if (!GetReturnURL().empty() && GetAttachedToProjectSuccessfully()) {
            wxLaunchDefaultBrowser(GetReturnURL());
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

