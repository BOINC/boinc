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
// Based on code by Lucian Wischik

#include <afxwin.h>

#include "boinc_ss_res.h"
#include "win_util.h"

void RunSaver( void );
BOOL CALLBACK ConfigDialogProc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam);
void DoPreviewWindow(HWND hparwnd);
LRESULT CALLBACK SaverWindowProc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam);

enum TScrMode {smNone,smConfig,smPassword,smPreview,smSaver};
HINSTANCE hInstance=NULL;
HWND hScrWindow=NULL;
TScrMode ScrMode=smNone;

int WINAPI WinMain(HINSTANCE h,HINSTANCE,LPSTR,int) {
	char *cmd_line, *c;
	HWND hwnd=NULL;

	cmd_line=GetCommandLine();
	c = cmd_line;

	// Skip past the screensaver name
	if (*c=='\"') {
		c++;
		while (*c!=0 && *c!='\"') c++;
	} else {
		while (*c!=0 && *c!=' ') c++;
	}
	if (*c!=0) c++;
	while (*c==' ') c++;
	if (*c==0) {
		ScrMode=smConfig;
		hwnd=NULL;
	} else {
		if (*c=='-' || *c=='/') c++;
		if (*c=='p' || *c=='P' || *c=='l' || *c=='L') {	// Preview mode
			c++;
			while (*c==' ' || *c==':') c++;
			hwnd=(HWND)atoi(c);
			ScrMode=smPreview;
		} else if (*c=='s' || *c=='S') {	// Full screensaver mode
			ScrMode=smSaver;
		} else if (*c=='c' || *c=='C') {	// Configuration dialog mode
			c++;
			while (*c==' ' || *c==':') c++;
			if (*c==0)
				hwnd=GetForegroundWindow();
			else
				hwnd=(HWND)atoi(c);
			ScrMode=smConfig;
		}
		else if (*c=='a' || *c=='A') {		// Password configuration dialog
			c++;
			while (*c==' ' || *c==':') c++;
			hwnd=(HWND)atoi(c);
			ScrMode=smPassword;
		}
	}

	UtilInitOSVersion();

	switch (ScrMode) {
		case smPreview:
			DoPreviewWindow(hwnd);
			break;
		case smSaver:
			RunSaver();
			break;
		case smPassword:
			//ChangePassword(hwnd);
			break;
		case smConfig:
			DialogBox(hInstance,MAKEINTRESOURCE(DLG_CONFIG),hwnd,ConfigDialogProc);
			break;
	}
	return 0;
}

void RunSaver( void ) {
	int BOINC_SS_START_MSG;
	int oldval;
	char client_path[256], client_dir[256];
    PROCESS_INFORMATION process_info;
    STARTUPINFO startup_info;
	HANDLE boinc_mutex;

	// If BOINC isn't running, then start it up!
	boinc_mutex = CreateMutex(NULL, false, RUN_MUTEX);
	if(boinc_mutex != NULL && GetLastError() != ERROR_ALREADY_EXISTS) {
		CloseHandle(boinc_mutex);
		// Get the path to the client
		UtilGetRegStr( "ClientPath", client_path );
		UtilGetRegStr( "ClientDir", client_dir );

		memset( &process_info, 0, sizeof( process_info ) );
		memset( &startup_info, 0, sizeof( startup_info ) );
		startup_info.cb = sizeof(startup_info);
		startup_info.lpReserved = NULL;
		startup_info.lpDesktop = "";

		// Start the client in the background
		oldval = CreateProcess(  client_path,	// path to the client
					"boinc -saver",				// start the screensaver
					NULL, NULL,					// no process, thread security attributes
					FALSE,						// doesn't inherit handles
					CREATE_NEW_PROCESS_GROUP|CREATE_NO_WINDOW|IDLE_PRIORITY_CLASS,
					NULL,						// same environment
					client_dir,					// start in the standard client directory
					&startup_info,
					&process_info );
	}

	BOINC_SS_START_MSG = RegisterWindowMessage( START_SS_MSG );

	PostMessage(HWND_BROADCAST, BOINC_SS_START_MSG, 0, 0);
}

void DoPreviewWindow(HWND hparwnd)
{
	WNDCLASS wc;
	wc.style=CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc=SaverWindowProc;
	wc.cbClsExtra=0;
	wc.cbWndExtra=0;
	wc.hInstance=hInstance;
	wc.hIcon=NULL;
	wc.hCursor=NULL;
	wc.hbrBackground=(HBRUSH)GetStockObject(BLACK_BRUSH);
	wc.lpszMenuName=NULL;
	wc.lpszClassName="ScrClass";
	RegisterClass(&wc);
	RECT rc;
	GetWindowRect(hparwnd,&rc);
	int cx=rc.right-rc.left, cy=rc.bottom-rc.top;
	hScrWindow=CreateWindowEx(0,"ScrClass","SaverPreview",WS_CHILD|WS_VISIBLE,0,0,cx,cy,hparwnd,NULL,hInstance,NULL);
	if (hScrWindow==NULL) return;
	MSG msg;
	while (GetMessage(&msg,NULL,0,0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return;
}


BOOL CALLBACK ConfigDialogProc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam) {
	unsigned long screen_blank=0, blank_time=0;
	char buf[256];

	switch (msg) {
		case WM_INITDIALOG:
			UtilGetRegKey( REG_BLANK_NAME, screen_blank );
			CheckDlgButton(hwnd,IDC_BLANK,screen_blank);
			UtilGetRegKey( REG_BLANK_TIME, blank_time );
			sprintf( buf, "%d", blank_time );
			SetDlgItemText(hwnd,IDC_BLANK_TIME,buf);
			return TRUE;
		case WM_COMMAND:
			int id=LOWORD(wParam);
			if (id==IDOK) {
				screen_blank=(IsDlgButtonChecked(hwnd,IDC_BLANK)==BST_CHECKED);
				UtilSetRegKey( REG_BLANK_NAME, screen_blank );
				GetDlgItemText(hwnd, IDC_BLANK_TIME, buf, 256 );
				blank_time=atoi(buf);
				UtilSetRegKey( REG_BLANK_TIME, blank_time );
			}
			if (id==IDOK || id==IDCANCEL)
				EndDialog(hwnd,id);
			break;
	}
	return FALSE;
}


LRESULT CALLBACK SaverWindowProc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam)
{
	switch (msg) {
	case WM_CREATE:
      //GetCursorPos(&(ss->InitCursorPos));
	  break;
	case WM_SETCURSOR:
		if (ScrMode==smSaver) SetCursor(NULL);
		else SetCursor(LoadCursor(NULL,IDC_ARROW));
		break;
	case WM_SYSCOMMAND:
    if (ScrMode==smSaver)
      { if (wParam==SC_SCREENSAVE) {return FALSE;}
		if (wParam==SC_CLOSE) {return FALSE;}
      }
    break;
	case WM_CLOSE:
	/*if (ScrMode==smSaver && ss->ReallyClose && !ss->IsDialogActive)
      {
        BOOL CanClose=TRUE;
      }
      if (ScrMode==smSaver) return FALSE;*/ // so that DefWindowProc doesn't get called, because it would just DestroyWindow
    break;
	case (WM_DESTROY):
    
      PostQuitMessage(0);
    break;
  }
  return DefWindowProc(hwnd,msg,wParam,lParam);
}

