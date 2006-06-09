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

class FILE_CACHE {
    mutable string _md5sum;
public:
    const string filedata;

    FILE_CACHE(string const& filedata0) : filedata(filedata0) {}
    ~FILE_CACHE(){}

    string const md5sum() const {
        if (_md5sum.empty()) {
            _md5sum = md5_string(filedata);
        }
        return _md5sum;
    }
};

bool files_match(FILE_CACHE const& f1, FILE_CACHE const& f2) {
    return (f1.md5sum() == f2.md5sum() && f1.filedata == f2.filedata);
}

// read file into memory
//
int init_result(RESULT const & result, void*& data) {
    int retval;
    string path;

    retval = get_output_file_path(result, path);
    if (retval) {
        log_messages.printf(
            SCHED_MSG_LOG::MSG_CRITICAL,
            "[RESULT#%d %s] check_set: can't get output filename\n",
            result.id, result.name
        );
        return retval;
    }

    string filedata;
    retval = read_file_string(path.c_str(), filedata);
    if (retval) {
        log_messages.printf(
            SCHED_MSG_LOG::MSG_CRITICAL,
            "[RESULT#%d %s] Couldn't open %s\n",
            result.id, result.name, path.c_str()
        );
        return retval;
    }
    data = (void*) new FILE_CACHE(filedata);
    return 0;
}

int compare_results(
    RESULT & /*r1*/, void* data1,
    RESULT const& /*r2*/, void* data2,
    bool& match
) {
    FILE_CACHE const* f1 = (FILE_CACHE*) data1;
    FILE_CACHE const* f2 = (FILE_CACHE*) data2;

    match = files_match(*f1, *f2);
    return 0;
}

int cleanup_result(RESULT const& /*result*/, void* data) {
    delete (FILE_CACHE*) data;
    return 0;
}

const char *BOINC_RCSID_7ab2b7189c = "$Id$";
