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
// function:	void
CProgressBarCtrl::CProgressBarCtrl()
{
}

//////////
// CProgressBarCtrl::OnPaint
// arguments:	void
// returns:		void
// function:	writes the progress in text
void CProgressBarCtrl::OnPaint()
{
	CProgressCtrl::OnPaint();

	CString strProg;
	strProg.Format("%d%%", this->GetPos());

	CClientDC cdc(this);
	CRect rt;
	GetClientRect(&rt);
	rt.top -= 2; rt.right += 2;
	CFont* pOldFont = NULL;
	int nOldMode;
	//COLORREF crOldColor;
	pOldFont = cdc.SelectObject(GetParent()->GetFont());
	//crOldColor = cdc.SetTextColor(RGB(0, 0, 64));
	nOldMode = cdc.SetBkMode(TRANSPARENT);
	cdc.DrawText(strProg, &rt, DT_CENTER|DT_VCENTER);
	cdc.SelectObject(pOldFont);
	//cdc.SetTextColor(crOldColor);
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
	ON_WM_LBUTTONDOWN()
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
// returns:		index of new column if successful,otherwise -1
// function:	adds a new column to the list control
int CProgressListCtrl::InsertColumn(int nCol, LPCTSTR lpszColumnHeading, int nFormat = LVCFMT_LEFT, int nWidth = -1, int nSubItem = -1)
{
	m_ColWidths.SetAtGrow(nCol, nWidth);
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
	CString empty;
	m_ProjectURLs.RemoveAt(nItem);

	CString strbuf;
	CProgressCtrl* pProgCtrl = NULL;

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
//				nProg: position to set progress control
// returns:		void
// function:	sets the position of a progress control for a given
//				item and subitem; if there is none there, creates a new 
//				one, otherwise sets the progress of the one it finds.
void CProgressListCtrl::SetItemProgress(int nItem, int nSubItem, int nProg)
{
	CRect rt;
	CString strbuf;
	CProgressCtrl* pProgCtrl = NULL;
	if(nProg < 0) nProg = 0;
	if(nProg > 100) nProg = 100;

	// lookup the position of the progress control
	strbuf.Format("%d:%d", nItem, nSubItem);
	m_Progs.Lookup(strbuf, (CObject*&)pProgCtrl);
	if(pProgCtrl) {

		// found, so just update it's progress
		pProgCtrl->SetPos(nProg);
	} else {

		// not found, create one and put it in the map
		GetSubItemRect(nItem, nSubItem, LVIR_BOUNDS, rt);
		pProgCtrl = new CProgressBarCtrl();
		pProgCtrl->Create(PBS_SMOOTH|WS_CHILD|WS_VISIBLE, rt, this, 0);
		pProgCtrl->SetPos(nProg);
		m_Progs.SetAt(strbuf, pProgCtrl);
	}
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
	CProgressCtrl* pProgCtrl = NULL;
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
	CProgressCtrl* pProgCtrl1;
	CProgressCtrl* pProgCtrl2;
	CString StrTxt1, StrTxt2;
	DWORD dwData1, dwData2;
	int nSubItem;

	// check item indicies
	if(nItem1 >= GetItemCount() || nItem2 >= GetItemCount()) {
		return;
	}

	// swap url data
	bool bOk1 = false, bOk2 = false;
	CString StrTemp1, StrTemp2, StrEmpty;
	if(nItem1 < m_ProjectURLs.GetSize()) {
		StrTemp1 = m_ProjectURLs.GetAt(nItem1);
		bOk1 = true;
	}
	if(nItem2 < m_ProjectURLs.GetSize()) {
		StrTemp2 = m_ProjectURLs.GetAt(nItem2);
		bOk2 = true;
	}
	if(bOk1) {
		m_ProjectURLs.SetAtGrow(nItem2, StrTemp1);
	} else {
		m_ProjectURLs.SetAtGrow(nItem2, StrEmpty);
	}
	if(bOk2) {
		m_ProjectURLs.SetAtGrow(nItem1, StrTemp2);
	} else {
		m_ProjectURLs.SetAtGrow(nItem1, StrEmpty);
	}

	// swap color data
	bOk1 = false;
	bOk2 = false;
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
// CProgressListCtrl::Sort
// arguments:	nSubItem: subitem to sort by
//				nOrder: the order to sort by, either SORT_ASCEND or SORT_DESCEND
// returns:		void
// function:	sorts items by the given subitem into the given order. if there
//				is a progress control, converts the position to a string for 
//				comparison, otherwise sorts by the string at that subitem.
void CProgressListCtrl::Sort(int nSubItem, int nOrder)
{
	int i, j, min, z;
	CString Stri, Strj;
	CProgressCtrl* pProgCtrli = NULL;
	CProgressCtrl* pProgCtrlj = NULL;

	// check subitem is in bounds
	if(nSubItem >= GetHeaderCtrl()->GetItemCount()) {
		return;
	}

	// run selection sort for now
	int items = GetItemCount();
	for(z = 0; z < GetItemCount(); z ++) {
		for(i = 0; i < items-1; i ++) {
			min = i;
			for(j = i+1; j < items; j ++) {

				// see if there is a progress control here, and set its
				// progress as the comparison string, otherwise,
				// just get the text
				Stri.Format("%d:%d", i, nSubItem);
				Strj.Format("%d:%d", j, nSubItem);
				pProgCtrli = NULL;
				pProgCtrlj = NULL;
				m_Progs.Lookup(Stri, (CObject*&)pProgCtrli);
				m_Progs.Lookup(Strj, (CObject*&)pProgCtrlj);
				if(pProgCtrli) {
					Stri.Format("%0.3d", pProgCtrli->GetPos());
				} else {
					Stri = GetItemText(i, nSubItem);
				}
				if(pProgCtrlj) {
					Strj.Format("%0.3d", pProgCtrlj->GetPos());
				} else {
					Strj = GetItemText(j, nSubItem);
				}
				if(nOrder == SORT_ASCEND && strcmp(Stri, Strj) > 0) min = j;
				if(nOrder == SORT_DESCEND && strcmp(Stri, Strj) < 0) min = j;
			}
			SwapItems(i, min);
		}
	}
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
}

//////////
// CProgressListCtrl::SetProjectURL
// arguments:	nItem: the item to set the url for
//				szUrl: the url for the link
// returns:		void
// function:	sets the url for a project's link, causing the text of
//				the first subitem for the given item to be displayed
//				as a link
void CProgressListCtrl::SetProjectURL(int nItem, char* szUrl)
{
	CString StrUrl;
	StrUrl.Format("%s", szUrl);
	m_ProjectURLs.SetAtGrow(nItem, StrUrl);
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
		if(pLVCD->nmcd.dwItemSpec < m_ItemColors.GetSize()) {
			pLVCD->clrText = m_ItemColors.GetAt(pLVCD->nmcd.dwItemSpec);
		}
		if(pLVCD->nmcd.dwItemSpec < m_ProjectURLs.GetSize() && !m_ProjectURLs.GetAt(pLVCD->nmcd.dwItemSpec).IsEmpty()) {
			if(pLVCD->iSubItem == 0) {
				lf.lfUnderline = true;
				pLVCD->clrText = RGB(0, 0, 255);
			}
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
	CProgressCtrl* pProgCtrl = NULL;

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
// CProgressListCtrl::OnSetCursor
// arguments:	pWnd: window containing the cursor
//				nHitTest: hit test area code
//				message: mouse message number
// returns:		true if the message should not be processed further, false otherwise
// function:	checks if the cursor is over a link, if so,
//				changes it to the hand
BOOL CProgressListCtrl::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	CPoint point;
	GetCursorPos(&point);
	ScreenToClient(&point);
	CRect hit;
	for(int i = 0; i < GetItemCount(); i ++) {
		if(i < m_ProjectURLs.GetSize() && !m_ProjectURLs.GetAt(i).IsEmpty()) {
			GetTextRect(i, 0, &hit);
			if(hit.PtInRect(point)) {
				HCURSOR hand = LoadCursor(NULL, IDC_HAND);
				if(hand) {
					SetCursor(hand);
					return TRUE;
				}
			}
		}
	}
	return CWnd::OnSetCursor(pWnd, nHitTest, message);
}

//////////
// CProgressListCtrl::OnLButtonDown
// arguments:	nFlags: message flags (keys down)
//				point: mouse's point
// returns:		void
// function:	stops control from highlighting items, opens links
void CProgressListCtrl::OnLButtonDown(UINT nFlags, CPoint point)
{
	CRect hit;
	for(int i = 0; i < GetItemCount(); i ++) {
		if(i < m_ProjectURLs.GetSize() && !m_ProjectURLs.GetAt(i).IsEmpty()) {
			GetTextRect(i, 0, &hit);
			if(hit.PtInRect(point)) {
				ShellExecute(GetSafeHwnd(), "open", m_ProjectURLs.GetAt(i).GetBuffer(0), "", "", SW_SHOWNORMAL);
			}
		}
	}
}

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
//				user double clicking a header does not resize it.
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
		char szTitle[256];
		int nMax = 0;
		LVCOLUMN lvcol;
		ZeroMemory(&lvcol, sizeof(LVCOLUMN));
		lvcol.mask = LVCF_TEXT;
		lvcol.pszText = szTitle;
		lvcol.cchTextMax = 256;
		GetColumn(phdn->iItem, &lvcol);
		nMax = GetStringWidth(szTitle) + 12;
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
	CProgressCtrl* progCtrl = NULL;
	CString str;
	POSITION pos = m_Progs.GetStartPosition();
	while (pos != NULL) {

		// remove the control and delete it
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
