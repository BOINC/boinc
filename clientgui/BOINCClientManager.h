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

    void                DisableBOINCStartedByManager() { m_bBOINCStartedByManager = false; };
    void                EnableBOINCStartedByManager() { m_bBOINCStartedByManager = true; };
    bool                WasBOINCStartedByManager() { return m_bBOINCStartedByManager; };

    bool                IsBOINCCoreRunning();
    bool                StartupBOINCCore();
    void                ShutdownBOINCCore();
    void                KillClient();

protected:
    bool                m_bBOINCStartedByManager;
    int                 m_lBOINCCoreProcessId;

#ifdef __WXMSW__
    HANDLE              m_hBOINCCoreProcess;
#endif

};


#endif

