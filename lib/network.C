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

#ifndef _WIN32
#include <unistd.h>
#include <sys/socket.h>
#endif

#include "network.h"

void boinc_close_socket(int sock) {
#ifdef _WIN32
    closesocket(sock);
#else
    close(sock);
#endif
}

int get_socket_error(int fd) {
    socklen_t intsize = sizeof(int);
    int n;
#ifdef WIN32
    getsockopt(fd, SOL_SOCKET, SO_ERROR, (char *)&n, &intsize);
#elif __APPLE__
    getsockopt(fd, SOL_SOCKET, SO_ERROR, &n, (int *)&intsize);
#else
    getsockopt(fd, SOL_SOCKET, SO_ERROR, (void*)&n, &intsize);
#endif
    return n;
}
