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
#ifndef _BOINC_NETWORK_H_
#define _BOINC_NETWORK_H_


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
    void fdset_union(FDSET_GROUP& g1, FDSET_GROUP& g2) {
        max_fd = g1.max_fd>g2.max_fd?g1.max_fd:g2.max_fd;
        zero();
        for (int i=0; i<=max_fd; i++) {
            if (FD_ISSET(i, &g1.read_fds)) FD_SET(i, &read_fds);
            if (FD_ISSET(i, &g1.write_fds)) FD_SET(i, &write_fds);
            if (FD_ISSET(i, &g1.exc_fds)) FD_SET(i, &exc_fds);
        }
    }
};

extern int resolve_hostname(char* hostname, int& ip_addr, char* msg);
extern int boinc_socket(int& sock);
extern int boinc_socket_asynch(int sock, bool asynch);
extern void boinc_close_socket(int sock);
extern int get_socket_error(int fd);
extern const char* socket_error_str();

#if defined(_WIN32)
typedef int boinc_socklen_t;
#define SHUT_WR SD_SEND
#elif defined( __APPLE__)
typedef int32_t boinc_socklen_t;
#else
typedef BOINC_SOCKLEN_T boinc_socklen_t;
#endif

#define CONNECTED_STATE_NOT_CONNECTED   0
#define CONNECTED_STATE_CONNECTED       1
#define CONNECTED_STATE_UNKNOWN         2

extern int get_connected_state();

#ifdef _WIN32
extern int WinsockInitialize();
extern int WinsockCleanup();
#endif
#endif
