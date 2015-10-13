// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2015 University of California
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

// Include this in assimilate handlers

#include <vector>
#include "boinc_db.h"
#define DEFER_ASSIMILATION 123321
    // if the assimilate handler returns this,
    // don't mark WU as assimilated; instead, wait for another
    // result to be returned and try again
    // (kludge for WCG)

// This function is called to handle a completed job.
// Return zero on success.
//
extern int assimilate_handler(
    WORKUNIT&,              // the workunit
    std::vector<RESULT>&,   // all the instances (successful or not)
    RESULT&                 // the canonical instance
);

extern int assimilate_handler_init(int argc, char** argv);
extern void assimilate_handler_usage();
