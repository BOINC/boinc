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

#ifdef _WIN32
#include "boinc_win.h"

#else 

#include "config.h"
#include <cstdio>
#include <cstring>

#if HAVE_UNISTD_H
#include <unistd.h>
#endif
#if HAVE_NETDB_H
#include <netdb.h>
#endif
#if HAVE_ARPA_INET_H
#include <arpa/inet.h>
#endif
#if HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#if HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif
#endif

#ifdef __APPLE__
#include <Carbon/Carbon.h>
#endif

#include "util.h"
#include "parse.h"
#include "file_names.h"
#include "client_msgs.h"
#include "error_numbers.h"

#include "hostinfo.h"

// get domain name and IP address of this host
//
int HOST_INFO::get_local_network_info() {
    char buf[256];
    struct in_addr addr;
    struct hostent* he;
    
    if (gethostname(buf, 256)) return ERR_GETHOSTBYNAME;
    he = gethostbyname(buf);
    if (!he || !he->h_addr_list[0]) {
        msg_printf(NULL, MSG_ERROR, "gethostbyname (%s) failed", buf);
        return ERR_GETHOSTBYNAME;
    }
    strlcpy(domain_name, he->h_name, sizeof(domain_name));
    memcpy(&addr, he->h_addr_list[0], sizeof(addr));
    strlcpy(ip_addr, inet_ntoa(addr), sizeof(ip_addr));
    return 0;
}

const char *BOINC_RCSID_9275b20aa5 = "$Id$";
