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
