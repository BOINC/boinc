// The contents of this file are subject to the BOINC Public License
// Version 1.0 (the "License"); you may not use this file except in
// compliance with the License. You may obtain a copy of the License at
// http://boinc.berkeley.edu/license_1.0.txt
// 
// Software distributed under the License is distributed on an "AS IS"
// basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the
// License for the specific language governing rights and limitations
// under the License. 
// 
// The Original Code is the Berkeley Open Infrastructure for Network Computing. 
// 
// The Initial Developer of the Original Code is the SETI@home project.
// Portions created by the SETI@home project are Copyright (C) 2002
// University of California at Berkeley. All Rights Reserved. 
// 
// Contributor(s):
//

#ifndef __WIN_PROXYDLG_H_
#define __WIN_PROXYDLG_H_
#include "afxwin.h"


// CProxyDlg dialog

class CProxyServerDlg : public CDialog
{
	DECLARE_DYNAMIC(CProxyServerDlg)

public:
	CProxyServerDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CProxyServerDlg();

// Dialog Data
	enum { IDD = IDD_PROXY };

protected:
	virtual void    DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	void		    LoadLanguage();

	DECLARE_MESSAGE_MAP()
private:
    CButton m_UseHTTPProxyServerCtrl;
    CButton m_UseSOCKSProxyServerCtrl;
	CButton m_UseHTTPProxyAuthenticationCtrl;
    CEdit   m_HTTPProxyServerAddressCtrl;
    CEdit   m_HTTPProxyServerPortCtrl;
	CEdit   m_HTTPProxyServerUsernameCtrl;
	CEdit   m_HTTPProxyServerPasswordCtrl;
    CEdit   m_SOCKSProxyServerAddressCtrl;
    CEdit   m_SOCKSProxyServerPortCtrl;
    CEdit   m_SOCKSProxyServerUsernameCtrl;
    CEdit   m_SOCKSProxyServerPasswordCtrl;

public:
    virtual BOOL OnInitDialog();
    afx_msg void OnBnClickedUseHTTPProxyServer();
    afx_msg void OnBnClickedUseSOCKSProxyServer();
	afx_msg void OnBnClickedUseHttpAuth();

protected:
    virtual void OnOK();

private:
    CString m_strHTTPProxyServerAddress;
    UINT    m_uiHTTPProxyServerPort;
	CString m_strHTTPProxyServerUsername;
	CString m_strHTTPProxyServerPassword;
    CString m_strSOCKSProxyServerAddress;
    UINT    m_uiSOCKSProxyServerPort;
    CString m_strSOCKSProxyServerUsername;
    CString m_strSOCKSProxyServerPassword;
};

#endif