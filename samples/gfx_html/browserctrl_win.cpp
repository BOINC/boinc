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
#include <mshtml.h>
#include <urlmon.h>
#include <string>
#include "win_util.h"
#include "version.h"
#include "boinc_api.h"
#include "diagnostics.h"
#include "filesys.h"
#include "browser_i.h"
#include "browser_win.h"
#include "browserlog.h"
#include "browserctrl_win.h"


// New for IE10
#ifndef IDM_ADDCONSOLEMESSAGERECEIVER
#define IDM_ADDCONSOLEMESSAGERECEIVER     3800
#endif
#ifndef IDM_REMOVECONSOLEMESSAGERECEIVER
#define IDM_REMOVECONSOLEMESSAGERECEIVER  3801
#endif


CWndClassInfo& CHTMLBrowserHost::GetWndClassInfo()
{
    static CWndClassInfo wc =
    {
        { sizeof(WNDCLASSEX), 0, StartWindowProc, 0, 0, 0, 0, 0, (HBRUSH)(COLOR_WINDOW + 1), 0, NULL, 0 },
        NULL, NULL, IDC_ARROW, TRUE, 0, _T("")
    };
    return wc;
}

HRESULT CHTMLBrowserHost::FinalConstruct()
{
    return S_OK;
}

void CHTMLBrowserHost::FinalRelease()
{
    HRESULT hr;

    // Unregister the developer console message receiver
    //
    CComPtr<IUnknown> pBrowser;
    hr = QueryControl(__uuidof(IWebBrowser2), (void**)&pBrowser);
    if (SUCCEEDED(hr))
    {
        CComPtr<IDispatch> spDocDisp;
        CComPtr<IWebBrowser2> spWebBrowser;
        spWebBrowser = pBrowser;
        hr = spWebBrowser->get_Document(&spDocDisp);
        if (SUCCEEDED(hr))
        {
            CComQIPtr<IOleCommandTarget> spOleCommandTarget(spDocDisp);
            spOleCommandTarget->Exec(
                &CGID_MSHTML,
                IDM_REMOVECONSOLEMESSAGERECEIVER,
                OLECMDEXECOPT_DODEFAULT,
                &varConsoleCookie,
                NULL
            );
        }
    }

	ReleaseAll();
}

void CHTMLBrowserHost::OnFinalMessage(HWND /*hWnd*/)
{
}

HWND CHTMLBrowserHost::Create(
    HWND hWndParent, _U_RECT rect, LPCTSTR szWindowName, DWORD dwStyle, DWORD dwExStyle, _U_MENUorID MenuOrID, LPVOID lpCreateParam
){
    ATOM atom = GetWndClassInfo().Register(&m_pfnSuperWindowProc);
    if (!atom)
        return NULL;

    // Allocate the thunk structure here, where we can fail gracefully.
    BOOL result = m_thunk.Init(NULL, NULL);
    if (result == FALSE)
    {
        SetLastError(ERROR_OUTOFMEMORY);
        return NULL;
    }

    _AtlWinModule.AddCreateWndData(&m_thunk.cd, this);

    dwStyle = GetWndStyle(dwStyle);
    dwExStyle = GetWndExStyle(dwExStyle);

    // Set Caption
    if (szWindowName == NULL)
    {
        szWindowName = GetWndCaption();
    }

    // Create window
    return CWindow::Create((LPCTSTR)atom, hWndParent, rect, szWindowName, dwStyle, dwExStyle, MenuOrID, lpCreateParam);
}

STDMETHODIMP CHTMLBrowserHost::CreateControlEx(
    LPCOLESTR lpszTricsData, HWND hWnd, IStream* pStream, IUnknown** ppUnk, REFIID iidAdvise, IUnknown* punkSink
){
    HRESULT hr = CreateControlLicEx(lpszTricsData, hWnd, pStream, ppUnk, iidAdvise, punkSink, NULL);

    if (SUCCEEDED(hr))
    {
        CComPtr<IWebBrowser2> spWebBrowser;
        CComPtr<IUnknown> pBrowser;
        hr = QueryControl(__uuidof(IWebBrowser2), (void**)&pBrowser);
        if (SUCCEEDED(hr) && pBrowser != NULL)
        {
            CComVariant v;
            CComVariant url("about:blank");

            spWebBrowser = pBrowser;
            spWebBrowser->Navigate2(&url, &v, &v, &v, &v);

            // Register to receive the developer console message receiver events
            // javascript: 'window.console'
            //
            CComPtr<IUnknown> pConsole;
            hr = QueryInterface(__uuidof(IDeveloperConsoleMessageReceiver), (void**)&pConsole);
            if (SUCCEEDED(hr))
            {
                CComPtr<IDispatch> spDocDisp;
                CComVariant varListener(pConsole);
                spWebBrowser = pBrowser;
                hr = spWebBrowser->get_Document(&spDocDisp);
                if (SUCCEEDED(hr))
                {
                    CComQIPtr<IOleCommandTarget> spOleCommandTarget(spDocDisp);
                    hr = spOleCommandTarget->Exec(
                        &CGID_MSHTML,
                        IDM_ADDCONSOLEMESSAGERECEIVER,
                        OLECMDEXECOPT_DODEFAULT,
                        &varListener,
                        &varConsoleCookie
                    );
                }
            }
        }
    }

	return hr;
}


LPCWSTR MapMessageLevel(DEV_CONSOLE_MESSAGE_LEVEL level)
{
    switch(level)
    {
    case DCML_WARNING: return L"WARNING";
    case DCML_ERROR: return L"ERROR";
    case DCML_INFORMATIONAL:
    default: return L"INFO";
    }
}

STDMETHODIMP CHTMLBrowserHost::Write(
    LPCWSTR source, DEV_CONSOLE_MESSAGE_LEVEL level, int messageId, LPCWSTR messageText
){
    return WriteWithUrl(source, level, messageId, messageText, L"");
}

STDMETHODIMP CHTMLBrowserHost::WriteWithUrl(
    LPCWSTR source, DEV_CONSOLE_MESSAGE_LEVEL level, int messageId, LPCWSTR messageText, LPCWSTR fileUrl
){
    return WriteWithUrlAndLine(source, level, messageId, messageText, fileUrl, 0);
}

STDMETHODIMP CHTMLBrowserHost::WriteWithUrlAndLine(
    LPCWSTR source, DEV_CONSOLE_MESSAGE_LEVEL level, int messageId, LPCWSTR messageText, LPCWSTR fileUrl, ULONG line
){
    return WriteWithUrlLineAndColumn(source, level, messageId, messageText, fileUrl, line, 0);
}

STDMETHODIMP CHTMLBrowserHost::WriteWithUrlLineAndColumn(
    LPCWSTR source, DEV_CONSOLE_MESSAGE_LEVEL level, int messageId, LPCWSTR messageText, LPCWSTR fileUrl, ULONG line, ULONG column
){
    if ((CComBSTR("DOM") == CComBSTR(source)) && (7011 == messageId))
    {
        // We do not need to worry about forward/backward caching being disabled
    }
    else if ((CComBSTR("HTML") == CComBSTR(source)) && (CComBSTR("about:blank") == CComBSTR(fileUrl)))
    {
        // We do not need to worry about warnings and errors from a blank page
    }
    else
    {
        if (wcslen(fileUrl))
        {
            browserlog_msg(
                "Console: (%S) (%S%d) %S\n"
                "    File: %S, Line: %d, Column: %d",
                MapMessageLevel(level), source, messageId, messageText, fileUrl, line, column
            );
        }
        else
        {
            browserlog_msg(
                "Console: (%S) (%S%d) %S",
                MapMessageLevel(level), source, messageId, messageText
            );
        }
    }
    return S_OK;
}

STDMETHODIMP CHTMLBrowserHost::ShowMessage(
    HWND hwnd, LPOLESTR lpstrText, LPOLESTR lpstrCaption, DWORD dwType, LPOLESTR lpstrHelpFile, DWORD dwHelpContext, LRESULT *plResult
){
    browserlog_msg(
        "Show Message:\n"
        "    Caption: %S\n"
        "    Text: %S\n",
        lpstrCaption,
        lpstrText
    );
    return S_OK;
}

STDMETHODIMP CHTMLBrowserHost::ShowHelp(
    HWND hwnd, LPOLESTR pszHelpFile, UINT uCommand, DWORD dwData, POINT ptMouse, IDispatch *pDispatchObjectHit
){
    return S_OK;
};

STDMETHODIMP CHTMLBrowserHost::QueryStatus(
    const GUID *pguidCmdGroup, ULONG cCmds, OLECMD prgCmds[], OLECMDTEXT *pCmdText
){
    return E_NOTIMPL;
}

STDMETHODIMP CHTMLBrowserHost::Exec(
    const GUID* pguidCmdGroup, DWORD nCmdID, DWORD nCmdexecopt, VARIANTARG* pvaIn, VARIANTARG* pvaOut
){
    HRESULT hr = S_OK;
    if (pguidCmdGroup && IsEqualGUID(*pguidCmdGroup, CGID_DocHostCommandHandler))
    {
        switch (nCmdID) 
        {
            case OLECMDID_SHOWSCRIPTERROR:
            {
                // Stop running scripts on the page.
                (*pvaOut).vt = VT_BOOL;
                (*pvaOut).boolVal = VARIANT_FALSE;			
                break;
            }
            default:
                hr = OLECMDERR_E_NOTSUPPORTED;
                break;
        }
    }
    else
    {
        hr = OLECMDERR_E_UNKNOWNGROUP;
    }
    return hr;
}
