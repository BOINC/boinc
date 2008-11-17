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
// 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma implementation "ViewProjects.h"
#endif

#include "stdwx.h"
#include "str_util.h"
#include "BOINCGUIApp.h"
#include "BOINCBaseFrame.h"
#include "MainDocument.h"
#include "AdvancedFrame.h"
#include "BOINCTaskCtrl.h"
#include "BOINCListCtrl.h"
#include "ViewProjects.h"
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
#define BTN_UPDATE       0
#define BTN_SUSPEND      1
#define BTN_NOWORK       2
#define BTN_RESET        3
#define BTN_DETACH       4


CProject::CProject() {
}


CProject::~CProject() {
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
    EVT_CUSTOM_RANGE(wxEVT_COMMAND_BUTTON_CLICKED, ID_TASK_PROJECT_WEB_PROJDEF_MIN, ID_TASK_PROJECT_WEB_PROJDEF_MAX, CViewProjects::OnProjectWebsiteClicked)
    EVT_LIST_ITEM_SELECTED(ID_LIST_PROJECTSVIEW, CViewProjects::OnListSelected)
    EVT_LIST_ITEM_DESELECTED(ID_LIST_PROJECTSVIEW, CViewProjects::OnListDeselected)
END_EVENT_TABLE ()


CViewProjects::CViewProjects()
{}


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
	pGroup = new CTaskItemGroup( _("Commands") );
	m_TaskGroups.push_back( pGroup );

	pItem = new CTaskItem(
        _("Update"),
        _("Report all completed tasks, get latest credit, "
          "get latest preferences, and possibly get more tasks."),
        ID_TASK_PROJECT_UPDATE 
    );
    pGroup->m_Tasks.push_back( pItem );

	pItem = new CTaskItem(
        _("Suspend"),
        _("Suspend tasks for this project."),
        ID_TASK_PROJECT_SUSPEND 
    );
    pGroup->m_Tasks.push_back( pItem );

	pItem = new CTaskItem(
        _("No new tasks"),
        _("Don't get new tasks for this project."),
        ID_TASK_PROJECT_NONEWWORK 
    );
    pGroup->m_Tasks.push_back( pItem );

	pItem = new CTaskItem(
        _("Reset project"),
        _("Delete all files and tasks associated with this project, "
          "and get new tasks.  "
          "You can update the project "
          "first to report any completed tasks."),
        ID_TASK_PROJECT_RESET 
    );
    pGroup->m_Tasks.push_back( pItem );

	pItem = new CTaskItem(
        _("Detach"),
        _("Detach computer from this project.  "
          "Tasks in progress will be lost "
          "(use 'Update' first to report any completed tasks)."),
        ID_TASK_PROJECT_DETACH 
    );
    pGroup->m_Tasks.push_back( pItem );

    // Create Task Pane Items
    m_pTaskPane->UpdateControls();

    // Create List Pane Items
    m_pListPane->InsertColumn(COLUMN_PROJECT, _("Project"), wxLIST_FORMAT_LEFT, 150);
    m_pListPane->InsertColumn(COLUMN_ACCOUNTNAME, _("Account"), wxLIST_FORMAT_LEFT, 80);
    m_pListPane->InsertColumn(COLUMN_TEAMNAME, _("Team"), wxLIST_FORMAT_LEFT, 80);
    m_pListPane->InsertColumn(COLUMN_TOTALCREDIT, _("Work done"), wxLIST_FORMAT_RIGHT, 80);
    m_pListPane->InsertColumn(COLUMN_AVGCREDIT, _("Avg. work done"), wxLIST_FORMAT_RIGHT, 80);
    m_pListPane->InsertColumn(COLUMN_RESOURCESHARE, _("Resource share"), wxLIST_FORMAT_CENTRE, 85);
    m_pListPane->InsertColumn(COLUMN_STATUS, _("Status"), wxLIST_FORMAT_LEFT, 150);

    UpdateSelection();
}


CViewProjects::~CViewProjects() {
    EmptyCache();
    EmptyTasks();
}


wxString& CViewProjects::GetViewName() {
    static wxString strViewName(_("Projects"));
    return strViewName;
}


wxString& CViewProjects::GetViewDisplayName() {
    static wxString strViewName(_("Projects"));
    return strViewName;
}


const char** CViewProjects::GetViewIcon() {
    return proj_xpm;
}


void CViewProjects::OnProjectUpdate( wxCommandEvent& WXUNUSED(event) ) {
    wxLogTrace(wxT("Function Start/End"), wxT("CViewProjects::OnProjectUpdate - Function Begin"));

    CMainDocument*  pDoc   = wxGetApp().GetDocument();
    CAdvancedFrame* pFrame = wxDynamicCast(GetParent()->GetParent()->GetParent(), CAdvancedFrame);

    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));
    wxASSERT(pFrame);
    wxASSERT(wxDynamicCast(pFrame, CAdvancedFrame));

    pFrame->UpdateStatusText(_("Updating project..."));
    pDoc->ProjectUpdate(m_pListPane->GetFirstSelected());
    pFrame->UpdateStatusText(wxT(""));

    m_bForceUpdateSelection = true;
    UpdateSelection();
    pFrame->ResetReminderTimers();
    pFrame->FireRefreshView();

    wxLogTrace(wxT("Function Start/End"), wxT("CViewProjects::OnProjectUpdate - Function End"));
}


void CViewProjects::OnProjectSuspend( wxCommandEvent& WXUNUSED(event) ) {
    wxLogTrace(wxT("Function Start/End"), wxT("CViewProjects::OnProjectSuspend - Function Begin"));

    CMainDocument*  pDoc   = wxGetApp().GetDocument();
    CAdvancedFrame* pFrame = wxDynamicCast(GetParent()->GetParent()->GetParent(), CAdvancedFrame);

    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));
    wxASSERT(pFrame);
    wxASSERT(wxDynamicCast(pFrame, CAdvancedFrame));
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

    m_bForceUpdateSelection = true;
    UpdateSelection();
    pFrame->FireRefreshView();

    wxLogTrace(wxT("Function Start/End"), wxT("CViewProjects::OnProjectSuspend - Function End"));
}


void CViewProjects::OnProjectNoNewWork( wxCommandEvent& WXUNUSED(event) ) {
    wxLogTrace(wxT("Function Start/End"), wxT("CViewProjects::OnProjectNoNewWork - Function Begin"));

    CMainDocument*  pDoc   = wxGetApp().GetDocument();
    CAdvancedFrame* pFrame = wxDynamicCast(GetParent()->GetParent()->GetParent(), CAdvancedFrame);

    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));
    wxASSERT(pFrame);
    wxASSERT(wxDynamicCast(pFrame, CAdvancedFrame));
    wxASSERT(m_pListPane);

    PROJECT* project = pDoc->project(m_pListPane->GetFirstSelected());
    if (project->dont_request_more_work) {
        pFrame->UpdateStatusText(_("Telling project to allow additional task downloads..."));
        pDoc->ProjectAllowMoreWork(m_pListPane->GetFirstSelected());
        pFrame->UpdateStatusText(wxT(""));
    } else {
        pFrame->UpdateStatusText(_("Telling project to not fetch any additional tasks..."));
        pDoc->ProjectNoMoreWork(m_pListPane->GetFirstSelected());
        pFrame->UpdateStatusText(wxT(""));
    }

    m_bForceUpdateSelection = true;
    UpdateSelection();
    pFrame->FireRefreshView();

    wxLogTrace(wxT("Function Start/End"), wxT("CViewProjects::OnProjectNoNewWork - Function End"));
}




void CViewProjects::OnProjectReset( wxCommandEvent& WXUNUSED(event) ) {
    wxLogTrace(wxT("Function Start/End"), wxT("CViewProjects::OnProjectReset - Function Begin"));

    wxInt32         iAnswer        = 0; 
    wxString        strMessage     = wxEmptyString;
    CMainDocument*  pDoc           = wxGetApp().GetDocument();
    CAdvancedFrame* pFrame         = wxDynamicCast(GetParent()->GetParent()->GetParent(), CAdvancedFrame);
    CProject*       pProject       = NULL;

    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));
    wxASSERT(pFrame);
    wxASSERT(wxDynamicCast(pFrame, CAdvancedFrame));

    if (!pDoc->IsUserAuthorized())
        return;

    pFrame->UpdateStatusText(_("Resetting project..."));

    pProject = m_ProjectCache.at(m_pListPane->GetFirstSelected());

    strMessage.Printf(
        _("Are you sure you want to reset project '%s'?"), 
        pProject->m_strProjectName.c_str()
    );

    iAnswer = ::wxMessageBox(
        strMessage,
        _("Reset Project"),
        wxYES_NO | wxICON_QUESTION,
        this
    );

    if (wxYES == iAnswer) {
        pDoc->ProjectReset(m_pListPane->GetFirstSelected());
    }

    pFrame->UpdateStatusText(wxT(""));

    m_bForceUpdateSelection = true;
    UpdateSelection();
    pFrame->FireRefreshView();

    wxLogTrace(wxT("Function Start/End"), wxT("CViewProjects::OnProjectReset - Function End"));
}


void CViewProjects::OnProjectDetach( wxCommandEvent& WXUNUSED(event) ) {
    wxLogTrace(wxT("Function Start/End"), wxT("CViewProjects::OnProjectDetach - Function Begin"));

    wxInt32         iAnswer        = 0; 
    wxString        strMessage     = wxEmptyString;
    CMainDocument*  pDoc           = wxGetApp().GetDocument();
    CAdvancedFrame* pFrame         = wxDynamicCast(GetParent()->GetParent()->GetParent(), CAdvancedFrame);
    CProject*       pProject       = NULL;

    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));
    wxASSERT(pFrame);
    wxASSERT(wxDynamicCast(pFrame, CAdvancedFrame));

    if (!pDoc->IsUserAuthorized())
        return;

    pFrame->UpdateStatusText(_("Detaching from project..."));

    pProject = m_ProjectCache.at(m_pListPane->GetFirstSelected());

    strMessage.Printf(
        _("Are you sure you want to detach from project '%s'?"), 
        pProject->m_strProjectName.c_str()
    );

    iAnswer = ::wxMessageBox(
        strMessage,
        _("Detach from Project"),
        wxYES_NO | wxICON_QUESTION,
        this
    );

    if (wxYES == iAnswer) {
        pDoc->ProjectDetach(m_pListPane->GetFirstSelected());
    }

    pFrame->UpdateStatusText(wxT(""));

    m_bForceUpdateSelection = true;
    UpdateSelection();
    pFrame->FireRefreshView();

    wxLogTrace(wxT("Function Start/End"), wxT("CViewProjects::OnProjectDetach - Function End"));
}


void CViewProjects::OnProjectWebsiteClicked( wxEvent& event ) {
    wxLogTrace(wxT("Function Start/End"), wxT("CViewProjects::OnProjectWebsiteClicked - Function Begin"));

    CAdvancedFrame* pFrame      = wxDynamicCast(GetParent()->GetParent()->GetParent(), CAdvancedFrame);

    wxASSERT(pFrame);
    wxASSERT(wxDynamicCast(pFrame, CAdvancedFrame));

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
    return wxGetApp().GetDocument()->GetProjectCount();
}


wxString CViewProjects::OnListGetItemText(long item, long column) const {
    CProject* project     = NULL;
    wxString  strBuffer   = wxEmptyString;

    try {
        project = m_ProjectCache.at(item);
    } catch ( std::out_of_range ) {
        project = NULL;
    }

    if (project) {
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
    }

    return strBuffer;
}


wxString CViewProjects::OnDocGetItemText(long item, long column) const {
    wxString       strBuffer = wxEmptyString;

    switch(column) {
    case COLUMN_PROJECT:
        FormatProjectName(item, strBuffer);
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
    return (wxInt32)m_ProjectCache.size();
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
    CTaskItemGroup*     pGroup = NULL;
    PROJECT*            project = NULL;
    CMainDocument*      pDoc = wxGetApp().GetDocument();

    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));
    wxASSERT(m_pTaskPane);
    wxASSERT(m_pListPane);


    CBOINCBaseView::PreUpdateSelection();


    // Update the tasks static box buttons
    //
    pGroup = m_TaskGroups[0];
    if (m_pListPane->GetSelectedItemCount()) {
        project = pDoc->project(m_pListPane->GetFirstSelected());
        m_pTaskPane->EnableTask(pGroup->m_Tasks[BTN_UPDATE]);
        m_pTaskPane->EnableTask(pGroup->m_Tasks[BTN_SUSPEND]);
        if (project) {
            if (project->suspended_via_gui) {
                m_pTaskPane->UpdateTask(
                    pGroup->m_Tasks[BTN_SUSPEND], _("Resume"), _("Resume tasks for this project.")
                );
            } else {
                m_pTaskPane->UpdateTask(
                    pGroup->m_Tasks[BTN_SUSPEND], _("Suspend"), _("Suspend tasks for this project.")
                );
            }
        }
        m_pTaskPane->EnableTask(pGroup->m_Tasks[BTN_NOWORK]);
        if (project) {
            if (project->dont_request_more_work) {
                m_pTaskPane->UpdateTask(
                    pGroup->m_Tasks[BTN_NOWORK], _("Allow new tasks"), _("Allow fetching new tasks for this project.")
                );
            } else {
                m_pTaskPane->UpdateTask(
                    pGroup->m_Tasks[BTN_NOWORK], _("No new tasks"), _("Don't fetch new tasks for this project.")
                );
            }
        }
        m_pTaskPane->EnableTask(pGroup->m_Tasks[BTN_RESET]);
        if (project && project->attached_via_acct_mgr) {
            m_pTaskPane->DisableTask(pGroup->m_Tasks[BTN_DETACH]);
        } else {
            m_pTaskPane->EnableTask(pGroup->m_Tasks[BTN_DETACH]);
        }

        CBOINCBaseView::UpdateWebsiteSelection(GRP_WEBSITES, project);

    } else {
        m_pTaskPane->DisableTaskGroupTasks(pGroup);
        if (GetDocCount() <= 0) {
            if ( m_TaskGroups.size() > 1) {
                pGroup = m_TaskGroups[GRP_WEBSITES];
                m_pTaskPane->DeleteTaskGroupAndTasks(pGroup);
                pGroup->m_Tasks.clear();
            }
        }
    }

    CBOINCBaseView::PostUpdateSelection();
}



wxInt32 CViewProjects::FormatProjectName(wxInt32 item, wxString& strBuffer) const {
    PROJECT* project = wxGetApp().GetDocument()->project(item);
    std::string project_name;

    if (project) {
        project->get_name(project_name);
        strBuffer = HtmlEntityDecode(wxString(project_name.c_str(), wxConvUTF8));
    }

    return 0;
}


wxInt32 CViewProjects::FormatAccountName(wxInt32 item, wxString& strBuffer) const {
    PROJECT* project = wxGetApp().GetDocument()->project(item);

    if (project) {
        strBuffer = HtmlEntityDecode(wxString(project->user_name.c_str(), wxConvUTF8));
    }

    return 0;
}


wxInt32 CViewProjects::FormatTeamName(wxInt32 item, wxString& strBuffer) const {
    PROJECT* project = wxGetApp().GetDocument()->project(item);

    if (project) {
        strBuffer = HtmlEntityDecode(wxString(project->team_name.c_str(), wxConvUTF8));
    }

    return 0;
}


wxInt32 CViewProjects::FormatTotalCredit(wxInt32 item, wxString& strBuffer) const {
    PROJECT* project = wxGetApp().GetDocument()->project(item);

    if (project) {
        strBuffer.Printf(wxT("%0.2f"), project->user_total_credit);
    }

    return 0;
}


wxInt32 CViewProjects::FormatAVGCredit(wxInt32 item, wxString& strBuffer) const {
    PROJECT* project = wxGetApp().GetDocument()->project(item);

    if (project) {
        strBuffer.Printf(wxT("%0.2f"), project->user_expavg_credit);
    }

    return 0;
}


wxInt32 CViewProjects::FormatResourceShare(wxInt32 item, wxString& strBuffer) const {
    CMainDocument* pDoc = wxGetApp().GetDocument();
    PROJECT*       project = wxGetApp().GetDocument()->project(item);

    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));

    if (project && pDoc) {
        strBuffer.Printf(wxT("%0.0f (%0.2f%%)"), 
            project->resource_share, 
            ((project->resource_share / pDoc->m_fProjectTotalResourceShare) * 100)
        );
    }

    return 0;
}


wxInt32 CViewProjects::FormatStatus(wxInt32 item, wxString& status) const {
    PROJECT* project = wxGetApp().GetDocument()->project(item);

    if (project) {
        if (project->suspended_via_gui) {
            append_to_status(status, _("Suspended by user"));
        }
        if (project->dont_request_more_work) {
            append_to_status(status, _("Won't get new tasks"));
        }
        if (project->ended) {
            append_to_status(status, _("Project ended - OK to detach"));
        }
        if (project->detach_when_done) {
            append_to_status(status, _("Will detach when tasks done"));
        }
        if (project->sched_rpc_pending) {
            append_to_status(status, _("Scheduler request pending"));
			append_to_status(status, wxString(rpc_reason_string(project->sched_rpc_pending), wxConvUTF8));
        }
        if (project->scheduler_rpc_in_progress) {
            append_to_status(status, _("Scheduler request in progress"));
        }
        wxDateTime dtNextRPC((time_t)project->min_rpc_time);
        wxDateTime dtNow(wxDateTime::Now());
        if (dtNextRPC > dtNow) {
            wxTimeSpan tsNextRPC(dtNextRPC - dtNow);
            append_to_status(status, _("Communication deferred ") + tsNextRPC.Format());
        }
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


const char *BOINC_RCSID_b4edf777fc = "$Id: ViewProjects.cpp 14634 2008-01-29 14:28:56Z charlief $";
