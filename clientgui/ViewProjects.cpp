// This file is part of BOINC.
// https://boinc.berkeley.edu
// Copyright (C) 2025 University of California
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
#include "DlgItemProperties.h"


#include "res/proj.xpm"

// Column IDs must be equal to the column's default
// position (left to right, zero-based) when all
// columns are shown.  However, any column may be
// hidden, either by default or by the user.
// (On MS Windows only, the user can also rearrange
// the columns from the default order.)
//
// Column IDs
#define COLUMN_PROJECT              0
#define COLUMN_ACCOUNTNAME          1
#define COLUMN_TEAMNAME             2
#define COLUMN_TOTALCREDIT          3
#define COLUMN_AVGCREDIT            4
#define COLUMN_RESOURCESHARE        5
#define COLUMN_STATUS               6

// DefaultShownColumns is an array containing the
// columnIDs of the columns to be shown by default,
// in ascending order.  It may or may not include
// all columns.
//
// For now, show all columns by default
static int DefaultShownColumns[] = { COLUMN_PROJECT, COLUMN_ACCOUNTNAME, COLUMN_TEAMNAME,
                                COLUMN_TOTALCREDIT, COLUMN_AVGCREDIT,
                                COLUMN_RESOURCESHARE, COLUMN_STATUS};

// groups that contain buttons
#define GRP_TASKS    0
#define GRP_WEBSITES 1

// buttons in the "tasks" area
#define BTN_UPDATE       0
#define BTN_SUSPEND      1
#define BTN_NOWORK       2
#define BTN_RESET        3
#define BTN_DETACH       4
#define BTN_PROPERTIES   5

static void format_total_credit(double credit, wxString& strBuffer)  {
    strBuffer = format_number(credit, 0);
}
static void format_avg_credit(double credit, wxString& strBuffer)  {
    strBuffer = format_number(credit, 2);
}

CProject::CProject() {
    m_fTotalCredit = -1.0;
    m_fAVGCredit = -1.0;
    m_fResourceShare = -1.0;
    m_fResourcePercent = -1.0;
}


CProject::~CProject() {
    m_strProjectName.Clear();
    m_strAccountName.Clear();
    m_strTeamName.Clear();
    m_strStatus.Clear();
}


IMPLEMENT_DYNAMIC_CLASS(CViewProjects, CBOINCBaseView)

BEGIN_EVENT_TABLE (CViewProjects, CBOINCBaseView)
    EVT_BUTTON(ID_TASK_PROJECT_UPDATE, CViewProjects::OnProjectUpdate)
    EVT_BUTTON(ID_TASK_PROJECT_SUSPEND, CViewProjects::OnProjectSuspend)
    EVT_BUTTON(ID_TASK_PROJECT_NONEWWORK, CViewProjects::OnProjectNoNewWork)
    EVT_BUTTON(ID_TASK_PROJECT_RESET, CViewProjects::OnProjectReset)
    EVT_BUTTON(ID_TASK_PROJECT_DETACH, CViewProjects::OnProjectDetach)
    EVT_BUTTON(ID_TASK_PROJECT_SHOW_PROPERTIES, CViewProjects::OnShowItemProperties)
    EVT_CUSTOM_RANGE(wxEVT_COMMAND_BUTTON_CLICKED, ID_TASK_PROJECT_WEB_PROJDEF_MIN, ID_TASK_PROJECT_WEB_PROJDEF_MAX, CViewProjects::OnProjectWebsiteClicked)
// We currently handle EVT_LIST_CACHE_HINT on Windows or
// EVT_CHECK_SELECTION_CHANGED on Mac & Linux instead of EVT_LIST_ITEM_SELECTED
// or EVT_LIST_ITEM_DESELECTED.  See CBOINCBaseView::OnCacheHint() for info.
#if USE_LIST_CACHE_HINT
    EVT_LIST_CACHE_HINT(ID_LIST_PROJECTSVIEW, CViewProjects::OnCacheHint)
#else
	EVT_CHECK_SELECTION_CHANGED(CViewProjects::OnCheckSelectionChanged)
#endif
    EVT_LIST_COL_CLICK(ID_LIST_PROJECTSVIEW, CViewProjects::OnColClick)
    EVT_LIST_COL_END_DRAG(ID_LIST_PROJECTSVIEW, CViewProjects::OnColResize)
END_EVENT_TABLE ()


static CViewProjects* myCViewProjects;

static bool CompareViewProjectsItems(int iRowIndex1, int iRowIndex2) {
    CProject*   project1;
    CProject*   project2;
    int         result = 0;

    try {
        project1 = myCViewProjects->m_ProjectCache.at(iRowIndex1);
    } catch ( std::out_of_range& ) {
        return 0;
    }

    try {
        project2 = myCViewProjects->m_ProjectCache.at(iRowIndex2);
    } catch ( std::out_of_range& ) {
        return 0;
    }

    switch (myCViewProjects->m_iSortColumnID) {
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

    // Always return FALSE for equality (result == 0)
    return (myCViewProjects->m_bReverseSort ? (result > 0) : (result < 0));
}


CViewProjects::CViewProjects()
{}


CViewProjects::CViewProjects(wxNotebook* pNotebook) :
    CBOINCBaseView(pNotebook, ID_TASK_PROJECTSVIEW, DEFAULT_TASK_FLAGS, ID_LIST_PROJECTSVIEW, DEFAULT_LIST_FLAGS)
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
        _("Report all completed tasks, get latest credit, get latest preferences, and possibly get more tasks."),
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
        _("Delete all files and tasks associated with this project, and get new tasks.  You can update the project first to report any completed tasks."),
        ID_TASK_PROJECT_RESET
    );
    pGroup->m_Tasks.push_back( pItem );

    pItem = new CTaskItem(
        _("Remove"),
        _("Remove this project.  Tasks in progress will be lost (use 'Update' first to report any completed tasks)."),
        ID_TASK_PROJECT_DETACH
    );
    pGroup->m_Tasks.push_back( pItem );

    pItem = new CTaskItem(
        _("Properties"),
        _("Show project details."),
        ID_TASK_PROJECT_SHOW_PROPERTIES
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
    m_aStdColNameOrder->Insert(_("Account"), COLUMN_ACCOUNTNAME);
    m_aStdColNameOrder->Insert(_("Team"), COLUMN_TEAMNAME);
    m_aStdColNameOrder->Insert(_("Work done"), COLUMN_TOTALCREDIT);
    m_aStdColNameOrder->Insert(_("Avg. work done"), COLUMN_AVGCREDIT);
    m_aStdColNameOrder->Insert(_("Resource share"), COLUMN_RESOURCESHARE);
    m_aStdColNameOrder->Insert(_("Status"), COLUMN_STATUS);

    // m_iStdColWidthOrder is an array of the width for each column.
    // Entries must be in order of ascending Column ID.  We initialize
    // it here to the default column widths.  It is updated by
    // CBOINCListCtrl::OnRestoreState() and also when a user resizes
    // a column by dragging the divider between two columns.
    //
    m_iStdColWidthOrder.Clear();
    m_iStdColWidthOrder.Insert(150, COLUMN_PROJECT);
    m_iStdColWidthOrder.Insert(80, COLUMN_ACCOUNTNAME);
    m_iStdColWidthOrder.Insert(80, COLUMN_TEAMNAME);
    m_iStdColWidthOrder.Insert(80, COLUMN_TOTALCREDIT);
    m_iStdColWidthOrder.Insert(80, COLUMN_AVGCREDIT);
    m_iStdColWidthOrder.Insert(80, COLUMN_RESOURCESHARE);
    m_iStdColWidthOrder.Insert(80, COLUMN_STATUS);

    wxASSERT(m_iStdColWidthOrder.size() == m_aStdColNameOrder->size());

    m_iDefaultShownColumns = DefaultShownColumns;
    m_iNumDefaultShownColumns = sizeof(DefaultShownColumns) / sizeof(int);
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


// Create List Pane Items
void CViewProjects::AppendColumn(int columnID){
    switch(columnID) {
        case COLUMN_PROJECT:
            m_pListPane->AppendColumn((*m_aStdColNameOrder)[COLUMN_PROJECT],
                    wxLIST_FORMAT_LEFT, m_iStdColWidthOrder[COLUMN_PROJECT]);
            break;
        case COLUMN_ACCOUNTNAME:
            m_pListPane->AppendColumn((*m_aStdColNameOrder)[COLUMN_ACCOUNTNAME],
                    wxLIST_FORMAT_LEFT, m_iStdColWidthOrder[COLUMN_ACCOUNTNAME]);
            break;
        case COLUMN_TEAMNAME:
            m_pListPane->AppendColumn((*m_aStdColNameOrder)[COLUMN_TEAMNAME],
                    wxLIST_FORMAT_LEFT, m_iStdColWidthOrder[COLUMN_TEAMNAME]);
            break;
        case COLUMN_TOTALCREDIT:
            m_pListPane->AppendColumn((*m_aStdColNameOrder)[COLUMN_TOTALCREDIT],
                    wxLIST_FORMAT_RIGHT, m_iStdColWidthOrder[COLUMN_TOTALCREDIT]);
            break;
        case COLUMN_AVGCREDIT:
            m_pListPane->AppendColumn((*m_aStdColNameOrder)[COLUMN_AVGCREDIT],
                    wxLIST_FORMAT_RIGHT, m_iStdColWidthOrder[COLUMN_AVGCREDIT]);
            break;
        case COLUMN_RESOURCESHARE:
            m_pListPane->AppendColumn((*m_aStdColNameOrder)[COLUMN_RESOURCESHARE],
                    wxLIST_FORMAT_CENTRE, m_iStdColWidthOrder[COLUMN_RESOURCESHARE]);
            break;
        case COLUMN_STATUS:
            m_pListPane->AppendColumn((*m_aStdColNameOrder)[COLUMN_STATUS],
                    wxLIST_FORMAT_LEFT, m_iStdColWidthOrder[COLUMN_STATUS]);
            break;
    }
}


wxString& CViewProjects::GetViewName() {
    static wxString strViewName(wxT("Projects"));
    return strViewName;
}


wxString& CViewProjects::GetViewDisplayName() {
    static wxString strViewName(_("Projects"));
    return strViewName;
}


const char** CViewProjects::GetViewIcon() {
    return proj_xpm;
}


int CViewProjects::GetViewCurrentViewPage() {
    return VW_PROJ;
}


wxString CViewProjects::GetKeyValue1(int iRowIndex) {
    CProject*   project;

    if (GetProjectCacheAtIndex(project, m_iSortedIndexes[iRowIndex])) {
        return wxEmptyString;
    }

    if (m_iColumnIDToColumnIndex[COLUMN_PROJECT] < 0) {
        // Column is hidden, so SynchronizeCacheItem() did not set its value
        GetDocProjectURL(m_iSortedIndexes[iRowIndex], project->m_strProjectURL);
    }

    return project->m_strProjectURL;
}


int CViewProjects::FindRowIndexByKeyValues(wxString& key1, wxString&) {
    CProject* project;
    unsigned int iRowIndex, n = GetCacheCount();
    for(iRowIndex=0; iRowIndex < n; iRowIndex++) {
        if (GetProjectCacheAtIndex(project, m_iSortedIndexes[iRowIndex])) {
            continue;
        }
        if((project->m_strProjectURL).IsSameAs(key1)) return iRowIndex;
    }
    return -1;
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
    wxASSERT(m_pListPane);

    row = -1;
    while (1) {
        // Step through all selected items
        row = m_pListPane->GetNextItem(row, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
        if (row < 0) break;

        pDoc->ProjectUpdate(m_iSortedIndexes[row]);
    }

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
                pDoc->ProjectResume(m_iSortedIndexes[row]);
            } else {
                pDoc->ProjectSuspend(m_iSortedIndexes[row]);
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
                pDoc->ProjectAllowMoreWork(m_iSortedIndexes[row]);
            } else {
                pDoc->ProjectNoMoreWork(m_iSortedIndexes[row]);
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
    wxASSERT(m_pListPane);

    if (!pDoc->IsUserAuthorized())
        return;

    row = -1;
    while (1) {
        // Step through all selected items
        row = m_pListPane->GetNextItem(row, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
        if (row < 0) break;

        if (GetProjectCacheAtIndex(pProject, m_iSortedIndexes[row])) {
            return;
        }

        strMessage.Printf(
            _("Are you sure you want to reset project '%s'?"),
            pProject->m_strProjectName.c_str()
        );

        iAnswer = wxGetApp().SafeMessageBox(
            strMessage,
            _("Reset Project"),
            wxYES_NO | wxICON_QUESTION,
            this
        );

        if (wxYES == iAnswer) {
            pDoc->ProjectReset(m_iSortedIndexes[row]);
        }
    }

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
    wxASSERT(m_pListPane);

    if (!pDoc->IsUserAuthorized())
        return;

    row = -1;
    while (1) {
        // Step through all selected items
        row = m_pListPane->GetNextItem(row, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
        if (row < 0) break;

        if (GetProjectCacheAtIndex(pProject, m_iSortedIndexes[row])) {
            return;
        }

        strMessage.Printf(
            _("Are you sure you want to remove project '%s'?"),
            pProject->m_strProjectName.c_str()
        );

        iAnswer = wxGetApp().SafeMessageBox(
            strMessage,
            _("Remove Project"),
            wxYES_NO | wxICON_QUESTION,
            this
        );

        if (wxYES == iAnswer) {
            pDoc->ProjectDetach(m_iSortedIndexes[row]);
        }
    }

    m_bForceUpdateSelection = true;
    UpdateSelection();
    pFrame->FireRefreshView();

    wxLogTrace(wxT("Function Start/End"), wxT("CViewProjects::OnProjectDetach - Function End"));
}


void CViewProjects::OnShowItemProperties( wxCommandEvent& WXUNUSED(event) ) {
    wxASSERT(m_pListPane);

    long item = m_pListPane->GetFirstSelected();
    PROJECT* project = wxGetApp().GetDocument()->project(m_iSortedIndexes[item]);

    if(!project) return;     // TODO: display some sort of error alert?
    //displaying the infos on a dialog
    CDlgItemProperties dlg(this);
    dlg.renderInfos(project);
    dlg.ShowModal();
}


void CViewProjects::OnProjectWebsiteClicked( wxEvent& event ) {
    wxLogTrace(wxT("Function Start/End"), wxT("CViewProjects::OnProjectWebsiteClicked - Function Begin"));

    CAdvancedFrame* pFrame      = wxDynamicCast(GetParent()->GetParent()->GetParent(), CAdvancedFrame);

    wxASSERT(pFrame);
    wxASSERT(wxDynamicCast(pFrame, CAdvancedFrame));

    int website_task_index = event.GetId() - ID_TASK_PROJECT_WEB_PROJDEF_MIN;
    wxLaunchDefaultBrowser(
        m_TaskGroups[1]->m_Tasks[website_task_index]->m_strWebSiteLink
    );

    UpdateSelection();
    pFrame->FireRefreshView();

    wxLogTrace(wxT("Function Start/End"), wxT("CViewProjects::OnProjectWebsiteClicked - Function End"));
}

void CViewProjects::OnColResize( wxListEvent& ) {
    // Register the new column widths immediately
    CAdvancedFrame* pFrame = wxDynamicCast(GetParent()->GetParent()->GetParent(), CAdvancedFrame);

    wxASSERT(pFrame);
    wxASSERT(wxDynamicCast(pFrame, CAdvancedFrame));
    pFrame->SaveState();
}


wxInt32 CViewProjects::GetDocCount() {
    CMainDocument* pDoc = wxGetApp().GetDocument();
    if (pDoc) {
        return pDoc->GetProjectCount();
    }
    return -1;
}


wxString CViewProjects::OnListGetItemText(long item, long column) const {
    CProject* project     = NULL;
    wxString       strBuffer = wxEmptyString;

    m_pListPane->AddPendingProgressBar(item);

    try {
        project = m_ProjectCache.at(m_iSortedIndexes[item]);
    } catch ( std::out_of_range& ) {
        project = NULL;
    }

    if (project && (column >= 0)) {
        switch(m_iColumnIndexToColumnID[column]) {
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
                // CBOINCListCtrl::DrawProgressBars() will draw this using
                // data provided by GetProgressText() and GetProgressValue(),
                // but we need it here for accessibility programs.
                strBuffer = project->m_strResourceShare;
                break;
            case COLUMN_STATUS:
                strBuffer = project->m_strStatus;
                break;
        }
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


bool CViewProjects::IsSelectionManagementNeeded() {
    return true;
}

void CViewProjects::UpdateSelection() {
    CTaskItemGroup*     pGroup = NULL;
    PROJECT*            project = NULL;
    CMainDocument*      pDoc = wxGetApp().GetDocument();
    int                 i, n, row;
    bool                wasSuspended=false, wasNoNewWork=false;
    bool                enableUpdate = false;
    bool                enableSuspendResume = false;
    bool                enableNoNewTasks = false;
    bool                enableReset = false;
    bool                enableDetach = false;
    bool                enableProperties = false;

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
        enableUpdate = true;
        enableSuspendResume = true;
        enableNoNewTasks = true;
        enableReset = true;
        enableDetach = true;
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
                enableSuspendResume = false;
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
                enableNoNewTasks = false;
            }
        }

        if (project->attached_via_acct_mgr) {
            enableDetach = false;
        }
    }

    if (n == 1) {
        enableProperties = true;

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

    // To minimize flicker, set each button only once to the final desired state
    pGroup->m_Tasks[BTN_UPDATE]->m_pButton->Enable(enableUpdate);
    pGroup->m_Tasks[BTN_SUSPEND]->m_pButton->Enable(enableSuspendResume);
    pGroup->m_Tasks[BTN_NOWORK]->m_pButton->Enable(enableNoNewTasks);
    pGroup->m_Tasks[BTN_RESET]->m_pButton->Enable(enableReset);
    pGroup->m_Tasks[BTN_DETACH]->m_pButton->Enable(enableDetach);
    pGroup->m_Tasks[BTN_PROPERTIES]->m_pButton->Enable(enableProperties);

    CBOINCBaseView::PostUpdateSelection();
}


bool CViewProjects::SynchronizeCacheItem(wxInt32 iRowIndex, wxInt32 iColumnIndex) {
    wxString    strDocumentText  = wxEmptyString;
    wxString    strDocumentText2 = wxEmptyString;
    double       x = 0.0;
    double       fDocumentPercent = 0.0;
    CProject*   project;
    bool        dirty = false;

    if (GetProjectCacheAtIndex(project, m_iSortedIndexes[iRowIndex])) {
            return false;
    }

    if (iColumnIndex < 0) return false;

    strDocumentText.Empty();

   switch (m_iColumnIndexToColumnID[iColumnIndex]) {
        case COLUMN_PROJECT:
            GetDocProjectName(m_iSortedIndexes[iRowIndex], strDocumentText);
            GetDocProjectURL(m_iSortedIndexes[iRowIndex], strDocumentText2);
            if (!strDocumentText.IsSameAs(project->m_strProjectName) || !strDocumentText2.IsSameAs(project->m_strProjectURL)) {
                project->m_strProjectName = strDocumentText;
                project->m_strProjectURL = strDocumentText2;
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
            GetDocTotalCredit(m_iSortedIndexes[iRowIndex], x);
            if (x != project->m_fTotalCredit) {
                project->m_fTotalCredit = x;
                format_total_credit(x, project->m_strTotalCredit);
                return true;
            }
            break;
        case COLUMN_AVGCREDIT:
            GetDocAVGCredit(m_iSortedIndexes[iRowIndex], x);
            if (x != project->m_fAVGCredit) {
                project->m_fAVGCredit = x;
                format_avg_credit(x, project->m_strAVGCredit);
                return true;
            }
            break;
        case COLUMN_RESOURCESHARE:
            GetDocResourceShare(m_iSortedIndexes[iRowIndex], x);
            if (x != project->m_fResourceShare) {
                project->m_fResourceShare = x;
                dirty = true;
            }
            GetDocResourcePercent(m_iSortedIndexes[iRowIndex], fDocumentPercent);
            if (fDocumentPercent != project->m_fResourcePercent) {
                project->m_fResourcePercent = fDocumentPercent;
                dirty = true;
            }
            if (dirty) {
                FormatResourceShare(x, fDocumentPercent, project->m_strResourceShare);
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
    PROJECT* project = NULL;
    CMainDocument* pDoc = wxGetApp().GetDocument();
    std::string project_name;

    if (pDoc) {
        project = pDoc->project(item);
    }

    if (project) {
        project->get_name(project_name);
        strBuffer = HtmlEntityDecode(wxString(project_name.c_str(), wxConvUTF8));
    } else {
        strBuffer = wxEmptyString;
    }
}


wxInt32 CViewProjects::FormatProjectName(wxInt32 item, wxString& strBuffer) const {
    CProject* project;

    try {
        project = m_ProjectCache.at(m_iSortedIndexes[item]);
    } catch ( std::out_of_range& ) {
        project = NULL;
    }

    if (project) {
        strBuffer = project->m_strProjectName;
    } else {
        strBuffer = wxEmptyString;
    }

    return 0;
}


void CViewProjects::GetDocAccountName(wxInt32 item, wxString& strBuffer) const {
    PROJECT* project = NULL;
    CMainDocument* pDoc = wxGetApp().GetDocument();

    if (pDoc) {
        project = pDoc->project(item);
    }

    if (project) {
        strBuffer = HtmlEntityDecode(wxString(project->user_name.c_str(), wxConvUTF8));
    } else {
        strBuffer = wxEmptyString;
    }
}


wxInt32 CViewProjects::FormatAccountName(wxInt32 item, wxString& strBuffer) const {
    CProject* project;

    try {
        project = m_ProjectCache.at(m_iSortedIndexes[item]);
    } catch ( std::out_of_range& ) {
        project = NULL;
    }
    if (project) {
        strBuffer = project->m_strAccountName;
    } else {
        strBuffer = wxEmptyString;
    }

    return 0;
}


void CViewProjects::GetDocTeamName(wxInt32 item, wxString& strBuffer) const {
    PROJECT* project = NULL;
    CMainDocument* pDoc = wxGetApp().GetDocument();

    if (pDoc) {
        project = pDoc->project(item);
    }

    if (project) {
        strBuffer = HtmlEntityDecode(wxString(project->team_name.c_str(), wxConvUTF8));
    } else {
        strBuffer = wxEmptyString;
    }
}


wxInt32 CViewProjects::FormatTeamName(wxInt32 item, wxString& strBuffer) const {
    CProject* project;

    try {
        project = m_ProjectCache.at(m_iSortedIndexes[item]);
    } catch ( std::out_of_range& ) {
        project = NULL;
    }

    if (project) {
        strBuffer = project->m_strTeamName;
    } else {
        strBuffer = wxEmptyString;
    }

    return 0;
}


void CViewProjects::GetDocTotalCredit(wxInt32 item, double& fBuffer) const {
    PROJECT* project = NULL;
    CMainDocument* pDoc = wxGetApp().GetDocument();

    if (pDoc) {
        project = pDoc->project(item);
    }

    if (project) {
        fBuffer = project->user_total_credit;
    } else {
        fBuffer = 0.0;
    }
}


void CViewProjects::GetDocAVGCredit(wxInt32 item, double& fBuffer) const {
    PROJECT* project = NULL;
    CMainDocument* pDoc = wxGetApp().GetDocument();

    if (pDoc) {
        project = pDoc->project(item);
    }

    if (project) {
        fBuffer = project->user_expavg_credit;
    } else {
        fBuffer = 0.0;
    }
}

void CViewProjects::GetDocResourceShare(wxInt32 item, double& fBuffer) const {
    PROJECT* project = NULL;
    CMainDocument* pDoc = wxGetApp().GetDocument();

    if (pDoc) {
        project = pDoc->project(item);
    }

    if (project) {
        fBuffer = project->resource_share;
    } else {
        fBuffer = 0.0;
    }
}


void CViewProjects::GetDocResourcePercent(wxInt32 item, double& fBuffer) const {
    PROJECT* project = NULL;
    CMainDocument* pDoc = wxGetApp().GetDocument();

    if (pDoc) {
        project = pDoc->project(item);
    }

    if (project && pDoc) {
        fBuffer = (project->resource_share / pDoc->m_fProjectTotalResourceShare) * 100;
    } else {
        fBuffer = 0.0;
    }
}


wxInt32 CViewProjects::FormatResourceShare(double share, double share_pct, wxString& strBuffer) const {
    strBuffer.Printf(wxT("%s (%s%%)"), format_number(share, 0), format_number(share_pct, 2));

    return 0;
}

wxString rpc_reason_string_translated(int reason) {
    switch (reason) {
    case RPC_REASON_USER_REQ: return _("Requested by user");
    case RPC_REASON_NEED_WORK: return _("To fetch work");
    case RPC_REASON_RESULTS_DUE: return _("To report completed tasks");
    case RPC_REASON_TRICKLE_UP: return _("To send trickle-up message");
    case RPC_REASON_ACCT_MGR_REQ: return _("Requested by account manager");
    case RPC_REASON_INIT: return _("Project initialization");
    case RPC_REASON_PROJECT_REQ: return _("Requested by project");
    default: return _("Unknown reason");
    }
}

void CViewProjects::GetDocStatus(wxInt32 item, wxString& strBuffer) const {
    PROJECT* project = NULL;
    CMainDocument* pDoc = wxGetApp().GetDocument();

    if (pDoc) {
        project = pDoc->project(item);
    }

    if (project) {
        if (project->suspended_via_gui) {
            append_to_status(strBuffer, _("Suspended by user"));
        }
        if (project->dont_request_more_work) {
            append_to_status(strBuffer, _("Won't get new tasks"));
        }
        if (project->ended) {
            append_to_status(strBuffer, _("Project ended - OK to remove"));
        }
        if (project->detach_when_done) {
            append_to_status(strBuffer, _("Will remove when tasks done"));
        }
        if (project->sched_rpc_pending) {
            append_to_status(strBuffer, _("Scheduler request pending"));
            append_to_status(strBuffer,
                rpc_reason_string_translated(project->sched_rpc_pending)
            );
        }
        if (project->scheduler_rpc_in_progress) {
            append_to_status(strBuffer, _("Scheduler request in progress"));
        }
        if (project->trickle_up_pending) {
            append_to_status(strBuffer, _("Trickle up message pending"));
        }
        wxDateTime dtNextRPC((time_t)project->min_rpc_time);
        wxDateTime dtNow(wxDateTime::Now());
        if (dtNextRPC > dtNow) {
            wxTimeSpan tsNextRPC(dtNextRPC - dtNow);
            append_to_status(strBuffer, _("Communication deferred") + wxString(" ") + tsNextRPC.Format());
        }
    }
}


wxInt32 CViewProjects::FormatStatus(wxInt32 item, wxString& strBuffer) const {
    CProject* project;

    try {
        project = m_ProjectCache.at(m_iSortedIndexes[item]);
    } catch ( std::out_of_range& ) {
        project = NULL;
    }

    if (project) {
        strBuffer = project->m_strStatus;
    } else {
        strBuffer = wxEmptyString;
    }

    return 0;
}


void CViewProjects::GetDocProjectURL(wxInt32 item, wxString& strBuffer) const {
    PROJECT* project = NULL;
    CMainDocument* pDoc = wxGetApp().GetDocument();

    if (pDoc) {
        project = pDoc->project(item);
    }

    if (project) {
        strBuffer = wxString(project->master_url, wxConvUTF8);
    } else {
        strBuffer = wxEmptyString;
    }
}


double CViewProjects::GetProgressValue(long item) {
    CProject* project;

    try {
        project = m_ProjectCache.at(m_iSortedIndexes[item]);
    } catch ( std::out_of_range& ) {
        project = NULL;
    }

    if (project) {
        return (project->m_fResourcePercent) / 100.0;
    }

    return 0.0;
}


wxString CViewProjects::GetProgressText( long item) {
    CProject* project     = NULL;
    wxString       strBuffer = wxEmptyString;

    try {
        project = m_ProjectCache.at(m_iSortedIndexes[item]);
    } catch ( std::out_of_range& ) {
        project = NULL;
    }

    if (project) {
        strBuffer = project->m_strResourceShare;
    }
    return strBuffer;
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
    long lProjectIndex, lWebsiteIndex;

    strTemplate.Replace(wxT("web:"), wxEmptyString);

    strBuffer = strTemplate;
    strBuffer.Remove(strBuffer.Find(wxT(":")));
    strBuffer.ToLong((long*) &lProjectIndex);
    iProjectIndex = lProjectIndex;

    strBuffer = strTemplate;
    strBuffer = strBuffer.Mid(strBuffer.Find(wxT(":")) + 1);
    strBuffer.ToLong((long*) &lWebsiteIndex);
    iWebsiteIndex = lWebsiteIndex;

    return 0;
}


int CViewProjects::GetProjectCacheAtIndex(CProject*& projectPtr, int index) {
    try {
        projectPtr = m_ProjectCache.at(index);
    } catch ( std::out_of_range& ) {
        projectPtr = NULL;
        return -1;
    }

    return 0;
}

