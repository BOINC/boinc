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

#include "parse.h"
#include "hostinfo.h"

int HOST_INFO::parse(FILE* in) {
    char buf[256];
    
    memset(this, 0, sizeof(HOST_INFO));
    while (fgets(buf, 256, in)) {
        if (match_tag(buf, "</host_info>")) return 0;
        else if (parse_int(buf, "<timezone>", timezone)) continue;
        else if (parse_str(buf, "<domain_name>", domain_name)) continue;
        else if (parse_str(buf, "<ip_addr>", ip_addr)) continue;
        else if (parse_int(buf, "<p_ncpus>", p_ncpus)) continue;
        else if (parse_str(buf, "<p_vendor>", p_vendor)) continue;
        else if (parse_str(buf, "<p_model>", p_model)) continue;
        else if (parse_double(buf, "<p_fpops>", p_fpops)) continue;
        else if (parse_double(buf, "<p_iops>", p_iops)) continue;
        else if (parse_double(buf, "<p_membw>", p_membw)) continue;
        else if (parse_str(buf, "<os_name>", os_name)) continue;
        else if (parse_str(buf, "<os_version>", os_version)) continue;
        else if (parse_double(buf, "<m_nbytes>", m_nbytes)) continue;
        else if (parse_double(buf, "<m_cache>", m_cache)) continue;
        else if (parse_double(buf, "<m_swap>", m_swap)) continue;
        else if (parse_double(buf, "<d_total>", d_total)) continue;
        else if (parse_double(buf, "<d_free>", d_free)) continue;
        else fprintf(stderr, "HOST_INFO::parse(): unrecognized: %s\n", buf);
    }
    return 0;
}

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
