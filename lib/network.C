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

#if defined(_WIN32) && !defined(__STDWX_H__) && !defined(_BOINC_WIN_) && !defined(_AFX_STDAFX_H_)
#include "boinc_win.h"
#endif

#ifndef _WIN32
#include <unistd.h>
#include <cstdio>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <fcntl.h>
#endif

#include "error_numbers.h"
#include "network.h"

const char* socket_error_str() {
    static char buf[80];
#ifdef _WIN32
    int e = WSAGetLastError();
    switch (e) {
    case WSANOTINITIALISED:
        return "WSA not initialized";
    case WSAENETDOWN:
        return "the network subsystem has failed";
    case WSAHOST_NOT_FOUND:
        return "host name not found";
    case WSATRY_AGAIN:
        return "no response from server";
    case WSANO_RECOVERY:
        return "a nonrecoverable error occurred";
    case WSANO_DATA:
        return "valid name, no data record of requested type";
    case WSAEINPROGRESS:
        return "a blocking socket call in progress";
    case WSAEFAULT:
        return "invalid part of user address space";
    case WSAEINTR:
        return "a blocking socket call was canceled";
    case WSAENOTSOCK:
        return "not a socket";
    }
    sprintf(buf, "error %d", e);
    return buf;
#else
    switch (h_errno) {
    case HOST_NOT_FOUND:
        return "host not found";
    case NO_DATA:
        return "valid name, no data record of requested type";
    case NO_RECOVERY:
        return "a nonrecoverable error occurred";
    case TRY_AGAIN:
        return "host not found or server failure";
    }
    sprintf(buf, "error %d", h_errno);
    return buf;
#endif
}

int resolve_hostname(char* hostname, int &ip_addr, char* msg) {

    // if the hostname is in Internet Standard dotted notation, 
    // return that address.
    //
    ip_addr = inet_addr(hostname);
    if (ip_addr != -1) {
        return 0;
    }

    // else resolve the name
    //
    hostent* hep;
    hep = gethostbyname(hostname);
    if (!hep) {
        sprintf(msg, "Can't resolve hostname [%s] %s", hostname, socket_error_str());
        return ERR_GETHOSTBYNAME;
    }
    ip_addr = *(int*)hep->h_addr_list[0];
    return 0;
}

int boinc_socket(int& fd) {
    fd = (int)socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) {
        perror("socket");
        return ERR_SOCKET;
    }
    return 0;
}

int boinc_socket_asynch(int fd, bool asynch) {
    if (asynch) {
#ifdef WIN32
        unsigned long one = 1;
        ioctlsocket(fd, FIONBIO, &one);
#else
        int flags;
        flags = fcntl(fd, F_GETFL, 0);
        if (flags < 0) {
            return ERR_FCNTL;
        }
        if (fcntl(fd, F_SETFL, flags|O_NONBLOCK) < 0 ) {
            return ERR_FCNTL;
        }
#endif
    } else {
#ifdef WIN32
        unsigned long zero = 0;
        ioctlsocket(fd, FIONBIO, &zero);
#else
        int flags;
        flags = fcntl(fd, F_GETFL, 0);
        if (flags < 0) {
            return ERR_FCNTL;
        }
        if (fcntl(fd, F_SETFL, flags&(~O_NONBLOCK)) < 0 ) {
            return ERR_FCNTL;
        }
#endif
    }
    return 0;
}

void boinc_close_socket(int sock) {
#ifdef _WIN32
    closesocket(sock);
#else
    close(sock);
#endif
}

int get_socket_error(int fd) {
    boinc_socklen_t intsize = sizeof(int);
    int n;
#ifdef WIN32
    getsockopt(fd, SOL_SOCKET, SO_ERROR, (char *)&n, &intsize);
#elif defined(__APPLE__)
    getsockopt(fd, SOL_SOCKET, SO_ERROR, &n, (int *)&intsize);
#elif defined(__FreeBSD__)
    // workaround for FreeBSD. I don't understand this.
    struct sockaddr_in sin;
    socklen_t sinsz = sizeof(sin);
    n = getpeername(fd, (struct sockaddr *)&sin, &sinsz);
#else
    getsockopt(fd, SOL_SOCKET, SO_ERROR, (void*)&n, &intsize);
#endif
    return n;
}

#ifdef _WIN32

typedef BOOL (WINAPI *GetStateProc)( OUT LPDWORD  lpdwFlags, IN DWORD    dwReserved);

int get_connected_state( ) {
    int online = 0;
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

int WinsockInitialize() {
    WSADATA wsdata;
    return WSAStartup( MAKEWORD( 1, 1 ), &wsdata);
}

int WinsockCleanup() {
    return WSACleanup();
}

#else

// anyone know how to see if this host has physical network connection?
//
int get_connected_state() {
    return CONNECTED_STATE_UNKNOWN;
}

#endif
const char *BOINC_RCSID_557bf0741f="$Id$";
