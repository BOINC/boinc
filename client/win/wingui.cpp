#include <afxwin.h>
#include "client_state.h"
#include "resource.h"
#include "wingui.h"

#define ID_TIMER		104
#define IDM_CLOSE		105
#define IDM_LOGIN		106
#if 0
#define IDD_LOGIN		105
#define IDC_LOGIN_URL	1000
#define IDC_LOGIN_AUTH	1001
#define IDC_LOGIN_OK	3
#endif

CMainWindow* main_window;
CFont m_fontMain;
CMyApp myApp;
TEXT_TABLE results;
TEXT_TABLE file_xfers;
TEXT_TABLE disk_usage;
TEXT_TABLE projects;
TEXT_TABLE user_info;
int m_cxChar;
int m_cyChar;

CLoginDialog::CLoginDialog(UINT y) : CDialog(y){
}

BOOL CLoginDialog::OnInitDialog() {
	CDialog::OnInitDialog();
	CenterWindow();
	return TRUE;
}

void CLoginDialog::OnOK() {
	CString url, auth;
	GetDlgItemText(IDC_LOGIN_URL, url);
	GetDlgItemText(IDC_LOGIN_AUTH, auth);
	CDialog::OnOK();
}

BEGIN_MESSAGE_MAP (CLoginDialog, CDialog)
	ON_BN_CLICKED(IDC_LOGIN_OK, OnOK)
END_MESSAGE_MAP()

char* result_titles[] = {"Project", "Application", "CPU time", "% done"};
int result_widths[] = {12, 20, 12, 18};
char* file_xfer_titles[] = {"Project", "File", "Size", "% done"};
int file_xfer_widths[] = {12, 20, 12, 12};
char* disk_usage_titles[] = {"Project", "space used"};
int disk_usage_widths[] = {12, 20};
char* project_titles[] = {"Project", "total CPU", "share"};
int project_widths[] = {12, 15, 15};
char* user_info_titles[] = {"Name", "Team", "Total credit", "Recent credit"};
int user_info_widths[] = {20, 20, 15, 15};

void TEXT_TABLE::create(char* title, int nf, int sl, int nl, char** titles, int* w) {
	int i, j, col, rcol;
	CRect rect;
	char* text;

	nfields = nf;
	nlines = nl;

	rect.SetRect (m_cxChar * BOX_LEFT, m_cyChar * sl, m_cxChar * BOX_RIGHT, m_cyChar * (sl+nl+2));
    group_box.Create (title,  WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
        rect, main_window, (UINT) -1);
    group_box.SetFont (&m_fontMain, FALSE);

	for (i=0; i<nl; i++) {
		col = TEXT_LEFT;
		for (j=0; j<nf; j++) {
			rcol = col + w[j];
			rect.SetRect (m_cxChar * col, m_cyChar * (sl+i+1), m_cxChar * rcol, m_cyChar * (sl+i+2));
			text = (i==0)?titles[j]:"xxx";
			lines[i].fields[j].Create(text, WS_CHILD|WS_VISIBLE|SS_LEFT, rect, main_window);
            lines[i].fields[j].SetFont (&m_fontMain, FALSE);
			col = rcol;
		}
	}
}

void TEXT_TABLE::set_field(int line, int field, char* text) {
	lines[line].set_field(field, text);
}

void TEXT_TABLE::blank_line(int line) {
	int i;

	for (i=0; i<nfields; i++) {
		lines[line].set_field(i, "");
	}
}

void TEXT_LINE::set_field(int field, char* text) {
	fields[field].SetWindowText(text);
}

void make_menus(CMenu* main_menu) {
	CMenu popup;

	main_menu->CreateMenu();
	popup.CreatePopupMenu();
	popup.AppendMenu(MF_STRING, IDM_CLOSE, "&Close");
	main_menu->AppendMenu(MF_POPUP, (UINT)popup.Detach(), "&File");
	popup.CreatePopupMenu();
	popup.AppendMenu(MF_STRING, IDM_LOGIN, "&Login");
	main_menu->AppendMenu(MF_POPUP, (UINT)popup.Detach(), "&Account");
}

/////////////////////////////////////////////////////////////////////////
// CMyApp member functions

BOOL CMyApp::InitInstance ()
{
    m_pMainWnd = new CMainWindow;
    m_pMainWnd->ShowWindow (m_nCmdShow);
    m_pMainWnd->UpdateWindow ();
    return TRUE;
}

/////////////////////////////////////////////////////////////////////////
// CMainWindow message map and member functions

BEGIN_MESSAGE_MAP (CMainWindow, CWnd)
    ON_WM_CREATE ()
	ON_COMMAND (IDM_CLOSE, OnCloseMenu)
	ON_COMMAND (IDM_LOGIN, OnLoginMenu)
END_MESSAGE_MAP ()

CMainWindow::CMainWindow ()
{
    CString strWndClass = AfxRegisterWndClass (
        0,
        myApp.LoadStandardCursor (IDC_ARROW),
        (HBRUSH) (COLOR_3DFACE+1),
        myApp.LoadStandardIcon (IDI_APPLICATION)
    );

    CreateEx (0, strWndClass, "BOINC",
        WS_OVERLAPPEDWINDOW | WS_EX_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
        NULL, NULL, NULL);

    CRect rect (0, 0, m_cxChar * WIN_NCOLS, m_cyChar * WIN_NLINES);
    CalcWindowRect (&rect);

    SetWindowPos (NULL, 0, 0, rect.Width (), rect.Height (),
        SWP_NOZORDER | SWP_NOMOVE | SWP_NOREDRAW);
}

void CALLBACK CMainWindow::TimerProc(HWND h, UINT x, UINT id, DWORD time) {
	static int n=0;
	char buf[256];
	n++;
	sprintf(buf, "%d", n);
	results.set_field(0, 0, buf);
}

int CMainWindow::OnCreate (LPCREATESTRUCT lpcs)
{
    if (CWnd::OnCreate (lpcs) == -1)
        return -1;

    CClientDC dc (this);
    int nHeight = -((dc.GetDeviceCaps (LOGPIXELSY) * 8) / 72);

    m_fontMain.CreateFont (nHeight, 0, 0, 0, FW_NORMAL, 0, 0, 0,
        DEFAULT_CHARSET, OUT_CHARACTER_PRECIS, CLIP_CHARACTER_PRECIS,
        DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "MS Sans Serif");

    CFont* pOldFont = dc.SelectObject (&m_fontMain);
    TEXTMETRIC tm;
    dc.GetTextMetrics (&tm);
    m_cxChar = tm.tmAveCharWidth;
    m_cyChar = tm.tmHeight + tm.tmExternalLeading;
	
	main_window = this;

	CMenu main_menu;
    make_menus(&main_menu);
	SetMenu(&main_menu);
	main_menu.Detach();

	results.create("Work units", 4, 1, 5, result_titles, result_widths);
	file_xfers.create("File transfers", 4, 9, 4, file_xfer_titles, file_xfer_widths);
	disk_usage.create("Disk usage", 2, 16, 3, disk_usage_titles, disk_usage_widths);
	projects.create("Projects", 3, 22, 3, project_titles, project_widths);
	user_info.create("User info", 4, 28, 2, user_info_titles, user_info_widths);

	SetTimer(ID_TIMER, 1000, TimerProc);

    return 0;
}

void CMainWindow::PostNcDestroy ()
{
    delete this;
}

void CMainWindow::OnCloseMenu() {
	SendMessage(WM_CLOSE, 0, 0);
}

void CMainWindow::OnLoginMenu() {
	CLoginDialog dlg(IDD_LOGIN);
	int retval = dlg.DoModal();
}

void show_result(TEXT_LINE& line, RESULT& result) {
	char buf[256];
	line.set_field(0, result.project->project_name);
	line.set_field(1, result.app->name);
	sprintf(buf, "%f", result.final_cpu_time);
	line.set_field(2, buf);
#if 0
	sprintf(buf, "%f", result.fraction_done);
	line.set_field(3, buf);
#endif
}

void show_file_xfer(TEXT_LINE& line, FILE_XFER& fx) {
	char buf[256];

	line.set_field(0, fx.fip->project->project_name);
	line.set_field(1, fx.fip->name);
	sprintf(buf, "%f", fx.fip->nbytes);
	line.set_field(2, buf);
#if 0
	sprintf(buf, "%f", fx.fip->fraction_done*100);
	line.set_field(3, buf);
#endif
}

void update_gui(CLIENT_STATE& cs) {
	int i, n;

	n = min(results.nlines, cs.results.size());
	for (i=0; i<n; i++) {
		show_result(results.lines[i], *cs.results[i]);
	}
	for (i=n; i<results.nlines; i++) {
		results.blank_line(i);
	}

	n = min(file_xfers.nlines, cs.file_xfers->file_xfers.size());
	for (i=0; i<n; i++) {
		show_file_xfer(file_xfers.lines[i], *cs.file_xfers->file_xfers[i]);
	}
	for (i=n; i<file_xfers.nlines; i++) {
		file_xfers.blank_line(i);
	}
}
