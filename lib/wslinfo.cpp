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

#include "wslinfo.h"

WSL_DISTRO::WSL_DISTRO() {
    clear();
}

void WSL_DISTRO::clear() {
    distro_name = "";
    os_name = "";
    os_version = "";
    is_default = false;
    wsl_version = 1;
    is_docker_available = false;
    is_docker_compose_available = false;
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
        "            <is_default>%d</is_default>\n"
        "            <wsl_version>%d</wsl_version>\n"
        "            <is_docker_available>%d</is_docker_available>\n"
        "            <is_docker_compose_available>%d</is_docker_compose_available>\n"
        "            <docker_version>%s</docker_version>\n"
        "            <docker_compose_version>%s</docker_compose_version>\n"
        "        </distro>\n",
        dn,
        n,
        v,
        is_default ? 1 : 0,
        wsl_version,
        is_docker_available ? 1 : 0,
        is_docker_compose_available ? 1 : 0,
        docker_version.c_str(),
        docker_compose_version.c_str()
    );
}

int WSL_DISTRO::parse(XML_PARSER& xp) {
    clear();
    while (!xp.get_tag()) {
        if (xp.match_tag("/distro")) {
            return 0;
        }
        if (xp.parse_string("distro_name", distro_name)) continue;
        if (xp.parse_string("os_name", os_name)) continue;
        if (xp.parse_string("os_version", os_version)) continue;
        if (xp.parse_bool("is_default", is_default)) continue;
        if (xp.parse_int("wsl_version", wsl_version)) continue;
        if (xp.parse_bool("is_docker_available", is_docker_available)) continue;
        if (xp.parse_bool("is_docker_compose_available", is_docker_compose_available)) continue;
        if (xp.parse_string("docker_version", docker_version)) continue;
        if (xp.parse_string("docker_compose_version", docker_compose_version)) continue;
    }
    return ERR_XML_PARSE;
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

#ifdef _WIN64
typedef HRESULT(WINAPI *PWslLaunch)(
    PCWSTR, PCWSTR, BOOL, HANDLE, HANDLE, HANDLE, HANDLE*
);

static PWslLaunch pWslLaunch = NULL;
static HINSTANCE wsl_lib = NULL;

int WSL_CMD::setup() {
    in_read = NULL;
    in_write = NULL;
    out_read = NULL;
    out_write = NULL;

    if (!pWslLaunch) {
        wsl_lib = LoadLibrary("wslapi.dll");
        if (!wsl_lib) return -`1;
        pWslLaunch = (PWslLaunch)GetProcAddress(wsl_lib, "WslLaunch");
        if (!pWslLaunch) return -1;
    }

    SECURITY_ATTRIBUTES sa;
    sa.nLength = sizeof(SECURITY_ATTRIBUTES);
    sa.bInheritHandle = TRUE;
    sa.lpSecurityDescriptor = NULL;

    if (!CreatePipe(&out_read, &out_write, &sa, 0)) return -1;
    if (!SetHandleInformation(out_read, HANDLE_FLAG_INHERIT, 0)) return -1;
    if (!CreatePipe(&in_read, &in_write, &sa, 0)) return -1;
    if (!SetHandleInformation(in_write, HANDLE_FLAG_INHERIT, 0)) return -1;
    return 0;
}

int WSL_CMD::run_command(
    const std::string distro_name, const std::string command,
    HANDLE* proc_handle, bool use_cwd = false
) {
    HRESULT ret = rs.pWslLaunch(
        boinc_ascii_to_wide(distro_name).c_str(),
        boinc_ascii_to_wide(command).c_str(),
        use_cwd, in_read, out_write, out_write,
        handle
    );
    return (ret == S_OK)?0:-1;
}
#endif // _WIN64
