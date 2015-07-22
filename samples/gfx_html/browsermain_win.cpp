// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2014-2015 University of California
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

#define _ATL_FREE_THREADED
#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS
#include <AtlBase.h>
#include <AtlCom.h>
#include <AtlCtl.h>
#include <AtlWin.h>
#include <AtlStr.h>
#include <AtlFile.h>
#include <AtlTypes.h>
#include <exdisp.h>
#include <exdispid.h>
#include <urlmon.h>
#include <string>
#include "win_util.h"
#include "version.h"
#include "boinc_api.h"
#include "diagnostics.h"
#include "filesys.h"
#include "webserver.h"
#include "browser.h"
#include "browser_i.h"
#include "browser_i.c"
#include "browser_win.h"
#include "browserlog.h"
#include "browserctrl_win.h"
#include "browserwnd_win.h"
#include "browsermain_win.h"


#ifdef _MSC_VER
#define snprintf _snprintf
#endif


CBrowserModule _AtlModule;


CBrowserModule::CBrowserModule()
{
    m_pWnd = NULL;
}

int CBrowserModule::RegisterWebControlCompatiblity()
{
    LONG    lReturnValue;
    HKEY    hKey;
    TCHAR   szPath[MAX_PATH-1];
    LPTSTR  lpszFileName = NULL;
    DWORD   dwSize = 0;


    // Isolate the executable name
    GetModuleFileName(NULL, szPath, (sizeof(szPath)/sizeof(TCHAR)));
    lpszFileName = _tcsrchr(szPath, '\\');
    if (lpszFileName)
        lpszFileName++;
    else
        return ERROR_FILE_NOT_FOUND;

    lReturnValue = RegOpenKeyEx(
        HKEY_CURRENT_USER,
        _T("SOFTWARE\\Microsoft\\Internet Explorer\\Main\\FeatureControl\\FEATURE_BROWSER_EMULATION"),
        0,
        KEY_WRITE,
        &hKey
    );
    if (lReturnValue == ERROR_SUCCESS) {
        DWORD dwIE11 = 11001;
        RegSetValueEx(hKey, lpszFileName, 0, REG_DWORD, (const BYTE *)&dwIE11, sizeof(DWORD));
        RegCloseKey(hKey);
    }


    return 0;
}

HRESULT CBrowserModule::InitializeCom() throw()
{
    return OleInitialize(NULL);
}

void CBrowserModule::UninitializeCom() throw()
{
    OleUninitialize();
}

HRESULT CBrowserModule::PreMessageLoop(int nShowCmd) throw()
{
    HRESULT hr = 0;
    RECT rc = {0, 0, 0, 0};
    DWORD dwExStyle = 0;
    DWORD dwStyle = 0;
    APP_INIT_DATA aid;
    char szWindowTitle[256];
    char szWindowInfo[256];
    char szDebuggingInfo[256];


	hr = __super::PreMessageLoop(nShowCmd);
	if (FAILED(hr)) {
        return hr;
	}

    // Initialize ATL Window Classes
    //
    AtlAxWinInit();

    // Prepare environment for web browser control
    //
    RegisterWebControlCompatiblity();

    // Prepare environment for detecting system conditions
    //
    boinc_parse_init_data_file();
    boinc_get_init_data(aid);

    // Create Window Instance
    //
    m_pWnd = new CHTMLBrowserWnd();
	if (m_pWnd == NULL)
	{
		__super::PostMessageLoop();
		return E_OUTOFMEMORY;
	}

    // Construct the window caption
    //
    if (aid.app_version) {
        snprintf(
            szWindowInfo, sizeof(szWindowInfo),
            "%s version %.2f [workunit: %s]",
            aid.app_name, aid.app_version/100.0, aid.wu_name
        );
    } else {
        snprintf(
            szWindowInfo, sizeof(szWindowInfo),
            "%s [workunit: %s]",
            aid.app_name, aid.wu_name
        );
    }

    if (is_htmlgfx_in_debug_mode()) {
        snprintf(szDebuggingInfo, sizeof(szDebuggingInfo), "[Web Server Port: %d]", get_htmlgfx_webserver_port());
    } else {
        strcpy(szDebuggingInfo, "");
    }

    snprintf(szWindowTitle, sizeof(szWindowTitle)-1, "%s %s", szWindowInfo, szDebuggingInfo);

    // Determine window size and placement
    if (is_htmlgfx_in_fullscreen_mode()) {
        HDC dc = GetDC(NULL);
        rc.left = 0;
        rc.top = 0;
        rc.right = GetDeviceCaps(dc, HORZRES);
        rc.bottom = GetDeviceCaps(dc, VERTRES);
        ReleaseDC(NULL, dc);

        dwStyle = WS_CLIPSIBLINGS|WS_CLIPCHILDREN|WS_POPUP;
        dwExStyle = WS_EX_APPWINDOW|WS_EX_TOPMOST;
        while(ShowCursor(false) >= 0);
    } else {
        rc.left = 0;
        rc.top = 0;
        rc.right = 1024;
        rc.bottom = 768;

        dwStyle = WS_CLIPSIBLINGS|WS_CLIPCHILDREN|WS_OVERLAPPEDWINDOW;
        dwExStyle = WS_EX_APPWINDOW|WS_EX_WINDOWEDGE;
        while(ShowCursor(true) < 0);
    }

    // Create Window
    //
	m_pWnd->Create(GetDesktopWindow(), rc, szWindowTitle, dwStyle, dwExStyle);
	m_pWnd->ShowWindow(nShowCmd);

	return S_OK;
}

HRESULT CBrowserModule::PostMessageLoop() throw()
{
    if (m_pWnd)
    {
        delete m_pWnd;
        m_pWnd = NULL;
    }
    
    AtlAxWinTerm();

    return __super::PostMessageLoop();
}


int run(int argc, char** argv) {
    return _AtlModule.WinMain(SW_SHOWDEFAULT);
}


extern int main(int, char**);
int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR Args, int WinMode) {
    LPSTR command_line;
    char* argv[100];
    int argc;
    int retval = 0;

    command_line = GetCommandLine();
    argc = parse_command_line(command_line, argv);

    WSADATA wsdata;
    WSAStartup( MAKEWORD( 1, 1 ), &wsdata);

    retval = main(argc, argv);

    WSACleanup();

    return retval;
}
