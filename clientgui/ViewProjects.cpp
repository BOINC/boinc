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

#include "wx/arrimpl.cpp" 


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


const wxString LINKDESC_DEFAULT         = 
     _("Please click a project to see additional options.");

const wxString LINK_TASKATTACH      = wxT(SECTION_TASK "attach");
const wxString LINKDESC_TASKATTACH  = 
     _("<b>Attach to new project</b><br>"
       "Attach this computer to a BOINC project.  "
       "You'll need a project URL and account key "
       "(visit the project's web site to get these).");

const wxString LINK_TASKDETACH      = wxT(SECTION_TASK "detach");
const wxString LINKDESC_TASKDETACH  = 
     _("<b>Detach from project</b><br>"
       "Detach this computer from this project.  "
       "You may wish to update the project first to report "
       "any completed work.");

const wxString LINK_TASKRESET       = wxT(SECTION_TASK "reset");
const wxString LINKDESC_TASKRESET   = 
     _("<b>Reset project</b><br>"
       "Remove all workunits and applications from this project.  "
       "You may wish to update the project "
       "first to report any completed work.");

const wxString LINK_TASKSUSPEND     = wxT(SECTION_TASK "suspend");
const wxString LINKDESC_TASKSUSPEND = 
     _("<b>Suspend project</b><br>"
       "Stop work for this project "
       "(you can resume later).");

const wxString LINK_TASKRESUME      = wxT(SECTION_TASK "resume");
const wxString LINKDESC_TASKRESUME  = 
     _("<b>Resume project</b><br>"
       "Resume work for this project");

const wxString LINK_TASKUPDATE      = wxT(SECTION_TASK "update");
const wxString LINKDESC_TASKUPDATE  = 
     _("<b>Update project</b><br>"
       "Report all completed work and refresh "
       "your credit and preferences for this project.");

const wxString LINK_WEBBOINC        = wxT(SECTION_WEB "boinc");
const wxString LINKDESC_WEBBOINC    = 
     _("<b>BOINC home page</b><br>"
       "Open the BOINC home page in a web browser.");

const wxString LINK_WEBPROJECT      = wxT(SECTION_WEB "project");
const wxString LINKDESC_WEBPROJECT  = 
     _("<b>Project home page</b><br>"
       "Open this project's home page in a web browser.");

const wxString LINK_WEB             = wxT(SECTION_WEB ":");


WX_DEFINE_OBJARRAY( CProjectCache );


CProject::CProject()
{
    m_strProjectName = wxEmptyString;
    m_strAccountName = wxEmptyString;
    m_strTeamName = wxEmptyString;
    m_strTotalCredit = wxEmptyString;
    m_strAVGCredit = wxEmptyString;
    m_strResourceShare = wxEmptyString;
    m_strStatus = wxEmptyString;
}


CProject::~CProject()
{
    m_strProjectName.Clear();
    m_strAccountName.Clear();
    m_strTeamName.Clear();
    m_strTotalCredit.Clear();
    m_strAVGCredit.Clear();
    m_strResourceShare.Clear();
    m_strStatus.Clear();
}


wxInt32 CProject::GetProjectName( wxString& strProjectName )
{
    strProjectName = m_strProjectName;	
	return 0;
}


wxInt32 CProject::GetAccountName( wxString& strAccountName )
{
    strAccountName = m_strAccountName;	
	return 0;
}


wxInt32 CProject::GetTeamName( wxString& strTeamName )
{
    strTeamName = m_strTeamName;	
	return 0;
}


wxInt32 CProject::GetTotalCredit( wxString& strTotalCredit )
{
    strTotalCredit = m_strTotalCredit;	
	return 0;
}


wxInt32 CProject::GetAVGCredit( wxString& strAVGCredit )
{
    strAVGCredit = m_strAVGCredit;	
	return 0;
}


wxInt32 CProject::GetResourceShare( wxString& strResourceShare )
{
    strResourceShare = m_strResourceShare;	
	return 0;
}


wxInt32 CProject::GetStatus( wxString& strStatus )
{
    strStatus = m_strStatus;	
	return 0;
}


wxInt32 CProject::SetProjectName( wxString& strProjectName )
{
    m_strProjectName = strProjectName;	
	return 0;
}


wxInt32 CProject::SetAccountName( wxString& strAccountName )
{
    m_strAccountName = strAccountName;	
	return 0;
}


wxInt32 CProject::SetTeamName( wxString& strTeamName )
{
    m_strTeamName = strTeamName;	
	return 0;
}


wxInt32 CProject::SetTotalCredit( wxString& strTotalCredit )
{
    m_strTotalCredit = strTotalCredit;	
	return 0;
}


wxInt32 CProject::SetAVGCredit( wxString& strAVGCredit )
{
    m_strAVGCredit = strAVGCredit;	
	return 0;
}


wxInt32 CProject::SetResourceShare( wxString& strResourceShare )
{
    m_strResourceShare = strResourceShare;	
	return 0;
}


wxInt32 CProject::SetStatus( wxString& strStatus )
{
    m_strStatus = strStatus;	
	return 0;
}


IMPLEMENT_DYNAMIC_CLASS(CViewProjects, CBOINCBaseView)


CViewProjects::CViewProjects()
{
}


CViewProjects::CViewProjects(wxNotebook* pNotebook) :
    CBOINCBaseView( pNotebook, ID_HTML_PROJECTSVIEW, DEFAULT_HTML_FLAGS, ID_LIST_PROJECTSVIEW, DEFAULT_LIST_SINGLE_SEL_FLAGS )
{
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
    m_pTaskPane->CreateTaskHeader(BITMAP_WEBHEADER, bmpWeb, _("Web sites"));
    m_pTaskPane->CreateTaskHeader(BITMAP_TIPSHEADER, bmpTips, _("Quick Tips"));

    m_pListPane->InsertColumn(COLUMN_PROJECT, _("Project"), wxLIST_FORMAT_LEFT, 150);
    m_pListPane->InsertColumn(COLUMN_ACCOUNTNAME, _("Account"), wxLIST_FORMAT_LEFT, 80);
    m_pListPane->InsertColumn(COLUMN_TEAMNAME, _("Team"), wxLIST_FORMAT_LEFT, 80);
    m_pListPane->InsertColumn(COLUMN_TOTALCREDIT, _("Total Credit"), wxLIST_FORMAT_RIGHT, 80);
    m_pListPane->InsertColumn(COLUMN_AVGCREDIT, _("Avg. Credit"), wxLIST_FORMAT_RIGHT, 80);
    m_pListPane->InsertColumn(COLUMN_RESOURCESHARE, _("Resource Share"), wxLIST_FORMAT_CENTRE, 85);
    m_pListPane->InsertColumn(COLUMN_STATUS, _("Status"), wxLIST_FORMAT_LEFT, 150);

    m_bTipsHeaderHidden = false;
    m_bItemSelected = false;

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


wxInt32 CViewProjects::GetDocCount()
{
    CMainDocument* pDoc      = wxGetApp().GetDocument();

    wxASSERT(NULL != pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));

    return pDoc->GetProjectCount();
}


wxString CViewProjects::OnListGetItemText(long item, long column) const 
{
    CProject& project     = m_ProjectCache.Item( item );
    wxString  strBuffer   = wxEmptyString;

    switch(column)
    {
        case COLUMN_PROJECT:
            project.GetProjectName( strBuffer );
            break;
        case COLUMN_ACCOUNTNAME:
            project.GetAccountName( strBuffer );
            break;
        case COLUMN_TEAMNAME:
            project.GetTeamName( strBuffer );
            break;
        case COLUMN_TOTALCREDIT:
            project.GetTotalCredit( strBuffer );
            break;
        case COLUMN_AVGCREDIT:
            project.GetAVGCredit( strBuffer );
            break;
        case COLUMN_RESOURCESHARE:
            project.GetResourceShare( strBuffer );
            break;
        case COLUMN_STATUS:
            project.GetStatus( strBuffer );
            break;
    }

    return strBuffer;
}


wxString CViewProjects::OnDocGetItemText(long item, long column) const 
{
    wxString       strBuffer = wxEmptyString;

    switch(column)
    {
        case COLUMN_PROJECT:
            FormatProjectName( item, strBuffer );
            break;
        case COLUMN_ACCOUNTNAME:
            FormatAccountName( item, strBuffer );
            break;
        case COLUMN_TEAMNAME:
            FormatTeamName( item, strBuffer );
            break;
        case COLUMN_TOTALCREDIT:
            FormatTotalCredit( item, strBuffer );
            break;
        case COLUMN_AVGCREDIT:
            FormatAVGCredit( item, strBuffer );
            break;
        case COLUMN_RESOURCESHARE:
            FormatResourceShare( item, strBuffer );
            break;
        case COLUMN_STATUS:
            FormatStatus( item, strBuffer );
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
 
        if ( wxID_OK == iAnswer )
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
            _("Are you sure you want to detach from project '%s'?"), 
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
            _("Are you sure you want to reset project '%s'?"), 
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


void CViewProjects::OnTaskCellMouseHover( wxHtmlCell* cell, wxCoord WXUNUSED(x), wxCoord WXUNUSED(y) )
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


wxInt32 CViewProjects::AddCacheElement()
{
    CProject* pItem = new CProject();
    wxASSERT( NULL != pItem );
    if ( NULL != pItem )
    {
        m_ProjectCache.Add( pItem );
        return 0;
    }
    return -1;
}


wxInt32 CViewProjects::EmptyCache()
{
    m_ProjectCache.Empty();
    return 0;
}


wxInt32 CViewProjects::GetCacheCount()
{
    return m_ProjectCache.GetCount();
}


wxInt32 CViewProjects::RemoveCacheElement()
{
    m_ProjectCache.RemoveAt( GetCacheCount() - 1 );
    return 0;
}


wxInt32 CViewProjects::UpdateCache( long item, long column, wxString& strNewData )
{
    CProject& project     = m_ProjectCache.Item( item );

    switch(column)
    {
        case COLUMN_PROJECT:
            project.SetProjectName( strNewData );
            break;
        case COLUMN_ACCOUNTNAME:
            project.SetAccountName( strNewData );
            break;
        case COLUMN_TEAMNAME:
            project.SetTeamName( strNewData );
            break;
        case COLUMN_TOTALCREDIT:
            project.SetTotalCredit( strNewData );
            break;
        case COLUMN_AVGCREDIT:
            project.SetAVGCredit( strNewData );
            break;
        case COLUMN_RESOURCESHARE:
            project.SetResourceShare( strNewData );
            break;
        case COLUMN_STATUS:
            project.SetStatus( strNewData );
            break;
    }

    return 0;
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
        m_pTaskPane->CreateTask( LINK_TASKATTACH, BITMAP_PROJECTS, _("Attach to new project"), m_bTaskAttachHidden );
        m_pTaskPane->CreateTask( LINK_TASKDETACH, BITMAP_PROJECTS, _("Detach from project"), m_bTaskDetachHidden );
        m_pTaskPane->CreateTask( LINK_TASKRESET, BITMAP_PROJECTS, _("Reset project"), m_bTaskResetHidden );
        m_pTaskPane->CreateTask( LINK_TASKSUSPEND, BITMAP_PROJECTS, _("Suspend project"), m_bTaskSuspendHidden );
        m_pTaskPane->CreateTask( LINK_TASKRESUME, BITMAP_PROJECTS, _("Resume project"), m_bTaskResumeHidden );
        m_pTaskPane->CreateTask( LINK_TASKUPDATE, BITMAP_PROJECTS, _("Update project"), m_bTaskUpdateHidden );
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
    strLink.Printf(wxT("%s:%d:%d"), LINK_WEB.c_str(), iProjectIndex, iWebsiteIndex);
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


#ifdef __GNUC__
static volatile const char  __attribute__((unused)) *BOINCrcsid="$Id$";
#else
static volatile const char *BOINCrcsid="$Id$";
#endif
