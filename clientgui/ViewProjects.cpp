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
// Revision 1.4  2004/09/24 02:01:51  rwalton
// *** empty log message ***
//
// Revision 1.3  2004/09/23 08:28:50  rwalton
// *** empty log message ***
//
// Revision 1.2  2004/09/22 21:53:03  rwalton
// *** empty log message ***
//
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
#include "DlgAttachProject.h"
#include "Events.h"

#include "res/boinc.xpm"
#include "res/proj.xpm"
#include "res/task.xpm"
#include "res/web.xpm"
#include "res/tips.xpm"


#define VIEW_HEADER                 "proj"

#define SECTION_TASK                VIEW_HEADER "task"
#define SECTION_WEB                 VIEW_HEADER "web"
#define SECTION_TIPS                VIEW_HEADER "tips"

#define BITMAP_PROJECTS             VIEW_HEADER ".xpm"
#define BITMAP_TASKHEADER           SECTION_TASK ".xpm"
#define BITMAP_WEBHEADER            SECTION_WEB ".xpm"
#define BITMAP_TIPSHEADER           SECTION_TIPS ".xpm"
#define BITMAP_BOINC                "boinc.xpm"

#define LINK_TASKATTACH             SECTION_TASK "attach"
#define LINK_TASKDETACH             SECTION_TASK "detach"
#define LINK_TASKUPDATE             SECTION_TASK "update"
#define LINK_TASKRESET              SECTION_TASK "reset"

#define LINK_WEBBOINC               SECTION_WEB "boinc"
#define LINK_WEBFAQ                 SECTION_WEB "faq"
#define LINK_WEBPROJECT             SECTION_WEB "project"
#define LINK_WEBTEAM                SECTION_WEB "team"
#define LINK_WEBUSER                SECTION_WEB "user"

#define LINK_DEFAULT                "default"
#define LINK_BLANK                  "blank"

#define COLUMN_PROJECT              0
#define COLUMN_ACCOUNTNAME          1
#define COLUMN_TEAMNAME             2
#define COLUMN_TOTALCREDIT          3
#define COLUMN_AVGCREDIT            4
#define COLUMN_RESOURCESHARE        5


IMPLEMENT_DYNAMIC_CLASS(CViewProjects, CBOINCBaseView)


CViewProjects::CViewProjects()
{
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
    wxBitmap bmpBOINC(boinc_xpm);

    bmpProject.SetMask(new wxMask(bmpProject, wxColour(255, 0, 255)));
    bmpTask.SetMask(new wxMask(bmpTask, wxColour(255, 0, 255)));
    bmpWeb.SetMask(new wxMask(bmpWeb, wxColour(255, 0, 255)));
    bmpTips.SetMask(new wxMask(bmpTips, wxColour(255, 0, 255)));
    bmpBOINC.SetMask(new wxMask(bmpBOINC, wxColour(255, 0, 255)));

    m_pTaskPane->AddVirtualFile(wxT(BITMAP_PROJECTS), bmpProject, wxBITMAP_TYPE_XPM);
    m_pTaskPane->AddVirtualFile(wxT(BITMAP_BOINC), bmpBOINC, wxBITMAP_TYPE_XPM);

    m_pTaskPane->CreateTaskHeader(wxT(BITMAP_TASKHEADER), bmpTask, _("Tasks"));
    m_pTaskPane->CreateTaskHeader(wxT(BITMAP_WEBHEADER), bmpWeb, _("Websites"));
    m_pTaskPane->CreateTaskHeader(wxT(BITMAP_TIPSHEADER), bmpTips, _("Quick Tips"));

    m_pListPane->InsertColumn(COLUMN_PROJECT, _("Project"), wxLIST_FORMAT_LEFT, -1);
    m_pListPane->InsertColumn(COLUMN_ACCOUNTNAME, _("Account"), wxLIST_FORMAT_LEFT, -1);
    m_pListPane->InsertColumn(COLUMN_TEAMNAME, _("Team"), wxLIST_FORMAT_LEFT, -1);
    m_pListPane->InsertColumn(COLUMN_TOTALCREDIT, _("Total Credit"), wxLIST_FORMAT_LEFT, -1);
    m_pListPane->InsertColumn(COLUMN_AVGCREDIT, _("Avg. Credit"), wxLIST_FORMAT_LEFT, -1);
    m_pListPane->InsertColumn(COLUMN_RESOURCESHARE, _("Resource Share"), wxLIST_FORMAT_LEFT, -1);

    m_bTipsHeaderHidden = false;

    SetCurrentQuickTip(
        wxT(LINK_DEFAULT), 
        _("Please select a project to see additional options.")
    );

    UpdateSelection();
}


CViewProjects::~CViewProjects()
{
}


wxString CViewProjects::GetViewName()
{
    return wxString(_("Projects"));
}


char** CViewProjects::GetViewIcon()
{
    return proj_xpm;
}


void CViewProjects::OnRender(wxTimerEvent &event)
{
    wxASSERT(NULL != m_pTaskPane);
    wxASSERT(NULL != m_pListPane);

    wxLogTrace(wxT("CViewProjects::OnRender - Processing Render Event..."));
    if (!m_bProcessingRenderEvent)
    {
        m_bProcessingRenderEvent = true;

        wxInt32 iProjectCount = wxGetApp().GetDocument()->GetProjectCount();
        m_pListPane->SetItemCount(iProjectCount);

        long lSelected = m_pListPane->GetFirstSelected();
        if ( (-1 == lSelected) && m_bItemSelected )
        {
            UpdateSelection();
        }

        m_bProcessingRenderEvent = false;
    }
    else
    {
        event.Skip();
    }
}


void CViewProjects::OnListSelected ( wxListEvent& event )
{
    wxLogTrace("CViewProjects::OnListSelected - Processing Event...");
    UpdateSelection();
    event.Skip();
}


void CViewProjects::OnListDeselected ( wxListEvent& event )
{
    wxLogTrace("CViewProjects::OnListDeselected - Processing Event...");
    UpdateSelection();
    event.Skip();
}


void CViewProjects::OnListActivated ( wxListEvent& event )
{
    wxLogTrace("CViewProjects::OnListActivated - Processing Event...");
    UpdateSelection();
    event.Skip();
}


void CViewProjects::OnListFocused ( wxListEvent& event )
{
    wxLogTrace("CViewProjects::OnListFocused - Processing Event...");
    UpdateSelection();
    event.Skip();
}


wxString CViewProjects::OnListGetItemText(long item, long column) const {
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


void CViewProjects::OnTaskLinkClicked( const wxHtmlLinkInfo& link )
{
    wxASSERT(NULL != m_pTaskPane);
    wxASSERT(NULL != m_pListPane);

    wxString strMessage;

    if ( link.GetHref() == wxT(SECTION_TASK) )
        m_bTaskHeaderHidden ? m_bTaskHeaderHidden = false : m_bTaskHeaderHidden = true;


    if ( link.GetHref() == wxT(LINK_TASKATTACH) )
    {
        CDlgAttachProject* pDlg = new CDlgAttachProject(this);
        wxASSERT(NULL != pDlg);

        pDlg->ShowModal();

        if (pDlg)
            pDlg->Destroy();
    }

    if ( link.GetHref() == wxT(LINK_TASKDETACH) )
    {
        strMessage.Printf(
            _("Are you sure you wish to detach from project '%s'?"), 
            wxGetApp().GetDocument()->GetProjectProjectName(m_pListPane->GetFirstSelected()).c_str());

        int iAnswer = wxMessageBox(
            strMessage,
            _("Detach from Project"),
            wxYES_NO | wxICON_QUESTION, 
            this
        );

        if ( wxYES == iAnswer )
        {

        }
    }

    if ( link.GetHref() == wxT(LINK_TASKUPDATE) )
    {
        
    }

    if ( link.GetHref() == wxT(LINK_TASKRESET) )
    {
        strMessage.Printf(
            _("Are you sure you wish to reset project '%s'?"), 
            wxGetApp().GetDocument()->GetProjectProjectName(m_pListPane->GetFirstSelected()).c_str());

        int iAnswer = wxMessageBox(
            strMessage,
            _("Reset Project"),
            wxYES_NO | wxICON_QUESTION, 
            this
        );

        if ( wxYES == iAnswer )
        {

        }
    }


    if ( link.GetHref() == wxT(SECTION_WEB) )
        m_bWebsiteHeaderHidden ? m_bWebsiteHeaderHidden = false : m_bWebsiteHeaderHidden = true;

    if ( link.GetHref() == wxT(SECTION_TIPS) )
        m_bTipsHeaderHidden ? m_bTipsHeaderHidden = false : m_bTipsHeaderHidden = true;


    UpdateTaskPane();
}


void CViewProjects::OnTaskCellMouseHover( wxHtmlCell* cell, wxCoord x, wxCoord y )
{
    if ( NULL != cell->GetLink() )
    {
        bool        bUpdateTaskPane = false;
        wxString    strLink;

        strLink = cell->GetLink()->GetHref();

        if      ( wxT(LINK_TASKATTACH) == strLink )
        {
            if  ( wxT(LINK_TASKATTACH) != GetCurrentQuickTip() )
            {
                SetCurrentQuickTip(
                    wxT(LINK_TASKATTACH), 
                    _("<b>Attach to Project</b><br>"
                    "Selecting attach to project allows you to join other BOINC "
                    "projects.  You will need a valid project URL and Authenticator.")
                );

                bUpdateTaskPane = true;
            }
        }
        else if ( wxT(LINK_TASKDETACH) == strLink )
        {
            if  ( wxT(LINK_TASKDETACH) != GetCurrentQuickTip() )
            {
                SetCurrentQuickTip(
                    wxT(LINK_TASKDETACH), 
                    _("<b>Detach from Project</b><br>"
                    "Selecting detach from project removes the computer from the currently "
                    "selected project.  You may wish to update the project first to submit "
                    "any completed work.")
                );

                bUpdateTaskPane = true;
            }
        }
        else if ( wxT(LINK_TASKUPDATE) == strLink )
        {
            if  ( wxT(LINK_TASKUPDATE) != GetCurrentQuickTip() )
            {
                SetCurrentQuickTip(
                    wxT(LINK_TASKUPDATE), 
                    _("<b>Update Project</b><br>"
                    "Selecting update project submits any outstanding work and refreshes "
                    "your credit and preferences for the currently selected project.")
                );

                bUpdateTaskPane = true;
            }
        }
        else if ( wxT(LINK_TASKRESET) == strLink )
        {
            if  ( wxT(LINK_TASKRESET) != GetCurrentQuickTip() )
            {
                SetCurrentQuickTip(
                    wxT(LINK_TASKRESET), 
                    _("<b>Reset Project</b><br>"
                    "Selecting reset project removes all workunits and applications from "
                    "the currently selected project.  You may wish to update the project "
                    "first to submit any completed work.")
                );

                bUpdateTaskPane = true;
            }
        }
        else if ( wxT(LINK_WEBBOINC) == strLink )
        {
            if  ( wxT(LINK_WEBBOINC) != GetCurrentQuickTip() )
            {
                SetCurrentQuickTip(
                    wxT(LINK_WEBBOINC), 
                    _("<b>BOINC Homepage</b><br>"
                    "This will open a browser window to the BOINC homepage.")
                );

                bUpdateTaskPane = true;
            }
        }
        else if ( wxT(LINK_WEBFAQ) == strLink )
        {
            if  ( wxT(LINK_WEBFAQ) != GetCurrentQuickTip() )
            {
                SetCurrentQuickTip(
                    wxT(LINK_WEBFAQ), 
                    _("<b>FAQ</b><br>"
                    "This will open a browser window to the BOINC FAQ.")
                );

                bUpdateTaskPane = true;
            }
        }
        else if ( wxT(LINK_WEBPROJECT) == strLink )
        {
            if  ( wxT(LINK_WEBPROJECT) != GetCurrentQuickTip() )
            {
                SetCurrentQuickTip(
                    wxT(LINK_WEBPROJECT), 
                    _("<b>Project Homepage</b><br>"
                    "This will open a browser window to the currently selected project "
                    "homepage.")
                );

                bUpdateTaskPane = true;
            }
        }
        else if ( wxT(LINK_WEBTEAM) == strLink )
        {
            if  ( wxT(LINK_WEBTEAM) != GetCurrentQuickTip() )
            {
                SetCurrentQuickTip(
                    wxT(LINK_WEBTEAM), 
                    _("<b>Team Homepage</b><br>"
                    "This will open a browser window to your team homepage for the currently "
                    "selected project.")
                );

                bUpdateTaskPane = true;
            }
        }
        else if ( wxT(LINK_WEBUSER) == strLink )
        {
            if  ( wxT(LINK_WEBUSER) != GetCurrentQuickTip() )
            {
                SetCurrentQuickTip(
                    wxT(LINK_WEBTEAM), 
                    _("<b>Your Homepage</b><br>"
                    "This will open a browser window to your homepage for the currently "
                    "selected project.")
                );

                bUpdateTaskPane = true;
            }
        }
        else
        {
            long lSelected = m_pListPane->GetFirstSelected();
            if ( -1 == lSelected )
            {
                if  ( wxT(LINK_DEFAULT) != GetCurrentQuickTip() )
                {
                    SetCurrentQuickTip(
                        wxT(LINK_DEFAULT), 
                        _("Please select a project to see additional options.")
                    );

                    bUpdateTaskPane = true;
                }
            }
            else
            {
                if  ( wxT(LINK_BLANK) != GetCurrentQuickTip() )
                {
                    SetCurrentQuickTip(
                        wxT(LINK_BLANK), 
                        wxT("")
                    );

                    bUpdateTaskPane = true;
                }
            }
        }

        if ( bUpdateTaskPane )
        {
            UpdateTaskPane();
        }
    }
}


void CViewProjects::UpdateSelection()
{
    wxASSERT(NULL != m_pTaskPane);
    wxASSERT(NULL != m_pListPane);

    long lSelected = m_pListPane->GetFirstSelected();
    if ( -1 == lSelected )
    {
        m_bTaskHeaderHidden = false;
        m_bTaskAttachToProjectHidden = false;
        m_bTaskDetachFromProjectHidden = true;
        m_bTaskUpdateProjectHidden = true;
        m_bTaskResetProjectHidden = true;

        m_bWebsiteHeaderHidden = false;
        m_bWebsiteBOINCHidden = false;
        m_bWebsiteFAQHidden = false;
        m_bWebsiteProjectHidden = true;
        m_bWebsiteTeamHidden = true;
        m_bWebsiteUserHidden = true;

        m_bItemSelected = false;
    }
    else
    {
        m_bTaskHeaderHidden = false;
        m_bTaskAttachToProjectHidden = false;
        m_bTaskDetachFromProjectHidden = false;
        m_bTaskUpdateProjectHidden = false;
        m_bTaskResetProjectHidden = false;

        m_bWebsiteHeaderHidden = false;
        m_bWebsiteBOINCHidden = false;
        m_bWebsiteFAQHidden = false;
        m_bWebsiteProjectHidden = false;
        m_bWebsiteTeamHidden = false;
        m_bWebsiteUserHidden = false;

        m_bItemSelected = true;
    }
    UpdateTaskPane();
}


void CViewProjects::UpdateTaskPane()
{
    wxASSERT(NULL != m_pTaskPane);

    m_pTaskPane->BeginTaskPage();

    m_pTaskPane->BeginTaskSection( wxT(SECTION_TASK), wxT(BITMAP_TASKHEADER), m_bTaskHeaderHidden );
    if (!m_bTaskHeaderHidden)
    {
        m_pTaskPane->CreateTask( wxT(LINK_TASKATTACH), wxT(BITMAP_PROJECTS), _("Attach to Project"), m_bTaskAttachToProjectHidden );
        m_pTaskPane->CreateTask( wxT(LINK_TASKDETACH), wxT(BITMAP_PROJECTS), _("Detach from Project"), m_bTaskDetachFromProjectHidden );
        m_pTaskPane->CreateTask( wxT(LINK_TASKUPDATE), wxT(BITMAP_PROJECTS), _("Update Project"), m_bTaskUpdateProjectHidden );
        m_pTaskPane->CreateTask( wxT(LINK_TASKRESET), wxT(BITMAP_PROJECTS), _("Reset Project"), m_bTaskResetProjectHidden );
    }
    m_pTaskPane->EndTaskSection( m_bTaskHeaderHidden );


    m_pTaskPane->BeginTaskSection( wxT(SECTION_WEB), wxT(BITMAP_WEBHEADER), m_bWebsiteHeaderHidden );
    if (!m_bWebsiteHeaderHidden)
    {
        m_pTaskPane->CreateTask( wxT(LINK_WEBBOINC), wxT(BITMAP_BOINC), _("BOINC"), m_bWebsiteBOINCHidden );
        m_pTaskPane->CreateTask( wxT(LINK_WEBFAQ), wxT(BITMAP_BOINC), _("FAQ"), m_bWebsiteFAQHidden );
        m_pTaskPane->CreateTask( wxT(LINK_WEBPROJECT), wxT(BITMAP_PROJECTS), _("Project"), m_bWebsiteProjectHidden );
        m_pTaskPane->CreateTask( wxT(LINK_WEBTEAM), wxT(BITMAP_PROJECTS), _("Team"), m_bWebsiteTeamHidden );
        m_pTaskPane->CreateTask( wxT(LINK_WEBUSER), wxT(BITMAP_PROJECTS), _("User"), m_bWebsiteUserHidden );
    }
    m_pTaskPane->EndTaskSection(m_bWebsiteHeaderHidden);

    m_pTaskPane->UpdateQuickTip(wxT(SECTION_TIPS), wxT(BITMAP_TIPSHEADER), GetCurrentQuickTipText(), m_bTipsHeaderHidden);

    m_pTaskPane->EndTaskPage();
}

