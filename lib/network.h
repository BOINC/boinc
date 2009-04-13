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

#ifndef _BOINC_NETWORK_H_
#define _BOINC_NETWORK_H_

#include <string.h>
#ifndef _WIN32
#include <unistd.h>
#include "config.h"
#else
#include "boinc_win.h"
#endif

struct FDSET_GROUP {
    fd_set read_fds;
    fd_set write_fds;
    fd_set exc_fds;
    int max_fd;

    void zero() {
        FD_ZERO(&read_fds);
        FD_ZERO(&write_fds);
        FD_ZERO(&exc_fds);
        max_fd = -1;
    }
};

extern int resolve_hostname(char* hostname, int& ip_addr);
extern int boinc_socket(int& sock);
extern int boinc_socket_asynch(int sock, bool asynch);
extern void boinc_close_socket(int sock);
extern int get_socket_error(int fd);
extern const char* socket_error_str();
extern void reset_dns();

#if defined(_WIN32) && defined(USE_WINSOCK)
typedef int boinc_socklen_t;
#define SHUT_WR SD_SEND
#else
typedef BOINC_SOCKLEN_T boinc_socklen_t;
#endif


#ifndef NETWORK_ALIVE_LAN
#define NETWORK_ALIVE_LAN   0x00000001
#endif

#ifndef NETWORK_ALIVE_WAN
#define NETWORK_ALIVE_WAN   0x00000002
#endif

#ifndef NETWORK_ALIVE_AOL
#define NETWORK_ALIVE_AOL   0x00000004
#endif


#if defined(_WIN32) && defined(USE_WINSOCK)
extern int WinsockInitialize();
extern int WinsockCleanup();
#endif
#endif
