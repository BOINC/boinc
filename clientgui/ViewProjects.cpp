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
    m_fTotalCredit = 0.0;
    m_fAVGCredit = 0.0;
    m_fResourceShare = 0.0;
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
    EVT_LIST_COL_CLICK(ID_LIST_PROJECTSVIEW, CViewProjects::OnColClick)
    EVT_LIST_CACHE_HINT(ID_LIST_PROJECTSVIEW, CViewProjects::OnCacheHint)
END_EVENT_TABLE ()


static CViewProjects* myCViewProjects;

static int CompareViewProjectsItems(int *iRowIndex1, int *iRowIndex2) {
    CProject*  project1 = myCViewProjects->m_ProjectCache.at(*iRowIndex1);
    CProject*  project2 = myCViewProjects->m_ProjectCache.at(*iRowIndex2);
    int             result = 0;
    
    switch (myCViewProjects->m_iSortColumn) {
        case COLUMN_PROJECT:
	result = project1->m_strProjectName.CmpNoCase(project2->m_strProjectName);
        break;
    case COLUMN_ACCOUNTNAME:
	result = project1->m_strAccountName.CmpNoCase(project2->m_strAccountName);
        break;
    case COLUMN_TEAMNAME:
	result = project1->m_strTeamName.CmpNoCase(project2->m_strTeamName);
        break;
    case COLUMN_TOTALCREDIT:
        if (project1->m_fTotalCredit < project2->m_fTotalCredit) {
            result = -1;
        } else if (project1->m_fTotalCredit > project2->m_fTotalCredit) {
            result = 1;
        }
        break;
    case COLUMN_AVGCREDIT:
        if (project1->m_fAVGCredit < project2->m_fAVGCredit) {
            result = -1;
        } else if (project1->m_fAVGCredit > project2->m_fAVGCredit) {
            result = 1;
        }
        break;
    case COLUMN_RESOURCESHARE:
        if (project1->m_fResourceShare < project2->m_fResourceShare) {
            result = -1;
        } else if (project1->m_fResourceShare > project2->m_fResourceShare) {
            result = 1;
        }
        break;
    case COLUMN_STATUS:
	result = project1->m_strStatus.CmpNoCase(project2->m_strStatus);
        break;
    }

    return (myCViewProjects->m_bReverseSort ? result * (-1) : result);
}


CViewProjects::CViewProjects()
{}


CViewProjects::CViewProjects(wxNotebook* pNotebook) :
    CBOINCBaseView(pNotebook, ID_TASK_PROJECTSVIEW, DEFAULT_TASK_FLAGS, ID_LIST_PROJECTSVIEW, DEFAULT_LIST_MULTI_SEL_FLAGS)
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

    m_iProgressColumn = COLUMN_RESOURCESHARE;
 
    // Needed by static sort routine;
    myCViewProjects = this;
    m_funcSortCompare = CompareViewProjectsItems;
   
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
    int row;

    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));
    wxASSERT(pFrame);
    wxASSERT(wxDynamicCast(pFrame, CAdvancedFrame));

    pFrame->UpdateStatusText(_("Updating project..."));
    row = -1;
    while (1) {
        // Step through all selected items
        row = m_pListPane->GetNextItem(row, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
        if (row < 0) break;
        
        pDoc->ProjectUpdate(m_iSortedIndexes[row]);
    }
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
    int row;

    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));
    wxASSERT(pFrame);
    wxASSERT(wxDynamicCast(pFrame, CAdvancedFrame));
    wxASSERT(m_pListPane);

    row = -1;
    while (1) {
        // Step through all selected items
        row = m_pListPane->GetNextItem(row, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
        if (row < 0) break;
        
        PROJECT* project = pDoc->project(m_iSortedIndexes[row]);
        if (project) {
            if (project->suspended_via_gui) {
                pFrame->UpdateStatusText(_("Resuming project..."));
                pDoc->ProjectResume(m_iSortedIndexes[row]);
                pFrame->UpdateStatusText(wxT(""));
            } else {
                pFrame->UpdateStatusText(_("Suspending project..."));
                pDoc->ProjectSuspend(m_iSortedIndexes[row]);
                pFrame->UpdateStatusText(wxT(""));
            }
        }
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
    int row;

    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));
    wxASSERT(pFrame);
    wxASSERT(wxDynamicCast(pFrame, CAdvancedFrame));
    wxASSERT(m_pListPane);

    row = -1;
    while (1) {
        // Step through all selected items
        row = m_pListPane->GetNextItem(row, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
        if (row < 0) break;
        
        PROJECT* project = pDoc->project(m_iSortedIndexes[row]);
        if (project) {
            if (project->dont_request_more_work) {
                pFrame->UpdateStatusText(_("Telling project to allow additional task downloads..."));
                pDoc->ProjectAllowMoreWork(m_iSortedIndexes[row]);
                pFrame->UpdateStatusText(wxT(""));
            } else {
                pFrame->UpdateStatusText(_("Telling project to not fetch any additional tasks..."));
                pDoc->ProjectNoMoreWork(m_iSortedIndexes[row]);
                pFrame->UpdateStatusText(wxT(""));
            }
        }
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
    int row;

    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));
    wxASSERT(pFrame);
    wxASSERT(wxDynamicCast(pFrame, CAdvancedFrame));

    if (!pDoc->IsUserAuthorized())
        return;

    pFrame->UpdateStatusText(_("Resetting project..."));

    row = -1;
    while (1) {
        // Step through all selected items
        row = m_pListPane->GetNextItem(row, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
        if (row < 0) break;
        
        pProject = m_ProjectCache.at(m_iSortedIndexes[row]);

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
            pDoc->ProjectReset(m_iSortedIndexes[row]);
        }
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
    int row;

    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));
    wxASSERT(pFrame);
    wxASSERT(wxDynamicCast(pFrame, CAdvancedFrame));

    if (!pDoc->IsUserAuthorized())
        return;

    pFrame->UpdateStatusText(_("Detaching from project..."));

    row = -1;
    while (1) {
        // Step through all selected items
        row = m_pListPane->GetNextItem(row, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
        if (row < 0) break;
        
        pProject = m_ProjectCache.at(m_iSortedIndexes[row]);

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
            pDoc->ProjectDetach(m_iSortedIndexes[row]);
        }
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
        m_iSortedIndexes.Add((int)m_ProjectCache.size()-1);
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
    m_iSortedIndexes.Clear();
    return 0;
}


wxInt32 CViewProjects::GetCacheCount() {
    return (wxInt32)m_ProjectCache.size();
}


wxInt32 CViewProjects::RemoveCacheElement() {
    unsigned int i;
    delete m_ProjectCache.back();
    m_ProjectCache.erase(m_ProjectCache.end() - 1);
    m_iSortedIndexes.Clear();
    for (i=0; i<m_ProjectCache.size(); i++) {
        m_iSortedIndexes.Add(i);
    }
    return 0;
}


void CViewProjects::UpdateSelection() {
    CTaskItemGroup*     pGroup = NULL;
    PROJECT*            project = NULL;
    CMainDocument*      pDoc = wxGetApp().GetDocument();
    int                 i, n, row;
    bool                wasSuspended=false, wasNoNewWork=false;

    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));
    wxASSERT(m_pTaskPane);
    wxASSERT(m_pListPane);


    CBOINCBaseView::PreUpdateSelection();


    // Update the tasks static box buttons
    //
    pGroup = m_TaskGroups[0];

    n = m_pListPane->GetSelectedItemCount();
    if (n > 0) {
        m_pTaskPane->EnableTaskGroupTasks(pGroup);
    } else {
        m_pTaskPane->DisableTaskGroupTasks(pGroup);
        UpdateWebsiteSelection(GRP_WEBSITES, NULL);
        if(m_TaskGroups.size()>1) {
            m_pTaskPane->DisableTaskGroupTasks(m_TaskGroups[1]);
        }
    }
   
    row = -1;
    for (i=0; i<n; i++) {
        // Step through all selected items
        row = m_pListPane->GetNextItem(row, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
        if (row < 0) break;     // Should never happen
        
        project = pDoc->project(m_iSortedIndexes[row]);
        if (!project) {
            m_pTaskPane->DisableTaskGroupTasks(pGroup);
            if(m_TaskGroups.size()>1) {
                m_pTaskPane->DisableTaskGroupTasks(m_TaskGroups[1]);
            }
            return;
        }

        if (i == 0) {
            wasSuspended = project->suspended_via_gui;
             if (project->suspended_via_gui) {
                m_pTaskPane->UpdateTask(
                    pGroup->m_Tasks[BTN_SUSPEND], _("Resume"), _("Resume tasks for this project.")
                );
            } else {
                m_pTaskPane->UpdateTask(
                    pGroup->m_Tasks[BTN_SUSPEND], _("Suspend"), _("Suspend tasks for this project.")
                );
            }
        } else {
            if (wasSuspended != project->suspended_via_gui) {
                // Disable Suspend / Resume button if the multiple selection
                // has a mix of suspended and not suspended projects
                m_pTaskPane->DisableTask(pGroup->m_Tasks[BTN_SUSPEND]);
            }
        }

        if (i == 0) {
            wasNoNewWork = project->dont_request_more_work;
            if (project->dont_request_more_work) {
                m_pTaskPane->UpdateTask(
                    pGroup->m_Tasks[BTN_NOWORK], _("Allow new tasks"), _("Allow fetching new tasks for this project.")
                );
            } else {
                m_pTaskPane->UpdateTask(
                    pGroup->m_Tasks[BTN_NOWORK], _("No new tasks"), _("Don't fetch new tasks for this project.")
                );
            }
        } else {
            if (wasNoNewWork != project->dont_request_more_work) {
                // Disable Allow New Work / No New Work button if the multiple 
                // selection has a mix of Allow New Work and No New Work projects
                m_pTaskPane->DisableTask(pGroup->m_Tasks[BTN_NOWORK]);
            }
        }
        
        if (project->attached_via_acct_mgr) {
            m_pTaskPane->DisableTask(pGroup->m_Tasks[BTN_DETACH]);
        }
        
        if (n == 1) {
            UpdateWebsiteSelection(GRP_WEBSITES, project);
            if(m_TaskGroups.size()>1) {
                m_pTaskPane->EnableTaskGroupTasks(m_TaskGroups[1]);
            }
        } else {
            UpdateWebsiteSelection(GRP_WEBSITES, NULL);
            if(m_TaskGroups.size()>1) {
                m_pTaskPane->DisableTaskGroupTasks(m_TaskGroups[1]);
            }
        }
    }

    CBOINCBaseView::PostUpdateSelection();
}


bool CViewProjects::SynchronizeCacheItem(wxInt32 iRowIndex, wxInt32 iColumnIndex) {
    wxString    strDocumentText  = wxEmptyString;
    float       fDocumentFloat = 0.0;
    CProject*   project = m_ProjectCache.at(m_iSortedIndexes[iRowIndex]);

    strDocumentText.Empty();

    switch (iColumnIndex) {
        case COLUMN_PROJECT:
            GetDocProjectName(m_iSortedIndexes[iRowIndex], strDocumentText);
            if (!strDocumentText.IsSameAs(project->m_strProjectName)) {
                project->m_strProjectName = strDocumentText;
                return true;
            }
            break;
        case COLUMN_ACCOUNTNAME:
            GetDocAccountName(m_iSortedIndexes[iRowIndex], strDocumentText);
            if (!strDocumentText.IsSameAs(project->m_strAccountName)) {
                project->m_strAccountName = strDocumentText;
                return true;
            }
           break;
        case COLUMN_TEAMNAME:
            GetDocTeamName(m_iSortedIndexes[iRowIndex], strDocumentText);
            if (!strDocumentText.IsSameAs(project->m_strTeamName)) {
                project->m_strTeamName = strDocumentText;
                return true;
            }
            break;
        case COLUMN_TOTALCREDIT:
            GetDocTotalCredit(m_iSortedIndexes[iRowIndex], fDocumentFloat);
            if (fDocumentFloat != project->m_fTotalCredit) {
                project->m_fTotalCredit = fDocumentFloat;
                return true;
            }
            break;
        case COLUMN_AVGCREDIT:
            GetDocAVGCredit(m_iSortedIndexes[iRowIndex], fDocumentFloat);
            if (fDocumentFloat != project->m_fAVGCredit) {
                project->m_fAVGCredit = fDocumentFloat;
                return true;
            }
            break;
        case COLUMN_RESOURCESHARE:
            GetDocResourceShare(m_iSortedIndexes[iRowIndex], fDocumentFloat);
            if (fDocumentFloat != project->m_fResourceShare) {
                project->m_fResourceShare = fDocumentFloat;
                return true;
            }
            break;
        case COLUMN_STATUS:
            GetDocStatus(m_iSortedIndexes[iRowIndex], strDocumentText);
            if (!strDocumentText.IsSameAs(project->m_strStatus)) {
                project->m_strStatus = strDocumentText;
                return true;
            }
            break;
    }

    return false;
}


void CViewProjects::GetDocProjectName(wxInt32 item, wxString& strBuffer) const {
    PROJECT* project = wxGetApp().GetDocument()->project(item);
    std::string project_name;

    if (project) {
        project->get_name(project_name);
        strBuffer = wxString(project_name.c_str(), wxConvUTF8);
    } else {
        strBuffer = wxEmptyString;
    }
}


wxInt32 CViewProjects::FormatProjectName(wxInt32 item, wxString& strBuffer) const {
    CProject* project = m_ProjectCache.at(m_iSortedIndexes[item]);
    strBuffer = HtmlEntityDecode(project->m_strProjectName);

    return 0;
}


void CViewProjects::GetDocAccountName(wxInt32 item, wxString& strBuffer) const {
    PROJECT* project = wxGetApp().GetDocument()->project(item);

    if (project) {
        strBuffer = wxString(project->user_name.c_str(), wxConvUTF8);
    } else {
        strBuffer = wxEmptyString;
    }
}


wxInt32 CViewProjects::FormatAccountName(wxInt32 item, wxString& strBuffer) const {
    CProject* project = m_ProjectCache.at(m_iSortedIndexes[item]);
    strBuffer = HtmlEntityDecode(project->m_strAccountName);

    return 0;
}


void CViewProjects::GetDocTeamName(wxInt32 item, wxString& strBuffer) const {
    PROJECT* project = wxGetApp().GetDocument()->project(item);

    if (project) {
        strBuffer = wxString(project->team_name.c_str(), wxConvUTF8);
    } else {
        strBuffer = wxEmptyString;
    }
}


wxInt32 CViewProjects::FormatTeamName(wxInt32 item, wxString& strBuffer) const {
    CProject* project = m_ProjectCache.at(m_iSortedIndexes[item]);
    strBuffer = HtmlEntityDecode(project->m_strTeamName);

    return 0;
}


void CViewProjects::GetDocTotalCredit(wxInt32 item, float& fBuffer) const {
    PROJECT* project = wxGetApp().GetDocument()->project(item);

    if (project) {
        fBuffer = project->user_total_credit;
    } else {
        fBuffer = 0.0;
    }
}


wxInt32 CViewProjects::FormatTotalCredit(wxInt32 item, wxString& strBuffer) const {
    CProject* project = m_ProjectCache.at(m_iSortedIndexes[item]);
    strBuffer.Printf(wxT("%0.2f"), project->m_fTotalCredit);

    return 0;
}


void CViewProjects::GetDocAVGCredit(wxInt32 item, float& fBuffer) const {
    PROJECT* project = wxGetApp().GetDocument()->project(item);

    if (project) {
        fBuffer = project->user_expavg_credit;
    } else {
        fBuffer = 0.0;
    }
}


wxInt32 CViewProjects::FormatAVGCredit(wxInt32 item, wxString& strBuffer) const {
    CProject* project = m_ProjectCache.at(m_iSortedIndexes[item]);
    strBuffer.Printf(wxT("%0.2f"), project->m_fAVGCredit);

    return 0;
}


void CViewProjects::GetDocResourceShare(wxInt32 item, float& fBuffer) const {
    PROJECT* project = wxGetApp().GetDocument()->project(item);

    if (project) {
        fBuffer = project->resource_share;
    } else {
        fBuffer = 0.0;
    }
}


wxInt32 CViewProjects::FormatResourceShare(wxInt32 item, wxString& strBuffer) const {
    CMainDocument* pDoc = wxGetApp().GetDocument();
    CProject* project = m_ProjectCache.at(m_iSortedIndexes[item]);

    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));

    if (project && pDoc) {
        strBuffer.Printf(wxT("%0.0f (%0.2f%%)"), 
            project->m_fResourceShare, 
            ((project->m_fResourceShare / pDoc->m_fProjectTotalResourceShare) * 100)
        );
    }
        
    return 0;
}


void CViewProjects::GetDocStatus(wxInt32 item, wxString& strBuffer) const {
    PROJECT* project = wxGetApp().GetDocument()->project(item);

    if (project) {
        if (project->suspended_via_gui) {
            append_to_status(strBuffer, _("Suspended by user"));
        }
        if (project->dont_request_more_work) {
            append_to_status(strBuffer, _("Won't get new tasks"));
        }
        if (project->ended) {
            append_to_status(strBuffer, _("Project ended - OK to detach"));
        }
        if (project->detach_when_done) {
            append_to_status(strBuffer, _("Will detach when tasks done"));
        }
        if (project->sched_rpc_pending) {
            append_to_status(strBuffer, _("Scheduler request pending"));
			append_to_status(strBuffer, wxString(rpc_reason_string(project->sched_rpc_pending), wxConvUTF8));
        }
        if (project->scheduler_rpc_in_progress) {
            append_to_status(strBuffer, _("Scheduler request in progress"));
        }
        wxDateTime dtNextRPC((time_t)project->min_rpc_time);
        wxDateTime dtNow(wxDateTime::Now());
        if (dtNextRPC > dtNow) {
            wxTimeSpan tsNextRPC(dtNextRPC - dtNow);
            append_to_status(strBuffer, _("Communication deferred ") + tsNextRPC.Format());
        }
    }
}


wxInt32 CViewProjects::FormatStatus(wxInt32 item, wxString& strBuffer) const {
    CProject* project = m_ProjectCache.at(m_iSortedIndexes[item]);
    strBuffer = project->m_strStatus;

    return 0;
}


double CViewProjects::GetProgressValue(long item) {
    CMainDocument* pDoc = wxGetApp().GetDocument();
    CProject* project = m_ProjectCache.at(m_iSortedIndexes[item]);

    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));

    if (project && pDoc) {
        return (project->m_fResourceShare / pDoc->m_fProjectTotalResourceShare);
    }

    return 0.0;
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
