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
// Revision 1.7  2004/07/13 05:56:01  rwalton
// Hooked up the Project and Work tab for the new GUI.
//
// Revision 1.6  2004/07/12 08:46:25  rwalton
// Document parsing of the <get_state/> message
//
// Revision 1.5  2004/06/25 22:50:56  rwalton
// Client spamming server hotfix
//
// Revision 1.4  2004/05/17 22:15:09  rwalton
// *** empty log message ***
//
//

#ifndef _MAINDOCUMENT_H_
#define _MAINDOCUMENT_H_

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma interface "MainDocument.cpp"
#endif

#include "XMLParser.h"
#include "ActiveTask.h"
#include "App.h"
#include "AppVersion.h"
#include "FileInfo.h"
#include "Message.h"
#include "Project.h"
#include "ProxyInfo.h"
#include "Result.h"
#include "Workunit.h"


class CMainDocument : public CXMLParser
{
    DECLARE_DYNAMIC_CLASS(CMainDocument)

private:
    wxSocketClient*         m_pSocket;
    wxSocketOutputStream*   m_pSocketOutput;
    wxSocketInputStream*    m_pSocketInput;
    wxTextOutputStream*     m_pTextOutput;
    wxTextInputStream*      m_pTextInput;


    wxDateTime              m_dtCachedStateTimestamp;
    wxDateTime              m_dtCachedStateLockTimestamp;
    bool                    m_bCachedStateLocked;

    CArrayActiveTask        m_ActiveTasks;
    CArrayApp               m_Apps;
    CArrayAppVersion        m_AppVersions;
    CArrayFileInfo          m_FileInfos;
    CArrayProject           m_Projects;
    CArrayResult            m_Results;
    CArrayWorkunit          m_Workunits;

    CArrayMessage           m_Messages;
    
    CProxyInfo              m_ProxyInfo;

    wxInt32                 SendMessageToCore(wxString& strMessage, wxTextInputStream** pTextInput);

    wxInt32                 CachedStateUpdate();

    CApp*                   LookupApp(wxString& strName);
    CAppVersion*            LookupAppVersion(wxString& strName, wxInt32 iVersionNumber);
    CWorkunit*              LookupWorkunit(wxString& strName);
    CResult*                LookupResult(wxString& strName);

public:
    CMainDocument();
    ~CMainDocument();

    wxInt32                 CachedStateLock();
    wxInt32                 CachedStateUnlock();

    wxInt32                 GetProjectCount();
    wxString                GetProjectName(wxInt32 iIndex);
    wxString                GetProjectAccountName(wxInt32 iIndex);
    wxString                GetProjectTotalCredit(wxInt32 iIndex);
    wxString                GetProjectAvgCredit(wxInt32 iIndex);
    wxString                GetProjectResourceShare(wxInt32 iIndex);

    wxInt32                 GetResultCount();
    wxString                GetResultProjectName(wxInt32 iIndex);
    wxString                GetResultApplicationName(wxInt32 iIndex);
    wxString                GetResultName(wxInt32 iIndex);
    wxString                GetResultCPUTime(wxInt32 iIndex);
    wxString                GetResultProgress(wxInt32 iIndex);
    wxString                GetResultTimeToCompletion(wxInt32 iIndex);
    wxString                GetResultReportDeadline(wxInt32 iIndex);
    wxString                GetResultStatus(wxInt32 iIndex);

};

#endif

