// The contents of this file are subject to the BOINC Public License
// Version 1.0 (the "License"); you may not use this file except in
// compliance with the License. You may obtain a copy of the License at
// http://boinc.berkeley.edu/license_1.0.txt
//
// Software distributed under the License is distributed on an "AS IS"
// basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the
// License for the specific language governing rights and limitations
// under the License.
//
// The Original Code is the Berkeley Open Infrastructure for Network Computing.
//
// The Initial Developer of the Original Code is the SETI@home project.
// Portions created by the SETI@home project are Copyright (C) 2002
// University of California at Berkeley. All Rights Reserved.
//
// Contributor(s):
//

#ifdef _WIN32
#include "stdafx.h"
#endif

#ifndef _WIN32
#include <stdio.h>
#include <string.h>

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

#include "util.h"
#include "parse.h"
#include "client_msgs.h"
#include "error_numbers.h"

#include "hostinfo.h"

// Returns the domain of the local host
//
int get_local_domain_name(char* p, int len) {
    char buf[256];

    if (gethostname(buf, 256)) return ERR_GETHOSTBYNAME;
    struct hostent* he = gethostbyname(buf);
    if (!he) return ERR_GETHOSTBYNAME;
    safe_strncpy(p, he->h_name, len);
    return 0;
}

// Get the IP address of the local host
//
static int get_local_ip_addr(struct in_addr& addr) {
#if HAVE_NETDB_H || _WIN32
    char buf[256];
    if (gethostname(buf, 256)) {
        msg_printf(NULL, MSG_ERROR, "get_local_ip_addr(): gethostname failed\n");
        return ERR_GETHOSTNAME;
    }
    struct hostent* he = gethostbyname(buf);
    if (!he || !he->h_addr_list[0]) {
        msg_printf(NULL, MSG_ERROR, "get_local_ip_addr(): gethostbyname failed\n");
        return ERR_GETHOSTBYNAME;
    }
    memcpy(&addr, he->h_addr_list[0], sizeof(addr));
    return 0;
#elif
    GET IP ADDR NOT IMPLEMENTED
#endif
}

// Get the IP address as a string
//
int get_local_ip_addr_str(char* p, int len) {
    int retval;
    struct in_addr addr;

    strcpy(p, "");
    retval = get_local_ip_addr(addr);
    if (retval) return retval;
    safe_strncpy(p, inet_ntoa(addr), len);
    return 0;
}
