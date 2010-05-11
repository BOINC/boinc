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

#if   defined(_WIN32) && !defined(__STDWX_H__)
#include "boinc_win.h"
#elif defined(_WIN32) && defined(__STDWX_H__)
#include "stdwx.h"
#else
#include "config.h"
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <cstdio>
#include <cstdlib>
#ifdef HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif
#ifdef HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif
#ifdef HAVE_NETINET_TCP_H
#include <netinet/tcp.h>
#endif
#ifdef HAVE_ARPA_INET_H
#include <arpa/inet.h>
#endif
#include <resolv.h>
#include <netdb.h>
#include <fcntl.h>
#include <errno.h>
#endif

using std::perror;
using std::sprintf;

#include "error_numbers.h"
#include "network.h"

const char* socket_error_str() {
    static char buf[80];
#if defined(_WIN32) && defined(USE_WINSOCK)
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
#ifdef NETDB_INTERNAL
    case NETDB_INTERNAL:
		sprintf(buf,"network internal error %d",errno);
		return buf;
#endif
    }
    sprintf(buf, "error %d", h_errno);
    return buf;
#endif
}

int resolve_hostname(char* hostname, int &ip_addr) {

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
#ifndef _WIN32
    fcntl(fd, F_SETFD, FD_CLOEXEC);
#endif
    return 0;
}

int boinc_socket_asynch(int fd, bool asynch) {
    if (asynch) {
#if defined(_WIN32) && defined(USE_WINSOCK)
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
#if defined(_WIN32) && defined(USE_WINSOCK)
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
#if defined(_WIN32) && defined(USE_WINSOCK)
    closesocket(sock);
#else
    close(sock);
#endif
}

int get_socket_error(int fd) {
    int n;
#if defined(_WIN32) && defined(USE_WINSOCK)
    int intsize = sizeof(int);
    getsockopt(fd, SOL_SOCKET, SO_ERROR, (char *)&n, &intsize);
#elif defined(__FreeBSD__)
    // workaround for FreeBSD. I don't understand this.
    struct sockaddr_in sin;
    socklen_t sinsz = sizeof(sin);
    n = getpeername(fd, (struct sockaddr *)&sin, &sinsz);
#else
    socklen_t intsize = sizeof(int);
    getsockopt(fd, SOL_SOCKET, SO_ERROR, (void*)&n, (socklen_t*)&intsize);
#endif
    return n;
}

#if defined(_WIN32) && defined(USE_WINSOCK)

int WinsockInitialize() {
    WSADATA wsdata;
    return WSAStartup( MAKEWORD( 1, 1 ), &wsdata);
}

int WinsockCleanup() {
    return WSACleanup();
}

#endif

void reset_dns() {
#if !defined(_WIN32) && !defined(__APPLE__)
    // Windows doesn't have this, and it crashes Macs
    res_init();
#endif
}

