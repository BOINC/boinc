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

#ifndef _BROWSERCTRL_WIN_H_
#define _BROWSERCTRL_WIN_H_


#ifndef __IDeveloperConsoleMessageReceiver_FWD_DEFINED__

#define __IDeveloperConsoleMessageReceiver_FWD_DEFINED__
#define __IDeveloperConsoleMessageReceiver_INTERFACE_DEFINED__
#define __IDeveloperConsoleMessageReceiver_INTERNAL__

typedef interface IDeveloperConsoleMessageReceiver IDeveloperConsoleMessageReceiver;

typedef 
enum _DEV_CONSOLE_MESSAGE_LEVEL
    {
        DCML_INFORMATIONAL	= 0,
        DCML_WARNING	= 0x1,
        DCML_ERROR	= 0x2,
        DEV_CONSOLE_MESSAGE_LEVEL_Max	= 2147483647L
    } 	DEV_CONSOLE_MESSAGE_LEVEL;

/* interface IDeveloperConsoleMessageReceiver */
/* [local][unique][helpstring][uuid][object] */ 
EXTERN_C const IID IID_IDeveloperConsoleMessageReceiver;

MIDL_INTERFACE("30510808-98b5-11cf-bb82-00aa00bdce0b")
IDeveloperConsoleMessageReceiver : public IUnknown
{
public:
    virtual HRESULT STDMETHODCALLTYPE Write( 
        /* [annotation][in] */ 
        _In_  LPCWSTR source,
        /* [annotation][in] */ 
        _In_  DEV_CONSOLE_MESSAGE_LEVEL level,
        /* [annotation][in] */ 
        _In_  int messageId,
        /* [annotation][in] */ 
        _In_  LPCWSTR messageText) = 0;
        
    virtual HRESULT STDMETHODCALLTYPE WriteWithUrl( 
        /* [annotation][in] */ 
        _In_  LPCWSTR source,
        /* [annotation][in] */ 
        _In_  DEV_CONSOLE_MESSAGE_LEVEL level,
        /* [annotation][in] */ 
        _In_  int messageId,
        /* [annotation][in] */ 
        _In_  LPCWSTR messageText,
        /* [annotation][in] */ 
        _In_  LPCWSTR fileUrl) = 0;
        
    virtual HRESULT STDMETHODCALLTYPE WriteWithUrlAndLine( 
        /* [annotation][in] */ 
        _In_  LPCWSTR source,
        /* [annotation][in] */ 
        _In_  DEV_CONSOLE_MESSAGE_LEVEL level,
        /* [annotation][in] */ 
        _In_  int messageId,
        /* [annotation][in] */ 
        _In_  LPCWSTR messageText,
        /* [annotation][in] */ 
        _In_  LPCWSTR fileUrl,
        /* [annotation][in] */ 
        _In_  ULONG line) = 0;
        
    virtual HRESULT STDMETHODCALLTYPE WriteWithUrlLineAndColumn( 
        /* [annotation][in] */ 
        _In_  LPCWSTR source,
        /* [annotation][in] */ 
        _In_  DEV_CONSOLE_MESSAGE_LEVEL level,
        /* [annotation][in] */ 
        _In_  int messageId,
        /* [annotation][in] */ 
        _In_  LPCWSTR messageText,
        /* [annotation][in] */ 
        _In_  LPCWSTR fileUrl,
        /* [annotation][in] */ 
        _In_  ULONG line,
        /* [annotation][in] */ 
        _In_  ULONG column) = 0;
        
};

#endif 	/* __IDeveloperConsoleMessageReceiver_FWD_DEFINED__ */


/////////////////////////////////////////////////////////////////////////////
// CHTMLBrowserHost class

class ATL_NO_VTABLE CHTMLBrowserHost :
    public CAxHostWindow,
	public IDispatchImpl<IHTMLBrowserHost, &IID_IHTMLBrowserHost, &LIBID_HTMLGfxLib, 0xFFFF, 0xFFFF>,
    public IDocHostShowUI,
    public IDeveloperConsoleMessageReceiver,
    public IOleCommandTarget
{
public:
    DECLARE_NO_REGISTRY()
    DECLARE_POLY_AGGREGATABLE(CHTMLBrowserHost)

    BEGIN_COM_MAP(CHTMLBrowserHost)
	    COM_INTERFACE_ENTRY(IHTMLBrowserHost)
        COM_INTERFACE_ENTRY(IDocHostShowUI)
        COM_INTERFACE_ENTRY(IDeveloperConsoleMessageReceiver)
        COM_INTERFACE_ENTRY(IOleCommandTarget)
        COM_INTERFACE_ENTRY_CHAIN(CAxHostWindow)
    END_COM_MAP()

    static CWndClassInfo& GetWndClassInfo();


	HRESULT FinalConstruct();
	void FinalRelease();
	virtual void OnFinalMessage(HWND hWnd);

    HWND Create(HWND hWndParent, _U_RECT rect = NULL, LPCTSTR szWindowName = NULL, DWORD dwStyle = 0, DWORD dwExStyle = 0, _U_MENUorID MenuOrID = 0U, LPVOID lpCreateParam = NULL);

    STDMETHOD(CreateControlEx)(LPCOLESTR lpszTricsData, HWND hWnd, IStream* pStream, IUnknown** ppUnk, REFIID iidAdvise, IUnknown* punkSink);


    // COM Interface - IDeveloperConsoleMessageReceiver
    // http://msdn.microsoft.com/en-us/library/jj126732(v=vs.85).aspx
    //
    STDMETHOD(Write)(LPCWSTR source, DEV_CONSOLE_MESSAGE_LEVEL level, int messageId, LPCWSTR messageText);
    STDMETHOD(WriteWithUrl)(LPCWSTR source, DEV_CONSOLE_MESSAGE_LEVEL level, int messageId, LPCWSTR messageText, LPCWSTR fileUrl);
    STDMETHOD(WriteWithUrlAndLine)(LPCWSTR source, DEV_CONSOLE_MESSAGE_LEVEL level, int messageId, LPCWSTR messageText, LPCWSTR fileUrl, ULONG line);
    STDMETHOD(WriteWithUrlLineAndColumn)(LPCWSTR source, DEV_CONSOLE_MESSAGE_LEVEL level, int messageId, LPCWSTR messageText, LPCWSTR fileUrl, ULONG line, ULONG column);

    // COM Interface - IDocHostShowUI
    // http://msdn.microsoft.com/en-us/library/aa770041(v=vs.85).aspx
    //
    STDMETHOD(ShowMessage)(HWND hwnd, LPOLESTR lpstrText, LPOLESTR lpstrCaption, DWORD dwType, LPOLESTR lpstrHelpFile, DWORD dwHelpContext, LRESULT *plResult);
    STDMETHOD(ShowHelp)(HWND hwnd, LPOLESTR pszHelpFile, UINT uCommand, DWORD dwData, POINT ptMouse, IDispatch *pDispatchObjectHit);

    // COM Interface - IOleCommandTarget
    // http://support.microsoft.com/kb/261003
    //
    STDMETHOD(QueryStatus)(const GUID *pguidCmdGroup, ULONG cCmds, OLECMD prgCmds[], OLECMDTEXT *pCmdText);
    STDMETHOD(Exec)(const GUID* pguidCmdGroup, DWORD nCmdID, DWORD nCmdexecopt, VARIANTARG* pvaIn, VARIANTARG* pvaOut);

    CComVariant varConsoleCookie;
};

#endif