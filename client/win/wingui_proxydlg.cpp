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

#include "boinc_win.h"
#include "boinc_gui.h"
#include "wingui_proxydlg.h"
#include "wingui.h"
#include "client_state.h"
#include "proxy.h"
#include "wingui_proxydlg.h"


// CProxyServerDlg dialog

IMPLEMENT_DYNAMIC(CProxyServerDlg, CDialog)
CProxyServerDlg::CProxyServerDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CProxyServerDlg::IDD, pParent)
    , m_strHTTPProxyServerAddress(_T(""))
    , m_uiHTTPProxyServerPort(80)
	, m_strHTTPProxyServerUsername(_T(""))
	, m_strHTTPProxyServerPassword(_T(""))
    , m_strSOCKSProxyServerAddress(_T(""))
    , m_uiSOCKSProxyServerPort(1080)
    , m_strSOCKSProxyServerUsername(_T(""))
    , m_strSOCKSProxyServerPassword(_T(""))
{
}

CProxyServerDlg::~CProxyServerDlg()
{
}

void CProxyServerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_CHECK_HTTP, m_UseHTTPProxyServerCtrl);
	DDX_Control(pDX, IDC_CHECK_SOCKS, m_UseSOCKSProxyServerCtrl);
	DDX_Control(pDX, IDC_CHECK_HTTP_AUTH, m_UseHTTPProxyAuthenticationCtrl);
	DDX_Control(pDX, IDC_EDIT_HTTP_ADDR, m_HTTPProxyServerAddressCtrl);
	DDX_Control(pDX, IDC_EDIT_HTTP_PORT, m_HTTPProxyServerPortCtrl);
	DDX_Control(pDX, IDC_EDIT_HTTP_NAME, m_HTTPProxyServerUsernameCtrl);
	DDX_Control(pDX, IDC_EDIT_HTTP_PASS, m_HTTPProxyServerPasswordCtrl);
	DDX_Control(pDX, IDC_EDIT_SOCKS_ADDR, m_SOCKSProxyServerAddressCtrl);
	DDX_Control(pDX, IDC_EDIT_SOCKS_PORT, m_SOCKSProxyServerPortCtrl);
	DDX_Control(pDX, IDC_EDIT_SOCKS_NAME, m_SOCKSProxyServerUsernameCtrl);
	DDX_Control(pDX, IDC_EDIT_SOCKS_PASS, m_SOCKSProxyServerPasswordCtrl);

	DDX_Text(pDX, IDC_EDIT_HTTP_ADDR, m_strHTTPProxyServerAddress);
	DDV_MaxChars(pDX, m_strHTTPProxyServerAddress, 256);
	DDX_Text(pDX, IDC_EDIT_HTTP_PORT, m_uiHTTPProxyServerPort);
	DDV_MinMaxUInt(pDX, m_uiHTTPProxyServerPort, 0, 65536);
	DDX_Text(pDX, IDC_EDIT_HTTP_NAME, m_strHTTPProxyServerUsername);
	DDV_MaxChars(pDX, m_strHTTPProxyServerUsername, 256);
	DDX_Text(pDX, IDC_EDIT_HTTP_PASS, m_strHTTPProxyServerPassword);
	DDV_MaxChars(pDX, m_strHTTPProxyServerPassword, 256);
	DDX_Text(pDX, IDC_EDIT_SOCKS_ADDR, m_strSOCKSProxyServerAddress);
	DDV_MaxChars(pDX, m_strSOCKSProxyServerAddress, 256);
	DDX_Text(pDX, IDC_EDIT_SOCKS_PORT, m_uiSOCKSProxyServerPort);
	DDV_MinMaxUInt(pDX, m_uiSOCKSProxyServerPort, 0, 65536);
	DDX_Text(pDX, IDC_EDIT_SOCKS_NAME, m_strSOCKSProxyServerUsername);
	DDV_MaxChars(pDX, m_strSOCKSProxyServerUsername, 256);
	DDX_Text(pDX, IDC_EDIT_SOCKS_PASS, m_strSOCKSProxyServerPassword);
	DDV_MaxChars(pDX, m_strSOCKSProxyServerPassword, 256);
}


BEGIN_MESSAGE_MAP(CProxyServerDlg, CDialog)
    ON_BN_CLICKED(IDC_CHECK_HTTP, OnBnClickedUseHTTPProxyServer)
    ON_BN_CLICKED(IDC_CHECK_SOCKS, OnBnClickedUseSOCKSProxyServer)
	ON_BN_CLICKED(IDC_CHECK_HTTP_AUTH, OnBnClickedUseHttpAuth)
END_MESSAGE_MAP()


// Utility Functions


//////////
// CProxyServerDlg::LoadLanguage
// arguments:	void
// returns:		void
// function:	loads new captions from language file
void CProxyServerDlg::LoadLanguage()
{
	int const static nIDs[] = { IDC_STATIC_PROXY, IDC_STATIC_HTTP, IDC_CHECK_HTTP,
		IDC_STATIC_HTTP_ADDR, IDC_STATIC_HTTP_PORT, IDC_STATIC_SOCKS,
		IDC_CHECK_SOCKS, IDC_STATIC_SOCKS_ADDR, IDC_STATIC_SOCKS_PORT,
		IDC_STATIC_SOCKS_BLANK, IDC_STATIC_SOCKS_NAME, IDC_STATIC_SOCKS_PASS,
		IDOK, IDCANCEL, 0 };

	UpdateLanguageStrings(this, "DIALOG-PROXY", nIDs);
}


// CProxyServerDlg message handlers

BOOL CProxyServerDlg::OnInitDialog()
{
    CDialog::OnInitDialog();

    LoadLanguage();

    m_strHTTPProxyServerAddress = gstate.pi.http_server_name;
    m_uiHTTPProxyServerPort = gstate.pi.http_server_port;
    m_strHTTPProxyServerUsername = gstate.pi.http_user_name;
    m_strHTTPProxyServerPassword = gstate.pi.http_user_passwd;
    m_strSOCKSProxyServerAddress = gstate.pi.socks_server_name;
    m_uiSOCKSProxyServerPort = gstate.pi.socks_server_port;
    m_strSOCKSProxyServerUsername = gstate.pi.socks5_user_name;
    m_strSOCKSProxyServerPassword = gstate.pi.socks5_user_passwd;

    if (gstate.pi.use_http_proxy) {
        m_UseHTTPProxyServerCtrl.SetCheck(BST_CHECKED);
    } else {
        m_UseHTTPProxyServerCtrl.SetCheck(BST_UNCHECKED);
    }

    if (gstate.pi.use_socks_proxy) {
        m_UseSOCKSProxyServerCtrl.SetCheck(BST_CHECKED);
    } else {
        m_UseSOCKSProxyServerCtrl.SetCheck(BST_UNCHECKED);
    }

    if (gstate.pi.use_http_auth) {
        m_UseHTTPProxyAuthenticationCtrl.SetCheck(BST_CHECKED);
    } else {
        m_UseHTTPProxyAuthenticationCtrl.SetCheck(BST_UNCHECKED);
    }

    OnBnClickedUseHTTPProxyServer();
    OnBnClickedUseSOCKSProxyServer();
    OnBnClickedUseHttpAuth();

    UpdateData(FALSE);
    CenterWindow();

    return TRUE;  // return TRUE unless you set the focus to a control
}


void CProxyServerDlg::OnBnClickedUseHTTPProxyServer()
{
    if ( BST_CHECKED == m_UseHTTPProxyServerCtrl.GetCheck() ) {
        m_HTTPProxyServerAddressCtrl.EnableWindow();
        m_HTTPProxyServerPortCtrl.EnableWindow();
        m_UseHTTPProxyAuthenticationCtrl.EnableWindow();
		CProxyServerDlg::OnBnClickedUseHttpAuth();
    } else {
        m_HTTPProxyServerAddressCtrl.EnableWindow(FALSE);
        m_HTTPProxyServerPortCtrl.EnableWindow(FALSE);
        m_UseHTTPProxyAuthenticationCtrl.EnableWindow(FALSE);
        m_HTTPProxyServerUsernameCtrl.EnableWindow(FALSE);
        m_HTTPProxyServerPasswordCtrl.EnableWindow(FALSE);
    }
}


void CProxyServerDlg::OnBnClickedUseSOCKSProxyServer()
{
    if ( BST_CHECKED == m_UseSOCKSProxyServerCtrl.GetCheck() ) {
        m_SOCKSProxyServerAddressCtrl.EnableWindow();
        m_SOCKSProxyServerPortCtrl.EnableWindow();
        m_SOCKSProxyServerUsernameCtrl.EnableWindow();
        m_SOCKSProxyServerPasswordCtrl.EnableWindow();
    } else {
        m_SOCKSProxyServerAddressCtrl.EnableWindow(FALSE);
        m_SOCKSProxyServerPortCtrl.EnableWindow(FALSE);
        m_SOCKSProxyServerUsernameCtrl.EnableWindow(FALSE);
        m_SOCKSProxyServerPasswordCtrl.EnableWindow(FALSE);
    }
}


void CProxyServerDlg::OnBnClickedUseHttpAuth()
{
    if ( BST_CHECKED == m_UseHTTPProxyAuthenticationCtrl.GetCheck() ) {
        m_HTTPProxyServerUsernameCtrl.EnableWindow();
        m_HTTPProxyServerPasswordCtrl.EnableWindow();
    } else {
        m_HTTPProxyServerUsernameCtrl.EnableWindow(FALSE);
        m_HTTPProxyServerPasswordCtrl.EnableWindow(FALSE);
    }
}

void CProxyServerDlg::OnOK()
{
    UpdateData(TRUE);

    gstate.pi.use_http_proxy = (m_UseHTTPProxyServerCtrl.GetCheck() != BST_UNCHECKED);
    safe_strncpy(gstate.pi.http_server_name, m_strHTTPProxyServerAddress.GetBuffer(), sizeof(gstate.pi.http_server_name));
    gstate.pi.http_server_port = m_uiHTTPProxyServerPort;

    gstate.pi.use_http_auth = (m_UseHTTPProxyAuthenticationCtrl.GetCheck() != BST_UNCHECKED);
    safe_strncpy(gstate.pi.http_user_name, m_strHTTPProxyServerUsername.GetBuffer(), sizeof(gstate.pi.http_user_name));
    safe_strncpy(gstate.pi.http_user_passwd, m_strHTTPProxyServerPassword.GetBuffer(), sizeof(gstate.pi.http_user_passwd));

	gstate.pi.use_socks_proxy = (m_UseSOCKSProxyServerCtrl.GetCheck() != BST_UNCHECKED);
    safe_strncpy(gstate.pi.socks_server_name, m_strSOCKSProxyServerAddress.GetBuffer(), sizeof(gstate.pi.socks_server_name));
    gstate.pi.socks_server_port = m_uiSOCKSProxyServerPort;
    safe_strncpy(gstate.pi.socks5_user_name, m_strSOCKSProxyServerUsername.GetBuffer(), sizeof(gstate.pi.socks5_user_name));
    safe_strncpy(gstate.pi.socks5_user_passwd, m_strSOCKSProxyServerPassword.GetBuffer(), sizeof(gstate.pi.socks5_user_passwd));

    m_strHTTPProxyServerAddress.ReleaseBuffer();
    m_strHTTPProxyServerUsername.ReleaseBuffer();
    m_strHTTPProxyServerPassword.ReleaseBuffer();
    m_strSOCKSProxyServerAddress.ReleaseBuffer();
    m_strSOCKSProxyServerUsername.ReleaseBuffer();
    m_strSOCKSProxyServerPassword.ReleaseBuffer();

    if (m_strSOCKSProxyServerUsername.IsEmpty()) {
        gstate.pi.socks_version = SOCKS_VERSION_4;
    } else {
        gstate.pi.socks_version = SOCKS_VERSION_5;
    }

    gstate.set_client_state_dirty(_T("CProxyServerDlg::OnOK"));

    CDialog::OnOK();
}

