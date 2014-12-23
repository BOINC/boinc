// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2010-2012 University of California
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
#include <string>
#include "win_util.h"
#include "version.h"
#include "boinc_api.h"
#include "diagnostics.h"
#include "filesys.h"
#include "vboxhtmlgfx_win.h"
#include "vboxlogging.h"
#include "vboxcheckpoint.h"
#include "browserctrl_win.h"
#include "browserwnd_win.h"
#include "browsermain_win.h"


#ifdef _MSC_VER
#define snprintf _snprintf
#endif


CBrowserModule::CBrowserModule()
{
    m_pWnd = NULL;
    m_bFullscreen = false;
}

HRESULT CBrowserModule::InitializeCom() throw()
{
    return ::OleInitialize(NULL);
}

void CBrowserModule::UninitializeCom() throw()
{
    ::OleUninitialize();
}

HRESULT CBrowserModule::PreMessageLoop(int nShowCmd) throw()
{
    RECT rc = {0, 0, 0, 0};
    DWORD dwExStyle = 0;
    DWORD dwStyle = 0;
    APP_INIT_DATA aid;
    char szWindowTitle[256];


	HRESULT hr = __super::PreMessageLoop(nShowCmd);
	if (FAILED(hr)) {
        return hr;
	}

    // Initialize ATL Window Classes
    //
    AtlAxWinInit();

    // Prepare environment for detecting system conditions
    //
    boinc_parse_init_data_file();
    boinc_get_init_data(aid);

    // Construct the window caption
    if (aid.app_version) {
        snprintf(
            szWindowTitle, sizeof(szWindowTitle),
            "%s version %.2f [workunit: %s]",
            aid.app_name, aid.app_version/100.0, aid.wu_name
        );
    } else {
        snprintf(
            szWindowTitle, sizeof(szWindowTitle),
            "%s [workunit: %s]",
            aid.app_name, aid.wu_name
        );
    }

    if (m_bFullscreen) {

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
        rc.right = 800;
        rc.bottom = 600;

        dwStyle = WS_CLIPSIBLINGS|WS_CLIPCHILDREN|WS_OVERLAPPEDWINDOW;
        dwExStyle = WS_EX_APPWINDOW|WS_EX_WINDOWEDGE;
        while(ShowCursor(true) < 0);

    }

    m_pWnd = new CHTMLBrowserWnd();
	if (m_pWnd == NULL)
	{
		__super::PostMessageLoop();
		return E_OUTOFMEMORY;
	}

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

int CBrowserModule::BOINCParseCommandLine(int argc, char** argv) {
    for (int i=1; i<argc; i++) {
        if (!strcmp(argv[i], "--fullscreen")) {
            vboxlog_msg("Fullscreen mode requested.");
            m_bFullscreen = true;
        }
    }
    return 0;
}


CBrowserModule _AtlModule;


int run(int argc, char** argv) {
    _AtlModule.BOINCParseCommandLine(argc, argv);
    return _AtlModule.WinMain(SW_SHOWDEFAULT);
}


extern int main(int, char**);
int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR Args, int WinMode) {
    LPSTR command_line;
    char* argv[100];
    int argc;

    command_line = GetCommandLine();
    argc = parse_command_line(command_line, argv);
    return main(argc, argv);
}
