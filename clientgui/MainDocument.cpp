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
// Revision 1.17  2004/10/01 00:06:32  rwalton
// *** empty log message ***
//
// Revision 1.16  2004/09/30 20:32:15  davea
// *** empty log message ***
//
// Revision 1.15  2004/09/29 22:20:43  rwalton
// *** empty log message ***
//
// Revision 1.14  2004/09/28 01:19:46  rwalton
// *** empty log message ***
//
// Revision 1.13  2004/09/25 21:33:23  rwalton
// *** empty log message ***
//
// Revision 1.12  2004/09/10 23:17:07  rwalton
// *** empty log message ***
//
// Revision 1.11  2004/09/01 04:59:32  rwalton
// *** empty log message ***
//
// Revision 1.10  2004/08/11 23:52:11  rwalton
// *** empty log message ***
//
// Revision 1.9  2004/07/13 06:10:32  rwalton
// Fixed a typo
//
// Revision 1.8  2004/07/13 05:56:01  rwalton
// Hooked up the Project and Work tab for the new GUI.
//
// Revision 1.7  2004/07/12 08:46:25  rwalton
// Document parsing of the <get_state/> message
//
// Revision 1.6  2004/06/25 22:50:56  rwalton
// Client spamming server hotfix
//
// Revision 1.5  2004/05/17 22:15:09  rwalton
// *** empty log message ***
//
//

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma implementation "MainDocument.h"
#endif

#include "stdwx.h"
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
    fBuffer = this->m_fProjectTotalResourceShare;
    return 0;
}


wxInt32 CMainDocument::GetProjectMinRPCTime(wxInt32 iIndex, wxInt32& iBuffer)
{
    PROJECT* pProject = project_status.projects.at( iIndex );
    if ( NULL != pProject )
        iBuffer = pProject->min_rpc_time;

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





wxInt32 CMainDocument::GetWorkCount() {
    CachedStateUpdate();
    return state.results.size();
}


wxString CMainDocument::GetWorkProjectName(wxInt32 iIndex) {
    CachedStateUpdate();
    return state.results[iIndex]->project->project_name.c_str();
}


wxString CMainDocument::GetWorkApplicationName(wxInt32 iIndex) {
    CachedStateUpdate();
    return state.results[iIndex]->app->name.c_str();
}


wxString CMainDocument::GetWorkName(wxInt32 iIndex) {
    CachedStateUpdate();
    return state.results[iIndex]->name.c_str();
}


wxString CMainDocument::GetWorkCPUTime(wxInt32 iIndex) {
    CachedStateUpdate();
    return wxString::Format(_T(""));
}


wxString CMainDocument::GetWorkProgress(wxInt32 iIndex) {
    CachedStateUpdate();
    return wxString::Format(_T(""));
}


wxString CMainDocument::GetWorkTimeToCompletion(wxInt32 iIndex) {
    CachedStateUpdate();
    return wxString::Format(_T(""));
}


wxString CMainDocument::GetWorkReportDeadline(wxInt32 iIndex) {
    CachedStateUpdate();
    wxDateTime dtReportDeadline;
    dtReportDeadline.Set((time_t)state.results[iIndex]->report_deadline);
    return dtReportDeadline.Format();
}


wxString CMainDocument::GetWorkStatus(wxInt32 iIndex) {
    CachedStateUpdate();
    return wxString::Format(_T(""));
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


wxInt32 CMainDocument::CachedStateLock() {
    m_bCachedStateLocked = true;
    m_dtCachedStateLockTimestamp = wxDateTime::Now();
    return 0;
}


wxInt32 CMainDocument::CachedStateUnlock() {
    m_bCachedStateLocked = false;
    return 0;
}


wxInt32 CMainDocument::CachedStateUpdate() {

    wxInt32 retval = 0;

    wxTimeSpan ts(m_dtCachedStateLockTimestamp - m_dtCachedStateTimestamp);
    if (!m_bCachedStateLocked && (ts > wxTimeSpan::Seconds(300)))
    {
        wxLogTrace("CMainDocument::CachedStateUpdate - State Cache Updating...");
        m_dtCachedStateTimestamp = m_dtCachedStateLockTimestamp;

        if (!m_bIsConnected)
        {
            retval = rpc.init(NULL);
            if (retval)
                wxLogTrace("CMainDocument::CachedStateUpdate - RPC Initialization Failed '%d'", retval);
        }

        retval = rpc.get_state(state);
        if (retval)
            wxLogTrace("CMainDocument::CachedStateUpdate - Get State Failed '%d'", retval);

        retval = rpc.get_results(results);
        if (retval)
            wxLogTrace("CMainDocument::CachedStateUpdate - Get Results Failed '%d'", retval);

        retval = rpc.get_file_transfers(ft);
        if (retval)
            wxLogTrace("CMainDocument::CachedStateUpdate - Get File Transfers Failed '%d'", retval);

        wxLogTrace("CMainDocument::CachedStateUpdate - State Cache Updated...");
    }

    return retval;
}

