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
#include "app_ipc.h"
#include "util.h"
#include "win_idle_tracker.h"

static HDC			hdc;
static HGLRC		hRC;
static HWND			hWnd=NULL;		// Holds Our Window Handle
static HINSTANCE	hInstance;		// Holds The Instance Of The Application
static RECT			rect = {50, 50, 50+640, 50+480};
static int			current_graphics_mode = MODE_HIDE_GRAPHICS;
static POINT		mousePos;
static UINT			m_uEndSSMsg;

BOOL		win_loop_done;

extern bool using_opengl;
extern HANDLE hQuitEvent;

LRESULT	CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);	// Declaration For WndProc
DWORD WINAPI win_graphics_event_loop( LPVOID duff );
BOOL reg_win_class();
BOOL unreg_win_class();

// switch to the given graphics mode.  This is called:
// - on initialization
// - when get mode change msg (via shared mem)
// - when in SS mode and get user input
//
void SetMode(int mode) {
	RECT WindowRect = {0,0,0,0};
	int width, height;
	DWORD dwExStyle;
	DWORD dwStyle;

	if(hWnd) {
		if (hRC) wglDeleteContext(hRC);
		if (hdc) ReleaseDC(hWnd, hdc);

		if (current_graphics_mode != MODE_FULLSCREEN) GetWindowRect(hWnd, &rect);
		KillTimer(hWnd, 1);
		DestroyWindow(hWnd);
	}

	current_graphics_mode = mode;

	if (current_graphics_mode == MODE_FULLSCREEN) {
		HDC screenDC=GetDC(NULL);
		WindowRect.left = WindowRect.top = 0;
		WindowRect.right=GetDeviceCaps(screenDC, HORZRES);
		WindowRect.bottom=GetDeviceCaps(screenDC, VERTRES);
		ReleaseDC(NULL, screenDC);
		dwExStyle=WS_EX_TOPMOST;
		dwStyle=WS_POPUP;
		while(ShowCursor(false) >= 0);
	} else {
		WindowRect = rect;
		dwExStyle=WS_EX_APPWINDOW|WS_EX_WINDOWEDGE;
		dwStyle=WS_OVERLAPPEDWINDOW;
		while(ShowCursor(true) < 0);
	}

	hWnd = CreateWindowEx(dwExStyle, "BOINC_OpenGL", "BOINC Graphics",
		dwStyle|WS_CLIPSIBLINGS|WS_CLIPCHILDREN, WindowRect.left, WindowRect.top,
		WindowRect.right-WindowRect.left,WindowRect.bottom-WindowRect.top,
		NULL, NULL, hInstance, NULL);

	SetTimer(hWnd, 1, 100, NULL);

	GetCursorPos(&mousePos);

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

	hdc = GetDC(hWnd);
	int PixelFormat;
	PixelFormat = ChoosePixelFormat(hdc, &pfd);
	SetPixelFormat(hdc, PixelFormat, &pfd);

	if(!(hRC = wglCreateContext(hdc))) {
		ReleaseDC(hWnd, hdc);
		return;
	}

	if(!wglMakeCurrent(hdc, hRC)) {
		ReleaseDC(hWnd, hdc);
		wglDeleteContext(hRC);
		return;
	}

	width = WindowRect.right-WindowRect.left;
	height = WindowRect.bottom-WindowRect.top;

	ReSizeGLScene(width, height);
	InitGL();
	app_init_gl();

	if(current_graphics_mode == MODE_FULLSCREEN || current_graphics_mode == MODE_WINDOW) {
		ShowWindow(hWnd, SW_SHOW);
		SetFocus(hWnd);
	} else {
		ShowWindow(hWnd, SW_HIDE);
	}

	app_client_shm->send_graphics_mode_msg(APP_CORE_GFX_SEG, current_graphics_mode);
}

// message handler (includes timer, Windows msgs)
//
LRESULT CALLBACK WndProc(	HWND	hWnd,			// Handle For This Window
							UINT	uMsg,			// Message For This Window
							WPARAM	wParam,			// Additional Message Information
							LPARAM	lParam)			// Additional Message Information
{
	RECT rt;
	int width, height, new_mode;

	switch(uMsg) {
		case WM_KEYDOWN:
		case WM_KEYUP:
		case WM_LBUTTONDOWN:
		case WM_MBUTTONDOWN:
		case WM_RBUTTONDOWN:
			if(current_graphics_mode == MODE_FULLSCREEN) {
				SetMode(MODE_HIDE_GRAPHICS);
				PostMessage(HWND_BROADCAST, m_uEndSSMsg, 0, 0);
			}
			return 0;
		case WM_MOUSEMOVE:
			if(current_graphics_mode == MODE_FULLSCREEN) {
				POINT cPos;
				GetCursorPos(&cPos);
				if(cPos.x != mousePos.x || cPos.y != mousePos.y) {
					SetMode(MODE_HIDE_GRAPHICS);
					PostMessage(HWND_BROADCAST, m_uEndSSMsg, 0, 0);
				}
			}
			return 0;
		case WM_CLOSE:
			SetMode(MODE_HIDE_GRAPHICS);
			return 0;
		case WM_PAINT:
			PAINTSTRUCT ps;
			RECT winRect;
			HDC pdc;
			pdc = BeginPaint(hWnd, &ps);
			GetClientRect(hWnd, &winRect);
			FillRect(pdc, &winRect, (HBRUSH)GetStockObject(BLACK_BRUSH));
			EndPaint(hWnd, &ps);
			return 0;
		case WM_SIZE:
			ReSizeGLScene(LOWORD(lParam),HIWORD(lParam));
			return 0;
		case WM_TIMER:
			if (app_client_shm->get_graphics_mode_msg(CORE_APP_GFX_SEG, new_mode)) {
				SetMode(new_mode);
			}
			if (current_graphics_mode == MODE_HIDE_GRAPHICS) return 0;

			GetClientRect(hWnd, &rt);
			width = rt.right-rt.left;
			height = rt.bottom-rt.top;

			app_render(width, height, dtime());

			SwapBuffers(hdc);
			return 0;
	}

	// Pass All Unhandled Messages To DefWindowProc
	return DefWindowProc(hWnd,uMsg,wParam,lParam);
}

DWORD WINAPI win_graphics_event_loop( LPVOID gi ) {
	MSG					msg;		// Windows Message Structure

	double start = (double)time(0);
	m_uEndSSMsg = RegisterWindowMessage(END_SS_MSG);

	// Register window class and graphics mode message
	reg_win_class();

	SetMode(MODE_HIDE_GRAPHICS);

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

