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

#ifdef _WIN32

#include <windows.h>
#include <string.h>
#include <stdio.h>
#include <process.h>
#include <pbt.h>

#include "win_net.h"

//#include "win_globals.h"

#define WIN_WIDTH 760
#define WIN_HEIGHT 491

//
//	DATA
//

static char ParentName[] = "BOINCClientParent";
static char ChildName[]  = "BOINCClientChild";


//
//	PROTOTYPES
//
LRESULT CALLBACK ParentFunc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK ChildFunc(HWND, UINT, WPARAM, LPARAM);
void SafeShutDown(void);


//////////////////////////////////////////////////////////////////////////////////////
//
// Function    : WinMain
//
//
///////////////////////////////////////////////////////////////////////////////////////

int main(int argc, char** argv);


int WINAPI WinMain( HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR Args, int WinMode )
{
	HWND parent, child;
	HDC hdc;
	MSG msg;
	WNDCLASSEX wcl;
	int screenw,w,h;
	char *command_line;
	char **argv;
	int argc;

	NetOpen();
	//command_line = GetCommandLine();
	//argv = CommandLineToArgvW( command_line, &argc );
	main( argc, argv );
	// If instance of client is already running, quit
	//
	/*HWND awin = FindWindow( ParentName, NULL );
	if ( awin ) 
		return 1;

	// Create invisible parent window of type TOOL_WINDOW:  This
	// prevents program from showing up in the taskbar
	//
	wcl.hInstance = hInst;
	wcl.lpszClassName = ParentName;
	//wcl.lpfnWndProc = ParentFunc;
	wcl.style = 0;
	wcl.cbSize = sizeof(WNDCLASSEX);
	//wcl.hIcon = LoadIcon( hInst, MAKEINTRESOURCE(IDI_GREEN_K) );
	//wcl.hIconSm = LoadIcon( hInst, MAKEINTRESOURCE(IDI_GREEN_K) );
	wcl.hCursor = LoadCursor( NULL, IDC_ARROW );
	wcl.lpszMenuName = NULL;
	wcl.cbClsExtra = 0;
	wcl.cbWndExtra = 4;
	wcl.hbrBackground = (HBRUSH) GetStockObject( BLACK_BRUSH );

	if ( !RegisterClassEx( &wcl) ) return 0;

	parent = CreateWindowEx( 
		0, // WS_EX_TOOLWINDOW,
		ParentName,
		"SETI@Home Client",
		WS_OVERLAPPEDWINDOW|WS_MINIMIZEBOX,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		HWND_DESKTOP,
		NULL,
		hInst,
		NULL );


	UpdateWindow( parent );

	return 0;

	// Check display coordinates; if where on a small
	// screen (640x480), use Windows default for width
	// and height;
	hdc = GetDC( parent );
	screenw = ( GetDeviceCaps( hdc, HORZRES ) );
	ReleaseDC( parent, hdc );
	if ( screenw < 800 )
	{
		w = CW_USEDEFAULT;
		h = CW_USEDEFAULT;
	}
	else
	{
		w = WIN_WIDTH;
		h = WIN_HEIGHT;
	}



	// Create child window. Normal type

	wcl.hInstance = hInst;
	wcl.lpszClassName = ChildName;
	wcl.lpfnWndProc = ChildFunc;
	wcl.style = 0;
	wcl.cbSize = sizeof(WNDCLASSEX);
	//wcl.hIcon = LoadIcon( hInst, MAKEINTRESOURCE(IDI_GREEN_K) );
	//wcl.hIconSm = LoadIcon( hInst, MAKEINTRESOURCE(IDI_GREEN_K) );
	wcl.hCursor = LoadCursor( NULL, IDC_ARROW );
	wcl.lpszMenuName = "SETIMENU";
	wcl.cbClsExtra = 0;
	wcl.cbWndExtra = 4;
	wcl.hbrBackground = (HBRUSH) GetStockObject( BLACK_BRUSH );

	if ( !RegisterClassEx( &wcl) ) return 0;

	child = CreateWindowEx( 
		0,
		ChildName,
		"SETI@Home Client",
		WS_OVERLAPPEDWINDOW|WS_EX_TOPMOST,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		w,
		h,
		parent,
		NULL,
		hInst,
		NULL );

*/
	/*if ( !Initialize( hInst, child ) ) return 1;
	Globals->parent = parent;

	SetTimer( child, 313, 1000/TIMER_EVENTS_PER_SECOND, NULL );*/


//#ifdef DONT_MINIMIZE
//	Globals->status |= STATUS_FLAG_MAXIMIZED;
//	ShowWindow( child, WinMode );
//#endif

//#ifdef START_WORKER
//	MessageSend( MESSAGE_ANALYSISON );
//#endif


	UpdateWindow( child );


	// Turn the taskbar icon on
	//
	//UtilSetIcon( ICON_NORMAL );

	// Force initial paint
	//
	//SendMessage( Globals->child, WM_PAINT, 0, 0 );

	/*if ( strcmp( Args, "-min" ) )
		// maximizing sets process priority to normal
		MessageSend( MESSAGE_MAXIMIZE );
	else
		// do explicit minimize call
		//
		MessageSend( MESSAGE_MINIMIZE );*/
		//UtilSetProcessPriority( SETI_PRIORITY_LOW );

	/*while( GetMessage( &msg, NULL, 0, 0 ) )
	{
		//Loop();
		TranslateMessage( &msg );
		DispatchMessage( &msg );
	}

	SafeShutDown();
	
	DeInitialize();						// Calls UtilEndWorker()

	UtilSetIcon( ICON_OFF );*/

	return msg.wParam;
}


// If the worker thread is running, exiting might kill the worker 
//  thread just after updating one of the checksummed files but 
//  before writing the revised checksum.
//  Since check_suspend_flag() is never called when we are in this 
//  unsafe situation, we set a flag to tell check_suspend_flag() 
//  to kill the worker thread before we actually exit.
//  For safety, we exit anyway if no response after 2 seconds.
/*void SafeShutDown()
{
	if (Globals->status & STATUS_FLAG_ANALYSISON) {
		Globals->status &= ~STATUS_FLAG_ANALYSISON;
		Globals->status |= STATUS_FLAG_EXIT_CLIENT;
		long loopStart = clock();
		while(Globals->status & STATUS_FLAG_WORKERON) {
			Sleep( 100 );
			if ( (clock() - loopStart) > (2 * CLOCKS_PER_SEC))	// Wait 2 seconds max
	          break;
		}
	}
}


// Called from the worker thread to suspend itself until all pending 
//	drawing completes.  For safety, gives up after 5 seconds.
void WaitForPaint()
{
	long loopStart = clock();
	
	while (gdata->anything_to_draw())
	{
		if (Globals->status & STATUS_FLAG_EXIT_CLIENT)
			return;
			
		if ( (clock() - loopStart) > (5 * CLOCKS_PER_SEC))	// Wait 5 second max
			return;

		if (Globals->status & STATUS_FLAG_SAVER)
		{
			if (Globals->status & STATUS_FLAG_BLANKED)
				return;

			// MSDN Library says: Threads that do not contain windows should
			//	use the Sleep function with a sleep time of zero to give up 
			//	the remainder of their current time slice.
			Sleep(0);
		}
		else
		{
			if (!Globals->status & STATUS_FLAG_MAXIMIZED)
				return;

			// MSDN Library says: The SendMessage function ... does not 
			//	return until the window procedure has processed the message	
			SendMessage( Globals->child, WM_TIMER, 0, 0 );	// Don't wait for timer
		}
	}
}


LRESULT CALLBACK ParentFunc( HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam )
{
	WORD cmd;

	switch( message )
	{
	case WM_PAINT:
		break;

	case WM_CLOSE:
		DestroyWindow( hwnd );
		break;

	case WM_DESTROY:
		PostQuitMessage(0);
		break;


	case WM_COMMAND:
		cmd = LOWORD(wParam);         
		switch ( cmd )
		{
		case ID_FILE_EXIT:
		case ID_POPUP_EXIT:
			PostQuitMessage(0);
			break;
		}

	default:
		return DefWindowProc( hwnd, message, wParam, lParam );
	}
	return 0;
}


LRESULT CALLBACK ChildFunc( HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam )
{
	WORD cmd;
	HDC hdc;

	switch( message )
	{
	case WM_PAINT:
		Paint( hwnd );
		break;

	case WM_CLOSE:
		// DestroyWindow( hwnd );
		MessageSend( MESSAGE_MINIMIZE );
		break;

	case WM_TIMER:
		UtilTimer( hwnd );
		break;

	case WM_QUERYNEWPALETTE:
		hdc = GetDC( hwnd );
		SelectPalette( hdc, Globals->hpalette, FALSE );
		RealizePalette( hdc );
		ReleaseDC( hwnd, hdc );
		break;

	case WM_PALETTECHANGED:
		hdc = GetDC( hwnd );
		SelectPalette( hdc, Globals->hpalette, TRUE );
		RealizePalette( hdc );
		ReleaseDC( hwnd, hdc );
		break;

	case WM_DESTROY:
		PostQuitMessage(0);
		break;
		
	case WM_QUERYENDSESSION:
		SafeShutDown();
		return TRUE;

	case SETI_NOTIFY_ICON:
		if ( lParam == WM_LBUTTONDBLCLK )
		{
			MessageSend( MESSAGE_MAXIMIZE );
		}
		else if ( lParam == WM_RBUTTONDOWN )
		{
			POINT point;
				
			// This is necessary in order to cancel menu by clicking outside of it.
			// (stupid, stupid windows...)
			//
			SetForegroundWindow( Globals->child );

			GetCursorPos( &point );
			TrackPopupMenuEx( Globals->PopupSubMenu,
				TPM_RIGHTALIGN | TPM_BOTTOMALIGN,
				point.x,
				point.y,
				Globals->child,
				NULL );
		}
		break;

	case WM_SIZE:
		switch (wParam)
		{
		case SIZE_MINIMIZED:
			MessageSend( MESSAGE_MINIMIZE );
			break;

		case SIZE_MAXIMIZED:
			MessageSend( MESSAGE_MAXIMIZE );
			break;
			
//		case SIZE_RESTORED:
//		case SIZE_MAXSHOW:
//		case SIZE_MAXHIDE:
		}
		break;

	case WM_KEYDOWN:
		switch ( wParam )
		{
		case VK_F1:
			UtilOpenHTMLHelp();
			break;
		}
		break;

	case WM_COMMAND:
		// Suspend calls to grender() for faster response to menu commands
		Globals->status |= STATUS_FLAG_IN_MENU_CMD;   
		     
		cmd = LOWORD(wParam); 
		switch ( cmd )
		{
		case ID_FILE_EXIT:
			MessageSend( MESSAGE_MINIMIZE );
			break;

		case ID_POPUP_EXIT:
			PostQuitMessage(0);
			break;

		case ID_FILE_CONNECTNOW:
		case ID_POPUP_CONNECTNOW:
			if ( Globals->net_state == NETSTATE_WAIT || 
									Globals->net_state == NETSTATE_NOTIFY ) {
				MessageSend( MESSAGE_MAXIMIZE );	// So user can see network progress
				NetSetState( NETSTATE_PERMISSION );
			}
			break;

		case ID_SETTINGS_WELCOMEMENU:
			{
				connection.UserInfo = 1;
				// UtilMakeUserInfo( MAKE_UI_WELCOMEMENU );
				UtilStartLogin( MAKE_UI_WELCOMEMENU );
			}
			break;

		case ID_SETTINGS_CONFIG:
		case ID_POPUP_CONFIG:
			ConfigDlg();
			if ( Globals->status & STATUS_FLAG_MAXIMIZED ||
				 Globals->status & STATUS_FLAG_SAVER ||
				 config.AlwaysRun )
				MessageSend( MESSAGE_ANALYSISON );
			else
				MessageSend( MESSAGE_ANALYSISOFF );
			break;

		case ID_SETTINGS_PROXY:
			ProxyDlg();
			break;

		case ID_HELP_HELP:
		case ID_POPUP_HELP:
			UtilOpenHTMLHelp();
			break;

		case ID_HELP_ABOUTSETI:
			AboutDlg();
			break;

		case ID_POPUP_MAXIMIZE:
			MessageSend( MESSAGE_MAXIMIZE );
			break;

		}
		// resume calls to grender()
		Globals->status &= ~STATUS_FLAG_IN_MENU_CMD;        
		break;


	case WM_POWERBROADCAST:	// Don't accumulate CPU time if hibernating
		switch (wParam)
		{
		case PBT_APMSUSPEND:
		case PBT_APMSTANDBY:
			Globals->status |= STATUS_FLAG_HIBERNATING;
			UtilStopCPUClock();
			break;
//		case PBT_APMRESUMEAUTOMATIC:
		case PBT_APMRESUMESTANDBY:
		case PBT_APMRESUMECRITICAL:
		case PBT_APMRESUMESUSPEND:
			Globals->status &= ~STATUS_FLAG_HIBERNATING;
			UtilStartCPUClock();
			break;
		}
	// Fall through to default handler
		
	default:
		return DefWindowProc( hwnd, message, wParam, lParam );
	}
	return 0;
}
*/
#endif



