// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2010-2012 University of California
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


CHTMLBrowserWnd::CHTMLBrowserWnd()
{
    m_pBrowserHost = NULL;
    m_hIcon = NULL;
    m_hIconSmall = NULL;
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
        ::GetSystemMetrics(SM_CXSMICON),
        ::GetSystemMetrics(SM_CYSMICON),
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

    // Query for the IHTMLBrowserHostUI interface so we can setup the browser with
    // information that doesn't change very much.
    CComQIPtr<IHTMLBrowserHostUI> pHostUI;
    hr = m_pBrowserHost->GetExternal((IDispatch**)&pHostUI);

    // Set the static information
    pHostUI->put_IsScreensaver(m_bScreensaverMode);
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

    // Show something to the user
    NavigateToStateURL(true);

    bHandled = TRUE;
	return 0;
}

LRESULT CHTMLBrowserWnd::OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    DestroyWindow();
    ::PostQuitMessage(0);
	bHandled = TRUE;
	return 0;
}

LRESULT CHTMLBrowserWnd::OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    m_pBrowserHost->MoveWindow(0, 0, LOWORD(lParam), HIWORD(lParam));
	bHandled = TRUE;
	return 0;
}

LRESULT CHTMLBrowserWnd::OnTimer(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	bHandled = TRUE;
	return 0;
}


STDMETHODIMP_(void) CHTMLBrowserWnd::OnNavigateComplete(IDispatch* pDisp, VARIANT* URL)
{
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
    bstr = m_strDefaultURL;

    // See if we need to override the default
    if        (status.abort_request || status.quit_request || status.no_heartbeat) {
        bstr = m_strQuitURL;
    } else if (status.suspended) {
        bstr = m_strSuspendedURL;
    } else if (status.network_suspended) {
        bstr = m_strNetworkSuspendedURL;
    } else {
        bstr = m_strRunningURL;
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
