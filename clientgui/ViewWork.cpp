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
#include "DlgItemProperties.h"

#include "res/result.xpm"


// Column IDs must be equal to the column's default
// position (left to right, zero-based) when all
// columns are shown.  However, any column may be
// hidden, either by default or by the user.
// (On MS Windows only, the user can also rearrange
// the columns from the default order.)
//
// Column IDs
#define COLUMN_PROJECT              0
#define COLUMN_PROGRESS             1
#define COLUMN_STATUS               2
#define COLUMN_CPUTIME              3
#define COLUMN_TOCOMPLETION         4
#define COLUMN_REPORTDEADLINE       5
#define COLUMN_APPLICATION          6
#define COLUMN_NAME                 7

// DefaultShownColumns is an array containing the
// columnIDs of the columns to be shown by default,
// in ascending order.  It may or may not include
// all columns.
//
// For now, show all columns by default
static int DefaultShownColumns[] = { COLUMN_PROJECT, COLUMN_PROGRESS, COLUMN_STATUS, 
                                COLUMN_CPUTIME, COLUMN_TOCOMPLETION,
                                COLUMN_REPORTDEADLINE, COLUMN_APPLICATION, 
                                COLUMN_NAME};

// groups that contain buttons
#define GRP_TASKS    0
#define GRP_WEBSITES 1

// buttons in the "tasks" area
#define BTN_ACTIVE_ONLY             0
#define BTN_GRAPHICS                1
#define BTN_VMCONSOLE               2
#define BTN_SUSPEND                 3
#define BTN_ABORT                   4
#define BTN_PROPERTIES              5


CWork::CWork() {
    m_fCPUTime = -1.0;
    m_fProgress = -1.0;
    m_fTimeToCompletion = -1.0;
    m_tReportDeadline = (time_t)0;
}


CWork::~CWork() {
    m_strProjectName.Clear();
    m_strApplicationName.Clear();
    m_strName.Clear();
    m_strStatus.Clear();
    m_strProjectURL.Clear();
    m_strCPUTime.Clear();
    m_strProgress.Clear();
    m_strTimeToCompletion.Clear();
    m_strReportDeadline.Clear();
}


IMPLEMENT_DYNAMIC_CLASS(CViewWork, CBOINCBaseView)

BEGIN_EVENT_TABLE (CViewWork, CBOINCBaseView)
    EVT_BUTTON(ID_TASK_WORK_SUSPEND, CViewWork::OnWorkSuspend)
    EVT_BUTTON(ID_TASK_WORK_SHOWGRAPHICS, CViewWork::OnWorkShowGraphics)
    EVT_BUTTON(ID_TASK_WORK_VMCONSOLE, CViewWork::OnWorkShowVMConsole)
    EVT_BUTTON(ID_TASK_WORK_ABORT, CViewWork::OnWorkAbort)
    EVT_BUTTON(ID_TASK_SHOW_PROPERTIES, CViewWork::OnShowItemProperties)
    EVT_BUTTON(ID_TASK_ACTIVE_ONLY, CViewWork::OnActiveTasksOnly)
    EVT_CUSTOM_RANGE(wxEVT_COMMAND_BUTTON_CLICKED, ID_TASK_PROJECT_WEB_PROJDEF_MIN, ID_TASK_PROJECT_WEB_PROJDEF_MAX, CViewWork::OnProjectWebsiteClicked)
// We currently handle EVT_LIST_CACHE_HINT on Windows or 
// EVT_CHECK_SELECTION_CHANGED on Mac & Linux instead of EVT_LIST_ITEM_SELECTED
// or EVT_LIST_ITEM_DESELECTED.  See CBOINCBaseView::OnCacheHint() for info.
#if USE_LIST_CACHE_HINT
    EVT_LIST_CACHE_HINT(ID_LIST_WORKVIEW, CViewWork::OnCacheHint)
#else
	EVT_CHECK_SELECTION_CHANGED(CViewWork::OnCheckSelectionChanged)
#endif
    EVT_LIST_COL_CLICK(ID_LIST_WORKVIEW, CViewWork::OnColClick)
    EVT_LIST_COL_END_DRAG(ID_LIST_WORKVIEW, CViewWork::OnColResize)
END_EVENT_TABLE ()


static CViewWork* myCViewWork;

static bool CompareViewWorkItems(int iRowIndex1, int iRowIndex2) {
    CWork*          work1;
    CWork*          work2;
    int             result = false;
    
    try {
        work1 = myCViewWork->m_WorkCache.at(iRowIndex1);
    } catch ( std::out_of_range ) {
        return 0;
    }

    try {
        work2 = myCViewWork->m_WorkCache.at(iRowIndex2);
    } catch ( std::out_of_range ) {
        return 0;
    }

    switch (myCViewWork->m_iSortColumnID) {
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

    // Always return FALSE for equality (result == 0)
    return (myCViewWork->m_bReverseSort ? (result > 0) : (result < 0));
}


CViewWork::CViewWork()
{}


CViewWork::CViewWork(wxNotebook* pNotebook) :
    CBOINCBaseView(pNotebook, ID_TASK_WORKVIEW, DEFAULT_TASK_FLAGS, ID_LIST_WORKVIEW, DEFAULT_LIST_FLAGS)
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
        _("Show active tasks"),
        _("Show only active tasks."),
        ID_TASK_ACTIVE_ONLY 
    );
    pGroup->m_Tasks.push_back( pItem );

    pItem = new CTaskItem(
        _("Show graphics"),
        _("Show application graphics in a window."),
        ID_TASK_WORK_SHOWGRAPHICS 
    );
    pGroup->m_Tasks.push_back( pItem );

    pItem = new CTaskItem(
        _("Show VM Console"),
        _("Show VM Console in a window."),
        ID_TASK_WORK_VMCONSOLE 
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
        _("Abandon work on the result. You will get no credit for it."),
        ID_TASK_WORK_ABORT 
    );
    pGroup->m_Tasks.push_back( pItem );

    pItem = new CTaskItem(
        _("Properties"),
        _("Show task details."),
        ID_TASK_SHOW_PROPERTIES 
    );
    pGroup->m_Tasks.push_back( pItem );

    // Create Task Pane Items
    m_pTaskPane->UpdateControls();

    // m_aStdColNameOrder is an array of all column heading labels
    // (localized) in order of ascending Column ID.
    // Once initialized, it should not be modified.
    //
    m_aStdColNameOrder = new wxArrayString;
    m_aStdColNameOrder->Insert(_("Project"), COLUMN_PROJECT);
    m_aStdColNameOrder->Insert(_("Progress"), COLUMN_PROGRESS);
    m_aStdColNameOrder->Insert(_("Status"), COLUMN_STATUS);
    m_aStdColNameOrder->Insert(_("Elapsed"), COLUMN_CPUTIME);
    m_aStdColNameOrder->Insert(_("Remaining (estimated)"), COLUMN_TOCOMPLETION);
    m_aStdColNameOrder->Insert(_("Deadline"), COLUMN_REPORTDEADLINE);
    m_aStdColNameOrder->Insert(_("Application"), COLUMN_APPLICATION);
    m_aStdColNameOrder->Insert(_("Name"), COLUMN_NAME);
    
    // m_iStdColWidthOrder is an array of the width for each column.
    // Entries must be in order of ascending Column ID.  We initalize
    // it here to the default column widths.  It is updated by
    // CBOINCListCtrl::OnRestoreState() and also when a user resizes
    // a column by dragging the divider between two columns.
    //
    m_iStdColWidthOrder.Clear();
    m_iStdColWidthOrder.Insert(125, COLUMN_PROJECT);
    m_iStdColWidthOrder.Insert(60, COLUMN_PROGRESS);
    m_iStdColWidthOrder.Insert(135, COLUMN_STATUS);
    m_iStdColWidthOrder.Insert(80, COLUMN_CPUTIME);
    m_iStdColWidthOrder.Insert(100, COLUMN_TOCOMPLETION);
    m_iStdColWidthOrder.Insert(150, COLUMN_REPORTDEADLINE);
    m_iStdColWidthOrder.Insert(95, COLUMN_APPLICATION);
    m_iStdColWidthOrder.Insert(285, COLUMN_NAME);

    wxASSERT(m_iStdColWidthOrder.size() == m_aStdColNameOrder->size());

    m_iDefaultShownColumns = DefaultShownColumns;
    m_iNumDefaultShownColumns = sizeof(DefaultShownColumns) / sizeof(int);
    m_iProgressColumn = COLUMN_PROGRESS;

    // Needed by static sort routine;
    myCViewWork = this;
    m_funcSortCompare = CompareViewWorkItems;

    UpdateSelection();
}


// Create List Pane Items
void CViewWork::AppendColumn(int columnID){
    switch(columnID) {
        case COLUMN_PROJECT:
            m_pListPane->AppendColumn((*m_aStdColNameOrder)[COLUMN_PROJECT],
                wxLIST_FORMAT_LEFT, m_iStdColWidthOrder[COLUMN_PROJECT]);
            break;
        case COLUMN_PROGRESS:
            m_pListPane->AppendColumn((*m_aStdColNameOrder)[COLUMN_PROGRESS],
                wxLIST_FORMAT_RIGHT, m_iStdColWidthOrder[COLUMN_PROGRESS]);
            break;
        case COLUMN_STATUS:
            m_pListPane->AppendColumn((*m_aStdColNameOrder)[COLUMN_STATUS],
                wxLIST_FORMAT_LEFT, m_iStdColWidthOrder[COLUMN_STATUS]);
            break;
        case COLUMN_CPUTIME:
            m_pListPane->AppendColumn((*m_aStdColNameOrder)[COLUMN_CPUTIME],
                wxLIST_FORMAT_RIGHT, m_iStdColWidthOrder[COLUMN_CPUTIME]);
            break;
        case COLUMN_TOCOMPLETION:
            m_pListPane->AppendColumn((*m_aStdColNameOrder)[COLUMN_TOCOMPLETION],
                wxLIST_FORMAT_RIGHT, m_iStdColWidthOrder[COLUMN_TOCOMPLETION]);
            break;
        case COLUMN_REPORTDEADLINE:
            m_pListPane->AppendColumn((*m_aStdColNameOrder)[COLUMN_REPORTDEADLINE],
                wxLIST_FORMAT_RIGHT, m_iStdColWidthOrder[COLUMN_REPORTDEADLINE]);
            break;
        case COLUMN_APPLICATION:
            m_pListPane->AppendColumn((*m_aStdColNameOrder)[COLUMN_APPLICATION],
                wxLIST_FORMAT_LEFT, m_iStdColWidthOrder[COLUMN_APPLICATION]);
            break;
        case COLUMN_NAME:
            m_pListPane->AppendColumn((*m_aStdColNameOrder)[COLUMN_NAME],
                wxLIST_FORMAT_LEFT, m_iStdColWidthOrder[COLUMN_NAME]);
            break;
    }
}


CViewWork::~CViewWork() {
    EmptyCache();
    EmptyTasks();
}


wxString& CViewWork::GetViewName() {
    static wxString strViewName(wxT("Tasks"));
    return strViewName;
}


wxString& CViewWork::GetViewDisplayName() {
    static wxString strViewName(_("Tasks"));
    return strViewName;
}


const char** CViewWork::GetViewIcon() {
    return result_xpm;
}


int CViewWork::GetViewCurrentViewPage() {
    return VW_TASK;
}


wxString CViewWork::GetKeyValue1(int iRowIndex) {
    CWork*          work;

    if (GetWorkCacheAtIndex(work, m_iSortedIndexes[iRowIndex])) {
        return wxEmptyString;
    }

    if (m_iColumnIDToColumnIndex[COLUMN_NAME] < 0) {
        // Column is hidden, so SynchronizeCacheItem() did not set its value
        GetDocName(m_iSortedIndexes[iRowIndex], work->m_strName);
    }

    return work->m_strName;
}


wxString CViewWork::GetKeyValue2(int iRowIndex) {
     CWork*          work;

    if (GetWorkCacheAtIndex(work, m_iSortedIndexes[iRowIndex])) {
        return wxEmptyString;
    }

    if (m_iColumnIDToColumnIndex[COLUMN_PROJECT] < 0) {
        // Column is hidden, so SynchronizeCacheItem() did not set its value
        GetDocProjectURL(m_iSortedIndexes[iRowIndex], work->m_strProjectURL);
    }
    
    return work->m_strProjectURL;
}


int CViewWork::FindRowIndexByKeyValues(wxString& key1, wxString& key2) {
    CWork* work;
    unsigned int iRowIndex, n = GetCacheCount();
    for(iRowIndex=0; iRowIndex < n; iRowIndex++) {
        if (GetWorkCacheAtIndex(work, m_iSortedIndexes[iRowIndex])) {
            continue;
        }
        if(! (work->m_strName).IsSameAs(key1)) continue;
        if((work->m_strProjectURL).IsSameAs(key2)) return iRowIndex;
    }
    return -1;
}


void CViewWork::OnActiveTasksOnly( wxCommandEvent& WXUNUSED(event) ) {
    wxLogTrace(wxT("Function Start/End"), wxT("CViewWork::OnActiveTasksOnly - Function Begin"));

    CMainDocument* pDoc     = wxGetApp().GetDocument();
    CAdvancedFrame* pFrame      = wxDynamicCast(GetParent()->GetParent()->GetParent(), CAdvancedFrame);

    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));
    wxASSERT(pFrame);
    wxASSERT(wxDynamicCast(pFrame, CAdvancedFrame));

    pDoc->m_ActiveTasksOnly = !pDoc->m_ActiveTasksOnly;
    UpdateSelection();
    pFrame->FireRefreshView();

    wxLogTrace(wxT("Function Start/End"), wxT("CViewWork::OnActiveTasksOnly - Function End"));
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
                pDoc->WorkResume(result->project_url, result->name);
            } else {
                pDoc->WorkSuspend(result->project_url, result->name);
            }
        }
    }

    UpdateSelection();
    pFrame->FireRefreshView();

    wxLogTrace(wxT("Function Start/End"), wxT("CViewWork::OnWorkSuspend - Function End"));
}


void CViewWork::OnWorkShowGraphics( wxCommandEvent& WXUNUSED(event) ) {
    wxLogTrace(wxT("Function Start/End"), wxT("CViewWork::OnWorkShowGraphics - Function Begin"));

    CMainDocument* pDoc     = wxGetApp().GetDocument();
    CAdvancedFrame* pFrame  = wxDynamicCast(GetParent()->GetParent()->GetParent(), CAdvancedFrame);
    RESULT* result;
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
        
        result = pDoc->result(m_iSortedIndexes[row]);
        if (result) {
            pDoc->WorkShowGraphics(result);
        }
    }

    UpdateSelection();
    pFrame->FireRefreshView();

    wxLogTrace(wxT("Function Start/End"), wxT("CViewWork::OnWorkShowGraphics - Function End"));
}


void CViewWork::OnWorkShowVMConsole( wxCommandEvent& WXUNUSED(event) ) {
    wxLogTrace(wxT("Function Start/End"), wxT("CViewWork::OnWorkShowVMConsole - Function Begin"));

    CMainDocument* pDoc     = wxGetApp().GetDocument();
    CAdvancedFrame* pFrame  = wxDynamicCast(GetParent()->GetParent()->GetParent(), CAdvancedFrame);
    RESULT* result;
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
        
        result = pDoc->result(m_iSortedIndexes[row]);
        if (result) {
            pDoc->WorkShowVMConsole(result);
        }
    }

    UpdateSelection();
    pFrame->FireRefreshView();

    wxLogTrace(wxT("Function Start/End"), wxT("CViewWork::OnWorkShowVMConsole - Function End"));
}


void CViewWork::OnWorkAbort( wxCommandEvent& WXUNUSED(event) ) {
    wxLogTrace(wxT("Function Start/End"), wxT("CViewWork::OnWorkAbort - Function Begin"));

    wxInt32  iAnswer        = 0;
    wxString strMessage     = wxEmptyString;
    CMainDocument* pDoc     = wxGetApp().GetDocument();
    CAdvancedFrame* pFrame  = wxDynamicCast(GetParent()->GetParent()->GetParent(), CAdvancedFrame);
    CWork* work;
    int row, n;

    wxASSERT(pDoc);
    wxASSERT(pFrame);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));
    wxASSERT(wxDynamicCast(pFrame, CAdvancedFrame));
    wxASSERT(m_pTaskPane);
    wxASSERT(m_pListPane);

    if (!pDoc->IsUserAuthorized())
        return;

    n = m_pListPane->GetSelectedItemCount();
    
    if (n == 1) {
        row = -1;
        row = m_pListPane->GetNextItem(row, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
        if (row < 0) return;
        if (GetWorkCacheAtIndex(work, m_iSortedIndexes[row])) {
            return;
        }
        strMessage.Printf(
           _("Are you sure you want to abort this task '%s'?\n(Progress: %s, Status: %s)"), 
           (work->m_strName).c_str(),
           (work->m_strProgress).c_str(),
           (work->m_strStatus).c_str()
        );
    } else {
        strMessage.Printf(_("Are you sure you want to abort these %d tasks?"), n);
    }

    iAnswer = wxGetApp().SafeMessageBox(
        strMessage,
        _("Abort task"),
        wxYES_NO | wxICON_QUESTION,
        this
    );

    if (wxYES != iAnswer) {
        return;
    }

    row = -1;
    while (1) {
        // Step through all selected items
        row = m_pListPane->GetNextItem(row, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
        if (row < 0) break;
        
        RESULT* result = pDoc->result(m_iSortedIndexes[row]);
        if (result) {
            pDoc->WorkAbort(result->project_url, result->name);
        }
    }

    UpdateSelection();
    pFrame->FireRefreshView();

    wxLogTrace(wxT("Function Start/End"), wxT("CViewWork::OnWorkAbort - Function End"));
}


void CViewWork::OnShowItemProperties( wxCommandEvent& WXUNUSED(event) ) {
    wxASSERT(m_pListPane);

    long item = m_pListPane->GetFirstSelected();
    RESULT* result = wxGetApp().GetDocument()->result(m_iSortedIndexes[item]);

    if(!result) return;     // TODO: display some sort of error alert?
    //displaying the infos on a dialog
    CDlgItemProperties dlg(this);
    dlg.renderInfos(result);
    dlg.ShowModal();
}


bool CViewWork::OnSaveState(wxConfigBase* pConfig) {
    bool bReturnValue = true;
    CMainDocument* pDoc     = wxGetApp().GetDocument();

    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));
    wxASSERT(pConfig);
    wxASSERT(m_pTaskPane);
    wxASSERT(m_pListPane);

    if (!m_pTaskPane->OnSaveState(pConfig)) {
        bReturnValue = false;
    }
    if (!m_pListPane->OnSaveState(pConfig)) {
        bReturnValue = false;
    }

    wxString    strBaseConfigLocation = wxEmptyString;
    strBaseConfigLocation = wxT("/Tasks");
    pConfig->SetPath(strBaseConfigLocation);
    pConfig->Write(wxT("ActiveTasksOnly"), (pDoc->m_ActiveTasksOnly ? 1 : 0));

    return bReturnValue;
}


bool CViewWork::OnRestoreState(wxConfigBase* pConfig) {
    CMainDocument* pDoc     = wxGetApp().GetDocument();

    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));
    wxASSERT(pConfig);
    wxASSERT(m_pTaskPane);
    wxASSERT(m_pListPane);

    if (!m_pTaskPane->OnRestoreState(pConfig)) {
        return false;
    }
    if (!m_pListPane->OnRestoreState(pConfig)) {
        return false;
    }

    int     iTempValue = 0;
    wxString    strBaseConfigLocation = wxEmptyString;
    strBaseConfigLocation = wxT("/Tasks");
    pConfig->SetPath(strBaseConfigLocation);
    pConfig->Read(wxT("ActiveTasksOnly"), &iTempValue, 0);
    pDoc->m_ActiveTasksOnly = (iTempValue != 0);

    return true;
}


void CViewWork::OnProjectWebsiteClicked( wxEvent& event ) {
    wxLogTrace(wxT("Function Start/End"), wxT("CViewWork::OnProjectWebsiteClicked - Function Begin"));

    CAdvancedFrame* pFrame      = wxDynamicCast(GetParent()->GetParent()->GetParent(), CAdvancedFrame);

    wxASSERT(pFrame);
    wxASSERT(wxDynamicCast(pFrame, CAdvancedFrame));
    wxASSERT(m_pTaskPane);
    wxASSERT(m_pListPane);

    int website_task_index = event.GetId() - ID_TASK_PROJECT_WEB_PROJDEF_MIN;
    wxLaunchDefaultBrowser(
        m_TaskGroups[1]->m_Tasks[website_task_index]->m_strWebSiteLink
    );

    UpdateSelection();
    pFrame->FireRefreshView();

    wxLogTrace(wxT("Function Start/End"), wxT("CViewWork::OnProjectWebsiteClicked - Function End"));
}


void CViewWork::OnColResize( wxListEvent& ) {
    // Register the new column widths immediately
    CAdvancedFrame* pFrame = wxDynamicCast(GetParent()->GetParent()->GetParent(), CAdvancedFrame);

    wxASSERT(pFrame);
    wxASSERT(wxDynamicCast(pFrame, CAdvancedFrame));
    pFrame->SaveState();
}


wxInt32 CViewWork::GetDocCount() {
    return wxGetApp().GetDocument()->GetWorkCount();
}


wxString CViewWork::OnListGetItemText(long item, long column) const {
    CWork*    work      = NULL;
    wxString  strBuffer = wxEmptyString;
    
    m_pListPane->AddPendingProgressBar(item);

    try {
        work = m_WorkCache.at(m_iSortedIndexes[item]);
    } catch ( std::out_of_range ) {
        work = NULL;
    }

    if (work && (column >= 0)) {
        switch(m_iColumnIndexToColumnID[column]) {
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
                // CBOINCListCtrl::DrawProgressBars() will draw this using 
                // data provided by GetProgressText() and GetProgressValue(), 
                // but we need it here for accessibility programs.
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


bool CViewWork::IsSelectionManagementNeeded() {
    return true;
}


void CViewWork::UpdateSelection() {
    int                 i, n, row;
    CTaskItemGroup*     pGroup = NULL;
    RESULT*             result = NULL;
    PROJECT*            project = NULL;
    CC_STATUS           status;
    CMainDocument*      pDoc = wxGetApp().GetDocument();
    std::string         first_project_url;
    wxString            strMachineName;
    bool                wasSuspended=false;
    bool                all_same_project=false;
    bool                enableShowGraphics = false;
    bool                enableShowVMConsole = false;
    bool                enableSuspendResume = false;
    bool                enableAbort = false;
    bool                enableProperties = false;

    wxASSERT(NULL != pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));
    wxASSERT(NULL != m_pTaskPane);

    CBOINCBaseView::PreUpdateSelection();

    pGroup = m_TaskGroups[0];
    
    n = m_pListPane->GetSelectedItemCount();
    if (n > 0) {
        enableShowGraphics = true;
        enableShowVMConsole = true;
        enableSuspendResume = true;
        enableAbort = true;
        
        pDoc->GetCoreClientStatus(status);
        if (status.task_suspend_reason & ~(SUSPEND_REASON_CPU_THROTTLE)) {
            enableShowGraphics = false;
            enableShowVMConsole = false;
        }

        pDoc->GetConnectedComputerName(strMachineName);
        if (!pDoc->IsComputerNameLocal(strMachineName)) {
            enableShowGraphics = false;
            enableShowVMConsole = false;
        }
    }

    if (pDoc->m_ActiveTasksOnly) {
        m_pTaskPane->UpdateTask(
            pGroup->m_Tasks[BTN_ACTIVE_ONLY],
            _("Show all tasks"),
            _("Show all tasks.")
        );
    } else {
        m_pTaskPane->UpdateTask(
            pGroup->m_Tasks[BTN_ACTIVE_ONLY],
            _("Show active tasks"),
            _("Show only active tasks.")
        );
    }

    row = -1;
    for (i=0; i<n; i++) {
        // Step through all selected items
        row = m_pListPane->GetNextItem(row, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
        if (row < 0) break;     // Should never happen
        
        result = pDoc->result(m_iSortedIndexes[row]);
        if (!result) continue;
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
                enableSuspendResume = false;
            }
        }
        
        // Disable Show VM console if the selected task hasn't registered a remote
        // desktop connection
        //
        if (!strlen(result->remote_desktop_addr)) {
            enableShowVMConsole = false;
        }

        // Disable Show Graphics button if the selected task can't display graphics
        //
        if (!strlen(result->web_graphics_url) && !strlen(result->graphics_exec_path)) {
            enableShowGraphics = false;
        }

        if (result->suspended_via_gui ||
            result->project_suspended_via_gui || 
            (result->scheduler_state != CPU_SCHED_SCHEDULED)
        ) {
            enableShowGraphics = false;
        }
       
        // Disable Abort button if any selected task already aborted
        if (
            result->active_task_state == PROCESS_ABORT_PENDING ||
            result->active_task_state == PROCESS_ABORTED ||
            result->state == RESULT_ABORTED 
        ) {
            enableAbort = false;
        }

       if (i == 0) {
            first_project_url = result->project_url;
            all_same_project = true;
        } else {
            if (first_project_url != result->project_url) {
                all_same_project = false;
            }
        }
        
        if (n == 1) {
            enableProperties = true;
        }
    }

    // To minimize flicker, set each button only once to the final desired state
    pGroup->m_Tasks[BTN_GRAPHICS]->m_pButton->Enable(enableShowGraphics);
    if (enableShowVMConsole) {
        pGroup->m_Tasks[BTN_VMCONSOLE]->m_pButton->Enable();
        if (pGroup->m_Tasks[BTN_VMCONSOLE]->m_pButton->Show()) {
            m_pTaskPane->FitInside();
        }
    } else {
        pGroup->m_Tasks[BTN_VMCONSOLE]->m_pButton->Disable();
        if (pGroup->m_Tasks[BTN_VMCONSOLE]->m_pButton->Hide()) {
            m_pTaskPane->FitInside();
        };
    }
    pGroup->m_Tasks[BTN_SUSPEND]->m_pButton->Enable(enableSuspendResume);
    pGroup->m_Tasks[BTN_ABORT]->m_pButton->Enable(enableAbort);
    pGroup->m_Tasks[BTN_PROPERTIES]->m_pButton->Enable(enableProperties);

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
    wxString    strDocumentText2 = wxEmptyString;
    double       x = 0.0;
    time_t      tDocumentTime = (time_t)0;
    CWork*      work;

    strDocumentText.Empty();

     if (GetWorkCacheAtIndex(work, m_iSortedIndexes[iRowIndex])) {
        return false;
    }
    
    if (iColumnIndex < 0) return false;
    
    switch (m_iColumnIndexToColumnID[iColumnIndex]) {
        case COLUMN_PROJECT:
            GetDocProjectName(m_iSortedIndexes[iRowIndex], strDocumentText);
            GetDocProjectURL(m_iSortedIndexes[iRowIndex], strDocumentText2);
            if (!strDocumentText.IsSameAs(work->m_strProjectName) || !strDocumentText2.IsSameAs(work->m_strProjectURL)) {
                work->m_strProjectName = strDocumentText;
                work->m_strProjectURL = strDocumentText2;
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
            GetDocCPUTime(m_iSortedIndexes[iRowIndex], x);
            if (x != work->m_fCPUTime) {
                work->m_fCPUTime = x;
                work->m_strCPUTime = FormatTime(x);
                return true;
            }
            break;
        case COLUMN_PROGRESS:
            GetDocProgress(m_iSortedIndexes[iRowIndex], x);
            if (x != work->m_fProgress) {
                work->m_fProgress = x;
                FormatProgress(x, work->m_strProgress);
                return true;
            }
            break;
        case COLUMN_TOCOMPLETION:
            GetDocTimeToCompletion(m_iSortedIndexes[iRowIndex], x);
            if (x != work->m_fTimeToCompletion) {
                work->m_fTimeToCompletion = x;
                work->m_strTimeToCompletion = FormatTime(x);
                return true;
            }
            break;
        case COLUMN_REPORTDEADLINE:
            GetDocReportDeadline(m_iSortedIndexes[iRowIndex], tDocumentTime);
            if (tDocumentTime != work->m_tReportDeadline) {
                work->m_tReportDeadline = tDocumentTime;
                FormatReportDeadline(tDocumentTime, work->m_strReportDeadline);
                return true;
            }
            break;
        case COLUMN_STATUS:
            int i = m_iSortedIndexes[iRowIndex];
            RESULT* result = wxGetApp().GetDocument()->result(i);
            strDocumentText = result_description(result);
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
        // TODO: should we get the name directly with result->project->get_name(project_name) ?
        state_project = doc->state.lookup_project(result->project_url);
        if (state_project) {
            state_project->get_name(project_name);
            strBuffer = HtmlEntityDecode(wxString(project_name.c_str(), wxConvUTF8));
         } else {
            doc->ForceCacheUpdate();
        }
    }
}


void CViewWork::GetDocApplicationName(wxInt32 item, wxString& strBuffer) const {
    CMainDocument* pDoc = wxGetApp().GetDocument();
    RESULT*        result = wxGetApp().GetDocument()->result(item);
    RESULT*        state_result = NULL;
    wxString       strAppBuffer = wxEmptyString;
    wxString       strClassBuffer = wxEmptyString;

    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));

    if (result) {
        state_result = pDoc->state.lookup_result(result->project_url, result->name);
        if (!state_result) {
            pDoc->ForceCacheUpdate();
            state_result = pDoc->state.lookup_result(result->project_url, result->name);
        }

        if (!state_result) return;
        WORKUNIT* wup = state_result->wup;
        if (!wup) return;
        APP* app = wup->app;
        if (!app) return;
        APP_VERSION* avp = state_result->avp;
        if (!avp) return;

        if (strlen(app->user_friendly_name)) {
            strAppBuffer = HtmlEntityDecode(wxString(state_result->app->user_friendly_name, wxConvUTF8));
        } else {
            strAppBuffer = HtmlEntityDecode(wxString(state_result->avp->app_name, wxConvUTF8));
        }
        
        if (strlen(avp->plan_class)) {
            strClassBuffer.Printf(
                wxT(" (%s)"),
                wxString(avp->plan_class, wxConvUTF8).c_str()
            );
        }

        strBuffer.Printf(
            wxT(" %s%s %d.%02d %s"),
            state_result->project->anonymous_platform?_("Local: "):wxT(""),
            strAppBuffer.c_str(),
            state_result->avp->version_num / 100,
            state_result->avp->version_num % 100,
            strClassBuffer.c_str()
        );
    }
}


void CViewWork::GetDocName(wxInt32 item, wxString& strBuffer) const {
    RESULT* result = wxGetApp().GetDocument()->result(item);

    if (result) {
        strBuffer = wxString(result->name, wxConvUTF8);
    }
}


void CViewWork::GetDocCPUTime(wxInt32 item, double& fBuffer) const {
    RESULT*        result = wxGetApp().GetDocument()->result(item);

    fBuffer = 0;
    if (result) {
        if (result->active_task) {
            fBuffer = result->elapsed_time;
            if (!fBuffer) fBuffer = result->current_cpu_time;
        } else {
            if(result->state < RESULT_COMPUTE_ERROR) {
                fBuffer = 0;
            } else {
                fBuffer = result->final_elapsed_time;
                if (!fBuffer) fBuffer = result->final_cpu_time;
            }
        }
    }
}

void CViewWork::GetDocProgress(wxInt32 item, double& fBuffer) const {
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


wxInt32 CViewWork::FormatProgress(double fBuffer, wxString& strBuffer) const {
    strBuffer.Printf(wxT("%.3f%%"), fBuffer);

    return 0;
}


void CViewWork::GetDocTimeToCompletion(wxInt32 item, double& fBuffer) const {
    RESULT*        result = wxGetApp().GetDocument()->result(item);

    fBuffer = 0;
    if (result) {
        fBuffer = result->estimated_cpu_time_remaining;
    }
}

void CViewWork::GetDocReportDeadline(wxInt32 item, time_t& time) const {
    RESULT*        result = wxGetApp().GetDocument()->result(item);

    if (result) {
        time = (time_t)result->report_deadline;
    } else {
        time = (time_t)0;
    }
}


wxInt32 CViewWork::FormatReportDeadline(time_t deadline, wxString& strBuffer) const {
#ifdef __WXMAC__
    // Work around a wxCocoa bug(?) in wxDateTime::Format()
    char buf[80];
    struct tm * timeinfo = localtime(&deadline);
    strftime(buf, sizeof(buf), "%c", timeinfo);
    strBuffer = buf;
#else
    wxDateTime     dtTemp;

    dtTemp.Set(deadline);
    strBuffer = dtTemp.Format();
#endif

    return 0;
}




wxInt32 CViewWork::FormatStatus(wxInt32 item, wxString& strBuffer) const {
    CWork*          work;

    try {
        work = m_WorkCache.at(m_iSortedIndexes[item]);
    } catch ( std::out_of_range ) {
        work = NULL;
    }

    if (work) {
        strBuffer = work->m_strStatus;
    } else {
        strBuffer = wxEmptyString;
    }
    return 0;
}


void CViewWork::GetDocProjectURL(wxInt32 item, wxString& strBuffer) const {
    RESULT* result = wxGetApp().GetDocument()->result(item);

    if (result) {
        strBuffer = wxString(result->project_url, wxConvUTF8);
    }
}


double CViewWork::GetProgressValue(long item) {
    double          fBuffer = 0;
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
    if (fBuffer > 1) fBuffer = 1;

    return fBuffer;
}


wxString CViewWork::GetProgressText( long item) {
    CWork*    work      = NULL;
    wxString  strBuffer = wxEmptyString;
    
    if (GetWorkCacheAtIndex(work, m_iSortedIndexes[item])) {
        strBuffer = wxEmptyString;
    } else {
        strBuffer = work->m_strProgress;
    }

    return strBuffer;
}


int CViewWork::GetWorkCacheAtIndex(CWork*& workPtr, int index) {
    try {
        workPtr = m_WorkCache.at(index);
    } catch ( std::out_of_range ) {
        workPtr = NULL;
        return -1;
    }
    
    return 0;
}

