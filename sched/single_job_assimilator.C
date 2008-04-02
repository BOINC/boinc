// Berkeley Open Infrastructure for Network Computing
// http://boinc.berkeley.edu
// Copyright (C) 2008 University of California
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
//
// assimilator for single jobs.
// - if success, move the output file(s) to job directory
// - delete job description file
// - delete WU template file

#include <vector>
#include <string>
#include <cstdlib>
#include <cstring>

#include "boinc_db.h"
#include "error_numbers.h"
#include "filesys.h"
#include "sched_msgs.h"
#include "validate_util.h"
#include "sched_config.h"
#include "sched_util.h"

using std::vector;
using std::string;

int assimilate_handler(
    WORKUNIT& wu, vector<RESULT>& /*results*/, RESULT& canonical_result
) {
    int retval;
    char buf[1024], filename[256], job_dir[256];
    unsigned int i;

    sprintf(filename, "sj_%d", wu.id);
    dir_hier_path(filename, config.upload_dir, config.uldl_dir_fanout, buf);
    FILE* f = fopen(buf, "r");
    if (!f) {
        log_messages.printf(MSG_CRITICAL, "Can't open job file %s\n", buf);
        return 0;
    }
    fgets(buf, 1024, f);
    char* p = strstr(buf, "<job_dir>");
    if (!p) {
        log_messages.printf(MSG_CRITICAL, "garbage in job file: %s\n", buf);
        return 0;
    }
    strcpy(job_dir, buf+strlen("<job_dir>"));
    p = strstr(job_dir, "</job_dir>");
    if (!p) {
        log_messages.printf(MSG_CRITICAL, "garbage in job file: %s\n", buf);
        return 0;
    }
    *p = 0;
    if (wu.canonical_resultid) {
        vector<string> output_file_paths;
        char copy_path[256];
        get_output_file_paths(canonical_result, output_file_paths);
        unsigned int n = output_file_paths.size();
        for (i=0; i<n; i++) {
            string path = output_file_paths[i], logical_name;
            retval = get_logical_name(canonical_result, path, logical_name);
            if (retval) {
                log_messages.printf(MSG_CRITICAL,
                    "Couldn't get logical name for %s: %d\n",
                    path.c_str(), retval
                );
                return retval;
            }
            sprintf(copy_path, "%s/%s", job_dir, logical_name.c_str());
            retval = boinc_copy(path.c_str() , copy_path);
            if (retval) {
                log_messages.printf(MSG_CRITICAL,
                    "couldn't copy file %s to %s\n",
                    path.c_str(), copy_path
                );
                return retval;
            }
        }
    } else {
        sprintf(buf, "%s/error_msg", job_dir);
        f = fopen(buf, "w");
        fprintf(f, "Error: 0x%x\n", wu.error_mask);
    }
    return 0;
}
