// Berkeley Open Infrastructure for Network Computing
// http://boinc.berkeley.edu
// Copyright (C) 2005 University of California
//
// This is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation;
// either version 2.1 of the License, or (at your option) any later version.
//
// This software is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//
// To view the GNU Lesser General Public License visit
// http://www.gnu.org/copyleft/lesser.html
// or write to the Free Software Foundation, Inc.,
// 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

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
