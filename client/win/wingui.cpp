#include <afxwin.h>

#include "log_flags.h"
#include "client_state.h"
#include "resource.h"
#include "win_net.h"
#include "wingui.h"

#define ID_TIMER        104
#define IDM_CLOSE        105
#define IDM_LOGIN        106
#if 0
#define IDD_LOGIN        105
#define IDC_LOGIN_URL    1000
#define IDC_LOGIN_AUTH    1001
#define IDC_LOGIN_OK    3
#endif

// Global vars

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

char* result_titles[] = {"Project", "Application", "CPU time", "status"};
int result_widths[] = {12, 20, 12, 18};
char* file_xfer_titles[] = {"Project", "File", "Size", "direction"};
int file_xfer_widths[] = {12, 20, 12, 12};
char* disk_usage_titles[] = {"Project", "space used"};
int disk_usage_widths[] = {12, 20};
char* project_titles[] = {"Project", "total CPU", "share"};
int project_widths[] = {12, 15, 15};
char* user_info_titles[] = {"Name", "Team", "Total credit", "Recent credit"};
int user_info_widths[] = {20, 20, 15, 15};

void show_message(char* p, char* prior) {
    //MessageBox(NULL, p, prior, MB_OK);
	printf("Message (%s): %s\n", prior, p);
}

int initialize_prefs() {
    CLoginDialog dlg(IDD_LOGIN);
    int retval = dlg.DoModal();
    if (retval != IDOK) return -1;
    write_initial_prefs((char*)(LPCTSTR) dlg.url, (char*)(LPCTSTR) dlg.auth);
    return 0;
}

void show_result(TEXT_LINE& line, RESULT& result) {
    char buf[256];
    line.set_field(0, result.project->project_name);
    line.set_field(1, result.app->name);
    sprintf(buf, "%f", result.final_cpu_time);
    line.set_field(2, buf);
    switch(result.state) {
    case RESULT_NEW:
        line.set_field(3, "New"); break;
    case RESULT_FILES_DOWNLOADED:
        line.set_field(3, "Ready to run"); break;
    case RESULT_COMPUTE_DONE:
        line.set_field(3, "Computation done"); break;
    case RESULT_READY_TO_ACK:
        line.set_field(3, "Results uploaded"); break;
    case RESULT_SERVER_ACK:
        line.set_field(3, "Acknowledged"); break;
    }
}

void show_file_xfer(TEXT_LINE& line, FILE_XFER& fx) {
    char buf[256];

    line.set_field(0, fx.fip->project->project_name);
    line.set_field(1, fx.fip->name);
    sprintf(buf, "%f", fx.fip->nbytes);
    line.set_field(2, buf);
    line.set_field(3, fx.fip->generated_locally?"upload":"download");
}

void update_gui(CLIENT_STATE& cs) {
    int i, n;

    n = min(results.nlines, cs.results.size());
    for (i=0; i<n; i++) {
        show_result(results.lines[i+1], *cs.results[i]);
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

// ------------ CLoginDialog -------------

CLoginDialog::CLoginDialog(UINT y) : CDialog(y){
}

BOOL CLoginDialog::OnInitDialog() {
    CDialog::OnInitDialog();
    CenterWindow();
    return TRUE;
}

void CLoginDialog::OnOK() {
    GetDlgItemText(IDC_LOGIN_URL, url);
    GetDlgItemText(IDC_LOGIN_AUTH, auth);
    CDialog::OnOK();
}

BEGIN_MESSAGE_MAP (CLoginDialog, CDialog)
    ON_BN_CLICKED(IDC_LOGIN_OK, OnOK)
END_MESSAGE_MAP()

// ----------- TEXT_TABLE ------------

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
    user_info.set_field(0, 0, buf);

#if 1
    while (gstate.do_something()) {
    }
#else
	gstate.do_something();
#endif
	fflush(stdout);
	fflush(stderr);
	update_gui(gstate);
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

    NetOpen();
    freopen("stdout.txt", "w", stdout);
    freopen("stderr.txt", "w", stderr);
    read_log_flags();
    int retval = gstate.init();
    if (retval) exit(retval);
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

