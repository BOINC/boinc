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


#include "res/result.xpm"


#define COLUMN_PROJECT              0
#define COLUMN_APPLICATION          1
#define COLUMN_NAME                 2
#define COLUMN_CPUTIME              3
#define COLUMN_PROGRESS             4
#define COLUMN_TOCOMPLETION         5
#define COLUMN_REPORTDEADLINE       6
#define COLUMN_STATUS               7

// buttons in the "tasks" area
#define BTN_SUSPEND     0
#define BTN_GRAPHICS    1
#define BTN_ABORT       2


CWork::CWork() {
    m_strProjectName = wxEmptyString;
    m_strApplicationName = wxEmptyString;
    m_strName = wxEmptyString;
    m_strCPUTime = wxEmptyString;
    m_strProgress = wxEmptyString;
    m_strTimeToCompletion = wxEmptyString;
    m_strReportDeadline = wxEmptyString;
    m_strStatus = wxEmptyString;
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
    EVT_LIST_ITEM_SELECTED(ID_LIST_WORKVIEW, CViewWork::OnListSelected)
    EVT_LIST_ITEM_DESELECTED(ID_LIST_WORKVIEW, CViewWork::OnListDeselected)
END_EVENT_TABLE ()


CViewWork::CViewWork() {}


CViewWork::CViewWork(wxNotebook* pNotebook) :
    CBOINCBaseView(pNotebook, ID_TASK_WORKVIEW, DEFAULT_TASK_FLAGS, ID_LIST_WORKVIEW, DEFAULT_LIST_SINGLE_SEL_FLAGS)
{
	CTaskItemGroup* pGroup = NULL;
	CTaskItem*      pItem = NULL;

    wxASSERT(NULL != m_pTaskPane);
    wxASSERT(NULL != m_pListPane);


    //
    // Setup View
    //
	pGroup = new CTaskItemGroup( _("Tasks") );
	m_TaskGroups.push_back( pGroup );

	pItem = new CTaskItem(
        _("Suspend"),
        _("Suspend work for this result."),
        ID_TASK_WORK_SUSPEND 
    );
    pGroup->m_Tasks.push_back( pItem );

	pItem = new CTaskItem(
        _("Show graphics"),
        _("Show application graphics in a window."),
        ID_TASK_WORK_SHOWGRAPHICS 
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


wxString CViewWork::GetViewName() {
    return wxString(_("Work"));
}


const char** CViewWork::GetViewIcon() {
    return result_xpm;
}


void CViewWork::OnWorkSuspend( wxCommandEvent& event ) {
    wxLogTrace(wxT("Function Start/End"), wxT("CViewWork::OnWorkSuspend - Function Begin"));

    CMainDocument* pDoc     = wxGetApp().GetDocument();
    CMainFrame* pFrame      = wxGetApp().GetFrame();

    wxASSERT(NULL != pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));
    wxASSERT(NULL != pFrame);
    wxASSERT(wxDynamicCast(pFrame, CMainFrame));
    wxASSERT(NULL != m_pTaskPane);
    wxASSERT(NULL != m_pListPane);

    RESULT* result = pDoc->result(m_pListPane->GetFirstSelected());
    if (result->suspended_via_gui) {
        pFrame->UpdateStatusText(_("Resuming result..."));
        pDoc->WorkResume(m_pListPane->GetFirstSelected());
        pFrame->UpdateStatusText(wxT(""));
    } else {
        pFrame->UpdateStatusText(_("Suspending result..."));
        pDoc->WorkSuspend(m_pListPane->GetFirstSelected());
        pFrame->UpdateStatusText(wxT(""));
    }

    UpdateSelection();
    pFrame->ProcessRefreshView();

    wxLogTrace(wxT("Function Start/End"), wxT("CViewWork::OnWorkSuspend - Function End"));
}

void CViewWork::OnWorkShowGraphics( wxCommandEvent& event ) {
    wxLogTrace(wxT("Function Start/End"), wxT("CViewWork::OnWorkShowGraphics - Function Begin"));

    wxInt32  iAnswer        = 0; 
    wxString strMachineName = wxEmptyString;
    CMainDocument* pDoc     = wxGetApp().GetDocument();
    CMainFrame* pFrame      = wxGetApp().GetFrame();

    wxASSERT(NULL != pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));
    wxASSERT(NULL != pFrame);
    wxASSERT(wxDynamicCast(pFrame, CMainFrame));
    wxASSERT(NULL != m_pTaskPane);
    wxASSERT(NULL != m_pListPane);

    pFrame->UpdateStatusText(_("Showing graphics for result..."));

    pDoc->GetConnectedComputerName(strMachineName);

    RESULT* result = pDoc->result(m_pListPane->GetFirstSelected());

    // TODO: implement hide as well as show
    if (1) {
#ifdef _WIN32
        if (!strMachineName.empty()) {
            iAnswer = wxMessageBox(
                _("Are you sure you wish to display graphics on a remote machine?"),
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
    }

    pFrame->UpdateStatusText(wxT(""));

    UpdateSelection();
    pFrame->ProcessRefreshView();

    wxLogTrace(wxT("Function Start/End"), wxT("CViewWork::OnWorkShowGraphics - Function End"));
}


void CViewWork::OnWorkAbort( wxCommandEvent& event ) {
    wxLogTrace(wxT("Function Start/End"), wxT("CViewWork::OnWorkAbort - Function Begin"));

    wxInt32  iAnswer        = 0; 
    wxString strResultName  = wxEmptyString;
    wxString strMessage     = wxEmptyString;
    CMainDocument* pDoc     = wxGetApp().GetDocument();
    CMainFrame* pFrame      = wxGetApp().GetFrame();

    wxASSERT(NULL != pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));
    wxASSERT(NULL != pFrame);
    wxASSERT(wxDynamicCast(pFrame, CMainFrame));
    wxASSERT(NULL != m_pTaskPane);
    wxASSERT(NULL != m_pListPane);

    pFrame->UpdateStatusText(_("Aborting result..."));

    pDoc->GetWorkName(m_pListPane->GetFirstSelected(), strResultName);
    strMessage.Printf(
        _("Are you sure you want to abort this result '%s'?"), 
        strResultName.c_str());

    iAnswer = wxMessageBox(
        strMessage,
        _("Abort result"),
        wxYES_NO | wxICON_QUESTION, 
        this
    );

    if (wxYES == iAnswer) {
        pDoc->WorkAbort(m_pListPane->GetFirstSelected());
    }

    pFrame->UpdateStatusText(wxT(""));

    UpdateSelection();
    pFrame->ProcessRefreshView();

    wxLogTrace(wxT("Function Start/End"), wxT("CViewWork::OnWorkAbort - Function End"));
}


wxInt32 CViewWork::GetDocCount() {
    CMainDocument* pDoc      = wxGetApp().GetDocument();

    wxASSERT(NULL != pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));

    return pDoc->GetWorkCount();
}


wxString CViewWork::OnListGetItemText(long item, long column) const {
    CWork*    work      = m_WorkCache.at(item);
    wxString  strBuffer = wxEmptyString;

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
    wxASSERT(NULL != pItem);
    if (NULL != pItem) {
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
    CMainDocument*      pDoc = wxGetApp().GetDocument();

    wxASSERT(NULL != pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));
    wxASSERT(NULL != m_pTaskPane);

    pGroup = m_TaskGroups[0];
    if (m_pListPane->GetSelectedItemCount()) {
        RESULT* result = pDoc->result(m_pListPane->GetFirstSelected());
        m_pTaskPane->EnableTask(pGroup->m_Tasks[BTN_SUSPEND]);
        if (result->suspended_via_gui) {
            m_pTaskPane->UpdateTask(
                pGroup->m_Tasks[BTN_SUSPEND], _("Resume"), _("Resume work for this result.")
            );
        } else {
            m_pTaskPane->UpdateTask(
                pGroup->m_Tasks[BTN_SUSPEND], _("Suspend"), _("Suspend work for this result.")
            );
        }
        if (result->supports_graphics) {
            m_pTaskPane->EnableTask(pGroup->m_Tasks[BTN_GRAPHICS]);
        } else {
            m_pTaskPane->DisableTask(pGroup->m_Tasks[BTN_GRAPHICS]);
        }
        m_pTaskPane->EnableTask(pGroup->m_Tasks[BTN_ABORT]);
    } else {
        m_pTaskPane->DisableTaskGroupTasks(pGroup);
    }
}


wxInt32 CViewWork::FormatProjectName(wxInt32 item, wxString& strBuffer) const {
    CMainDocument* pDoc = wxGetApp().GetDocument();

    wxASSERT(NULL != pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));

    strBuffer.Clear();

    pDoc->GetWorkProjectName(item, strBuffer);

    return 0;
}


wxInt32 CViewWork::FormatApplicationName(wxInt32 item, wxString& strBuffer) const {
    wxInt32        iBuffer = 0;
    wxString       strTempName = wxEmptyString;
    CMainDocument* pDoc = wxGetApp().GetDocument();

    wxASSERT(NULL != pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));

    strBuffer.Clear();

    pDoc->GetWorkApplicationName(item, strTempName);
    pDoc->GetWorkApplicationVersion(item, iBuffer);

    wxString strLocale = setlocale(LC_NUMERIC, NULL);
    setlocale(LC_NUMERIC, "C");
    strBuffer.Printf(wxT("%s %.2f"), strTempName.c_str(), iBuffer/100.0);
    setlocale(LC_NUMERIC, strLocale.c_str());

    return 0;
}


wxInt32 CViewWork::FormatName(wxInt32 item, wxString& strBuffer) const {
    CMainDocument* pDoc = wxGetApp().GetDocument();

    wxASSERT(NULL != pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));

    strBuffer.Clear();

    pDoc->GetWorkName(item, strBuffer);

    return 0;
}


wxInt32 CViewWork::FormatCPUTime(wxInt32 item, wxString& strBuffer) const {
    float          fBuffer = 0;
    wxInt32        iHour = 0;
    wxInt32        iMin = 0;
    wxInt32        iSec = 0;
    wxTimeSpan     ts;
    CMainDocument* pDoc = wxGetApp().GetDocument();

    wxASSERT(NULL != pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));

    strBuffer.Clear();

    if (pDoc->IsWorkActive(item)) {
        pDoc->GetWorkCurrentCPUTime(item, fBuffer);
    } else {
        if(pDoc->GetWorkState(item) < CMainDocument::COMPUTE_ERROR)
            fBuffer = 0;
        else 
            pDoc->GetWorkFinalCPUTime(item, fBuffer);
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
    CMainDocument* pDoc = wxGetApp().GetDocument();

    wxASSERT(NULL != pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));

    strBuffer.Clear();

    if (!pDoc->IsWorkActive(item)) {
        if(pDoc->GetWorkState(item) < CMainDocument::COMPUTE_ERROR)
            strBuffer.Printf(wxT("%.2f%%"), 0.0);
        else 
            strBuffer.Printf(wxT("%.2f%%"), 100.00);
    } else {
        pDoc->GetWorkFractionDone(item, fBuffer);
        strBuffer.Printf(wxT("%.2f%%"), fBuffer * 100);
    }

    return 0;
}


wxInt32 CViewWork::FormatTimeToCompletion(wxInt32 item, wxString& strBuffer) const
{
    float          fBuffer = 0;
    wxInt32        iHour = 0;
    wxInt32        iMin = 0;
    wxInt32        iSec = 0;
    wxTimeSpan     ts;
    CMainDocument* pDoc = wxGetApp().GetDocument();

    wxASSERT(NULL != pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));

    strBuffer.Clear();

    pDoc->GetWorkEstimatedCPUTime(item, fBuffer);

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
    wxInt32        iBuffer = 0;
    wxDateTime     dtTemp;
    CMainDocument* pDoc = wxGetApp().GetDocument();

    wxASSERT(NULL != pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));

    strBuffer.Clear();

    pDoc->GetWorkReportDeadline(item, iBuffer);
    dtTemp.Set((time_t)iBuffer);

    strBuffer = dtTemp.Format();

    return 0;
}


wxInt32 CViewWork::FormatStatus(wxInt32 item, wxString& strBuffer) const {
    CMainDocument* pDoc = wxGetApp().GetDocument();
    wxInt32        iActivityMode = -1;
    bool           bActivitiesSuspended = false;
    bool           bNetworkSuspended = false;


    wxASSERT(NULL != pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));

    strBuffer.Clear();
    pDoc->GetActivityState(bActivitiesSuspended, bNetworkSuspended);
    pDoc->GetActivityRunMode(iActivityMode);

    switch(pDoc->GetWorkState(item)) {
        case CMainDocument::NEW:
            strBuffer = _("New"); 
            break;
        case CMainDocument::FILES_DOWNLOADING:
            if (pDoc->IsWorkReadyToReport(item)) {
                strBuffer = _("Download failed");
            } else {
                strBuffer = _("Downloading");
            }
            break;
        case CMainDocument::FILES_DOWNLOADED:
            if      (pDoc->IsWorkAborted(item)) {
                strBuffer = _("Aborted");
            } else if (!pDoc->IsWorkActive(item) && pDoc->IsWorkSuspended(item)) {
                strBuffer = _("Suspended");
            } else if (pDoc->IsWorkActive(item)) {
                wxInt32 iSchedulerState = pDoc->GetWorkSchedulerState(item);
                if      (CMainDocument::SCHED_SCHEDULED == iSchedulerState) {
                    if (bActivitiesSuspended) { strBuffer = _("Suspended");
                    } else {
                        strBuffer = _("Running");
                    }
                } else if (CMainDocument::SCHED_PREEMPTED == iSchedulerState) {
                    if (pDoc->IsWorkSuspended(item)) {
                        strBuffer = _("Suspended");
                    } else {
                        strBuffer = _("Paused");
                    }
                } else if (CMainDocument::SCHED_UNINITIALIZED == iSchedulerState) {
                    strBuffer = _("Ready to run");
                }
            } else {
                strBuffer = _("Ready to run");
            }
            break;
        case CMainDocument::COMPUTE_ERROR:
            strBuffer = _("Computation error");
            break;
        case CMainDocument::FILES_UPLOADING:
            if (pDoc->IsWorkReadyToReport(item)) {
                strBuffer = _("Upload failed");
            } else {
                strBuffer = _("Uploading");
            }
            break;
        default:
            if      (pDoc->IsWorkAcknowledged(item)) {
                strBuffer = _("Acknowledged");
            } else if (pDoc->IsWorkReadyToReport(item)) {
                strBuffer = _("Ready to report");
            } else {
                strBuffer.Format(_("Error: invalid state '%d'"), pDoc->GetWorkState(item));
            }
            break;
    }

    if (CMainDocument::MODE_NEVER == iActivityMode) {
        strBuffer = wxT(" (") + strBuffer + wxT(") ");
        strBuffer = _("Suspended") + strBuffer;
    }

    return 0;
}


const char *BOINC_RCSID_34f860f736 = "$Id$";
