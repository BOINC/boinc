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

#include "stdafx.h"

#include "wingui_dialog.h"

/////////////////////////////////////////////////////////////////////////
// CLoginDialog message map and member functions

BEGIN_MESSAGE_MAP(CLoginDialog, CDialog)
    ON_BN_CLICKED(IDOK, OnOK)
	ON_NOTIFY_EX(TTN_NEEDTEXT, 0, OnToolTipNotify)
END_MESSAGE_MAP()

//////////
// CLoginDialog::CLoginDialog
// arguments:	y: dialog box resource id
//				szUrl: the initial url
//				szAuth: the initial autorization
// returns:		void
// function:	calls parents contructor, sets member variables
CLoginDialog::CLoginDialog(UINT y, LPCTSTR szUrl, LPCTSTR szAuth) : CDialog(y)
{
	m_strUrl.Format("%s", szUrl);
	m_strAuth.Format("%s", szAuth);
}

//////////
// CLoginDialog::LoadLanguage
// arguments:	void
// returns:		void
// function:	loads new captions from language file
void CLoginDialog::LoadLanguage()
{
	int const static nIDs[] = { IDC_STATIC_URL, IDC_STATIC_AUTH, IDOK, IDCANCEL, 0 };
	CString * const pStrs[] = { &m_strUrlTT, &m_strAuthTT, 0 };
	UpdateLanguageStrings(this, "DIALOG-LOGIN", nIDs, pStrs);
}

//////////
// CLoginDialog::OnInitDialog
// arguments:	void
// returns:		true if windows needs to give dialog focus, false if dialog has taken focus
// function:	initializes and centers dialog box
BOOL CLoginDialog::OnInitDialog() 
{
    CDialog::OnInitDialog();
	m_strUrlTT.Format("The URL for the website of the project.");
	m_strAuthTT.Format("The authorization code received in your confirmation email.");
	LoadLanguage();
	CWnd* pWndUrl = GetDlgItem(IDC_LOGIN_URL);
	if(pWndUrl) {
		pWndUrl->SetWindowText(m_strUrl);
	}
	CWnd* pWndAuth = GetDlgItem(IDC_LOGIN_AUTH);
	if(pWndAuth) {
		pWndAuth->SetWindowText(m_strAuth);
	}
	CWnd* pWndFocus = GetDlgItem(IDC_LOGIN_URL);
	if(pWndFocus) pWndFocus->SetFocus();
    CenterWindow();
	EnableToolTips(TRUE);
    return FALSE;
}

//////////
// CLoginDialog::OnOK
// arguments:	void
// returns:		void
// function:	copies strings from edit controls to member variables.
void CLoginDialog::OnOK() 
{
    GetDlgItemText(IDC_LOGIN_URL, m_strUrl);
    GetDlgItemText(IDC_LOGIN_AUTH, m_strAuth);
    CDialog::OnOK();
}

//////////
// CLoginDialog::OnToolTipNotify
// arguments:	id: id of the window, actually its hwnd
//				pNMHDR: pointer to notification message header
//				pResult: pointer to result of notification
// returns:		true if the notification is processed, otherwise false
// function:	handles notifications of tool tips by filling in 
//				text for tool tips
BOOL CLoginDialog::OnToolTipNotify(UINT id, NMHDR *pNMHDR, LRESULT *pResult)
{
    TOOLTIPTEXT* pTTT = (TOOLTIPTEXT*)pNMHDR;

    if(pTTT->uFlags & TTF_IDISHWND) {

        // idFrom is actually the HWND of the tool
        UINT nID = ::GetDlgCtrlID((HWND)pNMHDR->idFrom);

        if(nID == IDC_LOGIN_URL) {
            lstrcpyn(pTTT->szText, m_strUrlTT, (sizeof(pTTT->szText)/sizeof(TCHAR)));
        }
        if(nID == IDC_LOGIN_AUTH) {
            lstrcpyn(pTTT->szText, m_strAuthTT, (sizeof(pTTT->szText)/sizeof(TCHAR)));
        }
    }

    *pResult = 0;

    // message was handled
    return TRUE;
}

/////////////////////////////////////////////////////////////////////////
// CProxyDialog message map and member functions

BEGIN_MESSAGE_MAP(CProxyDialog, CDialog)
    ON_BN_CLICKED(IDC_CHECK_HTTP, OnHttp)
    ON_BN_CLICKED(IDC_CHECK_SOCKS, OnSocks)
    ON_BN_CLICKED(IDOK, OnOK)
END_MESSAGE_MAP()

//////////
// CProxyDialog::CProxyDialog
// arguments:	y: dialog box resource id
// returns:		void
// function:	calls parents contructor.
CProxyDialog::CProxyDialog(UINT y) : CDialog(y)
{
}

//////////
// CProxyDialog::LoadLanguage
// arguments:	void
// returns:		void
// function:	loads new captions from language file
void CProxyDialog::LoadLanguage()
{
	int const static nIDs[] = { IDC_STATIC_PROXY, IDC_STATIC_HTTP, IDC_CHECK_HTTP,
		IDC_STATIC_HTTP_ADDR, IDC_STATIC_HTTP_PORT, IDC_STATIC_SOCKS,
		IDC_CHECK_SOCKS, IDC_STATIC_SOCKS_ADDR, IDC_STATIC_SOCKS_PORT,
		IDC_STATIC_SOCKS_BLANK, IDC_STATIC_SOCKS_NAME, IDC_STATIC_SOCKS_PASS,
		IDOK, IDCANCEL, 0 };

	UpdateLanguageStrings(this, "DIALOG-PROXY", nIDs);
}

//////////
// CProxyDialog::OnInitDialog
// arguments:	void
// returns:		true if windows needs to give dialog focus, false if dialog has taken focus
// function:	initializes and centers dialog box
BOOL CProxyDialog::OnInitDialog() 
{
    CDialog::OnInitDialog();
	LoadLanguage();
	CButton* pBtn;

	// fill in http
	pBtn = (CButton*)GetDlgItem(IDC_CHECK_HTTP);
	if(pBtn) pBtn->SetCheck(gstate.pi.use_http_proxy?BST_CHECKED:BST_UNCHECKED);
    SetDlgItemText(IDC_EDIT_HTTP_ADDR, gstate.pi.http_server_name);
	CString portBuf;
    if(gstate.pi.http_server_port > 0) portBuf.Format("%d", gstate.pi.http_server_port);
	else portBuf.Format("80");
	SetDlgItemText(IDC_EDIT_HTTP_PORT, portBuf.GetBuffer(0));
    EnableHttp(gstate.pi.use_http_proxy);
	
	// fill in socks
	pBtn = (CButton*)GetDlgItem(IDC_CHECK_SOCKS);
	if(pBtn) pBtn->EnableWindow(false);
	SetDlgItemText(IDC_EDIT_SOCKS_PORT, "1080");
	EnableSocks(false);
    CenterWindow();
    return TRUE;
}

//////////
// CProxyDialog::EnableHttp
// arguments:	bEnable: true to enable, false to disable
// returns:		void
// function:	enables or disables the http section of the dialog
void CProxyDialog::EnableHttp(BOOL bEnable)
{
	CEdit* pEdit;
	pEdit = (CEdit*)GetDlgItem(IDC_EDIT_HTTP_ADDR);
	if(pEdit) pEdit->EnableWindow(bEnable);
	pEdit = (CEdit*)GetDlgItem(IDC_EDIT_HTTP_PORT);
	if(pEdit) pEdit->EnableWindow(bEnable);
}

//////////
// CProxyDialog::EnableSocks
// arguments:	bEnable: true to enable, false to disable
// returns:		void
// function:	enables or disables the socks section of the dialog
void CProxyDialog::EnableSocks(BOOL bEnable)
{
	CEdit* pEdit;
	pEdit = (CEdit*)GetDlgItem(IDC_EDIT_SOCKS_ADDR);
	if(pEdit) pEdit->EnableWindow(bEnable);
	pEdit = (CEdit*)GetDlgItem(IDC_EDIT_SOCKS_PORT);
	if(pEdit) pEdit->EnableWindow(bEnable);
	pEdit = (CEdit*)GetDlgItem(IDC_EDIT_SOCKS_NAME);
	if(pEdit) pEdit->EnableWindow(bEnable);
	pEdit = (CEdit*)GetDlgItem(IDC_EDIT_SOCKS_PASS);
	if(pEdit) pEdit->EnableWindow(bEnable);
}

//////////
// CProxyDialog::OnHttp
// arguments:	void
// returns:		void
// function:	handles http check box
void CProxyDialog::OnHttp() 
{
	CButton* pBtn;
	pBtn = (CButton*)GetDlgItem(IDC_CHECK_HTTP);
	if(pBtn) {
		if(pBtn->GetCheck() == BST_CHECKED) {
			EnableHttp(true);
		} else {
			EnableHttp(false);
		}
	}
}

//////////
// CProxyDialog::OnSocks
// arguments:	void
// returns:		void
// function:	handles socks check box
void CProxyDialog::OnSocks() 
{
}

//////////
// CProxyDialog::OnOK
// arguments:	void
// returns:		void
// function:	sets member variables
void CProxyDialog::OnOK() 
{
	CButton* pBtn;
	CString strbuf;

	// get http info
	pBtn = (CButton*)GetDlgItem(IDC_CHECK_HTTP);
	if(pBtn) {
		if(pBtn->GetCheck() == BST_CHECKED) {
			gstate.pi.use_http_proxy = true;
		} else {
			gstate.pi.use_http_proxy = false;
		}
	}
	GetDlgItemText(IDC_EDIT_HTTP_ADDR, strbuf);
    safe_strncpy(gstate.pi.http_server_name, strbuf.GetBuffer(0),
        sizeof(gstate.pi.http_server_name));
	GetDlgItemText(IDC_EDIT_HTTP_PORT, strbuf);
    gstate.pi.http_server_port = atoi(strbuf.GetBuffer(0));
	CDialog::OnOK();
}

/////////////////////////////////////////////////////////////////////////
// CConnectDialog message map and member functions

BEGIN_MESSAGE_MAP(CConnectDialog, CDialog)
    ON_BN_CLICKED(IDOK, OnOK)
END_MESSAGE_MAP()

//////////
// CConnectDialog::CConnectDialog
// arguments:	y: dialog box resource id
// returns:		void
// function:	calls parents contructor.
CConnectDialog::CConnectDialog(UINT y) : CDialog(y)
{
}

//////////
// CConnectDialog::LoadLanguage
// arguments:	void
// returns:		void
// function:	loads new captions from language file
void CConnectDialog::LoadLanguage()
{
	int const static nIDs[] = { IDC_STATIC_ASK, IDC_DONTASK, IDOK, IDCANCEL, 0 };
	UpdateLanguageStrings(this, "DIALOG-CONNECT", nIDs);
}

//////////
// CConnectDialog::OnInitDialog
// arguments:	void
// returns:		true if windows needs to give dialog focus, false if dialog has taken focus
// function:	initializes and centers dialog box
BOOL CConnectDialog::OnInitDialog() 
{
	CDialog::OnInitDialog();
	LoadLanguage();
	return FALSE;
}

//////////
// CConnectDialog::OnOK
// arguments:	void
// returns:		void
// function:	sets member variables
void CConnectDialog::OnOK() 
{
	CDialog::OnOK();
}

/////////////////////////////////////////////////////////////////////////
// CAboutDialog message map and member functions

BEGIN_MESSAGE_MAP(CAboutDialog, CDialog)
END_MESSAGE_MAP()

//////////
// CAboutDialog::CAboutDialog
// arguments:	y: dialog box resource id
// returns:		void
// function:	calls parents contructor.
CAboutDialog::CAboutDialog(UINT y) : CDialog(y)
{
}

//////////
// CAboutDialog::LoadLanguage
// arguments:	void
// returns:		void
// function:	loads new captions from language file
void CAboutDialog::LoadLanguage()
{
	const int nIDs[] = { IDC_STATIC_TOP, IDC_STATIC_BOTTOM, IDOK, 0 };
	UpdateLanguageStrings(this, "DIALOG-ABOUT", nIDs);
}

//////////
// CAboutDialog::OnInitDialog
// arguments:	void
// returns:		true if windows needs to give dialog focus, false if dialog has taken focus
// function:	initializes and centers dialog box
BOOL CAboutDialog::OnInitDialog() 
{
    CDialog::OnInitDialog();
	LoadLanguage();

	double xVersion = MAJOR_VERSION + MINOR_VERSION / 100.0;
	CString strOldTitle, strVersion, strTitle;
	GetWindowText(strOldTitle);
	strVersion.Format("%0.2f", xVersion);
	strTitle.Format("%s %s", strOldTitle, strVersion);
	SetWindowText(strTitle);
    CenterWindow();
    return TRUE;
}
