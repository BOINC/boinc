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
// Revision 1.9  2004/10/05 02:55:26  rwalton
// *** empty log message ***
//
// Revision 1.8  2004/09/29 22:20:43  rwalton
// *** empty log message ***
//
// Revision 1.7  2004/09/28 01:19:46  rwalton
// *** empty log message ***
//
// Revision 1.6  2004/09/25 21:33:23  rwalton
// *** empty log message ***
//
// Revision 1.5  2004/09/24 22:18:58  rwalton
// *** empty log message ***
//
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
#include "BOINCTaskCtrl.h"
#include "BOINCListCtrl.h"
#include "ViewProjects.h"
#include "DlgAttachProject.h"
#include "Events.h"

#include "res/boinc.xpm"
#include "res/proj.xpm"
#include "res/task.xpm"
#include "res/web.xpm"
#include "res/tips.xpm"


#define VIEW_HEADER                 wxT("proj")

#define SECTION_TASK                wxT(VIEW_HEADER "task")
#define SECTION_WEB                 wxT(VIEW_HEADER "web")
#define SECTION_TIPS                wxT(VIEW_HEADER "tips")

#define BITMAP_PROJECTS             wxT(VIEW_HEADER ".xpm")
#define BITMAP_TASKHEADER           wxT(SECTION_TASK ".xpm")
#define BITMAP_WEBHEADER            wxT(SECTION_WEB ".xpm")
#define BITMAP_TIPSHEADER           wxT(SECTION_TIPS ".xpm")
#define BITMAP_BOINC                wxT("boinc.xpm")

#define LINK_DEFAULT                wxT("default")

#define COLUMN_PROJECT              0
#define COLUMN_ACCOUNTNAME          1
#define COLUMN_TEAMNAME             2
#define COLUMN_TOTALCREDIT          3
#define COLUMN_AVGCREDIT            4
#define COLUMN_RESOURCESHARE        5
#define COLUMN_STATUS               6


const wxString LINK_TASKATTACH      = wxT(SECTION_TASK "attach");
const wxString LINKDESC_TASKATTACH  = 
     _("<b>Attach to Project</b><br>"
       "Selecting attach to project allows you to join other BOINC "
       "projects.  You will need a valid project URL and Authenticator.");


const wxString LINK_TASKDETACH      = wxT(SECTION_TASK "detach");
const wxString LINKDESC_TASKDETACH  = 
     _("<b>Detach from Project</b><br>"
       "Selecting detach from project removes the computer from the currently "
       "selected project.  You may wish to update the project first to submit "
       "any completed work.");

const wxString LINK_TASKRESET       = wxT(SECTION_TASK "reset");
const wxString LINKDESC_TASKRESET   = 
     _("<b>Reset Project</b><br>"
       "Selecting reset project removes all workunits and applications from "
       "the currently selected project.  You may wish to update the project "
       "first to submit any completed work.");

const wxString LINK_TASKSUSPEND     = wxT(SECTION_TASK "suspend");
const wxString LINKDESC_TASKSUSPEND = 
     _("<b>Suspend Project</b><br>"
       "Selecting suspend project will pause the project from any additional "
       "computation for that project until the resume project option is selected.");

const wxString LINK_TASKRESUME      = wxT(SECTION_TASK "resume");
const wxString LINKDESC_TASKRESUME  = 
     _("<b>Resume Project</b><br>"
       "Selecting resume project resumes computation for a project that has been"
       "previously suspended.");

const wxString LINK_TASKUPDATE      = wxT(SECTION_TASK "update");
const wxString LINKDESC_TASKUPDATE  = 
     _("<b>Update Project</b><br>"
       "Selecting update project submits any outstanding work and refreshes "
       "your credit and preferences for the currently selected project.");

const wxString LINK_WEBBOINC        = wxT(SECTION_WEB "boinc");
const wxString LINKDESC_WEBBOINC    = 
     _("<b>BOINC Homepage</b><br>"
       "This will open a browser window to the BOINC homepage.");

const wxString LINK_WEBPROJECT      = wxT(SECTION_WEB "project");
const wxString LINKDESC_WEBPROJECT  = 
     _("<b>Project Homepage</b><br>"
       "This will open a browser window to the currently selected project "
       "homepage.");


IMPLEMENT_DYNAMIC_CLASS(CViewProjects, CBOINCBaseView)


CViewProjects::CViewProjects()
{
}


CViewProjects::CViewProjects(wxNotebook* pNotebook) :
    CBOINCBaseView(pNotebook, ID_HTML_PROJECTSVIEW, ID_LIST_PROJECTSVIEW)
{
    m_bProcessingTaskRenderEvent = false;
    m_bProcessingListRenderEvent = false;

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

    m_pTaskPane->AddVirtualFile(BITMAP_PROJECTS, bmpProject, wxBITMAP_TYPE_XPM);
    m_pTaskPane->AddVirtualFile(BITMAP_BOINC, bmpBOINC, wxBITMAP_TYPE_XPM);

    m_pTaskPane->CreateTaskHeader(BITMAP_TASKHEADER, bmpTask, _("Tasks"));
    m_pTaskPane->CreateTaskHeader(BITMAP_WEBHEADER, bmpWeb, _("Websites"));
    m_pTaskPane->CreateTaskHeader(BITMAP_TIPSHEADER, bmpTips, _("Quick Tips"));

    m_pListPane->InsertColumn(COLUMN_PROJECT, _("Project"), wxLIST_FORMAT_LEFT, -1);
    m_pListPane->InsertColumn(COLUMN_ACCOUNTNAME, _("Account"), wxLIST_FORMAT_LEFT, -1);
    m_pListPane->InsertColumn(COLUMN_TEAMNAME, _("Team"), wxLIST_FORMAT_LEFT, -1);
    m_pListPane->InsertColumn(COLUMN_TOTALCREDIT, _("Total Credit"), wxLIST_FORMAT_LEFT, -1);
    m_pListPane->InsertColumn(COLUMN_AVGCREDIT, _("Avg. Credit"), wxLIST_FORMAT_LEFT, -1);
    m_pListPane->InsertColumn(COLUMN_RESOURCESHARE, _("Resource Share"), wxLIST_FORMAT_LEFT, -1);
    m_pListPane->InsertColumn(COLUMN_STATUS, _("Status"), wxLIST_FORMAT_LEFT, -1);

    m_bTipsHeaderHidden = false;

    SetCurrentQuickTip(
        LINK_DEFAULT, 
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


void CViewProjects::OnTaskRender(wxTimerEvent &event)
{
    if (!m_bProcessingTaskRenderEvent)
    {
        m_bProcessingTaskRenderEvent = true;

        wxASSERT(NULL != m_pListPane);

        if ( ( 0 == m_pListPane->GetSelectedItemCount() ) && m_bItemSelected )
        {
            UpdateSelection();
        }

        m_bProcessingTaskRenderEvent = false;
    }
    else
    {
        event.Skip();
    }
}


void CViewProjects::OnListRender(wxTimerEvent &event)
{
    if (!m_bProcessingListRenderEvent)
    {
        m_bProcessingListRenderEvent = true;

        wxString strBuffer;

        wxASSERT(NULL != m_pListPane);

        wxInt32 iCount = wxGetApp().GetDocument()->GetProjectCount();
        if ( iCount != m_iCount )
        {
            m_iCount = iCount;
            m_pListPane->SetItemCount(iCount);
        }
        else
        {
            wxInt32         iRowIndex;
            wxInt32         iColumnIndex;
            wxInt32         iColumnTotal;
            wxString        strDocumentText;
            wxString        strListPaneText;
            wxListItem      liItem;
            bool            bNeedRefreshData;


            liItem.SetMask(wxLIST_MASK_TEXT);
            iColumnTotal = m_pListPane->GetColumnCount();

            for ( iRowIndex = m_iCacheFrom; iRowIndex <= m_iCacheTo; iRowIndex++ )
            {
                bNeedRefreshData = false;
                liItem.SetId(iRowIndex);

                for ( iColumnIndex = 0; iColumnIndex < iColumnTotal; iColumnIndex++ )
                {
                    strDocumentText.Clear();
                    strListPaneText.Clear();

                    switch(iColumnIndex)
                    {
                        case COLUMN_PROJECT:
                            FormatProjectName(iRowIndex, strDocumentText);
                            break;
                        case COLUMN_ACCOUNTNAME:
                            FormatAccountName(iRowIndex, strDocumentText);
                            break;
                        case COLUMN_TEAMNAME:
                            FormatTeamName(iRowIndex, strDocumentText);
                            break;
                        case COLUMN_TOTALCREDIT:
                            FormatTotalCredit(iRowIndex, strDocumentText);
                            break;
                        case COLUMN_AVGCREDIT:
                            FormatAVGCredit(iRowIndex, strDocumentText);
                            break;
                        case COLUMN_RESOURCESHARE:
                            FormatResourceShare(iRowIndex, strDocumentText);
                            break;
                        case COLUMN_STATUS:
                            FormatStatus(iRowIndex, strDocumentText);
                            break;
                    }

                    liItem.SetColumn(iColumnIndex);
                    m_pListPane->GetItem(liItem);
                    strListPaneText = liItem.GetText();

                    if ( !strBuffer.IsSameAs(strListPaneText) )
                        bNeedRefreshData = true;
                }

                if ( bNeedRefreshData )
                {
                    m_pListPane->RefreshItem( iRowIndex );
                }
            }
        }

        m_bProcessingListRenderEvent = false;
    }

    event.Skip();
}


void CViewProjects::OnListSelected ( wxListEvent& event )
{
    wxLogTrace("CViewProjects::OnListSelected - Event Processed");
    UpdateSelection();

    event.Skip();
}


void CViewProjects::OnListDeselected ( wxListEvent& event )
{
    wxLogTrace("CViewProjects::OnListDeselected - Event Processed");
    UpdateSelection();

    event.Skip();
}


wxString CViewProjects::OnListGetItemText(long item, long column) const 
{
    wxString strBuffer;

    switch(column)
    {
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


void CViewProjects::OnTaskLinkClicked( const wxHtmlLinkInfo& link )
{
    wxASSERT(NULL != m_pTaskPane);
    wxASSERT(NULL != m_pListPane);

    wxInt32  iAnswer; 
    wxInt32  iProject; 
    wxString strProjectName;
    wxString strProjectURL;
    wxString strMessage;


    if ( link.GetHref() == SECTION_TASK )
        m_bTaskHeaderHidden ? m_bTaskHeaderHidden = false : m_bTaskHeaderHidden = true;


    if ( link.GetHref() == LINK_TASKATTACH )
    {
        CDlgAttachProject* pDlg = new CDlgAttachProject(this);
        wxASSERT(NULL != pDlg);

        iAnswer = pDlg->ShowModal();
 
        if ( wxOK == iAnswer )
        {
            wxGetApp().GetDocument()->ProjectAttach(
                pDlg->m_strProjectAddress, 
                pDlg->m_strProjectAccountKey
            );
        }

        if (pDlg)
            pDlg->Destroy();
    }

    if ( link.GetHref() == LINK_TASKDETACH )
    {
        iProject = m_pListPane->GetFirstSelected();
        wxGetApp().GetDocument()->GetProjectProjectName(iProject, strProjectName);
        wxGetApp().GetDocument()->GetProjectProjectURL(iProject, strProjectURL);

        strMessage.Printf(
            _("Are you sure you wish to detach from project '%s'?"), 
            strProjectName.c_str());

        iAnswer = wxMessageBox(
            strMessage,
            _("Detach from Project"),
            wxYES_NO | wxICON_QUESTION, 
            this
        );

        if ( wxYES == iAnswer )
        {
            wxGetApp().GetDocument()->ProjectDetach(
                strProjectURL 
            );
        }
    }

    if ( link.GetHref() == LINK_TASKRESET )
    {
        iProject = m_pListPane->GetFirstSelected();
        wxGetApp().GetDocument()->GetProjectProjectName(iProject, strProjectName);
        wxGetApp().GetDocument()->GetProjectProjectURL(iProject, strProjectURL);

        strMessage.Printf(
            _("Are you sure you wish to reset project '%s'?"), 
            strProjectName.c_str());

        iAnswer = wxMessageBox(
            strMessage,
            _("Reset Project"),
            wxYES_NO | wxICON_QUESTION, 
            this
        );

        if ( wxYES == iAnswer )
        {
            wxGetApp().GetDocument()->ProjectReset(
                strProjectURL 
            );
        }
    }

    if ( link.GetHref() == LINK_TASKUPDATE )
    {
        iProject = m_pListPane->GetFirstSelected();
        wxGetApp().GetDocument()->GetProjectProjectURL(iProject, strProjectURL);

        wxGetApp().GetDocument()->ProjectUpdate(
            strProjectURL 
        );
    }

    if ( link.GetHref() == LINK_TASKSUSPEND )
    {
        iProject = m_pListPane->GetFirstSelected();
        wxGetApp().GetDocument()->GetProjectProjectURL(iProject, strProjectURL);

        wxGetApp().GetDocument()->ProjectSuspend(
            strProjectURL 
        );
    }

    if ( link.GetHref() == LINK_TASKRESUME )
    {
        iProject = m_pListPane->GetFirstSelected();
        wxGetApp().GetDocument()->GetProjectProjectURL(iProject, strProjectURL);

        wxGetApp().GetDocument()->ProjectResume(
            strProjectURL 
        );
    }


    if ( link.GetHref() == SECTION_WEB )
        m_bWebsiteHeaderHidden ? m_bWebsiteHeaderHidden = false : m_bWebsiteHeaderHidden = true;

    if ( link.GetHref() == SECTION_TIPS )
        m_bTipsHeaderHidden ? m_bTipsHeaderHidden = false : m_bTipsHeaderHidden = true;


    UpdateSelection();
}


void CViewProjects::OnTaskCellMouseHover( wxHtmlCell* cell, wxCoord x, wxCoord y )
{
    if ( NULL != cell->GetLink() )
    {
        bool        bUpdateSelection = false;
        wxString    strLink;

        strLink = cell->GetLink()->GetHref();

        if      ( UpdateQuickTip( strLink, LINK_TASKATTACH, LINKDESC_TASKATTACH ) )
            bUpdateSelection = true;
        else if ( UpdateQuickTip( strLink, LINK_TASKDETACH, LINKDESC_TASKDETACH ) )
            bUpdateSelection = true;
        else if ( UpdateQuickTip( strLink, LINK_TASKRESET, LINKDESC_TASKRESET ) )
            bUpdateSelection = true;
        else if ( UpdateQuickTip( strLink, LINK_TASKSUSPEND, LINKDESC_TASKSUSPEND ) )
            bUpdateSelection = true;
        else if ( UpdateQuickTip( strLink, LINK_TASKRESUME, LINKDESC_TASKRESUME ) )
            bUpdateSelection = true;
        else if ( UpdateQuickTip( strLink, LINK_TASKUPDATE, LINKDESC_TASKUPDATE ) )
            bUpdateSelection = true;
        else if ( UpdateQuickTip( strLink, LINK_WEBBOINC, LINKDESC_WEBBOINC ) )
            bUpdateSelection = true;
        else if ( UpdateQuickTip( strLink, LINK_WEBPROJECT, LINKDESC_WEBPROJECT ) )
            bUpdateSelection = true;
        else
        {
            if ( 0 == m_pListPane->GetSelectedItemCount() )
            {
                if  ( wxT(LINK_DEFAULT) != GetCurrentQuickTip() )
                {
                    SetCurrentQuickTip(
                        wxT(LINK_DEFAULT), 
                        _("Please select a project to see additional options.")
                    );

                    bUpdateSelection = true;
                }
            }
        }

        if ( bUpdateSelection )
        {
            UpdateSelection();
        }
    }
}


void CViewProjects::UpdateSelection()
{
    wxASSERT(NULL != m_pTaskPane);
    wxASSERT(NULL != m_pListPane);

    if ( 0 == m_pListPane->GetSelectedItemCount() )
    {
        m_bTaskHeaderHidden = false;
        m_bTaskAttachToProjectHidden = false;
        m_bTaskDetachFromProjectHidden = true;
        m_bTaskResetProjectHidden = true;
        m_bTaskSuspendProjectHidden = true;
        m_bTaskResumeProjectHidden = true;
        m_bTaskUpdateProjectHidden = true;

        m_bWebsiteHeaderHidden = false;
        m_bWebsiteBOINCHidden = false;
        m_bWebsiteProjectHidden = true;

        m_bItemSelected = false;
    }
    else
    {
        m_bTaskHeaderHidden = false;
        m_bTaskAttachToProjectHidden = false;
        m_bTaskDetachFromProjectHidden = false;
        m_bTaskResetProjectHidden = false;

        if ( wxGetApp().GetDocument()->IsProjectSuspended(m_pListPane->GetFirstSelected()) )
        {
            m_bTaskSuspendProjectHidden = true;
            m_bTaskResumeProjectHidden = false;
        }
        else
        {
            m_bTaskSuspendProjectHidden = false;
            m_bTaskResumeProjectHidden = true;
        }

        m_bTaskUpdateProjectHidden = false;

        m_bWebsiteHeaderHidden = false;
        m_bWebsiteBOINCHidden = false;
        m_bWebsiteProjectHidden = false;

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
        m_pTaskPane->CreateTask( wxT(LINK_TASKRESET), wxT(BITMAP_PROJECTS), _("Reset Project"), m_bTaskResetProjectHidden );
        m_pTaskPane->CreateTask( wxT(LINK_TASKSUSPEND), wxT(BITMAP_PROJECTS), _("Suspend Project"), m_bTaskSuspendProjectHidden );
        m_pTaskPane->CreateTask( wxT(LINK_TASKRESUME), wxT(BITMAP_PROJECTS), _("Resume Project"), m_bTaskResumeProjectHidden );
        m_pTaskPane->CreateTask( wxT(LINK_TASKUPDATE), wxT(BITMAP_PROJECTS), _("Update Project"), m_bTaskUpdateProjectHidden );
    }
    m_pTaskPane->EndTaskSection( m_bTaskHeaderHidden );


    m_pTaskPane->BeginTaskSection( wxT(SECTION_WEB), wxT(BITMAP_WEBHEADER), m_bWebsiteHeaderHidden );
    if (!m_bWebsiteHeaderHidden)
    {
        m_pTaskPane->CreateTask( wxT(LINK_WEBBOINC), wxT(BITMAP_BOINC), _("BOINC"), m_bWebsiteBOINCHidden );
        m_pTaskPane->CreateTask( wxT(LINK_WEBPROJECT), wxT(BITMAP_PROJECTS), _("Project"), m_bWebsiteProjectHidden );
    }
    m_pTaskPane->EndTaskSection(m_bWebsiteHeaderHidden);

    m_pTaskPane->UpdateQuickTip(wxT(SECTION_TIPS), wxT(BITMAP_TIPSHEADER), GetCurrentQuickTipText(), m_bTipsHeaderHidden);

    m_pTaskPane->EndTaskPage();
}


wxInt32 CViewProjects::FormatProjectName( wxInt32 item, wxString& strBuffer ) const
{
    strBuffer.Clear();

    wxGetApp().GetDocument()->GetProjectProjectName(item, strBuffer);

    return 0;
}


wxInt32 CViewProjects::FormatAccountName( wxInt32 item, wxString& strBuffer ) const
{
    strBuffer.Clear();

    wxGetApp().GetDocument()->GetProjectAccountName(item, strBuffer);

    return 0;
}


wxInt32 CViewProjects::FormatTeamName( wxInt32 item, wxString& strBuffer ) const
{
    strBuffer.Clear();

    wxGetApp().GetDocument()->GetProjectTeamName(item, strBuffer);

    return 0;
}


wxInt32 CViewProjects::FormatTotalCredit( wxInt32 item, wxString& strBuffer ) const
{
    float fBuffer;

    strBuffer.Clear();

    wxGetApp().GetDocument()->GetProjectTotalCredit(item, fBuffer);
    strBuffer.Printf(wxT("%0.2f"), fBuffer);

    return 0;
}


wxInt32 CViewProjects::FormatAVGCredit( wxInt32 item, wxString& strBuffer ) const
{
    float fBuffer;

    strBuffer.Clear();

    wxGetApp().GetDocument()->GetProjectAvgCredit(item, fBuffer);
    strBuffer.Printf(wxT("%0.2f"), fBuffer);

    return 0;
}


wxInt32 CViewProjects::FormatResourceShare( wxInt32 item, wxString& strBuffer ) const
{
    float fResourceShareBuffer;
    float fTotalResourceShareBuffer;

    strBuffer.Clear();

    wxGetApp().GetDocument()->GetProjectResourceShare(item, fResourceShareBuffer);
    wxGetApp().GetDocument()->GetProjectTotalResourceShare(item, fTotalResourceShareBuffer);
    strBuffer.Printf(wxT("%0.0f ( %0.2f%% )"), fResourceShareBuffer, ((fResourceShareBuffer / fTotalResourceShareBuffer) * 100));

    return 0;
}


wxInt32 CViewProjects::FormatStatus( wxInt32 item, wxString& strBuffer ) const
{
    wxInt32 iNextRPC;

    strBuffer.Clear();

    if      (wxGetApp().GetDocument()->IsProjectSuspended(item))
    {
        strBuffer = _("Project Suspended");
    } 
    else if (wxGetApp().GetDocument()->IsProjectRPCPending(item))
    {
        wxGetApp().GetDocument()->GetProjectMinRPCTime(item, iNextRPC);

        wxDateTime dtNextRPC((time_t)iNextRPC);
        wxDateTime dtNow(wxDateTime::Now());

        if (dtNextRPC > dtNow)
        {
            wxTimeSpan tsNextRPC(dtNextRPC - dtNow);
            strBuffer = _("Retry in ") + tsNextRPC.Format();
        }
    }

    return 0;
}

