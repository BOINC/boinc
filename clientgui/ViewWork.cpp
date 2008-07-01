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
#pragma implementation "ViewWork.h"
#endif

#include "stdwx.h"
#include "BOINCGUIApp.h"
#include "BOINCBaseFrame.h"
#include "MainDocument.h"
#include "AdvancedFrame.h"
#include "BOINCTaskCtrl.h"
#include "BOINCListCtrl.h"
#include "ViewWork.h"
#include "Events.h"
#include "error_numbers.h"
#include "app_ipc.h"
#include "util.h"

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
    m_fCPUTime = 0.0;
    m_fProgress = 0.0;
    m_fTimeToCompletion = 0.0;
    m_tReportDeadline = (time_t)0;
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
    EVT_LIST_COL_CLICK(ID_LIST_WORKVIEW, CViewWork::OnColClick)
    EVT_LIST_CACHE_HINT(ID_LIST_WORKVIEW, CViewWork::OnCacheHint)
END_EVENT_TABLE ()


static CViewWork* myCViewWork;

static int CompareViewWorkItems(int *iRowIndex1, int *iRowIndex2) {
    CWork*          work1 = myCViewWork->m_WorkCache.at(*iRowIndex1);
    CWork*          work2 = myCViewWork->m_WorkCache.at(*iRowIndex2);
    int             result = 0;
    
    switch (myCViewWork->m_iSortColumn) {
        case COLUMN_PROJECT:
	result = work1->m_strProjectName.CmpNoCase(work2->m_strProjectName);
        break;
    case COLUMN_APPLICATION:
	result = work1->m_strApplicationName.CmpNoCase(work2->m_strApplicationName);
        break;
    case COLUMN_NAME:
	result = work1->m_strName.CmpNoCase(work2->m_strName);
        break;
    case COLUMN_CPUTIME:
        if (work1->m_fCPUTime < work2->m_fCPUTime) {
            result = -1;
        } else if (work1->m_fCPUTime > work2->m_fCPUTime) {
            result = 1;
        }
        break;
    case COLUMN_PROGRESS:
        if (work1->m_fProgress < work2->m_fProgress) {
            result = -1;
        } else if (work1->m_fProgress > work2->m_fProgress) {
            result = 1;
        }
        break;
    case COLUMN_TOCOMPLETION:
        if (work1->m_fTimeToCompletion < work2->m_fTimeToCompletion) {
            result = -1;
        } else if (work1->m_fTimeToCompletion > work2->m_fTimeToCompletion) {
            result = 1;
        }
        break;
    case COLUMN_REPORTDEADLINE:
        if (work1->m_tReportDeadline < work2->m_tReportDeadline) {
            result = -1;
        } else if (work1->m_tReportDeadline > work2->m_tReportDeadline) {
            result = 1;
        }
        break;
    case COLUMN_STATUS:
	result = work1->m_strStatus.CmpNoCase(work2->m_strStatus);
        break;
    }

    return (myCViewWork->m_bReverseSort ? result * (-1) : result);
}


CViewWork::CViewWork()
{}


CViewWork::CViewWork(wxNotebook* pNotebook) :
    CBOINCBaseView(pNotebook, ID_TASK_WORKVIEW, DEFAULT_TASK_FLAGS, ID_LIST_WORKVIEW, DEFAULT_LIST_MULTI_SEL_FLAGS)
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
    m_pListPane->InsertColumn(COLUMN_PROGRESS, _("Progress"), wxLIST_FORMAT_RIGHT, 60);
    m_pListPane->InsertColumn(COLUMN_TOCOMPLETION, _("To completion"), wxLIST_FORMAT_RIGHT, 100);
    m_pListPane->InsertColumn(COLUMN_REPORTDEADLINE, _("Report deadline"), wxLIST_FORMAT_LEFT, 150);
    m_pListPane->InsertColumn(COLUMN_STATUS, _("Status"), wxLIST_FORMAT_LEFT, 135);

    m_iProgressColumn = COLUMN_PROGRESS;

    // Needed by static sort routine;
    myCViewWork = this;
    m_funcSortCompare = CompareViewWorkItems;

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


wxString& CViewWork::GetViewDisplayName() {
    static wxString strViewName(_("Tasks"));
    return strViewName;
}


const char** CViewWork::GetViewIcon() {
    return result_xpm;
}


void CViewWork::OnWorkSuspend( wxCommandEvent& WXUNUSED(event) ) {
    wxLogTrace(wxT("Function Start/End"), wxT("CViewWork::OnWorkSuspend - Function Begin"));

    CMainDocument* pDoc     = wxGetApp().GetDocument();
    CAdvancedFrame* pFrame  = wxDynamicCast(GetParent()->GetParent()->GetParent(), CAdvancedFrame);
    int row;

    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));
    wxASSERT(pFrame);
    wxASSERT(wxDynamicCast(pFrame, CAdvancedFrame));
    wxASSERT(m_pTaskPane);
    wxASSERT(m_pListPane);

    row = -1;
    while (1) {
        // Step through all selected items
        row = m_pListPane->GetNextItem(row, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
        if (row < 0) break;
        
        RESULT* result = pDoc->result(m_iSortedIndexes[row]);
        if (result) {
            if (result->suspended_via_gui) {
                pFrame->UpdateStatusText(_("Resuming task..."));
                pDoc->WorkResume(result->project_url, result->name);
            } else {
                pFrame->UpdateStatusText(_("Suspending task..."));
                pDoc->WorkSuspend(result->project_url, result->name);
            }
        }
    }
    pFrame->UpdateStatusText(wxT(""));
    
    UpdateSelection();
    pFrame->FireRefreshView();

    wxLogTrace(wxT("Function Start/End"), wxT("CViewWork::OnWorkSuspend - Function End"));
}

void CViewWork::OnWorkShowGraphics( wxCommandEvent& WXUNUSED(event) ) {
    wxLogTrace(wxT("Function Start/End"), wxT("CViewWork::OnWorkShowGraphics - Function Begin"));

    wxInt32  iAnswer        = 0; 
    wxString strMachineName = wxEmptyString;
    CMainDocument* pDoc     = wxGetApp().GetDocument();
    CAdvancedFrame* pFrame  = wxDynamicCast(GetParent()->GetParent()->GetParent(), CAdvancedFrame);
    int row;

    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));
    wxASSERT(pFrame);
    wxASSERT(wxDynamicCast(pFrame, CAdvancedFrame));
    wxASSERT(m_pListPane);

    pFrame->UpdateStatusText(_("Showing graphics for task..."));

    // We don't change "Show Graphics" button to "Hide Graphics" because Mac allows user to bring 
    // a graphics window to the foreground when necessary by clicking Show Graphics button again
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
        row = -1;
        while (1) {
            // Step through all selected items
            row = m_pListPane->GetNextItem(row, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
            if (row < 0) break;
            
            RESULT* result = pDoc->result(m_iSortedIndexes[row]);
            if (result) {
                pDoc->WorkShowGraphics(result);
            }
        }
    }
    pFrame->UpdateStatusText(wxT(""));

    UpdateSelection();
    pFrame->FireRefreshView();

    wxLogTrace(wxT("Function Start/End"), wxT("CViewWork::OnWorkShowGraphics - Function End"));
}


void CViewWork::OnWorkAbort( wxCommandEvent& WXUNUSED(event) ) {
    wxLogTrace(wxT("Function Start/End"), wxT("CViewWork::OnWorkAbort - Function Begin"));

    wxInt32  iAnswer        = 0;
    wxInt32  iResult        = 0;
    wxString strMessage     = wxEmptyString;
    wxString strName        = wxEmptyString;
    wxString strProgress    = wxEmptyString;
    wxString strStatus      = wxEmptyString;
    CMainDocument* pDoc     = wxGetApp().GetDocument();
    CAdvancedFrame* pFrame  = wxDynamicCast(GetParent()->GetParent()->GetParent(), CAdvancedFrame);
    int row;

    wxASSERT(pDoc);
    wxASSERT(pFrame);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));
    wxASSERT(wxDynamicCast(pFrame, CAdvancedFrame));
    wxASSERT(m_pTaskPane);
    wxASSERT(m_pListPane);

    if (!pDoc->IsUserAuthorized())
        return;

    pFrame->UpdateStatusText(_("Aborting result..."));

    row = -1;
    while (1) {
        // Step through all selected items
        row = m_pListPane->GetNextItem(row, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
        if (row < 0) break;
        
        iResult = m_iSortedIndexes[row];
        FormatName(iResult, strName);
        FormatProgress(iResult, strProgress);
        FormatStatus(iResult, strStatus);

        strMessage.Printf(
            _("Are you sure you want to abort this task '%s'?\n"
              "(Progress: %s, Status: %s)"), 
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
            RESULT* result = pDoc->result(m_iSortedIndexes[row]);
            if (result) {
                pDoc->WorkAbort(result->project_url, result->name);
            }
        }
    }

    pFrame->UpdateStatusText(wxT(""));

    UpdateSelection();
    pFrame->FireRefreshView();

    wxLogTrace(wxT("Function Start/End"), wxT("CViewWork::OnWorkAbort - Function End"));
}


void CViewWork::OnProjectWebsiteClicked( wxEvent& event ) {
    wxLogTrace(wxT("Function Start/End"), wxT("CViewWork::OnProjectWebsiteClicked - Function Begin"));

    CAdvancedFrame* pFrame      = wxDynamicCast(GetParent()->GetParent()->GetParent(), CAdvancedFrame);

    wxASSERT(pFrame);
    wxASSERT(wxDynamicCast(pFrame, CAdvancedFrame));
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
    return wxGetApp().GetDocument()->GetWorkCount();
}


wxString CViewWork::OnListGetItemText(long item, long column) const {
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
        m_iSortedIndexes.Add((int)m_WorkCache.size()-1);
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
    m_iSortedIndexes.Clear();
    return 0;
}


wxInt32 CViewWork::GetCacheCount() {
    return (wxInt32)m_WorkCache.size();
}


wxInt32 CViewWork::RemoveCacheElement() {
    unsigned int i;
    delete m_WorkCache.back();
    m_WorkCache.erase(m_WorkCache.end() - 1);
    m_iSortedIndexes.Clear();
    for (i=0; i<m_WorkCache.size(); i++) {
        m_iSortedIndexes.Add(i);
    }
    return 0;
}


void CViewWork::UpdateSelection() {
    CTaskItemGroup*     pGroup = NULL;
    RESULT*             result = NULL;
    PROJECT*            project = NULL;
    CC_STATUS           status;
    CMainDocument*      pDoc = wxGetApp().GetDocument();
    int                 i, n, row;
    bool                wasSuspended=false, all_same_project=false;
    std::string         first_project_url;
 
    wxASSERT(NULL != pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));
    wxASSERT(NULL != m_pTaskPane);

    CBOINCBaseView::PreUpdateSelection();

    pGroup = m_TaskGroups[0];
    
    n = m_pListPane->GetSelectedItemCount();
    if (n > 0) {
        m_pTaskPane->EnableTaskGroupTasks(pGroup);
        pDoc->GetCoreClientStatus(status);
        if (status.task_suspend_reason & ~(SUSPEND_REASON_CPU_USAGE_LIMIT)) {
            m_pTaskPane->DisableTask(pGroup->m_Tasks[BTN_GRAPHICS]);
        }
    } else {
        m_pTaskPane->DisableTaskGroupTasks(pGroup);
    }
   
    row = -1;
    for (i=0; i<n; i++) {
        // Step through all selected items
        row = m_pListPane->GetNextItem(row, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
        if (row < 0) break;     // Should never happen
        
        result = pDoc->result(m_iSortedIndexes[row]);
        if (result) {
            if (i == 0) {
                wasSuspended = result->suspended_via_gui;
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
            } else {
                if (wasSuspended != result->suspended_via_gui) {
                    // Disable Suspend / Resume button if the multiple selection
                    // has a mix of suspended and not suspended tasks
                    m_pTaskPane->DisableTask(pGroup->m_Tasks[BTN_SUSPEND]);
                }
            }
            
            // Disable Show Graphics button if any selected task can't display graphics
            if (((!result->supports_graphics) || pDoc->GetState()->executing_as_daemon) 
                        && result->graphics_exec_path.empty()
                ) {
                     m_pTaskPane->DisableTask(pGroup->m_Tasks[BTN_GRAPHICS]);
                }
 
            if (result->suspended_via_gui || result->project_suspended_via_gui) {
                 m_pTaskPane->DisableTask(pGroup->m_Tasks[BTN_GRAPHICS]);
            }
           
            // Disable Abort button if any selected task already aborted
            if (
                result->active_task_state == PROCESS_ABORT_PENDING ||
                result->active_task_state == PROCESS_ABORTED ||
                result->state == RESULT_ABORTED 
            ) {
                m_pTaskPane->DisableTask(pGroup->m_Tasks[BTN_ABORT]);
            }

           if (i == 0) {
                first_project_url = result->project_url;
                all_same_project = true;
            } else {
                if (first_project_url != result->project_url) {
                    all_same_project = false;
                }
            }
        }
    }

    if (all_same_project) {
        project = pDoc->state.lookup_project(result->project_url);
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

    CBOINCBaseView::PostUpdateSelection();
}


bool CViewWork::SynchronizeCacheItem(wxInt32 iRowIndex, wxInt32 iColumnIndex) {
    wxString    strDocumentText  = wxEmptyString;
    float       fDocumentFloat = 0.0;
    time_t      tDocumentTime = (time_t)0;
    CWork*      work = m_WorkCache.at(m_iSortedIndexes[iRowIndex]);

    strDocumentText.Empty();

    switch (iColumnIndex) {
        case COLUMN_PROJECT:
            GetDocProjectName(m_iSortedIndexes[iRowIndex], strDocumentText);
            if (!strDocumentText.IsSameAs(work->m_strProjectName)) {
                work->m_strProjectName = strDocumentText;
                return true;
            }
            break;
        case COLUMN_APPLICATION:
            GetDocApplicationName(m_iSortedIndexes[iRowIndex], strDocumentText);
            if (!strDocumentText.IsSameAs(work->m_strApplicationName)) {
                work->m_strApplicationName = strDocumentText;
                return true;
            }
            break;
        case COLUMN_NAME:
            GetDocName(m_iSortedIndexes[iRowIndex], strDocumentText);
            if (!strDocumentText.IsSameAs(work->m_strName)) {
                work->m_strName = strDocumentText;
                return true;
            }
            break;
        case COLUMN_CPUTIME:
            GetDocCPUTime(m_iSortedIndexes[iRowIndex], fDocumentFloat);
            if (fDocumentFloat != work->m_fCPUTime) {
                work->m_fCPUTime = fDocumentFloat;
                return true;
            }
            break;
        case COLUMN_PROGRESS:
            GetDocProgress(m_iSortedIndexes[iRowIndex], fDocumentFloat);
            if (fDocumentFloat != work->m_fProgress) {
                work->m_fProgress = fDocumentFloat;
                return true;
            }
            break;
        case COLUMN_TOCOMPLETION:
            GetDocTimeToCompletion(m_iSortedIndexes[iRowIndex], fDocumentFloat);
            if (fDocumentFloat != work->m_fTimeToCompletion) {
                work->m_fTimeToCompletion = fDocumentFloat;
                return true;
            }
            break;
        case COLUMN_REPORTDEADLINE:
            GetDocReportDeadline(m_iSortedIndexes[iRowIndex], tDocumentTime);
            if (tDocumentTime != work->m_tReportDeadline) {
                work->m_tReportDeadline = tDocumentTime;
                return true;
            }
            break;
        case COLUMN_STATUS:
            GetDocStatus(m_iSortedIndexes[iRowIndex], strDocumentText);
            if (!strDocumentText.IsSameAs(work->m_strStatus)) {
                work->m_strStatus = strDocumentText;
                return true;
            }
            break;
    }

    return false;
}


void CViewWork::GetDocProjectName(wxInt32 item, wxString& strBuffer) const {
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
            strBuffer = wxString(project_name.c_str(), wxConvUTF8);
         } else {
            doc->ForceCacheUpdate();
        }
    }
}


wxInt32 CViewWork::FormatProjectName(wxInt32 item, wxString& strBuffer) const {
    CWork*          work = m_WorkCache.at(m_iSortedIndexes[item]);
    strBuffer = HtmlEntityDecode(work->m_strProjectName);

    return 0;
}


void CViewWork::GetDocApplicationName(wxInt32 item, wxString& strBuffer) const {
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
        char buf[256];
        if (state_result->wup->avp->plan_class.size()) {
            sprintf(buf, " (%s)", state_result->wup->avp->plan_class.c_str());
        } else {
            strcpy(buf, "");
        }
        strBuffer.Printf(
            wxT(" %s %.2f%s"), 
            strLocalBuffer.c_str(),
            state_result->wup->avp->version_num/100.0,
            buf
        );
        setlocale(LC_NUMERIC, (const char*)strLocale.mb_str());
    }
}


wxInt32 CViewWork::FormatApplicationName(wxInt32 item, wxString& strBuffer) const {
    CWork*          work = m_WorkCache.at(m_iSortedIndexes[item]);
    strBuffer = work->m_strApplicationName;

    return 0;
}


void CViewWork::GetDocName(wxInt32 item, wxString& strBuffer) const {
    RESULT* result = wxGetApp().GetDocument()->result(item);

    wxASSERT(result);

    if (result) {
        strBuffer = wxString(result->name.c_str(), wxConvUTF8);
    }
}


wxInt32 CViewWork::FormatName(wxInt32 item, wxString& strBuffer) const {
    CWork*          work = m_WorkCache.at(m_iSortedIndexes[item]);
    strBuffer = wxString(work->m_strName.c_str(), wxConvUTF8);

    return 0;
}


void CViewWork::GetDocCPUTime(wxInt32 item, float& fBuffer) const {
    RESULT*        result = wxGetApp().GetDocument()->result(item);

    fBuffer = 0;
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
}


wxInt32 CViewWork::FormatCPUTime(wxInt32 item, wxString& strBuffer) const {
    CWork*          work = m_WorkCache.at(m_iSortedIndexes[item]);

    wxInt32        iHour = 0;
    wxInt32        iMin = 0;
    wxInt32        iSec = 0;
    wxTimeSpan     ts;
    float          fBuffer = work->m_fCPUTime;
    
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


void CViewWork::GetDocProgress(wxInt32 item, float& fBuffer) const {
    RESULT*        result = wxGetApp().GetDocument()->result(item);

    fBuffer = 0;
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
}


wxInt32 CViewWork::FormatProgress(wxInt32 item, wxString& strBuffer) const {
    CWork*          work = m_WorkCache.at(m_iSortedIndexes[item]);
    strBuffer.Printf(wxT("%.3f%%"), work->m_fProgress);

    return 0;
}


void CViewWork::GetDocTimeToCompletion(wxInt32 item, float& fBuffer) const {
    RESULT*        result = wxGetApp().GetDocument()->result(item);

    fBuffer = 0;
    if (result) {
        fBuffer = result->estimated_cpu_time_remaining;
    }
}


wxInt32 CViewWork::FormatTimeToCompletion(wxInt32 item, wxString& strBuffer) const {
    CWork*          work = m_WorkCache.at(m_iSortedIndexes[item]); 
    float          fBuffer = work->m_fTimeToCompletion;
    wxInt32        iHour = 0;
    wxInt32        iMin = 0;
    wxInt32        iSec = 0;
    wxTimeSpan     ts;

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


void CViewWork::GetDocReportDeadline(wxInt32 item, time_t& time) const {
    RESULT*        result = wxGetApp().GetDocument()->result(item);

    if (result) {
        time = result->report_deadline;
    } else {
        time = (time_t)0;
    }
}


wxInt32 CViewWork::FormatReportDeadline(wxInt32 item, wxString& strBuffer) const {
    CWork*          work = m_WorkCache.at(m_iSortedIndexes[item]);
    wxDateTime     dtTemp;

    dtTemp.Set(work->m_tReportDeadline);
    strBuffer = dtTemp.Format();

    return 0;
}


void CViewWork::GetDocStatus(wxInt32 item, wxString& strBuffer) const {
    CMainDocument* doc = wxGetApp().GetDocument();
    RESULT*        result = wxGetApp().GetDocument()->result(item);
    CC_STATUS      status;

    wxASSERT(doc);
    wxASSERT(wxDynamicCast(doc, CMainDocument));

    doc->GetCoreClientStatus(status);

    if (!result) {
        strBuffer.Clear();
        return;
    }
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
#if 0
                // doesn't work - result pointer not there
                if (result->project->non_cpu_intensive) {
                    strBuffer += _(" (non-CPU-intensive)");
                }
#endif
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
}


wxInt32 CViewWork::FormatStatus(wxInt32 item, wxString& strBuffer) const {
    CWork*          work = m_WorkCache.at(m_iSortedIndexes[item]);
    strBuffer = work->m_strStatus;

    return 0;
}


double CViewWork::GetProgressValue(long item) {
    float          fBuffer = 0;
    RESULT*        result = wxGetApp().GetDocument()->result(m_iSortedIndexes[item]);

    if (result) {
        if (result->active_task) {
            fBuffer = result->fraction_done;
        } else {
            if(result->state < RESULT_COMPUTE_ERROR) {
                fBuffer = 0.0;
            } else {
                fBuffer = 1.0;
            }
        }
    }

    return fBuffer;
}


const char *BOINC_RCSID_34f860f736 = "$Id$";
