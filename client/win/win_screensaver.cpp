
// Based on code by Lucian Wischik

#include <afxwin.h>
#include <windows.h>

#include "boinc_ss_res.h"
#include "win_util.h"

//#define SS_DEBUG
#ifdef SS_DEBUG
FILE* fout;
#endif

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

#ifdef SS_DEBUG
    fout = fopen("C:/temp/boinc_scr.txt", "w");
    fprintf(fout, "cmdline: %s\n", c);
#endif

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
#ifdef SS_DEBUG
    fprintf(fout, "screensaver exiting\n");
    fflush(fout);
#endif
	return 0;
}

void RunSaver( void ) {
	int BOINC_SS_START_MSG;
	BOOL flag;
	char client_path[256], client_dir[256], cmdline[256];
    PROCESS_INFORMATION process_info;
    STARTUPINFO startup_info;
	HANDLE boinc_mutex;

	// If BOINC isn't running, then start it up!
	boinc_mutex = CreateMutex(NULL, false, RUN_MUTEX);
	if(boinc_mutex != NULL && GetLastError() != ERROR_ALREADY_EXISTS) {
		CloseHandle(boinc_mutex);
		// Get the path to the client
		if (UtilGetRegStr( "ClientPath", client_path ) ||
			 UtilGetRegStr( "ClientDir", client_dir )) 
		{
			return /* error */;
		}

		memset( &process_info, 0, sizeof( process_info ) );
		memset( &startup_info, 0, sizeof( startup_info ) );
		startup_info.cb = sizeof(startup_info);
		startup_info.lpReserved = NULL;
		startup_info.lpDesktop = "";
#ifdef SS_DEBUG
        fprintf(fout, "launching core client: %s\n", client_path);
        fprintf(fout, "dir: %s\n", client_dir);
#endif
        // tell core client it's started by SS
        sprintf(cmdline, "\"%s\" -saver", client_path);

		// Start the core client
		flag = CreateProcess(
            client_path,	            // path of core client executable
			cmdline,
			NULL, NULL,					// no process, thread security attributes
			FALSE,						// doesn't inherit handles
			CREATE_NEW_PROCESS_GROUP,
			NULL,						// same environment
			client_dir,					// start in the standard client directory
			&startup_info,
			&process_info
        );

        if (!flag) {
            int retval = GetLastError();
#ifdef SS_DEBUG
            fprintf(fout, "Can't launch BOINC core client: error %d\n", retval);
#endif
        } else {
#ifdef SS_DEBUG
            fprintf(fout, "Launched BOINC core client\n");
#endif
		    // wait up to 3 seconds for BOINC to start
		    //WaitForInputIdle(process_info.hProcess, 3000);
        }
    } else {
#ifdef SS_DEBUG
        fprintf(fout, "core client already running, sending msg\n");
#endif
	    BOINC_SS_START_MSG = RegisterWindowMessage( START_SS_MSG );
	    PostMessage(HWND_BROADCAST, BOINC_SS_START_MSG, 0, 0);
    }
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
#ifdef SS_DEBUG
    fprintf(fout, "Doing preview window\n");
#endif
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
	int retval;

	switch (msg) {
		case WM_INITDIALOG:
			// make sure you check return value of registry queries
			// in case the item in question doesn't happen to exist.
			retval=UtilGetRegKey( REG_BLANK_NAME, screen_blank );
			if ( retval < 0 ) { screen_blank=0; }
			CheckDlgButton(hwnd,IDC_BLANK,screen_blank);
			retval=UtilGetRegKey( REG_BLANK_TIME, blank_time );
			if ( retval < 0 ) { blank_time=0; }
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

