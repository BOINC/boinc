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

#ifndef BOINC_BOINCDIALUPMANAGER_H
#define BOINC_BOINCDIALUPMANAGER_H

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma interface "BOINCDialupManager.cpp"
#endif


class CBOINCDialUpManager : public wxObject {
public:

    CBOINCDialUpManager();
    ~CBOINCDialUpManager();

    bool IsOk();
    size_t GetISPNames(wxArrayString& names);

    void OnPoll();

    int Connect();
    int ConnectionSucceeded();
    int ConnectionFailed();

    int NetworkAvailable();

    int Disconnect();

    void ResetReminderTimers();

protected:
    wxDialUpManager* m_pDialupManager;
    wxDateTime       m_dtLastDialupRequest;
    wxDateTime       m_dtDialupConnectionTimeout;
    bool             m_bSetConnectionTimer;
    bool             m_bNotifyConnectionAvailable;
    bool             m_bConnectedSuccessfully;
    bool             m_bResetTimers;
    bool             m_bWasDialing;
    int              m_iNetworkStatus;
    int              m_iConnectAttemptRetVal;

    wxString         m_strDialogTitle;
};

#endif
