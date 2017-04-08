// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2011 University of California
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

#ifndef BOINC_PROCESS_INPUT_TEMPLATE_H
#define BOINC_PROCESS_INPUT_TEMPLATE_H

#include <vector>

#include "boinc_db.h"
#include "sched_config.h"
#include "backend_lib.h"

extern int process_input_template(
    WORKUNIT& wu,
    char* tmplate,
    std::vector<INFILE_DESC> &infiles,
    SCHED_CONFIG& config_loc,
    const char* command_line,
    const char* additional_xml
);

#endif
