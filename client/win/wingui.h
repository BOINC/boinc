// The contents of this file are subject to the Mozilla Public License
// Version 1.0 (the "License"); you may not use this file except in
// compliance with the License. You may obtain a copy of the License at
// http://www.mozilla.org/MPL/ 
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

#ifndef __WINGUI_H_
#define __WINGUI_H_

// includes

#include <afxwin.h>
#include <afxcmn.h>
#include <afxtempl.h>
#include <afxcoll.h>
#include <afxext.h>
#include <afxhtml.h>
#include <math.h>
#include "filesys.h"
#include "log_flags.h"
#include "client_state.h"
#include "account.h"
#include "resource.h"
#include "win_net.h"

// constants

#define ICON_OFF			0			// remove icon
#define ICON_NORMAL			1			// normal icon
#define ICON_HIGHLIGHT		2			// highlighted icon

#define DEF_COL_WIDTH		80			// default width of list columns

#define SORT_ASCEND			0			// sorting orders
#define SORT_DESCEND		1

#define EDGE_BUFFER			2			// buffer pixels around edge of client

#define MAX_MESSAGE_LINES	20			// the maximum lines in the message control

#define ID_TIMER			104			// timer id

#define PIE_BUFFER			20			// buffer pixels around edge of pie chart
#define PIE_DEPTH			0.25		// depth of pie chart
#define PI					3.14159		// pi

// typedefs

typedef BOOL (CALLBACK* InitFn)();
typedef void (CALLBACK* TermFn)();
typedef DWORD (CALLBACK* GetFn)();

// classes

//////////
// class:		CProgressBarCtrl
// parent:		CProgressCtrl
// description:	forwards specific mouse messages to parent window.
class CProgressBarCtrl : public CProgressCtrl
{
public:
							CProgressBarCtrl();

protected:
	afx_msg void			OnLButtonDown(UINT, CPoint);
	afx_msg void			OnLButtonUp(UINT, CPoint);
    DECLARE_MESSAGE_MAP()
};

//////////
// class:		CProgressHeaderCtrl
// parent:		CHeaderCtrl
// description:	forwards specific mouse messages to parent window.
class CProgressHeaderCtrl : public CHeaderCtrl
{
public:
							CProgressHeaderCtrl();

protected:
	afx_msg void			OnRButtonDown(UINT, CPoint);
	afx_msg void			OnRButtonUp(UINT, CPoint);
    DECLARE_MESSAGE_MAP()
};

//////////
// class:		CProgressListCtrl
// parent:		CListCtrl
// description:	extends basic functionality of standard list control by
//				allowind embedded progress controls, sorting by columns,
//				and hiding columns.
class CProgressListCtrl : public CListCtrl
{
public:
							CProgressListCtrl();
							~CProgressListCtrl();
	BOOL					DeleteItem(int);
	void					SetItemProgress(int, int, int);
	void					RepositionProgress();
	int						InsertColumn(int, LPCTSTR, int, int, int);
	int						GetColumnWidth(int);
	BOOL					SetColumnWidth(int, int);

protected:
	CMenu					m_PopupMenu;			// context menu for header
	CMapStringToOb			m_Progs;				// maps coordinate string to progress control
	CProgressHeaderCtrl		m_Header;				// header for subclassing
	CArray<int,int>			m_ColWidths;			// column widths for hiding and unhiding; a[i] > 0: col i shown; a[i] < 0: col i hidden, previous width -(a[i] - 1)
	int						m_iSort;				// column and order of last sort: i = 0: no sort; i > 0: sorted ascending by col i - 1; < 0 sorted descending by col -(i-1)

	void					SwapItems(int, int);
	// TODO: fix selection sort algorithm
	void					Sort(int, int);
	void					SwapColumnVisibility(int);

	afx_msg BOOL			OnCommand(WPARAM, LPARAM);
    afx_msg int				OnCreate(LPCREATESTRUCT);
    afx_msg void			OnDestroy();
	afx_msg BOOL			OnNotify(WPARAM, LPARAM, LRESULT*);
	afx_msg void			OnPaint();
	// TODO: context menu for items?
	afx_msg void			OnRButtonDown(UINT, CPoint);
    DECLARE_MESSAGE_MAP()
};

//////////
// class:		CPieChartCtrl
// parent:		CWnd
// description:	contains the functionality of a pie chart
class CPieChartCtrl : public CWnd
{
public:
							CPieChartCtrl();
	void					AddPiece(LPTSTR, COLORREF, float, float);
	void					SetPiece(int, float);
	BOOL					Create(DWORD, const RECT&, CWnd*, UINT);
	void					SetFont(CFont*);
	void					SetTag(char*);

protected:
	float					m_Total;				// total percentage taken up by pieces
	CArray<float,float>		m_Percents;				// specific percentages of pieces
	CArray<COLORREF,COLORREF>		m_Colors;		// colors of pieces
	CArray<CString,CString>			m_Labels;		// labels of pieces
	CFont*					m_Font;					// font for control
	float					m_Base;					// base units of pie
	CString					m_Tag;					// tag for numbers on labels

	void					DrawPiePiece(CDC*, float, float);
	void					CirclePoint(CPoint*, int, float, CPoint*);
	void					EllipsePoint(CRect*, float, CPoint*);

	afx_msg void			OnPaint();
    DECLARE_MESSAGE_MAP()
};

//////////
// class:		CMyApp
// parent:		CWinApp
// description:	subclasses CWinApp to create main window.
class CMyApp : public CWinApp 
{
public:
    virtual BOOL			InitInstance();
};

//////////
// class:		CMainWindow
// parent:		CWnd
// description:	the main window, organizes child control windows and client
//				state, handles timer updates, some display features.
class CMainWindow : public CWnd
{
public:
							CMainWindow ();
	static void CALLBACK	TimerProc(HWND, UINT, UINT, DWORD);
	void					UpdateGUI(CLIENT_STATE*);
	void					MessageUser(char*,char*);
	int						GetInitialProject();
    BOOL					IsSuspended();

protected:
	CMenu					m_MainMenu;				// window's main menu
	CBitmap					m_Logo;					// bitmap of the boinc logo
	CProgressListCtrl		m_ProjectListCtrl;		// list control
	CProgressListCtrl		m_XferListCtrl;			// list control
	CProgressListCtrl		m_ResultListCtrl;		// list control
	CPieChartCtrl			m_UsagePieCtrl;			// pie chart control
	CEdit					m_MessageEditCtrl;		// edit control for messages to user
	CFont					m_Font;					// window's font
	CTabCtrl				m_TabCtrl;				// tab control for choosing display
	CImageList				m_TabIL;				// image list for tab control
	CBitmap					m_TabBMP[5];			// bitmaps for tab image list
	HINSTANCE				m_IdleDll;				// handle to dll for user idle
	int						m_IconState;			// state of the status icon
	BOOL					m_Message;				// does the user have a new message?
	BOOL					m_Suspend;				// should apps be suspended?

    void					SetStatusIcon(DWORD);
    void					SaveUserSettings();
    void					LoadUserSettings();
    double					GetDiskSize();
    double					GetDiskFree();
	DWORD					GetUserIdleTime();
	void					Syncronize(CProgressListCtrl*, vector<void*>*);
    virtual void			PostNcDestroy();

    afx_msg void			OnClose();
	afx_msg void			OnCommandAccountQuit();
	afx_msg void			OnCommandAccountLogin();
	afx_msg void			OnCommandHelpAbout();
	afx_msg void			OnCommandFileClose();
	afx_msg void			OnCommandStatusIconHide();
	afx_msg void			OnCommandStatusIconQuit();
	afx_msg void			OnCommandSuspend();
    afx_msg int				OnCreate(LPCREATESTRUCT);
	afx_msg BOOL			OnNotify(WPARAM, LPARAM, LRESULT*);
    afx_msg void			OnPaint();
    afx_msg void			OnSetFocus(CWnd*);
    afx_msg void			OnSize(UINT, int, int);
	afx_msg LRESULT			OnStatusIcon(WPARAM, LPARAM);
    DECLARE_MESSAGE_MAP()
};

//////////
// class:		CLoginDialog
// parent:		CDialog
// description:	gets login information from user.
class CLoginDialog : public CDialog 
{
public:
							CLoginDialog(UINT);
	afx_msg BOOL			OnInitDialog();
	CString					m_url;
	CString					m_auth;

protected:
	afx_msg void			OnOK();
	afx_msg BOOL			OnToolTipNotify(UINT, NMHDR*, LRESULT*);
	DECLARE_MESSAGE_MAP()
};

//////////
// class:		CQuitDialog
// parent:		CDialog
// description:	gets project quit information from user.
class CQuitDialog : public CDialog
{
public:
							CQuitDialog(UINT);
	afx_msg BOOL			OnInitDialog();
	int						m_sel;

protected:
	afx_msg void			OnOK();
	afx_msg BOOL			OnToolTipNotify(UINT, NMHDR*, LRESULT*);
	DECLARE_MESSAGE_MAP()
};

// globals

extern CMyApp g_myApp;
extern CMainWindow* g_myWnd;

#endif