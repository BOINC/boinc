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
#include "message.h"
#include "error_numbers.h"

#include "hostinfo.h"

// Reset the host info struct to default values
//
void HOST_INFO::clear_host_info() {
    timezone = 0;        // seconds added to local time to get UTC
    strcpy(domain_name, "");
    strcpy(serialnum, "");
    strcpy(ip_addr, "");

    p_ncpus = 0;
    strcpy(p_vendor, "");
    strcpy(p_model, "");
    p_fpops = 0;
    p_iops = 0;
    p_membw = 0;
    p_fpop_err = 0;
    p_iop_err = 0;
    p_membw_err = 0;
    p_calculated = 0;

    strcpy(os_name, "");
    strcpy(os_version, "");

    m_nbytes = 0;
    m_cache = 0;
    m_swap = 0;

    d_total = 0;
    d_free = 0;
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
        else if (parse_double(buf, "<p_fpops>", p_fpops)) {
            // fix foolishness that could result in negative value here
            //
            if (p_fpops < 0) p_fpops = -p_fpops;
            continue;
        }
        else if (parse_double(buf, "<p_iops>", p_iops)) {
            if (p_iops < 0) p_iops = -p_iops;
            continue;
        }
        else if (parse_double(buf, "<p_membw>", p_membw)) {
            if (p_membw < 0) p_membw = -p_membw;
            continue;
        }
        else if (parse_int(buf, "<p_fpop_err>", p_fpop_err)) continue;
        else if (parse_int(buf, "<p_iop_err>", p_iop_err)) continue;
        else if (parse_int(buf, "<p_membw_err>", p_membw_err)) continue;
        else if (parse_double(buf, "<p_calculated>", p_calculated)) continue;
        else if (parse_str(buf, "<os_name>", os_name, sizeof(os_name))) continue;
        else if (parse_str(buf, "<os_version>", os_version, sizeof(os_version))) continue;
        else if (parse_double(buf, "<m_nbytes>", m_nbytes)) continue;
        else if (parse_double(buf, "<m_cache>", m_cache)) continue;
        else if (parse_double(buf, "<m_swap>", m_swap)) continue;
        else if (parse_double(buf, "<d_total>", d_total)) continue;
        else if (parse_double(buf, "<d_free>", d_free)) continue;
        else msg_printf(NULL, MSG_ERROR, "HOST_INFO::parse(): unrecognized: %s\n", buf);
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
        "    <p_fpop_err>%d</p_fpop_err>\n"
        "    <p_iop_err>%d</p_iop_err>\n"
        "    <p_membw_err>%d</p_membw_err>\n"
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
        p_fpop_err,
        p_iop_err,
        p_membw_err,
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

// CPU benchmarks are run in a separate process,
// which communicates its result via a file.
// The following functions read and write this file.
//
int HOST_INFO::parse_cpu_benchmarks(FILE* in) {
    char buf[256];

    fgets(buf, 256, in);
    while (fgets(buf, 256, in)) {
        if (match_tag(buf, "<cpu_benchmarks>"));
        else if (match_tag(buf, "</cpu_benchmarks>")) return 0;
        else if (parse_double(buf, "<p_fpops>", p_fpops)) continue;
        else if (parse_double(buf, "<p_iops>", p_iops)) continue;
        else if (parse_double(buf, "<p_membw>", p_membw)) continue;
        else if (parse_int(buf, "<p_fpop_err>", p_fpop_err)) continue;
        else if (parse_int(buf, "<p_iop_err>", p_iop_err)) continue;
        else if (parse_int(buf, "<p_membw_err>", p_membw_err)) continue;
        else if (parse_double(buf, "<p_calculated>", p_calculated)) continue;
        else if (parse_double(buf, "<m_cache>", m_cache)) continue;
        else msg_printf(NULL, MSG_ERROR, "HOST_INFO::parse(): unrecognized: %s\n", buf);
    }
    return 0;
}

int HOST_INFO::write_cpu_benchmarks(FILE* out) {
    fprintf(out,
        "<cpu_benchmarks>\n"
        "    <p_fpops>%f</p_fpops>\n"
        "    <p_iops>%f</p_iops>\n"
        "    <p_membw>%f</p_membw>\n"
        "    <p_fpop_err>%d</p_fpop_err>\n"
        "    <p_iop_err>%d</p_iop_err>\n"
        "    <p_membw_err>%d</p_membw_err>\n"
        "    <p_calculated>%f</p_calculated>\n"
        "    <m_cache>%f</m_cache>\n"
        "</cpu_benchmarks>\n",
        p_fpops,
        p_iops,
        p_membw,
        p_fpop_err,
        p_iop_err,
        p_membw_err,
        p_calculated,
        m_cache
    );
    return 0;
}

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
