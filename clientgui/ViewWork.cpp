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
#include "res/task.xpm"
#include "res/tips.xpm"


#define COLUMN_PROJECT              0
#define COLUMN_APPLICATION          1
#define COLUMN_NAME                 2
#define COLUMN_CPUTIME              3
#define COLUMN_PROGRESS             4
#define COLUMN_TOCOMPLETETION       5
#define COLUMN_REPORTDEADLINE       6
#define COLUMN_STATUS               7


CWork::CWork()
{
    m_strProjectName = wxEmptyString;
    m_strApplicationName = wxEmptyString;
    m_strName = wxEmptyString;
    m_strCPUTime = wxEmptyString;
    m_strProgress = wxEmptyString;
    m_strTimeToCompletion = wxEmptyString;
    m_strReportDeadline = wxEmptyString;
    m_strStatus = wxEmptyString;
}


CWork::~CWork()
{
    m_strProjectName.Clear();
    m_strApplicationName.Clear();
    m_strName.Clear();
    m_strCPUTime.Clear();
    m_strProgress.Clear();
    m_strTimeToCompletion.Clear();
    m_strReportDeadline.Clear();
    m_strStatus.Clear();
}


wxInt32 CWork::GetProjectName( wxString& strProjectName )
{
    strProjectName = m_strProjectName;	
	return 0;
}


wxInt32 CWork::GetApplicationName( wxString& strApplicationName )
{
    strApplicationName = m_strApplicationName;	
	return 0;
}


wxInt32 CWork::GetName( wxString& strName )
{
    strName = m_strName;	
	return 0;
}


wxInt32 CWork::GetCPUTime( wxString& strCPUTime )
{
    strCPUTime = m_strCPUTime;	
	return 0;
}


wxInt32 CWork::GetProgress( wxString& strProgress )
{
    strProgress = m_strProgress;	
	return 0;
}


wxInt32 CWork::GetTimeToCompletion( wxString& strTimeToCompletion )
{
    strTimeToCompletion = m_strTimeToCompletion;	
	return 0;
}


wxInt32 CWork::GetReportDeadline( wxString& strReportDeadline )
{
    strReportDeadline = m_strReportDeadline;	
	return 0;
}


wxInt32 CWork::GetStatus( wxString& strStatus )
{
    strStatus = m_strStatus;	
	return 0;
}


wxInt32 CWork::SetProjectName( wxString& strProjectName )
{
    m_strProjectName = strProjectName;	
	return 0;
}


wxInt32 CWork::SetApplicationName( wxString& strApplicationName )
{
    m_strApplicationName = strApplicationName;	
	return 0;
}


wxInt32 CWork::SetName( wxString& strName )
{
    m_strName = strName;	
	return 0;
}


wxInt32 CWork::SetCPUTime( wxString& strCPUTime )
{
    m_strCPUTime = strCPUTime;	
	return 0;
}


wxInt32 CWork::SetProgress( wxString& strProgress )
{
    m_strProgress = strProgress;	
	return 0;
}


wxInt32 CWork::SetTimeToCompletion( wxString& strTimeToCompletion )
{
    m_strTimeToCompletion = strTimeToCompletion;	
	return 0;
}


wxInt32 CWork::SetReportDeadline( wxString& strReportDeadline )
{
    m_strReportDeadline = strReportDeadline;	
	return 0;
}


wxInt32 CWork::SetStatus( wxString& strStatus )
{
    m_strStatus = strStatus;	
	return 0;
}


IMPLEMENT_DYNAMIC_CLASS(CViewWork, CBOINCBaseView)


CViewWork::CViewWork()
{
}


CViewWork::CViewWork(wxNotebook* pNotebook) :
    CBOINCBaseView( pNotebook, ID_HTML_WORKVIEW, DEFAULT_HTML_FLAGS, ID_LIST_WORKVIEW, DEFAULT_LIST_SINGLE_SEL_FLAGS )
{
    wxASSERT(NULL != m_pTaskPane);
    wxASSERT(NULL != m_pListPane);

    //
    // Globalization/Localization
    //
    VIEW_HEADER              = wxT("result");

    SECTION_TASK             = VIEW_HEADER + wxT("task");
    SECTION_TIPS             = VIEW_HEADER + wxT("tips");

    BITMAP_RESULTS           = VIEW_HEADER + wxT(".xpm");
    BITMAP_TASKHEADER        = SECTION_TASK + wxT(".xpm");
    BITMAP_TIPSHEADER        = SECTION_TIPS + wxT(".xpm");

    LINKDESC_DEFAULT         = 
        _("Click a result to see additional options.");

    LINK_TASKSUSPEND         = SECTION_TASK + wxT("suspend");
    LINKDESC_TASKSUSPEND     = 
        _("<b>Suspend</b><br>"
          "Suspend the result.");

    LINK_TASKRESUME          = SECTION_TASK + wxT("resume");
    LINKDESC_TASKRESUME      = 
        _("<b>Resume</b><br>"
          "Resume a suspended result.");

    LINK_TASKSHOWGRAPHICS    = SECTION_TASK + wxT("showgraphics");
    LINKDESC_TASKSHOWGRAPHICS= 
        _("<b>Show graphics</b><br>"
          "Show application graphics in a window.");

    LINK_TASKABORT           = SECTION_TASK + wxT("abort");
    LINKDESC_TASKABORT       = 
        _("<b>Abort result</b><br>"
          "Delete the result from the work queue. "
          "This will prevent you from being granted credit for the result.");


    //
    // Setup View
    //
    wxBitmap bmpResult(result_xpm);
    wxBitmap bmpTask(task_xpm);
    wxBitmap bmpTips(tips_xpm);

    bmpResult.SetMask(new wxMask(bmpResult, wxColour(255, 0, 255)));
    bmpTask.SetMask(new wxMask(bmpTask, wxColour(255, 0, 255)));
    bmpTips.SetMask(new wxMask(bmpTips, wxColour(255, 0, 255)));

    m_pTaskPane->AddVirtualFile(BITMAP_RESULTS, bmpResult, wxBITMAP_TYPE_XPM);

    m_pTaskPane->CreateTaskHeader(BITMAP_TASKHEADER, bmpTask, _("Tasks"));
    m_pTaskPane->CreateTaskHeader(BITMAP_TIPSHEADER, bmpTips, _("Tips"));

    m_pListPane->InsertColumn(COLUMN_PROJECT, _("Project"), wxLIST_FORMAT_LEFT, 125);
    m_pListPane->InsertColumn(COLUMN_APPLICATION, _("Application"), wxLIST_FORMAT_LEFT, 95);
    m_pListPane->InsertColumn(COLUMN_NAME, _("Name"), wxLIST_FORMAT_LEFT, 285);
    m_pListPane->InsertColumn(COLUMN_CPUTIME, _("CPU time"), wxLIST_FORMAT_RIGHT, 80);
    m_pListPane->InsertColumn(COLUMN_PROGRESS, _("Progress"), wxLIST_FORMAT_CENTRE, 60);
    m_pListPane->InsertColumn(COLUMN_TOCOMPLETETION, _("To completetion"), wxLIST_FORMAT_RIGHT, 100);
    m_pListPane->InsertColumn(COLUMN_REPORTDEADLINE, _("Report deadline"), wxLIST_FORMAT_LEFT, 150);
    m_pListPane->InsertColumn(COLUMN_STATUS, _("Status"), wxLIST_FORMAT_LEFT, 135);

    m_bTipsHeaderHidden = false;
    m_bItemSelected = false;

    SetCurrentQuickTip(
        LINK_DEFAULT, 
        LINKDESC_DEFAULT
    );

    UpdateSelection();
}


CViewWork::~CViewWork()
{
    EmptyCache();
}


wxString CViewWork::GetViewName()
{
    return wxString(_("Work"));
}


char** CViewWork::GetViewIcon()
{
    return result_xpm;
}


wxInt32 CViewWork::GetDocCount()
{
    CMainDocument* pDoc      = wxGetApp().GetDocument();

    wxASSERT(NULL != pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));

    return pDoc->GetWorkCount();
}


wxString CViewWork::OnListGetItemText( long item, long column ) const
{
    CWork*    work      = m_WorkCache.at( item );
    wxString  strBuffer = wxEmptyString;

    switch(column)
    {
        case COLUMN_PROJECT:
            work->GetProjectName( strBuffer );
            break;
        case COLUMN_APPLICATION:
            work->GetApplicationName( strBuffer );
            break;
        case COLUMN_NAME:
            work->GetName( strBuffer );
            break;
        case COLUMN_CPUTIME:
            work->GetCPUTime( strBuffer );
            break;
        case COLUMN_PROGRESS:
            work->GetProgress( strBuffer );
            break;
        case COLUMN_TOCOMPLETETION:
            work->GetTimeToCompletion( strBuffer );
            break;
        case COLUMN_REPORTDEADLINE:
            work->GetReportDeadline( strBuffer );
            break;
        case COLUMN_STATUS:
            work->GetStatus( strBuffer );
            break;
    }

    return strBuffer;
}


wxString CViewWork::OnDocGetItemText( long item, long column ) const
{
    wxString       strBuffer = wxEmptyString;

    switch(column)
    {
        case COLUMN_PROJECT:
            FormatProjectName( item, strBuffer );
            break;
        case COLUMN_APPLICATION:
            FormatApplicationName( item, strBuffer );
            break;
        case COLUMN_NAME:
            FormatName( item, strBuffer );
            break;
        case COLUMN_CPUTIME:
            FormatCPUTime( item, strBuffer );
            break;
        case COLUMN_PROGRESS:
            FormatProgress( item, strBuffer );
            break;
        case COLUMN_TOCOMPLETETION:
            FormatTimeToCompletion( item, strBuffer );
            break;
        case COLUMN_REPORTDEADLINE:
            FormatReportDeadline( item, strBuffer );
            break;
        case COLUMN_STATUS:
            FormatStatus( item, strBuffer );
            break;
    }

    return strBuffer;
}


void CViewWork::OnTaskLinkClicked( const wxHtmlLinkInfo& link )
{
    wxInt32  iAnswer        = 0; 
    wxInt32  iProjectIndex  = 0; 
    wxString strProjectURL  = wxEmptyString;
    wxString strResultName  = wxEmptyString;
    wxString strMachineName = wxEmptyString;
    wxString strMessage     = wxEmptyString;
    CMainDocument* pDoc     = wxGetApp().GetDocument();

    wxASSERT(NULL != pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));
    wxASSERT(NULL != m_pTaskPane);
    wxASSERT(NULL != m_pListPane);

    m_bTaskHeaderHidden = false;
    m_bTipsHeaderHidden = false;

    if ( link.GetHref() == LINK_TASKSUSPEND )
    {
        iProjectIndex = m_pListPane->GetFirstSelected();

        pDoc->WorkSuspend(
            iProjectIndex
        );
    }
    else if ( link.GetHref() == LINK_TASKRESUME )
    {
        iProjectIndex = m_pListPane->GetFirstSelected();

        pDoc->WorkResume(
            iProjectIndex
        );
    }
    else if ( link.GetHref() == LINK_TASKSHOWGRAPHICS )
    {
        iProjectIndex = m_pListPane->GetFirstSelected();
        pDoc->GetConnectedComputerName( strMachineName );

        if ( !strMachineName.empty() )
        {
            iAnswer = wxMessageBox(
                _("Are you sure you wish to display graphics on a remote machine?"),
                _("Show graphics"),
                wxYES_NO | wxICON_QUESTION, 
                this
            );
        }
        else
        {
            iAnswer = wxYES;
        }

        if ( wxYES == iAnswer )
        {
			int foo = iProjectIndex;
            pDoc->WorkShowGraphics(
                foo,
                false,
                wxGetApp().m_strDefaultWindowStation,
                wxGetApp().m_strDefaultDesktop,
                wxGetApp().m_strDefaultDisplay
            );
        }
    }
    else if ( link.GetHref() == LINK_TASKABORT )
    {
        iProjectIndex = m_pListPane->GetFirstSelected();
        pDoc->GetWorkName(iProjectIndex, strResultName);

        strMessage.Printf(
            _("Are you sure you want to abort this result '%s'?"), 
            strResultName.c_str());

        iAnswer = wxMessageBox(
            strMessage,
            _("Abort result"),
            wxYES_NO | wxICON_QUESTION, 
            this
        );

        if ( wxYES == iAnswer )
        {
            pDoc->WorkAbort(
                iProjectIndex
            );
        }
    }

    UpdateSelection();
    m_pListPane->Refresh();
}


void CViewWork::OnTaskCellMouseHover( wxHtmlCell* cell, wxCoord WXUNUSED(x), wxCoord WXUNUSED(y) )
{
    if ( NULL != cell->GetLink() )
    {
        bool        bUpdateSelection = false;
        wxString    strLink;

        strLink = cell->GetLink()->GetHref();

        if      ( UpdateQuickTip( strLink, LINK_TASKSUSPEND, LINKDESC_TASKSUSPEND ) )
            bUpdateSelection = true;
        else if ( UpdateQuickTip( strLink, LINK_TASKRESUME, LINKDESC_TASKRESUME ) )
            bUpdateSelection = true;
        else if ( UpdateQuickTip( strLink, LINK_TASKSHOWGRAPHICS, LINKDESC_TASKSHOWGRAPHICS ) )
            bUpdateSelection = true;
        else if ( UpdateQuickTip( strLink, LINK_TASKABORT, LINKDESC_TASKABORT ) )
            bUpdateSelection = true;
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


wxInt32 CViewWork::AddCacheElement()
{
    CWork* pItem = new CWork();
    wxASSERT( NULL != pItem );
    if ( NULL != pItem )
    {
        m_WorkCache.push_back( pItem );
        return 0;
    }
    return -1;
}


wxInt32 CViewWork::EmptyCache()
{
    unsigned int i;
    for (i=0; i<m_WorkCache.size(); i++) {
        delete m_WorkCache[i];
    }
    m_WorkCache.clear();
    return 0;
}


wxInt32 CViewWork::GetCacheCount()
{
    return m_WorkCache.size();
}


wxInt32 CViewWork::RemoveCacheElement()
{
    delete m_WorkCache.back();
    m_WorkCache.erase( m_WorkCache.end() - 1 );
    return 0;
}


wxInt32 CViewWork::UpdateCache( long item, long column, wxString& strNewData )
{
    CWork* work   = m_WorkCache.at( item );

    switch(column)
    {
        case COLUMN_PROJECT:
            work->SetProjectName( strNewData );
            break;
        case COLUMN_APPLICATION:
            work->SetApplicationName( strNewData );
            break;
        case COLUMN_NAME:
            work->SetName( strNewData );
            break;
        case COLUMN_CPUTIME:
            work->SetCPUTime( strNewData );
            break;
        case COLUMN_PROGRESS:
            work->SetProgress( strNewData );
            break;
        case COLUMN_TOCOMPLETETION:
            work->SetTimeToCompletion( strNewData );
            break;
        case COLUMN_REPORTDEADLINE:
            work->SetReportDeadline( strNewData );
            break;
        case COLUMN_STATUS:
            work->SetStatus( strNewData );
            break;
    }

    return 0;
}


void CViewWork::UpdateSelection()
{
    CMainDocument* pDoc = wxGetApp().GetDocument();
    wxInt32        iSelectedRow   = -1;

    wxASSERT(NULL != pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));
    wxASSERT(NULL != m_pTaskPane);
    wxASSERT(NULL != m_pListPane);

    if ( 0 == m_pListPane->GetSelectedItemCount() )
    {
        m_bTaskHeaderHidden = true;
        m_bTaskSuspendHidden = true;
        m_bTaskResumeHidden = true;
        m_bTaskShowGraphicsHidden = true;
        m_bTaskAbortHidden = true;

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
        iSelectedRow = m_pListPane->GetFirstSelected();

        m_bTaskHeaderHidden = false;

        if ( pDoc->IsWorkSuspended( iSelectedRow ) )
        {
            m_bTaskSuspendHidden = true;
            m_bTaskResumeHidden = false;
        }
        else
        {
            m_bTaskSuspendHidden = false;
            m_bTaskResumeHidden = true;
        }

        if ( pDoc->IsWorkGraphicsSupported( iSelectedRow ) && !pDoc->IsWorkSuspended( iSelectedRow ) )
            m_bTaskShowGraphicsHidden = false;
        else
            m_bTaskShowGraphicsHidden = true;

        if ( !pDoc->IsWorkAborted( iSelectedRow ) )
            m_bTaskAbortHidden = false;
        else
        {
            m_bTaskAbortHidden = true;
            m_bTaskSuspendHidden = true;
            m_bTaskResumeHidden = true;
            UpdateQuickTip( LINK_TASKABORT, LINK_TASKABORT, LINKDESC_TASKABORT );
        }

        if ( m_bTaskSuspendHidden && m_bTaskResumeHidden && m_bTaskShowGraphicsHidden && m_bTaskAbortHidden )
            m_bTaskHeaderHidden = true;

        m_bItemSelected = true;
    }
    UpdateTaskPane();
}


void CViewWork::UpdateTaskPane()
{
    wxASSERT(NULL != m_pTaskPane);

    m_pTaskPane->BeginTaskPage();

    m_pTaskPane->BeginTaskSection( BITMAP_TASKHEADER, m_bTaskHeaderHidden );
    if (!m_bTaskHeaderHidden)
    {
        m_pTaskPane->CreateTask( LINK_TASKSUSPEND, _("Suspend"), m_bTaskSuspendHidden );
        m_pTaskPane->CreateTask( LINK_TASKRESUME, _("Resume"), m_bTaskResumeHidden );
        m_pTaskPane->CreateTask( LINK_TASKSHOWGRAPHICS, _("Show graphics"), m_bTaskShowGraphicsHidden );
        m_pTaskPane->CreateTask( LINK_TASKABORT, _("Abort result"), m_bTaskAbortHidden );
    }
    m_pTaskPane->EndTaskSection( m_bTaskHeaderHidden );

    m_pTaskPane->UpdateQuickTip( BITMAP_TIPSHEADER, GetCurrentQuickTipText(), m_bTipsHeaderHidden );

    m_pTaskPane->EndTaskPage();
}


wxInt32 CViewWork::FormatProjectName( wxInt32 item, wxString& strBuffer ) const
{
    CMainDocument* pDoc = wxGetApp().GetDocument();

    wxASSERT(NULL != pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));

    strBuffer.Clear();

    pDoc->GetWorkProjectName(item, strBuffer);

    return 0;
}


wxInt32 CViewWork::FormatApplicationName( wxInt32 item, wxString& strBuffer ) const
{
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


wxInt32 CViewWork::FormatName( wxInt32 item, wxString& strBuffer ) const
{
    CMainDocument* pDoc = wxGetApp().GetDocument();

    wxASSERT(NULL != pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));

    strBuffer.Clear();

    pDoc->GetWorkName(item, strBuffer);

    return 0;
}


wxInt32 CViewWork::FormatCPUTime( wxInt32 item, wxString& strBuffer ) const
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

    if (pDoc->IsWorkActive(item))
    {
        pDoc->GetWorkCurrentCPUTime(item, fBuffer);
    }
    else
    {
        if(pDoc->GetWorkState(item) < CMainDocument::COMPUTE_ERROR)
            fBuffer = 0;
        else 
            pDoc->GetWorkFinalCPUTime(item, fBuffer);
    }

    if ( 0 == fBuffer )
    {
        strBuffer = wxT("---");
    }
    else
    {
        iHour = (wxInt32)(fBuffer / (60 * 60));
        iMin  = (wxInt32)(fBuffer / 60) % 60;
        iSec  = (wxInt32)(fBuffer) % 60;

        ts = wxTimeSpan( iHour, iMin, iSec );

        strBuffer = ts.Format();
    }

    return 0;
}


wxInt32 CViewWork::FormatProgress( wxInt32 item, wxString& strBuffer ) const
{
    float          fBuffer = 0;
    CMainDocument* pDoc = wxGetApp().GetDocument();

    wxASSERT(NULL != pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));

    strBuffer.Clear();

    if (!pDoc->IsWorkActive(item))
    {
        if( pDoc->GetWorkState(item) < CMainDocument::COMPUTE_ERROR )
            strBuffer.Printf(wxT("%.2f%%"), 0.0);
        else 
            strBuffer.Printf(wxT("%.2f%%"), 100.00);
    }
    else
    {
        pDoc->GetWorkFractionDone(item, fBuffer);
        strBuffer.Printf(wxT("%.2f%%"), fBuffer * 100);
    }

    return 0;
}


wxInt32 CViewWork::FormatTimeToCompletion( wxInt32 item, wxString& strBuffer ) const
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

    if ( 0 >= fBuffer )
    {
        strBuffer = wxT("---");
    }
    else
    {
        iHour = (wxInt32)(fBuffer / (60 * 60));
        iMin  = (wxInt32)(fBuffer / 60) % 60;
        iSec  = (wxInt32)(fBuffer) % 60;

        ts = wxTimeSpan( iHour, iMin, iSec );

        strBuffer = ts.Format();
    }

    return 0;
}


wxInt32 CViewWork::FormatReportDeadline( wxInt32 item, wxString& strBuffer ) const
{
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


wxInt32 CViewWork::FormatStatus( wxInt32 item, wxString& strBuffer ) const
{
    CMainDocument* pDoc = wxGetApp().GetDocument();
    wxInt32        iActivityMode = -1;

    wxASSERT(NULL != pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));

    strBuffer.Clear();

    switch( pDoc->GetWorkState(item) )
    {
        case CMainDocument::NEW:
            strBuffer = _("New"); 
            break;
        case CMainDocument::FILES_DOWNLOADING:
            if (pDoc->IsWorkReadyToReport(item))
            {
                strBuffer = _("Download failed");
            }
            else
            {
                strBuffer = _("Downloading");
            }
            break;
        case CMainDocument::FILES_DOWNLOADED:
            if      ( pDoc->IsWorkAborted(item) )
            {
                strBuffer = _("Aborted");
            }
            else if ( !pDoc->IsWorkActive(item) && pDoc->IsWorkSuspended(item) )
            {
                strBuffer = _("Suspended");
            }
            else if ( pDoc->IsWorkActive(item) )
            {
                wxInt32 iSchedulerState = pDoc->GetWorkSchedulerState(item);
                if      ( CMainDocument::SCHED_SCHEDULED == iSchedulerState )
                {
                    strBuffer = _("Running");
                }
                else if ( CMainDocument::SCHED_PREEMPTED == iSchedulerState )
                {
                    if ( pDoc->IsWorkSuspended(item) )
                    {
                        strBuffer = _("Suspended");
                    }
                    else
                    {
                        strBuffer = _("Paused");
                    }
                }
                else if ( CMainDocument::SCHED_UNINITIALIZED == iSchedulerState )
                {
                    strBuffer = _("Ready to run");
                }
            }
            else
            {
                strBuffer = _("Ready to run");
            }
            break;
        case CMainDocument::COMPUTE_ERROR:
            strBuffer = _("Computation error");
            break;
        case CMainDocument::FILES_UPLOADING:
            if ( pDoc->IsWorkReadyToReport(item) )
            {
                strBuffer = _("Upload failed");
            }
            else
            {
                strBuffer = _("Uploading");
            }
            break;
        default:
            if      ( pDoc->IsWorkAcknowledged(item) ) 
            {
                strBuffer = _("Acknowledged");
            }
            else if ( pDoc->IsWorkReadyToReport(item) )
            {
                strBuffer = _("Ready to report");
            }
            else
            {
                strBuffer.Format(_("Error: invalid state '%d'"), pDoc->GetWorkState(item));
            }
            break;
    }

    pDoc->GetActivityRunMode( iActivityMode );
    if ( CMainDocument::MODE_NEVER == iActivityMode )
    {
        strBuffer = wxT(" ( ") + strBuffer + wxT(" ) ");
        strBuffer = _("Suspended") + strBuffer;
    }

    return 0;
}


const char *BOINC_RCSID_34f860f736 = "$Id$";
