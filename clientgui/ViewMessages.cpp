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
// Revision 1.4  2004/09/25 21:33:23  rwalton
// *** empty log message ***
//
// Revision 1.3  2004/09/24 22:18:56  rwalton
// *** empty log message ***
//
// Revision 1.2  2004/09/24 02:01:50  rwalton
// *** empty log message ***
//
// Revision 1.1  2004/09/21 01:26:25  rwalton
// *** empty log message ***
//
//

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma implementation "ViewMessages.h"
#endif

#include "stdwx.h"
#include "BOINCGUIApp.h"
#include "MainDocument.h"
#include "BOINCTaskCtrl.h"
#include "BOINCListCtrl.h"
#include "ViewMessages.h"
#include "Events.h"

#include "res/mess.xpm"
#include "res/task.xpm"
#include "res/tips.xpm"


#define VIEW_HEADER                 "mess"

#define SECTION_TASK                VIEW_HEADER "task"
#define SECTION_TIPS                VIEW_HEADER "tips"

#define BITMAP_MESSAGE              VIEW_HEADER ".xpm"
#define BITMAP_TASKHEADER           SECTION_TASK ".xpm"
#define BITMAP_TIPSHEADER           SECTION_TIPS ".xpm"

#define LINK_TASKCOPYALL            SECTION_TASK "copyall"
#define LINK_TASKCOPYMESSAGE        SECTION_TASK "copymessage"

#define LINK_DEFAULT                "default"

#define COLUMN_PROJECT              0
#define COLUMN_TIME                 1
#define COLUMN_MESSAGE              2


IMPLEMENT_DYNAMIC_CLASS(CViewMessages, CBOINCBaseView)


CViewMessages::CViewMessages()
{
}


CViewMessages::CViewMessages(wxNotebook* pNotebook) :
    CBOINCBaseView(pNotebook, ID_HTML_MESSAGESVIEW, ID_LIST_MESSAGESVIEW)
{
    m_bProcessingTaskRenderEvent = false;
    m_bProcessingListRenderEvent = false;

    wxASSERT(NULL != m_pTaskPane);
    wxASSERT(NULL != m_pListPane);

    wxBitmap bmpMessage(mess_xpm);
    wxBitmap bmpTask(task_xpm);
    wxBitmap bmpTips(tips_xpm);

    bmpMessage.SetMask(new wxMask(bmpMessage, wxColour(255, 0, 255)));
    bmpTask.SetMask(new wxMask(bmpTask, wxColour(255, 0, 255)));
    bmpTips.SetMask(new wxMask(bmpTips, wxColour(255, 0, 255)));

    m_pTaskPane->AddVirtualFile(wxT(BITMAP_MESSAGE), bmpMessage, wxBITMAP_TYPE_XPM);

    m_pTaskPane->CreateTaskHeader(wxT(BITMAP_TASKHEADER), bmpTask, _("Tasks"));
    m_pTaskPane->CreateTaskHeader(wxT(BITMAP_TIPSHEADER), bmpTips, _("Quick Tips"));

    m_pListPane->InsertColumn(COLUMN_PROJECT, _("Project"), wxLIST_FORMAT_LEFT, -1);
    m_pListPane->InsertColumn(COLUMN_TIME, _("Time"), wxLIST_FORMAT_LEFT, -1);
    m_pListPane->InsertColumn(COLUMN_MESSAGE, _("Message"), wxLIST_FORMAT_LEFT, -1);

    m_bTipsHeaderHidden = false;

    SetCurrentQuickTip(
        wxT(LINK_DEFAULT), 
        _("Please select a message to see additional options.")
    );

    UpdateSelection();
}


CViewMessages::~CViewMessages()
{
}


wxString CViewMessages::GetViewName()
{
    return wxString(_("Messages"));
}


char** CViewMessages::GetViewIcon()
{
    return mess_xpm;
}


void CViewMessages::OnTaskRender(wxTimerEvent &event)
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


void CViewMessages::OnListRender(wxTimerEvent &event)
{
    if (!m_bProcessingListRenderEvent)
    {
        m_bProcessingListRenderEvent = true;

        wxASSERT(NULL != m_pListPane);

        wxInt32 iCount = wxGetApp().GetDocument()->GetMessageCount();
        m_pListPane->SetItemCount(iCount);

        m_bProcessingListRenderEvent = false;
    }
    else
    {
        event.Skip();
    }
}


void CViewMessages::OnListSelected ( wxListEvent& event )
{
    UpdateSelection();
    event.Skip();
}


void CViewMessages::OnListDeselected ( wxListEvent& event )
{
    UpdateSelection();
    event.Skip();
}


wxString CViewMessages::OnListGetItemText( long item, long column ) const
{
    wxString strBuffer;
    switch(column) {
        case COLUMN_PROJECT:
            if (item == m_iCacheFrom) wxGetApp().GetDocument()->CachedStateLock();
            strBuffer = wxGetApp().GetDocument()->GetMessageProjectName(item);
            break;
        case COLUMN_TIME:
            strBuffer = wxGetApp().GetDocument()->GetMessageTime(item);
            break;
        case COLUMN_MESSAGE:
            strBuffer = wxGetApp().GetDocument()->GetMessageMessage(item);
            if (item == m_iCacheTo) wxGetApp().GetDocument()->CachedStateUnlock();
            break;
    }
    return strBuffer;
}


wxListItemAttr* CViewMessages::OnListGetItemAttr( long item ) const
{
    return NULL;
}


void CViewMessages::OnTaskLinkClicked( const wxHtmlLinkInfo& link )
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


void CViewMessages::OnTaskCellMouseHover( wxHtmlCell* cell, wxCoord x, wxCoord y )
{
    if ( NULL != cell->GetLink() )
    {
        bool        bUpdateSelection = false;
        wxString    strLink;

        strLink = cell->GetLink()->GetHref();

        if      ( wxT(LINK_TASKCOPYALL) == strLink )
        {
            if  ( wxT(LINK_TASKCOPYALL) != GetCurrentQuickTip() )
            {
                SetCurrentQuickTip(
                    wxT(LINK_TASKCOPYALL), 
                    _("<b>Copy All</b><br>"
                      "Selecting copy all, copies all the messages to the system clipboard.")
                );

                bUpdateSelection = true;
            }
        }
        else if ( wxT(LINK_TASKCOPYMESSAGE) == strLink )
        {
            if  ( wxT(LINK_TASKCOPYMESSAGE) != GetCurrentQuickTip() )
            {
                SetCurrentQuickTip(
                    wxT(LINK_TASKCOPYMESSAGE), 
                    _("<b>Copy Message</b><br>"
                      "Selecting copy message, copies the single message to the system clipboard.")
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
                        _("Please select a message to see additional options.")
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


void CViewMessages::UpdateSelection()
{
    wxASSERT(NULL != m_pTaskPane);
    wxASSERT(NULL != m_pListPane);

    long lSelected = m_pListPane->GetFirstSelected();
    if ( -1 == lSelected )
    {
        m_bTaskHeaderHidden = false;
        m_bTaskCopyAllHidden = false;
        m_bTaskCopyMessageHidden = true;

        m_bItemSelected = false;
    }
    else
    {
        m_bTaskHeaderHidden = false;
        m_bTaskCopyAllHidden = false;
        m_bTaskCopyMessageHidden = false;

        m_bItemSelected = true;
    }
    UpdateTaskPane();
}


void CViewMessages::UpdateTaskPane()
{
    wxASSERT(NULL != m_pTaskPane);

    m_pTaskPane->BeginTaskPage();

    m_pTaskPane->BeginTaskSection( wxT(SECTION_TASK), wxT(BITMAP_TASKHEADER), m_bTaskHeaderHidden );
    if (!m_bTaskHeaderHidden)
    {
        m_pTaskPane->CreateTask( wxT(LINK_TASKCOPYALL), wxT(BITMAP_MESSAGE), _("Copy All"), m_bTaskCopyAllHidden );
        m_pTaskPane->CreateTask( wxT(LINK_TASKCOPYMESSAGE), wxT(BITMAP_MESSAGE), _("Copy Message"), m_bTaskCopyMessageHidden );
    }
    m_pTaskPane->EndTaskSection( m_bTaskHeaderHidden );

    m_pTaskPane->UpdateQuickTip(wxT(SECTION_TIPS), wxT(BITMAP_TIPSHEADER), GetCurrentQuickTipText(), m_bTipsHeaderHidden);

    m_pTaskPane->EndTaskPage();
}

