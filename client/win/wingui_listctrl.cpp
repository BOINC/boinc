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

#include "wingui_listctrl.h"

/////////////////////////////////////////////////////////////////////////
// CProgressHeaderCtrl message map and member functions

BEGIN_MESSAGE_MAP(CProgressBarCtrl, CProgressCtrl)
	ON_WM_PAINT()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_RBUTTONDOWN()
END_MESSAGE_MAP()

//////////
// CProgressBarCtrl::CProgressBarCtrl
// arguments:	void
// returns:		void
// function:	initializes color and position
CProgressBarCtrl::CProgressBarCtrl()
{
	m_crText = RGB(0, 0, 0);
	m_xPos = 0;
}

//////////
// CProgressBarCtrl::SetPos
// arguments:	xPos: the new position for hte progress bar
// returns:		the previous position of the progress bar
// function:	sets the position of the progress bar
double CProgressBarCtrl::SetPos(double xPos)
{
	double oldPos = m_xPos;
	m_xPos = xPos;
	CProgressCtrl::SetPos((int)xPos);
	return oldPos;
}

//////////
// CProgressBarCtrl::GetPos
// arguments:	void
// returns:		the position of the progress bar
// function:	gets the position of the progress bar
double CProgressBarCtrl::GetPos()
{
	return m_xPos;
}

//////////
// CProgressBarCtrl::SetTextColor
// arguments:	crNew: new color for the text
// returns:		void
// function:	sets the color of the text in the progress bar
void CProgressBarCtrl::SetTextColor(COLORREF crNew)
{
	m_crText = crNew;
}

//////////
// CProgressBarCtrl::SetBarColor
// arguments:	crNew: new color for the bar
// returns:		void
// function:	sets the color of the progress bar
void CProgressBarCtrl::SetBarColor(COLORREF crNew)
{
	SendMessage(PBM_SETBARCOLOR, 0, (LPARAM)crNew);
}

//////////
// CProgressBarCtrl::SetBkColor
// arguments:	crNew: new color for the background
// returns:		void
// function:	sets the color of the background of the progress bar
void CProgressBarCtrl::SetBkColor(COLORREF crNew)
{
	SendMessage(PBM_SETBKCOLOR, 0, (LPARAM)crNew);
}

//////////
// CProgressBarCtrl::OnPaint
// arguments:	void
// returns:		void
// function:	writes the progress in text
void CProgressBarCtrl::OnPaint()
{
	InvalidateRect(NULL, TRUE);
	CProgressCtrl::OnPaint();

	CString strProg;
	strProg.Format("%0.2f%%", GetPos());

	CClientDC cdc(this);
	CRect rt;
	GetClientRect(&rt);
	rt.top -= 2; rt.right += 2;
	CFont* pOldFont = NULL;
	int nOldMode;
	COLORREF crOldColor;
	pOldFont = cdc.SelectObject(GetParent()->GetFont());
	crOldColor = cdc.SetTextColor(m_crText);
	nOldMode = cdc.SetBkMode(TRANSPARENT);
	cdc.DrawText(strProg, &rt, DT_CENTER|DT_VCENTER|DT_END_ELLIPSIS);
	cdc.SelectObject(pOldFont);
	cdc.SetTextColor(crOldColor);
	cdc.SetBkMode(nOldMode);
}

//////////
// CProgressBarCtrl::OnLButtonDown
// arguments:	nFlags: message flags (keys down)
//				point: mouse's point
// returns:		void
// function:	convert point to parent window's coordinates and forward message.
void CProgressBarCtrl::OnLButtonDown(UINT nFlags, CPoint point)
{
	CProgressCtrl::OnLButtonDown(nFlags, point);

	// if this control has a parent, repackage this message and forward it
	CWnd* pWndParent = GetParent();
	if(pWndParent) {
		MapWindowPoints(pWndParent, &point, 1);
		WPARAM wParam = nFlags;
		LPARAM lParam = MAKELPARAM(point.x, point.y);
		pWndParent->SendMessage(WM_LBUTTONDOWN, wParam, lParam);
	}
}

//////////
// CProgressBarCtrl::OnLButtonUp
// arguments:	nFlags: message flags (keys down)
//				point: mouse's point
// returns:		void
// function:	convert point to parent window's coordinates and forward message.
void CProgressBarCtrl::OnLButtonUp(UINT nFlags, CPoint point)
{
	CProgressCtrl::OnLButtonUp(nFlags, point);

	// if this control has a parent, repackage this message and forward it
	CWnd* pWndParent = GetParent();
	if(pWndParent) {
		MapWindowPoints(pWndParent, &point, 1);
		WPARAM wParam = nFlags;
		LPARAM lParam = MAKELPARAM(point.x, point.y);
		pWndParent->SendMessage(WM_LBUTTONUP, wParam, lParam);
	}
}

//////////
// CProgressBarCtrl::OnRButtonDown
// arguments:	nFlags: message flags (keys down)
//				point: mouse's point
// returns:		void
// function:	convert point to parent window's coordinates and forward message.
void CProgressBarCtrl::OnRButtonDown(UINT nFlags, CPoint point)
{
	CProgressCtrl::OnRButtonDown(nFlags, point);

	// if this control has a parent, repackage this message and forward it
	CWnd* pWndParent = GetParent();
	if(pWndParent) {
		MapWindowPoints(pWndParent, &point, 1);
		WPARAM wParam = nFlags;
		LPARAM lParam = MAKELPARAM(point.x, point.y);
		pWndParent->SendMessage(WM_RBUTTONDOWN, wParam, lParam);
	}
}

/////////////////////////////////////////////////////////////////////////
// CProgressHeaderCtrl message map and member functions

BEGIN_MESSAGE_MAP(CProgressHeaderCtrl, CHeaderCtrl)
	ON_WM_RBUTTONDOWN()
	ON_WM_RBUTTONUP()
END_MESSAGE_MAP()

//////////
// CProgressHeaderCtrl::CProgressHeaderCtrl
// arguments:	void
// returns:		void
// function:	void
CProgressHeaderCtrl::CProgressHeaderCtrl()
{
}

//////////
// CProgressHeaderCtrl::OnRButtonDown
// arguments:	nFlags: message flags (keys down)
//				point: mouse's point
// returns:		void
// function:	convert point to parent window's coordinates and forward message.
void CProgressHeaderCtrl::OnRButtonDown(UINT nFlags, CPoint point)
{
	CHeaderCtrl::OnRButtonDown(nFlags, point);

	// if this control has a parent, repackage this message and forward it
	CWnd* pWndParent = GetParent();
	if(pWndParent) {
		MapWindowPoints(pWndParent, &point, 1);
		WPARAM wParam = nFlags;
		LPARAM lParam = MAKELPARAM(point.x, point.y);
		pWndParent->SendMessage(WM_RBUTTONDOWN, wParam, lParam);
	}
}

//////////
// CProgressHeaderCtrl::OnRButtonUp
// arguments:	nFlags: message flags (keys down)
//				point: mouse's point
// returns:		void
// function:	convert point to parent window's coordinates and forward message.
void CProgressHeaderCtrl::OnRButtonUp(UINT nFlags, CPoint point)
{
	CHeaderCtrl::OnRButtonUp(nFlags, point);

	// if this control has a parent, repackage this message and forward it
	CWnd* pWndParent = GetParent();
	if(pWndParent) {
		MapWindowPoints(pWndParent, &point, 1);
		WPARAM wParam = nFlags;
		LPARAM lParam = MAKELPARAM(point.x, point.y);
		pWndParent->SendMessage(WM_RBUTTONUP, wParam, lParam);
	}
}

/////////////////////////////////////////////////////////////////////////
// CProgressListCtrl message map and member functions

BEGIN_MESSAGE_MAP(CProgressListCtrl, CListCtrl)
    ON_WM_CREATE()
    ON_WM_DESTROY()
    ON_WM_PAINT()
	ON_WM_SETCURSOR()
	//ON_WM_LBUTTONDOWN()
	ON_WM_RBUTTONDOWN()
	ON_NOTIFY_REFLECT(NM_CUSTOMDRAW, OnCustomDraw)
END_MESSAGE_MAP()

//////////
// CProgressListCtrl::CProgressListCtrl
// arguments:	void
// returns:		void
// function:	void
CProgressListCtrl::CProgressListCtrl()
{
}

//////////
// CProgressListCtrl::~CProgressListCtrl
// arguments:	void
// returns:		void
// function:	destroys menu
CProgressListCtrl::~CProgressListCtrl()
{
	if(m_PopupMenu.GetSafeHmenu()) {
		m_PopupMenu.DestroyMenu();
	}
}

//////////
// CProgressListCtrl::InsertColumn
// arguments:	nCol: index of new column
//				lpszColumnHeading: string for column heading
//				nFormat: text alignment
//				nWidth: width of column
//				nSubitem: subitem assosciated with column
//				nSortingType: type of sorting (alpha or numeric)
// returns:		index of new column if successful,otherwise -1
// function:	adds a new column to the list control
int CProgressListCtrl::InsertColumn(int nCol, LPCTSTR lpszColumnHeading, int nFormat = LVCFMT_LEFT, int nWidth = -1, int nSubItem = -1,int nSortingType = SORT_ALPHA)
{
	m_ColWidths.SetAtGrow(nCol, nWidth);
	m_ColType.SetAtGrow(nCol,nSortingType);
	return CListCtrl::InsertColumn(nCol, lpszColumnHeading, nFormat, nWidth, nSubItem);
}

//////////
// CProgressListCtrl::InsertItem
// arguments:	nItem: index of new item
//				lpszItem: text of new item
// returns:		index of new item if successful,otherwise -1
// function:	adds a new item to the list control
int CProgressListCtrl::InsertItem(int nItem, LPCTSTR lpszItem)
{
	m_ItemColors.InsertAt(nItem, RGB(0, 0, 0));
	CString StrEmpty;
	m_ProjectURLs.InsertAt(nItem, StrEmpty);
	return CListCtrl::InsertItem(nItem, lpszItem);
}

//////////
// CProgressListCtrl::GetColumnTitle
// arguments:	nCol: column to get title of
//				strTitle: reference to string to put title in
// returns:		void
// function:	gets the title of a column and puts it in a string,
//				if the column is out of bounds, sets the empty string
void CProgressListCtrl::GetColumnTitle(int nCol, CString& strTitle)
{
	if(nCol < 0 || nCol >= GetHeaderCtrl()->GetItemCount()) {
		strTitle.Empty();
		return;
	}
	char szTitle[256];
	LVCOLUMN lvcol;
	ZeroMemory(&lvcol, sizeof(LVCOLUMN));
	lvcol.mask = LVCF_TEXT;
	lvcol.pszText = szTitle;
	lvcol.cchTextMax = 256;
	GetColumn(nCol, &lvcol);
	strTitle.Format("%s", szTitle);
}

//////////
// CProgressListCtrl::GetColumnWidth
// arguments:	nCol: column to get width of
// returns:		width of column
// function:	gets the width of a column, negative width means hidden
int CProgressListCtrl::GetColumnWidth(int nCol)
{
	if(m_ColWidths.GetAt(nCol) >= 0) {
		return CListCtrl::GetColumnWidth(nCol);
	} else {
		return m_ColWidths.GetAt(nCol);
	}
}

//////////
// CProgressListCtrl::SetColumnWidth
// arguments:	nCol: column to set width of
//				cx: new width of column
// returns:		true if successful, otherwise false
// function:	sets the width of the given column, negative means hidden,
//				also checking or unchecking the menu item appropriate to
//				the column's visibility
BOOL CProgressListCtrl::SetColumnWidth(int nCol, int cx)
{
	if(cx < 0) {
		if(m_PopupMenu.GetSafeHmenu()) {
			m_PopupMenu.CheckMenuItem(nCol, MF_UNCHECKED);
			m_ColWidths.SetAtGrow(nCol, cx);
		}
		return CListCtrl::SetColumnWidth(nCol, 0);
	} else {
		if(m_PopupMenu.GetSafeHmenu()) {
			m_PopupMenu.CheckMenuItem(nCol, MF_CHECKED);
			m_ColWidths.SetAtGrow(nCol, cx);
		}
		return CListCtrl::SetColumnWidth(nCol, cx);
	}
}

//////////
// CProgressListCtrl::DeleteItem
// arguments:	nItem: item index
// returns:		true if sucessful, false otherwise
// function:	deletes given item from the list control
BOOL CProgressListCtrl::DeleteItem(int nItem)
{
	int i, si;

	// remove array info
	m_ItemColors.RemoveAt(nItem);
	m_ProjectURLs.RemoveAt(nItem);
	CString empty, strbuf;
	CProgressBarCtrl* pProgCtrl = NULL;

	// go through all the subitems and see if they have a progess control
	for(si = 0; si < GetHeaderCtrl()->GetItemCount(); si ++) {
		strbuf.Format("%d:%d", nItem, si);
		pProgCtrl = NULL;
		m_Progs.Lookup(strbuf, (CObject*&)pProgCtrl);
		if(pProgCtrl) {
			m_Progs.RemoveKey(strbuf);
			delete pProgCtrl;
		}
	}


	// move other progress controls up
	for(i = nItem + 1; i < GetItemCount(); i ++) {
		for(si = 0; si < GetHeaderCtrl()->GetItemCount(); si ++) {
			strbuf.Format("%d:%d", i, si);
			pProgCtrl = NULL;
			m_Progs.Lookup(strbuf, (CObject*&)pProgCtrl);
			if(pProgCtrl) {
				m_Progs.RemoveKey(strbuf);
				strbuf.Format("%d:%d", i - 1, si);
				m_Progs.SetAt(strbuf, pProgCtrl);
			}
		}
	}

	return CListCtrl::DeleteItem(nItem);
}

//////////
// CProgressListCtrl::SetItemProgress
// arguments:	nItem: item index
//				nSubitem: item's subitem to set progress for
//				xProg: position to set progress control
// returns:		void
// function:	sets the position of a progress control for a given
//				item and subitem; if there is none there, creates a new 
//				one, otherwise sets the progress of the one it finds.
void CProgressListCtrl::SetItemProgress(int nItem, int nSubItem, double xProg)
{
	CRect rt;
	CString strbuf;
	CProgressBarCtrl* pProgCtrl = NULL;
	if(xProg < 0) xProg = 0;
	if(xProg > 100) xProg = 100;

	// lookup the position of the progress control
	strbuf.Format("%d:%d", nItem, nSubItem);
	m_Progs.Lookup(strbuf, (CObject*&)pProgCtrl);
	if(pProgCtrl) {

		// found, so just update it's progress
		pProgCtrl->SetPos(xProg);
	} else {

		// not found, create one and put it in the map
		GetSubItemRect(nItem, nSubItem, LVIR_BOUNDS, rt);
		pProgCtrl = new CProgressBarCtrl();
		pProgCtrl->Create(PBS_SMOOTH|WS_CHILD|WS_VISIBLE, rt, this, 0);
		pProgCtrl->SetPos(xProg);
		pProgCtrl->SetBarColor(RGB(255, 255, 128));
		m_Progs.SetAt(strbuf, pProgCtrl);
	}
}

//////////
// CProgressListCtrl::GetItemProgress
// arguments:	nItem: item index
//				nSubitem: item's subitem to set progress for
// returns:		double
// function:	returns the position of a progress control for a given
//				item and subitem; if there is none there, return 0
double CProgressListCtrl::GetItemProgress(int nItem, int nSubItem)
{
	CString strbuf;
	CProgressBarCtrl* pProgCtrl = NULL;

	// lookup the position of the progress control
	strbuf.Format("%d:%d", nItem, nSubItem);
	m_Progs.Lookup(strbuf, (CObject*&)pProgCtrl);

	if(pProgCtrl)
        return pProgCtrl->GetPos();
    return 0;
}

//////////
// CProgressListCtrl::RepositionProgress
// arguments:	void
// returns:		void
// function:	repositions and resizes all progress controls appropriate
//				to the current window, fitting them into their given subitem.
void CProgressListCtrl::RepositionProgress()
{
	int nItem, nSubItem;
	CRect rt, hrt;
	CString strbuf;
	CProgressBarCtrl* pProgCtrl = NULL;
    GetHeaderCtrl()->GetClientRect(hrt);

	// iterate through each progress control
	POSITION pos = m_Progs.GetStartPosition();
	while (pos != NULL) {

		// look at the progress control and move it
		m_Progs.GetNextAssoc(pos, strbuf, (CObject*&)pProgCtrl);
		sscanf(strbuf.GetBuffer(0), "%d:%d", &nItem, &nSubItem);
		GetSubItemRect(nItem, nSubItem, LVIR_BOUNDS, rt);
		rt.top ++; rt.left +=2;
		rt.bottom --; rt.right --;

		// if it's over the header, move it to where it can't be seen
		if(rt.top < hrt.bottom) {
			rt.top = -10;
			rt.bottom = 0;
		}
		pProgCtrl->MoveWindow(rt, false);
	}
}

//////////
// CProgressListCtrl::SwapItems
// arguments:	nItem1: index of the first item to swap
//				nItem2: index of the second item to swap
// returns:		void
// function:	swaps all relevant information of the two given items. this
//				includes text and progress controls of subitems and 
//				item data
void CProgressListCtrl::SwapItems(int nItem1, int nItem2)
{
	int nCols = GetHeaderCtrl()->GetItemCount();
	CProgressBarCtrl* pProgCtrl1;
	CProgressBarCtrl* pProgCtrl2;
	CString StrTxt1, StrTxt2;
	DWORD dwData1, dwData2;
	int nSubItem;

	// check item indicies
	if(nItem1 >= GetItemCount() || nItem2 >= GetItemCount()) {
		return;
	}

	// swap color data
	bool bOk1 = false, bOk2 = false;
	COLORREF tempclr1, tempclr2, emptyclr = RGB(0, 0, 0);
	if(nItem1 < m_ItemColors.GetSize()) {
		tempclr1 = m_ItemColors.GetAt(nItem1);
		bOk1 = true;
	}
	if(nItem2 < m_ItemColors.GetSize()) {
		tempclr2 = m_ItemColors.GetAt(nItem2);
		bOk2 = true;
	}
	if(bOk1) {
		m_ItemColors.SetAtGrow(nItem2, tempclr1);
	} else {
		m_ItemColors.SetAtGrow(nItem2, emptyclr);
	}
	if(bOk2) {
		m_ItemColors.SetAtGrow(nItem1, tempclr2);
	} else {
		m_ItemColors.SetAtGrow(nItem1, emptyclr);
	}

	// swap indices
	dwData1 = GetItemData(nItem1);
	dwData2 = GetItemData(nItem2);
	SetItemData(nItem1, dwData2);
	SetItemData(nItem2, dwData1);
	for(nSubItem = 0; nSubItem < nCols; nSubItem ++) {

		// swap text
		StrTxt1 = GetItemText(nItem1, nSubItem);
		StrTxt2 = GetItemText(nItem2, nSubItem);
		SetItemText(nItem1, nSubItem, StrTxt2);
		SetItemText(nItem2, nSubItem, StrTxt1);

		// swap progress control if found
		StrTxt1.Format("%d:%d", nItem1, nSubItem);
		StrTxt2.Format("%d:%d", nItem2, nSubItem);
		pProgCtrl1 = NULL;
		pProgCtrl2 = NULL;
		m_Progs.Lookup(StrTxt1, (CObject*&)pProgCtrl1);
		m_Progs.Lookup(StrTxt2, (CObject*&)pProgCtrl2);
		if(pProgCtrl1) {
			m_Progs.RemoveKey(StrTxt2);
			m_Progs.SetAt(StrTxt2, (CObject*&)pProgCtrl1);
		}
		if(pProgCtrl2) {
			m_Progs.RemoveKey(StrTxt1);
			m_Progs.SetAt(StrTxt1, (CObject*&)pProgCtrl2);
		}
	}
}

//////////
// CProgressListCtrl::GetItemTextOrPos
// arguments:	nItem: item position
//				nSubItem: subitem position
// returns:		the string at the given location or the string representation
//				of the progress control there
// function:	if there is no progress control at the given location, gets the
//				text there, otherwise, formats the position of the progress control
//				to a string and gets that.
CString CProgressListCtrl::GetItemTextOrPos(int nItem, int nSubItem)
{
	CString strRet;
	CProgressBarCtrl* pProgCtrl = NULL;
	strRet.Format("%d:%d", nItem, nSubItem);
	m_Progs.Lookup(strRet, (CObject*&)pProgCtrl);
	if(pProgCtrl) strRet.Format("%10.5f", pProgCtrl->GetPos());
	else {
		strRet = GetItemText(nItem, nSubItem);
		int typ = m_ColType.GetAt(nSubItem);
		if(typ == SORT_NUMERIC) {
			strRet.Format("%10.5f",atof(strRet));
			CString msg;
			msg.Format("value for column  %u is %s\n",nSubItem,strRet);			
			TRACE(TEXT(msg));
		}
	}
	return strRet;
}

//////////
// CProgressListCtrl::QSort
// arguments:	lo: the low index of sorting
//				hi: the high index of sorting
//				nSubItem: subitem to sort by
//				nOrder: the order to sort by, either SORT_ASCEND or SORT_DESCEND
// returns:		void
// function:	sorts items between lo and hi by the given subitem into the given
//				order using quicksort.
void CProgressListCtrl::QSort(int lo, int hi, int nSubItem, int nOrder)
{
	int i = lo, j = hi;
	CString x = GetItemTextOrPos((lo+hi)/2, nSubItem);
	while(i <= j) {
		if(nOrder == SORT_ASCEND) {
			while(strcmp(GetItemTextOrPos(i, nSubItem), x) < 0) i ++;
			while(strcmp(GetItemTextOrPos(j, nSubItem), x) > 0) j --;
		} else {
			while(strcmp(GetItemTextOrPos(i, nSubItem), x) > 0) i ++;
			while(strcmp(GetItemTextOrPos(j, nSubItem), x) < 0) j --;
		}
		if(i <= j) {
			SwapItems(i, j);
			i++;
			j--;
		}
	}
	if(lo < j) QSort(lo, j, nSubItem, nOrder);
	if(i < hi) QSort(i, hi, nSubItem, nOrder);	
}

//////////
// CProgressListCtrl::Sort
// arguments:	nSubItem: subitem to sort by
//				nOrder: the order to sort by, either SORT_ASCEND or SORT_DESCEND
// returns:		void
// function:	sorts items by the given subitem into the given order by calling QSort
void CProgressListCtrl::Sort(int nSubItem, int nOrder)
{
	QSort(0, GetItemCount() - 1, nSubItem, nOrder);
	RepositionProgress();
}

//////////
// CProgressListCtrl::SwapColumnVisibility
// arguments:	nCol: the column whose visibility to swap
// returns:		void
// function:	if the given column is visible, makes it invisible, otherwise
//				makes it visible.
void CProgressListCtrl::SwapColumnVisibility(int nCol)
{
	int nOldWidth;
	CHeaderCtrl* header = GetHeaderCtrl();
	if(header && nCol < header->GetItemCount()) {
		nOldWidth = m_ColWidths.GetAt(nCol);
		if(nOldWidth < 0) {
			CListCtrl::SetColumnWidth(nCol, -1 * (nOldWidth - 1));
			m_ColWidths.SetAtGrow(nCol, -1 * (nOldWidth - 1));
			if(m_PopupMenu.GetSafeHmenu()) m_PopupMenu.CheckMenuItem(nCol, MF_CHECKED);
		} else {
			CListCtrl::SetColumnWidth(nCol, 0);
			m_ColWidths.SetAtGrow(nCol, -1 * (nOldWidth + 1));
			if(m_PopupMenu.GetSafeHmenu()) m_PopupMenu.CheckMenuItem(nCol, MF_UNCHECKED);
		}
	}
}

//////////
// CProgressListCtrl::SetItemColor
// arguments:	nItem: item whose color is to be set
//				crNew: the new color
// returns:		void
// function:	causes an item to be displayed in the given color
void CProgressListCtrl::SetItemColor(int nItem, COLORREF newclr)
{
	m_ItemColors.SetAtGrow(nItem, newclr);

	CProgressBarCtrl *pProgCtrl;
	CString strBuf;
	// find progress controls and set their color
	for(int si = 0; si < GetHeaderCtrl()->GetItemCount(); si ++) {
		strBuf.Format("%d:%d", nItem, si);
		pProgCtrl = NULL;
		m_Progs.Lookup(strBuf, (CObject*&)pProgCtrl);
		if(pProgCtrl) {
			pProgCtrl->SetTextColor(newclr);
		}
	}
}

//////////
// CProgressListCtrl::SetProjectURL
// arguments:	nItem: the item to set the url for
//				szUrl: the url for the link
// returns:		void
// function:	sets the project's master url
void CProgressListCtrl::SetProjectURL(int nItem, char* szUrl)
{
	CString StrUrl;
	StrUrl.Format("%s", szUrl);
	m_ProjectURLs.SetAtGrow(nItem, StrUrl);
}

//////////
// CProgressListCtrl::GetProjectURL
// arguments:	nItem: the item to set the url for
// returns:		CString of project URL
// function:	gets the master url for a project's link
CString CProgressListCtrl::GetProjectURL(int nItem)
{
	return m_ProjectURLs.GetAt(nItem);
}

//////////
// CProgressListCtrl::GetTextRect
// arguments:	nItem: item to get the rect of
//				nSubItem: subitem to get the rect of
//				lpRect: pointer to rect to fill with result
// returns:		void
// function:	calculates the rect of the text for the given
//				item and subitem
void CProgressListCtrl::GetTextRect(int nItem, int nSubItem, LPRECT lpRect)
{
	CRect hrt, vrt;
	int left, top, right, bottom;
	GetItemRect(nItem, vrt, LVIR_BOUNDS);
	GetHeaderCtrl()->GetItemRect(nSubItem, hrt);
	left = hrt.left + 3;
	top = vrt.top + 1;
	right = hrt.left + GetStringWidth(GetItemText(nItem, nSubItem)) + 5;
	if(hrt.right < right) right = hrt.right - 5;
	bottom = vrt.bottom - 1;
	if(right < left) right = left;
	if(bottom < top) bottom = top;
	lpRect->left = left;
	lpRect->top = top;
	lpRect->right = right;
	lpRect->bottom = bottom;
}

//////////
// CProgressListCtrl::SetMenuItems
// arguments:	szTitles: an array of strings which contains the column titles
//				nLength: the length of the array
// returns:		void
// function:	sets up the menu for hiding and showing columns
void CProgressListCtrl::SetMenuItems(char** szTitles, int nLength)
{
	if(m_PopupMenu.GetSafeHmenu()) {
		m_PopupMenu.DestroyMenu();
	}
	m_PopupMenu.CreatePopupMenu();
	for(int i = 0; i < nLength; i ++) {
		if(szTitles[i]) {
			m_PopupMenu.AppendMenu(MF_STRING, i, szTitles[i]);
			m_PopupMenu.CheckMenuItem(i, MF_CHECKED);
		}
	}
}

//////////
// CProgressListCtrl::SaveInactive
// arguments:	szFile: name of file to save to
//				szSection: section of file to save to
// returns:		void
// function:	saves inactive elements of list to a file.
void CProgressListCtrl::SaveInactive(char* szFile, char* szSection)
{
	CString strSection, strKey, strValue;
	int nMax = 0;
	for(int i = 0; i < GetItemCount(); i ++) {
		if(GetItemData(i) != NULL) continue;
		strSection.Format("%s-%d", szSection, nMax);
		for(int si = 0; si < GetHeaderCtrl()->GetItemCount(); si ++) {
			GetColumnTitle(si, strKey);
			strValue = GetItemText(i, si);
			WritePrivateProfileString(strSection, strKey, strValue, szFile);
		}
		strValue = GetProjectURL(i);
		WritePrivateProfileString(strSection, "proj_url", strValue, szFile);
		nMax ++;
	}

	strValue.Format("%d", nMax);
	WritePrivateProfileString(szSection, "max", strValue, szFile);
}

//////////
// CProgressListCtrl::LoadInactive
// arguments:	szFile: name of file to load from
//				szSection: section of file to load from
// returns:		void
// function:	loads inactive elements of list from a file.
void CProgressListCtrl::LoadInactive(char* szFile, char* szSection)
{
	CString strSection, strKey;
	char szValue[512];
	int nMax = GetPrivateProfileInt(szSection, "max", 0, szFile);

	for(int i = 0; i < nMax; i ++) {
		strSection.Format("%s-%d", szSection, i);
		GetColumnTitle(0, strKey);
		GetPrivateProfileString(strSection, strKey, "", szValue, 512, szFile);
		InsertItem(GetItemCount(), szValue);
		for(int si = 1; si < GetHeaderCtrl()->GetItemCount(); si ++) {
			GetColumnTitle(si, strKey);
			GetPrivateProfileString(strSection, strKey, "", szValue, 512, szFile);
			SetItemText(GetItemCount() - 1, si, szValue);
		}
		GetPrivateProfileString(strSection, "proj_url", "", szValue, 512, szFile);
		SetProjectURL(i, szValue);
	}
}

//////////
// CProgressListCtrl::OnCreate
// arguments:	lpcs: a pointer to the create structure
// returns:		0 if successful, otherwise -1
// function:	sets up the context menu and subclasses the header.
int CProgressListCtrl::OnCreate(LPCREATESTRUCT lpcs)
{
    if(CListCtrl::OnCreate(lpcs) == -1) {
		return -1;
	}

	// subclass header
	CHeaderCtrl* pHeader = GetHeaderCtrl();
	if(pHeader) {
		HWND hWnd = pHeader->GetSafeHwnd();
		if(hWnd) {
			m_Header.SubclassWindow(hWnd);
		}
	}

	m_OldFont = NULL;
	m_nSort = 0;
    return 0;
}

//////////
// CProgressListCtrl::OnCustomDraw
// arguments:	pNMHDR: pointer to notification message header
//				pResult: pointer to result
// returns:		void
// function:	handles special cases of drawing text, including changing
//				an item's color or displaying project names as links
void CProgressListCtrl::OnCustomDraw(NMHDR* pNMHDR, LRESULT* pResult)
{
	NMLVCUSTOMDRAW* pLVCD = (NMLVCUSTOMDRAW*)pNMHDR;

    *pResult = CDRF_DODEFAULT;
    if(pLVCD->nmcd.dwDrawStage == CDDS_PREPAINT) {

		// before painting, get notifications for each item
		*pResult = CDRF_NOTIFYITEMDRAW;
    } else if(pLVCD->nmcd.dwDrawStage == CDDS_ITEMPREPAINT) {

		// before item, get notifications for each subitem
        *pResult = CDRF_NOTIFYSUBITEMDRAW;
	} else if(pLVCD->nmcd.dwDrawStage == (CDDS_ITEMPREPAINT | CDDS_SUBITEM)) {

		// before subitem, set font
		pLVCD->clrText = RGB(0, 0, 0);
		CDC* cdc = CDC::FromHandle(pLVCD->nmcd.hdc);
		CFont* curFont = cdc->GetCurrentFont();
		LOGFONT lf;
		curFont->GetLogFont(&lf);
		if((int)pLVCD->nmcd.dwItemSpec < m_ItemColors.GetSize()) {
			pLVCD->clrText = m_ItemColors.GetAt(pLVCD->nmcd.dwItemSpec);
		}
		CFont* pNewFont = new CFont;
		pNewFont->CreateFontIndirect(&lf);
		m_OldFont = cdc->SelectObject(pNewFont);
		*pResult = CDRF_NOTIFYPOSTPAINT;
	} else if(pLVCD->nmcd.dwDrawStage == (CDDS_ITEMPOSTPAINT | CDDS_SUBITEM)) {

		// after subitem, restore font
		CDC* pDC = CDC::FromHandle(pLVCD->nmcd.hdc);
		m_OldFont = pDC->SelectObject(m_OldFont);
		m_OldFont->DeleteObject();
		delete m_OldFont;
		m_OldFont = NULL;
	    *pResult = CDRF_DODEFAULT;
	}
}

//////////
// CProgressListCtrl::OnDestroy
// arguments:	void
// returns:		void
// function:	deletes progress controls
void CProgressListCtrl::OnDestroy()
{
	CString sKey;
	CProgressBarCtrl* pProgCtrl = NULL;

	// iterate through each progress control
	POSITION pos = m_Progs.GetStartPosition();
	while (pos != NULL) {

		// remove the control and delete it
		m_Progs.GetNextAssoc(pos, sKey, (CObject*&)pProgCtrl);
		m_Progs.RemoveKey(sKey);
		delete pProgCtrl;
	}
}

//////////
// CProgressListCtrl::OnLButtonDown
// arguments:	nFlags: message flags (keys down)
//				point: mouse's point
// returns:		void
// function:	stops control from highlighting items, opens links

// NOTE: Removed this function to allow highlighting; unsure if this
//       will produce unwanted side-effects. -JBK

/*
void CProgressListCtrl::OnLButtonDown(UINT nFlags, CPoint point)
{
}
*/

//////////
// CProgressListCtrl::OnRButtonDown
// arguments:	nFlags: message flags (keys down)
//				point: mouse's point
// returns:		void
// function:	if user clicks on a header, show its context menu, otherwise
//				do nothing.
void CProgressListCtrl::OnRButtonDown(UINT nFlags, CPoint point)
{
	ClientToScreen(&point);

	// see if user clicked on header and do its popup menu
	CHeaderCtrl* header = GetHeaderCtrl();
	if(header) {
		CRect rt;
		header->GetWindowRect(&rt);
		if(rt.PtInRect(point)) {
			if(m_PopupMenu.GetSafeHmenu()) m_PopupMenu.TrackPopupMenu(TPM_LEFTALIGN|TPM_RIGHTBUTTON, point.x, point.y, this);
			return;
		}
	}

	CWnd* pParentWnd = GetParent();
	if(pParentWnd) {
		MapWindowPoints(pParentWnd, &point, 1);
		WPARAM wParam = nFlags;
		LPARAM lParam = MAKELPARAM(point.x, point.y);
		pParentWnd->SendMessage(WM_RBUTTONDOWN, wParam, lParam);
	}
	//CListCtrl::OnRButtonDown(nFlags, point);
}

//////////
// CProgressListCtrl::OnNotify
// arguments:	wParam: notification's wparam
//				lParam: notification's lparam
//				pResult: pointer to result of notification
// returns:		true if the notification is processed, otherwise false
// function:	handles notifications from children, including:
//				user clicking a header sorts by that column.
//				user double clicking a header resizes it to longest string in that column.
//				user tracking a hidden column does not resize it.
BOOL CProgressListCtrl::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult)
{
	HD_NOTIFY* phdn = (HD_NOTIFY*)lParam;

	// notification from header, user has clicked a header
	if(phdn->hdr.code == HDN_ITEMCLICKA || phdn->hdr.code == HDN_ITEMCLICKW) {
		int newSort = phdn->iItem + 1;

		// if this header was clicked before, alternate sorts, other wise just sort
		if(newSort == abs(m_nSort)) {
			m_nSort *= -1;
			if(m_nSort < 0) Sort(abs(m_nSort)-1, SORT_DESCEND);
			else Sort(abs(m_nSort)-1, SORT_ASCEND);
		} else {
			m_nSort = newSort;
			Sort(abs(m_nSort)-1, SORT_ASCEND);
		}
	}

	// notification from header, user has double clicked a column divider
	if(phdn->hdr.code == HDN_DIVIDERDBLCLICKA || phdn->hdr.code == HDN_DIVIDERDBLCLICKW) {

		// don't resize if it's hidden
		if(m_ColWidths.GetAt(phdn->iItem) < 0) {
			*pResult = TRUE;
			return TRUE;
		}

		// find longest string and resize to its length
		CString strTitle;
		GetColumnTitle(phdn->iItem, strTitle);
		int nMax = GetStringWidth(strTitle) + 12;
		for(int i = 0; i < GetItemCount(); i ++) {
			CString strBuf;
			strBuf = GetItemText(i, phdn->iItem);
			int nWidth = GetStringWidth(strBuf) + 12;
			if(nWidth > nMax) nMax = nWidth;
		}
		SetColumnWidth(phdn->iItem, nMax);

		*pResult = TRUE;
		return TRUE;
	}

	// notification from header, user has started tracking a header
	if(phdn->hdr.code == HDN_BEGINTRACKA || phdn->hdr.code == HDN_BEGINTRACKW) {

		// stop the header from tracking
		int nCol = phdn->iItem;
		if(m_ColWidths.GetAt(nCol) < 0) {
			*pResult = TRUE;
			return TRUE;
		}
	}
	return CListCtrl::OnNotify(wParam, lParam, pResult);
}

//////////
// CProgressListCtrl::OnPaint
// arguments:	void
// returns:		void
// function:	repositions progress bars.
void CProgressListCtrl::OnPaint()
{
	RepositionProgress();
	CListCtrl::OnPaint();

	// iterate through each progress control
	CProgressBarCtrl* progCtrl = NULL;
	CString str;
	POSITION pos = m_Progs.GetStartPosition();
	while (pos != NULL) {

		// redraw control
		m_Progs.GetNextAssoc(pos, str, (CObject*&)progCtrl);
		progCtrl->RedrawWindow(NULL, NULL, RDW_INVALIDATE|RDW_UPDATENOW|RDW_NOERASE);
	}
}

//////////
// CProgressListCtrl::OnCommand
// arguments:	wParam: command's wparam
//				lParam: command's lparam
// returns:		true if the command is processed, otherwise false
// function:	assumes this command is from the context menu and the wparam
//				is the column number to swap visibility of.
BOOL CProgressListCtrl::OnCommand(WPARAM wParam, LPARAM lParam)
{
	SwapColumnVisibility(wParam);
	return CListCtrl::OnCommand(wParam, lParam);
}

const char *BOINC_RCSID_577b1dbbc2 = "$Id$";
