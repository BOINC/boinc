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


#define COLUMN_PROJECT              0
#define COLUMN_FILE                 1
#define COLUMN_PROGRESS             2
#define COLUMN_SIZE                 3
#define COLUMN_TIME                 4
#define COLUMN_SPEED                5
#define COLUMN_STATUS               6


CTransfer::CTransfer()
{
    m_strProjectName = wxEmptyString;
    m_strFileName = wxEmptyString;
    m_strProgress = wxEmptyString;
    m_strSize = wxEmptyString;
    m_strTime = wxEmptyString;
    m_strSpeed = wxEmptyString;
    m_strStatus = wxEmptyString;
}


CTransfer::~CTransfer()
{
    m_strProjectName.Clear();
    m_strFileName.Clear();
    m_strProgress.Clear();
    m_strSize.Clear();
    m_strTime.Clear();
    m_strSpeed.Clear();
    m_strStatus.Clear();
}


wxInt32 CTransfer::GetProjectName( wxString& strProjectName )
{
    strProjectName = m_strProjectName;	
	return 0;
}


wxInt32 CTransfer::GetFileName( wxString& strFileName )
{
    strFileName = m_strFileName;	
	return 0;
}


wxInt32 CTransfer::GetProgress( wxString& strProgress )
{
    strProgress = m_strProgress;	
	return 0;
}


wxInt32 CTransfer::GetSize( wxString& strSize )
{
    strSize = m_strSize;	
	return 0;
}


wxInt32 CTransfer::GetTime( wxString& strTime )
{
    strTime = m_strTime;	
	return 0;
}


wxInt32 CTransfer::GetSpeed( wxString& strSpeed )
{
    strSpeed = m_strSpeed;	
	return 0;
}


wxInt32 CTransfer::GetStatus( wxString& strStatus )
{
    strStatus = m_strStatus;	
	return 0;
}


wxInt32 CTransfer::SetProjectName( wxString& strProjectName )
{
    m_strProjectName = strProjectName;	
	return 0;
}


wxInt32 CTransfer::SetFileName( wxString& strFileName )
{
    m_strFileName = strFileName;	
	return 0;
}


wxInt32 CTransfer::SetProgress( wxString& strProgress )
{
    m_strProgress = strProgress;	
	return 0;
}


wxInt32 CTransfer::SetSize( wxString& strSize )
{
    m_strSize = strSize;	
	return 0;
}


wxInt32 CTransfer::SetTime( wxString& strTime )
{
    m_strTime = strTime;	
	return 0;
}


wxInt32 CTransfer::SetSpeed( wxString& strSpeed )
{
    m_strSpeed = strSpeed;	
	return 0;
}


wxInt32 CTransfer::SetStatus( wxString& strStatus )
{
    m_strStatus = strStatus;	
	return 0;
}

	
IMPLEMENT_DYNAMIC_CLASS(CViewTransfers, CBOINCBaseView)


CViewTransfers::CViewTransfers()
{
}


CViewTransfers::CViewTransfers(wxNotebook* pNotebook) :
    CBOINCBaseView( pNotebook, ID_HTML_TRANSFERSVIEW, DEFAULT_HTML_FLAGS, ID_LIST_TRANSFERSVIEW, DEFAULT_LIST_SINGLE_SEL_FLAGS )
{
    wxASSERT(NULL != m_pTaskPane);
    wxASSERT(NULL != m_pListPane);

    //
    // Globalization/Localization
    //
    VIEW_HEADER              = wxT("xfer");

    SECTION_TASK             = VIEW_HEADER + wxT("task");
    SECTION_TIPS             = VIEW_HEADER + wxT("tips");

    BITMAP_TRANSFER          = VIEW_HEADER + wxT(".xpm");
    BITMAP_TASKHEADER        = SECTION_TASK + wxT(".xpm");
    BITMAP_TIPSHEADER        = SECTION_TIPS + wxT(".xpm");

    LINKDESC_DEFAULT         = 
        _("Click an item to see additional options.");

    LINK_TASKRETRY           = SECTION_TASK + wxT("retry");
    LINKDESC_TASKRETRY       = 
        _("<b>Retry now</b><br>"
        "Click <b>Retry now</b> to transfer the file now");

    LINK_TASKABORT           = SECTION_TASK + wxT("abort");
    LINKDESC_TASKABORT       = 
        _("<b>Abort transfer</b><br>"
        "Click <b>Abort transfer</b> to delete the file from the transfer queue. "
        "This will prevent you from being granted credit for this result.");


    //
    // Setup View
    //
    wxBitmap bmpTransfer(xfer_xpm);
    wxBitmap bmpTask(task_xpm);
    wxBitmap bmpTips(tips_xpm);

    bmpTransfer.SetMask(new wxMask(bmpTransfer, wxColour(255, 0, 255)));
    bmpTask.SetMask(new wxMask(bmpTask, wxColour(255, 0, 255)));
    bmpTips.SetMask(new wxMask(bmpTips, wxColour(255, 0, 255)));

    m_pTaskPane->AddVirtualFile(BITMAP_TRANSFER, bmpTransfer, wxBITMAP_TYPE_XPM);

    m_pTaskPane->CreateTaskHeader(BITMAP_TASKHEADER, bmpTask, _("Tasks"));
    m_pTaskPane->CreateTaskHeader(BITMAP_TIPSHEADER, bmpTips, _("Tips"));

    m_pListPane->InsertColumn(COLUMN_PROJECT, _("Project"), wxLIST_FORMAT_LEFT, 125);
    m_pListPane->InsertColumn(COLUMN_FILE, _("File"), wxLIST_FORMAT_LEFT, 205);
    m_pListPane->InsertColumn(COLUMN_PROGRESS, _("Progress"), wxLIST_FORMAT_CENTRE, 60);
    m_pListPane->InsertColumn(COLUMN_SIZE, _("Size"), wxLIST_FORMAT_LEFT, 80);
    m_pListPane->InsertColumn(COLUMN_TIME, _("Time"), wxLIST_FORMAT_LEFT, 80);
    m_pListPane->InsertColumn(COLUMN_SPEED, _("Speed"), wxLIST_FORMAT_LEFT, 80);
    m_pListPane->InsertColumn(COLUMN_STATUS, _("Status"), wxLIST_FORMAT_LEFT, 150);

    m_bTipsHeaderHidden = false;
    m_bItemSelected = false;

    SetCurrentQuickTip(
        LINK_DEFAULT, 
        LINKDESC_DEFAULT
    );

    UpdateSelection();
}


CViewTransfers::~CViewTransfers()
{
    EmptyCache();
}


wxString CViewTransfers::GetViewName()
{
    return wxString(_("Transfers"));
}


const char** CViewTransfers::GetViewIcon()
{
    return xfer_xpm;
}


wxInt32 CViewTransfers::GetDocCount()
{
    CMainDocument* pDoc      = wxGetApp().GetDocument();

    wxASSERT(NULL != pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));

    return pDoc->GetTransferCount();
}


wxString CViewTransfers::OnListGetItemText(long item, long column) const
{
    CTransfer* transfer   = m_TransferCache.at( item );
    wxString   strBuffer  = wxEmptyString;

    switch(column)
    {
        case COLUMN_PROJECT:
            transfer->GetProjectName( strBuffer );
            break;
        case COLUMN_FILE:
            transfer->GetFileName( strBuffer );
            break;
        case COLUMN_PROGRESS:
            transfer->GetProgress( strBuffer );
            break;
        case COLUMN_SIZE:
            transfer->GetSize( strBuffer );
            break;
        case COLUMN_TIME:
            transfer->GetTime( strBuffer );
            break;
        case COLUMN_SPEED:
            transfer->GetSpeed( strBuffer );
            break;
        case COLUMN_STATUS:
            transfer->GetStatus( strBuffer );
            break;
    }

    return strBuffer;
}


wxString CViewTransfers::OnDocGetItemText(long item, long column) const
{
    wxString       strBuffer = wxEmptyString;

    switch(column)
    {
        case COLUMN_PROJECT:
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
            break;
    }

    return strBuffer;
}


void CViewTransfers::OnTaskLinkClicked( const wxHtmlLinkInfo& link )
{
    wxInt32  iAnswer        = 0; 
    wxInt32  iProjectIndex  = 0; 
    wxInt32  iWebsiteIndex  = 0; 
    wxString strName        = wxEmptyString;
    wxString strMessage     = wxEmptyString;
    CMainDocument* pDoc     = wxGetApp().GetDocument();

    wxASSERT(NULL != pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));
    wxASSERT(NULL != m_pTaskPane);
    wxASSERT(NULL != m_pListPane);

    m_bTaskHeaderHidden = false;
    m_bTipsHeaderHidden = false;

    if ( link.GetHref() == LINK_TASKRETRY )
    {
        iProjectIndex = m_pListPane->GetFirstSelected();

        pDoc->TransferRetryNow(
            iProjectIndex 
        );
    }
    else if ( link.GetHref() == LINK_TASKABORT )
    {
        iProjectIndex = m_pListPane->GetFirstSelected();
        pDoc->GetTransferFileName(iProjectIndex, strName);

        strMessage.Printf(
            _("Are you sure you want to abort this file transfer '%s'?"), 
            strName.c_str());

        iAnswer = wxMessageBox(
            strMessage,
            _("Abort File Transfer"),
            wxYES_NO | wxICON_QUESTION, 
            this
        );

        if ( wxYES == iAnswer )
        {
            pDoc->TransferAbort(
                iProjectIndex
            );
        }
    }

    UpdateSelection();
    m_pListPane->Refresh();
}


void CViewTransfers::OnTaskCellMouseHover( wxHtmlCell* cell, wxCoord WXUNUSED(x), wxCoord WXUNUSED(y) )
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


wxInt32 CViewTransfers::AddCacheElement()
{
    CTransfer* pItem = new CTransfer();
    wxASSERT( NULL != pItem );
    if ( NULL != pItem )
    {
        m_TransferCache.push_back( pItem );
        return 0;
    }
    return -1;
}


wxInt32 CViewTransfers::EmptyCache()
{
    unsigned int i;
    for (i=0; i<m_TransferCache.size(); i++) {
        delete m_TransferCache[i];
    }
    m_TransferCache.clear();
    return 0;
}


wxInt32 CViewTransfers::GetCacheCount()
{
    return m_TransferCache.size();
}


wxInt32 CViewTransfers::RemoveCacheElement()
{
    delete m_TransferCache.back();
    m_TransferCache.erase( m_TransferCache.end() - 1 );
    return 0;
}


wxInt32 CViewTransfers::UpdateCache( long item, long column, wxString& strNewData )
{
    CTransfer* transfer   = m_TransferCache.at( item );

    switch(column)
    {
        case COLUMN_PROJECT:
            transfer->SetProjectName( strNewData );
            break;
        case COLUMN_FILE:
            transfer->SetFileName( strNewData );
            break;
        case COLUMN_PROGRESS:
            transfer->SetProgress( strNewData );
            break;
        case COLUMN_SIZE:
            transfer->SetSize( strNewData );
            break;
        case COLUMN_TIME:
            transfer->SetTime( strNewData );
            break;
        case COLUMN_SPEED:
            transfer->SetSpeed( strNewData );
            break;
        case COLUMN_STATUS:
            transfer->SetStatus( strNewData );
            break;
    }

    return 0;
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

    m_pTaskPane->BeginTaskSection( BITMAP_TASKHEADER, m_bTaskHeaderHidden );
    if (!m_bTaskHeaderHidden)
    {
        m_pTaskPane->CreateTask( LINK_TASKRETRY, _("Retry Now"), m_bTaskRetryHidden );
        m_pTaskPane->CreateTask( LINK_TASKABORT, _("Abort Transfer"), m_bTaskAbortHidden );
    }
    m_pTaskPane->EndTaskSection( m_bTaskHeaderHidden );

    m_pTaskPane->UpdateQuickTip( BITMAP_TIPSHEADER, GetCurrentQuickTipText(), m_bTipsHeaderHidden );

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
    wxInt32        iHour = 0;
    wxInt32        iMin = 0;
    wxInt32        iSec = 0;
    wxTimeSpan     ts;
    CMainDocument* pDoc = wxGetApp().GetDocument();

    wxASSERT(NULL != pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));

    strBuffer.Clear();

    pDoc->GetTransferTime(item, fBuffer);

    iHour = (wxInt32)(fBuffer / (60 * 60));
    iMin  = (wxInt32)(fBuffer / 60) % 60;
    iSec  = (wxInt32)(fBuffer) % 60;

    ts = wxTimeSpan( iHour, iMin, iSec );

    strBuffer = ts.Format();

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
        strBuffer.Printf(wxT("%.2f KBps"), 0.0);

    return 0;
}


wxInt32 CViewTransfers::FormatStatus( wxInt32 item, wxString& strBuffer ) const
{
    wxInt32        iTime = 0;
    wxInt32        iStatus = 0;
    CMainDocument* pDoc = wxGetApp().GetDocument();
    wxInt32        iActivityMode = -1;

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

    pDoc->GetActivityRunMode( iActivityMode );
    if ( CMainDocument::MODE_NEVER == iActivityMode )
    {
        strBuffer = wxT(" ( ") + strBuffer + wxT(" ) ");
        strBuffer = _("Suspended") + strBuffer;
    }

    return 0;
}


const char *BOINC_RCSID_7aadb93332 = "$Id$";
