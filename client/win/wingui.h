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
#include <math.h>
#include "graphics_api.h"
#include "file_names.h"
#include "filesys.h"
#include "log_flags.h"
#include "client_state.h"
#include "account.h"
#include "resource.h"
#include "win_net.h"

// constants

#define WND_TITLE			"BOINC"		// window's title

#ifndef IDC_HAND
#define IDC_HAND			MAKEINTRESOURCE(32649)	// hand pointer, the "hidden resource"
#endif

#define ICON_OFF			0			// remove icon
#define ICON_NORMAL			1			// normal icon
#define ICON_HIGHLIGHT		2			// highlighted icon

#define DEF_COL_WIDTH		80			// default width of list columns

#define SORT_ASCEND			0			// sorting orders
#define SORT_DESCEND		1

#define EDGE_BUFFER			2			// buffer pixels around edge of client

#define MAX_MESSAGE_LINES	20			// the maximum lines in the message control

#define ID_TIMER			104			// timer id

#define PIE_MAJOR_MAX		0.25		// max size of the screen of the pie's major axis
#define PIE_MINOR_MAX		0.25		// max size of the screen of the pie's minor axis
#define PIE_BUFFER			20			// buffer pixels around edge of pie chart
#define PIE_DEPTH			0.25		// depth of pie chart
#define PI					3.14159		// pi

#define STATUS_ICON_ID		(WM_USER + 1)	// id for notifications from status icon

#define STATUS_MENU			0			// submenus for context menus
#define PROJECT_MENU		1
#define RESULT_MENU			2
#define XFER_MENU			3

#define PROJECT_ID			0			// child control ids
#define RESULT_ID			1
#define XFER_ID				2
#define MESSAGE_ID			3
#define MAX_LIST_ID			4			// for column titles
#define USAGE_ID			4
#define TAB_ID				5

#define PROJECT_COLS		5			// number of columns for each control
#define RESULT_COLS			7
#define XFER_COLS			6
#define MESSAGE_COLS		3
#define MAX_COLS			7

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
	afx_msg void			OnRButtonDown(UINT, CPoint);
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
	int						InsertItem(int, LPCTSTR);
	void					GetTextRect(int, int, LPRECT);
	int						GetColumnWidth(int);
	BOOL					SetColumnWidth(int, int);
	void					SetItemColor(int, COLORREF);
	void					SetProjectURL(int, char*);

protected:
	CMenu					m_PopupMenu;			// context menu for header
	CMapStringToOb			m_Progs;				// maps coordinate string to progress control
	CProgressHeaderCtrl		m_Header;				// header for subclassing
	CArray<int,int>			m_ColWidths;			// column widths for hiding and unhiding; a[i] > 0: col i shown; a[i] < 0: col i hidden, previous width -(a[i] - 1)
	int						m_nSort;				// column and order of last sort: i = 0: no sort; i > 0: sorted ascending by col i - 1; < 0 sorted descending by col -(i-1)
	CFont*					m_OldFont;				// old font for setting subitem font
	CArray<COLORREF,COLORREF>		m_ItemColors;	// special colors of items
	CArray<CString,CString>			m_ProjectURLs;	// urls for project links

	void					SwapItems(int, int);
	// TODO: fix selection sort algorithm
	void					Sort(int, int);
	void					SwapColumnVisibility(int);

	afx_msg BOOL			OnCommand(WPARAM, LPARAM);
    afx_msg int				OnCreate(LPCREATESTRUCT);
    afx_msg void			OnDestroy();
	afx_msg BOOL			OnNotify(WPARAM, LPARAM, LRESULT*);
	afx_msg void			OnCustomDraw(NMHDR*, LRESULT*);
	afx_msg void			OnPaint();
	// TODO: context menu for items?
	afx_msg BOOL			OnSetCursor(CWnd*, UINT, UINT);
	afx_msg void			OnLButtonDown(UINT, CPoint);
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
	void					AddPiece(LPTSTR, COLORREF, double);
	void					SetPiece(int, double);
	BOOL					Create(DWORD, const RECT&, CWnd*, UINT);
	void					SetFont(CFont*);
	void					SetTotal(double);

protected:
	double					m_xTotal;				// total amount of pie
	CArray<double,double>	m_xValues;				// specific values of pieces
	CArray<COLORREF,COLORREF>	m_colors;			// colors of pieces
	CArray<CString,CString>		m_strLabels;		// labels of pieces
	CFont*					m_pFont;				// font for control

	void					DrawPiePiece(CDC*, double, double);
	void					CirclePoint(CPoint*, int, double, CPoint*);
	void					EllipsePoint(CRect*, double, CPoint*);

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
	void					MessageUser(char*,char*,char*);
    BOOL					IsSuspended();

protected:
	CMenu					m_MainMenu;				// window's main menu
	CProgressListCtrl		m_ProjectListCtrl;		// list control
	CProgressListCtrl		m_XferListCtrl;			// list control
	CProgressListCtrl		m_ResultListCtrl;		// list control
	CProgressListCtrl		m_MessageListCtrl;		// list control for messages to user
	CPieChartCtrl			m_UsagePieCtrl;			// pie chart control
	CFont					m_Font;					// window's font
	CTabCtrl				m_TabCtrl;				// tab control for choosing display
	CImageList				m_TabIL;				// image list for tab control
	CBitmap					m_TabBMP[5];			// bitmaps for tab image list
	HINSTANCE				m_hIdleDll;				// handle to dll for user idle
	int						m_nIconState;			// state of the status icon
	BOOL					m_bMessage;				// does the user have a new message?
	BOOL					m_bSuspend;				// should apps be suspended?
	int						m_nContextItem;			// item selected for context menu

    void					SetStatusIcon(DWORD);
    void					SaveUserSettings();
    void					LoadUserSettings();
	DWORD					GetUserIdleTime();
	void					Syncronize(CProgressListCtrl*, vector<void*>*);
    virtual void			PostNcDestroy();

    afx_msg void			OnClose();
	afx_msg void			OnCommandSettingsQuit();
	afx_msg void			OnCommandSettingsLogin();
	afx_msg void			OnCommandSettingsProxyServer();
	afx_msg void			OnCommandHelpAbout();
	afx_msg void			OnCommandProjectRelogin();
	afx_msg void			OnCommandProjectQuit();
	afx_msg void			OnCommandFileShowGraphics();
	afx_msg void			OnCommandFileClearInactive();
	afx_msg void			OnCommandFileClearMessages();
	afx_msg void			OnCommandHide();
	afx_msg void			OnCommandSuspend();
	afx_msg void			OnCommandExit();
    afx_msg int				OnCreate(LPCREATESTRUCT);
	afx_msg BOOL			OnNotify(WPARAM, LPARAM, LRESULT*);
	afx_msg void			OnRButtonDown(UINT, CPoint);
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
							CLoginDialog(UINT, LPCTSTR, LPCTSTR);
	afx_msg BOOL			OnInitDialog();
	CString					m_strUrl;
	CString					m_strAuth;

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
	int						m_nSel;

protected:
	afx_msg void			OnOK();
	afx_msg BOOL			OnToolTipNotify(UINT, NMHDR*, LRESULT*);
	DECLARE_MESSAGE_MAP()
};

//////////
// class:		CProxyDialog
// parent:		CDialog
// description:	allows user to set up proxy information
class CProxyDialog : public CDialog
{
public:
							CProxyDialog(UINT);
	afx_msg BOOL			OnInitDialog();
	
protected:
	void					EnableHttp(BOOL bEnable);
	void					EnableSocks(BOOL bEnable);
	afx_msg void			OnHttp();
	afx_msg void			OnSocks();
	afx_msg void			OnOK();
	DECLARE_MESSAGE_MAP()
};

// globals

extern CMyApp g_myApp;
extern CMainWindow* g_myWnd;

#endif