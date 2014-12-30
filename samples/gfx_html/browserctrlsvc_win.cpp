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
#include "browserctrlsvc_win.h"


HRESULT CHTMLBrowserHostServices::FinalConstruct()
{
    m_pHostUI = NULL;
	return S_OK;
}

void CHTMLBrowserHostServices::FinalRelease()
{
}

STDMETHODIMP CHTMLBrowserHostServices::SetSecuritySite(IInternetSecurityMgrSite *pSite)
{
    return INET_E_DEFAULT_ACTION;
}

STDMETHODIMP CHTMLBrowserHostServices::GetSecuritySite(IInternetSecurityMgrSite **ppSite)
{
    return INET_E_DEFAULT_ACTION;
}

STDMETHODIMP CHTMLBrowserHostServices::MapUrlToZone(LPCWSTR pwszUrl, DWORD *pdwZone, DWORD dwFlags)
{
    return INET_E_DEFAULT_ACTION;
}

STDMETHODIMP CHTMLBrowserHostServices::GetSecurityId(LPCWSTR pwszUrl, BYTE *pbSecurityId, DWORD *pcbSecurityId, DWORD_PTR dwReserved)
{
    return INET_E_DEFAULT_ACTION;
}

STDMETHODIMP CHTMLBrowserHostServices::ProcessUrlAction(LPCWSTR pwszUrl, DWORD dwAction, BYTE *pPolicy, DWORD cbPolicy, BYTE *pContext, DWORD cbContext, DWORD dwFlags, DWORD dwReserved)
{
    return INET_E_DEFAULT_ACTION;
}

STDMETHODIMP CHTMLBrowserHostServices::QueryCustomPolicy(LPCWSTR pwszUrl, REFGUID guidKey, BYTE **ppPolicy, DWORD *pcbPolicy, BYTE *pContext, DWORD cbContext, DWORD dwReserved)
{
    return INET_E_DEFAULT_ACTION;
}

STDMETHODIMP CHTMLBrowserHostServices::SetZoneMapping(DWORD dwZone, LPCWSTR lpszPattern, DWORD dwFlags)
{
    return INET_E_DEFAULT_ACTION;
}

STDMETHODIMP CHTMLBrowserHostServices::GetZoneMappings(DWORD dwZone, IEnumString **ppenumString, DWORD dwFlags)
{
    return INET_E_DEFAULT_ACTION;
}
