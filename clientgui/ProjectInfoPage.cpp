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
#pragma implementation "ProjectInfoPage.h"
#endif

#include "stdwx.h"
#include "diagnostics.h"
#include "util.h"
#include "mfile.h"
#include "miofile.h"
#include "parse.h"
#include "url.h"
#include "error_numbers.h"

#include "wizardex.h"
#include "error_numbers.h"
#include "BOINCGUIApp.h"
#include "SkinManager.h"
#include "MainDocument.h"
#include "ValidateURL.h"
#include "BOINCWizards.h"
#include "BOINCBaseWizard.h"
#include "WizardAttachProject.h"
#include "ProjectInfoPage.h"
#include "ProjectListCtrl.h"


/*!
 * CProjectInfoPage type definition
 */
 
IMPLEMENT_DYNAMIC_CLASS( CProjectInfoPage, wxWizardPageEx )
 
/*!
 * CProjectInfoPage event table definition
 */
 
BEGIN_EVENT_TABLE( CProjectInfoPage, wxWizardPageEx )
 
////@begin CProjectInfoPage event table entries
    EVT_WIZARDEX_PAGE_CHANGED( -1, CProjectInfoPage::OnPageChanged )
    EVT_WIZARDEX_PAGE_CHANGING( -1, CProjectInfoPage::OnPageChanging )
    EVT_PROJECTLIST_ITEM_CHANGE( CProjectInfoPage::OnProjectItemChange )
    EVT_PROJECTLIST_ITEM_DISPLAY( CProjectInfoPage::OnProjectItemDisplay )
    EVT_WIZARDEX_CANCEL( -1, CProjectInfoPage::OnCancel )
////@end CProjectInfoPage event table entries
 
END_EVENT_TABLE()


/*!
 * CProjectInfoPage constructors
 */
 
CProjectInfoPage::CProjectInfoPage( )
{
}

CProjectInfoPage::CProjectInfoPage( CBOINCBaseWizard* parent )
{
    Create( parent );
}


/*!
 * CProjectInfoPage creator
 */
 
bool CProjectInfoPage::Create( CBOINCBaseWizard* parent )
{
////@begin CProjectInfoPage member initialisation
    m_pTitleStaticCtrl = NULL;
    m_pDescriptionStaticCtrl = NULL;
    m_pProjectListCtrl = NULL;
    m_pProjectUrlStaticCtrl = NULL;
    m_pProjectUrlCtrl = NULL;
////@end CProjectInfoPage member initialisation
    m_strProjectURL = wxEmptyString;
    m_bProjectSupported = false;
    m_bProjectListPopulated = false;
 
////@begin CProjectInfoPage creation
    wxWizardPageEx::Create( parent, ID_PROJECTINFOPAGE );

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

    m_pTitleStaticCtrl = new wxStaticText;
    m_pTitleStaticCtrl->Create( itemWizardPage23, wxID_STATIC, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    m_pTitleStaticCtrl->SetFont(wxFont(10, wxSWISS, wxNORMAL, wxBOLD, FALSE, _T("Verdana")));
    itemBoxSizer24->Add(m_pTitleStaticCtrl, 0, wxALIGN_LEFT|wxALL, 5);

    m_pDescriptionStaticCtrl = new wxStaticText;
    m_pDescriptionStaticCtrl->Create( itemWizardPage23, wxID_STATIC, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer24->Add(m_pDescriptionStaticCtrl, 0, wxALIGN_LEFT|wxALL, 5);

    wxFlexGridSizer* itemFlexGridSizer3 = new wxFlexGridSizer(2, 1, 0, 0);
    itemFlexGridSizer3->AddGrowableRow(0);
    itemFlexGridSizer3->AddGrowableCol(0);
    itemBoxSizer24->Add(itemFlexGridSizer3, 1, wxGROW|wxALL, 5);

    m_pProjectListCtrl = new CProjectListCtrl;
    m_pProjectListCtrl->Create( itemWizardPage23 );
    itemFlexGridSizer3->Add(m_pProjectListCtrl, 0, wxGROW|wxRIGHT, 10);

    wxFlexGridSizer* itemFlexGridSizer11 = new wxFlexGridSizer(2, 1, 0, 0);
    itemFlexGridSizer11->AddGrowableRow(0);
    itemFlexGridSizer11->AddGrowableCol(0);
    itemBoxSizer24->Add(itemFlexGridSizer11, 0, wxGROW|wxALL, 0);

    wxBoxSizer* itemBoxSizer22 = new wxBoxSizer(wxVERTICAL);
    itemFlexGridSizer11->Add(itemBoxSizer22, 0, wxGROW|wxALL, 0);

    wxFlexGridSizer* itemFlexGridSizer14 = new wxFlexGridSizer(1, 2, 0, 0);
    itemFlexGridSizer14->AddGrowableCol(1);
    itemBoxSizer24->Add(itemFlexGridSizer14, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxRIGHT, 10);

    m_pProjectUrlStaticCtrl = new wxStaticText;
    m_pProjectUrlStaticCtrl->Create( itemWizardPage23, ID_PROJECTURLSTATICCTRL, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer14->Add(m_pProjectUrlStaticCtrl, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_pProjectUrlCtrl = new wxTextCtrl;
    m_pProjectUrlCtrl->Create( itemWizardPage23, ID_PROJECTURLCTRL, wxEmptyString, wxDefaultPosition, wxSize(200, -1), 0 );
    itemFlexGridSizer14->Add(m_pProjectUrlCtrl, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    // Set validators
    m_pProjectUrlCtrl->SetValidator( CValidateURL( & m_strProjectURL ) );

#ifdef __WXMAC__
    //Accessibility
    HIViewRef listView = (HIViewRef)m_pProjectListCtrl->GetHandle();
    HIObjectRef   theObject = (HIObjectRef)HIViewGetSuperview(listView);
    HIObjectSetAccessibilityIgnored(theObject, true);
#endif
    ////@end CProjectInfoPage content construction
}


/*!
 * Gets the previous page.
 */
wxWizardPageEx* CProjectInfoPage::GetPrev() const
{
    return PAGE_TRANSITION_BACK;
}


/*!
 * Gets the next page.
 */
 
wxWizardPageEx* CProjectInfoPage::GetNext() const
{
    if (CHECK_CLOSINGINPROGRESS()) {
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
 
wxBitmap CProjectInfoPage::GetBitmapResource( const wxString& WXUNUSED(name) )
{
    // Bitmap retrieval
////@begin CProjectInfoPage bitmap retrieval
    return wxNullBitmap;
////@end CProjectInfoPage bitmap retrieval
}


/*!
 * Get icon resources
 */
 
wxIcon CProjectInfoPage::GetIconResource( const wxString& WXUNUSED(name) )
{
    // Icon retrieval
////@begin CProjectInfoPage icon retrieval
    return wxNullIcon;
////@end CProjectInfoPage icon retrieval
}


/*!
 * wxEVT_WIZARD_PAGE_CHANGED event handler for ID_PROJECTINFOPAGE
 */

void CProjectInfoPage::OnPageChanged( wxWizardExEvent& event ) {
    if (event.GetDirection() == false) return;
    wxLogTrace(wxT("Function Start/End"), wxT("CProjectInfoPage::OnPageChanged - Function Begin"));

    unsigned int                i, j, k;
    bool                        bSupportedPlatformFound = false;
    bool                        bProjectSupportsNvidiaGPU = false;
    bool                        bProjectSupportsATIGPU = false;
    bool                        bProjectSupportsMulticore = false;
    ALL_PROJECTS_LIST           pl;
    CMainDocument*              pDoc = wxGetApp().GetDocument();

    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));
    wxASSERT(m_pTitleStaticCtrl);
    wxASSERT(m_pDescriptionStaticCtrl);
    wxASSERT(m_pProjectListCtrl);
    wxASSERT(m_pProjectUrlStaticCtrl);
    wxASSERT(m_pProjectUrlCtrl);

    m_pTitleStaticCtrl->SetLabel(
        _("Choose a project")
    );
    m_pDescriptionStaticCtrl->SetLabel(
        _("To choose a project, click its name or type its URL below.")
    );
    m_pProjectUrlStaticCtrl->SetLabel(
        _("Project &URL:")
    );


    // Populate the virtual list control with project information
    //
    if (!m_bProjectListPopulated) {
        std::vector<std::string> &client_platforms = pDoc->state.platforms;
        pDoc->rpc.get_all_projects_list(pl);

        for (i=0; i<pl.projects.size(); i++) {
            bSupportedPlatformFound = false;
            bProjectSupportsNvidiaGPU = false;
            bProjectSupportsATIGPU = false;
            bProjectSupportsMulticore = false;

            wxLogTrace(
                wxT("Function Status"),
                wxT("CProjectInfoPage::OnPageChanged - Name: '%s', URL: '%s'"),
                wxString(pl.projects[i]->name.c_str(), wxConvUTF8).c_str(),
                wxString(pl.projects[i]->url.c_str(), wxConvUTF8).c_str()
            );

            std::vector<std::string> &project_platforms = pl.projects[i]->platforms;

            // Can the core client support a platform that this project
            //   supports?
            for (j = 0;j < client_platforms.size(); j++) {
                for (k = 0;k < project_platforms.size(); k++) {
                    wxString strClientPlatform = wxString(client_platforms[j].c_str(), wxConvUTF8);
                    wxString strProjectPlatform = wxString(project_platforms[k].c_str(), wxConvUTF8);
                    wxString strRootProjectPlatform = strProjectPlatform.SubString(0, strProjectPlatform.Find(_T("[")) - 1);
                    
                    if (strProjectPlatform.Find(_T("[cuda")) != wxNOT_FOUND) {
                        if ((pDoc->state.have_cuda) && (strClientPlatform == strRootProjectPlatform)) {
                            bProjectSupportsNvidiaGPU = true;
                            bSupportedPlatformFound = true;
                        }
                    }

                    if (strProjectPlatform.Find(_T("[ati")) != wxNOT_FOUND) {
                        if ((pDoc->state.have_ati) && (strClientPlatform == strRootProjectPlatform)) {
                            bProjectSupportsATIGPU = true;
                            bSupportedPlatformFound = true;
                        }
                    }

                    if (strProjectPlatform.Find(_T("[mt")) != wxNOT_FOUND) {
                        if ((pDoc->host.p_ncpus >= 4) && (strClientPlatform == strRootProjectPlatform)) {
                            bProjectSupportsMulticore = true;
                            bSupportedPlatformFound = true;
                        }
                    }

                    if (strClientPlatform == strRootProjectPlatform) {
                        bSupportedPlatformFound = true;
                    }
                }
            }

            wxLogTrace(
                wxT("Function Status"),
                wxT("CProjectInfoPage::OnPageChanged - Nvidia: '%d', ATI: '%d', Multicore: '%d', Platform: '%d'"),
                bProjectSupportsNvidiaGPU,
                bProjectSupportsATIGPU,
                bProjectSupportsMulticore,
                bSupportedPlatformFound
            );

            m_pProjectListCtrl->Append(
                wxString(pl.projects[i]->url.c_str(), wxConvUTF8),
                wxString(pl.projects[i]->name.c_str(), wxConvUTF8),
                wxString(pl.projects[i]->image.c_str(), wxConvUTF8),
                wxString(pl.projects[i]->description.c_str(), wxConvUTF8),
                bProjectSupportsNvidiaGPU,
                bProjectSupportsATIGPU,
                bProjectSupportsMulticore,
                bSupportedPlatformFound
            );
        }

        // Pre select the first element
        m_pProjectListCtrl->SetSelection(0);
        m_strProjectURL = m_pProjectListCtrl->GetItem(0)->GetURL();
        m_bProjectSupported = m_pProjectListCtrl->GetItem(0)->IsPlatformSupported();

        TransferDataToWindow();

        m_bProjectListPopulated = true;
    }

    Layout();
    FitInside();
    m_pProjectListCtrl->SetFocus();

    wxLogTrace(wxT("Function Start/End"), wxT("CProjectInfoPage::OnPageChanged - Function End"));
}


/*!
 * wxEVT_WIZARD_PAGE_CHANGING event handler for ID_PROJECTINFOPAGE
 */

void CProjectInfoPage::OnPageChanging( wxWizardExEvent& event ) {
    if (event.GetDirection() == false) return;
    wxLogTrace(wxT("Function Start/End"), wxT("CProjectInfoPage::OnPageChanging - Function Begin"));

    CMainDocument* pDoc = wxGetApp().GetDocument(); 
    CSkinAdvanced* pSkinAdvanced = wxGetApp().GetSkinManager()->GetAdvanced();
    wxString       strTitle;
    int            iAnswer;

    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));
    wxASSERT(pSkinAdvanced);
    wxASSERT(wxDynamicCast(pSkinAdvanced, CSkinAdvanced));


    strTitle.Printf(
        wxT("%s"), 
        pSkinAdvanced->GetApplicationName().c_str()
    );


    // Check to see if the project is supported:
    if ( !GetProjectSupported() ) {

        iAnswer = wxGetApp().SafeMessageBox(
            _("This project may not have work for your type of computer. Are you sure you wish to continue?"),
            strTitle,
            wxCENTER | wxYES_NO | wxICON_INFORMATION
        );

        // Project is not supported
        if (wxNO == iAnswer) {
            event.Veto();
        }

    } else {

        // Check if we are already attached to that project: 
 	    for (int i = 0; i < pDoc->GetProjectCount(); ++i) { 
 	        PROJECT* project = pDoc->project(i);
            if (project) {
                std::string project_url = project->master_url;
                std::string new_project_url = (const char*)m_strProjectURL.mb_str();

                canonicalize_master_url(project_url);
                canonicalize_master_url(new_project_url);
                
                if (project_url == new_project_url) {
                    wxGetApp().SafeMessageBox(
                        _("You are already attached to this project. Please choose a different project."),
                        strTitle,
                        wxCENTER | wxICON_INFORMATION
                    );

                    // We are already attached to that project, 
                    event.Veto();
                    break;
                }
            } 
        } 
    }

    wxLogTrace(wxT("Function Start/End"), wxT("CProjectInfoPage::OnPageChanging - Function End"));
}


/*!
 * wxEVT_PROJECTLIST_ITEM_CHANGE event handler for ID_PROJECTSELECTIONCTRL
 */

void CProjectInfoPage::OnProjectItemChange( ProjectListCtrlEvent& event ) {
    SetProjectURL( event.GetURL() );
    SetProjectSupported( event.IsSupported() );
    TransferDataToWindow();
}


/*!
 * wxEVT_PROJECTLIST_ITEM_DISPLAY event handler for ID_PROJECTSELECTIONCTRL
 */

void CProjectInfoPage::OnProjectItemDisplay( ProjectListCtrlEvent& event ) {
    wxLaunchDefaultBrowser( event.GetURL() );
}


/*!
 * wxEVT_WIZARD_CANCEL event handler for ID_PROJECTINFOPAGE
 */

void CProjectInfoPage::OnCancel( wxWizardExEvent& event ) {
    PROCESS_CANCELEVENT(event);
}

