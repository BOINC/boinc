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

/*		This Code Was Created By Jeff Molofee 2000
 *		A HUGE Thanks To Fredric Echols For Cleaning Up
 *		And Optimizing This Code, Making It More Flexible!
 *		If You've Found This Code Useful, Please Let Me Know.
 *		Visit My Site At nehe.gamedev.net
 *		Adapted to BOINC by Eric Heien
 */

#include <afxwin.h>
#include <gl\gl.h>			// Header File For The OpenGL32 Library
#include <gl\glu.h>			// Header File For The GLu32 Library
#include <stdio.h>

#include "graphics_api.h"
#include "util.h"
#include "win_idle_tracker.h"

HWND		hWnd=NULL;		// Holds Our Window Handle
HINSTANCE	hInstance;		// Holds The Instance Of The Application
BOOL		win_loop_done;
BOOL		has_failed = false;
BOOL		painting = false;
UINT		BOINC_PAINT;

extern bool using_opengl;
extern HANDLE hQuitEvent;

LRESULT	CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);	// Declaration For WndProc
DWORD WINAPI win_graphics_event_loop( LPVOID duff );
BOOL reg_win_class();
BOOL unreg_win_class();

LRESULT CALLBACK WndProc(	HWND	hWnd,			// Handle For This Window
							UINT	uMsg,			// Message For This Window
							WPARAM	wParam,			// Additional Message Information
							LPARAM	lParam)			// Additional Message Information
{
	if (uMsg == BOINC_PAINT) {
		if(has_failed) return 0;
		if(painting) return 0;
		painting = true;

		HWND bwnd;
		if(!(bwnd = (HWND)lParam)) {
			has_failed = true;
			return 0;
		}

		HDC bdc;
		if(!(bdc = GetDC(bwnd))) {
			has_failed = true;
			return 0;
		}

		RECT rt;
		GetClientRect(bwnd, &rt);
		int width = rt.right - rt.left;
		int height = rt.bottom - rt.top;

		PIXELFORMATDESCRIPTOR pfd=				// pfd Tells Windows How We Want Things To Be
		{
			sizeof(PIXELFORMATDESCRIPTOR),				// Size Of This Pixel Format Descriptor
			1,											// Version Number
			PFD_DRAW_TO_WINDOW |						// Format Must Support Window
			PFD_SUPPORT_OPENGL |						// Format Must Support OpenGL
			PFD_DOUBLEBUFFER,							// Format Must Support OpenGL
			PFD_TYPE_RGBA,								// Request An RGBA Format
			16,											// Select Our Color Depth
			0, 0, 0, 0, 0, 0,							// Color Bits Ignored
			0,											// No Alpha Buffer
			0,											// Shift Bit Ignored
			0,											// No Accumulation Buffer
			0, 0, 0, 0,									// Accumulation Bits Ignored
			16,											// 16Bit Z-Buffer (Depth Buffer)  
			0,											// No Stencil Buffer
			0,											// No Auxiliary Buffer
			PFD_MAIN_PLANE,								// Main Drawing Layer
			0,											// Reserved
			0, 0, 0										// Layer Masks Ignored
		};

		int PixelFormat;
		if(!(PixelFormat = ChoosePixelFormat(bdc, &pfd))) {
			ReleaseDC(bwnd, bdc);
			has_failed = true;
			return 0;
		}

		if(!SetPixelFormat(bdc, PixelFormat, &pfd)) {
			ReleaseDC(bwnd, bdc);
			has_failed = true;
			return 0;
		}

		HGLRC hRC;
		if(!(hRC = wglCreateContext(bdc))) {
			ReleaseDC(bwnd, bdc);
			has_failed = true;
			return 0;
		}

		if(!wglMakeCurrent(bdc, hRC)) {
			ReleaseDC(bwnd, bdc);
			wglDeleteContext(hRC);
			has_failed = true;
			return 0;
		}

		ReSizeGLScene(width, height);
		InitGL();
		app_init_gl();

		app_render(width, height, dtime());

		SwapBuffers(bdc);

		if(!wglMakeCurrent(NULL, NULL)) {
			ReleaseDC(bwnd, bdc);
			wglDeleteContext(hRC);
			has_failed = true;
		}

		if(!ReleaseDC(bwnd, bdc)) has_failed = true;
		if(!wglDeleteContext(hRC)) has_failed = true;

		painting = false;
		return 1;
	}

	// Pass All Unhandled Messages To DefWindowProc
	return DefWindowProc(hWnd,uMsg,wParam,lParam);
}

DWORD WINAPI win_graphics_event_loop( LPVOID gi ) {
	MSG					msg;		// Windows Message Structure

	// Register window class and graphics mode message
	reg_win_class();
	BOINC_PAINT = RegisterWindowMessage( "BOINC_PAINT" );

	// Create Our OpenGL Window
	hWnd = CreateWindowEx(WS_EX_APPWINDOW|WS_EX_WINDOWEDGE,							// Extended Style For The Window
								"BOINC_OpenGL",						// Class Name
								"BOINC App",								// Window Title
								WS_OVERLAPPEDWINDOW |							// Defined Window Style
								WS_CLIPSIBLINGS |					// Required Window Style
								WS_CLIPCHILDREN,					// Required Window Style
								0, 0, 100, 100,
								NULL,								// No Parent Window
								NULL,								// No Menu
								hInstance,							// Instance
								NULL);								// Dont Pass Anything To WM_CREATE
	SetWindowPos(hWnd, HWND_TOPMOST, 0, 0, 100, 100, SWP_HIDEWINDOW);

	win_loop_done = false;
	using_opengl = true;
	while(!win_loop_done) {
		if (GetMessage(&msg,NULL,0,0)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		} else {
			win_loop_done = true;
		}
	}

	unreg_win_class();

	SetEvent(hQuitEvent);		// Signal to the worker thread that we're quitting
	return (msg.wParam);		// Exit The thread
}

BOOL VerifyPassword(HWND hwnd)
{ // Under NT, we return TRUE immediately. This lets the saver quit,
  // and the system manages passwords. Under '95, we call VerifyScreenSavePwd.
  // This checks the appropriate registry key and, if necessary,
  // pops up a verify dialog
  OSVERSIONINFO osv; osv.dwOSVersionInfoSize=sizeof(osv); GetVersionEx(&osv);
  if (osv.dwPlatformId==VER_PLATFORM_WIN32_NT) return TRUE;
  HINSTANCE hpwdcpl=::LoadLibrary("PASSWORD.CPL");
  if (hpwdcpl==NULL) {return TRUE;}
  typedef BOOL (WINAPI *VERIFYSCREENSAVEPWD)(HWND hwnd);
  VERIFYSCREENSAVEPWD VerifyScreenSavePwd;
  VerifyScreenSavePwd=
      (VERIFYSCREENSAVEPWD)GetProcAddress(hpwdcpl,"VerifyScreenSavePwd");
  if (VerifyScreenSavePwd==NULL)
  { 
    FreeLibrary(hpwdcpl);return TRUE;
  }
  BOOL bres=VerifyScreenSavePwd(hwnd); FreeLibrary(hpwdcpl);
  return bres;
}

BOOL reg_win_class() {
	WNDCLASS	wc;						// Windows Class Structure

	hInstance			= GetModuleHandle(NULL);				// Grab An Instance For Our Window
	wc.style			= 0;	// Redraw On Size, And Own DC For Window.
	wc.lpfnWndProc		= (WNDPROC) WndProc;					// WndProc Handles Messages
	wc.cbClsExtra		= 0;									// No Extra Window Data
	wc.cbWndExtra		= 0;									// No Extra Window Data
	wc.hInstance		= hInstance;							// Set The Instance
	wc.hIcon			= LoadIcon(NULL, IDI_WINLOGO);			// Load The Default Icon
	wc.hCursor			= LoadCursor(NULL, IDC_ARROW);			// Load The Arrow Pointer
	wc.hbrBackground	= NULL;									// No Background Required For GL
	wc.lpszMenuName		= NULL;									// We Don't Want A Menu
	wc.lpszClassName	= "BOINC_OpenGL";						// Set The Class Name

	if (!RegisterClass(&wc))									// Attempt To Register The Window Class
	{
		MessageBox(NULL,"Failed To Register The Window Class.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;											// Return FALSE
	}

	return TRUE;
}

BOOL unreg_win_class() {
	if (!UnregisterClass("BOINC_OpenGL",hInstance))		// Are We Able To Unregister Class
	{
		MessageBox(NULL,"Could Not Unregister Class.","SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION);
		hInstance=NULL;									// Set hInstance To NULL
	}

	return TRUE;
}

