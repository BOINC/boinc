// The contents of this file are subject to the BOINC Public License
// Version 1.0 (the "License"); you may not use this file except in
// compliance with the License. You may obtain a copy of the License at
// http://boinc.berkeley.edu/license_1.0.txt
//
// Software distributed under the License is distributed on an "AS IS"
// basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the
// License for the specific language governing rights and limitations
// under the License.
//
// The Original Code is the Berkeley Open Infrastructure for Network Computing.
//
// The Initial Developer of the Original Code is the SETI@home project.
// Portions created by the SETI@home project are Copyright (C) 2002
// University of California at Berkeley. All Rights Reserved.
//
// Contributor(s):
//

#ifndef H_VALIDATE_UTIL
#define H_VALIDATE_UTIL

#include "boinc_db.h"
#include <vector>
#include <string>

typedef int (*init_result_f)(RESULT const&, void*&);
typedef int (*check_pair_with_data_f)(RESULT const&, void*, RESULT const&, void*, bool&);
typedef int (*cleanup_result_f)(RESULT const&, void*);
extern int get_output_file_path(RESULT const& result, std::string& path);
extern double median_mean_credit(std::vector<RESULT> const& results);

int generic_check_set_majority(
    std::vector<RESULT>& results, int& canonicalid, double& credit,
    init_result_f init_result_f,
    check_pair_with_data_f check_pair_with_data_f,
    cleanup_result_f cleanup_result_f
);
int generic_check_pair(
    RESULT const& r1, RESULT const& r2, bool& match,
    init_result_f init_result_f,
    check_pair_with_data_f check_pair_with_data_f,
    cleanup_result_f cleanup_result_f
);

#endif
