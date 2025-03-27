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

#if defined(_WIN32)
#include "boinc_win.h"
#include <fcntl.h>
#else
#include "config.h"
#if HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <cstdio>
#include <cstdlib>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <resolv.h>
#include <netdb.h>
#include <fcntl.h>
#include <errno.h>
#endif

#include "error_numbers.h"
#include "str_util.h"
#include "util.h"

#include "network.h"

using std::perror;
using std::sprintf;

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
    snprintf(buf, sizeof(buf), "error %d", e);
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
        snprintf(buf, sizeof(buf), "network internal error %d", errno);
        return buf;
#endif
    }
    snprintf(buf, sizeof(buf), "error %d", h_errno);
    return buf;
#endif
}

bool is_localhost(sockaddr_storage& s) {
#ifdef _WIN32
    if (ntohl(s.sin_addr.s_addr) == 0x7f000001) return true;
#else
    switch (s.ss_family) {
        case AF_INET: {
            sockaddr_in* sin = (sockaddr_in*)&s;
            return (ntohl(sin->sin_addr.s_addr) == 0x7f000001);
            break;
        }
        case AF_INET6: {
            sockaddr_in6* sin = (sockaddr_in6*)&s;
            char buf[256];
            inet_ntop(AF_INET6, (void*)(&sin->sin6_addr), buf, sizeof(buf));
            return (strcmp(buf, "::1") == 0);
            break;
        }
    }
#endif
    return false;
}

bool same_ip_addr(sockaddr_storage& s1, sockaddr_storage& s2) {
#ifdef _WIN32
    return (s1.sin_addr.s_addr == s2.sin_addr.s_addr);
#else
    if (s1.ss_family != s2.ss_family) return false;
    switch (s1.ss_family) {
        case AF_INET: {
            sockaddr_in* sin1 = (sockaddr_in*)&s1;
            sockaddr_in* sin2 = (sockaddr_in*)&s2;
            return (memcmp((void*)(&sin1->sin_addr), (void*)(&sin2->sin_addr), sizeof(in_addr)) == 0);
            break;
        }
        case AF_INET6: {
            sockaddr_in6* sin1 = (sockaddr_in6*)&s1;
            sockaddr_in6* sin2 = (sockaddr_in6*)&s2;
            return (memcmp((void*)(&sin1->sin6_addr), (void*)(&sin2->sin6_addr), sizeof(in6_addr)) == 0);
            break;
        }
    }
    return false;
#endif
}

int resolve_hostname(const char* hostname, sockaddr_storage &ip_addr) {
#ifdef _WIN32
    hostent* hep;
    hep = gethostbyname(hostname);
    if (!hep) {
        return ERR_GETHOSTBYNAME;
    }
    for (int i=0; ; i++) {
        if (!hep->h_addr_list[i]) break;
        ip_addr.sin_family = AF_INET;
        ip_addr.sin_addr.s_addr = *(int*)hep->h_addr_list[i];
        if ((ip_addr.sin_addr.s_addr&0xff) != 0x7f) return 0;     // look for non-loopback addr
    }
    return 0;

#else
    struct addrinfo *res, hints;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    char buf[512];
    snprintf(buf, sizeof(buf), "%s getaddrinfo(%s)", time_to_string(dtime()), hostname);
    int retval = getaddrinfo(hostname, NULL, &hints, &res);
    if (retval) {
        if (retval == EAI_SYSTEM) {
            perror(buf);
        } else {
            fprintf(stderr, "%s: %s\n", buf, gai_strerror(retval));
        }
        return ERR_GETADDRINFO;
    }
    struct addrinfo* aip = res;
    while (aip) {
        memcpy(&ip_addr, aip->ai_addr, aip->ai_addrlen);
        sockaddr_in* sin = (sockaddr_in*)&ip_addr;
        if ((sin->sin_addr.s_addr&0xff) != 0x7f) break;
        aip = aip->ai_next;
    }
    freeaddrinfo(res);
    return 0;
#endif
}

int resolve_hostname_or_ip_addr(
    const char* hostname, sockaddr_storage &ip_addr
) {
#ifdef _WIN32   // inet_pton() only on Vista or later!!
    int x = inet_addr(hostname);
    if (x != -1) {
        sockaddr_in* sin = (sockaddr_in*)&ip_addr;
        sin->sin_family = AF_INET;
        sin->sin_addr.s_addr = x;
        return 0;
    }
#else
    int retval;
    // check for IPV4 and IPV6 notation
    //
    sockaddr_in* sin = (sockaddr_in*)&ip_addr;
    retval = inet_pton(AF_INET, hostname, &sin->sin_addr);
    if (retval > 0) {
        ip_addr.ss_family = AF_INET;
        return 0;
    }
    sockaddr_in6* sin6 = (sockaddr_in6*)&ip_addr;
    retval = inet_pton(AF_INET6, hostname, &sin6->sin6_addr);
    if (retval > 0) {
        ip_addr.ss_family = AF_INET6;
        return 0;
    }
#endif

    // else resolve the name
    //
    return resolve_hostname(hostname, ip_addr);
}

int boinc_socket(int& fd, int protocol) {
    fd = (int)socket(protocol, SOCK_STREAM, 0);
    if (fd < 0) {
        char buf[256];
        snprintf(buf, sizeof(buf), "%s socket()", time_to_string(dtime()));
        perror(buf);
        return ERR_SOCKET;
    }
#ifndef _WIN32
    if (-1 == fcntl(fd, F_SETFD, FD_CLOEXEC)) {
        return ERR_FCNTL;
    }
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
    if (getsockopt(fd, SOL_SOCKET, SO_ERROR, (void*)&n, (socklen_t*)&intsize)) {
        return errno;
    }
#endif
    return n;
}

#if defined(_WIN32) && defined(USE_WINSOCK)

int WinsockInitialize() {
    WSADATA wsdata;
    return WSAStartup(MAKEWORD(2, 0), &wsdata);
}

int WinsockCleanup() {
    return WSACleanup();
}

#endif

void reset_dns() {
#if !defined(ANDROID) && !defined(_WIN32) && !defined(__APPLE__)
    // Windows doesn't have this, and it crashes Macs
    res_init();
#endif
}

// Get an unused port number by creating and binding a socket
// note: there's a slight race condition here;
// someone else might use the resulting port before you do.
// Used by vboxwrapper and docker_wrapper.
// I'm not sure if is_remote is relevant here - a port is a port, right?
//
int boinc_get_port(bool is_remote, int& port) {
    sockaddr_in addr;
    BOINC_SOCKLEN_T addrsize;
    int sock;
    int retval;

    addrsize = sizeof(sockaddr_in);

    memset(&addr, 0, sizeof(sockaddr_in));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(0);
    addr.sin_addr.s_addr = htonl(is_remote?INADDR_ANY:INADDR_LOOPBACK);

    retval = boinc_socket(sock);
    if (retval) return retval;

    retval = bind(sock, (const sockaddr*)&addr, addrsize);
    if (retval < 0) {
        boinc_close_socket(sock);
        return ERR_BIND;
    }

    retval = getsockname(sock, (sockaddr*)&addr, &addrsize);
    if (retval) {
        boinc_close_socket(sock);
        return errno;
    }
    port = ntohs(addr.sin_port);

    boinc_close_socket(sock);
    return 0;
}
