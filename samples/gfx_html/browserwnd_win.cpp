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
#include "browser.h"
#include "browser_i.h"
#include "browser_win.h"
#include "browserlog.h"
#include "webapi.h"
#include "browsermain_win.h"
#include "browserctrl_win.h"
#include "browserwnd_win.h"


CHTMLBrowserWnd::CHTMLBrowserWnd()
{
    m_pBrowserHost = NULL;
    m_hIcon = NULL;
    m_hIconSmall = NULL;
    m_bInitializing = false;
    m_bScreensaverMode = false;
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
    m_strEmbeddedURL += "/index.html";

    //
    m_bInitializing = true;

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
    int bIsExiting = FALSE;
    double dExitTimeout = 0.0;

    bIsExiting = determine_exit_state(dExitTimeout);

    if (bIsExiting && (dExitTimeout > 5.0))
    {
        if (!is_htmlgfx_in_debug_mode())
        {
            PostMessage(WM_CLOSE);
        }
    }

    if (m_bInitializing)
    {
        m_bInitializing = false;

        // Forcefully switch to the state URL
        NavigateToStateURL(true);
    }
    else
    {
        // Switch to the correct state URL
        NavigateToStateURL(false);
    }

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
    std::string strDesiredURL;
    CComVariant vt;
    CComVariant vtTargetURL;
    CComBSTR strTargetURL;
    
    determine_state_url(strDesiredURL);
    strTargetURL = strDesiredURL.c_str();

    // Navigate to URL
    if ((m_strCurrentURL != strTargetURL) || bForce) {
        browserlog_msg("State Change Detected (%S).", strTargetURL.m_str);

        m_strCurrentURL = strTargetURL;
        vtTargetURL = strTargetURL;

        m_pBrowserCtrl->Navigate2(&vtTargetURL, &vt, &vt, &vt, &vt);
    }
}
