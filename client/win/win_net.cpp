// The contents of this file are subject to the BOINC Public License
// Version 1.0 (the "License"); you may not use this file except in
// compliance with the License. You may obtain a copy of the License at
// http://boinc.berkeley.edu/license_1.0.txt
//
// Software distributed under the License is distributed on an "AS IS"
// basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the
// License for the specific language governing rights and limitations
// under the License.
//
// The Original Code is the Berkeley Open Infrastructure for Network Computing.
//
// The Initial Developer of the Original Code is the SETI@home project.
// Portions created by the SETI@home project are Copyright (C) 2002
// University of California at Berkeley. All Rights Reserved.
//
// Contributor(s):
//

#include "stdafx.h"

#include "win_net.h"
#include "client_state.h"

#define DIAL_WAIT       60          // seconds after dial to wait (in case of cancel)
#define CONFIRM_WAIT    60          // seconds after user says not to connect to ask again
#define CLOSE_WAIT      5           // seconds after last call to close that the connection should be terminated

int net_ref_count = -1;             // -1 closed, 0 open but not used, >0 number of users
double net_last_req_time = 0;       // last time user was requested to connect in seconds
double net_last_dial_time = 0;      // last time modem was dialed
double net_close_time = 0;          // 0 don't close, >0 time when network connection should be terminated in seconds
bool dialed = false;

int NetOpen( void )
{
    if(net_ref_count >= 0) {
        net_ref_count ++;
        return 0;
    }

    WSADATA wsdata;
    WORD    wVersionRequested;
    int rc, addrlen = 16;

    typedef BOOL (WINAPI *GetStateProc)( OUT LPDWORD  lpdwFlags, IN DWORD    dwReserved);
    typedef BOOL (WINAPI *AutoDialProc)( IN DWORD    dwFlags, IN DWORD    dwReserved);

    GetStateProc GetState = NULL;
    AutoDialProc AutoDial = NULL;
    DWORD connectionFlags;
    HMODULE libmodule = NULL;

    libmodule = LoadLibrary("wininet.dll");
    if (libmodule) {
        GetState = (GetStateProc)GetProcAddress(libmodule, "InternetGetConnectedState");
        AutoDial = (AutoDialProc)GetProcAddress(libmodule, "InternetAutodial");

        if (GetState && AutoDial) {
            rc = (*GetState)(&connectionFlags, 0);

            // Don't Autodial if already connected to Internet by Modem or LAN
            if (!rc) {
                if((double)time(NULL) < net_last_dial_time + CONFIRM_WAIT) {
                    return -1;
                }
#if !defined(_WIN32) && !defined(_CONSOLE)
                if(gstate.global_prefs.confirm_before_connecting) {
                    net_last_req_time = (double)time(NULL);
                    if(!RequestNetConnect()) {
                        return -1;
                    }
                }
#endif
                net_last_dial_time = (double)time(NULL);
                rc = (*AutoDial)(INTERNET_AUTODIAL_FORCE_UNATTENDED, 0);
                if (rc) {
                    dialed = true;
                } else {
                    // InternetAutodial() returns error 86 for some users
                    // and 668 for some other users, but a subsequent call
                    // to gethostbyname() or connect() autodials successfully.
                    // So (with one exception) we ignore failure returns
                    // from InternetAutodial() to work around this problem.
                    // Error 86 is "The specified Network Password is not correct."
                    // Error 668 is RAS Error "The connection dropped."
                    rc = GetLastError();
                    // Don't continue if busy signal, no answer or user cancelled
                    if (rc == ERROR_USER_DISCONNECTION) {
                        return -1;
                    }
                }
            }
        }
    }


    wVersionRequested = MAKEWORD(1, 1);
    rc = WSAStartup(wVersionRequested, &wsdata);
    if (rc) {
        return -1;
    }

    net_ref_count = 1;
    return 0;
}

void NetClose( void )
{
    if(net_ref_count > 0) net_ref_count --;
    if(net_ref_count == 0) {
        net_close_time = (double)time(NULL) + CLOSE_WAIT;
    }
}

void NetCheck( void ) {
    if(net_ref_count == 0 && net_close_time > 0 && net_close_time < (double)time(NULL)) {
        WSACleanup();

        typedef BOOL (WINAPI *HangupProc)(IN DWORD    dwReserved);
        HangupProc HangUp = NULL;
        HMODULE libmodule = NULL;

        // Hang up the modem if we dialed it
        if (dialed && gstate.global_prefs.hangup_if_dialed) {
            libmodule = LoadLibrary("wininet.dll");
            if (libmodule) {
                HangUp = (HangupProc)GetProcAddress(libmodule, "InternetAutodialHangup");
                if (HangUp)     int rc = (* HangUp)(0);
            }
        }
        dialed = false;
        net_ref_count = -1;
        net_close_time = 0;
    }
}
