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

// assimilator for single jobs.
// - if success, move the output file(s) to job directory
// - delete job description file
// - delete WU template file

#include <stdlib.h>
#include <string.h>
#include <string>
#include <unistd.h>
#include <vector>

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
    char buf[1024], filename[256], job_dir[256], job_dir_file[256];
    unsigned int i;

    // delete the template files
    //
    unlink(config.project_path("templates/sj_wu_template_%d", wu.id));
    unlink(config.project_path("templates/sj_result_template_%d", wu.id));

    // read and delete the job directory file
    //
    sprintf(filename, "sj_%d", wu.id);
    dir_hier_path(
        filename, config.upload_dir, config.uldl_dir_fanout, job_dir_file
    );
    FILE* f = fopen(job_dir_file, "r");
    if (!f) {
        log_messages.printf(MSG_CRITICAL, "Can't open job file %s\n", buf);
        return 0;
    }
    if (!fgets(buf, 1024, f)) {
        log_messages.printf(MSG_CRITICAL, "Can't read job file %s\n", buf);
        fclose(f);
        return 0;
    }
    fclose(f);
    unlink(job_dir_file);

    // parse the job directory file
    //
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

    // Create a job summary file
    //
    sprintf(filename, "%s/job_summary_%d", job_dir, wu.id);
    f = fopen(filename, "w");

    // If job was successful, copy the output files
    //
    if (wu.canonical_resultid) {
        fprintf(f,
            "Job was completed by host %d.\n"
            "CPU time: %f seconds\n",
            canonical_result.hostid,
            canonical_result.cpu_time
        );
        vector<OUTPUT_FILE_INFO> output_files;
        char copy_path[MAXPATHLEN];
        get_output_file_infos(canonical_result, output_files);
        unsigned int n = output_files.size();
        for (i=0; i<n; i++) {
            OUTPUT_FILE_INFO& fi = output_files[i];
            string logical_name;
            retval = get_logical_name(canonical_result, fi.path, logical_name);
            if (retval) {
                fprintf(f,
                    "Couldn't get logical name for %s: %s\n",
                    fi.path.c_str(), boincerror(retval)
                );
                continue;
            }
            sprintf(copy_path, "%s/%s", job_dir, logical_name.c_str());
            retval = boinc_copy(fi.path.c_str() , copy_path);
            if (retval) {
                fprintf(f,
                    "Output file %s not present.\n", logical_name.c_str()
                );
                continue;
            }
        }
    } else {
        fprintf(f,
            "The job was not successfully completed.\n"
            "Error: 0x%x\n", wu.error_mask
        );
    }
    fclose(f);
    return 0;
}
