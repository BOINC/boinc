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

#include "wingui_mainwindow.h"

CMyApp g_myApp;
CMainWindow* g_myWnd = NULL;

char g_szTabItems[MAX_TABS][256] = {
	"Projects",
	"Work",
	"Transfers",
	"Messages",
	"Disk"
};

char g_szColumnTitles[MAX_LIST_ID][MAX_COLS][256] = {
        {"Project",	"Account",		"Total Credit",	"Avg. Credit",	"Resource Share",	"",					""},
        {"Project",	"Application",	"Name",			"CPU time",		"Progress",			"To Completion",	"Status"},
        {"Project",	"File",			"Progress",		"Size",			"Time",				"Direction",		""},
        {"Project",	"Time",			"Message",		"",				"",					"",					""}
};

char g_szUsageItems[MAX_USAGE_STR][256] = {
	"Free space: not available for use",
	"Free space: available for use",
	"Used space: other than BOINC",
	"Used space: BOINC",
	"Used space:"
};

char g_szMiscItems[MAX_MISC_STR][256] = {
	"New",
	"Running",
	"Ready to run",
	"Computation done",
	"Results uploaded",
	"Acknowledged",
	"Error: invalid state",
	"Completed",
	"Upload",
	"Download"
};

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
	if (gstate.minimize)
		m_pMainWnd->ShowWindow(SW_HIDE);
	else
		m_pMainWnd->ShowWindow(SW_SHOW);

	m_pMainWnd->UpdateWindow();
	if(gstate.projects.size() == 0) {
		((CMainWindow*)m_pMainWnd)->SendMessage(WM_COMMAND, ID_SETTINGS_LOGIN);
	}
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
    ON_COMMAND(ID_CONNECTION_CONNECTNOW, OnCommandConnectionConnectNow)
    ON_COMMAND(ID_CONNECTION_HANGUPIFDIALED, OnCommandConnectionHangupIfDialed)
    ON_COMMAND(ID_CONNECTION_CONFIRMBEFORECONNECTING, OnCommandConnectionConfirmBeforeConnecting)
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
	ON_WM_TIMER()
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
// CMainWindow::GetPieColor
// arguments:	nPiece: index of pie piece
// returns:		the color that piece should be
// function:	detemines colors for pie pieces in usage control
COLORREF CMainWindow::GetPieColor(int nPiece)
{
	int nR = 0, nG = 0, nB = 0;
	if(nPiece == 0) {
		return RGB(255, 0, 255);
	} else if(nPiece == 1) {
		return RGB(192, 64, 192);
	}
	nPiece -= 2;
	switch(nPiece % 4) {
		case 0:
			return RGB(0, 0, 255);
		case 1:
			return RGB(64, 0, 192);
		case 2:
			return RGB(128, 0, 128);
		default:
			return RGB(192, 0, 64);
	}
	return RGB(0, 0, 0);
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
	m_ProjectListCtrl.SetRedraw(FALSE);
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
	m_ProjectListCtrl.SetRedraw(TRUE);

	// update results
	m_ResultListCtrl.SetRedraw(FALSE);
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
			m_ResultListCtrl.SetItemProgress(i, 4, at->fraction_done * 100);
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
				strBuf.Format(g_szMiscItems[0]); break;
			case RESULT_FILES_DOWNLOADED:
				if (at)
					strBuf.Format(g_szMiscItems[1]);
				else
					strBuf.Format(g_szMiscItems[2]);
				break;
			case RESULT_COMPUTE_DONE:
				strBuf.Format(g_szMiscItems[3]); break;
			case RESULT_READY_TO_ACK:
				strBuf.Format(g_szMiscItems[4]); break;
			case RESULT_SERVER_ACK:
				strBuf.Format(g_szMiscItems[5]); break;
			default:
				strBuf.Format(g_szMiscItems[6]); break;
		}
		m_ResultListCtrl.SetItemText(i, 6, strBuf);
	}
	m_ResultListCtrl.SetRedraw(TRUE);

	// update xfers
	m_XferListCtrl.SetRedraw(FALSE);
	Syncronize(&m_XferListCtrl, (vector<void*>*)(&pcs->pers_xfers->pers_file_xfers));
	for(i = 0; i < m_XferListCtrl.GetItemCount(); i ++) {
		PERS_FILE_XFER* fi = (PERS_FILE_XFER*)m_XferListCtrl.GetItemData(i);
		if(!fi) {
			m_XferListCtrl.SetItemColor(i, RGB(128, 128, 128));
			m_XferListCtrl.SetItemProgress(i, 2, 100);
			m_XferListCtrl.SetItemText(i, 3, g_szMiscItems[7]);
			continue;
		}

		// project
		m_XferListCtrl.SetItemText(i, 0, fi->fip->project->project_name);

		// file
		m_XferListCtrl.SetItemText(i, 1, fi->fip->name);

		// progress
		double xSent = 0;
		if(fi->fip->generated_locally) {
			xSent = fi->fip->upload_offset;
		} else {
		    char pathname[256];
			get_pathname(fi->fip, pathname);
			file_size(pathname, xSent);
		}
		m_XferListCtrl.SetItemProgress(i, 2, 100 * xSent / fi->fip->nbytes);

		// size
		strBuf.Format("%0.0f/%0.0fKB", xSent / 1024, fi->fip->nbytes / 1024);
		m_XferListCtrl.SetItemText(i, 3, strBuf.GetBuffer(0));

		// time
		double xtime = 0;
		if(fi->fxp) {
			xtime = (double)time(0) - fi->fxp->start_time;
		}
		int xhour = (int)(xtime / (60 * 60));
		int xmin = (int)(xtime / 60) % 60;
		int xsec = (int)(xtime) % 60;
		strBuf.Format("%0.2d:%0.2d:%0.2d", xhour, xmin, xsec);
		m_XferListCtrl.SetItemText(i, 4, strBuf.GetBuffer(0));

		// direction
		m_XferListCtrl.SetItemText(i, 5, fi->fip->generated_locally?g_szMiscItems[8]:g_szMiscItems[9]);
	}
	m_XferListCtrl.SetRedraw(TRUE);

	// update usage
	double xDiskTotal;
	double xDiskFree; get_host_disk_info(xDiskTotal, xDiskFree);
	double xDiskUsed = xDiskTotal - xDiskFree;
	double xDiskAllow; gstate.allowed_disk_usage(xDiskAllow);
	double xDiskUsage; gstate.current_disk_usage(xDiskUsage);

	while(m_UsagePieCtrl.GetItemCount() - 4 < gstate.projects.size()) {
		m_UsagePieCtrl.AddPiece("", GetPieColor(m_UsagePieCtrl.GetItemCount()), 0);
	}

	while(m_UsagePieCtrl.GetItemCount() - 4 > gstate.projects.size()) {
		m_UsagePieCtrl.RemovePiece(m_UsagePieCtrl.GetItemCount() - 1);
	}

	m_UsagePieCtrl.SetTotal(xDiskTotal);
	m_UsagePieCtrl.SetPiece(0, xDiskFree - (xDiskAllow - xDiskUsage)); // Free (non-BOINC)
	m_UsagePieCtrl.SetPiece(1, xDiskAllow - xDiskUsage); // Free (BOINC)
	m_UsagePieCtrl.SetPiece(2, xDiskUsed - xDiskUsage); // Used (non-BOINC)
	for(i = 0; i < gstate.projects.size(); i ++) {
		double xUsage;
		CString strLabel;
		strLabel.Format("%s %s", g_szUsageItems[4], gstate.projects[i]->project_name);
		gstate.project_disk_usage(gstate.projects[i], xUsage);
		m_UsagePieCtrl.SetPieceLabel(i + 4, strLabel.GetBuffer(0));
		m_UsagePieCtrl.SetPiece(i + 4, xUsage);
		xDiskUsage -= xUsage;
	}
	m_UsagePieCtrl.SetPiece(3, xDiskUsage); // Used (BOINC)
	m_UsagePieCtrl.RedrawWindow(NULL, NULL, RDW_INVALIDATE|RDW_UPDATENOW|RDW_NOERASE|RDW_FRAME);

	// make icon flash if needed
	if(m_bMessage || m_bRequest) {
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

	int nNewPos = m_MessageListCtrl.GetItemCount();
	m_MessageListCtrl.InsertItem(nNewPos, szProject);

	CTime curTime = CTime::GetCurrentTime();
	CString strTime;
	strTime = curTime.Format("%c");
	m_MessageListCtrl.SetItemText(nNewPos, 1, strTime);

	m_MessageListCtrl.SetItemText(nNewPos, 2, szMessage);
	
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
// CMainWindow::RequestNetConnect
// arguments:	void
// returns:		true if the user can connect, false otherwise
// function:	asks the user for permission to connect to the network
BOOL CMainWindow::RequestNetConnect()
{
	if(GetForegroundWindow() != this) {
		m_bRequest = true;
		return FALSE;
	}
	CConnectDialog dlg(IDD_CONNECT);
	int retval = dlg.DoModal();
	m_bRequest = false;
	if(retval == IDOK) {
		gstate.global_prefs.confirm_before_connecting = !dlg.m_bDontAsk;
		CMenu* ConMenu = NULL;
		ConMenu = m_MainMenu.GetSubMenu(2);
		if(ConMenu) {
			if(gstate.global_prefs.confirm_before_connecting) ConMenu->CheckMenuItem(ID_CONNECTION_CONFIRMBEFORECONNECTING, MF_CHECKED);
			else ConMenu->CheckMenuItem(ID_CONNECTION_CONFIRMBEFORECONNECTING, MF_UNCHECKED);
		}
		return TRUE;
	}
	return FALSE;
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
// CMainWindow::SaveListControls
// arguments:	void
// returns:		void
// function:	saves relevant elements of list controls
void CMainWindow::SaveListControls()
{
	char szPath[256];
	CString strKey, strVal;
	GetCurrentDirectory(256, szPath);
	strcat(szPath, "\\");
	strcat(szPath, LIST_STATE_FILE_NAME);
	file_delete(szPath);
	m_ProjectListCtrl.SaveInactive(szPath, "PROJECTS");
	m_ResultListCtrl.SaveInactive(szPath, "WORK");
	m_XferListCtrl.SaveInactive(szPath, "TRANSFERS");
	m_MessageListCtrl.SaveInactive(szPath, "MESSAGES");
}

//////////
// CMainWindow::LoadListControls
// arguments:	void
// returns:		void
// function:	loads relevant elements of list controls
void CMainWindow::LoadListControls()
{
	char szPath[256];
	CString strKey, strVal;
	GetCurrentDirectory(256, szPath);
	strcat(szPath, "\\");
	strcat(szPath, LIST_STATE_FILE_NAME);
	m_ProjectListCtrl.LoadInactive(szPath, "PROJECTS");
	m_ResultListCtrl.LoadInactive(szPath, "WORK");
	m_XferListCtrl.LoadInactive(szPath, "TRANSFERS");
	m_MessageListCtrl.LoadInactive(szPath, "MESSAGES");
	file_delete(szPath);
}

//////////
// CMainWindow::SaveUserSettings
// arguments:	void
// returns:		void
// function:	saves relevant user settings to boinc ini file
void CMainWindow::SaveUserSettings()
{
	char szPath[256];
	CString strKey, strVal;
	GetCurrentDirectory(256, szPath);
	strcat(szPath, "\\");
	strcat(szPath, INI_FILE_NAME);
	int colorder[MAX_COLS];
	int i;

	// save suspend
	if(m_bSuspend) {
		WritePrivateProfileString("WINDOW", "suspend", "yes", szPath);
	} else {
		WritePrivateProfileString("WINDOW", "suspend", "no", szPath);
	}

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
// function:	loads relevant user settings from boinc ini file
void CMainWindow::LoadUserSettings()
{
	char szPath[256], szVal[256];
	CString strKey;
	GetCurrentDirectory(256, szPath);
	strcat(szPath, "\\");
	strcat(szPath, INI_FILE_NAME);
	int i, nBuf;
	int colorder[MAX_COLS];

	// load suspend
	GetPrivateProfileString("WINDOW", "suspend", "no", szVal, 256, szPath);
	if(!strcmp(szVal, "no")) OnCommandResume();
	else OnCommandSuspend();

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
	SetWindowPos(&wndNoTopMost, rt.left, rt.top, rt.Width(), rt.Height(), gstate.minimize?SWP_HIDEWINDOW:SWP_SHOWWINDOW);

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
// CMainWindow::LoadLanguage
// arguments:	void
// returns:		void
// function:	loads new captions from language file
void CMainWindow::LoadLanguage()
{
	char szPath[256];
	int col;
	CString strSection;
	GetCurrentDirectory(256, szPath);
	strcat(szPath, "\\");
	strcat(szPath, LANGUAGE_FILE_NAME);

	// load column headers
	strSection.Format("HEADER-%s", g_szTabItems[PROJECT_ID]);
	for(col = 0; col < PROJECT_COLS; col ++) {
		GetPrivateProfileString(strSection, g_szColumnTitles[PROJECT_ID][col], g_szColumnTitles[PROJECT_ID][col], g_szColumnTitles[PROJECT_ID][col], 256, szPath);
	}
	GetPrivateProfileString(strSection, "Title", g_szTabItems[PROJECT_ID], g_szTabItems[PROJECT_ID], 16, szPath);
	strSection.Format("HEADER-%s", g_szTabItems[RESULT_ID]);
	for(col = 0; col < RESULT_COLS; col ++) {
		GetPrivateProfileString(strSection, g_szColumnTitles[RESULT_ID][col], g_szColumnTitles[RESULT_ID][col], g_szColumnTitles[RESULT_ID][col], 256, szPath);
	}
	GetPrivateProfileString(strSection, "Title", g_szTabItems[RESULT_ID], g_szTabItems[RESULT_ID], 16, szPath);
	strSection.Format("HEADER-%s", g_szTabItems[XFER_ID]);
	for(col = 0; col < XFER_COLS; col ++) {
		GetPrivateProfileString(strSection, g_szColumnTitles[XFER_ID][col], g_szColumnTitles[XFER_ID][col], g_szColumnTitles[XFER_ID][col], 256, szPath);
	}
	GetPrivateProfileString(strSection, "Title", g_szTabItems[XFER_ID], g_szTabItems[XFER_ID], 16, szPath);
	strSection.Format("HEADER-%s", g_szTabItems[MESSAGE_ID]);
	for(col = 0; col < MESSAGE_COLS; col ++) {
		GetPrivateProfileString(strSection, g_szColumnTitles[MESSAGE_ID][col], g_szColumnTitles[MESSAGE_ID][col], g_szColumnTitles[MESSAGE_ID][col], 256, szPath);
	}
	GetPrivateProfileString(strSection, "Title", g_szTabItems[MESSAGE_ID], g_szTabItems[MESSAGE_ID], 16, szPath);

	// load usage labels
	strSection.Format("HEADER-%s", g_szTabItems[USAGE_ID]);
	for(col = 0; col < MAX_USAGE_STR; col ++) {
		GetPrivateProfileString(strSection, g_szUsageItems[col], g_szUsageItems[col], g_szUsageItems[col], 256, szPath);
	}
	GetPrivateProfileString(strSection, "Title", g_szTabItems[USAGE_ID], g_szTabItems[USAGE_ID], 16, szPath);

	// load miscellaneous text
	strSection.Format("HEADER-MISC");
	for(col = 0; col < MAX_MISC_STR; col ++) {
		GetPrivateProfileString(strSection, g_szMiscItems[col], g_szMiscItems[col], g_szMiscItems[col], 256, szPath);
	}

	// load menu items
	CString strItem, strItemNoAmp;
	char szItem[256];
	int i, is;
	for(i = 0; i < m_MainMenu.GetMenuItemCount(); i ++) {
		m_MainMenu.GetMenuString(i, strItem, MF_BYPOSITION);
		strItemNoAmp = strItem;	strItemNoAmp.Remove('&');
		strSection.Format("MENU-%s", strItemNoAmp);
		GetPrivateProfileString(strSection, "Title", strItem, szItem, 256, szPath);
		m_MainMenu.ModifyMenu(i, MF_BYPOSITION|MF_STRING, 0, szItem); 
		CMenu* pSubMenu = m_MainMenu.GetSubMenu(i);
		if(!pSubMenu) continue;
		for(is = 0; is < pSubMenu->GetMenuItemCount(); is ++) {
			pSubMenu->GetMenuString(is, strItem, MF_BYPOSITION);
			if(strItem.IsEmpty()) continue;
			strItemNoAmp = strItem;	strItemNoAmp.Remove('&');
			GetPrivateProfileString(strSection, strItemNoAmp, strItem, szItem, 256, szPath);
			pSubMenu->ModifyMenu(is, MF_BYPOSITION|MF_STRING, pSubMenu->GetMenuItemID(is), szItem); 
		}
	}
	for(i = 0; i < m_ContextMenu.GetMenuItemCount(); i ++) {
		m_ContextMenu.GetMenuString(i, strItem, MF_BYPOSITION);
		strItemNoAmp = strItem;	strItemNoAmp.Remove('&');
		strSection.Format("MENU-%s", strItemNoAmp);
		GetPrivateProfileString(strSection, "Title", strItem, szItem, 256, szPath);
		m_ContextMenu.ModifyMenu(i, MF_BYPOSITION|MF_STRING, 0, szItem); 
		CMenu* pSubMenu = m_ContextMenu.GetSubMenu(i);
		if(!pSubMenu) continue;
		for(is = 0; is < pSubMenu->GetMenuItemCount(); is ++) {
			pSubMenu->GetMenuString(is, strItem, MF_BYPOSITION);
			if(strItem.IsEmpty()) continue;
			strItemNoAmp = strItem;	strItemNoAmp.Remove('&');
			GetPrivateProfileString(strSection, strItemNoAmp, strItem, szItem, 256, szPath);
			pSubMenu->ModifyMenu(is, MF_BYPOSITION|MF_STRING, pSubMenu->GetMenuItemID(is), szItem); 
		}
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
		typedef DWORD (CALLBACK* GetFn)();
		GetFn fn;
		fn = (GetFn)GetProcAddress(m_hIdleDll, "IdleTrackerGetLastTickCount");
		if(fn) {
			return GetTickCount() - fn();
		} else {
			typedef void (CALLBACK* TermFn)();
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
// CMainWindow::SetTimeOut
// arguments:	void
// returns:		void
// function:	creates a thread to signal a timeout
void CMainWindow::SetTimeOut()
{
	CreateThread(NULL, 0, TimeOutThreadProc, GetSafeHwnd(), NULL, NULL);
}

//////////
// CMainWindow::TimeOutThreadProc
// arguments:	hWnd: handle to window to signal cast as LPVOID
// returns:		true for success, false otherwise
// function:	sleeps for some time then signals the given window
DWORD CMainWindow::TimeOutThreadProc(LPVOID hWnd)
{
	CWnd* pWnd = CWnd::FromHandle((HWND)hWnd);
	Sleep(GUI_REFRESH);
	pWnd->SendMessage(WM_TIMER, 0, 0);
	return 1;
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

	int nGraphicsMsg = RegisterWindowMessage("BOINC_GFX_MODE");
	if(nGraphicsMsg == message) {

		// if no active tasks, runs the standard "blank" screen
		// saver in the system directory
		if(gstate.active_tasks.active_tasks.size() == 0) {
			char szCmd[256];
			GetSystemDirectory(szCmd, 256);
			strcat(szCmd, "\\Blank Screen.scr");
			PROCESS_INFORMATION process_info;
			STARTUPINFO startup_info;
			memset( &process_info, 0, sizeof( process_info ) );
			memset( &startup_info, 0, sizeof( startup_info ) );
			startup_info.cb = sizeof(startup_info);
			startup_info.lpReserved = NULL;
			startup_info.lpDesktop = "";
			CreateProcess(  szCmd,	// path to the screen saver
							" -S",	// run in saver mode
							NULL,					// no process security attributes
							NULL,					// no thread security attribute
							FALSE,					// doesn't inherit handles
							CREATE_NEW_PROCESS_GROUP|CREATE_NO_WINDOW|IDLE_PRIORITY_CLASS,
							NULL,					// same environment
							NULL,				// start in the standard client directory
							&startup_info,
							&process_info );
		}
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
	CAboutDialog dlg(IDD_ABOUTBOX);
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
// CMainWindow::OnCommandConnectionConnectNow
// arguments:	void
// returns:		void
// function:	causes the client to connect to the network
void CMainWindow::OnCommandConnectionConnectNow()
{
	bool bOldConfirm = gstate.global_prefs.confirm_before_connecting;
	gstate.global_prefs.confirm_before_connecting = false;
	NetOpen();
	gstate.global_prefs.confirm_before_connecting = bOldConfirm;
	NetClose();
}

//////////
// CMainWindow::OnCommandConnectionHangupIfDialed
// arguments:	void
// returns:		void
// function:	changes preference and menu item
void CMainWindow::OnCommandConnectionHangupIfDialed()
{
	gstate.global_prefs.hangup_if_dialed = !gstate.global_prefs.hangup_if_dialed;
	CMenu* ConMenu = NULL;
	ConMenu = m_MainMenu.GetSubMenu(2);
	if(ConMenu) {
		if(gstate.global_prefs.hangup_if_dialed) ConMenu->CheckMenuItem(ID_CONNECTION_HANGUPIFDIALED, MF_CHECKED);
		else ConMenu->CheckMenuItem(ID_CONNECTION_HANGUPIFDIALED, MF_UNCHECKED);
	}
}

//////////
// CMainWindow::OnCommandConnectionConfirmBeforeConnecting
// arguments:	void
// returns:		void
// function:	changes preference and menu item
void CMainWindow::OnCommandConnectionConfirmBeforeConnecting()
{
	gstate.global_prefs.confirm_before_connecting = !gstate.global_prefs.confirm_before_connecting;
	CMenu* ConMenu = NULL;
	ConMenu = m_MainMenu.GetSubMenu(2);
	if(ConMenu) {
		if(gstate.global_prefs.confirm_before_connecting) ConMenu->CheckMenuItem(ID_CONNECTION_CONFIRMBEFORECONNECTING, MF_CHECKED);
		else ConMenu->CheckMenuItem(ID_CONNECTION_CONFIRMBEFORECONNECTING, MF_UNCHECKED);
	}
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
	gstate.cleanup_and_exit();
	PostQuitMessage(0);
	KillTimer(ID_TIMER);

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
	m_ContextMenu.DestroyMenu();

	// free dll and idle detection
	if(m_hIdleDll) {
		typedef void (CALLBACK* TermFn)();
		TermFn fn;
		fn = (TermFn)GetProcAddress(m_hIdleDll, "IdleTrackerTerm");
		if(!fn) {
			show_message("Error in DLL \"boinc.dll\"", "low");
		} else {
			fn();
		}
		FreeLibrary(m_hIdleDll);
		m_hIdleDll = NULL;
	}

	SaveUserSettings();
	SaveListControls();
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
	char curDir[512];
	char* szTitles[MAX_COLS];
	int i;

    if (CWnd::OnCreate(lpcs) == -1) {
		return -1;
	}

    g_myWnd = this;
	m_nIconState = ICON_OFF;
	m_bMessage = false;
	m_bRequest = false;
	m_nContextItem = -1;

	// load menus
	m_ContextMenu.LoadMenu(IDR_CONTEXT);
	m_MainMenu.LoadMenu(IDR_MAINFRAME);
	SetMenu(&m_MainMenu);

	LoadLanguage();

	// create project list control
	m_ProjectListCtrl.Create(LVS_REPORT|WS_CHILD|WS_BORDER|WS_VISIBLE, CRect(0,0,0,0), this, PROJECT_ID);
	m_ProjectListCtrl.SetExtendedStyle(m_ProjectListCtrl.GetExtendedStyle()|LVS_EX_HEADERDRAGDROP|LVS_EX_FULLROWSELECT);
	for(i = 0; i < MAX_COLS; i ++) szTitles[i] = g_szColumnTitles[PROJECT_ID][i];
	m_ProjectListCtrl.SetMenuItems(szTitles, PROJECT_COLS);
	for(i = 0; i < PROJECT_COLS; i ++) {
		m_ProjectListCtrl.InsertColumn(i, g_szColumnTitles[PROJECT_ID][i], LVCFMT_LEFT, DEF_COL_WIDTH, -1);
	}

	// create result list control
	m_ResultListCtrl.Create(LVS_REPORT|WS_CHILD|WS_BORDER|WS_VISIBLE, CRect(0,0,0,0), this, RESULT_ID);
	m_ResultListCtrl.SetExtendedStyle(m_ResultListCtrl.GetExtendedStyle()|LVS_EX_HEADERDRAGDROP|LVS_EX_FULLROWSELECT);
	m_ResultListCtrl.ModifyStyle(WS_VISIBLE, 0);
	for(i = 0; i < MAX_COLS; i ++) szTitles[i] = g_szColumnTitles[RESULT_ID][i];
	m_ResultListCtrl.SetMenuItems(szTitles, RESULT_COLS);
	for(i = 0; i < RESULT_COLS; i ++) {
		m_ResultListCtrl.InsertColumn(i, g_szColumnTitles[RESULT_ID][i], LVCFMT_LEFT, DEF_COL_WIDTH, -1);
	}

	// create xfer list control
	m_XferListCtrl.Create(LVS_REPORT|WS_CHILD|WS_BORDER|WS_VISIBLE, CRect(0,0,0,0), this, XFER_ID);
	m_XferListCtrl.SetExtendedStyle(m_XferListCtrl.GetExtendedStyle()|LVS_EX_HEADERDRAGDROP|LVS_EX_FULLROWSELECT);
	m_XferListCtrl.ModifyStyle(WS_VISIBLE, 0);
	for(i = 0; i < MAX_COLS; i ++) szTitles[i] = g_szColumnTitles[XFER_ID][i];
	m_XferListCtrl.SetMenuItems(szTitles, XFER_COLS);
	for(i = 0; i < XFER_COLS; i ++) {
		m_XferListCtrl.InsertColumn(i, g_szColumnTitles[XFER_ID][i], LVCFMT_LEFT, DEF_COL_WIDTH, -1);
	}

	// create message edit control
	// create xfer list control
	m_MessageListCtrl.Create(LVS_REPORT|WS_CHILD|WS_BORDER|WS_VISIBLE, CRect(0,0,0,0), this, MESSAGE_ID);
	m_MessageListCtrl.SetExtendedStyle(m_MessageListCtrl.GetExtendedStyle()|LVS_EX_HEADERDRAGDROP|LVS_EX_FULLROWSELECT);
	m_MessageListCtrl.ModifyStyle(WS_VISIBLE, 0);
	for(i = 0; i < MAX_COLS; i ++) szTitles[i] = g_szColumnTitles[MESSAGE_ID][i];
	m_MessageListCtrl.SetMenuItems(szTitles, MESSAGE_COLS);
	for(i = 0; i < MESSAGE_COLS; i ++) {
		int width = DEF_COL_WIDTH;
		if(i == 1) width *= 1.5;
		if(i == 2) width *= 4;
		m_MessageListCtrl.InsertColumn(i, g_szColumnTitles[MESSAGE_ID][i], LVCFMT_LEFT, width, -1);
	}

	// create usage pie control
	m_UsagePieCtrl.Create(WS_CHILD|WS_BORDER|WS_VISIBLE, CRect(0,0,0,0), this, USAGE_ID);
	m_UsagePieCtrl.ModifyStyle(WS_VISIBLE, 0);
	m_UsagePieCtrl.AddPiece(g_szUsageItems[0], GetPieColor(0), 0);
	m_UsagePieCtrl.AddPiece(g_szUsageItems[1], GetPieColor(1), 0);
	m_UsagePieCtrl.AddPiece(g_szUsageItems[2], GetPieColor(2), 0);
	m_UsagePieCtrl.AddPiece(g_szUsageItems[3], GetPieColor(3), 0);

	// set up image list for tab control
	m_TabIL.Create(16, 16, ILC_COLOR8|ILC_MASK, MAX_TABS, 1);
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
	m_TabCtrl.InsertItem(1, g_szTabItems[0], 0);
	m_TabCtrl.InsertItem(2, g_szTabItems[1], 1);
	m_TabCtrl.InsertItem(3, g_szTabItems[2], 2);
	m_TabCtrl.InsertItem(4, g_szTabItems[3], 3);
	m_TabCtrl.InsertItem(5, g_szTabItems[4], 4);

	// make all fonts the same nice font
	CFont* pFont;
	pFont = m_ProjectListCtrl.GetFont();
	LOGFONT lf;
	ZeroMemory(&lf, sizeof(LOGFONT));
	pFont->GetLogFont(&lf);
	m_Font.CreateFontIndirect(&lf);
	m_TabCtrl.SetFont(&m_Font);
	m_UsagePieCtrl.SetFont(&m_Font);

	// Determine the OS version
	UtilInitOSVersion();

	// Set the current directory to the default
	UtilGetRegStr("ClientDir", curDir);
	if (strlen(curDir))
		SetCurrentDirectory(curDir);

	// add status icon to taskbar
	SetStatusIcon(ICON_NORMAL);

	// take care of other things
	// 
	// Redirect stdout and stderr to files
    freopen(STDOUT_FILE_NAME, "w", stdout);
    freopen(STDERR_FILE_NAME, "w", stderr);

	// Check what (if any) activities should be logged
    read_log_flags();

	LPSTR command_line;
	char* argv[100];
	int argc;

	command_line = GetCommandLine();
    argc = parse_command_line( command_line, argv );
    gstate.parse_cmdline(argc, argv);

    int retval = gstate.init();
    if (retval) {
		OnCommandExit();
		return 0;
	}
	SetTimeOut();

	// load dll and start idle detection
	m_hIdleDll = LoadLibrary("boinc.dll");
	if(!m_hIdleDll) {
		show_message("Can't load \"boinc.dll\", will not be able to determine idle time", "high");
	} else {
		typedef BOOL (CALLBACK* InitFn)();
		InitFn fn;
		fn = (InitFn)GetProcAddress(m_hIdleDll, "IdleTrackerInit");
		if(!fn) {
			show_message("Error in DLL \"boinc.dll\", will not be able to determine idle time", "low");
			FreeLibrary(m_hIdleDll);
			m_hIdleDll = NULL;
		} else {
			if(!fn()) {
				show_message("Error in DLL \"boinc.dll\", will not be able to determine idle time", "low");
				FreeLibrary(m_hIdleDll);
				m_hIdleDll = NULL;
			}
		}
	}

	LoadUserSettings();
	LoadListControls();

	CMenu* ConMenu = NULL;
	ConMenu = m_MainMenu.GetSubMenu(2);
	if(ConMenu) {
		if(gstate.global_prefs.hangup_if_dialed) ConMenu->CheckMenuItem(ID_CONNECTION_HANGUPIFDIALED, MF_CHECKED);
		else ConMenu->CheckMenuItem(ID_CONNECTION_HANGUPIFDIALED, MF_UNCHECKED);
		if(gstate.global_prefs.confirm_before_connecting) ConMenu->CheckMenuItem(ID_CONNECTION_CONFIRMBEFORECONNECTING, MF_CHECKED);
		else ConMenu->CheckMenuItem(ID_CONNECTION_CONFIRMBEFORECONNECTING, MF_UNCHECKED);
	}

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
				pContextMenu = m_ContextMenu.GetSubMenu(nMenuId);
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
	if(m_bRequest) {
		m_bRequest = false;
		if(RequestNetConnect()) OnCommandConnectionConnectNow();
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
	RECT rt = {EDGE_BUFFER, TOP_BUFFER, cx-EDGE_BUFFER, cy-EDGE_BUFFER*2};
	RECT irt = {0, 0, 0, 0};
	if(m_TabCtrl.GetSafeHwnd()) {
		m_TabCtrl.MoveWindow(&rt, false);
		m_TabCtrl.GetItemRect(0, &irt);

		// calculate the rects for other controls inside the tab control
		RECT srt = {rt.left+EDGE_BUFFER, irt.bottom+EDGE_BUFFER*2+TOP_BUFFER, rt.right-EDGE_BUFFER, rt.bottom-EDGE_BUFFER};
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
		CMenu* pSubmenu;
		pSubmenu = m_ContextMenu.GetSubMenu(STATUS_MENU);
		if(m_bSuspend) {
			pSubmenu->EnableMenuItem(ID_STATUSICON_SUSPEND, MF_GRAYED);
			pSubmenu->EnableMenuItem(ID_STATUSICON_RESUME, MF_ENABLED);
		} else {
			pSubmenu->EnableMenuItem(ID_STATUSICON_SUSPEND, MF_ENABLED);
			pSubmenu->EnableMenuItem(ID_STATUSICON_RESUME, MF_GRAYED);
		}
		pSubmenu->TrackPopupMenu(TPM_LEFTALIGN|TPM_RIGHTBUTTON, point.x, point.y, this);
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

//////////
// CMainWindow::OnTimer
// arguments:	uEventID: timer's id
// returns:		void
// function:	checks idle time, updates client state, flushed output streams,
//				and updates gui display.
void CMainWindow::OnTimer(UINT uEventID)
{
	// update state and gui
	while(gstate.do_something());
	NetCheck(); // need to check if network connection can be terminated
	fflush(stdout);
	fflush(stderr);
	if(!IsSuspended()) {
		// check user's idle time for suspension of apps
		if (gstate.global_prefs.idle_time_to_run > 0) {
			if (GetUserIdleTime() > 1000 * gstate.global_prefs.idle_time_to_run) {
				gstate.user_idle = true;
			} else {
				gstate.user_idle = false;
			}
		} else {
			gstate.user_idle = true;
		}

		UpdateGUI(&gstate);
	}
	SetTimeOut();
}
