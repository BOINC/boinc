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
#include <mshtml.h>
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


HRESULT CHTMLBrowserHostUI::FinalConstruct()
{
    m_bScreensaver = false;
    m_lApplicationVersion = 0;
    m_dUserCreditTotal = 0.0;
    m_dUserCreditAverage = 0.0;
    m_dHostCreditTotal = 0.0;
    m_dHostCreditAverage = 0.0;
	return S_OK;
}

void CHTMLBrowserHostUI::FinalRelease()
{
}

STDMETHODIMP CHTMLBrowserHostUI::get_IsScreensaver(BOOL* pVal)
{
    *pVal = m_bScreensaver;
    return S_OK;
}

STDMETHODIMP CHTMLBrowserHostUI::put_IsScreensaver(BOOL newVal)
{
    m_bScreensaver = newVal;
    return S_OK;
}

STDMETHODIMP CHTMLBrowserHostUI::get_ApplicationName(BSTR* pVal)
{
    *pVal = m_strApplicationName;
    return S_OK;
}

STDMETHODIMP CHTMLBrowserHostUI::put_ApplicationName(BSTR newVal)
{
    m_strApplicationName = newVal;
    return S_OK;
}

STDMETHODIMP CHTMLBrowserHostUI::get_ApplicationVersion(LONG* pVal)
{
    *pVal = m_lApplicationVersion;
    return S_OK;
}

STDMETHODIMP CHTMLBrowserHostUI::put_ApplicationVersion(LONG newVal)
{
    m_lApplicationVersion = newVal;
    return S_OK;
}

STDMETHODIMP CHTMLBrowserHostUI::get_WorkunitName(BSTR* pVal)
{
    *pVal = m_strWorkunitName;
    return S_OK;
}

STDMETHODIMP CHTMLBrowserHostUI::put_WorkunitName(BSTR newVal)
{
    m_strWorkunitName = newVal;
    return S_OK;
}

STDMETHODIMP CHTMLBrowserHostUI::get_ResultName(BSTR* pVal)
{
    *pVal = m_strResultName;
    return S_OK;
}

STDMETHODIMP CHTMLBrowserHostUI::put_ResultName(BSTR newVal)
{
    m_strResultName = newVal;
    return S_OK;
}

STDMETHODIMP CHTMLBrowserHostUI::get_TeamName(BSTR* pVal)
{
    *pVal = m_strTeamName;
    return S_OK;
}

STDMETHODIMP CHTMLBrowserHostUI::put_TeamName(BSTR newVal)
{
    m_strTeamName = newVal;
    return S_OK;
}

STDMETHODIMP CHTMLBrowserHostUI::get_UserName(BSTR* pVal)
{
    *pVal = m_strUserName;
    return S_OK;
}

STDMETHODIMP CHTMLBrowserHostUI::put_UserName(BSTR newVal)
{
    m_strUserName = newVal;
    return S_OK;
}

STDMETHODIMP CHTMLBrowserHostUI::get_UserCreditTotal(DOUBLE* pVal)
{
    *pVal = m_dUserCreditTotal;
    return S_OK;
}

STDMETHODIMP CHTMLBrowserHostUI::put_UserCreditTotal(DOUBLE newVal)
{
    m_dUserCreditTotal = newVal;
    return S_OK;
}

STDMETHODIMP CHTMLBrowserHostUI::get_UserCreditAverage(DOUBLE* pVal)
{
    *pVal = m_dUserCreditAverage;
    return S_OK;
}

STDMETHODIMP CHTMLBrowserHostUI::put_UserCreditAverage(DOUBLE newVal)
{
    m_dUserCreditAverage = newVal;
    return S_OK;
}

STDMETHODIMP CHTMLBrowserHostUI::get_HostCreditTotal(DOUBLE* pVal)
{
    *pVal = m_dHostCreditTotal;
    return S_OK;
}

STDMETHODIMP CHTMLBrowserHostUI::put_HostCreditTotal(DOUBLE newVal)
{
    m_dHostCreditTotal = newVal;
    return S_OK;
}

STDMETHODIMP CHTMLBrowserHostUI::get_HostCreditAverage(DOUBLE* pVal)
{
    *pVal = m_dHostCreditAverage;
    return S_OK;
}

STDMETHODIMP CHTMLBrowserHostUI::put_HostCreditAverage(DOUBLE newVal)
{
    m_dHostCreditAverage = newVal;
    return S_OK;
}
