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
// 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

#include "boinc_win.h"

#include "win_net.h"
#include "client_state.h"
#include "hostinfo_network.h"

#define DIAL_WAIT       60          // seconds after dial to wait (in case of cancel)
#define CONFIRM_WAIT    60          // seconds after user says not to connect to ask again
#define CLOSE_WAIT      5           // seconds after last call to close that the connection should be terminated

int net_ref_count = -1;             // -1 closed, 0 open but not used, >0 number of users
double net_last_req_time = 0;       // last time user was requested to connect in seconds
double net_last_dial_time = 0;      // last time modem was dialed
double net_close_time = 0;          // 0 don't close, >0 time when network connection should be terminated in seconds
bool dialed = false;

int WinsockInitialize()
{
    WSADATA wsdata;
    return WSAStartup( MAKEWORD( 1, 1 ), &wsdata);
}

int WinsockCleanup()
{
    return WSACleanup();
}

typedef BOOL (WINAPI *GetStateProc)( OUT LPDWORD  lpdwFlags, IN DWORD    dwReserved);

int get_connected_state( ) {
    int        online = 0;
    static bool first=true;
    static HMODULE libmodule;
    static GetStateProc GetState;
    DWORD connectionFlags;

    if (first) {
        libmodule = LoadLibrary("wininet.dll");
        if (libmodule) {
            GetState = (GetStateProc) GetProcAddress(libmodule, "InternetGetConnectedState");
        }
        first = false;
    }
    if (libmodule && GetState) {
        online = (*GetState)(&connectionFlags, 0);
        if (online) {
            return CONNECTED_STATE_CONNECTED;
        } else {
            return CONNECTED_STATE_NOT_CONNECTED;
        }
    }
    return CONNECTED_STATE_UNKNOWN;
}

int NetOpen( void )
{
    int rc;

    typedef BOOL (WINAPI *GetStateProc)( OUT LPDWORD  lpdwFlags, IN DWORD    dwReserved);
    typedef BOOL (WINAPI *AutoDialProc)( IN DWORD    dwFlags, IN DWORD    dwReserved);

    if(net_ref_count >= 0) {
        net_ref_count ++;
        return 0;
    }

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

const char *BOINC_RCSID_4971b5333e = "$Id$";
