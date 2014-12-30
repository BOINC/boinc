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

#ifndef _BROWSERCTRLSERVICES_WIN_H_
#define _BROWSERCTRLSERVICES_WIN_H_

/////////////////////////////////////////////////////////////////////////////
// CHTMLBrowserHostServices class

class ATL_NO_VTABLE CHTMLBrowserHostServices :
	public CComObjectRoot,
	public IDispatchImpl<IHTMLBrowserHostSvc, &IID_IHTMLBrowserHostSvc, &LIBID_HTMLGfxLib, 0xFFFF, 0xFFFF>,
    public IServiceProviderImpl<CHTMLBrowserHostServices>,
    public IInternetSecurityManager
{
public:
    BEGIN_COM_MAP(CHTMLBrowserHostServices)
	    COM_INTERFACE_ENTRY(IHTMLBrowserHostSvc)
	    COM_INTERFACE_ENTRY(IDispatch)
	    COM_INTERFACE_ENTRY(IServiceProvider)
	    COM_INTERFACE_ENTRY(IInternetSecurityManager)
    END_COM_MAP()

    BEGIN_SERVICE_MAP(CHTMLBrowserHostServices)
        SERVICE_ENTRY(SID_SInternetSecurityManager)
    END_SERVICE_MAP()


    HRESULT FinalConstruct();
	void FinalRelease();


    // COM Interface - IInternetSecurityManager
    // http://msdn.microsoft.com/en-us/library/ie/ms537130(v=vs.85).aspx
    //
    STDMETHOD(SetSecuritySite)(IInternetSecurityMgrSite *pSite);
    STDMETHOD(GetSecuritySite)(IInternetSecurityMgrSite **ppSite);
    STDMETHOD(MapUrlToZone)(LPCWSTR pwszUrl, DWORD *pdwZone, DWORD dwFlags);
    STDMETHOD(GetSecurityId)(LPCWSTR pwszUrl, BYTE *pbSecurityId, DWORD *pcbSecurityId, DWORD_PTR dwReserved);
    STDMETHOD(ProcessUrlAction)(LPCWSTR pwszUrl, DWORD dwAction, BYTE *pPolicy, DWORD cbPolicy, BYTE *pContext, DWORD cbContext, DWORD dwFlags, DWORD dwReserved);
    STDMETHOD(QueryCustomPolicy)(LPCWSTR pwszUrl, REFGUID guidKey, BYTE **ppPolicy, DWORD *pcbPolicy, BYTE *pContext, DWORD cbContext, DWORD dwReserved);
    STDMETHOD(SetZoneMapping)(DWORD dwZone, LPCWSTR lpszPattern, DWORD dwFlags);
    STDMETHOD(GetZoneMappings)(DWORD dwZone, IEnumString **ppenumString, DWORD dwFlags);


    CComPtr<IHTMLBrowserHostUI> m_pHostUI;
};

#endif