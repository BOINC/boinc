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

#ifndef BOINC_AUTO_UPDATE_H
#define BOINC_AUTO_UPDATE_H

#include <string>
#include <vector>
#include "common_defs.h"
#include "client_types.h"

using std::string;
using std::vector;

class AUTO_UPDATE {
public:
    bool present;
    bool install_failed;
    VERSION_INFO version;
    vector<FILE_REF> file_refs;
    PROJECT* project;

    AUTO_UPDATE();
    void init();
    int parse(MIOFILE&);
    void write(MIOFILE&);
    int validate_and_link(PROJECT*);
    void install();
    void poll();
};

#endif

