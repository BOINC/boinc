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

#ifndef __WIN_LISTCTRL_H_
#define __WIN_LISTCTRL_H_

// includes

#include "wingui.h"

// constants

#ifndef IDC_HAND
#define IDC_HAND			MAKEINTRESOURCE(32649)	// hand pointer, the "hidden resource"
#endif

#define SORT_ASCEND			0			// sorting orders
#define SORT_DESCEND		1

//////////
// class:		CProgressBarCtrl
// parent:		CProgressCtrl
// description:	forwards specific mouse messages to parent window.
class CProgressBarCtrl : public CProgressCtrl
{
public:
							CProgressBarCtrl();
	double					SetPos(double);
	double					GetPos();
	void					SetTextColor(COLORREF);
	void					SetBarColor(COLORREF);
	void					SetBkColor(COLORREF);

protected:
	COLORREF				m_crText;
	double					m_xPos;

	afx_msg void			OnPaint();
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
	void					SetItemProgress(int, int, double);
	double					GetItemProgress(int, int);
	void					RepositionProgress();
	int						InsertColumn(int, LPCTSTR, int, int, int);
	int						InsertItem(int, LPCTSTR);
	void					GetTextRect(int, int, LPRECT);
	void					GetColumnTitle(int, CString&);
	int						GetColumnWidth(int);
	CString					GetItemTextOrPos(int, int);
	BOOL					SetColumnWidth(int, int);
	void					SetItemColor(int, COLORREF);
	void					SetMenuItems(char**, int);
	void					SaveInactive(char*, char*);
	void					LoadInactive(char*, char*);
	void					SetProjectURL(int, char*);
	CString					GetProjectURL(int);

protected:
	CMenu					m_PopupMenu;			// context menu for header
	CMapStringToOb			m_Progs;				// maps coordinate string to progress control
	CProgressHeaderCtrl		m_Header;				// header for subclassing
	CArray<int,int>			m_ColWidths;			// column widths for hiding and unhiding; a[i] > 0: col i shown; a[i] < 0: col i hidden, previous width -(a[i] - 1)
	int						m_nSort;				// column and order of last sort: i = 0: no sort; i > 0: sorted ascending by col i - 1; < 0 sorted descending by col -(i-1)
	CFont*					m_OldFont;				// old font for setting subitem font
	CArray<COLORREF,COLORREF>	m_ItemColors;		// special colors of items
	CArray<CString,CString>		m_ProjectURLs;		// urls for project links

	void					SwapItems(int, int);
	void					QSort(int, int, int, int);
	void					Sort(int, int);
	void					SwapColumnVisibility(int);

	afx_msg BOOL			OnCommand(WPARAM, LPARAM);
    afx_msg int				OnCreate(LPCREATESTRUCT);
    afx_msg void			OnDestroy();
	afx_msg BOOL			OnNotify(WPARAM, LPARAM, LRESULT*);
	afx_msg void			OnCustomDraw(NMHDR*, LRESULT*);
	afx_msg void			OnPaint();
	afx_msg void			OnLButtonDown(UINT, CPoint);
	afx_msg void			OnRButtonDown(UINT, CPoint);
    DECLARE_MESSAGE_MAP()
};

#endif
