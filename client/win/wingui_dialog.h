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

#ifndef __WIN_DIALOG_H_
#define __WIN_DIALOG_H_

#include "wingui.h"

//////////
// class:		CLoginDialog
// parent:		CDialog
// description:	gets login information from user.
class CLoginDialog : public CDialog 
{
public:
							CLoginDialog(UINT, LPCTSTR, LPCTSTR);
	afx_msg BOOL			OnInitDialog();
	CString					m_strUrl;
	CString					m_strAuth;

protected:
	CString					m_strUrlTT;
	CString					m_strAuthTT;
	void					LoadLanguage();

	afx_msg void			OnOK();
	afx_msg BOOL			OnToolTipNotify(UINT, NMHDR*, LRESULT*);
	DECLARE_MESSAGE_MAP()
};

//////////
// class:		CConnectDialog
// parent:		CDialog
// description:	request users permission to connect to the network
class CConnectDialog : public CDialog
{
public:
							CConnectDialog(UINT);
	afx_msg BOOL			OnInitDialog();
protected:
	void					LoadLanguage();

	afx_msg void			OnOK();
	DECLARE_MESSAGE_MAP()
};

//////////
// class:		CAboutDialog
// parent:		CDialog
// description:	displays information about BOINC
class CAboutDialog : public CDialog
{
public:
							CAboutDialog(UINT);
	afx_msg BOOL			OnInitDialog();
	
protected:
	void					LoadLanguage();

	DECLARE_MESSAGE_MAP()
};

#endif
