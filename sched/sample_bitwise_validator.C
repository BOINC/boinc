// The contents of this file are subject to the BOINC Public License
// Version 1.0 (the "License"); you may not use this file except in
// compliance with the License. You may obtain a copy of the License at
// http://boinc.berkeley.edu/license_1.0.txt
//
// Software distributed under the License is distributed on an "AS IS"
// basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the
// License for the specific language governing rights and limitations
// under the License.
//
// The Original Code is the Berkeley Open Infrastructure for Network Computing.
//
// The Initial Developer of the Original Code is the SETI@home project.
// Portions created by the SETI@home project are Copyright (C) 2002
// University of California at Berkeley. All Rights Reserved.
//
// Contributor(s):
//

// A sample validator that grants credit if the majority of results are
// bit-wise identical.

#include "util.h"
#include "sched_util.h"
#include "sched_msgs.h"
#include "validate_util.h"
#include "md5_file.h"

using std::string;
using std::vector;


class FileCache {
    mutable string _md5sum;
public:
    const string filedata;

    FileCache(string const& filedata0) : filedata(filedata0) {}

    string const md5sum() const {
        if (_md5sum.empty()) {
            _md5sum = md5_string(filedata);
        }
        return _md5sum;
    }
};

bool operator ==(FileCache const& f1, FileCache const& f2)
{
    return (f1.md5sum() == f2.md5sum() &&
            f1.filedata == f2.filedata);
}

// read file into memory
//
int init_result_read_file(RESULT const & result, void*& data)
{
    int retval;
    string path;

    retval = get_output_file_path(result, path);
    if (retval) {
        log_messages.printf(
            SCHED_MSG_LOG::CRITICAL,
            "[RESULT#%d %s] check_set: can't get output filename\n",
            result.id, result.name
        );
        return retval;
    }

    string filedata;
    retval = read_file_string(path.c_str(), filedata);
    if (retval) {
        log_messages.printf(
            SCHED_MSG_LOG::CRITICAL,
            "[RESULT#%d %s] Couldn't open %s\n",
            result.id, result.name, path.c_str()
        );
        return retval;
    }
    FileCache *f = new FileCache(filedata);
    data = (void*) f;

    return 0;
}

int check_pair_initialized_identical(
    RESULT & /*r1*/, void* data1,
    RESULT const& /*r2*/, void* data2,
    bool& match)
{
    FileCache const* f1 = (FileCache*) data1;
    FileCache const* f2 = (FileCache*) data2;

    match = (*f1 == *f2);
    return 0;
}

int cleanup_result_string(RESULT const& /*result*/, void* data) {
    delete (FileCache*) data;
    return 0;
}

// See if there's a strict majority under equality.
//
int check_set(
    vector<RESULT>& results, DB_WORKUNIT&, int& canonicalid, double& credit,
    bool& retry
) {
    retry = false;
    return generic_check_set_majority(
        results, canonicalid, credit,
        init_result_read_file,
        check_pair_initialized_identical,
        cleanup_result_string
    );
}

int check_pair(
    RESULT & r1, RESULT const& r2, bool& retry
) {
    retry = false;
    int retval = generic_check_pair(
        r1, r2,
        init_result_read_file,
        check_pair_initialized_identical,
        cleanup_result_string
    );
    return retval;
}

