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

#define STATUS_ICON_ID		(WM_USER + 1)

#define STATUS_MENU			0
#define PROJECT_MENU		1
#define RESULT_MENU			2
#define XFER_MENU			3

#define PROJECT_ID			0
#define RESULT_ID			1
#define XFER_ID				2
#define MESSAGE_ID			3
#define MAX_LIST_ID			4
#define USAGE_ID			4
#define TAB_ID				5

#define PROJECT_COLS		5
#define RESULT_COLS			7
#define XFER_COLS			6
#define MESSAGE_COLS		3
#define MAX_COLS			7

char* column_titles[MAX_LIST_ID][MAX_COLS] = {
        {"Project",	"Account",		"Total Credit",	"Avg. Credit",	"Resource Share",	NULL,				NULL},
        {"Project",	"Application",	"Name",			"CPU time",		"Progress",			"To Completion",	"Status"},
        {"Project",	"File",			"Progress",		"Size",			"Time",				"Direction",		NULL},
        {"Project",	"Time",			"Message",		NULL,			NULL,				NULL,				NULL}
};

double GetDiskSize();
double GetDiskFree();

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
    if (nbytes > 1e12) {
        str->Format("%0.2f TB", nbytes/(1024.0*1024*1024*1024));
    } else if (nbytes > 1e9) {
        str->Format("%0.2f GB", nbytes/(1024.0*1024*1024));
    } else if (nbytes > 1e6) {
        str->Format("%0.2f MB", nbytes/(1024.0*1024));
    } else if (nbytes > 1e3) {
        str->Format("%0.2f KB", nbytes/(1024.0));
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
	CWnd* parent = GetParent();
	if(parent) {
		MapWindowPoints(parent,&point,1);
		WPARAM wParam = nFlags;
		LPARAM lParam = MAKELPARAM(point.x, point.y);
		parent->SendMessage(WM_LBUTTONDOWN, wParam, lParam);
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
	CWnd* parent = GetParent();
	if(parent) {
		MapWindowPoints(parent,&point,1);
		WPARAM wParam = nFlags;
		LPARAM lParam = MAKELPARAM(point.x, point.y);
		parent->SendMessage(WM_LBUTTONUP, wParam, lParam);
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
	CWnd* parent = GetParent();
	if(parent) {
		MapWindowPoints(parent,&point,1);
		WPARAM wParam = nFlags;
		LPARAM lParam = MAKELPARAM(point.x, point.y);
		parent->SendMessage(WM_RBUTTONDOWN, wParam, lParam);
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
	CWnd* parent = GetParent();
	if(parent) {
		MapWindowPoints(parent,&point,1);
		WPARAM wParam = nFlags;
		LPARAM lParam = MAKELPARAM(point.x, point.y);
		parent->SendMessage(WM_RBUTTONDOWN, wParam, lParam);
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
	CWnd* parent = GetParent();
	if(parent) {
		MapWindowPoints(parent,&point,1);
		WPARAM wParam = nFlags;
		LPARAM lParam = MAKELPARAM(point.x, point.y);
		parent->SendMessage(WM_RBUTTONUP, wParam, lParam);
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
	CString empty;
	m_ProjectURLs.InsertAt(nItem, empty);
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

	CString str;
	CProgressCtrl* progCtrl = NULL;

	// go through all the subitems and see if they have a progess control
	for(si = 0; si < GetHeaderCtrl()->GetItemCount(); si ++) {
		str.Format("%d:%d", nItem, si);
		progCtrl = NULL;
		m_Progs.Lookup(str, (CObject*&)progCtrl);
		if(progCtrl) {
			m_Progs.RemoveKey(str);
			delete progCtrl;
		}
	}


	// move other progress controls up
	for(i = nItem + 1; i < GetItemCount(); i ++) {
		for(si = 0; si < GetHeaderCtrl()->GetItemCount(); si ++) {
			str.Format("%d:%d", i, si);
			progCtrl = NULL;
			m_Progs.Lookup(str, (CObject*&)progCtrl);
			if(progCtrl) {
				m_Progs.RemoveKey(str);
				str.Format("%d:%d", i - 1, si);
				m_Progs.SetAt(str, progCtrl);
			}
		}
	}

	return CListCtrl::DeleteItem(nItem);
}

//////////
// CProgressListCtrl::SetItemProgress
// arguments:	item: item index
//				subitem: item's subitem to set progress for
//				prog: position to set progress control
// returns:		void
// function:	sets the position of a progress control for a given
//				item and subitem; if there is none there, creates a new 
//				one, otherwise sets the progress of the one it finds.
void CProgressListCtrl::SetItemProgress(int item, int subitem, int prog)
{
	CRect rt;
	CString str;
	CProgressCtrl* progCtrl = NULL;
	if(prog < 0) prog = 0;
	if(prog > 100) prog = 100;

	// lookup the position of the progress control
	str.Format("%d:%d", item, subitem);
	m_Progs.Lookup(str, (CObject*&)progCtrl);
	if(progCtrl) {

		// found, so just update it's progress
		progCtrl->SetPos(prog);
	} else {

		// not found, create one and put it in the map
		GetSubItemRect(item, subitem, LVIR_BOUNDS, rt);
		progCtrl = new CProgressBarCtrl();
		progCtrl->Create(PBS_SMOOTH|WS_CHILD|WS_VISIBLE, rt, this, 0);
		progCtrl->SetPos(prog);
		m_Progs.SetAt(str, progCtrl);
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
	int item, subitem;
	CRect rt, hrt;
	CString str;
	CProgressCtrl* progCtrl = NULL;
    GetHeaderCtrl()->GetClientRect(hrt);

	// iterate through each progress control
	POSITION pos = m_Progs.GetStartPosition();
	while (pos != NULL) {

		// look at the progress control and move it
		m_Progs.GetNextAssoc(pos, str, (CObject*&)progCtrl);
		sscanf(str.GetBuffer(0), "%d:%d", &item, &subitem);
		GetSubItemRect(item, subitem, LVIR_BOUNDS, rt);
		rt.top ++; rt.left ++;
		rt.bottom --; rt.right --;

		// if it's over the header, move it to where it can't be seen
		if(rt.top < hrt.bottom) {
			rt.top = -10;
			rt.bottom = 0;
		}
		progCtrl->MoveWindow(rt, false);
	}
}

//////////
// CProgressListCtrl::SwapItems
// arguments:	i1: index of the first item to swap
//				i2: index of the second item to swap
// returns:		void
// function:	swaps all relevant information of the two given items. this
//				includes text and progress controls of subitems and 
//				item data
void CProgressListCtrl::SwapItems(int i1, int i2)
{
	int nCols = GetHeaderCtrl()->GetItemCount();
	CProgressCtrl* progCtrl1;
	CProgressCtrl* progCtrl2;
	CString txt1, txt2;
	DWORD data1, data2;
	int si;

	// check item indicies
	if(i1 >= GetItemCount() || i2 >= GetItemCount()) {
		return;
	}

	// swap url data
	bool ok1 = false, ok2 = false;
	CString stemp1, stemp2, sempty;
	if(i1 < m_ProjectURLs.GetSize()) {
		stemp1 = m_ProjectURLs.GetAt(i1);
		ok1 = true;
	}
	if(i2 < m_ProjectURLs.GetSize()) {
		stemp2 = m_ProjectURLs.GetAt(i2);
		ok2 = true;
	}
	if(ok1) {
		m_ProjectURLs.SetAtGrow(i2, stemp1);
	} else {
		m_ProjectURLs.SetAtGrow(i2, sempty);
	}
	if(ok2) {
		m_ProjectURLs.SetAtGrow(i1, stemp2);
	} else {
		m_ProjectURLs.SetAtGrow(i1, sempty);
	}

	// swap color data
	ok1 = false;
	ok2 = false;
	COLORREF ctemp1, ctemp2, cempty = RGB(0, 0, 0);
	if(i1 < m_ItemColors.GetSize()) {
		ctemp1 = m_ItemColors.GetAt(i1);
		ok1 = true;
	}
	if(i2 < m_ItemColors.GetSize()) {
		ctemp2 = m_ItemColors.GetAt(i2);
		ok2 = true;
	}
	if(ok1) {
		m_ItemColors.SetAtGrow(i2, ctemp1);
	} else {
		m_ItemColors.SetAtGrow(i2, cempty);
	}
	if(ok2) {
		m_ItemColors.SetAtGrow(i1, ctemp2);
	} else {
		m_ItemColors.SetAtGrow(i1, cempty);
	}

	// swap indices
	data1 = GetItemData(i1);
	data2 = GetItemData(i2);
	SetItemData(i1, data2);
	SetItemData(i2, data1);
	for(si = 0; si < nCols; si ++) {

		// swap text
		txt1 = GetItemText(i1, si);
		txt2 = GetItemText(i2, si);
		SetItemText(i1, si, txt2);
		SetItemText(i2, si, txt1);

		// swap progress control if found
		txt1.Format("%d:%d", i1, si);
		txt2.Format("%d:%d", i2, si);
		progCtrl1 = NULL;
		progCtrl2 = NULL;
		m_Progs.Lookup(txt1, (CObject*&)progCtrl1);
		m_Progs.Lookup(txt2, (CObject*&)progCtrl2);
		if(progCtrl1) {
			m_Progs.RemoveKey(txt2);
			m_Progs.SetAt(txt2, (CObject*&)progCtrl1);
		}
		if(progCtrl2) {
			m_Progs.RemoveKey(txt1);
			m_Progs.SetAt(txt1, (CObject*&)progCtrl2);
		}
	}
}

//////////
// CProgressListCtrl::Sort
// arguments:	si: subitem to sort by
//				order: the order to sort by, either SORT_ASCEND or SORT_DESCEND
// returns:		void
// function:	sorts items by the given subitem into the given order. if there
//				is a progress control, converts the position to a string for 
//				comparison, otherwise sorts by the string at that subitem.
void CProgressListCtrl::Sort(int si, int order)
{
	int i, j, min, z;
	CString stri, strj;
	CProgressCtrl* progi = NULL;
	CProgressCtrl* progj = NULL;

	// check subitem is in bounds
	if(si >= GetHeaderCtrl()->GetItemCount()) {
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
				stri.Format("%d:%d", i, si);
				strj.Format("%d:%d", j, si);
				progi = NULL;
				progj = NULL;
				m_Progs.Lookup(stri, (CObject*&)progi);
				m_Progs.Lookup(strj, (CObject*&)progj);
				if(progi) {
					stri.Format("%0.3d", progi->GetPos());
				} else {
					stri = GetItemText(i, si);
				}
				if(progj) {
					strj.Format("%0.3d", progj->GetPos());
				} else {
					strj = GetItemText(j, si);
				}
				if(order == SORT_ASCEND && strcmp(stri, strj) > 0) min = j;
				if(order == SORT_DESCEND && strcmp(stri, strj) < 0) min = j;
			}
			SwapItems(i, min);
		}
	}
	RepositionProgress();
}

//////////
// CProgressListCtrl::SwapColumnVisibility
// arguments:	col: the column whose visibility to swap
// returns:		void
// function:	if the given column is visible, makes it invisible, otherwise
//				makes it visible.
void CProgressListCtrl::SwapColumnVisibility(int col)
{
	int oldw;
	CHeaderCtrl* header = GetHeaderCtrl();
	if(header && col < header->GetItemCount()) {
		oldw = m_ColWidths.GetAt(col);
		if(oldw < 0) {
			CListCtrl::SetColumnWidth(col, -1 * (oldw - 1));
			m_ColWidths.SetAtGrow(col, -1 * (oldw - 1));
			m_PopupMenu.CheckMenuItem(col, MF_CHECKED);
		} else {
			CListCtrl::SetColumnWidth(col, 0);
			m_ColWidths.SetAtGrow(col, -1 * (oldw + 1));
			m_PopupMenu.CheckMenuItem(col, MF_UNCHECKED);
		}
	}
}

//////////
// CProgressListCtrl::SetItemColor
// arguments:	nItem: item whose color is to be set
//				clr: the new color
// returns:		void
// function:	causes an item to be displayed in the given color
void CProgressListCtrl::SetItemColor(int nItem, COLORREF clr)
{
	m_ItemColors.SetAtGrow(nItem, clr);
}

//////////
// CProgressListCtrl::SetProjectURL
// arguments:	nItem: the item to set the url for
//				url: the url for the link
// returns:		void
// function:	sets the url for a project's link, causing the text of
//				the first subitem for the given item to be displayed
//				as a link
void CProgressListCtrl::SetProjectURL(int nItem, char* url)
{
	CString str;
	str.Format("%s", url);
	m_ProjectURLs.SetAtGrow(nItem, str);
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
		if(column_titles[GetDlgCtrlID()][i]) {
			m_PopupMenu.AppendMenu(MF_STRING, i, column_titles[GetDlgCtrlID()][i]);
			m_PopupMenu.CheckMenuItem(i, MF_CHECKED);
		}
	}

	// subclass header
	CHeaderCtrl* header = GetHeaderCtrl();
	if(header) {
		HWND hWnd = header->GetSafeHwnd();
		if(hWnd) {
			m_Header.SubclassWindow(hWnd);
		}
	}

	m_OldFont = NULL;
	m_iSort = 0;
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
		CFont* newFont = new CFont;
		newFont->CreateFontIndirect(&lf);
		m_OldFont = cdc->SelectObject(newFont);
		*pResult = CDRF_NOTIFYPOSTPAINT;
	} else if(pLVCD->nmcd.dwDrawStage == (CDDS_ITEMPOSTPAINT | CDDS_SUBITEM)) {

		// after subitem, restore font
		CDC* cdc = CDC::FromHandle(pLVCD->nmcd.hdc);
		m_OldFont = cdc->SelectObject(m_OldFont);
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
	CString str;
	CProgressCtrl* progCtrl = NULL;

	// iterate through each progress control
	POSITION pos = m_Progs.GetStartPosition();
	while (pos != NULL) {

		// remove the control and delete it
		m_Progs.GetNextAssoc(pos, str, (CObject*&)progCtrl);
		m_Progs.RemoveKey(str);
		delete progCtrl;
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

	CWnd* parent = GetParent();
	if(parent) {
		MapWindowPoints(parent,&point,1);
		WPARAM wParam = nFlags;
		LPARAM lParam = MAKELPARAM(point.x, point.y);
		parent->SendMessage(WM_RBUTTONDOWN, wParam, lParam);
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
		if(newSort == abs(m_iSort)) {
			m_iSort *= -1;
			if(m_iSort < 0) Sort(abs(m_iSort)-1, SORT_DESCEND);
			else Sort(abs(m_iSort)-1, SORT_ASCEND);
		} else {
			m_iSort = newSort;
			Sort(abs(m_iSort)-1, SORT_ASCEND);
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
		int col = phdn->iItem;
		if(m_ColWidths.GetAt(col) < 0) {
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
	m_total = 0;
	m_Font = NULL;
}

//////////
// CPieChartCtrl::AddPiece
// arguments:	label: label for the piece
//				color: color of the piece
//				percent: percent of the pie the piece takes
//				base: sets base units for whole pie
// returns:		void
// function:	adds a piece to the pie, truncating at 100%, starting at
//				index 0, the first piece is the base piece that changes
//				size to complete the pie and its percent is meaningless
void CPieChartCtrl::AddPiece(LPTSTR label, COLORREF color, double value)
{
	if(value < 0) value = 0;
	m_Values.Add(value);
	m_Colors.Add(color);
	CString str;
	str.Format("%s", label);
	m_Labels.Add(str);
}

//////////
// CPieChartCtrl::SetPiece
// arguments:	index: index of piece to change
//				percent: percent of the pie the piece takes
// returns:		void
// function:	changes the piece's value
void CPieChartCtrl::SetPiece(int index, double value)
{
	if(index < 0 || index >= m_Values.GetSize()) return;
	if(value < 0) value = 0;
	m_Values.SetAt(index, value);
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
// arguments:	dc: pointer to dc to draw in
//				xStartAngle: starting angle of piece
//				xEndAngle: ending angle of piece
// returns:		void
// function:	draws a pie piece in the dc
void CPieChartCtrl::DrawPiePiece(CDC* dc, double xStartAngle, double xEndAngle)
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
	int maxmajor = dc->GetDeviceCaps(HORZRES) * PIE_MAJOR_MAX;
	int maxminor = dc->GetDeviceCaps(VERTRES) * PIE_MINOR_MAX;
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
	dc->FillRgn(&rpie, dc->GetCurrentBrush());

	// outline
	dc->MoveTo(rt.CenterPoint());
	dc->LineTo(poly[1]);
	dc->Arc(&rt, poly[1], poly[7]);
	dc->MoveTo(poly[7]);
	dc->LineTo(rt.CenterPoint());

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
		dc->FillRgn(&rdepth, dc->GetCurrentBrush());
		
		// ouline
		dc->Arc(&rt, pt1, pt2);
		dc->Arc(&rt2, pt3, pt4);
		dc->MoveTo(pt1);
		dc->LineTo(pt3);
		dc->MoveTo(pt4);
		dc->LineTo(pt2);

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
//				rad: radius of circle
//				angle: angle of radius
//				pt: pointer to CPoint to put result in
// returns:		void
// function:	calculates the point on the circle at the given angle
void CPieChartCtrl::CirclePoint(CPoint* center, int rad, double angle, CPoint* pt)
{
	pt->x = center->x + rad * cos(angle * (PI / 180));
	pt->y = center->y - rad * sin(angle * (PI / 180));
}

//////////
// CPieChartCtrl::EllipsePoint
// arguments:	rt: pointer to rect of ellipse
//				angle: angle of radius
//				pt: pointer to CPoint to put result in
// returns:		void
// function:	calculates the point on the ellipse in the given rect at
//				the given angle
void CPieChartCtrl::EllipsePoint(CRect* rt, double angle, CPoint* pt)
{
	pt->x = rt->CenterPoint().x + (rt->Width() / 2) * cos(angle * (PI / 180));
	pt->y = rt->CenterPoint().y - (rt->Height() / 2) * sin(angle * (PI / 180));
}

//////////
// CPieChartCtrl::SetFont
// arguments:	pcf: pointer to font to set
// returns:		void
// function:	sets this control's font
void CPieChartCtrl::SetFont(CFont* pcf)
{
	m_Font = pcf;
}

//////////
// CPieChartCtrl::SetTotal
// arguments:	tag: the string to be set as the tag
// returns:		void
// function:	sets the tag for data, which is a string that will be displayed
//				after the numbers in the label
void CPieChartCtrl::SetTotal(double total)
{
	if(total >= 0) m_total = total;
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
	if(m_Values.GetSize() == 0) return;

	// gdi objects needed
	CClientDC cdc(this);
	CRect rt;
	CDC memdc;
	CBitmap membmp;
	CBrush cb;
	CPen cp;
	CBitmap* oldbmp = NULL;
	CBrush* oldbrush = NULL;
	CPen* oldpen = NULL;
	CFont* oldfont = NULL;

	// create offscreen buffer
	GetClientRect(&rt);
	memdc.CreateCompatibleDC(&cdc);
	membmp.CreateCompatibleBitmap(&cdc, rt.Width(), rt.Height());
	cp.CreatePen(PS_SOLID, 0, RGB(0, 0, 0));

	// select gdi objects
	oldbmp = memdc.SelectObject(&membmp);
	oldpen = memdc.SelectObject(&cp);
	oldfont = memdc.SelectObject(m_Font);
	memdc.FillSolidRect(&rt, RGB(255, 255, 255));

	// go through each percent and draw its label and pie
	double sofar = 0;
	CRect wndrect;
	CRect textrect;
	GetWindowRect(&wndrect);
	for(int i = 0; i < m_Values.GetSize(); i ++) {
		cb.CreateSolidBrush(m_Colors.GetAt(i));
		oldbrush = memdc.SelectObject(&cb);

		int texti = i;
		if(texti == 2) texti = 3;
		else if(texti == 3) texti = 2;

		// display color box and label
		if(PIE_BUFFER + 20 + texti * 20 < wndrect.Height() / 2) {
			textrect.SetRect(PIE_BUFFER + 0, PIE_BUFFER + texti * 20 + 2, PIE_BUFFER + 10, PIE_BUFFER + 20 + texti * 20 - 2);
			memdc.FillRect(&textrect, &cb);
			memdc.MoveTo(textrect.left, textrect.top);
			memdc.LineTo(textrect.right, textrect.top);
			memdc.LineTo(textrect.right, textrect.bottom);
			memdc.LineTo(textrect.left, textrect.bottom);
			memdc.LineTo(textrect.left, textrect.top);
			textrect.SetRect(PIE_BUFFER + 15, PIE_BUFFER + texti * 20, wndrect.Width() - PIE_BUFFER, PIE_BUFFER + 20 + texti * 20);
			CString sbytes;
			GetByteString(m_Values.GetAt(i), &sbytes);
			CString str;
			str.Format("%s (%s)", m_Labels.GetAt(i).GetBuffer(0), sbytes.GetBuffer(0));
			memdc.DrawText(str, textrect, DT_SINGLELINE|DT_VCENTER|DT_LEFT);
		}

		// display pie piece
		double percent = 0;
		if(m_total > 0) percent = m_Values.GetAt(i) / m_total;
		DrawPiePiece(&memdc, sofar * 360, (sofar + percent) * 360);
		sofar += percent;

		memdc.SelectObject(oldbrush);
		cb.DeleteObject();
	}

	// copy offscreen buffer to screen
	cdc.BitBlt(0, 0, rt.Width(), rt.Height(), &memdc, 0, 0, SRCCOPY);

	// clean up
	memdc.SelectObject(oldbmp);
	memdc.SelectObject(oldpen);
	memdc.SelectObject(oldfont);
	cp.DeleteObject();
	membmp.DeleteObject();
	cb.DeleteObject();
	memdc.DeleteDC();
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
	CWnd* boincWnd;
	boincWnd = CWnd::FindWindow(NULL, WND_TITLE);
	if(boincWnd) {
		if(boincWnd->GetStyle() & WS_EX_TOOLWINDOW) {
			boincWnd->ShowWindow(SW_SHOW);
			boincWnd->SetForegroundWindow();
			return FALSE;
		}
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
    ON_COMMAND(ID_FILE_CLEARINACTIVE, OnCommandFileClearInactive)
    ON_COMMAND(ID_FILE_CLEARMESSAGES, OnCommandFileClearMessages)
    ON_COMMAND(ID_FILE_HIDE, OnCommandHide)
    ON_COMMAND(ID_FILE_SUSPEND, OnCommandSuspend)
    ON_COMMAND(ID_FILE_EXIT, OnCommandExit)
    ON_COMMAND(ID_SETTINGS_LOGIN, OnCommandSettingsLogin)
    ON_COMMAND(ID_SETTINGS_QUIT, OnCommandSettingsQuit)
    ON_COMMAND(ID_SETTINGS_PROXYSERVER, OnCommandSettingsProxyServer)
    ON_COMMAND(ID_HELP_ABOUT, OnCommandHelpAbout)
    ON_COMMAND(ID_PROJECT_RELOGIN, OnCommandProjectRelogin)
    ON_COMMAND(ID_PROJECT_QUIT, OnCommandProjectQuit)
    ON_COMMAND(ID_STATUSICON_HIDE, OnCommandHide)
    ON_COMMAND(ID_STATUSICON_SUSPEND, OnCommandSuspend)
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
    CRect rect(0, 0, 600, 400);
    CalcWindowRect(&rect);
    SetWindowPos(NULL, 0, 0, rect.Width(), rect.Height(), SWP_NOZORDER|SWP_NOMOVE|SWP_NOREDRAW);
}

//////////
// CMainWindow::TimerProc
// arguments:	h: the window assosciated with the timer
//				msg: the WM_TIMER message
//				id: the timer's id
//				time: milliseconds since system started
// returns:		void
// function:	checks idle time, updates client state, flushed output streams,
//				and updates gui display.
void CALLBACK CMainWindow::TimerProc(HWND h, UINT msg, UINT id, DWORD time)
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
// arguments:	cs: pointer to the client state for the gui to display
// returns:		void
// function:	syncronizes list controls with vectors in client state
//				and displays them.
void CMainWindow::UpdateGUI(CLIENT_STATE* cs)
{
	CString buf;
	int i;

	// display projects
	float totalres = 0;
	Syncronize(&m_ProjectListCtrl, (vector<void*>*)(&cs->projects));
	for(i = 0; i < cs->projects.size(); i ++) {
		totalres += cs->projects[i]->resource_share;
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
		buf.Format("%0.2f", pr->user_total_credit);
		m_ProjectListCtrl.SetItemText(i, 2, buf);

		// avg credit
		buf.Format("%0.2f", pr->user_expavg_credit);
		m_ProjectListCtrl.SetItemText(i, 3, buf);

		// resource share
		if(totalres <= 0) {
			m_ProjectListCtrl.SetItemProgress(i, 4, 100);
		} else {
			m_ProjectListCtrl.SetItemProgress(i, 4, (100 * pr->resource_share) / totalres);
		}
	}

	// update results
	Syncronize(&m_ResultListCtrl, (vector<void*>*)(&cs->results));
	for(i = 0; i < m_ResultListCtrl.GetItemCount(); i ++) {
		RESULT* re = (RESULT*)m_ResultListCtrl.GetItemData(i);
		if(!re) {
			m_ResultListCtrl.SetItemColor(i, RGB(128, 128, 128));
			m_ResultListCtrl.SetItemProgress(i, 4, 100);
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
			cur_cpu = 0;
		}
		int cpuhour = (int)(cur_cpu / (60 * 60));
		int cpumin = (int)(cur_cpu / 60) % 60;
		int cpusec = (int)(cur_cpu) % 60;
		buf.Format("%0.2d:%0.2d:%0.2d", cpuhour, cpumin, cpusec);
		m_ResultListCtrl.SetItemText(i, 3, buf);

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
		buf.Format("%0.2d:%0.2d:%0.2d", cpuhour, cpumin, cpusec);
		m_ResultListCtrl.SetItemText(i, 5, buf);

		// status
		switch(re->state) {
			case RESULT_NEW:
				buf.Format("%s", "New"); break;
			case RESULT_FILES_DOWNLOADED:
				if (at)
					buf.Format("%s", "Running");
				else
					buf.Format("%s", "Ready to run");
				break;
			case RESULT_COMPUTE_DONE:
				buf.Format("%s", "Computation done"); break;
			case RESULT_READY_TO_ACK:
				buf.Format("%s", "Results uploaded"); break;
			case RESULT_SERVER_ACK:
				buf.Format("%s", "Acknowledged"); break;
			default:
				buf.Format("%s", "Error: invalid state"); break;
		}
		m_ResultListCtrl.SetItemText(i, 6, buf);
	}

	// update xfers
	Syncronize(&m_XferListCtrl, (vector<void*>*)(&cs->pers_xfers->pers_file_xfers));
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
			buf.Format("%0.0f/%0.0fKB", fi->fxp->nbytes_xfered / 1024, fi->fip->nbytes / 1024);
		} else {
			buf.Format("%0.0f/%0.0fKB", 0, fi->fip->nbytes / 1024);
		}
		m_XferListCtrl.SetItemText(i, 3, buf.GetBuffer(0));

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
		buf.Format("%0.2d:%0.2d:%0.2d", xhour, xmin, xsec);
		m_XferListCtrl.SetItemText(i, 4, buf.GetBuffer(0));

		// direction
		m_XferListCtrl.SetItemText(i, 5, fi->fip->generated_locally?"Upload":"Download");
	}

	// update usage
	double disktotal = GetDiskSize();
	double diskfree = GetDiskFree();
	double diskused = disktotal - diskfree;
	double diskallow; gstate.allowed_disk_usage(diskallow);
	double diskusage; gstate.current_disk_usage(diskusage);
	m_UsagePieCtrl.SetTotal(disktotal);
	m_UsagePieCtrl.SetPiece(0, diskfree - (diskallow - diskusage)); // Free not available for BOINC
	m_UsagePieCtrl.SetPiece(1, diskused - diskusage); // Space used
	m_UsagePieCtrl.SetPiece(2, diskusage); // Used by BOINC
	m_UsagePieCtrl.SetPiece(3, diskallow - diskusage); // Space available to BOINC

	// make icon flash if needed
	if(m_Message) {
		if(m_IconState == ICON_NORMAL) {
			SetStatusIcon(ICON_HIGHLIGHT);
		} else if(m_IconState == ICON_HIGHLIGHT) {
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
void CMainWindow::MessageUser(char* project, char* message, char* priority)
{
	if(!m_MessageListCtrl.GetSafeHwnd()) return;

	m_MessageListCtrl.InsertItem(0, project);

	CTime curTime = CTime::GetCurrentTime();
	CString timeStr;
	timeStr = curTime.Format("%c");
	m_MessageListCtrl.SetItemText(0, 1, timeStr);

	m_MessageListCtrl.SetItemText(0, 2, message);
	
	// set status icon to flash
	if(!strcmp(priority, "high") && (m_TabCtrl.GetCurSel() != MESSAGE_ID || GetForegroundWindow() != this)) {
		m_Message = true;
	}
}

//////////
// CMainWindow::IsSuspended
// arguments:	void
// returns:		true if the window is suspended, false otherwise
// function:	tells if the window is suspended
BOOL CMainWindow::IsSuspended()
{
	return m_Suspend;
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
	if(dwMessage == m_IconState) return;
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
		if(m_IconState == ICON_OFF) {
			Shell_NotifyIcon(NIM_ADD, &icon_data);
		} else {
			Shell_NotifyIcon(NIM_MODIFY, &icon_data);
		}
	} else if(dwMessage == ICON_HIGHLIGHT) {
		icon_data.hIcon = g_myApp.LoadIcon(IDI_ICONHIGHLIGHT);
		if(m_IconState == ICON_OFF) {
			Shell_NotifyIcon(NIM_ADD, &icon_data);
		} else {
			Shell_NotifyIcon(NIM_MODIFY, &icon_data);
		}
	}
	m_IconState = dwMessage;
}

//////////
// CMainWindow::SaveUserSettings
// arguments:	void
// returns:		void
// function:	saves relevant user settings to boinc.ini
void CMainWindow::SaveUserSettings()
{
	char path[256];
	CString keybuf, valbuf;
	GetCurrentDirectory(256, path);
	strcat(path, "\\boinc.ini");
	int colorder[MAX_COLS];
	int i;

	// save project columns
	m_ProjectListCtrl.GetColumnOrderArray(colorder, PROJECT_COLS);
	WritePrivateProfileStruct("HEADERS", "projects-order", colorder, sizeof(colorder), path);
	for(i = 0; i < m_ProjectListCtrl.GetHeaderCtrl()->GetItemCount(); i ++) {
		keybuf.Format("projects-%d", i);
		valbuf.Format("%d", m_ProjectListCtrl.GetColumnWidth(i));
		WritePrivateProfileString("HEADERS", keybuf.GetBuffer(0), valbuf.GetBuffer(0), path);
	}

	// save result columns
	m_ResultListCtrl.GetColumnOrderArray(colorder, RESULT_COLS);
	WritePrivateProfileStruct("HEADERS", "results-order", colorder, sizeof(colorder), path);
	for(i = 0; i < m_ResultListCtrl.GetHeaderCtrl()->GetItemCount(); i ++) {
		keybuf.Format("results-%d", i);
		valbuf.Format("%d", m_ResultListCtrl.GetColumnWidth(i));
		WritePrivateProfileString("HEADERS", keybuf.GetBuffer(0), valbuf.GetBuffer(0), path);
	}

	// save xfer columns
	m_XferListCtrl.GetColumnOrderArray(colorder, XFER_COLS);
	WritePrivateProfileStruct("HEADERS", "xfers-order", colorder, sizeof(colorder), path);
	for(i = 0; i < m_XferListCtrl.GetHeaderCtrl()->GetItemCount(); i ++) {
		keybuf.Format("xfers-%d", i);
		valbuf.Format("%d", m_XferListCtrl.GetColumnWidth(i));
		WritePrivateProfileString("HEADERS", keybuf.GetBuffer(0), valbuf.GetBuffer(0), path);
	}

	// save xfer columns
	m_MessageListCtrl.GetColumnOrderArray(colorder, MESSAGE_COLS);
	WritePrivateProfileStruct("HEADERS", "messages-order", colorder, sizeof(colorder), path);
	for(i = 0; i < m_MessageListCtrl.GetHeaderCtrl()->GetItemCount(); i ++) {
		keybuf.Format("messages-%d", i);
		valbuf.Format("%d", m_MessageListCtrl.GetColumnWidth(i));
		WritePrivateProfileString("HEADERS", keybuf.GetBuffer(0), valbuf.GetBuffer(0), path);
	}
}

//////////
// CMainWindow::LoadUserSettings
// arguments:	void
// returns:		void
// function:	loads relevant user settings from boinc.ini
void CMainWindow::LoadUserSettings()
{
	char path[256];
	CString keybuf;
	GetCurrentDirectory(256, path);
	strcat(path, "\\boinc.ini");
	int i, intbuf;
	int colorder[MAX_COLS];

	// load project columns
	if(GetPrivateProfileStruct("HEADERS", "projects-order", colorder, sizeof(colorder), path)) {
		m_ProjectListCtrl.SetColumnOrderArray(PROJECT_COLS, colorder);
	}
	for(i = 0; i < m_ProjectListCtrl.GetHeaderCtrl()->GetItemCount(); i ++) {
		keybuf.Format("projects-%d", i);
		intbuf = GetPrivateProfileInt("HEADERS", keybuf.GetBuffer(0), DEF_COL_WIDTH, path);
		m_ProjectListCtrl.SetColumnWidth(i, intbuf);
	}

	// load result columns
	if(GetPrivateProfileStruct("HEADERS", "results-order", colorder, sizeof(colorder), path)) {
		m_ResultListCtrl.SetColumnOrderArray(RESULT_COLS, colorder);
	}
	for(i = 0; i < m_ResultListCtrl.GetHeaderCtrl()->GetItemCount(); i ++) {
		keybuf.Format("results-%d", i);
		intbuf = GetPrivateProfileInt("HEADERS", keybuf.GetBuffer(0), DEF_COL_WIDTH, path);
		m_ResultListCtrl.SetColumnWidth(i, intbuf);
	}

	// load xfer columns
	if(GetPrivateProfileStruct("HEADERS", "xfers-order", colorder, sizeof(colorder), path)) {
		m_XferListCtrl.SetColumnOrderArray(XFER_COLS, colorder);
	}
	for(i = 0; i < m_XferListCtrl.GetHeaderCtrl()->GetItemCount(); i ++) {
		keybuf.Format("xfers-%d", i);
		intbuf = GetPrivateProfileInt("HEADERS", keybuf.GetBuffer(0), DEF_COL_WIDTH, path);
		m_XferListCtrl.SetColumnWidth(i, intbuf);
	}

	// load message columns
	if(GetPrivateProfileStruct("HEADERS", "messages-order", colorder, sizeof(colorder), path)) {
		m_MessageListCtrl.SetColumnOrderArray(MESSAGE_COLS, colorder);
	}
	for(i = 0; i < m_MessageListCtrl.GetHeaderCtrl()->GetItemCount(); i ++) {
		keybuf.Format("messages-%d", i);
		int width = DEF_COL_WIDTH;
		if(i == 1) width *= 1.5;
		if(i == 2) width *= 4;
		intbuf = GetPrivateProfileInt("HEADERS", keybuf.GetBuffer(0), width, path);
		m_MessageListCtrl.SetColumnWidth(i, intbuf);
	}
}

//////////
// CMainWindow::GetUserIdleTime
// arguments:	void
// returns:		time the user has been idle in milliseconds
// function:	calls a dll function to determine the the user's idle time
DWORD CMainWindow::GetUserIdleTime()
{
	if(m_IdleDll) {
		GetFn fn;
		fn = (GetFn)GetProcAddress(m_IdleDll, "IdleTrackerGetLastTickCount");
		if(fn) {
			return GetTickCount() - fn();
		} else {
			TermFn tfn;
			tfn = (TermFn)GetProcAddress(m_IdleDll, "IdleTrackerTerm");
			if(tfn) {
				tfn();
			}
			FreeLibrary(m_IdleDll);
			m_IdleDll = NULL;
		}
	}
	return 0;
}

//////////
// CMainWindow::Syncronize
// arguments:	prog: pointer to a progress list control
//				vect: pointer to a vector of pointers
// returns:		void
// function:	first, goes through the vector and adds items to the list
//				control for any pointers it does not already contain, then
//				goes through the list control and removes any pointers the
//				vector does not contain.
void CMainWindow::Syncronize(CProgressListCtrl* prog, vector<void*>* vect)
{
	int i, j;

	// add items to list that are not already in it
	for(i = 0; i < vect->size(); i ++) {
		void* item = (*vect)[i];
		BOOL contained = false;
		for(j = 0; j < prog->GetItemCount(); j ++) {
			if((DWORD)item == prog->GetItemData(j)) {
				contained = true;
				break;
			}
		}
		if(!contained) {
			prog->InsertItem(i, "");
			prog->SetItemData(i, (DWORD)item);
		}
	}

	// remove items from list that are not in vector
	// now just set the pointer to NULL but leave the item in the list
	for(i = 0; i < prog->GetItemCount(); i ++) {
		DWORD item = prog->GetItemData(i);
		BOOL contained = false;
		for(j = 0; j < vect->size(); j ++) {
			if(item == (DWORD)(*vect)[j]) {
				contained = true;
				break;
			}
		}
		if(!contained) {
			prog->SetItemData(i, (DWORD)NULL);
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
    int retval = dlg.DoModal();
	if(retval == IDOK) {
		CString str;
		if(strcmp(gstate.projects[dlg.m_sel]->project_name, "")) {
			str.Format("Are you sure you want to quit the project %s?", gstate.projects[dlg.m_sel]->project_name);
		} else {
			str.Format("Are you sure you want to quit the project %s?", gstate.projects[dlg.m_sel]->master_url);
		}
		if(AfxMessageBox(str, MB_YESNO, 0) == IDYES) {
			gstate.quit_project(dlg.m_sel);
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
    int retval = dlg.DoModal();
	if(retval == IDOK) {
	    gstate.add_project(dlg.m_url.GetBuffer(0), dlg.m_auth.GetBuffer(0));
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
	int retval = dlg.DoModal();
}

//////////
// CMainWindow::OnCommandHelpAbout
// arguments:	void
// returns:		void
// function:	shows the about dialog box
void CMainWindow::OnCommandHelpAbout()
{
	CDialog dlg(IDD_ABOUTBOX);
	int retval = dlg.DoModal();
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
	if(m_ContextItem < 0 || m_ContextItem > m_ProjectListCtrl.GetItemCount()) return;
	PROJECT* toRelogin = (PROJECT*)m_ProjectListCtrl.GetItemData(m_ContextItem);
	m_ContextItem = -1;
	if(!toRelogin) return;

	// find project index
	int i;
	for(i = 0; i < gstate.projects.size(); i ++) {
		if(gstate.projects[i] == toRelogin) break;
	}
	if(i == gstate.projects.size()) return;

	// get info and relogin
    CLoginDialog dlg(IDD_LOGIN, gstate.projects[i]->master_url, gstate.projects[i]->authenticator);
    int retval = dlg.DoModal();
	if(retval == IDOK) {
		gstate.change_project(i, dlg.m_url.GetBuffer(0), dlg.m_auth.GetBuffer(0));
	}
}

//////////
// CMainWindow::OnCommandProjectQuit
// arguments:	void
// returns:		void
// function:	lets the user quit a project
void CMainWindow::OnCommandProjectQuit()
{
	if(m_ContextItem < 0 || m_ContextItem > m_ProjectListCtrl.GetItemCount()) return;
	PROJECT* toQuit = (PROJECT*)m_ProjectListCtrl.GetItemData(m_ContextItem);
	m_ContextItem = -1;
	if(!toQuit) return;

	// find project index
	int i;
	for(i = 0; i < gstate.projects.size(); i ++) {
		if(gstate.projects[i] == toQuit) break;
	}
	if(i == gstate.projects.size()) return;

	// confirm and quit
	CString str;
	if(strcmp(gstate.projects[i]->project_name, "")) {
		str.Format("Are you sure you want to quit the project %s?", gstate.projects[i]->project_name);
	} else {
		str.Format("Are you sure you want to quit the project %s?", gstate.projects[i]->master_url);
	}
	if(AfxMessageBox(str, MB_YESNO, 0) == IDYES) {
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
	CMenu* mainMenu;
	CMenu* fileMenu;
	mainMenu = GetMenu();
	if(mainMenu) {
		fileMenu = mainMenu->GetSubMenu(0);
	}
	if(IsWindowVisible()) {
		ShowWindow(SW_HIDE);
		if(fileMenu) {
			fileMenu->CheckMenuItem(ID_FILE_HIDE, MF_CHECKED);
		}
	} else {
		ShowWindow(SW_SHOW);
		if(fileMenu) {
			fileMenu->CheckMenuItem(ID_FILE_HIDE, MF_UNCHECKED);
		}
	}
}

//////////
// CMainWindow::OnCommandSuspend
// arguments:	void
// returns:		void
// function:	suspends or unsuspends the window
void CMainWindow::OnCommandSuspend()
{
	CMenu* mainMenu;
	CMenu* fileMenu;
	mainMenu = GetMenu();
	if(mainMenu) {
		fileMenu = mainMenu->GetSubMenu(0);
	}
	if(m_Suspend) {
		gstate.suspend_requested = false;
		m_Suspend = false;
		if(fileMenu) {
			fileMenu->CheckMenuItem(ID_FILE_SUSPEND, MF_UNCHECKED);
		}
	} else {
		gstate.suspend_requested = true;
		m_Suspend = true;
		if(fileMenu) {
			fileMenu->CheckMenuItem(ID_FILE_SUSPEND, MF_CHECKED);
		}
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
	if(m_IdleDll) {
		TermFn fn;
		fn = (TermFn)GetProcAddress(m_IdleDll, "IdleTrackerTerm");
		if(!fn) {
			MessageUser("", "Error in DLL \"boinc.dll\"", "low");
		} else {
			fn();
		}
		FreeLibrary(m_IdleDll);
		m_IdleDll = NULL;
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
	m_IconState = ICON_OFF;
	m_Message = false;
	m_Suspend = false;
	m_ContextItem = -1;

	// load main menu
	m_MainMenu.LoadMenu(IDR_MAINFRAME);
	SetMenu(&m_MainMenu);

	// create project list control
	m_ProjectListCtrl.Create(LVS_REPORT|WS_CHILD|WS_BORDER|WS_VISIBLE, CRect(0,0,0,0), this, PROJECT_ID);
	m_ProjectListCtrl.SetExtendedStyle(m_ProjectListCtrl.GetExtendedStyle()|LVS_EX_HEADERDRAGDROP|LVS_EX_FULLROWSELECT);
	for(int i = 0; i < PROJECT_COLS; i ++) {
		m_ProjectListCtrl.InsertColumn(i, column_titles[PROJECT_ID][i], LVCFMT_LEFT, DEF_COL_WIDTH);
	}

	// create result list control
	m_ResultListCtrl.Create(LVS_REPORT|WS_CHILD|WS_BORDER|WS_VISIBLE, CRect(0,0,0,0), this, RESULT_ID);
	m_ResultListCtrl.SetExtendedStyle(m_ResultListCtrl.GetExtendedStyle()|LVS_EX_HEADERDRAGDROP|LVS_EX_FULLROWSELECT);
	m_ResultListCtrl.ModifyStyle(WS_VISIBLE, 0);
	for(i = 0; i < RESULT_COLS; i ++) {
		m_ResultListCtrl.InsertColumn(i, column_titles[RESULT_ID][i], LVCFMT_LEFT, DEF_COL_WIDTH);
	}

	// create xfer list control
	m_XferListCtrl.Create(LVS_REPORT|WS_CHILD|WS_BORDER|WS_VISIBLE, CRect(0,0,0,0), this, XFER_ID);
	m_XferListCtrl.SetExtendedStyle(m_XferListCtrl.GetExtendedStyle()|LVS_EX_HEADERDRAGDROP|LVS_EX_FULLROWSELECT);
	m_XferListCtrl.ModifyStyle(WS_VISIBLE, 0);
	for(i = 0; i < XFER_COLS; i ++) {
		m_XferListCtrl.InsertColumn(i, column_titles[XFER_ID][i], LVCFMT_LEFT, DEF_COL_WIDTH);
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
		m_MessageListCtrl.InsertColumn(i, column_titles[MESSAGE_ID][i], LVCFMT_LEFT, width);
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
	m_UsagePieCtrl.AddPiece("Free space (Non-BOINC)", RGB(255, 0, 255), 0);
	m_UsagePieCtrl.AddPiece("Used space (Non-BOINC)", RGB(0, 0, 255), 0);
	m_UsagePieCtrl.AddPiece("Used space (BOINC)", RGB(0, 0, 192), 0);
	m_UsagePieCtrl.AddPiece("Free space (BOINC)", RGB(192, 0, 192), 0);

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
	CFont* pcf;
	pcf = m_ProjectListCtrl.GetFont();
	LOGFONT lf;
	ZeroMemory(&lf, sizeof(LOGFONT));
	pcf->GetLogFont(&lf);
	m_Font.CreateFontIndirect(&lf);
	m_TabCtrl.SetFont(&m_Font);
	m_UsagePieCtrl.SetFont(&m_Font);

	// remove button from taskbar and add status icon in taskbar
	ModifyStyleEx(WS_EX_APPWINDOW, WS_EX_TOOLWINDOW);
	SetStatusIcon(ICON_NORMAL);

	// take care of other things
	// 
    NetOpen();
	// Redirect stdout and stderr to files
    freopen("stdout.txt", "w", stdout);
    freopen("stderr.txt", "w", stderr);
	// Check what (if any) activities should be logged
    read_log_flags();
    int retval = gstate.init();
    if (retval) {
		OnCommandExit();
		return 0;
	}
    SetTimer(ID_TIMER, 1000, TimerProc);

	// load dll and start idle detection
	m_IdleDll = LoadLibrary("boinc.dll");
	if(!m_IdleDll) {
		MessageUser("", "Can't load \"boinc.dll\", will not be able to determine idle time", "high");
	} else {
		InitFn fn;
		fn = (InitFn)GetProcAddress(m_IdleDll, "IdleTrackerInit");
		if(!fn) {
			MessageUser("", "Error in DLL \"boinc.dll\", will not be able to determine idle time", "low");
			FreeLibrary(m_IdleDll);
			m_IdleDll = NULL;
		} else {
			if(!fn()) {
				MessageUser("", "Error in DLL \"boinc.dll\", will not be able to determine idle time", "low");
				FreeLibrary(m_IdleDll);
				m_IdleDll = NULL;
			}
		}
	}

	LoadUserSettings();
	UpdateGUI(&gstate);

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

		// make the selected control visible, all the rest invisible
		if(newTab == PROJECT_ID) {
			m_ProjectListCtrl.ModifyStyle(0, WS_VISIBLE);
			m_ResultListCtrl.ModifyStyle(WS_VISIBLE, 0);
			m_XferListCtrl.ModifyStyle(WS_VISIBLE, 0);
			m_MessageListCtrl.ModifyStyle(WS_VISIBLE, 0);
			m_UsagePieCtrl.ModifyStyle(WS_VISIBLE, 0);
			m_ProjectListCtrl.RedrawWindow(NULL, NULL, RDW_INVALIDATE|RDW_UPDATENOW|RDW_ERASE|RDW_FRAME);
		} else if(newTab == RESULT_ID) {
			m_ProjectListCtrl.ModifyStyle(WS_VISIBLE, 0);
			m_ResultListCtrl.ModifyStyle(0, WS_VISIBLE);
			m_XferListCtrl.ModifyStyle(WS_VISIBLE, 0);
			m_MessageListCtrl.ModifyStyle(WS_VISIBLE, 0);
			m_UsagePieCtrl.ModifyStyle(WS_VISIBLE, 0);
			m_ResultListCtrl.RedrawWindow(NULL, NULL, RDW_INVALIDATE|RDW_UPDATENOW|RDW_ERASE|RDW_FRAME);
		} else if(newTab == XFER_ID) {
			m_ProjectListCtrl.ModifyStyle(WS_VISIBLE, 0);
			m_ResultListCtrl.ModifyStyle(WS_VISIBLE, 0);
			m_XferListCtrl.ModifyStyle(0, WS_VISIBLE);
			m_MessageListCtrl.ModifyStyle(WS_VISIBLE, 0);
			m_UsagePieCtrl.ModifyStyle(WS_VISIBLE, 0);
			m_XferListCtrl.RedrawWindow(NULL, NULL, RDW_INVALIDATE|RDW_UPDATENOW|RDW_ERASE|RDW_FRAME);
		} else if(newTab == MESSAGE_ID) {
			m_ProjectListCtrl.ModifyStyle(WS_VISIBLE, 0);
			m_ResultListCtrl.ModifyStyle(WS_VISIBLE, 0);
			m_XferListCtrl.ModifyStyle(WS_VISIBLE, 0);
			m_MessageListCtrl.ModifyStyle(0, WS_VISIBLE);
			m_UsagePieCtrl.ModifyStyle(WS_VISIBLE, 0);
			if(m_Message) {
				m_Message = false;
				SetStatusIcon(ICON_NORMAL);
			}
			m_MessageListCtrl.RedrawWindow(NULL, NULL, RDW_INVALIDATE|RDW_UPDATENOW|RDW_ERASE|RDW_FRAME);
		} else if(newTab == USAGE_ID) {
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
	CMenu wholeMenu;
	CMenu* contextMenu = NULL;
	GetCursorPos(&point);
	CRect rt;
	CListCtrl* menuCtrl = NULL;
	int menuId = -1;
	if(m_ProjectListCtrl.IsWindowVisible()) {
		menuCtrl = &m_ProjectListCtrl;
		menuId = PROJECT_MENU;
	} else if(m_ResultListCtrl.IsWindowVisible()) {
		menuCtrl = &m_ResultListCtrl;
		menuId = RESULT_MENU;
	} else if(m_XferListCtrl.IsWindowVisible()) {
		menuCtrl = &m_XferListCtrl;
		menuId = XFER_MENU;
	}
	if(menuCtrl) {
		for(int i = 0; i < menuCtrl->GetItemCount(); i ++) {
			menuCtrl->GetItemRect(i, &rt, LVIR_BOUNDS);
			menuCtrl->ClientToScreen(&rt);
			if(rt.PtInRect(point)) {
				wholeMenu.LoadMenu(IDR_CONTEXT);
				contextMenu = wholeMenu.GetSubMenu(menuId);
				if(contextMenu) {
					contextMenu->TrackPopupMenu(TPM_LEFTALIGN|TPM_RIGHTBUTTON, point.x, point.y, this);
					m_ContextItem = i;
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
	if(m_TabCtrl.GetSafeHwnd() && m_Message) {
		m_TabCtrl.SetCurSel(MESSAGE_ID);
		m_ProjectListCtrl.ModifyStyle(WS_VISIBLE, 0);
		m_ResultListCtrl.ModifyStyle(WS_VISIBLE, 0);
		m_XferListCtrl.ModifyStyle(WS_VISIBLE, 0);
		m_MessageListCtrl.ModifyStyle(0, WS_VISIBLE);
		m_UsagePieCtrl.ModifyStyle(WS_VISIBLE, 0);
		m_MessageListCtrl.RedrawWindow(NULL, NULL, RDW_INVALIDATE|RDW_UPDATENOW|RDW_ERASE|RDW_FRAME);
		m_Message = false;
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
	float szDiv = (rt.bottom-rt.top)/100.0;
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
		CMenu menu, *submenu;
		if(!menu.LoadMenu(IDR_CONTEXT)) {
			return FALSE;
		}
		submenu = menu.GetSubMenu(STATUS_MENU);
		if(!submenu) {
			menu.DestroyMenu();
			return FALSE;
		}
		if(IsWindowVisible()) {
			submenu->CheckMenuItem(ID_STATUSICON_HIDE, MF_UNCHECKED);
		} else {
			submenu->CheckMenuItem(ID_STATUSICON_HIDE, MF_CHECKED);
		}
		if(m_Suspend) {
			submenu->CheckMenuItem(ID_STATUSICON_SUSPEND, MF_CHECKED);
		} else {
			submenu->CheckMenuItem(ID_STATUSICON_SUSPEND, MF_UNCHECKED);
		}
		submenu->TrackPopupMenu(TPM_LEFTALIGN|TPM_RIGHTBUTTON, point.x, point.y, this);
		menu.DestroyMenu();
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
// returns:		void
// function:	calls parents contructor.
CLoginDialog::CLoginDialog(UINT y, LPCTSTR url, LPCTSTR auth) : CDialog(y)
{
	m_url.Format("%s", url);
	m_auth.Format("%s", auth);
}

//////////
// CLoginDialog::OnInitDialog
// arguments:	void
// returns:		true if windows needs to give dialog focus, false if dialog has taken focus
// function:	initializes and centers dialog box
BOOL CLoginDialog::OnInitDialog() 
{
    CDialog::OnInitDialog();
	CWnd* wurl = GetDlgItem(IDC_LOGIN_URL);
	if(wurl) {
		wurl->SetWindowText(m_url);
	}
	CWnd* wauth = GetDlgItem(IDC_LOGIN_AUTH);
	if(wauth) {
		wauth->SetWindowText(m_auth);
	}
	CWnd* toFocus = GetDlgItem(IDC_LOGIN_URL);
	if(toFocus) toFocus->SetFocus();
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
    GetDlgItemText(IDC_LOGIN_URL, m_url);
    GetDlgItemText(IDC_LOGIN_AUTH, m_auth);
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
	CListBox* List = (CListBox*)GetDlgItem(IDC_LIST);
	if(List) {
		for(int i = 0; i < gstate.projects.size(); i ++) {
			if(!strcmp(gstate.projects[i]->project_name, "")) {
				List->AddString(gstate.projects[i]->master_url);
			} else {
				List->AddString(gstate.projects[i]->project_name);
			}
		}
		List->SetFocus();
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
	CString buf;
	m_sel = -1;
	CListBox* List = (CListBox*)GetDlgItem(IDC_LIST);
	if(List) {
		m_sel = List->GetCurSel();
	}
    if(m_sel >= 0) CDialog::OnOK();
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
	CButton* btnTmp;

	// fill in http
	btnTmp = (CButton*)GetDlgItem(IDC_CHECK_HTTP);
	if(btnTmp) btnTmp->SetCheck(gstate.use_proxy?BST_CHECKED:BST_UNCHECKED);
	SetDlgItemText(IDC_EDIT_HTTP_ADDR, gstate.proxy_server_name);
	CString portBuf;
	if(gstate.proxy_server_port > 0) portBuf.Format("%d", gstate.proxy_server_port);
	else portBuf.Format("80");
	SetDlgItemText(IDC_EDIT_HTTP_PORT, portBuf.GetBuffer(0));
	EnableHttp(gstate.use_proxy);
	
	// fill in socks
	btnTmp = (CButton*)GetDlgItem(IDC_CHECK_SOCKS);
	if(btnTmp) btnTmp->EnableWindow(false);
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
	CEdit* editTmp;
	editTmp = (CEdit*)GetDlgItem(IDC_EDIT_HTTP_ADDR);
	if(editTmp) editTmp->EnableWindow(bEnable);
	editTmp = (CEdit*)GetDlgItem(IDC_EDIT_HTTP_PORT);
	if(editTmp) editTmp->EnableWindow(bEnable);
}

//////////
// CProxyDialog::EnableSocks
// arguments:	bEnable: true to enable, false to disable
// returns:		void
// function:	enables or disables the socks section of the dialog
void CProxyDialog::EnableSocks(BOOL bEnable)
{
	CEdit* editTmp;
	editTmp = (CEdit*)GetDlgItem(IDC_EDIT_SOCKS_ADDR);
	if(editTmp) editTmp->EnableWindow(bEnable);
	editTmp = (CEdit*)GetDlgItem(IDC_EDIT_SOCKS_PORT);
	if(editTmp) editTmp->EnableWindow(bEnable);
	editTmp = (CEdit*)GetDlgItem(IDC_EDIT_SOCKS_NAME);
	if(editTmp) editTmp->EnableWindow(bEnable);
	editTmp = (CEdit*)GetDlgItem(IDC_EDIT_SOCKS_PASS);
	if(editTmp) editTmp->EnableWindow(bEnable);
}

//////////
// CProxyDialog::OnOK
// arguments:	void
// returns:		void
// function:	handles http check box
void CProxyDialog::OnHttp() 
{
	CButton* btnTmp;
	btnTmp = (CButton*)GetDlgItem(IDC_CHECK_HTTP);
	if(btnTmp) {
		if(btnTmp->GetCheck() == BST_CHECKED) {
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
	CButton* btnTmp;
	CString strTmp;

	// get http info
	btnTmp = (CButton*)GetDlgItem(IDC_CHECK_HTTP);
	if(btnTmp) {
		if(btnTmp->GetCheck() == BST_CHECKED) {
			gstate.use_proxy = true;
		} else {
			gstate.use_proxy = false;
		}
	}
	GetDlgItemText(IDC_EDIT_HTTP_ADDR, strTmp);
	strcpy(gstate.proxy_server_name, strTmp.GetBuffer(0));
	GetDlgItemText(IDC_EDIT_HTTP_PORT, strTmp);
	gstate.proxy_server_port = atoi(strTmp.GetBuffer(0));
	CDialog::OnOK();
}
