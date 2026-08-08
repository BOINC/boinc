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

#ifndef _BOINCTRAY_WIN_H
#define _BOINCTRAY_WIN_H


//-----------------------------------------------------------------------------
// Name: class CBOINCTray
// Desc: BOINC Tray class
//-----------------------------------------------------------------------------
class CBOINCTray
{
public:
    CBOINCTray();

    virtual INT            Run( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow );

protected:
    BOOL                    CreateDataManagementThread();
    BOOL                    DestroyDataManagementThread();

    DWORD WINAPI            DataManagementProc();
    static DWORD WINAPI     DataManagementProcStub( LPVOID lpParam );
	LRESULT                 TrayProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );
    static LRESULT CALLBACK TrayProcStub( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );

    HANDLE                  m_hDataManagementThread;
    BOOL                    m_bIdleTrackerInitialized;
};

#endif
