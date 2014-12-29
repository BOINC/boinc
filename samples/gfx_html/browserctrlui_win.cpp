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
    m_bStateUpdated = false;
    m_strApplicationName.Empty();
    m_lApplicationVersion = 0;
    m_strWorkunitName.Empty();
    m_strResultName.Empty();
    m_strTeamName.Empty();
    m_strUserName.Empty();
    m_dUserCreditTotal = 0.0;
    m_dUserCreditAverage = 0.0;
    m_dHostCreditTotal = 0.0;
    m_dHostCreditAverage = 0.0;
    m_bScreensaver = false;
    m_bSuspended = false;
    m_bNetworkSuspended = false;
    m_bExiting = false;
    m_dExitCountdown = 0.0;
    m_dCPUTime = 0.0;
    m_dElapsedTime = 0.0;
    m_dFractionDone = 0.0;
    m_bVboxwrapperJob = false;
    m_lRemoteDesktopPort = 0;
    m_lWebAPIPort = 0;
	return S_OK;
}

void CHTMLBrowserHostUI::FinalRelease()
{
    m_strApplicationName.Empty();
    m_strWorkunitName.Empty();
    m_strResultName.Empty();
    m_strTeamName.Empty();
    m_strUserName.Empty();
}

STDMETHODIMP CHTMLBrowserHostUI::log(BSTR message)
{
    browserlog_msg("Console: (%S%d) %S",
        L"SCRIPT", 0, message
    );
    return S_OK;
}

STDMETHODIMP CHTMLBrowserHostUI::debug(BSTR message)
{
    browserlog_msg("Console: (%S) (%S%d) %S",
        L"DEBUG", L"SCRIPT", 0, message
    );
    return S_OK;
}

STDMETHODIMP CHTMLBrowserHostUI::info(BSTR message)
{
    browserlog_msg("Console: (%S) (%S%d) %S",
        L"INFO", L"SCRIPT", 0, message
    );
    return S_OK;
}

STDMETHODIMP CHTMLBrowserHostUI::warn(BSTR message)
{
    browserlog_msg("Console: (%S) (%S%d) %S",
        L"WARNING", L"SCRIPT", 0, message
    );
    return S_OK;
}

STDMETHODIMP CHTMLBrowserHostUI::error(BSTR message)
{
    browserlog_msg("Console: (%S) (%S%d) %S",
        L"ERROR", L"SCRIPT", 0, message
    );
    return S_OK;
}

STDMETHODIMP CHTMLBrowserHostUI::isStateUpdated(BOOL *pVal)
{
    *pVal = m_bStateUpdated;
    return S_OK;
}

STDMETHODIMP CHTMLBrowserHostUI::resetStateUpdate(BOOL newVal)
{
    m_bStateUpdated = newVal;
    return S_OK;
}

STDMETHODIMP CHTMLBrowserHostUI::get_appName(BSTR* pVal)
{
    *pVal = m_strApplicationName;
    return S_OK;
}

STDMETHODIMP CHTMLBrowserHostUI::put_appName(BSTR newVal)
{
    m_strApplicationName = newVal;
    return S_OK;
}

STDMETHODIMP CHTMLBrowserHostUI::get_appVersion(LONG* pVal)
{
    *pVal = m_lApplicationVersion;
    return S_OK;
}

STDMETHODIMP CHTMLBrowserHostUI::put_appVersion(LONG newVal)
{
    m_lApplicationVersion = newVal;
    return S_OK;
}

STDMETHODIMP CHTMLBrowserHostUI::get_wuName(BSTR* pVal)
{
    *pVal = m_strWorkunitName;
    return S_OK;
}

STDMETHODIMP CHTMLBrowserHostUI::put_wuName(BSTR newVal)
{
    m_strWorkunitName = newVal;
    return S_OK;
}

STDMETHODIMP CHTMLBrowserHostUI::get_resName(BSTR* pVal)
{
    *pVal = m_strResultName;
    return S_OK;
}

STDMETHODIMP CHTMLBrowserHostUI::put_resName(BSTR newVal)
{
    m_strResultName = newVal;
    return S_OK;
}

STDMETHODIMP CHTMLBrowserHostUI::get_teamName(BSTR* pVal)
{
    *pVal = m_strTeamName;
    return S_OK;
}

STDMETHODIMP CHTMLBrowserHostUI::put_teamName(BSTR newVal)
{
    m_strTeamName = newVal;
    return S_OK;
}

STDMETHODIMP CHTMLBrowserHostUI::get_userName(BSTR* pVal)
{
    *pVal = m_strUserName;
    return S_OK;
}

STDMETHODIMP CHTMLBrowserHostUI::put_userName(BSTR newVal)
{
    m_strUserName = newVal;
    return S_OK;
}

STDMETHODIMP CHTMLBrowserHostUI::get_userCreditTotal(DOUBLE* pVal)
{
    *pVal = m_dUserCreditTotal;
    return S_OK;
}

STDMETHODIMP CHTMLBrowserHostUI::put_userCreditTotal(DOUBLE newVal)
{
    m_dUserCreditTotal = newVal;
    return S_OK;
}

STDMETHODIMP CHTMLBrowserHostUI::get_userCreditAverage(DOUBLE* pVal)
{
    *pVal = m_dUserCreditAverage;
    return S_OK;
}

STDMETHODIMP CHTMLBrowserHostUI::put_userCreditAverage(DOUBLE newVal)
{
    m_dUserCreditAverage = newVal;
    return S_OK;
}

STDMETHODIMP CHTMLBrowserHostUI::get_hostCreditTotal(DOUBLE* pVal)
{
    *pVal = m_dHostCreditTotal;
    return S_OK;
}

STDMETHODIMP CHTMLBrowserHostUI::put_hostCreditTotal(DOUBLE newVal)
{
    m_dHostCreditTotal = newVal;
    return S_OK;
}

STDMETHODIMP CHTMLBrowserHostUI::get_hostCreditAverage(DOUBLE* pVal)
{
    *pVal = m_dHostCreditAverage;
    return S_OK;
}

STDMETHODIMP CHTMLBrowserHostUI::put_hostCreditAverage(DOUBLE newVal)
{
    m_dHostCreditAverage = newVal;
    return S_OK;
}

STDMETHODIMP CHTMLBrowserHostUI::get_scrsaveMode(BOOL *pVal)
{
    *pVal = m_bScreensaver;
    return S_OK;
}

STDMETHODIMP CHTMLBrowserHostUI::put_scrsaveMode(BOOL newVal)
{
    m_bScreensaver = newVal;
    return S_OK;
}

STDMETHODIMP CHTMLBrowserHostUI::get_suspended(BOOL *pVal)
{
    *pVal = m_bSuspended;
    return S_OK;
}

STDMETHODIMP CHTMLBrowserHostUI::put_suspended(BOOL newVal)
{
    m_bSuspended = newVal;
    return S_OK;
}

STDMETHODIMP CHTMLBrowserHostUI::get_networkSuspended(BOOL *pVal)
{
    *pVal = m_bNetworkSuspended;
    return S_OK;
}

STDMETHODIMP CHTMLBrowserHostUI::put_networkSuspended(BOOL newVal)
{
    m_bNetworkSuspended = newVal;
    return S_OK;
}

STDMETHODIMP CHTMLBrowserHostUI::get_exiting(BOOL *pVal)
{
    *pVal = m_bExiting;
    return S_OK;
}

STDMETHODIMP CHTMLBrowserHostUI::put_exiting(BOOL newVal)
{
    m_bExiting = newVal;
    return S_OK;
}

STDMETHODIMP CHTMLBrowserHostUI::get_exitTimeout(DOUBLE *pVal)
{
    *pVal = m_dExitCountdown;
    return S_OK;
}

STDMETHODIMP CHTMLBrowserHostUI::put_exitTimeout(DOUBLE newVal)
{
    m_dExitCountdown = newVal;
    return S_OK;
}

STDMETHODIMP CHTMLBrowserHostUI::get_cpuTime(DOUBLE *pVal)
{
    *pVal = m_dCPUTime;
    return S_OK;
}

STDMETHODIMP CHTMLBrowserHostUI::put_cpuTime(DOUBLE newVal)
{
    m_dCPUTime = newVal;
    return S_OK;
}

STDMETHODIMP CHTMLBrowserHostUI::get_elapsedTime(DOUBLE *pVal)
{
    *pVal = m_dElapsedTime;
    return S_OK;
}

STDMETHODIMP CHTMLBrowserHostUI::put_elapsedTime(DOUBLE newVal)
{
    m_dElapsedTime = newVal;
    return S_OK;
}

STDMETHODIMP CHTMLBrowserHostUI::get_fractionDone(DOUBLE *pVal)
{
    *pVal = m_dFractionDone;
    return S_OK;
}

STDMETHODIMP CHTMLBrowserHostUI::put_fractionDone(DOUBLE newVal)
{
    m_dFractionDone = newVal;
    return S_OK;
}

STDMETHODIMP CHTMLBrowserHostUI::get_vboxJob(BOOL *pVal)
{
    *pVal = m_bVboxwrapperJob;
    return S_OK;
}

STDMETHODIMP CHTMLBrowserHostUI::put_vboxJob(BOOL newVal)
{
    m_bVboxwrapperJob = newVal;
    return S_OK;
}

STDMETHODIMP CHTMLBrowserHostUI::get_rdpPort(LONG* pVal)
{
    *pVal = m_lRemoteDesktopPort;
    return S_OK;
}

STDMETHODIMP CHTMLBrowserHostUI::put_rdpPort(LONG newVal)
{
    m_lRemoteDesktopPort = newVal;
    return S_OK;
}

STDMETHODIMP CHTMLBrowserHostUI::get_apiPort(LONG* pVal)
{
    *pVal = m_lWebAPIPort;
    return S_OK;
}

STDMETHODIMP CHTMLBrowserHostUI::put_apiPort(LONG newVal)
{
    m_lWebAPIPort = newVal;
    return S_OK;
}

