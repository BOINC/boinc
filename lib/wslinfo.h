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

// describes a WSL (Windows Subsystem for Linux) distro
//
struct WSL_DISTRO {
    std::string distro_name;
        // name (unique) of distro
    std::string os_name;
        // name of the operating system
    std::string os_version;
        // version of the operating system
    int wsl_version;
        // version of WSL (currently 1 or 2)
    bool is_default;
        // this is the default distro
    bool is_docker_available;
        // Docker is present and allowed by config
    bool is_docker_compose_available;
        // Docker Compose is present and allowed by config
    std::string docker_version;
        // version of Docker
    std::string docker_compose_version;
        // version of Docker Compose

    WSL_DISTRO(){
        clear();
    };
    void clear();
    void write_xml(MIOFILE&);
    int parse(XML_PARSER&);
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
        const char *os_name_regexp, const char *os_version_regexp
    );
};

#endif
