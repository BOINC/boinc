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

#include "validate_util.h"
#include "sched_util.h"

// TODO: use md5 hash

// read file into memory
int init_result_read_file(RESULT const& result, void*& data)
{
    int retval;
    string path;

    retval = get_output_file_path(result, path);
    if (retval) {
        log_messages.printf(
            SchedMessages::CRITICAL,
            "[RESULT#%d %s] check_set: can't get output filename\n",
            result.id, result.name
            );
        return retval;
    }

    string* s = new string;
    data = (void*) s;

    retval = read_file_string(path.c_str(), *s);
    if (retval) {
        log_messages.printf(
            SchedMessages::CRITICAL,
            "[RESULT#%d %s] Couldn't open %s\n",
            result.id, result.name, path.c_str()
            );
        return retval;
    }

    return 0;
}

int check_pair_initialized_identical(RESULT const& /*r1*/, void* data1,
                                     RESULT const& /*r2*/, void* data2,
                                     bool& match)
{
    string const* s1 = (string*) data1;
    string const* s2 = (string*) data2;

    match = (*s1 == *s2);
    return 0;
}

int cleanup_result_string(RESULT const& /*result*/, void* data)
{
    string* s = (string*) data;
    delete s;
    return 0;
}

// See if there's a strict majority under equality.
//
int check_set(vector<RESULT>& results, int& canonicalid, double& credit)
{
    return generic_check_set_majority(results, canonicalid, credit,
                                      init_result_read_file,
                                      check_pair_initialized_identical,
                                      cleanup_result_string);
}

int check_pair(RESULT const& r1, RESULT const& r2, bool& match)
{
    return generic_check_pair(r1, r2, match,
                              init_result_read_file,
                              check_pair_initialized_identical,
                              cleanup_result_string);
}

