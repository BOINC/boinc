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

#ifndef UNIX_UTIL_H
#define UNIX_UTIL_H

// Nothing in this file is needed on WIN32
#ifndef _WIN32

#include "config.h"

// Notice that this has an ifndef around it.  If it is causing you problem,
// then try defining HAVE_SETENV in your configuration file.
#ifndef HAVE_SETENV
extern "C" int setenv(const char *name, const char *value, int overwrite);
#endif

// Notice that this has an ifndef around it.  If it is causing you problem,
// then try defining HAVE_DAEMON in your configuration file.
#ifndef HAVE_DAEMON
extern "C" int daemon(int nochdir, int noclose);
#endif /* HAVE_DAEMON */

// Notice that this has an ifndef around it.  If it is causing you problem,
// then try defining HAVE_ETHER_NTOA in your configuration file.
#ifndef HAVE_ETHER_NTOA

#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif
#ifdef HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif
#ifdef HAVE_ARPA_INET_H
#include <arpa/inet.h>
#endif
#ifdef HAVE_NET_IF_H
#include <net/if.h>
#endif
#ifdef HAVE_NETINET_IF_ETHER_H
#include <netinet/if_ether.h>
#endif
#ifdef HAVE_NETINET_ETHER_H
#include <netinet/ether.h>
#endif

// Notice that this has an ifndef around it.  If it is causing you problem,
// then try defining HAVE_STRUCT_ETHER_ADDR in your configuration file.
#ifndef HAVE_STRUCT_ETHER_ADDR
struct ether_addr {
    unsigned char ether_addr_octet[6];
};
#endif

extern "C" char *ether_ntoa(const struct ether_addr *addr);
#endif

#endif /* _WIN32 */

#endif /* UNIX_UTIL_H */

