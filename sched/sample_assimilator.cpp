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

// A sample assimilator that:
// 1) if success, copy the output file(s) to a directory
// 2) if failure, append a message to an error log

#include <vector>
#include <string>
#include <cstdlib>

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
        f = fopen(config.project_path("sample_results/errors"), "a");
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

    retval = boinc_mkdir(config.project_path("sample_results"));
    if (retval) return retval;

    if (wu.canonical_resultid) {
        vector<OUTPUT_FILE_INFO> output_files;
        const char *copy_path;
        get_output_file_infos(canonical_result, output_files);
        unsigned int n = output_files.size();
        bool file_copied = false;
        for (i=0; i<n; i++) {
            OUTPUT_FILE_INFO& fi = output_files[i];
            if (n==1) {
                copy_path = config.project_path("sample_results/%s", wu.name);
            } else {
                copy_path = config.project_path("sample_results/%s_%d", wu.name, i);
            }
            retval = boinc_copy(fi.path.c_str() , copy_path);
            if (!retval) {
                file_copied = true;
            }
        }
        if (!file_copied) {
            copy_path = config.project_path(
                "sample_results/%s_%s", wu.name, "no_output_files"
            );
            FILE* f = fopen(copy_path, "w");
            fclose(f);
        }
    } else {
        sprintf(buf, "%s: 0x%x\n", wu.name, wu.error_mask);
        return write_error(buf);
    }
    return 0;
}
