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

#ifndef _BROWSERCTRLUI_WIN_H_
#define _BROWSERCTRLUI_WIN_H_

/////////////////////////////////////////////////////////////////////////////
// CHTMLBrowserHostUI class

class ATL_NO_VTABLE CHTMLBrowserHostUI :
	public CComObjectRootEx<CComSingleThreadModel>,
	public IDispatchImpl<IHTMLBrowserHostUI, &IID_IHTMLBrowserHostUI, &LIBID_HTMLGfxLib, 0xFFFF, 0xFFFF>
{
BEGIN_COM_MAP(CHTMLBrowserHostUI)
	COM_INTERFACE_ENTRY(IHTMLBrowserHostUI)
	COM_INTERFACE_ENTRY(IDispatch)
END_COM_MAP()
public:
	DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct();
	void FinalRelease();

    STDMETHOD(log)(BSTR message);
    STDMETHOD(debug)(BSTR message);
    STDMETHOD(info)(BSTR message);
    STDMETHOD(warn)(BSTR message);
    STDMETHOD(error)(BSTR message);
    STDMETHOD(isStateUpdated)(BOOL* pVal);
    STDMETHOD(resetStateUpdate)(BOOL newVal);
    STDMETHOD(get_appName)(BSTR* pVal);
    STDMETHOD(put_appName)(BSTR newVal);
    STDMETHOD(get_appVersion)(LONG* pVal);
    STDMETHOD(put_appVersion)(LONG newVal);
    STDMETHOD(get_wuName)(BSTR* pVal);
    STDMETHOD(put_wuName)(BSTR newVal);
    STDMETHOD(get_resName)(BSTR* pVal);
    STDMETHOD(put_resName)(BSTR newVal);
    STDMETHOD(get_teamName)(BSTR* pVal);
    STDMETHOD(put_teamName)(BSTR newVal);
    STDMETHOD(get_userName)(BSTR* pVal);
    STDMETHOD(put_userName)(BSTR newVal);
    STDMETHOD(get_userCreditTotal)(DOUBLE* pVal);
    STDMETHOD(put_userCreditTotal)(DOUBLE newVal);
    STDMETHOD(get_userCreditAverage)(DOUBLE* pVal);
    STDMETHOD(put_userCreditAverage)(DOUBLE newVal);
    STDMETHOD(get_hostCreditTotal)(DOUBLE* pVal);
    STDMETHOD(put_hostCreditTotal)(DOUBLE newVal);
    STDMETHOD(get_hostCreditAverage)(DOUBLE* pVal);
    STDMETHOD(put_hostCreditAverage)(DOUBLE newVal);
    STDMETHOD(get_scrsaveMode)(BOOL* pVal);
    STDMETHOD(put_scrsaveMode)(BOOL newVal);
    STDMETHOD(get_suspended)(BOOL* pVal);
    STDMETHOD(put_suspended)(BOOL newVal);
    STDMETHOD(get_networkSuspended)(BOOL* pVal);
    STDMETHOD(put_networkSuspended)(BOOL newVal);
    STDMETHOD(get_exiting)(BOOL* pVal);
    STDMETHOD(put_exiting)(BOOL newVal);
    STDMETHOD(get_exitTimeout)(DOUBLE* pVal);
    STDMETHOD(put_exitTimeout)(DOUBLE newVal);
    STDMETHOD(get_cpuTime)(DOUBLE* pVal);
    STDMETHOD(put_cpuTime)(DOUBLE newVal);
    STDMETHOD(get_elapsedTime)(DOUBLE* pVal);
    STDMETHOD(put_elapsedTime)(DOUBLE newVal);
    STDMETHOD(get_fractionDone)(DOUBLE* pVal);
    STDMETHOD(put_fractionDone)(DOUBLE newVal);
    STDMETHOD(get_vboxJob)(BOOL* pVal);
    STDMETHOD(put_vboxJob)(BOOL newVal);
    STDMETHOD(get_rdpPort)(LONG* pVal);
    STDMETHOD(put_rdpPort)(LONG newVal);
    STDMETHOD(get_apiPort)(LONG* pVal);
    STDMETHOD(put_apiPort)(LONG newVal);

    BOOL m_bStateUpdated;
    CComBSTR m_strApplicationName;
    LONG m_lApplicationVersion;
    CComBSTR m_strWorkunitName;
    CComBSTR m_strResultName;
    CComBSTR m_strTeamName;
    CComBSTR m_strUserName;
    double m_dUserCreditTotal;
    double m_dUserCreditAverage;
    double m_dHostCreditTotal;
    double m_dHostCreditAverage;
    BOOL m_bScreensaver;
    BOOL m_bSuspended;
    BOOL m_bNetworkSuspended;
    BOOL m_bExiting;
    double m_dExitCountdown;
    double m_dCPUTime;
    double m_dElapsedTime;
    double m_dFractionDone;
    BOOL m_bVboxwrapperJob;
    LONG m_lRemoteDesktopPort;
    LONG m_lWebAPIPort;
};

#endif