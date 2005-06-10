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

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma implementation "ViewProjects.h"
#endif

#include "stdwx.h"
#include "BOINCGUIApp.h"
#include "MainDocument.h"
#include "BOINCTaskCtrl.h"
#include "BOINCListCtrl.h"
#include "ViewProjects.h"
#include "DlgAttachProject.h"
#include "Events.h"


#include "res/proj.xpm"


#define COLUMN_PROJECT              0
#define COLUMN_ACCOUNTNAME          1
#define COLUMN_TEAMNAME             2
#define COLUMN_TOTALCREDIT          3
#define COLUMN_AVGCREDIT            4
#define COLUMN_RESOURCESHARE        5
#define COLUMN_STATUS               6

// groups that contain buttons
#define GRP_TASKS    0
#define GRP_WEBSITES 1

// buttons in the "tasks" area
#define BTN_UPDATE   0
#define BTN_SUSPEND  1
#define BTN_NOWORK   2
#define BTN_RESET    3
#define BTN_DETACH   4
#define BTN_ATTACH   5


CProject::CProject() {
}


CProject::~CProject() {
    // ??? NOT NEEDED
    m_strProjectName.Clear();
    m_strAccountName.Clear();
    m_strTeamName.Clear();
    m_strTotalCredit.Clear();
    m_strAVGCredit.Clear();
    m_strResourceShare.Clear();
    m_strStatus.Clear();
}


IMPLEMENT_DYNAMIC_CLASS(CViewProjects, CBOINCBaseView)

BEGIN_EVENT_TABLE (CViewProjects, CBOINCBaseView)
    EVT_BUTTON(ID_TASK_PROJECT_UPDATE, CViewProjects::OnProjectUpdate)
    EVT_BUTTON(ID_TASK_PROJECT_SUSPEND, CViewProjects::OnProjectSuspend)
    EVT_BUTTON(ID_TASK_PROJECT_NONEWWORK, CViewProjects::OnProjectNoNewWork)
    EVT_BUTTON(ID_TASK_PROJECT_RESET, CViewProjects::OnProjectReset)
    EVT_BUTTON(ID_TASK_PROJECT_DETACH, CViewProjects::OnProjectDetach)
    EVT_BUTTON(ID_TASK_PROJECT_ATTACH, CViewProjects::OnProjectAttach)
    EVT_CUSTOM_RANGE(wxEVT_COMMAND_BUTTON_CLICKED, ID_TASK_PROJECT_WEB_PROJDEF_MIN, ID_TASK_PROJECT_WEB_PROJDEF_MAX, CViewProjects::OnProjectWebsiteClicked)
    EVT_LIST_ITEM_SELECTED(ID_LIST_PROJECTSVIEW, CViewProjects::OnListSelected)
    EVT_LIST_ITEM_DESELECTED(ID_LIST_PROJECTSVIEW, CViewProjects::OnListDeselected)
END_EVENT_TABLE ()


CViewProjects::CViewProjects() {}


CViewProjects::CViewProjects(wxNotebook* pNotebook) :
    CBOINCBaseView(pNotebook, ID_TASK_PROJECTSVIEW, DEFAULT_TASK_FLAGS, ID_LIST_PROJECTSVIEW, DEFAULT_LIST_SINGLE_SEL_FLAGS)
{
	CTaskItemGroup* pGroup = NULL;
	CTaskItem*      pItem = NULL;

    wxASSERT(m_pTaskPane);
    wxASSERT(m_pListPane);


    //
    // Setup View
    //
	pGroup = new CTaskItemGroup( _("Tasks") );
	m_TaskGroups.push_back( pGroup );

	pItem = new CTaskItem(
        _("Update"),
        _("Report all completed work, get latest credit, "
          "get latest preferences, and possibly get more work."),
        ID_TASK_PROJECT_UPDATE 
    );
    pGroup->m_Tasks.push_back( pItem );

	pItem = new CTaskItem(
        _("Suspend"),
        _("Suspend work for this project."),
        ID_TASK_PROJECT_SUSPEND 
    );
    pGroup->m_Tasks.push_back( pItem );

	pItem = new CTaskItem(
        _("No new work"),
        _("Don't fetch new work for this project."),
        ID_TASK_PROJECT_NONEWWORK 
    );
    pGroup->m_Tasks.push_back( pItem );

	pItem = new CTaskItem(
        _("Reset project"),
        _("Delete all files and work associated with this project, "
          "and get new work.  "
          "You can update the project "
          "first to report any completed work."),
        ID_TASK_PROJECT_RESET 
    );
    pGroup->m_Tasks.push_back( pItem );

	pItem = new CTaskItem(
        _("Detach"),
        _("Detach this computer from this project.  "
          "Work in progress will be lost. "
          "You can update the project first to report "
          "any completed work."),
        ID_TASK_PROJECT_DETACH 
    );
    pGroup->m_Tasks.push_back( pItem );

	pItem = new CTaskItem(
        _("Attach to new project"),
        _("Attach this computer to a BOINC project.  "
          "You'll need a project URL and account key "
          "(visit the project's web site to get these)."),
        ID_TASK_PROJECT_ATTACH 
    );
    pGroup->m_Tasks.push_back( pItem );


    // Create Task Pane Items
    m_pTaskPane->UpdateControls();

    // Create List Pane Items
    m_pListPane->InsertColumn(COLUMN_PROJECT, _("Project"), wxLIST_FORMAT_LEFT, 150);
    m_pListPane->InsertColumn(COLUMN_ACCOUNTNAME, _("Account"), wxLIST_FORMAT_LEFT, 80);
    m_pListPane->InsertColumn(COLUMN_TEAMNAME, _("Team"), wxLIST_FORMAT_LEFT, 80);
    m_pListPane->InsertColumn(COLUMN_TOTALCREDIT, _("Total credit"), wxLIST_FORMAT_RIGHT, 80);
    m_pListPane->InsertColumn(COLUMN_AVGCREDIT, _("Avg. credit"), wxLIST_FORMAT_RIGHT, 80);
    m_pListPane->InsertColumn(COLUMN_RESOURCESHARE, _("Resource share"), wxLIST_FORMAT_CENTRE, 85);
    m_pListPane->InsertColumn(COLUMN_STATUS, _("Status"), wxLIST_FORMAT_LEFT, 150);

    UpdateSelection();
}


CViewProjects::~CViewProjects() {
    EmptyCache();
    EmptyTasks();
}


wxString CViewProjects::GetViewName() {
    return wxString(_("Projects"));
}


const char** CViewProjects::GetViewIcon() {
    return proj_xpm;
}


void CViewProjects::OnProjectUpdate( wxCommandEvent& event ) {
    wxLogTrace(wxT("Function Start/End"), wxT("CViewProjects::OnProjectUpdate - Function Begin"));

    CMainDocument* pDoc     = wxGetApp().GetDocument();
    CMainFrame* pFrame      = wxGetApp().GetFrame();

    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));
    wxASSERT(pFrame);
    wxASSERT(wxDynamicCast(pFrame, CMainFrame));
    wxASSERT(m_pTaskPane);
    wxASSERT(m_pListPane);

    pFrame->UpdateStatusText(_("Updating project..."));
    pDoc->ProjectUpdate(m_pListPane->GetFirstSelected());
    pFrame->UpdateStatusText(wxT(""));

    UpdateSelection();
    pFrame->FireRefreshView();

    wxLogTrace(wxT("Function Start/End"), wxT("CViewProjects::OnProjectUpdate - Function End"));
}


void CViewProjects::OnProjectSuspend( wxCommandEvent& event ) {
    wxLogTrace(wxT("Function Start/End"), wxT("CViewProjects::OnProjectSuspend - Function Begin"));

    CMainDocument* pDoc     = wxGetApp().GetDocument();
    CMainFrame* pFrame      = wxGetApp().GetFrame();

    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));
    wxASSERT(pFrame);
    wxASSERT(wxDynamicCast(pFrame, CMainFrame));
    wxASSERT(m_pTaskPane);
    wxASSERT(m_pListPane);

    PROJECT* project = pDoc->project(m_pListPane->GetFirstSelected());
    if (project->suspended_via_gui) {
        pFrame->UpdateStatusText(_("Resuming project..."));
        pDoc->ProjectResume(m_pListPane->GetFirstSelected());
        pFrame->UpdateStatusText(wxT(""));
    } else {
        pFrame->UpdateStatusText(_("Suspending project..."));
        pDoc->ProjectSuspend(m_pListPane->GetFirstSelected());
        pFrame->UpdateStatusText(wxT(""));
    }

    UpdateSelection();
    pFrame->FireRefreshView();

    wxLogTrace(wxT("Function Start/End"), wxT("CViewProjects::OnProjectSuspend - Function End"));
}


void CViewProjects::OnProjectNoNewWork( wxCommandEvent& event ) {
    wxLogTrace(wxT("Function Start/End"), wxT("CViewProjects::OnProjectNoNewWork - Function Begin"));

    CMainDocument* pDoc     = wxGetApp().GetDocument();
    CMainFrame* pFrame      = wxGetApp().GetFrame();

    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));
    wxASSERT(pFrame);
    wxASSERT(wxDynamicCast(pFrame, CMainFrame));
    wxASSERT(m_pTaskPane);
    wxASSERT(m_pListPane);

    PROJECT* project = pDoc->project(m_pListPane->GetFirstSelected());
    if (project->dont_request_more_work) {
        pFrame->UpdateStatusText(_("Telling project to allow additional work downloads..."));
        pDoc->ProjectAllowMoreWork(m_pListPane->GetFirstSelected());
        pFrame->UpdateStatusText(wxT(""));
    } else {
        pFrame->UpdateStatusText(_("Telling project to not fetch additional work..."));
        pDoc->ProjectNoMoreWork(m_pListPane->GetFirstSelected());
        pFrame->UpdateStatusText(wxT(""));
    }

    UpdateSelection();
    pFrame->FireRefreshView();

    wxLogTrace(wxT("Function Start/End"), wxT("CViewProjects::OnProjectNoNewWork - Function End"));
}




void CViewProjects::OnProjectReset( wxCommandEvent& event ) {
    wxLogTrace(wxT("Function Start/End"), wxT("CViewProjects::OnProjectReset - Function Begin"));

    wxInt32  iAnswer        = 0; 
    std::string strProjectName;
    wxString strMessage     = wxEmptyString;
    CMainDocument* pDoc     = wxGetApp().GetDocument();
    CMainFrame* pFrame      = wxGetApp().GetFrame();

    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));
    wxASSERT(pFrame);
    wxASSERT(wxDynamicCast(pFrame, CMainFrame));
    wxASSERT(m_pTaskPane);
    wxASSERT(m_pListPane);

    pFrame->UpdateStatusText(_("Resetting project..."));

    PROJECT* project = pDoc->project(m_pListPane->GetFirstSelected());
    project->get_name(strProjectName);

    strMessage.Printf(
        _("Are you sure you want to reset project '%s'?"), 
        strProjectName.c_str()
    );

    iAnswer = wxMessageBox(
        strMessage,
        _("Reset Project"),
        wxYES_NO | wxICON_QUESTION, 
        this
    );

    if (wxYES == iAnswer) {
        pDoc->ProjectReset(m_pListPane->GetFirstSelected());
    }

    pFrame->UpdateStatusText(wxT(""));

    UpdateSelection();
    pFrame->FireRefreshView();

    wxLogTrace(wxT("Function Start/End"), wxT("CViewProjects::OnProjectReset - Function End"));
}


void CViewProjects::OnProjectDetach( wxCommandEvent& event ) {
    wxLogTrace(wxT("Function Start/End"), wxT("CViewProjects::OnProjectDetach - Function Begin"));

    wxInt32  iAnswer        = 0; 
    std::string strProjectName;
    wxString strMessage     = wxEmptyString;
    CMainDocument* pDoc     = wxGetApp().GetDocument();
    CMainFrame* pFrame      = wxGetApp().GetFrame();

    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));
    wxASSERT(pFrame);
    wxASSERT(wxDynamicCast(pFrame, CMainFrame));
    wxASSERT(m_pTaskPane);
    wxASSERT(m_pListPane);

    pFrame->UpdateStatusText(_("Detaching from project..."));

    PROJECT* project = pDoc->project(m_pListPane->GetFirstSelected());
    project->get_name(strProjectName);

    strMessage.Printf(
        _("Are you sure you want to detach from project '%s'?"), 
        strProjectName.c_str()
    );

    iAnswer = wxMessageBox(
        strMessage,
        _("Detach from Project"),
        wxYES_NO | wxICON_QUESTION, 
        this
    );

    if (wxYES == iAnswer) {
        pDoc->ProjectDetach(m_pListPane->GetFirstSelected());
    }

    pFrame->UpdateStatusText(wxT(""));

    UpdateSelection();
    pFrame->FireRefreshView();

    wxLogTrace(wxT("Function Start/End"), wxT("CViewProjects::OnProjectDetach - Function End"));
}


void CViewProjects::OnProjectAttach( wxCommandEvent& event ) {
    wxLogTrace(wxT("Function Start/End"), wxT("CViewProjects::OnProjectAttach - Function Begin"));

    wxInt32  iAnswer        = 0; 
    CMainDocument* pDoc     = wxGetApp().GetDocument();
    CMainFrame* pFrame      = wxGetApp().GetFrame();

    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));
    wxASSERT(pFrame);
    wxASSERT(wxDynamicCast(pFrame, CMainFrame));
    wxASSERT(m_pTaskPane);
    wxASSERT(m_pListPane);

    pFrame->UpdateStatusText(_("Attaching to project..."));

    CDlgAttachProject* pDlg = new CDlgAttachProject(this);
    wxASSERT(pDlg);

    iAnswer = pDlg->ShowModal();

    if (wxID_OK == iAnswer) {
        pDoc->ProjectAttach(
            pDlg->GetProjectAddress(), 
            pDlg->GetProjectAccountKey()
        );
    }

    if (pDlg)
        pDlg->Destroy();

    pFrame->UpdateStatusText(wxT(""));

    UpdateSelection();
    pFrame->FireRefreshView();

    wxLogTrace(wxT("Function Start/End"), wxT("CViewProjects::OnProjectAttach - Function End"));
}


void CViewProjects::OnProjectWebsiteClicked( wxEvent& event ) {
    wxLogTrace(wxT("Function Start/End"), wxT("CViewProjects::OnProjectWebsiteClicked - Function Begin"));

    CMainFrame*         pFrame = wxGetApp().GetFrame();

    wxASSERT(pFrame);
    wxASSERT(wxDynamicCast(pFrame, CMainFrame));
    wxASSERT(m_pTaskPane);
    wxASSERT(m_pListPane);

    pFrame->UpdateStatusText(_("Launching browser..."));

    int website_task_index = event.GetId() - ID_TASK_PROJECT_WEB_PROJDEF_MIN;
    pFrame->ExecuteBrowserLink(
        m_TaskGroups[1]->m_Tasks[website_task_index]->m_strWebSiteLink
    );

    pFrame->UpdateStatusText(wxT(""));

    UpdateSelection();
    pFrame->FireRefreshView();

    wxLogTrace(wxT("Function Start/End"), wxT("CViewProjects::OnProjectWebsiteClicked - Function End"));
}


wxInt32 CViewProjects::GetDocCount() {
    CMainDocument* pDoc      = wxGetApp().GetDocument();

    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));

    return pDoc->GetProjectCount();
}


wxString CViewProjects::OnListGetItemText(long item, long column) const {
    CProject* project     = m_ProjectCache.at(item);
    wxString  strBuffer   = wxEmptyString;

    switch(column) {
    case COLUMN_PROJECT:
        strBuffer = project->m_strProjectName;
        break;
    case COLUMN_ACCOUNTNAME:
        strBuffer = project->m_strAccountName;
        break;
    case COLUMN_TEAMNAME:
        strBuffer = project->m_strTeamName;
        break;
    case COLUMN_TOTALCREDIT:
        strBuffer = project->m_strTotalCredit;
        break;
    case COLUMN_AVGCREDIT:
        strBuffer = project->m_strAVGCredit;
        break;
    case COLUMN_RESOURCESHARE:
        strBuffer = project->m_strResourceShare;
        break;
    case COLUMN_STATUS:
        strBuffer = project->m_strStatus;
        break;
    }

    return strBuffer;
}


wxString CViewProjects::OnDocGetItemText(long item, long column) const {
    wxString       strBuffer = wxEmptyString;
    std::string foo;

    CMainDocument* pDoc = wxGetApp().GetDocument();
    PROJECT* project = pDoc->project(item);
    switch(column) {
    case COLUMN_PROJECT:
        project->get_name(foo);
        strBuffer = wxString(foo.c_str());
        break;
    case COLUMN_ACCOUNTNAME:
        FormatAccountName(item, strBuffer);
        break;
    case COLUMN_TEAMNAME:
        FormatTeamName(item, strBuffer);
        break;
    case COLUMN_TOTALCREDIT:
        FormatTotalCredit(item, strBuffer);
        break;
    case COLUMN_AVGCREDIT:
        FormatAVGCredit(item, strBuffer);
        break;
    case COLUMN_RESOURCESHARE:
        FormatResourceShare(item, strBuffer);
        break;
    case COLUMN_STATUS:
        FormatStatus(item, strBuffer);
        break;
    }

    return strBuffer;
}


wxInt32 CViewProjects::AddCacheElement() {
    CProject* pItem = new CProject();
    wxASSERT(pItem);
    if (pItem) {
        m_ProjectCache.push_back(pItem);
        return 0;
    }
    return -1;
}


wxInt32 CViewProjects::EmptyCache() {
    unsigned int i;
    for (i=0; i<m_ProjectCache.size(); i++) {
        delete m_ProjectCache[i];
    }
    m_ProjectCache.clear();
    return 0;
}


wxInt32 CViewProjects::GetCacheCount() {
    return m_ProjectCache.size();
}


wxInt32 CViewProjects::RemoveCacheElement() {
    delete m_ProjectCache.back();
    m_ProjectCache.erase(m_ProjectCache.end() - 1);
    return 0;
}


wxInt32 CViewProjects::UpdateCache(long item, long column, wxString& strNewData) {
    CProject* project     = m_ProjectCache.at(item);

    switch(column) {
        case COLUMN_PROJECT:
            project->m_strProjectName = strNewData;
            break;
        case COLUMN_ACCOUNTNAME:
            project->m_strAccountName = strNewData;
            break;
        case COLUMN_TEAMNAME:
            project->m_strTeamName = strNewData;
            break;
        case COLUMN_TOTALCREDIT:
            project->m_strTotalCredit = strNewData;
            break;
        case COLUMN_AVGCREDIT:
            project->m_strAVGCredit = strNewData;
            break;
        case COLUMN_RESOURCESHARE:
            project->m_strResourceShare = strNewData;
            break;
        case COLUMN_STATUS:
            project->m_strStatus = strNewData;
            break;
    }

    return 0;
}


void CViewProjects::UpdateSelection() {
    unsigned int        i;
    CTaskItemGroup*     pGroup = NULL;
    CTaskItem*          pItem = NULL;
    PROJECT*            project = NULL;
    CMainDocument*      pDoc = wxGetApp().GetDocument();

    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));
    wxASSERT(NULL != m_pTaskPane);

    // Update the tasks static box buttons
    //
    pGroup = m_TaskGroups[0];
    if (m_pListPane->GetSelectedItemCount()) {
        project = pDoc->project(m_pListPane->GetFirstSelected());
        m_pTaskPane->EnableTask(pGroup->m_Tasks[BTN_UPDATE]);
        m_pTaskPane->EnableTask(pGroup->m_Tasks[BTN_SUSPEND]);
        if (project->suspended_via_gui) {
            m_pTaskPane->UpdateTask(
                pGroup->m_Tasks[BTN_SUSPEND], _("Resume"), _("Resume work for this project.")
            );
        } else {
            m_pTaskPane->UpdateTask(
                pGroup->m_Tasks[BTN_SUSPEND], _("Suspend"), _("Suspend work for this project.")
            );
        }
        m_pTaskPane->EnableTask(pGroup->m_Tasks[BTN_NOWORK]);
        if (project->dont_request_more_work) {
            m_pTaskPane->UpdateTask(
                pGroup->m_Tasks[BTN_NOWORK], _("Allow new work"), _("Allow fetching new work for this project.")
            );
        } else {
            m_pTaskPane->UpdateTask(
                pGroup->m_Tasks[BTN_NOWORK], _("No new work"), _("Don't fetch new work for this project.")
            );
        }
        m_pTaskPane->EnableTask(pGroup->m_Tasks[BTN_RESET]);
        m_pTaskPane->EnableTask(pGroup->m_Tasks[BTN_DETACH]);
        m_pTaskPane->EnableTask(pGroup->m_Tasks[BTN_ATTACH]);
    } else {
        m_pTaskPane->DisableTaskGroupTasks(pGroup);
        m_pTaskPane->EnableTask(pGroup->m_Tasks[BTN_ATTACH]);
    }

    // Update the websites list
    //
    if (m_TaskGroups.size() > 1) {

        // Delete task group, objects, and controls.
        pGroup = m_TaskGroups[1];

        m_pTaskPane->DeleteTaskGroupAndTasks(pGroup);
        for (i=0; i<pGroup->m_Tasks.size(); i++) {
            delete pGroup->m_Tasks[i];
        }
        pGroup->m_Tasks.clear();
        delete pGroup;

        pGroup = NULL;

        m_TaskGroups.erase( m_TaskGroups.begin() + 1 );
    }

    // If something is selected create the tasks and controls
    if (m_pListPane->GetSelectedItemCount()) {
        project = pDoc->project(m_pListPane->GetFirstSelected());

        // Create the web sites task group
  	    pGroup = new CTaskItemGroup( _("Web sites") );
	    m_TaskGroups.push_back( pGroup );

        // Default project url
        pItem = new CTaskItem(
            project->project_name.c_str(), 
            wxT(""), 
            project->master_url.c_str(),
            ID_TASK_PROJECT_WEB_PROJDEF_MIN
        );
        pGroup->m_Tasks.push_back(pItem);


        // Project defined urls
        for (i=0;(i<project->gui_urls.size())&&(i<=ID_TASK_PROJECT_WEB_PROJDEF_MAX);i++) {
            pItem = new CTaskItem(
                project->gui_urls[i].name.c_str(),
                project->gui_urls[i].description.c_str(),
                project->gui_urls[i].url.c_str(),
                ID_TASK_PROJECT_WEB_PROJDEF_MIN + 1 + i
            );
            pGroup->m_Tasks.push_back(pItem);
        }
    }

    m_pTaskPane->UpdateControls();
}



wxInt32 CViewProjects::FormatAccountName(wxInt32 item, wxString& strBuffer) const {
    CMainDocument* pDoc = wxGetApp().GetDocument();

    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));

    strBuffer.Clear();

    pDoc->GetProjectAccountName(item, strBuffer);

    return 0;
}


wxInt32 CViewProjects::FormatTeamName(wxInt32 item, wxString& strBuffer) const {
    CMainDocument* pDoc = wxGetApp().GetDocument();

    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));

    //strBuffer.Clear();

    PROJECT* project = pDoc->project(item);
    strBuffer = wxString(project->team_name.c_str());

    return 0;
}


wxInt32 CViewProjects::FormatTotalCredit(wxInt32 item, wxString& strBuffer) const {
    float fBuffer;
    CMainDocument* pDoc = wxGetApp().GetDocument();

    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));

    strBuffer.Clear();

    pDoc->GetProjectTotalCredit(item, fBuffer);
    strBuffer.Printf(wxT("%0.2f"), fBuffer);

    return 0;
}


wxInt32 CViewProjects::FormatAVGCredit(wxInt32 item, wxString& strBuffer) const {
    float fBuffer;
    CMainDocument* pDoc = wxGetApp().GetDocument();

    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));

    strBuffer.Clear();

    pDoc->GetProjectAvgCredit(item, fBuffer);
    strBuffer.Printf(wxT("%0.2f"), fBuffer);

    return 0;
}


wxInt32 CViewProjects::FormatResourceShare(wxInt32 item, wxString& strBuffer) const {
    float fResourceShareBuffer;
    float fTotalResourceShareBuffer;
    CMainDocument* pDoc = wxGetApp().GetDocument();

    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));

    strBuffer.Clear();

    pDoc->GetProjectResourceShare(item, fResourceShareBuffer);
    pDoc->GetProjectTotalResourceShare(item, fTotalResourceShareBuffer);
    strBuffer.Printf(wxT("%0.0f (%0.2f%%)"), fResourceShareBuffer, ((fResourceShareBuffer / fTotalResourceShareBuffer) * 100));

    return 0;
}

static void comma_append(wxString& existing, const wxChar* additional) {
    if (existing.size() == 0) {
        existing = additional;
    } else {
        existing = existing + ", " + additional;
    }
}

wxInt32 CViewProjects::FormatStatus(wxInt32 item, wxString& status) const {
    wxInt32 iNextRPC;
    CMainDocument* pDoc = wxGetApp().GetDocument();

    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));

    status.Clear();

    PROJECT* p = pDoc->project(item);
    if (p->suspended_via_gui) {
        comma_append(status, _("Suspended by user"));
    }
    if (p->dont_request_more_work) {
        comma_append(status, _("Won't get new work"));
    }
    if (p->sched_rpc_pending) {
        comma_append(status, _("Scheduler request pending"));
    }
    pDoc->GetProjectMinRPCTime(item, iNextRPC);
    wxDateTime dtNextRPC((time_t)iNextRPC);
    wxDateTime dtNow(wxDateTime::Now());
    if (dtNextRPC > dtNow) {
        wxTimeSpan tsNextRPC(dtNextRPC - dtNow);
        comma_append(status, _("Communication deferred ") + tsNextRPC.Format());
    }

    return 0;
}


bool CViewProjects::IsWebsiteLink(const wxString& strLink) {
    bool bReturnValue = false;

    if (strLink.StartsWith(wxT("web:")))
        bReturnValue = true;

    return bReturnValue;
}


wxInt32 CViewProjects::ConvertWebsiteIndexToLink(wxInt32 iProjectIndex, wxInt32 iWebsiteIndex, wxString& strLink) {
    strLink.Printf(wxT("web:%d:%d"), iProjectIndex, iWebsiteIndex);
    return 0;
}


wxInt32 CViewProjects::ConvertLinkToWebsiteIndex(const wxString& strLink, wxInt32& iProjectIndex, wxInt32& iWebsiteIndex) {
    wxString strTemplate = strLink;
    wxString strBuffer = wxEmptyString;

    strTemplate.Replace(wxT("web:"), wxEmptyString);

    strBuffer = strTemplate;
    strBuffer.Remove(strBuffer.Find(wxT(":")));
    strBuffer.ToLong((long*) &iProjectIndex);

    strBuffer = strTemplate;
    strBuffer = strBuffer.Mid(strBuffer.Find(wxT(":")) + 1);
    strBuffer.ToLong((long*) &iWebsiteIndex);

    return 0;
}


const char *BOINC_RCSID_b4edf777fc = "$Id$";
