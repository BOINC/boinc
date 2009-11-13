// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2008 University of California
//
// BOINC is free software; you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation,
// either version 3 of the License, or (at your option) any later version.
//
// BOINC is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with BOINC.  If not, see <http://www.gnu.org/licenses/>.
// 

#include "boinc_win.h"

#include "boinc_tray.h"
#include "tray_win.h"
#include "idlemon.h"


         HMODULE        g_hModule = NULL;
static   CBOINCTray*    gspBOINCTray = NULL;


INT WINAPI WinMain(
    HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow
) {
    CBOINCTray BOINCTray;
    return BOINCTray.Run( hInstance, hPrevInstance, lpCmdLine, nCmdShow );
}


CBOINCTray::CBOINCTray() {
    gspBOINCTray = this;
    m_hDataManagementThread = NULL;
    m_bIdleTrackerInitialized = FALSE;
}


// Starts main execution of BOINC Tray.
//
INT CBOINCTray::Run( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR /* lpCmdLine */ , int /* nCmdShow */ ) {

    if (!hPrevInstance) {
        // Register an appropriate window class for the primary window
        WNDCLASS cls;
        cls.hCursor        = NULL;
        cls.hIcon          = NULL; 
        cls.lpszMenuName   = NULL;
        cls.lpszClassName  = _T("BOINCTrayWndClass");
        cls.hbrBackground  = (HBRUSH)GetStockObject(BLACK_BRUSH);
        cls.hInstance      = hInstance; 
        cls.style          = CS_OWNDC|CS_VREDRAW|CS_HREDRAW;
        cls.lpfnWndProc    = TrayProcStub;
        cls.cbWndExtra     = 0; 
        cls.cbClsExtra     = 0; 
        RegisterClass(&cls);
    }

    // Create the window
    MSG msg;
    HWND hWnd;

    hWnd = CreateWindow(
        _T("BOINCTrayWndClass"),
        _T("BOINC System Tray Applet"),
        WS_OVERLAPPEDWINDOW|WS_HSCROLL|WS_VSCROLL,
        0, 0, CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, hInstance, NULL
    );


    // Process the message pump
    while( GetMessage( &msg, NULL, 0, 0 ) ) {
        TranslateMessage( &msg );
        DispatchMessage( &msg );
    }


    // Cleanup and shutdown the BOINC idle tracking system.
    detach_idle_monitor();

    return msg.wParam;
}




// Create the thread that is used to talk to the daemon.
//
BOOL CBOINCTray::CreateDataManagementThread() {
    DWORD dwThreadID = 0;
    m_hDataManagementThread = CreateThread(
        NULL,                        // default security attributes 
        0,                           // use default stack size  
        DataManagementProcStub,      // thread function 
        NULL,                        // argument to thread function 
        0,                           // use default creation flags 
        &dwThreadID );               // returns the thread identifier 
 
   if (m_hDataManagementThread == NULL) {
        return FALSE;
   }
   return TRUE;
}




// Terminate the thread that is used to talk to the daemon.
//
BOOL CBOINCTray::DestroyDataManagementThread() {
    if (!TerminateThread(m_hDataManagementThread, 0)) {
        return FALSE;
    }
    return TRUE;
}




// Update the time since last input activity every 5 seconds or so.
//
DWORD WINAPI CBOINCTray::DataManagementProc() {
    while (true) {
        // On Vista systems, only elevated processes can create shared memory
        //   area's across various user sessions. In this case we need to wait
        //   for BOINC to create the shared memory area and then boinctray can
        //   successfully attach to it. What a PITA.
        if (!m_bIdleTrackerInitialized) {
            m_bIdleTrackerInitialized = attach_idle_monitor();
        }

        get_idle_tick_count();
        Sleep(5000);
    }
}




// This function forwards to DataManagementProc, which has access to the
//       "this" pointer.
//
DWORD WINAPI CBOINCTray::DataManagementProcStub(LPVOID) {
    return gspBOINCTray->DataManagementProc();
}




// Handle window messages for main screensaver windows.
//
LRESULT CBOINCTray::TrayProc(
    HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam
) {
    switch (uMsg) {
        case WM_CREATE:
            CreateDataManagementThread();
            return 0;
            break;

        case WM_CLOSE:
            DestroyDataManagementThread();
            return 0;
            break;
    }
    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}




// This function forwards all window messages to SaverProc, which has
//       access to the "this" pointer.
//
LRESULT CALLBACK CBOINCTray::TrayProcStub(
    HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam
) {
    return gspBOINCTray->TrayProc(hWnd, uMsg, wParam, lParam);
}
