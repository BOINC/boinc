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


IMPLEMENT_DYNAMIC_CLASS(CMainDocument, CXMLParser)


CMainDocument::CMainDocument()
{
    m_bCachedStateLocked = false;
    m_dtCachedStateLockTimestamp = wxDateTime::Now();
    m_dtCachedStateTimestamp = 0;
}


CMainDocument::~CMainDocument()
{
    m_dtCachedStateTimestamp = wxDateTime::Now();
    m_dtCachedStateLockTimestamp = wxDateTime::Now();
    m_bCachedStateLocked = false;
}


wxInt32 CMainDocument::GetProjectCount() {
    CachedStateUpdate();
    return 0;
}


wxString CMainDocument::GetProjectProjectName(wxInt32 iIndex) {
    CachedStateUpdate();
    return wxString::Format(_T(""));
}


wxString CMainDocument::GetProjectAccountName(wxInt32 iIndex) {
    CachedStateUpdate();
    return wxString::Format(_T(""));
}


wxString CMainDocument::GetProjectTeamName(wxInt32 iIndex) {
    CachedStateUpdate();
    return wxString::Format(_T(""));
}


wxString CMainDocument::GetProjectTotalCredit(wxInt32 iIndex) {
    CachedStateUpdate();
    return wxString::Format(_T(""));
}


wxString CMainDocument::GetProjectAvgCredit(wxInt32 iIndex) {
    CachedStateUpdate();
    return wxString::Format(_T(""));
}


wxString CMainDocument::GetProjectResourceShare(wxInt32 iIndex) {
    CachedStateUpdate();
    return wxString::Format(_T(""));
}


wxInt32 CMainDocument::GetWorkCount() {
    CachedStateUpdate();
    return 0;
}


wxString CMainDocument::GetWorkProjectName(wxInt32 iIndex) {
    CachedStateUpdate();
    return wxString::Format(_T(""));
}


wxString CMainDocument::GetWorkApplicationName(wxInt32 iIndex) {
    CachedStateUpdate();
    return wxString::Format(_T(""));
}


wxString CMainDocument::GetWorkName(wxInt32 iIndex) {
    CachedStateUpdate();
    return wxString::Format(_T(""));
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
    return wxString::Format(_T(""));
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
    return 0;
}

