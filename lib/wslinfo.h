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

// structs describing WSL (Windows Subsystem for Linux) distros.
// Used in Win client and also in server code (for plan class logic)

#ifndef BOINC_WSLINFO_H
#define BOINC_WSLINFO_H

#include <string>

#include "miofile.h"
#include "parse.h"
#include "common_defs.h"

#define BOINC_WSL_DISTRO_NAME "boinc-buda-runner"

// describes a WSL (Windows Subsystem for Linux) distro,
// and its Docker features
//
struct WSL_DISTRO {
    std::string distro_name;
        // name (unique) of distro
    std::string os_name;
        // name of the operating system
    std::string os_version;
        // version of the operating system
    std::string libc_version;
        // version of libc, as reported by ldd --version
    int wsl_version;
        // version of WSL (currently 1 or 2)
    bool is_default;
        // this is the default distro
    bool disallowed;
        // disallowed in cc_config.xml
    std::string docker_version;
        // version of Docker (or podman)
        // empty if not present
    DOCKER_TYPE docker_type;
    std::string docker_compose_version;
        // version of Docker Compose; empty if none
    DOCKER_TYPE docker_compose_type;
    int boinc_buda_runner_version;
        // if this distro is boinc_buda_runner, the version

    WSL_DISTRO(){
        clear();
    };
    void clear();
    void write_xml(MIOFILE&);
    int parse(XML_PARSER&);
    int libc_version_int();
};

// a set of WSL distros
//
struct WSL_DISTROS {
    std::vector<WSL_DISTRO> distros;

    WSL_DISTROS();
    void clear();
    void write_xml(MIOFILE&);
    int parse(XML_PARSER&);
    WSL_DISTRO *find_match(
        const char *os_name_regexp, const char *os_version_regexp,
        int min_libc_version
    );
    WSL_DISTRO *find_docker();
        // find a distro containing Docker
    int boinc_distro_version();
        // version number of BOINC distro, if present, else 0
};

#endif
