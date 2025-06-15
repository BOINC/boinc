// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2015 University of California
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
//      ../results/<batchid>/<wu_name>__file_<log_name>
//      where <log_name> is the file's logical name
// 2) if failure, write a message to result/<batch_id>/<wuname>_error

// Note: daemons like this run in project/tmp_<host>

#include <vector>
#include <string>
#include <cstdlib>

#include "boinc_db.h"
#include "error_numbers.h"
#include "filesys.h"
#include "sched_msgs.h"
#include "validate_util.h"
#include "assimilate_handler.h"

using std::vector;
using std::string;

const char* outdir = "../results";

int write_error(WORKUNIT &wu, char* p) {
    char path[1024];
    sprintf(path, "%s/%d/%s_error", outdir, wu.batch, wu.name);
    FILE* f = fopen(path, "a");
    if (!f) return ERR_FOPEN;
    fprintf(f, "%s", p);
    fclose(f);
    return 0;
}

int assimilate_handler_init(int argc, char** argv) {
    for (int i=1; i<argc; i++) {
        if (!strcmp(argv[i], "--outdir")) {
            outdir = argv[++i];
        } else {
            fprintf(stderr, "bad arg %s\n", argv[i]);
        }
    }
    return 0;
}

void assimilate_handler_usage() {
    // describe the project specific arguments here
    fprintf(stderr,
        "    Custom options:\n"
        "    [--outdir X]  output dir for result files\n"
    );
}

int assimilate_handler(
    WORKUNIT& wu, vector<RESULT>& /*results*/, RESULT& canonical_result
) {
    int retval;
    char buf[1024];
    unsigned int i;

    retval = boinc_mkdir(outdir);
    if (retval) return retval;
    sprintf(buf, "%s/%d", outdir, wu.batch);
    retval = boinc_mkdir(buf);
    if (retval) return retval;

    if (wu.canonical_resultid) {
        vector<OUTPUT_FILE_INFO> output_files;
        get_output_file_infos(canonical_result, output_files);
        unsigned int n = output_files.size();
        bool file_copied = false;
        for (i=0; i<n; i++) {
            OUTPUT_FILE_INFO& fi = output_files[i];
            sprintf(buf, "%s/%d/%s__file_%s",
                outdir, wu.batch, wu.name, fi.logical_name.c_str()
            );
            retval = boinc_rename(fi.path.c_str() , buf);
            if (!retval) {
                file_copied = true;
            }
        }
        if (!file_copied) {
            sprintf(buf, "%s/%d/%s_no_output_files", outdir, wu.batch, wu.name);
            FILE* f = fopen(buf, "w");
            if (!f) return ERR_FOPEN;
            fclose(f);
        }
    } else {
        sprintf(buf, "0x%x\n", wu.error_mask);
        return write_error(wu, buf);
    }
    return 0;
}
