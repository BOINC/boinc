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
#pragma implementation "ViewMessages.h"
#endif

#include "stdwx.h"
#include "BOINCGUIApp.h"
#include "MainDocument.h"
#include "BOINCTaskCtrl.h"
#include "BOINCListCtrl.h"
#include "ViewMessages.h"
#include "Events.h"

#include "wx/arrimpl.cpp" 


#include "res/mess.xpm"
#include "res/task.xpm"
#include "res/tips.xpm"


#define COLUMN_PROJECT              0
#define COLUMN_TIME                 1
#define COLUMN_MESSAGE              2

#define PRIORITY_INFO               1
#define PRIORITY_ERROR              2


IMPLEMENT_DYNAMIC_CLASS( CViewMessages, CBOINCBaseView )


CViewMessages::CViewMessages()
{
}


CViewMessages::CViewMessages(wxNotebook* pNotebook) :
    CBOINCBaseView( pNotebook, ID_HTML_MESSAGESVIEW, DEFAULT_HTML_FLAGS, ID_LIST_MESSAGESVIEW, DEFAULT_LIST_MULTI_SEL_FLAGS )
{
    wxASSERT(NULL != m_pTaskPane);
    wxASSERT(NULL != m_pListPane);

    //
    // Initialize variables used in later parts of the class
    //
    m_iPreviousDocCount = 0;


    //
    // Globalization/Localization
    //
    VIEW_HEADER              = wxT("mess");

    SECTION_TASK             = VIEW_HEADER + wxT("task");
    SECTION_TIPS             = VIEW_HEADER + wxT("tips");

    BITMAP_MESSAGE           = VIEW_HEADER + wxT(".xpm");
    BITMAP_TASKHEADER        = SECTION_TASK + wxT(".xpm");
    BITMAP_TIPSHEADER        = SECTION_TIPS + wxT(".xpm");

    LINKDESC_DEFAULT         = 
        _("Click a message to see additional options.");

    LINK_TASKCOPYALL         = SECTION_TASK + wxT("copyall");
    LINKDESC_TASKCOPYALL     = 
        _("<b>Copy all</b><br>"
          "Copy all the messages to the clipboard.");

    LINK_TASKCOPYMESSAGE     = SECTION_TASK + wxT("copymessage");
    LINKDESC_TASKCOPYMESSAGE = 
        _("<b>Copy selected messages</b><br>"
          "Copy the selected messages to the clipboard. "
          "You can select multiple messages by holding down the shift "
          "or control key while clicking on messages.");

    //
    // Setup View
    //
    wxBitmap bmpMessage(mess_xpm);
    wxBitmap bmpTask(task_xpm);
    wxBitmap bmpTips(tips_xpm);

    bmpMessage.SetMask(new wxMask(bmpMessage, wxColour(255, 0, 255)));
    bmpTask.SetMask(new wxMask(bmpTask, wxColour(255, 0, 255)));
    bmpTips.SetMask(new wxMask(bmpTips, wxColour(255, 0, 255)));

    m_pTaskPane->AddVirtualFile(BITMAP_MESSAGE, bmpMessage, wxBITMAP_TYPE_XPM);

    m_pTaskPane->CreateTaskHeader(BITMAP_TASKHEADER, bmpTask, _("Tasks"));
    m_pTaskPane->CreateTaskHeader(BITMAP_TIPSHEADER, bmpTips, _("Tips"));

    m_pListPane->InsertColumn(COLUMN_PROJECT, _("Project"), wxLIST_FORMAT_LEFT, 115);
    m_pListPane->InsertColumn(COLUMN_TIME, _("Time"), wxLIST_FORMAT_LEFT, 145);
    m_pListPane->InsertColumn(COLUMN_MESSAGE, _("Message"), wxLIST_FORMAT_LEFT, 550);

    m_pMessageInfoAttr = new wxListItemAttr( *wxBLACK, *wxWHITE, wxNullFont );
    m_pMessageErrorAttr = new wxListItemAttr( *wxRED, *wxWHITE, wxNullFont );

    m_bTipsHeaderHidden = false;
    m_bItemSelected = false;

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


const char** CViewMessages::GetViewIcon()
{
    return mess_xpm;
}


wxInt32 CViewMessages::GetDocCount()
{
    CMainDocument* pDoc      = wxGetApp().GetDocument();

    wxASSERT(NULL != pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));

    return pDoc->GetMessageCount();
}


void CViewMessages::OnListRender ( wxTimerEvent& event )
{
    if (!m_bProcessingListRenderEvent)
    {
        m_bProcessingListRenderEvent = true;

        wxASSERT(NULL != m_pListPane);

        wxInt32 iDocCount = GetDocCount();
        if ( 0 >= iDocCount )
        {
            m_pListPane->DeleteAllItems();
        }
        else
        {
            if ( m_iPreviousDocCount != iDocCount )
                m_pListPane->SetItemCount( iDocCount );
        }

        if ( (iDocCount) && (_EnsureLastItemVisible()) && (m_iPreviousDocCount != iDocCount) )
            m_pListPane->EnsureVisible( iDocCount - 1 );

        if ( m_iPreviousDocCount != iDocCount )
            m_iPreviousDocCount = iDocCount;

        m_bProcessingListRenderEvent = false;
    }

    event.Skip();
}


wxString CViewMessages::OnListGetItemText( long item, long column ) const
{
    wxString        strBuffer   = wxEmptyString;

    switch(column)
    {
        case COLUMN_PROJECT:
            FormatProjectName( item, strBuffer );
            break;
        case COLUMN_TIME:
            FormatTime( item, strBuffer );
            break;
        case COLUMN_MESSAGE:
            FormatMessage( item, strBuffer );
            break;
    }

    return strBuffer;
}


wxListItemAttr* CViewMessages::OnListGetItemAttr( long item ) const
{
    wxListItemAttr* pAttribute  = NULL;
    wxString        strBuffer   = wxEmptyString;

    FormatPriority( item, strBuffer );

    if ( wxT("E") == strBuffer )
    {
        pAttribute = m_pMessageErrorAttr;
    }
    else
    {
        pAttribute = m_pMessageInfoAttr;
    }

    return pAttribute;
}


void CViewMessages::OnTaskLinkClicked( const wxHtmlLinkInfo& link )
{
    wxInt32 iIndex = -1;

    wxASSERT(NULL != m_pTaskPane);
    wxASSERT(NULL != m_pListPane);

    m_bTaskHeaderHidden = false;
    m_bTipsHeaderHidden = false;

#ifndef NOCLIPBOARD
    if ( link.GetHref() == LINK_TASKCOPYALL )
    {
        wxInt32 iRowCount = 0;

        OpenClipboard();

        iRowCount = m_pListPane->GetItemCount();
        for ( iIndex = 0; iIndex < iRowCount; iIndex++ )
        {
            CopyToClipboard( iIndex );            
        }

        CloseClipboard();
    }
    else if ( link.GetHref() == LINK_TASKCOPYMESSAGE )
    {
        OpenClipboard();

        for ( ;; )
        {
            iIndex = m_pListPane->GetNextItem(iIndex,
                                        wxLIST_NEXT_ALL,
                                        wxLIST_STATE_SELECTED);
            if ( iIndex == -1 )
                break;

            CopyToClipboard( iIndex );            
        }

        CloseClipboard();
    }
#endif

    UpdateSelection();
}


void CViewMessages::OnTaskCellMouseHover( wxHtmlCell* cell, wxCoord WXUNUSED(x), wxCoord WXUNUSED(y) )
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


bool CViewMessages::EnsureLastItemVisible()
{
    return true;
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

    m_pTaskPane->BeginTaskSection( BITMAP_TASKHEADER, m_bTaskHeaderHidden );
    if (!m_bTaskHeaderHidden)
    {
        m_pTaskPane->CreateTask( LINK_TASKCOPYALL, _("Copy all messages"), m_bTaskCopyAllHidden );
        m_pTaskPane->CreateTask( LINK_TASKCOPYMESSAGE, _("Copy selected messages"), m_bTaskCopyMessageHidden );
    }
    m_pTaskPane->EndTaskSection( m_bTaskHeaderHidden );

    m_pTaskPane->UpdateQuickTip( BITMAP_TIPSHEADER, GetCurrentQuickTipText(), m_bTipsHeaderHidden );

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


wxInt32 CViewMessages::FormatPriority( wxInt32 item, wxString& strBuffer ) const
{
    CMainDocument*  pDoc = wxGetApp().GetDocument();
    wxInt32         iBuffer = 0;

    wxASSERT(NULL != pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));

    strBuffer.Clear();

    pDoc->GetMessagePriority(item, iBuffer);

    switch(iBuffer)
    {
        case PRIORITY_INFO:
            strBuffer = wxT("I");
            break;
        case PRIORITY_ERROR:
            strBuffer = wxT("E");
            break;
    }

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


#ifndef NOCLIPBOARD
bool CViewMessages::OpenClipboard()
{
    bool bRetVal = false;

    bRetVal = wxTheClipboard->Open();
    if ( bRetVal )
    {
        m_bClipboardOpen = true;
        m_strClipboardData = wxEmptyString;
        wxTheClipboard->Clear();
    }

    return bRetVal;
}


wxInt32 CViewMessages::CopyToClipboard( wxInt32 item )
{
    wxInt32        iRetVal = -1;

    if ( m_bClipboardOpen )
    {
        wxString       strBuffer = wxEmptyString;
        wxString       strTimeStamp = wxEmptyString;
        wxString       strProject = wxEmptyString;
        wxString       strMessage = wxEmptyString;

        FormatTime( item, strTimeStamp );
        FormatProjectName( item, strProject );
        FormatMessage( item, strMessage );

#ifdef __WXMSW__
        strBuffer.Printf( wxT("%s|%s|%s\r\n"), strTimeStamp.c_str(), strProject.c_str(), strMessage.c_str() );
#else
        strBuffer.Printf( wxT("%s|%s|%s\n"), strTimeStamp.c_str(), strProject.c_str(), strMessage.c_str() );
#endif

        m_strClipboardData += strBuffer;

        iRetVal = 0;
    }

    return iRetVal;
}


bool CViewMessages::CloseClipboard()
{
    bool bRetVal = false;

    if ( m_bClipboardOpen )
    {
        wxTheClipboard->SetData( new wxTextDataObject( m_strClipboardData ) );
        wxTheClipboard->Close();

        m_bClipboardOpen = false;
        m_strClipboardData = wxEmptyString;
    }

    return bRetVal;
}

#endif

const char *BOINC_RCSID_0be7149475 = "$Id$";
