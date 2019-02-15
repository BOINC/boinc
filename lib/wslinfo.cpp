// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2018 University of California
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

#include "wslinfo.h"

WSL::WSL() {
    clear();
}

void WSL::clear() {
    distro_name = "";
    name = "";
    version = "";
    is_default = false;
}

void WSL::write_xml(MIOFILE& f) {
    char dn[256], n[256], v[256];
    xml_escape(distro_name.c_str(), dn, sizeof(dn));
    xml_escape(name.c_str(), n, sizeof(n));
    xml_escape(version.c_str(), v, sizeof(v));
    f.printf(
        "        <distro>\n"
        "            <distro_name>%s</distro_name>\n"
        "            <name>%s</name>\n"
        "            <version>%s</version>\n"
        "            <is_default>%d</is_default>\n"
        "        </distro>\n",
        dn,
        n,
        v,
        is_default ? 1 : 0
    );
}

int WSL::parse(XML_PARSER& xp) {
    clear();
    while (!xp.get_tag()) {
        if (xp.match_tag("/distro")) {
            return 0;
        }
        if (xp.parse_string("distro_name", distro_name)) continue;
        if (xp.parse_string("name", name)) continue;
        if (xp.parse_string("version", version)) continue;
        if (xp.parse_bool("is_default", is_default)) continue;
    }
    return ERR_XML_PARSE;
}

WSLS::WSLS() {
    clear();
}

void WSLS::clear() {
    wsls.clear();
}

void WSLS::write_xml(MIOFILE& f) {
    f.printf("    <wsl>\n");
    for (size_t i = 0; i < wsls.size(); ++i) {
        wsls[i].write_xml(f);
    }
    f.printf("    </wsl>\n");
}

int WSLS::parse(XML_PARSER& xp) {
    clear();
    while (!xp.get_tag()) {
        if (xp.match_tag("/wsl")) {
            return 0;
        }
        if (xp.match_tag("distro"))
        {
            WSL wsl;
            wsl.parse(xp);
            wsls.push_back(wsl);
        }
    }
    return ERR_XML_PARSE;
}
