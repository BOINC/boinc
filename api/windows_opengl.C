// Event loop and support functions for BOINC applications w/ graphics.
// Is any of this related to or dependent on OpenGL??
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
#include <winuser.h>

#include "boinc_api.h"
#include "graphics_api.h"
#include "app_ipc.h"
#include "util.h"
//#include "win_idle_tracker.h"

//remove if there are windows problems
#define WIN32_LEAN_AND_MEAN   // This trims down the windows libraries.
#define WIN32_EXTRA_LEAN      // Trims even farther.

//#define DRAW_WITH_DLL

#ifdef DRAW_WITH_DLL
__declspec(dllimport) void vis_render(int,int,double,float*,int);
__declspec(dllimport) void vis_unload();
__declspec(dllimport) void vis_init();
#pragma comment(lib,"../../vis_dll/debug/vis.lib")
#endif


static HDC			hDC;
static HGLRC		hRC;
static HWND			hWnd=NULL;		// Holds Our Window Handle
static HINSTANCE	hInstance;		// Holds The Instance Of The Application
static RECT			rect = {50, 50, 50+640, 50+480};
static int			current_graphics_mode = MODE_HIDE_GRAPHICS;
static POINT		mousePos;
static UINT			m_uEndSSMsg;

HDC myhDC;

BOOL		win_loop_done;

extern bool using_opengl;
extern bool standalone;
extern HANDLE hQuitEvent;
extern void MyCreateFont(unsigned int &base, char *fontName, int Size, int weight);

void SetupPixelFormat(HDC hDC);

double starttime;
double fps=60.;

LRESULT	CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);	// Declaration For WndProc
BOOL reg_win_class();
BOOL unreg_win_class();

bool KillWindow() {	
#ifdef DRAW_WITH_DLL
	if(hDC) vis_unload();
#else
	//if(hDC) app_unload_gl();
#endif

	if (hRC) {											// Do We Have A Rendering Context?
		if (!wglMakeCurrent(NULL,NULL)) {				// Are We Able To Release The DC And RC Contexts?
			return false;
		}
		if (!wglDeleteContext(hRC)) {					// Are We Able To Delete The RC?
			return false;
		}
		hRC=NULL;										// Set RC To NULL
	}

	if (hDC && !ReleaseDC(hWnd,hDC)) {					// Are We Able To Release The DC
		hDC=NULL;										// Set DC To NULL
		return false;
	}

	KillTimer(hWnd, 1);

	if (hWnd && !DestroyWindow(hWnd)) {					// Are We Able To Destroy The Window?
		return false;
		hWnd=NULL;										// Set hWnd To NULL
	}
	
	return true;
}

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

	if (current_graphics_mode != MODE_FULLSCREEN) GetWindowRect(hWnd, &rect);

	KillWindow();

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

	// Do not do AdjustWindowRectEx here, this will
	// cause the window to creep upwards

    APP_INIT_DATA aid;
    boinc_get_init_data(aid);
    if (!strlen(aid.app_name)) strcpy(aid.app_name, "BOINC Application");
	hWnd = CreateWindowEx(dwExStyle, "BOINC_OpenGL", aid.app_name,
		dwStyle|WS_CLIPSIBLINGS|WS_CLIPCHILDREN, WindowRect.left, WindowRect.top,
		WindowRect.right-WindowRect.left,WindowRect.bottom-WindowRect.top,
		NULL, NULL, hInstance, NULL);



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

	SetTimer(hWnd, 1, 100, NULL);

	starttime=dtime();

	if(current_graphics_mode == MODE_FULLSCREEN || current_graphics_mode == MODE_WINDOW) {
		ShowWindow(hWnd, SW_SHOW);
		SetFocus(hWnd);
	} else {
		ShowWindow(hWnd, SW_HIDE);
	}	

	ReSizeGLScene(width, height);	
	InitGL();

#ifdef DRAW_WITH_DLL
	vis_init();
#else
	app_init_gl();
#endif
	

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

			if(dtime()-starttime>1./fps)
			{
				GetClientRect(hWnd, &rt);
				width = rt.right-rt.left;
				height = rt.bottom-rt.top;

#ifdef DRAW_WITH_DLL
				float* data;
				int data_size=100;
				data=(float*)malloc(sizeof(float)*data_size);
				for(int i=0;i<data_size;i++)
				{
					data[i]=float(rand()%1000/1000.);
				}
				vis_render(width,height,dtime(),data,data_size);
#else
				app_render(width, height, dtime());			
#endif
				starttime=dtime();
				SwapBuffers(hDC);
			}		
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

    if (standalone) {
        SetMode(MODE_WINDOW);
    } else {
        SetMode(MODE_HIDE_GRAPHICS);
    }

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
	wc.style			= CS_HREDRAW | CS_VREDRAW | CS_OWNDC;	// Redraw On Size, And Own DC For Window.
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

void SetupPixelFormat(HDC hDC)
{
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


void MyCreateFont(unsigned int &base, char *fontName, int Size, int weight)
{	
   // windows font
   HFONT hFont;   

   // Create space for 96 characters.
   base = glGenLists(96);

   if(stricmp(fontName, "symbol")==0)
      {
         hFont = CreateFont(Size, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
                            SYMBOL_CHARSET, OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS,
                            ANTIALIASED_QUALITY, FF_DONTCARE | DEFAULT_PITCH, fontName);
      }
   else
      {
         hFont = CreateFont(Size, 0, 0, 0, weight, FALSE, FALSE, FALSE,
                            ANSI_CHARSET, OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS,
                            ANTIALIASED_QUALITY, FF_DONTCARE | DEFAULT_PITCH, fontName);
      }

   if(!hFont)
      return;
   	
   SelectObject(myhDC, hFont);
   wglUseFontBitmaps(myhDC, 32, 96, base);   
}
