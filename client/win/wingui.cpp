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

// include header

#include "wingui.h"

// globals

CMainWindow* g_myWnd = NULL;
CMyApp g_myApp;
char* g_szColumnTitles[MAX_LIST_ID][MAX_COLS] = {
        {"Project",	"Account",		"Total Credit",	"Avg. Credit",	"Resource Share",	NULL,				NULL},
        {"Project",	"Application",	"Name",			"CPU time",		"Progress",			"To Completion",	"Status"},
        {"Project",	"File",			"Progress",		"Size",			"Time",				"Direction",		NULL},
        {"Project",	"Time",			"Message",		NULL,			NULL,				NULL,				NULL}
};

void show_message(char* message, char* priority) {
	if(g_myWnd) {
		g_myWnd->MessageUser("BOINC", message, priority);
	}
}

void show_project_message(PROJECT* project, char* message, char* priority) {
	if(g_myWnd) {
		if(strcmp(project->project_name, "")) {
			g_myWnd->MessageUser(project->project_name, message, priority);
		} else {
			g_myWnd->MessageUser(project->master_url, message, priority);
		}
	}
}

int get_initial_project() {
	return 0;
}

void GetByteString(double nbytes, CString* str) {
	double xTera = (1024.0*1024*1024*1024);
	double xGiga = (1024.0*1024*1024);
	double xMega = (1024.0*1024);
	double xKilo = (1024.0);
    if (nbytes >= xTera) {
        str->Format("%0.2f TB", nbytes/xTera);
    } else if (nbytes >= xGiga) {
        str->Format("%0.2f GB", nbytes/xGiga);
    } else if (nbytes >= xMega) {
        str->Format("%0.2f MB", nbytes/xMega);
    } else if (nbytes >= xKilo) {
        str->Format("%0.2f KB", nbytes/xKilo);
    } else {
        str->Format("%0.0f bytes", nbytes);
    }
}

/////////////////////////////////////////////////////////////////////////
// CProgressHeaderCtrl message map and member functions

BEGIN_MESSAGE_MAP(CProgressBarCtrl, CProgressCtrl)
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
	m_PopupMenu.DestroyMenu();
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
		m_PopupMenu.CheckMenuItem(nCol, MF_UNCHECKED);
		m_ColWidths.SetAtGrow(nCol, cx);
		return CListCtrl::SetColumnWidth(nCol, 0);
	} else {
		m_PopupMenu.CheckMenuItem(nCol, MF_CHECKED);
		m_ColWidths.SetAtGrow(nCol, cx);
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
		rt.top ++; rt.left ++;
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
			m_PopupMenu.CheckMenuItem(nCol, MF_CHECKED);
		} else {
			CListCtrl::SetColumnWidth(nCol, 0);
			m_ColWidths.SetAtGrow(nCol, -1 * (nOldWidth + 1));
			m_PopupMenu.CheckMenuItem(nCol, MF_UNCHECKED);
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
// CProgressListCtrl::OnCreate
// arguments:	lpcs: a pointer to the create structure
// returns:		0 if successful, otherwise -1
// function:	sets up the context menu and subclasses the header.
int CProgressListCtrl::OnCreate(LPCREATESTRUCT lpcs)
{
    if(CListCtrl::OnCreate(lpcs) == -1) {
		return -1;
	}

	// load popup menu
	m_PopupMenu.CreatePopupMenu();
	for(int i = 0; i < MAX_COLS; i ++) {
		if(g_szColumnTitles[GetDlgCtrlID()][i]) {
			m_PopupMenu.AppendMenu(MF_STRING, i, g_szColumnTitles[GetDlgCtrlID()][i]);
			m_PopupMenu.CheckMenuItem(i, MF_CHECKED);
		}
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
			m_PopupMenu.TrackPopupMenu(TPM_LEFTALIGN|TPM_RIGHTBUTTON, point.x, point.y, this);
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

		// stop the column from resizing
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


/////////////////////////////////////////////////////////////////////////
// CPieChartCtrl member functions

BEGIN_MESSAGE_MAP(CPieChartCtrl, CWnd)
    ON_WM_PAINT()
END_MESSAGE_MAP()

//////////
// CPieChartCtrl::CPieChartCtrl
// arguments:	void
// returns:		void
// function:	initializes members
CPieChartCtrl::CPieChartCtrl()
{
	m_xTotal = 0;
	m_pFont = NULL;
}

//////////
// CPieChartCtrl::AddPiece
// arguments:	szLabel: label for the piece
//				clr: color of the piece
//				xValue: the initial value for the piece
// returns:		void
// function:	adds a piece to the pie
void CPieChartCtrl::AddPiece(LPTSTR szLabel, COLORREF clr, double xValue)
{
	if(xValue < 0) xValue = 0;
	m_xValues.Add(xValue);
	m_colors.Add(clr);
	CString strLabel;
	strLabel.Format("%s", szLabel);
	m_strLabels.Add(strLabel);
}

//////////
// CPieChartCtrl::SetPiece
// arguments:	nIndex: index of piece to change
//				xValue: the new value for the piece
// returns:		void
// function:	changes the piece's value
void CPieChartCtrl::SetPiece(int nIndex, double xValue)
{
	if(nIndex < 0 || nIndex >= m_xValues.GetSize()) return;
	if(xValue < 0) xValue = 0;
	m_xValues.SetAt(nIndex, xValue);
}

//////////
// CPieChartCtrl::Create
// arguments:	dwStyle: the style of control to create
//				rect: size and position
//				pParentWnd: control's parent window
//				nID: control's id
// returns:		true if successful, otherwise false
// function:	creates this control
BOOL CPieChartCtrl::Create(DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID)
{
    CString strWndClass = AfxRegisterWndClass (0, NULL, (HBRUSH)GetStockObject(WHITE_BRUSH), NULL);
	return CWnd::Create(strWndClass, NULL, dwStyle, rect, pParentWnd, nID);
}

//////////
// CPieChartCtrl::DrawPiece
// arguments:	pDC: pointer to dc to draw in
//				xStartAngle: starting angle of piece
//				xEndAngle: ending angle of piece
// returns:		void
// function:	draws a pie piece in the dc
void CPieChartCtrl::DrawPiePiece(CDC* pDC, double xStartAngle, double xEndAngle)
{
	if(xEndAngle - xStartAngle <= 0.05) return;

	// gdi objects needed
	CRect rt, rt2;
	CRgn rellipsehi, rellipselow, rrect, rdepthcurve, rdepth;
	CPoint pt1, pt2, pt3, pt4;
	CRgn rpie, rangle, rellipse;
	CPoint poly[8];

	// set up coordinates
	GetWindowRect(&rt);
	ScreenToClient(&rt);
	CPoint cp;
	int major = (int)((rt.Width() - 2 * PIE_BUFFER) * 0.5);
	int minor = (int)((rt.Height() - 2 * PIE_BUFFER) * 0.25);
	int maxmajor = pDC->GetDeviceCaps(HORZRES) * PIE_MAJOR_MAX;
	int maxminor = pDC->GetDeviceCaps(VERTRES) * PIE_MINOR_MAX;
	if(major > maxmajor) major = maxmajor;
	if(minor > maxminor) minor = maxminor;

	int depth = minor * PIE_DEPTH;
	cp.x = rt.Width() * 0.5;
	cp.y = rt.Height() - minor - PIE_BUFFER - depth;
	rt.SetRect(cp.x - major, cp.y - minor, cp.x + major, cp.y + minor);
	rt2.SetRect(cp.x - major, cp.y - minor + depth, cp.x + major, cp.y + minor + depth);

	// draw elliptical part of piece

	// set up coordinates
	poly[0].x = cp.x; poly[0].y = cp.y;
	EllipsePoint(&rt, xStartAngle + (xEndAngle - xStartAngle) * 0.00f, &poly[1]);
	CirclePoint(&cp, major * 2, xStartAngle + (xEndAngle - xStartAngle) * 0.00f, &poly[2]);
	CirclePoint(&cp, major * 2, xStartAngle + (xEndAngle - xStartAngle) * 0.25f, &poly[3]);
	CirclePoint(&cp, major * 2, xStartAngle + (xEndAngle - xStartAngle) * 0.50f, &poly[4]);
	CirclePoint(&cp, major * 2, xStartAngle + (xEndAngle - xStartAngle) * 0.75f, &poly[5]);
	CirclePoint(&cp, major * 2, xStartAngle + (xEndAngle - xStartAngle) * 1.00f, &poly[6]);
	EllipsePoint(&rt, xStartAngle + (xEndAngle - xStartAngle) * 1.00f, &poly[7]);
	
	// filled part
	rellipse.CreateEllipticRgnIndirect(&rt);
	rangle.CreatePolygonRgn(poly, 8, ALTERNATE);
	rpie.CreateRectRgnIndirect(&rt);
	rpie.CombineRgn(&rellipse, &rangle, RGN_AND);
	pDC->FillRgn(&rpie, pDC->GetCurrentBrush());

	// outline
	pDC->MoveTo(rt.CenterPoint());
	pDC->LineTo(poly[1]);
	pDC->Arc(&rt, poly[1], poly[7]);
	pDC->MoveTo(poly[7]);
	pDC->LineTo(rt.CenterPoint());

	// clean up
	rellipse.DeleteObject();
	rangle.DeleteObject();
	rpie.DeleteObject();

	// draw depth part of pie piece if needed
	if(xStartAngle >= 180 || xEndAngle >= 180) {

		// set up coordinates
		int xLowerAngle = 180;
		if(xStartAngle > 180) xLowerAngle = xStartAngle;
		int xHigherAngle = 360;
		if(xEndAngle < 360) xHigherAngle = xEndAngle;
		EllipsePoint(&rt, xLowerAngle, &pt1);
		EllipsePoint(&rt, xHigherAngle, &pt2);
		EllipsePoint(&rt2, xLowerAngle, &pt3);
		EllipsePoint(&rt2, xHigherAngle, &pt4);
		if(xStartAngle > 180) {
			pt1.x = poly[1].x;
			pt3.x = poly[1].x;
		}
		if(xEndAngle < 360) {
			pt2.x = poly[7].x;
			pt4.x = poly[7].x;
		}

		// filled part
		rellipsehi.CreateEllipticRgnIndirect(&rt);
		rellipselow.CreateEllipticRgnIndirect(&rt2);
		rrect.CreateRectRgn(pt1.x, rt.top, pt4.x, rt2.bottom);
		rdepthcurve.CreateRectRgnIndirect(&rt2);
		rdepthcurve.CombineRgn(&rellipselow, &rellipsehi, RGN_DIFF);
		rdepth.CreateRectRgnIndirect(&rt2);
		rdepth.CombineRgn(&rdepthcurve, &rrect, RGN_AND);
		pDC->FillRgn(&rdepth, pDC->GetCurrentBrush());
		
		// ouline
		pDC->Arc(&rt, pt1, pt2);
		pDC->Arc(&rt2, pt3, pt4);
		pDC->MoveTo(pt1);
		pDC->LineTo(pt3);
		pDC->MoveTo(pt4);
		pDC->LineTo(pt2);

		// clean up
		rellipsehi.DeleteObject();
		rellipselow.DeleteObject();
		rdepthcurve.DeleteObject();
		rdepth.DeleteObject();
	}
}

//////////
// CPieChartCtrl::CirclePoint
// arguments:	center: center point of circle
//				nRadius: radius of circle
//				xAngle: angle of radius
//				pResult: pointer to CPoint to put result in
// returns:		void
// function:	calculates the point on the circle at the given angle
void CPieChartCtrl::CirclePoint(CPoint* center, int nRadius, double xAngle, CPoint* pResult)
{
	pResult->x = center->x + nRadius * cos(xAngle * (PI / 180));
	pResult->y = center->y - nRadius * sin(xAngle * (PI / 180));
}

//////////
// CPieChartCtrl::EllipsePoint
// arguments:	rt: pointer to rect of ellipse
//				xAngle: angle of radius
//				pResult: pointer to CPoint to put result in
// returns:		void
// function:	calculates the point on the ellipse in the given rect at
//				the given angle
void CPieChartCtrl::EllipsePoint(CRect* rt, double xAngle, CPoint* pResult)
{
	pResult->x = rt->CenterPoint().x + (rt->Width() / 2) * cos(xAngle * (PI / 180));
	pResult->y = rt->CenterPoint().y - (rt->Height() / 2) * sin(xAngle * (PI / 180));
}

//////////
// CPieChartCtrl::SetFont
// arguments:	pFont: pointer to font to set
// returns:		void
// function:	sets this control's font
void CPieChartCtrl::SetFont(CFont* pFont)
{
	m_pFont = pFont;
}

//////////
// CPieChartCtrl::SetTotal
// arguments:	xTotal: the total amount of the pie chart
// returns:		void
// function:	sets the tag for data, which is a string that will be displayed
//				after the numbers in the label
void CPieChartCtrl::SetTotal(double xTotal)
{
	if(xTotal >= 0) m_xTotal = xTotal;
}

//////////
// CPieChartCtrl::OnPaint
// arguments:	void
// returns:		void
// function:	draws the control by drawing the labels and pie pieces for
//				each piece of the pie
void CPieChartCtrl::OnPaint()
{
	CWnd::OnPaint();

	// no pieces, so dont do anything
	if(m_xValues.GetSize() == 0) return;

	// gdi objects needed
	CClientDC cdc(this);
	CRect rt;
	CDC MemDC;
	CBitmap MemBmp;
	CBrush MemBrush;
	CPen MemPen;
	CBitmap* pOldBmp = NULL;
	CBrush* pOldBrush = NULL;
	CPen* pOldPen = NULL;
	CFont* pOldFont = NULL;

	// create offscreen buffer
	GetClientRect(&rt);
	MemDC.CreateCompatibleDC(&cdc);
	MemBmp.CreateCompatibleBitmap(&cdc, rt.Width(), rt.Height());
	MemPen.CreatePen(PS_SOLID, 0, RGB(0, 0, 0));

	// select gdi objects
	pOldBmp = MemDC.SelectObject(&MemBmp);
	pOldPen = MemDC.SelectObject(&MemPen);
	pOldFont = MemDC.SelectObject(m_pFont);
	MemDC.FillSolidRect(&rt, RGB(255, 255, 255));

	// go through each xPercent and draw its label and pie
	double xSoFar = 0;
	CRect wndrect;
	CRect textrect;
	GetWindowRect(&wndrect);
	for(int i = 0; i < m_xValues.GetSize(); i ++) {
		MemBrush.CreateSolidBrush(m_colors.GetAt(i));
		pOldBrush = MemDC.SelectObject(&MemBrush);

		int texti = i;
		if(texti == 2) texti = 3;
		else if(texti == 3) texti = 2;

		// display color box and label
		if(PIE_BUFFER + 20 + texti * 20 < wndrect.Height() / 2) {
			textrect.SetRect(PIE_BUFFER + 0, PIE_BUFFER + texti * 20 + 4, PIE_BUFFER + 11, PIE_BUFFER + 20 + texti * 20 - 4);
			MemDC.FillRect(&textrect, &MemBrush);
			MemDC.MoveTo(textrect.left, textrect.top);
			MemDC.LineTo(textrect.right, textrect.top);
			MemDC.LineTo(textrect.right, textrect.bottom);
			MemDC.LineTo(textrect.left, textrect.bottom);
			MemDC.LineTo(textrect.left, textrect.top);
			textrect.SetRect(PIE_BUFFER + 16, PIE_BUFFER + texti * 20, wndrect.Width() - PIE_BUFFER, PIE_BUFFER + 20 + texti * 20);
			CString strBytes;
			GetByteString(m_xValues.GetAt(i), &strBytes);
			CString strBuf;
			strBuf.Format("%s (%s)", m_strLabels.GetAt(i).GetBuffer(0), strBytes.GetBuffer(0));
			MemDC.DrawText(strBuf, textrect, DT_SINGLELINE|DT_VCENTER|DT_LEFT);
		}

		// display pie piece
		double xPercent = 0;
		if(m_xTotal > 0) xPercent = m_xValues.GetAt(i) / m_xTotal;
		DrawPiePiece(&MemDC, xSoFar * 360, (xSoFar + xPercent) * 360);
		xSoFar += xPercent;

		MemDC.SelectObject(pOldBrush);
		MemBrush.DeleteObject();
	}

	// copy offscreen buffer to screen
	cdc.BitBlt(0, 0, rt.Width(), rt.Height(), &MemDC, 0, 0, SRCCOPY);

	// clean up
	MemDC.SelectObject(pOldBmp);
	MemDC.SelectObject(pOldPen);
	MemDC.SelectObject(pOldFont);
	MemPen.DeleteObject();
	MemBmp.DeleteObject();
	MemBrush.DeleteObject();
	MemDC.DeleteDC();
}

/////////////////////////////////////////////////////////////////////////
// CMyApp member functions

//////////
// CMyApp::InitInstance
// arguments:	void
// returns:		true if initialization is successful, otherwise false
// function:	creates and shows the main window if boinc is not running,
//				otherwise shows the currently running window
BOOL CMyApp::InitInstance()
{
	if(CreateMutex(NULL, false, "BOINC_MUTEX") == 0 || GetLastError() == ERROR_ALREADY_EXISTS) {
		UINT nShowMsg = RegisterWindowMessage("BOINC_SHOW_MESSAGE");
		PostMessage(HWND_BROADCAST, nShowMsg, 0, 0);
		return FALSE;
	}
    m_pMainWnd = new CMainWindow();
	m_pMainWnd->ShowWindow(SW_SHOW);
	m_pMainWnd->UpdateWindow();
    return TRUE;
}

/////////////////////////////////////////////////////////////////////////
// CMainWindow message map and member functions

BEGIN_MESSAGE_MAP(CMainWindow, CWnd)
    ON_WM_CLOSE()
    ON_COMMAND(ID_FILE_SHOWGRAPHICS, OnCommandFileShowGraphics)
    ON_COMMAND(ID_FILE_CLEARINACTIVE, OnCommandFileClearInactive)
    ON_COMMAND(ID_FILE_CLEARMESSAGES, OnCommandFileClearMessages)
    ON_COMMAND(ID_FILE_HIDE, OnCommandHide)
    ON_COMMAND(ID_FILE_SUSPEND, OnCommandSuspend)
    ON_COMMAND(ID_FILE_RESUME, OnCommandResume)
    ON_COMMAND(ID_FILE_EXIT, OnCommandExit)
    ON_COMMAND(ID_SETTINGS_LOGIN, OnCommandSettingsLogin)
    ON_COMMAND(ID_SETTINGS_QUIT, OnCommandSettingsQuit)
    ON_COMMAND(ID_SETTINGS_PROXYSERVER, OnCommandSettingsProxyServer)
    ON_COMMAND(ID_HELP_ABOUT, OnCommandHelpAbout)
    ON_COMMAND(ID_PROJECT_RELOGIN, OnCommandProjectRelogin)
    ON_COMMAND(ID_PROJECT_QUIT, OnCommandProjectQuit)
    ON_COMMAND(ID_STATUSICON_HIDE, OnCommandHide)
    ON_COMMAND(ID_STATUSICON_SUSPEND, OnCommandSuspend)
    ON_COMMAND(ID_STATUSICON_RESUME, OnCommandResume)
    ON_COMMAND(ID_STATUSICON_EXIT, OnCommandExit)
    ON_WM_CREATE()
    ON_WM_RBUTTONDOWN()
    ON_WM_SIZE()
    ON_WM_SETFOCUS()
    ON_MESSAGE(STATUS_ICON_ID, OnStatusIcon)
END_MESSAGE_MAP()

//////////
// CMainWindow::CMainWindow
// arguments:	void
// returns:		void
// function:	registers window class, creates and poisitions window.
CMainWindow::CMainWindow()
{
	// register window class
    CString strWndClass = AfxRegisterWndClass (0, g_myApp.LoadStandardCursor(IDC_ARROW),
        (HBRUSH)(COLOR_3DFACE+1), g_myApp.LoadIcon(IDI_ICON));

	// create and position window
    CreateEx(0, strWndClass, WND_TITLE, WS_OVERLAPPEDWINDOW|WS_EX_OVERLAPPEDWINDOW|WS_CLIPCHILDREN,
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
		NULL, NULL, NULL);
}

//////////
// CMainWindow::TimerProc
// arguments:	hWnd: the window assosciated with the timer
//				uMsg: the WM_TIMER message
//				uID: the timer's id
//				dwTime: milliseconds since system started
// returns:		void
// function:	checks idle time, updates client state, flushed output streams,
//				and updates gui display.
void CALLBACK CMainWindow::TimerProc(HWND hWnd, UINT uMsg, UINT uID, DWORD dwTime)
{
	// update state and gui
    while(gstate.do_something());
	fflush(stdout);
	fflush(stderr);
	if(g_myWnd && !g_myWnd->IsSuspended()) {
		// check user's idle time for suspension of apps
		if (gstate.global_prefs.idle_time_to_run > 0) {
			if (g_myWnd->GetUserIdleTime() > 1000 * gstate.global_prefs.idle_time_to_run) {
				gstate.user_idle = true;
			} else {
				gstate.user_idle = false;
			}
		} else {
			gstate.user_idle = true;
		}

		g_myWnd->UpdateGUI(&gstate);
	}
}

//////////
// CMainWindow::UpdateGUI
// arguments:	pcs: pointer to the client state for the gui to display
// returns:		void
// function:	syncronizes list controls with vectors in client state
//				and displays them.
void CMainWindow::UpdateGUI(CLIENT_STATE* pcs)
{
	CString strBuf;
	int i;

	// display projects
	float totalres = 0;
	Syncronize(&m_ProjectListCtrl, (vector<void*>*)(&pcs->projects));
	for(i = 0; i < pcs->projects.size(); i ++) {
		totalres += pcs->projects[i]->resource_share;
	}
	for(i = 0; i < m_ProjectListCtrl.GetItemCount(); i ++) {
		PROJECT* pr = (PROJECT*)m_ProjectListCtrl.GetItemData(i);
		if(!pr) {
			m_ProjectListCtrl.SetItemColor(i, RGB(128, 128, 128));
			m_ProjectListCtrl.SetProjectURL(i, "");
			m_ProjectListCtrl.SetItemProgress(i, 4, 0);
			continue;
		}

		// project
		if(!strcmp(pr->project_name, "")) {
			m_ProjectListCtrl.SetItemText(i, 0, pr->master_url);
		} else {
			m_ProjectListCtrl.SetItemText(i, 0, pr->project_name);
		}
		m_ProjectListCtrl.SetProjectURL(i, pr->master_url);

		// account
		m_ProjectListCtrl.SetItemText(i, 1, pr->user_name);

		// total credit
		strBuf.Format("%0.2f", pr->user_total_credit);
		m_ProjectListCtrl.SetItemText(i, 2, strBuf);

		// avg credit
		strBuf.Format("%0.2f", pr->user_expavg_credit);
		m_ProjectListCtrl.SetItemText(i, 3, strBuf);

		// resource share
		if(totalres <= 0) {
			m_ProjectListCtrl.SetItemProgress(i, 4, 100);
		} else {
			m_ProjectListCtrl.SetItemProgress(i, 4, (100 * pr->resource_share) / totalres);
		}
	}

	// update results
	Syncronize(&m_ResultListCtrl, (vector<void*>*)(&pcs->results));
	for(i = 0; i < m_ResultListCtrl.GetItemCount(); i ++) {
		RESULT* re = (RESULT*)m_ResultListCtrl.GetItemData(i);
		if(!re) {
			m_ResultListCtrl.SetItemColor(i, RGB(128, 128, 128));
			m_ResultListCtrl.SetItemProgress(i, 4, 100);
			m_ResultListCtrl.SetItemText(i, 5, "00:00:00");
			continue;
		}

		// project
		m_ResultListCtrl.SetItemText(i, 0, re->project->project_name);

		// application
		m_ResultListCtrl.SetItemText(i, 1, re->app->name);

		// name
		m_ResultListCtrl.SetItemText(i, 2, re->name);

		// cpu time
		ACTIVE_TASK* at = gstate.lookup_active_task_by_result(re);
		double cur_cpu;
		if (at) {
			cur_cpu = at->current_cpu_time;
		} else {
			if(re->state < RESULT_COMPUTE_DONE) cur_cpu = 0;
			else cur_cpu = re->final_cpu_time;
		}
		int cpuhour = (int)(cur_cpu / (60 * 60));
		int cpumin = (int)(cur_cpu / 60) % 60;
		int cpusec = (int)(cur_cpu) % 60;
		strBuf.Format("%0.2d:%0.2d:%0.2d", cpuhour, cpumin, cpusec);
		m_ResultListCtrl.SetItemText(i, 3, strBuf);

		// progress
		if(!at) {
			m_ResultListCtrl.SetItemProgress(i, 4, 0);
		} else {	
			m_ResultListCtrl.SetItemProgress(i, 4, (int)(at->fraction_done * 100));
		}

		// to completion
		if(!at || at->fraction_done == 0) {
			double tocomp = re->wup->seconds_to_complete;
			cpuhour = (int)(tocomp / (60 * 60));
			cpumin = (int)(tocomp / 60) % 60;
			cpusec = (int)(tocomp) % 60;
		} else {
			double tocomp = at->est_time_to_completion();
			cpuhour = (int)(tocomp / (60 * 60));
			cpumin = (int)(tocomp / 60) % 60;
			cpusec = (int)(tocomp) % 60;
		}
		strBuf.Format("%0.2d:%0.2d:%0.2d", cpuhour, cpumin, cpusec);
		m_ResultListCtrl.SetItemText(i, 5, strBuf);

		// status
		switch(re->state) {
			case RESULT_NEW:
				strBuf.Format("%s", "New"); break;
			case RESULT_FILES_DOWNLOADED:
				if (at)
					strBuf.Format("%s", "Running");
				else
					strBuf.Format("%s", "Ready to run");
				break;
			case RESULT_COMPUTE_DONE:
				strBuf.Format("%s", "Computation done"); break;
			case RESULT_READY_TO_ACK:
				strBuf.Format("%s", "Results uploaded"); break;
			case RESULT_SERVER_ACK:
				strBuf.Format("%s", "Acknowledged"); break;
			default:
				strBuf.Format("%s", "Error: invalid state"); break;
		}
		m_ResultListCtrl.SetItemText(i, 6, strBuf);
	}

	// update xfers
	Syncronize(&m_XferListCtrl, (vector<void*>*)(&pcs->pers_xfers->pers_file_xfers));
	for(i = 0; i < m_XferListCtrl.GetItemCount(); i ++) {
		PERS_FILE_XFER* fi = (PERS_FILE_XFER*)m_XferListCtrl.GetItemData(i);
		if(!fi) {
			m_XferListCtrl.SetItemColor(i, RGB(128, 128, 128));
			m_XferListCtrl.SetItemProgress(i, 2, 100);
			m_XferListCtrl.SetItemText(i, 3, "Completed");
			continue;
		}

		// project
		m_XferListCtrl.SetItemText(i, 0, fi->fip->project->project_name);

		// file
		m_XferListCtrl.SetItemText(i, 1, fi->fip->name);

		// progress
		if(fi->fxp) {
			m_XferListCtrl.SetItemProgress(i, 2, 100 * (fi->fxp->nbytes_xfered / fi->fip->nbytes));
		} else {
			m_XferListCtrl.SetItemProgress(i, 2, 0);
		}

		// size
		if(fi->fxp) {
			strBuf.Format("%0.0f/%0.0fKB", fi->fxp->nbytes_xfered / 1024, fi->fip->nbytes / 1024);
		} else {
			strBuf.Format("%0.0f/%0.0fKB", 0, fi->fip->nbytes / 1024);
		}
		m_XferListCtrl.SetItemText(i, 3, strBuf.GetBuffer(0));

		// time
		double xtime;
		if(fi->fxp) {
			xtime = (double)time(0) - fi->fxp->start_time;
		} else {
			xtime = 0;
		}
		int xhour = (int)(xtime / (60 * 60));
		int xmin = (int)(xtime / 60) % 60;
		int xsec = (int)(xtime) % 60;
		strBuf.Format("%0.2d:%0.2d:%0.2d", xhour, xmin, xsec);
		m_XferListCtrl.SetItemText(i, 4, strBuf.GetBuffer(0));

		// direction
		m_XferListCtrl.SetItemText(i, 5, fi->fip->generated_locally?"Upload":"Download");
	}

	// update usage
	double xDiskTotal;
	double xDiskFree; get_host_disk_info(xDiskTotal, xDiskFree);
	double xDiskUsed = xDiskTotal - xDiskFree;
	double xDiskAllow; gstate.allowed_disk_usage(xDiskAllow);
	double xDiskUsage; gstate.current_disk_usage(xDiskUsage);
	m_UsagePieCtrl.SetTotal(xDiskTotal);
	m_UsagePieCtrl.SetPiece(0, xDiskUsed - xDiskUsage); // Used (non-BOINC)
	m_UsagePieCtrl.SetPiece(1, xDiskFree - (xDiskAllow - xDiskUsage)); // Free (non-BOINC)
	m_UsagePieCtrl.SetPiece(2, xDiskAllow - xDiskUsage); // Free (non-BOINC)
	m_UsagePieCtrl.SetPiece(3, xDiskUsage); // Used (BOINC)

	// make icon flash if needed
	if(m_bMessage) {
		if(m_nIconState == ICON_NORMAL) {
			SetStatusIcon(ICON_HIGHLIGHT);
		} else if(m_nIconState == ICON_HIGHLIGHT) {
			SetStatusIcon(ICON_NORMAL);
		}
	}
}

//////////
// CMainWindow::MessageUser
// arguments:	message: message string to display
//				priority: string with priority of message
// returns:		void
// function:	if message is "high" priority, flashes the status icon, then
//				adds to message edit control.
void CMainWindow::MessageUser(char* szProject, char* szMessage, char* szPriority)
{
	if(!m_MessageListCtrl.GetSafeHwnd()) return;

	m_MessageListCtrl.InsertItem(0, szProject);

	CTime curTime = CTime::GetCurrentTime();
	CString strTime;
	strTime = curTime.Format("%c");
	m_MessageListCtrl.SetItemText(0, 1, strTime);

	m_MessageListCtrl.SetItemText(0, 2, szMessage);
	
	// set status icon to flash
	if(!strcmp(szPriority, "high") && (m_TabCtrl.GetCurSel() != MESSAGE_ID || GetForegroundWindow() != this)) {
		m_bMessage = true;
	}
}

//////////
// CMainWindow::IsSuspended
// arguments:	void
// returns:		true if the window is suspended, false otherwise
// function:	tells if the window is suspended
BOOL CMainWindow::IsSuspended()
{
	return m_bSuspend;
}

//////////
// CMainWindow::ShowTab
// arguments:	nTab: tab number to show
// returns:		void
// function:	handles everything necessary to switch to a new tab
void CMainWindow::ShowTab(int nTab)
{
	m_TabCtrl.SetCurSel(nTab);

	// make the selected control visible, all the rest invisible
	if(nTab == PROJECT_ID) {
		m_ProjectListCtrl.ModifyStyle(0, WS_VISIBLE);
		m_ResultListCtrl.ModifyStyle(WS_VISIBLE, 0);
		m_XferListCtrl.ModifyStyle(WS_VISIBLE, 0);
		m_MessageListCtrl.ModifyStyle(WS_VISIBLE, 0);
		m_UsagePieCtrl.ModifyStyle(WS_VISIBLE, 0);
		m_ProjectListCtrl.RedrawWindow(NULL, NULL, RDW_INVALIDATE|RDW_UPDATENOW|RDW_ERASE|RDW_FRAME);
	} else if(nTab == RESULT_ID) {
		m_ProjectListCtrl.ModifyStyle(WS_VISIBLE, 0);
		m_ResultListCtrl.ModifyStyle(0, WS_VISIBLE);
		m_XferListCtrl.ModifyStyle(WS_VISIBLE, 0);
		m_MessageListCtrl.ModifyStyle(WS_VISIBLE, 0);
		m_UsagePieCtrl.ModifyStyle(WS_VISIBLE, 0);
		m_ResultListCtrl.RedrawWindow(NULL, NULL, RDW_INVALIDATE|RDW_UPDATENOW|RDW_ERASE|RDW_FRAME);
	} else if(nTab == XFER_ID) {
		m_ProjectListCtrl.ModifyStyle(WS_VISIBLE, 0);
		m_ResultListCtrl.ModifyStyle(WS_VISIBLE, 0);
		m_XferListCtrl.ModifyStyle(0, WS_VISIBLE);
		m_MessageListCtrl.ModifyStyle(WS_VISIBLE, 0);
		m_UsagePieCtrl.ModifyStyle(WS_VISIBLE, 0);
		m_XferListCtrl.RedrawWindow(NULL, NULL, RDW_INVALIDATE|RDW_UPDATENOW|RDW_ERASE|RDW_FRAME);
	} else if(nTab == MESSAGE_ID) {
		m_ProjectListCtrl.ModifyStyle(WS_VISIBLE, 0);
		m_ResultListCtrl.ModifyStyle(WS_VISIBLE, 0);
		m_XferListCtrl.ModifyStyle(WS_VISIBLE, 0);
		m_MessageListCtrl.ModifyStyle(0, WS_VISIBLE);
		m_UsagePieCtrl.ModifyStyle(WS_VISIBLE, 0);
		if(m_bMessage) {
			m_bMessage = false;
			SetStatusIcon(ICON_NORMAL);
		}
		m_MessageListCtrl.RedrawWindow(NULL, NULL, RDW_INVALIDATE|RDW_UPDATENOW|RDW_ERASE|RDW_FRAME);
	} else if(nTab == USAGE_ID) {
		m_ProjectListCtrl.ModifyStyle(WS_VISIBLE, 0);
		m_ResultListCtrl.ModifyStyle(WS_VISIBLE, 0);
		m_XferListCtrl.ModifyStyle(WS_VISIBLE, 0);
		m_MessageListCtrl.ModifyStyle(WS_VISIBLE, 0);
		m_UsagePieCtrl.ModifyStyle(0, WS_VISIBLE);
		m_UsagePieCtrl.RedrawWindow(NULL, NULL, RDW_INVALIDATE|RDW_UPDATENOW|RDW_ERASE|RDW_FRAME);
	}
	m_TabCtrl.RedrawWindow(NULL, NULL, RDW_INVALIDATE|RDW_UPDATENOW|RDW_ERASE|RDW_FRAME);
	RedrawWindow();
}

//////////
// CMainWindow::SetStatusIcon
// arguments:	dwMessage: hide or show the icon
// returns:		void
// function:	controls the status icon in the taskbar
void CMainWindow::SetStatusIcon(DWORD dwMessage)
{
	if(dwMessage != ICON_OFF && dwMessage != ICON_NORMAL && dwMessage != ICON_HIGHLIGHT) {
		return;
	}
	// if icon is in that state already, there is nothing to do
	if(dwMessage == m_nIconState) return;
	NOTIFYICONDATA icon_data;
	icon_data.cbSize = sizeof(icon_data);
    icon_data.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
    icon_data.hWnd = GetSafeHwnd();
    icon_data.uID = STATUS_ICON_ID;
    strcpy(icon_data.szTip, WND_TITLE);
    icon_data.uCallbackMessage = STATUS_ICON_ID;
	if(dwMessage == ICON_OFF) {
		icon_data.hIcon = NULL;
		Shell_NotifyIcon(NIM_DELETE, &icon_data);
	} else if(dwMessage == ICON_NORMAL) {
		icon_data.hIcon = g_myApp.LoadIcon(IDI_ICON);
		if(m_nIconState == ICON_OFF) {
			Shell_NotifyIcon(NIM_ADD, &icon_data);
		} else {
			Shell_NotifyIcon(NIM_MODIFY, &icon_data);
		}
	} else if(dwMessage == ICON_HIGHLIGHT) {
		icon_data.hIcon = g_myApp.LoadIcon(IDI_ICONHIGHLIGHT);
		if(m_nIconState == ICON_OFF) {
			Shell_NotifyIcon(NIM_ADD, &icon_data);
		} else {
			Shell_NotifyIcon(NIM_MODIFY, &icon_data);
		}
	}
	m_nIconState = dwMessage;
}

//////////
// CMainWindow::SaveUserSettings
// arguments:	void
// returns:		void
// function:	saves relevant user settings to boinc.ini
void CMainWindow::SaveUserSettings()
{
	char szPath[256];
	CString strKey, strVal;
	GetCurrentDirectory(256, szPath);
	strcat(szPath, "\\boinc.ini");
	int colorder[MAX_COLS];
	int i;

	// save window size/position
	CRect rt;
	GetWindowRect(&rt);
	strVal.Format("%d", rt.left);
	WritePrivateProfileString("WINDOW", "xposition", strVal, szPath);
	strVal.Format("%d", rt.top);
	WritePrivateProfileString("WINDOW", "yposition", strVal, szPath);
	strVal.Format("%d", rt.Width());
	WritePrivateProfileString("WINDOW", "width", strVal, szPath);
	strVal.Format("%d", rt.Height());
	WritePrivateProfileString("WINDOW", "height", strVal, szPath);

	// save selected tab
	strVal.Format("%d", m_TabCtrl.GetCurSel());
	WritePrivateProfileString("WINDOW", "selection", strVal, szPath);

	// save project columns
	m_ProjectListCtrl.GetColumnOrderArray(colorder, PROJECT_COLS);
	WritePrivateProfileStruct("HEADERS", "projects-order", colorder, sizeof(colorder), szPath);
	for(i = 0; i < m_ProjectListCtrl.GetHeaderCtrl()->GetItemCount(); i ++) {
		strKey.Format("projects-%d", i);
		strVal.Format("%d", m_ProjectListCtrl.GetColumnWidth(i));
		WritePrivateProfileString("HEADERS", strKey, strVal, szPath);
	}

	// save result columns
	m_ResultListCtrl.GetColumnOrderArray(colorder, RESULT_COLS);
	WritePrivateProfileStruct("HEADERS", "results-order", colorder, sizeof(colorder), szPath);
	for(i = 0; i < m_ResultListCtrl.GetHeaderCtrl()->GetItemCount(); i ++) {
		strKey.Format("results-%d", i);
		strVal.Format("%d", m_ResultListCtrl.GetColumnWidth(i));
		WritePrivateProfileString("HEADERS", strKey, strVal, szPath);
	}

	// save xfer columns
	m_XferListCtrl.GetColumnOrderArray(colorder, XFER_COLS);
	WritePrivateProfileStruct("HEADERS", "xfers-order", colorder, sizeof(colorder), szPath);
	for(i = 0; i < m_XferListCtrl.GetHeaderCtrl()->GetItemCount(); i ++) {
		strKey.Format("xfers-%d", i);
		strVal.Format("%d", m_XferListCtrl.GetColumnWidth(i));
		WritePrivateProfileString("HEADERS", strKey, strVal, szPath);
	}

	// save xfer columns
	m_MessageListCtrl.GetColumnOrderArray(colorder, MESSAGE_COLS);
	WritePrivateProfileStruct("HEADERS", "messages-order", colorder, sizeof(colorder), szPath);
	for(i = 0; i < m_MessageListCtrl.GetHeaderCtrl()->GetItemCount(); i ++) {
		strKey.Format("messages-%d", i);
		strVal.Format("%d", m_MessageListCtrl.GetColumnWidth(i));
		WritePrivateProfileString("HEADERS", strKey, strVal, szPath);
	}
}

//////////
// CMainWindow::LoadUserSettings
// arguments:	void
// returns:		void
// function:	loads relevant user settings from boinc.ini
void CMainWindow::LoadUserSettings()
{
	char szPath[256];
	CString strKey;
	GetCurrentDirectory(256, szPath);
	strcat(szPath, "\\boinc.ini");
	int i, nBuf;
	int colorder[MAX_COLS];

	// load window size/position
	CRect rt;
	nBuf = GetPrivateProfileInt("WINDOW", "xposition", 100, szPath);
	rt.left = nBuf;
	nBuf = GetPrivateProfileInt("WINDOW", "yposition", 100, szPath);
	rt.top = nBuf;
	nBuf = GetPrivateProfileInt("WINDOW", "width", 600, szPath);
	rt.right = nBuf + rt.left;
	nBuf = GetPrivateProfileInt("WINDOW", "height", 400, szPath);
	rt.bottom = nBuf + rt.top;
	SetWindowPos(&wndNoTopMost, rt.left, rt.top, rt.Width(), rt.Height(), SWP_SHOWWINDOW);

	// load selected tab
	nBuf = GetPrivateProfileInt("WINDOW", "selection", 0, szPath);
	ShowTab(nBuf);

	// load project columns
	if(GetPrivateProfileStruct("HEADERS", "projects-order", colorder, sizeof(colorder), szPath)) {
		m_ProjectListCtrl.SetColumnOrderArray(PROJECT_COLS, colorder);
	}
	for(i = 0; i < m_ProjectListCtrl.GetHeaderCtrl()->GetItemCount(); i ++) {
		strKey.Format("projects-%d", i);
		nBuf = GetPrivateProfileInt("HEADERS", strKey.GetBuffer(0), DEF_COL_WIDTH, szPath);
		m_ProjectListCtrl.SetColumnWidth(i, nBuf);
	}

	// load result columns
	if(GetPrivateProfileStruct("HEADERS", "results-order", colorder, sizeof(colorder), szPath)) {
		m_ResultListCtrl.SetColumnOrderArray(RESULT_COLS, colorder);
	}
	for(i = 0; i < m_ResultListCtrl.GetHeaderCtrl()->GetItemCount(); i ++) {
		strKey.Format("results-%d", i);
		nBuf = GetPrivateProfileInt("HEADERS", strKey.GetBuffer(0), DEF_COL_WIDTH, szPath);
		m_ResultListCtrl.SetColumnWidth(i, nBuf);
	}

	// load xfer columns
	if(GetPrivateProfileStruct("HEADERS", "xfers-order", colorder, sizeof(colorder), szPath)) {
		m_XferListCtrl.SetColumnOrderArray(XFER_COLS, colorder);
	}
	for(i = 0; i < m_XferListCtrl.GetHeaderCtrl()->GetItemCount(); i ++) {
		strKey.Format("xfers-%d", i);
		nBuf = GetPrivateProfileInt("HEADERS", strKey.GetBuffer(0), DEF_COL_WIDTH, szPath);
		m_XferListCtrl.SetColumnWidth(i, nBuf);
	}

	// load message columns
	if(GetPrivateProfileStruct("HEADERS", "messages-order", colorder, sizeof(colorder), szPath)) {
		m_MessageListCtrl.SetColumnOrderArray(MESSAGE_COLS, colorder);
	}
	for(i = 0; i < m_MessageListCtrl.GetHeaderCtrl()->GetItemCount(); i ++) {
		strKey.Format("messages-%d", i);
		int nWidth = DEF_COL_WIDTH;
		if(i == 1) nWidth *= 1.5;
		if(i == 2) nWidth *= 4;
		nBuf = GetPrivateProfileInt("HEADERS", strKey.GetBuffer(0), nWidth, szPath);
		m_MessageListCtrl.SetColumnWidth(i, nBuf);
	}
}

//////////
// CMainWindow::GetUserIdleTime
// arguments:	void
// returns:		time the user has been idle in milliseconds
// function:	calls a dll function to determine the the user's idle time
DWORD CMainWindow::GetUserIdleTime()
{
	if(m_hIdleDll) {
		GetFn fn;
		fn = (GetFn)GetProcAddress(m_hIdleDll, "IdleTrackerGetLastTickCount");
		if(fn) {
			return GetTickCount() - fn();
		} else {
			TermFn tfn;
			tfn = (TermFn)GetProcAddress(m_hIdleDll, "IdleTrackerTerm");
			if(tfn) {
				tfn();
			}
			FreeLibrary(m_hIdleDll);
			m_hIdleDll = NULL;
		}
	}
	return 0;
}

//////////
// CMainWindow::Syncronize
// arguments:	pProg: pointer to a progress list control
//				pVect: pointer to a vector of pointers
// returns:		void
// function:	first, goes through the vector and adds items to the list
//				control for any pointers it does not already contain, then
//				goes through the list control and removes any pointers the
//				vector does not contain.
void CMainWindow::Syncronize(CProgressListCtrl* pProg, vector<void*>* pVect)
{
	int i, j;

	// add items to list that are not already in it
	for(i = 0; i < pVect->size(); i ++) {
		void* item = (*pVect)[i];
		BOOL contained = false;
		for(j = 0; j < pProg->GetItemCount(); j ++) {
			if((DWORD)item == pProg->GetItemData(j)) {
				contained = true;
				break;
			}
		}
		if(!contained) {
			pProg->InsertItem(i, "");
			pProg->SetItemData(i, (DWORD)item);
		}
	}

	// remove items from list that are not in vector
	// now just set the pointer to NULL but leave the item in the list
	for(i = 0; i < pProg->GetItemCount(); i ++) {
		DWORD item = pProg->GetItemData(i);
		BOOL contained = false;
		for(j = 0; j < pVect->size(); j ++) {
			if(item == (DWORD)(*pVect)[j]) {
				contained = true;
				break;
			}
		}
		if(!contained) {
			pProg->SetItemData(i, (DWORD)NULL);
		}
	}
}

//////////
// CMainWindow::PostNcDestroy
// arguments:	void
// returns:		void
// function:	takes care of window being destroyed
void CMainWindow::PostNcDestroy()
{
    delete this;
}

//////////
// CMainWindow::DefWindowProc
// arguments:	message: message received
//				wParam: message's wparam
//				lParam: message's lparam
// returns:		dependent on message
// function:	handles any messages not handled by the window previously
LRESULT CMainWindow::DefWindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	UINT uShowMsg = RegisterWindowMessage("BOINC_SHOW_MESSAGE");
	if(uShowMsg == message) {
		ShowWindow(SW_SHOW);
		SetForegroundWindow();
		return 0;
	}
	return CWnd::DefWindowProc(message, wParam, lParam);
}

//////////
// CMainWindow::OnClose
// arguments:	void
// returns:		void
// function:	hides the window, keeps status icon
void CMainWindow::OnClose()
{
	ShowWindow(SW_HIDE);
}

//////////
// CMainWindow::OnCommandSettingsQuit
// arguments:	void
// returns:		void
// function:	shows the account quit dialog box
void CMainWindow::OnCommandSettingsQuit()
{
    CQuitDialog dlg(IDD_QUIT);
    int nResult = dlg.DoModal();
	if(nResult == IDOK) {
		CString str;
		if(strcmp(gstate.projects[dlg.m_nSel]->project_name, "")) {
			str.Format("Are you sure you want to quit the project %s?", gstate.projects[dlg.m_nSel]->project_name);
		} else {
			str.Format("Are you sure you want to quit the project %s?", gstate.projects[dlg.m_nSel]->master_url);
		}
		if(AfxMessageBox(str, MB_YESNO, 0) == IDYES) {
			gstate.quit_project(dlg.m_nSel);
		}
	}
}

//////////
// CMainWindow::OnCommandSettingsLogin
// arguments:	void
// returns:		void
// function:	shows the account login dialog box
void CMainWindow::OnCommandSettingsLogin()
{
    CLoginDialog dlg(IDD_LOGIN, "", "");
    int nResult = dlg.DoModal();
	if(nResult == IDOK) {
	    gstate.add_project(dlg.m_strUrl.GetBuffer(0), dlg.m_strAuth.GetBuffer(0));
	}
}

//////////
// CMainWindow::OnCommandSettingsProxyServer
// arguments:	void
// returns:		void
// function:	shows the proxy dialog box
void CMainWindow::OnCommandSettingsProxyServer()
{
	CProxyDialog dlg(IDD_PROXY);
	int nResult = dlg.DoModal();
}

//////////
// CMainWindow::OnCommandHelpAbout
// arguments:	void
// returns:		void
// function:	shows the about dialog box
void CMainWindow::OnCommandHelpAbout()
{
	CDialog dlg(IDD_ABOUTBOX);
	int nResult = dlg.DoModal();
}

//////////
// CMainWindow::OnCommandFileShowGraphics
// arguments:	void
// returns:		void
// function:	brings up the current app's graphics window by
//				broadcasting a message
void CMainWindow::OnCommandFileShowGraphics()
{
	int nGraphicsMsg = RegisterWindowMessage("BOINC_GFX_MODE");
	::PostMessage(HWND_BROADCAST, nGraphicsMsg, 0, MODE_WINDOW);	
}

//////////
// CMainWindow::OnCommandFileClearInactive
// arguments:	void
// returns:		void
// function:	clears inactive items from lists
void CMainWindow::OnCommandFileClearInactive()
{
	int i;
	for(i = 0; i < m_ProjectListCtrl.GetItemCount();) {
		if(!m_ProjectListCtrl.GetItemData(i)) {
			m_ProjectListCtrl.DeleteItem(i);
		} else {
			i ++;
		}
	}
	for(i = 0; i < m_ResultListCtrl.GetItemCount();) {
		if(!m_ResultListCtrl.GetItemData(i)) {
			m_ResultListCtrl.DeleteItem(i);
		} else {
			i ++;
		}
	}
	for(i = 0; i < m_XferListCtrl.GetItemCount();) {
		if(!m_XferListCtrl.GetItemData(i)) {
			m_XferListCtrl.DeleteItem(i);
		} else {
			i ++;
		}
	}
}

//////////
// CMainWindow::OnCommandFileClearMessages
// arguments:	void
// returns:		void
// function:	clears messages
void CMainWindow::OnCommandFileClearMessages()
{
	m_MessageListCtrl.DeleteAllItems();
}

//////////
// CMainWindow::OnCommandProjectRelogin
// arguments:	void
// returns:		void
// function:	lets a user change the properties for an account
void CMainWindow::OnCommandProjectRelogin()
{
	if(m_nContextItem < 0 || m_nContextItem > m_ProjectListCtrl.GetItemCount()) return;
	PROJECT* pToRelogin = (PROJECT*)m_ProjectListCtrl.GetItemData(m_nContextItem);
	m_nContextItem = -1;
	if(!pToRelogin) return;

	// find project index
	int i;
	for(i = 0; i < gstate.projects.size(); i ++) {
		if(gstate.projects[i] == pToRelogin) break;
	}
	if(i == gstate.projects.size()) return;

	// get info and relogin
    CLoginDialog dlg(IDD_LOGIN, gstate.projects[i]->master_url, gstate.projects[i]->authenticator);
    int retval = dlg.DoModal();
	if(retval == IDOK) {
		gstate.change_project(i, dlg.m_strUrl.GetBuffer(0), dlg.m_strAuth.GetBuffer(0));
	}
}

//////////
// CMainWindow::OnCommandProjectQuit
// arguments:	void
// returns:		void
// function:	lets the user quit a project
void CMainWindow::OnCommandProjectQuit()
{
	if(m_nContextItem < 0 || m_nContextItem > m_ProjectListCtrl.GetItemCount()) return;
	PROJECT* pToQuit = (PROJECT*)m_ProjectListCtrl.GetItemData(m_nContextItem);
	m_nContextItem = -1;
	if(!pToQuit) return;

	// find project index
	int i;
	for(i = 0; i < gstate.projects.size(); i ++) {
		if(gstate.projects[i] == pToQuit) break;
	}
	if(i == gstate.projects.size()) return;

	// confirm and quit
	CString strBuf;
	if(strcmp(gstate.projects[i]->project_name, "")) {
		strBuf.Format("Are you sure you want to quit the project %s?", gstate.projects[i]->project_name);
	} else {
		strBuf.Format("Are you sure you want to quit the project %s?", gstate.projects[i]->master_url);
	}
	if(AfxMessageBox(strBuf, MB_YESNO, 0) == IDYES) {
		gstate.quit_project(i);
	}
}

//////////
// CMainWindow::OnCommandHide
// arguments:	void
// returns:		void
// function:	hides or shows the window
void CMainWindow::OnCommandHide()
{
	CMenu* pMainMenu;
	CMenu* pFileMenu;
	pMainMenu = GetMenu();
	if(pMainMenu) {
		pFileMenu = pMainMenu->GetSubMenu(0);
	}
	if(IsWindowVisible()) {
		ShowWindow(SW_HIDE);
		if(pFileMenu) {
			pFileMenu->CheckMenuItem(ID_FILE_HIDE, MF_CHECKED);
		}
	} else {
		ShowWindow(SW_SHOW);
		if(pFileMenu) {
			pFileMenu->CheckMenuItem(ID_FILE_HIDE, MF_UNCHECKED);
		}
	}
}

//////////
// CMainWindow::OnCommandSuspend
// arguments:	void
// returns:		void
// function:	suspends client
void CMainWindow::OnCommandSuspend()
{
	gstate.suspend_requested = true;
	m_bSuspend = true;

	CMenu* pMainMenu;
	CMenu* pFileMenu;
	pMainMenu = GetMenu();
	if(pMainMenu) {
		pFileMenu = pMainMenu->GetSubMenu(0);
	}
	if(pFileMenu) {
		pFileMenu->EnableMenuItem(ID_FILE_SUSPEND, MF_GRAYED);
		pFileMenu->EnableMenuItem(ID_FILE_RESUME, MF_ENABLED);
	}
}

//////////
// CMainWindow::OnCommandResume
// arguments:	void
// returns:		void
// function:	resumes client
void CMainWindow::OnCommandResume()
{
	gstate.suspend_requested = false;
	m_bSuspend = false;

	CMenu* pMainMenu;
	CMenu* pFileMenu;
	pMainMenu = GetMenu();
	if(pMainMenu) {
		pFileMenu = pMainMenu->GetSubMenu(0);
	}
	if(pFileMenu) {
		pFileMenu->EnableMenuItem(ID_FILE_SUSPEND, MF_ENABLED);
		pFileMenu->EnableMenuItem(ID_FILE_RESUME, MF_GRAYED);
	}
}

//////////
// CMainWindow::OnCommandExit
// arguments:	void
// returns:		void
// function:	cleans up, closes and quits everything
void CMainWindow::OnCommandExit()
{
	// quit
	gstate.exit();
	PostQuitMessage(0);

	// status icon in taskbar
	SetStatusIcon(ICON_OFF);

	// clean up and delete objects
	m_Font.DeleteObject();
	m_TabBMP[0].DeleteObject();
	m_TabBMP[1].DeleteObject();
	m_TabBMP[2].DeleteObject();
	m_TabBMP[3].DeleteObject();
	m_TabBMP[4].DeleteObject();
	m_TabIL.DeleteImageList();
	m_MainMenu.DestroyMenu();

	// free dll and idle detection
	if(m_hIdleDll) {
		TermFn fn;
		fn = (TermFn)GetProcAddress(m_hIdleDll, "IdleTrackerTerm");
		if(!fn) {
			MessageUser("", "Error in DLL \"boinc.dll\"", "low");
		} else {
			fn();
		}
		FreeLibrary(m_hIdleDll);
		m_hIdleDll = NULL;
	}

	SaveUserSettings();
	CWnd::OnClose();
}

//////////
// CMainWindow::OnCreate
// arguments:	lpcs: a pointer to the create structure
// returns:		0 if successful, otherwise -1
// function:	sets window's global variable, loads resource, creates child
//				windows, and initializes client state and timer
int CMainWindow::OnCreate(LPCREATESTRUCT lpcs)
{
    if (CWnd::OnCreate(lpcs) == -1) {
		return -1;
	}

    g_myWnd = this;
	m_nIconState = ICON_OFF;
	m_bSuspend = false;
	m_bMessage = false;
	m_nContextItem = -1;

	// load main menu
	m_MainMenu.LoadMenu(IDR_MAINFRAME);
	SetMenu(&m_MainMenu);

	// create project list control
	m_ProjectListCtrl.Create(LVS_REPORT|WS_CHILD|WS_BORDER|WS_VISIBLE, CRect(0,0,0,0), this, PROJECT_ID);
	m_ProjectListCtrl.SetExtendedStyle(m_ProjectListCtrl.GetExtendedStyle()|LVS_EX_HEADERDRAGDROP|LVS_EX_FULLROWSELECT);
	for(int i = 0; i < PROJECT_COLS; i ++) {
		m_ProjectListCtrl.InsertColumn(i, g_szColumnTitles[PROJECT_ID][i], LVCFMT_LEFT, DEF_COL_WIDTH);
	}

	// create result list control
	m_ResultListCtrl.Create(LVS_REPORT|WS_CHILD|WS_BORDER|WS_VISIBLE, CRect(0,0,0,0), this, RESULT_ID);
	m_ResultListCtrl.SetExtendedStyle(m_ResultListCtrl.GetExtendedStyle()|LVS_EX_HEADERDRAGDROP|LVS_EX_FULLROWSELECT);
	m_ResultListCtrl.ModifyStyle(WS_VISIBLE, 0);
	for(i = 0; i < RESULT_COLS; i ++) {
		m_ResultListCtrl.InsertColumn(i, g_szColumnTitles[RESULT_ID][i], LVCFMT_LEFT, DEF_COL_WIDTH);
	}

	// create xfer list control
	m_XferListCtrl.Create(LVS_REPORT|WS_CHILD|WS_BORDER|WS_VISIBLE, CRect(0,0,0,0), this, XFER_ID);
	m_XferListCtrl.SetExtendedStyle(m_XferListCtrl.GetExtendedStyle()|LVS_EX_HEADERDRAGDROP|LVS_EX_FULLROWSELECT);
	m_XferListCtrl.ModifyStyle(WS_VISIBLE, 0);
	for(i = 0; i < XFER_COLS; i ++) {
		m_XferListCtrl.InsertColumn(i, g_szColumnTitles[XFER_ID][i], LVCFMT_LEFT, DEF_COL_WIDTH);
	}

	// create message edit control
	// create xfer list control
	m_MessageListCtrl.Create(LVS_REPORT|WS_CHILD|WS_BORDER|WS_VISIBLE, CRect(0,0,0,0), this, MESSAGE_ID);
	m_MessageListCtrl.SetExtendedStyle(m_MessageListCtrl.GetExtendedStyle()|LVS_EX_HEADERDRAGDROP|LVS_EX_FULLROWSELECT);
	m_MessageListCtrl.ModifyStyle(WS_VISIBLE, 0);
	for(i = 0; i < MESSAGE_COLS; i ++) {
		int width = DEF_COL_WIDTH;
		if(i == 1) width *= 1.5;
		if(i == 2) width *= 4;
		m_MessageListCtrl.InsertColumn(i, g_szColumnTitles[MESSAGE_ID][i], LVCFMT_LEFT, width);
	}

	// create usage pie control
	m_UsagePieCtrl.Create(WS_CHILD|WS_BORDER|WS_VISIBLE, CRect(0,0,0,0), this, USAGE_ID);
	m_UsagePieCtrl.ModifyStyle(WS_VISIBLE, 0);
	/*
	m_UsagePieCtrl.AddPiece("Free space", RGB(192, 192, 192), 0);
	m_UsagePieCtrl.AddPiece("Used space", RGB(0, 0, 255), 0);
	m_UsagePieCtrl.AddPiece("Used space (BOINC)", RGB(255, 255, 0), 0);
	m_UsagePieCtrl.AddPiece("Free space (BOINC)", RGB(255, 128, 0), 0);
	*/
	m_UsagePieCtrl.AddPiece("Used space (Non-BOINC)", RGB(0, 0, 255), 0);
	m_UsagePieCtrl.AddPiece("Free space (Non-BOINC)", RGB(255, 0, 255), 0);
	m_UsagePieCtrl.AddPiece("Free space (BOINC)", RGB(192, 0, 192), 0);
	m_UsagePieCtrl.AddPiece("Used space (BOINC)", RGB(0, 0, 192), 0);

	// set up image list for tab control
	m_TabIL.Create(16, 16, ILC_COLOR8|ILC_MASK, 5, 1);
	m_TabBMP[0].LoadBitmap(IDB_PROJ);
	m_TabIL.Add(&m_TabBMP[0], RGB(255, 0, 255));
	m_TabBMP[1].LoadBitmap(IDB_RESULT);
	m_TabIL.Add(&m_TabBMP[1], RGB(255, 0, 255));
	m_TabBMP[2].LoadBitmap(IDB_XFER);
	m_TabIL.Add(&m_TabBMP[2], RGB(255, 0, 255));
	m_TabBMP[3].LoadBitmap(IDB_MESS);
	m_TabIL.Add(&m_TabBMP[3], RGB(255, 0, 255));
	m_TabBMP[4].LoadBitmap(IDB_USAGE);
	m_TabIL.Add(&m_TabBMP[4], RGB(255, 0, 255));

	// create tab control
	m_TabCtrl.Create(TCS_FIXEDWIDTH|TCS_BUTTONS|TCS_FLATBUTTONS|TCS_FOCUSNEVER|WS_CHILD|WS_VISIBLE, CRect(0,0,0,0), this, TAB_ID);
	m_TabCtrl.SetImageList(&m_TabIL);
	m_TabCtrl.InsertItem(1, "Projects", 0);
	m_TabCtrl.InsertItem(2, "Work", 1);
	m_TabCtrl.InsertItem(3, "Transfers", 2);
	m_TabCtrl.InsertItem(4, "Messages", 3);
	m_TabCtrl.InsertItem(5, "Usage", 4);

	// make all fonts the same nice font
	CFont* pFont;
	pFont = m_ProjectListCtrl.GetFont();
	LOGFONT lf;
	ZeroMemory(&lf, sizeof(LOGFONT));
	pFont->GetLogFont(&lf);
	m_Font.CreateFontIndirect(&lf);
	m_TabCtrl.SetFont(&m_Font);
	m_UsagePieCtrl.SetFont(&m_Font);

	// add status icon to taskbar
	SetStatusIcon(ICON_NORMAL);

	// take care of other things
	// 
    NetOpen();
	// Redirect stdout and stderr to files
    freopen(STDOUT_FILE_NAME, "w", stdout);
    freopen(STDERR_FILE_NAME, "w", stderr);
	// Check what (if any) activities should be logged
    read_log_flags();
    int retval = gstate.init();
    if (retval) {
		OnCommandExit();
		return 0;
	}
    SetTimer(ID_TIMER, 1000, TimerProc);

	// load dll and start idle detection
	m_hIdleDll = LoadLibrary("boinc.dll");
	if(!m_hIdleDll) {
		MessageUser("", "Can't load \"boinc.dll\", will not be able to determine idle time", "high");
	} else {
		InitFn fn;
		fn = (InitFn)GetProcAddress(m_hIdleDll, "IdleTrackerInit");
		if(!fn) {
			MessageUser("", "Error in DLL \"boinc.dll\", will not be able to determine idle time", "low");
			FreeLibrary(m_hIdleDll);
			m_hIdleDll = NULL;
		} else {
			if(!fn()) {
				MessageUser("", "Error in DLL \"boinc.dll\", will not be able to determine idle time", "low");
				FreeLibrary(m_hIdleDll);
				m_hIdleDll = NULL;
			}
		}
	}

	LoadUserSettings();
	UpdateGUI(&gstate);
	OnCommandResume();

    return 0;
}

//////////
// CMainWindow::OnNotify
// arguments:	wParam: notification's wparam
//				lParam: notification's lparam
//				pResult: pointer to result of notification
// returns:		true if the notification is processed, otherwise false
// function:	handles notifications from children, including:
//				user selecting a new tab sets display to selected tab's control
BOOL CMainWindow::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult)
{
	HD_NOTIFY* phdn = (HD_NOTIFY*)lParam;

	// notification from tab control, user is changing the selection
	if(phdn->hdr.code == TCN_SELCHANGE) {
		int newTab = m_TabCtrl.GetCurSel();
		ShowTab(newTab);
	}
	return CWnd::OnNotify(wParam, lParam, pResult);
}

//////////
// CMainWindow::OnRButtonDown
// arguments:	nFlags: message flags (keys down)
//				point: mouse's point
// returns:		void
// function:	shows context menu for list items
void CMainWindow::OnRButtonDown(UINT nFlags, CPoint point)
{
	CMenu WholeMenu;
	CMenu* pContextMenu = NULL;
	GetCursorPos(&point);
	CRect rt;
	CListCtrl* pMenuCtrl = NULL;
	int nMenuId = -1;
	if(m_ProjectListCtrl.IsWindowVisible()) {
		pMenuCtrl = &m_ProjectListCtrl;
		nMenuId = PROJECT_MENU;
	} else if(m_ResultListCtrl.IsWindowVisible()) {
		pMenuCtrl = &m_ResultListCtrl;
		nMenuId = RESULT_MENU;
	} else if(m_XferListCtrl.IsWindowVisible()) {
		pMenuCtrl = &m_XferListCtrl;
		nMenuId = XFER_MENU;
	}
	if(pMenuCtrl) {
		for(int i = 0; i < pMenuCtrl->GetItemCount(); i ++) {
			pMenuCtrl->GetItemRect(i, &rt, LVIR_BOUNDS);
			pMenuCtrl->ClientToScreen(&rt);
			if(rt.PtInRect(point)) {
				WholeMenu.LoadMenu(IDR_CONTEXT);
				pContextMenu = WholeMenu.GetSubMenu(nMenuId);
				if(pContextMenu) {
					pContextMenu->TrackPopupMenu(TPM_LEFTALIGN|TPM_RIGHTBUTTON, point.x, point.y, this);
					m_nContextItem = i;
				}
				break;
			}
		}
	}
	//CWnd::OnRButtonDown(nFlags, point);
}

//////////
// CMainWindow::OnFocus
// arguments:	pOldWnd: pointer to previous window that had focus
// returns:		void
// function:	if there is a message for the user when this window
//				gets the focus, selects the message tab
void CMainWindow::OnSetFocus(CWnd* pOldWnd)
{
	if(m_TabCtrl.GetSafeHwnd() && m_bMessage) {
		m_TabCtrl.SetCurSel(MESSAGE_ID);
		m_ProjectListCtrl.ModifyStyle(WS_VISIBLE, 0);
		m_ResultListCtrl.ModifyStyle(WS_VISIBLE, 0);
		m_XferListCtrl.ModifyStyle(WS_VISIBLE, 0);
		m_MessageListCtrl.ModifyStyle(0, WS_VISIBLE);
		m_UsagePieCtrl.ModifyStyle(WS_VISIBLE, 0);
		m_MessageListCtrl.RedrawWindow(NULL, NULL, RDW_INVALIDATE|RDW_UPDATENOW|RDW_ERASE|RDW_FRAME);
		m_bMessage = false;
		SetStatusIcon(ICON_NORMAL);
	}
}

//////////
// CMainWindow::OnSize
// arguments:	nType: type of resizing
//				cx: new width of window
//				cy: new height of window
// returns:		void
// function:	calculates new rectangles for child windows and resizes
//				them appropriately
void CMainWindow::OnSize(UINT nType, int cx, int cy)
{
	CWnd::OnSize(nType, cx, cy);

	// calculate the main rect for the tab control
	RECT rt = {EDGE_BUFFER, EDGE_BUFFER, cx-EDGE_BUFFER, cy-EDGE_BUFFER*2};
	RECT irt = {0, 0, 0, 0};
	if(m_TabCtrl.GetSafeHwnd()) {
		m_TabCtrl.MoveWindow(&rt, false);
		m_TabCtrl.GetItemRect(0, &irt);

		// calculate the rects for other controls inside the tab control
		RECT srt = {rt.left+EDGE_BUFFER, irt.bottom+EDGE_BUFFER*2, rt.right-EDGE_BUFFER, rt.bottom-EDGE_BUFFER};
		if(m_ProjectListCtrl.GetSafeHwnd()) {
			m_ProjectListCtrl.MoveWindow(&srt, false);
			m_ProjectListCtrl.RedrawWindow(NULL,NULL,RDW_INVALIDATE|RDW_UPDATENOW|RDW_ERASE|RDW_FRAME);
		}
		if(m_ResultListCtrl.GetSafeHwnd()) {
			m_ResultListCtrl.MoveWindow(&srt, false);
			m_ResultListCtrl.RedrawWindow(NULL,NULL,RDW_INVALIDATE|RDW_UPDATENOW|RDW_ERASE|RDW_FRAME);
		}
		if(m_XferListCtrl.GetSafeHwnd()) {
			m_XferListCtrl.MoveWindow(&srt, false);
			m_XferListCtrl.RedrawWindow(NULL,NULL,RDW_INVALIDATE|RDW_UPDATENOW|RDW_ERASE|RDW_FRAME);
		}
		if(m_MessageListCtrl.GetSafeHwnd()) {
			m_MessageListCtrl.MoveWindow(&srt, false);
			m_MessageListCtrl.RedrawWindow(NULL,NULL,RDW_INVALIDATE|RDW_UPDATENOW|RDW_ERASE|RDW_FRAME);
		}
		if(m_UsagePieCtrl.GetSafeHwnd()) {
			m_UsagePieCtrl.MoveWindow(&srt, false);
			m_UsagePieCtrl.RedrawWindow(NULL,NULL,RDW_INVALIDATE|RDW_UPDATENOW|RDW_NOERASE|RDW_FRAME);
		}
		m_TabCtrl.RedrawWindow(NULL,NULL,RDW_INVALIDATE|RDW_UPDATENOW|RDW_NOERASE|RDW_FRAME);
		RedrawWindow(NULL,NULL,RDW_INVALIDATE|RDW_UPDATENOW|RDW_ERASE|RDW_FRAME);
	}
}

//////////
// CMainWindow::OnStatusIcon
// arguments:	wParam: id of icon clicked
//				lParam: message from icon
// returns:		true if the menu is shown, false otherwise
// function:	handles messages from status icon, including:
//				right click: shows popup menu
//				double click: alternates visibility of window
LRESULT CMainWindow::OnStatusIcon(WPARAM wParam, LPARAM lParam)
{
	if(lParam == WM_RBUTTONDOWN) {
		CPoint point;
		SetForegroundWindow();
		GetCursorPos(&point);
		CMenu Menu, *pSubmenu;
		if(!Menu.LoadMenu(IDR_CONTEXT)) {
			return FALSE;
		}
		pSubmenu = Menu.GetSubMenu(STATUS_MENU);
		if(!pSubmenu) {
			Menu.DestroyMenu();
			return FALSE;
		}
		if(m_bSuspend) {
			pSubmenu->EnableMenuItem(ID_STATUSICON_SUSPEND, MF_GRAYED);
		} else {
			pSubmenu->EnableMenuItem(ID_STATUSICON_RESUME, MF_GRAYED);
		}
		pSubmenu->TrackPopupMenu(TPM_LEFTALIGN|TPM_RIGHTBUTTON, point.x, point.y, this);
		Menu.DestroyMenu();
	} else if(lParam == WM_LBUTTONDOWN) {
		if(IsWindowVisible()) {
			SetForegroundWindow();
		}
	} else if(lParam == WM_LBUTTONDBLCLK) {
		if(IsWindowVisible()) {
			ShowWindow(SW_HIDE);
		} else {
			ShowWindow(SW_SHOW);
		}
	}
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////
// CLoginDialog message map and member functions

BEGIN_MESSAGE_MAP(CLoginDialog, CDialog)
    ON_BN_CLICKED(IDOK, OnOK)
	ON_NOTIFY_EX_RANGE(TTN_NEEDTEXTW, 0, 0xFFFF, OnToolTipNotify)
	ON_NOTIFY_EX_RANGE(TTN_NEEDTEXTA, 0, 0xFFFF, OnToolTipNotify)
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
// CLoginDialog::OnInitDialog
// arguments:	void
// returns:		true if windows needs to give dialog focus, false if dialog has taken focus
// function:	initializes and centers dialog box
BOOL CLoginDialog::OnInitDialog() 
{
    CDialog::OnInitDialog();
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
	// need to handle both ANSI and UNICODE versions of the message
	TOOLTIPTEXTA* pTTTA = (TOOLTIPTEXTA*)pNMHDR;
	TOOLTIPTEXTW* pTTTW = (TOOLTIPTEXTW*)pNMHDR;
	CString strTipText;
	UINT nID = pNMHDR->idFrom;
	if(pNMHDR->code == TTN_NEEDTEXTA && (pTTTA->uFlags & TTF_IDISHWND) ||
		pNMHDR->code == TTN_NEEDTEXTW && (pTTTW->uFlags & TTF_IDISHWND)) {

		// idFrom is actually the HWND of the tool
		CWnd* wnd = CWnd::FromHandle((HWND)nID);
		if(wnd) nID = wnd->GetDlgCtrlID();
	}

	if(nID == IDC_LOGIN_URL) strTipText.Format("The url for the website of the project.");
	if(nID == IDC_LOGIN_AUTH) strTipText.Format("The authorization code recieved in your confirmation email.");
	if(pNMHDR->code == TTN_NEEDTEXTA) {
		lstrcpyn(pTTTA->szText, strTipText, sizeof(pTTTA->szText));
	} else {
		_mbstowcsz(pTTTW->szText, strTipText, sizeof(pTTTW->szText));
	}
	*pResult = 0;

    // message was handled
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////
// CQuitDialog message map and member functions

BEGIN_MESSAGE_MAP(CQuitDialog, CDialog)
    ON_BN_CLICKED(IDOK, OnOK)
	ON_NOTIFY_EX_RANGE(TTN_NEEDTEXTW, 0, 0xFFFF, OnToolTipNotify)
	ON_NOTIFY_EX_RANGE(TTN_NEEDTEXTA, 0, 0xFFFF, OnToolTipNotify)
END_MESSAGE_MAP()

//////////
// CQuitDialog::CQuitDialog
// arguments:	y: dialog box resource id
// returns:		void
// function:	calls parents contructor.
CQuitDialog::CQuitDialog(UINT y) : CDialog(y)
{
}

//////////
// CQuitDialog::OnInitDialog
// arguments:	void
// returns:		true if windows needs to give dialog focus, false if dialog has taken focus
// function:	initializes and centers dialog box
BOOL CQuitDialog::OnInitDialog() 
{
    CDialog::OnInitDialog();
	CListBox* pListBox = (CListBox*)GetDlgItem(IDC_LIST);
	if(pListBox) {
		for(int i = 0; i < gstate.projects.size(); i ++) {
			if(!strcmp(gstate.projects[i]->project_name, "")) {
				pListBox->AddString(gstate.projects[i]->master_url);
			} else {
				pListBox->AddString(gstate.projects[i]->project_name);
			}
		}
		pListBox->SetFocus();
	}
    CenterWindow();
	EnableToolTips(TRUE);
    return TRUE;
}

//////////
// CQuitDialog::OnOK
// arguments:	void
// returns:		void
// function:	sets member variables, selected project to quit
void CQuitDialog::OnOK() 
{
	m_nSel = -1;
	CListBox* pListBox = (CListBox*)GetDlgItem(IDC_LIST);
	if(pListBox) {
		m_nSel = pListBox->GetCurSel();
	}
    if(m_nSel >= 0) CDialog::OnOK();
	else CDialog::OnCancel();
}

//////////
// CQuitDialog::OnToolTipNotify
// arguments:	id: id of the window, actually its hwnd
//				pNMHDR: pointer to notification message header
//				pResult: pointer to result of notification
// returns:		true if the notification is processed, otherwise false
// function:	handles notifications of tool tips by filling in 
//				text for tool tips
BOOL CQuitDialog::OnToolTipNotify(UINT id, NMHDR *pNMHDR, LRESULT *pResult)
{
	// need to handle both ANSI and UNICODE versions of the message
	TOOLTIPTEXTA* pTTTA = (TOOLTIPTEXTA*)pNMHDR;
	TOOLTIPTEXTW* pTTTW = (TOOLTIPTEXTW*)pNMHDR;
	CString strTipText;
	UINT nID = pNMHDR->idFrom;
	if(pNMHDR->code == TTN_NEEDTEXTA && (pTTTA->uFlags & TTF_IDISHWND) ||
		pNMHDR->code == TTN_NEEDTEXTW && (pTTTW->uFlags & TTF_IDISHWND)) {

		// idFrom is actually the HWND of the tool
		CWnd* wnd = CWnd::FromHandle((HWND)nID);
		if(wnd) nID = wnd->GetDlgCtrlID();
	}

	if(nID == IDC_LIST) strTipText.Format("Select the project you wish to quit.");
	if(pNMHDR->code == TTN_NEEDTEXTA) {
		lstrcpyn(pTTTA->szText, strTipText, sizeof(pTTTA->szText));
	} else {
		_mbstowcsz(pTTTW->szText, strTipText, sizeof(pTTTW->szText));
	}
	*pResult = 0;

    // message was handled
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////
// CProxyDialog message map and member functions

BEGIN_MESSAGE_MAP(CProxyDialog, CDialog)
    ON_BN_CLICKED(IDC_CHECK_HTTP, OnHttp)
    ON_BN_CLICKED(IDC_CHECK_SOCKS, OnSocks)
    ON_BN_CLICKED(IDOK, OnOK)
END_MESSAGE_MAP()

//////////
// CProxyDialog::CProxyDialog
// arguments:	y: dialog box resource id
// returns:		void
// function:	calls parents contructor.
CProxyDialog::CProxyDialog(UINT y) : CDialog(y)
{
}

//////////
// CProxyDialog::OnInitDialog
// arguments:	void
// returns:		true if windows needs to give dialog focus, false if dialog has taken focus
// function:	initializes and centers dialog box
BOOL CProxyDialog::OnInitDialog() 
{
    CDialog::OnInitDialog();
	CButton* pBtn;

	// fill in http
	pBtn = (CButton*)GetDlgItem(IDC_CHECK_HTTP);
	if(pBtn) pBtn->SetCheck(gstate.use_proxy?BST_CHECKED:BST_UNCHECKED);
	SetDlgItemText(IDC_EDIT_HTTP_ADDR, gstate.proxy_server_name);
	CString portBuf;
	if(gstate.proxy_server_port > 0) portBuf.Format("%d", gstate.proxy_server_port);
	else portBuf.Format("80");
	SetDlgItemText(IDC_EDIT_HTTP_PORT, portBuf.GetBuffer(0));
	EnableHttp(gstate.use_proxy);
	
	// fill in socks
	pBtn = (CButton*)GetDlgItem(IDC_CHECK_SOCKS);
	if(pBtn) pBtn->EnableWindow(false);
	SetDlgItemText(IDC_EDIT_SOCKS_PORT, "1080");
	EnableSocks(false);
    CenterWindow();
    return TRUE;
}

//////////
// CProxyDialog::EnableHttp
// arguments:	bEnable: true to enable, false to disable
// returns:		void
// function:	enables or disables the http section of the dialog
void CProxyDialog::EnableHttp(BOOL bEnable)
{
	CEdit* pEdit;
	pEdit = (CEdit*)GetDlgItem(IDC_EDIT_HTTP_ADDR);
	if(pEdit) pEdit->EnableWindow(bEnable);
	pEdit = (CEdit*)GetDlgItem(IDC_EDIT_HTTP_PORT);
	if(pEdit) pEdit->EnableWindow(bEnable);
}

//////////
// CProxyDialog::EnableSocks
// arguments:	bEnable: true to enable, false to disable
// returns:		void
// function:	enables or disables the socks section of the dialog
void CProxyDialog::EnableSocks(BOOL bEnable)
{
	CEdit* pEdit;
	pEdit = (CEdit*)GetDlgItem(IDC_EDIT_SOCKS_ADDR);
	if(pEdit) pEdit->EnableWindow(bEnable);
	pEdit = (CEdit*)GetDlgItem(IDC_EDIT_SOCKS_PORT);
	if(pEdit) pEdit->EnableWindow(bEnable);
	pEdit = (CEdit*)GetDlgItem(IDC_EDIT_SOCKS_NAME);
	if(pEdit) pEdit->EnableWindow(bEnable);
	pEdit = (CEdit*)GetDlgItem(IDC_EDIT_SOCKS_PASS);
	if(pEdit) pEdit->EnableWindow(bEnable);
}

//////////
// CProxyDialog::OnOK
// arguments:	void
// returns:		void
// function:	handles http check box
void CProxyDialog::OnHttp() 
{
	CButton* pBtn;
	pBtn = (CButton*)GetDlgItem(IDC_CHECK_HTTP);
	if(pBtn) {
		if(pBtn->GetCheck() == BST_CHECKED) {
			EnableHttp(true);
		} else {
			EnableHttp(false);
		}
	}
}

//////////
// CProxyDialog::OnSocks
// arguments:	void
// returns:		void
// function:	handles socks check box
void CProxyDialog::OnSocks() 
{
}

//////////
// CProxyDialog::OnOK
// arguments:	void
// returns:		void
// function:	sets member variables, selected project to quit
void CProxyDialog::OnOK() 
{
	CButton* pBtn;
	CString strbuf;

	// get http info
	pBtn = (CButton*)GetDlgItem(IDC_CHECK_HTTP);
	if(pBtn) {
		if(pBtn->GetCheck() == BST_CHECKED) {
			gstate.use_proxy = true;
		} else {
			gstate.use_proxy = false;
		}
	}
	GetDlgItemText(IDC_EDIT_HTTP_ADDR, strbuf);
	strcpy(gstate.proxy_server_name, strbuf.GetBuffer(0));
	GetDlgItemText(IDC_EDIT_HTTP_PORT, strbuf);
	gstate.proxy_server_port = atoi(strbuf.GetBuffer(0));
	CDialog::OnOK();
}
