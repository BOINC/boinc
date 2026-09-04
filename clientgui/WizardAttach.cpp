// This file is part of BOINC.
// https://boinc.berkeley.edu
// Copyright (C) 2026 University of California
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

#include "stdwx.h"
#include "diagnostics.h"
#include "util.h"
#include "mfile.h"
#include "miofile.h"
#include "parse.h"
#include "error_numbers.h"
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
#include "NotFoundPage.h"
#include "AlreadyExistsPage.h"
#include "ProxyInfoPage.h"
#include "ProxyPage.h"

IMPLEMENT_DYNAMIC_CLASS(CWizardAttach, CBOINCBaseWizard)

BEGIN_EVENT_TABLE(CWizardAttach, CBOINCBaseWizard)
EVT_WIZARD_FINISHED(ID_ATTACHWIZARD, CWizardAttach::OnFinished)
EVT_BUTTON(wxID_BACKWARD, CWizardAttach::OnWizardBack)
EVT_BUTTON(wxID_FORWARD, CWizardAttach::OnWizardNext)
END_EVENT_TABLE()

CWizardAttach::CWizardAttach() {
}

CWizardAttach::CWizardAttach(wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, long style) {
    Create(parent, id, title, pos, style);
}

bool CWizardAttach::Create(wxWindow* parent, wxWindowID id, const wxString&, const wxPoint& pos, long style) {
    m_ProjectInfoPage = nullptr;
    m_ProjectPropertiesPage = nullptr;
    m_ProjectProcessingPage = nullptr;
    m_ProjectWelcomePage = nullptr;
    m_AccountManagerInfoPage = nullptr;
    m_AccountManagerPropertiesPage = nullptr;
    m_AccountManagerProcessingPage = nullptr;
    m_TermsOfUsePage = nullptr;
    m_AccountInfoPage = nullptr;
    m_CompletionPage = nullptr;
    m_CompletionErrorPage = nullptr;
    m_ErrNotDetectedPage = nullptr;
    m_ErrUnavailablePage = nullptr;
    m_ErrNotFoundPage = nullptr;
    m_ErrAlreadyExistsPage = nullptr;
    m_ErrProxyInfoPage = nullptr;
    m_ErrProxyPage = nullptr;
    m_ErrUserDisagreesPage = nullptr;

    m_bCancelInProgress = false;

    IsAttachToProjectWizard = true;
    IsAccountManagerWizard = false;
    IsAccountManagerUpdateWizard = false;

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

    m_direction = 0;

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

    CBOINCBaseWizard::Create( parent, id, strTitle, wxBitmapBundle(), pos, style );

    CreateControls();

    return true;
}

void CWizardAttach::CreateControls() {
    wxLogTrace(wxT("Function Start/End"), wxT("CWizardAttach::CreateControls - Function Begin"));

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

    wxLogTrace(wxT("Function Start/End"), wxT("CWizardAttach::CreateControls - Function End"));
}

bool CWizardAttach::Run(
        wxString strProjectName,
        wxString strProjectURL,
        wxString strProjectAuthenticator,
        wxString strProjectInstitution,
        wxString strProjectDescription,
        wxString strProjectKnown,
        bool     bAccountKeyDetected,
        bool     /* bEmbedded */
) {
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

    return false;
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

    return false;
}

bool CWizardAttach::ShowToolTips() {
    return true;
}

wxBitmap CWizardAttach::GetBitmapResource(const wxString& WXUNUSED(name)) {
    return wxNullBitmap;
}

wxIcon CWizardAttach::GetIconResource(const wxString& WXUNUSED(name)) {
    return wxNullIcon;
}

bool CWizardAttach::HasNextPage(wxWizardPage* page) {
    if (!page) {
        return false;
    }
    return dynamic_cast<CBOINCWizardPage*>(page)->HasNextPage();
}

bool CWizardAttach::HasPrevPage(wxWizardPage* page) {
    if (!page) {
        return false;
    }
    return dynamic_cast<CBOINCWizardPage*>(page)->HasPrevPage();
}

void CWizardAttach::_ProcessCancelEvent(wxWizardEvent& event) {
    wxWizardPage* page = GetCurrentPage();

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

void CWizardAttach::OnFinished(wxWizardEvent& event) {
    if (IsAccountManagerWizard) {
        // Attached to an account manager
        if (!GetReturnURL().empty() && GetAttachedToProjectSuccessfully()) {
            wxLaunchDefaultBrowser(GetReturnURL());
        }
    } else {
        // Attached to a project
        if (GetAccountCreatedSuccessfully() && GetAttachedToProjectSuccessfully()) {
            wxLaunchDefaultBrowser(GetProjectURL() + wxT("account_finish.php"));
        }
    }

    // Let the framework clean things up.
    event.Skip();
}

void CWizardAttach::OnWizardBack(wxCommandEvent& event) {
    m_direction = -1;
    event.Skip();
}

void CWizardAttach::OnWizardNext(wxCommandEvent& event) {
    m_direction = 1;
    event.Skip();
}

