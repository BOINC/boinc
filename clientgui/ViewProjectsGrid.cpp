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
#pragma implementation "ViewProjectsGrid.h"
#endif

#include "stdwx.h"
#include "str_util.h"
#include "BOINCGUIApp.h"
#include "BOINCBaseFrame.h"
#include "BOINCBaseView.h"
#include "BOINCGridCtrl.h"
#include "BOINCTaskCtrl.h"
#include "MainDocument.h"
#include "AdvancedFrame.h"
#include "ViewProjectsGrid.h"
#include "Events.h"


#include "res/proj.xpm"

#define COLUMN_PROJECT              0
#define COLUMN_ACCOUNTNAME          1
#define COLUMN_TEAMNAME             2
#define COLUMN_TOTALCREDIT          3
#define COLUMN_AVGCREDIT            4
#define COLUMN_RESOURCESHARE        5
#define COLUMN_STATUS               6
#define COLUMN_HIDDEN_URL           7
#define NUM_COLUMNS                 (COLUMN_HIDDEN_URL+1)

// groups that contain buttons
#define GRP_TASKS    0
#define GRP_WEBSITES 1

// buttons in the "tasks" area
#define BTN_UPDATE       0
#define BTN_SUSPEND      1
#define BTN_NOWORK       2
#define BTN_RESET        3
#define BTN_DETACH       4


IMPLEMENT_DYNAMIC_CLASS(CViewProjectsGrid, CBOINCBaseView)

BEGIN_EVENT_TABLE (CViewProjectsGrid, CBOINCBaseView)
    EVT_BUTTON(ID_TASK_PROJECT_UPDATE, CViewProjectsGrid::OnProjectUpdate)
    EVT_BUTTON(ID_TASK_PROJECT_SUSPEND, CViewProjectsGrid::OnProjectSuspend)
    EVT_BUTTON(ID_TASK_PROJECT_NONEWWORK, CViewProjectsGrid::OnProjectNoNewWork)
    EVT_BUTTON(ID_TASK_PROJECT_RESET, CViewProjectsGrid::OnProjectReset)
    EVT_BUTTON(ID_TASK_PROJECT_DETACH, CViewProjectsGrid::OnProjectDetach)
    EVT_CUSTOM_RANGE(wxEVT_COMMAND_BUTTON_CLICKED, ID_TASK_PROJECT_WEB_PROJDEF_MIN, ID_TASK_PROJECT_WEB_PROJDEF_MAX, CViewProjectsGrid::OnProjectWebsiteClicked)
	EVT_GRID_SELECT_CELL(CViewProjectsGrid::OnGridSelectCell)
	EVT_GRID_RANGE_SELECT(CViewProjectsGrid::OnGridSelectRange)
#if PREVENT_MULTIPLE_PROJECT_SELECTIONS
        EVT_GRID_CELL_LEFT_CLICK(CViewProjectsGrid::OnCellLeftClick)
#endif
END_EVENT_TABLE ()


CViewProjectsGrid::CViewProjectsGrid()
{}


CViewProjectsGrid::CViewProjectsGrid(wxNotebook* pNotebook) :
    CBOINCBaseView(pNotebook)
{
    //
    // Setup View
    //
    wxFlexGridSizer* itemFlexGridSizer = new wxFlexGridSizer(2, 0, 0);
    wxASSERT(itemFlexGridSizer);

    itemFlexGridSizer->AddGrowableRow(0);
    itemFlexGridSizer->AddGrowableCol(1);

	m_pTaskPane = new CBOINCTaskCtrl(this, ID_TASK_PROJECTSGRIDVIEW, DEFAULT_TASK_FLAGS);
    wxASSERT(m_pTaskPane);

	m_pGridPane = new CBOINCGridCtrl(this, ID_LIST_PROJECTSGRIDVIEW);
    wxASSERT(m_pGridPane);

    itemFlexGridSizer->Add(m_pTaskPane, 1, wxGROW|wxALL, 1);
    itemFlexGridSizer->Add(m_pGridPane, 1, wxGROW|wxALL, 1);

    SetSizer(itemFlexGridSizer);

    Layout();

	//setup task pane
	CTaskItemGroup* pGroup = NULL;
	CTaskItem*      pItem = NULL;

    wxASSERT(m_pTaskPane);
    wxASSERT(m_pGridPane);


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

    // Create Grid
    m_pGridPane->Setup();
    m_pGridPane->SetTable(new CBOINCGridTable(1,NUM_COLUMNS));
    m_pGridPane->SetSelectionMode(wxGrid::wxGridSelectRows);
    // init grid columns
    wxInt32 colSizes[] = {150,80,80,80,80,85,150,0};
    wxString colTitles[] = {_("Project"),_("Account"),_("Team"),_("Work done"),
                            _("Avg. work done"),_("Resource share"),_("Status"),wxEmptyString
                            };
    for(int i=0; i<NUM_COLUMNS;i++){
            m_pGridPane->SetColLabelValue(i,colTitles[i]);
            m_pGridPane->SetColSize(i,colSizes[i]);
    }
    //change the default cell renderer
    m_pGridPane->SetDefaultRenderer(new CBOINCGridCellProgressRenderer(COLUMN_RESOURCESHARE,false));
    //set column sort types
    m_pGridPane->SetColumnSortType(COLUMN_TOTALCREDIT,CST_FLOAT);
    m_pGridPane->SetColumnSortType(COLUMN_RESOURCESHARE,CST_FLOAT);
    m_pGridPane->SetColumnSortType(COLUMN_AVGCREDIT,CST_FLOAT);
    //
    m_pGridPane->SetPrimaryKeyColumns(COLUMN_HIDDEN_URL,-1);
    // Hide the URL column
    int min_width = m_pGridPane->GetColMinimalAcceptableWidth();
    m_pGridPane->SetColMinimalAcceptableWidth(0);
    m_pGridPane->SetColSize(COLUMN_HIDDEN_URL,0);
    m_pGridPane->SetColMinimalAcceptableWidth(min_width);
    UpdateSelection();
}


CViewProjectsGrid::~CViewProjectsGrid() {
    EmptyTasks();
}


wxString& CViewProjectsGrid::GetViewName() {
    static wxString strViewName(_("ProjectsGrid"));
    return strViewName;
}


wxString& CViewProjectsGrid::GetViewDisplayName() {
    static wxString strViewName(_("Projects"));
    return strViewName;
}


const char** CViewProjectsGrid::GetViewIcon() {
    return proj_xpm;
}


void CViewProjectsGrid::OnProjectUpdate( wxCommandEvent& WXUNUSED(event) ) {
    wxLogTrace(wxT("Function Start/End"), wxT("CViewProjectsGrid::OnProjectUpdate - Function Begin"));

    wxString        strProjectURL  = wxEmptyString;
    CMainDocument*  pDoc           = wxGetApp().GetDocument();
    CAdvancedFrame* pFrame         = wxDynamicCast(GetParent()->GetParent()->GetParent(), CAdvancedFrame);
    int i, n;

    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));
    wxASSERT(pFrame);
    wxASSERT(wxDynamicCast(pFrame, CAdvancedFrame));
    wxASSERT(m_pGridPane);

    pFrame->UpdateStatusText(_("Updating project..."));

    wxArrayInt arrSelRows = m_pGridPane->GetSelectedRows2();	
    n = (int)arrSelRows.GetCount();
    for(i=0; i<n; i++) {
        strProjectURL = m_pGridPane->GetCellValue(arrSelRows[i], COLUMN_HIDDEN_URL);

        pDoc->ProjectUpdate(HtmlEntityEncode(strProjectURL).Trim(false));
    }
    
    pFrame->UpdateStatusText(wxT(""));

    m_bForceUpdateSelection = true;
    UpdateSelection();
    pFrame->ResetReminderTimers();
    pFrame->FireRefreshView();

    wxLogTrace(wxT("Function Start/End"), wxT("CViewProjectsGrid::OnProjectUpdate - Function End"));
}


void CViewProjectsGrid::OnProjectSuspend( wxCommandEvent& WXUNUSED(event) ) {
    wxLogTrace(wxT("Function Start/End"), wxT("CViewProjectsGrid::OnProjectSuspend - Function Begin"));

    wxString        strProjectURL  = wxEmptyString;
    CMainDocument*  pDoc           = wxGetApp().GetDocument();
    CAdvancedFrame* pFrame         = wxDynamicCast(GetParent()->GetParent()->GetParent(), CAdvancedFrame);
    int i, n;

    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));
    wxASSERT(pFrame);
    wxASSERT(wxDynamicCast(pFrame, CAdvancedFrame));
    wxASSERT(m_pGridPane);

    wxArrayInt arrSelRows = m_pGridPane->GetSelectedRows2();	
    n = (int)arrSelRows.GetCount();
    for(i=0; i<n; i++) {
        strProjectURL = 
            HtmlEntityEncode(
                m_pGridPane->GetCellValue(
                    arrSelRows[i],
                    COLUMN_HIDDEN_URL
                ).Trim(false)
            );
        PROJECT* project = pDoc->project(strProjectURL);
        
        if (project->suspended_via_gui) {
            pFrame->UpdateStatusText(_("Resuming project..."));
            pDoc->ProjectResume(strProjectURL);
        } else {
            pFrame->UpdateStatusText(_("Suspending project..."));
            pDoc->ProjectSuspend(strProjectURL);
        }
    }
    pFrame->UpdateStatusText(wxT(""));
	
    m_bForceUpdateSelection = true;
    UpdateSelection();
    pFrame->FireRefreshView();

    wxLogTrace(wxT("Function Start/End"), wxT("CViewProjectsGrid::OnProjectSuspend - Function End"));
}


void CViewProjectsGrid::OnProjectNoNewWork( wxCommandEvent& WXUNUSED(event) ) {
    wxLogTrace(wxT("Function Start/End"), wxT("CViewProjectsGrid::OnProjectNoNewWork - Function Begin"));

    wxString        strProjectURL = wxEmptyString;
    CMainDocument*  pDoc           = wxGetApp().GetDocument();
    CAdvancedFrame* pFrame         = wxDynamicCast(GetParent()->GetParent()->GetParent(), CAdvancedFrame);
    int i, n;

    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));
    wxASSERT(pFrame);
    wxASSERT(wxDynamicCast(pFrame, CAdvancedFrame));
    wxASSERT(m_pGridPane);

    wxArrayInt arrSelRows = m_pGridPane->GetSelectedRows2();	
    n = (int)arrSelRows.GetCount();
    for(i=0; i<n; i++) {
        strProjectURL = 
            HtmlEntityEncode(
                m_pGridPane->GetCellValue(
                    arrSelRows[i],
                    COLUMN_HIDDEN_URL
                ).Trim(false)
            );
        PROJECT* project = pDoc->project(strProjectURL);

        if (project->dont_request_more_work) {
            pFrame->UpdateStatusText(_("Telling project to allow additional task downloads..."));
            pDoc->ProjectAllowMoreWork(strProjectURL);
        } else {
            pFrame->UpdateStatusText(_("Telling project to not fetch any additional tasks..."));
            pDoc->ProjectNoMoreWork(strProjectURL);
        }
    }
    pFrame->UpdateStatusText(wxT(""));
    
    m_bForceUpdateSelection = true;
    UpdateSelection();
    pFrame->FireRefreshView();

    wxLogTrace(wxT("Function Start/End"), wxT("CViewProjectsGrid::OnProjectNoNewWork - Function End"));
}




void CViewProjectsGrid::OnProjectReset( wxCommandEvent& WXUNUSED(event) ) {
    wxLogTrace(wxT("Function Start/End"), wxT("CViewProjectsGrid::OnProjectReset - Function Begin"));

    wxInt32         iAnswer        = 0;
    wxString        strProjectName = wxEmptyString;
    wxString        strProjectURL  = wxEmptyString;
    wxString        strMessage     = wxEmptyString;
    CMainDocument*  pDoc           = wxGetApp().GetDocument();
    CAdvancedFrame* pFrame         = wxDynamicCast(GetParent()->GetParent()->GetParent(), CAdvancedFrame);
    int i, n;

    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));
    wxASSERT(pFrame);
    wxASSERT(wxDynamicCast(pFrame, CAdvancedFrame));
    wxASSERT(m_pGridPane);

    if (!pDoc->IsUserAuthorized())
        return;

    pFrame->UpdateStatusText(_("Resetting project..."));

    wxArrayInt arrSelRows = m_pGridPane->GetSelectedRows2();	
    n = (int)arrSelRows.GetCount();
    for(i=0; i<n; i++) {
        strProjectName = m_pGridPane->GetCellValue(arrSelRows[i], COLUMN_PROJECT);
        strProjectURL = m_pGridPane->GetCellValue(arrSelRows[i], COLUMN_HIDDEN_URL);

        strMessage.Printf(
            _("Are you sure you want to reset project '%s'?"),
            strProjectName.c_str()    
        );

        iAnswer = ::wxMessageBox(
            strMessage,
            _("Reset Project"),
            wxYES_NO | wxICON_QUESTION,
            this
        );

        if (wxYES == iAnswer) {
            pDoc->ProjectReset(HtmlEntityEncode(strProjectURL.Trim(false)));
        }
    }
    
    pFrame->UpdateStatusText(wxT(""));

    m_bForceUpdateSelection = true;
    UpdateSelection();
    pFrame->FireRefreshView();

    wxLogTrace(wxT("Function Start/End"), wxT("CViewProjectsGrid::OnProjectReset - Function End"));
}


void CViewProjectsGrid::OnProjectDetach( wxCommandEvent& WXUNUSED(event) ) {
    wxLogTrace(wxT("Function Start/End"), wxT("CViewProjectsGrid::OnProjectDetach - Function Begin"));

    wxInt32         iAnswer        = 0;
    wxString        strProjectName = wxEmptyString;
    wxString        strProjectURL  = wxEmptyString;
    wxString        strMessage     = wxEmptyString;
    CMainDocument*  pDoc           = wxGetApp().GetDocument();
    CAdvancedFrame* pFrame         = wxDynamicCast(GetParent()->GetParent()->GetParent(), CAdvancedFrame);
    int i, n;

    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));
    wxASSERT(pFrame);
    wxASSERT(wxDynamicCast(pFrame, CAdvancedFrame));
    wxASSERT(m_pGridPane);

    if (!pDoc->IsUserAuthorized())
        return;

    pFrame->UpdateStatusText(_("Detaching from project..."));

    wxArrayInt arrSelRows = m_pGridPane->GetSelectedRows2();	
    n = (int)arrSelRows.GetCount();
    for(i=0; i<n; i++) {
        strProjectName = m_pGridPane->GetCellValue(arrSelRows[i], COLUMN_PROJECT);
        strProjectURL = m_pGridPane->GetCellValue(arrSelRows[i], COLUMN_HIDDEN_URL);

        strMessage.Printf(
            _("Are you sure you want to detach from project '%s'?"),
            strProjectName.c_str()
        );

        iAnswer = ::wxMessageBox(
            strMessage,
            _("Detach from Project"),
            wxYES_NO | wxICON_QUESTION,
            this
        );

        if (wxYES == iAnswer) {
            pDoc->ProjectDetach(HtmlEntityEncode(strProjectURL.Trim(false)));
        }
    }
    
    pFrame->UpdateStatusText(wxT(""));

    m_bForceUpdateSelection = true;
    UpdateSelection();
    pFrame->FireRefreshView();

    wxLogTrace(wxT("Function Start/End"), wxT("CViewProjectsGrid::OnProjectDetach - Function End"));
}


void CViewProjectsGrid::OnProjectWebsiteClicked( wxEvent& event ) {
    wxLogTrace(wxT("Function Start/End"), wxT("CViewProjectsGrid::OnProjectWebsiteClicked - Function Begin"));

    CAdvancedFrame* pFrame      = wxDynamicCast(GetParent()->GetParent()->GetParent(), CAdvancedFrame);

    wxASSERT(pFrame);
    wxASSERT(wxDynamicCast(pFrame, CAdvancedFrame));
    wxASSERT(m_pGridPane);

    pFrame->UpdateStatusText(_("Launching browser..."));

    int website_task_index = event.GetId() - ID_TASK_PROJECT_WEB_PROJDEF_MIN;
    pFrame->ExecuteBrowserLink(
        m_TaskGroups[1]->m_Tasks[website_task_index]->m_strWebSiteLink
    );

    pFrame->UpdateStatusText(wxT(""));

    UpdateSelection();
    pFrame->FireRefreshView();

    wxLogTrace(wxT("Function Start/End"), wxT("CViewProjectsGrid::OnProjectWebsiteClicked - Function End"));
}


wxInt32 CViewProjectsGrid::GetDocCount() {
    return wxGetApp().GetDocument()->GetProjectCount();
}

#if PREVENT_MULTIPLE_PROJECT_SELECTIONS
void CViewProjectsGrid::OnGridSelectRange( wxGridRangeSelectEvent& event ) {
    // Disallow multiple selections
    if (m_pGridPane->GetSelectedRows2().size() > 1) {
        int theRow = event.GetBottomRow();
        m_pGridPane->ClearSelection();
        m_pGridPane->SelectRow(theRow);
    }

    CBOINCBaseView::OnGridSelectRange(event);
}

void CViewProjectsGrid::OnCellLeftClick( wxGridEvent& event ) {
    // Disallow multiple selections
    int theRow = event.GetRow();
    m_pGridPane->ClearSelection();
    m_pGridPane->SelectRow(theRow);
}
#endif

void CViewProjectsGrid::UpdateSelection() {
    wxString        strProjectURL = wxEmptyString;
    CTaskItemGroup* pGroup = NULL;
    PROJECT*        project = NULL;
    CMainDocument*  pDoc = wxGetApp().GetDocument();
    int             i, n;
    bool            wasSuspended=false, wasNoNewWork=false;
    static int      lastCount = 0;

    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));
    wxASSERT(m_pTaskPane);
    wxASSERT(m_pGridPane);

    wxArrayInt arrSelRows = m_pGridPane->GetSelectedRows2();	
    n = (int)arrSelRows.GetCount();

    // Normally, OnGridSelectRange() is called twice: one for deselecting the previous item 
    //  and again for selecting the new item, but occasionally it is not triggered for the 
    //  new selection.  This hack works around that bug.
    if (n != lastCount) {
        m_bForceUpdateSelection = true;
        lastCount = n;
    }
    
    if(!m_bForceUpdateSelection) {
        return;
    }

    CBOINCBaseView::PreUpdateSelection();


    // Update the tasks static box buttons
    //
    pGroup = m_TaskGroups[0];
    
    if (n > 0) {
        m_pTaskPane->EnableTaskGroupTasks(pGroup);
    } else {
        m_pTaskPane->DisableTaskGroupTasks(pGroup);
        UpdateWebsiteSelection(GRP_WEBSITES, NULL);
        if(m_TaskGroups.size()>1) {
            m_pTaskPane->DisableTaskGroupTasks(m_TaskGroups[1]);
        }
    }
   
    for(i=0; i<n; i++) {
        strProjectURL = HtmlEntityEncode(
            m_pGridPane->GetCellValue(arrSelRows[i],COLUMN_HIDDEN_URL).Trim(false)
        );
        project = pDoc->project(strProjectURL);
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

    m_bForceUpdateSelection=false;
}

void CViewProjectsGrid::UpdateWebsiteSelection(long lControlGroup, PROJECT* project){
    unsigned int        i;
    CTaskItemGroup*     pGroup = NULL;
    CTaskItem*          pItem = NULL;

    wxASSERT(m_pTaskPane);
    wxASSERT(m_pGridPane);

    // Update the websites list
    //
    if (m_bForceUpdateSelection) {
        if (m_TaskGroups.size() > 1) {

            // Delete task group, objects, and controls.
            pGroup = m_TaskGroups[lControlGroup];

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
        if (m_pGridPane->GetSelectedRows2().size() == 1) {
            if (project) {
                // Create the web sites task group
                pGroup = new CTaskItemGroup( _("Web sites") );
                m_TaskGroups.push_back( pGroup );

                // Default project url
                pItem = new CTaskItem(
                    wxString(project->project_name.c_str(), wxConvUTF8),
                    wxT(""),
                    wxString(project->master_url.c_str(), wxConvUTF8),
                    ID_TASK_PROJECT_WEB_PROJDEF_MIN
                );
                pGroup->m_Tasks.push_back(pItem);


                // Project defined urls
                for (i=0;(i<project->gui_urls.size())&&(i<=ID_TASK_PROJECT_WEB_PROJDEF_MAX);i++) {
                    pItem = new CTaskItem(
                        wxGetTranslation(wxString(project->gui_urls[i].name.c_str(), wxConvUTF8)),
                        wxGetTranslation(wxString(project->gui_urls[i].description.c_str(), wxConvUTF8)),
                        wxString(project->gui_urls[i].url.c_str(), wxConvUTF8),
                        ID_TASK_PROJECT_WEB_PROJDEF_MIN + 1 + i
                    );
                    pGroup->m_Tasks.push_back(pItem);
                }
            }
        }

        m_bForceUpdateSelection = false;
    }
}

void CViewProjectsGrid::FormatProjectName(wxInt32 item, wxString& strBuffer) {
    PROJECT* project = wxGetApp().GetDocument()->project(item);
    std::string project_name;
    if (project) {
        project->get_name(project_name);
        strBuffer = wxT(" ") + HtmlEntityDecode(wxString(project_name.c_str(), wxConvUTF8));
    }
}

void CViewProjectsGrid::FormatAccountName(wxInt32 item, wxString& strBuffer) {
    PROJECT* project = wxGetApp().GetDocument()->project(item);
    if (project) {
        strBuffer = wxT(" ") + HtmlEntityDecode(wxString(project->user_name.c_str(), wxConvUTF8));
    }
}

void CViewProjectsGrid::FormatTeamName(wxInt32 item, wxString& strBuffer) {
    PROJECT* project = wxGetApp().GetDocument()->project(item);
    if (project) {
        strBuffer = wxT(" ") + HtmlEntityDecode(wxString(project->team_name.c_str(), wxConvUTF8));
    }
}

void CViewProjectsGrid::FormatTotalCredit(wxInt32 item, wxString& strBuffer) {
    PROJECT* project = wxGetApp().GetDocument()->project(item);
    if (project) {
        strBuffer.Printf(wxT(" %0.2f"), project->user_total_credit);
    }
}

void CViewProjectsGrid::FormatAVGCredit(wxInt32 item, wxString& strBuffer) {
    PROJECT* project = wxGetApp().GetDocument()->project(item);
    if (project) {
        strBuffer.Printf(wxT(" %0.2f"), project->user_expavg_credit);
    }
}

void CViewProjectsGrid::FormatResourceShare(wxInt32 item, wxString& strBuffer){
    CMainDocument* pDoc = wxGetApp().GetDocument();
    PROJECT* project = pDoc->project(item);

    if (project && pDoc) {
        strBuffer.Printf(wxT(" %0.0f (%0.2f%%)"), 
            project->resource_share, 
            ((project->resource_share / pDoc->m_fProjectTotalResourceShare) * 100)
        );
    }
}

void CViewProjectsGrid::FormatStatus(wxInt32 item, wxString& strBuffer) {
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
    
    if(wxEmptyString == strBuffer) {
        append_to_status(strBuffer, wxT("---"));
    }
}


void CViewProjectsGrid::FormatProjectURL(wxInt32 item, wxString& strBuffer) {
    PROJECT* project = wxGetApp().GetDocument()->project(item);
    if (project) {
         strBuffer = wxString(project->master_url.c_str(), wxConvUTF8);
    }
}


bool CViewProjectsGrid::IsWebsiteLink(const wxString& strLink) {
    bool bReturnValue = false;

    if (strLink.StartsWith(wxT("web:")))
        bReturnValue = true;

    return bReturnValue;
}


wxInt32 CViewProjectsGrid::ConvertWebsiteIndexToLink(wxInt32 iProjectIndex, wxInt32 iWebsiteIndex, wxString& strLink) {
    strLink.Printf(wxT("web:%d:%d"), iProjectIndex, iWebsiteIndex);
    return 0;
}


wxInt32 CViewProjectsGrid::ConvertLinkToWebsiteIndex(const wxString& strLink, wxInt32& iProjectIndex, wxInt32& iWebsiteIndex) {
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

bool CViewProjectsGrid::OnSaveState(wxConfigBase* pConfig) {
    bool bReturnValue = true;

    wxASSERT(pConfig);
    wxASSERT(m_pTaskPane);
    wxASSERT(m_pGridPane);

    if (!m_pTaskPane->OnSaveState(pConfig)) {
        bReturnValue = false;
    }

    if (!m_pGridPane->OnSaveState(pConfig)) {
        bReturnValue = false;
    }

    return bReturnValue;
}


bool CViewProjectsGrid::OnRestoreState(wxConfigBase* pConfig) {
    wxASSERT(pConfig);
    wxASSERT(m_pTaskPane);
	wxASSERT(m_pGridPane);

    if (!m_pTaskPane->OnRestoreState(pConfig)) {
        return false;
    }

    if (!m_pGridPane->OnRestoreState(pConfig)) {
        return false;
    }

    return true;
}

// set up the grid's content
void CViewProjectsGrid::OnListRender( wxTimerEvent& WXUNUSED(event) ) {
	wxLogTrace(wxT("Function Start/End"), wxT("CViewProjectsGrid::OnListRender - Function Start"));
    wxInt32 docCount = GetDocCount();
    

    // We haven't connected up to the CC yet, there is nothing to display, make sure
    //   everything is deleted.
    if ( docCount <= 0 ) {
        if ( m_pGridPane->GetNumberRows() ) {
            m_pGridPane->DeleteRows(0, m_pGridPane->GetNumberRows());
            m_bForceUpdateSelection = true;
            UpdateSelection();
            UpdateWebsiteSelection(GRP_WEBSITES, NULL);
        }
        return;
    }

    // Right-size the grid so that the number of rows matches
    //   the document state.
    if(docCount != m_pGridPane->GetNumberRows()) {
        if (docCount > m_pGridPane->GetNumberRows()) {
    	    m_pGridPane->AppendRows(docCount - m_pGridPane->GetNumberRows());
         } else {
            m_pGridPane->DeleteRows(0, m_pGridPane->GetNumberRows() - docCount);
            m_bForceUpdateSelection = true;
        }
        wxASSERT(docCount == m_pGridPane->GetNumberRows());
    }

    m_bIgnoreUIEvents = true;
    m_pGridPane->SaveSelection();
    m_bIgnoreUIEvents = false;

    wxString strBuffer;
    int iMax = m_pGridPane->GetNumberRows();
    for(int iRow = 0; iRow < iMax; iRow++) {
		
        FormatProjectName(iRow, strBuffer);
        if (m_pGridPane->GetCellValue(iRow, COLUMN_PROJECT) != strBuffer) {
            m_pGridPane->SetCellValue(iRow, COLUMN_PROJECT, strBuffer);
        }

        FormatAccountName(iRow, strBuffer);
        if (m_pGridPane->GetCellValue(iRow, COLUMN_ACCOUNTNAME) != strBuffer) {
            m_pGridPane->SetCellValue(iRow, COLUMN_ACCOUNTNAME, strBuffer);
        }

        FormatTeamName(iRow, strBuffer);
        if (m_pGridPane->GetCellValue(iRow, COLUMN_TEAMNAME) != strBuffer) {
            m_pGridPane->SetCellValue(iRow, COLUMN_TEAMNAME, strBuffer);
        }

        FormatTotalCredit(iRow, strBuffer);
        if (m_pGridPane->GetCellValue(iRow, COLUMN_TOTALCREDIT) != strBuffer) {
            m_pGridPane->SetCellValue(iRow, COLUMN_TOTALCREDIT, strBuffer);
        }

        FormatAVGCredit(iRow, strBuffer);
        if (m_pGridPane->GetCellValue(iRow, COLUMN_AVGCREDIT) != strBuffer) {
            m_pGridPane->SetCellValue(iRow, COLUMN_AVGCREDIT, strBuffer);
        }

        FormatResourceShare(iRow, strBuffer);
        if (m_pGridPane->GetCellValue(iRow, COLUMN_RESOURCESHARE) != strBuffer) {
            m_pGridPane->SetCellValue(iRow, COLUMN_RESOURCESHARE, strBuffer);
            m_pGridPane->SetCellAlignment(iRow, COLUMN_RESOURCESHARE, wxALIGN_CENTRE, wxALIGN_CENTRE);
    }

        strBuffer = wxEmptyString;
        FormatStatus(iRow, strBuffer);
        if (m_pGridPane->GetCellValue(iRow, COLUMN_STATUS) != strBuffer) {
            m_pGridPane->SetCellValue(iRow, COLUMN_STATUS, strBuffer);
        }

        FormatProjectURL(iRow, strBuffer);
        if (m_pGridPane->GetCellValue(iRow, COLUMN_HIDDEN_URL) != strBuffer) {
            m_pGridPane->SetCellValue(iRow, COLUMN_HIDDEN_URL, strBuffer);
        }
    }

    m_pGridPane->SortData();

    m_bIgnoreUIEvents = true;
    m_pGridPane->RestoreSelection();
    m_bIgnoreUIEvents = false;

    UpdateSelection();

    wxLogTrace(wxT("Function Start/End"), wxT("CViewProjectsGrid::OnListRender - Function End"));
}

