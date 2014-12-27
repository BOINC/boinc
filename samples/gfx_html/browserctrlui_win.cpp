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
    m_bAppInitDataUpdated = false;
    m_bScreensaver = false;
    m_bSuspended = false;
    m_bNetworkSuspended = false;
    m_bExiting = false;
    m_lApplicationVersion = 0;
    m_dUserCreditTotal = 0.0;
    m_dUserCreditAverage = 0.0;
    m_dHostCreditTotal = 0.0;
    m_dHostCreditAverage = 0.0;
    m_dExitCountdown = 0.0;
    m_dCPUTime = 0.0;
    m_dElapsedTime = 0.0;
    m_dFractionDone = 0.0;
	return S_OK;
}

void CHTMLBrowserHostUI::FinalRelease()
{
}

STDMETHODIMP CHTMLBrowserHostUI::IsAppInitDataUpdated(BOOL *pVal)
{
    *pVal = m_bAppInitDataUpdated;
    return S_OK;
}

STDMETHODIMP CHTMLBrowserHostUI::SetAppInitDataUpdate(BOOL newVal)
{
    m_bAppInitDataUpdated = newVal;
    return S_OK;
}

STDMETHODIMP CHTMLBrowserHostUI::IsScreensaver(BOOL *pVal)
{
    *pVal = m_bScreensaver;
    return S_OK;
}

STDMETHODIMP CHTMLBrowserHostUI::SetScreensaver(BOOL newVal)
{
    m_bScreensaver = newVal;
    return S_OK;
}

STDMETHODIMP CHTMLBrowserHostUI::IsSuspended(BOOL *pVal)
{
    *pVal = m_bSuspended;
    return S_OK;
}

STDMETHODIMP CHTMLBrowserHostUI::SetSuspended(BOOL newVal)
{
    m_bSuspended = newVal;
    return S_OK;
}

STDMETHODIMP CHTMLBrowserHostUI::IsNetworkSuspended(BOOL *pVal)
{
    *pVal = m_bNetworkSuspended;
    return S_OK;
}

STDMETHODIMP CHTMLBrowserHostUI::SetNetworkSuspended(BOOL newVal)
{
    m_bNetworkSuspended = newVal;
    return S_OK;
}

STDMETHODIMP CHTMLBrowserHostUI::IsExiting(BOOL *pVal)
{
    *pVal = m_bExiting;
    return S_OK;
}

STDMETHODIMP CHTMLBrowserHostUI::SetExiting(BOOL newVal)
{
    m_bExiting = newVal;
    return S_OK;
}

STDMETHODIMP CHTMLBrowserHostUI::IsVboxwrapperJob(BOOL *pVal)
{
    *pVal = m_bVboxwrapperJob;
    return S_OK;
}

STDMETHODIMP CHTMLBrowserHostUI::SetVboxwrapperJob(BOOL newVal)
{
    m_bVboxwrapperJob = newVal;
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

STDMETHODIMP CHTMLBrowserHostUI::get_ExitCountdown(DOUBLE *pVal)
{
    *pVal = m_dExitCountdown;
    return S_OK;
}

STDMETHODIMP CHTMLBrowserHostUI::put_ExitCountdown(DOUBLE newVal)
{
    m_dExitCountdown = newVal;
    return S_OK;
}

STDMETHODIMP CHTMLBrowserHostUI::get_CPUTime(DOUBLE *pVal)
{
    *pVal = m_dCPUTime;
    return S_OK;
}

STDMETHODIMP CHTMLBrowserHostUI::put_CPUTime(DOUBLE newVal)
{
    m_dCPUTime = newVal;
    return S_OK;
}

STDMETHODIMP CHTMLBrowserHostUI::get_ElapsedTime(DOUBLE *pVal)
{
    *pVal = m_dElapsedTime;
    return S_OK;
}

STDMETHODIMP CHTMLBrowserHostUI::put_ElapsedTime(DOUBLE newVal)
{
    m_dElapsedTime = newVal;
    return S_OK;
}

STDMETHODIMP CHTMLBrowserHostUI::get_FractionDone(DOUBLE *pVal)
{
    *pVal = m_dFractionDone;
    return S_OK;
}

STDMETHODIMP CHTMLBrowserHostUI::put_FractionDone(DOUBLE newVal)
{
    m_dFractionDone = newVal;
    return S_OK;
}

STDMETHODIMP CHTMLBrowserHostUI::get_RemoteDesktopPort(LONG* pVal)
{
    *pVal = m_lRemoteDesktopPort;
    return S_OK;
}

STDMETHODIMP CHTMLBrowserHostUI::put_RemoteDesktopPort(LONG newVal)
{
    m_lRemoteDesktopPort = newVal;
    return S_OK;
}

STDMETHODIMP CHTMLBrowserHostUI::get_WebAPIPort(LONG* pVal)
{
    *pVal = m_lWebAPIPort;
    return S_OK;
}

STDMETHODIMP CHTMLBrowserHostUI::put_WebAPIPort(LONG newVal)
{
    m_lWebAPIPort = newVal;
    return S_OK;
}
