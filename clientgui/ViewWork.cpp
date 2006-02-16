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
#pragma implementation "ViewWork.h"
#endif

#include "stdwx.h"
#include "BOINCGUIApp.h"
#include "MainDocument.h"
#include "BOINCTaskCtrl.h"
#include "BOINCListCtrl.h"
#include "ViewWork.h"
#include "Events.h"
#include "../lib/error_numbers.h"

#include "res/result.xpm"


#define COLUMN_PROJECT              0
#define COLUMN_APPLICATION          1
#define COLUMN_NAME                 2
#define COLUMN_CPUTIME              3
#define COLUMN_PROGRESS             4
#define COLUMN_TOCOMPLETION         5
#define COLUMN_REPORTDEADLINE       6
#define COLUMN_STATUS               7

// groups that contain buttons
#define GRP_TASKS    0
#define GRP_WEBSITES 1

// buttons in the "tasks" area
#define BTN_GRAPHICS                0
#define BTN_SUSPEND                 1
#define BTN_ABORT                   2


CWork::CWork() {
}


CWork::~CWork() {
    m_strProjectName.Clear();
    m_strApplicationName.Clear();
    m_strName.Clear();
    m_strCPUTime.Clear();
    m_strProgress.Clear();
    m_strTimeToCompletion.Clear();
    m_strReportDeadline.Clear();
    m_strStatus.Clear();
}


IMPLEMENT_DYNAMIC_CLASS(CViewWork, CBOINCBaseView)

BEGIN_EVENT_TABLE (CViewWork, CBOINCBaseView)
    EVT_BUTTON(ID_TASK_WORK_SUSPEND, CViewWork::OnWorkSuspend)
    EVT_BUTTON(ID_TASK_WORK_SHOWGRAPHICS, CViewWork::OnWorkShowGraphics)
    EVT_BUTTON(ID_TASK_WORK_ABORT, CViewWork::OnWorkAbort)
    EVT_CUSTOM_RANGE(wxEVT_COMMAND_BUTTON_CLICKED, ID_TASK_PROJECT_WEB_PROJDEF_MIN, ID_TASK_PROJECT_WEB_PROJDEF_MAX, CViewWork::OnProjectWebsiteClicked)
    EVT_LIST_ITEM_SELECTED(ID_LIST_WORKVIEW, CViewWork::OnListSelected)
    EVT_LIST_ITEM_DESELECTED(ID_LIST_WORKVIEW, CViewWork::OnListDeselected)
END_EVENT_TABLE ()


CViewWork::CViewWork()
{}


CViewWork::CViewWork(wxNotebook* pNotebook) :
    CBOINCBaseView(pNotebook, ID_TASK_WORKVIEW, DEFAULT_TASK_FLAGS, ID_LIST_WORKVIEW, DEFAULT_LIST_SINGLE_SEL_FLAGS)
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

    // Create List Pane Items
    m_pListPane->InsertColumn(COLUMN_PROJECT, _("Project"), wxLIST_FORMAT_LEFT, 125);
    m_pListPane->InsertColumn(COLUMN_APPLICATION, _("Application"), wxLIST_FORMAT_LEFT, 95);
    m_pListPane->InsertColumn(COLUMN_NAME, _("Name"), wxLIST_FORMAT_LEFT, 285);
    m_pListPane->InsertColumn(COLUMN_CPUTIME, _("CPU time"), wxLIST_FORMAT_RIGHT, 80);
    m_pListPane->InsertColumn(COLUMN_PROGRESS, _("Progress"), wxLIST_FORMAT_CENTRE, 60);
    m_pListPane->InsertColumn(COLUMN_TOCOMPLETION, _("To completion"), wxLIST_FORMAT_RIGHT, 100);
    m_pListPane->InsertColumn(COLUMN_REPORTDEADLINE, _("Report deadline"), wxLIST_FORMAT_LEFT, 150);
    m_pListPane->InsertColumn(COLUMN_STATUS, _("Status"), wxLIST_FORMAT_LEFT, 135);

    UpdateSelection();
}


CViewWork::~CViewWork() {
    EmptyCache();
    EmptyTasks();
}


wxString& CViewWork::GetViewName() {
    static wxString strViewName(_("Tasks"));
    return strViewName;
}


const char** CViewWork::GetViewIcon() {
    return result_xpm;
}


void CViewWork::OnWorkSuspend( wxCommandEvent& WXUNUSED(event) ) {
    wxLogTrace(wxT("Function Start/End"), wxT("CViewWork::OnWorkSuspend - Function Begin"));

    CMainDocument* pDoc     = wxGetApp().GetDocument();
    CMainFrame* pFrame      = wxGetApp().GetFrame();

    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));
    wxASSERT(pFrame);
    wxASSERT(wxDynamicCast(pFrame, CMainFrame));
    wxASSERT(m_pTaskPane);
    wxASSERT(m_pListPane);

    RESULT* result = pDoc->result(m_pListPane->GetFirstSelected());
    if (result->suspended_via_gui) {
        pFrame->UpdateStatusText(_("Resuming task..."));
        pDoc->WorkResume(m_pListPane->GetFirstSelected());
        pFrame->UpdateStatusText(wxT(""));
    } else {
        pFrame->UpdateStatusText(_("Suspending task..."));
        pDoc->WorkSuspend(m_pListPane->GetFirstSelected());
        pFrame->UpdateStatusText(wxT(""));
    }

    UpdateSelection();
    pFrame->FireRefreshView();

    wxLogTrace(wxT("Function Start/End"), wxT("CViewWork::OnWorkSuspend - Function End"));
}

void CViewWork::OnWorkShowGraphics( wxCommandEvent& WXUNUSED(event) ) {
    wxLogTrace(wxT("Function Start/End"), wxT("CViewWork::OnWorkShowGraphics - Function Begin"));

    wxInt32  iAnswer        = 0; 
    wxString strMachineName = wxEmptyString;
    CMainDocument* pDoc     = wxGetApp().GetDocument();
    CMainFrame* pFrame      = wxGetApp().GetFrame();

    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));
    wxASSERT(pFrame);
    wxASSERT(wxDynamicCast(pFrame, CMainFrame));
    wxASSERT(m_pTaskPane);
    wxASSERT(m_pListPane);

    pFrame->UpdateStatusText(_("Showing graphics for task..."));

    // TODO: implement hide as well as show
#ifdef _WIN32
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
        pDoc->WorkShowGraphics(
            m_pListPane->GetFirstSelected(),
            false,
            wxGetApp().m_strDefaultWindowStation,
            wxGetApp().m_strDefaultDesktop,
            wxGetApp().m_strDefaultDisplay
        );
    }

    pFrame->UpdateStatusText(wxT(""));

    UpdateSelection();
    pFrame->FireRefreshView();

    wxLogTrace(wxT("Function Start/End"), wxT("CViewWork::OnWorkShowGraphics - Function End"));
}


void CViewWork::OnWorkAbort( wxCommandEvent& WXUNUSED(event) ) {
    wxLogTrace(wxT("Function Start/End"), wxT("CViewWork::OnWorkAbort - Function Begin"));

    wxInt32  iAnswer        = 0; 
    wxString strMessage     = wxEmptyString;
    CMainDocument* pDoc     = wxGetApp().GetDocument();
    CMainFrame* pFrame      = wxGetApp().GetFrame();

    wxASSERT(pDoc);
    wxASSERT(pFrame);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));
    wxASSERT(wxDynamicCast(pFrame, CMainFrame));
    wxASSERT(m_pTaskPane);
    wxASSERT(m_pListPane);

    pFrame->UpdateStatusText(_("Aborting result..."));

    strMessage.Printf(
        _("Are you sure you want to abort this task '%s'?"), 
        pDoc->result(m_pListPane->GetFirstSelected())->name.c_str()
    );

    iAnswer = ::wxMessageBox(
        strMessage,
        _("Abort task"),
        wxYES_NO | wxICON_QUESTION,
        this
    );

    if (wxYES == iAnswer) {
        pDoc->WorkAbort(m_pListPane->GetFirstSelected());
    }

    pFrame->UpdateStatusText(wxT(""));

    UpdateSelection();
    pFrame->FireRefreshView();

    wxLogTrace(wxT("Function Start/End"), wxT("CViewWork::OnWorkAbort - Function End"));
}


void CViewWork::OnProjectWebsiteClicked( wxEvent& event ) {
    wxLogTrace(wxT("Function Start/End"), wxT("CViewWork::OnProjectWebsiteClicked - Function Begin"));

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

    wxLogTrace(wxT("Function Start/End"), wxT("CViewWork::OnProjectWebsiteClicked - Function End"));
}


wxInt32 CViewWork::GetDocCount() {
    CMainDocument* pDoc      = wxGetApp().GetDocument();

    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));

    return pDoc->GetWorkCount();
}


wxString CViewWork::OnListGetItemText(long item, long column) const {
    CWork*    work      = NULL;
    wxString  strBuffer = wxEmptyString;

    try {
        work = m_WorkCache.at(item);
    } catch ( std::out_of_range ) {
        work = NULL;
    }

    if (work) {
        switch(column) {
            case COLUMN_PROJECT:
                strBuffer = work->m_strProjectName;
                break;
            case COLUMN_APPLICATION:
                strBuffer = work->m_strApplicationName;
                break;
            case COLUMN_NAME:
                strBuffer = work->m_strName;
                break;
            case COLUMN_CPUTIME:
                strBuffer = work->m_strCPUTime;
                break;
            case COLUMN_PROGRESS:
                strBuffer = work->m_strProgress;
                break;
            case COLUMN_TOCOMPLETION:
                strBuffer = work->m_strTimeToCompletion;
                break;
            case COLUMN_REPORTDEADLINE:
                strBuffer = work->m_strReportDeadline;
                break;
            case COLUMN_STATUS:
                strBuffer = work->m_strStatus;
                break;
        }
    }

    return strBuffer;
}


wxString CViewWork::OnDocGetItemText(long item, long column) const {
    wxString       strBuffer = wxEmptyString;

    switch(column) {
        case COLUMN_PROJECT:
            FormatProjectName(item, strBuffer);
            break;
        case COLUMN_APPLICATION:
            FormatApplicationName(item, strBuffer);
            break;
        case COLUMN_NAME:
            FormatName(item, strBuffer);
            break;
        case COLUMN_CPUTIME:
            FormatCPUTime(item, strBuffer);
            break;
        case COLUMN_PROGRESS:
            FormatProgress(item, strBuffer);
            break;
        case COLUMN_TOCOMPLETION:
            FormatTimeToCompletion(item, strBuffer);
            break;
        case COLUMN_REPORTDEADLINE:
            FormatReportDeadline(item, strBuffer);
            break;
        case COLUMN_STATUS:
            FormatStatus(item, strBuffer);
            break;
    }

    return strBuffer;
}


wxInt32 CViewWork::AddCacheElement() {
    CWork* pItem = new CWork();
    wxASSERT(pItem);
    if (pItem) {
        m_WorkCache.push_back(pItem);
        return 0;
    }
    return -1;
}


wxInt32 CViewWork::EmptyCache() {
    unsigned int i;
    for (i=0; i<m_WorkCache.size(); i++) {
        delete m_WorkCache[i];
    }
    m_WorkCache.clear();
    return 0;
}


wxInt32 CViewWork::GetCacheCount() {
    return m_WorkCache.size();
}


wxInt32 CViewWork::RemoveCacheElement() {
    delete m_WorkCache.back();
    m_WorkCache.erase(m_WorkCache.end() - 1);
    return 0;
}


wxInt32 CViewWork::UpdateCache(long item, long column, wxString& strNewData) {
    CWork* work   = m_WorkCache.at(item);

    switch(column) {
        case COLUMN_PROJECT:
            work->m_strProjectName = strNewData;
            break;
        case COLUMN_APPLICATION:
            work->m_strApplicationName = strNewData;
            break;
        case COLUMN_NAME:
            work->m_strName = strNewData;
            break;
        case COLUMN_CPUTIME:
            work->m_strCPUTime = strNewData;
            break;
        case COLUMN_PROGRESS:
            work->m_strProgress = strNewData;
            break;
        case COLUMN_TOCOMPLETION:
            work->m_strTimeToCompletion = strNewData;
            break;
        case COLUMN_REPORTDEADLINE:
            work->m_strReportDeadline = strNewData;
            break;
        case COLUMN_STATUS:
            work->m_strStatus = strNewData;
            break;
    }

    return 0;
}


void CViewWork::UpdateSelection() {
    CTaskItemGroup*     pGroup = NULL;
    RESULT*             result = NULL;
    PROJECT*            project = NULL;
    CMainDocument*      pDoc = wxGetApp().GetDocument();

    wxASSERT(NULL != pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));
    wxASSERT(NULL != m_pTaskPane);


    CBOINCBaseView::PreUpdateSelection();


    pGroup = m_TaskGroups[0];
    if (m_pListPane->GetSelectedItemCount()) {
        result = pDoc->result(m_pListPane->GetFirstSelected());
        m_pTaskPane->EnableTask(pGroup->m_Tasks[BTN_SUSPEND]);
        if (result) {
            if (result->suspended_via_gui) {
                m_pTaskPane->UpdateTask(
                    pGroup->m_Tasks[BTN_SUSPEND], _("Resume"), _("Resume work for this task.")
                );
            } else {
                m_pTaskPane->UpdateTask(
                    pGroup->m_Tasks[BTN_SUSPEND], _("Suspend"), _("Suspend work for this task.")
                );
            }
            if (result->supports_graphics) {
                m_pTaskPane->EnableTask(pGroup->m_Tasks[BTN_GRAPHICS]);
            } else {
                m_pTaskPane->DisableTask(pGroup->m_Tasks[BTN_GRAPHICS]);
            }
        }
        m_pTaskPane->EnableTask(pGroup->m_Tasks[BTN_ABORT]);

        project = pDoc->state.lookup_project(result->project_url);
        CBOINCBaseView::UpdateWebsiteSelection(GRP_WEBSITES, project);

    } else {
        m_pTaskPane->DisableTaskGroupTasks(pGroup);
    }

    CBOINCBaseView::PostUpdateSelection();
}


wxInt32 CViewWork::FormatProjectName(wxInt32 item, wxString& strBuffer) const {
    CMainDocument* doc = wxGetApp().GetDocument();
    RESULT* result = wxGetApp().GetDocument()->result(item);
    RESULT* state_result = NULL;
    std::string project_name;

    wxASSERT(doc);
    wxASSERT(wxDynamicCast(doc, CMainDocument));

    if (result) {
        state_result = doc->state.lookup_result(result->project_url, result->name);
        if (state_result) {
            state_result->project->get_name(project_name);
            strBuffer = wxString(project_name.c_str());
        } else {
            doc->ForceCacheUpdate();
        }
    }

    return 0;
}


wxInt32 CViewWork::FormatApplicationName(wxInt32 item, wxString& strBuffer) const {
    CMainDocument* doc = wxGetApp().GetDocument();
    RESULT* result = wxGetApp().GetDocument()->result(item);
    RESULT* state_result = NULL;

    wxASSERT(doc);
    wxASSERT(wxDynamicCast(doc, CMainDocument));

    if (result) {
        state_result = doc->state.lookup_result(result->project_url, result->name);
        if (state_result) {
            wxString strLocale = setlocale(LC_NUMERIC, NULL);
            setlocale(LC_NUMERIC, "C");
            strBuffer.Printf(
                wxT("%s %.2f"), 
                state_result->wup->avp->app_name.c_str(),
                state_result->wup->avp->version_num/100.0
            );
            setlocale(LC_NUMERIC, strLocale.c_str());
        } else {
            doc->ForceCacheUpdate();
        }
    }

    return 0;
}


wxInt32 CViewWork::FormatName(wxInt32 item, wxString& strBuffer) const {
    RESULT* result = wxGetApp().GetDocument()->result(item);

    wxASSERT(result);

    if (result) {
        strBuffer = wxString(result->name.c_str());
    }

    return 0;
}


wxInt32 CViewWork::FormatCPUTime(wxInt32 item, wxString& strBuffer) const {
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
            if(result->state < RESULT_COMPUTE_ERROR)
                fBuffer = 0;
            else 
                fBuffer = result->final_cpu_time;
        }
    }

    if (0 == fBuffer) {
        strBuffer = wxT("---");
    } else {
        iHour = (wxInt32)(fBuffer / (60 * 60));
        iMin  = (wxInt32)(fBuffer / 60) % 60;
        iSec  = (wxInt32)(fBuffer) % 60;

        ts = wxTimeSpan(iHour, iMin, iSec);

        strBuffer = ts.Format();
    }

    return 0;
}


wxInt32 CViewWork::FormatProgress(wxInt32 item, wxString& strBuffer) const {
    float          fBuffer = 0;
    RESULT*        result = wxGetApp().GetDocument()->result(item);

    if (result) {
        if (result->active_task) {
            fBuffer = floor(result->fraction_done * 10000)/100;
        } else {
            if(result->state < RESULT_COMPUTE_ERROR) {
                fBuffer = 0.0;
            } else {
                fBuffer = 100.0;
            }
        }
    }

    strBuffer.Printf(wxT("%.2f%%"), fBuffer);

    return 0;
}


wxInt32 CViewWork::FormatTimeToCompletion(wxInt32 item, wxString& strBuffer) const {
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

        strBuffer = ts.Format();
    }

    return 0;
}


wxInt32 CViewWork::FormatReportDeadline(wxInt32 item, wxString& strBuffer) const {
    wxDateTime     dtTemp;
    RESULT*        result = wxGetApp().GetDocument()->result(item);

    if (result) {
        dtTemp.Set((time_t)result->report_deadline);
        strBuffer = dtTemp.Format();
    }

    return 0;
}


wxInt32 CViewWork::FormatStatus(wxInt32 item, wxString& strBuffer) const {
    wxInt32        iActivityMode = -1;
    bool           bActivitiesSuspended = false;
    bool           bNetworkSuspended = false;
    CMainDocument* doc = wxGetApp().GetDocument();
    RESULT*        result = wxGetApp().GetDocument()->result(item);
    PROJECT* project;

    wxASSERT(doc);
    wxASSERT(wxDynamicCast(doc, CMainDocument));

    doc->GetActivityState(bActivitiesSuspended, bNetworkSuspended);
    doc->GetActivityRunMode(iActivityMode);

    if (result) {
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
                project = doc->state.lookup_project(result->project_url);
                if (result->aborted_via_gui) {
                    strBuffer = _("Aborted by user");
                } else if (project && project->suspended_via_gui) {
                    strBuffer = _("Project suspended by user");
                } else if (result->suspended_via_gui) {
                    strBuffer = _("Suspended by user");
                } else if (bActivitiesSuspended) {
                    strBuffer = _("Activities suspended");
                } else if (result->active_task) {
                    if (result->scheduler_state == CPU_SCHED_SCHEDULED) {
                        strBuffer = _("Running");
                    } else if (result->scheduler_state == CPU_SCHED_PREEMPTED) {
                        strBuffer = _("Preempted");
                    } else if (result->scheduler_state == CPU_SCHED_UNINITIALIZED) {
                        strBuffer = _("Ready to run");
                    }
                } else {
                    strBuffer = _("Ready to run");
                }
                break;
            case RESULT_COMPUTE_ERROR:
                // both tests below are needed, because the boolean flag is not changed
                // right away
                //
                if (result->aborted_via_gui || result->exit_status == ERR_ABORTED_VIA_GUI) {
                    strBuffer = _("Aborted by user");
                } else {
                    strBuffer = _("Computation error");
                }
                break;
            case RESULT_FILES_UPLOADING:
                if (result->ready_to_report) {
                    strBuffer = _("Upload failed");
                } else {
                    strBuffer = _("Uploading");
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
    }

    if (!bActivitiesSuspended && iActivityMode == RUN_MODE_NEVER) {
        strBuffer = wxT(" (") + strBuffer + wxT(") ");
        strBuffer = _("Activities suspended by user") + strBuffer;
    }

    return 0;
}


const char *BOINC_RCSID_34f860f736 = "$Id$";
