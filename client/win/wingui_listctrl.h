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

#ifndef __WIN_LISTCTRL_H_
#define __WIN_LISTCTRL_H_

// constants

#ifndef IDC_HAND
#define IDC_HAND			MAKEINTRESOURCE(32649)	// hand pointer, the "hidden resource"
#endif

#define SORT_ASCEND			0			// sorting orders
#define SORT_DESCEND		1

#define SORT_ALPHA		0 //sorting types
#define SORT_NUMERIC	1

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
	int						InsertColumn(int, LPCTSTR, int, int, int, int);
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
	int						m_nSort;				// column and order of last sort: i = 0: no sort; i > 0: sorted ascending by col i - 1; < 0 sorted descending by col -(i-1)
	CFont*					m_OldFont;				// old font for setting subitem font
	CArray<int,int>			m_ColWidths;			// column widths for hiding and unhiding; a[i] > 0: col i shown; a[i] < 0: col i hidden, previous width -(a[i] - 1)
	CArray<int,int>			m_ColType;				// column type for sorting; a[i] == 0: alpha; a[i]==1 :numeric
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
	//afx_msg void			OnLButtonDown(UINT, CPoint);  // Removed to allow highlighting.
	afx_msg void			OnRButtonDown(UINT, CPoint);
    DECLARE_MESSAGE_MAP()
};


#endif
