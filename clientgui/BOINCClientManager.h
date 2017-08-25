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

#ifndef BOINC_BOINCCLIENTMANAGER_H
#define BOINC_BOINCCLIENTMANAGER_H

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma interface "BOINCClientManager.cpp"
#endif

class CBOINCClientManager : public wxObject {
public:

    CBOINCClientManager();
    ~CBOINCClientManager();

    bool                AutoRestart();

    bool                IsSystemBooting();
    int                 IsBOINCConfiguredAsDaemon();

    bool                WasBOINCStartedByManager() { return m_bBOINCStartedByManager; };

    bool                IsBOINCCoreRunning();
    bool                StartupBOINCCore();
    void                ShutdownBOINCCore(bool ShuttingDownManager);
    void                KillClient();

protected:
    bool                m_bBOINCStartedByManager;
    int                 m_lBOINCCoreProcessId;
    double              m_fAutoRestart1Time;
    double              m_fAutoRestart2Time;

#ifdef __WXMSW__
    HANDLE              m_hBOINCCoreProcess;
#endif

};

class ClientCrashDlg : public wxDialog {
    DECLARE_DYNAMIC_CLASS( ClientCrashDlg )
    DECLARE_EVENT_TABLE()

public:
    ClientCrashDlg(double timeDiff);
    void                        OnHelp(wxCommandEvent& event);
};

#endif
