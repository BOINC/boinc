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


#include "stdwx.h"
#include "Events.h"
#include "BOINCGUIApp.h"
#include "MainDocument.h"
#include "DlgItemProperties.h"
#include "sg_ProjectPanel.h"
#include "sg_ProjectCommandPopup.h"

IMPLEMENT_DYNAMIC_CLASS(CSimpleProjectCommandPopupButton, CTransparentButton)

BEGIN_EVENT_TABLE(CSimpleProjectCommandPopupButton, CTransparentButton)
    EVT_LEFT_DOWN(CSimpleProjectCommandPopupButton::OnProjectCommandsMouseDown)
    EVT_MENU(ID_TASK_PROJECT_UPDATE, CSimpleProjectCommandPopupButton::OnProjectUpdate)
    EVT_MENU(ID_TASK_PROJECT_SUSPEND, CSimpleProjectCommandPopupButton::OnProjectSuspendResume)
    EVT_MENU(ID_TASK_PROJECT_NONEWWORK, CSimpleProjectCommandPopupButton::OnProjectNoNewWork)
    EVT_MENU(ID_TASK_PROJECT_RESET, CSimpleProjectCommandPopupButton::OnResetProject)
    EVT_MENU(ID_TASK_PROJECT_DETACH, CSimpleProjectCommandPopupButton::OnProjectDetach)
    EVT_MENU(ID_TASK_PROJECT_SHOW_PROPERTIES, CSimpleProjectCommandPopupButton::OnProjectShowProperties)
END_EVENT_TABLE()

CSimpleProjectCommandPopupButton::CSimpleProjectCommandPopupButton() {
}

CSimpleProjectCommandPopupButton::CSimpleProjectCommandPopupButton(wxWindow* parent, wxWindowID id,
        const wxString& label, const wxPoint& pos, const wxSize& size,
        long style, const wxValidator& validator, const wxString& name) :
        CTransparentButton(parent, id, label, pos, size, style, validator, name)
    {

    m_ProjectCommandsPopUpMenu = new wxMenu();
    AddMenuItems();
    Connect(
        id,
        wxEVT_BUTTON,
        (wxObjectEventFunction) (wxEventFunction) (wxCommandEventFunction) &CSimpleProjectCommandPopupButton::OnProjectCommandsKeyboardNav
    );
}


CSimpleProjectCommandPopupButton::~CSimpleProjectCommandPopupButton() {
    delete m_ProjectCommandsPopUpMenu;
}


void CSimpleProjectCommandPopupButton::AddMenuItems() {
    m_UpdateProjectMenuItem = m_ProjectCommandsPopUpMenu->Append(
        ID_TASK_PROJECT_UPDATE,
        _("Update"),
        _("Report all completed tasks, get latest credit, get latest preferences, and possibly get more tasks.")
    );

    m_SuspendResumeMenuItem = m_ProjectCommandsPopUpMenu->Append(
        ID_TASK_PROJECT_SUSPEND,
        _("Suspend"),
        _("Suspend tasks for this project.")
    );

    m_NoNewTasksMenuItem = m_ProjectCommandsPopUpMenu->Append(
        ID_TASK_PROJECT_NONEWWORK,
        _("No new tasks"),
        _("Don't get new tasks for this project.")
    );

    m_ResetProjectMenuItem = m_ProjectCommandsPopUpMenu->Append(
        ID_TASK_PROJECT_RESET,
        _("Reset project"),
        _("Delete all files and tasks associated with this project, and get new tasks.  You can update the project first to report any completed tasks.")
    );

    m_RemoveProjectMenuItem = m_ProjectCommandsPopUpMenu->Append(
        ID_TASK_PROJECT_DETACH,
        _("Remove"),
        _("Remove this project.  Tasks in progress will be lost (use 'Update' first to report any completed tasks).")
    );

    m_ShowPropertiesMenuItem = m_ProjectCommandsPopUpMenu->Append(
        ID_TASK_PROJECT_SHOW_PROPERTIES,
        _("Properties"),
        _("Show project details.")
    );
}


void CSimpleProjectCommandPopupButton::OnProjectCommandsMouseDown(wxMouseEvent&) {
    ShowProjectCommandsMenu(ScreenToClient(wxGetMousePosition()));
}


void CSimpleProjectCommandPopupButton::OnProjectCommandsKeyboardNav(wxCommandEvent&) {
    ShowProjectCommandsMenu(wxPoint(GetSize().GetWidth()/2, GetSize().GetHeight()/2));
}


void CSimpleProjectCommandPopupButton::ShowProjectCommandsMenu(wxPoint pos) {
    CMainDocument*      pDoc = wxGetApp().GetDocument();

    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));

    ProjectSelectionData*  selData = ((CSimpleProjectPanel*)GetParent())->GetProjectSelectionData();
    if (selData == NULL) return;
    char* ctrl_url = selData->project_url;
    PROJECT* project = pDoc->state.lookup_project(ctrl_url);
    if (!project) return;

    if (project->suspended_via_gui) {
        m_SuspendResumeMenuItem->SetItemLabel(_("Resume"));
        m_SuspendResumeMenuItem->SetHelp(_("Resume tasks for this project."));
    } else {
        m_SuspendResumeMenuItem->SetItemLabel(_("Suspend"));
        m_SuspendResumeMenuItem->SetHelp(_("Suspend tasks for this project."));
    }

    if (project->dont_request_more_work) {
        m_NoNewTasksMenuItem->SetItemLabel(_("Allow new tasks"));
        m_NoNewTasksMenuItem->SetHelp(_("Allow fetching new tasks for this project."));
    } else {
        m_NoNewTasksMenuItem->SetItemLabel(_("No new tasks"));
        m_NoNewTasksMenuItem->SetHelp(_("Don't fetch new tasks for this project."));
    }

    m_RemoveProjectMenuItem->Enable(!project->attached_via_acct_mgr);

#ifdef __WXMAC__
    // Disable tooltips on Mac while menus are popped up because they cover menus
    wxToolTip::Enable(false);
#endif

	PopupMenu(m_ProjectCommandsPopUpMenu, pos.x, pos.y);
}


void CSimpleProjectCommandPopupButton::OnProjectUpdate( wxCommandEvent& WXUNUSED(event) ) {
    int projectIndex;
    CMainDocument*  pDoc   = wxGetApp().GetDocument();
    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));

    ProjectSelectionData*  selData = ((CSimpleProjectPanel*)GetParent())->GetProjectSelectionData();
    if (selData == NULL) return;
    char* ctrl_url = selData->project_url;
    PROJECT* project = FindProjectIndexFromURL(ctrl_url, &projectIndex);
    if (!project) return;

    pDoc->ProjectUpdate(projectIndex);
}


void CSimpleProjectCommandPopupButton::OnProjectSuspendResume(wxCommandEvent& WXUNUSED(event)) {
    int projectIndex;
    CMainDocument* pDoc     = wxGetApp().GetDocument();

    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));

    ProjectSelectionData*  selData = ((CSimpleProjectPanel*)GetParent())->GetProjectSelectionData();
    if (selData == NULL) return;
    char* ctrl_url = selData->project_url;
    PROJECT* project = FindProjectIndexFromURL(ctrl_url, &projectIndex);
    if (!project) return;

    if (project->suspended_via_gui) {
        pDoc->ProjectResume(projectIndex);
    } else {
        pDoc->ProjectSuspend(projectIndex);
    }
}


void CSimpleProjectCommandPopupButton::OnProjectNoNewWork(wxCommandEvent& WXUNUSED(event)) {
    int projectIndex;
    CMainDocument* pDoc     = wxGetApp().GetDocument();

    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));

    ProjectSelectionData*  selData = ((CSimpleProjectPanel*)GetParent())->GetProjectSelectionData();
    if (selData == NULL) return;
    char* ctrl_url = selData->project_url;
    PROJECT* project = FindProjectIndexFromURL(ctrl_url, &projectIndex);
    if (!project) return;

    if (project->dont_request_more_work) {
        pDoc->ProjectAllowMoreWork(projectIndex);
    } else {
        pDoc->ProjectNoMoreWork(projectIndex);
    }
}


void CSimpleProjectCommandPopupButton::OnResetProject(wxCommandEvent& WXUNUSED(event)) {
    int             projectIndex;
    wxInt32         iAnswer        = 0;
    wxString        strMessage     = wxEmptyString;
    CMainDocument*  pDoc     = wxGetApp().GetDocument();

    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));

    if (!pDoc->IsUserAuthorized())
        return;

    ProjectSelectionData*  selData = ((CSimpleProjectPanel*)GetParent())->GetProjectSelectionData();
    if (selData == NULL) return;
    char* ctrl_url = selData->project_url;
    PROJECT* project = FindProjectIndexFromURL(ctrl_url, &projectIndex);
    if (!project) return;

    wxString projname(project->project_name.c_str(), wxConvUTF8);
    strMessage.Printf(
        _("Are you sure you want to reset project '%s'?"),
        projname.c_str()
    );

    iAnswer = wxGetApp().SafeMessageBox(
        strMessage,
        _("Reset Project"),
        wxYES_NO | wxICON_QUESTION,
        this
    );

    if (wxYES == iAnswer) {
        pDoc->ProjectReset(projectIndex);
    }
}


void CSimpleProjectCommandPopupButton::OnProjectDetach(wxCommandEvent& WXUNUSED(event)) {
    int             projectIndex;
    wxInt32         iAnswer        = 0;
    wxString        strMessage     = wxEmptyString;
    CMainDocument*  pDoc     = wxGetApp().GetDocument();

    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));

    if (!pDoc->IsUserAuthorized())
        return;

    ProjectSelectionData*  selData = ((CSimpleProjectPanel*)GetParent())->GetProjectSelectionData();
    if (selData == NULL) return;
    char* ctrl_url = selData->project_url;
    PROJECT* project = FindProjectIndexFromURL(ctrl_url, &projectIndex);
    if (!project) return;

    wxString projname(project->project_name.c_str(), wxConvUTF8);
    strMessage.Printf(
        _("Are you sure you want to remove project '%s'?"),
        projname.c_str()
    );

    iAnswer = wxGetApp().SafeMessageBox(
        strMessage,
        _("Remove Project"),
        wxYES_NO | wxICON_QUESTION,
        this
    );

    if (wxYES == iAnswer) {
        pDoc->ProjectDetach(projectIndex);
    }
}


void CSimpleProjectCommandPopupButton::OnProjectShowProperties(wxCommandEvent& WXUNUSED(event)) {
    CMainDocument* pDoc     = wxGetApp().GetDocument();

    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));

    ProjectSelectionData*  selData = ((CSimpleProjectPanel*)GetParent())->GetProjectSelectionData();
    if (selData == NULL) return;
    char* ctrl_url = selData->project_url;
    PROJECT* project = pDoc->state.lookup_project(ctrl_url);
    if (!project) return;

    CDlgItemProperties dlg(this);
    dlg.renderInfos(project);
    dlg.ShowModal();
}


PROJECT* CSimpleProjectCommandPopupButton::FindProjectIndexFromURL(char *project_url, int *index) {
    CMainDocument*  pDoc     = wxGetApp().GetDocument();

    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));

	int prjCount = pDoc->GetSimpleProjectCount();
	for(int i = 0; i < prjCount; i++){
		PROJECT* project = pDoc->project(i);
		if(!strcmp(project->master_url, project_url)){
			*index = i;
			return project;
		}
	}
    *index = -1;
    return NULL;
}

