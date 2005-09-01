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

#ifndef H_VALIDATE_UTIL
#define H_VALIDATE_UTIL

#include "boinc_db.h"
#include <vector>
#include <string>

typedef int (*init_result_f)(RESULT const&, void*&);
typedef int (*check_pair_with_data_f)(RESULT &, void*, RESULT const&, void*, bool&);
typedef int (*cleanup_result_f)(RESULT const&, void*);
extern int get_output_file_path(RESULT const& result, std::string& path);
extern double median_mean_credit(std::vector<RESULT>& results);

extern int generic_check_set(
    std::vector<RESULT>& results, int& canonicalid, double& credit,
    init_result_f init_result_f,
    check_pair_with_data_f check_pair_with_data_f,
    cleanup_result_f cleanup_result_f,
    int min_valid
);

extern int generic_check_pair(
    RESULT & r1, RESULT const& r2,
    init_result_f init_result_f,
    check_pair_with_data_f check_pair_with_data_f,
    cleanup_result_f cleanup_result_f
);

#endif
