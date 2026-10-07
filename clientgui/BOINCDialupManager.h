// This file is part of BOINC.
// https://boinc.berkeley.edu
// Copyright (C) 2026 University of California
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
    wxDialUpManager* m_pDialupManager = nullptr;
    wxDateTime       m_dtLastDialupRequest;
    wxDateTime       m_dtDialupConnectionTimeout;
    bool             m_bSetConnectionTimer = false;
    bool             m_bNotifyConnectionAvailable = false;
    bool             m_bConnectedSuccessfully = false;
    bool             m_bResetTimers = false;
    bool             m_bWasDialing = false;
    int              m_iNetworkStatus = 0;
    int              m_iConnectAttemptRetVal = 0;

    wxString         m_strDialogTitle;
};

#endif
