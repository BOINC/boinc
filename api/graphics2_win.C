// Event loop and support functions for Windows versions
// of BOINC applications w/ graphics.
// Platform-independent code should NOT be here.
//

/* This Code Was Created By Jeff Molofee 2000.
 * A HUGE thanks to fredric echols for cleaning up
 * and optimizing this code, making it more flexible!
 * If you've found this code useful, please let me know.
 * Visit My Site At nehe.gamedev.net
 * Adapted to BOINC by Eric Heien
 */
#if defined(_WIN32) && !defined(__STDWX_H__) && !defined(_BOINC_WIN_) && !defined(_AFX_STDAFX_H_)
#include "boinc_win.h"
#else
#include "config.h"
#endif

#include "diagnostics.h"
#include "boinc_api.h"
#include "app_ipc.h"
#include "util.h"
#include "str_util.h"
#include "graphics2.h"

#define BOINC_WINDOW_CLASS_NAME     "BOINC_app"
#define WM_SHUTDOWNGFX              WM_USER+1

const UINT WM_BOINCSFW = RegisterWindowMessage(TEXT("BOINCSetForegroundWindow"));

static HDC hDC = NULL;
static HGLRC hRC = NULL;
static HWND hWnd = NULL;        // Holds Our Window Handle
static HINSTANCE hInstance;     // Holds The Instance Of The Application
static RECT rect = {50, 50, 50+640, 50+480};
static int current_graphics_mode = MODE_HIDE_GRAPHICS;
static int acked_graphics_mode = -1;
static POINT mousePos;
static HDC myhDC;
static bool visible = true;
static bool window_ready=false;
static UINT_PTR gfx_timer_id = 0;
static bool fullscreen;

void close_window() {
    window_ready=false;
    wglMakeCurrent(NULL,NULL);  // release GL rendering context
    if (hRC) {
        wglDeleteContext(hRC);
    }

    if (hWnd && hDC) {
        ReleaseDC(hWnd,hDC);
    }

    if (hWnd) {
        DestroyWindow(hWnd);
    }
    exit(0);
}

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
   BOOL flag = SetPixelFormat(hDC, nPixelFormat, &pfd);
   if (!flag) {
       DWORD x = GetLastError();
   }
}

static void make_window() {
    RECT WindowRect = {0,0,0,0};
    int width, height;
    DWORD dwExStyle;
    DWORD dwStyle;

    if (fullscreen) {
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
    char window_title[256];
    get_window_title(window_title, 256);
    hWnd = CreateWindowEx(dwExStyle, BOINC_WINDOW_CLASS_NAME, window_title,
        dwStyle|WS_CLIPSIBLINGS|WS_CLIPCHILDREN, WindowRect.left, WindowRect.top,
        WindowRect.right-WindowRect.left,WindowRect.bottom-WindowRect.top,
        NULL, NULL, hInstance, NULL
    );

    SetForegroundWindow(hWnd);

    GetCursorPos(&mousePos);

    hDC = GetDC(hWnd);
    myhDC=hDC;
    SetupPixelFormat(myhDC);

    hRC = wglCreateContext(hDC);
    if (hRC == 0) {
        DWORD x = GetLastError();
        ReleaseDC(hWnd, hDC);
        return;
    }

    if(!wglMakeCurrent(hDC, hRC)) {
        ReleaseDC(hWnd, hDC);
        wglDeleteContext(hRC);
        return;
    }

    // use client area for resize when not fullscreen
    if (current_graphics_mode != MODE_FULLSCREEN) {
        GetClientRect(hWnd, &WindowRect);
	}

    width = WindowRect.right-WindowRect.left;
    height = WindowRect.bottom-WindowRect.top;

    ShowWindow(hWnd, SW_SHOW);
    SetFocus(hWnd);

    app_graphics_init();
    app_graphics_resize(width, height);     
    window_ready=true;
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
    HWND    hWnd,            // Handle For This Window
    UINT    uMsg,            // Message For This Window
    WPARAM    wParam,            // Additional Message Information
    LPARAM    lParam            // Additional Message Information
) {
    switch(uMsg) {
    case WM_ERASEBKGND:        // Check To See If Windows Is Trying To Erase The Background
            return 0;
    case WM_KEYDOWN:
        if(!window_ready) return 0;    
        if (fullscreen) {
            close_window();
        } else {           
            boinc_app_key_press((int)wParam, (int)lParam);
        }
        return 0;
    case WM_KEYUP:
        if(!window_ready) return 0;    
        if (fullscreen) {
            close_window();
        } else {
            boinc_app_key_release((int)wParam, (int)lParam);           
        }
        return 0;
    case WM_LBUTTONDOWN:
    case WM_MBUTTONDOWN:
    case WM_RBUTTONDOWN:
    case WM_LBUTTONUP:
    case WM_MBUTTONUP:
    case WM_RBUTTONUP:
        if(!window_ready) return 0;    

        if (fullscreen) {
            close_window();
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
        if(!window_ready) return 0;    
        POINT cPos;
        GetCursorPos(&cPos);
        if (fullscreen) { 
            if(cPos.x != mousePos.x || cPos.y != mousePos.y) {
                close_window();
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
        boinc_exit(0);
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
        if ( SIZE_MINIMIZED == wParam ) {
            visible = FALSE;
        } else {
            visible = TRUE;
        }          
        if(!window_ready) return 0;    
        app_graphics_resize(LOWORD(lParam), HIWORD(lParam));
        return 0;
    case WM_SHUTDOWNGFX:
        close_window();
        return 0;
    default:
        if ( WM_BOINCSFW == uMsg ) {
            SetForegroundWindow(hWnd);
        }
    }

    // Pass All Unhandled Messages To DefWindowProc
    return DefWindowProc(hWnd,uMsg,wParam,lParam);
}

BOOL reg_win_class() {
    WNDCLASS    wc;                        // Windows Class Structure

    hInstance = GetModuleHandle(NULL);       // Grab An Instance For Our Window
    wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
        // Redraw On Size, And Own DC For Window.
    wc.lpfnWndProc = (WNDPROC) WndProc;             // WndProc Handles Messages
    wc.cbClsExtra = 0;                                // No Extra Window Data
    wc.cbWndExtra = 0;                                  // No Extra Window Data
    wc.hInstance = hInstance;                            // Set The Instance
    wc.hIcon = LoadIcon(NULL, IDI_WINLOGO);            // Load The Default Icon
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);            // Load The Arrow Pointer
    wc.hbrBackground = NULL;                 // No Background Required For GL
    wc.lpszMenuName = NULL;                  // We Don't Want A Menu
    wc.lpszClassName = BOINC_WINDOW_CLASS_NAME;       // Set The Class Name

    // Attempt To Register The Window Class
    if (!RegisterClass(&wc)) {
        MessageBox(NULL,"Failed To Register The Window Class.","ERROR",MB_OK|MB_ICONEXCLAMATION);
        return FALSE;                                            // Return FALSE
    }

    return TRUE;
}

BOOL unreg_win_class() {
    if (!UnregisterClass(BOINC_WINDOW_CLASS_NAME,hInstance)) {
        MessageBox(NULL,"Could Not Unregister Class.","SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION);
        hInstance=NULL;
    }

    return TRUE;
}

static VOID CALLBACK timer_handler(HWND, UINT, UINT, DWORD) {
    RECT rt;
    int width, height;

    GetClientRect(hWnd, &rt);
    width = rt.right-rt.left;
    height = rt.bottom-rt.top;

    if (throttled_app_render(width, height, dtime())) {
        SwapBuffers(hDC);
    }
}

void boinc_graphics_loop(int argc, char** argv) {
    MSG msg;
    for (int i=1; i<argc; i++) {
        if (!strcmp(argv[i], "--fullscreen")) {
            fullscreen = true;
        }
    }

    reg_win_class();
    gfx_timer_id = SetTimer(NULL, 1, 30, (TIMERPROC)&timer_handler);
    wglMakeCurrent(NULL,NULL); 
    make_window();
    while (1) {
        if (GetMessage(&msg,NULL,0,0)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        } else {
            break;
        }
    }
    unreg_win_class();
}

extern int main(int, char**);

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR Args, int WinMode) {
    LPSTR command_line;
    char* argv[100];
    int argc;

    command_line = GetCommandLine();
    argc = parse_command_line(command_line, argv);
    main(argc, argv);
}

