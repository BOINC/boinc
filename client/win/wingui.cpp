#include <afxwin.h>

#include "log_flags.h"
#include "client_state.h"
#include "resource.h"
#include "win_net.h"
#include "wingui.h"

#define ID_TIMER			104

#define EDGE_BUFFER			2			// buffer pixels around edge of client

// global vars

CMainWindow* main_window;
CMyApp myApp;

int initialize_prefs() {
	CLoginDialog dlg(IDD_LOGIN);
	int retval = dlg.DoModal();
	if (retval != IDOK) return -1;
//	write_initial_prefs();
    return 0;
}

/////////////////////////////////////////////////////////////////////////
// CProgressHeaderCtrl message map and member functions

BEGIN_MESSAGE_MAP (CProgressBarCtrl, CProgressCtrl)
	ON_WM_LBUTTONDOWN ()
	ON_WM_LBUTTONUP ()
END_MESSAGE_MAP ()

CProgressBarCtrl::CProgressBarCtrl()
{
}

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

/////////////////////////////////////////////////////////////////////////
// CProgressHeaderCtrl message map and member functions

BEGIN_MESSAGE_MAP (CProgressHeaderCtrl, CHeaderCtrl)
	ON_WM_RBUTTONDOWN ()
END_MESSAGE_MAP ()

CProgressHeaderCtrl::CProgressHeaderCtrl()
{
}

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

/////////////////////////////////////////////////////////////////////////
// CProgressListCtrl message map and member functions

BEGIN_MESSAGE_MAP(CProgressListCtrl, CListCtrl)
    ON_WM_CREATE()
    ON_WM_DESTROY()
    ON_WM_PAINT()
	ON_WM_RBUTTONDOWN()
END_MESSAGE_MAP()

CProgressListCtrl::CProgressListCtrl()
{
}

int CProgressListCtrl::InsertColumn(int nCol, LPCTSTR lpszColumnHeading, int nFormat = LVCFMT_LEFT, int nWidth = -1, int nSubItem = -1)
{
	m_ColWidths.SetAtGrow(nCol, nWidth);
	return CListCtrl::InsertColumn(nCol, lpszColumnHeading, nFormat, nWidth, nSubItem);
}

BOOL CProgressListCtrl::SetColumnWidth(int nCol, int cx)
{
	return CListCtrl::SetColumnWidth(nCol, cx);
}

void CProgressListCtrl::SetItemProgress(int item, int subitem, int prog)
{
	CRect rt;
	CString str;
	CProgressCtrl* progCtrl = NULL;
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
	Invalidate(false);
}

void CProgressListCtrl::SwapItems(int i1, int i2)
{
	int nCols = GetHeaderCtrl()->GetItemCount();
	CProgressCtrl* progCtrl1 = NULL;
	CProgressCtrl* progCtrl2 = NULL;
	CString txt1, txt2;
	int si;

	// check item indicies
	if(i1 >= GetItemCount() || i2 >= GetItemCount()) {
		return;
	}
	for(si = 0; si < nCols; si ++) {
		// swap text
		txt1 = GetItemText(i1, si);
		txt2 = GetItemText(i2, si);
		SetItemText(i1, si, txt2);
		SetItemText(i2, si, txt1);
		// swap progress control if found
		txt1.Format("%d:%d", i1, si);
		txt2.Format("%d:%d", i2, si);
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

void CProgressListCtrl::SwapColumnVisibility(int col)
{
	CHeaderCtrl* header = GetHeaderCtrl();
	CMenu* menu = m_PopupMenu.GetSubMenu(GetDlgCtrlID());
	if(header && col < header->GetItemCount()) {
		if(GetColumnWidth(col) == 0) {
			SetColumnWidth(col, m_ColWidths.GetAt(col));
			if(menu) {
				menu->CheckMenuItem(PopupFromColumn(col), MF_CHECKED);
			}
		} else {
			SetColumnWidth(col, 0);
			if(menu) {
				menu->CheckMenuItem(PopupFromColumn(col), MF_UNCHECKED);
			}
		}
	}
}

int CProgressListCtrl::ColumnFromPopup(int p)
{
	switch(p) {
		case ID_POPUP_0:
			return 0;
		case ID_POPUP_1:
			return 1;
		case ID_POPUP_2:
			return 2;
		case ID_POPUP_3:
			return 3;
	}
	return -1;
}

int CProgressListCtrl::PopupFromColumn(int c)
{
	switch(c) {
		case 0:
			return ID_POPUP_0;
		case 1:
			return ID_POPUP_1;
		case 2:
			return ID_POPUP_2;
		case 3:
			return ID_POPUP_3;
	}
	return -1;
}

int CProgressListCtrl::OnCreate(LPCREATESTRUCT lpcs)
{
    if(CListCtrl::OnCreate(lpcs) == -1) {
		return -1;
	}

	// load popup menu
	m_PopupMenu.LoadMenu(IDR_POPUP);

	// subclass header
	m_iSort = 0;
	CHeaderCtrl* header = GetHeaderCtrl();
	if(header) {
		HWND hWnd = header->GetSafeHwnd();
		if(hWnd) {
			m_Header.SubclassWindow(hWnd);
		}
	}
    return 0;
}

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

void CProgressListCtrl::OnRButtonDown(UINT nFlags, CPoint point)
{
	ClientToScreen(&point);

	// see if user clicked on header and do its popup menu
	CHeaderCtrl* header = GetHeaderCtrl();
	if(header) {
		CRect rt;
		header->GetWindowRect(&rt);
		if(rt.PtInRect(point)) {
			CMenu* menu = m_PopupMenu.GetSubMenu(GetDlgCtrlID());
			if(menu) {
				menu->TrackPopupMenu(TPM_LEFTALIGN|TPM_RIGHTBUTTON, point.x, point.y, this);
			}
		}
	}
	//CListCtrl::OnRButtonDown(nFlags, point);
}

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

	// notification from header, user has started tracking a header
	if(phdn->hdr.code == HDN_BEGINTRACKA || phdn->hdr.code == HDN_BEGINTRACKW) {
		// stop the header from tracking
		int col = phdn->iItem;
		if(GetColumnWidth(col) == 0) {
			*pResult = TRUE;
			return TRUE;
		}
	}

	// notification from header, user has finished tracking a header
	if(phdn->hdr.code == HDN_ENDTRACKA || phdn->hdr.code == HDN_ENDTRACKW) {
		// store the new width
		int col = phdn->iItem;
		m_ColWidths.SetAtGrow(col, GetColumnWidth(col));
	}
	return CListCtrl::OnNotify(wParam, lParam, pResult);
}

void CProgressListCtrl::OnPaint()
{
	RepositionProgress();
	CListCtrl::OnPaint();
}

BOOL CProgressListCtrl::OnCommand(WPARAM wParam, LPARAM lParam)
{
	if(ColumnFromPopup(wParam) != -1) {
		SwapColumnVisibility(ColumnFromPopup(wParam));
	}
	return CListCtrl::OnCommand(wParam, lParam);
}


/////////////////////////////////////////////////////////////////////////
// CMyApp member functions

BOOL CMyApp::InitInstance()
{
    m_pMainWnd = new CMainWindow();
    m_pMainWnd->ShowWindow(m_nCmdShow);
    m_pMainWnd->UpdateWindow();
    return TRUE;
}

/////////////////////////////////////////////////////////////////////////
// CMainWindow message map and member functions

BEGIN_MESSAGE_MAP(CMainWindow, CWnd)
    ON_WM_CREATE()
    ON_WM_PAINT()
    ON_WM_SIZE()
    ON_COMMAND(ID_FILE_CLOSE, OnCommandFileClose)
    ON_COMMAND(ID_ACCT_LOGIN, OnCommandAccountLogin)
    ON_COMMAND(ID_HELP_ABOUT, OnCommandHelpAbout)
END_MESSAGE_MAP()

CMainWindow::CMainWindow()
{
	// clear pointers...
	m_bCreated = false;

	// register window class
    CString strWndClass = AfxRegisterWndClass (0, myApp.LoadStandardCursor(IDC_ARROW),
        (HBRUSH)(COLOR_3DFACE+1), myApp.LoadStandardIcon(IDI_APPLICATION));

	// create and position window
    CreateEx(0, strWndClass, "BOINC", WS_OVERLAPPEDWINDOW|WS_EX_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
		NULL, NULL, NULL);
    CRect rect(0, 0, 420, 500);
    CalcWindowRect(&rect);
    SetWindowPos(NULL, 0, 0, rect.Width(), rect.Height(), SWP_NOZORDER|SWP_NOMOVE|SWP_NOREDRAW);
}

void CALLBACK CMainWindow::TimerProc(HWND h, UINT x, UINT id, DWORD time)
{
    while (gstate.do_something());
	fflush(stdout);
	fflush(stderr);
}

void CMainWindow::MessageUser(char* s)
{
	// put message in control
	if(m_bCreated) {
		CString text;
		m_CtrlMessages.GetWindowText(text);
		text += s;
		m_CtrlMessages.SetWindowText(text);
		m_CtrlMessages.RedrawWindow();
	}
}

void CMainWindow::PostNcDestroy()
{
    delete this;
}

void CMainWindow::OnCommandAccountLogin()
{
    CLoginDialog dlg(IDD_LOGIN);
    int retval = dlg.DoModal();
}

void CMainWindow::OnCommandFileClose()
{
    SendMessage(WM_CLOSE, 0, 0);
}

void CMainWindow::OnCommandHelpAbout()
{
    //CDialog dlg(IDD_ABOUTBOX);
    //int retval = dlg.DoModal();

	// add some random info
	CString strText;
	for (int j=0;j < 5;j++) {
		int i = m_CtrlProjects.GetItemCount();
		strText.Format("Project-%d", i);
		m_CtrlProjects.InsertItem(i, strText);
		strText.Format("http://%d", i);
		m_CtrlProjects.SetItemText(i, 1, strText);
		strText.Format("%d", rand());
		m_CtrlProjects.SetItemText(i, 2, strText);
		strText.Format("");
		m_CtrlProjects.SetItemText(i, 3, strText);
		m_CtrlProjects.SetItemProgress(i, 3, rand()%100);
	}
}

int CMainWindow::OnCreate(LPCREATESTRUCT lpcs)
{
    if (CWnd::OnCreate(lpcs) == -1) {
		return -1;
	}

    CClientDC dc(this);
    main_window = this;

	// load main menu
	m_MainMenu.LoadMenu(IDR_MAINFRAME);
	SetMenu(&m_MainMenu);

	// create controls and temporarily position them
	m_CtrlProjects.Create(LVS_REPORT|WS_CHILD|WS_BORDER|WS_VISIBLE, CRect(0,0,0,0), this, 0);
	m_CtrlProjects.SetExtendedStyle(m_CtrlProjects.GetExtendedStyle()|LVS_EX_HEADERDRAGDROP|LVS_EX_FULLROWSELECT);
	m_CtrlProjects.InsertColumn(0, "Name", LVCFMT_LEFT, 100);
	m_CtrlProjects.InsertColumn(1, "URL", LVCFMT_LEFT, 100);
	m_CtrlProjects.InsertColumn(2, "Total", LVCFMT_LEFT, 100);
	m_CtrlProjects.InsertColumn(3, "Share", LVCFMT_LEFT, 100);
	m_CtrlXfers.Create(LVS_REPORT|WS_CHILD|WS_BORDER|WS_VISIBLE, CRect(0,0,0,0), this, 1);
	m_CtrlXfers.SetExtendedStyle(m_CtrlXfers.GetExtendedStyle()|LVS_EX_HEADERDRAGDROP|LVS_EX_FULLROWSELECT);
	m_CtrlXfers.ModifyStyle(WS_VISIBLE, 0);
	m_CtrlXfers.InsertColumn(0, "Project", LVCFMT_LEFT, 100);
	m_CtrlXfers.InsertColumn(1, "Name", LVCFMT_LEFT, 100);
	m_CtrlXfers.InsertColumn(2, "Completion", LVCFMT_LEFT, 100);
	m_CtrlWorkunits.Create(LVS_REPORT|WS_CHILD|WS_BORDER|WS_VISIBLE, CRect(0,0,0,0), this, 2);
	m_CtrlWorkunits.SetExtendedStyle(m_CtrlWorkunits.GetExtendedStyle()|LVS_EX_HEADERDRAGDROP|LVS_EX_FULLROWSELECT);
	m_CtrlWorkunits.ModifyStyle(WS_VISIBLE, 0);
	m_CtrlWorkunits.InsertColumn(0, "Project", LVCFMT_LEFT, 100);
	m_CtrlWorkunits.InsertColumn(1, "Name", LVCFMT_LEFT, 100);
	m_CtrlWorkunits.InsertColumn(2, "Completion", LVCFMT_LEFT, 100);
	m_CtrlMessages.Create(ES_MULTILINE|ES_READONLY|WS_CHILD|WS_TABSTOP|WS_BORDER|WS_VISIBLE, CRect(0,0,0,0), this, 3);
	m_CtrlMessages.ModifyStyle(WS_VISIBLE, 0);
	m_Tabs.Create(TCS_BUTTONS|TCS_FIXEDWIDTH|WS_CHILD|WS_VISIBLE, CRect(0,0,0,0), this, 0);
	m_Tabs.InsertItem(0, "Projects");
	m_Tabs.InsertItem(1, "Xfers");
	m_Tabs.InsertItem(2, "Workunits");
	m_Tabs.InsertItem(3, "Messages");

	// load images
	m_Logo.LoadBitmap(IDB_LOGO);

	// take care of other things
    NetOpen();
    freopen("stdout.txt", "w", stdout);
    freopen("stderr.txt", "w", stderr);
    read_log_flags();
    //int retval = gstate.init();
    //if (retval) exit(retval);
    SetTimer(ID_TIMER, 1000, TimerProc);
	m_bCreated = true;
    return 0;
}

BOOL CMainWindow::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult)
{
	HD_NOTIFY* phdn = (HD_NOTIFY*)lParam;

	// notification from tab control, user is changing the selection
	if(phdn->hdr.code == TCN_SELCHANGE) {
		int newTab = m_Tabs.GetCurSel();

		// make the selected control visible, all the rest invisible
		if(newTab == 0) {
			m_CtrlProjects.ModifyStyle(0, WS_VISIBLE);
			m_CtrlXfers.ModifyStyle(WS_VISIBLE, 0);
			m_CtrlWorkunits.ModifyStyle(WS_VISIBLE, 0);
			m_CtrlMessages.ModifyStyle(WS_VISIBLE, 0);
		} else if(newTab == 1) {
			m_CtrlProjects.ModifyStyle(WS_VISIBLE, 0);
			m_CtrlXfers.ModifyStyle(0, WS_VISIBLE);
			m_CtrlWorkunits.ModifyStyle(WS_VISIBLE, 0);
			m_CtrlMessages.ModifyStyle(WS_VISIBLE, 0);
		} else if(newTab == 2) {
			m_CtrlProjects.ModifyStyle(WS_VISIBLE, 0);
			m_CtrlXfers.ModifyStyle(WS_VISIBLE, 0);
			m_CtrlWorkunits.ModifyStyle(0, WS_VISIBLE);
			m_CtrlMessages.ModifyStyle(WS_VISIBLE, 0);
		} else if(newTab == 3) {
			m_CtrlProjects.ModifyStyle(WS_VISIBLE, 0);
			m_CtrlXfers.ModifyStyle(WS_VISIBLE, 0);
			m_CtrlWorkunits.ModifyStyle(WS_VISIBLE, 0);
			m_CtrlMessages.ModifyStyle(0, WS_VISIBLE);
		}
		Invalidate(false);
	}
	return CWnd::OnNotify(wParam, lParam, pResult);
}

void CMainWindow::OnPaint()
{
	CWnd::OnPaint();
	/*
	CClientDC dc(this);
	CDC logoDC;
	logoDC.CreateCompatibleDC(&dc);
	logoDC.SelectObject(&m_Logo);
	dc.BitBlt(0,0,166,150,&logoDC,0,0,SRCCOPY);
	*/
}

void CMainWindow::OnSize(UINT nType, int cx, int cy)
{
	CWnd::OnSize(nType, cx, cy);

	// calculate the main rect for the tab control
	RECT rt = {EDGE_BUFFER, EDGE_BUFFER, cx-EDGE_BUFFER, cy-EDGE_BUFFER*2};
	RECT irt = {0, 0, 0, 0};
	float szDiv = (rt.bottom-rt.top)/100.0;
	if(m_bCreated) {
		m_Tabs.MoveWindow(&rt, false);
		m_Tabs.GetItemRect(0, &irt);

		// calculate the rects for other controls inside the tab control
		RECT srt = {rt.left+EDGE_BUFFER, irt.bottom+EDGE_BUFFER*2, rt.right-EDGE_BUFFER, rt.bottom-EDGE_BUFFER};
		m_CtrlProjects.MoveWindow(&srt, false);
		m_CtrlXfers.MoveWindow(&srt, false);
		m_CtrlWorkunits.MoveWindow(&srt, false);
		m_CtrlMessages.MoveWindow(&srt, false);
		Invalidate(false);
	}
}

/////////////////////////////////////////////////////////////////////////
// CLoginDialog message map and member functions

BEGIN_MESSAGE_MAP(CLoginDialog, CDialog)
    ON_BN_CLICKED(IDOK, OnOK)
END_MESSAGE_MAP()

CLoginDialog::CLoginDialog(UINT y) : CDialog(y)
{
}

BOOL CLoginDialog::OnInitDialog() 
{
    CDialog::OnInitDialog();
    CenterWindow();
    return TRUE;
}

void CLoginDialog::OnOK() 
{
    GetDlgItemText(IDC_LOGIN_URL, m_url);
    GetDlgItemText(IDC_LOGIN_AUTH, m_auth);
    CDialog::OnOK();
}

