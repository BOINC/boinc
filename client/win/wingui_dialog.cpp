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
	m_strAuthTT.Format("The account ID received in your confirmation email.");
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

    CString strOldTitle, strTitle;
    GetWindowText(strOldTitle);
#ifdef _DEBUG
    strTitle.Format("%s %s (DEBUG)", strOldTitle, BOINC_VERSION_STRING);
#else
    strTitle.Format("%s %s", strOldTitle, BOINC_VERSION_STRING);
#endif
    SetWindowText(strTitle);
    CenterWindow();
    return TRUE;
}

const char *BOINC_RCSID_5a34f25aa3 = "$Id$";
