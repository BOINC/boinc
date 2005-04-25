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
#pragma implementation "ViewProjects.h"
#endif

#include "stdwx.h"
#include "BOINCGUIApp.h"
#include "MainDocument.h"
#include "BOINCTaskCtrl.h"
#include "BOINCListCtrl.h"
#include "ViewProjects.h"
#include "DlgAttachProject.h"
#include "Events.h"


#include "res/proj.xpm"


#define COLUMN_PROJECT              0
#define COLUMN_ACCOUNTNAME          1
#define COLUMN_TEAMNAME             2
#define COLUMN_TOTALCREDIT          3
#define COLUMN_AVGCREDIT            4
#define COLUMN_RESOURCESHARE        5
#define COLUMN_STATUS               6

// buttons in the "tasks" area
#define BTN_UPDATE  0
#define BTN_SUSPEND 1
#define BTN_NOWORK  2
#define BTN_RESET   3
#define BTN_DETACH  4
#define BTN_ATTACH  5


CProject::CProject() {
    m_strProjectName = wxEmptyString;
    m_strAccountName = wxEmptyString;
    m_strTeamName = wxEmptyString;
    m_strTotalCredit = wxEmptyString;
    m_strAVGCredit = wxEmptyString;
    m_strResourceShare = wxEmptyString;
    m_strStatus = wxEmptyString;
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
    EVT_BUTTON(ID_TASK_PROJECT_ATTACH, CViewProjects::OnProjectAttach)
    EVT_CUSTOM_RANGE(wxEVT_COMMAND_BUTTON_CLICKED, ID_TASK_PROJECT_WEB_PROJDEF_MIN, ID_TASK_PROJECT_WEB_PROJDEF_MAX, CViewProjects::OnProjectWebsiteClicked)
    EVT_LIST_ITEM_SELECTED(ID_LIST_PROJECTSVIEW, CViewProjects::OnListSelected)
    EVT_LIST_ITEM_DESELECTED(ID_LIST_PROJECTSVIEW, CViewProjects::OnListDeselected)
END_EVENT_TABLE ()


CViewProjects::CViewProjects() {}


CViewProjects::CViewProjects(wxNotebook* pNotebook) :
    CBOINCBaseView(pNotebook, ID_TASK_PROJECTSVIEW, DEFAULT_TASK_FLAGS, ID_LIST_PROJECTSVIEW, DEFAULT_LIST_SINGLE_SEL_FLAGS)
{
    wxInt32         iCurrentEventID = 0;
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
        _("Update"),
        _("Report all completed work, get latest credit, "
          "get latest preferences, and possibly get more work."),
        ID_TASK_PROJECT_UPDATE 
    );
    pGroup->m_Tasks.push_back( pItem );

	pItem = new CTaskItem(
        _("Suspend"),
        _("Stop work for this project "
          "(you can resume later)."),
        ID_TASK_PROJECT_SUSPEND 
    );
    pGroup->m_Tasks.push_back( pItem );

	pItem = new CTaskItem(
        _("Don't Get New Work"),
        _("Tell the project to not "
          "fetch additional work for this "
          "project. Any work already downloaded will "
          "still be processed and returned."),
        ID_TASK_PROJECT_NONEWWORK 
    );
    pGroup->m_Tasks.push_back( pItem );

	pItem = new CTaskItem(
        _("Reset project"),
        _("Delete all files and work associated with this project, "
          "and get new work.  "
          "You can update the project "
          "first to report any completed work."),
        ID_TASK_PROJECT_RESET 
    );
    pGroup->m_Tasks.push_back( pItem );

	pItem = new CTaskItem(
        _("Detach"),
        _("Detach this computer from this project.  "
          "Work in progress will be lost. "
          "You can update the project first to report "
          "any completed work."),
        ID_TASK_PROJECT_DETACH 
    );
    pGroup->m_Tasks.push_back( pItem );

	pItem = new CTaskItem(
        _("Attach to new project"),
        _("Attach this computer to a BOINC project.  "
          "You'll need a project URL and account key "
          "(visit the project's web site to get these)."),
        ID_TASK_PROJECT_ATTACH 
    );
    pGroup->m_Tasks.push_back( pItem );


	pGroup = new CTaskItemGroup( _("Web sites") );
	m_TaskGroups.push_back( pGroup );

    for (iCurrentEventID = ID_TASK_PROJECT_WEB_PROJDEF_MIN; iCurrentEventID <= ID_TASK_PROJECT_WEB_PROJDEF_MAX; iCurrentEventID++) {
        pItem = new CTaskItem(
            wxT(""),
            wxT(""),
            iCurrentEventID 
        );
        pGroup->m_Tasks.push_back( pItem );
    }

    // Create Task Pane Items
    m_pTaskPane->UpdateControls();

    // Create List Pane Items
    m_pListPane->InsertColumn(COLUMN_PROJECT, _("Project"), wxLIST_FORMAT_LEFT, 150);
    m_pListPane->InsertColumn(COLUMN_ACCOUNTNAME, _("Account"), wxLIST_FORMAT_LEFT, 80);
    m_pListPane->InsertColumn(COLUMN_TEAMNAME, _("Team"), wxLIST_FORMAT_LEFT, 80);
    m_pListPane->InsertColumn(COLUMN_TOTALCREDIT, _("Total credit"), wxLIST_FORMAT_RIGHT, 80);
    m_pListPane->InsertColumn(COLUMN_AVGCREDIT, _("Avg. credit"), wxLIST_FORMAT_RIGHT, 80);
    m_pListPane->InsertColumn(COLUMN_RESOURCESHARE, _("Resource share"), wxLIST_FORMAT_CENTRE, 85);
    m_pListPane->InsertColumn(COLUMN_STATUS, _("Status"), wxLIST_FORMAT_LEFT, 150);

    UpdateSelection();
}


CViewProjects::~CViewProjects() {
    EmptyCache();
    EmptyTasks();
}


wxString CViewProjects::GetViewName() {
    return wxString(_("Projects"));
}


const char** CViewProjects::GetViewIcon() {
    return proj_xpm;
}


void CViewProjects::OnProjectUpdate( wxCommandEvent& event ) {
    wxLogTrace(wxT("Function Start/End"), wxT("CViewProjects::OnProjectUpdate - Function Begin"));

    CMainDocument* pDoc     = wxGetApp().GetDocument();
    CMainFrame* pFrame      = wxGetApp().GetFrame();

    wxASSERT(NULL != pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));
    wxASSERT(NULL != pFrame);
    wxASSERT(wxDynamicCast(pFrame, CMainFrame));
    wxASSERT(NULL != m_pTaskPane);
    wxASSERT(NULL != m_pListPane);

    pFrame->UpdateStatusText(_("Updating project..."));
    pDoc->ProjectUpdate(m_pListPane->GetFirstSelected());
    pFrame->UpdateStatusText(wxT(""));

    UpdateSelection();
    pFrame->ProcessRefreshView();

    wxLogTrace(wxT("Function Start/End"), wxT("CViewProjects::OnProjectUpdate - Function End"));
}


void CViewProjects::OnProjectSuspend( wxCommandEvent& event ) {
    wxLogTrace(wxT("Function Start/End"), wxT("CViewProjects::OnProjectSuspend - Function Begin"));

    CMainDocument* pDoc     = wxGetApp().GetDocument();
    CMainFrame* pFrame      = wxGetApp().GetFrame();

    wxASSERT(NULL != pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));
    wxASSERT(NULL != pFrame);
    wxASSERT(wxDynamicCast(pFrame, CMainFrame));
    wxASSERT(NULL != m_pTaskPane);
    wxASSERT(NULL != m_pListPane);

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

    UpdateSelection();
    pFrame->ProcessRefreshView();

    wxLogTrace(wxT("Function Start/End"), wxT("CViewProjects::OnProjectSuspend - Function End"));
}


void CViewProjects::OnProjectNoNewWork( wxCommandEvent& event ) {
    wxLogTrace(wxT("Function Start/End"), wxT("CViewProjects::OnProjectNoNewWork - Function Begin"));

    CMainDocument* pDoc     = wxGetApp().GetDocument();
    CMainFrame* pFrame      = wxGetApp().GetFrame();

    wxASSERT(NULL != pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));
    wxASSERT(NULL != pFrame);
    wxASSERT(wxDynamicCast(pFrame, CMainFrame));
    wxASSERT(NULL != m_pTaskPane);
    wxASSERT(NULL != m_pListPane);

    PROJECT* project = pDoc->project(m_pListPane->GetFirstSelected());
    if (project->dont_request_more_work) {
        pFrame->UpdateStatusText(_("Telling project to allow additional work downloads..."));
        pDoc->ProjectAllowMoreWork(m_pListPane->GetFirstSelected());
        pFrame->UpdateStatusText(wxT(""));
    } else {
        pFrame->UpdateStatusText(_("Telling project to not fetch additional work..."));
        pDoc->ProjectNoMoreWork(m_pListPane->GetFirstSelected());
        pFrame->UpdateStatusText(wxT(""));
    }

    UpdateSelection();
    pFrame->ProcessRefreshView();

    wxLogTrace(wxT("Function Start/End"), wxT("CViewProjects::OnProjectNoNewWork - Function End"));
}




void CViewProjects::OnProjectReset( wxCommandEvent& event ) {
    wxLogTrace(wxT("Function Start/End"), wxT("CViewProjects::OnProjectReset - Function Begin"));

    wxInt32  iAnswer        = 0; 
    wxString strProjectName = wxEmptyString;
    wxString strMessage     = wxEmptyString;
    CMainDocument* pDoc     = wxGetApp().GetDocument();
    CMainFrame* pFrame      = wxGetApp().GetFrame();

    wxASSERT(NULL != pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));
    wxASSERT(NULL != pFrame);
    wxASSERT(wxDynamicCast(pFrame, CMainFrame));
    wxASSERT(NULL != m_pTaskPane);
    wxASSERT(NULL != m_pListPane);

    pFrame->UpdateStatusText(_("Resetting project..."));

    pDoc->GetProjectProjectName(m_pListPane->GetFirstSelected(), strProjectName);

    strMessage.Printf(
        _("Are you sure you want to reset project '%s'?"), 
        strProjectName.c_str());

    iAnswer = wxMessageBox(
        strMessage,
        _("Reset Project"),
        wxYES_NO | wxICON_QUESTION, 
        this
    );

    if (wxYES == iAnswer) {
        pDoc->ProjectReset(m_pListPane->GetFirstSelected());
    }

    pFrame->UpdateStatusText(wxT(""));

    UpdateSelection();
    pFrame->ProcessRefreshView();

    wxLogTrace(wxT("Function Start/End"), wxT("CViewProjects::OnProjectReset - Function End"));
}


void CViewProjects::OnProjectDetach( wxCommandEvent& event ) {
    wxLogTrace(wxT("Function Start/End"), wxT("CViewProjects::OnProjectDetach - Function Begin"));

    wxInt32  iAnswer        = 0; 
    wxString strProjectName = wxEmptyString;
    wxString strMessage     = wxEmptyString;
    CMainDocument* pDoc     = wxGetApp().GetDocument();
    CMainFrame* pFrame      = wxGetApp().GetFrame();

    wxASSERT(NULL != pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));
    wxASSERT(NULL != pFrame);
    wxASSERT(wxDynamicCast(pFrame, CMainFrame));
    wxASSERT(NULL != m_pTaskPane);
    wxASSERT(NULL != m_pListPane);

    pFrame->UpdateStatusText(_("Detaching from project..."));

    pDoc->GetProjectProjectName(m_pListPane->GetFirstSelected(), strProjectName);

    strMessage.Printf(
        _("Are you sure you want to detach from project '%s'?"), 
        strProjectName.c_str()
    );

    iAnswer = wxMessageBox(
        strMessage,
        _("Detach from Project"),
        wxYES_NO | wxICON_QUESTION, 
        this
    );

    if (wxYES == iAnswer) {
        pDoc->ProjectDetach(m_pListPane->GetFirstSelected());
    }

    pFrame->UpdateStatusText(wxT(""));

    UpdateSelection();
    pFrame->ProcessRefreshView();

    wxLogTrace(wxT("Function Start/End"), wxT("CViewProjects::OnProjectDetach - Function End"));
}


void CViewProjects::OnProjectAttach( wxCommandEvent& event ) {
    wxLogTrace(wxT("Function Start/End"), wxT("CViewProjects::OnProjectAttach - Function Begin"));

    wxInt32  iAnswer        = 0; 
    CMainDocument* pDoc     = wxGetApp().GetDocument();
    CMainFrame* pFrame      = wxGetApp().GetFrame();

    wxASSERT(NULL != pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));
    wxASSERT(NULL != pFrame);
    wxASSERT(wxDynamicCast(pFrame, CMainFrame));
    wxASSERT(NULL != m_pTaskPane);
    wxASSERT(NULL != m_pListPane);

    pFrame->UpdateStatusText(_("Attaching to project..."));

    CDlgAttachProject* pDlg = new CDlgAttachProject(this);
    wxASSERT(NULL != pDlg);

    iAnswer = pDlg->ShowModal();

    if (wxID_OK == iAnswer) {
        pDoc->ProjectAttach(
            pDlg->GetProjectAddress(), 
            pDlg->GetProjectAccountKey()
        );
    }

    if (pDlg)
        pDlg->Destroy();

    pFrame->UpdateStatusText(wxT(""));

    UpdateSelection();
    pFrame->ProcessRefreshView();

    wxLogTrace(wxT("Function Start/End"), wxT("CViewProjects::OnProjectAttach - Function End"));
}


void CViewProjects::OnProjectWebsiteClicked( wxEvent& event ) {
    wxLogTrace(wxT("Function Start/End"), wxT("CViewProjects::OnProjectWebsiteClicked - Function Begin"));

    CMainFrame*         pFrame = wxGetApp().GetFrame();

    wxASSERT(NULL != pFrame);
    wxASSERT(wxDynamicCast(pFrame, CMainFrame));
    wxASSERT(NULL != m_pTaskPane);
    wxASSERT(NULL != m_pListPane);

    pFrame->UpdateStatusText(_("Launching browser..."));

    int website_task_index = event.GetId() - ID_TASK_PROJECT_WEB_PROJDEF_MIN;
    pFrame->ExecuteBrowserLink(
        m_TaskGroups[1]->m_Tasks[website_task_index]->m_strWebSiteLink
        );

    pFrame->UpdateStatusText(wxT(""));

    UpdateSelection();
    pFrame->ProcessRefreshView();

    wxLogTrace(wxT("Function Start/End"), wxT("CViewProjects::OnProjectWebsiteClicked - Function End"));
}


wxInt32 CViewProjects::GetDocCount() {
    CMainDocument* pDoc      = wxGetApp().GetDocument();

    wxASSERT(NULL != pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));

    return pDoc->GetProjectCount();
}


wxString CViewProjects::OnListGetItemText(long item, long column) const {
    CProject* project     = m_ProjectCache.at(item);
    wxString  strBuffer   = wxEmptyString;

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
    wxASSERT(NULL != pItem);
    if (NULL != pItem) {
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
    return m_ProjectCache.size();
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
    unsigned int        j;
    CTaskItemGroup*     pGroup = NULL;
    CTaskItem*          pItem = NULL;
    PROJECT*            project = NULL;
    CMainDocument*      pDoc = wxGetApp().GetDocument();

    wxASSERT(NULL != pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));

    // Update the tasks static box buttons
    pGroup = m_TaskGroups[0];
    if (m_pListPane->GetSelectedItemCount() == 0) {
        pGroup->button(BTN_UPDATE)->Disable();
        pGroup->button(BTN_SUSPEND)->Disable();
        pGroup->button(BTN_NOWORK)->Disable();
        pGroup->button(BTN_RESET)->Disable();
        pGroup->button(BTN_DETACH)->Disable();
        pGroup->button(BTN_ATTACH)->Enable();
    } else {
        project = pDoc->project(m_pListPane->GetFirstSelected());
        pGroup->button(BTN_UPDATE)->Enable();
        pGroup->button(BTN_SUSPEND)->Enable();
        if (project->suspended_via_gui) {
            pGroup->button(BTN_SUSPEND)->SetLabel(_("Resume"));
#if wxUSE_TOOLTIPS
            pGroup->button(BTN_SUSPEND)->SetToolTip(_("Resume work for this project"));
#endif
        } else {
            pGroup->button(BTN_SUSPEND)->SetLabel(_("Suspend"));
#if wxUSE_TOOLTIPS
            pGroup->button(BTN_SUSPEND)->SetToolTip(_("Suspend work for this project"));
#endif
        }
        pGroup->button(BTN_NOWORK)->Enable();
        if (project->dont_request_more_work) {
            pGroup->button(BTN_NOWORK)->SetLabel(_("Allow new work"));
#if wxUSE_TOOLTIPS
            pGroup->button(BTN_NOWORK)->SetToolTip(_("Allow fetching new work for this project"));
#endif
        } else {
            pGroup->button(BTN_NOWORK)->SetLabel(_("No new work"));
#if wxUSE_TOOLTIPS
            pGroup->button(BTN_NOWORK)->SetToolTip(_("Don't fetch new work for this project"));
#endif
        }
        pGroup->button(BTN_RESET)->Enable();
        pGroup->button(BTN_DETACH)->Enable();
        pGroup->button(BTN_ATTACH)->Enable();
    }

    // Update the websites list
    pGroup = m_TaskGroups[1];
    pGroup->m_pStaticBox->Hide();
    for (j=0; j < pGroup->m_Tasks.size(); j++) {
        pItem = pGroup->m_Tasks[j];
        pItem->m_pButton->Hide();
    }
    if (m_pListPane->GetSelectedItemCount()) {
        project = pDoc->project(m_pListPane->GetFirstSelected());

        pGroup->m_pStaticBox->Show();

        // Default project url
        pItem = pGroup->m_Tasks[0];
        pItem->m_pButton->SetLabel(project->project_name.c_str());
#if wxUSE_TOOLTIPS
        pItem->m_pButton->SetToolTip(wxT(""));
#endif
        pItem->m_pButton->Show();
        pItem->m_strWebSiteLink = project->master_url.c_str();


        // Project defined urls
        unsigned int number_of_gui_urls = project->gui_urls.size();
        unsigned int max_number_of_buttons = ID_TASK_PROJECT_WEB_PROJDEF_MAX-ID_TASK_PROJECT_WEB_PROJDEF_MIN;
        int gui_url_index = 0;
        for (j=1;j<=number_of_gui_urls && j<=max_number_of_buttons;j++) {
            gui_url_index = j - 1;
            pItem = pGroup->m_Tasks[j];
            pItem->m_pButton->SetLabel(project->gui_urls[gui_url_index].name.c_str());
#if wxUSE_TOOLTIPS
            pItem->m_pButton->SetToolTip(project->gui_urls[gui_url_index].description.c_str());
#endif
            pItem->m_pButton->Show();
            pItem->m_strWebSiteLink = project->gui_urls[gui_url_index].url.c_str();
        }
    }
}


wxInt32 CViewProjects::FormatProjectName(wxInt32 item, wxString& strBuffer) const {
    CMainDocument* pDoc = wxGetApp().GetDocument();

    wxASSERT(NULL != pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));

    strBuffer.Clear();

    pDoc->GetProjectProjectName(item, strBuffer);

    return 0;
}


wxInt32 CViewProjects::FormatAccountName(wxInt32 item, wxString& strBuffer) const {
    CMainDocument* pDoc = wxGetApp().GetDocument();

    wxASSERT(NULL != pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));

    strBuffer.Clear();

    pDoc->GetProjectAccountName(item, strBuffer);

    return 0;
}


wxInt32 CViewProjects::FormatTeamName(wxInt32 item, wxString& strBuffer) const {
    CMainDocument* pDoc = wxGetApp().GetDocument();

    wxASSERT(NULL != pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));

    strBuffer.Clear();

    pDoc->GetProjectTeamName(item, strBuffer);

    return 0;
}


wxInt32 CViewProjects::FormatTotalCredit(wxInt32 item, wxString& strBuffer) const {
    float fBuffer;
    CMainDocument* pDoc = wxGetApp().GetDocument();

    wxASSERT(NULL != pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));

    strBuffer.Clear();

    pDoc->GetProjectTotalCredit(item, fBuffer);
    strBuffer.Printf(wxT("%0.2f"), fBuffer);

    return 0;
}


wxInt32 CViewProjects::FormatAVGCredit(wxInt32 item, wxString& strBuffer) const {
    float fBuffer;
    CMainDocument* pDoc = wxGetApp().GetDocument();

    wxASSERT(NULL != pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));

    strBuffer.Clear();

    pDoc->GetProjectAvgCredit(item, fBuffer);
    strBuffer.Printf(wxT("%0.2f"), fBuffer);

    return 0;
}


wxInt32 CViewProjects::FormatResourceShare(wxInt32 item, wxString& strBuffer) const {
    float fResourceShareBuffer;
    float fTotalResourceShareBuffer;
    CMainDocument* pDoc = wxGetApp().GetDocument();

    wxASSERT(NULL != pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));

    strBuffer.Clear();

    pDoc->GetProjectResourceShare(item, fResourceShareBuffer);
    pDoc->GetProjectTotalResourceShare(item, fTotalResourceShareBuffer);
    strBuffer.Printf(wxT("%0.0f (%0.2f%%)"), fResourceShareBuffer, ((fResourceShareBuffer / fTotalResourceShareBuffer) * 100));

    return 0;
}


wxInt32 CViewProjects::FormatStatus(wxInt32 item, wxString& strBuffer) const {
    wxInt32 iNextRPC;
    CMainDocument* pDoc = wxGetApp().GetDocument();

    wxASSERT(NULL != pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));

    strBuffer.Clear();

    if (pDoc->IsProjectSuspended(item)) {
        strBuffer = _("Project suspended");
    } else if (pDoc->IsProjectAllowedToGetWork(item)) {
        strBuffer = _("Won't get new work");
    } else if (pDoc->IsProjectRPCPending(item)) {
        pDoc->GetProjectMinRPCTime(item, iNextRPC);

        wxDateTime dtNextRPC((time_t)iNextRPC);
        wxDateTime dtNow(wxDateTime::Now());

        if (dtNextRPC > dtNow) {
            wxTimeSpan tsNextRPC(dtNextRPC - dtNow);
            strBuffer = _("Retry in ") + tsNextRPC.Format();
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


const char *BOINC_RCSID_b4edf777fc = "$Id$";
