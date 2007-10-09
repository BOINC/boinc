// Berkeley Open Infrastructure for Network Computing
// http://boinc.berkeley.edu
// Copyright (C) 2006 University of California
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
// 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

#ifndef _AUTO_UPDATE_
#define _AUTO_UPDATE_

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

