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

// Include this in assimilate handlers

#include <vector>
#include "boinc_db.h"
#define DEFER_ASSIMILATION 123321
    // if the assimilate handler returns this,
    // don't mark WU as assimilated; instead, wait for another
    // result to be returned and try again
    // (kludge for WCG)

extern int assimilate_handler(WORKUNIT&, std::vector<RESULT>&, RESULT&);

extern int g_argc;
extern char** g_argv;
