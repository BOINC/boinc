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

extern void boinc_close_socket(int sock);
extern int get_socket_error(int fd);

#if defined(_WIN32)
typedef int socklen_t;
#define SHUT_WR SD_SEND
#elif defined( __APPLE__)
typedef int32_t socklen_t;
#elif !defined(GETSOCKOPT_SOCKLEN_T) && !defined(_SOCKLEN_T_DECLARED) && !defined(socklen_t)
typedef size_t socklen_t;
#endif

#define CONNECTED_STATE_NOT_CONNECTED   0
#define CONNECTED_STATE_CONNECTED       1
#define CONNECTED_STATE_UNKNOWN         2

extern int get_connected_state();

#ifdef _WIN32
extern int WinsockInitialize();
extern int WinsockCleanup();
extern int  NetOpen();
extern void NetClose();
extern void NetCheck(bool hangup_if_dialed);
#endif
