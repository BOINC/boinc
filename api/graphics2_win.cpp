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
#include "filesys.h"
#include "graphics2.h"

#define BOINC_WINDOW_CLASS_NAME     "BOINC_app"
#define WM_SHUTDOWNGFX              WM_USER+1

static HDC hDC = NULL;
static HGLRC hRC = NULL;
static HWND hWnd = NULL;        // Holds Our Window Handle
static HINSTANCE hInstance;     // Holds The Instance Of The Application
static RECT rect = {50, 50, 50+640, 50+480};
static int current_graphics_mode = MODE_HIDE_GRAPHICS;
static POINT mousePos;
static bool visible = true;
static bool window_ready=false;
static UINT_PTR gfx_timer_id = 0;
static bool fullscreen;

void boinc_close_window_and_quit(const char* p) {
    fprintf(stderr, "%s Close event (%s) detected, shutting down.\n",
        boinc_msg_prefix(), p
    );

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
    SendMessage(hWnd, WM_QUIT, 0, 0);
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
   if (!SetPixelFormat(hDC, nPixelFormat, &pfd)) {
       fprintf(stderr,
            "%s ERROR: Couldn't set pixel format for device context (0x%x).\n",
            boinc_msg_prefix(), GetLastError()
        );
   }
}

static void make_window(const char* title) {
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
        // Version 5 screensaver logic kills all MODE_WINDOW graphics before starting one
        // in fullscreen mode, then restarts the ones it killed when screensaver stops.
        // To be compatible with V5, we remember and restore the MODE_WINDOW dimensions.
        FILE *f = boinc_fopen("gfx_info", "r");
        if (f) {
            // ToDo: change this to XML parsing
            fscanf(f, "%d %d %d %d\n", &rect.left, &rect.top, &rect.right, &rect.bottom);
            fclose(f);
        }
        WindowRect = rect;
        dwExStyle=WS_EX_APPWINDOW|WS_EX_WINDOWEDGE;
        dwStyle=WS_OVERLAPPEDWINDOW;
        while(ShowCursor(true) < 0);
    }

    char window_title[256];
    if (title) {
        strcpy(window_title, title);
    } else {
        APP_INIT_DATA aid;
        boinc_get_init_data(aid);
        if (!strlen(aid.app_name)) strcpy(aid.app_name, "BOINC Application");
        get_window_title(window_title, 256);
    }

    //fprintf(stderr, "Setting window title to '%s'.\n", window_title);

    hWnd = CreateWindowEx(dwExStyle, BOINC_WINDOW_CLASS_NAME, window_title,
        dwStyle|WS_CLIPSIBLINGS|WS_CLIPCHILDREN, WindowRect.left, WindowRect.top,
        WindowRect.right-WindowRect.left,WindowRect.bottom-WindowRect.top,
        NULL, NULL, hInstance, NULL
    );

    if (!SetForegroundWindow(hWnd)) {
        fprintf(stderr,
            "%s ERROR: Unable to set foreground window (0x%x).\n",
            boinc_msg_prefix(), GetLastError()
        );
    }

    if (!GetCursorPos(&mousePos)) {
        fprintf(stderr,
            "%s ERROR: Unable to get mouse cursor position (0x%x).\n",
            boinc_msg_prefix(), GetLastError()
        );
    }

    hDC = GetDC(hWnd);
    if (!hDC) {
        fprintf(stderr,
            "%s ERROR: Couldn't get a device context for the window (0x%x).\n",
            boinc_msg_prefix(), GetLastError()
        );
    }
    SetupPixelFormat(hDC);

    hRC = wglCreateContext(hDC);
    if (!hRC) {
        fprintf(stderr,
            "%s ERROR: Unable to create OpenGL context (0x%x).\n",
            boinc_msg_prefix(), GetLastError()
        );
        ReleaseDC(hWnd, hDC);
        return;
    }

    if(!wglMakeCurrent(hDC, hRC)) {
        fprintf(stderr,
            "%s ERROR: Unable to make OpenGL context current (0x%x).\n",
            boinc_msg_prefix(), GetLastError()
        );
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
            boinc_close_window_and_quit("key down");
        } else {           
            boinc_app_key_press((int)wParam, (int)lParam);
        }
        return 0;
    case WM_KEYUP:
        if(!window_ready) return 0;    
        if (fullscreen) {
            boinc_close_window_and_quit("key up");
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
            boinc_close_window_and_quit("button up");
        } else  {
            int which;
            bool down;
            parse_mouse_event(uMsg, which, down);
            boinc_app_mouse_button(
                (int)(short)LOWORD(lParam), (int)(short)HIWORD(lParam),
                which, down
            );
        }
        return 0;
    case WM_MOUSEMOVE:
        if(!window_ready) return 0;    
        if (fullscreen) { 
            if((int)(short)LOWORD(lParam) != mousePos.x || (int)(short)HIWORD(lParam) != mousePos.y) {
                boinc_close_window_and_quit("mouse move");
            }
        } else {
            boinc_app_mouse_move(
                (int)(short)LOWORD(lParam), (int)(short)HIWORD(lParam),
                (wParam&MK_LBUTTON)!=0,
                (wParam&MK_MBUTTON)!=0,
                (wParam&MK_RBUTTON)!=0
            );
        }
        return 0;
    case WM_CLOSE:
        boinc_close_window_and_quit("WM_CLOSE");
        return 0;
    case WM_SHUTDOWNGFX:
        boinc_close_window_and_quit("WM_SHUTDOWNGFX");
        return 0;
    case WM_DESTROY:
        PostQuitMessage(0);
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
    }

    // Pass All Unhandled Messages To DefWindowProc
    return DefWindowProc(hWnd,uMsg,wParam,lParam);
}

BOOL reg_win_class() {
    WNDCLASS    wc;                                 // Windows Class Structure

    wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;  // Redraw On Size, And Own DC For Window.
    wc.lpfnWndProc = (WNDPROC) WndProc;             // WndProc Handles Messages
    wc.cbClsExtra = 0;                              // No Extra Window Data
    wc.cbWndExtra = 0;                              // No Extra Window Data
    wc.hInstance = hInstance;                       // Set The Instance
    wc.hIcon = LoadIcon(NULL, IDI_WINLOGO);         // Load The Default Icon
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);       // Load The Arrow Pointer
    wc.hbrBackground = NULL;                        // No Background Required For GL
    wc.lpszMenuName = NULL;                         // We Don't Want A Menu
    wc.lpszClassName = BOINC_WINDOW_CLASS_NAME;     // Set The Class Name

    // Attempt To Register The Window Class
    if (!RegisterClass(&wc)) {
        MessageBox(
            NULL,
            "Failed To Register The Window Class.",
            "ERROR",
            MB_OK|MB_ICONEXCLAMATION
        );
        return FALSE;                                            // Return FALSE
    }

    return TRUE;
}

BOOL unreg_win_class() {

    if (!UnregisterClass(BOINC_WINDOW_CLASS_NAME, hInstance)) {
        MessageBox(
            NULL,
            "Could Not Unregister Class.",
            "ERROR",
            MB_OK|MB_ICONINFORMATION
        );
        hInstance=NULL;
    }

    return TRUE;
}

static VOID CALLBACK timer_handler(HWND, UINT, UINT, DWORD) {
    RECT rt;
    int width, height;
    static int size_changed = 0;

    GetWindowRect(hWnd, &rt);
    width = rt.right-rt.left;
    height = rt.bottom-rt.top;

    if (throttled_app_render(width, height, dtime())) {
        SwapBuffers(hDC);
        if (! fullscreen) {
            // If user has changed window size, wait until it stops 
            // changing and then write the new dimensions to file
            if ((rt.left != rect.left) || (rt.top != rect.top) || 
                (rt.right != rect.right) || (rt.bottom != rect.bottom)
            ) {
				if (IsZoomed(hWnd)) return;
				if ((rt.left < 0) && (rt.right < 0)) return;
				if ((rt.top < 0) && (rt.bottom < 0)) return;
                size_changed = 1;
                rect.left = rt.left;
                rect.top = rt.top;
                rect.right = rt.right;
                rect.bottom = rt.bottom;
            } else {
                if (size_changed && (++size_changed > 10)) {
                    size_changed = 0;
                    FILE *f = boinc_fopen("gfx_info", "w");
                    if (f) {
                        // ToDo: change this to XML
                        fprintf(f, "%d %d %d %d\n", rect.left, rect.top, rect.right, rect.bottom);
                        fclose(f);
                    }
                }
            }               // End if (new size != previous size) else 
        }
    }
}

void boinc_graphics_loop(int argc, char** argv, const char* title) {
    if (!diagnostics_is_initialized()) {
        boinc_init_graphics_diagnostics(BOINC_DIAG_DEFAULTS);
    }

    fprintf(stderr, "%s Starting graphics application.\n", boinc_msg_prefix());

    for (int i=1; i<argc; i++) {
        if (!strcmp(argv[i], "--fullscreen")) {
            fullscreen = true;
            fprintf(stderr, "%s fullscreen mode requested.\n",
                boinc_msg_prefix()
            );
        }
    }

    // Register the BOINC App window class
    //
    reg_win_class();

    wglMakeCurrent(NULL,NULL); 
    make_window(title);

    // Create a timer thread to do rendering
    //
    gfx_timer_id = SetTimer(NULL, 1, 30, (TIMERPROC)&timer_handler);

    // Process the window message pump
    //
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    // Unregister the BOINC App window class
    unreg_win_class();

    fprintf(stderr, "%s Shutting down graphics application.\n",
        boinc_msg_prefix()
    );
}

extern int main(int, char**);

// call this with the resource names you compiled the icons with
// (16x16 and 48x48 pixel)
//
void boinc_set_windows_icon(const char* icon16, const char* icon48) {
    LONGLONG ic;
    HWND hWnd = FindWindow("BOINC_app",NULL);

    if (ic = (LONGLONG)LoadIcon(hInstance, icon48)) {
#ifdef _WIN64
        SetClassLongPtr(hWnd, GCLP_HICON, (LONG_PTR)ic);
#else
        SetClassLongPtr(hWnd, GCLP_HICON, (LONG)ic);
#endif
    }
    if (ic = (LONGLONG)LoadImage(hInstance, icon16, IMAGE_ICON, 16, 16, 0)) {
#ifdef _WIN64
        SetClassLongPtr(hWnd, GCLP_HICONSM, (LONG_PTR)ic);
#else
        SetClassLongPtr(hWnd, GCLP_HICONSM, (LONG)ic);
#endif
    }
}

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR Args, int WinMode) {
    LPSTR command_line;
    char* argv[100];
    int argc;

    hInstance = hInst;
    command_line = GetCommandLine();
    argc = parse_command_line(command_line, argv);
    main(argc, argv);
}
