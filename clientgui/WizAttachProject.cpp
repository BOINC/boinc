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
#include "res/wizquestion.xpm"
#include "res/wizfailure.xpm"
#include "res/wizsuccess.xpm"
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
    EVT_WIZARD_CANCEL( ID_ATTACHPROJECTWIZARD, CWizAttachProject::OnWizardCancel )
    EVT_WIZARD_FINISHED( ID_ATTACHPROJECTWIZARD, CWizAttachProject::OnWizardFinished )

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
    m_AccountCreationPage = NULL;
    m_CompletionPage = NULL;
    m_ErrProjectNotDetectedPage = NULL;
    m_ErrProjectUnavailablePage = NULL;
    m_ErrNoInternetConnectionPage = NULL;
    m_ErrAccountAlreadyExistsPage = NULL;
    m_ErrAccountCreationDisabledPage = NULL;
    m_ErrProxyInfoPage = NULL;
    m_ErrProxyHTTPPage = NULL;
    m_ErrProxySOCKSPage = NULL;
    m_ErrProxyCompletionPage = NULL;
    m_ErrRefCountPage = NULL;
////@end CWizAttachProject member initialisation

    // Button pointer cache
    m_pbtnBack = NULL;
    m_pbtnNext = NULL;

    // Wizard support
    m_ulDiagFlags = 0;
    
    // Global wizard status
    project_config.clear();
    proxy_info.clear();
    account_in.clear();
    account_out.clear();

////@begin CWizAttachProject creation
    wxBitmap wizardBitmap(GetBitmapResource(wxT("res/attachprojectwizard.xpm")));
    wxWizard::Create( parent, id, _("Attach to Project Wizard"), wizardBitmap, pos );

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
    m_AccountCreationPage = new CAccountCreationPage;
    m_AccountCreationPage->Create( itemWizard1 );

    itemWizard1->FitToPage(m_AccountCreationPage);
    m_CompletionPage = new CCompletionPage;
    m_CompletionPage->Create( itemWizard1 );

    itemWizard1->FitToPage(m_CompletionPage);
    m_ErrProjectNotDetectedPage = new CErrProjectNotDetectedPage;
    m_ErrProjectNotDetectedPage->Create( itemWizard1 );

    itemWizard1->FitToPage(m_ErrProjectNotDetectedPage);
    m_ErrProjectUnavailablePage = new CErrProjectUnavailablePage;
    m_ErrProjectUnavailablePage->Create( itemWizard1 );

    itemWizard1->FitToPage(m_ErrProjectUnavailablePage);
    m_ErrNoInternetConnectionPage = new CErrNoInternetConnectionPage;
    m_ErrNoInternetConnectionPage->Create( itemWizard1 );

    itemWizard1->FitToPage(m_ErrNoInternetConnectionPage);
    m_ErrAccountAlreadyExistsPage = new CErrAccountAlreadyExistsPage;
    m_ErrAccountAlreadyExistsPage->Create( itemWizard1 );

    itemWizard1->FitToPage(m_ErrAccountAlreadyExistsPage);
    m_ErrAccountCreationDisabledPage = new CErrAccountCreationDisabledPage;
    m_ErrAccountCreationDisabledPage->Create( itemWizard1 );

    itemWizard1->FitToPage(m_ErrAccountCreationDisabledPage);
    m_ErrProxyInfoPage = new CErrProxyInfoPage;
    m_ErrProxyInfoPage->Create( itemWizard1 );

    itemWizard1->FitToPage(m_ErrProxyInfoPage);
    m_ErrProxyHTTPPage = new CErrProxyHTTPPage;
    m_ErrProxyHTTPPage->Create( itemWizard1 );

    itemWizard1->FitToPage(m_ErrProxyHTTPPage);
    m_ErrProxySOCKSPage = new CErrProxySOCKSPage;
    m_ErrProxySOCKSPage->Create( itemWizard1 );

    itemWizard1->FitToPage(m_ErrProxySOCKSPage);
    m_ErrProxyCompletionPage = new CErrProxyComplationPage;
    m_ErrProxyCompletionPage->Create( itemWizard1 );

    itemWizard1->FitToPage(m_ErrProxyCompletionPage);
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
    wxLogTrace(wxT("Function Status"), wxT("CWizAttachProject::CreateControls -     m_AccountCreationPage = id: '%d', location: '%p'"), ID_ACCOUNTCREATIONPAGE, m_AccountCreationPage);
    wxLogTrace(wxT("Function Status"), wxT("CWizAttachProject::CreateControls -     m_CompletionPage = id: '%d', location: '%p'"), ID_COMPLETIONPAGE, m_CompletionPage);
    wxLogTrace(wxT("Function Status"), wxT("CWizAttachProject::CreateControls -     m_ErrProjectNotDetectedPage = id: '%d', location: '%p'"), ID_ERRPROJECTNOTDETECTEDPAGE, m_ErrProjectNotDetectedPage);
    wxLogTrace(wxT("Function Status"), wxT("CWizAttachProject::CreateControls -     m_ErrProjectUnavailablePage = id: '%d', location: '%p'"), ID_ERRPROJECTUNAVAILABLEPAGE, m_ErrProjectUnavailablePage);
    wxLogTrace(wxT("Function Status"), wxT("CWizAttachProject::CreateControls -     m_ErrNoInternetConnectionPage = id: '%d', location: '%p'"), ID_ERRNOINTERNETCONNECTIONPAGE, m_ErrNoInternetConnectionPage);
    wxLogTrace(wxT("Function Status"), wxT("CWizAttachProject::CreateControls -     m_ErrAccountAlreadyExistsPage = id: '%d', location: '%p'"), ID_ERRACCOUNTALREADYEXISTSPAGE, m_ErrAccountAlreadyExistsPage);
    wxLogTrace(wxT("Function Status"), wxT("CWizAttachProject::CreateControls -     m_ErrAccountCreationDisabledPage = id: '%d', location: '%p'"), ID_ERRACCOUNTCREATIONDISABLEDPAGE, m_ErrAccountCreationDisabledPage);
    wxLogTrace(wxT("Function Status"), wxT("CWizAttachProject::CreateControls -     m_ErrProxyInfoPage = id: '%d', location: '%p'"), ID_ERRPROXYINFOPAGE, m_ErrProxyInfoPage);
    wxLogTrace(wxT("Function Status"), wxT("CWizAttachProject::CreateControls -     m_ErrProxyHTTPPage = id: '%d', location: '%p'"), ID_ERRPROXYHTTPPAGE, m_ErrProxyHTTPPage);
    wxLogTrace(wxT("Function Status"), wxT("CWizAttachProject::CreateControls -     m_ErrProxySOCKSPage = id: '%d', location: '%p'"), ID_ERRPROXYSOCKSPAGE, m_ErrProxySOCKSPage);
    wxLogTrace(wxT("Function Status"), wxT("CWizAttachProject::CreateControls -     m_ErrProxyCompletionPage = id: '%d', location: '%p'"), ID_ERRPROXYCOMPLETIONPAGE, m_ErrProxyCompletionPage);
    wxLogTrace(wxT("Function Status"), wxT("CWizAttachProject::CreateControls - End Page Map"));

    wxLogTrace(wxT("Function Start/End"), wxT("CWizAttachProject::CreateControls - Function End"));
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
    if (page == m_CompletionPage)
        return false;
    return true;
}

/*!
 * Determine if the wizard page has a previous page
 */

bool CWizAttachProject::HasPrevPage( wxWizardPage* page )
{
    if (page == m_WelcomePage)
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

wxWizardPage* CWizAttachProject::PopPageTransition() 
{
    if (GetCurrentPage()) {
        if (m_PageTransition.size() > 0) {
            wxWizardPage* pPage = m_PageTransition.top();
            m_PageTransition.pop();
            wxASSERT(pPage);
            return pPage;
        }
    }
    return NULL;
}

/*!
 * Add the page transition to the stack.
 */
wxWizardPage* CWizAttachProject::PushPageTransition( wxWizardPage* pCurrentPage, unsigned long ulPageID )
{
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

        if (ID_ACCOUNTCREATIONPAGE == ulPageID)
            pPage = m_AccountCreationPage;

        if (ID_COMPLETIONPAGE == ulPageID)
            pPage = m_CompletionPage;

        if (ID_ERRPROJECTNOTDETECTEDPAGE == ulPageID)
            pPage = m_ErrProjectNotDetectedPage;

        if (ID_ERRPROJECTUNAVAILABLEPAGE == ulPageID)
            pPage = m_ErrProjectUnavailablePage;

        if (ID_ERRNOINTERNETCONNECTIONPAGE == ulPageID)
            pPage = m_ErrNoInternetConnectionPage;

        if (ID_ERRACCOUNTALREADYEXISTSPAGE == ulPageID)
            pPage = m_ErrAccountAlreadyExistsPage;

        if (ID_ERRACCOUNTCREATIONDISABLEDPAGE == ulPageID)
            pPage = m_ErrAccountCreationDisabledPage;

        if (ID_ERRPROXYINFOPAGE == ulPageID)
            pPage = m_ErrProxyInfoPage;

        if (ID_ERRPROXYHTTPPAGE == ulPageID)
            pPage = m_ErrProxyHTTPPage;

        if (ID_ERRPROXYSOCKSPAGE == ulPageID)
            pPage = m_ErrProxySOCKSPage;

        if (ID_ERRPROXYCOMPLETIONPAGE == ulPageID)
            pPage = m_ErrProxyCompletionPage;

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

void CWizAttachProject::OnWizardCancel( wxWizardEvent& event ) {
    if ( wxMessageBox( _("Do you really want to cancel?"), _("Question"),
         wxICON_QUESTION | wxYES_NO, this ) != wxYES
       )
    {
        // not confirmed
        event.Veto();
    }
}

void CWizAttachProject::OnWizardFinished( wxWizardEvent& event ) {
    CMainDocument* pDoc      = wxGetApp().GetDocument();

    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));

    // If the wizard was successful and everything is cool, tell the
    //   core client to attach to the project with the authenticator.
    if ( m_AccountCreationPage->GetProjectCommunitcationsSucceeded() &&
         account_out.authenticator.length() > 0 &&
         account_in.url.length() > 0
       )
    {
        pDoc->rpc.project_attach(
            account_in.url.c_str(),
            account_out.authenticator.c_str()
        );
    } else {
#if defined(__WXDEBUG__)
        if (!m_AccountCreationPage->GetProjectCommunitcationsSucceeded()) {
            ::wxMessageBox(
                wxT("m_AccountCreationPage->GetProjectCommunitcationsSucceeded() = false"),
                wxT("Attach to Project Wizard"),
                wxICON_ERROR | wxOK,
                this
            );
        } else if (account_in.url.length() == 0) {
            ::wxMessageBox(
                wxT("account_in.url.length() == 0"),
                wxT("Attach to Project Wizard"),
                wxICON_ERROR | wxOK,
                this
            );
        } else if (account_out.authenticator.length() == 0) {
            ::wxMessageBox(
                wxT("account_out.authenticator.length() == 0"),
                wxT("Attach to Project Wizard"),
                wxICON_ERROR | wxOK,
                this
            );
        }
#endif
    }
    
    // Let the framework clean everything up.
    event.Skip();
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
 * CWelcomePage type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CWelcomePage, wxWizardPage )

/*!
 * CWelcomePage event table definition
 */

BEGIN_EVENT_TABLE( CWelcomePage, wxWizardPage )

////@begin CWelcomePage event table entries
    EVT_WIZARD_PAGE_CHANGING( -1, CWelcomePage::OnPageChanging )

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
    itemStaticText4->Create( itemWizardPage2, wxID_STATIC, _("Welcome to the Attach to Project\nWizard"), wxDefaultPosition, wxDefaultSize, 0 );
    itemStaticText4->SetFont(wxFont(12, wxSWISS, wxNORMAL, wxBOLD, FALSE, _T("Verdana")));
    itemBoxSizer3->Add(itemStaticText4, 0, wxALIGN_LEFT|wxALL, 5);

    wxStaticText* itemStaticText5 = new wxStaticText;
    itemStaticText5->Create( itemWizardPage2, wxID_STATIC, _("This wizard will guide you through the process of attaching to a project."), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer3->Add(itemStaticText5, 0, wxALIGN_LEFT|wxALL, 5);

    itemBoxSizer3->Add(5, 5, 0, wxALIGN_LEFT|wxALL, 5);

    wxStaticText* itemStaticText7 = new wxStaticText;
    itemStaticText7->Create( itemWizardPage2, wxID_STATIC, _("This wizard will require access to the Internet in order to attach to\na project using an existing account or creating a new one."), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer3->Add(itemStaticText7, 0, wxALIGN_LEFT|wxALL, 5);

#if defined(__WXDEBUG__)
    wxStaticBox* itemStaticBoxSizer8Static = new wxStaticBox(itemWizardPage2, wxID_ANY, _("Debug Flags"));
    wxStaticBoxSizer* itemStaticBoxSizer8 = new wxStaticBoxSizer(itemStaticBoxSizer8Static, wxVERTICAL);
    itemBoxSizer3->Add(itemStaticBoxSizer8, 0, wxGROW|wxALL, 5);

    wxFlexGridSizer* itemFlexGridSizer9 = new wxFlexGridSizer(-1, 2, 0, 0);
    itemFlexGridSizer9->AddGrowableCol(0);
    itemFlexGridSizer9->AddGrowableCol(1);
    itemStaticBoxSizer8->Add(itemFlexGridSizer9, 0, wxGROW|wxALL, 5);

    m_ErrProjectPropertiesCtrl = new wxCheckBox;
    m_ErrProjectPropertiesCtrl->Create( itemWizardPage2, ID_ERRPROJECTPROPERTIES, _("Project Properties Failure"), wxDefaultPosition, wxDefaultSize, wxCHK_2STATE );
    m_ErrProjectPropertiesCtrl->SetValue(FALSE);
    itemFlexGridSizer9->Add(m_ErrProjectPropertiesCtrl, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 0);

    m_ErrProjectCommCtrl = new wxCheckBox;
    m_ErrProjectCommCtrl->Create( itemWizardPage2, ID_ERRPROJECTCOMM, _("Project Comm Failure"), wxDefaultPosition, wxDefaultSize, wxCHK_2STATE );
    m_ErrProjectCommCtrl->SetValue(FALSE);
    itemFlexGridSizer9->Add(m_ErrProjectCommCtrl, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 0);

    m_ErrProjectPropertiesURLCtrl = new wxCheckBox;
    m_ErrProjectPropertiesURLCtrl->Create( itemWizardPage2, ID_ERRPROJECTPROPERTIESURL, _("Project Properties URL Failure"), wxDefaultPosition, wxDefaultSize, wxCHK_2STATE );
    m_ErrProjectPropertiesURLCtrl->SetValue(FALSE);
    itemFlexGridSizer9->Add(m_ErrProjectPropertiesURLCtrl, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 0);

    m_ErrAccountCreationDisabledCtrl = new wxCheckBox;
    m_ErrAccountCreationDisabledCtrl->Create( itemWizardPage2, ID_ERRACCOUNTCREATIONDISABLED, _("Account Creation Disabled"), wxDefaultPosition, wxDefaultSize, wxCHK_2STATE );
    m_ErrAccountCreationDisabledCtrl->SetValue(FALSE);
    itemFlexGridSizer9->Add(m_ErrAccountCreationDisabledCtrl, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 0);

    m_ErrClientAccountCreationDisabledCtrl = new wxCheckBox;
    m_ErrClientAccountCreationDisabledCtrl->Create( itemWizardPage2, ID_ERRCLIENTACCOUNTCREATIONDISABLED, _("Client Account Creation Disabled"), wxDefaultPosition, wxDefaultSize, wxCHK_2STATE );
    m_ErrClientAccountCreationDisabledCtrl->SetValue(FALSE);
    itemFlexGridSizer9->Add(m_ErrClientAccountCreationDisabledCtrl, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 0);

    m_ErrAccountAlreadyExistsCtrl = new wxCheckBox;
    m_ErrAccountAlreadyExistsCtrl->Create( itemWizardPage2, ID_ERRACCOUNTALREADYEXISTS, _("Account Already Exists"), wxDefaultPosition, wxDefaultSize, wxCHK_2STATE );
    m_ErrAccountAlreadyExistsCtrl->SetValue(FALSE);
    itemFlexGridSizer9->Add(m_ErrAccountAlreadyExistsCtrl, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 0);

    m_ErrGoogleCommCtrl = new wxCheckBox;
    m_ErrGoogleCommCtrl->Create( itemWizardPage2, ID_ERRGOOGLECOMM, _("Google Comm Failure"), wxDefaultPosition, wxDefaultSize, wxCHK_2STATE );
    m_ErrGoogleCommCtrl->SetValue(FALSE);
    itemFlexGridSizer9->Add(m_ErrGoogleCommCtrl, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 0);

    itemFlexGridSizer9->Add(5, 5, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 0);

    m_ErrYahooCommCtrl = new wxCheckBox;
    m_ErrYahooCommCtrl->Create( itemWizardPage2, ID_ERRYAHOOCOMM, _("Yahoo Comm Failure"), wxDefaultPosition, wxDefaultSize, wxCHK_2STATE );
    m_ErrYahooCommCtrl->SetValue(FALSE);
    itemFlexGridSizer9->Add(m_ErrYahooCommCtrl, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 0);

    itemFlexGridSizer9->Add(5, 5, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 0);

    m_ErrNetDetectionCtrl = new wxCheckBox;
    m_ErrNetDetectionCtrl->Create( itemWizardPage2, ID_ERRNETDETECTION, _("Net Detection Failure"), wxDefaultPosition, wxDefaultSize, wxCHK_2STATE );
    m_ErrNetDetectionCtrl->SetValue(FALSE);
    itemFlexGridSizer9->Add(m_ErrNetDetectionCtrl, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 0);

#endif

    wxStaticText* itemStaticText21 = new wxStaticText;
    itemStaticText21->Create( itemWizardPage2, wxID_STATIC, _("To continue, click Next."), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer3->Add(itemStaticText21, 0, wxALIGN_LEFT|wxALL, 5);

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
    return PAGE_TRANSITION_NEXT(ID_PROJECTINFOPAGE);
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
    if (m_ErrNetDetectionCtrl->GetValue()) 
        ulFlags |= WIZDEBUG_ERRNETDETECTION;
#endif

    ((CWizAttachProject*)GetParent())->SetDiagFlags( ulFlags );
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
    CProjectInfoPage* itemWizardPage22 = this;

    wxBoxSizer* itemBoxSizer23 = new wxBoxSizer(wxVERTICAL);
    itemWizardPage22->SetSizer(itemBoxSizer23);

    wxStaticText* itemStaticText24 = new wxStaticText;
    itemStaticText24->Create( itemWizardPage22, wxID_STATIC, _("Project Information"), wxDefaultPosition, wxDefaultSize, 0 );
    itemStaticText24->SetFont(wxFont(10, wxSWISS, wxNORMAL, wxBOLD, FALSE, _T("Verdana")));
    itemBoxSizer23->Add(itemStaticText24, 0, wxALIGN_LEFT|wxALL, 5);

    wxStaticText* itemStaticText25 = new wxStaticText;
    itemStaticText25->Create( itemWizardPage22, wxID_STATIC, _("Which project do you wish to attach to?"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer23->Add(itemStaticText25, 0, wxALIGN_LEFT|wxALL, 5);

    itemBoxSizer23->Add(5, 5, 0, wxALIGN_LEFT|wxALL, 5);

    wxStaticText* itemStaticText27 = new wxStaticText;
    itemStaticText27->Create( itemWizardPage22, wxID_STATIC, _("The project URL is generally the project's homepage.  It is in the form of\na web address and can be found in your browsers address bar."), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer23->Add(itemStaticText27, 0, wxALIGN_LEFT|wxALL, 5);

    wxFlexGridSizer* itemFlexGridSizer28 = new wxFlexGridSizer(1, 2, 0, 0);
    itemFlexGridSizer28->AddGrowableCol(1);
    itemBoxSizer23->Add(itemFlexGridSizer28, 0, wxALIGN_LEFT|wxALL, 5);

    m_ProjectUrlStaticCtrl = new wxStaticText;
    m_ProjectUrlStaticCtrl->Create( itemWizardPage22, ID_PROJECTURLSTATICCTRL, _("Project URL:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer28->Add(m_ProjectUrlStaticCtrl, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_ProjectUrlCtrl = new wxTextCtrl;
    m_ProjectUrlCtrl->Create( itemWizardPage22, ID_PROJECTURLCTRL, _T(""), wxDefaultPosition, wxSize(250, -1), 0 );
    itemFlexGridSizer28->Add(m_ProjectUrlCtrl, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    itemBoxSizer23->Add(5, 5, 0, wxALIGN_LEFT|wxALL, 5);

    wxStaticText* itemStaticText32 = new wxStaticText;
    itemStaticText32->Create( itemWizardPage22, wxID_STATIC, _("For more information, and to see a list of some BOINC-based projects,\ngo to:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer23->Add(itemStaticText32, 0, wxALIGN_LEFT|wxALL, 5);

    wxHyperLink* itemHyperLink33 = new wxHyperLink;
    itemHyperLink33->Create( itemWizardPage22, ID_PROJECRINFOBOINCLINK, wxT("http://boinc.berkeley.edu/"), wxDefaultPosition, wxDefaultSize, wxNO_BORDER );
    itemBoxSizer23->Add(itemHyperLink33, 0, wxALIGN_LEFT|wxALL, 5);

    // Set validators
    m_ProjectUrlCtrl->SetValidator( wxTextValidator(wxFILTER_NONE, & m_strProjectURL) );
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
    return PAGE_TRANSITION_NEXT(ID_PROJECTPROPERTIESPAGE);
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
    m_RetrProjectPropertiesImageCtrl = NULL;
    m_RetrProjectPropertiesCtrl = NULL;
    m_FinalProjectPropertiesStatusCtrl = NULL;
////@end CProjectPropertiesPage member initialisation
    m_bProjectPropertiesSucceeded = false;
    m_bProjectPropertiesURLFailure = false;
    m_bProjectAccountCreationDisabled = false;
    m_bProjectClientAccountCreationDisabled = false;
    m_bCommunicateYahooSucceeded = false;
    m_bCommunicateGoogleSucceeded = false;
    m_bDeterminingConnectionStatusSucceeded = false;
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
    CProjectPropertiesPage* itemWizardPage34 = this;

    wxBoxSizer* itemBoxSizer35 = new wxBoxSizer(wxVERTICAL);
    itemWizardPage34->SetSizer(itemBoxSizer35);

    wxStaticText* itemStaticText36 = new wxStaticText;
    itemStaticText36->Create( itemWizardPage34, wxID_STATIC, _("Project Server Communication"), wxDefaultPosition, wxDefaultSize, 0 );
    itemStaticText36->SetFont(wxFont(10, wxSWISS, wxNORMAL, wxBOLD, FALSE, _T("Verdana")));
    itemBoxSizer35->Add(itemStaticText36, 0, wxALIGN_LEFT|wxALL, 5);

    wxStaticText* itemStaticText37 = new wxStaticText;
    itemStaticText37->Create( itemWizardPage34, wxID_STATIC, _("This wizard is now attempting to communicate with the project\nserver."), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer35->Add(itemStaticText37, 0, wxALIGN_LEFT|wxALL, 5);

    itemBoxSizer35->Add(5, 5, 0, wxALIGN_LEFT|wxALL, 5);

    wxFlexGridSizer* itemFlexGridSizer39 = new wxFlexGridSizer(0, 2, 0, 0);
    itemBoxSizer35->Add(itemFlexGridSizer39, 0, wxALIGN_LEFT|wxALL, 5);

    wxBitmap m_RetrProjectPropertiesImageCtrlBitmap(itemWizardPage34->GetBitmapResource(wxT("res/wizquestion.xpm")));
    m_RetrProjectPropertiesImageCtrl = new wxStaticBitmap;
    m_RetrProjectPropertiesImageCtrl->Create( itemWizardPage34, ID_RETRPROJECTPROPERTIESIMAGECTRL, m_RetrProjectPropertiesImageCtrlBitmap, wxDefaultPosition, wxSize(16, 16), 0 );
    itemFlexGridSizer39->Add(m_RetrProjectPropertiesImageCtrl, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_RetrProjectPropertiesCtrl = new wxStaticText;
    m_RetrProjectPropertiesCtrl->Create( itemWizardPage34, ID_RETRPROJECTPROPERTIESCTRL, _("Communicating with the project server."), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer39->Add(m_RetrProjectPropertiesCtrl, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_FinalProjectPropertiesStatusCtrl = new wxStaticText;
    m_FinalProjectPropertiesStatusCtrl->Create( itemWizardPage34, ID_FINALPROJECTPROPERTIESTATUSCTRL, _T(""), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer35->Add(m_FinalProjectPropertiesStatusCtrl, 0, wxALIGN_LEFT|wxALL, 5);

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
    if (GetProjectPropertiesSucceeded() && GetProjectAccountCreationDisabled()) {
        // Account Creation Disabled
        return PAGE_TRANSITION_NEXT(ID_ERRACCOUNTCREATIONDISABLEDPAGE);
    } else if (GetProjectPropertiesSucceeded() && GetProjectClientAccountCreationDisabled()) {
        // Client Account Creation Disabled - Use Legacy Authentication Scheme
        return PAGE_TRANSITION_NEXT(ID_ACCOUNTKEYPAGE);
    } else if (GetProjectPropertiesSucceeded()) {
        // We were successful in retrieving the project properties
        return PAGE_TRANSITION_NEXT(ID_ACCOUNTINFOPAGE);
    } else if (!GetProjectPropertiesSucceeded() && !GetProjectPropertiesURLFailure()) {
        // Not a BOINC based project
        return PAGE_TRANSITION_NEXT(ID_ERRPROJECTNOTDETECTEDPAGE);
    } else if ((!GetCommunicateYahooSucceeded() && !GetCommunicateGoogleSucceeded()) && GetDeterminingConnectionStatusSucceeded()) {
        // Possible proxy problem
        return PAGE_TRANSITION_NEXT(ID_ERRPROXYINFOPAGE);
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

/*!
 * Get bitmap resources
 */

wxBitmap CProjectPropertiesPage::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
    if (name == wxT("res/wizquestion.xpm"))
    {
        wxBitmap bitmap(wizquestion_xpm);
        return bitmap;
    }
    else if (name == wxT("res/wizsuccess.xpm"))
    {
        wxBitmap bitmap(wizsuccess_xpm);
        return bitmap;
    }
    else if (name == wxT("res/wizfailure.xpm"))
    {
        wxBitmap bitmap(wizfailure_xpm);
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
 * wxEVT_PROJECTPROPERTIES_STATECHANGE event handler for ID_PROJECTPROPERTIESPAGE
 */

void CProjectPropertiesPage::OnStateChange( CProjectPropertiesPageEvent& event )
{
    CMainDocument* pDoc      = wxGetApp().GetDocument();
    PROJECT_CONFIG* pc       = &((CWizAttachProject*)GetParent())->project_config;
    bool bPostNewEvent = true;
    int iReturnValue = 0;
    bool bSuccessfulCondition = false;

    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));

    wxFont fontOriginal = m_FinalProjectPropertiesStatusCtrl->GetFont();
    wxFont fontBold = m_FinalProjectPropertiesStatusCtrl->GetFont();
    fontBold.SetWeight(wxBOLD);

    switch(GetCurrentState()) {
        case PROJPROP_INIT:
            // Change the cursor to an hourglass
            ::wxBeginBusyCursor();

            // Set initial bitmaps to question marks since we don't yet know how
            //   things will turn out.
            m_RetrProjectPropertiesImageCtrl->SetBitmap(GetBitmapResource(wxT("res/wizquestion.xpm")));

            // Clear out any text that might exist in the final status field
            m_FinalProjectPropertiesStatusCtrl->SetLabel(wxT(""));

            SetNextState(PROJPROP_RETRPROJECTPROPERTIES_BEGIN);
            break;
        case PROJPROP_RETRPROJECTPROPERTIES_BEGIN:
            // Highlight the current activity by making it bold
            m_RetrProjectPropertiesCtrl->SetFont(fontBold);

            SetNextState(PROJPROP_RETRPROJECTPROPERTIES_EXECUTE);
            break;
        case PROJPROP_RETRPROJECTPROPERTIES_EXECUTE:
            // Attempt to retrieve the project’s account creation policies
            pDoc->rpc.get_project_config(
                ((CWizAttachProject*)GetParent())->m_ProjectInfoPage->GetProjectURL().c_str()
            );

            // Wait until we are done processing the request.
            iReturnValue = ERR_IN_PROGRESS;
            while (ERR_IN_PROGRESS == iReturnValue) {
                iReturnValue = pDoc->rpc.get_project_config_poll(*pc);
                wxSleep(1);
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

                bSuccessfulCondition = pc->client_account_creation_disabled;
                if (bSuccessfulCondition || CHECK_DEBUG_FLAG(WIZDEBUG_ERRCLIENTACCOUNTCREATIONDISABLED)) {
                    SetProjectClientAccountCreationDisabled(true);
                } else {
                    SetProjectClientAccountCreationDisabled(false);
                }


                SetNextState(PROJPROP_CLEANUP);
            } else {
                SetProjectPropertiesSucceeded(false);

                bSuccessfulCondition = (HTTP_STATUS_NOT_FOUND == iReturnValue);
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
            iReturnValue = ERR_IN_PROGRESS;
            while (ERR_IN_PROGRESS == iReturnValue) {
                iReturnValue = pDoc->rpc.lookup_website_poll();
                wxSleep(1);
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
            iReturnValue = ERR_IN_PROGRESS;
            while (ERR_IN_PROGRESS == iReturnValue) {
                iReturnValue = pDoc->rpc.lookup_website_poll();
                wxSleep(1);
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
            if (GetProjectPropertiesSucceeded()) {
                m_RetrProjectPropertiesImageCtrl->SetBitmap(GetBitmapResource(wxT("res/wizsuccess.xpm")));
            } else {
                m_RetrProjectPropertiesImageCtrl->SetBitmap(GetBitmapResource(wxT("res/wizfailure.xpm")));
            }
            m_RetrProjectPropertiesCtrl->SetFont(fontOriginal);
            SetNextState(PROJPROP_END);
            break;
        default:
            // Allow a glimps of what the result was before advancing to the next page.
            wxSleep(1);
            wxCommandEvent eventNext(wxEVT_COMMAND_BUTTON_CLICKED, ((CWizAttachProject*)GetParent())->GetNextButton()->GetId());
            eventNext.SetEventObject(((CWizAttachProject*)GetParent())->GetNextButton());
            GetParent()->AddPendingEvent(eventNext);

            // Change the cursor to a normal cursor
            ::wxEndBusyCursor();

            bPostNewEvent = false;
            break;
    }

    Update();

    if (bPostNewEvent) {
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
    CAccountKeyPage* itemWizardPage43 = this;

    wxBoxSizer* itemBoxSizer44 = new wxBoxSizer(wxVERTICAL);
    itemWizardPage43->SetSizer(itemBoxSizer44);

    wxStaticText* itemStaticText45 = new wxStaticText;
    itemStaticText45->Create( itemWizardPage43, wxID_STATIC, _("Account Key"), wxDefaultPosition, wxDefaultSize, 0 );
    itemStaticText45->SetFont(wxFont(10, wxSWISS, wxNORMAL, wxBOLD, FALSE, _T("Verdana")));
    itemBoxSizer44->Add(itemStaticText45, 0, wxALIGN_LEFT|wxALL, 5);

    wxStaticText* itemStaticText46 = new wxStaticText;
    itemStaticText46->Create( itemWizardPage43, wxID_STATIC, _("Do you have your project account key handy?"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer44->Add(itemStaticText46, 0, wxALIGN_LEFT|wxALL, 5);

    itemBoxSizer44->Add(5, 5, 0, wxALIGN_LEFT|wxALL, 5);

    wxStaticText* itemStaticText48 = new wxStaticText;
    itemStaticText48->Create( itemWizardPage43, wxID_STATIC, _("This project does not support account management through this\nwizard and uses BOINC's account key authentication."), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer44->Add(itemStaticText48, 0, wxALIGN_LEFT|wxALL, 5);

    wxStaticText* itemStaticText49 = new wxStaticText;
    itemStaticText49->Create( itemWizardPage43, wxID_STATIC, _("An account key is a string of 32 random letters and numbers that\nare assigned during the account creation process and is treated\nlike a username and password on other web based systems.  It will\nhave been sent to you via email as part of the confirmation email\nmessage. Please copy and paste it into the text box below."), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer44->Add(itemStaticText49, 0, wxALIGN_LEFT|wxALL, 5);

    wxStaticText* itemStaticText50 = new wxStaticText;
    itemStaticText50->Create( itemWizardPage43, wxID_STATIC, _("An account key typically looks like:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer44->Add(itemStaticText50, 0, wxALIGN_LEFT|wxALL, 5);

    wxStaticText* itemStaticText51 = new wxStaticText;
    itemStaticText51->Create( itemWizardPage43, wxID_STATIC, _("82412313ac88e9a3638f66ea82186948"), wxDefaultPosition, wxDefaultSize, 0 );
    itemStaticText51->SetFont(wxFont(8, wxSWISS, wxNORMAL, wxNORMAL, FALSE, _T("Courier New")));
    itemBoxSizer44->Add(itemStaticText51, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 0);

    wxFlexGridSizer* itemFlexGridSizer52 = new wxFlexGridSizer(1, 2, 0, 0);
    itemFlexGridSizer52->AddGrowableCol(1);
    itemBoxSizer44->Add(itemFlexGridSizer52, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    m_AccountKeyStaticCtrl = new wxStaticText;
    m_AccountKeyStaticCtrl->Create( itemWizardPage43, ID_ACCOUNTKEYSTATICCTRL, _("Account key:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer52->Add(m_AccountKeyStaticCtrl, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_AccountKeyCtrl = new wxTextCtrl;
    m_AccountKeyCtrl->Create( itemWizardPage43, ID_ACCOUNTKEYCTRL, _T(""), wxDefaultPosition, wxSize(225, -1), 0 );
    itemFlexGridSizer52->Add(m_AccountKeyCtrl, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    // Set validators
    m_AccountKeyCtrl->SetValidator( wxTextValidator(wxFILTER_ALPHANUMERIC, & m_strAccountKey) );
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
    return PAGE_TRANSITION_NEXT(ID_COMPLETIONPAGE);
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
    CAccountInfoPage* itemWizardPage55 = this;

    wxBoxSizer* itemBoxSizer56 = new wxBoxSizer(wxVERTICAL);
    itemWizardPage55->SetSizer(itemBoxSizer56);

    wxStaticText* itemStaticText57 = new wxStaticText;
    itemStaticText57->Create( itemWizardPage55, wxID_STATIC, _("Account Information"), wxDefaultPosition, wxDefaultSize, 0 );
    itemStaticText57->SetFont(wxFont(10, wxSWISS, wxNORMAL, wxBOLD, FALSE, _T("Verdana")));
    itemBoxSizer56->Add(itemStaticText57, 0, wxALIGN_LEFT|wxALL, 5);

    wxStaticText* itemStaticText58 = new wxStaticText;
    itemStaticText58->Create( itemWizardPage55, wxID_STATIC, _("Do you wish to use an existing account or create a new one?"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer56->Add(itemStaticText58, 0, wxALIGN_LEFT|wxALL, 5);

    itemBoxSizer56->Add(5, 5, 0, wxALIGN_LEFT|wxALL, 5);

    wxStaticText* itemStaticText60 = new wxStaticText;
    itemStaticText60->Create( itemWizardPage55, wxID_STATIC, _("If this is the first time you have attempted to attach to this project then\nyou should create a new account.  If you already have an account you\nshould use your existing email address and password to attach to the\nproject."), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer56->Add(itemStaticText60, 0, wxALIGN_LEFT|wxALL, 5);

    wxFlexGridSizer* itemFlexGridSizer61 = new wxFlexGridSizer(1, 2, 0, 0);
    itemFlexGridSizer61->AddGrowableCol(1);
    itemBoxSizer56->Add(itemFlexGridSizer61, 0, wxGROW|wxALL, 5);

    m_AccountCreateCtrl = new wxRadioButton;
    m_AccountCreateCtrl->Create( itemWizardPage55, ID_ACCOUNTCREATECTRL, _("Create new account"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP );
    m_AccountCreateCtrl->SetValue(FALSE);
    itemFlexGridSizer61->Add(m_AccountCreateCtrl, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_AccountUseExistingCtrl = new wxRadioButton;
    m_AccountUseExistingCtrl->Create( itemWizardPage55, ID_ACCOUNTUSEEXISTINGCTRL, _("Use existing account"), wxDefaultPosition, wxDefaultSize, 0 );
    m_AccountUseExistingCtrl->SetValue(FALSE);
    itemFlexGridSizer61->Add(m_AccountUseExistingCtrl, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxFlexGridSizer* itemFlexGridSizer64 = new wxFlexGridSizer(3, 2, 0, 0);
    itemFlexGridSizer64->AddGrowableCol(1);
    itemBoxSizer56->Add(itemFlexGridSizer64, 0, wxGROW|wxALL, 5);

    m_AccountEmailAddressStaticCtrl = new wxStaticText;
    m_AccountEmailAddressStaticCtrl->Create( itemWizardPage55, ID_ACCOUNTEMAILADDRESSSTATICCTRL, _("Email address:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer64->Add(m_AccountEmailAddressStaticCtrl, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_AccountEmailAddressCtrl = new wxTextCtrl;
    m_AccountEmailAddressCtrl->Create( itemWizardPage55, ID_ACCOUNTEMAILADDRESSCTRL, _T(""), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer64->Add(m_AccountEmailAddressCtrl, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_AccountPasswordStaticCtrl = new wxStaticText;
    m_AccountPasswordStaticCtrl->Create( itemWizardPage55, ID_ACCOUNTPASSWORDSTATICCTRL, _("Password:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer64->Add(m_AccountPasswordStaticCtrl, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_AccountPasswordCtrl = new wxTextCtrl;
    m_AccountPasswordCtrl->Create( itemWizardPage55, ID_ACCOUNTPASSWORDCTRL, _T(""), wxDefaultPosition, wxDefaultSize, wxTE_PASSWORD );
    itemFlexGridSizer64->Add(m_AccountPasswordCtrl, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_AccountConfirmPasswordStaticCtrl = new wxStaticText;
    m_AccountConfirmPasswordStaticCtrl->Create( itemWizardPage55, ID_ACCOUNTCONFIRMPASSWORDSTATICCTRL, _("Confirm password:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer64->Add(m_AccountConfirmPasswordStaticCtrl, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_AccountConfirmPasswordCtrl = new wxTextCtrl;
    m_AccountConfirmPasswordCtrl->Create( itemWizardPage55, ID_ACCOUNTCONFIRMPASSWORDCTRL, _T(""), wxDefaultPosition, wxDefaultSize, wxTE_PASSWORD );
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
    return PAGE_TRANSITION_NEXT(ID_ACCOUNTCREATIONPAGE);
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
 * wxEVT_COMMAND_RADIOBUTTON_SELECTED event handler for ID_ACCOUNTUSEXISTINGBUTTON
 */

void CAccountInfoPage::OnAccountUseExistingCtrlSelected( wxCommandEvent& event )
{
    m_AccountConfirmPasswordStaticCtrl->Hide();
    m_AccountConfirmPasswordCtrl->Hide();
}


/*!
 * wxEVT_COMMAND_RADIOBUTTON_SELECTED event handler for ID_ACCOUNTCREATEBUTTON
 */

void CAccountInfoPage::OnAccountCreateCtrlSelected( wxCommandEvent& event )
{
    m_AccountConfirmPasswordStaticCtrl->Show();
    m_AccountConfirmPasswordCtrl->Show();
}


/*!
 * wxEVT_WIZARD_PAGE_CHANGING event handler for ID_ACCOUNTINFOPAGE
 */

void CAccountInfoPage::OnPageChanging( wxWizardEvent& event )
{
    if (event.GetDirection() == false) return;

    wxString strTitle = _("Attach to Project Wizard");
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

/*!
 * wxEVT_WIZARD_PAGE_CHANGED event handler for ID_ACCOUNTINFOPAGE
 */

void CAccountInfoPage::OnPageChanged( wxWizardEvent& event ) 
{
    if (event.GetDirection() == false) return;

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
 * CProjectPropertiesPage custom event definition
 */
DEFINE_EVENT_TYPE(wxEVT_ACCOUNTCREATION_STATECHANGE)


/*!
 * CAccountCreationPage type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CAccountCreationPage, wxWizardPage )

/*!
 * CAccountCreationPage event table definition
 */

BEGIN_EVENT_TABLE( CAccountCreationPage, wxWizardPage )

    EVT_ACCOUNTCREATION_STATECHANGE( CAccountCreationPage::OnStateChange )

////@begin CAccountCreationPage event table entries
    EVT_WIZARD_PAGE_CHANGED( -1, CAccountCreationPage::OnPageChanged )

////@end CAccountCreationPage event table entries

END_EVENT_TABLE()

/*!
 * CAccountCreationPage constructors
 */

CAccountCreationPage::CAccountCreationPage( )
{
}

CAccountCreationPage::CAccountCreationPage( wxWizard* parent )
{
    Create( parent );
}

/*!
 * CProjectPropertiesPage creator
 */

bool CAccountCreationPage::Create( wxWizard* parent )
{
////@begin CAccountCreationPage member initialisation
    m_ProjectCommunitcationsImageCtrl = NULL;
    m_ProjectCommunitcationsCtrl = NULL;
    m_FinalAccountCreationStatusCtrl = NULL;
////@end CAccountCreationPage member initialisation
    m_bProjectCommunitcationsSucceeded = false;
    m_bProjectUnavailable = false;
    m_bProjectAccountAlreadyExists = false;
    m_iCurrentState = ACCOUNTCREATION_INIT;

////@begin CAccountCreationPage creation
    wxBitmap wizardBitmap(wxNullBitmap);
    wxWizardPage::Create( parent, wizardBitmap );

    CreateControls();
    GetSizer()->Fit(this);
////@end CAccountCreationPage creation
    return TRUE;
}

/*!
 * Control creation for CProjectPropertiesPage
 */

void CAccountCreationPage::CreateControls()
{    
////@begin CAccountCreationPage content construction
    CAccountCreationPage* itemWizardPage71 = this;

    wxBoxSizer* itemBoxSizer72 = new wxBoxSizer(wxVERTICAL);
    itemWizardPage71->SetSizer(itemBoxSizer72);

    wxStaticText* itemStaticText73 = new wxStaticText;
    itemStaticText73->Create( itemWizardPage71, wxID_STATIC, _("Account Creation"), wxDefaultPosition, wxDefaultSize, 0 );
    itemStaticText73->SetFont(wxFont(10, wxSWISS, wxNORMAL, wxBOLD, FALSE, _T("Verdana")));
    itemBoxSizer72->Add(itemStaticText73, 0, wxALIGN_LEFT|wxALL, 5);

    wxStaticText* itemStaticText74 = new wxStaticText;
    itemStaticText74->Create( itemWizardPage71, wxID_STATIC, _("This wizard is now attempting to create a new account or validate your\nexisting account."), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer72->Add(itemStaticText74, 0, wxALIGN_LEFT|wxALL, 5);

    itemBoxSizer72->Add(5, 5, 0, wxALIGN_LEFT|wxALL, 5);

    wxFlexGridSizer* itemFlexGridSizer76 = new wxFlexGridSizer(0, 2, 0, 0);
    itemBoxSizer72->Add(itemFlexGridSizer76, 0, wxALIGN_LEFT|wxALL, 5);

    wxBitmap m_ProjectCommunitcationsImageCtrlBitmap(itemWizardPage71->GetBitmapResource(wxT("res/wizquestion.xpm")));
    m_ProjectCommunitcationsImageCtrl = new wxStaticBitmap;
    m_ProjectCommunitcationsImageCtrl->Create( itemWizardPage71, ID_PROJECTCOMMUNICATIONSIMAGECTRL, m_ProjectCommunitcationsImageCtrlBitmap, wxDefaultPosition, wxSize(16, 16), 0 );
    itemFlexGridSizer76->Add(m_ProjectCommunitcationsImageCtrl, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_ProjectCommunitcationsCtrl = new wxStaticText;
    m_ProjectCommunitcationsCtrl->Create( itemWizardPage71, ID_PROJECTCOMMUNICATIONSCTRL, _("Communicating with BOINC project"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer76->Add(m_ProjectCommunitcationsCtrl, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_FinalAccountCreationStatusCtrl = new wxStaticText;
    m_FinalAccountCreationStatusCtrl->Create( itemWizardPage71, ID_FINALACCOUNTCREATIONSTATUSCTRL, _T(""), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer72->Add(m_FinalAccountCreationStatusCtrl, 0, wxALIGN_LEFT|wxALL, 5);

////@end CAccountCreationPage content construction
}

/*!
 * Gets the previous page.
 */

wxWizardPage* CAccountCreationPage::GetPrev() const
{
    return PAGE_TRANSITION_BACK;
}

/*!
 * Gets the next page.
 */

wxWizardPage* CAccountCreationPage::GetNext() const
{
    if (GetProjectCommunitcationsSucceeded()) {
        // We were successful in creating or retrieving an account
        return PAGE_TRANSITION_NEXT(ID_COMPLETIONPAGE);
    } else if (!GetProjectCommunitcationsSucceeded() && GetProjectAccountAlreadyExists()) {
        // The requested account already exists
        return PAGE_TRANSITION_NEXT(ID_ERRACCOUNTALREADYEXISTSPAGE);
    } else {
        // The project much be down for maintenance
        return PAGE_TRANSITION_NEXT(ID_ERRPROJECTUNAVAILABLEPAGE);
    } 
    return NULL;
}

/*!
 * Should we show tooltips?
 */

bool CAccountCreationPage::ShowToolTips()
{
    return TRUE;
}

/*!
 * Get bitmap resources
 */

wxBitmap CAccountCreationPage::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
    if (name == wxT("res/wizquestion.xpm"))
    {
        wxBitmap bitmap(wizquestion_xpm);
        return bitmap;
    }
    else if (name == wxT("res/wizsuccess.xpm"))
    {
        wxBitmap bitmap(wizsuccess_xpm);
        return bitmap;
    }
    else if (name == wxT("res/wizfailure.xpm"))
    {
        wxBitmap bitmap(wizfailure_xpm);
        return bitmap;
    }
    return wxNullBitmap;
}

/*!
 * Get icon resources
 */

wxIcon CAccountCreationPage::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CAccountCreationPage icon retrieval
    return wxNullIcon;
////@end CAccountCreationPage icon retrieval
}

/*!
 * wxEVT_WIZARD_PAGE_CHANGED event handler for ID_ACCOUNTCREATIONPAGE
 */

void CAccountCreationPage::OnPageChanged( wxWizardEvent& event )
{
    if (event.GetDirection() == false) return;

    SetProjectCommunitcationsSucceeded(false);
    SetProjectUnavailable(false);
    SetProjectAccountAlreadyExists(false);
    SetNextState(ACCOUNTCREATION_INIT);

    CAccountCreationPageEvent TransitionEvent(wxEVT_ACCOUNTCREATION_STATECHANGE, this);
    AddPendingEvent(TransitionEvent);
}

/*!
 * wxEVT_ACCOUNTCREATION_STATECHANGE event handler for ID_ACCOUNTCREATIONPAGE
 */

void CAccountCreationPage::OnStateChange( CAccountCreationPageEvent& event )
{
    CMainDocument* pDoc      = wxGetApp().GetDocument();
    ACCOUNT_IN* ai           = &((CWizAttachProject*)GetParent())->account_in;
    ACCOUNT_OUT* ao          = &((CWizAttachProject*)GetParent())->account_out;
    bool bPostNewEvent = true;
    int iReturnValue = 0;
    bool bSuccessfulCondition = false;

    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));

    wxFont fontOriginal = m_FinalAccountCreationStatusCtrl->GetFont();
    wxFont fontBold = m_FinalAccountCreationStatusCtrl->GetFont();
    fontBold.SetWeight(wxBOLD);

    switch(GetCurrentState()) {
        case ACCOUNTCREATION_INIT:
            // Change the cursor to an hourglass
            ::wxBeginBusyCursor();

            // Set initial bitmaps to question marks since we don't yet know how
            //   things will turn out.
            m_ProjectCommunitcationsImageCtrl->SetBitmap(GetBitmapResource(wxT("res/wizquestion.xpm")));

            // Clear out any text that might exist in the final status field
            m_FinalAccountCreationStatusCtrl->SetLabel(wxT(""));

            SetNextState(ACCOUNTCREATION_PROJECTCOMM_BEGIN);
            break;
        case ACCOUNTCREATION_PROJECTCOMM_BEGIN:
            // Highlight the current activity by making it bold
            m_ProjectCommunitcationsCtrl->SetFont(fontBold);

            SetNextState(ACCOUNTCREATION_PROJECTCOMM_EXECUTE);
            break;
        case ACCOUNTCREATION_PROJECTCOMM_EXECUTE:
            // Attempt to create the account or reterieve the authenticator.
            ai->clear();
            ao->clear();
            if (((CWizAttachProject*)GetParent())->m_AccountInfoPage->m_AccountCreateCtrl->GetValue()) {
                if (!((CWizAttachProject*)GetParent())->project_config.uses_username) {
                    ai->email_addr = ((CWizAttachProject*)GetParent())->m_AccountInfoPage->GetAccountEmailAddress().c_str();
                    ai->user_name = ::wxGetUserName().c_str();
                } else {
                    ai->email_addr = wxT("");
                    ai->user_name = ((CWizAttachProject*)GetParent())->m_AccountInfoPage->GetAccountEmailAddress().c_str();
                }
                ai->passwd = ((CWizAttachProject*)GetParent())->m_AccountInfoPage->GetAccountPassword().c_str();
                ai->url = ((CWizAttachProject*)GetParent())->m_ProjectInfoPage->GetProjectURL().c_str();
                pDoc->rpc.create_account(*ai);

                // Wait until we are done processing the request.
                iReturnValue = ERR_IN_PROGRESS;
                while (ERR_IN_PROGRESS == iReturnValue) {
                    iReturnValue = pDoc->rpc.create_account_poll(*ao);
                    wxSleep(1);
                }
            } else {
                if (!((CWizAttachProject*)GetParent())->project_config.uses_username) {
                    ai->email_addr = ((CWizAttachProject*)GetParent())->m_AccountInfoPage->GetAccountEmailAddress().c_str();
                } else {
                    ai->user_name= ((CWizAttachProject*)GetParent())->m_AccountInfoPage->GetAccountEmailAddress().c_str();
                }
                ai->passwd = ((CWizAttachProject*)GetParent())->m_AccountInfoPage->GetAccountPassword().c_str();
                ai->url = ((CWizAttachProject*)GetParent())->m_ProjectInfoPage->GetProjectURL().c_str();
                pDoc->rpc.lookup_account(*ai);

                // Wait until we are done processing the request.
                iReturnValue = ERR_IN_PROGRESS;
                while (ERR_IN_PROGRESS == iReturnValue) {
                    iReturnValue = pDoc->rpc.lookup_account_poll(*ao);
                    wxSleep(1);
                }
            }

            if ((BOINC_SUCCESS == iReturnValue) && !CHECK_DEBUG_FLAG(WIZDEBUG_ERRPROJECTCOMM)) {
                m_ProjectCommunitcationsImageCtrl->SetBitmap(GetBitmapResource(wxT("res/wizsuccess.xpm")));
                SetProjectCommunitcationsSucceeded(true);
            } else {
                m_ProjectCommunitcationsImageCtrl->SetBitmap(GetBitmapResource(wxT("res/wizfailure.xpm")));
                SetProjectCommunitcationsSucceeded(false);

                if ((ERR_NONUNIQUE_EMAIL == iReturnValue) || CHECK_DEBUG_FLAG(WIZDEBUG_ERRACCOUNTALREADYEXISTS)) {
                    SetProjectAccountAlreadyExists(true);
                } else {
                    SetProjectAccountAlreadyExists(false);
                }
            }

            SetNextState(ACCOUNTCREATION_CLEANUP);
            break;
        case ACCOUNTCREATION_CLEANUP:
            m_ProjectCommunitcationsCtrl->SetFont(fontOriginal);
            SetNextState(ACCOUNTCREATION_END);
            break;
        default:
            // Allow a glimps of what the result was before advancing to the next page.
            wxSleep(1);
            wxCommandEvent eventNext(wxEVT_COMMAND_BUTTON_CLICKED, ((CWizAttachProject*)GetParent())->GetNextButton()->GetId());
            eventNext.SetEventObject(((CWizAttachProject*)GetParent())->GetNextButton());
            GetParent()->AddPendingEvent(eventNext);

            // Change the cursor to a normal cursor
            ::wxEndBusyCursor();

            bPostNewEvent = false;
            break;
    }

    Update();

    if (bPostNewEvent) {
        CAccountCreationPageEvent TransitionEvent(wxEVT_ACCOUNTCREATION_STATECHANGE, this);
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
 * CAccountResultPage creator
 */

bool CCompletionPage::Create( wxWizard* parent )
{
////@begin CCompletionPage member initialisation
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
 * Control creation for CAccountResultPage
 */

void CCompletionPage::CreateControls()
{    
////@begin CCompletionPage content construction
    CCompletionPage* itemWizardPage80 = this;

    wxBoxSizer* itemBoxSizer81 = new wxBoxSizer(wxVERTICAL);
    itemWizardPage80->SetSizer(itemBoxSizer81);

    wxStaticText* itemStaticText82 = new wxStaticText;
    itemStaticText82->Create( itemWizardPage80, wxID_STATIC, _("Wizard Completion"), wxDefaultPosition, wxDefaultSize, 0 );
    itemStaticText82->SetFont(wxFont(10, wxSWISS, wxNORMAL, wxBOLD, FALSE, _T("Verdana")));
    itemBoxSizer81->Add(itemStaticText82, 0, wxALIGN_LEFT|wxALL, 5);

    wxStaticText* itemStaticText83 = new wxStaticText;
    itemStaticText83->Create( itemWizardPage80, wxID_STATIC, _("Congratulations"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer81->Add(itemStaticText83, 0, wxALIGN_LEFT|wxALL, 5);

    itemBoxSizer81->Add(5, 5, 0, wxALIGN_LEFT|wxALL, 5);

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
 * CErrProjectNotDetectedPage type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CErrProjectNotDetectedPage, wxWizardPage )

/*!
 * CErrProjectNotDetectedPage event table definition
 */

BEGIN_EVENT_TABLE( CErrProjectNotDetectedPage, wxWizardPage )

////@begin CErrProjectNotDetectedPage event table entries
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
    CErrProjectNotDetectedPage* itemWizardPage85 = this;

    wxBoxSizer* itemBoxSizer86 = new wxBoxSizer(wxVERTICAL);
    itemWizardPage85->SetSizer(itemBoxSizer86);

    wxStaticText* itemStaticText87 = new wxStaticText;
    itemStaticText87->Create( itemWizardPage85, wxID_STATIC, _("Project Unavailable"), wxDefaultPosition, wxDefaultSize, 0 );
    itemStaticText87->SetFont(wxFont(10, wxSWISS, wxNORMAL, wxBOLD, FALSE, _T("Verdana")));
    itemBoxSizer86->Add(itemStaticText87, 0, wxALIGN_LEFT|wxALL, 5);

    wxStaticText* itemStaticText88 = new wxStaticText;
    itemStaticText88->Create( itemWizardPage85, wxID_STATIC, _("This project does not appear to exist."), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer86->Add(itemStaticText88, 0, wxALIGN_LEFT|wxALL, 5);

    itemBoxSizer86->Add(5, 5, 0, wxALIGN_LEFT|wxALL, 5);

    wxStaticText* itemStaticText90 = new wxStaticText;
    itemStaticText90->Create( itemWizardPage85, wxID_STATIC, _("The project URL provided does not appear to be a BOINC based\nproject."), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer86->Add(itemStaticText90, 0, wxALIGN_LEFT|wxALL, 5);

    wxStaticText* itemStaticText91 = new wxStaticText;
    itemStaticText91->Create( itemWizardPage85, wxID_STATIC, _("You might want to checkout the project’s homepage for news\nabout possible project issues and verify that the project URL\nprovided is correct."), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer86->Add(itemStaticText91, 0, wxALIGN_LEFT|wxALL, 5);

    wxStaticText* itemStaticText92 = new wxStaticText;
    itemStaticText92->Create( itemWizardPage85, wxID_STATIC, _("More information about BOINC can be found here:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer86->Add(itemStaticText92, 0, wxALIGN_LEFT|wxALL, 5);

    wxHyperLink* itemHyperLink93 = new wxHyperLink;
    itemHyperLink93->Create( itemWizardPage85, ID_HYPERLINK, wxT("http://boinc.berkeley.edu/"), wxDefaultPosition, wxDefaultSize, wxNO_BORDER );
    itemBoxSizer86->Add(itemHyperLink93, 0, wxALIGN_LEFT|wxALL, 5);

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
    return PAGE_TRANSITION_NEXT(ID_COMPLETIONPAGE);
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
 * CErrProjectUnavailablePage type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CErrProjectUnavailablePage, wxWizardPage )

/*!
 * CErrProjectUnavailablePage event table definition
 */

BEGIN_EVENT_TABLE( CErrProjectUnavailablePage, wxWizardPage )

////@begin CErrProjectUnavailablePage event table entries
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
    CErrProjectUnavailablePage* itemWizardPage94 = this;

    wxBoxSizer* itemBoxSizer95 = new wxBoxSizer(wxVERTICAL);
    itemWizardPage94->SetSizer(itemBoxSizer95);

    wxStaticText* itemStaticText96 = new wxStaticText;
    itemStaticText96->Create( itemWizardPage94, wxID_STATIC, _("Project Temporarily Unavailable"), wxDefaultPosition, wxDefaultSize, 0 );
    itemStaticText96->SetFont(wxFont(10, wxSWISS, wxNORMAL, wxBOLD, FALSE, _T("Verdana")));
    itemBoxSizer95->Add(itemStaticText96, 0, wxALIGN_LEFT|wxALL, 5);

    wxStaticText* itemStaticText97 = new wxStaticText;
    itemStaticText97->Create( itemWizardPage94, wxID_STATIC, _("This project appears to be offline or down for maintenance."), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer95->Add(itemStaticText97, 0, wxALIGN_LEFT|wxALL, 5);

    itemBoxSizer95->Add(5, 5, 0, wxALIGN_LEFT|wxALL, 5);

    wxStaticText* itemStaticText99 = new wxStaticText;
    itemStaticText99->Create( itemWizardPage94, wxID_STATIC, _("This wizard was able to detect a network connection and able to\ncommunicate with Yahoo and/or Google which would indicate that\nnetwork communication is not obstructed.  This seems to be a\ntransient error and trying to attach to this project at a later time\nwould be better."), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer95->Add(itemStaticText99, 0, wxALIGN_LEFT|wxALL, 5);

    wxStaticText* itemStaticText100 = new wxStaticText;
    itemStaticText100->Create( itemWizardPage94, wxID_STATIC, _("You might want to checkout the project’s homepage for news\nabout possible network or project issues."), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer95->Add(itemStaticText100, 0, wxALIGN_LEFT|wxALL, 5);

    wxStaticText* itemStaticText101 = new wxStaticText;
    itemStaticText101->Create( itemWizardPage94, wxID_STATIC, _("More information about BOINC can be found here:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer95->Add(itemStaticText101, 0, wxALIGN_LEFT|wxALL, 5);

    wxHyperLink* itemHyperLink102 = new wxHyperLink;
    itemHyperLink102->Create( itemWizardPage94, ID_PROJECTUNAVAILABLEBOINCLINK, wxT("http://boinc.berkeley.edu/"), wxDefaultPosition, wxDefaultSize, wxNO_BORDER );
    itemBoxSizer95->Add(itemHyperLink102, 0, wxALIGN_LEFT|wxALL, 5);

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
    return PAGE_TRANSITION_NEXT(ID_COMPLETIONPAGE);
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
 * CErrNoInternetConnectionPage type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CErrNoInternetConnectionPage, wxWizardPage )

/*!
 * CErrNoInternetConnectionPage event table definition
 */

BEGIN_EVENT_TABLE( CErrNoInternetConnectionPage, wxWizardPage )

////@begin CErrNoInternetConnectionPage event table entries
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
 * CErrNoInternetConnection creator
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
 * Control creation for CErrNoInternetConnection
 */

void CErrNoInternetConnectionPage::CreateControls()
{    
////@begin CErrNoInternetConnectionPage content construction
    CErrNoInternetConnectionPage* itemWizardPage103 = this;

    wxBoxSizer* itemBoxSizer104 = new wxBoxSizer(wxVERTICAL);
    itemWizardPage103->SetSizer(itemBoxSizer104);

    wxStaticText* itemStaticText105 = new wxStaticText;
    itemStaticText105->Create( itemWizardPage103, wxID_STATIC, _("No Internet Connection Detected"), wxDefaultPosition, wxDefaultSize, 0 );
    itemStaticText105->SetFont(wxFont(10, wxSWISS, wxNORMAL, wxBOLD, FALSE, _T("Verdana")));
    itemBoxSizer104->Add(itemStaticText105, 0, wxALIGN_LEFT|wxALL, 5);

    wxStaticText* itemStaticText106 = new wxStaticText;
    itemStaticText106->Create( itemWizardPage103, wxID_STATIC, _("Could not communicate with the desired project or any of the known\ncomputers on the Internet."), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer104->Add(itemStaticText106, 0, wxALIGN_LEFT|wxALL, 5);

    itemBoxSizer104->Add(5, 5, 0, wxALIGN_LEFT|wxALL, 5);

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
    return PAGE_TRANSITION_NEXT(ID_PROJECTINFOPAGE);
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
 * CErrAccountAlreadyExistsPage type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CErrAccountAlreadyExistsPage, wxWizardPage )

/*!
 * CErrAccountAlreadyExistsPage event table definition
 */

BEGIN_EVENT_TABLE( CErrAccountAlreadyExistsPage, wxWizardPage )

////@begin CErrAccountAlreadyExistsPage event table entries
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
    CErrAccountAlreadyExistsPage* itemWizardPage108 = this;

    wxBoxSizer* itemBoxSizer109 = new wxBoxSizer(wxVERTICAL);
    itemWizardPage108->SetSizer(itemBoxSizer109);

    wxStaticText* itemStaticText110 = new wxStaticText;
    itemStaticText110->Create( itemWizardPage108, wxID_STATIC, _("Account Already Exists"), wxDefaultPosition, wxDefaultSize, 0 );
    itemStaticText110->SetFont(wxFont(10, wxSWISS, wxNORMAL, wxBOLD, FALSE, _T("Verdana")));
    itemBoxSizer109->Add(itemStaticText110, 0, wxALIGN_LEFT|wxALL, 5);

    wxStaticText* itemStaticText111 = new wxStaticText;
    itemStaticText111->Create( itemWizardPage108, wxID_STATIC, _("The requested account is already in use."), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer109->Add(itemStaticText111, 0, wxALIGN_LEFT|wxALL, 5);

    itemBoxSizer109->Add(5, 5, 0, wxALIGN_LEFT|wxALL, 5);

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
    return PAGE_TRANSITION_NEXT(ID_ACCOUNTINFOPAGE);
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
 * CErrAccountCreationDisabledPage type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CErrAccountCreationDisabledPage, wxWizardPage )

/*!
 * CErrAccountCreationDisabledPage event table definition
 */

BEGIN_EVENT_TABLE( CErrAccountCreationDisabledPage, wxWizardPage )

////@begin CErrAccountCreationDisabledPage event table entries
////@end CErrAccountCreationDisabledPage event table entries

END_EVENT_TABLE()

/*!
 * CErrAccountCreationDisabledPage constructors
 */

CErrAccountCreationDisabledPage::CErrAccountCreationDisabledPage( )
{
}

CErrAccountCreationDisabledPage::CErrAccountCreationDisabledPage( wxWizard* parent )
{
    Create( parent );
}

/*!
 * CErrAccountCreationDisabledPage creator
 */

bool CErrAccountCreationDisabledPage::Create( wxWizard* parent )
{
////@begin CErrAccountCreationDisabledPage member initialisation
////@end CErrAccountCreationDisabledPage member initialisation

////@begin CErrAccountCreationDisabledPage creation
    wxBitmap wizardBitmap(wxNullBitmap);
    wxWizardPage::Create( parent, wizardBitmap );

    CreateControls();
    GetSizer()->Fit(this);
////@end CErrAccountCreationDisabledPage creation
    return TRUE;
}

/*!
 * Control creation for CErrAccountCreationDisabledPage
 */

void CErrAccountCreationDisabledPage::CreateControls()
{    
////@begin CErrAccountCreationDisabledPage content construction
    CErrAccountCreationDisabledPage* itemWizardPage113 = this;

    wxBoxSizer* itemBoxSizer114 = new wxBoxSizer(wxVERTICAL);
    itemWizardPage113->SetSizer(itemBoxSizer114);

    wxStaticText* itemStaticText115 = new wxStaticText;
    itemStaticText115->Create( itemWizardPage113, wxID_STATIC, _("Account Creation Disabled"), wxDefaultPosition, wxDefaultSize, 0 );
    itemStaticText115->SetFont(wxFont(10, wxSWISS, wxNORMAL, wxBOLD, FALSE, _T("Verdana")));
    itemBoxSizer114->Add(itemStaticText115, 0, wxALIGN_LEFT|wxALL, 5);

    wxStaticText* itemStaticText116 = new wxStaticText;
    itemStaticText116->Create( itemWizardPage113, wxID_STATIC, _("This project is not accepting any new clients at this time."), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer114->Add(itemStaticText116, 0, wxALIGN_LEFT|wxALL, 5);

    itemBoxSizer114->Add(5, 5, 0, wxALIGN_LEFT|wxALL, 5);

////@end CErrAccountCreationDisabledPage content construction
}

/*!
 * Gets the previous page.
 */

wxWizardPage* CErrAccountCreationDisabledPage::GetPrev() const
{
    return PAGE_TRANSITION_BACK;
}

/*!
 * Gets the next page.
 */

wxWizardPage* CErrAccountCreationDisabledPage::GetNext() const
{
    return PAGE_TRANSITION_NEXT(ID_COMPLETIONPAGE);
}

/*!
 * Should we show tooltips?
 */

bool CErrAccountCreationDisabledPage::ShowToolTips()
{
    return TRUE;
}

/*!
 * Get bitmap resources
 */

wxBitmap CErrAccountCreationDisabledPage::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CErrAccountCreationDisabledPage bitmap retrieval
    return wxNullBitmap;
////@end CErrAccountCreationDisabledPage bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CErrAccountCreationDisabledPage::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CErrAccountCreationDisabledPage icon retrieval
    return wxNullIcon;
////@end CErrAccountCreationDisabledPage icon retrieval
}


/*!
 * CErrProxyInfoPage type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CErrProxyInfoPage, wxWizardPage )

/*!
 * CErrProxyInfoPage event table definition
 */

BEGIN_EVENT_TABLE( CErrProxyInfoPage, wxWizardPage )

////@begin CErrProxyInfoPage event table entries
////@end CErrProxyInfoPage event table entries

END_EVENT_TABLE()

/*!
 * CErrProxyInfoPage constructors
 */

CErrProxyInfoPage::CErrProxyInfoPage( )
{
}

CErrProxyInfoPage::CErrProxyInfoPage( wxWizard* parent )
{
    Create( parent );
}

/*!
 * CErrProxyInfoPage creator
 */

bool CErrProxyInfoPage::Create( wxWizard* parent )
{
////@begin CErrProxyInfoPage member initialisation
////@end CErrProxyInfoPage member initialisation

////@begin CErrProxyInfoPage creation
    wxBitmap wizardBitmap(wxNullBitmap);
    wxWizardPage::Create( parent, wizardBitmap );

    CreateControls();
    GetSizer()->Fit(this);
////@end CErrProxyInfoPage creation
    return TRUE;
}

/*!
 * Control creation for CErrProxyInfoPage
 */

void CErrProxyInfoPage::CreateControls()
{    
////@begin CErrProxyInfoPage content construction
    CErrProxyInfoPage* itemWizardPage118 = this;

    wxBoxSizer* itemBoxSizer119 = new wxBoxSizer(wxVERTICAL);
    itemWizardPage118->SetSizer(itemBoxSizer119);

    wxStaticText* itemStaticText120 = new wxStaticText;
    itemStaticText120->Create( itemWizardPage118, wxID_STATIC, _("Proxy Configuration"), wxDefaultPosition, wxDefaultSize, 0 );
    itemStaticText120->SetFont(wxFont(10, wxSWISS, wxNORMAL, wxBOLD, FALSE, _T("Verdana")));
    itemBoxSizer119->Add(itemStaticText120, 0, wxALIGN_LEFT|wxALL, 5);

    wxStaticText* itemStaticText121 = new wxStaticText;
    itemStaticText121->Create( itemWizardPage118, wxID_STATIC, _("Do you need to configure a proxy server?"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer119->Add(itemStaticText121, 0, wxALIGN_LEFT|wxALL, 5);

    itemBoxSizer119->Add(5, 5, 0, wxALIGN_LEFT|wxALL, 5);

////@end CErrProxyInfoPage content construction
}

/*!
 * Gets the previous page.
 */

wxWizardPage* CErrProxyInfoPage::GetPrev() const
{
    return PAGE_TRANSITION_BACK;
}

/*!
 * Gets the next page.
 */

wxWizardPage* CErrProxyInfoPage::GetNext() const
{
    return PAGE_TRANSITION_NEXT(ID_ERRPROXYHTTPPAGE);
}

/*!
 * Should we show tooltips?
 */

bool CErrProxyInfoPage::ShowToolTips()
{
    return TRUE;
}

/*!
 * Get bitmap resources
 */

wxBitmap CErrProxyInfoPage::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CErrProxyInfoPage bitmap retrieval
    return wxNullBitmap;
////@end CErrProxyInfoPage bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CErrProxyInfoPage::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CErrProxyInfoPage icon retrieval
    return wxNullIcon;
////@end CErrProxyInfoPage icon retrieval
}

/*!
 * CErrProxyHTTPPage type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CErrProxyHTTPPage, wxWizardPage )

/*!
 * CErrProxyHTTPPage event table definition
 */

BEGIN_EVENT_TABLE( CErrProxyHTTPPage, wxWizardPage )

////@begin CErrProxyHTTPPage event table entries
    EVT_BUTTON( ID_HTTPAUTODETECT, CErrProxyHTTPPage::OnAutodetectClick )

////@end CErrProxyHTTPPage event table entries

END_EVENT_TABLE()

/*!
 * CErrProxyHTTPPage constructors
 */

CErrProxyHTTPPage::CErrProxyHTTPPage( )
{
}

CErrProxyHTTPPage::CErrProxyHTTPPage( wxWizard* parent )
{
    Create( parent );
}

/*!
 * CErrProxyHTTPPage creator
 */

bool CErrProxyHTTPPage::Create( wxWizard* parent )
{
////@begin CErrProxyHTTPPage member initialisation
    m_ProxyHTTPServerStaticCtrl = NULL;
    m_ProxyHTTPServerCtrl = NULL;
    m_ProxyHTTPPortStaticCtrl = NULL;
    m_ProxyHTTPPortCtrl = NULL;
    m_ProxyHTTPUsernameStaticCtrl = NULL;
    m_ProxyHTTPUsernameCtrl = NULL;
    m_ProxyHTTPPasswordStaticCtrl = NULL;
    m_ProxyHTTPPasswordCtrl = NULL;
////@end CErrProxyHTTPPage member initialisation

////@begin CErrProxyHTTPPage creation
    wxBitmap wizardBitmap(wxNullBitmap);
    wxWizardPage::Create( parent, wizardBitmap );

    CreateControls();
    GetSizer()->Fit(this);
////@end CErrProxyHTTPPage creation
    return TRUE;
}

/*!
 * Control creation for CErrProxyHTTPPage
 */

void CErrProxyHTTPPage::CreateControls()
{    
////@begin CErrProxyHTTPPage content construction
    CErrProxyHTTPPage* itemWizardPage123 = this;

    wxBoxSizer* itemBoxSizer124 = new wxBoxSizer(wxVERTICAL);
    itemWizardPage123->SetSizer(itemBoxSizer124);

    wxStaticText* itemStaticText125 = new wxStaticText;
    itemStaticText125->Create( itemWizardPage123, wxID_STATIC, _("Proxy Configuration - HTTP Proxy"), wxDefaultPosition, wxDefaultSize, 0 );
    itemStaticText125->SetFont(wxFont(10, wxSWISS, wxNORMAL, wxBOLD, FALSE, _T("Verdana")));
    itemBoxSizer124->Add(itemStaticText125, 0, wxALIGN_LEFT|wxALL, 5);

    wxStaticText* itemStaticText126 = new wxStaticText;
    itemStaticText126->Create( itemWizardPage123, wxID_STATIC, _("Do you need to configure a proxy server?"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer124->Add(itemStaticText126, 0, wxALIGN_LEFT|wxALL, 5);

    itemBoxSizer124->Add(5, 5, 0, wxALIGN_LEFT|wxALL, 5);

    itemBoxSizer124->Add(5, 5, 0, wxALIGN_LEFT|wxALL, 5);

    wxButton* itemButton129 = new wxButton;
    itemButton129->Create( itemWizardPage123, ID_HTTPAUTODETECT, _("Autodetect"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer124->Add(itemButton129, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxFlexGridSizer* itemFlexGridSizer130 = new wxFlexGridSizer(3, 2, 0, 0);
    itemFlexGridSizer130->AddGrowableCol(1);
    itemBoxSizer124->Add(itemFlexGridSizer130, 0, wxGROW|wxALL, 5);

    m_ProxyHTTPServerStaticCtrl = new wxStaticText;
    m_ProxyHTTPServerStaticCtrl->Create( itemWizardPage123, ID_PROXYHTTPSERVERSTATICCTRL, _("Server:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer130->Add(m_ProxyHTTPServerStaticCtrl, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxFlexGridSizer* itemFlexGridSizer132 = new wxFlexGridSizer(1, 3, 0, 0);
    itemFlexGridSizer132->AddGrowableCol(0);
    itemFlexGridSizer130->Add(itemFlexGridSizer132, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 0);

    m_ProxyHTTPServerCtrl = new wxTextCtrl;
    m_ProxyHTTPServerCtrl->Create( itemWizardPage123, ID_PROXYHTTPSERVERCTRL, _T(""), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer132->Add(m_ProxyHTTPServerCtrl, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_ProxyHTTPPortStaticCtrl = new wxStaticText;
    m_ProxyHTTPPortStaticCtrl->Create( itemWizardPage123, ID_PROXYHTTPPORTSTATICCTRL, _("Port:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer132->Add(m_ProxyHTTPPortStaticCtrl, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_ProxyHTTPPortCtrl = new wxTextCtrl;
    m_ProxyHTTPPortCtrl->Create( itemWizardPage123, ID_PROXYHTTPPORTCTRL, _T(""), wxDefaultPosition, wxSize(50, -1), 0 );
    itemFlexGridSizer132->Add(m_ProxyHTTPPortCtrl, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_ProxyHTTPUsernameStaticCtrl = new wxStaticText;
    m_ProxyHTTPUsernameStaticCtrl->Create( itemWizardPage123, ID_PROXYHTTPUSERNAMESTATICCTRL, _("User Name:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer130->Add(m_ProxyHTTPUsernameStaticCtrl, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_ProxyHTTPUsernameCtrl = new wxTextCtrl;
    m_ProxyHTTPUsernameCtrl->Create( itemWizardPage123, ID_PROXYHTTPUSERNAMECTRL, _T(""), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer130->Add(m_ProxyHTTPUsernameCtrl, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_ProxyHTTPPasswordStaticCtrl = new wxStaticText;
    m_ProxyHTTPPasswordStaticCtrl->Create( itemWizardPage123, ID_PROXYHTTPPASSWORDSTATICCTRL, _("Password:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer130->Add(m_ProxyHTTPPasswordStaticCtrl, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_ProxyHTTPPasswordCtrl = new wxTextCtrl;
    m_ProxyHTTPPasswordCtrl->Create( itemWizardPage123, ID_PROXYHTTPPASSWORDCTRL, _T(""), wxDefaultPosition, wxDefaultSize, wxTE_PASSWORD );
    itemFlexGridSizer130->Add(m_ProxyHTTPPasswordCtrl, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    // Set validators
    m_ProxyHTTPServerCtrl->SetValidator( wxTextValidator(wxFILTER_NONE, & m_strProxyHTTPServer) );
    m_ProxyHTTPPortCtrl->SetValidator( wxTextValidator(wxFILTER_NUMERIC, & m_strProxyHTTPPort) );
    m_ProxyHTTPUsernameCtrl->SetValidator( wxTextValidator(wxFILTER_NONE, & m_strProxyHTTPUsername) );
    m_ProxyHTTPPasswordCtrl->SetValidator( wxTextValidator(wxFILTER_NONE, & m_strProxyHTTPPassword) );
////@end CErrProxyHTTPPage content construction
}

/*!
 * Gets the previous page.
 */

wxWizardPage* CErrProxyHTTPPage::GetPrev() const
{
    return PAGE_TRANSITION_BACK;
}

/*!
 * Gets the next page.
 */

wxWizardPage* CErrProxyHTTPPage::GetNext() const
{
    return PAGE_TRANSITION_NEXT(ID_ERRPROXYSOCKSPAGE);
}

/*!
 * Should we show tooltips?
 */

bool CErrProxyHTTPPage::ShowToolTips()
{
    return TRUE;
}

/*!
 * Get bitmap resources
 */

wxBitmap CErrProxyHTTPPage::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CErrProxyHTTPPage bitmap retrieval
    return wxNullBitmap;
////@end CErrProxyHTTPPage bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CErrProxyHTTPPage::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CErrProxyHTTPPage icon retrieval
    return wxNullIcon;
////@end CErrProxyHTTPPage icon retrieval
}

/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_HTTPAUTODETECT
 */

void CErrProxyHTTPPage::OnAutodetectClick( wxCommandEvent& event )
{
    event.Skip();
}


/*!
 * CErrProxySOCKSPage type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CErrProxySOCKSPage, wxWizardPage )

/*!
 * CErrProxySOCKSPage event table definition
 */

BEGIN_EVENT_TABLE( CErrProxySOCKSPage, wxWizardPage )

////@begin CErrProxySOCKSPage event table entries
    EVT_BUTTON( ID_SOCKSAUTODETECT, CErrProxySOCKSPage::OnAutodetectClick )

////@end CErrProxySOCKSPage event table entries

END_EVENT_TABLE()

/*!
 * CErrProxySOCKSPage constructors
 */

CErrProxySOCKSPage::CErrProxySOCKSPage( )
{
}

CErrProxySOCKSPage::CErrProxySOCKSPage( wxWizard* parent )
{
    Create( parent );
}

/*!
 * CErrProxyInfoPage creator
 */

bool CErrProxySOCKSPage::Create( wxWizard* parent )
{
////@begin CErrProxySOCKSPage member initialisation
    m_ProxySOCKSServerStaticCtrl = NULL;
    m_ProxySOCKSServerCtrl = NULL;
    m_ProxySOCKSPortStaticCtrl = NULL;
    m_ProxySOCKSPortCtrl = NULL;
    m_ProxySOCKSUsernameStaticCtrl = NULL;
    m_ProxySOCKSUsernameCtrl = NULL;
    m_ProxySOCKSPasswordStaticCtrl = NULL;
    m_ProxySOCKSPasswordCtrl = NULL;
////@end CErrProxySOCKSPage member initialisation

////@begin CErrProxySOCKSPage creation
    wxBitmap wizardBitmap(wxNullBitmap);
    wxWizardPage::Create( parent, wizardBitmap );

    CreateControls();
    GetSizer()->Fit(this);
////@end CErrProxySOCKSPage creation
    return TRUE;
}

/*!
 * Control creation for CErrProxyInfoPage
 */

void CErrProxySOCKSPage::CreateControls()
{    
////@begin CErrProxySOCKSPage content construction
    CErrProxySOCKSPage* itemWizardPage140 = this;

    wxBoxSizer* itemBoxSizer141 = new wxBoxSizer(wxVERTICAL);
    itemWizardPage140->SetSizer(itemBoxSizer141);

    wxStaticText* itemStaticText142 = new wxStaticText;
    itemStaticText142->Create( itemWizardPage140, wxID_STATIC, _("Proxy Configuration - SOCKS Proxy"), wxDefaultPosition, wxDefaultSize, 0 );
    itemStaticText142->SetFont(wxFont(10, wxSWISS, wxNORMAL, wxBOLD, FALSE, _T("Verdana")));
    itemBoxSizer141->Add(itemStaticText142, 0, wxALIGN_LEFT|wxALL, 5);

    wxStaticText* itemStaticText143 = new wxStaticText;
    itemStaticText143->Create( itemWizardPage140, wxID_STATIC, _("Do you need to configure a proxy server?"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer141->Add(itemStaticText143, 0, wxALIGN_LEFT|wxALL, 5);

    itemBoxSizer141->Add(5, 5, 0, wxALIGN_LEFT|wxALL, 5);

    itemBoxSizer141->Add(5, 5, 0, wxALIGN_LEFT|wxALL, 5);

    wxButton* itemButton146 = new wxButton;
    itemButton146->Create( itemWizardPage140, ID_SOCKSAUTODETECT, _("Autodetect"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer141->Add(itemButton146, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxFlexGridSizer* itemFlexGridSizer147 = new wxFlexGridSizer(3, 2, 0, 0);
    itemFlexGridSizer147->AddGrowableCol(1);
    itemBoxSizer141->Add(itemFlexGridSizer147, 0, wxGROW|wxALL, 5);

    m_ProxySOCKSServerStaticCtrl = new wxStaticText;
    m_ProxySOCKSServerStaticCtrl->Create( itemWizardPage140, ID_PROXYSOCKSSERVERSTATICCTRL, _("Server:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer147->Add(m_ProxySOCKSServerStaticCtrl, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxFlexGridSizer* itemFlexGridSizer149 = new wxFlexGridSizer(1, 3, 0, 0);
    itemFlexGridSizer149->AddGrowableCol(0);
    itemFlexGridSizer147->Add(itemFlexGridSizer149, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 0);

    m_ProxySOCKSServerCtrl = new wxTextCtrl;
    m_ProxySOCKSServerCtrl->Create( itemWizardPage140, ID_PROXYSOCKSSERVERCTRL, _T(""), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer149->Add(m_ProxySOCKSServerCtrl, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_ProxySOCKSPortStaticCtrl = new wxStaticText;
    m_ProxySOCKSPortStaticCtrl->Create( itemWizardPage140, ID_PROXYSOCKSPORTSTATICCTRL, _("Port:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer149->Add(m_ProxySOCKSPortStaticCtrl, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_ProxySOCKSPortCtrl = new wxTextCtrl;
    m_ProxySOCKSPortCtrl->Create( itemWizardPage140, ID_PROXYSOCKSPORTCTRL, _T(""), wxDefaultPosition, wxSize(50, -1), 0 );
    itemFlexGridSizer149->Add(m_ProxySOCKSPortCtrl, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_ProxySOCKSUsernameStaticCtrl = new wxStaticText;
    m_ProxySOCKSUsernameStaticCtrl->Create( itemWizardPage140, ID_PROXYSOCKSUSERNAMESTATICCTRL, _("User Name:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer147->Add(m_ProxySOCKSUsernameStaticCtrl, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_ProxySOCKSUsernameCtrl = new wxTextCtrl;
    m_ProxySOCKSUsernameCtrl->Create( itemWizardPage140, ID_PROXYSOCKSUSERNAMECTRL, _T(""), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer147->Add(m_ProxySOCKSUsernameCtrl, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_ProxySOCKSPasswordStaticCtrl = new wxStaticText;
    m_ProxySOCKSPasswordStaticCtrl->Create( itemWizardPage140, ID_PROXYSOCKSPASSWORDSTATICCTRL, _("Password:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer147->Add(m_ProxySOCKSPasswordStaticCtrl, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_ProxySOCKSPasswordCtrl = new wxTextCtrl;
    m_ProxySOCKSPasswordCtrl->Create( itemWizardPage140, ID_PROXYSOCKSPASSWORDCTRL, _T(""), wxDefaultPosition, wxDefaultSize, wxTE_PASSWORD );
    itemFlexGridSizer147->Add(m_ProxySOCKSPasswordCtrl, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    // Set validators
    m_ProxySOCKSServerCtrl->SetValidator( wxTextValidator(wxFILTER_NONE, & m_strProxySOCKSServer) );
    m_ProxySOCKSPortCtrl->SetValidator( wxTextValidator(wxFILTER_NUMERIC, & m_strProxySOCKSPort) );
    m_ProxySOCKSUsernameCtrl->SetValidator( wxTextValidator(wxFILTER_NONE, & m_strProxySOCKSUsername) );
    m_ProxySOCKSPasswordCtrl->SetValidator( wxTextValidator(wxFILTER_NONE, & m_strProxySOCKSPassword) );
////@end CErrProxySOCKSPage content construction
}

/*!
 * Gets the previous page.
 */

wxWizardPage* CErrProxySOCKSPage::GetPrev() const
{
    return PAGE_TRANSITION_BACK;
}

/*!
 * Gets the next page.
 */

wxWizardPage* CErrProxySOCKSPage::GetNext() const
{
    return PAGE_TRANSITION_NEXT(ID_ERRPROXYCOMPLETIONPAGE);
}

/*!
 * Should we show tooltips?
 */

bool CErrProxySOCKSPage::ShowToolTips()
{
    return TRUE;
}

/*!
 * Get bitmap resources
 */

wxBitmap CErrProxySOCKSPage::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CErrProxySOCKSPage bitmap retrieval
    return wxNullBitmap;
////@end CErrProxySOCKSPage bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CErrProxySOCKSPage::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CErrProxySOCKSPage icon retrieval
    return wxNullIcon;
////@end CErrProxySOCKSPage icon retrieval
}

/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_SOCKSAUTODETECT
 */

void CErrProxySOCKSPage::OnAutodetectClick( wxCommandEvent& event )
{
    event.Skip();
}


/*!
 * CErrProxyComplationPage type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CErrProxyComplationPage, wxWizardPage )

/*!
 * CErrProxyComplationPage event table definition
 */

BEGIN_EVENT_TABLE( CErrProxyComplationPage, wxWizardPage )

////@begin CErrProxyComplationPage event table entries
////@end CErrProxyComplationPage event table entries

END_EVENT_TABLE()

/*!
 * CErrProxyComplationPage constructors
 */

CErrProxyComplationPage::CErrProxyComplationPage( )
{
}

CErrProxyComplationPage::CErrProxyComplationPage( wxWizard* parent )
{
    Create( parent );
}

/*!
 * CErrProxyComplationPage creator
 */

bool CErrProxyComplationPage::Create( wxWizard* parent )
{
////@begin CErrProxyComplationPage member initialisation
////@end CErrProxyComplationPage member initialisation

////@begin CErrProxyComplationPage creation
    wxBitmap wizardBitmap(wxNullBitmap);
    wxWizardPage::Create( parent, wizardBitmap );

    CreateControls();
    GetSizer()->Fit(this);
////@end CErrProxyComplationPage creation
    return TRUE;
}

/*!
 * Control creation for CErrProxyComplationPage
 */

void CErrProxyComplationPage::CreateControls()
{    
////@begin CErrProxyComplationPage content construction
    CErrProxyComplationPage* itemWizardPage157 = this;

    wxBoxSizer* itemBoxSizer158 = new wxBoxSizer(wxVERTICAL);
    itemWizardPage157->SetSizer(itemBoxSizer158);

    wxStaticText* itemStaticText159 = new wxStaticText;
    itemStaticText159->Create( itemWizardPage157, wxID_STATIC, _("Proxy Configuration Completion"), wxDefaultPosition, wxDefaultSize, 0 );
    itemStaticText159->SetFont(wxFont(10, wxSWISS, wxNORMAL, wxBOLD, FALSE, _T("Verdana")));
    itemBoxSizer158->Add(itemStaticText159, 0, wxALIGN_LEFT|wxALL, 5);

    wxStaticText* itemStaticText160 = new wxStaticText;
    itemStaticText160->Create( itemWizardPage157, wxID_STATIC, _("Do you need to configure a proxy server?"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer158->Add(itemStaticText160, 0, wxALIGN_LEFT|wxALL, 5);

    itemBoxSizer158->Add(5, 5, 0, wxALIGN_LEFT|wxALL, 5);

////@end CErrProxyComplationPage content construction
}

/*!
 * Gets the previous page.
 */

wxWizardPage* CErrProxyComplationPage::GetPrev() const
{
    return PAGE_TRANSITION_BACK;
}

/*!
 * Gets the next page.
 */

wxWizardPage* CErrProxyComplationPage::GetNext() const
{
    return PAGE_TRANSITION_NEXT(ID_PROJECTPROPERTIESPAGE);
}

/*!
 * Should we show tooltips?
 */

bool CErrProxyComplationPage::ShowToolTips()
{
    return TRUE;
}

/*!
 * Get bitmap resources
 */

wxBitmap CErrProxyComplationPage::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CErrProxyComplationPage bitmap retrieval
    return wxNullBitmap;
////@end CErrProxyComplationPage bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CErrProxyComplationPage::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CErrProxyComplationPage icon retrieval
    return wxNullIcon;
////@end CErrProxyComplationPage icon retrieval
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
    CErrRefCountPage* itemWizardPage162 = this;

    wxBoxSizer* itemBoxSizer163 = new wxBoxSizer(wxVERTICAL);
    itemWizardPage162->SetSizer(itemBoxSizer163);

    wxStaticText* itemStaticText164 = new wxStaticText;
    itemStaticText164->Create( itemWizardPage162, wxID_STATIC, _("Ref Count Page"), wxDefaultPosition, wxDefaultSize, 0 );
    itemStaticText164->SetFont(wxFont(10, wxSWISS, wxNORMAL, wxBOLD, FALSE, _T("Verdana")));
    itemBoxSizer163->Add(itemStaticText164, 0, wxALIGN_LEFT|wxALL, 5);

    wxStaticText* itemStaticText165 = new wxStaticText;
    itemStaticText165->Create( itemWizardPage162, wxID_STATIC, _("This page should never be used in the wizard itself."), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer163->Add(itemStaticText165, 0, wxALIGN_LEFT|wxALL, 5);

    itemBoxSizer163->Add(5, 5, 0, wxALIGN_LEFT|wxALL, 5);

    wxStaticText* itemStaticText167 = new wxStaticText;
    itemStaticText167->Create( itemWizardPage162, wxID_STATIC, _("This page just increases the refcount of various bitmap resources\nso that DialogBlocks doesn't nuke the refences to them."), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer163->Add(itemStaticText167, 0, wxALIGN_LEFT|wxALL, 5);

    wxBoxSizer* itemBoxSizer168 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer163->Add(itemBoxSizer168, 0, wxALIGN_LEFT|wxALL, 5);

    wxBitmap itemStaticBitmap169Bitmap(itemWizardPage162->GetBitmapResource(wxT("res/wizfailure.xpm")));
    wxStaticBitmap* itemStaticBitmap169 = new wxStaticBitmap;
    itemStaticBitmap169->Create( itemWizardPage162, wxID_STATIC, itemStaticBitmap169Bitmap, wxDefaultPosition, wxSize(16, 16), 0 );
    itemBoxSizer168->Add(itemStaticBitmap169, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxBitmap itemStaticBitmap170Bitmap(itemWizardPage162->GetBitmapResource(wxT("res/wizquestion.xpm")));
    wxStaticBitmap* itemStaticBitmap170 = new wxStaticBitmap;
    itemStaticBitmap170->Create( itemWizardPage162, wxID_STATIC, itemStaticBitmap170Bitmap, wxDefaultPosition, wxSize(16, 16), 0 );
    itemBoxSizer168->Add(itemStaticBitmap170, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxBitmap itemStaticBitmap171Bitmap(itemWizardPage162->GetBitmapResource(wxT("res/wizsuccess.xpm")));
    wxStaticBitmap* itemStaticBitmap171 = new wxStaticBitmap;
    itemStaticBitmap171->Create( itemWizardPage162, wxID_STATIC, itemStaticBitmap171Bitmap, wxDefaultPosition, wxSize(16, 16), 0 );
    itemBoxSizer168->Add(itemStaticBitmap171, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

////@end CErrRefCountPage content construction
}

/*!
 * Gets the previous page.
 */

wxWizardPage* CErrRefCountPage::GetPrev() const
{
    // TODO: return the previous page
    return NULL;
}

/*!
 * Gets the next page.
 */

wxWizardPage* CErrRefCountPage::GetNext() const
{
    // TODO: return the next page
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
    if (name == wxT("res/wizfailure.xpm"))
    {
        wxBitmap bitmap(wizfailure_xpm);
        return bitmap;
    }
    else if (name == wxT("res/wizquestion.xpm"))
    {
        wxBitmap bitmap(wizquestion_xpm);
        return bitmap;
    }
    else if (name == wxT("res/wizsuccess.xpm"))
    {
        wxBitmap bitmap(wizsuccess_xpm);
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

