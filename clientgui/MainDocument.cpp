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
    m_pSocket = new wxSocketClient();
}


CMainDocument::~CMainDocument()
{
    if (m_pSocket)
        delete m_pSocket;
}


wxInt32 CMainDocument::SendMessageToCore(wxString& strMessage, wxSocketInputStream** pSocketInput)
{
    if (!m_pSocket->IsConnected())
    {
        wxIPV4address addr;

        addr.Hostname("localhost");
        addr.Service(31416);

        if (!m_pSocket->Connect(addr, TRUE))
        {
            return ERR_CONNECT;
        }
    }

    wxSocketOutputStream* pSocketOutput = new wxSocketOutputStream((*m_pSocket));
    if (!pSocketOutput)
        return ERR_MALLOC;

    pSocketOutput->Write("<get_state/>", sizeof("<get_state/>"));

    (*pSocketInput) = new wxSocketInputStream((*m_pSocket));
    if (!(*pSocketInput))
        return ERR_MALLOC;
    
    delete pSocketOutput;

    return 0;
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


wxInt32 CMainDocument::CachedStateUpdate()
{
    wxSocketInputStream* pSocketInput = NULL;
    wxTextInputStream*   pTextInput = NULL;
    CProject*            pProject = NULL;
    wxString             strBuffer;
    wxString             strMessage;

    wxTimeSpan ts(m_dtCachedStateLockTimestamp - m_dtCachedStateTimestamp);
    if (!m_bCachedStateLocked || (m_bCachedStateLocked && (ts > wxTimeSpan::Seconds(1))))
    {
        m_dtCachedStateTimestamp = m_dtCachedStateLockTimestamp;

        strMessage = "<get_state/>";

        SendMessageToCore(strMessage, &pSocketInput);

        pTextInput = new wxTextInputStream((*pSocketInput));

        while (strBuffer = pTextInput->ReadLine()) 
        {
            if (match_tag(strBuffer, "</client_state>")) break;
            else if (match_tag(strBuffer, "<project>"))
            {
                pProject = new CProject;
                pProject->Parse(pTextInput);
                m_Projects.Add(pProject);
                continue;
            }
            else if (match_tag(strBuffer, "<app>"))
            {
                CApp* pApp = new CApp;
                pApp->Parse(pTextInput);
                pApp->SetProject(pProject);
                m_Apps.Add(pApp);
                continue;
            }
            else if (match_tag(strBuffer, "<app_version>"))
            {
                CAppVersion* pAppVersion = new CAppVersion;
                pAppVersion->Parse(pTextInput);
                pAppVersion->SetProject(pProject);
                pAppVersion->SetApp(LookupApp(pAppVersion->GetName()));
                m_AppVersions.Add(pAppVersion);
                continue;
            }
            else if (match_tag(strBuffer, "<workunit>"))
            {
                CWorkunit* pWorkunit = new CWorkunit;
                pWorkunit->Parse(pTextInput);
                pWorkunit->SetProject(pProject);
                pWorkunit->SetApp(LookupApp(pWorkunit->GetName()));
                pWorkunit->SetAppVersion(LookupAppVersion(pWorkunit->GetName(), pWorkunit->GetApplicationVersion()));
                m_Workunits.Add(pWorkunit);
                continue;
            }
            else if (match_tag(strBuffer, "<result>"))
            {
                CResult* pResult = new CResult;
                pResult->Parse(pTextInput);
                pResult->SetProject(pProject);
                pResult->SetWorkunit(LookupWorkunit(pResult->GetWorkunitName()));
                pResult->SetApp(pResult->GetWorkunit()->GetApp());
                m_Results.Add(pResult);
                continue;
            }
            else if (match_tag(strBuffer, "<file_info>"))
            {
                CFileInfo* pFileInfo = new CFileInfo;
                pFileInfo->Parse(pTextInput);
                pFileInfo->SetProject(pProject);
                m_FileInfos.Add(pFileInfo);
                continue;
            }
            else if (match_tag(strBuffer, "<active_task>"))
            {
                CActiveTask* pActiveTask = new CActiveTask;
                pActiveTask->Parse(pTextInput);
                pActiveTask->SetProject(pProject);
                pActiveTask->SetResult(LookupResult(pActiveTask->GetName()));
                m_ActiveTasks.Add(pActiveTask);
                continue;
            }
            else if (match_tag(strBuffer, "<proxy_info>"))
            {
                m_ProxyInfo.Parse(pTextInput);
                continue;
            }
        }
    }

    return 0;
}


CApp* CMainDocument::LookupApp(wxString& strName)
{
    wxLogTrace("CMainDocument::LookupApp - Function Begining");

    unsigned int i;
    CApp*        pApp = NULL;

    for (i=0; i<m_Apps.GetCount(); i++) {
        if (m_Apps.Item(i).GetName() == strName) pApp = &m_Apps.Item(i);
    }

    wxASSERT(NULL != pApp);

    wxLogTrace("CMainDocument::LookupApp - Function Ending");
    return pApp;
}


CWorkunit* CMainDocument::LookupWorkunit(wxString& strName)
{
    wxLogTrace("CMainDocument::LookupWorkunit - Function Begining");

    unsigned int i;
    CWorkunit*   pWorkunit = NULL;

    for (i=0; i<m_Workunits.GetCount(); i++) {
        if (m_Workunits.Item(i).GetName() == strName) pWorkunit = &m_Workunits.Item(i);
    }

    wxASSERT(NULL != pWorkunit);

    wxLogTrace("CMainDocument::LookupWorkunit - Function Ending");
    return pWorkunit;
}


CResult* CMainDocument::LookupResult(wxString& strName)
{
    wxLogTrace("CMainDocument::LookupResult - Function Begining");

    unsigned int i;
    CResult*     pResult = NULL;

    for (i=0; i<m_Results.GetCount(); i++) {
        if (m_Results.Item(i).GetName() == strName) pResult = &m_Results.Item(i);
    }

    wxASSERT(NULL != pResult);

    wxLogTrace("CMainDocument::LookupResult - Function Ending");
    return pResult;
}


CAppVersion* CMainDocument::LookupAppVersion(wxString& strName, int iVersionNumber)
{
    wxLogTrace("CMainDocument::LookupAppVersion - Function Begining");

    unsigned int i;
    CAppVersion* pAppVersion = NULL;

    for (i=0; i<m_AppVersions.GetCount(); i++) {
        if ((m_AppVersions.Item(i).GetName() == strName) && 
            (m_AppVersions.Item(i).GetAppVersionNumber() == iVersionNumber))
            pAppVersion = &m_AppVersions.Item(i);
    }

    wxASSERT(NULL != pAppVersion);

    wxLogTrace("CMainDocument::LookupAppVersion - Function Ending");
    return pAppVersion;
}

