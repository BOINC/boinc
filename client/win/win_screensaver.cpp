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

#include "graphics_api.h"
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
	int BOINC_GFX_MODE_MSG;
	int oldval;

	// Set a flag in the system to indicate that we're in screensaver mode
	SystemParametersInfo(SPI_SCREENSAVERRUNNING,1,&oldval,0);

	BOINC_SS_START_MSG = RegisterWindowMessage( "BOINC_SS_START" );
	BOINC_GFX_MODE_MSG = RegisterWindowMessage( "BOINC_GFX_MODE" );

	PostMessage(HWND_BROADCAST, BOINC_GFX_MODE_MSG, 0, MODE_FULLSCREEN);

	// We should go into a wait state here

	// Unset the system screensaver flag
	SystemParametersInfo(SPI_SCREENSAVERRUNNING,0,&oldval,0);
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
	unsigned long screen_blank, blank_time;
	char buf[256];

	switch (msg) {
		case WM_INITDIALOG:
			UtilGetRegKey( "Blank", screen_blank );
			CheckDlgButton(hwnd,IDC_BLANK,screen_blank);
			UtilGetRegKey( "Blank Time", blank_time );
			sprintf( buf, "%d", blank_time );
			SetDlgItemText(hwnd,IDC_BLANK_TIME,buf);
			return TRUE;
		case WM_COMMAND:
			int id=LOWORD(wParam);
			if (id==IDOK) {
				screen_blank=(IsDlgButtonChecked(hwnd,IDC_BLANK)==BST_CHECKED);
				UtilSetRegKey( "Blank", screen_blank );
				GetDlgItemText(hwnd, IDC_BLANK_TIME, buf, 256 );
				blank_time=atoi(buf);
				UtilSetRegKey( "Blank Time", blank_time );
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

