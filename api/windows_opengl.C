// Event loop and support functions for Windows versions
// of BOINC applications w/ graphics.
// Platform-independent code should NOT be here.
//
// TODO: Is any of this related to or dependent on OpenGL??
// Why not make it independent of OpenGL?
//

/*		This Code Was Created By Jeff Molofee 2000
 *		A HUGE Thanks To Fredric Echols For Cleaning Up
 *		And Optimizing This Code, Making It More Flexible!
 *		If You've Found This Code Useful, Please Let Me Know.
 *		Visit My Site At nehe.gamedev.net
 *		Adapted to BOINC by Eric Heien
 */
#include "stdafx.h"

#include "boinc_api.h"
#include "graphics_api.h"
#include "app_ipc.h"
#include "util.h"
#include "win_util.h"
//#include "win_idle_tracker.h"

// application needs to define mouse handlers
//
extern void boinc_app_mouse_button(int x, int y, int which, bool is_down);
extern void boinc_app_mouse_move(int x, int y, bool left, bool middle, bool right);


#define BOINC_WINDOW_CLASS_NAME "BOINC_app"

static HDC			hDC=NULL;
static HGLRC		hRC=NULL;
static HWND			hWnd=NULL;		// Holds Our Window Handle
static HINSTANCE	hInstance;		// Holds The Instance Of The Application
static RECT			rect = {50, 50, 50+640, 50+480};
static int			current_graphics_mode = MODE_HIDE_GRAPHICS;
static POINT		mousePos;
static UINT			m_uEndSSMsg;
static HDC myhDC;
BOOL		win_loop_done;

static bool visible = true;

void KillWindow();

void SetupPixelFormat(HDC hDC) {
   int nPixelFormat;

   static PIXELFORMATDESCRIPTOR pfd = {
         sizeof(PIXELFORMATDESCRIPTOR),   // size of structure.
         1,                               // always 1.
         PFD_DRAW_TO_WINDOW |             // support window
         PFD_SUPPORT_OPENGL |             // support OpenGl
         PFD_DOUBLEBUFFER,                // support double buffering
         PFD_TYPE_RGBA,                   // support RGBA
         32,                              // 32 bit color mode
         0, 0, 0, 0, 0, 0,                // ignore color bits
         0,                               // no alpha buffer
         0,                               // ignore shift bit
         0,                               // no accumulation buffer
         0, 0, 0, 0,                      // ignore accumulation bits.
         16,                              // number of depth buffer bits.
         0,                               // number of stencil buffer bits.
         0,                               // 0 means no auxiliary buffer
         PFD_MAIN_PLANE,                  // The main drawing plane
         0,                               // this is reserved
         0, 0, 0 };                       // layer masks ignored.



   // this chooses the best pixel format and returns index.
   nPixelFormat = ChoosePixelFormat(hDC, &pfd);

   // This set pixel format to device context.
   SetPixelFormat(hDC, nPixelFormat, &pfd);

   // Remember that its not important to fully understand the pixel format,
   // just remember to include in all of your applications and you'll be
   // good to go.
}

static void make_new_window(int mode) {
	RECT WindowRect = {0,0,0,0};
	int width, height;
	DWORD dwExStyle;
	DWORD dwStyle;

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

    APP_INIT_DATA aid;
    boinc_get_init_data(aid);
    if (!strlen(aid.app_name)) strcpy(aid.app_name, "BOINC Application");
	hWnd = CreateWindowEx(dwExStyle, BOINC_WINDOW_CLASS_NAME, aid.app_name,
		dwStyle|WS_CLIPSIBLINGS|WS_CLIPCHILDREN, WindowRect.left, WindowRect.top,
		WindowRect.right-WindowRect.left,WindowRect.bottom-WindowRect.top,
		NULL, NULL, hInstance, NULL
    );

	SetForegroundWindow(hWnd);

	GetCursorPos(&mousePos);
	
	hDC = GetDC(hWnd);
	myhDC=hDC;
	SetupPixelFormat(myhDC);
	
	if(!(hRC = wglCreateContext(hDC))) {
		ReleaseDC(hWnd, hDC);
		return;
	}

	if(!wglMakeCurrent(hDC, hRC)) {
		ReleaseDC(hWnd, hDC);
		wglDeleteContext(hRC);
		return;
	}

	width = WindowRect.right-WindowRect.left;
	height = WindowRect.bottom-WindowRect.top;

	if(current_graphics_mode == MODE_FULLSCREEN || current_graphics_mode == MODE_WINDOW) {
		ShowWindow(hWnd, SW_SHOW);
		SetFocus(hWnd);
	} else {
		KillWindow();
	}	
	
    app_graphics_init();
}

void KillWindow() {	
	wglMakeCurrent(NULL,NULL);  // release GL rendering context
	if (hRC) {
		wglDeleteContext(hRC);
		hRC=NULL;
	}

    if (hWnd && hDC) {
        ReleaseDC(hWnd,hDC);
	}
    hDC = NULL;

    if (hWnd) {
        DestroyWindow(hWnd);
	}
    hWnd = NULL;
}

// switch to the given graphics mode.  This is called:
// - on initialization
// - when get mode change msg (via shared mem)
// - when in SS mode and get user input
//
void SetMode(int mode) {

	if (current_graphics_mode != MODE_FULLSCREEN) GetWindowRect(hWnd, &rect);

	KillWindow();

	current_graphics_mode = mode;

    if (mode != MODE_HIDE_GRAPHICS) {
        make_new_window(mode);
    }

    // tell the core client that we're entering new mode
    //
    if (app_client_shm) {
        app_client_shm->send_graphics_msg(
            APP_CORE_GFX_SEG, GRAPHICS_MSG_SET_MODE, current_graphics_mode
        );
    }
}

void parse_mouse_event(UINT uMsg, int& which, bool& down) {
	switch(uMsg) {
	case WM_LBUTTONDOWN: which = 0; down = true; break;
	case WM_MBUTTONDOWN: which = 1; down = true; break;
	case WM_RBUTTONDOWN: which = 2; down = true; break;
	case WM_LBUTTONUP: which = 0; down = false; break;
	case WM_MBUTTONUP: which = 1; down = false; break;
	case WM_RBUTTONUP: which = 2; down = false; break;
    }
}
    
// message handler (includes timer, Windows msgs)
//
LRESULT CALLBACK WndProc(
    HWND	hWnd,			// Handle For This Window
    UINT	uMsg,			// Message For This Window
    WPARAM	wParam,			// Additional Message Information
    LPARAM	lParam			// Additional Message Information
) {
	switch(uMsg) {
	case WM_ERASEBKGND:		// Check To See If Windows Is Trying To Erase The Background
			return 0;	
	case WM_KEYDOWN:
	case WM_KEYUP:
		if(current_graphics_mode == MODE_FULLSCREEN) {
			SetMode(MODE_HIDE_GRAPHICS);
			PostMessage(HWND_BROADCAST, m_uEndSSMsg, 0, 0);
        }
        return 0;
	case WM_LBUTTONDOWN:
	case WM_MBUTTONDOWN:
	case WM_RBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_MBUTTONUP:
	case WM_RBUTTONUP:
		if(current_graphics_mode == MODE_FULLSCREEN) {
			SetMode(MODE_HIDE_GRAPHICS);
			PostMessage(HWND_BROADCAST, m_uEndSSMsg, 0, 0);
		} else  {
            int which;
            bool down;
		    POINT cPos;
		    GetCursorPos(&cPos);
            parse_mouse_event(uMsg, which, down);
			boinc_app_mouse_button(cPos.x, cPos.y, which, down);
		}
		return 0;
	case WM_MOUSEMOVE:
		POINT cPos;
		GetCursorPos(&cPos);
		if(current_graphics_mode == MODE_FULLSCREEN) {
			if(cPos.x != mousePos.x || cPos.y != mousePos.y) {
				SetMode(MODE_HIDE_GRAPHICS);
				PostMessage(HWND_BROADCAST, m_uEndSSMsg, 0, 0);
			}
		} else {
			boinc_app_mouse_move(
                cPos.x, cPos.y,
                (wParam&MK_LBUTTON)!=0,
                (wParam&MK_MBUTTON)!=0,
                (wParam&MK_RBUTTON)!=0
            );
		}
		return 0;
	case WM_CLOSE:
        if (boinc_is_standalone()) {
            exit(0);
        } else {
		    KillWindow();
		    return 0;
        }
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
        if ( SIZE_MINIMIZED == wParam ) {
            visible = FALSE;
        } else {
            visible = TRUE;
        }
		ReSizeGLScene(LOWORD(lParam),HIWORD(lParam));
		return 0;
	}

	// Pass All Unhandled Messages To DefWindowProc
	return DefWindowProc(hWnd,uMsg,wParam,lParam);
}

BOOL reg_win_class() {
	WNDCLASS	wc;						// Windows Class Structure

	hInstance			= GetModuleHandle(NULL);				// Grab An Instance For Our Window
	wc.style			= CS_HREDRAW | CS_VREDRAW | CS_OWNDC;	// Redraw On Size, And Own DC For Window.
	wc.lpfnWndProc		= (WNDPROC) WndProc;					// WndProc Handles Messages
	wc.cbClsExtra		= 0;									// No Extra Window Data
	wc.cbWndExtra		= 0;									// No Extra Window Data
	wc.hInstance		= hInstance;							// Set The Instance
	wc.hIcon			= LoadIcon(NULL, IDI_WINLOGO);			// Load The Default Icon
	wc.hCursor			= LoadCursor(NULL, IDC_ARROW);			// Load The Arrow Pointer
	wc.hbrBackground	= NULL;									// No Background Required For GL
	wc.lpszMenuName		= NULL;									// We Don't Want A Menu
	wc.lpszClassName	= BOINC_WINDOW_CLASS_NAME;				// Set The Class Name

	if (!RegisterClass(&wc))									// Attempt To Register The Window Class
	{
		MessageBox(NULL,"Failed To Register The Window Class.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;											// Return FALSE
	}

	return TRUE;
}

BOOL unreg_win_class() {
	if (!UnregisterClass(BOINC_WINDOW_CLASS_NAME,hInstance)) {
		MessageBox(NULL,"Could Not Unregister Class.","SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION);
		hInstance=NULL;									// Set hInstance To NULL
	}

	return TRUE;
}

static VOID CALLBACK timer_handler(HWND, UINT, UINT, DWORD) {
	RECT rt;
	int width, height, new_mode, msg;
    if (app_client_shm) {
        if (app_client_shm->get_graphics_msg(CORE_APP_GFX_SEG, msg, new_mode)) {
            switch (msg) {
            case GRAPHICS_MSG_SET_MODE:
                SetMode(new_mode);
                break;
            case GRAPHICS_MSG_REREAD_PREFS:
				// only reread graphics prefs if we have a window open
				//
				switch(current_graphics_mode) {
				case MODE_WINDOW:
				case MODE_FULLSCREEN:
					app_graphics_reread_prefs();
					break;
				}
                break;
            }
        }
	}
    if (!visible) return;
	if (current_graphics_mode == MODE_HIDE_GRAPHICS) return;
    if (!hWnd) return;

    // TODO: remove width, height from API
    //
	GetClientRect(hWnd, &rt);
	width = rt.right-rt.left;
	height = rt.bottom-rt.top;

    if (throttled_app_render(width, height, dtime())) {
		SwapBuffers(hDC);
    }
}

DWORD WINAPI win_graphics_event_loop( LPVOID gi ) {
	MSG					msg;		// Windows Message Structure
	m_uEndSSMsg = RegisterWindowMessage(STOP_SS_MSG);

	// Register window class and graphics mode message
	reg_win_class();

	SetTimer(NULL, 1, 100, &timer_handler);

    if (boinc_is_standalone()) {
        SetMode(MODE_WINDOW);
    } else {
        SetMode(MODE_HIDE_GRAPHICS);
    }
	win_loop_done = false;
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
	return (DWORD)msg.wParam;		// Exit The thread
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


float txt_widths[256];

unsigned int MyCreateFont(char *fontName, int Size, int weight) {	
    // windows font
    HFONT hFont;   
    unsigned int mylistbase =0;

    // Create space for 96 characters.
    mylistbase= glGenLists(256);

    if(stricmp(fontName, "symbol")==0) {
        hFont = CreateFont(
            Size, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
            SYMBOL_CHARSET, OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS,
            ANTIALIASED_QUALITY, FF_DONTCARE | DEFAULT_PITCH, fontName
        );
    } else {
        hFont = CreateFont(
            Size, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
            ANSI_CHARSET, OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS,
            ANTIALIASED_QUALITY, FF_DONTCARE | DEFAULT_PITCH, fontName
        );
    }

    if(!hFont) return -1;
    SelectObject(myhDC, hFont);
#if 1 //no idea why this has to be twice
    wglUseFontBitmaps(myhDC, 0, 256, mylistbase);   
    wglUseFontBitmaps(myhDC, 0, 256, mylistbase);   
#endif 
#if 0
    wglUseFontOutlines(hDC,0,255,mylistbase,0.0f,0.2f,WGL_FONT_POLYGONS,gmf);   
#endif
 
     TEXTMETRIC met;
     GetTextMetrics(myhDC,&met);   
     GetCharWidthFloat(myhDC,met.tmFirstChar,met.tmLastChar,txt_widths);

    return mylistbase;
}

float get_char_width(unsigned char c) {
	return txt_widths[c];	
}
