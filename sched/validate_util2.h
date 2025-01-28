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

#ifndef BOINC_VALIDATE_UTIL2_H
#define BOINC_VALIDATE_UTIL2_H

#include <vector>

#include "boinc_db_types.h"

// return values of init_result() and compare_result()
// (in addition to usual error codes)
//
#define VAL_RESULT_SUSPICIOUS       1
    // the result looks 'suspicious'.
    // if we're using adaptive replication and this is the only copy,
    // create a 2nd replica rather than accepting it.
    // (Are any projects using this?)
#define VAL_RESULT_LONG_TERM_FAIL   2
    // host is unlikely to handle this app version; stop using it
#define VAL_RESULT_TRANSIENT_ERROR  3
    // a transient error happened (e.g. couldn't read file due to NFS failure).
    // Retry validation later.
    // ERR_OPENDIR is also treated this way.

extern int init_result(RESULT&, void*&);
extern int compare_results(RESULT &, void*, RESULT const&, void*, bool&);
extern int cleanup_result(RESULT const&, void*);

// old/internal interface:
//
extern double compute_granted_credit(WORKUNIT&, std::vector<RESULT>& results);
extern int check_set(
    std::vector<RESULT>& results, WORKUNIT& wu,
    DB_ID_TYPE& canonicalid, double& credit_deprecated, bool& retry
);
extern void check_pair(RESULT& r1, RESULT& r2, bool& retry);

#endif
