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


#define VIEW_HEADER                 wxT("mess")

#define SECTION_TASK                wxT(VIEW_HEADER "task")
#define SECTION_TIPS                wxT(VIEW_HEADER "tips")

#define BITMAP_MESSAGE              wxT(VIEW_HEADER ".xpm")
#define BITMAP_TASKHEADER           wxT(SECTION_TASK ".xpm")
#define BITMAP_TIPSHEADER           wxT(SECTION_TIPS ".xpm")

#define COLUMN_PROJECT              0
#define COLUMN_TIME                 1
#define COLUMN_MESSAGE              2

#define PRIORITY_INFO               1
#define PRIORITY_ERROR              2


const wxString LINK_DEFAULT             = wxT("default");
const wxString LINKDESC_DEFAULT         = 
     _("Please click a message to see additional options.");

const wxString LINK_TASKCOPYALL         = wxT(SECTION_TASK "copyall");
const wxString LINKDESC_TASKCOPYALL     = 
     _("<b>Copy All</b><br>"
       "Clicking copy all, copies all the messages to the system clipboard.");

const wxString LINK_TASKCOPYMESSAGE     = wxT(SECTION_TASK "copymessage");
const wxString LINKDESC_TASKCOPYMESSAGE = 
     _("<b>Copy Message</b><br>"
       "Clicking copy message, copies the single message to the system clipboard.");


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

    m_pTaskPane->AddVirtualFile(BITMAP_MESSAGE, bmpMessage, wxBITMAP_TYPE_XPM);

    m_pTaskPane->CreateTaskHeader(BITMAP_TASKHEADER, bmpTask, _("Tasks"));
    m_pTaskPane->CreateTaskHeader(BITMAP_TIPSHEADER, bmpTips, _("Quick Tips"));

    m_pListPane->InsertColumn(COLUMN_PROJECT, _("Project"), wxLIST_FORMAT_LEFT, 115);
    m_pListPane->InsertColumn(COLUMN_TIME, _("Time"), wxLIST_FORMAT_LEFT, 145);
    m_pListPane->InsertColumn(COLUMN_MESSAGE, _("Message"), wxLIST_FORMAT_LEFT, 550);

    m_pMessageInfoAttr = new wxListItemAttr( *wxBLACK, *wxWHITE, wxNullFont );
    m_pMessageErrorAttr = new wxListItemAttr( *wxRED, *wxWHITE, wxNullFont );

    m_bTipsHeaderHidden = false;

    SetCurrentQuickTip(
        LINK_DEFAULT, 
        LINKDESC_DEFAULT
    );

    UpdateSelection();
}


CViewMessages::~CViewMessages()
{
    if ( m_pMessageInfoAttr )
    {
        delete m_pMessageInfoAttr;
        m_pMessageInfoAttr = NULL;
    }

    if ( m_pMessageErrorAttr )
    {
        delete m_pMessageErrorAttr;
        m_pMessageErrorAttr = NULL;
    }
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


void CViewMessages::OnListRender(wxTimerEvent &event)
{
    if (!m_bProcessingListRenderEvent)
    {
        m_bProcessingListRenderEvent = true;

        CMainDocument*  pDoc = wxGetApp().GetDocument();

        wxASSERT(NULL != pDoc);
        wxASSERT(wxDynamicCast(pDoc, CMainDocument));
        wxASSERT(NULL != m_pListPane);

        wxInt32 iCount = pDoc->GetMessageCount();
        if ( iCount != m_iCount )
        {
            m_iCount = iCount;
            m_pListPane->SetItemCount(iCount);
            m_pListPane->EnsureVisible(iCount-1);  
        }
        else
        {
            wxListItem liListItemMessage;
            wxString   strListItemMessage;
            wxString   strDocumentItemMessage;

            FormatMessage(m_iCacheTo, strDocumentItemMessage);

            liListItemMessage.SetId(m_iCacheTo);
            liListItemMessage.SetColumn(COLUMN_MESSAGE);
            liListItemMessage.SetMask(wxLIST_MASK_TEXT);

            m_pListPane->GetItem(liListItemMessage);

            strListItemMessage = liListItemMessage.GetText();

            if ( !strDocumentItemMessage.IsSameAs(strListItemMessage) )
            {
                m_pListPane->RefreshItems(m_iCacheFrom, m_iCacheTo);
                m_pListPane->EnsureVisible(m_iCacheTo);
            }
        }

        m_bProcessingListRenderEvent = false;
    }

    m_pListPane->Refresh();

    event.Skip();
}


void CViewMessages::OnListSelected ( wxListEvent& event )
{
    SetCurrentQuickTip(
        LINK_DEFAULT, 
        wxT("")
    );

    UpdateSelection();
    event.Skip();
}


void CViewMessages::OnListDeselected ( wxListEvent& event )
{
    SetCurrentQuickTip(
        LINK_DEFAULT, 
        wxT("")
    );

    UpdateSelection();
    event.Skip();
}


wxString CViewMessages::OnListGetItemText( long item, long column ) const
{
    wxString   strBuffer;

    switch(column)
    {
        case COLUMN_PROJECT:
            if (item == m_iCacheFrom) wxGetApp().GetDocument()->CachedStateLock();
            FormatProjectName( item, strBuffer );
            break;
        case COLUMN_TIME:
            FormatTime( item, strBuffer );
            break;
        case COLUMN_MESSAGE:
            FormatMessage( item, strBuffer );
            if (item == m_iCacheTo) wxGetApp().GetDocument()->CachedStateUnlock();
            break;
    }

    return strBuffer;
}


wxListItemAttr* CViewMessages::OnListGetItemAttr( long item ) const
{
    wxListItemAttr* pAttribute = NULL;
    wxInt32 iBuffer = 0;

    wxGetApp().GetDocument()->GetMessagePriority(item, iBuffer);

    switch(iBuffer)
    {
        case PRIORITY_INFO:
            pAttribute = m_pMessageInfoAttr;
            break;
        case PRIORITY_ERROR:
            pAttribute = m_pMessageErrorAttr;
            break;
    }

    return pAttribute;
}


void CViewMessages::OnTaskLinkClicked( const wxHtmlLinkInfo& link )
{
    wxASSERT(NULL != m_pTaskPane);
    wxASSERT(NULL != m_pListPane);

    wxString strMessage;

    if ( link.GetHref() == SECTION_TASK )
        m_bTaskHeaderHidden ? m_bTaskHeaderHidden = false : m_bTaskHeaderHidden = true;

    if ( link.GetHref() == SECTION_TIPS )
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

        if      ( UpdateQuickTip( strLink, LINK_TASKCOPYALL, LINKDESC_TASKCOPYALL ) )
            bUpdateSelection = true;
        else if ( UpdateQuickTip( strLink, LINK_TASKCOPYMESSAGE, LINKDESC_TASKCOPYMESSAGE ) )
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


void CViewMessages::UpdateSelection()
{
    wxASSERT(NULL != m_pTaskPane);
    wxASSERT(NULL != m_pListPane);

    if ( 0 == m_pListPane->GetSelectedItemCount() )
    {
        m_bTaskHeaderHidden = false;
        m_bTaskCopyAllHidden = false;
        m_bTaskCopyMessageHidden = true;

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

    m_pTaskPane->BeginTaskSection( SECTION_TASK, BITMAP_TASKHEADER, m_bTaskHeaderHidden );
    if (!m_bTaskHeaderHidden)
    {
        m_pTaskPane->CreateTask( LINK_TASKCOPYALL, BITMAP_MESSAGE, _("Copy All"), m_bTaskCopyAllHidden );
        m_pTaskPane->CreateTask( LINK_TASKCOPYMESSAGE, BITMAP_MESSAGE, _("Copy Message"), m_bTaskCopyMessageHidden );
    }
    m_pTaskPane->EndTaskSection( m_bTaskHeaderHidden );

    m_pTaskPane->UpdateQuickTip( SECTION_TIPS, BITMAP_TIPSHEADER, GetCurrentQuickTipText(), m_bTipsHeaderHidden );

    m_pTaskPane->EndTaskPage();
}


wxInt32 CViewMessages::FormatProjectName( wxInt32 item, wxString& strBuffer ) const
{
    CMainDocument* pDoc = wxGetApp().GetDocument();

    wxASSERT(NULL != pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));

    strBuffer.Clear();

    pDoc->GetMessageProjectName(item, strBuffer);

    return 0;
}


wxInt32 CViewMessages::FormatTime( wxInt32 item, wxString& strBuffer ) const
{
    wxDateTime     dtBuffer(wxDateTime::Now());
    CMainDocument* pDoc = wxGetApp().GetDocument();

    wxASSERT(NULL != pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));

    strBuffer.Clear();

    pDoc->GetMessageTime(item, dtBuffer);
    strBuffer = dtBuffer.Format();

    return 0;
}


wxInt32 CViewMessages::FormatMessage( wxInt32 item, wxString& strBuffer ) const
{
    CMainDocument* pDoc = wxGetApp().GetDocument();

    wxASSERT(NULL != pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));

    strBuffer.Clear();

    pDoc->GetMessageMessage(item, strBuffer);

    strBuffer.Replace( wxT("\n"), wxT(""), true );

    return 0;
}

