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
// 1) if success, copy the output file(s) to a directory
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

int write_error(char* p) {
    static FILE* f = 0;
    if (!f) {
        f = fopen("../sample_results/errors", "a");
        if (!f) return ERR_FOPEN;
    }
    fprintf(f, "%s", p);
    fflush(f);
    return 0;
}

int assimilate_handler(
    WORKUNIT& wu, vector<RESULT>& /*results*/, RESULT& canonical_result
) {
    int retval;
    char buf[1024];
    unsigned int i;

    retval = boinc_mkdir("../sample_results");
    if (retval) return retval;

    if (wu.canonical_resultid) {
        vector<string> output_file_paths;
        char copy_path[256];
        get_output_file_paths(canonical_result, output_file_paths);
        unsigned int n = output_file_paths.size();
        for (i=0; i<n; i++) {
            string path = output_file_paths[i];
            if (n==1) {
                sprintf(copy_path, "../sample_results/%s", wu.name);
            } else {
                sprintf(copy_path, "../sample_results/%s_%d", wu.name, i);
            }
            retval = boinc_copy(path.c_str() , copy_path);
            if (retval) {
                sprintf(buf, "couldn't copy file %s\n", path.c_str());
                write_error(buf);
                return retval;
            }
        }
    } else {
        sprintf(buf, "%s: 0x%x\n", wu.name, wu.error_mask);
        return write_error(buf);
    }
    return 0;
}
