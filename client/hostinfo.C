// The contents of this file are subject to the Mozilla Public License
// Version 1.0 (the "License"); you may not use this file except in
// compliance with the License. You may obtain a copy of the License at
// http://www.mozilla.org/MPL/ 
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

#ifdef _WIN32
#include <winsock.h>
#endif

#include "parse.h"
#include "hostinfo.h"
#include "error_numbers.h"

// Reset the host info struct to default values
//
void clear_host_info(HOST_INFO& host) {
    host.timezone = 0;		// seconds added to local time to get UTC
    strcpy(host.domain_name,"");
    strcpy(host.serialnum,"");
    strcpy(host.ip_addr,"");

    host.on_frac = 0;
    host.conn_frac = 0;
    host.active_frac = 0;

    host.p_ncpus = 0;
    strcpy(host.p_vendor,"");
    strcpy(host.p_model,"");
    host.p_fpops = 0;
    host.p_iops = 0;
    host.p_membw = 0;
    host.p_calculated = 0;
    
    strcpy(host.os_name,"");
    strcpy(host.os_version,"");

    host.m_nbytes = 0;
    host.m_cache = 0;
    host.m_swap = 0;

    host.d_total = 0;
    host.d_free = 0;
}

// Parse the host information, usually from the client state XML file
//
int HOST_INFO::parse(FILE* in) {
    char buf[256];

    memset(this, 0, sizeof(HOST_INFO));
    while (fgets(buf, 256, in)) {
        if (match_tag(buf, "</host_info>")) return 0;
        else if (parse_int(buf, "<timezone>", timezone)) continue;
        else if (parse_str(buf, "<domain_name>", domain_name, sizeof(domain_name))) continue;
        else if (parse_str(buf, "<ip_addr>", ip_addr, sizeof(ip_addr))) continue;
        else if (parse_int(buf, "<p_ncpus>", p_ncpus)) continue;
        else if (parse_str(buf, "<p_vendor>", p_vendor, sizeof(p_vendor))) continue;
        else if (parse_str(buf, "<p_model>", p_model, sizeof(p_model))) continue;
        else if (parse_double(buf, "<p_fpops>", p_fpops)) continue;
        else if (parse_double(buf, "<p_iops>", p_iops)) continue;
        else if (parse_double(buf, "<p_membw>", p_membw)) continue;
	else if (parse_double(buf, "<p_calculated>", p_calculated)) continue;
        else if (parse_str(buf, "<os_name>", os_name, sizeof(os_name))) continue;
        else if (parse_str(buf, "<os_version>", os_version, sizeof(os_version))) continue;
        else if (parse_double(buf, "<m_nbytes>", m_nbytes)) continue;
        else if (parse_double(buf, "<m_cache>", m_cache)) continue;
        else if (parse_double(buf, "<m_swap>", m_swap)) continue;
        else if (parse_double(buf, "<d_total>", d_total)) continue;
        else if (parse_double(buf, "<d_free>", d_free)) continue;
        else fprintf(stderr, "HOST_INFO::parse(): unrecognized: %s\n", buf);
    }
    return 0;
}

// Write the host information, usually to the client state XML file
//
int HOST_INFO::write(FILE* out) {
    fprintf(out,
        "<host_info>\n"
        "    <timezone>%d</timezone>\n"
        "    <domain_name>%s</domain_name>\n"
        "    <ip_addr>%s</ip_addr>\n"
        "    <p_ncpus>%d</p_ncpus>\n"
        "    <p_vendor>%s</p_vendor>\n"
        "    <p_model>%s</p_model>\n"
        "    <p_fpops>%f</p_fpops>\n"
        "    <p_iops>%f</p_iops>\n"
        "    <p_membw>%f</p_membw>\n"
	"    <p_calculated>%f</p_calculated>\n"
        "    <os_name>%s</os_name>\n"
        "    <os_version>%s</os_version>\n"
        "    <m_nbytes>%f</m_nbytes>\n"
        "    <m_cache>%f</m_cache>\n"
        "    <m_swap>%f</m_swap>\n"
        "    <d_total>%f</d_total>\n"
        "    <d_free>%f</d_free>\n"
        "</host_info>\n",
        timezone,
        domain_name,
        ip_addr,
        p_ncpus,
        p_vendor,
        p_model,
        p_fpops,
        p_iops,
        p_membw,
	p_calculated, 
        os_name,
        os_version,
        m_nbytes,
        m_cache,
        m_swap,
        d_total,
        d_free
    );
    return 0;
}


// Returns the domain of the local host
// TODO: Should the 256 be MAXHOSTNAMELEN instead?
//
int get_local_domain_name(char* p) {
    char buf[256];

    gethostname(buf, 256);
    struct hostent* he = gethostbyname(buf);
	if (!he) return -1;
    strcpy(p, he->h_name);
    return 0;
}

// Returns the name of the local host
// TODO: Should the 256 be MAXHOSTNAMELEN instead?
//
int get_local_ip_addr_str(char* p) {
    strcpy( p,"" );
#if HAVE_NETDB_H
    char buf[256];
    struct in_addr addr;
    gethostname(buf, 256);
    struct hostent* he = gethostbyname(buf);
    memcpy(&addr, he->h_addr_list[0], sizeof(addr));
    strcpy(p, inet_ntoa(addr));
#endif
    return 0;
}

// Gets the ip address of the local host
//
int get_local_ip_addr(int& p) {
    p = 0;

#if HAVE_NETDB_H
    char buf[256];
    struct in_addr addr;
    gethostname(buf, 256);
    struct hostent* he = gethostbyname(buf);
    memcpy(&addr, he->h_addr_list[0], sizeof(addr));
    p = addr.s_addr;
#endif
    return 0;
}
