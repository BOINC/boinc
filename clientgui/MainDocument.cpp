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
    m_strConnectedComputerName = wxEmptyString;

    m_iCachedActivityRunMode = 0;
    m_iCachedNetworkRunMode = 0;

    m_fProjectTotalResourceShare = 0.0;

    m_iMessageSequenceNumber = 0;

    m_bCachedStateLocked = false;
    m_dtCachedStateLockTimestamp = wxDateTime::Now();
    m_dtCachedStateTimestamp = wxDateTime( (time_t)0 );
    m_dtCachedActivityRunModeTimestamp = wxDateTime( (time_t)0 );
    m_dtCachedNetworkRunModeTimestamp = wxDateTime( (time_t)0 );
}


CMainDocument::~CMainDocument()
{
    m_dtCachedNetworkRunModeTimestamp = wxDateTime::Now();
    m_dtCachedActivityRunModeTimestamp = wxDateTime::Now();
    m_dtCachedStateTimestamp = wxDateTime::Now();
    m_dtCachedStateLockTimestamp = wxDateTime::Now();
    m_bCachedStateLocked = false;

    m_iMessageSequenceNumber = 0;

    m_fProjectTotalResourceShare = 0.0;

    m_iCachedActivityRunMode = 0;
    m_iCachedNetworkRunMode = 0;

    m_strConnectedComputerName = wxEmptyString;
    m_bIsConnected = false;

#ifdef __WIN32__
    WSACleanup();
#endif
}


wxInt32 CMainDocument::CachedStateUpdate()
{
    wxInt32     retval = 0;
    wxString    strEmpty = wxEmptyString;

    wxTimeSpan ts(m_dtCachedStateLockTimestamp - m_dtCachedStateTimestamp);
    if (!m_bCachedStateLocked && (ts.GetSeconds() > 3600))
    {
        wxLogStatus( _("Retrieving system state; please wait...") );

        m_dtCachedStateTimestamp = m_dtCachedStateLockTimestamp;
        retval = rpc.get_state(state);
        if (retval)
        {
            wxLogTrace("CMainDocument::CachedStateUpdate - Get State Failed '%d'", retval);
            Connect( strEmpty );
        }

        wxLogStatus( _("Retrieving host information; please wait...") );

        retval = rpc.get_host_info(host);
        if (retval)
        {
            wxLogTrace("CMainDocument::CachedStateUpdate - Get Host Information Failed '%d'", retval);
            Connect( strEmpty );
        }

        wxLogStatus( wxEmptyString );
    }

    return retval;
}


wxInt32 CMainDocument::ForceCacheUpdate()
{
    m_dtCachedStateLockTimestamp = wxDateTime::Now();
    m_dtCachedStateTimestamp = wxDateTime( (time_t)0 );
    return 0;
}


wxInt32 CMainDocument::OnInit()
{
    wxInt32 iRetVal = -1;
    wxString strMachine = wxEmptyString;

    if ( !IsConnected() )
        iRetVal = Connect( strMachine );

    return iRetVal;
}


wxInt32 CMainDocument::OnExit()
{
    wxInt32 iRetVal = 0;

    if ( IsConnected() )
        iRetVal = Disconnect();

    return iRetVal;
}


wxInt32 CMainDocument::OnIdle()
{
    if ( IsConnected() )
        CachedStateUpdate();

    return 0;
}


wxInt32 CMainDocument::Connect( wxString& strMachine,  bool bDisconnect )
{
    wxInt32 iRetVal = -1;
    std::string str;

    if ( IsConnected() && bDisconnect )
        Disconnect();

    if ( IsConnected() )
        return BOINC_SUCCESS;

    str.clear();

    if ( strMachine.empty() && !m_strConnectedComputerName.empty() )
        str = m_strConnectedComputerName.c_str();
    else
        str = strMachine.c_str();

    if ( str.empty() )
        iRetVal = rpc.init( NULL );
    else
        iRetVal = rpc.init( str.c_str() );

    if ( 0 == iRetVal )
    {
        m_bIsConnected = true;
        m_strConnectedComputerName = strMachine;
    }
    else
        wxLogTrace("CMainDocument::Connect - RPC Initialization Failed '%d'", iRetVal);

    return iRetVal;
}


wxInt32 CMainDocument::Disconnect()
{
    if ( IsConnected() )
    {
        rpc.close();
        state.clear();
        host.clear();
        project_status.clear();
        results.clear();
        messages.clear();
        ft.clear();
        resource_status.clear();
        proxy_info.clear();
        
        m_dtCachedStateLockTimestamp = wxDateTime::Now();
        m_dtCachedStateTimestamp = wxDateTime( (time_t)0 );

        m_iMessageSequenceNumber = 0;

        m_bIsConnected = false;
    }

    return 0;
}


wxInt32 CMainDocument::GetConnectedComputerName( wxString& strMachine )
{
    strMachine = m_strConnectedComputerName;
    return 0;
}


bool CMainDocument::IsConnected()
{
    return m_bIsConnected;
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


wxInt32 CMainDocument::GetActivityRunMode( wxInt32& iMode )
{
    wxInt32     iRetVal = 0;

    wxTimeSpan ts(wxDateTime::Now() - m_dtCachedActivityRunModeTimestamp);
    if ( ts.GetSeconds() > 10 )
    {
        m_dtCachedActivityRunModeTimestamp = wxDateTime::Now();

        if ( IsConnected() )
        {
            iRetVal = rpc.get_run_mode( iMode );
            if ( 0 == iRetVal )
            {
                m_iCachedActivityRunMode = iMode;
            }
        }
    }
    else
    {
        iMode = m_iCachedActivityRunMode;
    }

    return iRetVal;
}


wxInt32 CMainDocument::SetActivityRunMode( wxInt32 iMode )
{
    wxInt32     iRetVal = 0;

    if ( IsConnected() )
    {
        iRetVal = rpc.set_run_mode( iMode );
        if ( 0 == iRetVal )
        {
            m_dtCachedActivityRunModeTimestamp = wxDateTime::Now();
            m_iCachedActivityRunMode = iMode;
        }
    }

    return iRetVal;
}


wxInt32 CMainDocument::GetNetworkRunMode( wxInt32& iMode )
{
    wxInt32     iRetVal = 0;

    wxTimeSpan ts(wxDateTime::Now() - m_dtCachedNetworkRunModeTimestamp);
    if ( ts.GetSeconds() > 10 )
    {
        m_dtCachedNetworkRunModeTimestamp = wxDateTime::Now();

        if ( IsConnected() )
        {
            iRetVal = rpc.get_network_mode( iMode );
            if ( 0 == iRetVal )
            {
                m_iCachedNetworkRunMode = iMode;
            }
        }
    }
    else
    {
        iMode = m_iCachedNetworkRunMode;
    }

    return iRetVal;
}


wxInt32 CMainDocument::SetNetworkRunMode( wxInt32 iMode )
{
    wxInt32     iRetVal = 0;

    if ( IsConnected() )
    {
        iRetVal = rpc.set_network_mode( iMode );
        if ( 0 == iRetVal )
        {
            m_dtCachedNetworkRunModeTimestamp = wxDateTime::Now();
            m_iCachedNetworkRunMode = iMode;
        }
    }

    return iRetVal;
}


wxInt32 CMainDocument::GetActivityState( bool& bActivitiesSuspended, bool& bNetworkSuspended )
{
    wxInt32     iRetVal = 0;

    wxTimeSpan ts(wxDateTime::Now() - m_dtCachedActivityStateTimestamp);
    if ( ts.GetSeconds() > 10 )
    {
        m_dtCachedActivityStateTimestamp = wxDateTime::Now();

        if ( IsConnected() )
        {
			iRetVal = rpc.get_activity_state( bActivitiesSuspended, bNetworkSuspended );
            if ( 0 == iRetVal )
            {
                m_iCachedActivitiesSuspended = bActivitiesSuspended;
                m_iCachedNetworkSuspended = bNetworkSuspended;
            }
        }
    }
    else
    {
        bActivitiesSuspended = m_iCachedActivitiesSuspended;
        bNetworkSuspended = m_iCachedNetworkSuspended;
    }

    return iRetVal;
}


wxInt32 CMainDocument::RunBenchmarks()
{
    return rpc.run_benchmarks();
}


wxInt32 CMainDocument::CoreClientQuit()
{
    return rpc.quit();
}


wxInt32 CMainDocument::CachedProjectStatusUpdate()
{
    wxInt32     iRetVal = 0;
    wxString    strEmpty = wxEmptyString;
    wxInt32 i = 0;

    iRetVal = rpc.get_project_status(project_status);
    if (iRetVal)
    {
        wxLogTrace("CMainDocument::CachedProjectStatusUpdate - Get Project Status Failed '%d'", iRetVal);
        Connect( strEmpty );
    }

    m_fProjectTotalResourceShare = 0.0;
    for (i=0; i < (long)project_status.projects.size(); i++) {
        m_fProjectTotalResourceShare += project_status.projects.at( i )->resource_share;
    }

    return iRetVal;
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

    try
    {
        if ( !project_status.projects.empty() )
            pProject = project_status.projects.at( iIndex );
    }
    catch ( std::out_of_range e )
    {
        pProject = NULL;
    }

    if ( NULL != pProject )
        strBuffer = pProject->project_name.c_str();

    return 0;
}


wxInt32 CMainDocument::GetProjectProjectURL( wxInt32 iIndex, wxString& strBuffer )
{
    PROJECT* pProject = NULL;

    try
    {
        if ( !project_status.projects.empty() )
            pProject = project_status.projects.at( iIndex );
    }
    catch ( std::out_of_range e )
    {
        pProject = NULL;
    }

    if ( NULL != pProject )
        strBuffer = pProject->master_url.c_str();

    return 0;
}


wxInt32 CMainDocument::GetProjectAccountName( wxInt32 iIndex, wxString& strBuffer )
{
    PROJECT* pProject = NULL;

    try
    {
        if ( !project_status.projects.empty() )
            pProject = project_status.projects.at( iIndex );
    }
    catch ( std::out_of_range e )
    {
        pProject = NULL;
    }

    if ( NULL != pProject )
        strBuffer = pProject->user_name.c_str();

    return 0;
}


wxInt32 CMainDocument::GetProjectTeamName( wxInt32 iIndex, wxString& strBuffer )
{
    PROJECT* pProject = NULL;

    try
    {
        if ( !project_status.projects.empty() )
            pProject = project_status.projects.at( iIndex );
    }
    catch ( std::out_of_range e )
    {
        pProject = NULL;
    }

    if ( NULL != pProject )
        strBuffer = pProject->team_name.c_str();

    return 0;
}


wxInt32 CMainDocument::GetProjectTotalCredit( wxInt32 iIndex, float& fBuffer )
{
    PROJECT* pProject = NULL;

    try
    {
        if ( !project_status.projects.empty() )
            pProject = project_status.projects.at( iIndex );
    }
    catch ( std::out_of_range e )
    {
        pProject = NULL;
    }

    if ( NULL != pProject )
        fBuffer = pProject->user_total_credit;

    return 0;
}


wxInt32 CMainDocument::GetProjectAvgCredit( wxInt32 iIndex, float& fBuffer )
{
    PROJECT* pProject = NULL;

    try
    {
        if ( !project_status.projects.empty() )
            pProject = project_status.projects.at( iIndex );
    }
    catch ( std::out_of_range e )
    {
        pProject = NULL;
    }

    if ( NULL != pProject )
        fBuffer = pProject->user_expavg_credit;

    return 0;
}


wxInt32 CMainDocument::GetProjectResourceShare( wxInt32 iIndex, float& fBuffer )
{
    PROJECT* pProject = NULL;

    try
    {
        if ( !project_status.projects.empty() )
            pProject = project_status.projects.at( iIndex );
    }
    catch ( std::out_of_range e )
    {
        pProject = NULL;
    }

    if ( NULL != pProject )
        fBuffer = pProject->resource_share;

    return 0;
}


wxInt32 CMainDocument::GetProjectTotalResourceShare( wxInt32 WXUNUSED(iIndex), float& fBuffer )
{
    fBuffer = m_fProjectTotalResourceShare;
    return 0;
}


wxInt32 CMainDocument::GetProjectMinRPCTime( wxInt32 iIndex, wxInt32& iBuffer )
{
    PROJECT* pProject = NULL;

    try
    {
        if ( !project_status.projects.empty() )
            pProject = project_status.projects.at( iIndex );
    }
    catch ( std::out_of_range e )
    {
        pProject = NULL;
    }

    if ( NULL != pProject ) {
        iBuffer = (int)pProject->min_rpc_time;
	}

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

    try
    {
        if ( !project_status.projects.empty() )
            pProject = project_status.projects.at( iIndex );
    }
    catch ( std::out_of_range e )
    {
        pProject = NULL;
    }

    if ( NULL != pProject )
        bRetVal = pProject->suspended_via_gui;

    return bRetVal;
}


bool CMainDocument::IsProjectRPCPending( wxInt32 iIndex )
{
    PROJECT* pProject = NULL;
    bool     bRetVal  = false;

    try
    {
        if ( !project_status.projects.empty() )
            pProject = project_status.projects.at( iIndex );
    }
    catch ( std::out_of_range e )
    {
        pProject = NULL;
    }

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

    try
    {
        if ( !project_status.projects.empty() )
            pProject = project_status.projects.at( iIndex );
    }
    catch ( std::out_of_range e )
    {
        pProject = NULL;
    }

    if ( NULL != pProject )
        iRetVal = rpc.project_op( (*pProject), wxT("detach") );

    return iRetVal;
}


wxInt32 CMainDocument::ProjectUpdate( wxInt32 iIndex )
{
    PROJECT* pProject = NULL;
    wxInt32 iRetVal = -1;

    try
    {
        if ( !project_status.projects.empty() )
            pProject = project_status.projects.at( iIndex );
    }
    catch ( std::out_of_range e )
    {
        pProject = NULL;
    }

    if ( NULL != pProject )
        iRetVal = rpc.project_op( (*pProject), wxT("update") );

    return iRetVal;
}


wxInt32 CMainDocument::ProjectReset( wxInt32 iIndex )
{
    PROJECT* pProject = NULL;
    wxInt32 iRetVal = -1;

    try
    {
        if ( !project_status.projects.empty() )
            pProject = project_status.projects.at( iIndex );
    }
    catch ( std::out_of_range e )
    {
        pProject = NULL;
    }

    if ( NULL != pProject )
        iRetVal = rpc.project_op( (*pProject), wxT("reset") );

    return iRetVal;
}


wxInt32 CMainDocument::ProjectSuspend( wxInt32 iIndex )
{
    PROJECT* pProject = NULL;
    PROJECT* pStateProject = NULL;
    wxInt32 iRetVal = -1;

    try
    {
        if ( !project_status.projects.empty() )
            pProject = project_status.projects.at( iIndex );
    }
    catch ( std::out_of_range e )
    {
        pProject = NULL;
    }

    if ( NULL != pProject )
    {
        iRetVal = rpc.project_op( (*pProject), wxT("suspend") );
        if ( 0 == iRetVal )
        {
            pProject->suspended_via_gui = true;
            pStateProject = state.lookup_project( pProject->master_url );
            if ( NULL != pStateProject )
                pStateProject->suspended_via_gui = true;
            else
                ForceCacheUpdate();
        }
    }

    return iRetVal;
}


wxInt32 CMainDocument::ProjectResume( wxInt32 iIndex )
{
    PROJECT* pProject = NULL;
    PROJECT* pStateProject = NULL;
    wxInt32 iRetVal = -1;

    try
    {
        if ( !project_status.projects.empty() )
            pProject = project_status.projects.at( iIndex );
    }
    catch ( std::out_of_range e )
    {
        pProject = NULL;
    }

    if ( NULL != pProject )
    {
        iRetVal = rpc.project_op( (*pProject), wxT("resume") );
        if ( 0 == iRetVal )
        {
            pProject->suspended_via_gui = false;
            pStateProject = state.lookup_project( pProject->master_url );
            if ( NULL != pStateProject )
                pStateProject->suspended_via_gui = false;
            else
                ForceCacheUpdate();
        }
    }

    return iRetVal;
}


wxInt32 CMainDocument::CachedResultsStatusUpdate()
{
    wxInt32     iRetVal = 0;
    wxString    strEmpty = wxEmptyString;

    iRetVal = rpc.get_results(results);
    if (iRetVal)
    {
        wxLogTrace("CMainDocument::CachedResultsStatusUpdate - Get Result Status Failed '%d'", iRetVal);
        Connect( strEmpty );
    }

    return iRetVal;
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

    try
    {
        if ( !results.results.empty() )
            pResult = results.results.at( iIndex );
    }
    catch ( std::out_of_range e )
    {
        pResult = NULL;
    }

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
        else
            ForceCacheUpdate();
    }

    return 0;
}


wxInt32 CMainDocument::GetWorkProjectURL( wxInt32 iIndex, wxString& strBuffer )
{
    RESULT* pResult = NULL;

    try
    {
        if ( !results.results.empty() )
            pResult = results.results.at( iIndex );
    }
    catch ( std::out_of_range e )
    {
        pResult = NULL;
    }

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

    try
    {
        if ( !results.results.empty() )
            pResult = results.results.at( iIndex );
    }
    catch ( std::out_of_range e )
    {
        pResult = NULL;
    }

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
        else
            ForceCacheUpdate();
    }

    return 0;
}


wxInt32 CMainDocument::GetWorkApplicationVersion( wxInt32 iIndex, wxInt32& iBuffer )
{
    RESULT* pResult = NULL;
    RESULT* pStateResult = NULL;
    WORKUNIT* pWorkunit = NULL;
    APP_VERSION* pAppVersion = NULL;

    try
    {
        if ( !results.results.empty() )
            pResult = results.results.at( iIndex );
    }
    catch ( std::out_of_range e )
    {
        pResult = NULL;
    }

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
        else
            ForceCacheUpdate();
    }

    return 0;
}


wxInt32 CMainDocument::GetWorkName( wxInt32 iIndex, wxString& strBuffer )
{
    RESULT* pResult = NULL;

    try
    {
        if ( !results.results.empty() )
            pResult = results.results.at( iIndex );
    }
    catch ( std::out_of_range e )
    {
        pResult = NULL;
    }

    if ( NULL != pResult )
        strBuffer = pResult->name.c_str();

    return 0;
}


wxInt32 CMainDocument::GetWorkCurrentCPUTime( wxInt32 iIndex, float& fBuffer )
{
    RESULT* pResult = NULL;

    try
    {
        if ( !results.results.empty() )
            pResult = results.results.at( iIndex );
    }
    catch ( std::out_of_range e )
    {
        pResult = NULL;
    }

    if ( NULL != pResult )
        fBuffer = pResult->current_cpu_time;

    return 0;
}


wxInt32 CMainDocument::GetWorkEstimatedCPUTime( wxInt32 iIndex, float& fBuffer )
{
    RESULT* pResult = NULL;

    try
    {
        if ( !results.results.empty() )
            pResult = results.results.at( iIndex );
    }
    catch ( std::out_of_range e )
    {
        pResult = NULL;
    }

    if ( NULL != pResult )
        fBuffer = pResult->estimated_cpu_time_remaining;

    return 0;
}


wxInt32 CMainDocument::GetWorkFinalCPUTime( wxInt32 iIndex, float& fBuffer )
{
    RESULT* pResult = NULL;

    try
    {
        if ( !results.results.empty() )
            pResult = results.results.at( iIndex );
    }
    catch ( std::out_of_range e )
    {
        pResult = NULL;
    }

    if ( NULL != pResult )
        fBuffer = pResult->final_cpu_time;

    return 0;
}


wxInt32 CMainDocument::GetWorkFractionDone( wxInt32 iIndex, float& fBuffer )
{
    RESULT* pResult = NULL;

    try
    {
        if ( !results.results.empty() )
            pResult = results.results.at( iIndex );
    }
    catch ( std::out_of_range e )
    {
        pResult = NULL;
    }

    if ( NULL != pResult )
        fBuffer = pResult->fraction_done;

    return 0;
}


wxInt32 CMainDocument::GetWorkReportDeadline( wxInt32 iIndex, wxInt32& iBuffer )
{
    RESULT* pResult = NULL;

    try
    {
        if ( !results.results.empty() )
            pResult = results.results.at( iIndex );
    }
    catch ( std::out_of_range e )
    {
        pResult = NULL;
    }

    if ( NULL != pResult )
        iBuffer = pResult->report_deadline;

    return 0;
}


wxInt32 CMainDocument::GetWorkState( wxInt32 iIndex )
{
    wxInt32 iBuffer = 0;
    RESULT* pResult = NULL;

    try
    {
        if ( !results.results.empty() )
            pResult = results.results.at( iIndex );
    }
    catch ( std::out_of_range e )
    {
        pResult = NULL;
    }

    if ( NULL != pResult )
        iBuffer = pResult->state;

    return iBuffer;
}


wxInt32 CMainDocument::GetWorkSchedulerState( wxInt32 iIndex )
{
    wxInt32 iBuffer = 0;
    RESULT* pResult = NULL;

    try
    {
        if ( !results.results.empty() )
            pResult = results.results.at( iIndex );
    }
    catch ( std::out_of_range e )
    {
        pResult = NULL;
    }

    if ( NULL != pResult )
        iBuffer = pResult->scheduler_state;

    return iBuffer;
}


bool CMainDocument::IsWorkAborted( wxInt32 iIndex )
{
    RESULT* pResult = NULL;
    bool bRetVal    = false;

    try
    {
        if ( !results.results.empty() )
            pResult = results.results.at( iIndex );
    }
    catch ( std::out_of_range e )
    {
        pResult = NULL;
    }

    if ( NULL != pResult )
        bRetVal = pResult->aborted_via_gui;

    return bRetVal;
}


bool CMainDocument::IsWorkAcknowledged( wxInt32 iIndex )
{
    RESULT* pResult = NULL;
    bool bRetVal    = false;

    try
    {
        if ( !results.results.empty() )
            pResult = results.results.at( iIndex );
    }
    catch ( std::out_of_range e )
    {
        pResult = NULL;
    }

    if ( NULL != pResult )
        bRetVal = pResult->got_server_ack;

    return bRetVal;
}


bool CMainDocument::IsWorkActive( wxInt32 iIndex )
{
    RESULT* pResult = NULL;
    bool bRetVal    = false;

    try
    {
        if ( !results.results.empty() )
            pResult = results.results.at( iIndex );
    }
    catch ( std::out_of_range e )
    {
        pResult = NULL;
    }

    if ( NULL != pResult )
        bRetVal = pResult->active_task;

    return bRetVal;
}


bool CMainDocument::IsWorkReadyToReport( wxInt32 iIndex )
{
    RESULT* pResult = NULL;
    bool bRetVal    = false;

    try
    {
        if ( !results.results.empty() )
            pResult = results.results.at( iIndex );
    }
    catch ( std::out_of_range e )
    {
        pResult = NULL;
    }

    if ( NULL != pResult )
        bRetVal = pResult->ready_to_report;

    return bRetVal;
}


bool CMainDocument::IsWorkSuspended( wxInt32 iIndex )
{
    RESULT* pResult = NULL;
    bool bRetVal    = false;

    try
    {
        if ( !results.results.empty() )
            pResult = results.results.at( iIndex );
    }
    catch ( std::out_of_range e )
    {
        pResult = NULL;
    }

    if ( NULL != pResult )
        bRetVal = pResult->suspended_via_gui;

    return bRetVal;
}


bool CMainDocument::IsWorkGraphicsSupported( wxInt32 iIndex )
{
    RESULT* pResult = NULL;
    bool bRetVal    = false;

    try
    {
        if ( !results.results.empty() )
            pResult = results.results.at( iIndex );
    }
    catch ( std::out_of_range e )
    {
        pResult = NULL;
    }

    if ( NULL != pResult )
        bRetVal = pResult->supports_graphics;

    return bRetVal;
}


wxInt32 CMainDocument::WorkSuspend( wxInt32 iIndex )
{
    RESULT* pResult = NULL;
    RESULT* pStateResult = NULL;
    wxInt32 iRetVal = 0;

    try
    {
        if ( !results.results.empty() )
            pResult = results.results.at( iIndex );
    }
    catch ( std::out_of_range e )
    {
        pResult = NULL;
    }

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
        else
            ForceCacheUpdate();
    }

    return iRetVal;
}


wxInt32 CMainDocument::WorkResume( wxInt32 iIndex )
{
    RESULT* pResult = NULL;
    RESULT* pStateResult = NULL;
    wxInt32 iRetVal = 0;

    try
    {
        if ( !results.results.empty() )
            pResult = results.results.at( iIndex );
    }
    catch ( std::out_of_range e )
    {
        pResult = NULL;
    }

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
        else
            ForceCacheUpdate();
    }

    return iRetVal;
}


wxInt32 CMainDocument::WorkShowGraphics( wxInt32 iIndex, bool bFullScreen,
    std::string WindowStation, std::string Desktop, std::string Display)
{
    RESULT* pResult = NULL;
    wxInt32 iRetVal = 0;

    try
    {
        if ( !results.results.empty() )
            pResult = results.results.at( iIndex );
    }
    catch ( std::out_of_range e )
    {
        pResult = NULL;
    }

    if ( NULL != pResult ) {
        DISPLAY_INFO di;
        strcpy(di.window_station, WindowStation.c_str());
        strcpy(di.desktop, Desktop.c_str());
        strcpy(di.display, Display.c_str());

        iRetVal = rpc.show_graphics(
            pResult->project_url.c_str(), pResult->name.c_str(),
            bFullScreen, di
        );
    }

    return iRetVal;
}


wxInt32 CMainDocument::WorkAbort( wxInt32 iIndex )
{
    RESULT* pResult = NULL;
    RESULT* pStateResult = NULL;
    wxInt32 iRetVal = 0;

    try
    {
        if ( !results.results.empty() )
            pResult = results.results.at( iIndex );
    }
    catch ( std::out_of_range e )
    {
        pResult = NULL;
    }

    if ( NULL != pResult )
    {
        pStateResult = state.lookup_result( pResult->project_url, pResult->name );
        if ( NULL != pStateResult )
        {
            iRetVal = rpc.result_op( (*pStateResult), wxT("abort") );
            if ( 0 == iRetVal )
            {
                pResult->aborted_via_gui = true;
                pStateResult->aborted_via_gui = true;
            }
        }
        else
            ForceCacheUpdate();
    }

    return iRetVal;
}


wxInt32 CMainDocument::CachedMessageUpdate()
{
    wxInt32     iRetVal = 0;
    wxString    strEmpty = wxEmptyString;

    iRetVal = rpc.get_messages( m_iMessageSequenceNumber, messages );
    if (iRetVal)
    {
        wxLogTrace("CMainDocument::CachedMessageUpdate - Get Messages Failed '%d'", iRetVal);
        Connect( strEmpty );
    }

    if ( messages.messages.size() != 0 )
        m_iMessageSequenceNumber = messages.messages.at( messages.messages.size()-1 )->seqno;

    return iRetVal;
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

    try
    {
        if ( !messages.messages.empty() )
            pMessage = messages.messages.at( iIndex );
    }
    catch ( std::out_of_range e )
    {
        pMessage = NULL;
    }

    if ( NULL != pMessage )
        strBuffer = pMessage->project.c_str();

    return 0;
}


wxInt32 CMainDocument::GetMessageTime( wxInt32 iIndex, wxDateTime& dtBuffer ) 
{
    MESSAGE* pMessage = NULL;

    try
    {
        if ( !messages.messages.empty() )
            pMessage = messages.messages.at( iIndex );
    }
    catch ( std::out_of_range e )
    {
        pMessage = NULL;
    }

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

    try
    {
        if ( !messages.messages.empty() )
            pMessage = messages.messages.at( iIndex );
    }
    catch ( std::out_of_range e )
    {
        pMessage = NULL;
    }

    if ( NULL != pMessage )
        iBuffer = pMessage->priority;

    return 0;
}


wxInt32 CMainDocument::GetMessageMessage( wxInt32 iIndex, wxString& strBuffer ) 
{
    MESSAGE* pMessage = NULL;

    try
    {
        if ( !messages.messages.empty() )
            pMessage = messages.messages.at( iIndex );
    }
    catch ( std::out_of_range e )
    {
        pMessage = NULL;
    }

    if ( NULL != pMessage )
        strBuffer = pMessage->body.c_str();

    return 0;
}


wxInt32 CMainDocument::ResetMessageState()
{
    messages.clear();
    m_iMessageSequenceNumber = 0;
    return 0;
}

wxInt32 CMainDocument::CachedFileTransfersUpdate()
{
    wxInt32     iRetVal = 0;
    wxString    strEmpty = wxEmptyString;

    iRetVal = rpc.get_file_transfers( ft );
    if (iRetVal)
    {
        wxLogTrace("CMainDocument::CachedFileTransfersUpdate - Get File Transfers Failed '%d'", iRetVal);
        Connect( strEmpty );
    }

    return iRetVal;
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

    try
    {
        if ( !ft.file_transfers.empty() )
            pFT = ft.file_transfers.at( iIndex );
    }
    catch ( std::out_of_range e )
    {
        pFT = NULL;
    }

    if ( NULL != pFT )
        strBuffer = pFT->project_name.c_str();

    return 0;
}


wxInt32 CMainDocument::GetTransferFileName( wxInt32 iIndex, wxString& strBuffer )
{
    FILE_TRANSFER* pFT = NULL;

    try
    {
        if ( !ft.file_transfers.empty() )
            pFT = ft.file_transfers.at( iIndex );
    }
    catch ( std::out_of_range e )
    {
        pFT = NULL;
    }

    if ( NULL != pFT )
        strBuffer = pFT->name.c_str();

    return 0;
}


wxInt32 CMainDocument::GetTransferFileSize( wxInt32 iIndex, float& fBuffer )
{
    FILE_TRANSFER* pFT = NULL;

    try
    {
        if ( !ft.file_transfers.empty() )
            pFT = ft.file_transfers.at( iIndex );
    }
    catch ( std::out_of_range e )
    {
        pFT = NULL;
    }

    if ( NULL != pFT )
        fBuffer = pFT->nbytes;

    return 0;
}


wxInt32 CMainDocument::GetTransferBytesXfered( wxInt32 iIndex, float& fBuffer )
{
    FILE_TRANSFER* pFT = NULL;

    try
    {
        if ( !ft.file_transfers.empty() )
            pFT = ft.file_transfers.at( iIndex );
    }
    catch ( std::out_of_range e )
    {
        pFT = NULL;
    }

    if ( NULL != pFT )
        fBuffer = pFT->bytes_xferred;

    return 0;
}


wxInt32 CMainDocument::GetTransferSpeed( wxInt32 iIndex, float& fBuffer )
{
    FILE_TRANSFER* pFT = NULL;

    try
    {
        if ( !ft.file_transfers.empty() )
            pFT = ft.file_transfers.at( iIndex );
    }
    catch ( std::out_of_range e )
    {
        pFT = NULL;
    }

    if ( NULL != pFT )
        fBuffer = pFT->xfer_speed;

    return 0;
}


wxInt32 CMainDocument::GetTransferTime( wxInt32 iIndex, float& fBuffer )
{
    FILE_TRANSFER* pFT = NULL;

    try
    {
        if ( !ft.file_transfers.empty() )
            pFT = ft.file_transfers.at( iIndex );
    }
    catch ( std::out_of_range e )
    {
        pFT = NULL;
    }

    if ( NULL != pFT )
        fBuffer = pFT->time_so_far;

    return 0;
}


wxInt32 CMainDocument::GetTransferNextRequestTime( wxInt32 iIndex, wxInt32& iBuffer )
{
    FILE_TRANSFER* pFT = NULL;

    try
    {
        if ( !ft.file_transfers.empty() )
            pFT = ft.file_transfers.at( iIndex );
    }
    catch ( std::out_of_range e )
    {
        pFT = NULL;
    }

    if ( NULL != pFT )
        iBuffer = pFT->next_request_time;

    return 0;
}


wxInt32 CMainDocument::GetTransferStatus( wxInt32 iIndex, wxInt32& iBuffer )
{
    FILE_TRANSFER* pFT = NULL;

    try
    {
        if ( !ft.file_transfers.empty() )
            pFT = ft.file_transfers.at( iIndex );
    }
    catch ( std::out_of_range e )
    {
        pFT = NULL;
    }

    if ( NULL != pFT )
        iBuffer = pFT->status;

    return 0;
}


bool CMainDocument::IsTransferActive( wxInt32 iIndex )
{
    FILE_TRANSFER* pFT = NULL;
    bool bRetVal    = false;

    try
    {
        if ( !ft.file_transfers.empty() )
            pFT = ft.file_transfers.at( iIndex );
    }
    catch ( std::out_of_range e )
    {
        pFT = NULL;
    }

    if ( NULL != pFT )
        bRetVal = pFT->pers_xfer_active;

    return bRetVal;
}

bool CMainDocument::IsTransferGeneratedLocally( wxInt32 iIndex )
{
    FILE_TRANSFER* pFT = NULL;
    bool bRetVal    = false;

    try
    {
        if ( !ft.file_transfers.empty() )
            pFT = ft.file_transfers.at( iIndex );
    }
    catch ( std::out_of_range e )
    {
        pFT = NULL;
    }

    if ( NULL != pFT )
        bRetVal = pFT->generated_locally;

    return bRetVal;
}


wxInt32 CMainDocument::TransferRetryNow( wxInt32 iIndex )
{
    FILE_TRANSFER* pFT = NULL;
    wxInt32 iRetVal = 0;

    try
    {
        if ( !ft.file_transfers.empty() )
            pFT = ft.file_transfers.at( iIndex );
    }
    catch ( std::out_of_range e )
    {
        pFT = NULL;
    }

    if ( NULL != pFT )
        iRetVal = rpc.file_transfer_op( (*pFT), wxT("retry") );

    return iRetVal;
}


wxInt32 CMainDocument::TransferAbort( wxInt32 iIndex )
{
    FILE_TRANSFER* pFT = NULL;
    wxInt32 iRetVal = 0;

    try
    {
        if ( !ft.file_transfers.empty() )
            pFT = ft.file_transfers.at( iIndex );
    }
    catch ( std::out_of_range e )
    {
        pFT = NULL;
    }

    if ( NULL != pFT )
        iRetVal = rpc.file_transfer_op( (*pFT), wxT("abort") );

    return iRetVal;
}


wxInt32 CMainDocument::CachedResourceStatusUpdate()
{
    wxInt32     iRetVal = 0;
    wxString    strEmpty = wxEmptyString;

    iRetVal = rpc.get_disk_usage(resource_status);
    if (iRetVal)
    {
        wxLogTrace("CMainDocument::CachedResourceStatusUpdate - Get Disk Usage Failed '%d'", iRetVal);
        Connect( strEmpty );
    }

    return iRetVal;
}


wxInt32 CMainDocument::GetResourceCount()
{
    wxInt32 iCount = 0;

    CachedStateUpdate();
    CachedResourceStatusUpdate();

    if ( !resource_status.projects.empty() )
        iCount = resource_status.projects.size();

    return iCount;
}


wxInt32 CMainDocument::GetResourceProjectName( wxInt32 iIndex, wxString& strBuffer )
{
    PROJECT* pProject = NULL;
    PROJECT* pStateProject = NULL;

    try
    {
        if ( !resource_status.projects.empty() )
            pProject = resource_status.projects.at( iIndex );
    }
    catch ( std::out_of_range e )
    {
        pProject = NULL;
    }

    if ( NULL != pProject )
    {
        pStateProject = state.lookup_project( pProject->master_url );
        if ( NULL != pStateProject )
        {
            strBuffer = pStateProject->project_name.c_str();
        }
        else
            ForceCacheUpdate();
    }

    return 0;
}


wxInt32 CMainDocument::GetResourceDiskspace( wxInt32 iIndex, float& fBuffer )
{
    PROJECT* pProject = NULL;

    try
    {
        if ( !resource_status.projects.empty() )
            pProject = resource_status.projects.at( iIndex );
    }
    catch ( std::out_of_range e )
    {
        pProject = NULL;
    }

    if ( NULL != pProject )
        fBuffer = pProject->disk_usage;

    return 0;
}


wxInt32 CMainDocument::GetProxyConfiguration()
{
    wxInt32     iRetVal = 0;
    wxString    strEmpty = wxEmptyString;

	iRetVal = rpc.get_proxy_settings(proxy_info);
    if (iRetVal)
    {
        wxLogTrace("CMainDocument::GetProxyInfo - Get Proxy Info Failed '%d'", iRetVal);
        Connect( strEmpty );
    }

    return iRetVal;
}


wxInt32 CMainDocument::GetProxyHTTPProxyEnabled( bool& bEnabled )
{
    bEnabled = proxy_info.use_http_proxy;
    return 0;
}


wxInt32 CMainDocument::GetProxyHTTPServerName( wxString& strServerName )
{
    strServerName.Clear();
    strServerName = proxy_info.http_server_name.c_str();
    return 0;
}


wxInt32 CMainDocument::GetProxyHTTPServerPort( wxInt32& iPortNumber )
{
    iPortNumber = proxy_info.http_server_port;
    return 0;
}


wxInt32 CMainDocument::GetProxyHTTPUserName( wxString& strUserName )
{
    strUserName.Clear();
    strUserName = proxy_info.http_user_name.c_str();
    return 0;
}


wxInt32 CMainDocument::GetProxyHTTPPassword( wxString& strPassword )
{
    strPassword.Clear();
    strPassword = proxy_info.http_user_passwd.c_str();
    return 0;
}


wxInt32 CMainDocument::GetProxySOCKSProxyEnabled( bool& bEnabled )
{
    bEnabled = proxy_info.use_socks_proxy;
    return 0;
}


wxInt32 CMainDocument::GetProxySOCKSServerName( wxString& strServerName )
{
    strServerName.Clear();
    strServerName = proxy_info.socks_server_name.c_str();
    return 0;
}


wxInt32 CMainDocument::GetProxySOCKSServerPort( wxInt32& iPortNumber )
{
    iPortNumber = proxy_info.socks_server_port;
    return 0;
}


wxInt32 CMainDocument::GetProxySOCKSUserName( wxString& strUserName )
{
    strUserName.Clear();
    strUserName = proxy_info.socks5_user_name.c_str();
    return 0;
}


wxInt32 CMainDocument::GetProxySOCKSPassword( wxString& strPassword )
{
    strPassword.Clear();
    strPassword = proxy_info.socks5_user_passwd.c_str();
    return 0;
}

    
wxInt32 CMainDocument::SetProxyConfiguration()
{
    wxInt32     iRetVal = 0;
    wxString    strEmpty = wxEmptyString;

    if ( !proxy_info.http_user_name.empty() || !proxy_info.http_user_passwd.empty() )
        proxy_info.use_http_authentication = true;

    proxy_info.socks_version = 4;
    if ( !proxy_info.socks5_user_name.empty() || !proxy_info.socks5_user_passwd.empty() )
        proxy_info.socks_version = 5;

	iRetVal = rpc.set_proxy_settings( proxy_info );
    if (iRetVal)
    {
        wxLogTrace("CMainDocument::SetProxyInfo - Set Proxy Info Failed '%d'", iRetVal);
        Connect( strEmpty );
    }

    return iRetVal;
}


wxInt32 CMainDocument::SetProxyHTTPProxyEnabled( const bool bEnabled )
{
    proxy_info.use_http_proxy = bEnabled;
    return 0;
}


wxInt32 CMainDocument::SetProxyHTTPServerName( const wxString& strServerName )
{
    proxy_info.http_server_name = strServerName.c_str();
    return 0;
}


wxInt32 CMainDocument::SetProxyHTTPServerPort( const wxInt32 iPortNumber )
{
    proxy_info.http_server_port = iPortNumber;
    return 0;
}


wxInt32 CMainDocument::SetProxyHTTPUserName( const wxString& strUserName )
{
    proxy_info.http_user_name = strUserName.c_str();
    return 0;
}


wxInt32 CMainDocument::SetProxyHTTPPassword( const wxString& strPassword )
{
    proxy_info.http_user_passwd = strPassword.c_str();
    return 0;
}


wxInt32 CMainDocument::SetProxySOCKSProxyEnabled( const bool bEnabled )
{
    proxy_info.use_socks_proxy = bEnabled;
    return 0;
}


wxInt32 CMainDocument::SetProxySOCKSServerName( const wxString& strServerName )
{
    proxy_info.socks_server_name = strServerName.c_str();
    return 0;
}


wxInt32 CMainDocument::SetProxySOCKSServerPort( const wxInt32 iPortNumber )
{
    proxy_info.socks_server_port = iPortNumber;
    return 0;
}


wxInt32 CMainDocument::SetProxySOCKSUserName( const wxString& strUserName )
{
    proxy_info.socks5_user_name = strUserName.c_str();
    return 0;
}


wxInt32 CMainDocument::SetProxySOCKSPassword( const wxString& strPassword )
{
    proxy_info.socks5_user_passwd = strPassword.c_str();
    return 0;
}


const char *BOINC_RCSID_aa03a835ba = "$Id$";
