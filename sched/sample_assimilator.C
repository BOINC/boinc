// Berkeley Open Infrastructure for Network Computing
// http://boinc.berkeley.edu
// Copyright (C) 2007 University of California
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

// A sample assimilator that:
// 1) if success, copy the output file to a directory
// 2) if failure, append a message to an error log

#include <vector>
#include <string>

#include "boinc_db.h"
#include "error_numbers.h"
#include "filesys.h"
#include "sched_msgs.h"
#include "validate_util.h"
#include "sched_config.h"

using std::vector;
using std::string;

FILE* f = 0;
int assimilate_handler(
    WORKUNIT& wu, vector<RESULT>& /*results*/, RESULT& canonical_result
) {
    int retval;

    retval = boinc_mkdir("../sample_results");
    if (retval) return retval;

    if (wu.canonical_resultid) {
        string output_file_path;
        char copy_path[256];
        get_output_file_path(canonical_result, output_file_path);
        sprintf(copy_path, "../sample_results/%s", wu.name);
        retval = boinc_copy(output_file_path.c_str(), copy_path);
        if (retval) return retval;
    } else {
        if (!f) {
            f = fopen("../sample_results/errors", "a");
            if (!f) return ERR_FOPEN;
        }
        fprintf(f, "%s: 0x%x\n", wu.name, wu.error_mask);
        fflush(f);
    }
    return 0;
}
