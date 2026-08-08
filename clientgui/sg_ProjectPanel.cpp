// This file is part of BOINC.
// https://boinc.berkeley.edu
// Copyright (C) 2025 University of California
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

#define TESTBIGICONPOPUP 0

#define SORTPROJECTLIST 1

#include "stdwx.h"

#include "app_ipc.h"
#include "str_replace.h"

#include "Events.h"
#include "BOINCGUIApp.h"
#include "MainDocument.h"
#include "SkinManager.h"
#include "wizardex.h"
#include "BOINCBaseWizard.h"
#include "WizardAttach.h"
#include "sg_ProjectPanel.h"
#if TESTBIGICONPOPUP
#include "test/sah_40.xpm"
#include "test/einstein_icon.xpm"
#endif

IMPLEMENT_DYNAMIC_CLASS(CSimpleProjectPanel, CSimplePanelBase)

BEGIN_EVENT_TABLE(CSimpleProjectPanel, CSimplePanelBase)
#ifdef __WXMAC__
    EVT_CHOICE(ID_SGPROJECTSELECTOR, CSimpleProjectPanel::OnProjectSelection)
#else
    EVT_COMBOBOX(ID_SGPROJECTSELECTOR, CSimpleProjectPanel::OnProjectSelection)
#endif
    EVT_BUTTON(ID_ADDROJECTBUTTON, CSimpleProjectPanel::OnAddProject)
#if TESTBIGICONPOPUP
    EVT_BUTTON(ID_PROJECTWEBSITESBUTTON, CSimpleProjectPanel::OnProjectWebSiteButton)
    EVT_BUTTON(ID_PROJECTCOMMANDBUTTON, CSimpleProjectPanel::OnProjectCommandButton)
#endif
END_EVENT_TABLE()


#if TESTBIGICONPOPUP
static wxString tempArray[] = {_T("String1"), _T("String2"), _T("String3"), _T("String4") };
static wxBitmap bmArray[3];
#endif

CSimpleProjectPanel::CSimpleProjectPanel() {
}


CSimpleProjectPanel::CSimpleProjectPanel( wxWindow* parent ) :
    CSimplePanelBase( parent )
{
    wxString str = wxEmptyString;
    wxWindowDC dc(this);

    m_UsingAccountManager = -1; // Force action the first time
    m_CurrentSelectedProjectURL[0] = '\0';

    m_sAddProjectString = _("Add Project");
    m_sSynchronizeString = _("Synchronize");
    m_sTotalWorkDoneString = _("Work done for this project");

    m_sAddProjectToolTip = _("Volunteer for any or all of 30+ projects in many areas of science");
    m_sSynchronizeToolTip = _("Synchronize projects with account manager system");

    m_GotBGBitMap = false; // Can't be made until parent has been laid out.
    SetForegroundColour(wxGetApp().GetIsDarkMode() ? *wxWHITE : *wxBLACK);

    wxBoxSizer* bSizer1;
    bSizer1 = new wxBoxSizer( wxVERTICAL );

    wxBoxSizer* bSizer2;
    bSizer2 = new wxBoxSizer( wxHORIZONTAL );

    bSizer1->AddSpacer(5);
    m_myProjectsLabel = new CTransparentStaticText( this, wxID_ANY, _("Projects:"), wxDefaultPosition, wxDefaultSize, 0 );
    m_myProjectsLabel->Wrap( -1 );
    bSizer2->Add( m_myProjectsLabel, 0, wxRIGHT, 5 );
    bSizer2->AddStretchSpacer();

    int addProjectWidth, synchronizeWidth, y;
    GetTextExtent(m_sAddProjectString, &addProjectWidth, &y);
    GetTextExtent(m_sSynchronizeString, &synchronizeWidth, &y);
    m_TaskAddProjectButton = new CTransparentButton( this, ID_ADDROJECTBUTTON,
        (addProjectWidth > synchronizeWidth) ? m_sAddProjectString : m_sSynchronizeString,
        wxDefaultPosition, wxDefaultSize, 0
    );

    bSizer2->Add( m_TaskAddProjectButton, 0, wxRIGHT | wxEXPAND, sideMargins);
    bSizer1->Add( bSizer2, 0, wxEXPAND | wxTOP | wxLEFT, 10 );

#ifndef __WXMAC__
    bSizer1->AddSpacer(5);
#endif

#if TESTBIGICONPOPUP
    m_ProjectSelectionCtrl = new CBOINCBitmapComboBox( this, ID_SGPROJECTSELECTOR, wxT(""), wxDefaultPosition, wxSize(-1, 42), 4, tempArray, wxCB_READONLY );
    CSkinSimple* pSkinSimple = wxGetApp().GetSkinManager()->GetSimple();
    bmArray[0] = *pSkinSimple->GetProjectImage()->GetBitmap();
    m_ProjectSelectionCtrl->SetItemBitmap(0, bmArray[0]);
    bmArray[1] = wxBitmap((const char**)sah_40_xpm);
    m_ProjectSelectionCtrl->SetItemBitmap(1, bmArray[1]);
    bmArray[2] = wxBitmap((const char**)einstein_icon_xpm);
    m_ProjectSelectionCtrl->SetItemBitmap(3, bmArray[2]);
//    m_ProjectSelectionCtrl->SetStringSelection(tempArray[1]);
    m_ProjectSelectionCtrl->SetSelection(1);
#else
    m_ProjectSelectionCtrl = new CBOINCBitmapComboBox( this, ID_SGPROJECTSELECTOR, wxT(""), wxDefaultPosition, wxSize(-1, 42), 0, NULL, wxCB_READONLY);
#endif
    // TODO: Might want better wording for Project Selection Combo Box tooltip
    str = _("Select a project to access with the controls below");
    m_ProjectSelectionCtrl->SetToolTip(str);
    bSizer1->Add( m_ProjectSelectionCtrl, 0, wxLEFT | wxRIGHT | wxEXPAND, sideMargins);

#ifndef __WXMAC__
    bSizer1->AddSpacer(8);
#endif

    // Make sure m_TotalCreditValue string is large enough
    m_fDisplayedCredit = 9999999999.99;
    str.Printf(wxT("%s: %.0f"), m_sTotalWorkDoneString.c_str(), m_fDisplayedCredit);
    m_TotalCreditValue = new CTransparentStaticText( this, wxID_ANY, str, wxDefaultPosition, wxDefaultSize, wxST_NO_AUTORESIZE );
    m_TotalCreditValue->Wrap( -1 );

    bSizer1->Add( m_TotalCreditValue, 0, wxLEFT | wxRIGHT | wxEXPAND, sideMargins);

    bSizer1->AddSpacer(5);

    wxBoxSizer* bSizer3;
    bSizer3 = new wxBoxSizer( wxHORIZONTAL );

    m_ProjectWebSitesButton = new CSimpleProjectWebSitesPopupButton( this, ID_PROJECTWEBSITESBUTTON, _("Project Web Pages"), wxDefaultPosition, wxDefaultSize, 0 );
    bSizer3->Add( m_ProjectWebSitesButton, 0, wxEXPAND | wxALIGN_LEFT, 0 );
    bSizer3->AddStretchSpacer();

    m_ProjectCommandsButton = new CSimpleProjectCommandPopupButton( this, ID_PROJECTCOMMANDBUTTON, _("Project Commands"), wxDefaultPosition, wxDefaultSize, 0 );
    bSizer3->Add( m_ProjectCommandsButton, 0, wxEXPAND, 0 );

    bSizer1->Add( bSizer3, 0, wxLEFT | wxRIGHT | wxEXPAND, sideMargins);

    bSizer1->AddSpacer(10);

    // Temporarily insert a dummy entry so sizer can
    // get correct height of m_ProjectSelectionCtrl
    CSkinSimple* pSkinSimple = wxGetApp().GetSkinManager()->GetSimple();
    wxBitmap* defaultBM = pSkinSimple->GetProjectImage()->GetBitmap();
    m_ProjectSelectionCtrl->Insert("", *defaultBM, 0, (void*)NULL);

    this->SetSizer( bSizer1 );
    this->Layout();

    // Remove the dummy entry
    m_ProjectSelectionCtrl->Delete(0);

    m_TaskAddProjectButton->SetToolTip(wxEmptyString);
    m_TaskAddProjectButton->Disable();
}


CSimpleProjectPanel::~CSimpleProjectPanel()
{
    ProjectSelectionData *selData;
    int count = m_ProjectSelectionCtrl->GetCount();
    for(int j = count-1; j >=0; --j) {
        selData = (ProjectSelectionData*)m_ProjectSelectionCtrl->GetClientData(j);
        delete selData;
        // Indicate to Clear() we have cleaned up the Selection Data
        m_ProjectSelectionCtrl->SetClientData(j, NULL);
    }
    m_ProjectSelectionCtrl->Clear();
}


ProjectSelectionData* CSimpleProjectPanel::GetProjectSelectionData() {
    int count = m_ProjectSelectionCtrl->GetCount();
    if (count <= 0) {
        return NULL;
    }

    int n = m_ProjectSelectionCtrl->GetSelection();
    return (ProjectSelectionData*)m_ProjectSelectionCtrl->GetClientData(n);
}


void CSimpleProjectPanel::UpdateInterface() {
    int n, count = -1;
    bool b_needMenuRebuild = false;
    wxString str = wxEmptyString;
    wxString projName = wxEmptyString;
    CMainDocument* pDoc = wxGetApp().GetDocument();

    wxASSERT(pDoc);

    // Should we display the synchronize button instead of the
    //   attach to project button?
    if ( pDoc->IsConnected() ) {
        CC_STATUS       status;
        int             is_acct_mgr_detected = 0;

        pDoc->GetCoreClientStatus(status);

        if (pDoc->m_iAcct_mgr_info_rpc_result == 0) {
            // We use an integer rather than a bool to force action the first time
            is_acct_mgr_detected = pDoc->ami.acct_mgr_url.size() ? 1 : 0;

            if ((m_UsingAccountManager != is_acct_mgr_detected) || (!m_TaskAddProjectButton->IsEnabled())) {
                m_UsingAccountManager = is_acct_mgr_detected;
                if (is_acct_mgr_detected) {
                    m_TaskAddProjectButton->SetLabel(m_sSynchronizeString);
                    m_TaskAddProjectButton->Enable();
                    m_TaskAddProjectButton->SetToolTip(m_sSynchronizeToolTip);
                } else {
                    m_TaskAddProjectButton->SetLabel(m_sAddProjectString);
                    if (!status.disallow_attach) {
                        m_TaskAddProjectButton->Enable();
                        m_TaskAddProjectButton->SetToolTip(m_sAddProjectToolTip);
                   }
                }
                this->Layout();
            }
        } else {
            m_TaskAddProjectButton->Disable();
        }

        UpdateProjectList();

        count = m_ProjectSelectionCtrl->GetCount();
    }
    if (count > 0) {
        n = m_ProjectSelectionCtrl->GetSelection();
        if ((n < 0) || (n > count -1)) {
            m_ProjectSelectionCtrl->SetSelection(0);
            n = 0;
        }

        // Check to see if we need to rebuild the menu
        char* ctrl_url = ((ProjectSelectionData*)m_ProjectSelectionCtrl->GetClientData(n))->project_url;
        if (strcmp(m_CurrentSelectedProjectURL, ctrl_url)) {
            b_needMenuRebuild = true;
            strlcpy(m_CurrentSelectedProjectURL, ctrl_url, sizeof(m_CurrentSelectedProjectURL));
        }

        PROJECT* project = pDoc->state.lookup_project(ctrl_url);
        if ( project != NULL && project->last_rpc_time > m_Project_last_rpc_time ) {
            b_needMenuRebuild = true;
            m_Project_last_rpc_time = project->last_rpc_time;
        }

        if (b_needMenuRebuild) {
            m_ProjectWebSitesButton->RebuildMenu();
        }

        m_ProjectWebSitesButton->Enable();
        m_ProjectCommandsButton->Enable();

        if (m_fDisplayedCredit != project->user_total_credit) {
            str.Printf(wxT("%s: %s"),
                m_sTotalWorkDoneString.c_str(),
                format_number(project->user_total_credit, 0)
            );
            UpdateStaticText(&m_TotalCreditValue, str);
            m_TotalCreditValue->SetName(str);   // For accessibility on Windows
        }
        projName = m_ProjectSelectionCtrl->GetStringSelection();
        str.Printf(_("Pop up a menu of web sites for project %s"), projName.c_str());
        m_ProjectWebSitesButton->SetToolTip(str);
        str.Printf(_("Pop up a menu of commands to apply to project %s"), projName.c_str());
        m_ProjectCommandsButton->SetToolTip(str);
    } else {
        m_ProjectWebSitesButton->Disable();
        m_ProjectCommandsButton->Disable();
        m_CurrentSelectedProjectURL[0] = '\0';
        m_fDisplayedCredit = -1.0;
        UpdateStaticText(&m_TotalCreditValue, wxEmptyString);
        m_TotalCreditValue->SetName(wxEmptyString);   // For accessibility on Windows
        m_ProjectWebSitesButton->SetToolTip(wxEmptyString);
        m_ProjectCommandsButton->SetToolTip(wxEmptyString);
    }
}


void CSimpleProjectPanel::ReskinInterface() {
    wxLogTrace(wxT("Function Start/End"), wxT("CSimpleProjectPanel::ReskinInterface - Function Begin"));

    ProjectSelectionData* selData;
    char* ctrl_url;

    CSimplePanelBase::ReskinInterface();

    // Check to see if we need to reload the project icon
    int ctrlCount = m_ProjectSelectionCtrl->GetCount();
    for(int j=0; j<ctrlCount; j++) {
        selData = (ProjectSelectionData*)m_ProjectSelectionCtrl->GetClientData(j);
        ctrl_url = selData->project_url;
        wxBitmap* projectBM = GetProjectSpecificBitmap(ctrl_url);
        m_ProjectSelectionCtrl->SetItemBitmap(j, *projectBM);
    }

    wxLogTrace(wxT("Function Start/End"), wxT("CSimpleProjectPanel::ReskinInterface - Function Begin"));
}


void CSimpleProjectPanel::OnAddProject(wxCommandEvent& event) {
    if (m_UsingAccountManager) {
        OnWizardUpdate();
    } else {
        OnWizardAttach(event);
    }
}


void CSimpleProjectPanel::OnWizardAttach(wxCommandEvent& event) {
    wxLogTrace(wxT("Function Start/End"), wxT("CProjectsComponent::OnWizardAttach - Function Begin"));

    CMainDocument* pDoc = wxGetApp().GetDocument();
    CSimpleGUIPanel*  pPanel = wxDynamicCast(GetParent(), CSimpleGUIPanel);

    wxASSERT(pDoc);
    wxASSERT(pPanel);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));

    if (!pDoc->IsUserAuthorized()) return;

    if (!pDoc->IsConnected()) return;

    pPanel->SetDlgOpen(true);

    pPanel->OnProjectsAttachToProject(event);
//    btnAddProj->Refresh();

    pPanel->SetDlgOpen(false);

    wxLogTrace(wxT("Function Start/End"), wxT("CProjectsComponent::OnWizardAttach - Function End"));
}


void CSimpleProjectPanel::OnWizardUpdate() {
    wxLogTrace(wxT("Function Start/End"), wxT("CProjectsComponent::OnWizardUpdate - Function Begin"));

    CMainDocument* pDoc = wxGetApp().GetDocument();
    CSimpleGUIPanel*  pPanel = wxDynamicCast(GetParent(), CSimpleGUIPanel);

    wxASSERT(pDoc);
    wxASSERT(pPanel);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));

    if (!pDoc->IsUserAuthorized()) return;

    if (!pDoc->IsConnected()) return;

    pPanel->SetDlgOpen(true);

    CWizardAttach* pWizard = new CWizardAttach(this);

    pWizard->SyncToAccountManager();

    if (pWizard)
        pWizard->Destroy();

//    btnSynchronize->Refresh();

    pPanel->SetDlgOpen(false);

    wxLogTrace(wxT("Function Start/End"), wxT("CProjectsComponent::OnWizardUpdate - Function End"));
}


#if TESTBIGICONPOPUP
void CSimpleProjectPanel::OnProjectWebSiteButton(wxCommandEvent& /*event*/) {
    m_ProjectSelectionCtrl->Delete(0); /*** CAF *** FOR TESTING ONLY ***/
}


void CSimpleProjectPanel::OnProjectCommandButton(wxCommandEvent& /*event*/) {
/*** CAF *** FOR TESTING ONLY ***/
    static int i = 1;
    wxString s;

    if (++i > 8) i = 0;
    int sel = i % 3;
//    m_ProjectSelectionCtrl->SetStringSelection(tempArray[sel]);
    m_ProjectSelectionCtrl->SetSelection(sel);
#if 0 //TESTBIGICONPOPUP
    wxRect r;
    wxWindowDC dc(this);
    r = m_ProjectSelectionCtrl->GetRect();
    dc.DrawBitmap(bmArray[sel], r.x + 9, r.y, false);
#endif

}
#endif


void CSimpleProjectPanel::OnProjectSelection(wxCommandEvent& /*event*/) {
    UpdateInterface();

//    const int sel = m_ProjectSelectionCtrl->GetSelection();
#if 0 //TESTBIGICONPOPUP
    wxRect r;
    wxWindowDC dc(this);

    r = m_ProjectSelectionCtrl->GetRect();
    dc.DrawBitmap(bmArray[sel], r.x + 9, r.y - 10, false);
#endif
}


void CSimpleProjectPanel::UpdateProjectList() {
    CMainDocument* pDoc = wxGetApp().GetDocument();
    ProjectSelectionData* selData;
    PROJECT* project;
    int oldProjectSelection, newProjectSelection;

    if ( pDoc->IsConnected() ) {
        int projCnt = pDoc->GetSimpleProjectCount();
        int ctrlCount = m_ProjectSelectionCtrl->GetCount();
        oldProjectSelection = m_ProjectSelectionCtrl->GetSelection();

        // If a new project has been added, figure out which one
        for(int i=0; i<projCnt; i++) {
            char* ctrl_url;
            project = pDoc->state.projects[i];
            bool found = false;
            for(int j=0; j<ctrlCount; j++) {
                ctrl_url = ((ProjectSelectionData*)m_ProjectSelectionCtrl->GetClientData(j))->project_url;
                if (!strcmp(project->master_url, ctrl_url)) {
                    found = true;
                    break;
                }
            }

            // add new project
            //
            if (!found) {
                const char* p = project->project_name.c_str();
                if (strlen(p) == 0) {
                    p = project->master_url;
                }
                wxString projname(p, wxConvUTF8);
#if SORTPROJECTLIST
                int alphaOrder,j;
                for(j = 0; j < ctrlCount; ++j) {
                    alphaOrder = (m_ProjectSelectionCtrl->GetString(j)).CmpNoCase(projname);
                    if (alphaOrder > 0) {
                        break;  // Insert the new item here (sorted by item label)
                    }
                }
#endif
                selData = new ProjectSelectionData;
                strlcpy(selData->project_url, project->master_url, sizeof(selData->project_url));
                selData->project_files_downloaded_time = project->project_files_downloaded_time;
                wxBitmap* projectBM = GetProjectSpecificBitmap(selData->project_url);
#if SORTPROJECTLIST
                if (j < ctrlCount) {
                    m_ProjectSelectionCtrl->Insert(projname, *projectBM, j, (void*)selData);
                    if (j <= oldProjectSelection) {
                        ++oldProjectSelection;
                        m_ProjectSelectionCtrl->SetSelection(oldProjectSelection);
                    }
                } else
#endif
                {
                    m_ProjectSelectionCtrl->Append(projname, *projectBM, (void*)selData);
                }
                ctrlCount = m_ProjectSelectionCtrl->GetCount();
            }
        }

        newProjectSelection = oldProjectSelection;
        if ( projCnt < ctrlCount ) {
            project = NULL;
            // Check items in descending order so deletion won't change indexes of items yet to be checked
            for(int j=ctrlCount-1; j>=0; --j) {
                char* ctrl_url = ((ProjectSelectionData*)m_ProjectSelectionCtrl->GetClientData(j))->project_url;
                project = pDoc->state.lookup_project(ctrl_url);
                if ( project == NULL ) {
                    selData = (ProjectSelectionData*)m_ProjectSelectionCtrl->GetClientData(j);
                    delete selData;
                    // Indicate to Delete() we have cleaned up the Selection Data
                    m_ProjectSelectionCtrl->SetClientData(j, NULL);
                    m_ProjectSelectionCtrl->Delete(j);
                    if (j == oldProjectSelection) {
                        int newCount = m_ProjectSelectionCtrl->GetCount();
                        if (newProjectSelection < newCount) {
                            // Select the next item if one exists
                            m_ProjectSelectionCtrl->SetSelection(newProjectSelection);
                        } else if (newCount > 0) {
                            // Select the previous item if one exists
                            newProjectSelection = newCount-1;
                            m_ProjectSelectionCtrl->SetSelection(newProjectSelection);
                        } else {
                            newProjectSelection = -1;
                            m_ProjectSelectionCtrl->SetSelection(wxNOT_FOUND);
                        }
                    }
                }
            }
        }

        // Check to see if we need to reload the project icon
        ctrlCount = m_ProjectSelectionCtrl->GetCount();
        for(int j=0; j<ctrlCount; j++) {
            selData = (ProjectSelectionData*)m_ProjectSelectionCtrl->GetClientData(j);
            char* ctrl_url = selData->project_url;
            project = pDoc->state.lookup_project(ctrl_url);
            if ( (project != NULL) && (project->project_files_downloaded_time > selData->project_files_downloaded_time) ) {
                wxBitmap* projectBM = GetProjectSpecificBitmap(ctrl_url);
                selData->project_files_downloaded_time = project->project_files_downloaded_time;
                m_ProjectSelectionCtrl->SetItemBitmap(j, *projectBM);
            }
        }
    }
}


std::string CSimpleProjectPanel::GetProjectIconLoc(char* project_url) {
    char proj_dir[256];
    CMainDocument* pDoc = wxGetApp().GetDocument();
    PROJECT* project = pDoc->state.lookup_project(project_url);
    if (!project) return (std::string)"";
    url_to_project_dir(project->master_url, proj_dir, sizeof(proj_dir));
    return (std::string)proj_dir + "/stat_icon";
}


wxBitmap* CSimpleProjectPanel::GetProjectSpecificBitmap(char* project_url) {
    char defaultIcnPath[256];
    CSkinSimple* pSkinSimple = wxGetApp().GetSkinManager()->GetSimple();

    wxASSERT(pSkinSimple);

    // Only update it if project specific is found
    if (resolve_soft_link(GetProjectIconLoc(project_url).c_str(), defaultIcnPath, sizeof(defaultIcnPath)) == 0) {
        wxString strIconPath = wxString(defaultIcnPath,wxConvUTF8);
        if (wxFile::Exists(strIconPath)) {
            // wxBitmapComboBox requires all its bitmaps to be the same size
            // Our "project icon" bitmaps should all be 40 X 40
            wxImage img = wxImage(strIconPath, wxBITMAP_TYPE_ANY);
            if (img.IsOk()) {
                if ((img.GetHeight() != 40) || (img.GetWidth() == 40)) {
                    img.Rescale(40, 40, wxIMAGE_QUALITY_BILINEAR);
                }
                wxBitmap* projectBM = new wxBitmap(img);
                if (projectBM->IsOk()) {
                    return projectBM;
                }
            }
        }
    }
    return pSkinSimple->GetProjectImage()->GetBitmap();
}
