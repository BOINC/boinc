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


#define VIEW_HEADER                 wxT("result")

#define SECTION_TASK                wxT(VIEW_HEADER "task")
#define SECTION_TIPS                wxT(VIEW_HEADER "tips")

#define BITMAP_RESULTS              wxT(VIEW_HEADER ".xpm")
#define BITMAP_TASKHEADER           wxT(SECTION_TASK ".xpm")
#define BITMAP_TIPSHEADER           wxT(SECTION_TIPS ".xpm")

#define LINK_DEFAULT                wxT("default")

#define COLUMN_PROJECT              0
#define COLUMN_APPLICATION          1
#define COLUMN_NAME                 2
#define COLUMN_CPUTIME              3
#define COLUMN_PROGRESS             4
#define COLUMN_TOCOMPLETETION       5
#define COLUMN_REPORTDEADLINE       6
#define COLUMN_STATUS               7


const wxString LINK_TASKSUSPEND         = wxT(SECTION_TASK "suspend");
const wxString LINKDESC_TASKSUSPEND     = 
     _("<b>Suspend</b><br>"
       "Selecting suspend allows you to suspend the currently selected result.");

const wxString LINK_TASKRESUME          = wxT(SECTION_TASK "resume");
const wxString LINKDESC_TASKRESUME      = 
     _("<b>Resume</b><br>"
       "Selecting resume allows you to resume a previously suspended result.");

const wxString LINK_TASKSHOWGRAPHICS    = wxT(SECTION_TASK "showgraphics");
const wxString LINKDESC_TASKSHOWGRAPHICS= 
     _("<b>Show Graphics</b><br>"
       "Selecting show graphics will display a window giving you a chance "
       "to see how the active result will look while in screensaver mode.");

const wxString LINK_TASKABORT           = wxT(SECTION_TASK "abort");
const wxString LINKDESC_TASKABORT       = 
     _("<b>Abort Result</b><br>"
       "Selecting abort result will delete the result from the work queue. "
       "Doing this will keep you from being granted any credit for this result.");


IMPLEMENT_DYNAMIC_CLASS(CViewWork, CBOINCBaseView)


CViewWork::CViewWork()
{
}


CViewWork::CViewWork(wxNotebook* pNotebook) :
    CBOINCBaseView(pNotebook, ID_HTML_WORKVIEW, ID_LIST_WORKVIEW)
{
    m_bProcessingTaskRenderEvent = false;
    m_bProcessingListRenderEvent = false;

    wxASSERT(NULL != m_pTaskPane);
    wxASSERT(NULL != m_pListPane);

    wxBitmap bmpResult(result_xpm);
    wxBitmap bmpTask(task_xpm);
    wxBitmap bmpTips(tips_xpm);

    bmpResult.SetMask(new wxMask(bmpResult, wxColour(255, 0, 255)));
    bmpTask.SetMask(new wxMask(bmpTask, wxColour(255, 0, 255)));
    bmpTips.SetMask(new wxMask(bmpTips, wxColour(255, 0, 255)));

    m_pTaskPane->AddVirtualFile(BITMAP_RESULTS, bmpResult, wxBITMAP_TYPE_XPM);

    m_pTaskPane->CreateTaskHeader(BITMAP_TASKHEADER, bmpTask, _("Tasks"));
    m_pTaskPane->CreateTaskHeader(BITMAP_TIPSHEADER, bmpTips, _("Quick Tips"));

    m_pListPane->InsertColumn(COLUMN_PROJECT, _("Project"), wxLIST_FORMAT_LEFT, -1);
    m_pListPane->InsertColumn(COLUMN_APPLICATION, _("Application"), wxLIST_FORMAT_LEFT, -1);
    m_pListPane->InsertColumn(COLUMN_NAME, _("Name"), wxLIST_FORMAT_LEFT, -1);
    m_pListPane->InsertColumn(COLUMN_CPUTIME, _("CPU time"), wxLIST_FORMAT_RIGHT, -1);
    m_pListPane->InsertColumn(COLUMN_PROGRESS, _("Progress"), wxLIST_FORMAT_CENTER, -1);
    m_pListPane->InsertColumn(COLUMN_TOCOMPLETETION, _("To Completetion"), wxLIST_FORMAT_LEFT, -1);
    m_pListPane->InsertColumn(COLUMN_REPORTDEADLINE, _("Report Deadline"), wxLIST_FORMAT_LEFT, -1);
    m_pListPane->InsertColumn(COLUMN_STATUS, _("Status"), wxLIST_FORMAT_LEFT, -1);

    m_bTipsHeaderHidden = false;

    SetCurrentQuickTip(
        LINK_DEFAULT, 
        _("Please select a result to see additional options.")
    );

    UpdateSelection();
}


CViewWork::~CViewWork()
{
}


wxString CViewWork::GetViewName()
{
    return wxString(_("Work"));
}


char** CViewWork::GetViewIcon()
{
    return result_xpm;
}


void CViewWork::OnTaskRender(wxTimerEvent &event)
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


void CViewWork::OnListRender(wxTimerEvent &event)
{
    if (!m_bProcessingListRenderEvent)
    {
        m_bProcessingListRenderEvent = true;

        wxASSERT(NULL != m_pListPane);

        wxInt32 iCount = wxGetApp().GetDocument()->GetWorkCount();
        if ( iCount != m_iCount )
        {
            m_iCount = iCount;
            m_pListPane->SetItemCount(iCount);
        }
        else
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
                            FormatProjectName(iRowIndex, strBuffer);
                            break;
                        case COLUMN_APPLICATION:
                            FormatApplicationName(iRowIndex, strBuffer);
                            break;
                        case COLUMN_NAME:
                            FormatName(iRowIndex, strBuffer);
                            break;
                        case COLUMN_CPUTIME:
                            FormatCPUTime(iRowIndex, strBuffer);
                            break;
                        case COLUMN_PROGRESS:
                            FormatProgress(iRowIndex, strBuffer);
                            break;
                        case COLUMN_TOCOMPLETETION:
                            FormatTimeToCompletion(iRowIndex, strBuffer);
                            break;
                        case COLUMN_REPORTDEADLINE:
                            FormatReportDeadline(iRowIndex, strBuffer);
                            break;
                        case COLUMN_STATUS:
                            FormatStatus(iRowIndex, strBuffer);
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

    m_pListPane->Refresh();

    event.Skip();
}


void CViewWork::OnListSelected ( wxListEvent& event )
{
    SetCurrentQuickTip(
        LINK_DEFAULT, 
        wxT("")
    );

    UpdateSelection();
    event.Skip();
}


void CViewWork::OnListDeselected ( wxListEvent& event )
{
    SetCurrentQuickTip(
        LINK_DEFAULT, 
        wxT("")
    );

    UpdateSelection();
    event.Skip();
}


wxString CViewWork::OnListGetItemText( long item, long column ) const
{
    wxString strBuffer = wxEmptyString;

    switch(column)
    {
        case COLUMN_PROJECT:
            if (item == m_iCacheFrom) wxGetApp().GetDocument()->CachedStateLock();
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
        case COLUMN_TOCOMPLETETION:
            FormatTimeToCompletion(item, strBuffer);
            break;
        case COLUMN_REPORTDEADLINE:
            FormatReportDeadline(item, strBuffer);
            break;
        case COLUMN_STATUS:
            FormatStatus(item, strBuffer);
            if (item == m_iCacheTo) wxGetApp().GetDocument()->CachedStateUnlock();
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
    wxString strMessage     = wxEmptyString;
    CMainDocument* pDoc     = wxGetApp().GetDocument();

    wxASSERT(NULL != pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));
    wxASSERT(NULL != m_pTaskPane);
    wxASSERT(NULL != m_pListPane);

    if      ( link.GetHref() == SECTION_TASK )
        m_bTaskHeaderHidden ? m_bTaskHeaderHidden = false : m_bTaskHeaderHidden = true;
    else if ( link.GetHref() == LINK_TASKSUSPEND )
    {
        iProjectIndex = m_pListPane->GetFirstSelected();
        pDoc->GetWorkProjectURL(iProjectIndex, strProjectURL);
        pDoc->GetWorkName(iProjectIndex, strResultName);

        pDoc->WorkSuspend(
            strProjectURL,
            strResultName
        );
    }
    else if ( link.GetHref() == LINK_TASKRESUME )
    {
        iProjectIndex = m_pListPane->GetFirstSelected();
        pDoc->GetWorkProjectURL(iProjectIndex, strProjectURL);
        pDoc->GetWorkName(iProjectIndex, strResultName);

        pDoc->WorkResume(
            strProjectURL,
            strResultName
        );
    }
    else if ( link.GetHref() == LINK_TASKSHOWGRAPHICS )
    {
        iProjectIndex = m_pListPane->GetFirstSelected();
        pDoc->GetWorkProjectURL(iProjectIndex, strProjectURL);
        pDoc->GetWorkName(iProjectIndex, strResultName);

        pDoc->WorkShowGraphics(
            strProjectURL,
            strResultName,
            false
        );
    }
    else if ( link.GetHref() == LINK_TASKABORT )
    {
        iProjectIndex = m_pListPane->GetFirstSelected();
        pDoc->GetWorkProjectURL(iProjectIndex, strProjectURL);
        pDoc->GetWorkName(iProjectIndex, strResultName);

        strMessage.Printf(
            _("Are you sure you wish to abort this result '%s'?"), 
            strResultName.c_str());

        iAnswer = wxMessageBox(
            strMessage,
            _("Abort Result"),
            wxYES_NO | wxICON_QUESTION, 
            this
        );

        if ( wxYES == iAnswer )
        {
            pDoc->WorkAbort(
                strProjectURL,
                strResultName
            );
        }
    }
    else if ( link.GetHref() == SECTION_TIPS )
        m_bTipsHeaderHidden ? m_bTipsHeaderHidden = false : m_bTipsHeaderHidden = true;

    UpdateSelection();
}


void CViewWork::OnTaskCellMouseHover( wxHtmlCell* cell, wxCoord x, wxCoord y )
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
                        _("Please select a result to see additional options.")
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


void CViewWork::UpdateSelection()
{
    CMainDocument* pDoc = wxGetApp().GetDocument();

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
        m_bTaskHeaderHidden = false;
        if ( pDoc->IsWorkSuspended(m_pListPane->GetFirstSelected()) )
        {
            m_bTaskSuspendHidden = true;
            m_bTaskResumeHidden = false;
        }
        else
        {
            m_bTaskSuspendHidden = false;
            m_bTaskResumeHidden = true;
        }
        m_bTaskShowGraphicsHidden = false;
        m_bTaskAbortHidden = false;

        m_bItemSelected = true;
    }
    UpdateTaskPane();
}


void CViewWork::UpdateTaskPane()
{
    wxASSERT(NULL != m_pTaskPane);

    m_pTaskPane->BeginTaskPage();

    m_pTaskPane->BeginTaskSection( SECTION_TASK, BITMAP_TASKHEADER, m_bTaskHeaderHidden );
    if (!m_bTaskHeaderHidden)
    {
        m_pTaskPane->CreateTask( LINK_TASKSUSPEND, BITMAP_RESULTS, _("Suspend"), m_bTaskSuspendHidden );
        m_pTaskPane->CreateTask( LINK_TASKRESUME, BITMAP_RESULTS, _("Resume"), m_bTaskResumeHidden );
        m_pTaskPane->CreateTask( LINK_TASKSHOWGRAPHICS, BITMAP_RESULTS, _("Show Graphics"), m_bTaskShowGraphicsHidden );
        m_pTaskPane->CreateTask( LINK_TASKABORT, BITMAP_RESULTS, _("Abort Result"), m_bTaskAbortHidden );
    }
    m_pTaskPane->EndTaskSection( m_bTaskHeaderHidden );

    m_pTaskPane->UpdateQuickTip( SECTION_TIPS, BITMAP_TIPSHEADER, GetCurrentQuickTipText(), m_bTipsHeaderHidden );

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
    wxString       strTemp = wxEmptyString;
    CMainDocument* pDoc = wxGetApp().GetDocument();

    wxASSERT(NULL != pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));

    strBuffer.Clear();

    pDoc->GetWorkApplicationName(item, strTemp);
    pDoc->GetWorkApplicationVersion(item, iBuffer);

    strBuffer.Printf(wxT("%s %.2f"), strTemp.c_str(), iBuffer/100.0);

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
    int            cpuhour = 0;
    int            cpumin = 0;
    int            cpusec = 0;
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
        if(pDoc->GetWorkState(item) < CMainDocument::RESULT_COMPUTE_ERROR)
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
        cpuhour = (int)(fBuffer / (60 * 60));
        cpumin = (int)(fBuffer / 60) % 60;
        cpusec = (int)(fBuffer) % 60;

        strBuffer.Printf(wxT("%0.2d:%0.2d:%0.2d"), cpuhour, cpumin, cpusec);
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
        if( pDoc->GetWorkState(item) < CMainDocument::RESULT_COMPUTE_ERROR )
            strBuffer = wxT("0.00%");
        else 
            strBuffer = wxT("100.00%");
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
    int            cpuhour = 0;
    int            cpumin = 0;
    int            cpusec = 0;
    CMainDocument* pDoc = wxGetApp().GetDocument();

    wxASSERT(NULL != pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));

    strBuffer.Clear();

    pDoc->GetWorkEstimatedCPUTime(item, fBuffer);

    cpuhour = (int)(fBuffer / (60 * 60));
    cpumin = (int)(fBuffer / 60) % 60;
    cpusec = (int)(fBuffer) % 60;

    strBuffer.Printf(wxT("%0.2d:%0.2d:%0.2d"), cpuhour, cpumin, cpusec);

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

    wxASSERT(NULL != pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));

    strBuffer.Clear();

    switch( pDoc->GetWorkState(item) )
    {
        case CMainDocument::RESULT_NEW:
            strBuffer = _("New"); 
            break;
        case CMainDocument::RESULT_FILES_DOWNLOADING:
            if (pDoc->IsWorkReadyToReport(item))
            {
                strBuffer = _("Download failed");
            }
            else
            {
                strBuffer = _("Downloading");
            }
            break;
        case CMainDocument::RESULT_FILES_DOWNLOADED:
            if ( pDoc->IsWorkActive(item) )
            {
                wxInt32 iSchedulerState = pDoc->GetWorkSchedulerState(item);
                if      ( CMainDocument::CPU_SCHED_SCHEDULED == iSchedulerState )
                {
                    strBuffer = _("Running");
                }
                else if ( CMainDocument::CPU_SCHED_PREEMPTED == iSchedulerState )
                {
                    strBuffer = _("Paused");
                }
                else if ( CMainDocument::CPU_SCHED_UNINITIALIZED == iSchedulerState )
                {
                    strBuffer = _("Ready to run");
                }
            }
            else
            {
                strBuffer = _("Ready to run");
            }
            break;
        case CMainDocument::RESULT_COMPUTE_ERROR:
            strBuffer = _("Computation error");
            break;
        case CMainDocument::RESULT_FILES_UPLOADING:
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
    //if (gstate.activities_suspended) {
    //    strBuf = CString(g_szMiscItems[13]) + " (" + strBuf + ")";
    //}

    return 0;
}

