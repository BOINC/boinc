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
#pragma implementation "ViewProjectsGrid.h"
#endif

#include "stdwx.h"
#include "str_util.h"
#include "BOINCGUIApp.h"
#include "BOINCBaseFrame.h"
#include "MainDocument.h"
#include "AdvancedFrame.h"
#include "BOINCTaskCtrl.h"
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

// groups that contain buttons
#define GRP_TASKS    0
#define GRP_WEBSITES 1

// buttons in the "tasks" area
#define BTN_UPDATE       0
#define BTN_SUSPEND      1
#define BTN_NOWORK       2
#define BTN_RESET        3
#define BTN_DETACH       4

static bool sortAscending;
static int sortColumn;

/* ompare function for projects */
static int compareProjects(CProjectInfo** pfirst,CProjectInfo** psecond) {
	int ret=0;
	CProjectInfo* first = *pfirst;
	CProjectInfo* second = *psecond;
	double diff;
	switch(sortColumn) {
		case COLUMN_ACCOUNTNAME:
			ret = first->accountname.CmpNoCase(second->accountname);
			break;
		case COLUMN_TEAMNAME:
			ret = first->teamname.CmpNoCase(second->teamname);
			break;
		case COLUMN_TOTALCREDIT:
			diff = first->totalcredit - second->totalcredit;
			ret =  diff > 0.0 ? 1 : diff==0 ? 0 : -1;
			break;
		case COLUMN_AVGCREDIT:
			diff = first->avgcredit - second->avgcredit;
			ret =  diff > 0.0 ? 1 : diff==0 ? 0 : -1;
			break;
		case COLUMN_RESOURCESHARE:
			diff = first->rspercent - second->rspercent;
			ret =  diff > 0.0 ? 1 : diff==0 ? 0 : -1;
			break;
		case COLUMN_STATUS:
			ret = first->status.CmpNoCase(second->status);
			break;
		default://sorting by project name as default
			ret = first->name.CmpNoCase(second->name);			
			break;
	}
	ret = sortAscending ? ret : ret * (-1);
	return ret;
}

CProjectInfo::CProjectInfo() {
	name.Clear();
	accountname.Clear();
	teamname.Clear();
	totalcredit=0.0;
	avgcredit=0.0;
	resourceshare=0.0;
	rspercent=0.0;
	status.Clear();
	hashKey.Clear();
}

void CProjectInfo::makeHashKey() {
	hashKey.Printf(wxT("%s%s%s%f%f%f%f%s"),
            name.c_str(), accountname.c_str(), teamname.c_str(), totalcredit,
            avgcredit, resourceshare, rspercent, status.c_str());
}

//###############
IMPLEMENT_DYNAMIC_CLASS(CViewProjectsGrid, CBOINCBaseView)

BEGIN_EVENT_TABLE (CViewProjectsGrid, CBOINCBaseView)
    EVT_BUTTON(ID_TASK_PROJECT_UPDATE, CViewProjectsGrid::OnProjectUpdate)
    EVT_BUTTON(ID_TASK_PROJECT_SUSPEND, CViewProjectsGrid::OnProjectSuspend)
    EVT_BUTTON(ID_TASK_PROJECT_NONEWWORK, CViewProjectsGrid::OnProjectNoNewWork)
    EVT_BUTTON(ID_TASK_PROJECT_RESET, CViewProjectsGrid::OnProjectReset)
    EVT_BUTTON(ID_TASK_PROJECT_DETACH, CViewProjectsGrid::OnProjectDetach)
    EVT_CUSTOM_RANGE(wxEVT_COMMAND_BUTTON_CLICKED, ID_TASK_PROJECT_WEB_PROJDEF_MIN, ID_TASK_PROJECT_WEB_PROJDEF_MAX, CViewProjectsGrid::OnProjectWebsiteClicked)
	EVT_GRID_SELECT_CELL( CViewProjectsGrid::OnSelectCell )
	EVT_GRID_RANGE_SELECT( CViewProjectsGrid::OnSelectRange)
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
	m_pGridPane->SetTable(new CBOINCGridTable(1,7));
	m_pGridPane->SetSelectionMode(wxGrid::wxGridSelectRows);
	// init grid columns
	wxInt32 colSizes[] = {150,80,80,80,80,85,150};
	wxString colTitles[] = {_("Project"),_("Account"),_("Team"),_("Work done"),_("Avg. work done"),_("Resource share"),_("Status")};
	for(int i=0; i<= COLUMN_STATUS;i++){
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
	m_pGridPane->SetPrimaryKeyColumn(COLUMN_PROJECT);
    UpdateSelection();
}


CViewProjectsGrid::~CViewProjectsGrid() {
    EmptyTasks();
}


wxString& CViewProjectsGrid::GetViewName() {
    static wxString strViewName(_("Projects"));
    return strViewName;
}


const char** CViewProjectsGrid::GetViewIcon() {
    return proj_xpm;
}


void CViewProjectsGrid::OnProjectUpdate( wxCommandEvent& WXUNUSED(event) ) {
    wxLogTrace(wxT("Function Start/End"), wxT("CViewProjectsGrid::OnProjectUpdate - Function Begin"));

    CMainDocument* pDoc     = wxGetApp().GetDocument();
    CAdvancedFrame* pFrame      = wxDynamicCast(GetParent()->GetParent()->GetParent(), CAdvancedFrame);

    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));
    wxASSERT(pFrame);
    wxASSERT(wxDynamicCast(pFrame, CAdvancedFrame));
    wxASSERT(m_pTaskPane);
    wxASSERT(m_pGridPane);

    pFrame->UpdateStatusText(_("Updating project..."));
	wxString searchName = m_projectCache.Item(m_pGridPane->GetFirstSelectedRow())->name;
    pDoc->ProjectUpdate(searchName);
    pFrame->UpdateStatusText(wxT(""));

	pDoc->ForceCacheUpdate();
    m_bForceUpdateSelection = true;
    UpdateSelection();
    pFrame->ResetReminderTimers();
    pFrame->FireRefreshView();

    wxLogTrace(wxT("Function Start/End"), wxT("CViewProjectsGrid::OnProjectUpdate - Function End"));
}


void CViewProjectsGrid::OnProjectSuspend( wxCommandEvent& WXUNUSED(event) ) {
    wxLogTrace(wxT("Function Start/End"), wxT("CViewProjectsGrid::OnProjectSuspend - Function Begin"));

    CMainDocument* pDoc     = wxGetApp().GetDocument();
    CAdvancedFrame* pFrame      = wxDynamicCast(GetParent()->GetParent()->GetParent(), CAdvancedFrame);

    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));
    wxASSERT(pFrame);
    wxASSERT(wxDynamicCast(pFrame, CAdvancedFrame));
    wxASSERT(m_pTaskPane);
    wxASSERT(m_pGridPane);

	wxString searchName = m_projectCache.Item(m_pGridPane->GetFirstSelectedRow())->name;
    PROJECT* project = pDoc->project(searchName);
    if (project->suspended_via_gui) {
        pFrame->UpdateStatusText(_("Resuming project..."));
        pDoc->ProjectResume(searchName);
        pFrame->UpdateStatusText(wxT(""));
    } else {
        pFrame->UpdateStatusText(_("Suspending project..."));
        pDoc->ProjectSuspend(searchName);
        pFrame->UpdateStatusText(wxT(""));
    }
	
	pDoc->ForceCacheUpdate();
    m_bForceUpdateSelection = true;
    UpdateSelection();
    pFrame->FireRefreshView();

    wxLogTrace(wxT("Function Start/End"), wxT("CViewProjectsGrid::OnProjectSuspend - Function End"));
}


void CViewProjectsGrid::OnProjectNoNewWork( wxCommandEvent& WXUNUSED(event) ) {
    wxLogTrace(wxT("Function Start/End"), wxT("CViewProjectsGrid::OnProjectNoNewWork - Function Begin"));

    CMainDocument* pDoc     = wxGetApp().GetDocument();
    CAdvancedFrame* pFrame      = wxDynamicCast(GetParent()->GetParent()->GetParent(), CAdvancedFrame);

    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));
    wxASSERT(pFrame);
    wxASSERT(wxDynamicCast(pFrame, CAdvancedFrame));
    wxASSERT(m_pTaskPane);
    wxASSERT(m_pGridPane);

	wxString searchName = m_projectCache.Item(m_pGridPane->GetFirstSelectedRow())->name;
    PROJECT* project = pDoc->project(searchName);
    if (project->dont_request_more_work) {
        pFrame->UpdateStatusText(_("Telling project to allow additional task downloads..."));
        pDoc->ProjectAllowMoreWork(searchName);
        pFrame->UpdateStatusText(wxT(""));
    } else {
        pFrame->UpdateStatusText(_("Telling project to not fetch any additional tasks..."));
        pDoc->ProjectNoMoreWork(searchName);
        pFrame->UpdateStatusText(wxT(""));
    }

	pDoc->ForceCacheUpdate();
    m_bForceUpdateSelection = true;
    UpdateSelection();
    pFrame->FireRefreshView();

    wxLogTrace(wxT("Function Start/End"), wxT("CViewProjectsGrid::OnProjectNoNewWork - Function End"));
}




void CViewProjectsGrid::OnProjectReset( wxCommandEvent& WXUNUSED(event) ) {
    wxLogTrace(wxT("Function Start/End"), wxT("CViewProjectsGrid::OnProjectReset - Function Begin"));

    wxInt32  iAnswer        = 0;
    wxString strMessage     = wxEmptyString;
    CMainDocument* pDoc     = wxGetApp().GetDocument();
    CAdvancedFrame* pFrame      = wxDynamicCast(GetParent()->GetParent()->GetParent(), CAdvancedFrame);

    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));
    wxASSERT(pFrame);
    wxASSERT(wxDynamicCast(pFrame, CAdvancedFrame));
    wxASSERT(m_pTaskPane);
    wxASSERT(m_pGridPane);

    if (!pDoc->IsUserAuthorized())
        return;

    pFrame->UpdateStatusText(_("Resetting project..."));

	wxString searchName = m_projectCache.Item(m_pGridPane->GetFirstSelectedRow())->name;
    strMessage.Printf(
        _("Are you sure you want to reset project '%s'?"),searchName.c_str());

    iAnswer = ::wxMessageBox(
        strMessage,
        _("Reset Project"),
        wxYES_NO | wxICON_QUESTION,
        this
    );

    if (wxYES == iAnswer) {
        pDoc->ProjectReset(searchName);
    }

    pFrame->UpdateStatusText(wxT(""));

	pDoc->ForceCacheUpdate();
    m_bForceUpdateSelection = true;
    UpdateSelection();
    pFrame->FireRefreshView();

    wxLogTrace(wxT("Function Start/End"), wxT("CViewProjectsGrid::OnProjectReset - Function End"));
}


void CViewProjectsGrid::OnProjectDetach( wxCommandEvent& WXUNUSED(event) ) {
    wxLogTrace(wxT("Function Start/End"), wxT("CViewProjectsGrid::OnProjectDetach - Function Begin"));

    wxInt32  iAnswer        = 0;
    std::string strProjectName;
    wxString strMessage     = wxEmptyString;
    CMainDocument* pDoc     = wxGetApp().GetDocument();
    CAdvancedFrame* pFrame      = wxDynamicCast(GetParent()->GetParent()->GetParent(), CAdvancedFrame);

    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));
    wxASSERT(pFrame);
    wxASSERT(wxDynamicCast(pFrame, CAdvancedFrame));
    wxASSERT(m_pTaskPane);
    wxASSERT(m_pGridPane);

    if (!pDoc->IsUserAuthorized())
        return;

    pFrame->UpdateStatusText(_("Detaching from project..."));

	wxString searchName = m_projectCache.Item(m_pGridPane->GetFirstSelectedRow())->name;
    strMessage.Printf(
        _("Are you sure you want to detach from project '%s'?"), searchName.c_str()
    );

    iAnswer = ::wxMessageBox(
        strMessage,
        _("Detach from Project"),
        wxYES_NO | wxICON_QUESTION,
        this
    );

    if (wxYES == iAnswer) {
        pDoc->ProjectDetach(searchName);
    }

    pFrame->UpdateStatusText(wxT(""));

	pDoc->ForceCacheUpdate();
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
    wxASSERT(m_pTaskPane);
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

void CViewProjectsGrid::UpdateSelection() {
    CTaskItemGroup*     pGroup = NULL;
    PROJECT*            project = NULL;
    CMainDocument*      pDoc = wxGetApp().GetDocument();

    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));
    wxASSERT(m_pTaskPane);
    wxASSERT(m_pGridPane);

	if(!m_bForceUpdateSelection) {
		return;
	}

    CBOINCBaseView::PreUpdateSelection();


    // Update the tasks static box buttons
    //
    pGroup = m_TaskGroups[0];

	if (m_pGridPane->GetSelectedRows2().size()==1) {
		wxString searchName = m_projectCache.Item(m_pGridPane->GetFirstSelectedRow())->name;
        project = pDoc->project(searchName);
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
        if (project->attached_via_acct_mgr) {
            m_pTaskPane->DisableTask(pGroup->m_Tasks[BTN_DETACH]);
        } else {
            m_pTaskPane->EnableTask(pGroup->m_Tasks[BTN_DETACH]);
        }
        UpdateWebsiteSelection(GRP_WEBSITES, project);

    } else {
        m_pTaskPane->DisableTaskGroupTasks(pGroup);
		//disable website buttons if they exist
		if(m_TaskGroups.size()>1) {
			m_pTaskPane->DisableTaskGroupTasks(m_TaskGroups[1]);
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
		if (m_pGridPane->GetSelectedRows2().size()==1) {
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
}

void CViewProjectsGrid::FormatProjectName(wxInt32 item, wxString& strBuffer) {
	strBuffer = wxString(" ",wxConvUTF8) + m_projectCache.Item(item)->name;
}


void CViewProjectsGrid::FormatAccountName(wxInt32 item, wxString& strBuffer) {
    strBuffer = wxString(" ",wxConvUTF8) + m_projectCache.Item(item)->accountname;
}

void CViewProjectsGrid::FormatTeamName(wxInt32 item, wxString& strBuffer) {
   strBuffer = wxString(" ",wxConvUTF8) + m_projectCache.Item(item)->teamname;
}

void CViewProjectsGrid::FormatTotalCredit(wxInt32 item, wxString& strBuffer) {
	strBuffer.Printf(wxT(" %0.2f"), m_projectCache.Item(item)->totalcredit);
}


void CViewProjectsGrid::FormatAVGCredit(wxInt32 item, wxString& strBuffer) {
	strBuffer.Printf(wxT(" %0.2f"), m_projectCache.Item(item)->avgcredit);
}


void CViewProjectsGrid::FormatResourceShare(wxInt32 item, wxString& strBuffer){
    strBuffer.Printf(wxT(" %0.2f%% (%0.0f)"),m_projectCache.Item(item)->rspercent,
            m_projectCache.Item(item)->resourceshare);
}


void CViewProjectsGrid::FormatStatus(wxInt32 item, wxString& status) {
	status = wxString(" ",wxConvUTF8) + m_projectCache.Item(item)->status;
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
	//remember grid cursor position (invisible)
	m_pGridPane->SaveGridCursorPosition();
	//remember selected row(s)
	m_pGridPane->SaveSelection();	
	//(re)create rows, if necessary
	if(this->GetDocCount()!= m_pGridPane->GetRows()) {
		//prevent grid from flicker
		m_pGridPane->BeginBatch();
		//at first, delete all current rows
		if(m_pGridPane->GetRows()>0) {
			m_pGridPane->DeleteRows(0,m_pGridPane->GetRows());
		}
		//append new rows
		m_pGridPane->AppendRows(this->GetDocCount());		
		m_pGridPane->EndBatch();
	}
	//update cell values only if project info or sorting were changed
	if(UpdateProjectCache() || SortProjects()) {
		//prevent grid from flicker
		m_pGridPane->BeginBatch();
		wxString buffer;
		int rowmax = m_pGridPane->GetRows();
		for(int rownum=0; rownum < rowmax;rownum++) {
			this->FormatProjectName(rownum,buffer);
			m_pGridPane->SetCellValue(rownum,COLUMN_PROJECT,buffer);

			this->FormatAccountName(rownum,buffer);
			m_pGridPane->SetCellValue(rownum,COLUMN_ACCOUNTNAME,buffer);

			this->FormatTeamName(rownum,buffer);
			m_pGridPane->SetCellValue(rownum,COLUMN_TEAMNAME,buffer);

			this->FormatTotalCredit(rownum,buffer);
			m_pGridPane->SetCellValue(rownum,COLUMN_TOTALCREDIT,buffer);

			this->FormatAVGCredit(rownum,buffer);
			m_pGridPane->SetCellValue(rownum,COLUMN_AVGCREDIT,buffer);

			this->FormatResourceShare(rownum,buffer);
			m_pGridPane->SetCellValue(rownum,COLUMN_RESOURCESHARE,buffer);
			m_pGridPane->SetCellAlignment(rownum,COLUMN_RESOURCESHARE,wxALIGN_CENTRE,wxALIGN_CENTRE);

			buffer = wxEmptyString;
			this->FormatStatus(rownum,buffer);
			m_pGridPane->SetCellValue(rownum,COLUMN_STATUS,buffer);
		}
		// restore grid cursor position, force ignore the internal from wxWidgets thrown selection events
		m_bIgnoreSelectionEvents =true;
		m_pGridPane->RestoreGridCursorPosition();
		m_bIgnoreSelectionEvents =false;
		//restore selection
		m_pGridPane->RestoreSelection();		
		m_pGridPane->EndBatch();
	}	
	//
	UpdateSelection();
	wxLogTrace(wxT("Function Start/End"), wxT("CViewProjectsGrid::OnListRender - Function End"));
}

/**
	handle selection events
*/
void CViewProjectsGrid::OnSelectCell( wxGridEvent& ev )
{
	m_pGridPane->ClearSavedSelection();
    // you must call Skip() if you want the default processing
    // to occur in wxGrid
    ev.Skip();
	if(!m_bIgnoreSelectionEvents) {
		m_bForceUpdateSelection=true;
	}	
}

// handles multi-selection events (to update TaskButtons)
void CViewProjectsGrid::OnSelectRange(wxGridRangeSelectEvent& ev) {
	ev.Skip();
	if(!m_bIgnoreSelectionEvents) {
		m_bForceUpdateSelection=true;
	}	
}

// sorts projects only, if sorting column or sort order were changed
bool CViewProjectsGrid::SortProjects() {
	bool didSorting=false;
	if(sortColumn != m_pGridPane->sortColumn || sortAscending != m_pGridPane->sortAscending) {
		sortColumn = m_pGridPane->sortColumn;
		sortAscending = m_pGridPane->sortAscending;
		m_projectCache.Sort(compareProjects);
		didSorting=true;
	}
	return didSorting;
}

// synchronizes values in view internal chache with document cache
bool CViewProjectsGrid::UpdateProjectCache()
{
	bool didUpdate=false;
	std::string temp;
	CMainDocument* pDoc = wxGetApp().GetDocument();
	wxASSERT(wxDynamicCast(pDoc, CMainDocument));
	//check if projects in cache that not exists any longer
	for(unsigned int i=0; i < m_projectCache.GetCount();i++) {
		PROJECT* p = pDoc->project(m_projectCache.Item(i)->name);
		if(p==NULL) {
			m_projectCache.RemoveAt(i);
			didUpdate=true;
		}
	}
	//
	int max = pDoc->GetProjectCount();	
	for(int i=0; i< max; i++) {
		PROJECT* p = pDoc->project(i);				
		p->get_name(temp);		
		wxString pname(temp.c_str(),wxConvUTF8);
		CProjectInfo* info = FindProjectInCache(pname);		
		if(info!=NULL) {
			if(IsProjectInfoOutOfDate(p,info)) {
				FillProjectInfo(p,info);
				didUpdate=true;
			}
		}
		else {
			info = new CProjectInfo();
			FillProjectInfo(p,info);
			m_projectCache.Add(info);
			didUpdate=true;
		}		
	}
	return didUpdate;
}

//
CProjectInfo* CViewProjectsGrid::FindProjectInCache(wxString& name) {
	for(unsigned int i=0; i< m_projectCache.Count();i++) {
		if(m_projectCache.Item(i)->name.IsSameAs(name)) {
			return m_projectCache.Item(i);
		}
	}
	return NULL;
}

// fills view internal data structure 
void CViewProjectsGrid::FillProjectInfo(PROJECT* p,CProjectInfo* info) {
	std::string temp;
	CMainDocument* pDoc = wxGetApp().GetDocument();

	p->get_name(temp);
	info->name = wxString(temp.c_str(),wxConvUTF8);
	info->accountname = wxString(p->user_name.c_str(),wxConvUTF8);
	info->teamname = wxString(p->team_name.c_str(),wxConvUTF8);
	info->totalcredit = p->user_total_credit;
	info->avgcredit = p->user_expavg_credit;
	info->resourceshare = p->resource_share;
	info->rspercent = (info->resourceshare / pDoc->m_fProjectTotalResourceShare) * 100;
	info->status = this->GetReadableStatus(p);
	info->makeHashKey();
}

// checks, if values in view internal projectcache identical with document cache
bool CViewProjectsGrid::IsProjectInfoOutOfDate(PROJECT* p,CProjectInfo* info) {	
	bool rc=false;
	CProjectInfo* infonew = new CProjectInfo();
	FillProjectInfo(p,infonew);
	rc = !infonew->hashKey.IsSameAs(info->hashKey);
	delete infonew;
	return rc;
}

// returns a human readable value for project status
wxString CViewProjectsGrid::GetReadableStatus(PROJECT* project) {
	wxString status=wxEmptyString;
    if (project) {
		if (project->suspended_via_gui) {
            append_to_status(status, _("Suspended by user"));
        }
        if (project->dont_request_more_work) {
            append_to_status(status, _("Won't get new tasks"));
        }
        if (project->sched_rpc_pending) {
            append_to_status(status, _("Scheduler request pending"));
			append_to_status(status, wxString(rpc_reason_string(project->sched_rpc_pending), wxConvUTF8));
        }
        wxDateTime dtNextRPC((time_t)project->min_rpc_time);
        wxDateTime dtNow(wxDateTime::Now());
        if (dtNextRPC > dtNow) {
            wxTimeSpan tsNextRPC(dtNextRPC - dtNow);
            append_to_status(status, _("Communication deferred ") + tsNextRPC.Format());
        }
    }
	if(status==wxEmptyString)
	{
		append_to_status(status,wxT("---"));
	}
	return status;
}
