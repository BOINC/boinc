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
            wxLogTrace("CMainDocument::CachedStateUpdate - RPC Initialization Failed '%d'", retval);
    }

    return retval;
}


wxInt32 CMainDocument::OnExit()
{
    return 0;
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
            wxLogTrace("CMainDocument::CachedStateUpdate - Get State Failed '%d'", retval);

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

    if (!m_bIsConnected)
    {
        retval = rpc.init(NULL);
        if (retval)
        {
            wxLogTrace("CMainDocument::CachedProjectStatusUpdate - RPC Initialization Failed '%d'", retval);
            return retval;
        }

        m_bIsConnected = true;
    }

    retval = rpc.get_project_status(project_status);
    if (retval)
    {
        wxLogTrace("CMainDocument::CachedProjectStatusUpdate - Get Project Status Failed '%d'", retval);
    }


    m_fProjectTotalResourceShare = 0.0;
    for (i=0; i < (long)project_status.projects.size(); i++) {
        m_fProjectTotalResourceShare += project_status.projects.at( i )->resource_share;
    }

    return retval;
}


wxInt32 CMainDocument::GetProjectCount()
{
    CachedStateUpdate();
    CachedProjectStatusUpdate();
    wxInt32 iCount = project_status.projects.size();

    return iCount;
}


wxInt32 CMainDocument::GetProjectProjectName(wxInt32 iIndex, wxString& strBuffer)
{
    PROJECT* pProject = project_status.projects.at( iIndex );

    if ( NULL != pProject )
        strBuffer = pProject->project_name.c_str();

    return 0;
}


wxInt32 CMainDocument::GetProjectProjectURL(wxInt32 iIndex, wxString& strBuffer)
{
    PROJECT* pProject = project_status.projects.at( iIndex );

    if ( NULL != pProject )
        strBuffer = pProject->master_url.c_str();

    return 0;
}


wxInt32 CMainDocument::GetProjectAccountName(wxInt32 iIndex, wxString& strBuffer)
{
    PROJECT* pProject = project_status.projects.at( iIndex );

    if ( NULL != pProject )
        strBuffer = pProject->user_name.c_str();

    return 0;
}


wxInt32 CMainDocument::GetProjectTeamName(wxInt32 iIndex, wxString& strBuffer)
{
    PROJECT* pProject = project_status.projects.at( iIndex );

    if ( NULL != pProject )
        strBuffer = pProject->team_name.c_str();

    return 0;
}


wxInt32 CMainDocument::GetProjectTotalCredit(wxInt32 iIndex, float& fBuffer)
{
    PROJECT* pProject = project_status.projects.at( iIndex );

    if ( NULL != pProject )
        fBuffer = pProject->user_total_credit;

    return 0;
}


wxInt32 CMainDocument::GetProjectAvgCredit(wxInt32 iIndex, float& fBuffer)
{
    PROJECT* pProject = project_status.projects.at( iIndex );

    if ( NULL != pProject )
        fBuffer = pProject->user_expavg_credit;

    return 0;
}


wxInt32 CMainDocument::GetProjectResourceShare(wxInt32 iIndex, float& fBuffer)
{
    PROJECT* pProject = project_status.projects.at( iIndex );

    if ( NULL != pProject )
        fBuffer = pProject->resource_share;

    return 0;
}


wxInt32 CMainDocument::GetProjectTotalResourceShare(wxInt32 iIndex, float& fBuffer)
{
    fBuffer = m_fProjectTotalResourceShare;
    return 0;
}


wxInt32 CMainDocument::GetProjectMinRPCTime(wxInt32 iIndex, wxInt32& iBuffer)
{
    PROJECT* pProject = project_status.projects.at( iIndex );

    if ( NULL != pProject )
        iBuffer = pProject->min_rpc_time;

    return 0;
}


wxInt32 CMainDocument::GetProjectWebsiteCount(wxInt32 iIndex)
{
    wxInt32 iCount = 0;
    PROJECT* pProject = NULL;

    CachedStateUpdate();

    pProject = state.projects.at( iIndex );

    if ( NULL != pProject )
        iCount = pProject->gui_urls.size();

    return iCount;
}


wxInt32 CMainDocument::GetProjectWebsiteName(wxInt32 iProjectIndex, wxInt32 iWebsiteIndex, wxString& strBuffer)
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


wxInt32 CMainDocument::GetProjectWebsiteDescription(wxInt32 iProjectIndex, wxInt32 iWebsiteIndex, wxString& strBuffer)
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


wxInt32 CMainDocument::GetProjectWebsiteLink(wxInt32 iProjectIndex, wxInt32 iWebsiteIndex, wxString& strBuffer)
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


bool CMainDocument::IsProjectSuspended(wxInt32 iIndex)
{
    PROJECT* pProject = project_status.projects.at( iIndex );
    return pProject->suspended_via_gui;
}


bool CMainDocument::IsProjectRPCPending(wxInt32 iIndex)
{
    PROJECT* pProject = project_status.projects.at( iIndex );
    return pProject->sched_rpc_pending;
}


wxInt32 CMainDocument::ProjectAttach( wxString& strURL, wxString& strAccountKey )
{
    return rpc.project_attach((char *)strURL.c_str(), (char *)strAccountKey.c_str());
}


wxInt32 CMainDocument::ProjectDetach( wxString& strURL )
{
    PROJECT p;
    p.master_url = strURL;
    return rpc.project_op(p, wxT("detach"));
}


wxInt32 CMainDocument::ProjectUpdate( wxString& strURL )
{
    PROJECT p;
    p.master_url = strURL;
    return rpc.project_op(p, wxT("update"));
}


wxInt32 CMainDocument::ProjectReset( wxString& strURL )
{
    PROJECT p;
    p.master_url = strURL;
    return rpc.project_op(p, wxT("reset"));
}


wxInt32 CMainDocument::ProjectSuspend( wxString& strURL )
{
    PROJECT p;
    p.master_url = strURL;
    return rpc.project_op(p, wxT("suspend"));
}


wxInt32 CMainDocument::ProjectResume( wxString& strURL )
{
    PROJECT p;
    p.master_url = strURL;
    return rpc.project_op(p, wxT("resume"));
}


wxInt32 CMainDocument::CachedResultsStatusUpdate()
{
    wxInt32 retval = 0;

    if (!m_bIsConnected)
    {
        retval = rpc.init(NULL);
        if (retval)
        {
            wxLogTrace("CMainDocument::CachedResultsStatusUpdate - RPC Initialization Failed '%d'", retval);
            return retval;
        }

        m_bIsConnected = true;
    }

    retval = rpc.get_results(results);
    if (retval)
    {
        wxLogTrace("CMainDocument::CachedResultsStatusUpdate - Get Result Status Failed '%d'", retval);
    }


    return retval;
}


wxInt32 CMainDocument::GetWorkCount()
{
    CachedStateUpdate();
    CachedResultsStatusUpdate();
    wxInt32 iCount = state.results.size();

    return iCount;
}


wxInt32 CMainDocument::GetWorkProjectName(wxInt32 iIndex, wxString& strBuffer)
{
    RESULT* pResult = state.results.at( iIndex );
    PROJECT* pProject = NULL;

    if ( NULL != pResult )
    {
        pProject = pResult->project;
        if ( NULL != pProject )
        {
            strBuffer = pProject->project_name.c_str();
        }
    }

    return 0;
}


wxInt32 CMainDocument::GetWorkApplicationName(wxInt32 iIndex, wxString& strBuffer)
{
    RESULT* pResult = state.results.at( iIndex );
    WORKUNIT* pWorkunit = NULL;
    APP_VERSION* pAppVersion = NULL;

    if ( NULL != pResult )
    {
        pWorkunit = pResult->wup;
        if ( NULL != pWorkunit )
        {
            pAppVersion = pWorkunit->avp;
            if ( NULL != pAppVersion )
            {
                strBuffer = pAppVersion->app_name.c_str();
            }
        }
    }

    return 0;
}


wxInt32 CMainDocument::GetWorkApplicationVersion(wxInt32 iIndex, wxInt32& iBuffer)
{
    RESULT* pResult = state.results.at( iIndex );
    WORKUNIT* pWorkunit = NULL;
    APP_VERSION* pAppVersion = NULL;

    if ( NULL != pResult )
    {
        pWorkunit = pResult->wup;
        if ( NULL != pWorkunit )
        {
            pAppVersion = pWorkunit->avp;
            if ( NULL != pAppVersion )
            {
                iBuffer = pAppVersion->version_num;
            }
        }
    }

    return 0;
}


wxInt32 CMainDocument::GetWorkName(wxInt32 iIndex, wxString& strBuffer)
{
    RESULT* pResult = state.results.at( iIndex );

    if ( NULL != pResult )
        strBuffer = pResult->name.c_str();

    return 0;
}


wxInt32 CMainDocument::GetWorkCurrentCPUTime(wxInt32 iIndex, float& fBuffer)
{
    RESULT* pResult = state.results.at( iIndex );

    if ( NULL != pResult )
        fBuffer = pResult->current_cpu_time;

    return 0;
}


wxInt32 CMainDocument::GetWorkFinalCPUTime(wxInt32 iIndex, float& fBuffer)
{
    RESULT* pResult = state.results.at( iIndex );

    if ( NULL != pResult )
        fBuffer = pResult->final_cpu_time;

    return 0;
}


wxInt32 CMainDocument::GetWorkFractionDone(wxInt32 iIndex, float& fBuffer)
{
    RESULT* pResult = state.results.at( iIndex );

    if ( NULL != pResult )
        fBuffer = pResult->fraction_done;

    return 0;
}


wxInt32 CMainDocument::GetWorkReportDeadline(wxInt32 iIndex, wxInt32& iBuffer)
{
    RESULT* pResult = state.results.at( iIndex );

    if ( NULL != pResult )
        iBuffer = pResult->report_deadline;

    return 0;
}


wxInt32 CMainDocument::GetWorkState(wxInt32 iIndex)
{
    wxInt32 iBuffer = 0;
    RESULT* pResult = state.results.at( iIndex );

    if ( NULL != pResult )
        iBuffer = pResult->state;

    return iBuffer;
}


bool CMainDocument::IsWorkActive(wxInt32 iIndex)
{
    RESULT* pResult = state.results.at( iIndex );
    return pResult->suspended_via_gui;
}


bool CMainDocument::IsWorkSuspended(wxInt32 iIndex)
{
    RESULT* pResult = state.results.at( iIndex );
    return pResult->suspended_via_gui;
}


wxInt32 CMainDocument::CachedMessageUpdate()
{
    wxInt32 retval = 0;
    wxInt32 i = 0;

    if (!m_bIsConnected)
    {
        retval = rpc.init(NULL);
        if (retval)
        {
            wxLogTrace("CMainDocument::CachedMessageUpdate - RPC Initialization Failed '%d'", retval);
            return retval;
        }

        m_bIsConnected = true;
    }

    retval = rpc.get_messages( m_iMessageSequenceNumber, messages );
    if (retval)
    {
        wxLogTrace("CMainDocument::CachedMessageUpdate - Get Messages Failed '%d'", retval);
    }

    m_iMessageSequenceNumber = messages.messages.at( messages.messages.size()-1 )->seqno;

    return retval;
}


wxInt32 CMainDocument::GetMessageCount() 
{
    CachedStateUpdate();
    CachedMessageUpdate();
    wxInt32 iCount = messages.messages.size();

    return iCount;
}


wxInt32 CMainDocument::GetMessageProjectName(wxInt32 iIndex, wxString& strBuffer) 
{
    MESSAGE* pMessage = messages.messages.at( iIndex );
    if ( NULL != pMessage )
        strBuffer = pMessage->project.c_str();

    return 0;
}


wxInt32 CMainDocument::GetMessageTime(wxInt32 iIndex, wxDateTime& dtBuffer) 
{
    MESSAGE* pMessage = messages.messages.at( iIndex );
    if ( NULL != pMessage )
    {
        wxDateTime dtTemp((time_t)pMessage->timestamp);
        dtBuffer = dtTemp;
    }

    return 0;
}


wxInt32 CMainDocument::GetMessagePriority(wxInt32 iIndex, wxInt32& iBuffer) 
{
    MESSAGE* pMessage = messages.messages.at( iIndex );
    if ( NULL != pMessage )
        iBuffer = pMessage->priority;

    return 0;
}


wxInt32 CMainDocument::GetMessageMessage(wxInt32 iIndex, wxString& strBuffer) 
{
    MESSAGE* pMessage = messages.messages.at( iIndex );
    if ( NULL != pMessage )
        strBuffer = pMessage->body.c_str();

    return 0;
}





wxInt32 CMainDocument::GetTransferCount() {
    CachedStateUpdate();
    return 0;
}


wxString CMainDocument::GetTransferFileName(wxInt32 iIndex) {
    CachedStateUpdate();
    return wxString::Format(_T(""));
}


wxString CMainDocument::GetTransferProgress(wxInt32 iIndex) {
    CachedStateUpdate();
    return wxString::Format(_T(""));
}


wxString CMainDocument::GetTransferProjectName(wxInt32 iIndex) {
    CachedStateUpdate();
    return wxString::Format(_T(""));
}


wxString CMainDocument::GetTransferSize(wxInt32 iIndex) {
    CachedStateUpdate();
    return wxString::Format(_T(""));
}


wxString CMainDocument::GetTransferSpeed(wxInt32 iIndex) {
    CachedStateUpdate();
    return wxString::Format(_T(""));
}


wxString CMainDocument::GetTransferStatus(wxInt32 iIndex) {
    CachedStateUpdate();
    return wxString::Format(_T(""));
}


wxString CMainDocument::GetTransferTime(wxInt32 iIndex) {
    CachedStateUpdate();
    return wxString::Format(_T(""));
}

