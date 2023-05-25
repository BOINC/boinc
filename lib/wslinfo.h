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

#ifndef BOINC_WSLINFO_H
#define BOINC_WSLINFO_H

#include <string>

#include "miofile.h"
#include "parse.h"

struct WSL {
    std::string distro_name;
    std::string name;
    std::string version;
    bool is_default;

    WSL();

    void clear();

    void write_xml(MIOFILE&);
    int parse(XML_PARSER&);
};

struct WSLS {
    std::vector<WSL> wsls;

    WSLS();

    void clear();

    void write_xml(MIOFILE&);
    int parse(XML_PARSER&);
};

#endif
