// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2019 University of California
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

// A sample validator that requires a majority of results to be
// bitwise identical.
// This is useful only if either
// 1) your application does no floating-point math, or
// 2) you use homogeneous redundancy
//
// if the --is_gzip option is used, all files are assumed to be gzipped.
// In this case, the 10-byte gzip header is skipped
// (it has stuff like a timestamp and OS code that can differ
// even if the archive contents are the same)

#include "config.h"
#include "util.h"
#include "sched_util.h"
#include "sched_msgs.h"
#include "validate_util.h"
#include "validate_util2.h"
#include "validator.h"
#include "md5_file.h"

using std::string;
using std::vector;

bool is_gzip = false;
    // if true, files are gzipped; skip header when comparing

struct FILE_CKSUM_LIST {
    vector<string> files;   // list of MD5s of files
    ~FILE_CKSUM_LIST(){}
};

int validate_handler_init(int argc, char** argv) {
    // handle project specific arguments here
    for (int i=1; i<argc; i++) {
        if (is_arg(argv[i], "is_gzip")) {
            is_gzip = true;
        }
    }
    return 0;
}

void validate_handler_usage() {
    // describe the project specific arguments here
    fprintf(stderr,
        "    Custom options:\n"
        "    [--is_gzip]  files are gzipped; skip header when comparing\n"
    );
}


bool files_match(FILE_CKSUM_LIST& f1, FILE_CKSUM_LIST& f2) {
    if (f1.files.size() != f2.files.size()) return false;
    for (unsigned int i=0; i<f1.files.size(); i++) {
        if (f1.files[i] != f2.files[i]) return false;
    }
    return true;
}

int init_result(RESULT& result, void*& data) {
    int retval;
    FILE_CKSUM_LIST* fcl = new FILE_CKSUM_LIST;
    vector<OUTPUT_FILE_INFO> files;
    char md5_buf[MD5_LEN];
    double nbytes;

    retval = get_output_file_infos(result, files);
    if (retval) {
        log_messages.printf(MSG_CRITICAL,
            "[RESULT#%lu %s] check_set: can't get output filenames\n",
            result.id, result.name
        );
        delete fcl;
        return retval;
    }

    for (unsigned int i=0; i<files.size(); i++) {
        OUTPUT_FILE_INFO& fi = files[i];
        if (fi.no_validate) continue;
        retval = md5_file(fi.path.c_str(), md5_buf, nbytes, is_gzip);
        if (retval) {
            if (fi.optional && retval == ERR_FOPEN) {
                strcpy(md5_buf, "");
                    // indicate file is missing; not the same as md5("")
            } else {
                log_messages.printf(MSG_CRITICAL,
                    "[RESULT#%lu %s] md5_file() failed for %s: %s\n",
                    result.id, result.name, fi.path.c_str(), boincerror(retval)
                );
                return retval;
            }
        }
        fcl->files.push_back(string(md5_buf));
    }
    data = (void*) fcl;
    return 0;
}

int compare_results(
    RESULT & /*r1*/, void* data1,
    RESULT const& /*r2*/, void* data2,
    bool& match
) {
    FILE_CKSUM_LIST* f1 = (FILE_CKSUM_LIST*) data1;
    FILE_CKSUM_LIST* f2 = (FILE_CKSUM_LIST*) data2;

    match = files_match(*f1, *f2);
    return 0;
}

int cleanup_result(RESULT const& /*result*/, void* data) {
    delete (FILE_CKSUM_LIST*) data;
    return 0;
}
