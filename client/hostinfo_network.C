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
#endif

#ifndef _WIN32
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

#include "util.h"
#include "parse.h"
#include "file_names.h"
#include "client_msgs.h"
#include "error_numbers.h"

#include "hostinfo.h"

// get domain name and IP address of this host
//
int get_local_network_info(
    char* domain_name, int domlen, char* ip_addr, int iplen
) {
    char buf[256];
    struct in_addr addr;

    if (gethostname(buf, 256)) return ERR_GETHOSTBYNAME;
    struct hostent* he = gethostbyname(buf);
    if (!he || !he->h_addr_list[0]) {
        msg_printf(NULL, MSG_ERROR, "get_local_network_info(): gethostbyname failed\n");
        return ERR_GETHOSTBYNAME;
    }
    safe_strncpy(domain_name, he->h_name, domlen);
    memcpy(&addr, he->h_addr_list[0], sizeof(addr));
    safe_strncpy(ip_addr, inet_ntoa(addr), iplen);
    return 0;
}

#if 0

// NEW POLICY: on UNIX, get just the hostname.
// the rest (domain name, IP address) is a can of worms,
// and doesn't really matter anyway

int get_local_network_info(
    char* domain_name, int domlen, char* ip_addr, int iplen
) {
    char hostname[256];
    char buf[256];
    int retval;

    strcpy(domain_name, "unknown");
    strcpy(ip_addr, "unknown");
    if (!gethostname(hostname, 256)) {
        safe_strncpy(domain_name, hostname, domlen);
    }
    return 0;
}

#if 0
// gethostbyname() is a linkage nightmare on UNIX systems (go figure)
// so use a kludge instead: run ping and parse the output.
// The output should have a line like
// "9 bytes from a.b.c.d (1.0.1.0): icmp_seq=0"
//
static int try_ping(
    char* cmd, char* domain_name, int domlen, char* ip_addr, int iplen
) {
    int retval,n;
    char buf[256];
    char *p, *q;

    retval = system(cmd);
    if (retval) return retval;
    FILE* f = fopen(TEMP_FILE_NAME, "r");
    if (!f) return ERR_FOPEN;
    while (fgets(buf, 256, f)) {
        p = strchr(buf, '(');
        if (!p) continue;
        q = strchr(p, ')');
        if (!q) continue;
        *q = 0;
        if (!strchr(p, '.')) continue;
        safe_strncpy(ip_addr, p+1, iplen);

        *p = 0;
        p = strrchr(buf, ' ');
        if (!p) continue;
        *p = 0;
        p = strrchr(buf, ' ');
        if (!p) continue;
        p++;
        if (!strchr(p, '.')) continue;
        safe_strncpy(domain_name, p, domlen);
        return 0;
    }
    return ERR_NULL;
}

int get_local_network_info(
    char* domain_name, int domlen, char* ip_addr, int iplen
) {
    char hostname[256];
    char buf[256];
    int retval;

    if (gethostname(hostname, 256)) return ERR_GETHOSTBYNAME;

    sprintf(buf, "ping -c 1 -w 1 %s > %s 2>/dev/null", hostname, TEMP_FILE_NAME);
    retval = try_ping(buf, domain_name, domlen, ip_addr, iplen);
    if (!retval) return 0;

    sprintf(buf, "ping -c 1 %s > %s 2>/dev/null", hostname, TEMP_FILE_NAME);
    retval = try_ping(buf, domain_name, domlen, ip_addr, iplen);
    if (!retval) return 0;

    sprintf(buf, "/usr/sbin/ping -s %s 1 1 > %s 2>/dev/null", hostname, TEMP_FILE_NAME);
    retval = try_ping(buf, domain_name, domlen, ip_addr, iplen);
    if (!retval) return 0;

    msg_printf(NULL, MSG_INFO, "Couldn't get hostname and IP address");
    msg_printf(NULL, MSG_INFO, "Make sure 'ping' is in your search path");

    strcpy(domain_name, "unknown");
    strcpy(ip_addr, "0.0.0.0");

    return 0;
}
#endif

#endif

const char *BOINC_RCSID_9275b20aa5 = "$Id$";
