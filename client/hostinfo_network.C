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
#include "boinc_win.h"
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
#include "file_names.h"
#include "client_msgs.h"
#include "error_numbers.h"

#include "hostinfo.h"

// get domain name and IP address of this host
//
#ifdef _WIN32
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

#else

// gethostbyname() is a linkage nightmare on UNIX systems (go figure)
// so use a kludge instead: run ping and parse the output
// should be "PING domainname (ipaddr) ..."
//
static int try_ping(
    char* cmd, char* domain_name, int domlen, char* ip_addr, int iplen
) {
    int retval;
    char buf[256];

    retval = system(cmd);
    if (retval) return retval;
    FILE* f = fopen(TEMP_FILE_NAME, "r");
    if (!f) return ERR_FOPEN;
    fgets(buf, 256, f);
    fclose(f);
    char *p, *q;
    p = strchr(buf, ' ');
    if (!p) return ERR_NULL;
    p++;
    q = strchr(p, ' ');
    if (!q) return ERR_NULL;
    *q = 0;
    safe_strncpy(domain_name, p, domlen);
    q++;
    p = strchr(q, '(');
    if (!p) return ERR_NULL;
    p++;
    q = strchr(p, ')');
    if (!q) return ERR_NULL;
    *q = 0;
    safe_strncpy(ip_addr, p, iplen);
    return 0;
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
    if (retval) {
        sprintf(buf, "ping -c 1 %s > %s 2>/dev/null", hostname, TEMP_FILE_NAME);
        return try_ping(buf, domain_name, domlen, ip_addr, iplen);
    }
    return 0;
}

#endif
