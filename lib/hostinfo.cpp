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

#if   defined(_WIN32) && !defined(__STDWX_H__)
#include "boinc_win.h"
#elif defined(_WIN32) && defined(__STDWX_H__)
#include "stdwx.h"
#else
#include "config.h"
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
    strcpy(p_features, "");
    p_fpops = 0;
    p_iops = 0;
    p_membw = 0;
    p_calculated = 0;

    m_nbytes = 0;
    m_cache = 0;
    m_swap = 0;

    d_total = 0;
    d_free = 0;

    strcpy(os_name, "");
    strcpy(os_version, "");
}

int HOST_INFO::parse(MIOFILE& in, bool benchmarks_only) {
    char buf[1024];

    while (in.fgets(buf, sizeof(buf))) {
        if (match_tag(buf, "</host_info>")) return 0;
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

        if (benchmarks_only) continue;

        if (parse_int(buf, "<timezone>", timezone)) continue;
        else if (parse_str(buf, "<domain_name>", domain_name, sizeof(domain_name))) continue;
        else if (parse_str(buf, "<ip_addr>", ip_addr, sizeof(ip_addr))) continue;
        else if (parse_str(buf, "<host_cpid>", host_cpid, sizeof(host_cpid))) continue;
        else if (parse_int(buf, "<p_ncpus>", p_ncpus)) continue;
        else if (parse_str(buf, "<p_vendor>", p_vendor, sizeof(p_vendor))) continue;
        else if (parse_str(buf, "<p_model>", p_model, sizeof(p_model))) continue;
        else if (parse_str(buf, "<p_features>", p_features, sizeof(p_features))) continue;
        else if (parse_double(buf, "<m_nbytes>", m_nbytes)) continue;
        else if (parse_double(buf, "<m_cache>", m_cache)) continue;
        else if (parse_double(buf, "<m_swap>", m_swap)) continue;
        else if (parse_double(buf, "<d_total>", d_total)) continue;
        else if (parse_double(buf, "<d_free>", d_free)) continue;
        else if (parse_str(buf, "<os_name>", os_name, sizeof(os_name))) continue;
        else if (parse_str(buf, "<os_version>", os_version, sizeof(os_version))) continue;
        else if (match_tag(buf, "<coprocs>")) {
            coprocs.parse(in);
        }
    }
    return ERR_XML_PARSE;
}

// Write the host information, to the client state XML file
// or in a scheduler request message
//
int HOST_INFO::write(
    MIOFILE& out, bool suppress_net_info, bool include_coprocs
) {
    char pv[265], pm[256], pf[256], osn[256], osv[256];
    out.printf(
        "<host_info>\n"
        "    <timezone>%d</timezone>\n",
        timezone
    );
    if (!suppress_net_info) {
        out.printf(
            "    <domain_name>%s</domain_name>\n"
            "    <ip_addr>%s</ip_addr>\n",
            domain_name,
            ip_addr
        );
    }
    xml_escape(p_vendor, pv, sizeof(pv));
    xml_escape(p_model, pm, sizeof(pm));
    xml_escape(p_features, pf, sizeof(pf));
    xml_escape(os_name, osn, sizeof(osn));
    xml_escape(os_version, osv, sizeof(osv));
    out.printf(
        "    <host_cpid>%s</host_cpid>\n"
        "    <p_ncpus>%d</p_ncpus>\n"
        "    <p_vendor>%s</p_vendor>\n"
        "    <p_model>%s</p_model>\n"
        "    <p_features>%s</p_features>\n"
        "    <p_fpops>%f</p_fpops>\n"
        "    <p_iops>%f</p_iops>\n"
        "    <p_membw>%f</p_membw>\n"
        "    <p_calculated>%f</p_calculated>\n"
        "    <m_nbytes>%f</m_nbytes>\n"
        "    <m_cache>%f</m_cache>\n"
        "    <m_swap>%f</m_swap>\n"
        "    <d_total>%f</d_total>\n"
        "    <d_free>%f</d_free>\n"
        "    <os_name>%s</os_name>\n"
        "    <os_version>%s</os_version>\n",
        host_cpid,
        p_ncpus,
        pv,
        pm,
        pf,
        p_fpops,
        p_iops,
        p_membw,
        p_calculated,
        m_nbytes,
        m_cache,
        m_swap,
        d_total,
        d_free,
        osn,
        osv
    );
    if (include_coprocs) {
        coprocs.write_xml(out);
    }
    out.printf(
        "</host_info>\n"
    );
    return 0;
}

// CPU benchmarks are run in a separate process,
// which communicates its result via a file.
// The following functions read and write this file.
//
int HOST_INFO::parse_cpu_benchmarks(FILE* in) {
    char buf[256];

    char* p = fgets(buf, 256, in);
    if (!p) return 0;           // Fixes compiler warning
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

