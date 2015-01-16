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

#ifndef _BROWSERWND_WIN_H_
#define _BROWSERWND_WIN_H_

class CHTMLBrowserWnd : 
    public CWindowImpl<CHTMLBrowserWnd, CWindow, CNullTraits>,
    public IDispEventImpl<1, CHTMLBrowserWnd, &__uuidof(DWebBrowserEvents2), &LIBID_SHDocVw, 1, 1>
{
public:
	DECLARE_WND_CLASS_EX(_T("BOINC_app"), 0, 0);
    DECLARE_NO_REGISTRY();

    BEGIN_SINK_MAP(CHTMLBrowserWnd)
        SINK_ENTRY_EX(1, __uuidof(DWebBrowserEvents2), DISPID_NEWPROCESS, OnNewProcess)
        SINK_ENTRY_EX(1, __uuidof(DWebBrowserEvents2), DISPID_NEWWINDOW2, OnNewWindow2)
        SINK_ENTRY_EX(1, __uuidof(DWebBrowserEvents2), DISPID_NEWWINDOW3, OnNewWindow3)
    END_SINK_MAP()

    BEGIN_MSG_MAP(CHTMLBrowserWnd)
	    MESSAGE_HANDLER(WM_CREATE, OnCreate)
	    MESSAGE_HANDLER(WM_CLOSE, OnClose)
	    MESSAGE_HANDLER(WM_SIZE, OnSize)
	    MESSAGE_HANDLER(WM_KEYUP, OnInputActivity)
	    MESSAGE_HANDLER(WM_KEYDOWN, OnInputActivity)
	    MESSAGE_HANDLER(WM_MOUSEMOVE, OnInputActivity)
	    MESSAGE_HANDLER(WM_TIMER, OnTimer)
    END_MSG_MAP()

    CHTMLBrowserWnd();
    ~CHTMLBrowserWnd();

    // Generic Window Events
    LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnTimer(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnInputActivity(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

    // HTML Browser Events
    STDMETHOD_(void, OnNewProcess)(LONG lCauseFlag, IDispatch* pDisp, VARIANT_BOOL* pCancel);
    STDMETHOD_(void, OnNewWindow2)(IDispatch** ppDisp, VARIANT_BOOL* pCancel);
    STDMETHOD_(void, OnNewWindow3)(IDispatch** ppDisp, VARIANT_BOOL* pCancel, DWORD dwFlags, BSTR bstrUrlContext, BSTR bstrUrl);

    //
    void NavigateToStateURL(bool bForce);

    CComObject<CHTMLBrowserHost>* m_pBrowserHost;
    CComQIPtr<IWebBrowser2> m_pBrowserCtrl;

    HICON m_hIcon;
    HICON m_hIconSmall;

    bool m_bInitializing;
    bool m_bScreensaverMode;
    CComBSTR m_strCurrentURL;
    CComBSTR m_strEmbeddedURL;
};

#endif