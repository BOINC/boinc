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


#ifdef __IDeveloperConsoleMessageReceiver_INTERNAL__

// New for IE10
#define IDM_ADDCONSOLEMESSAGERECEIVER     3800
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

    SetExternalDispatch(NULL);
	ReleaseAll();
}

HWND CHTMLBrowserHost::Create(
    HWND hWndParent, _U_RECT rect, LPCTSTR szWindowName, DWORD dwStyle, DWORD dwExStyle, _U_MENUorID MenuOrID, LPVOID lpCreateParam
){
    ATOM atom = GetWndClassInfo().Register(&m_pfnSuperWindowProc);
    if (!atom)
        return NULL;

    // Allocate the thunk structure here, where we can fail gracefully.
    BOOL result = m_thunk.Init(NULL,NULL);
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
    HRESULT hr2;

    if (SUCCEEDED(hr))
    {
        CComPtr<IWebBrowser2> spWebBrowser;
        CComPtr<IUnknown> pBrowser;
        hr = QueryControl(__uuidof(IWebBrowser2), (void**)&pBrowser);
        if (SUCCEEDED(hr) && pBrowser != NULL)
        {
            CComVariant v;
            CComVariant url("http://www.romwnet.org/");

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

        // Register a custom interface to extend the HTML Document Object Model
        // javascript: 'window.external'
        //
        CComObject<CHTMLBrowserHostUI> *pObject = NULL;
        hr = CComObject<CHTMLBrowserHostUI>::CreateInstance(&pObject);
        if (SUCCEEDED(hr) && pObject != NULL)
        {
    	    SetExternalDispatch((IHTMLBrowserHostUI*)pObject);
        }
    }

	return hr;
}


STDMETHODIMP CHTMLBrowserHost::Write(
    LPCWSTR source, DEV_CONSOLE_MESSAGE_LEVEL level, int messageId, LPCWSTR messageText
){
    return E_NOTIMPL;
}

STDMETHODIMP CHTMLBrowserHost::WriteWithUrl(
    LPCWSTR source, DEV_CONSOLE_MESSAGE_LEVEL level, int messageId, LPCWSTR messageText, LPCWSTR fileUrl
){
    return E_NOTIMPL;
}

STDMETHODIMP CHTMLBrowserHost::WriteWithUrlAndLine(
    LPCWSTR source, DEV_CONSOLE_MESSAGE_LEVEL level, int messageId, LPCWSTR messageText, LPCWSTR fileUrl, ULONG line
){
    return E_NOTIMPL;
}

STDMETHODIMP CHTMLBrowserHost::WriteWithUrlLineAndColumn(
    LPCWSTR source, DEV_CONSOLE_MESSAGE_LEVEL level, int messageId, LPCWSTR messageText, LPCWSTR fileUrl, ULONG line, ULONG column
){
    return E_NOTIMPL;
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
                CComPtr<IHTMLDocument2>     pDoc;
                CComPtr<IHTMLWindow2>       pWindow;
                CComPtr<IHTMLEventObj>      pEventObj;
                CComBSTR                    rgwszNames[5] = 
                                            { L"errorLine", L"errorCharacter", L"errorCode", L"errorMessage", L"errorUrl" };
                CComVariant                 rgvaEventInfo[5];
                DISPID                      rgDispIDs[5];
                DISPPARAMS                  params;

                params.cArgs = 0;
                params.cNamedArgs = 0;

                // Get the document that is currently being viewed.
                hr = pvaIn->punkVal->QueryInterface(IID_IHTMLDocument2, (void **) &pDoc);				
                // Get document.parentWindow.
                hr = pDoc->get_parentWindow(&pWindow);
                // Get the window.event object.
                hr = pWindow->get_event(&pEventObj);
                // Get the error info from the window.event object.
                for (int i = 0; i < 5; i++) 
                {  
                    // Get the property's dispID.
                    hr = pEventObj->GetIDsOfNames(IID_NULL, &rgwszNames[i], 1, LOCALE_SYSTEM_DEFAULT, &rgDispIDs[i]);
                    // Get the value of the property.
                    hr = pEventObj->Invoke(rgDispIDs[i], IID_NULL, LOCALE_SYSTEM_DEFAULT, DISPATCH_PROPERTYGET, &params, &rgvaEventInfo[i], NULL, NULL);
                    SysFreeString(rgwszNames[i]);
                }

                browserlog_msg(
                    "Show Script Error:\n"
                    "    URL: %S\n"
                    "    Message: %S\n"
                    "    Error Code: %d\n"
                    "    Error Line: %d\n"
                    "    Error Position: %d",
                    rgvaEventInfo[4].bstrVal,
                    rgvaEventInfo[3].bstrVal,
                    rgvaEventInfo[2].uiVal,
                    rgvaEventInfo[0].uiVal,
                    rgvaEventInfo[1].uiVal
                );

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
