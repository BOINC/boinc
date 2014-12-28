// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2014-2015 University of California
//
// BOINC is free software; you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation,
// either version 3 of the License, or (at your option) any later version.
//
// BOINC is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with BOINC.  If not, see <http://www.gnu.org/licenses/>.

#define _ATL_FREE_THREADED
#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS
#include <AtlBase.h>
#include <AtlCom.h>
#include <AtlCtl.h>
#include <AtlWin.h>
#include <AtlStr.h>
#include <AtlFile.h>
#include <AtlTypes.h>
#include <exdisp.h>
#include <exdispid.h>
#include <stdlib.h>
#include <string>
#include "util.h"
#include "win_util.h"
#include "version.h"
#include "boinc_api.h"
#include "diagnostics.h"
#include "filesys.h"
#include "browser_i.h"
#include "browser_win.h"
#include "browserlog.h"
#include "browserctrlui_win.h"
#include "browserctrl_win.h"
#include "browserwnd_win.h"
#include "graphics.h"
#include "vboxwrapper.h"


CHTMLBrowserWnd::CHTMLBrowserWnd()
{
    m_pBrowserHost = NULL;
    m_hIcon = NULL;
    m_hIconSmall = NULL;

    aid.clear();
    status.abort_request = 0;
    status.no_heartbeat = 0;
    status.quit_request = 0;
    status.reread_init_data_file = 0;
    status.suspended = 0;
    status.network_suspended = 0;
    m_dUpdateTime = 0.0;
    m_dCPUTime = 0.0;
    m_dElapsedTime = 0.0;
    m_dFractionDone = 0.0;
    m_bScreensaverMode = false;
    m_bVboxwrapperJob = false;
    m_lRemoteDesktopPort = 0;
    m_lWebAPIPort = 0;
}

CHTMLBrowserWnd::~CHTMLBrowserWnd()
{
    if(m_hIcon)
    {
        ::DestroyIcon(m_hIcon);
        m_hIcon = NULL;
    }

    if(m_hIconSmall)
    {
        ::DestroyIcon(m_hIconSmall);
        m_hIconSmall = NULL;
    }
}

LRESULT CHTMLBrowserWnd::OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    HRESULT hr;
	RECT rcClient;
    TCHAR szExecutable[MAX_PATH];
    CComPtr<IUnknown> pCtrl;


    // Load Icon Resources
    m_hIcon = (HICON)::LoadImage(
        _AtlBaseModule.GetResourceInstance(),
        MAKEINTRESOURCE(IDI_ICON),
        IMAGE_ICON,
        0, 0,
        LR_DEFAULTSIZE | LR_DEFAULTCOLOR);
    ATLASSERT(m_hIcon);

    m_hIconSmall = (HICON)::LoadImage(
        _AtlBaseModule.GetResourceInstance(),
        MAKEINTRESOURCE(IDI_ICON),
        IMAGE_ICON,
        GetSystemMetrics(SM_CXSMICON),
        GetSystemMetrics(SM_CYSMICON),
        LR_DEFAULTCOLOR);
    ATLASSERT(m_hIconSmall);

    SetIcon(m_hIcon);
    SetIcon(m_hIconSmall, FALSE);

    // Create Control Host
    hr = CComObject<CHTMLBrowserHost>::CreateInstance(&m_pBrowserHost);
    ATLASSERT(SUCCEEDED(hr));

    // Create Control Window
	GetClientRect(&rcClient);
    m_pBrowserHost->Create(m_hWnd, rcClient, NULL, WS_CHILD | WS_VISIBLE);
    ATLASSERT(m_pBrowserHost->IsWindow());

    // Create Control
    hr = m_pBrowserHost->CreateControlEx(
        L"Shell.Explorer",
        m_pBrowserHost->m_hWnd,
        NULL,
        &pCtrl,
        __uuidof(DWebBrowserEvents2),
        (IUnknown*)(IDispEventImpl<1, CHTMLBrowserWnd, &__uuidof(DWebBrowserEvents2), &LIBID_SHDocVw, 1, 1>*)this
    );

    // Get an IWebBrowser2 interface on the control and navigate to a page.
    m_pBrowserCtrl = pCtrl;

    // Configure the Embedded URL
    GetModuleFileName(NULL, szExecutable, sizeof(szExecutable));
    m_strEmbeddedURL += "res://";
    m_strEmbeddedURL += szExecutable;
    m_strEmbeddedURL += "/default_win.htm";

    // Stage rereading of all the state files
    status.reread_init_data_file = true;

    // Start the timer
    SetTimer(1, 1000);

    bHandled = TRUE;
	return 0;
}

LRESULT CHTMLBrowserWnd::OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    KillTimer(1);
    DestroyWindow();
    PostQuitMessage(0);
	bHandled = TRUE;
	return 0;
}

LRESULT CHTMLBrowserWnd::OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    m_pBrowserHost->MoveWindow(0, 0, LOWORD(lParam), HIWORD(lParam));
	bHandled = TRUE;
	return 0;
}

LRESULT CHTMLBrowserWnd::OnInputActivity(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    if (m_bScreensaverMode)
    {
        PostMessage(WM_CLOSE);
    }
    else
    {
        // Forward event to the browser control
        MSG msg = { m_hWnd, uMsg, wParam, lParam, 0, { 0, 0 } };
        SendMessage(m_pBrowserHost->m_hWnd, WM_FORWARDMSG, 0, (LPARAM)&msg);
    }
	bHandled = TRUE;
	return 0;
}

LRESULT CHTMLBrowserWnd::OnTimer(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    int retval = ERR_FREAD;
    HRESULT hr = E_FAIL;
    BOOL bExitStatus = false;
    double dExitCountdown = 0.0;
    int temp = 0;
    CComQIPtr<IHTMLBrowserHostUI> pHostUI;

    boinc_parse_graphics_status(
        &m_dUpdateTime,
        &m_dCPUTime,
        &m_dElapsedTime,
        &m_dFractionDone,
        &status
    );

    // Query for the IHTMLBrowserHostUI interface so we can setup the browser with
    // information that doesn't change very much.
    //
    hr = m_pBrowserHost->GetExternal((IDispatch**)&pHostUI);
    if (SUCCEEDED(hr) && pHostUI.p)
    {
        bExitStatus = status.abort_request || status.no_heartbeat || status.quit_request;
        if (bExitStatus && ((dtime() - m_dUpdateTime) > 5.0))
        {
            dExitCountdown = dtime() - m_dUpdateTime - 5;
        }
        else
        {
            dExitCountdown = 0.0;
        }

        if (dExitCountdown > 5.0)
        {
            PostMessage(WM_CLOSE);
        }

        pHostUI->SetSuspended(status.suspended);
        pHostUI->SetNetworkSuspended(status.network_suspended);
        pHostUI->SetExiting(bExitStatus);
        pHostUI->put_ExitCountdown(dExitCountdown);
        pHostUI->put_CPUTime(m_dCPUTime);
        pHostUI->put_ElapsedTime(m_dElapsedTime);
        pHostUI->put_FractionDone(m_dFractionDone);

        if (status.reread_init_data_file)
        {
            status.reread_init_data_file = 0;

            // Get updated state
            //
            if (aid.project_preferences) delete aid.project_preferences;
            boinc_parse_init_data_file();
            boinc_get_init_data(aid);

            // Inform the HTML Document DOM about the state changes
            //
            pHostUI->SetAppInitDataUpdate(TRUE);
            pHostUI->SetScreensaver(m_bScreensaverMode);
            pHostUI->put_ApplicationName(CComBSTR(aid.app_name));
            pHostUI->put_ApplicationVersion(aid.app_version);
            pHostUI->put_WorkunitName(CComBSTR(aid.wu_name));
            pHostUI->put_ResultName(CComBSTR(aid.result_name));
            pHostUI->put_UserName(CComBSTR(aid.user_name));
            pHostUI->put_TeamName(CComBSTR(aid.team_name));
            pHostUI->put_UserCreditTotal(aid.user_total_credit);
            pHostUI->put_UserCreditAverage(aid.user_expavg_credit);
            pHostUI->put_HostCreditTotal(aid.host_total_credit);
            pHostUI->put_HostCreditAverage(aid.host_expavg_credit);

            // Check for vboxwrapper state
            //
            m_bVboxwrapperJob = is_vboxwrapper_job();
            if (m_bVboxwrapperJob)
            {
                pHostUI->SetVboxwrapperJob(m_bVboxwrapperJob);
                if (0 == parse_vbox_remote_desktop_port(temp))
                {
                    m_lRemoteDesktopPort = temp;
                    pHostUI->put_RemoteDesktopPort(m_lRemoteDesktopPort);
                }
                if (0 == parse_vbox_webapi_port(temp))
                {
                    m_lWebAPIPort = temp;
                    pHostUI->put_WebAPIPort(m_lWebAPIPort);
                }
            }

            //
            //
            parse_graphics(
                m_strDefaultURL, m_strRunningURL, m_strSuspendedURL, m_strNetworkSuspendedURL, m_strExitingURL
            );

            // Forcefully switch to the required URL.
            NavigateToStateURL(true);
        }
    }

    // Switch to the correct URL
    NavigateToStateURL(false);

	bHandled = TRUE;
	return 0;
}


STDMETHODIMP_(void) CHTMLBrowserWnd::OnNewProcess(LONG lCauseFlag, IDispatch* pDisp, VARIANT_BOOL* pCancel)
{
    *pCancel = TRUE;
}

STDMETHODIMP_(void) CHTMLBrowserWnd::OnNewWindow2(IDispatch** ppDisp, VARIANT_BOOL* pCancel)
{
    *pCancel = TRUE;
}

STDMETHODIMP_(void) CHTMLBrowserWnd::OnNewWindow3(IDispatch** ppDisp, VARIANT_BOOL* pCancel, DWORD dwFlags, BSTR bstrUrlContext, BSTR bstrUrl)
{
    *pCancel = TRUE;
}


void CHTMLBrowserWnd::NavigateToStateURL(bool bForce)
{
    CComBSTR bstr;
    CComVariant v;
    
    // Start out with the default URL
    bstr = m_strDefaultURL.c_str();

    // See if we need to override the default
    if        ((status.abort_request || status.quit_request || status.no_heartbeat) && !m_strExitingURL.empty()) {
        bstr = m_strExitingURL.c_str();
    } else if (status.suspended && !m_strSuspendedURL.empty()) {
        bstr = m_strSuspendedURL.c_str();
    } else if (status.network_suspended && !m_strNetworkSuspendedURL.empty()) {
        bstr = m_strNetworkSuspendedURL.c_str();
    } else if (!m_strRunningURL.empty()) {
        bstr = m_strRunningURL.c_str();
    }

    // Are we running a vboxwrapper job?  If so, does it expose a webapi port number?
    if ((m_bVboxwrapperJob && m_lWebAPIPort) && (bstr.Length() > 0)) {
        bstr  = "http://localhost:";
        bstr += m_lWebAPIPort;
        bstr += "/";
    }

    // If nothing has been approved to the point, use the embedded HTML page
    if (bstr.Length() == 0) {
        bstr = m_strEmbeddedURL;
    }

    // Navigate to URL
    if ((m_strCurrentURL != bstr) || bForce) {
        m_strCurrentURL = bstr;
        m_pBrowserCtrl->Navigate(bstr, &v, &v, &v, &v);
    }
}
