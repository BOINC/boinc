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

    STDMETHOD(IsAppInitDataUpdated)(BOOL* pVal);
    STDMETHOD(SetAppInitDataUpdate)(BOOL newVal);
    STDMETHOD(IsScreensaver)(BOOL* pVal);
    STDMETHOD(SetScreensaver)(BOOL newVal);
    STDMETHOD(IsSuspended)(BOOL* pVal);
    STDMETHOD(SetSuspended)(BOOL newVal);
    STDMETHOD(IsNetworkSuspended)(BOOL* pVal);
    STDMETHOD(SetNetworkSuspended)(BOOL newVal);
    STDMETHOD(IsExiting)(BOOL* pVal);
    STDMETHOD(SetExiting)(BOOL newVal);
    STDMETHOD(IsVboxwrapperJob)(BOOL* pVal);
    STDMETHOD(SetVboxwrapperJob)(BOOL newVal);
    STDMETHOD(get_ApplicationName)(BSTR* pVal);
    STDMETHOD(put_ApplicationName)(BSTR newVal);
    STDMETHOD(get_ApplicationVersion)(LONG* pVal);
    STDMETHOD(put_ApplicationVersion)(LONG newVal);
    STDMETHOD(get_WorkunitName)(BSTR* pVal);
    STDMETHOD(put_WorkunitName)(BSTR newVal);
    STDMETHOD(get_ResultName)(BSTR* pVal);
    STDMETHOD(put_ResultName)(BSTR newVal);
    STDMETHOD(get_TeamName)(BSTR* pVal);
    STDMETHOD(put_TeamName)(BSTR newVal);
    STDMETHOD(get_UserName)(BSTR* pVal);
    STDMETHOD(put_UserName)(BSTR newVal);
    STDMETHOD(get_UserCreditTotal)(DOUBLE* pVal);
    STDMETHOD(put_UserCreditTotal)(DOUBLE newVal);
    STDMETHOD(get_UserCreditAverage)(DOUBLE* pVal);
    STDMETHOD(put_UserCreditAverage)(DOUBLE newVal);
    STDMETHOD(get_HostCreditTotal)(DOUBLE* pVal);
    STDMETHOD(put_HostCreditTotal)(DOUBLE newVal);
    STDMETHOD(get_HostCreditAverage)(DOUBLE* pVal);
    STDMETHOD(put_HostCreditAverage)(DOUBLE newVal);
    STDMETHOD(get_ExitCountdown)(DOUBLE* pVal);
    STDMETHOD(put_ExitCountdown)(DOUBLE newVal);
    STDMETHOD(get_CPUTime)(DOUBLE* pVal);
    STDMETHOD(put_CPUTime)(DOUBLE newVal);
    STDMETHOD(get_ElapsedTime)(DOUBLE* pVal);
    STDMETHOD(put_ElapsedTime)(DOUBLE newVal);
    STDMETHOD(get_FractionDone)(DOUBLE* pVal);
    STDMETHOD(put_FractionDone)(DOUBLE newVal);
    STDMETHOD(get_RemoteDesktopPort)(LONG* pVal);
    STDMETHOD(put_RemoteDesktopPort)(LONG newVal);
    STDMETHOD(get_WebAPIPort)(LONG* pVal);
    STDMETHOD(put_WebAPIPort)(LONG newVal);


    BOOL m_bAppInitDataUpdated;
    BOOL m_bScreensaver;
    BOOL m_bSuspended;
    BOOL m_bNetworkSuspended;
    BOOL m_bExiting;
    BOOL m_bVboxwrapperJob;
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
    double m_dExitCountdown;
    double m_dCPUTime;
    double m_dElapsedTime;
    double m_dFractionDone;
    LONG m_lRemoteDesktopPort;
    LONG m_lWebAPIPort;
};

#endif