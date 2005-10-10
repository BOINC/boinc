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

#if defined(_WIN32) && !defined(__STDWX_H__) && !defined(_BOINC_WIN_) && !defined(_AFX_STDAFX_H_)
#include "boinc_win.h"
#endif

#ifndef _WIN32
#include <cstdio>
#include <cstring>

#if HAVE_UNISTD_H
#include <unistd.h>
#endif
#endif

#include "util.h"
#include "parse.h"
#include "md5_file.h"
#include "error_numbers.h"

#include "hostinfo.h"

HOST_INFO::HOST_INFO() {
    clear_host_info();
}

void HOST_INFO::clear_host_info() {
    timezone = 0;
    strcpy(domain_name, "");
    strcpy(serialnum, "");
    strcpy(ip_addr, "");
    strcpy(host_cpid, "");

    p_ncpus = 0;
    strcpy(p_vendor, "");
    strcpy(p_model, "");
    p_fpops = 0;
    p_iops = 0;
    p_membw = 0;
    p_calculated = 0;

    strcpy(os_name, "");
    strcpy(os_version, "");

    m_nbytes = 0;
    m_cache = 0;
    m_swap = 0;

    d_total = 0;
    d_free = 0;
}

int HOST_INFO::parse(MIOFILE& in) {
    char buf[256];

    memset(this, 0, sizeof(HOST_INFO));
    while (in.fgets(buf, 256)) {
        if (match_tag(buf, "</host_info>")) return 0;
        else if (parse_int(buf, "<timezone>", timezone)) continue;
        else if (parse_str(buf, "<domain_name>", domain_name, sizeof(domain_name))) continue;
        else if (parse_str(buf, "<ip_addr>", ip_addr, sizeof(ip_addr))) continue;
        else if (parse_str(buf, "<host_cpid>", host_cpid, sizeof(host_cpid))) continue;
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
        else if (parse_double(buf, "<p_calculated>", p_calculated)) continue;
        else if (parse_str(buf, "<os_name>", os_name, sizeof(os_name))) continue;
        else if (parse_str(buf, "<os_version>", os_version, sizeof(os_version))) continue;
        else if (parse_double(buf, "<m_nbytes>", m_nbytes)) continue;
        else if (parse_double(buf, "<m_cache>", m_cache)) continue;
        else if (parse_double(buf, "<m_swap>", m_swap)) continue;
        else if (parse_double(buf, "<d_total>", d_total)) continue;
        else if (parse_double(buf, "<d_free>", d_free)) continue;
    }
    return ERR_XML_PARSE;
}

// Write the host information, usually to the client state XML file
//
int HOST_INFO::write(MIOFILE& out) {
    out.printf(
        "<host_info>\n"
        "    <timezone>%d</timezone>\n"
        "    <domain_name>%s</domain_name>\n"
        "    <ip_addr>%s</ip_addr>\n"
        "    <host_cpid>%s</host_cpid>\n"
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
        host_cpid,
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
        else if (parse_double(buf, "<p_calculated>", p_calculated)) continue;
        else if (parse_double(buf, "<m_cache>", m_cache)) continue;
    }
    return 0;
}

int HOST_INFO::write_cpu_benchmarks(FILE* out) {
    fprintf(out,
        "<cpu_benchmarks>\n"
        "    <p_fpops>%f</p_fpops>\n"
        "    <p_iops>%f</p_iops>\n"
        "    <p_membw>%f</p_membw>\n"
        "    <p_calculated>%f</p_calculated>\n"
        "    <m_cache>%f</m_cache>\n"
        "</cpu_benchmarks>\n",
        p_fpops,
        p_iops,
        p_membw,
        p_calculated,
        m_cache
    );
    return 0;
}

// make a host cross-project ID.
// Should be unique across hosts with very high probability
//
void HOST_INFO::generate_host_cpid() {
    char buf[1024];

    sprintf(buf, "%f%s%s%f", dtime(), domain_name, ip_addr, d_free);
    md5_block((const unsigned char*) buf, (int)strlen(buf), host_cpid);
}

const char *BOINC_RCSID_edf7e5c147 = "$Id$";
