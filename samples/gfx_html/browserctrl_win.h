// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2010-2015 University of California
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

/////////////////////////////////////////////////////////////////////////////
// IBrowserHostUI interface

MIDL_INTERFACE("A7275E6E-DE3D-4107-B34F-C4C28411A6F0")
IHTMLBrowserHostUI : public IDispatch
{
public:
    virtual HRESULT STDMETHODCALLTYPE Log(VARIANT* pvaLog) = 0;
};

/////////////////////////////////////////////////////////////////////////////
// CHTMLBrowserHost class

class ATL_NO_VTABLE CHTMLBrowserHost :
    public CAxHostWindow,
    public IDocHostShowUI,
    public IOleCommandTarget,
    public IDispatchImpl<IHTMLBrowserHostUI>
{
public:
    DECLARE_NO_REGISTRY()
    DECLARE_PROTECT_FINAL_CONSTRUCT()
    DECLARE_POLY_AGGREGATABLE(CHTMLBrowserHost)
    DECLARE_GET_CONTROLLING_UNKNOWN()

    BEGIN_COM_MAP(CHTMLBrowserHost)
        COM_INTERFACE_ENTRY(IHTMLBrowserHostUI)
        COM_INTERFACE_ENTRY(IDocHostShowUI)
        COM_INTERFACE_ENTRY(IOleCommandTarget)
        COM_INTERFACE_ENTRY_CHAIN(CAxHostWindow)
    END_COM_MAP()


    static CWndClassInfo& GetWndClassInfo();


    HWND Create(HWND hWndParent, _U_RECT rect = NULL, LPCTSTR szWindowName = NULL, DWORD dwStyle = 0, DWORD dwExStyle = 0, _U_MENUorID MenuOrID = 0U, LPVOID lpCreateParam = NULL);
	void FinalRelease();


    // COM Interface - IHTMLBrowserHostUI
    // Provide a basic set of services to HTML based applications
    //
    STDMETHOD(Log)(VARIANT* pvaLog);


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

};

#endif