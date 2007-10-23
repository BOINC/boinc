// Berkeley Open Infrastructure for Network Computing
// http://boinc.berkeley.edu
// Copyright (C) 2005 University of California
//
// This is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation;
// either version 2.1 of the License, or (at your option) any later version.
//
// This software is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//
// To view the GNU Lesser General Public License visit
// http://www.gnu.org/copyleft/lesser.html
// or write to the Free Software Foundation, Inc.,
// 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
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
    BOOL                    m_bClientLibraryInitialized;
};

#endif
