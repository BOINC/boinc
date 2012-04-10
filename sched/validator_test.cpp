// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2012 University of California
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

// Test harness for validators
// Link this with your validator functions
// init_result();
// compare_results();
// cleanup_result();

// Then run the resulting program as
// validator_test file1 file2
// and it will compare those two output files
// (this only works if your functions expect 1 file per result)

#include "validate_util.h"
#include "validate_util2.h"

void usage(char* prog) {
    fprintf(stderr,
        "usage: %s file1 file2\n", prog
    );
    exit(1);
}

int get_output_file_info(RESULT r, FILE_INFO& fi) {
    fi.path = r.name;
    return 0;
}

int main(int argc, char** argv) {
    if (argc != 3) {
        usage(argv[0]);
    }
    void* data1, *data2;
    RESULT r1, r2;
    bool match;

    strcpy(r1.name, argv[1]);
    strcpy(r2.name, argv[2]);
    int retval;
    retval = init_result(r1, data1);
    if (retval) {
        fprintf(stderr, "init_result(r1) returned %d\n", retval);
        exit(1);
    }
    retval = init_result(r2, data2);
    if (retval) {
        fprintf(stderr, "init_result(r2) returned %d\n", retval);
        exit(1);
    }
    retval = compare_results(r1, data1, r2, data2, match);
    if (retval) {
        fprintf(stderr, "compare_results() returned %d\n", retval);
        exit(1);
    }
    retval = cleanup_result(r1, data1);
    if (retval) {
        fprintf(stderr, "cleanup_result(r1) returned %d\n", retval);
        exit(1);
    }
    retval = cleanup_result(r2, data2);
    if (retval) {
        fprintf(stderr, "cleanup_result(r2) returned %d\n", retval);
        exit(1);
    }

    printf("compare_results returned %s\n", match?"true":"false");

}
