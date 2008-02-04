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

#ifndef _BOINCCLIENTMANAGER_H_
#define _BOINCCLIENTMANAGER_H_

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma interface "BOINCClientManager.cpp"
#endif


class CBOINCClientManager : public wxObject
{
public:

    CBOINCClientManager();
    ~CBOINCClientManager();

    bool                AutoRestart();

    bool                IsSystemBooting();
    int                 IsBOINCConfiguredAsDaemon();
    bool                WasBOINCStartedByManager() { return m_bBOINCStartedByManager; };

    bool                IsBOINCCoreRunning();
    bool                StartupBOINCCore();
    void                ShutdownBOINCCore();
#ifdef __WXMAC__
    bool                ProcessExists(pid_t thePID);
    static OSErr        QuitAppleEventHandler( const AppleEvent *appleEvt, AppleEvent* reply, UInt32 refcon );
#endif

protected:

    bool                m_bBOINCStartedByManager;
    int                 m_lBOINCCoreProcessId;

#ifdef __WXMSW__
    HANDLE              m_hBOINCCoreProcess;
#endif

};


#endif

