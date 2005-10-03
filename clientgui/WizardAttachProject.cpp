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
#pragma implementation "WizardAttachProject.h"
#endif

#include "stdwx.h"
#include "wizardex.h"
#include "error_numbers.h"
#include "BOINCGUIApp.h"
#include "BOINCWizards.h"
#include "BOINCBaseWizard.h"
#include "WizardAttachProject.h"
#include "WelcomePage.h"
#include "ProjectInfoPage.h"
#include "ProjectPropertiesPage.h"
#include "AccountInfoPage.h"
#include "AccountKeyPage.h"
#include "ProjectProcessingPage.h"
#include "CompletionPage.h"
#include "CompletionErrorPage.h"
#include "NotDetectedPage.h"
#include "UnavailablePage.h"
#include "AlreadyAttachedPage.h"
#include "NoInternetConnectionPage.h"
#include "NotFoundPage.h"
#include "AlreadyExistsPage.h"
#include "ProxyPage.h"

////@begin XPM images
#include "res/attachprojectwizard.xpm"
////@end XPM images


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
    m_AccountKeyPage = NULL;
    m_AccountInfoPage = NULL;
    m_ProjectProcessingPage = NULL;
    m_CompletionPage = NULL;
    m_CompletionErrorPage = NULL;
    m_ErrNotDetectedPage = NULL;
    m_ErrUnavailablePage = NULL;
    m_ErrAlreadyAttachedPage = NULL;
    m_ErrNoInternetConnectionPage = NULL;
    m_ErrNotFoundPage = NULL;
    m_ErrAlreadyExistsPage = NULL;
    m_ErrProxyPage = NULL;
////@end CWizardAttachProject member initialisation
  
    // Wizard support
    m_ulDiagFlags = 0;

    // Cancel Checking
    m_bCancelInProgress = false;
 
    // Wizard Detection
    IsAttachToProjectWizard = true;
    IsAccountManagerWizard = false;

    // Global wizard status
    project_config.clear();
    account_in.clear();
    account_out.clear();
    account_created_successfully = false;
    attached_to_project_successfully = false;
    project_url = wxEmptyString;
    project_authenticator = wxEmptyString;
    m_bCredentialsCached = false;
 
////@begin CWizardAttachProject creation
    wxBitmap wizardBitmap(GetBitmapResource(wxT("res/attachprojectwizard.xpm")));
    CBOINCBaseWizard::Create( parent, id, _("Attach to Project"), wizardBitmap, pos );

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

    itemWizard1->FitToPage(m_WelcomePage);
    m_ProjectInfoPage = new CProjectInfoPage;
    m_ProjectInfoPage->Create( itemWizard1 );

    itemWizard1->FitToPage(m_ProjectInfoPage);
    m_ProjectPropertiesPage = new CProjectPropertiesPage;
    m_ProjectPropertiesPage->Create( itemWizard1 );

    itemWizard1->FitToPage(m_ProjectPropertiesPage);
    m_AccountKeyPage = new CAccountKeyPage;
    m_AccountKeyPage->Create( itemWizard1 );

    itemWizard1->FitToPage(m_AccountKeyPage);
    m_AccountInfoPage = new CAccountInfoPage;
    m_AccountInfoPage->Create( itemWizard1 );

    itemWizard1->FitToPage(m_AccountInfoPage);
    m_ProjectProcessingPage = new CProjectProcessingPage;
    m_ProjectProcessingPage->Create( itemWizard1 );

    itemWizard1->FitToPage(m_ProjectProcessingPage);
    m_CompletionPage = new CCompletionPage;
    m_CompletionPage->Create( itemWizard1 );

    itemWizard1->FitToPage(m_CompletionPage);
    m_CompletionErrorPage = new CCompletionErrorPage;
    m_CompletionErrorPage->Create( itemWizard1 );

    itemWizard1->FitToPage(m_CompletionErrorPage);
    m_ErrNotDetectedPage = new CErrNotDetectedPage;
    m_ErrNotDetectedPage->Create( itemWizard1 );

    itemWizard1->FitToPage(m_ErrNotDetectedPage);
    m_ErrUnavailablePage = new CErrUnavailablePage;
    m_ErrUnavailablePage->Create( itemWizard1 );

    itemWizard1->FitToPage(m_ErrUnavailablePage);
    m_ErrAlreadyAttachedPage = new CErrAlreadyAttachedPage;
    m_ErrAlreadyAttachedPage->Create( itemWizard1 );

    itemWizard1->FitToPage(m_ErrAlreadyAttachedPage);
    m_ErrNoInternetConnectionPage = new CErrNoInternetConnectionPage;
    m_ErrNoInternetConnectionPage->Create( itemWizard1 );

    itemWizard1->FitToPage(m_ErrNoInternetConnectionPage);
    m_ErrNotFoundPage = new CErrNotFoundPage;
    m_ErrNotFoundPage->Create( itemWizard1 );

    itemWizard1->FitToPage(m_ErrNotFoundPage);
    m_ErrAlreadyExistsPage = new CErrAlreadyExistsPage;
    m_ErrAlreadyExistsPage->Create( itemWizard1 );

    itemWizard1->FitToPage(m_ErrAlreadyExistsPage);
    m_ErrProxyPage = new CErrProxyPage;
    m_ErrProxyPage->Create( itemWizard1 );

    itemWizard1->FitToPage(m_ErrProxyPage);
    wxWizardPageSimple* lastPage = NULL;
////@end CWizardAttachProject content construction
 
    wxLogTrace(wxT("Function Status"), wxT("CWizardAttachProject::CreateControls - Begin Page Map"));
    wxLogTrace(wxT("Function Status"), wxT("CWizardAttachProject::CreateControls -     m_WelcomePage = id: '%d', location: '%p'"), ID_WELCOMEPAGE, m_WelcomePage);
    wxLogTrace(wxT("Function Status"), wxT("CWizardAttachProject::CreateControls -     m_ProjectInfoPage = id: '%d', location: '%p'"), ID_PROJECTINFOPAGE, m_ProjectInfoPage);
    wxLogTrace(wxT("Function Status"), wxT("CWizardAttachProject::CreateControls -     m_ProjectPropertiesPage = id: '%d', location: '%p'"), ID_PROJECTPROPERTIESPAGE, m_ProjectPropertiesPage);
    wxLogTrace(wxT("Function Status"), wxT("CWizardAttachProject::CreateControls -     m_AccountKeyPage = id: '%d', location: '%p'"), ID_ACCOUNTKEYPAGE, m_AccountKeyPage);
    wxLogTrace(wxT("Function Status"), wxT("CWizardAttachProject::CreateControls -     m_AccountInfoPage = id: '%d', location: '%p'"), ID_ACCOUNTINFOPAGE, m_AccountInfoPage);
    wxLogTrace(wxT("Function Status"), wxT("CWizardAttachProject::CreateControls -     m_ProjectProcessingPage = id: '%d', location: '%p'"), ID_PROJECTPROCESSINGPAGE, m_ProjectProcessingPage);
    wxLogTrace(wxT("Function Status"), wxT("CWizardAttachProject::CreateControls -     m_CompletionPage = id: '%d', location: '%p'"), ID_COMPLETIONPAGE, m_CompletionPage);
    wxLogTrace(wxT("Function Status"), wxT("CWizardAttachProject::CreateControls -     m_CompletionErrorPage = id: '%d', location: '%p'"), ID_COMPLETIONERRORPAGE, m_CompletionErrorPage);
    wxLogTrace(wxT("Function Status"), wxT("CWizardAttachProject::CreateControls -     m_ErrNotDetectedPage = id: '%d', location: '%p'"), ID_ERRNOTDETECTEDPAGE, m_ErrNotDetectedPage);
    wxLogTrace(wxT("Function Status"), wxT("CWizardAttachProject::CreateControls -     m_ErrUnavailablePage = id: '%d', location: '%p'"), ID_ERRUNAVAILABLEPAGE, m_ErrUnavailablePage);
    wxLogTrace(wxT("Function Status"), wxT("CWizardAttachProject::CreateControls -     m_ErrAlreadyAttachedPage = id: '%d', location: '%p'"), ID_ERRALREADYATTACHEDPAGE, m_ErrAlreadyAttachedPage);
    wxLogTrace(wxT("Function Status"), wxT("CWizardAttachProject::CreateControls -     m_ErrNoInternetConnectionPage = id: '%d', location: '%p'"), ID_ERRNOINTERNETCONNECTIONPAGE, m_ErrNoInternetConnectionPage);
    wxLogTrace(wxT("Function Status"), wxT("CWizardAttachProject::CreateControls -     m_ErrNotFoundPage = id: '%d', location: '%p'"), ID_ERRNOTFOUNDPAGE, m_ErrNotFoundPage);
    wxLogTrace(wxT("Function Status"), wxT("CWizardAttachProject::CreateControls -     m_ErrAlreadyExistsPage = id: '%d', location: '%p'"), ID_ERRALREADYEXISTSPAGE, m_ErrAlreadyExistsPage);
    wxLogTrace(wxT("Function Status"), wxT("CWizardAttachProject::CreateControls -     m_ErrProxyPage = id: '%d', location: '%p'"), ID_ERRPROXYPAGE, m_ErrProxyPage);
    wxLogTrace(wxT("Function Status"), wxT("CWizardAttachProject::CreateControls - End Page Map"));
    wxLogTrace(wxT("Function Start/End"), wxT("CWizardAttachProject::CreateControls - Function End"));
}
 
/*!
 * Runs the wizard.
 */
 
bool CWizardAttachProject::Run( wxString& strName, wxString& strURL, bool bCredentialsCached )
{
    if (strURL.Length()) {
        if (strName.Length()) {
            wxString strTitle;
            strTitle = GetTitle();
            strTitle += wxT(" - ") + strName;
            SetTitle(strTitle);
        }

        m_ProjectInfoPage->SetProjectURL( strURL );
        m_bCredentialsCached = bCredentialsCached;
    }

    if ( strURL.Length() && bCredentialsCached && m_ProjectProcessingPage) {
        return RunWizard(m_ProjectProcessingPage);
    } else if (strURL.Length() && !bCredentialsCached && m_ProjectPropertiesPage) {
        return RunWizard(m_ProjectPropertiesPage);
    } else if (m_WelcomePage) {
        return RunWizard(m_WelcomePage);
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
 
wxBitmap CWizardAttachProject::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CWizardAttachProject bitmap retrieval
    if (name == wxT("res/attachprojectwizard.xpm"))
    {
        wxBitmap bitmap(attachprojectwizard_xpm);
        return bitmap;
    }
    return wxNullBitmap;
////@end CWizardAttachProject bitmap retrieval
}
 
/*!
 * Get icon resources
 */
 
wxIcon CWizardAttachProject::GetIconResource( const wxString& name )
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
    bNoNextPageDetected |= (page == m_ErrNotFoundPage);
    bNoNextPageDetected |= (page == m_ErrUnavailablePage);
    bNoNextPageDetected |= (page == m_ErrAlreadyAttachedPage);
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
    if ((page == m_WelcomePage) || (page == m_CompletionErrorPage))
        return false;
    return true;
}
 
/*!
 * Remove the page transition to the stack.
 */
wxWizardPageEx* CWizardAttachProject::_PopPageTransition() {
    wxWizardPageEx* pPage = NULL;
    if (GetCurrentPage()) {
        if (m_PageTransition.size() > 0) {
            pPage = m_PageTransition.top();
            m_PageTransition.pop();
            if ((pPage == m_ProjectPropertiesPage) || (pPage == m_ProjectProcessingPage)) {
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
 
        if (ID_ACCOUNTINFOPAGE == ulPageID)
            pPage = m_AccountInfoPage;
 
        if (ID_ACCOUNTKEYPAGE == ulPageID)
            pPage = m_AccountKeyPage;
 
        if (ID_PROJECTPROCESSINGPAGE == ulPageID)
            pPage = m_ProjectProcessingPage;
 
        if (ID_COMPLETIONPAGE == ulPageID)
            pPage = m_CompletionPage;
 
        if (ID_COMPLETIONERRORPAGE == ulPageID)
            pPage = m_CompletionErrorPage;
 
        if (ID_ERRNOTDETECTEDPAGE == ulPageID)
            pPage = m_ErrNotDetectedPage;
 
        if (ID_ERRUNAVAILABLEPAGE == ulPageID)
            pPage = m_ErrUnavailablePage;
 
        if (ID_ERRALREADYATTACHEDPAGE == ulPageID)
            pPage = m_ErrAlreadyAttachedPage;
 
        if (ID_ERRNOINTERNETCONNECTIONPAGE == ulPageID)
            pPage = m_ErrNoInternetConnectionPage;
 
        if (ID_ERRNOTFOUNDPAGE == ulPageID)
            pPage = m_ErrNotFoundPage;
 
        if (ID_ERRALREADYEXISTSPAGE == ulPageID)
            pPage = m_ErrAlreadyExistsPage;
 
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
        if (page == m_ProjectInfoPage) {
            m_ProjectInfoPage->m_ProjectUrlCtrl->SetValidator(wxDefaultValidator);
        } else if (page == m_AccountKeyPage) {
            m_AccountKeyPage->m_AccountKeyCtrl->SetValidator(wxDefaultValidator);
        } else if (page == m_AccountInfoPage) {
            m_AccountInfoPage->m_AccountEmailAddressCtrl->SetValidator(wxDefaultValidator);
            m_AccountInfoPage->m_AccountPasswordCtrl->SetValidator(wxDefaultValidator);
            m_AccountInfoPage->m_AccountConfirmPasswordCtrl->SetValidator(wxDefaultValidator);
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
    bCancelWithoutNextPage |= (page == m_ErrNotDetectedPage);
    bCancelWithoutNextPage |= (page == m_ErrUnavailablePage);
    bCancelWithoutNextPage |= (page == m_ErrAlreadyAttachedPage);
    bCancelWithoutNextPage |= (page == m_ErrNoInternetConnectionPage);
    bCancelWithoutNextPage |= (page == m_ErrAlreadyExistsPage);
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
}

/*!
 * wxEVT_WIZARD_FINISHED event handler for ID_ATTACHPROJECTWIZARD
 */

void CWizardAttachProject::OnFinished( wxWizardExEvent& event ) {
    CMainFrame* pFrame      = wxGetApp().GetFrame();

    if (GetAccountCreatedSuccessfully() && GetAttachedToProjectSuccessfully()) {
        pFrame->ExecuteBrowserLink(GetProjectURL() + wxT("account_finish.php?auth=") + GetProjectAuthenticator());
    }

    // Let the framework clean things up.
    event.Skip();
}

