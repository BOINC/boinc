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

#define COLUMN_PROJECT              0
#define COLUMN_ACCOUNTNAME          1
#define COLUMN_TEAMNAME             2
#define COLUMN_TOTALCREDIT          3
#define COLUMN_AVGCREDIT            4
#define COLUMN_RESOURCESHARE        5
#define COLUMN_STATUS               6


const wxString LINK_DEFAULT             = wxT("default");
const wxString LINKDESC_DEFAULT         = 
     _("Please click a project to see additional options.");

const wxString LINK_TASKATTACH      = wxT(SECTION_TASK "attach");
const wxString LINKDESC_TASKATTACH  = 
     _("<b>Attach to Project</b><br>"
       "Clicking attach to project allows you to join other BOINC "
       "projects.  You will need a valid project URL and Authenticator.");

const wxString LINK_TASKDETACH      = wxT(SECTION_TASK "detach");
const wxString LINKDESC_TASKDETACH  = 
     _("<b>Detach from Project</b><br>"
       "Clicking detach from project removes the computer from the currently "
       "selected project.  You may wish to update the project first to submit "
       "any completed work.");

const wxString LINK_TASKRESET       = wxT(SECTION_TASK "reset");
const wxString LINKDESC_TASKRESET   = 
     _("<b>Reset Project</b><br>"
       "Clicking reset project removes all workunits and applications from "
       "the currently selected project.  You may wish to update the project "
       "first to submit any completed work.");

const wxString LINK_TASKSUSPEND     = wxT(SECTION_TASK "suspend");
const wxString LINKDESC_TASKSUSPEND = 
     _("<b>Suspend Project</b><br>"
       "Clicking suspend project will pause the project from any additional "
       "computation for that project until the resume project option is selected.");

const wxString LINK_TASKRESUME      = wxT(SECTION_TASK "resume");
const wxString LINKDESC_TASKRESUME  = 
     _("<b>Resume Project</b><br>"
       "Clicking resume project resumes computation for a project that has been"
       "previously suspended.");

const wxString LINK_TASKUPDATE      = wxT(SECTION_TASK "update");
const wxString LINKDESC_TASKUPDATE  = 
     _("<b>Update Project</b><br>"
       "Clicking update project submits any outstanding work and refreshes "
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

const wxString LINK_WEB             = wxT(SECTION_WEB ":");


IMPLEMENT_DYNAMIC_CLASS(CViewProjects, CBOINCBaseView)


CViewProjects::CViewProjects()
{
}


CViewProjects::CViewProjects(wxNotebook* pNotebook) :
    CBOINCBaseView(pNotebook, ID_HTML_PROJECTSVIEW, ID_LIST_PROJECTSVIEW)
{
    m_bItemSelected = false;

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

    m_pListPane->InsertColumn(COLUMN_PROJECT, _("Project"), wxLIST_FORMAT_LEFT, 150);
    m_pListPane->InsertColumn(COLUMN_ACCOUNTNAME, _("Account"), wxLIST_FORMAT_LEFT, 80);
    m_pListPane->InsertColumn(COLUMN_TEAMNAME, _("Team"), wxLIST_FORMAT_LEFT, 80);
    m_pListPane->InsertColumn(COLUMN_TOTALCREDIT, _("Total Credit"), wxLIST_FORMAT_RIGHT, 80);
    m_pListPane->InsertColumn(COLUMN_AVGCREDIT, _("Avg. Credit"), wxLIST_FORMAT_RIGHT, 80);
    m_pListPane->InsertColumn(COLUMN_RESOURCESHARE, _("Resource Share"), wxLIST_FORMAT_CENTRE, 85);
    m_pListPane->InsertColumn(COLUMN_STATUS, _("Status"), wxLIST_FORMAT_LEFT, 150);

    m_bTipsHeaderHidden = false;

    SetCurrentQuickTip(
        LINK_DEFAULT, 
        LINKDESC_DEFAULT
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

        CMainDocument*  pDoc = wxGetApp().GetDocument();

        wxASSERT(NULL != pDoc);
        wxASSERT(wxDynamicCast(pDoc, CMainDocument));
        wxASSERT(NULL != m_pListPane);

        wxInt32 iCount = pDoc->GetProjectCount();
        if ( iCount != m_iCount )
        {
            m_iCount = iCount;
            if ( 0 <= iCount )
                m_pListPane->DeleteAllItems();
            else
                m_pListPane->SetItemCount(iCount);
        }
        else
        {
            if ( 1 <= m_iCacheTo )
            {
                wxInt32         iRowIndex        = 0;
                wxInt32         iColumnIndex     = 0;
                wxInt32         iColumnTotal     = 0;
                wxString        strDocumentText  = wxEmptyString;
                wxString        strListPaneText  = wxEmptyString;
                wxString        strBuffer        = wxEmptyString;
                bool            bNeedRefreshData = false;
                wxListItem      liItem;

                liItem.SetMask(wxLIST_MASK_TEXT);
                iColumnTotal = m_pListPane->GetColumnCount();

                for ( iRowIndex = m_iCacheFrom; iRowIndex <= m_iCacheTo; iRowIndex++ )
                {
                    bNeedRefreshData = false;
                    liItem.SetId(iRowIndex);

                    for ( iColumnIndex = 0; iColumnIndex < iColumnTotal; iColumnIndex++ )
                    {
                        strDocumentText.Empty();
                        strListPaneText.Empty();

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
        }

        m_bProcessingListRenderEvent = false;
    }

    m_pListPane->Refresh();

    event.Skip();
}


void CViewProjects::OnListSelected ( wxListEvent& event )
{
    SetCurrentQuickTip(
        LINK_DEFAULT, 
        wxT("")
    );

    UpdateSelection();
    event.Skip();
}


void CViewProjects::OnListDeselected ( wxListEvent& event )
{
    SetCurrentQuickTip(
        LINK_DEFAULT, 
        wxT("")
    );

    UpdateSelection();
    event.Skip();
}


wxString CViewProjects::OnListGetItemText(long item, long column) const 
{
    wxString       strBuffer = wxEmptyString;
    CMainDocument* pDoc      = wxGetApp().GetDocument();

    wxASSERT(NULL != pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));

    switch(column)
    {
        case COLUMN_PROJECT:
            if (item == m_iCacheFrom) pDoc->CachedStateLock();
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
            if (item == m_iCacheTo) pDoc->CachedStateUnlock();
            break;
    }

    return strBuffer;
}


void CViewProjects::OnTaskLinkClicked( const wxHtmlLinkInfo& link )
{
    wxInt32  iAnswer        = 0; 
    wxInt32  iProjectIndex  = 0; 
    wxInt32  iWebsiteIndex  = 0; 
    wxString strProjectName = wxEmptyString;
    wxString strURL         = wxEmptyString;
    wxString strMessage     = wxEmptyString;
    CMainDocument* pDoc     = wxGetApp().GetDocument();

    wxASSERT(NULL != pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));
    wxASSERT(NULL != m_pTaskPane);
    wxASSERT(NULL != m_pListPane);

    if      ( link.GetHref() == SECTION_TASK )
        m_bTaskHeaderHidden ? m_bTaskHeaderHidden = false : m_bTaskHeaderHidden = true;
    else if ( link.GetHref() == LINK_TASKATTACH )
    {
        CDlgAttachProject* pDlg = new CDlgAttachProject(this);
        wxASSERT(NULL != pDlg);

        iAnswer = pDlg->ShowModal();
 
        if ( wxOK == iAnswer )
        {
            pDoc->ProjectAttach(
                pDlg->m_strProjectAddress, 
                pDlg->m_strProjectAccountKey
            );
        }

        if (pDlg)
            pDlg->Destroy();
    }
    else if ( link.GetHref() == LINK_TASKDETACH )
    {
        iProjectIndex = m_pListPane->GetFirstSelected();
        pDoc->GetProjectProjectName(iProjectIndex, strProjectName);

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
            pDoc->ProjectDetach(
                iProjectIndex 
            );
        }
    }
    else if ( link.GetHref() == LINK_TASKRESET )
    {
        iProjectIndex = m_pListPane->GetFirstSelected();
        pDoc->GetProjectProjectName(iProjectIndex, strProjectName);

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
            pDoc->ProjectReset(
                iProjectIndex 
            );
        }
    }
    else if ( link.GetHref() == LINK_TASKUPDATE )
    {
        iProjectIndex = m_pListPane->GetFirstSelected();

        pDoc->ProjectUpdate(
            iProjectIndex 
        );
    }
    else if ( link.GetHref() == LINK_TASKSUSPEND )
    {
        iProjectIndex = m_pListPane->GetFirstSelected();

        pDoc->ProjectSuspend(
            iProjectIndex 
        );
    }
    else if ( link.GetHref() == LINK_TASKRESUME )
    {
        iProjectIndex = m_pListPane->GetFirstSelected();

        pDoc->ProjectResume(
            iProjectIndex 
        );
    }
    else if ( link.GetHref() == SECTION_WEB )
        m_bWebsiteHeaderHidden ? m_bWebsiteHeaderHidden = false : m_bWebsiteHeaderHidden = true;
    else if ( link.GetHref() == LINK_WEBBOINC )
    {
        ExecuteLink(wxT("http://boinc.berkeley.edu"));
    }
    else if ( link.GetHref() == LINK_WEBPROJECT )
    {
        iProjectIndex = m_pListPane->GetFirstSelected();
        pDoc->GetProjectProjectURL(iProjectIndex, strURL);

        ExecuteLink(strURL);
    }
    else if ( link.GetHref().StartsWith( LINK_WEB ) )
    {
        ConvertLinkToWebsiteIndex( link.GetHref(), iProjectIndex, iWebsiteIndex );
        pDoc->GetProjectWebsiteLink(iProjectIndex, iWebsiteIndex, strURL);

        ExecuteLink(strURL);
    }
    else if ( link.GetHref() == SECTION_TIPS )
        m_bTipsHeaderHidden ? m_bTipsHeaderHidden = false : m_bTipsHeaderHidden = true;

    UpdateSelection();
    m_pListPane->Refresh();
}


void CViewProjects::OnTaskCellMouseHover( wxHtmlCell* cell, wxCoord x, wxCoord y )
{
    if ( NULL != cell->GetLink() )
    {
        bool           bUpdateSelection     = false;
        wxString       strLink              = wxEmptyString;
        wxString       strWebsiteLink       = wxEmptyString;
        wxString       strWebsiteDescripton = wxEmptyString;
        wxInt32        iProjectIndex        = 0;
        wxInt32        iWebsiteIndex        = 0;
        CMainDocument* pDoc                 = wxGetApp().GetDocument();

        wxASSERT(NULL != pDoc);
        wxASSERT(wxDynamicCast(pDoc, CMainDocument));
        wxASSERT(NULL != m_pListPane);

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
        else if ( IsWebsiteLink( strLink ) )
        {
            ConvertLinkToWebsiteIndex( strLink, iProjectIndex, iWebsiteIndex );

            pDoc->GetProjectWebsiteDescription( iProjectIndex, iWebsiteIndex, strWebsiteDescripton );

            UpdateQuickTip( strLink, strLink, strWebsiteDescripton );

            bUpdateSelection = true;
        }
        else
        {
            if ( 0 == m_pListPane->GetSelectedItemCount() )
            {
                if  ( LINK_DEFAULT != GetCurrentQuickTip() )
                {
                    SetCurrentQuickTip(
                        LINK_DEFAULT, 
                        LINKDESC_DEFAULT
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
    CMainDocument* pDoc = wxGetApp().GetDocument();

    wxASSERT(NULL != pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));
    wxASSERT(NULL != m_pTaskPane);
    wxASSERT(NULL != m_pListPane);

    if ( 0 == m_pListPane->GetSelectedItemCount() )
    {
        m_bTaskHeaderHidden = false;
        m_bTaskAttachHidden = false;
        m_bTaskDetachHidden = true;
        m_bTaskResetHidden = true;
        m_bTaskSuspendHidden = true;
        m_bTaskResumeHidden = true;
        m_bTaskUpdateHidden = true;

        m_bWebsiteHeaderHidden = false;
        m_bWebsiteBOINCHidden = false;
        m_bWebsiteProjectHidden = true;

        if ( m_bItemSelected )
        {
            SetCurrentQuickTip(
                LINK_DEFAULT, 
                wxT("")
            );
        }
        m_bItemSelected = false;
    }
    else
    {
        m_bTaskHeaderHidden = false;
        m_bTaskAttachHidden = false;
        m_bTaskDetachHidden = false;
        m_bTaskResetHidden = false;

        if ( pDoc->IsProjectSuspended(m_pListPane->GetFirstSelected()) )
        {
            m_bTaskSuspendHidden = true;
            m_bTaskResumeHidden = false;
        }
        else
        {
            m_bTaskSuspendHidden = false;
            m_bTaskResumeHidden = true;
        }

        m_bTaskUpdateHidden = false;

        m_bWebsiteHeaderHidden = false;
        m_bWebsiteBOINCHidden = false;
        m_bWebsiteProjectHidden = false;

        m_bItemSelected = true;
    }
    UpdateTaskPane();
}


void CViewProjects::UpdateTaskPane()
{
    wxInt32  iProjectIndex  = 0;
    wxInt32  iWebsiteIndex  = 0;
    wxInt32  iWebsiteCount  = 0;
    wxString strWebsiteName = wxEmptyString;
    wxString strWebsiteLink = wxEmptyString;
    CMainDocument* pDoc = wxGetApp().GetDocument();

    wxASSERT(NULL != pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));
    wxASSERT(NULL != m_pTaskPane);

    m_pTaskPane->BeginTaskPage();

    m_pTaskPane->BeginTaskSection( SECTION_TASK, BITMAP_TASKHEADER, m_bTaskHeaderHidden );
    if (!m_bTaskHeaderHidden)
    {
        m_pTaskPane->CreateTask( LINK_TASKATTACH, BITMAP_PROJECTS, _("Attach to Project"), m_bTaskAttachHidden );
        m_pTaskPane->CreateTask( LINK_TASKDETACH, BITMAP_PROJECTS, _("Detach from Project"), m_bTaskDetachHidden );
        m_pTaskPane->CreateTask( LINK_TASKRESET, BITMAP_PROJECTS, _("Reset Project"), m_bTaskResetHidden );
        m_pTaskPane->CreateTask( LINK_TASKSUSPEND, BITMAP_PROJECTS, _("Suspend Project"), m_bTaskSuspendHidden );
        m_pTaskPane->CreateTask( LINK_TASKRESUME, BITMAP_PROJECTS, _("Resume Project"), m_bTaskResumeHidden );
        m_pTaskPane->CreateTask( LINK_TASKUPDATE, BITMAP_PROJECTS, _("Update Project"), m_bTaskUpdateHidden );
    }
    m_pTaskPane->EndTaskSection( m_bTaskHeaderHidden );


    m_pTaskPane->BeginTaskSection( wxT(SECTION_WEB), wxT(BITMAP_WEBHEADER), m_bWebsiteHeaderHidden );
    if (!m_bWebsiteHeaderHidden)
    {
        m_pTaskPane->CreateTask( LINK_WEBBOINC, BITMAP_BOINC, _("BOINC"), m_bWebsiteBOINCHidden );
        m_pTaskPane->CreateTask( LINK_WEBPROJECT, BITMAP_PROJECTS, _("Project"), m_bWebsiteProjectHidden );

        iProjectIndex = m_pListPane->GetFirstSelected();
        if ( -1 != iProjectIndex )
        {
            iWebsiteCount = pDoc->GetProjectWebsiteCount(iProjectIndex);
            for ( iWebsiteIndex = 0; iWebsiteIndex < iWebsiteCount; iWebsiteIndex++ )
            {
                ConvertWebsiteIndexToLink( iProjectIndex, iWebsiteIndex, strWebsiteLink );
                pDoc->GetProjectWebsiteName(iProjectIndex, iWebsiteIndex, strWebsiteName);
                
                m_pTaskPane->CreateTask( strWebsiteLink, BITMAP_PROJECTS, strWebsiteName, false );
            }
        }
    }
    m_pTaskPane->EndTaskSection(m_bWebsiteHeaderHidden);

    m_pTaskPane->UpdateQuickTip( SECTION_TIPS, BITMAP_TIPSHEADER, GetCurrentQuickTipText(), m_bTipsHeaderHidden );

    m_pTaskPane->EndTaskPage();
}


wxInt32 CViewProjects::FormatProjectName( wxInt32 item, wxString& strBuffer ) const
{
    CMainDocument* pDoc = wxGetApp().GetDocument();

    wxASSERT(NULL != pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));

    strBuffer.Clear();

    pDoc->GetProjectProjectName(item, strBuffer);

    return 0;
}


wxInt32 CViewProjects::FormatAccountName( wxInt32 item, wxString& strBuffer ) const
{
    CMainDocument* pDoc = wxGetApp().GetDocument();

    wxASSERT(NULL != pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));

    strBuffer.Clear();

    pDoc->GetProjectAccountName(item, strBuffer);

    return 0;
}


wxInt32 CViewProjects::FormatTeamName( wxInt32 item, wxString& strBuffer ) const
{
    CMainDocument* pDoc = wxGetApp().GetDocument();

    wxASSERT(NULL != pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));

    strBuffer.Clear();

    pDoc->GetProjectTeamName(item, strBuffer);

    return 0;
}


wxInt32 CViewProjects::FormatTotalCredit( wxInt32 item, wxString& strBuffer ) const
{
    float fBuffer;
    CMainDocument* pDoc = wxGetApp().GetDocument();

    wxASSERT(NULL != pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));

    strBuffer.Clear();

    pDoc->GetProjectTotalCredit(item, fBuffer);
    strBuffer.Printf(wxT("%0.2f"), fBuffer);

    return 0;
}


wxInt32 CViewProjects::FormatAVGCredit( wxInt32 item, wxString& strBuffer ) const
{
    float fBuffer;
    CMainDocument* pDoc = wxGetApp().GetDocument();

    wxASSERT(NULL != pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));

    strBuffer.Clear();

    pDoc->GetProjectAvgCredit(item, fBuffer);
    strBuffer.Printf(wxT("%0.2f"), fBuffer);

    return 0;
}


wxInt32 CViewProjects::FormatResourceShare( wxInt32 item, wxString& strBuffer ) const
{
    float fResourceShareBuffer;
    float fTotalResourceShareBuffer;
    CMainDocument* pDoc = wxGetApp().GetDocument();

    wxASSERT(NULL != pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));

    strBuffer.Clear();

    pDoc->GetProjectResourceShare(item, fResourceShareBuffer);
    pDoc->GetProjectTotalResourceShare(item, fTotalResourceShareBuffer);
    strBuffer.Printf(wxT("%0.0f ( %0.2f%% )"), fResourceShareBuffer, ((fResourceShareBuffer / fTotalResourceShareBuffer) * 100));

    return 0;
}


wxInt32 CViewProjects::FormatStatus( wxInt32 item, wxString& strBuffer ) const
{
    wxInt32 iNextRPC;
    CMainDocument* pDoc = wxGetApp().GetDocument();

    wxASSERT(NULL != pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));

    strBuffer.Clear();

    if      (pDoc->IsProjectSuspended(item))
    {
        strBuffer = _("Project Suspended");
    } 
    else if (pDoc->IsProjectRPCPending(item))
    {
        pDoc->GetProjectMinRPCTime(item, iNextRPC);

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


bool CViewProjects::IsWebsiteLink( const wxString& strLink )
{
    bool bReturnValue = false;

    if ( strLink.StartsWith( LINK_WEB + wxT(":") ) )
        bReturnValue = true;

    return bReturnValue;
}


wxInt32 CViewProjects::ConvertWebsiteIndexToLink( wxInt32 iProjectIndex, wxInt32 iWebsiteIndex, wxString& strLink )
{
    strLink.Printf(wxT("%s:%d:%d"), LINK_WEB, iProjectIndex, iWebsiteIndex);
    return 0;
}


wxInt32 CViewProjects::ConvertLinkToWebsiteIndex( const wxString& strLink, wxInt32& iProjectIndex, wxInt32& iWebsiteIndex )
{
    wxString strTemplate = strLink;
    wxString strBuffer = wxEmptyString;

    strTemplate.Replace( LINK_WEB + wxT(":"), wxEmptyString );

    strBuffer = strTemplate;
    strBuffer.Remove( strBuffer.Find( wxT(":") ) );
    strBuffer.ToLong( (long*) &iProjectIndex );

    strBuffer = strTemplate;
    strBuffer = strBuffer.Mid( strBuffer.Find( wxT(":") ) + 1 );
    strBuffer.ToLong( (long*) &iWebsiteIndex );

    return 0;
}


void CViewProjects::ExecuteLink( const wxString &strLink )
{
    wxString strMimeType = wxEmptyString;

    if      ( strLink.StartsWith(wxT("http://")) )
        strMimeType = wxT("text/html");
    else if ( strLink.StartsWith(wxT("ftp://")) )
        strMimeType = wxT("text/html");
    else if ( strLink.StartsWith(wxT("mailto:")) )
        strMimeType = wxT("message/rfc822");
    else
        return;

    wxFileType* ft = wxTheMimeTypesManager->GetFileTypeFromMimeType(strMimeType);
    if ( ft )
    {
        wxString cmd;
        if ( ft->GetOpenCommand( &cmd, wxFileType::MessageParameters(strLink) ) )
        {
            cmd.Replace(wxT("file://"), wxEmptyString);
            ::wxExecute(cmd);
        }

        delete ft;
    }
}

