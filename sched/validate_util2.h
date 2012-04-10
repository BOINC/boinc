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

#ifndef _VALIDATE_UTIL2_
#define _VALIDATE_UTIL2_

#include <vector>

#include "boinc_db_types.h"

extern int init_result(RESULT&, void*&);
extern int compare_results(RESULT &, void*, RESULT const&, void*, bool&);
extern int cleanup_result(RESULT const&, void*);
extern double compute_granted_credit(WORKUNIT&, std::vector<RESULT>& results);
extern int check_set(
    std::vector<RESULT>& results, WORKUNIT& wu,
    int& canonicalid, double& credit_deprecated, bool& retry
);
extern void check_pair(RESULT& r1, RESULT& r2, bool& retry);
#endif
