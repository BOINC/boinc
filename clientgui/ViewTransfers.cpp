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
#pragma implementation "ViewTransfers.h"
#endif

#include "stdwx.h"
#include "BOINCGUIApp.h"
#include "MainDocument.h"
#include "BOINCTaskCtrl.h"
#include "BOINCListCtrl.h"
#include "ViewTransfers.h"
#include "Events.h"
#include "error_numbers.h"

#include "res/xfer.xpm"
#include "res/task.xpm"
#include "res/tips.xpm"

#define VIEW_HEADER                 wxT("xfer")

#define SECTION_TASK                wxT(VIEW_HEADER "task")
#define SECTION_TIPS                wxT(VIEW_HEADER "tips")

#define BITMAP_TRANSFER             wxT(VIEW_HEADER ".xpm")
#define BITMAP_TASKHEADER           wxT(SECTION_TASK ".xpm")
#define BITMAP_TIPSHEADER           wxT(SECTION_TIPS ".xpm")

#define COLUMN_PROJECT              0
#define COLUMN_FILE                 1
#define COLUMN_PROGRESS             2
#define COLUMN_SIZE                 3
#define COLUMN_TIME                 4
#define COLUMN_SPEED                5
#define COLUMN_STATUS               6


const wxString LINK_DEFAULT             = wxT("default");
const wxString LINKDESC_DEFAULT         = 
     _("Please click a transfer item to see additional options.");

const wxString LINK_TASKRETRY           = wxT(SECTION_TASK "retry");
const wxString LINKDESC_TASKRETRY       = 
     _("<b>Retry Now</b><br>"
       "Clicking retry now will attempt to upload the result data file "
       "to the project server now.");

const wxString LINK_TASKABORT           = wxT(SECTION_TASK "abort");
const wxString LINKDESC_TASKABORT       = 
     _("<b>Abort Upload</b><br>"
       "Clicking abort upload will delete the result from the upload queue. "
       "Doing this will keep you from being granted any credit for this result.");


IMPLEMENT_DYNAMIC_CLASS(CViewTransfers, CBOINCBaseView)


CViewTransfers::CViewTransfers()
{
}


CViewTransfers::CViewTransfers(wxNotebook* pNotebook) :
    CBOINCBaseView(pNotebook, ID_HTML_TRANSFERSVIEW, ID_LIST_TRANSFERSVIEW)
{
    m_bItemSelected = false;

    wxASSERT(NULL != m_pTaskPane);
    wxASSERT(NULL != m_pListPane);

    wxBitmap bmpTransfer(xfer_xpm);
    wxBitmap bmpTask(task_xpm);
    wxBitmap bmpTips(tips_xpm);

    bmpTransfer.SetMask(new wxMask(bmpTransfer, wxColour(255, 0, 255)));
    bmpTask.SetMask(new wxMask(bmpTask, wxColour(255, 0, 255)));
    bmpTips.SetMask(new wxMask(bmpTips, wxColour(255, 0, 255)));

    m_pTaskPane->AddVirtualFile(BITMAP_TRANSFER, bmpTransfer, wxBITMAP_TYPE_XPM);

    m_pTaskPane->CreateTaskHeader(BITMAP_TASKHEADER, bmpTask, _("Tasks"));
    m_pTaskPane->CreateTaskHeader(BITMAP_TIPSHEADER, bmpTips, _("Quick Tips"));

    m_pListPane->InsertColumn(COLUMN_PROJECT, _("Project"), wxLIST_FORMAT_LEFT, 125);
    m_pListPane->InsertColumn(COLUMN_FILE, _("File"), wxLIST_FORMAT_LEFT, 205);
    m_pListPane->InsertColumn(COLUMN_PROGRESS, _("Progress"), wxLIST_FORMAT_CENTRE, 60);
    m_pListPane->InsertColumn(COLUMN_SIZE, _("Size"), wxLIST_FORMAT_LEFT, 80);
    m_pListPane->InsertColumn(COLUMN_TIME, _("Time"), wxLIST_FORMAT_LEFT, 80);
    m_pListPane->InsertColumn(COLUMN_SPEED, _("Speed"), wxLIST_FORMAT_LEFT, 80);
    m_pListPane->InsertColumn(COLUMN_STATUS, _("Status"), wxLIST_FORMAT_LEFT, 150);

    m_bTipsHeaderHidden = false;

    SetCurrentQuickTip(
        LINK_DEFAULT, 
        LINKDESC_DEFAULT
    );

    UpdateSelection();
}


CViewTransfers::~CViewTransfers()
{
}


wxString CViewTransfers::GetViewName()
{
    return wxString(_("Transfers"));
}


char** CViewTransfers::GetViewIcon()
{
    return xfer_xpm;
}


void CViewTransfers::OnTaskRender(wxTimerEvent &event)
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


void CViewTransfers::OnListRender(wxTimerEvent &event)
{
    if (!m_bProcessingListRenderEvent)
    {
        m_bProcessingListRenderEvent = true;

        wxASSERT(NULL != m_pListPane);

        wxInt32 iCount = wxGetApp().GetDocument()->GetTransferCount();
        if ( iCount != m_iCount )
        {
            m_iCount = iCount;
            if ( 0 >= iCount )
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
                                FormatProjectName( iRowIndex, strDocumentText );
                                break;
                            case COLUMN_FILE:
                                FormatFileName( iRowIndex, strDocumentText );
                                break;
                            case COLUMN_PROGRESS:
                                FormatProgress( iRowIndex, strDocumentText );
                                break;
                            case COLUMN_SIZE:
                                FormatSize( iRowIndex, strDocumentText );
                                break;
                            case COLUMN_TIME:
                                FormatTime( iRowIndex, strDocumentText );
                                break;
                            case COLUMN_SPEED:
                                FormatSpeed( iRowIndex, strDocumentText );
                                break;
                            case COLUMN_STATUS:
                                FormatStatus( iRowIndex, strDocumentText );
                                break;
                        }

                        liItem.SetColumn(iColumnIndex);
                        m_pListPane->GetItem(liItem);
                        strListPaneText = liItem.GetText();

                        if ( !strDocumentText.IsSameAs(strListPaneText) )
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


void CViewTransfers::OnListSelected ( wxListEvent& event )
{
    SetCurrentQuickTip(
        LINK_DEFAULT, 
        wxT("")
    );

    UpdateSelection();
    event.Skip();
}


void CViewTransfers::OnListDeselected ( wxListEvent& event )
{
    SetCurrentQuickTip(
        LINK_DEFAULT, 
        wxT("")
    );

    UpdateSelection();
    event.Skip();
}


wxString CViewTransfers::OnListGetItemText(long item, long column) const
{
    wxString       strBuffer = wxEmptyString;
    CMainDocument* pDoc      = wxGetApp().GetDocument();

    wxASSERT(NULL != pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));

    switch(column)
    {
        case COLUMN_PROJECT:
            if (item == m_iCacheFrom) pDoc->CachedStateLock();
            FormatProjectName( item, strBuffer );
            break;
        case COLUMN_FILE:
            FormatFileName( item, strBuffer );
            break;
        case COLUMN_PROGRESS:
            FormatProgress( item, strBuffer );
            break;
        case COLUMN_SIZE:
            FormatSize( item, strBuffer );
            break;
        case COLUMN_TIME:
            FormatTime( item, strBuffer );
            break;
        case COLUMN_SPEED:
            FormatSpeed( item, strBuffer );
            break;
        case COLUMN_STATUS:
            FormatStatus( item, strBuffer );
            if (item == m_iCacheTo) pDoc->CachedStateUnlock();
            break;
    }
    return strBuffer;
}


void CViewTransfers::OnTaskLinkClicked( const wxHtmlLinkInfo& link )
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


void CViewTransfers::OnTaskCellMouseHover( wxHtmlCell* cell, wxCoord x, wxCoord y )
{
    if ( NULL != cell->GetLink() )
    {
        bool        bUpdateSelection = false;
        wxString    strLink;

        strLink = cell->GetLink()->GetHref();

        if      ( UpdateQuickTip( strLink, LINK_TASKRETRY, LINKDESC_TASKRETRY ) )
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


void CViewTransfers::UpdateSelection()
{
    wxASSERT(NULL != m_pTaskPane);
    wxASSERT(NULL != m_pListPane);

    if ( 0 == m_pListPane->GetSelectedItemCount() )
    {
        m_bTaskHeaderHidden = true;
        m_bTaskRetryHidden = true;
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
        m_bTaskRetryHidden = false;
        m_bTaskAbortHidden = false;

        m_bItemSelected = true;
    }
    UpdateTaskPane();
}


void CViewTransfers::UpdateTaskPane()
{
    wxASSERT(NULL != m_pTaskPane);

    m_pTaskPane->BeginTaskPage();

    m_pTaskPane->BeginTaskSection( SECTION_TASK, BITMAP_TASKHEADER, m_bTaskHeaderHidden );
    if (!m_bTaskHeaderHidden)
    {
        m_pTaskPane->CreateTask( LINK_TASKRETRY, BITMAP_TRANSFER, _("Retry Now"), m_bTaskRetryHidden );
        m_pTaskPane->CreateTask( LINK_TASKABORT, BITMAP_TRANSFER, _("Abort Upload"), m_bTaskAbortHidden );
    }
    m_pTaskPane->EndTaskSection( m_bTaskHeaderHidden );

    m_pTaskPane->UpdateQuickTip( SECTION_TIPS, BITMAP_TIPSHEADER, GetCurrentQuickTipText(), m_bTipsHeaderHidden );

    m_pTaskPane->EndTaskPage();
}


wxInt32 CViewTransfers::FormatProjectName( wxInt32 item, wxString& strBuffer ) const
{
    CMainDocument* pDoc = wxGetApp().GetDocument();

    wxASSERT(NULL != pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));

    strBuffer.Clear();

    pDoc->GetTransferProjectName(item, strBuffer);

    return 0;
}


wxInt32 CViewTransfers::FormatFileName( wxInt32 item, wxString& strBuffer ) const
{
    CMainDocument* pDoc = wxGetApp().GetDocument();

    wxASSERT(NULL != pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));

    strBuffer.Clear();

    pDoc->GetTransferFileName(item, strBuffer);

    return 0;
}


wxInt32 CViewTransfers::FormatProgress( wxInt32 item, wxString& strBuffer ) const
{
    float          fBytesSent = 0;
    float          fFileSize = 0;
    CMainDocument* pDoc = wxGetApp().GetDocument();

    wxASSERT(NULL != pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));

    strBuffer.Clear();

    if ( pDoc->IsTransferActive(item) )
        pDoc->GetTransferBytesXfered( item, fBytesSent );
    else
        pDoc->GetTransferFileSize( item, fBytesSent );

    pDoc->GetTransferFileSize( item, fFileSize );

    strBuffer.Printf(wxT("%.2f%%"), ( 100 * ( fBytesSent / fFileSize ) ) );

    return 0;
}


wxInt32 CViewTransfers::FormatSize( wxInt32 item, wxString& strBuffer ) const
{
    float          fBytesSent = 0;
    float          fFileSize = 0;
    double         xTera = 1099511627776.0;
    double         xGiga = 1073741824.0;
    double         xMega = 1048576.0;
    double         xKilo = 1024.0;
    CMainDocument* pDoc = wxGetApp().GetDocument();

    wxASSERT(NULL != pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));

    strBuffer.Clear();

    if ( pDoc->IsTransferActive(item) )
        pDoc->GetTransferBytesXfered( item, fBytesSent );
    else
        pDoc->GetTransferFileSize( item, fBytesSent );

    pDoc->GetTransferFileSize( item, fFileSize );

    if (fFileSize != 0)
    {
        if      ( fFileSize >= xTera )
        {
            strBuffer.Printf( wxT("%0.2f/%0.2f TB"), fBytesSent/xTera, fFileSize/xTera);
        }
        else if ( fFileSize >= xGiga )
        {
            strBuffer.Printf( wxT("%0.2f/%0.2f GB"), fBytesSent/xGiga, fFileSize/xGiga);
        }
        else if ( fFileSize >= xMega )
        {
            strBuffer.Printf( wxT("%0.2f/%0.2f MB"), fBytesSent/xMega, fFileSize/xMega);
        }
        else if ( fFileSize >= xKilo )
        {
            strBuffer.Printf( wxT("%0.2f/%0.2f KB"), fBytesSent/xKilo, fFileSize/xKilo);
        }
        else
        {
            strBuffer.Printf( wxT("%0.0f/%0.0f bytes"), fBytesSent, fFileSize);
        }
    }
    else
    {
        if      ( fBytesSent >= xTera )
        {
            strBuffer.Printf( wxT("%0.2f TB"), fBytesSent/xTera);
        }
        else if ( fBytesSent >= xGiga )
        {
            strBuffer.Printf( wxT("%0.2f GB"), fBytesSent/xGiga);
        }
        else if ( fBytesSent >= xMega )
        {
            strBuffer.Printf( wxT("%0.2f MB"), fBytesSent/xMega);
        }
        else if ( fBytesSent >= xKilo )
        {
            strBuffer.Printf( wxT("%0.2f KB"), fBytesSent/xKilo);
        }
        else
        {
            strBuffer.Printf( wxT("%0.0f bytes"), fBytesSent);
        }
    }

    return 0;
}


wxInt32 CViewTransfers::FormatTime( wxInt32 item, wxString& strBuffer ) const
{
    float          fBuffer = 0;
    int            xhour = 0;
    int            xmin = 0;
    int            xsec = 0;
    CMainDocument* pDoc = wxGetApp().GetDocument();

    wxASSERT(NULL != pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));

    strBuffer.Clear();

    pDoc->GetTransferTime(item, fBuffer);

    xhour = (int)(fBuffer / (60 * 60));
    xmin = (int)(fBuffer / 60) % 60;
    xsec = (int)(fBuffer) % 60;

    strBuffer.Printf(wxT("%0.2d:%0.2d:%0.2d"), xhour, xmin, xsec);

    return 0;
}


wxInt32 CViewTransfers::FormatSpeed( wxInt32 item, wxString& strBuffer ) const
{
    float          fTransferSpeed = 0;
    CMainDocument* pDoc = wxGetApp().GetDocument();

    wxASSERT(NULL != pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));

    strBuffer.Clear();

    if ( pDoc->IsTransferActive(item) )
    {
        pDoc->GetTransferSpeed( item, fTransferSpeed );
        strBuffer.Printf( wxT("%0.2f KBps"), ( fTransferSpeed / 1024 ) );
    }
    else
        strBuffer = wxT("0.00 KBps");

    return 0;
}


wxInt32 CViewTransfers::FormatStatus( wxInt32 item, wxString& strBuffer ) const
{
    int            iTime = 0;
    int            iStatus = 0;
    CMainDocument* pDoc = wxGetApp().GetDocument();

    wxASSERT(NULL != pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));

    strBuffer.Clear();

    pDoc->GetTransferNextRequestTime( item, iTime );
    pDoc->GetTransferStatus( item, iStatus );

    wxDateTime dtNextRequest( (time_t)iTime );
    wxDateTime dtNow(wxDateTime::Now());

    if      ( dtNextRequest > dtNow )
    {
        wxTimeSpan tsNextRequest(dtNextRequest - dtNow);
        strBuffer = _("Retry in ") + tsNextRequest.Format();
    }
    else if ( ERR_GIVEUP_DOWNLOAD == iStatus )
    {
        strBuffer = _("Download failed");
    }
    else if ( ERR_GIVEUP_UPLOAD == iStatus )
    {
        strBuffer = _("Upload failed");
    }
    else
    {
        strBuffer = pDoc->IsTransferGeneratedLocally( item )? _("Uploading") : _("Downloading");
    }

    return 0;
}

