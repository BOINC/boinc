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
#pragma implementation "MainDocument.h"
#endif

#include "stdwx.h"
#include "BOINCGUIApp.h"
#include "MainDocument.h"
#include "error_numbers.h"


IMPLEMENT_DYNAMIC_CLASS(CMainDocument, wxObject)


CMainDocument::CMainDocument()
{
#ifdef __WIN32__
    wxInt32 retval;
    WSADATA wsdata;

    retval = WSAStartup( MAKEWORD( 1, 1 ), &wsdata);
    if (retval) 
    {
        wxLogTrace("CMainDocument::CMainDocument - Winsock Initialization Failure '%d'", retval);
    }
#endif

    m_bIsConnected = false;

    m_fProjectTotalResourceShare = 0.0;

    m_iMessageSequenceNumber = 0;

    m_bCachedStateLocked = false;
    m_dtCachedStateLockTimestamp = wxDateTime::Now();
    m_dtCachedStateTimestamp = 0;
}


CMainDocument::~CMainDocument()
{
    m_dtCachedStateTimestamp = wxDateTime::Now();
    m_dtCachedStateLockTimestamp = wxDateTime::Now();
    m_bCachedStateLocked = false;

    m_iMessageSequenceNumber = 0;

    m_fProjectTotalResourceShare = 0.0;

    m_bIsConnected = false;

#ifdef __WIN32__
    WSACleanup();
#endif
}


wxInt32 CMainDocument::OnInit()
{
    wxInt32 retval = 0;

    if (!m_bIsConnected)
    {
        retval = rpc.init(NULL);
        if (retval)
            wxLogTrace("CMainDocument::OnInit - RPC Initialization Failed '%d'", retval);
    }

    return retval;
}


wxInt32 CMainDocument::OnExit()
{
    wxInt32 retval = 0;

    if (m_bIsConnected)
        rpc.close();

    return retval;
}


wxInt32 CMainDocument::OnIdle()
{
    if (m_bIsConnected)
    {
        CachedStateUpdate();
    }

    return 0;
}


wxInt32 CMainDocument::CachedStateUpdate()
{
    wxInt32     retval = 0;
    CMainFrame* pFrame   = wxGetApp().GetFrame();
    wxString    strEmpty = wxEmptyString;

    wxASSERT(NULL != pFrame);
    wxASSERT(wxDynamicCast(pFrame, CMainFrame));

    wxTimeSpan ts(m_dtCachedStateLockTimestamp - m_dtCachedStateTimestamp);
    if (!m_bCachedStateLocked && (ts > wxTimeSpan::Seconds(3600)))
    {
        pFrame->UpdateStatusbar( _("Retrieving the BOINC system state.  Please wait...") );
        m_dtCachedStateTimestamp = m_dtCachedStateLockTimestamp;

        retval = rpc.get_state(state);
        if (retval)
        {
            wxLogTrace("CMainDocument::CachedStateUpdate - Get State Failed '%d'", retval);
            state.clear();
        }

        pFrame->UpdateStatusbar( _("Retrieving the BOINC host information.  Please wait...") );
        retval = rpc.get_host_info(host);
        if (retval)
        {
            wxLogTrace("CMainDocument::CachedStateUpdate - Get Host Information Failed '%d'", retval);
            host.clear();
        }

        pFrame->UpdateStatusbar( strEmpty );
    }

    return retval;
}


wxInt32 CMainDocument::CachedStateLock()
{
    m_bCachedStateLocked = true;
    m_dtCachedStateLockTimestamp = wxDateTime::Now();
    return 0;
}


wxInt32 CMainDocument::CachedStateUnlock()
{
    m_bCachedStateLocked = false;
    return 0;
}


wxInt32 CMainDocument::CachedProjectStatusUpdate()
{
    wxInt32 retval = 0;
    wxInt32 i = 0;

    retval = rpc.get_project_status(project_status);
    if (retval)
    {
        wxLogTrace("CMainDocument::CachedProjectStatusUpdate - Get Project Status Failed '%d'", retval);
        project_status.clear();
    }

    m_fProjectTotalResourceShare = 0.0;
    for (i=0; i < (long)project_status.projects.size(); i++) {
        m_fProjectTotalResourceShare += project_status.projects.at( i )->resource_share;
    }

    return retval;
}


wxInt32 CMainDocument::GetProjectCount()
{
    wxInt32 iCount = 0;

    CachedStateUpdate();
    CachedProjectStatusUpdate();

    if ( !project_status.projects.empty() )
        iCount = project_status.projects.size();

    return iCount;
}


wxInt32 CMainDocument::GetProjectProjectName( wxInt32 iIndex, wxString& strBuffer )
{
    PROJECT* pProject = NULL;

    if ( !project_status.projects.empty() )
        pProject = project_status.projects.at( iIndex );

    if ( NULL != pProject )
        strBuffer = pProject->project_name.c_str();

    return 0;
}


wxInt32 CMainDocument::GetProjectProjectURL( wxInt32 iIndex, wxString& strBuffer )
{
    PROJECT* pProject = NULL;

    if ( !project_status.projects.empty() )
        pProject = project_status.projects.at( iIndex );

    if ( NULL != pProject )
        strBuffer = pProject->master_url.c_str();

    return 0;
}


wxInt32 CMainDocument::GetProjectAccountName( wxInt32 iIndex, wxString& strBuffer )
{
    PROJECT* pProject = NULL;

    if ( !project_status.projects.empty() )
        pProject = project_status.projects.at( iIndex );

    if ( NULL != pProject )
        strBuffer = pProject->user_name.c_str();

    return 0;
}


wxInt32 CMainDocument::GetProjectTeamName( wxInt32 iIndex, wxString& strBuffer )
{
    PROJECT* pProject = NULL;

    if ( !project_status.projects.empty() )
        pProject = project_status.projects.at( iIndex );

    if ( NULL != pProject )
        strBuffer = pProject->team_name.c_str();

    return 0;
}


wxInt32 CMainDocument::GetProjectTotalCredit( wxInt32 iIndex, float& fBuffer )
{
    PROJECT* pProject = NULL;

    if ( !project_status.projects.empty() )
        pProject = project_status.projects.at( iIndex );

    if ( NULL != pProject )
        fBuffer = pProject->user_total_credit;

    return 0;
}


wxInt32 CMainDocument::GetProjectAvgCredit( wxInt32 iIndex, float& fBuffer )
{
    PROJECT* pProject = NULL;

    if ( !project_status.projects.empty() )
        pProject = project_status.projects.at( iIndex );

    if ( NULL != pProject )
        fBuffer = pProject->user_expavg_credit;

    return 0;
}


wxInt32 CMainDocument::GetProjectResourceShare( wxInt32 iIndex, float& fBuffer )
{
    PROJECT* pProject = NULL;

    if ( !project_status.projects.empty() )
        pProject = project_status.projects.at( iIndex );

    if ( NULL != pProject )
        fBuffer = pProject->resource_share;

    return 0;
}


wxInt32 CMainDocument::GetProjectTotalResourceShare( wxInt32 iIndex, float& fBuffer )
{
    fBuffer = m_fProjectTotalResourceShare;
    return 0;
}


wxInt32 CMainDocument::GetProjectMinRPCTime( wxInt32 iIndex, wxInt32& iBuffer )
{
    PROJECT* pProject = NULL;

    if ( !project_status.projects.empty() )
        pProject = project_status.projects.at( iIndex );

    if ( NULL != pProject )
        iBuffer = pProject->min_rpc_time;

    return 0;
}


wxInt32 CMainDocument::GetProjectWebsiteCount( wxInt32 iIndex )
{
    wxInt32     iCount = 0;
    wxString    strProjectURL = wxEmptyString;
    std::string str;
    PROJECT*    pProject = NULL;

    CachedStateUpdate();

    GetProjectProjectURL( iIndex, strProjectURL );
    str = strProjectURL.c_str();
    pProject = state.lookup_project( str );

    if ( NULL != pProject )
        iCount = pProject->gui_urls.size();

    return iCount;
}


wxInt32 CMainDocument::GetProjectWebsiteName( wxInt32 iProjectIndex, wxInt32 iWebsiteIndex, wxString& strBuffer )
{
    wxString    strProjectURL = wxEmptyString;
    std::string str;
    PROJECT*    pProject      = NULL;
    GUI_URL     Url;

    CachedStateUpdate();

    GetProjectProjectURL( iProjectIndex, strProjectURL );
    str = strProjectURL.c_str();
    pProject = state.lookup_project( str );

    if ( NULL != pProject )
    {
        Url = pProject->gui_urls.at( iWebsiteIndex );
        strBuffer = Url.name.c_str();
    }

    return 0;
}


wxInt32 CMainDocument::GetProjectWebsiteDescription( wxInt32 iProjectIndex, wxInt32 iWebsiteIndex, wxString& strBuffer )
{
    wxString    strProjectURL = wxEmptyString;
    std::string str;
    PROJECT*    pProject      = NULL;
    GUI_URL     Url;

    CachedStateUpdate();

    GetProjectProjectURL( iProjectIndex, strProjectURL );
    str = strProjectURL.c_str();
    pProject = state.lookup_project( str );

    if ( NULL != pProject )
    {
        Url = pProject->gui_urls.at( iWebsiteIndex );
        strBuffer = Url.description.c_str();
    }

    return 0;
}


wxInt32 CMainDocument::GetProjectWebsiteLink( wxInt32 iProjectIndex, wxInt32 iWebsiteIndex, wxString& strBuffer )
{
    wxString    strProjectURL = wxEmptyString;
    std::string str;
    PROJECT*    pProject      = NULL;
    GUI_URL     Url;

    CachedStateUpdate();

    GetProjectProjectURL( iProjectIndex, strProjectURL );
    str = strProjectURL.c_str();
    pProject = state.lookup_project( str );

    if ( NULL != pProject )
    {
        Url = pProject->gui_urls.at( iWebsiteIndex );
        strBuffer = Url.url.c_str();
    }

    return 0;
}


bool CMainDocument::IsProjectSuspended( wxInt32 iIndex )
{
    PROJECT* pProject = NULL;
    bool     bRetVal  = false;

    if ( !project_status.projects.empty() )
        pProject = project_status.projects.at( iIndex );

    if ( NULL != pProject )
        bRetVal = pProject->suspended_via_gui;

    return bRetVal;
}


bool CMainDocument::IsProjectRPCPending( wxInt32 iIndex )
{
    PROJECT* pProject = NULL;
    bool     bRetVal  = false;

    if ( !project_status.projects.empty() )
        pProject = project_status.projects.at( iIndex );

    if ( NULL != pProject )
        bRetVal = pProject->sched_rpc_pending;

    return bRetVal;
}


wxInt32 CMainDocument::ProjectAttach( wxString& strURL, wxString& strAccountKey )
{
    return rpc.project_attach((char *)strURL.c_str(), (char *)strAccountKey.c_str());
}


wxInt32 CMainDocument::ProjectDetach( wxInt32 iIndex )
{
    PROJECT* pProject = NULL;
    wxInt32 iRetVal = -1;

    if ( !project_status.projects.empty() )
        pProject = project_status.projects.at( iIndex );

    if ( NULL != pProject )
        iRetVal = rpc.project_op( (*pProject), wxT("detach") );

    return iRetVal;
}


wxInt32 CMainDocument::ProjectUpdate( wxInt32 iIndex )
{
    PROJECT* pProject = NULL;
    wxInt32 iRetVal = -1;

    if ( !project_status.projects.empty() )
        pProject = project_status.projects.at( iIndex );

    if ( NULL != pProject )
        iRetVal = rpc.project_op( (*pProject), wxT("update") );

    return iRetVal;
}


wxInt32 CMainDocument::ProjectReset( wxInt32 iIndex )
{
    PROJECT* pProject = NULL;
    wxInt32 iRetVal = -1;

    if ( !project_status.projects.empty() )
        pProject = project_status.projects.at( iIndex );

    if ( NULL != pProject )
        iRetVal = rpc.project_op( (*pProject), wxT("reset") );

    return iRetVal;
}


wxInt32 CMainDocument::ProjectSuspend( wxInt32 iIndex )
{
    PROJECT* pProject = NULL;
    PROJECT* pStateProject = NULL;
    wxInt32 iRetVal = -1;

    if ( !project_status.projects.empty() )
        pProject = project_status.projects.at( iIndex );

    if ( NULL != pProject )
    {
        iRetVal = rpc.project_op( (*pProject), wxT("suspend") );
        if ( 0 == iRetVal )
        {
            pProject->suspended_via_gui = true;
            pStateProject = state.lookup_project( pProject->master_url );
            if ( NULL != pStateProject )
                pStateProject->suspended_via_gui = true;
        }
    }

    return iRetVal;
}


wxInt32 CMainDocument::ProjectResume( wxInt32 iIndex )
{
    PROJECT* pProject = NULL;
    PROJECT* pStateProject = NULL;
    wxInt32 iRetVal = -1;

    if ( !project_status.projects.empty() )
        pProject = project_status.projects.at( iIndex );

    if ( NULL != pProject )
    {
        iRetVal = rpc.project_op( (*pProject), wxT("resume") );
        if ( 0 == iRetVal )
        {
            pProject->suspended_via_gui = false;
            pStateProject = state.lookup_project( pProject->master_url );
            if ( NULL != pStateProject )
                pStateProject->suspended_via_gui = false;
        }
    }

    return iRetVal;
}


wxInt32 CMainDocument::CachedResultsStatusUpdate()
{
    wxInt32 retval = 0;

    retval = rpc.get_results(results);
    if (retval)
    {
        wxLogTrace("CMainDocument::CachedResultsStatusUpdate - Get Result Status Failed '%d'", retval);
        results.clear();
    }

    return retval;
}


wxInt32 CMainDocument::GetWorkCount()
{
    wxInt32 iCount = 0;

    CachedStateUpdate();
    CachedResultsStatusUpdate();

    if ( !results.results.empty() )
        iCount = results.results.size();

    return iCount;
}


wxInt32 CMainDocument::GetWorkProjectName( wxInt32 iIndex, wxString& strBuffer )
{
    RESULT* pResult = NULL;
    RESULT* pStateResult = NULL;
    PROJECT* pProject = NULL;

    if ( !results.results.empty() )
        pResult = results.results.at( iIndex );

    if ( NULL != pResult )
    {
        pStateResult = state.lookup_result( pResult->project_url, pResult->name );
        if ( NULL != pStateResult )
        {
            pProject = pStateResult->project;
            if ( NULL != pProject )
            {
                strBuffer = pProject->project_name.c_str();
            }
        }
    }

    return 0;
}


wxInt32 CMainDocument::GetWorkProjectURL( wxInt32 iIndex, wxString& strBuffer )
{
    RESULT* pResult = NULL;

    if ( !results.results.empty() )
        pResult = results.results.at( iIndex );

    if ( NULL != pResult )
        strBuffer = pResult->project_url.c_str();

    return 0;
}


wxInt32 CMainDocument::GetWorkApplicationName( wxInt32 iIndex, wxString& strBuffer )
{
    RESULT* pResult = NULL;
    RESULT* pStateResult = NULL;
    WORKUNIT* pWorkunit = NULL;
    APP_VERSION* pAppVersion = NULL;

    if ( !results.results.empty() )
        pResult = results.results.at( iIndex );

    if ( NULL != pResult )
    {
        pStateResult = state.lookup_result( pResult->project_url, pResult->name );
        if ( NULL != pStateResult )
        {
            pWorkunit = pStateResult->wup;
            if ( NULL != pWorkunit )
            {
                pAppVersion = pWorkunit->avp;
                if ( NULL != pAppVersion )
                {
                    strBuffer = pAppVersion->app_name.c_str();
                }
            }
        }
    }

    return 0;
}


wxInt32 CMainDocument::GetWorkApplicationVersion( wxInt32 iIndex, wxInt32& iBuffer )
{
    RESULT* pResult = NULL;
    RESULT* pStateResult = NULL;
    WORKUNIT* pWorkunit = NULL;
    APP_VERSION* pAppVersion = NULL;

    if ( !results.results.empty() )
        pResult = results.results.at( iIndex );

    if ( NULL != pResult )
    {
        pStateResult = state.lookup_result( pResult->project_url, pResult->name );
        if ( NULL != pStateResult )
        {
            pWorkunit = pStateResult->wup;
            if ( NULL != pWorkunit )
            {
                pAppVersion = pWorkunit->avp;
                if ( NULL != pAppVersion )
                {
                    iBuffer = pAppVersion->version_num;
                }
            }
        }
    }

    return 0;
}


wxInt32 CMainDocument::GetWorkName( wxInt32 iIndex, wxString& strBuffer )
{
    RESULT* pResult = NULL;

    if ( !results.results.empty() )
        pResult = results.results.at( iIndex );

    if ( NULL != pResult )
        strBuffer = pResult->name.c_str();

    return 0;
}


wxInt32 CMainDocument::GetWorkCurrentCPUTime( wxInt32 iIndex, float& fBuffer )
{
    RESULT* pResult = NULL;

    if ( !results.results.empty() )
        pResult = results.results.at( iIndex );

    if ( NULL != pResult )
        fBuffer = pResult->current_cpu_time;

    return 0;
}


wxInt32 CMainDocument::GetWorkEstimatedCPUTime( wxInt32 iIndex, float& fBuffer )
{
    RESULT* pResult = NULL;

    if ( !results.results.empty() )
        pResult = results.results.at( iIndex );

    if ( NULL != pResult )
        fBuffer = pResult->estimated_cpu_time_remaining;

    return 0;
}


wxInt32 CMainDocument::GetWorkFinalCPUTime( wxInt32 iIndex, float& fBuffer )
{
    RESULT* pResult = NULL;

    if ( !results.results.empty() )
        pResult = results.results.at( iIndex );

    if ( NULL != pResult )
        fBuffer = pResult->final_cpu_time;

    return 0;
}


wxInt32 CMainDocument::GetWorkFractionDone( wxInt32 iIndex, float& fBuffer )
{
    RESULT* pResult = NULL;

    if ( !results.results.empty() )
        pResult = results.results.at( iIndex );

    if ( NULL != pResult )
        fBuffer = pResult->fraction_done;

    return 0;
}


wxInt32 CMainDocument::GetWorkReportDeadline( wxInt32 iIndex, wxInt32& iBuffer )
{
    RESULT* pResult = NULL;

    if ( !results.results.empty() )
        pResult = results.results.at( iIndex );

    if ( NULL != pResult )
        iBuffer = pResult->report_deadline;

    return 0;
}


wxInt32 CMainDocument::GetWorkState( wxInt32 iIndex )
{
    wxInt32 iBuffer = 0;
    RESULT* pResult = NULL;

    if ( !results.results.empty() )
        pResult = results.results.at( iIndex );

    if ( NULL != pResult )
        iBuffer = pResult->state;

    return iBuffer;
}


wxInt32 CMainDocument::GetWorkSchedulerState( wxInt32 iIndex )
{
    wxInt32 iBuffer = 0;
    RESULT* pResult = NULL;

    if ( !results.results.empty() )
        pResult = results.results.at( iIndex );

    if ( NULL != pResult )
        iBuffer = pResult->scheduler_state;

    return iBuffer;
}


bool CMainDocument::IsWorkAcknowledged( wxInt32 iIndex )
{
    RESULT* pResult = NULL;
    bool bRetVal    = false;

    if ( !results.results.empty() )
        pResult = results.results.at( iIndex );

    if ( NULL != pResult )
        bRetVal = pResult->got_server_ack;

    return bRetVal;
}


bool CMainDocument::IsWorkActive( wxInt32 iIndex )
{
    RESULT* pResult = NULL;
    bool bRetVal    = false;

    if ( !results.results.empty() )
        pResult = results.results.at( iIndex );

    if ( NULL != pResult )
        bRetVal = pResult->active_task;

    return bRetVal;
}


bool CMainDocument::IsWorkReadyToReport( wxInt32 iIndex )
{
    RESULT* pResult = NULL;
    bool bRetVal    = false;

    if ( !results.results.empty() )
        pResult = results.results.at( iIndex );

    if ( NULL != pResult )
        bRetVal = pResult->ready_to_report;

    return bRetVal;
}


bool CMainDocument::IsWorkSuspended( wxInt32 iIndex )
{
    RESULT* pResult = NULL;
    bool bRetVal    = false;

    if ( !results.results.empty() )
        pResult = results.results.at( iIndex );

    if ( NULL != pResult )
        bRetVal = pResult->suspended_via_gui;

    return bRetVal;
}


wxInt32 CMainDocument::WorkSuspend( wxInt32 iIndex )
{
    RESULT* pResult = NULL;
    RESULT* pStateResult = NULL;
    wxInt32 iRetVal = 0;

    if ( !results.results.empty() )
        pResult = results.results.at( iIndex );

    if ( NULL != pResult )
    {
        pStateResult = state.lookup_result( pResult->project_url, pResult->name );
        if ( NULL != pStateResult )
        {
            iRetVal = rpc.result_op( (*pStateResult), wxT("suspend") );
            if ( 0 == iRetVal )
            {
                pResult->suspended_via_gui = true;
                pStateResult->suspended_via_gui = true;
            }
        }
    }

    return iRetVal;
}


wxInt32 CMainDocument::WorkResume( wxInt32 iIndex )
{
    RESULT* pResult = NULL;
    RESULT* pStateResult = NULL;
    wxInt32 iRetVal = 0;

    if ( !results.results.empty() )
        pResult = results.results.at( iIndex );

    if ( NULL != pResult )
    {
        pStateResult = state.lookup_result( pResult->project_url, pResult->name );
        if ( NULL != pStateResult )
        {
            iRetVal = rpc.result_op( (*pStateResult), wxT("resume") );
            if ( 0 == iRetVal )
            {
                pResult->suspended_via_gui = false;
                pStateResult->suspended_via_gui = false;
            }
        }
    }

    return iRetVal;
}


wxInt32 CMainDocument::WorkShowGraphics( wxInt32 iIndex, bool bFullScreen )
{
    RESULT* pResult = NULL;
    wxInt32 iRetVal = 0;

    if ( !results.results.empty() )
        pResult = results.results.at( iIndex );

    if ( NULL != pResult )
        iRetVal = rpc.show_graphics( pResult->project_url.c_str(), pResult->name.c_str(), bFullScreen );

    return iRetVal;
}


wxInt32 CMainDocument::WorkAbort( wxInt32 iIndex )
{
    RESULT* pResult = NULL;
    RESULT* pStateResult = NULL;
    wxInt32 iRetVal = 0;

    if ( !results.results.empty() )
        pResult = results.results.at( iIndex );

    if ( NULL != pResult )
    {
        pStateResult = state.lookup_result( pResult->project_url, pResult->name );
        if ( NULL != pStateResult )
        {
            iRetVal = rpc.result_op( (*pStateResult), wxT("abort") );
        }
    }

    return iRetVal;
}


wxInt32 CMainDocument::CachedMessageUpdate()
{
    wxInt32 retval = 0;

    retval = rpc.get_messages( m_iMessageSequenceNumber, messages );
    if (retval)
    {
        wxLogTrace("CMainDocument::CachedMessageUpdate - Get Messages Failed '%d'", retval);
        messages.clear();
    }

    if ( messages.messages.size() != 0 )
        m_iMessageSequenceNumber = messages.messages.at( messages.messages.size()-1 )->seqno;

    return retval;
}


wxInt32 CMainDocument::GetMessageCount() 
{
    wxInt32 iCount = 0;

    CachedStateUpdate();
    CachedMessageUpdate();

    if ( !messages.messages.empty() )
        iCount = messages.messages.size();

    return iCount;
}


wxInt32 CMainDocument::GetMessageProjectName( wxInt32 iIndex, wxString& strBuffer ) 
{
    MESSAGE* pMessage = NULL;

    if ( !messages.messages.empty() )
        pMessage = messages.messages.at( iIndex );

    if ( NULL != pMessage )
        strBuffer = pMessage->project.c_str();

    return 0;
}


wxInt32 CMainDocument::GetMessageTime( wxInt32 iIndex, wxDateTime& dtBuffer ) 
{
    MESSAGE* pMessage = NULL;

    if ( !messages.messages.empty() )
        pMessage = messages.messages.at( iIndex );

    if ( NULL != pMessage )
    {
        wxDateTime dtTemp((time_t)pMessage->timestamp);
        dtBuffer = dtTemp;
    }

    return 0;
}


wxInt32 CMainDocument::GetMessagePriority( wxInt32 iIndex, wxInt32& iBuffer ) 
{
    MESSAGE* pMessage = NULL;

    if ( !messages.messages.empty() )
        pMessage = messages.messages.at( iIndex );

    if ( NULL != pMessage )
        iBuffer = pMessage->priority;

    return 0;
}


wxInt32 CMainDocument::GetMessageMessage( wxInt32 iIndex, wxString& strBuffer ) 
{
    MESSAGE* pMessage = NULL;

    if ( !messages.messages.empty() )
        pMessage = messages.messages.at( iIndex );

    if ( NULL != pMessage )
        strBuffer = pMessage->body.c_str();

    return 0;
}


wxInt32 CMainDocument::CachedFileTransfersUpdate()
{
    wxInt32 retval = 0;

    retval = rpc.get_file_transfers( ft );
    if (retval)
    {
        wxLogTrace("CMainDocument::CachedFileTransfersUpdate - Get File Transfers Failed '%d'", retval);
        ft.clear();
    }

    return retval;
}


wxInt32 CMainDocument::GetTransferCount()
{
    wxInt32 iCount = 0;

    CachedStateUpdate();
    CachedFileTransfersUpdate();

    if ( !ft.file_transfers.empty() )
        iCount = ft.file_transfers.size();

    return iCount;
}


wxInt32 CMainDocument::GetTransferProjectName( wxInt32 iIndex, wxString& strBuffer )
{
    FILE_TRANSFER* pFT = NULL;
    PROJECT* pProject = NULL;

    if ( !ft.file_transfers.empty() )
        pFT = ft.file_transfers.at( iIndex );

    if ( NULL != pFT )
    {
        pProject = state.lookup_project( pFT->project_url );
        if ( NULL != pProject )
        {
            strBuffer = pProject->project_name.c_str();
        }
    }

    return 0;
}


wxInt32 CMainDocument::GetTransferFileName( wxInt32 iIndex, wxString& strBuffer )
{
    FILE_TRANSFER* pFT = NULL;

    if ( !ft.file_transfers.empty() )
        pFT = ft.file_transfers.at( iIndex );

    if ( NULL != pFT )
        strBuffer = pFT->name.c_str();

    return 0;
}


wxInt32 CMainDocument::GetTransferFileSize( wxInt32 iIndex, float& fBuffer )
{
    FILE_TRANSFER* pFT = NULL;

    if ( !ft.file_transfers.empty() )
        pFT = ft.file_transfers.at( iIndex );

    if ( NULL != pFT )
        fBuffer = pFT->nbytes;

    return 0;
}


wxInt32 CMainDocument::GetTransferBytesXfered( wxInt32 iIndex, float& fBuffer )
{
    FILE_TRANSFER* pFT = NULL;

    if ( !ft.file_transfers.empty() )
        pFT = ft.file_transfers.at( iIndex );

    if ( NULL != pFT )
        fBuffer = pFT->bytes_xferred;

    return 0;
}


wxInt32 CMainDocument::GetTransferSpeed( wxInt32 iIndex, float& fBuffer )
{
    FILE_TRANSFER* pFT = NULL;

    if ( !ft.file_transfers.empty() )
        pFT = ft.file_transfers.at( iIndex );

    if ( NULL != pFT )
        fBuffer = pFT->xfer_speed;

    return 0;
}


wxInt32 CMainDocument::GetTransferTime( wxInt32 iIndex, float& fBuffer )
{
    FILE_TRANSFER* pFT = NULL;

    if ( !ft.file_transfers.empty() )
        pFT = ft.file_transfers.at( iIndex );

    if ( NULL != pFT )
        fBuffer = pFT->time_so_far;

    return 0;
}


wxInt32 CMainDocument::GetTransferNextRequestTime( wxInt32 iIndex, wxInt32& iBuffer )
{
    FILE_TRANSFER* pFT = NULL;

    if ( !ft.file_transfers.empty() )
        pFT = ft.file_transfers.at( iIndex );

    if ( NULL != pFT )
        iBuffer = pFT->next_request_time;

    return 0;
}


wxInt32 CMainDocument::GetTransferStatus( wxInt32 iIndex, wxInt32& iBuffer )
{
    FILE_TRANSFER* pFT = NULL;

    if ( !ft.file_transfers.empty() )
        pFT = ft.file_transfers.at( iIndex );

    if ( NULL != pFT )
        iBuffer = pFT->status;

    return 0;
}


bool CMainDocument::IsTransferActive( wxInt32 iIndex )
{
    FILE_TRANSFER* pFT = NULL;
    bool bRetVal    = false;

    if ( !ft.file_transfers.empty() )
        pFT = ft.file_transfers.at( iIndex );

    if ( NULL != pFT )
        bRetVal = pFT->pers_xfer_active;

    return bRetVal;
}

bool CMainDocument::IsTransferGeneratedLocally( wxInt32 iIndex )
{
    FILE_TRANSFER* pFT = NULL;
    bool bRetVal    = false;

    if ( !ft.file_transfers.empty() )
        pFT = ft.file_transfers.at( iIndex );

    if ( NULL != pFT )
        bRetVal = pFT->generated_locally;

    return bRetVal;
}


wxInt32 CMainDocument::TransferRetryNow( wxInt32 iIndex )
{
    FILE_TRANSFER* pFT = NULL;
    wxInt32 iRetVal = 0;

    if ( !ft.file_transfers.empty() )
        pFT = ft.file_transfers.at( iIndex );

    if ( NULL != pFT )
        iRetVal = rpc.file_transfer_op( (*pFT), wxT("retry") );

    return iRetVal;
}


wxInt32 CMainDocument::TransferAbort( wxInt32 iIndex )
{
    FILE_TRANSFER* pFT = NULL;
    wxInt32 iRetVal = 0;

    if ( !ft.file_transfers.empty() )
        pFT = ft.file_transfers.at( iIndex );

    if ( NULL != pFT )
        iRetVal = rpc.file_transfer_op( (*pFT), wxT("abort") );

    return iRetVal;
}

