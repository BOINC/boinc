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
// Revision 1.4  2004/09/25 21:33:24  rwalton
// *** empty log message ***
//
// Revision 1.3  2004/09/24 22:19:01  rwalton
// *** empty log message ***
//
// Revision 1.2  2004/09/24 02:01:53  rwalton
// *** empty log message ***
//
// Revision 1.1  2004/09/21 01:26:26  rwalton
// *** empty log message ***
//
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


#define VIEW_HEADER                 "result"

#define SECTION_TASK                VIEW_HEADER "task"
#define SECTION_TIPS                VIEW_HEADER "tips"

#define BITMAP_RESULTS              VIEW_HEADER ".xpm"
#define BITMAP_TASKHEADER           SECTION_TASK ".xpm"
#define BITMAP_TIPSHEADER           SECTION_TIPS ".xpm"

#define LINK_TASKSUSPENDRESUME      SECTION_TASK "suspendresume"
#define LINK_TASKSHOWGRAPHICS       SECTION_TASK "showgraphics"
#define LINK_TASKABORT              SECTION_TASK "abort"

#define LINK_DEFAULT                "default"

#define COLUMN_PROJECT              0
#define COLUMN_APPLICATION          1
#define COLUMN_NAME                 2
#define COLUMN_CPUTIME              3
#define COLUMN_PROGRESS             4
#define COLUMN_TOCOMPLETETION       5
#define COLUMN_REPORTDEADLINE       6
#define COLUMN_STATUS               7


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

    m_pTaskPane->AddVirtualFile(wxT(BITMAP_RESULTS), bmpResult, wxBITMAP_TYPE_XPM);

    m_pTaskPane->CreateTaskHeader(wxT(BITMAP_TASKHEADER), bmpTask, _("Tasks"));
    m_pTaskPane->CreateTaskHeader(wxT(BITMAP_TIPSHEADER), bmpTips, _("Quick Tips"));

    m_pListPane->InsertColumn(COLUMN_PROJECT, _("Project"), wxLIST_FORMAT_LEFT, -1);
    m_pListPane->InsertColumn(COLUMN_APPLICATION, _("Application"), wxLIST_FORMAT_LEFT, -1);
    m_pListPane->InsertColumn(COLUMN_NAME, _("Name"), wxLIST_FORMAT_LEFT, -1);
    m_pListPane->InsertColumn(COLUMN_CPUTIME, _("CPU time"), wxLIST_FORMAT_LEFT, -1);
    m_pListPane->InsertColumn(COLUMN_PROGRESS, _("Progress"), wxLIST_FORMAT_LEFT, -1);
    m_pListPane->InsertColumn(COLUMN_TOCOMPLETETION, _("To Completetion"), wxLIST_FORMAT_LEFT, -1);
    m_pListPane->InsertColumn(COLUMN_REPORTDEADLINE, _("Report Deadline"), wxLIST_FORMAT_LEFT, -1);
    m_pListPane->InsertColumn(COLUMN_STATUS, _("Status"), wxLIST_FORMAT_LEFT, -1);

    m_bTipsHeaderHidden = false;

    SetCurrentQuickTip(
        wxT(LINK_DEFAULT), 
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

        long lSelected = m_pListPane->GetFirstSelected();
        if ( (-1 == lSelected) && m_bItemSelected )
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
        m_pListPane->SetItemCount(iCount);

        m_bProcessingListRenderEvent = false;
    }
    else
    {
        event.Skip();
    }
}


void CViewWork::OnListSelected ( wxListEvent& event )
{
    UpdateSelection();
    event.Skip();
}


void CViewWork::OnListDeselected ( wxListEvent& event )
{
    UpdateSelection();
    event.Skip();
}


wxString CViewWork::OnListGetItemText( long item, long column ) const
{
    wxString strBuffer;
    switch(column) {
        case COLUMN_PROJECT:
            if (item == m_iCacheFrom) wxGetApp().GetDocument()->CachedStateLock();
            strBuffer = wxGetApp().GetDocument()->GetWorkProjectName(item);
            break;
        case COLUMN_APPLICATION:
            strBuffer = wxGetApp().GetDocument()->GetWorkApplicationName(item);
            break;
        case COLUMN_NAME:
            strBuffer = wxGetApp().GetDocument()->GetWorkName(item);
            break;
        case COLUMN_CPUTIME:
            strBuffer = wxGetApp().GetDocument()->GetWorkCPUTime(item);
            break;
        case COLUMN_PROGRESS:
            strBuffer = wxGetApp().GetDocument()->GetWorkProgress(item);
            break;
        case COLUMN_TOCOMPLETETION:
            strBuffer = wxGetApp().GetDocument()->GetWorkTimeToCompletion(item);
            break;
        case COLUMN_REPORTDEADLINE:
            strBuffer = wxGetApp().GetDocument()->GetWorkReportDeadline(item);
            break;
        case COLUMN_STATUS:
            strBuffer = wxGetApp().GetDocument()->GetWorkStatus(item);
            if (item == m_iCacheTo) wxGetApp().GetDocument()->CachedStateUnlock();
            break;
    }
    return strBuffer;
}


void CViewWork::OnTaskLinkClicked( const wxHtmlLinkInfo& link )
{
    wxASSERT(NULL != m_pTaskPane);
    wxASSERT(NULL != m_pListPane);

    wxString strMessage;

    if ( link.GetHref() == wxT(SECTION_TASK) )
        m_bTaskHeaderHidden ? m_bTaskHeaderHidden = false : m_bTaskHeaderHidden = true;

    if ( link.GetHref() == wxT(SECTION_TIPS) )
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

        if      ( wxT(LINK_TASKSUSPENDRESUME) == strLink )
        {
            if  ( wxT(LINK_TASKSUSPENDRESUME) != GetCurrentQuickTip() )
            {
                SetCurrentQuickTip(
                    wxT(LINK_TASKSUSPENDRESUME), 
                    _("<b>Suspend/Resume</b><br>"
                      "Selecting suspend/resume allows you to suspend or resume the "
                      "currently selected result.")
                );

                bUpdateSelection = true;
            }
        }
        else if ( wxT(LINK_TASKSHOWGRAPHICS) == strLink )
        {
            if  ( wxT(LINK_TASKSHOWGRAPHICS) != GetCurrentQuickTip() )
            {
                SetCurrentQuickTip(
                    wxT(LINK_TASKSHOWGRAPHICS), 
                    _("<b>Show Graphics</b><br>"
                      "Selecting show graphics will display a window giving you a chance "
                      "to see how the active result will look while in screensaver mode.")
                );

                bUpdateSelection = true;
            }
        }
        else if ( wxT(LINK_TASKABORT) == strLink )
        {
            if  ( wxT(LINK_TASKABORT) != GetCurrentQuickTip() )
            {
                SetCurrentQuickTip(
                    wxT(LINK_TASKABORT), 
                    _("<b>Abort Result</b><br>"
                      "Selecting abort result will delete the result from the work queue. "
                      "Doing this will keep you from being granted any credit for this result.")
                );

                bUpdateSelection = true;
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
    wxASSERT(NULL != m_pTaskPane);
    wxASSERT(NULL != m_pListPane);

    long lSelected = m_pListPane->GetFirstSelected();
    if ( -1 == lSelected )
    {
        m_bTaskHeaderHidden = true;
        m_bTaskSuspendResumeHidden = true;
        m_bTaskShowGraphicsHidden = true;
        m_bTaskAbortHidden = true;

        m_bItemSelected = false;
    }
    else
    {
        m_bTaskHeaderHidden = false;
        m_bTaskSuspendResumeHidden = false;
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

    m_pTaskPane->BeginTaskSection( wxT(SECTION_TASK), wxT(BITMAP_TASKHEADER), m_bTaskHeaderHidden );
    if (!m_bTaskHeaderHidden)
    {
        m_pTaskPane->CreateTask( wxT(LINK_TASKSUSPENDRESUME), wxT(BITMAP_RESULTS), _("Suspend/Resume"), m_bTaskSuspendResumeHidden );
        m_pTaskPane->CreateTask( wxT(LINK_TASKSHOWGRAPHICS), wxT(BITMAP_RESULTS), _("Show Graphics"), m_bTaskShowGraphicsHidden );
        m_pTaskPane->CreateTask( wxT(LINK_TASKABORT), wxT(BITMAP_RESULTS), _("Abort Result"), m_bTaskAbortHidden );
    }
    m_pTaskPane->EndTaskSection( m_bTaskHeaderHidden );

    m_pTaskPane->UpdateQuickTip(wxT(SECTION_TIPS), wxT(BITMAP_TIPSHEADER), GetCurrentQuickTipText(), m_bTipsHeaderHidden);

    m_pTaskPane->EndTaskPage();
}

