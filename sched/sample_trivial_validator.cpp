// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2019 University of California
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

// A sample validator that accepts all results

#include "validate_util2.h"

int validate_handler_init(int, char**) {
    return 0;
}

void validate_handler_usage() {
    // describe the project specific arguments here
    //fprintf(stderr,
    //    "    Custom options:\n"
    //    "    [--project_option X]  a project specific option\n"
    //);
}


int init_result(RESULT&, void*&) {
    return 0;
}

int compare_results(RESULT&, void*, RESULT const&, void*, bool& match) {
    match = true;
    return 0;
}

int cleanup_result(RESULT const&, void*) {
    return 0;
}
