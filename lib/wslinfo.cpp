// This file is part of BOINC.
// https://boinc.berkeley.edu
// Copyright (C) 2024 University of California
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

// write and parse WSL_DISTRO structs,
// which describe WSL distros and their possible Docker contents

#include <regex>

#include "common_defs.h"
#include "wslinfo.h"

void WSL_DISTRO::clear() {
    distro_name = "";
    os_name = "";
    os_version = "";
    libc_version = "";
    disallowed = false;
    is_default = false;
    wsl_version = 0;
    docker_version = "";
    docker_compose_version = "";
}

void WSL_DISTRO::write_xml(MIOFILE& f) {
    char dn[256], n[256], v[256];
    xml_escape(distro_name.c_str(), dn, sizeof(dn));
    xml_escape(os_name.c_str(), n, sizeof(n));
    xml_escape(os_version.c_str(), v, sizeof(v));
    f.printf(
        "        <distro>\n"
        "            <distro_name>%s</distro_name>\n"
        "            <os_name>%s</os_name>\n"
        "            <os_version>%s</os_version>\n"
        "            <wsl_version>%d</wsl_version>\n",
        dn,
        n,
        v,
        wsl_version
    );
    if (is_default) {
        f.printf(
            "            <is_default/>\n"
        );
    }
    if (disallowed) {
        f.printf(
            "            <disallowed/>\n"
        );
    }
    if (!libc_version.empty()) {
        f.printf(
            "           <libc_version>%s</libc_version>\n",
            libc_version.c_str()
        );
    }
    if (!docker_version.empty()) {
        f.printf(
            "            <docker_version>%s</docker_version>\n"
            "            <docker_type>%d</docker_type>\n",
            docker_version.c_str(),
            docker_type
        );
    }
    if (!docker_compose_version.empty()) {
        f.printf(
            "            <docker_compose_version>%s</docker_compose_version>\n"
            "            <docker_compose_type>%d</docker_compose_type>\n",
            docker_compose_version.c_str(),
            docker_compose_type
        );
    }
    f.printf(
        "        </distro>\n"
    );
}

int WSL_DISTRO::parse(XML_PARSER& xp) {
    int i;
    clear();
    while (!xp.get_tag()) {
        if (xp.match_tag("/distro")) {
            return 0;
        }
        if (xp.parse_string("distro_name", distro_name)) continue;
        if (xp.parse_string("os_name", os_name)) continue;
        if (xp.parse_string("os_version", os_version)) continue;
        if (xp.parse_string("libc_version", libc_version)) continue;
        if (xp.parse_bool("is_default", is_default)) continue;
        if (xp.parse_bool("disallowed", disallowed)) continue;
        if (xp.parse_int("wsl_version", wsl_version)) continue;
        if (xp.parse_string("docker_version", docker_version)) continue;
        if (xp.parse_int("docker_type", i)) {
            docker_type = (DOCKER_TYPE) i;
            continue;
        }
        if (xp.parse_string("docker_compose_version", docker_compose_version)) continue;
        if (xp.parse_int("docker_compose_type", i)) {
            docker_compose_type = (DOCKER_TYPE) i;
            continue;
        }
    }
    return ERR_XML_PARSE;
}

int WSL_DISTRO::libc_version_int() {
    int maj, min;
    int n = sscanf(libc_version.c_str(), "%d.%d", &maj, &min);
    if (n==2) return maj*100+min;
    return 0;
}

WSL_DISTROS::WSL_DISTROS() {
    clear();
}

void WSL_DISTROS::clear() {
    distros.clear();
}

void WSL_DISTROS::write_xml(MIOFILE& f) {
    f.printf("    <wsl>\n");
    for (WSL_DISTRO &wd: distros) {
        wd.write_xml(f);
    }
    f.printf("    </wsl>\n");
}

int WSL_DISTROS::parse(XML_PARSER& xp) {
    clear();
    while (!xp.get_tag()) {
        if (xp.match_tag("/wsl")) {
            return 0;
        }
        if (xp.match_tag("distro"))
        {
            WSL_DISTRO wd;
            wd.parse(xp);
            distros.push_back(wd);
        }
    }
    return ERR_XML_PARSE;
}

WSL_DISTRO* WSL_DISTROS::find_match(
    const char *os_name_regexp, const char *os_version_regexp,
    int min_libc_version
) {
    std::regex name_regex(os_name_regexp), version_regex(os_version_regexp);
    for (WSL_DISTRO &wd: distros) {
        if (!std::regex_match(wd.os_name.c_str(), name_regex)) {
            continue;
        }
        if (!std::regex_match(wd.os_version.c_str(), version_regex)) {
            continue;
        }
        if (wd.libc_version_int() < min_libc_version) {
            continue;
        }
        return &wd;
    }
    return NULL;
}

WSL_DISTRO* WSL_DISTROS::find_docker() {
    for (WSL_DISTRO &wd: distros) {
        if (!wd.docker_version.empty()) {
            return &wd;
        }
    }
    return NULL;
}
