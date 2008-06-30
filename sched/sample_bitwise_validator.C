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
// 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

// A sample validator that grants credit if the majority of results are
// bitwise identical.
// This is useful only if either
// 1) your application does no floating-point math, or
// 2) you use homogeneous redundancy

#include "config.h"
#include "util.h"
#include "sched_util.h"
#include "sched_msgs.h"
#include "validate_util.h"
#include "md5_file.h"

using std::string;
using std::vector;

struct FILE_CKSUM {
    string md5sum;

    FILE_CKSUM(string& filedata) {
        md5sum = md5_string(filedata);
    }
    ~FILE_CKSUM(){}
};

struct FILE_CKSUM_LIST {
    vector<FILE_CKSUM> files;
    ~FILE_CKSUM_LIST(){}
};

bool files_match(FILE_CKSUM_LIST& f1, FILE_CKSUM_LIST& f2) {
    if (f1.files.size() != f2.files.size()) return false;
    for (unsigned int i=0; i<f1.files.size(); i++) {
        if (f1.files[i].md5sum != f2.files[i].md5sum) return false;
    }
    return true;
}

int init_result(RESULT& result, void*& data) {
    int retval;
    FILE_CKSUM_LIST* fcl = new FILE_CKSUM_LIST;
    vector<FILE_INFO> files;

    retval = get_output_file_infos(result, files);
    if (retval) {
        log_messages.printf(MSG_CRITICAL,
            "[RESULT#%d %s] check_set: can't get output filenames\n",
            result.id, result.name
        );
        return retval;
    }

    string filedata;
    for (unsigned int i=0; i<files.size(); i++) {
        FILE_INFO& fi = files[i];
        if (fi.no_validate) continue;
        retval = read_file_string(fi.path.c_str(), filedata);
        if (retval) {
            if (fi.optional) {
                filedata = "";
            } else {
                log_messages.printf(MSG_CRITICAL,
                    "[RESULT#%d %s] Couldn't open %s\n",
                    result.id, result.name, fi.path.c_str()
                );
                return retval;
            }
        }
        FILE_CKSUM fc(filedata);
        fcl->files.push_back(fc);
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

double compute_granted_credit(WORKUNIT& wu, vector<RESULT>& results) {
    return median_mean_credit(wu, results);
}

const char *BOINC_RCSID_7ab2b7189c = "$Id$";
