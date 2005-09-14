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
#pragma implementation "WizAttachProject.h"
#endif

#include "stdwx.h"
#include "BOINCGUIApp.h"
#include "error_numbers.h"
#include "network.h"

////@begin includes
////@end includes

#include "WizAttachProject.h"

////@begin XPM images
#include "res/attachprojectwizard.xpm"
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
 * CWizAttachProject global helper macros
 */

#define PAGE_TRANSITION_NEXT(id) \
    ((CWizAttachProject*)GetParent())->PushPageTransition((wxWizardPage*)this, id)
 
#define PAGE_TRANSITION_BACK \
    ((CWizAttachProject*)GetParent())->PopPageTransition()
 
#define CHECK_DEBUG_FLAG(id) \
    ((CWizAttachProject*)GetParent())->IsDiagFlagsSet(id)
  
/*!
 * CWizAttachProject type definition
 */
 
IMPLEMENT_DYNAMIC_CLASS( CWizAttachProject, wxWizard )
 
/*!
 * CWizAttachProject event table definition
 */
 
BEGIN_EVENT_TABLE( CWizAttachProject, wxWizard )
 
    EVT_BUTTON(wxID_BACKWARD, CWizAttachProject::OnWizardBack)
    EVT_BUTTON(wxID_FORWARD, CWizAttachProject::OnWizardNext)

////@begin CWizAttachProject event table entries
    EVT_WIZARD_FINISHED( ID_ATTACHPROJECTWIZARD, CWizAttachProject::OnFinished )

////@end CWizAttachProject event table entries
 
END_EVENT_TABLE()
 
/*!
 * CWizAttachProject constructors
 */
 
CWizAttachProject::CWizAttachProject( )
{
}
 
CWizAttachProject::CWizAttachProject( wxWindow* parent, wxWindowID id, const wxPoint& pos )
{
    Create(parent, id, pos);
}
 
/*!
 * CWizAttachProject creator
 */
 
bool CWizAttachProject::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos )
{

////@begin CWizAttachProject member initialisation
    m_WelcomePage = NULL;
    m_ProjectInfoPage = NULL;
    m_ProjectPropertiesPage = NULL;
    m_AccountKeyPage = NULL;
    m_AccountInfoPage = NULL;
    m_AttachProjectPage = NULL;
    m_CompletionPage = NULL;
    m_CompletionErrorPage = NULL;
    m_ErrProjectNotDetectedPage = NULL;
    m_ErrProjectUnavailablePage = NULL;
    m_ErrProjectAlreadyAttachedPage = NULL;
    m_ErrNoInternetConnectionPage = NULL;
    m_ErrAccountNotFoundPage = NULL;
    m_ErrAccountAlreadyExistsPage = NULL;
    m_ErrProxyPage = NULL;
    m_ErrRefCountPage = NULL;
////@end CWizAttachProject member initialisation
  
    // Button pointer cache
    m_pbtnBack = NULL;
    m_pbtnNext = NULL;
 
    // Wizard support
    m_ulDiagFlags = 0;

    // Cancel Checking
    m_bCancelInProgress = false;
 
    // Global wizard status
    project_config.clear();
    account_in.clear();
    account_out.clear();
    account_created_successfully = false;
    attached_to_project_successfully = false;
    project_url = wxEmptyString;
    project_authenticator = wxEmptyString;
 
////@begin CWizAttachProject creation
    wxBitmap wizardBitmap(GetBitmapResource(wxT("res/attachprojectwizard.xpm")));
    wxWizard::Create( parent, id, _("Attach to Project"), wizardBitmap, pos );

    CreateControls();
////@end CWizAttachProject creation

    return TRUE;
}
 
/*!
 * Control creation for CWizAttachProject
 */

void CWizAttachProject::CreateControls()
{    
    wxLogTrace(wxT("Function Start/End"), wxT("CWizAttachProject::CreateControls - Function Begin"));
 
////@begin CWizAttachProject content construction
    wxWizard* itemWizard1 = this;

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
    m_AttachProjectPage = new CAttachProjectPage;
    m_AttachProjectPage->Create( itemWizard1 );

    itemWizard1->FitToPage(m_AttachProjectPage);
    m_CompletionPage = new CCompletionPage;
    m_CompletionPage->Create( itemWizard1 );

    itemWizard1->FitToPage(m_CompletionPage);
    m_CompletionErrorPage = new CCompletionErrorPage;
    m_CompletionErrorPage->Create( itemWizard1 );

    itemWizard1->FitToPage(m_CompletionErrorPage);
    m_ErrProjectNotDetectedPage = new CErrProjectNotDetectedPage;
    m_ErrProjectNotDetectedPage->Create( itemWizard1 );

    itemWizard1->FitToPage(m_ErrProjectNotDetectedPage);
    m_ErrProjectUnavailablePage = new CErrProjectUnavailablePage;
    m_ErrProjectUnavailablePage->Create( itemWizard1 );

    itemWizard1->FitToPage(m_ErrProjectUnavailablePage);
    m_ErrProjectAlreadyAttachedPage = new CErrProjectAlreadyAttachedPage;
    m_ErrProjectAlreadyAttachedPage->Create( itemWizard1 );

    itemWizard1->FitToPage(m_ErrProjectAlreadyAttachedPage);
    m_ErrNoInternetConnectionPage = new CErrNoInternetConnectionPage;
    m_ErrNoInternetConnectionPage->Create( itemWizard1 );

    itemWizard1->FitToPage(m_ErrNoInternetConnectionPage);
    m_ErrAccountNotFoundPage = new CErrAccountNotFoundPage;
    m_ErrAccountNotFoundPage->Create( itemWizard1 );

    itemWizard1->FitToPage(m_ErrAccountNotFoundPage);
    m_ErrAccountAlreadyExistsPage = new CErrAccountAlreadyExistsPage;
    m_ErrAccountAlreadyExistsPage->Create( itemWizard1 );

    itemWizard1->FitToPage(m_ErrAccountAlreadyExistsPage);
    m_ErrProxyPage = new CErrProxyPage;
    m_ErrProxyPage->Create( itemWizard1 );

    itemWizard1->FitToPage(m_ErrProxyPage);
    m_ErrRefCountPage = new CErrRefCountPage;
    m_ErrRefCountPage->Create( itemWizard1 );

    itemWizard1->FitToPage(m_ErrRefCountPage);
    wxWizardPageSimple* lastPage = NULL;
////@end CWizAttachProject content construction
 
    wxLogTrace(wxT("Function Status"), wxT("CWizAttachProject::CreateControls - Begin Page Map"));
    wxLogTrace(wxT("Function Status"), wxT("CWizAttachProject::CreateControls -     m_WelcomePage = id: '%d', location: '%p'"), ID_WELCOMEPAGE, m_WelcomePage);
    wxLogTrace(wxT("Function Status"), wxT("CWizAttachProject::CreateControls -     m_ProjectInfoPage = id: '%d', location: '%p'"), ID_PROJECTINFOPAGE, m_ProjectInfoPage);
    wxLogTrace(wxT("Function Status"), wxT("CWizAttachProject::CreateControls -     m_ProjectPropertiesPage = id: '%d', location: '%p'"), ID_PROJECTPROPERTIESPAGE, m_AccountKeyPage);
    wxLogTrace(wxT("Function Status"), wxT("CWizAttachProject::CreateControls -     m_AccountKeyPage = id: '%d', location: '%p'"), ID_ACCOUNTKEYPAGE, m_AccountKeyPage);
    wxLogTrace(wxT("Function Status"), wxT("CWizAttachProject::CreateControls -     m_AccountInfoPage = id: '%d', location: '%p'"), ID_ACCOUNTINFOPAGE, m_AccountInfoPage);
    wxLogTrace(wxT("Function Status"), wxT("CWizAttachProject::CreateControls -     m_AttachProjectPage = id: '%d', location: '%p'"), ID_ATTACHPROJECTPAGE, m_AttachProjectPage);
    wxLogTrace(wxT("Function Status"), wxT("CWizAttachProject::CreateControls -     m_CompletionPage = id: '%d', location: '%p'"), ID_COMPLETIONPAGE, m_CompletionPage);
    wxLogTrace(wxT("Function Status"), wxT("CWizAttachProject::CreateControls -     m_CompletionErrorPage = id: '%d', location: '%p'"), ID_COMPLETIONERRORPAGE, m_CompletionErrorPage);
    wxLogTrace(wxT("Function Status"), wxT("CWizAttachProject::CreateControls -     m_ErrProjectNotDetectedPage = id: '%d', location: '%p'"), ID_ERRPROJECTNOTDETECTEDPAGE, m_ErrProjectNotDetectedPage);
    wxLogTrace(wxT("Function Status"), wxT("CWizAttachProject::CreateControls -     m_ErrProjectUnavailablePage = id: '%d', location: '%p'"), ID_ERRPROJECTUNAVAILABLEPAGE, m_ErrProjectUnavailablePage);
    wxLogTrace(wxT("Function Status"), wxT("CWizAttachProject::CreateControls -     m_ErrProjectAlreadyAttachedPage = id: '%d', location: '%p'"), ID_ERRPROJECTALREADYATTACHEDPAGE, m_ErrProjectAlreadyAttachedPage);
    wxLogTrace(wxT("Function Status"), wxT("CWizAttachProject::CreateControls -     m_ErrNoInternetConnectionPage = id: '%d', location: '%p'"), ID_ERRNOINTERNETCONNECTIONPAGE, m_ErrNoInternetConnectionPage);
    wxLogTrace(wxT("Function Status"), wxT("CWizAttachProject::CreateControls -     m_ErrAccountNotFoundPage = id: '%d', location: '%p'"), ID_ERRACCOUNTNOTFOUNDPAGE, m_ErrAccountNotFoundPage);
    wxLogTrace(wxT("Function Status"), wxT("CWizAttachProject::CreateControls -     m_ErrAccountAlreadyExistsPage = id: '%d', location: '%p'"), ID_ERRACCOUNTALREADYEXISTSPAGE, m_ErrAccountAlreadyExistsPage);
    wxLogTrace(wxT("Function Status"), wxT("CWizAttachProject::CreateControls -     m_ErrProxyPage = id: '%d', location: '%p'"), ID_ERRPROXYPAGE, m_ErrProxyPage);
    wxLogTrace(wxT("Function Status"), wxT("CWizAttachProject::CreateControls - End Page Map"));
    wxLogTrace(wxT("Function Start/End"), wxT("CWizAttachProject::CreateControls - Function End"));
}
 
void CWizAttachProject::OnWizardBack( wxCommandEvent& event ) {
    if (!GetBackButton()) SetBackButton((wxButton*)event.GetEventObject());
    event.Skip();
}
 
void CWizAttachProject::OnWizardNext( wxCommandEvent& event ) {
    if (!GetNextButton()) SetNextButton((wxButton*)event.GetEventObject());
    event.Skip();
}
 
/*!
 * Runs the wizard.
 */
 
bool CWizAttachProject::Run()
{
    if (m_WelcomePage) return RunWizard(m_WelcomePage);
    return FALSE;
}
 
/*!
 * Should we show tooltips?
 */
 
bool CWizAttachProject::ShowToolTips()
{
    return TRUE;
}
 
/*!
 * Get bitmap resources
 */
 
wxBitmap CWizAttachProject::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CWizAttachProject bitmap retrieval
    if (name == wxT("res/attachprojectwizard.xpm"))
    {
        wxBitmap bitmap(attachprojectwizard_xpm);
        return bitmap;
    }
    return wxNullBitmap;
////@end CWizAttachProject bitmap retrieval
}
 
/*!
 * Get icon resources
 */
 
wxIcon CWizAttachProject::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CWizAttachProject icon retrieval
    return wxNullIcon;
////@end CWizAttachProject icon retrieval
}
 
/*!
 * Determine if the wizard page has a next page
 */

bool CWizAttachProject::HasNextPage( wxWizardPage* page )
{
    bool bNoNextPageDetected = false;

    bNoNextPageDetected |= (page == m_CompletionPage);
    bNoNextPageDetected |= (page == m_CompletionErrorPage);
    bNoNextPageDetected |= (page == m_ErrProjectNotDetectedPage);
    bNoNextPageDetected |= (page == m_ErrProjectUnavailablePage);
    bNoNextPageDetected |= (page == m_ErrProjectAlreadyAttachedPage);
    bNoNextPageDetected |= (page == m_ErrNoInternetConnectionPage);
    bNoNextPageDetected |= (page == m_ErrAccountAlreadyExistsPage);
 
    if (bNoNextPageDetected)
        return false;
    return true;
}
  
/*!
 * Determine if the wizard page has a previous page
 */
 
bool CWizAttachProject::HasPrevPage( wxWizardPage* page )
{
    if ((page == m_WelcomePage) || (page == m_CompletionErrorPage))
        return false;
    return true;
}
 
/*!
 * Set the diagnostics flags.
 */
 
void CWizAttachProject::SetDiagFlags( unsigned long ulFlags )
{
    m_ulDiagFlags = ulFlags;
}
 
/*!
 * Check the desired bitmask against our existing bitmask.
 */

bool CWizAttachProject::IsDiagFlagsSet( unsigned long ulFlags )
{
    if (ulFlags & m_ulDiagFlags) {
        return true;
    }
    return false;
}
 
/*!
 * Remove the page transition to the stack.
 */
 
wxWizardPage* CWizAttachProject::PopPageTransition() {
    wxWizardPage* pPage = NULL;
    if (GetCurrentPage()) {
        if (m_PageTransition.size() > 0) {
            pPage = m_PageTransition.top();
            m_PageTransition.pop();
            if ((pPage == m_ProjectPropertiesPage) || (pPage == m_AttachProjectPage)) {
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

wxWizardPage* CWizAttachProject::PushPageTransition( wxWizardPage* pCurrentPage, unsigned long ulPageID ) {
    if (GetCurrentPage()) {
        wxWizardPage* pPage = NULL;

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
 
        if (ID_ATTACHPROJECTPAGE == ulPageID)
            pPage = m_AttachProjectPage;
 
        if (ID_COMPLETIONPAGE == ulPageID)
            pPage = m_CompletionPage;
 
        if (ID_COMPLETIONERRORPAGE == ulPageID)
            pPage = m_CompletionErrorPage;
 
        if (ID_ERRPROJECTNOTDETECTEDPAGE == ulPageID)
            pPage = m_ErrProjectNotDetectedPage;
 
        if (ID_ERRPROJECTUNAVAILABLEPAGE == ulPageID)
            pPage = m_ErrProjectUnavailablePage;
 
        if (ID_ERRPROJECTALREADYATTACHEDPAGE == ulPageID)
            pPage = m_ErrProjectAlreadyAttachedPage;
 
        if (ID_ERRNOINTERNETCONNECTIONPAGE == ulPageID)
            pPage = m_ErrNoInternetConnectionPage;
 
        if (ID_ERRACCOUNTNOTFOUNDPAGE == ulPageID)
            pPage = m_ErrAccountNotFoundPage;
 
        if (ID_ERRACCOUNTALREADYEXISTSPAGE == ulPageID)
            pPage = m_ErrAccountAlreadyExistsPage;
 
        if (ID_ERRPROXYPAGE == ulPageID)
            pPage = m_ErrProxyPage;
 
        if (pPage) {
            if ((pCurrentPage == m_WelcomePage) && (m_PageTransition.size() == 0)) {
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
  
void CWizAttachProject::ProcessCancelEvent( wxWizardEvent& event ) {

    bool bCancelWithoutNextPage = false;
    wxWizardPage* page = GetCurrentPage();

    int iRetVal = ::wxMessageBox(
        _("Do you really want to cancel?"), 
        _("Question"),
        wxICON_QUESTION | wxYES_NO,
        this
    );

    // Reenable the next and back buttons if they have been disabled
    if (GetNextButton()) {
        GetNextButton()->Enable();
    }
    if (GetBackButton()) {
        GetBackButton()->Enable();
    }

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
    bCancelWithoutNextPage |= (page == m_WelcomePage);
    bCancelWithoutNextPage |= (page == m_ErrProjectNotDetectedPage);
    bCancelWithoutNextPage |= (page == m_ErrProjectUnavailablePage);
    bCancelWithoutNextPage |= (page == m_ErrProjectAlreadyAttachedPage);
    bCancelWithoutNextPage |= (page == m_ErrNoInternetConnectionPage);
    bCancelWithoutNextPage |= (page == m_ErrAccountAlreadyExistsPage);
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
 
void CWizAttachProject::SimulateNextButton() {
    if (!GetNextButton()) return;
    wxCommandEvent event(wxEVT_COMMAND_BUTTON_CLICKED, GetNextButton()->GetId());
    event.SetEventObject(GetNextButton());
    AddPendingEvent(event);
}

void CWizAttachProject::EnableNextButton() {
    if (!GetNextButton()) return;
    GetNextButton()->Enable();
}

void CWizAttachProject::DisableNextButton() {
    if (!GetNextButton()) return;
    GetNextButton()->Disable();
}

void CWizAttachProject::SimulateBackButton() {
    if (!GetBackButton()) return;
    wxCommandEvent event(wxEVT_COMMAND_BUTTON_CLICKED, GetBackButton()->GetId());
    event.SetEventObject(GetNextButton());
    AddPendingEvent(event);
}
 
void CWizAttachProject::EnableBackButton() {
    if (!GetBackButton()) return;
    GetBackButton()->Enable();
}

void CWizAttachProject::DisableBackButton() {
    if (!GetBackButton()) return;
    GetBackButton()->Disable();
}

/*!
 * wxEVT_WIZARD_FINISHED event handler for ID_ATTACHPROJECTWIZARD
 */

void CWizAttachProject::OnFinished( wxWizardEvent& event ) {
    CMainFrame* pFrame      = wxGetApp().GetFrame();

    if (GetAccountCreatedSuccessfully() && GetAttachedToProjectSuccessfully()) {
        pFrame->ExecuteBrowserLink(GetProjectURL() + wxT("account_finish.php?auth=") + GetProjectAuthenticator());
    }

    // Let the framework clean things up.
    event.Skip();
}

/*!
 * CWelcomePage type definition
 */
 
IMPLEMENT_DYNAMIC_CLASS( CWelcomePage, wxWizardPage )
 
/*!
 * CWelcomePage event table definition
 */
 
BEGIN_EVENT_TABLE( CWelcomePage, wxWizardPage )
 
////@begin CWelcomePage event table entries
    EVT_WIZARD_PAGE_CHANGED( -1, CWelcomePage::OnPageChanged )
    EVT_WIZARD_PAGE_CHANGING( -1, CWelcomePage::OnPageChanging )
    EVT_WIZARD_CANCEL( -1, CWelcomePage::OnCancel )

////@end CWelcomePage event table entries
 
END_EVENT_TABLE()
 
/*!
 * CWelcomePage constructors
 */
 
CWelcomePage::CWelcomePage( )
{
}
 
CWelcomePage::CWelcomePage( wxWizard* parent )
{
    Create( parent );
}
 
/*!
 * WizardPage creator
 */
 
bool CWelcomePage::Create( wxWizard* parent )
{

////@begin CWelcomePage member initialisation
#if defined(__WXDEBUG__)
    m_ErrProjectPropertiesCtrl = NULL;
#endif
#if defined(__WXDEBUG__)
    m_ErrProjectCommCtrl = NULL;
#endif
#if defined(__WXDEBUG__)
    m_ErrProjectPropertiesURLCtrl = NULL;
#endif
#if defined(__WXDEBUG__)
    m_ErrAccountCreationDisabledCtrl = NULL;
#endif
#if defined(__WXDEBUG__)
    m_ErrClientAccountCreationDisabledCtrl = NULL;
#endif
#if defined(__WXDEBUG__)
    m_ErrAccountAlreadyExistsCtrl = NULL;
#endif
#if defined(__WXDEBUG__)
    m_ErrProjectAlreadyAttachedCtrl = NULL;
#endif
#if defined(__WXDEBUG__)
    m_ErrProjectAttachFailureCtrl = NULL;
#endif
#if defined(__WXDEBUG__)
    m_ErrGoogleCommCtrl = NULL;
#endif
#if defined(__WXDEBUG__)
    m_ErrYahooCommCtrl = NULL;
#endif
#if defined(__WXDEBUG__)
    m_ErrNetDetectionCtrl = NULL;
#endif
////@end CWelcomePage member initialisation
 
////@begin CWelcomePage creation
    wxBitmap wizardBitmap(GetBitmapResource(wxT("res/attachprojectwizard.xpm")));
    wxWizardPage::Create( parent, wizardBitmap );

    CreateControls();
    GetSizer()->Fit(this);
////@end CWelcomePage creation

    return TRUE;
}
 
/*!
 * Control creation for WizardPage
 */
 
void CWelcomePage::CreateControls()
{    
////@begin CWelcomePage content construction
    CWelcomePage* itemWizardPage2 = this;

    wxBoxSizer* itemBoxSizer3 = new wxBoxSizer(wxVERTICAL);
    itemWizardPage2->SetSizer(itemBoxSizer3);

    wxStaticText* itemStaticText4 = new wxStaticText;
    itemStaticText4->Create( itemWizardPage2, wxID_STATIC, _("Attach to project"), wxDefaultPosition, wxDefaultSize, 0 );
    itemStaticText4->SetFont(wxFont(12, wxSWISS, wxNORMAL, wxBOLD, FALSE, _T("Verdana")));
    itemBoxSizer3->Add(itemStaticText4, 0, wxALIGN_LEFT|wxALL, 5);

    wxStaticText* itemStaticText5 = new wxStaticText;
    itemStaticText5->Create( itemWizardPage2, wxID_STATIC, _("We'll now guide you through the process of attaching to a project."), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer3->Add(itemStaticText5, 0, wxALIGN_LEFT|wxALL, 5);

    itemBoxSizer3->Add(5, 5, 0, wxALIGN_LEFT|wxALL, 5);

#if defined(__WXDEBUG__)
    wxStaticBox* itemStaticBoxSizer7Static = new wxStaticBox(itemWizardPage2, wxID_ANY, _("Debug Flags"));
    wxStaticBoxSizer* itemStaticBoxSizer7 = new wxStaticBoxSizer(itemStaticBoxSizer7Static, wxVERTICAL);
    itemBoxSizer3->Add(itemStaticBoxSizer7, 0, wxGROW|wxALL, 5);

    wxFlexGridSizer* itemFlexGridSizer8 = new wxFlexGridSizer(-1, 2, 0, 0);
    itemFlexGridSizer8->AddGrowableCol(0);
    itemFlexGridSizer8->AddGrowableCol(1);
    itemStaticBoxSizer7->Add(itemFlexGridSizer8, 0, wxGROW|wxALL, 5);

    m_ErrProjectPropertiesCtrl = new wxCheckBox;
    m_ErrProjectPropertiesCtrl->Create( itemWizardPage2, ID_ERRPROJECTPROPERTIES, _("Project Properties Failure"), wxDefaultPosition, wxDefaultSize, wxCHK_2STATE );
    m_ErrProjectPropertiesCtrl->SetValue(FALSE);
    itemFlexGridSizer8->Add(m_ErrProjectPropertiesCtrl, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 0);

    m_ErrProjectCommCtrl = new wxCheckBox;
    m_ErrProjectCommCtrl->Create( itemWizardPage2, ID_ERRPROJECTCOMM, _("Project Comm Failure"), wxDefaultPosition, wxDefaultSize, wxCHK_2STATE );
    m_ErrProjectCommCtrl->SetValue(FALSE);
    itemFlexGridSizer8->Add(m_ErrProjectCommCtrl, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 0);

    m_ErrProjectPropertiesURLCtrl = new wxCheckBox;
    m_ErrProjectPropertiesURLCtrl->Create( itemWizardPage2, ID_ERRPROJECTPROPERTIESURL, _("Project Properties URL Failure"), wxDefaultPosition, wxDefaultSize, wxCHK_2STATE );
    m_ErrProjectPropertiesURLCtrl->SetValue(FALSE);
    itemFlexGridSizer8->Add(m_ErrProjectPropertiesURLCtrl, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 0);

    m_ErrAccountCreationDisabledCtrl = new wxCheckBox;
    m_ErrAccountCreationDisabledCtrl->Create( itemWizardPage2, ID_ERRACCOUNTCREATIONDISABLED, _("Account Creation Disabled"), wxDefaultPosition, wxDefaultSize, wxCHK_2STATE );
    m_ErrAccountCreationDisabledCtrl->SetValue(FALSE);
    itemFlexGridSizer8->Add(m_ErrAccountCreationDisabledCtrl, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 0);

    m_ErrClientAccountCreationDisabledCtrl = new wxCheckBox;
    m_ErrClientAccountCreationDisabledCtrl->Create( itemWizardPage2, ID_ERRCLIENTACCOUNTCREATIONDISABLED, _("Client Account Creation Disabled"), wxDefaultPosition, wxDefaultSize, wxCHK_2STATE );
    m_ErrClientAccountCreationDisabledCtrl->SetValue(FALSE);
    itemFlexGridSizer8->Add(m_ErrClientAccountCreationDisabledCtrl, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 0);

    m_ErrAccountAlreadyExistsCtrl = new wxCheckBox;
    m_ErrAccountAlreadyExistsCtrl->Create( itemWizardPage2, ID_ERRACCOUNTALREADYEXISTS, _("Account Already Exists"), wxDefaultPosition, wxDefaultSize, wxCHK_2STATE );
    m_ErrAccountAlreadyExistsCtrl->SetValue(FALSE);
    itemFlexGridSizer8->Add(m_ErrAccountAlreadyExistsCtrl, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 0);

    m_ErrProjectAlreadyAttachedCtrl = new wxCheckBox;
    m_ErrProjectAlreadyAttachedCtrl->Create( itemWizardPage2, ID_ERRPROJECTALREADYATTACHED, _("Project Already Attached"), wxDefaultPosition, wxDefaultSize, wxCHK_2STATE );
    m_ErrProjectAlreadyAttachedCtrl->SetValue(FALSE);
    itemFlexGridSizer8->Add(m_ErrProjectAlreadyAttachedCtrl, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 0);

    m_ErrProjectAttachFailureCtrl = new wxCheckBox;
    m_ErrProjectAttachFailureCtrl->Create( itemWizardPage2, ID_ERRPROJECTATTACHFAILURE, _("Project Attach Failure"), wxDefaultPosition, wxDefaultSize, wxCHK_2STATE );
    m_ErrProjectAttachFailureCtrl->SetValue(FALSE);
    itemFlexGridSizer8->Add(m_ErrProjectAttachFailureCtrl, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 0);

    m_ErrGoogleCommCtrl = new wxCheckBox;
    m_ErrGoogleCommCtrl->Create( itemWizardPage2, ID_ERRGOOGLECOMM, _("Google Comm Failure"), wxDefaultPosition, wxDefaultSize, wxCHK_2STATE );
    m_ErrGoogleCommCtrl->SetValue(FALSE);
    itemFlexGridSizer8->Add(m_ErrGoogleCommCtrl, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 0);

    itemFlexGridSizer8->Add(5, 5, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_ErrYahooCommCtrl = new wxCheckBox;
    m_ErrYahooCommCtrl->Create( itemWizardPage2, ID_ERRYAHOOCOMM, _("Yahoo Comm Failure"), wxDefaultPosition, wxDefaultSize, wxCHK_2STATE );
    m_ErrYahooCommCtrl->SetValue(FALSE);
    itemFlexGridSizer8->Add(m_ErrYahooCommCtrl, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 0);

    itemFlexGridSizer8->Add(5, 5, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 0);

    m_ErrNetDetectionCtrl = new wxCheckBox;
    m_ErrNetDetectionCtrl->Create( itemWizardPage2, ID_ERRNETDETECTION, _("Net Detection Failure"), wxDefaultPosition, wxDefaultSize, wxCHK_2STATE );
    m_ErrNetDetectionCtrl->SetValue(FALSE);
    itemFlexGridSizer8->Add(m_ErrNetDetectionCtrl, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 0);

#endif

    wxStaticText* itemStaticText22 = new wxStaticText;
    itemStaticText22->Create( itemWizardPage2, wxID_STATIC, _("To continue, click Next."), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer3->Add(itemStaticText22, 0, wxALIGN_LEFT|wxALL, 5);

////@end CWelcomePage content construction

}
 
/*!
 * Gets the previous page.
 */
 
wxWizardPage* CWelcomePage::GetPrev() const
{
    return NULL;
}
 
/*!
 * Gets the next page.
 */
 
wxWizardPage* CWelcomePage::GetNext() const
{
    if (((CWizAttachProject*)GetParent())->IsCancelInProgress()) {
        // Cancel Event Detected
        return PAGE_TRANSITION_NEXT(ID_COMPLETIONERRORPAGE);
    } else {
        return PAGE_TRANSITION_NEXT(ID_PROJECTINFOPAGE);
    }
    return NULL;
}
 
/*!
 * Should we show tooltips?
 */
 
bool CWelcomePage::ShowToolTips()
{
    return TRUE;
}
 
/*!
 * Get bitmap resources
 */
 
wxBitmap CWelcomePage::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CWelcomePage bitmap retrieval
    if (name == wxT("res/attachprojectwizard.xpm"))
    {
        wxBitmap bitmap(attachprojectwizard_xpm);
        return bitmap;
    }
    return wxNullBitmap;
////@end CWelcomePage bitmap retrieval
}
 
/*!
 * Get icon resources
 */
 
wxIcon CWelcomePage::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CWelcomePage icon retrieval
    return wxNullIcon;
////@end CWelcomePage icon retrieval
}
   
/*!
 * wxEVT_WIZARD_PAGE_CHANGED event handler for ID_WELCOMEPAGE
 */
 
void CWelcomePage::OnPageChanged( wxWizardEvent& event ) {
    event.Skip();
}
  
/*!
 * wxEVT_WIZARD_PAGE_CHANGING event handler for ID_WELCOMEPAGE
 */

void CWelcomePage::OnPageChanging( wxWizardEvent& event )
{
    if (event.GetDirection() == false) return;
 
    unsigned long ulFlags = 0;
 
#if defined(__WXDEBUG__)
    if (m_ErrProjectPropertiesCtrl->GetValue()) 
        ulFlags |= WIZDEBUG_ERRPROJECTPROPERTIES;
#endif
#if defined(__WXDEBUG__)
    if (m_ErrProjectPropertiesURLCtrl->GetValue()) 
        ulFlags |= WIZDEBUG_ERRPROJECTPROPERTIESURL;
#endif
#if defined(__WXDEBUG__)
    if (m_ErrProjectCommCtrl->GetValue()) 
        ulFlags |= WIZDEBUG_ERRPROJECTCOMM;
#endif
#if defined(__WXDEBUG__)
    if (m_ErrGoogleCommCtrl->GetValue()) 
        ulFlags |= WIZDEBUG_ERRGOOGLECOMM;
#endif
#if defined(__WXDEBUG__)
    if (m_ErrYahooCommCtrl->GetValue()) 
        ulFlags |= WIZDEBUG_ERRYAHOOCOMM;
#endif
#if defined(__WXDEBUG__)
    if (m_ErrAccountAlreadyExistsCtrl->GetValue()) 
        ulFlags |= WIZDEBUG_ERRACCOUNTALREADYEXISTS;
#endif
#if defined(__WXDEBUG__)
    if (m_ErrAccountCreationDisabledCtrl->GetValue()) 
        ulFlags |= WIZDEBUG_ERRACCOUNTCREATIONDISABLED;
#endif
#if defined(__WXDEBUG__)
    if (m_ErrClientAccountCreationDisabledCtrl->GetValue()) 
        ulFlags |= WIZDEBUG_ERRCLIENTACCOUNTCREATIONDISABLED;
#endif
#if defined(__WXDEBUG__)
    if (m_ErrProjectAttachFailureCtrl->GetValue()) 
        ulFlags |= WIZDEBUG_ERRPROJECTATTACH;
#endif
#if defined(__WXDEBUG__)
    if (m_ErrProjectAlreadyAttachedCtrl->GetValue()) 
        ulFlags |= WIZDEBUG_ERRPROJECTALREADYATTACHED;
#endif
#if defined(__WXDEBUG__)
    if (m_ErrNetDetectionCtrl->GetValue()) 
        ulFlags |= WIZDEBUG_ERRNETDETECTION;
#endif
 
    ((CWizAttachProject*)GetParent())->SetDiagFlags( ulFlags );
}
  
/*!
 * wxEVT_WIZARD_CANCEL event handler for ID_WELCOMEPAGE
 */
 
void CWelcomePage::OnCancel( wxWizardEvent& event ) {
    ((CWizAttachProject*)GetParent())->ProcessCancelEvent(event);
}
 
/*!
 * CProjectInfoPage type definition
 */
 
IMPLEMENT_DYNAMIC_CLASS( CProjectInfoPage, wxWizardPage )
 
/*!
 * CProjectInfoPage event table definition
 */
 
BEGIN_EVENT_TABLE( CProjectInfoPage, wxWizardPage )
 
////@begin CProjectInfoPage event table entries
    EVT_WIZARD_PAGE_CHANGED( -1, CProjectInfoPage::OnPageChanged )
    EVT_WIZARD_PAGE_CHANGING( -1, CProjectInfoPage::OnPageChanging )
    EVT_WIZARD_CANCEL( -1, CProjectInfoPage::OnCancel )

////@end CProjectInfoPage event table entries
 
END_EVENT_TABLE()
 
/*!
 * CProjectInfoPage constructors
 */
 
CProjectInfoPage::CProjectInfoPage( )
{
}
 
CProjectInfoPage::CProjectInfoPage( wxWizard* parent )
{
    Create( parent );
}
 
/*!
 * WizardPage creator
 */
 
bool CProjectInfoPage::Create( wxWizard* parent )
{
////@begin CProjectInfoPage member initialisation
    m_ProjectUrlStaticCtrl = NULL;
    m_ProjectUrlCtrl = NULL;
////@end CProjectInfoPage member initialisation
 
////@begin CProjectInfoPage creation
    wxBitmap wizardBitmap(wxNullBitmap);
    wxWizardPage::Create( parent, wizardBitmap );

    CreateControls();
    GetSizer()->Fit(this);
////@end CProjectInfoPage creation

    return TRUE;
}
 
/*!
 * Control creation for WizardPage
 */
 
void CProjectInfoPage::CreateControls()
{    
////@begin CProjectInfoPage content construction
    CProjectInfoPage* itemWizardPage23 = this;

    wxBoxSizer* itemBoxSizer24 = new wxBoxSizer(wxVERTICAL);
    itemWizardPage23->SetSizer(itemBoxSizer24);

    wxStaticText* itemStaticText25 = new wxStaticText;
    itemStaticText25->Create( itemWizardPage23, wxID_STATIC, _("Project URL"), wxDefaultPosition, wxDefaultSize, 0 );
    itemStaticText25->SetFont(wxFont(10, wxSWISS, wxNORMAL, wxBOLD, FALSE, _T("Verdana")));
    itemBoxSizer24->Add(itemStaticText25, 0, wxALIGN_LEFT|wxALL, 5);

    wxStaticText* itemStaticText26 = new wxStaticText;
    itemStaticText26->Create( itemWizardPage23, wxID_STATIC, _("Enter the URL of the project's web site."), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer24->Add(itemStaticText26, 0, wxALIGN_LEFT|wxALL, 5);

    itemBoxSizer24->Add(5, 5, 0, wxALIGN_LEFT|wxALL, 5);

    wxStaticText* itemStaticText28 = new wxStaticText;
    itemStaticText28->Create( itemWizardPage23, wxID_STATIC, _("You can copy and paste the URL from your browser’s address bar."), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer24->Add(itemStaticText28, 0, wxALIGN_LEFT|wxALL, 5);

    itemBoxSizer24->Add(5, 5, 0, wxALIGN_LEFT|wxALL, 5);

    wxFlexGridSizer* itemFlexGridSizer30 = new wxFlexGridSizer(1, 2, 0, 0);
    itemFlexGridSizer30->AddGrowableCol(1);
    itemBoxSizer24->Add(itemFlexGridSizer30, 0, wxALIGN_LEFT|wxALL, 5);

    m_ProjectUrlStaticCtrl = new wxStaticText;
    m_ProjectUrlStaticCtrl->Create( itemWizardPage23, ID_PROJECTURLSTATICCTRL, _("Project URL:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer30->Add(m_ProjectUrlStaticCtrl, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_ProjectUrlCtrl = new wxTextCtrl;
    m_ProjectUrlCtrl->Create( itemWizardPage23, ID_PROJECTURLCTRL, _T(""), wxDefaultPosition, wxSize(250, -1), 0 );
    itemFlexGridSizer30->Add(m_ProjectUrlCtrl, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    itemBoxSizer24->Add(5, 5, 0, wxALIGN_LEFT|wxALL, 5);

    wxStaticText* itemStaticText34 = new wxStaticText;
    itemStaticText34->Create( itemWizardPage23, wxID_STATIC, _("For a list of BOINC-based projects go to:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer24->Add(itemStaticText34, 0, wxALIGN_LEFT|wxALL, 5);

    wxHyperLink* itemHyperLink35 = new wxHyperLink;
    itemHyperLink35->Create( itemWizardPage23, ID_PROJECRINFOBOINCLINK, wxT("http://boinc.berkeley.edu/"), wxDefaultPosition, wxDefaultSize, wxNO_BORDER );
    itemBoxSizer24->Add(itemHyperLink35, 0, wxALIGN_LEFT|wxALL, 5);

    // Set validators
    m_ProjectUrlCtrl->SetValidator( CValidateURL( & m_strProjectURL) );
////@end CProjectInfoPage content construction
}
 
/*!
 * Gets the previous page.
 */
wxWizardPage* CProjectInfoPage::GetPrev() const
{
    return PAGE_TRANSITION_BACK;
}
 
/*!
 * Gets the next page.
 */
 
wxWizardPage* CProjectInfoPage::GetNext() const
{
    if (((CWizAttachProject*)GetParent())->IsCancelInProgress()) {
        // Cancel Event Detected
        return PAGE_TRANSITION_NEXT(ID_COMPLETIONERRORPAGE);
    } else {
        return PAGE_TRANSITION_NEXT(ID_PROJECTPROPERTIESPAGE);
    }
    return NULL;
}
 
/*!
 * Should we show tooltips?
 */
 
bool CProjectInfoPage::ShowToolTips()
{
    return TRUE;
}
 
/*!
 * Get bitmap resources
 */
 
wxBitmap CProjectInfoPage::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CProjectInfoPage bitmap retrieval
    return wxNullBitmap;
////@end CProjectInfoPage bitmap retrieval
}
 
/*!
 * Get icon resources
 */
 
wxIcon CProjectInfoPage::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CProjectInfoPage icon retrieval
    return wxNullIcon;
////@end CProjectInfoPage icon retrieval
}
  
/*!
 * wxEVT_WIZARD_PAGE_CHANGED event handler for ID_PROJECTINFOPAGE
 */

void CProjectInfoPage::OnPageChanged( wxWizardEvent& event ) {
    event.Skip();
}
  
/*!
 * wxEVT_WIZARD_PAGE_CHANGING event handler for ID_PROJECTINFOPAGE
 */

void CProjectInfoPage::OnPageChanging( wxWizardEvent& event ) {
    event.Skip();
}
  
/*!
 * wxEVT_WIZARD_CANCEL event handler for ID_PROJECTINFOPAGE
 */

void CProjectInfoPage::OnCancel( wxWizardEvent& event ) {
    ((CWizAttachProject*)GetParent())->ProcessCancelEvent(event);
}
 
/*!
 * CProjectPropertiesPage custom event definition
 */
 
DEFINE_EVENT_TYPE(wxEVT_PROJECTPROPERTIES_STATECHANGE)
  
/*!
 * CProjectPropertiesPage type definition
 */
 
IMPLEMENT_DYNAMIC_CLASS( CProjectPropertiesPage, wxWizardPage )
 
/*!
 * CProjectPropertiesPage event table definition
 */
 
BEGIN_EVENT_TABLE( CProjectPropertiesPage, wxWizardPage )
 
    EVT_PROJECTPROPERTIES_STATECHANGE( CProjectPropertiesPage::OnStateChange )
 
////@begin CProjectPropertiesPage event table entries
    EVT_WIZARD_PAGE_CHANGED( -1, CProjectPropertiesPage::OnPageChanged )
    EVT_WIZARD_CANCEL( -1, CProjectPropertiesPage::OnCancel )

////@end CProjectPropertiesPage event table entries
 
END_EVENT_TABLE()
 
/*!
 * CProjectPropertiesPage constructors
 */
 
CProjectPropertiesPage::CProjectPropertiesPage( )
{
}
 
CProjectPropertiesPage::CProjectPropertiesPage( wxWizard* parent )
{
    Create( parent );
}
 
/*!
 * WizardPage creator
 */
 
bool CProjectPropertiesPage::Create( wxWizard* parent )
{
////@begin CProjectPropertiesPage member initialisation
    m_ProjectPropertiesProgress = NULL;
////@end CProjectPropertiesPage member initialisation
 
    m_bProjectPropertiesSucceeded = false;
    m_bProjectPropertiesURLFailure = false;
    m_bProjectAccountCreationDisabled = false;
    m_bProjectClientAccountCreationDisabled = false;
    m_bCommunicateYahooSucceeded = false;
    m_bCommunicateGoogleSucceeded = false;
    m_bDeterminingConnectionStatusSucceeded = false;
    m_iBitmapIndex = 0;
    m_iCurrentState = PROJPROP_INIT;
 
////@begin CProjectPropertiesPage creation
    wxBitmap wizardBitmap(wxNullBitmap);
    wxWizardPage::Create( parent, wizardBitmap );

    CreateControls();
    GetSizer()->Fit(this);
////@end CProjectPropertiesPage creation

    return TRUE;
}
 
/*!
 * Control creation for WizardPage
 */
 
void CProjectPropertiesPage::CreateControls()
{    
////@begin CProjectPropertiesPage content construction
    CProjectPropertiesPage* itemWizardPage36 = this;

    wxBoxSizer* itemBoxSizer37 = new wxBoxSizer(wxVERTICAL);
    itemWizardPage36->SetSizer(itemBoxSizer37);

    wxStaticText* itemStaticText38 = new wxStaticText;
    itemStaticText38->Create( itemWizardPage36, wxID_STATIC, _("Communicating with project \nPlease wait..."), wxDefaultPosition, wxDefaultSize, 0 );
    itemStaticText38->SetFont(wxFont(10, wxSWISS, wxNORMAL, wxBOLD, FALSE, _T("Verdana")));
    itemBoxSizer37->Add(itemStaticText38, 0, wxALIGN_LEFT|wxALL, 5);

    itemBoxSizer37->Add(5, 80, 0, wxALIGN_LEFT|wxALL, 5);

    wxFlexGridSizer* itemFlexGridSizer40 = new wxFlexGridSizer(1, 3, 0, 0);
    itemFlexGridSizer40->AddGrowableRow(0);
    itemFlexGridSizer40->AddGrowableCol(0);
    itemFlexGridSizer40->AddGrowableCol(1);
    itemFlexGridSizer40->AddGrowableCol(2);
    itemBoxSizer37->Add(itemFlexGridSizer40, 0, wxGROW|wxALL, 5);

    itemFlexGridSizer40->Add(5, 5, 0, wxGROW|wxGROW|wxALL, 5);

    wxBitmap m_ProjectPropertiesProgressBitmap(itemWizardPage36->GetBitmapResource(wxT("res/wizprogress01.xpm")));
    m_ProjectPropertiesProgress = new wxStaticBitmap;
    m_ProjectPropertiesProgress->Create( itemWizardPage36, ID_PROJECTPROPERTIESPROGRESS, m_ProjectPropertiesProgressBitmap, wxDefaultPosition, wxSize(184, 48), 0 );
    itemFlexGridSizer40->Add(m_ProjectPropertiesProgress, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    itemFlexGridSizer40->Add(5, 5, 0, wxGROW|wxGROW|wxALL, 5);

////@end CProjectPropertiesPage content construction
}
 
/*!
 * Gets the previous page.
 */
 
wxWizardPage* CProjectPropertiesPage::GetPrev() const
{
    return PAGE_TRANSITION_BACK;
}
 
/*!
 * Gets the next page.
 */
 
wxWizardPage* CProjectPropertiesPage::GetNext() const
{
    if (((CWizAttachProject*)GetParent())->IsCancelInProgress()) {
        // Cancel Event Detected
        return PAGE_TRANSITION_NEXT(ID_COMPLETIONERRORPAGE);
    } else if (GetProjectPropertiesSucceeded() && GetProjectAlreadyAttached()) {
        // Already attach to the project
        return PAGE_TRANSITION_NEXT(ID_ERRPROJECTALREADYATTACHEDPAGE);
    } else if (GetProjectPropertiesSucceeded() && GetProjectClientAccountCreationDisabled()) {
        // Client Account Creation Disabled - Use Legacy Authentication Scheme
        return PAGE_TRANSITION_NEXT(ID_ACCOUNTKEYPAGE);
    } else if (GetProjectPropertiesSucceeded()) {
        // We were successful in retrieving the project properties
        return PAGE_TRANSITION_NEXT(ID_ACCOUNTINFOPAGE);
    } else if (!GetProjectPropertiesSucceeded() && GetProjectPropertiesURLFailure()) {
        // Not a BOINC based project
        return PAGE_TRANSITION_NEXT(ID_ERRPROJECTNOTDETECTEDPAGE);
    } else if ((!GetCommunicateYahooSucceeded() && !GetCommunicateGoogleSucceeded()) && GetDeterminingConnectionStatusSucceeded()) {
        // Possible proxy problem
        return PAGE_TRANSITION_NEXT(ID_ERRPROXYPAGE);
    } else if ((!GetCommunicateYahooSucceeded() && !GetCommunicateGoogleSucceeded()) && !GetDeterminingConnectionStatusSucceeded()) {
        // No Internet Connection
        return PAGE_TRANSITION_NEXT(ID_ERRNOINTERNETCONNECTIONPAGE);
    } else {
        // The project much be down for maintenance
        return PAGE_TRANSITION_NEXT(ID_ERRPROJECTUNAVAILABLEPAGE);
    }
    return NULL;
}
 
/*!
 * Should we show tooltips?
 */
 
bool CProjectPropertiesPage::ShowToolTips()
{
    return TRUE;
}
 
void CProjectPropertiesPage::StartProgress(wxStaticBitmap* pBitmap) {
    m_iBitmapIndex = 1;
    pBitmap->SetBitmap(GetBitmapResource(wxT("res/wizprogress01.xpm")));
}
 
void CProjectPropertiesPage::IncrementProgress(wxStaticBitmap* pBitmap) {
    m_iBitmapIndex += 1;
    if (12 < m_iBitmapIndex) m_iBitmapIndex = 1;
 
    wxString str;
    str.Printf(wxT("res/wizprogress%02d.xpm"), m_iBitmapIndex);
 
    pBitmap->SetBitmap(GetBitmapResource(str));
    Update();
}
 
void CProjectPropertiesPage::FinishProgress(wxStaticBitmap* pBitmap) {
    m_iBitmapIndex = 12;
    pBitmap->SetBitmap(GetBitmapResource(wxT("res/wizprogress12.xpm")));
}
 
/*!
 * Get bitmap resources
 */
 
wxBitmap CProjectPropertiesPage::GetBitmapResource( const wxString& name )
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
 
wxIcon CProjectPropertiesPage::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CProjectPropertiesPage icon retrieval
    return wxNullIcon;
////@end CProjectPropertiesPage icon retrieval
}
 
/*!
 * wxEVT_WIZARD_PAGE_CHANGED event handler for ID_PROJECTPROPERTIESPAGE
 */
 
void CProjectPropertiesPage::OnPageChanged( wxWizardEvent& event )
{
    if (event.GetDirection() == false) return;
 
    SetProjectPropertiesSucceeded(false);
    SetProjectPropertiesURLFailure(false);
    SetProjectAccountCreationDisabled(false);
    SetProjectClientAccountCreationDisabled(false);
    SetCommunicateYahooSucceeded(false);
    SetCommunicateGoogleSucceeded(false);
    SetDeterminingConnectionStatusSucceeded(false);
    SetNextState(PROJPROP_INIT);

    CProjectPropertiesPageEvent TransitionEvent(wxEVT_PROJECTPROPERTIES_STATECHANGE, this);
    AddPendingEvent(TransitionEvent);
}
  
/*!
 * wxEVT_WIZARD_CANCEL event handler for ID_PROJECTPROPERTIESPAGE
 */

void CProjectPropertiesPage::OnCancel( wxWizardEvent& event ) {
    ((CWizAttachProject*)GetParent())->ProcessCancelEvent(event);
}
 
/*!
 * wxEVT_PROJECTPROPERTIES_STATECHANGE event handler for ID_PROJECTPROPERTIESPAGE
 */
 
void CProjectPropertiesPage::OnStateChange( CProjectPropertiesPageEvent& event )
{
    CMainDocument* pDoc      = wxGetApp().GetDocument();
    PROJECT_CONFIG* pc       = &((CWizAttachProject*)GetParent())->project_config;
    wxDateTime dtStartExecutionTime;
    wxDateTime dtCurrentExecutionTime;
    wxTimeSpan tsExecutionTime;
    bool bPostNewEvent = true;
    bool bSuccessfulCondition = false;
    int  iReturnValue = 0;
 
    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));
 
    switch(GetCurrentState()) {
        case PROJPROP_INIT:
            ((CWizAttachProject*)GetParent())->DisableNextButton();
            ((CWizAttachProject*)GetParent())->DisableBackButton();
            StartProgress(m_ProjectPropertiesProgress);
            SetNextState(PROJPROP_RETRPROJECTPROPERTIES_BEGIN);
            break;
        case PROJPROP_RETRPROJECTPROPERTIES_BEGIN:
            SetNextState(PROJPROP_RETRPROJECTPROPERTIES_EXECUTE);
            break;
        case PROJPROP_RETRPROJECTPROPERTIES_EXECUTE:
            // Attempt to retrieve the project's account creation policies
            pDoc->rpc.get_project_config(
                ((CWizAttachProject*)GetParent())->m_ProjectInfoPage->GetProjectURL().c_str()
            );
 
            // Wait until we are done processing the request.
            dtStartExecutionTime = wxDateTime::Now();
            dtCurrentExecutionTime = wxDateTime::Now();
            tsExecutionTime = dtCurrentExecutionTime - dtStartExecutionTime;
            iReturnValue = ERR_IN_PROGRESS;
            while (ERR_IN_PROGRESS == iReturnValue &&
                   tsExecutionTime.GetSeconds() <= 60 &&
                   !((CWizAttachProject*)GetParent())->IsCancelInProgress()
                  )
            {
                dtCurrentExecutionTime = wxDateTime::Now();
                tsExecutionTime = dtCurrentExecutionTime - dtStartExecutionTime;
                iReturnValue = pDoc->rpc.get_project_config_poll(*pc);
                IncrementProgress(m_ProjectPropertiesProgress);

                ::wxMilliSleep(500);
                ::wxSafeYield(GetParent());
            }
 
            // We either successfully retrieved the project's account creation 
            //   policies or we were able to talk to the web server and found out
            //   they do not support account creation through the wizard.  In either
            //   case we should claim success and set the correct flags to show the
            //   correct 'next' page.
            bSuccessfulCondition = (BOINC_SUCCESS == iReturnValue) || (ERR_ACCT_CREATION_DISABLED == iReturnValue);
            if (bSuccessfulCondition && !CHECK_DEBUG_FLAG(WIZDEBUG_ERRPROJECTPROPERTIES)) {
                SetProjectPropertiesSucceeded(true);

                bSuccessfulCondition = pc->account_creation_disabled;
                if (bSuccessfulCondition || CHECK_DEBUG_FLAG(WIZDEBUG_ERRACCOUNTCREATIONDISABLED)) {
                    SetProjectAccountCreationDisabled(true);
                } else {
                    SetProjectAccountCreationDisabled(false);
                }

                bSuccessfulCondition = (ERR_ALREADY_ATTACHED == pDoc->rpc.project_attach(
                    ((CWizAttachProject*)GetParent())->m_ProjectInfoPage->GetProjectURL().c_str(),
                    ""
                ));
                if (bSuccessfulCondition || CHECK_DEBUG_FLAG(WIZDEBUG_ERRPROJECTALREADYATTACHED)) {
                    SetProjectAlreadyAttached(true);
                } else {
                    SetProjectAlreadyAttached(false);
                }

                bSuccessfulCondition = pc->client_account_creation_disabled;
                if (bSuccessfulCondition || CHECK_DEBUG_FLAG(WIZDEBUG_ERRCLIENTACCOUNTCREATIONDISABLED)) {
                    SetProjectClientAccountCreationDisabled(true);
                } else {
                    SetProjectClientAccountCreationDisabled(false);
                }
 
                SetNextState(PROJPROP_CLEANUP);
            } else {
                SetProjectPropertiesSucceeded(false);
                bSuccessfulCondition = (HTTP_STATUS_NOT_FOUND == iReturnValue) ||
                                       (ERR_GETHOSTBYNAME == iReturnValue) ||
                                       (ERR_XML_PARSE == iReturnValue);
                if (bSuccessfulCondition || CHECK_DEBUG_FLAG(WIZDEBUG_ERRPROJECTPROPERTIESURL)) {
                    SetProjectPropertiesURLFailure(true);
                } else {
                    SetProjectPropertiesURLFailure(false);
                }
                SetNextState(PROJPROP_COMMUNICATEYAHOO_BEGIN);
            }
            break;
        case PROJPROP_COMMUNICATEYAHOO_BEGIN:
            SetNextState(PROJPROP_COMMUNICATEYAHOO_EXECUTE);
            break;
        case PROJPROP_COMMUNICATEYAHOO_EXECUTE:
            // Attempt to successfully download the Yahoo homepage
            pDoc->rpc.lookup_website(LOOKUP_YAHOO);
 
            // Wait until we are done processing the request.
            dtStartExecutionTime = wxDateTime::Now();
            dtCurrentExecutionTime = wxDateTime::Now();
            tsExecutionTime = dtCurrentExecutionTime - dtStartExecutionTime;
            iReturnValue = ERR_IN_PROGRESS;
            while (ERR_IN_PROGRESS == iReturnValue &&
                   tsExecutionTime.GetSeconds() <= 60 &&
                   !((CWizAttachProject*)GetParent())->IsCancelInProgress()
                  )
            {
                dtCurrentExecutionTime = wxDateTime::Now();
                tsExecutionTime = dtCurrentExecutionTime - dtStartExecutionTime;
                iReturnValue = pDoc->rpc.lookup_website_poll();
                IncrementProgress(m_ProjectPropertiesProgress);

                ::wxMilliSleep(500);
                ::wxSafeYield(GetParent());
            }
 
            if ((BOINC_SUCCESS == iReturnValue) && !CHECK_DEBUG_FLAG(WIZDEBUG_ERRYAHOOCOMM)) {
                SetCommunicateYahooSucceeded(true);
            } else {
                SetCommunicateYahooSucceeded(false);
            }

            SetNextState(PROJPROP_COMMUNICATEGOOGLE_BEGIN);
            break;
        case PROJPROP_COMMUNICATEGOOGLE_BEGIN:
            SetNextState(PROJPROP_COMMUNICATEGOOGLE_EXECUTE);
            break;
        case PROJPROP_COMMUNICATEGOOGLE_EXECUTE:
            // Attempt to successfully download the Google homepage
            pDoc->rpc.lookup_website(LOOKUP_GOOGLE);
 
            // Wait until we are done processing the request.
            dtStartExecutionTime = wxDateTime::Now();
            dtCurrentExecutionTime = wxDateTime::Now();
            tsExecutionTime = dtCurrentExecutionTime - dtStartExecutionTime;
            iReturnValue = ERR_IN_PROGRESS;
            while (ERR_IN_PROGRESS == iReturnValue &&
                   tsExecutionTime.GetSeconds() <= 60 &&
                   !((CWizAttachProject*)GetParent())->IsCancelInProgress()
                  )
            {
                dtCurrentExecutionTime = wxDateTime::Now();
                tsExecutionTime = dtCurrentExecutionTime - dtStartExecutionTime;
                iReturnValue = pDoc->rpc.lookup_website_poll();
                IncrementProgress(m_ProjectPropertiesProgress);

                ::wxMilliSleep(500);
                ::wxSafeYield(GetParent());
            }
 
            if ((BOINC_SUCCESS == iReturnValue) && !CHECK_DEBUG_FLAG(WIZDEBUG_ERRGOOGLECOMM)) {
                SetCommunicateGoogleSucceeded(true);
            } else {
                SetCommunicateGoogleSucceeded(false);
            }

            SetNextState(PROJPROP_DETERMINENETWORKSTATUS_BEGIN);
            break;
        case PROJPROP_DETERMINENETWORKSTATUS_BEGIN:
            SetNextState(PROJPROP_DETERMINENETWORKSTATUS_EXECUTE);
            break;
        case PROJPROP_DETERMINENETWORKSTATUS_EXECUTE:
            // Attempt to determine if we are even connected to a network
            bSuccessfulCondition = CONNECTED_STATE_CONNECTED == get_connected_state();
            if (bSuccessfulCondition && !CHECK_DEBUG_FLAG(WIZDEBUG_ERRNETDETECTION)) {
                SetDeterminingConnectionStatusSucceeded(true);
            } else {
                SetDeterminingConnectionStatusSucceeded(false);
            }
            SetNextState(PROJPROP_CLEANUP);
            break;
        case PROJPROP_CLEANUP:
            FinishProgress(m_ProjectPropertiesProgress);
            SetNextState(PROJPROP_END);
            break;
        default:
            // Allow a glimps of what the result was before advancing to the next page.
            wxSleep(1);
            ((CWizAttachProject*)GetParent())->EnableNextButton();
            ((CWizAttachProject*)GetParent())->EnableBackButton();
            ((CWizAttachProject*)GetParent())->SimulateNextButton();
            bPostNewEvent = false;
            break;
    }
 
    Update();
 
    if (bPostNewEvent &&
        !((CWizAttachProject*)GetParent())->IsCancelInProgress()
       )
    {
        CProjectPropertiesPageEvent TransitionEvent(wxEVT_PROJECTPROPERTIES_STATECHANGE, this);
        AddPendingEvent(TransitionEvent);
    }
}
   
/*!
 * CAccountKeyPage type definition
 */
 
IMPLEMENT_DYNAMIC_CLASS( CAccountKeyPage, wxWizardPage )
 
/*!
 * CAccountKeyPage event table definition
 */
 
BEGIN_EVENT_TABLE( CAccountKeyPage, wxWizardPage )
 
////@begin CAccountKeyPage event table entries
    EVT_WIZARD_PAGE_CHANGED( -1, CAccountKeyPage::OnPageChanged )
    EVT_WIZARD_CANCEL( -1, CAccountKeyPage::OnCancel )

////@end CAccountKeyPage event table entries
 
END_EVENT_TABLE()
  
/*!
 * CAccountKeyPage constructors
 */
 
CAccountKeyPage::CAccountKeyPage( )
{
}
  
CAccountKeyPage::CAccountKeyPage( wxWizard* parent )
{
    Create( parent );
}
  
/*!
 * CAuthenticatorPage creator
 */
 
bool CAccountKeyPage::Create( wxWizard* parent )
{
////@begin CAccountKeyPage member initialisation
    m_AccountKeyStaticCtrl = NULL;
    m_AccountKeyCtrl = NULL;
////@end CAccountKeyPage member initialisation
 
////@begin CAccountKeyPage creation
    wxBitmap wizardBitmap(wxNullBitmap);
    wxWizardPage::Create( parent, wizardBitmap );

    CreateControls();
    GetSizer()->Fit(this);
////@end CAccountKeyPage creation

    return TRUE;
}
  
/*!
 * Control creation for CAuthenticatorPage
 */
 
void CAccountKeyPage::CreateControls()
{    
 
////@begin CAccountKeyPage content construction
    CAccountKeyPage* itemWizardPage44 = this;

    wxBoxSizer* itemBoxSizer45 = new wxBoxSizer(wxVERTICAL);
    itemWizardPage44->SetSizer(itemBoxSizer45);

    wxStaticText* itemStaticText46 = new wxStaticText;
    itemStaticText46->Create( itemWizardPage44, wxID_STATIC, _("Enter account key"), wxDefaultPosition, wxDefaultSize, 0 );
    itemStaticText46->SetFont(wxFont(10, wxSWISS, wxNORMAL, wxBOLD, FALSE, _T("Verdana")));
    itemBoxSizer45->Add(itemStaticText46, 0, wxALIGN_LEFT|wxALL, 5);

    itemBoxSizer45->Add(5, 5, 0, wxALIGN_LEFT|wxALL, 5);

    wxStaticText* itemStaticText48 = new wxStaticText;
    itemStaticText48->Create( itemWizardPage44, wxID_STATIC, _("This project uses an \"account key\" to identify you."), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer45->Add(itemStaticText48, 0, wxALIGN_LEFT|wxALL, 5);

    wxStaticText* itemStaticText49 = new wxStaticText;
    itemStaticText49->Create( itemWizardPage44, wxID_STATIC, _("Go to the project's web site to create an account. Your account\nkey will be emailed to you."), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer45->Add(itemStaticText49, 0, wxALIGN_LEFT|wxALL, 5);

    wxStaticText* itemStaticText50 = new wxStaticText;
    itemStaticText50->Create( itemWizardPage44, wxID_STATIC, _("An account key looks like:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer45->Add(itemStaticText50, 0, wxALIGN_LEFT|wxALL, 5);

    wxStaticText* itemStaticText51 = new wxStaticText;
    itemStaticText51->Create( itemWizardPage44, wxID_STATIC, _("82412313ac88e9a3638f66ea82186948"), wxDefaultPosition, wxDefaultSize, 0 );
    itemStaticText51->SetFont(wxFont(8, wxSWISS, wxNORMAL, wxNORMAL, FALSE, _T("Courier New")));
    itemBoxSizer45->Add(itemStaticText51, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 0);

    itemBoxSizer45->Add(5, 5, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxFlexGridSizer* itemFlexGridSizer53 = new wxFlexGridSizer(1, 2, 0, 0);
    itemFlexGridSizer53->AddGrowableCol(1);
    itemBoxSizer45->Add(itemFlexGridSizer53, 0, wxGROW|wxALL, 5);

    m_AccountKeyStaticCtrl = new wxStaticText;
    m_AccountKeyStaticCtrl->Create( itemWizardPage44, ID_ACCOUNTKEYSTATICCTRL, _("Account key:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer53->Add(m_AccountKeyStaticCtrl, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_AccountKeyCtrl = new wxTextCtrl;
    m_AccountKeyCtrl->Create( itemWizardPage44, ID_ACCOUNTKEYCTRL, _T(""), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer53->Add(m_AccountKeyCtrl, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    // Set validators
    m_AccountKeyCtrl->SetValidator( CValidateAccountKey( & m_strAccountKey) );
////@end CAccountKeyPage content construction
 
}
  
/*!
 * Gets the previous page.
 */
 
wxWizardPage* CAccountKeyPage::GetPrev() const
{
    return PAGE_TRANSITION_BACK;
}
  
/*!
 * Gets the next page.
 */
 
wxWizardPage* CAccountKeyPage::GetNext() const
{
    if (((CWizAttachProject*)GetParent())->IsCancelInProgress()) {
        // Cancel Event Detected
        return PAGE_TRANSITION_NEXT(ID_COMPLETIONERRORPAGE);
    } else {
        return PAGE_TRANSITION_NEXT(ID_ATTACHPROJECTPAGE);
    }
    return NULL;
}
 
/*!
 * Should we show tooltips?
 */
 
bool CAccountKeyPage::ShowToolTips()
{
    return TRUE;
}
  
/*!
 * Get bitmap resources
 */
 
wxBitmap CAccountKeyPage::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CAccountKeyPage bitmap retrieval
    return wxNullBitmap;
////@end CAccountKeyPage bitmap retrieval
}
  
/*!
 * Get icon resources
 */

wxIcon CAccountKeyPage::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CAccountKeyPage icon retrieval
    return wxNullIcon;
////@end CAccountKeyPage icon retrieval
}
  
/*!
 * wxEVT_WIZARD_PAGE_CHANGED event handler for ID_ACCOUNTKEYPAGE
 */
 
void CAccountKeyPage::OnPageChanged( wxWizardEvent& event ) {
    event.Skip();
}
  
/*!
 * wxEVT_WIZARD_CANCEL event handler for ID_ACCOUNTKEYPAGE
 */
 
void CAccountKeyPage::OnCancel( wxWizardEvent& event ) {
    ((CWizAttachProject*)GetParent())->ProcessCancelEvent(event);
}
 
/*!
 * CAccountInfoPage type definition
 */
 
IMPLEMENT_DYNAMIC_CLASS( CAccountInfoPage, wxWizardPage )
 
/*!
 * CAccountInfoPage event table definition
 */
 
BEGIN_EVENT_TABLE( CAccountInfoPage, wxWizardPage )
 
////@begin CAccountInfoPage event table entries
    EVT_WIZARD_PAGE_CHANGED( -1, CAccountInfoPage::OnPageChanged )
    EVT_WIZARD_PAGE_CHANGING( -1, CAccountInfoPage::OnPageChanging )
    EVT_WIZARD_CANCEL( -1, CAccountInfoPage::OnCancel )

    EVT_RADIOBUTTON( ID_ACCOUNTCREATECTRL, CAccountInfoPage::OnAccountCreateCtrlSelected )

    EVT_RADIOBUTTON( ID_ACCOUNTUSEEXISTINGCTRL, CAccountInfoPage::OnAccountUseExistingCtrlSelected )

////@end CAccountInfoPage event table entries
 
END_EVENT_TABLE()
 
/*!
 * CAccountInfoPage constructors
 */
 
CAccountInfoPage::CAccountInfoPage( )
{
}
 
CAccountInfoPage::CAccountInfoPage( wxWizard* parent )
{
    Create( parent );
}
 
/*!
 * AccountInfoPage creator
 */
 
bool CAccountInfoPage::Create( wxWizard* parent )
{

////@begin CAccountInfoPage member initialisation
    m_AccountCreateCtrl = NULL;
    m_AccountUseExistingCtrl = NULL;
    m_AccountEmailAddressStaticCtrl = NULL;
    m_AccountEmailAddressCtrl = NULL;
    m_AccountPasswordStaticCtrl = NULL;
    m_AccountPasswordCtrl = NULL;
    m_AccountConfirmPasswordStaticCtrl = NULL;
    m_AccountConfirmPasswordCtrl = NULL;
////@end CAccountInfoPage member initialisation
 
////@begin CAccountInfoPage creation
    wxBitmap wizardBitmap(wxNullBitmap);
    wxWizardPage::Create( parent, wizardBitmap );

    CreateControls();
    GetSizer()->Fit(this);
////@end CAccountInfoPage creation

    return TRUE;

}
 
/*!
 * Control creation for AccountInfoPage
 */
 
void CAccountInfoPage::CreateControls()
{    

////@begin CAccountInfoPage content construction
    CAccountInfoPage* itemWizardPage56 = this;

    wxBoxSizer* itemBoxSizer57 = new wxBoxSizer(wxVERTICAL);
    itemWizardPage56->SetSizer(itemBoxSizer57);

    wxStaticText* itemStaticText58 = new wxStaticText;
    itemStaticText58->Create( itemWizardPage56, wxID_STATIC, _("Account information"), wxDefaultPosition, wxDefaultSize, 0 );
    itemStaticText58->SetFont(wxFont(10, wxSWISS, wxNORMAL, wxBOLD, FALSE, _T("Verdana")));
    itemBoxSizer57->Add(itemStaticText58, 0, wxALIGN_LEFT|wxALL, 5);

    wxStaticText* itemStaticText59 = new wxStaticText;
    itemStaticText59->Create( itemWizardPage56, wxID_STATIC, _("Do you wish to use an existing account or create a new one?"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer57->Add(itemStaticText59, 0, wxALIGN_LEFT|wxALL, 5);

    itemBoxSizer57->Add(5, 5, 0, wxALIGN_LEFT|wxALL, 5);

    wxFlexGridSizer* itemFlexGridSizer61 = new wxFlexGridSizer(1, 2, 0, 0);
    itemFlexGridSizer61->AddGrowableCol(1);
    itemBoxSizer57->Add(itemFlexGridSizer61, 0, wxGROW|wxALL, 5);

    m_AccountCreateCtrl = new wxRadioButton;
    m_AccountCreateCtrl->Create( itemWizardPage56, ID_ACCOUNTCREATECTRL, _("Create new account"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP );
    m_AccountCreateCtrl->SetValue(TRUE);
    itemFlexGridSizer61->Add(m_AccountCreateCtrl, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_AccountUseExistingCtrl = new wxRadioButton;
    m_AccountUseExistingCtrl->Create( itemWizardPage56, ID_ACCOUNTUSEEXISTINGCTRL, _("Use existing account"), wxDefaultPosition, wxDefaultSize, 0 );
    m_AccountUseExistingCtrl->SetValue(FALSE);
    itemFlexGridSizer61->Add(m_AccountUseExistingCtrl, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxFlexGridSizer* itemFlexGridSizer64 = new wxFlexGridSizer(3, 2, 0, 0);
    itemFlexGridSizer64->AddGrowableCol(1);
    itemBoxSizer57->Add(itemFlexGridSizer64, 0, wxGROW|wxALL, 5);

    m_AccountEmailAddressStaticCtrl = new wxStaticText;
    m_AccountEmailAddressStaticCtrl->Create( itemWizardPage56, ID_ACCOUNTEMAILADDRESSSTATICCTRL, _("Email address:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer64->Add(m_AccountEmailAddressStaticCtrl, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_AccountEmailAddressCtrl = new wxTextCtrl;
    m_AccountEmailAddressCtrl->Create( itemWizardPage56, ID_ACCOUNTEMAILADDRESSCTRL, _T(""), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer64->Add(m_AccountEmailAddressCtrl, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_AccountPasswordStaticCtrl = new wxStaticText;
    m_AccountPasswordStaticCtrl->Create( itemWizardPage56, ID_ACCOUNTPASSWORDSTATICCTRL, _("Password:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer64->Add(m_AccountPasswordStaticCtrl, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_AccountPasswordCtrl = new wxTextCtrl;
    m_AccountPasswordCtrl->Create( itemWizardPage56, ID_ACCOUNTPASSWORDCTRL, _T(""), wxDefaultPosition, wxDefaultSize, wxTE_PASSWORD );
    itemFlexGridSizer64->Add(m_AccountPasswordCtrl, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_AccountConfirmPasswordStaticCtrl = new wxStaticText;
    m_AccountConfirmPasswordStaticCtrl->Create( itemWizardPage56, ID_ACCOUNTCONFIRMPASSWORDSTATICCTRL, _("Confirm password:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer64->Add(m_AccountConfirmPasswordStaticCtrl, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_AccountConfirmPasswordCtrl = new wxTextCtrl;
    m_AccountConfirmPasswordCtrl->Create( itemWizardPage56, ID_ACCOUNTCONFIRMPASSWORDCTRL, _T(""), wxDefaultPosition, wxDefaultSize, wxTE_PASSWORD );
    itemFlexGridSizer64->Add(m_AccountConfirmPasswordCtrl, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    // Set validators
    m_AccountEmailAddressCtrl->SetValidator( wxGenericValidator(& m_strAccountEmailAddress) );
    m_AccountPasswordCtrl->SetValidator( wxGenericValidator(& m_strAccountPassword) );
    m_AccountConfirmPasswordCtrl->SetValidator( wxGenericValidator(& m_strAccountConfirmPassword) );
////@end CAccountInfoPage content construction

}
 
/*!
 * Gets the previous page.
 */
 
wxWizardPage* CAccountInfoPage::GetPrev() const
{
    return PAGE_TRANSITION_BACK;
}
 
/*!
 * Gets the next page.
 */
 
wxWizardPage* CAccountInfoPage::GetNext() const
{
    if (((CWizAttachProject*)GetParent())->IsCancelInProgress()) {
        // Cancel Event Detected
        return PAGE_TRANSITION_NEXT(ID_COMPLETIONERRORPAGE);
    } else {
        return PAGE_TRANSITION_NEXT(ID_ATTACHPROJECTPAGE);
    }
    return NULL;
}
 
/*!
 * Should we show tooltips?
 */
 
bool CAccountInfoPage::ShowToolTips()
{
    return TRUE;
}
 
/*!
 * Get bitmap resources
 */
 
wxBitmap CAccountInfoPage::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CAccountInfoPage bitmap retrieval
    return wxNullBitmap;
////@end CAccountInfoPage bitmap retrieval
}
 
/*!
 * Get icon resources
 */
 
wxIcon CAccountInfoPage::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CAccountInfoPage icon retrieval
    return wxNullIcon;
////@end CAccountInfoPage icon retrieval
}
  
/*!
 * wxEVT_WIZARD_PAGE_CHANGED event handler for ID_ACCOUNTINFOPAGE
 */
 
void CAccountInfoPage::OnPageChanged( wxWizardEvent& event ) 
{
    if (event.GetDirection() == false) return;

    static bool bRunOnce = true;
    if (bRunOnce) {
        bRunOnce = false;
        m_AccountCreateCtrl->SetValue(TRUE);
        m_AccountUseExistingCtrl->SetValue(FALSE);
    }

    if (((CWizAttachProject*)GetParent())->m_ProjectPropertiesPage->GetProjectAccountCreationDisabled()) {
        m_AccountCreateCtrl->SetValue(false);
        m_AccountUseExistingCtrl->SetValue(true);
        m_AccountConfirmPasswordStaticCtrl->Hide();
        m_AccountConfirmPasswordCtrl->Hide();

        m_AccountCreateCtrl->Disable();
    }

    if (((CWizAttachProject*)GetParent())->project_config.uses_username) {
        m_AccountEmailAddressStaticCtrl->SetLabel(
            _("Username:")
        );
    } else {
        m_AccountEmailAddressStaticCtrl->SetLabel(
            _("Email address:")
        );
    }
 
    Fit();
}
  
/*!
 * wxEVT_WIZARD_PAGE_CHANGING event handler for ID_ACCOUNTINFOPAGE
 */
 
void CAccountInfoPage::OnPageChanging( wxWizardEvent& event )
{
    if (event.GetDirection() == false) return;
 
    if (!((CWizAttachProject*)GetParent())->IsCancelInProgress()) {
        wxString strTitle = _("Attach to Project");
        wxString strMessage = wxT("");
        bool     bDisplayError = false;
 
        // Validate a new account against the account creation policies
        if (m_AccountCreateCtrl->GetValue()) {
            // Verify minimum password length
            unsigned int iMinLength = ((CWizAttachProject*)GetParent())->project_config.min_passwd_length;
            wxString strPassword = m_AccountPasswordCtrl->GetValue();
            if (strPassword.Length() < iMinLength) {
                strMessage.Printf(
                    _("The minimum password length for this project is %d. Please choose a different password."),
                    iMinLength
                );

                bDisplayError = true;
            }
 
            // Verify that the password and confirmation password math.
            if (m_AccountPasswordCtrl->GetValue() != m_AccountConfirmPasswordCtrl->GetValue()) {
                strMessage = _("The password and confirmation password do not match. Please type them again.");
                bDisplayError = true;
            }
        }
 
        if (bDisplayError) {
            ::wxMessageBox(
                strMessage,
                strTitle,
                wxICON_ERROR | wxOK,
                this
            );
            event.Veto();
        }
    }
}
  
/*!
 * wxEVT_WIZARD_CANCEL event handler for ID_ACCOUNTINFOPAGE
 */
 
void CAccountInfoPage::OnCancel( wxWizardEvent& event ) {
    ((CWizAttachProject*)GetParent())->ProcessCancelEvent(event);
}
 
/*!
 * wxEVT_COMMAND_RADIOBUTTON_SELECTED event handler for ID_ACCOUNTUSEXISTINGBUTTON
 */
 
void CAccountInfoPage::OnAccountUseExistingCtrlSelected( wxCommandEvent& event ) {
    m_AccountConfirmPasswordStaticCtrl->Hide();
    m_AccountConfirmPasswordCtrl->Hide();
}
  
/*!
 * wxEVT_COMMAND_RADIOBUTTON_SELECTED event handler for ID_ACCOUNTCREATEBUTTON
 */
 
void CAccountInfoPage::OnAccountCreateCtrlSelected( wxCommandEvent& event ) {
    m_AccountConfirmPasswordStaticCtrl->Show();
    m_AccountConfirmPasswordCtrl->Show();
}
 
/*!
 * CProjectPropertiesPage custom event definition
 */

DEFINE_EVENT_TYPE(wxEVT_ATTACHPROJECT_STATECHANGE)
  
/*!
 * CAttachProjectPage type definition
 */
 
IMPLEMENT_DYNAMIC_CLASS( CAttachProjectPage, wxWizardPage )
  
/*!
 * CAttachProjectPage event table definition
 */
 
BEGIN_EVENT_TABLE( CAttachProjectPage, wxWizardPage )
 
    EVT_ATTACHPROJECT_STATECHANGE( CAttachProjectPage::OnStateChange )
 
////@begin CAttachProjectPage event table entries
    EVT_WIZARD_PAGE_CHANGED( -1, CAttachProjectPage::OnPageChanged )
    EVT_WIZARD_CANCEL( -1, CAttachProjectPage::OnCancel )

////@end CAttachProjectPage event table entries
 
END_EVENT_TABLE()
  
/*!
 * CAttachProjectPage constructors
 */
 
CAttachProjectPage::CAttachProjectPage( )
{
}
  
CAttachProjectPage::CAttachProjectPage( wxWizard* parent )
{
    Create( parent );
}
  
/*!
 * CProjectPropertiesPage creator
 */
 
bool CAttachProjectPage::Create( wxWizard* parent )
{
 
////@begin CAttachProjectPage member initialisation
    m_AttachProjectProgress = NULL;
////@end CAttachProjectPage member initialisation
 
    m_bProjectCommunitcationsSucceeded = false;
    m_bProjectUnavailable = false;
    m_bProjectAccountAlreadyExists = false;
    m_iBitmapIndex = 0;
    m_iCurrentState = ATTACHPROJECT_INIT;
 
////@begin CAttachProjectPage creation
    wxBitmap wizardBitmap(wxNullBitmap);
    wxWizardPage::Create( parent, wizardBitmap );

    CreateControls();
    GetSizer()->Fit(this);
////@end CAttachProjectPage creation
 
    return TRUE;
}
 
/*!
 * Control creation for CProjectPropertiesPage
 */
 
void CAttachProjectPage::CreateControls()
{    
////@begin CAttachProjectPage content construction
    CAttachProjectPage* itemWizardPage71 = this;

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
    m_AttachProjectProgress = new wxStaticBitmap;
    m_AttachProjectProgress->Create( itemWizardPage71, ID_ATTACHPROJECTPROGRESS, m_AttachProjectProgressBitmap, wxDefaultPosition, wxSize(184, 48), 0 );
    itemFlexGridSizer75->Add(m_AttachProjectProgress, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    itemFlexGridSizer75->Add(5, 5, 0, wxGROW|wxGROW|wxALL, 5);

////@end CAttachProjectPage content construction
}
  
/*!
 * Gets the previous page.
 */
 
wxWizardPage* CAttachProjectPage::GetPrev() const
{
    return PAGE_TRANSITION_BACK;
}
  
/*!
 * Gets the next page.
 */
 
wxWizardPage* CAttachProjectPage::GetNext() const
{
    if (((CWizAttachProject*)GetParent())->IsCancelInProgress()) {
        // Cancel Event Detected
        return PAGE_TRANSITION_NEXT(ID_COMPLETIONERRORPAGE);
    } else if (GetProjectAttachSucceeded()) {
        // We were successful in creating or retrieving an account
        return PAGE_TRANSITION_NEXT(ID_COMPLETIONPAGE);
    } else if (!GetProjectCommunitcationsSucceeded() && GetProjectAccountAlreadyExists()) {
        // The requested account already exists
        return PAGE_TRANSITION_NEXT(ID_ERRACCOUNTALREADYEXISTSPAGE);
    } else if (!GetProjectCommunitcationsSucceeded() && GetProjectAccountNotFound()) {
        // The requested account does not exist or the password is bad
        return PAGE_TRANSITION_NEXT(ID_ERRACCOUNTNOTFOUNDPAGE);
    } else {
        // The project much be down for maintenance
        return PAGE_TRANSITION_NEXT(ID_ERRPROJECTUNAVAILABLEPAGE);
    } 
    return NULL;
}
  
/*!
 * Should we show tooltips?
 */
 
bool CAttachProjectPage::ShowToolTips()
{
    return TRUE;
}
 
void CAttachProjectPage::StartProgress(wxStaticBitmap* pBitmap) {
    m_iBitmapIndex = 1;
    pBitmap->SetBitmap(GetBitmapResource(wxT("res/wizprogress01.xpm")));
}
 
void CAttachProjectPage::IncrementProgress(wxStaticBitmap* pBitmap) {
    m_iBitmapIndex += 1;
    if (12 < m_iBitmapIndex) m_iBitmapIndex = 1;
 
    wxString str;
    str.Printf(wxT("res/wizprogress%02d.xpm"), m_iBitmapIndex);
 
    pBitmap->SetBitmap(GetBitmapResource(str));
    Update();
}
 
void CAttachProjectPage::FinishProgress(wxStaticBitmap* pBitmap) {
    m_iBitmapIndex = 12;
    pBitmap->SetBitmap(GetBitmapResource(wxT("res/wizprogress12.xpm")));
}
 
/*!
 * Get bitmap resources
 */
 
wxBitmap CAttachProjectPage::GetBitmapResource( const wxString& name )
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
 
wxIcon CAttachProjectPage::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CAttachProjectPage icon retrieval
    return wxNullIcon;
////@end CAttachProjectPage icon retrieval
}
  
/*!
 * wxEVT_WIZARD_PAGE_CHANGED event handler for ID_ATTACHPROJECTPAGE
 */
 
void CAttachProjectPage::OnPageChanged( wxWizardEvent& event ) {
    if (event.GetDirection() == false) return;

    SetProjectCommunitcationsSucceeded(false);
    SetProjectUnavailable(false);
    SetProjectAccountAlreadyExists(false);
    SetNextState(ATTACHPROJECT_INIT);
 
    CAttachProjectPageEvent TransitionEvent(wxEVT_ATTACHPROJECT_STATECHANGE, this);
    AddPendingEvent(TransitionEvent);
}
  
/*!
 * wxEVT_WIZARD_CANCEL event handler for ID_ACCOUNTCREATIONPAGE
 */
 
void CAttachProjectPage::OnCancel( wxWizardEvent& event ) {
    ((CWizAttachProject*)GetParent())->ProcessCancelEvent(event);
}
 
/*!
 * wxEVT_ACCOUNTCREATION_STATECHANGE event handler for ID_ACCOUNTCREATIONPAGE
 */
 
void CAttachProjectPage::OnStateChange( CAttachProjectPageEvent& event )
{
    CMainDocument* pDoc      = wxGetApp().GetDocument();
    ACCOUNT_IN* ai           = &((CWizAttachProject*)GetParent())->account_in;
    ACCOUNT_OUT* ao          = &((CWizAttachProject*)GetParent())->account_out;
    wxDateTime dtStartExecutionTime;
    wxDateTime dtCurrentExecutionTime;
    wxTimeSpan tsExecutionTime;
    bool bPostNewEvent = true;
    bool bSuccessfulCondition = false;
    int iReturnValue = 0;
 
    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));
 
    switch(GetCurrentState()) {
        case ATTACHPROJECT_INIT:
            ((CWizAttachProject*)GetParent())->DisableNextButton();
            ((CWizAttachProject*)GetParent())->DisableBackButton();

            StartProgress(m_AttachProjectProgress);
            SetNextState(ATTACHPROJECT_ACCOUNTQUERY_BEGIN);
            break;
        case ATTACHPROJECT_ACCOUNTQUERY_BEGIN:
            SetNextState(ATTACHPROJECT_ACCOUNTQUERY_EXECUTE);
            break;
        case ATTACHPROJECT_ACCOUNTQUERY_EXECUTE:
            // Attempt to create the account or reterieve the authenticator.
            ai->clear();
            ao->clear();

            ai->url = ((CWizAttachProject*)GetParent())->m_ProjectInfoPage->GetProjectURL().c_str();

            if (!((CWizAttachProject*)GetParent())->m_AccountKeyPage->m_strAccountKey.IsEmpty()) {
                ao->authenticator = ((CWizAttachProject*)GetParent())->m_AccountKeyPage->m_strAccountKey.c_str();
                SetProjectCommunitcationsSucceeded(true);
            } else {
                if (((CWizAttachProject*)GetParent())->m_AccountInfoPage->m_AccountCreateCtrl->GetValue()) {
                    if (!((CWizAttachProject*)GetParent())->project_config.uses_username) {
                        ai->email_addr = ((CWizAttachProject*)GetParent())->m_AccountInfoPage->GetAccountEmailAddress().c_str();
                        ai->user_name = ::wxGetUserName().c_str();
                    } else {
                        ai->email_addr = wxT("");
                        ai->user_name = ((CWizAttachProject*)GetParent())->m_AccountInfoPage->GetAccountEmailAddress().c_str();
                    }
                    ai->passwd = ((CWizAttachProject*)GetParent())->m_AccountInfoPage->GetAccountPassword().c_str();
                    pDoc->rpc.create_account(*ai);

                    // Wait until we are done processing the request.
                    dtStartExecutionTime = wxDateTime::Now();
                    dtCurrentExecutionTime = wxDateTime::Now();
                    tsExecutionTime = dtCurrentExecutionTime - dtStartExecutionTime;
                    iReturnValue = ERR_IN_PROGRESS;
                    while (ERR_IN_PROGRESS == iReturnValue &&
                        tsExecutionTime.GetSeconds() <= 60 &&
                        !((CWizAttachProject*)GetParent())->IsCancelInProgress()
                        )
                    {
                        dtCurrentExecutionTime = wxDateTime::Now();
                        tsExecutionTime = dtCurrentExecutionTime - dtStartExecutionTime;
                        iReturnValue = pDoc->rpc.create_account_poll(*ao);

                        IncrementProgress(m_AttachProjectProgress);

                        ::wxMilliSleep(500);
                        ::wxSafeYield(GetParent());
                    }

                    if ((BOINC_SUCCESS == iReturnValue) && !CHECK_DEBUG_FLAG(WIZDEBUG_ERRPROJECTCOMM)) {
                        ((CWizAttachProject*)GetParent())->SetAccountCreatedSuccessfully(true);
                    }
                } else {
                    if (!((CWizAttachProject*)GetParent())->project_config.uses_username) {
                        ai->email_addr = ((CWizAttachProject*)GetParent())->m_AccountInfoPage->GetAccountEmailAddress().c_str();
                    } else {
                        ai->user_name= ((CWizAttachProject*)GetParent())->m_AccountInfoPage->GetAccountEmailAddress().c_str();
                    }
                    ai->passwd = ((CWizAttachProject*)GetParent())->m_AccountInfoPage->GetAccountPassword().c_str();

                    pDoc->rpc.lookup_account(*ai);
 
                    // Wait until we are done processing the request.
                    dtStartExecutionTime = wxDateTime::Now();
                    dtCurrentExecutionTime = wxDateTime::Now();
                    tsExecutionTime = dtCurrentExecutionTime - dtStartExecutionTime;
                    iReturnValue = ERR_IN_PROGRESS;
                    while (ERR_IN_PROGRESS == iReturnValue &&
                        tsExecutionTime.GetSeconds() <= 60 &&
                        !((CWizAttachProject*)GetParent())->IsCancelInProgress()
                        )
                    {
                        dtCurrentExecutionTime = wxDateTime::Now();
                        tsExecutionTime = dtCurrentExecutionTime - dtStartExecutionTime;
                        iReturnValue = pDoc->rpc.lookup_account_poll(*ao);

                        IncrementProgress(m_AttachProjectProgress);

                        ::wxMilliSleep(500);
                        ::wxSafeYield(GetParent());
                    }
                }
 
                if ((BOINC_SUCCESS == iReturnValue) && !CHECK_DEBUG_FLAG(WIZDEBUG_ERRPROJECTCOMM)) {
                    SetProjectCommunitcationsSucceeded(true);
                } else {
                    SetProjectCommunitcationsSucceeded(false);
                    if ((ERR_NONUNIQUE_EMAIL == iReturnValue) || CHECK_DEBUG_FLAG(WIZDEBUG_ERRACCOUNTALREADYEXISTS)) {
                        SetProjectAccountAlreadyExists(true);
                    } else {
                        SetProjectAccountAlreadyExists(false);
                    }
                    if ((ERR_NOT_FOUND == iReturnValue) || CHECK_DEBUG_FLAG(WIZDEBUG_ERRACCOUNTNOTFOUND)) {
                        SetProjectAccountNotFound(true);
                    } else {
                        SetProjectAccountNotFound(false);
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
                pDoc->rpc.project_attach(
                    ai->url.c_str(),
                    ao->authenticator.c_str()
                );
     
                // Wait until we are done processing the request.
                dtStartExecutionTime = wxDateTime::Now();
                dtCurrentExecutionTime = wxDateTime::Now();
                tsExecutionTime = dtCurrentExecutionTime - dtStartExecutionTime;
                iReturnValue = ERR_IN_PROGRESS;
                while (ERR_IN_PROGRESS == iReturnValue &&
                    tsExecutionTime.GetSeconds() <= 60 &&
                    !((CWizAttachProject*)GetParent())->IsCancelInProgress()
                    )
                {
                    dtCurrentExecutionTime = wxDateTime::Now();
                    tsExecutionTime = dtCurrentExecutionTime - dtStartExecutionTime;
                    iReturnValue = pDoc->rpc.project_attach_poll();

                    IncrementProgress(m_AttachProjectProgress);

                    ::wxMilliSleep(500);
                    ::wxSafeYield(GetParent());
                }
     
                if ((BOINC_SUCCESS == iReturnValue) && !CHECK_DEBUG_FLAG(WIZDEBUG_ERRPROJECTATTACH)) {
                    SetProjectAttachSucceeded(true);
                    ((CWizAttachProject*)GetParent())->SetAttachedToProjectSuccessfully(true);
                    ((CWizAttachProject*)GetParent())->SetProjectURL(ai->url.c_str());
                    ((CWizAttachProject*)GetParent())->SetProjectAuthenticator(ao->authenticator.c_str());
                } else {
                    SetProjectAttachSucceeded(false);
                }
            } else {
                SetProjectAttachSucceeded(false);
            }
            SetNextState(ATTACHPROJECT_CLEANUP);
            break;
        case ATTACHPROJECT_CLEANUP:
            SetNextState(ATTACHPROJECT_END);
            break;
        default:
            // Allow a glimps of what the result was before advancing to the next page.
            wxSleep(1);
            ((CWizAttachProject*)GetParent())->EnableNextButton();
            ((CWizAttachProject*)GetParent())->EnableBackButton();
            ((CWizAttachProject*)GetParent())->SimulateNextButton();
            bPostNewEvent = false;
            break;
    }
 
    Update();
 
    if (bPostNewEvent &&
        !((CWizAttachProject*)GetParent())->IsCancelInProgress()
       )
    {
        CAttachProjectPageEvent TransitionEvent(wxEVT_ATTACHPROJECT_STATECHANGE, this);
        AddPendingEvent(TransitionEvent);
    }
}
  
/*!
 * CCompletionPage type definition
 */
 
IMPLEMENT_DYNAMIC_CLASS( CCompletionPage, wxWizardPage )
 
/*!
 * CCompletionPage event table definition
 */
 
BEGIN_EVENT_TABLE( CCompletionPage, wxWizardPage )
 
////@begin CCompletionPage event table entries
    EVT_WIZARD_PAGE_CHANGED( -1, CCompletionPage::OnPageChanged )
    EVT_WIZARD_CANCEL( -1, CCompletionPage::OnCancel )
    EVT_WIZARD_FINISHED( ID_COMPLETIONPAGE, CCompletionPage::OnFinished )

////@end CCompletionPage event table entries
 
END_EVENT_TABLE()
  
/*!
 * CCompletionPage constructors
 */
 
CCompletionPage::CCompletionPage( )
{
}
 
CCompletionPage::CCompletionPage( wxWizard* parent )
{
    Create( parent );
}
 
/*!
 * CCompletionPage creator
 */
 
bool CCompletionPage::Create( wxWizard* parent )
{
////@begin CCompletionPage member initialisation
    m_CompletionMessage = NULL;
////@end CCompletionPage member initialisation
 
////@begin CCompletionPage creation
    wxBitmap wizardBitmap(wxNullBitmap);
    wxWizardPage::Create( parent, wizardBitmap );

    CreateControls();
    GetSizer()->Fit(this);
////@end CCompletionPage creation
 
    return TRUE;
}
  
/*!
 * Control creation for CCompletionPage
 */
 
void CCompletionPage::CreateControls()
{    
////@begin CCompletionPage content construction
    CCompletionPage* itemWizardPage79 = this;

    wxBoxSizer* itemBoxSizer80 = new wxBoxSizer(wxVERTICAL);
    itemWizardPage79->SetSizer(itemBoxSizer80);

    wxStaticText* itemStaticText81 = new wxStaticText;
    itemStaticText81->Create( itemWizardPage79, wxID_STATIC, _("Attached to project"), wxDefaultPosition, wxDefaultSize, 0 );
    itemStaticText81->SetFont(wxFont(12, wxSWISS, wxNORMAL, wxBOLD, FALSE, _T("Verdana")));
    itemBoxSizer80->Add(itemStaticText81, 0, wxALIGN_LEFT|wxALL, 5);

    wxStaticText* itemStaticText82 = new wxStaticText;
    itemStaticText82->Create( itemWizardPage79, wxID_STATIC, _("You are now successfully attached to this project."), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer80->Add(itemStaticText82, 0, wxALIGN_LEFT|wxALL, 5);

    itemBoxSizer80->Add(5, 5, 0, wxALIGN_LEFT|wxALL, 5);

    m_CompletionMessage = new wxStaticText;
    m_CompletionMessage->Create( itemWizardPage79, wxID_STATIC, _("When you click Finish, your web browser will go to a page where\nyou can set your account name and preferences."), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer80->Add(m_CompletionMessage, 0, wxALIGN_LEFT|wxALL, 5);

////@end CCompletionPage content construction
}
  
/*!
 * Gets the previous page.
 */
 
wxWizardPage* CCompletionPage::GetPrev() const
{
    return PAGE_TRANSITION_BACK;
}
 
/*!
 * Gets the next page.
 */
 
wxWizardPage* CCompletionPage::GetNext() const
{
    return NULL;
}
  
/*!
 * Should we show tooltips?
 */
 
bool CCompletionPage::ShowToolTips()
{
    return TRUE;
}
  
/*!
 * Get bitmap resources
 */
 
wxBitmap CCompletionPage::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CCompletionPage bitmap retrieval
    return wxNullBitmap;
////@end CCompletionPage bitmap retrieval
}
  
/*!
 * Get icon resources
 */
 
wxIcon CCompletionPage::GetIconResource( const wxString& name )
{
    // Icon retrieval
 
////@begin CCompletionPage icon retrieval
    return wxNullIcon;
////@end CCompletionPage icon retrieval
}
  
/*!
 * wxEVT_WIZARD_PAGE_CHANGED event handler for ID_COMPLETIONPAGE
 */
 
void CCompletionPage::OnPageChanged( wxWizardEvent& event ) {
    if (event.GetDirection() == false) return;

    if (((CWizAttachProject*)GetParent())->m_AccountInfoPage->m_AccountCreateCtrl->GetValue()) {
        m_CompletionMessage->SetLabel(_("When you click Finish, your web browser will go to a page where\nyou can set your account name and preferences."));
    } else {
        m_CompletionMessage->SetLabel(_("Click Finish to close."));
    }
}
  
/*!
 * wxEVT_WIZARD_CANCEL event handler for ID_COMPLETIONPAGE
 */
 
void CCompletionPage::OnCancel( wxWizardEvent& event ) {
    ((CWizAttachProject*)GetParent())->ProcessCancelEvent(event);
}
 
/*!
 * wxEVT_WIZARD_FINISHED event handler for ID_COMPLETIONPAGE
 */
 
void CCompletionPage::OnFinished( wxWizardEvent& event ) {
    event.Skip();
}
 
/*!
 * CCompletionErrorPage type definition
 */
 
IMPLEMENT_DYNAMIC_CLASS( CCompletionErrorPage, wxWizardPage )
 
/*!
 * CCompletionErrorPage event table definition
 */
 
BEGIN_EVENT_TABLE( CCompletionErrorPage, wxWizardPage )
 
////@begin CCompletionErrorPage event table entries
    EVT_WIZARD_PAGE_CHANGED( -1, CCompletionErrorPage::OnPageChanged )
    EVT_WIZARD_CANCEL( -1, CCompletionErrorPage::OnCancel )

////@end CCompletionErrorPage event table entries
 
END_EVENT_TABLE()
 
/*!
 * CCompletionErrorPage constructors
 */
 
CCompletionErrorPage::CCompletionErrorPage( )
{
}
 
CCompletionErrorPage::CCompletionErrorPage( wxWizard* parent )
{
    Create( parent );
}
 
/*!
 * CAccountResultPage creator
 */
 
bool CCompletionErrorPage::Create( wxWizard* parent )
{
////@begin CCompletionErrorPage member initialisation
////@end CCompletionErrorPage member initialisation
 
////@begin CCompletionErrorPage creation
    wxBitmap wizardBitmap(wxNullBitmap);
    wxWizardPage::Create( parent, wizardBitmap );

    CreateControls();
    GetSizer()->Fit(this);
////@end CCompletionErrorPage creation

    return TRUE;
}
 
/*!
 * Control creation for CAccountResultPage
 */
 
void CCompletionErrorPage::CreateControls()
{    
////@begin CCompletionErrorPage content construction
    CCompletionErrorPage* itemWizardPage85 = this;

    wxBoxSizer* itemBoxSizer86 = new wxBoxSizer(wxVERTICAL);
    itemWizardPage85->SetSizer(itemBoxSizer86);

    wxStaticText* itemStaticText87 = new wxStaticText;
    itemStaticText87->Create( itemWizardPage85, wxID_STATIC, _("Failed to attach to project"), wxDefaultPosition, wxDefaultSize, 0 );
    itemStaticText87->SetFont(wxFont(12, wxSWISS, wxNORMAL, wxBOLD, FALSE, _T("Verdana")));
    itemBoxSizer86->Add(itemStaticText87, 0, wxALIGN_LEFT|wxALL, 5);

    itemBoxSizer86->Add(5, 5, 0, wxALIGN_LEFT|wxALL, 5);

    wxStaticText* itemStaticText89 = new wxStaticText;
    itemStaticText89->Create( itemWizardPage85, wxID_STATIC, _("Click Finish to close."), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer86->Add(itemStaticText89, 0, wxALIGN_LEFT|wxALL, 5);

////@end CCompletionErrorPage content construction
}
 
/*!
 * Gets the previous page.
 */
 
wxWizardPage* CCompletionErrorPage::GetPrev() const
{
    return NULL;
}
 
/*!
 * Gets the next page.
 */
 
wxWizardPage* CCompletionErrorPage::GetNext() const
{
    return NULL;
}
 
/*!
 * Should we show tooltips?
 */
 
bool CCompletionErrorPage::ShowToolTips()
{
    return TRUE;
}
 
/*!
 * Get bitmap resources
 */
 
wxBitmap CCompletionErrorPage::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval

////@begin CCompletionErrorPage bitmap retrieval
    return wxNullBitmap;
////@end CCompletionErrorPage bitmap retrieval
}
 
/*!
 * Get icon resources
 */
 
wxIcon CCompletionErrorPage::GetIconResource( const wxString& name )
{
    // Icon retrieval

////@begin CCompletionErrorPage icon retrieval
    return wxNullIcon;
////@end CCompletionErrorPage icon retrieval
}
  
/*!
 * wxEVT_WIZARD_PAGE_CHANGED event handler for ID_COMPLETIONERRORPAGE
 */
 
void CCompletionErrorPage::OnPageChanged( wxWizardEvent& event ) {
    event.Skip();
}
 
/*!
 * wxEVT_WIZARD_CANCEL event handler for ID_COMPLETIONERRORPAGE
 */
 
void CCompletionErrorPage::OnCancel( wxWizardEvent& event ) {
    ((CWizAttachProject*)GetParent())->ProcessCancelEvent(event);
}
 
/*!
 * CErrProjectNotDetectedPage type definition
 */
 
IMPLEMENT_DYNAMIC_CLASS( CErrProjectNotDetectedPage, wxWizardPage )
 
/*!

 * CErrProjectNotDetectedPage event table definition

 */
 
BEGIN_EVENT_TABLE( CErrProjectNotDetectedPage, wxWizardPage )
 
////@begin CErrProjectNotDetectedPage event table entries
    EVT_WIZARD_PAGE_CHANGED( -1, CErrProjectNotDetectedPage::OnPageChanged )
    EVT_WIZARD_CANCEL( -1, CErrProjectNotDetectedPage::OnCancel )

////@end CErrProjectNotDetectedPage event table entries
 
END_EVENT_TABLE()
 
/*!
 * CErrProjectNotDetectedPage constructors
 */
 
CErrProjectNotDetectedPage::CErrProjectNotDetectedPage( )
{
}
 
CErrProjectNotDetectedPage::CErrProjectNotDetectedPage( wxWizard* parent )
{
    Create( parent );
}
 
/*!
 * CErrProjectUnavailable creator
 */
 
bool CErrProjectNotDetectedPage::Create( wxWizard* parent )
{
////@begin CErrProjectNotDetectedPage member initialisation
////@end CErrProjectNotDetectedPage member initialisation
 
////@begin CErrProjectNotDetectedPage creation
    wxBitmap wizardBitmap(wxNullBitmap);
    wxWizardPage::Create( parent, wizardBitmap );

    CreateControls();
    GetSizer()->Fit(this);
////@end CErrProjectNotDetectedPage creation

    return TRUE;
}
 
/*!
 * Control creation for CErrProjectUnavailable
 */
 
void CErrProjectNotDetectedPage::CreateControls()
{    
////@begin CErrProjectNotDetectedPage content construction
    CErrProjectNotDetectedPage* itemWizardPage90 = this;

    wxBoxSizer* itemBoxSizer91 = new wxBoxSizer(wxVERTICAL);
    itemWizardPage90->SetSizer(itemBoxSizer91);

    wxStaticText* itemStaticText92 = new wxStaticText;
    itemStaticText92->Create( itemWizardPage90, wxID_STATIC, _("Project not found"), wxDefaultPosition, wxDefaultSize, 0 );
    itemStaticText92->SetFont(wxFont(10, wxSWISS, wxNORMAL, wxBOLD, FALSE, _T("Verdana")));
    itemBoxSizer91->Add(itemStaticText92, 0, wxALIGN_LEFT|wxALL, 5);

    itemBoxSizer91->Add(5, 5, 0, wxALIGN_LEFT|wxALL, 5);

    wxStaticText* itemStaticText94 = new wxStaticText;
    itemStaticText94->Create( itemWizardPage90, wxID_STATIC, _("The URL you supplied is not that of a BOINC-based project."), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer91->Add(itemStaticText94, 0, wxALIGN_LEFT|wxALL, 5);

    wxStaticText* itemStaticText95 = new wxStaticText;
    itemStaticText95->Create( itemWizardPage90, wxID_STATIC, _("Please check the URL and try again."), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer91->Add(itemStaticText95, 0, wxALIGN_LEFT|wxALL, 5);

////@end CErrProjectNotDetectedPage content construction
}
 
/*!
 * Gets the previous page.
 */
 
wxWizardPage* CErrProjectNotDetectedPage::GetPrev() const
{
    return PAGE_TRANSITION_BACK;
}
 
/*!
 * Gets the next page.
 */
 
wxWizardPage* CErrProjectNotDetectedPage::GetNext() const
{
    return NULL;
}
 
/*!
 * Should we show tooltips?
 */
 
bool CErrProjectNotDetectedPage::ShowToolTips()
{
    return TRUE;
}
 
/*!
 * Get bitmap resources
 */
 
wxBitmap CErrProjectNotDetectedPage::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CErrProjectNotDetectedPage bitmap retrieval
    return wxNullBitmap;
////@end CErrProjectNotDetectedPage bitmap retrieval
}
 
/*!
 * Get icon resources
 */
 
wxIcon CErrProjectNotDetectedPage::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CErrProjectNotDetectedPage icon retrieval
    return wxNullIcon;
////@end CErrProjectNotDetectedPage icon retrieval
}
  
/*!
 * wxEVT_WIZARD_PAGE_CHANGED event handler for ID_ERRPROJECTNOTDETECTEDPAGE
 */
 
void CErrProjectNotDetectedPage::OnPageChanged( wxWizardEvent& event ) {
    event.Skip();
}
  
/*!
 * wxEVT_WIZARD_CANCEL event handler for ID_ERRPROJECTNOTDETECTEDPAGE
 */
 
void CErrProjectNotDetectedPage::OnCancel( wxWizardEvent& event ) {
    ((CWizAttachProject*)GetParent())->ProcessCancelEvent(event);
}
  
/*!

 * CErrProjectUnavailablePage type definition

 */
 
IMPLEMENT_DYNAMIC_CLASS( CErrProjectUnavailablePage, wxWizardPage )
 
/*!

 * CErrProjectUnavailablePage event table definition

 */
 
BEGIN_EVENT_TABLE( CErrProjectUnavailablePage, wxWizardPage )
 
////@begin CErrProjectUnavailablePage event table entries
    EVT_WIZARD_PAGE_CHANGED( -1, CErrProjectUnavailablePage::OnPageChanged )
    EVT_WIZARD_CANCEL( -1, CErrProjectUnavailablePage::OnCancel )

////@end CErrProjectUnavailablePage event table entries
 
END_EVENT_TABLE()
  
/*!

 * CErrProjectUnavailablePage constructors

 */
 
CErrProjectUnavailablePage::CErrProjectUnavailablePage( )
{
}
  
CErrProjectUnavailablePage::CErrProjectUnavailablePage( wxWizard* parent )
{
    Create( parent );
}
 
/*!
 * CErrProjectUnavailablePage creator
 */
 
bool CErrProjectUnavailablePage::Create( wxWizard* parent )
{
////@begin CErrProjectUnavailablePage member initialisation
////@end CErrProjectUnavailablePage member initialisation
 
////@begin CErrProjectUnavailablePage creation
    wxBitmap wizardBitmap(wxNullBitmap);
    wxWizardPage::Create( parent, wizardBitmap );

    CreateControls();
    GetSizer()->Fit(this);
////@end CErrProjectUnavailablePage creation
 
    return TRUE;
}
  
/*!
 * Control creation for CErrProjectUnavailablePage
 */
 
void CErrProjectUnavailablePage::CreateControls()
{    
////@begin CErrProjectUnavailablePage content construction
    CErrProjectUnavailablePage* itemWizardPage96 = this;

    wxBoxSizer* itemBoxSizer97 = new wxBoxSizer(wxVERTICAL);
    itemWizardPage96->SetSizer(itemBoxSizer97);

    wxStaticText* itemStaticText98 = new wxStaticText;
    itemStaticText98->Create( itemWizardPage96, wxID_STATIC, _("Project temporarily unavailable"), wxDefaultPosition, wxDefaultSize, 0 );
    itemStaticText98->SetFont(wxFont(10, wxSWISS, wxNORMAL, wxBOLD, FALSE, _T("Verdana")));
    itemBoxSizer97->Add(itemStaticText98, 0, wxALIGN_LEFT|wxALL, 5);

    itemBoxSizer97->Add(5, 5, 0, wxALIGN_LEFT|wxALL, 5);

    wxStaticText* itemStaticText100 = new wxStaticText;
    itemStaticText100->Create( itemWizardPage96, wxID_STATIC, _("The project is currently unavailable.\n\nPlease try again later."), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer97->Add(itemStaticText100, 0, wxALIGN_LEFT|wxALL, 5);

////@end CErrProjectUnavailablePage content construction
}
  
/*!
 * Gets the previous page.
 */
 
wxWizardPage* CErrProjectUnavailablePage::GetPrev() const
{
    return PAGE_TRANSITION_BACK;
}
 
/*!
 * Gets the next page.
 */
 
wxWizardPage* CErrProjectUnavailablePage::GetNext() const
{
    return NULL;
}
 
/*!
 * Should we show tooltips?
 */

bool CErrProjectUnavailablePage::ShowToolTips()
{
    return TRUE;
}
  
/*!
 * Get bitmap resources
 */
 
wxBitmap CErrProjectUnavailablePage::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
 
////@begin CErrProjectUnavailablePage bitmap retrieval
    return wxNullBitmap;
////@end CErrProjectUnavailablePage bitmap retrieval
}
  
/*!
 * Get icon resources
 */

wxIcon CErrProjectUnavailablePage::GetIconResource( const wxString& name )
{
    // Icon retrieval

////@begin CErrProjectUnavailablePage icon retrieval
    return wxNullIcon;
////@end CErrProjectUnavailablePage icon retrieval
}
   
/*!
 * wxEVT_WIZARD_PAGE_CHANGED event handler for ID_ERRPROJECTUNAVAILABLEPAGE
 */

void CErrProjectUnavailablePage::OnPageChanged( wxWizardEvent& event ) {
    event.Skip();
}
 
/*!
 * wxEVT_WIZARD_CANCEL event handler for ID_ERRPROJECTUNAVAILABLEPAGE
 */

void CErrProjectUnavailablePage::OnCancel( wxWizardEvent& event ) {
    ((CWizAttachProject*)GetParent())->ProcessCancelEvent(event);
}
 
/*!
 * CErrProjectAlreadyAttachedPage type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CErrProjectAlreadyAttachedPage, wxWizardPage )

/*!
 * CErrProjectAlreadyAttachedPage event table definition
 */

BEGIN_EVENT_TABLE( CErrProjectAlreadyAttachedPage, wxWizardPage )

////@begin CErrProjectAlreadyAttachedPage event table entries
    EVT_WIZARD_PAGE_CHANGED( -1, CErrProjectAlreadyAttachedPage::OnPageChanged )
    EVT_WIZARD_CANCEL( -1, CErrProjectAlreadyAttachedPage::OnCancel )

////@end CErrProjectAlreadyAttachedPage event table entries

END_EVENT_TABLE()

/*!
 * CErrProjectAlreadyAttachedPage constructors
 */

CErrProjectAlreadyAttachedPage::CErrProjectAlreadyAttachedPage( )
{
}

CErrProjectAlreadyAttachedPage::CErrProjectAlreadyAttachedPage( wxWizard* parent )
{
    Create( parent );
}

/*!
 * WizardPage creator
 */

bool CErrProjectAlreadyAttachedPage::Create( wxWizard* parent )
{
////@begin CErrProjectAlreadyAttachedPage member initialisation
////@end CErrProjectAlreadyAttachedPage member initialisation

////@begin CErrProjectAlreadyAttachedPage creation
    wxBitmap wizardBitmap(wxNullBitmap);
    wxWizardPage::Create( parent, wizardBitmap );

    CreateControls();
    GetSizer()->Fit(this);
////@end CErrProjectAlreadyAttachedPage creation
    return TRUE;
}

/*!
 * Control creation for WizardPage
 */

void CErrProjectAlreadyAttachedPage::CreateControls()
{    
////@begin CErrProjectAlreadyAttachedPage content construction
    CErrProjectAlreadyAttachedPage* itemWizardPage101 = this;

    wxBoxSizer* itemBoxSizer102 = new wxBoxSizer(wxVERTICAL);
    itemWizardPage101->SetSizer(itemBoxSizer102);

    wxStaticText* itemStaticText103 = new wxStaticText;
    itemStaticText103->Create( itemWizardPage101, wxID_STATIC, _("Already attached to project"), wxDefaultPosition, wxDefaultSize, 0 );
    itemStaticText103->SetFont(wxFont(10, wxSWISS, wxNORMAL, wxBOLD, FALSE, _T("Verdana")));
    itemBoxSizer102->Add(itemStaticText103, 0, wxALIGN_LEFT|wxALL, 5);

    itemBoxSizer102->Add(5, 5, 0, wxALIGN_LEFT|wxALL, 5);

    wxStaticText* itemStaticText105 = new wxStaticText;
    itemStaticText105->Create( itemWizardPage101, wxID_STATIC, _("You are already attached to this project."), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer102->Add(itemStaticText105, 0, wxALIGN_LEFT|wxALL, 5);

////@end CErrProjectAlreadyAttachedPage content construction
}

/*!
 * Gets the previous page.
 */

wxWizardPage* CErrProjectAlreadyAttachedPage::GetPrev() const
{
    return PAGE_TRANSITION_BACK;
}

/*!
 * Gets the next page.
 */

wxWizardPage* CErrProjectAlreadyAttachedPage::GetNext() const
{
    return NULL;
}

/*!
 * Should we show tooltips?
 */

bool CErrProjectAlreadyAttachedPage::ShowToolTips()
{
    return TRUE;
}

/*!
 * Get bitmap resources
 */

wxBitmap CErrProjectAlreadyAttachedPage::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CErrProjectAlreadyAttachedPage bitmap retrieval
    return wxNullBitmap;
////@end CErrProjectAlreadyAttachedPage bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CErrProjectAlreadyAttachedPage::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CErrProjectAlreadyAttachedPage icon retrieval
    return wxNullIcon;
////@end CErrProjectAlreadyAttachedPage icon retrieval
}

/*!
 * wxEVT_WIZARD_PAGE_CHANGED event handler for ID_ERRPROJECTALREADYATTACHED
 */

void CErrProjectAlreadyAttachedPage::OnPageChanged( wxWizardEvent& event ) {
    event.Skip();
}

/*!
 * wxEVT_WIZARD_CANCEL event handler for ID_ERRPROJECTALREADYATTACHED
 */

void CErrProjectAlreadyAttachedPage::OnCancel( wxWizardEvent& event ) {
    ((CWizAttachProject*)GetParent())->ProcessCancelEvent(event);
}

/*!
 * CErrNoInternetConnectionPage type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CErrNoInternetConnectionPage, wxWizardPage )

/*!
 * CErrNoInternetConnectionPage event table definition
 */

BEGIN_EVENT_TABLE( CErrNoInternetConnectionPage, wxWizardPage )

////@begin CErrNoInternetConnectionPage event table entries
    EVT_WIZARD_PAGE_CHANGED( -1, CErrNoInternetConnectionPage::OnPageChanged )
    EVT_WIZARD_CANCEL( -1, CErrNoInternetConnectionPage::OnCancel )

////@end CErrNoInternetConnectionPage event table entries

END_EVENT_TABLE()

/*!
 * CErrNoInternetConnectionPage constructors
 */

CErrNoInternetConnectionPage::CErrNoInternetConnectionPage( )
{
}

CErrNoInternetConnectionPage::CErrNoInternetConnectionPage( wxWizard* parent )
{
    Create( parent );
}

/*!
 * CErrNoInternetConnectionPage creator
 */

bool CErrNoInternetConnectionPage::Create( wxWizard* parent )
{
////@begin CErrNoInternetConnectionPage member initialisation
////@end CErrNoInternetConnectionPage member initialisation

////@begin CErrNoInternetConnectionPage creation
    wxBitmap wizardBitmap(wxNullBitmap);
    wxWizardPage::Create( parent, wizardBitmap );

    CreateControls();
    GetSizer()->Fit(this);
////@end CErrNoInternetConnectionPage creation
    return TRUE;
}

/*!
 * Control creation for CErrNoInternetConnectionPage
 */

void CErrNoInternetConnectionPage::CreateControls()
{    
////@begin CErrNoInternetConnectionPage content construction
    CErrNoInternetConnectionPage* itemWizardPage106 = this;

    wxBoxSizer* itemBoxSizer107 = new wxBoxSizer(wxVERTICAL);
    itemWizardPage106->SetSizer(itemBoxSizer107);

    wxStaticText* itemStaticText108 = new wxStaticText;
    itemStaticText108->Create( itemWizardPage106, wxID_STATIC, _("No Internet connection"), wxDefaultPosition, wxDefaultSize, 0 );
    itemStaticText108->SetFont(wxFont(10, wxSWISS, wxNORMAL, wxBOLD, FALSE, _T("Verdana")));
    itemBoxSizer107->Add(itemStaticText108, 0, wxALIGN_LEFT|wxALL, 5);

    itemBoxSizer107->Add(5, 5, 0, wxALIGN_LEFT|wxALL, 5);

    wxStaticText* itemStaticText110 = new wxStaticText;
    itemStaticText110->Create( itemWizardPage106, wxID_STATIC, _("Please connect to the Internet and try again."), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer107->Add(itemStaticText110, 0, wxALIGN_LEFT|wxALL, 5);

////@end CErrNoInternetConnectionPage content construction
}

/*!
 * Gets the previous page.
 */

wxWizardPage* CErrNoInternetConnectionPage::GetPrev() const
{
    return PAGE_TRANSITION_BACK;
}

/*!
 * Gets the next page.
 */

wxWizardPage* CErrNoInternetConnectionPage::GetNext() const
{
    return NULL;
}

/*!
 * Should we show tooltips?
 */

bool CErrNoInternetConnectionPage::ShowToolTips()
{
    return TRUE;
}

/*!
 * Get bitmap resources
 */

wxBitmap CErrNoInternetConnectionPage::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CErrNoInternetConnectionPage bitmap retrieval
    return wxNullBitmap;
////@end CErrNoInternetConnectionPage bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CErrNoInternetConnectionPage::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CErrNoInternetConnectionPage icon retrieval
    return wxNullIcon;
////@end CErrNoInternetConnectionPage icon retrieval
}

/*!
 * wxEVT_WIZARD_PAGE_CHANGED event handler for ID_ERRNOINTERNETCONNECTIONPAGE
 */

void CErrNoInternetConnectionPage::OnPageChanged( wxWizardEvent& event ) {
    event.Skip();
}
  
/*!
 * wxEVT_WIZARD_CANCEL event handler for ID_ERRNOINTERNETCONNECTIONPAGE
 */
 
void CErrNoInternetConnectionPage::OnCancel( wxWizardEvent& event ) {
    ((CWizAttachProject*)GetParent())->ProcessCancelEvent(event);
}
 
/*!
 * CErrAccountNotFoundPage type definition
 */
 
IMPLEMENT_DYNAMIC_CLASS( CErrAccountNotFoundPage, wxWizardPage )
 
/*!
 * CErrAccountNotFoundPage event table definition
 */
 
BEGIN_EVENT_TABLE( CErrAccountNotFoundPage, wxWizardPage )

////@begin CErrAccountNotFoundPage event table entries
    EVT_WIZARD_PAGE_CHANGED( -1, CErrAccountNotFoundPage::OnPageChanged )
    EVT_WIZARD_CANCEL( -1, CErrAccountNotFoundPage::OnCancel )

////@end CErrAccountNotFoundPage event table entries
 
END_EVENT_TABLE()
 
/*!
 * CErrAccountNotFoundPage constructors
 */
 
CErrAccountNotFoundPage::CErrAccountNotFoundPage( )
{
}
 
CErrAccountNotFoundPage::CErrAccountNotFoundPage( wxWizard* parent )
{
    Create( parent );
}
 
/*!
 * CErrNoInternetConnection creator
 */
 
bool CErrAccountNotFoundPage::Create( wxWizard* parent )
{
////@begin CErrAccountNotFoundPage member initialisation
////@end CErrAccountNotFoundPage member initialisation
 
////@begin CErrAccountNotFoundPage creation
    wxBitmap wizardBitmap(wxNullBitmap);
    wxWizardPage::Create( parent, wizardBitmap );

    CreateControls();
    GetSizer()->Fit(this);
////@end CErrAccountNotFoundPage creation

    return TRUE;
}
 
/*!
 * Control creation for CErrNoInternetConnection
 */
 
void CErrAccountNotFoundPage::CreateControls()
{    
////@begin CErrAccountNotFoundPage content construction
    CErrAccountNotFoundPage* itemWizardPage111 = this;

    wxBoxSizer* itemBoxSizer112 = new wxBoxSizer(wxVERTICAL);
    itemWizardPage111->SetSizer(itemBoxSizer112);

    wxStaticText* itemStaticText113 = new wxStaticText;
    itemStaticText113->Create( itemWizardPage111, wxID_STATIC, _("Account not found"), wxDefaultPosition, wxDefaultSize, 0 );
    itemStaticText113->SetFont(wxFont(10, wxSWISS, wxNORMAL, wxBOLD, FALSE, _T("Verdana")));
    itemBoxSizer112->Add(itemStaticText113, 0, wxALIGN_LEFT|wxALL, 5);

    itemBoxSizer112->Add(5, 5, 0, wxALIGN_LEFT|wxALL, 5);

    wxStaticText* itemStaticText115 = new wxStaticText;
    itemStaticText115->Create( itemWizardPage111, wxID_STATIC, _("Verify your account name and password are correct and try again."), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer112->Add(itemStaticText115, 0, wxALIGN_LEFT|wxALL, 5);

////@end CErrAccountNotFoundPage content construction
}
 
/*!
 * Gets the previous page.
 */
 
wxWizardPage* CErrAccountNotFoundPage::GetPrev() const
{
    return PAGE_TRANSITION_BACK;
}
 
/*!
 * Gets the next page.
 */

wxWizardPage* CErrAccountNotFoundPage::GetNext() const
{
    return NULL;
}
 
/*!
 * Should we show tooltips?
 */
 
bool CErrAccountNotFoundPage::ShowToolTips()
{
    return TRUE;
}
 
/*!
 * Get bitmap resources
 */
 
wxBitmap CErrAccountNotFoundPage::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CErrAccountNotFoundPage bitmap retrieval
    return wxNullBitmap;
////@end CErrAccountNotFoundPage bitmap retrieval
}
 
/*!
 * Get icon resources
 */

wxIcon CErrAccountNotFoundPage::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CErrAccountNotFoundPage icon retrieval
    return wxNullIcon;
////@end CErrAccountNotFoundPage icon retrieval
}
  
/*!
 * wxEVT_WIZARD_PAGE_CHANGED event handler for ID_ERRNACCOUNTDOESNOTEXISTPAGE
 */

void CErrAccountNotFoundPage::OnPageChanged( wxWizardEvent& event ) {
    event.Skip();
}

/*!
 * wxEVT_WIZARD_CANCEL event handler for ID_ERRNACCOUNTDOESNOTEXISTPAGE
 */

void CErrAccountNotFoundPage::OnCancel( wxWizardEvent& event ) {
    ((CWizAttachProject*)GetParent())->ProcessCancelEvent(event);
}

/*!
 * CErrAccountAlreadyExistsPage type definition
 */
 
IMPLEMENT_DYNAMIC_CLASS( CErrAccountAlreadyExistsPage, wxWizardPage )
 
/*!
 * CErrAccountAlreadyExistsPage event table definition
 */
 
BEGIN_EVENT_TABLE( CErrAccountAlreadyExistsPage, wxWizardPage )
 
////@begin CErrAccountAlreadyExistsPage event table entries
    EVT_WIZARD_PAGE_CHANGED( -1, CErrAccountAlreadyExistsPage::OnPageChanged )
    EVT_WIZARD_CANCEL( -1, CErrAccountAlreadyExistsPage::OnCancel )

////@end CErrAccountAlreadyExistsPage event table entries
 
END_EVENT_TABLE()
 
/*!
 * CErrAccountAlreadyExistsPage constructors
 */
 
CErrAccountAlreadyExistsPage::CErrAccountAlreadyExistsPage( )
{
}
 
CErrAccountAlreadyExistsPage::CErrAccountAlreadyExistsPage( wxWizard* parent )
{
    Create( parent );
}
 
/*!
 * CErrAccountAlreadyExists creator
 */
 
bool CErrAccountAlreadyExistsPage::Create( wxWizard* parent )
{
////@begin CErrAccountAlreadyExistsPage member initialisation
////@end CErrAccountAlreadyExistsPage member initialisation
 
////@begin CErrAccountAlreadyExistsPage creation
    wxBitmap wizardBitmap(wxNullBitmap);
    wxWizardPage::Create( parent, wizardBitmap );

    CreateControls();
    GetSizer()->Fit(this);
////@end CErrAccountAlreadyExistsPage creation

    return TRUE;
}
 
/*!
 * Control creation for CErrAccountAlreadyExists
 */
 
void CErrAccountAlreadyExistsPage::CreateControls()
{    
////@begin CErrAccountAlreadyExistsPage content construction
    CErrAccountAlreadyExistsPage* itemWizardPage116 = this;

    wxBoxSizer* itemBoxSizer117 = new wxBoxSizer(wxVERTICAL);
    itemWizardPage116->SetSizer(itemBoxSizer117);

    wxStaticText* itemStaticText118 = new wxStaticText;
    itemStaticText118->Create( itemWizardPage116, wxID_STATIC, _("Email address already in use"), wxDefaultPosition, wxDefaultSize, 0 );
    itemStaticText118->SetFont(wxFont(10, wxSWISS, wxNORMAL, wxBOLD, FALSE, _T("Verdana")));
    itemBoxSizer117->Add(itemStaticText118, 0, wxALIGN_LEFT|wxALL, 5);

    itemBoxSizer117->Add(5, 5, 0, wxALIGN_LEFT|wxALL, 5);

    wxStaticText* itemStaticText120 = new wxStaticText;
    itemStaticText120->Create( itemWizardPage116, wxID_STATIC, _("An account with that email address already exists and has a\ndifferent password than the one you entered.\n\nPlease visit the project's web site and follow the instructions there."), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer117->Add(itemStaticText120, 0, wxALIGN_LEFT|wxALL, 5);

////@end CErrAccountAlreadyExistsPage content construction
}
 
/*!
 * Gets the previous page.
 */
 
wxWizardPage* CErrAccountAlreadyExistsPage::GetPrev() const
{
    return PAGE_TRANSITION_BACK;
}
 
/*!
 * Gets the next page.
 */
 
wxWizardPage* CErrAccountAlreadyExistsPage::GetNext() const
{
    return NULL;
}
 
/*!
 * Should we show tooltips?
 */
 
bool CErrAccountAlreadyExistsPage::ShowToolTips()
{
    return TRUE;
}
 
/*!
 * Get bitmap resources
 */
 
wxBitmap CErrAccountAlreadyExistsPage::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CErrAccountAlreadyExistsPage bitmap retrieval
    return wxNullBitmap;
////@end CErrAccountAlreadyExistsPage bitmap retrieval
}
 
/*!
 * Get icon resources
 */
 
wxIcon CErrAccountAlreadyExistsPage::GetIconResource( const wxString& name )
{
    // Icon retrieval

////@begin CErrAccountAlreadyExistsPage icon retrieval
    return wxNullIcon;
////@end CErrAccountAlreadyExistsPage icon retrieval
}
  
/*!
 * wxEVT_WIZARD_PAGE_CHANGED event handler for ID_ERRACCOUNTALREADYEXISTSPAGE
 */
 
void CErrAccountAlreadyExistsPage::OnPageChanged( wxWizardEvent& event ) {
    event.Skip();
}
  
/*!
 * wxEVT_WIZARD_CANCEL event handler for ID_ERRACCOUNTALREADYEXISTSPAGE
 */
 
void CErrAccountAlreadyExistsPage::OnCancel( wxWizardEvent& event ) {
    ((CWizAttachProject*)GetParent())->ProcessCancelEvent(event);
}
  
/*!
 * CErrProxyPage type definition
 */
 
IMPLEMENT_DYNAMIC_CLASS( CErrProxyPage, wxWizardPage )
 
/*!
 * CErrProxyPage event table definition
 */
 
BEGIN_EVENT_TABLE( CErrProxyPage, wxWizardPage )
 
////@begin CErrProxyPage event table entries
    EVT_WIZARD_PAGE_CHANGED( -1, CErrProxyPage::OnPageChanged )
    EVT_WIZARD_PAGE_CHANGING( -1, CErrProxyPage::OnPageChanging )
    EVT_WIZARD_CANCEL( -1, CErrProxyPage::OnCancel )

////@end CErrProxyPage event table entries
 
END_EVENT_TABLE()
 
/*!
 * CErrProxyPage constructors
 */
 
CErrProxyPage::CErrProxyPage( )
{
}
 
CErrProxyPage::CErrProxyPage( wxWizard* parent )
{
    Create( parent );
}
 
/*!
 * CErrProxyComplationPage creator
 */
 
bool CErrProxyPage::Create( wxWizard* parent )
{
////@begin CErrProxyPage member initialisation
    m_ProxyHTTPServerStaticCtrl = NULL;
    m_ProxyHTTPServerCtrl = NULL;
    m_ProxyHTTPPortStaticCtrl = NULL;
    m_ProxyHTTPPortCtrl = NULL;
    m_ProxyHTTPUsernameStaticCtrl = NULL;
    m_ProxyHTTPUsernameCtrl = NULL;
    m_ProxyHTTPPasswordStaticCtrl = NULL;
    m_ProxyHTTPPasswordCtrl = NULL;
    m_ProxyHTTPAutodetectCtrl = NULL;
    m_ProxySOCKSServerStaticCtrl = NULL;
    m_ProxySOCKSServerCtrl = NULL;
    m_ProxySOCKSPortStaticCtrl = NULL;
    m_ProxySOCKSPortCtrl = NULL;
    m_ProxySOCKSUsernameStaticCtrl = NULL;
    m_ProxySOCKSUsernameCtrl = NULL;
    m_ProxySOCKSPasswordStaticCtrl = NULL;
    m_ProxySOCKSPasswordCtrl = NULL;
////@end CErrProxyPage member initialisation
 
////@begin CErrProxyPage creation
    wxBitmap wizardBitmap(wxNullBitmap);
    wxWizardPage::Create( parent, wizardBitmap );

    CreateControls();
    GetSizer()->Fit(this);
////@end CErrProxyPage creation
 
    return TRUE;
}
 
/*!
 * Control creation for CErrProxyComplationPage
 */
 
void CErrProxyPage::CreateControls()
{    
////@begin CErrProxyPage content construction
    CErrProxyPage* itemWizardPage121 = this;

    wxBoxSizer* itemBoxSizer122 = new wxBoxSizer(wxVERTICAL);
    itemWizardPage121->SetSizer(itemBoxSizer122);

    wxStaticText* itemStaticText123 = new wxStaticText;
    itemStaticText123->Create( itemWizardPage121, wxID_STATIC, _("Proxy configuration"), wxDefaultPosition, wxDefaultSize, 0 );
    itemStaticText123->SetFont(wxFont(10, wxSWISS, wxNORMAL, wxBOLD, FALSE, _T("Verdana")));
    itemBoxSizer122->Add(itemStaticText123, 0, wxALIGN_LEFT|wxALL, 5);

    itemBoxSizer122->Add(5, 5, 0, wxALIGN_LEFT|wxALL, 5);

    wxStaticBox* itemStaticBoxSizer125Static = new wxStaticBox(itemWizardPage121, wxID_ANY, _("HTTP proxy"));
    wxStaticBoxSizer* itemStaticBoxSizer125 = new wxStaticBoxSizer(itemStaticBoxSizer125Static, wxVERTICAL);
    itemBoxSizer122->Add(itemStaticBoxSizer125, 0, wxGROW|wxALL, 0);

    wxFlexGridSizer* itemFlexGridSizer126 = new wxFlexGridSizer(3, 2, 0, 0);
    itemFlexGridSizer126->AddGrowableCol(1);
    itemStaticBoxSizer125->Add(itemFlexGridSizer126, 0, wxGROW|wxALL, 2);

    m_ProxyHTTPServerStaticCtrl = new wxStaticText;
    m_ProxyHTTPServerStaticCtrl->Create( itemWizardPage121, ID_PROXYHTTPSERVERSTATICCTRL, _("Server:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer126->Add(m_ProxyHTTPServerStaticCtrl, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 2);

    wxFlexGridSizer* itemFlexGridSizer128 = new wxFlexGridSizer(1, 3, 0, 0);
    itemFlexGridSizer128->AddGrowableCol(0);
    itemFlexGridSizer126->Add(itemFlexGridSizer128, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 0);

    m_ProxyHTTPServerCtrl = new wxTextCtrl;
    m_ProxyHTTPServerCtrl->Create( itemWizardPage121, ID_PROXYHTTPSERVERCTRL, _T(""), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer128->Add(m_ProxyHTTPServerCtrl, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 2);

    m_ProxyHTTPPortStaticCtrl = new wxStaticText;
    m_ProxyHTTPPortStaticCtrl->Create( itemWizardPage121, ID_PROXYHTTPPORTSTATICCTRL, _("Port:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer128->Add(m_ProxyHTTPPortStaticCtrl, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 2);

    m_ProxyHTTPPortCtrl = new wxTextCtrl;
    m_ProxyHTTPPortCtrl->Create( itemWizardPage121, ID_PROXYHTTPPORTCTRL, _T(""), wxDefaultPosition, wxSize(50, -1), 0 );
    itemFlexGridSizer128->Add(m_ProxyHTTPPortCtrl, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 2);

    m_ProxyHTTPUsernameStaticCtrl = new wxStaticText;
    m_ProxyHTTPUsernameStaticCtrl->Create( itemWizardPage121, ID_PROXYHTTPUSERNAMESTATICCTRL, _("User Name:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer126->Add(m_ProxyHTTPUsernameStaticCtrl, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 2);

    m_ProxyHTTPUsernameCtrl = new wxTextCtrl;
    m_ProxyHTTPUsernameCtrl->Create( itemWizardPage121, ID_PROXYHTTPUSERNAMECTRL, _T(""), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer126->Add(m_ProxyHTTPUsernameCtrl, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 2);

    m_ProxyHTTPPasswordStaticCtrl = new wxStaticText;
    m_ProxyHTTPPasswordStaticCtrl->Create( itemWizardPage121, ID_PROXYHTTPPASSWORDSTATICCTRL, _("Password:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer126->Add(m_ProxyHTTPPasswordStaticCtrl, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 2);

    m_ProxyHTTPPasswordCtrl = new wxTextCtrl;
    m_ProxyHTTPPasswordCtrl->Create( itemWizardPage121, ID_PROXYHTTPPASSWORDCTRL, _T(""), wxDefaultPosition, wxDefaultSize, wxTE_PASSWORD );
    itemFlexGridSizer126->Add(m_ProxyHTTPPasswordCtrl, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 2);

    m_ProxyHTTPAutodetectCtrl = new wxButton;
    m_ProxyHTTPAutodetectCtrl->Create( itemWizardPage121, ID_PROXYHTTPAUTODETECTCTRL, _("Autodetect"), wxDefaultPosition, wxDefaultSize, 0 );
    itemStaticBoxSizer125->Add(m_ProxyHTTPAutodetectCtrl, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 2);

    wxStaticBox* itemStaticBoxSizer137Static = new wxStaticBox(itemWizardPage121, wxID_ANY, _("SOCKS proxy"));
    wxStaticBoxSizer* itemStaticBoxSizer137 = new wxStaticBoxSizer(itemStaticBoxSizer137Static, wxVERTICAL);
    itemBoxSizer122->Add(itemStaticBoxSizer137, 0, wxGROW|wxALL, 0);

    wxFlexGridSizer* itemFlexGridSizer138 = new wxFlexGridSizer(3, 2, 0, 0);
    itemFlexGridSizer138->AddGrowableCol(1);
    itemStaticBoxSizer137->Add(itemFlexGridSizer138, 0, wxGROW|wxALL, 2);

    m_ProxySOCKSServerStaticCtrl = new wxStaticText;
    m_ProxySOCKSServerStaticCtrl->Create( itemWizardPage121, ID_PROXYSOCKSSERVERSTATICCTRL, _("Server:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer138->Add(m_ProxySOCKSServerStaticCtrl, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 2);

    wxFlexGridSizer* itemFlexGridSizer140 = new wxFlexGridSizer(1, 3, 0, 0);
    itemFlexGridSizer140->AddGrowableCol(0);
    itemFlexGridSizer138->Add(itemFlexGridSizer140, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 0);

    m_ProxySOCKSServerCtrl = new wxTextCtrl;
    m_ProxySOCKSServerCtrl->Create( itemWizardPage121, ID_PROXYSOCKSSERVERCTRL, _T(""), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer140->Add(m_ProxySOCKSServerCtrl, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 2);

    m_ProxySOCKSPortStaticCtrl = new wxStaticText;
    m_ProxySOCKSPortStaticCtrl->Create( itemWizardPage121, ID_PROXYSOCKSPORTSTATICCTRL, _("Port:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer140->Add(m_ProxySOCKSPortStaticCtrl, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 2);

    m_ProxySOCKSPortCtrl = new wxTextCtrl;
    m_ProxySOCKSPortCtrl->Create( itemWizardPage121, ID_PROXYSOCKSPORTCTRL, _T(""), wxDefaultPosition, wxSize(50, -1), 0 );
    itemFlexGridSizer140->Add(m_ProxySOCKSPortCtrl, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 2);

    m_ProxySOCKSUsernameStaticCtrl = new wxStaticText;
    m_ProxySOCKSUsernameStaticCtrl->Create( itemWizardPage121, ID_PROXYSOCKSUSERNAMESTATICCTRL, _("User Name:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer138->Add(m_ProxySOCKSUsernameStaticCtrl, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 2);

    m_ProxySOCKSUsernameCtrl = new wxTextCtrl;
    m_ProxySOCKSUsernameCtrl->Create( itemWizardPage121, ID_PROXYSOCKSUSERNAMECTRL, _T(""), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer138->Add(m_ProxySOCKSUsernameCtrl, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 2);

    m_ProxySOCKSPasswordStaticCtrl = new wxStaticText;
    m_ProxySOCKSPasswordStaticCtrl->Create( itemWizardPage121, ID_PROXYSOCKSPASSWORDSTATICCTRL, _("Password:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer138->Add(m_ProxySOCKSPasswordStaticCtrl, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 2);

    m_ProxySOCKSPasswordCtrl = new wxTextCtrl;
    m_ProxySOCKSPasswordCtrl->Create( itemWizardPage121, ID_PROXYSOCKSPASSWORDCTRL, _T(""), wxDefaultPosition, wxDefaultSize, wxTE_PASSWORD );
    itemFlexGridSizer138->Add(m_ProxySOCKSPasswordCtrl, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 2);

    // Set validators
    m_ProxyHTTPServerCtrl->SetValidator( wxTextValidator(wxFILTER_NONE, & m_strProxyHTTPServer) );
    m_ProxyHTTPPortCtrl->SetValidator( wxTextValidator(wxFILTER_NUMERIC, & m_strProxyHTTPPort) );
    m_ProxyHTTPUsernameCtrl->SetValidator( wxTextValidator(wxFILTER_NONE, & m_strProxyHTTPUsername) );
    m_ProxyHTTPPasswordCtrl->SetValidator( wxTextValidator(wxFILTER_NONE, & m_strProxyHTTPPassword) );
    m_ProxySOCKSServerCtrl->SetValidator( wxTextValidator(wxFILTER_NONE, & m_strProxySOCKSServer) );
    m_ProxySOCKSPortCtrl->SetValidator( wxTextValidator(wxFILTER_NUMERIC, & m_strProxySOCKSPort) );
    m_ProxySOCKSUsernameCtrl->SetValidator( wxTextValidator(wxFILTER_NONE, & m_strProxySOCKSUsername) );
    m_ProxySOCKSPasswordCtrl->SetValidator( wxTextValidator(wxFILTER_NONE, & m_strProxySOCKSPassword) );
////@end CErrProxyPage content construction
}
  
/*!
 * Gets the previous page.
 */
 
wxWizardPage* CErrProxyPage::GetPrev() const
{
    return PAGE_TRANSITION_BACK;
}
  
/*!
 * Gets the next page.
 */
 
wxWizardPage* CErrProxyPage::GetNext() const
{
    if (((CWizAttachProject*)GetParent())->IsCancelInProgress()) {
        // Cancel Event Detected
        return PAGE_TRANSITION_NEXT(ID_COMPLETIONERRORPAGE);
    } else {
        return PAGE_TRANSITION_NEXT(ID_PROJECTPROPERTIESPAGE);
    }
    return NULL;
}
 
/*!
 * Should we show tooltips?
 */
 
bool CErrProxyPage::ShowToolTips()
{
    return TRUE;
}
 
/*!
 * Get bitmap resources
 */
 
wxBitmap CErrProxyPage::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CErrProxyPage bitmap retrieval
    return wxNullBitmap;
////@end CErrProxyPage bitmap retrieval
}
  
/*!
 * Get icon resources
 */
 
wxIcon CErrProxyPage::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CErrProxyPage icon retrieval
    return wxNullIcon;
////@end CErrProxyPage icon retrieval
}
 
/*!
 * wxEVT_WIZARD_PAGE_CHANGED event handler for ID_ERRPROXYCOMPLETIONPAGE
 */
 
void CErrProxyPage::OnPageChanged( wxWizardEvent& event ) {
    CMainDocument* pDoc = wxGetApp().GetDocument();
    wxString       strBuffer = wxEmptyString;

    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));

    if (event.GetDirection() == true) {
        // Moving from the previous page, get state
        pDoc->GetProxyConfiguration();
        m_ProxyHTTPServerCtrl->SetValue(pDoc->proxy_info.http_server_name.c_str());
        m_ProxyHTTPUsernameCtrl->SetValue(pDoc->proxy_info.http_user_name.c_str());
        m_ProxyHTTPPasswordCtrl->SetValue(pDoc->proxy_info.http_user_passwd.c_str());

        strBuffer.Printf(wxT("%d"), pDoc->proxy_info.http_server_port);
        m_ProxyHTTPPortCtrl->SetValue(strBuffer);

        m_ProxySOCKSServerCtrl->SetValue(pDoc->proxy_info.socks_server_name.c_str());
        m_ProxySOCKSUsernameCtrl->SetValue(pDoc->proxy_info.socks5_user_name.c_str());
        m_ProxySOCKSPasswordCtrl->SetValue(pDoc->proxy_info.socks5_user_passwd.c_str());

        strBuffer.Printf(wxT("%d"), pDoc->proxy_info.socks_server_port);
        m_ProxySOCKSPortCtrl->SetValue(strBuffer);
    }
}
 
/*!
 * wxEVT_WIZARD_PAGE_CHANGING event handler for ID_ERRPROXYPAGE
 */

void CErrProxyPage::OnPageChanging( wxWizardEvent& event ) {
    CMainDocument* pDoc = wxGetApp().GetDocument();
    wxString       strBuffer = wxEmptyString;
    int            iBuffer = 0;

    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));

    if (event.GetDirection() == true) {
        // Moving to the next page, save state
        pDoc->proxy_info.use_http_proxy = (m_ProxyHTTPServerCtrl->GetValue().Length() > 0);
        pDoc->proxy_info.http_server_name = m_ProxyHTTPServerCtrl->GetValue().c_str();
        pDoc->proxy_info.http_user_name = m_ProxyHTTPUsernameCtrl->GetValue().c_str();
        pDoc->proxy_info.http_user_passwd = m_ProxyHTTPPasswordCtrl->GetValue().c_str();

        strBuffer = m_ProxyHTTPPortCtrl->GetValue();
        strBuffer.ToLong((long*)&iBuffer);
        pDoc->proxy_info.http_server_port = iBuffer;

        pDoc->proxy_info.use_socks_proxy = (m_ProxySOCKSServerCtrl->GetValue().Length() > 0);
        pDoc->proxy_info.socks_server_name = m_ProxySOCKSServerCtrl->GetValue().c_str();
        pDoc->proxy_info.socks5_user_name = m_ProxySOCKSUsernameCtrl->GetValue().c_str();
        pDoc->proxy_info.socks5_user_passwd = m_ProxySOCKSPasswordCtrl->GetValue().c_str();

        strBuffer = m_ProxySOCKSPortCtrl->GetValue();
        strBuffer.ToLong((long*)&iBuffer);
        pDoc->proxy_info.socks_server_port = iBuffer;

        pDoc->SetProxyConfiguration();
    }
}

/*!
 * wxEVT_WIZARD_CANCEL event handler for ID_ERRPROXYCOMPLETIONPAGE
 */
 
void CErrProxyPage::OnCancel( wxWizardEvent& event ) {
    ((CWizAttachProject*)GetParent())->ProcessCancelEvent(event);
}
 
/*!
 * CErrRefCountPage type definition
 */
 
IMPLEMENT_DYNAMIC_CLASS( CErrRefCountPage, wxWizardPage )
 
/*!
 * CErrRefCountPage event table definition
 */
 
BEGIN_EVENT_TABLE( CErrRefCountPage, wxWizardPage )
 
////@begin CErrRefCountPage event table entries
////@end CErrRefCountPage event table entries
 
END_EVENT_TABLE()
  
/*!
 * CErrRefCountPage constructors
 */
 
CErrRefCountPage::CErrRefCountPage( )
{
}
 
CErrRefCountPage::CErrRefCountPage( wxWizard* parent )
{
    Create( parent );
}
 
/*!
 * CErrRefCountPage creator
 */
 
bool CErrRefCountPage::Create( wxWizard* parent )
{
////@begin CErrRefCountPage member initialisation
////@end CErrRefCountPage member initialisation
  
////@begin CErrRefCountPage creation
    wxBitmap wizardBitmap(wxNullBitmap);
    wxWizardPage::Create( parent, wizardBitmap );

    CreateControls();
    GetSizer()->Fit(this);
////@end CErrRefCountPage creation
 
    return TRUE;
}
  
/*!
 * Control creation for CErrRefCountPage
 */
 
void CErrRefCountPage::CreateControls()
{    

////@begin CErrRefCountPage content construction
    CErrRefCountPage* itemWizardPage148 = this;

    wxBoxSizer* itemBoxSizer149 = new wxBoxSizer(wxVERTICAL);
    itemWizardPage148->SetSizer(itemBoxSizer149);

    wxStaticText* itemStaticText150 = new wxStaticText;
    itemStaticText150->Create( itemWizardPage148, wxID_STATIC, _("Ref Count Page"), wxDefaultPosition, wxDefaultSize, 0 );
    itemStaticText150->SetFont(wxFont(10, wxSWISS, wxNORMAL, wxBOLD, FALSE, _T("Verdana")));
    itemBoxSizer149->Add(itemStaticText150, 0, wxALIGN_LEFT|wxALL, 5);

    wxStaticText* itemStaticText151 = new wxStaticText;
    itemStaticText151->Create( itemWizardPage148, wxID_STATIC, _("This page should never be used in the wizard itself."), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer149->Add(itemStaticText151, 0, wxALIGN_LEFT|wxALL, 5);

    itemBoxSizer149->Add(5, 5, 0, wxALIGN_LEFT|wxALL, 5);

    wxStaticText* itemStaticText153 = new wxStaticText;
    itemStaticText153->Create( itemWizardPage148, wxID_STATIC, _("This page just increases the refcount of various bitmap resources\nso that DialogBlocks doesn't nuke the refences to them."), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer149->Add(itemStaticText153, 0, wxALIGN_LEFT|wxALL, 5);

    wxBoxSizer* itemBoxSizer154 = new wxBoxSizer(wxVERTICAL);
    itemBoxSizer149->Add(itemBoxSizer154, 0, wxALIGN_LEFT|wxALL, 5);

    wxBitmap itemStaticBitmap155Bitmap(itemWizardPage148->GetBitmapResource(wxT("res/wizprogress01.xpm")));
    wxStaticBitmap* itemStaticBitmap155 = new wxStaticBitmap;
    itemStaticBitmap155->Create( itemWizardPage148, wxID_STATIC, itemStaticBitmap155Bitmap, wxDefaultPosition, wxSize(184, 48), 0 );
    itemStaticBitmap155->Show(FALSE);
    itemBoxSizer154->Add(itemStaticBitmap155, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxBitmap itemStaticBitmap156Bitmap(itemWizardPage148->GetBitmapResource(wxT("res/wizprogress02.xpm")));
    wxStaticBitmap* itemStaticBitmap156 = new wxStaticBitmap;
    itemStaticBitmap156->Create( itemWizardPage148, wxID_STATIC, itemStaticBitmap156Bitmap, wxDefaultPosition, wxSize(184, 48), 0 );
    itemStaticBitmap156->Show(FALSE);
    itemBoxSizer154->Add(itemStaticBitmap156, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxBitmap itemStaticBitmap157Bitmap(itemWizardPage148->GetBitmapResource(wxT("res/wizprogress03.xpm")));
    wxStaticBitmap* itemStaticBitmap157 = new wxStaticBitmap;
    itemStaticBitmap157->Create( itemWizardPage148, wxID_STATIC, itemStaticBitmap157Bitmap, wxDefaultPosition, wxSize(184, 48), 0 );
    itemStaticBitmap157->Show(FALSE);
    itemBoxSizer154->Add(itemStaticBitmap157, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxBitmap itemStaticBitmap158Bitmap(itemWizardPage148->GetBitmapResource(wxT("res/wizprogress04.xpm")));
    wxStaticBitmap* itemStaticBitmap158 = new wxStaticBitmap;
    itemStaticBitmap158->Create( itemWizardPage148, wxID_STATIC, itemStaticBitmap158Bitmap, wxDefaultPosition, wxSize(184, 48), 0 );
    itemStaticBitmap158->Show(FALSE);
    itemBoxSizer154->Add(itemStaticBitmap158, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxBitmap itemStaticBitmap159Bitmap(itemWizardPage148->GetBitmapResource(wxT("res/wizprogress05.xpm")));
    wxStaticBitmap* itemStaticBitmap159 = new wxStaticBitmap;
    itemStaticBitmap159->Create( itemWizardPage148, wxID_STATIC, itemStaticBitmap159Bitmap, wxDefaultPosition, wxSize(184, 48), 0 );
    itemStaticBitmap159->Show(FALSE);
    itemBoxSizer154->Add(itemStaticBitmap159, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxBitmap itemStaticBitmap160Bitmap(itemWizardPage148->GetBitmapResource(wxT("res/wizprogress06.xpm")));
    wxStaticBitmap* itemStaticBitmap160 = new wxStaticBitmap;
    itemStaticBitmap160->Create( itemWizardPage148, wxID_STATIC, itemStaticBitmap160Bitmap, wxDefaultPosition, wxSize(184, 48), 0 );
    itemStaticBitmap160->Show(FALSE);
    itemBoxSizer154->Add(itemStaticBitmap160, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxBitmap itemStaticBitmap161Bitmap(itemWizardPage148->GetBitmapResource(wxT("res/wizprogress07.xpm")));
    wxStaticBitmap* itemStaticBitmap161 = new wxStaticBitmap;
    itemStaticBitmap161->Create( itemWizardPage148, wxID_STATIC, itemStaticBitmap161Bitmap, wxDefaultPosition, wxSize(184, 48), 0 );
    itemStaticBitmap161->Show(FALSE);
    itemBoxSizer154->Add(itemStaticBitmap161, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxBitmap itemStaticBitmap162Bitmap(itemWizardPage148->GetBitmapResource(wxT("res/wizprogress08.xpm")));
    wxStaticBitmap* itemStaticBitmap162 = new wxStaticBitmap;
    itemStaticBitmap162->Create( itemWizardPage148, wxID_STATIC, itemStaticBitmap162Bitmap, wxDefaultPosition, wxSize(184, 48), 0 );
    itemStaticBitmap162->Show(FALSE);
    itemBoxSizer154->Add(itemStaticBitmap162, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxBitmap itemStaticBitmap163Bitmap(itemWizardPage148->GetBitmapResource(wxT("res/wizprogress09.xpm")));
    wxStaticBitmap* itemStaticBitmap163 = new wxStaticBitmap;
    itemStaticBitmap163->Create( itemWizardPage148, wxID_STATIC, itemStaticBitmap163Bitmap, wxDefaultPosition, wxSize(184, 48), 0 );
    itemStaticBitmap163->Show(FALSE);
    itemBoxSizer154->Add(itemStaticBitmap163, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxBitmap itemStaticBitmap164Bitmap(itemWizardPage148->GetBitmapResource(wxT("res/wizprogress10.xpm")));
    wxStaticBitmap* itemStaticBitmap164 = new wxStaticBitmap;
    itemStaticBitmap164->Create( itemWizardPage148, wxID_STATIC, itemStaticBitmap164Bitmap, wxDefaultPosition, wxSize(184, 48), 0 );
    itemStaticBitmap164->Show(FALSE);
    itemBoxSizer154->Add(itemStaticBitmap164, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxBitmap itemStaticBitmap165Bitmap(itemWizardPage148->GetBitmapResource(wxT("res/wizprogress11.xpm")));
    wxStaticBitmap* itemStaticBitmap165 = new wxStaticBitmap;
    itemStaticBitmap165->Create( itemWizardPage148, wxID_STATIC, itemStaticBitmap165Bitmap, wxDefaultPosition, wxSize(184, 48), 0 );
    itemStaticBitmap165->Show(FALSE);
    itemBoxSizer154->Add(itemStaticBitmap165, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxBitmap itemStaticBitmap166Bitmap(itemWizardPage148->GetBitmapResource(wxT("res/wizprogress12.xpm")));
    wxStaticBitmap* itemStaticBitmap166 = new wxStaticBitmap;
    itemStaticBitmap166->Create( itemWizardPage148, wxID_STATIC, itemStaticBitmap166Bitmap, wxDefaultPosition, wxSize(184, 48), 0 );
    itemStaticBitmap166->Show(FALSE);
    itemBoxSizer154->Add(itemStaticBitmap166, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

////@end CErrRefCountPage content construction
}
  
/*!
 * Gets the previous page.
 */
 
wxWizardPage* CErrRefCountPage::GetPrev() const
{
    return NULL;
}
 
/*!
 * Gets the next page.
 */
 
wxWizardPage* CErrRefCountPage::GetNext() const
{
    return NULL;
}
 
/*!
 * Should we show tooltips?
 */
 
bool CErrRefCountPage::ShowToolTips()
{
    return TRUE;
}
 
/*!
 * Get bitmap resources
 */
 
wxBitmap CErrRefCountPage::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
 
////@begin CErrRefCountPage bitmap retrieval
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
////@end CErrRefCountPage bitmap retrieval

}
  
/*!
 * Get icon resources
 */
wxIcon CErrRefCountPage::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CErrRefCountPage icon retrieval
    return wxNullIcon;
////@end CErrRefCountPage icon retrieval
}

const char *BOINC_RCSID_03902eb5fb="$Id$";
