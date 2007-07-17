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
#pragma implementation "ViewWorkGrid.h"
#endif

#include "stdwx.h"
#include "BOINCGUIApp.h"
#include "BOINCBaseFrame.h"
#include "MainDocument.h"
#include "AdvancedFrame.h"
#include "BOINCTaskCtrl.h"
#include "ViewWorkGrid.h"
#include "Events.h"
#include "error_numbers.h"

#include "res/result.xpm"


#define COLUMN_PROJECT              0
#define COLUMN_APPLICATION          1
#define COLUMN_NAME                 2
#define COLUMN_CPUTIME              3
#define COLUMN_PROGRESS             4
#define COLUMN_TOCOMPLETION         5
#define COLUMN_REPORTDEADLINE       6
#define COLUMN_STATUS               7
#define COLUMN_RESULTS_INDEX        8
#define NUM_COLUMNS                 (COLUMN_RESULTS_INDEX+1)

// groups that contain buttons
#define GRP_TASKS    0
#define GRP_WEBSITES 1

// buttons in the "tasks" area
#define BTN_GRAPHICS                0
#define BTN_SUSPEND                 1
#define BTN_ABORT                   2



IMPLEMENT_DYNAMIC_CLASS(CViewWorkGrid, CBOINCBaseView)

BEGIN_EVENT_TABLE (CViewWorkGrid, CBOINCBaseView)
    EVT_BUTTON(ID_TASK_WORK_SUSPEND, CViewWorkGrid::OnWorkSuspend)
    EVT_BUTTON(ID_TASK_WORK_SHOWGRAPHICS, CViewWorkGrid::OnWorkShowGraphics)
    EVT_BUTTON(ID_TASK_WORK_ABORT, CViewWorkGrid::OnWorkAbort)
    EVT_CUSTOM_RANGE(wxEVT_COMMAND_BUTTON_CLICKED, ID_TASK_PROJECT_WEB_PROJDEF_MIN, ID_TASK_PROJECT_WEB_PROJDEF_MAX, CViewWorkGrid::OnProjectWebsiteClicked)
	EVT_GRID_SELECT_CELL(CViewWorkGrid::OnGridSelectCell)
	EVT_GRID_RANGE_SELECT(CViewWorkGrid::OnGridSelectRange)
END_EVENT_TABLE ()


CViewWorkGrid::CViewWorkGrid()
{
}


CViewWorkGrid::CViewWorkGrid(wxNotebook* pNotebook) :
    CBOINCBaseView(pNotebook)
{

    //
    // Setup View
    //
    wxFlexGridSizer* itemFlexGridSizer = new wxFlexGridSizer(2, 0, 0);
    wxASSERT(itemFlexGridSizer);

    itemFlexGridSizer->AddGrowableRow(0);
    itemFlexGridSizer->AddGrowableCol(1);
    
	m_pTaskPane = new CBOINCTaskCtrl(this, ID_TASK_WORKGRIDVIEW, DEFAULT_TASK_FLAGS);
    wxASSERT(m_pTaskPane);

    m_pGridPane = new CBOINCGridCtrl(this, ID_LIST_WORKGRIDVIEW);
    wxASSERT(m_pGridPane);

    itemFlexGridSizer->Add(m_pTaskPane, 1, wxGROW|wxALL, 1);
    itemFlexGridSizer->Add(m_pGridPane, 1, wxGROW|wxALL, 1);

    SetSizer(itemFlexGridSizer);

    Layout();

	// Setup TaskPane
	CTaskItemGroup* pGroup = NULL;
	CTaskItem*      pItem = NULL;

	pGroup = new CTaskItemGroup( _("Commands") );
	m_TaskGroups.push_back( pGroup );

	pItem = new CTaskItem(
        _("Show graphics"),
        _("Show application graphics in a window."),
        ID_TASK_WORK_SHOWGRAPHICS 
    );
    pGroup->m_Tasks.push_back( pItem );

	pItem = new CTaskItem(
        _("Suspend"),
        _("Suspend work for this result."),
        ID_TASK_WORK_SUSPEND 
    );
    pGroup->m_Tasks.push_back( pItem );

	pItem = new CTaskItem(
        _("Abort"),
        _("Abandon work on the result. "
          "You will get no credit for it."),
        ID_TASK_WORK_ABORT 
    );
    pGroup->m_Tasks.push_back( pItem );


    // Create Task Pane Items
    m_pTaskPane->UpdateControls();

	// Create Grid (one dummy row is needed)
	m_pGridPane->Setup();
	m_pGridPane->SetTable(new CBOINCGridTable(1,NUM_COLUMNS));
	//don't use wxGrid->Create() here !!!!
	m_pGridPane->SetSelectionMode(wxGrid::wxGridSelectRows);

	// init grid columns
	wxInt32 colSizes[] = {125,95,285,80,60,100,150,135, 0};
	wxString colTitles[] = {_("Project"),_("Application"),_("Name"),_("CPU time"),_("Progress"),_("To completion"),_("Report deadline"),_("Status"),_("Index")};
	for(int i=0; i< NUM_COLUMNS;i++){
		m_pGridPane->SetColLabelValue(i,colTitles[i]);
		m_pGridPane->SetColSize(i,colSizes[i]);		
	}
	// set alignment for cpu time column to right
	m_pGridPane->SetColAlignment(COLUMN_CPUTIME,wxALIGN_RIGHT,wxALIGN_CENTER);
	// set alignment for completion column to right
	m_pGridPane->SetColAlignment(COLUMN_TOCOMPLETION,wxALIGN_RIGHT,wxALIGN_CENTER);
	//change the default cell renderer
	m_pGridPane->SetDefaultRenderer(new CBOINCGridCellProgressRenderer(COLUMN_PROGRESS));
	//set column sort types
	m_pGridPane->SetColumnSortType(COLUMN_PROGRESS,CST_FLOAT);
	m_pGridPane->SetColumnSortType(COLUMN_CPUTIME,CST_TIME);
	m_pGridPane->SetColumnSortType(COLUMN_TOCOMPLETION,CST_TIME);
	m_pGridPane->SetColumnSortType(COLUMN_REPORTDEADLINE,CST_DATETIME);
	m_pGridPane->SetColumnSortType(COLUMN_RESULTS_INDEX,CST_LONG);
	//set primary key column index
	m_pGridPane->SetPrimaryKeyColumn(COLUMN_NAME);
        // Hide the Index column
        int min_width = m_pGridPane->GetColMinimalAcceptableWidth();
	m_pGridPane->SetColMinimalAcceptableWidth(0);
        m_pGridPane->SetColSize(COLUMN_RESULTS_INDEX,0);
	m_pGridPane->SetColMinimalAcceptableWidth(min_width);

    UpdateSelection();
}

CViewWorkGrid::~CViewWorkGrid() {
    EmptyTasks();
}


wxString& CViewWorkGrid::GetViewName() {
    static wxString strViewName(_("TasksGrid"));
    return strViewName;
}


wxString& CViewWorkGrid::GetViewDisplayName() {
    static wxString strViewName(_("Tasks"));
    return strViewName;
}


const char** CViewWorkGrid::GetViewIcon() {
    return result_xpm;
}


void CViewWorkGrid::OnWorkSuspend( wxCommandEvent& WXUNUSED(event) ) {
    wxLogTrace(wxT("Function Start/End"), wxT("CViewWorkGrid::OnWorkSuspend - Function Begin"));

    CMainDocument* pDoc     = wxGetApp().GetDocument();
    CAdvancedFrame* pFrame  = wxDynamicCast(GetParent()->GetParent()->GetParent(), CAdvancedFrame);

    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));
    wxASSERT(pFrame);
    wxASSERT(wxDynamicCast(pFrame, CAdvancedFrame));
    wxASSERT(m_pTaskPane);
    wxASSERT(m_pGridPane);

	wxString searchName = m_pGridPane->GetCellValue(m_pGridPane->GetFirstSelectedRow(),COLUMN_NAME).Trim(false);
    RESULT* result = pDoc->result(searchName);
    if (result->suspended_via_gui) {
        pFrame->UpdateStatusText(_("Resuming task..."));
        pDoc->WorkResume(result->project_url, result->name);
        pFrame->UpdateStatusText(wxT(""));
    } else {
        pFrame->UpdateStatusText(_("Suspending task..."));
        pDoc->WorkSuspend(result->project_url, result->name);
        pFrame->UpdateStatusText(wxT(""));
    }

    UpdateSelection();
    pFrame->FireRefreshView();

    wxLogTrace(wxT("Function Start/End"), wxT("CViewWorkGrid::OnWorkSuspend - Function End"));
}

void CViewWorkGrid::OnWorkShowGraphics( wxCommandEvent& WXUNUSED(event) ) {
    wxLogTrace(wxT("Function Start/End"), wxT("CViewWorkGrid::OnWorkShowGraphics - Function Begin"));

    wxInt32  iAnswer        = 0; 
    wxString strMachineName = wxEmptyString;
    CMainDocument* pDoc     = wxGetApp().GetDocument();
    CAdvancedFrame* pFrame  = wxDynamicCast(GetParent()->GetParent()->GetParent(), CAdvancedFrame);

    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));
    wxASSERT(pFrame);
    wxASSERT(wxDynamicCast(pFrame, CAdvancedFrame));
    wxASSERT(m_pTaskPane);
    wxASSERT(m_pGridPane);

    pFrame->UpdateStatusText(_("Showing graphics for task..."));

    // TODO: implement hide as well as show
#if (defined(_WIN32) || defined(__WXMAC__))
    pDoc->GetConnectedComputerName(strMachineName);
    if (!pDoc->IsComputerNameLocal(strMachineName)) {
        iAnswer = ::wxMessageBox(
            _("Are you sure you want to display graphics on a remote machine?"),
            _("Show graphics"),
            wxYES_NO | wxICON_QUESTION,
            this
        );
    } else {
        iAnswer = wxYES;
    }
#else
    iAnswer = wxYES;
#endif

    if (wxYES == iAnswer) {
		wxString searchName = m_pGridPane->GetCellValue(m_pGridPane->GetFirstSelectedRow(),COLUMN_NAME).Trim(false);
        RESULT* result = pDoc->result(searchName);
		std::string strDefaultWindowStation = std::string((const char*)wxGetApp().m_strDefaultWindowStation.mb_str());
		std::string strDefaultDesktop = std::string((const char*)wxGetApp().m_strDefaultDesktop.mb_str());
		std::string strDefaultDisplay = std::string((const char*)wxGetApp().m_strDefaultDisplay.mb_str());
        pDoc->WorkShowGraphics(
            result->project_url,
            result->name,
            MODE_WINDOW,
            strDefaultWindowStation,
            strDefaultDesktop,
            strDefaultDisplay
        );
    }

    pFrame->UpdateStatusText(wxT(""));

    UpdateSelection();
    pFrame->FireRefreshView();

    wxLogTrace(wxT("Function Start/End"), wxT("CViewWorkGrid::OnWorkShowGraphics - Function End"));
}


void CViewWorkGrid::OnWorkAbort( wxCommandEvent& WXUNUSED(event) ) {
    wxLogTrace(wxT("Function Start/End"), wxT("CViewWorkGrid::OnWorkAbort - Function Begin"));

    wxInt32  iAnswer        = 0;
    //wxInt32  iResult        = 0;
    wxString strMessage     = wxEmptyString;
    wxString strName        = wxEmptyString;
    wxString strProgress    = wxEmptyString;
    wxString strStatus      = wxEmptyString;
    CMainDocument* pDoc     = wxGetApp().GetDocument();
    CAdvancedFrame* pFrame  = wxDynamicCast(GetParent()->GetParent()->GetParent(), CAdvancedFrame);

    wxASSERT(pDoc);
    wxASSERT(pFrame);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));
    wxASSERT(wxDynamicCast(pFrame, CAdvancedFrame));
    wxASSERT(m_pTaskPane);
    wxASSERT(m_pGridPane);

    if (!pDoc->IsUserAuthorized())
        return;

    pFrame->UpdateStatusText(_("Aborting result..."));

	strName = m_pGridPane->GetCellValue(m_pGridPane->GetFirstSelectedRow(),COLUMN_NAME).Trim(false);
	strProgress = m_pGridPane->GetCellValue(m_pGridPane->GetFirstSelectedRow(),COLUMN_PROGRESS).Trim(false);
	strStatus = m_pGridPane->GetCellValue(m_pGridPane->GetFirstSelectedRow(),COLUMN_STATUS).Trim(false);
	
    //FormatName(iResult, strName);
    //FormatProgress(iResult, strProgress);
    //FormatStatus(iResult, strStatus);

    strMessage.Printf(
        _("Are you sure you want to abort this task '%s'?\n"
          "(Progress: %s %%, Status: %s)"), 
        strName.c_str(),
        strProgress.c_str(),
        strStatus.c_str()
    );

    iAnswer = ::wxMessageBox(
        strMessage,
        _("Abort task"),
        wxYES_NO | wxICON_QUESTION,
        this
    );

    if (wxYES == iAnswer) {
        RESULT* result = pDoc->result(strName);
        pDoc->WorkAbort(result->project_url, result->name);
    }

    pFrame->UpdateStatusText(wxT(""));

    UpdateSelection();
    pFrame->FireRefreshView();

    wxLogTrace(wxT("Function Start/End"), wxT("CViewWorkGrid::OnWorkAbort - Function End"));
}


void CViewWorkGrid::OnProjectWebsiteClicked( wxEvent& event ) {
    wxLogTrace(wxT("Function Start/End"), wxT("CViewWorkGrid::OnProjectWebsiteClicked - Function Begin"));

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

    wxLogTrace(wxT("Function Start/End"), wxT("CViewWorkGrid::OnProjectWebsiteClicked - Function End"));
}

void CViewWorkGrid::UpdateSelection() {
    CTaskItemGroup*     pGroup = NULL;
    RESULT*             result = NULL;
    PROJECT*            project = NULL;
    CMainDocument*      pDoc = wxGetApp().GetDocument();

    wxASSERT(NULL != pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));
    wxASSERT(NULL != m_pTaskPane);


    CBOINCBaseView::PreUpdateSelection();

    pGroup = m_TaskGroups[0];
	if (m_pGridPane->GetSelectedRows2().size() == 1) {
		wxString searchName = m_pGridPane->GetCellValue(m_pGridPane->GetFirstSelectedRow(),COLUMN_NAME).Trim(false);
		result = pDoc->result(searchName);
        m_pTaskPane->EnableTask(pGroup->m_Tasks[BTN_SUSPEND]);
        if (result) {
            if (result->suspended_via_gui) {
                m_pTaskPane->UpdateTask(
                    pGroup->m_Tasks[BTN_SUSPEND],
                    _("Resume"),
                    _("Resume work for this task.")
                );
            } else {
                m_pTaskPane->UpdateTask(
                    pGroup->m_Tasks[BTN_SUSPEND],
                    _("Suspend"),
                    _("Suspend work for this task.")
                );
            }
            if (result->supports_graphics || !result->graphics_exec_path.empty()) {
                m_pTaskPane->EnableTask(pGroup->m_Tasks[BTN_GRAPHICS]);
            } else {
                m_pTaskPane->DisableTask(pGroup->m_Tasks[BTN_GRAPHICS]);
            }

            if (
                result->active_task_state != PROCESS_ABORT_PENDING &&
                result->active_task_state != PROCESS_ABORTED &&
                result->state != RESULT_ABORTED 
            ) {
                m_pTaskPane->EnableTask(pGroup->m_Tasks[BTN_ABORT]);
            } else {
                m_pTaskPane->DisableTask(pGroup->m_Tasks[BTN_ABORT]);
            }

            project = pDoc->state.lookup_project(result->project_url);
            UpdateWebsiteSelection(GRP_WEBSITES, project);
        } else {
            UpdateWebsiteSelection(GRP_WEBSITES, NULL);
        }

        m_pTaskPane->EnableTask(pGroup->m_Tasks[BTN_ABORT]);
    } else {
        m_pTaskPane->DisableTaskGroupTasks(pGroup);
		//disable website buttons if they exist
		if(m_TaskGroups.size()>1) {
			m_pTaskPane->DisableTaskGroupTasks(m_TaskGroups[1]);
		}
    }

    CBOINCBaseView::PostUpdateSelection();

}

void CViewWorkGrid::UpdateWebsiteSelection(long lControlGroup, PROJECT* project){
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
        if (m_pGridPane->GetFirstSelectedRow()>=0) {
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


wxInt32 CViewWorkGrid::FormatProjectName(wxInt32 item, wxString& strBuffer) const {
    CMainDocument* doc = wxGetApp().GetDocument();
    RESULT* result = wxGetApp().GetDocument()->result(item);
    PROJECT* state_project = NULL;
    std::string project_name;

    wxASSERT(doc);
    wxASSERT(wxDynamicCast(doc, CMainDocument));

    if (result) {
        state_project = doc->state.lookup_project(result->project_url);
        if (state_project) {
            state_project->get_name(project_name);
            strBuffer = wxT(" ") + HtmlEntityDecode(wxString(project_name.c_str(), wxConvUTF8));
        } else {
            doc->ForceCacheUpdate();
        }
    }

    return 0;
}

wxInt32 CViewWorkGrid::FormatApplicationName(wxInt32 item, wxString& strBuffer) const {
    CMainDocument* pDoc = wxGetApp().GetDocument();
    RESULT* result = wxGetApp().GetDocument()->result(item);
    RESULT* state_result = NULL;
    wxString strLocalBuffer;

    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));

    if (result) {
        state_result = pDoc->state.lookup_result(result->project_url, result->name);
        if (!state_result) {
            pDoc->ForceCacheUpdate();
            state_result = pDoc->state.lookup_result(result->project_url, result->name);
        }
        wxASSERT(state_result);

        wxString strLocale = wxString(setlocale(LC_NUMERIC, NULL), wxConvUTF8);
        setlocale(LC_NUMERIC, "C");
        if (state_result->wup->app->user_friendly_name.size()) {
            strLocalBuffer = HtmlEntityDecode(wxString(state_result->app->user_friendly_name.c_str(), wxConvUTF8));
        } else {
            strLocalBuffer = HtmlEntityDecode(wxString(state_result->wup->avp->app_name.c_str(), wxConvUTF8));
        }
        strBuffer.Printf(
            wxT(" %s %.2f"), 
            strLocalBuffer.c_str(),
            state_result->wup->avp->version_num/100.0
        );
        setlocale(LC_NUMERIC, (const char*)strLocale.mb_str());

    }

    return 0;
}


wxInt32 CViewWorkGrid::FormatName(wxInt32 item, wxString& strBuffer) const {
    RESULT* result = wxGetApp().GetDocument()->result(item);

    wxASSERT(result);

    if (result) {
        strBuffer = _T(" ") + wxString(result->name.c_str(), wxConvUTF8);
    }

    return 0;
}


wxInt32 CViewWorkGrid::FormatCPUTime(wxInt32 item, wxString& strBuffer) const {
    float          fBuffer = 0;
    wxInt32        iHour = 0;
    wxInt32        iMin = 0;
    wxInt32        iSec = 0;
    wxTimeSpan     ts;
    RESULT*        result = wxGetApp().GetDocument()->result(item);

    if (result) {
        if (result->active_task) {
            fBuffer = result->current_cpu_time;
        } else {
            if(result->state < RESULT_COMPUTE_ERROR) {
                fBuffer = 0;
            } else {
                fBuffer = result->final_cpu_time;
            }
        }
    }

    if (0 == fBuffer) {
        strBuffer = wxT("---");
    } else {
        iHour = (wxInt32)(fBuffer / (60 * 60));
        iMin  = (wxInt32)(fBuffer / 60) % 60;
        iSec  = (wxInt32)(fBuffer) % 60;

        ts = wxTimeSpan(iHour, iMin, iSec);

        strBuffer = wxT(" ") + ts.Format();
    }

    return 0;
}


wxInt32 CViewWorkGrid::FormatProgress(wxInt32 item, wxString& strBuffer) const {
    float          fBuffer = 0;
    RESULT*        result = wxGetApp().GetDocument()->result(item);

    if (result) {
        if (result->active_task) {
            fBuffer = floor(result->fraction_done * 100000)/1000;
        } else {
            if(result->state < RESULT_COMPUTE_ERROR) {
                fBuffer = 0.0;
            } else {
                fBuffer = 100.0;
            }
        }
    }

    strBuffer.Printf(wxT("%.3f"), fBuffer);

    return 0;
}


wxInt32 CViewWorkGrid::FormatTimeToCompletion(wxInt32 item, wxString& strBuffer) const {
    float          fBuffer = 0;
    wxInt32        iHour = 0;
    wxInt32        iMin = 0;
    wxInt32        iSec = 0;
    wxTimeSpan     ts;
    RESULT*        result = wxGetApp().GetDocument()->result(item);

    if (result) {
        fBuffer = result->estimated_cpu_time_remaining;
    }

    if (0 >= fBuffer) {
        strBuffer = wxT("---");
    } else {
        iHour = (wxInt32)(fBuffer / (60 * 60));
        iMin  = (wxInt32)(fBuffer / 60) % 60;
        iSec  = (wxInt32)(fBuffer) % 60;

        ts = wxTimeSpan(iHour, iMin, iSec);

        strBuffer = wxT(" ") + ts.Format();
    }

    return 0;
}


wxInt32 CViewWorkGrid::FormatReportDeadline(wxInt32 item, wxString& strBuffer) const {
    wxDateTime     dtTemp;
    RESULT*        result = wxGetApp().GetDocument()->result(item);

    if (result) {
        dtTemp.Set((time_t)result->report_deadline);
		//don't trust default date string representation, this could prevent correct sorting 
        strBuffer = dtTemp.Format(wxT(" %x %X"));
    }

    return 0;
}


wxInt32 CViewWorkGrid::FormatStatus(wxInt32 item, wxString& strBuffer) const {
    CMainDocument* doc = wxGetApp().GetDocument();
    RESULT*        result = wxGetApp().GetDocument()->result(item);
    CC_STATUS      status;

    wxASSERT(doc);
    wxASSERT(wxDynamicCast(doc, CMainDocument));

    doc->GetCoreClientStatus(status);

    if (!result) return 0;
	int throttled = status.task_suspend_reason & SUSPEND_REASON_CPU_USAGE_LIMIT;
    switch(result->state) {
    case RESULT_NEW:
        strBuffer = _("New"); 
        break;
    case RESULT_FILES_DOWNLOADING:
        if (result->ready_to_report) {
            strBuffer = _("Download failed");
        } else {
            strBuffer = _("Downloading");
        }
        break;
    case RESULT_FILES_DOWNLOADED:
        if (result->project_suspended_via_gui) {
            strBuffer = _("Project suspended by user");
        } else if (result->suspended_via_gui) {
            strBuffer = _("Task suspended by user");
        } else if (status.task_suspend_reason && !throttled) {
            strBuffer = _("Suspended");
            if (status.task_suspend_reason & SUSPEND_REASON_BATTERIES) {
                strBuffer += _(" - on batteries");
            }
            if (status.task_suspend_reason & SUSPEND_REASON_USER_ACTIVE) {
                strBuffer += _(" - user active");
            }
            if (status.task_suspend_reason & SUSPEND_REASON_USER_REQ) {
                strBuffer += _(" - computation suspended");
            }
            if (status.task_suspend_reason & SUSPEND_REASON_TIME_OF_DAY) {
                strBuffer += _(" - time of day");
            }
            if (status.task_suspend_reason & SUSPEND_REASON_BENCHMARKS) {
                strBuffer += _(" - CPU benchmarks");
            }
            if (status.task_suspend_reason & SUSPEND_REASON_DISK_SIZE) {
                strBuffer += _(" - need disk space");
            }
        } else if (result->active_task) {
            if (result->too_large) {
                strBuffer = _("Waiting for memory");
            } else if (result->needs_shmem) {
                strBuffer = _("Waiting for shared memory");
            } else if (result->scheduler_state == CPU_SCHED_SCHEDULED) {
                if (result->edf_scheduled) {
                    strBuffer = _("Running, high priority");
                } else {
                    strBuffer = _("Running");
                }
            } else if (result->scheduler_state == CPU_SCHED_PREEMPTED) {
                strBuffer = _("Waiting to run");
            } else if (result->scheduler_state == CPU_SCHED_UNINITIALIZED) {
                strBuffer = _("Ready to start");
            }
        } else {
            strBuffer = _("Ready to start");
        }
        break;
    case RESULT_COMPUTE_ERROR:
        strBuffer = _("Computation error");
        break;
    case RESULT_FILES_UPLOADING:
        if (result->ready_to_report) {
            strBuffer = _("Upload failed");
        } else {
            strBuffer = _("Uploading");
        }
        break;
    case RESULT_ABORTED:
        switch(result->exit_status) {
        case ERR_ABORTED_VIA_GUI:
            strBuffer = _("Aborted by user");
            break;
        case ERR_ABORTED_BY_PROJECT:
            strBuffer = _("Aborted by project");
            break;
        default:
            strBuffer = _("Aborted");
        }
        break;
    default:
        if (result->got_server_ack) {
            strBuffer = _("Acknowledged");
        } else if (result->ready_to_report) {
            strBuffer = _("Ready to report");
        } else {
            strBuffer.Format(_("Error: invalid state '%d'"), result->state);
        }
        break;
    }
	strBuffer = wxT(" ") + strBuffer;
    return 0;
}

bool CViewWorkGrid::OnSaveState(wxConfigBase* pConfig) {
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


bool CViewWorkGrid::OnRestoreState(wxConfigBase* pConfig) {
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

wxInt32 CViewWorkGrid::GetDocCount() {
	return wxGetApp().GetDocument()->GetWorkCount();
}

void CViewWorkGrid::OnListRender( wxTimerEvent& WXUNUSED(event) ) {
    // We haven't connected up to the CC yet, there is nothing to display, make sure
    //   everything is deleted.
    if ( GetDocCount() <= 0 ) {
        if ( m_pGridPane->GetNumberRows() ) {
            m_pGridPane->DeleteRows(0, m_pGridPane->GetNumberRows());
        }
        return;
    }
    
	// flag for row count changes 
	bool rowCountChanged=false;
    // Right-size the grid so that the number of rows matches
    //   the document state.
    if(GetDocCount() != m_pGridPane->GetNumberRows()) {
        if (GetDocCount() > m_pGridPane->GetNumberRows()) {
    	    m_pGridPane->AppendRows(GetDocCount() - m_pGridPane->GetNumberRows());
    	    rowCountChanged=true;
        } else {
		    m_pGridPane->DeleteRows(0, m_pGridPane->GetNumberRows() - GetDocCount());
    	    rowCountChanged=true;
        }
        wxASSERT(GetDocCount() == m_pGridPane->GetNumberRows());
    }

    //init array to detect cell value changes
    wxArrayInt arrColumnDataChanged;
    for(int i=0; i< NUM_COLUMNS;i++) {
        arrColumnDataChanged.Add(0);
    }
    //update cell values	
    wxString strBuffer;
    int iMax = m_pGridPane->GetNumberRows();

    if (rowCountChanged) {
        for(int iRow = 0; iRow < iMax; iRow++) {
            m_pGridPane->SetCellValue(iRow, COLUMN_RESULTS_INDEX, strBuffer.Format("%d", iRow));
        }
    }

    for(int iRow = 0; iRow < iMax; iRow++) {
        long results_index;
        (m_pGridPane->GetCellValue(iRow, COLUMN_RESULTS_INDEX)).ToLong(&results_index);
        FormatProjectName(results_index, strBuffer);
        if (m_pGridPane->GetCellValue(iRow, COLUMN_PROJECT) != strBuffer) {
            m_pGridPane->SetCellValue(iRow, COLUMN_PROJECT, strBuffer);
            arrColumnDataChanged[COLUMN_PROJECT]=1;
        }

        FormatApplicationName(results_index, strBuffer);
        if (m_pGridPane->GetCellValue(iRow, COLUMN_APPLICATION) != strBuffer) {
            m_pGridPane->SetCellValue(iRow, COLUMN_APPLICATION, strBuffer);
            arrColumnDataChanged[COLUMN_APPLICATION]=1;
        }

        FormatName(results_index, strBuffer);
        if (m_pGridPane->GetCellValue(iRow, COLUMN_NAME) != strBuffer) {
            m_pGridPane->SetCellValue(iRow, COLUMN_NAME, strBuffer);
            arrColumnDataChanged[COLUMN_NAME]=1;
        }

        FormatCPUTime(results_index, strBuffer);
        if (m_pGridPane->GetCellValue(iRow, COLUMN_CPUTIME) != strBuffer) {
            m_pGridPane->SetCellValue(iRow, COLUMN_CPUTIME, strBuffer);
            arrColumnDataChanged[COLUMN_CPUTIME]=1;
        }

        FormatProgress(results_index, strBuffer);
        if (m_pGridPane->GetCellValue(iRow, COLUMN_PROGRESS) != strBuffer) {
            m_pGridPane->SetCellValue(iRow, COLUMN_PROGRESS, strBuffer);
            m_pGridPane->SetCellAlignment(iRow, COLUMN_PROGRESS, wxALIGN_CENTRE, wxALIGN_CENTRE);
            arrColumnDataChanged[COLUMN_PROGRESS]=1;
        }

        FormatTimeToCompletion(results_index, strBuffer);
        if (m_pGridPane->GetCellValue(iRow, COLUMN_TOCOMPLETION) != strBuffer) {
            m_pGridPane->SetCellValue(iRow, COLUMN_TOCOMPLETION, strBuffer);
            arrColumnDataChanged[COLUMN_TOCOMPLETION]=1;
        }

        FormatReportDeadline(results_index, strBuffer);
        if (m_pGridPane->GetCellValue(iRow, COLUMN_REPORTDEADLINE) != strBuffer) {
            m_pGridPane->SetCellValue(iRow, COLUMN_REPORTDEADLINE, strBuffer);
            arrColumnDataChanged[COLUMN_REPORTDEADLINE]=1;
        }
		
        strBuffer = wxEmptyString;
        FormatStatus(results_index, strBuffer);
        if (m_pGridPane->GetCellValue(iRow, COLUMN_STATUS) != strBuffer) {
            m_pGridPane->SetCellValue(iRow, COLUMN_STATUS, strBuffer);
            arrColumnDataChanged[COLUMN_STATUS]=1;
        }
    }

    //sort only
    //1. if row count changed
    //2. if sorting column has cell value changes
    //3. if sort is enforced by user through label click
    if( rowCountChanged || 
        (arrColumnDataChanged[m_pGridPane->sortColumn]==1) || 
        m_pGridPane->sortNeededByLabelClick) 
    {
        wxArrayString ordered_indexes;
        for(int iRow = 0; iRow < iMax; iRow++) {
            ordered_indexes.Add(m_pGridPane->GetCellValue(iRow, COLUMN_RESULTS_INDEX));
        }
        
        m_pGridPane->SortData();
    
        for(int iRow = 0; iRow < iMax; iRow++) {
            if (ordered_indexes[iRow] != m_pGridPane->GetCellValue(iRow, COLUMN_RESULTS_INDEX)) {
                // Refresh entire grid if sort order has changed
                m_pGridPane->ForceRefresh();
                break;
            }
        }
    }
    UpdateSelection();
}


