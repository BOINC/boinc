// $Id$
//
// The contents of this file are subject to the BOINC Public License
// Version 1.0 (the "License"); you may not use this file except in
// compliance with the License. You may obtain a copy of the License at
// http://boinc.berkeley.edu/license_1.0.txt
// 
// Software distributed under the License is distributed on an "AS IS"
// basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the
// License for the specific language governing rights and limitations
// under the License. 
// 
// The Original Code is the Berkeley Open Infrastructure for Network Computing. 
// 
// The Initial Developer of the Original Code is the SETI@home project.
// Portions created by the SETI@home project are Copyright (C) 2002
// University of California at Berkeley. All Rights Reserved. 
// 
// Contributor(s):
//
// Revision History:
//
// $Log$
// Revision 1.1  2004/09/21 01:26:25  rwalton
// *** empty log message ***
//
//

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma implementation "ViewProjects.h"
#endif

#include "stdwx.h"
#include "BOINCGUIApp.h"
#include "MainDocument.h"
#include "ViewProjects.h"
#include "Events.h"

#include "res/proj.xpm"
#include "res/task.xpm"
#include "res/web.xpm"
#include "res/tips.xpm"


#define COLUMN_PROJECT              0
#define COLUMN_ACCOUNTNAME          1
#define COLUMN_TEAMNAME             2
#define COLUMN_TOTALCREDIT          3
#define COLUMN_AVGCREDIT            4
#define COLUMN_RESOURCESHARE        5


IMPLEMENT_DYNAMIC_CLASS(CViewProjects, CBOINCBaseView)

BEGIN_EVENT_TABLE(CViewProjects, CBOINCBaseView)
    EVT_LIST_CACHE_HINT(ID_LIST_PROJECTSVIEW, CViewProjects::OnCacheHint)
END_EVENT_TABLE()


CViewProjects::CViewProjects()
{
    wxLogTrace("CViewProjects::CViewProjects - Function Begining");
    wxLogTrace("CViewProjects::CViewProjects - Function Ending");
}


CViewProjects::CViewProjects(wxNotebook* pNotebook) :
    CBOINCBaseView(pNotebook, ID_HTML_PROJECTSVIEW, ID_LIST_PROJECTSVIEW)
{
    m_bProcessingRenderEvent = false;

    wxASSERT(NULL != m_pTaskPane);
    wxASSERT(NULL != m_pListPane);

    wxBitmap bmpProject(proj_xpm);
    wxBitmap bmpTask(task_xpm);
    wxBitmap bmpWeb(web_xpm);
    wxBitmap bmpTips(tips_xpm);

    bmpProject.SetMask(new wxMask(bmpProject, wxColour(255, 0, 255)));

    m_pTaskPane->AddVirtualFile(_T("proj.xpm"), bmpProject, wxBITMAP_TYPE_XPM);

    m_pTaskPane->CreateTaskHeader(_T("projheader.xpm"), bmpTask, _("Tasks"));
    m_pTaskPane->CreateTaskHeader(_T("projweb.xpm"), bmpWeb, _("Websites"));
    m_pTaskPane->CreateTaskHeader(_T("projtips.xpm"), bmpTips, _("Quick Tips"));

    m_pListPane->InsertColumn(COLUMN_PROJECT, _("Project"), wxLIST_FORMAT_LEFT, -1);
    m_pListPane->InsertColumn(COLUMN_ACCOUNTNAME, _("Account"), wxLIST_FORMAT_LEFT, -1);
    m_pListPane->InsertColumn(COLUMN_TEAMNAME, _("Team"), wxLIST_FORMAT_LEFT, -1);
    m_pListPane->InsertColumn(COLUMN_TOTALCREDIT, _("Total Credit"), wxLIST_FORMAT_LEFT, -1);
    m_pListPane->InsertColumn(COLUMN_AVGCREDIT, _("Avg. Credit"), wxLIST_FORMAT_LEFT, -1);
    m_pListPane->InsertColumn(COLUMN_RESOURCESHARE, _("Resource Share"), wxLIST_FORMAT_LEFT, -1);

    UpdateTaskPane();
}


CViewProjects::~CViewProjects()
{
    wxLogTrace("CViewProjects::~CViewProjects - Function Begining");
    wxLogTrace("CViewProjects::~CViewProjects - Function Ending");
}


wxString CViewProjects::GetViewName()
{
    return wxString(_("Projects"));
}


char** CViewProjects::GetViewIcon()
{
    return proj_xpm;
}


void CViewProjects::UpdateTaskPane()
{
    wxLogTrace("CViewProjects::UpdateTaskPane - Function Begining");

    wxASSERT(NULL != m_pTaskPane);

    m_pTaskPane->BeginTaskPage();

    m_pTaskPane->BeginTaskSection( _T("projheader.xpm"), false );
    m_pTaskPane->CreateTask(_T("proj.xpm"), 
                            _("Attach to Project"), 
                            _(""));
    m_pTaskPane->CreateTask(_T("proj.xpm"), 
                            _("Detach from Project"), 
                            _(""));
    m_pTaskPane->CreateTask(_T("proj.xpm"), 
                            _("Update Project"), 
                            _(""));
    m_pTaskPane->CreateTask(_T("proj.xpm"), 
                            _("Reset Project"), 
                            _(""));
    m_pTaskPane->EndTaskSection();

    m_pTaskPane->BeginTaskSection( _T("projweb.xpm"), false );
    m_pTaskPane->CreateTask(_T("proj.xpm"), 
                            _("BOINC"), 
                            _(""));
    m_pTaskPane->CreateTask(_T("proj.xpm"), 
                            _("FAQ"),
                            _(""));
    m_pTaskPane->CreateTask(_T("proj.xpm"), 
                            _("Project"), 
                            _(""));
    m_pTaskPane->CreateTask(_T("proj.xpm"), 
                            _("Team"), 
                            _(""));
    m_pTaskPane->CreateTask(_T("proj.xpm"), 
                            _("User"), 
                            _(""));
    m_pTaskPane->EndTaskSection();

    m_pTaskPane->BeginTaskSection( _T("projtips.xpm"), false );
    m_pTaskPane->CreateQuickTip(_("Please select a project to see additional options."));
    m_pTaskPane->EndTaskSection();

    m_pTaskPane->EndTaskPage();

    wxLogTrace("CViewProjects::UpdateTaskPane - Function Ending");
}


void CViewProjects::OnRender(wxTimerEvent &event)
{
    wxLogTrace("CViewProjects::OnRender - Processing Render Event...");
    if (!m_bProcessingRenderEvent)
    {
        m_bProcessingRenderEvent = true;

        wxInt32 iProjectCount = wxGetApp().GetDocument()->GetProjectCount();
        wxASSERT(NULL != m_pListPane);
        m_pListPane->SetItemCount(iProjectCount);

        m_bProcessingRenderEvent = false;
    }
    else
    {
        event.Skip();
    }
}


wxString CViewProjects::OnGetItemText(long item, long column) const {
    wxString strBuffer;
    switch(column) {
        case COLUMN_PROJECT:
            if (item == m_iCacheFrom) wxGetApp().GetDocument()->CachedStateLock();
            strBuffer = wxGetApp().GetDocument()->GetProjectProjectName(item);
            break;
        case COLUMN_ACCOUNTNAME:
            strBuffer = wxGetApp().GetDocument()->GetProjectAccountName(item);
            break;
        case COLUMN_TEAMNAME:
            strBuffer = wxGetApp().GetDocument()->GetProjectTeamName(item);
            break;
        case COLUMN_TOTALCREDIT:
            strBuffer = wxGetApp().GetDocument()->GetProjectTotalCredit(item);
            break;
        case COLUMN_AVGCREDIT:
            strBuffer = wxGetApp().GetDocument()->GetProjectAvgCredit(item);
            break;
        case COLUMN_RESOURCESHARE:
            strBuffer = wxGetApp().GetDocument()->GetProjectResourceShare(item);
            if (item == m_iCacheTo) wxGetApp().GetDocument()->CachedStateUnlock();
            break;
    }
    return strBuffer;
}

