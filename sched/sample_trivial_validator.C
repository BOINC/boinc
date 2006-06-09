// Berkeley Open Infrastructure for Network Computing
// http://boinc.berkeley.edu
// Copyright (C) 2005 University of California
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
// 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

// A sample validator that grants credit to any result whose CPU time is above
// a certain minimum

#include "config.h"
#include "validate_util.h"

using std::vector;

static const double MIN_CPU_TIME = 0;

int init_result(RESULT const& /*result*/, void*& /*data*/) {
    return 0;
}

int compare_results(
    RESULT & r1, void* /*data1*/,
    RESULT const& r2, void* /*data2*/,
    bool& match
) {
    match = (r1.cpu_time >= MIN_CPU_TIME && r2.cpu_time >= MIN_CPU_TIME);
    return 0;
}

int cleanup_result(RESULT const&, void*) {
    return 0;
}

const char *BOINC_RCSID_f3a7a34795 = "$Id$";
