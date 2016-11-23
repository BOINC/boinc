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

#include <stdio.h>

#include "svn_version.h"
#include "sched_util_basic.h"
#include "validate_util.h"
#include "validate_util2.h"

void usage(char* prog) {
    fprintf(stderr,
        "This program is a test validator; \n"
        "You can test your custom handler with this\n"
    );
    fprintf(stderr, "usage: %s [options] file1 file2\n"
        "    Options:\n"
        "    [-h|--help]     Print this usage information and exit\n"
        "    [-v|--version]  Print version information and exit\n"
        "    file1           Path to file to be compared (must be second to last)\n"
        "    file2           Path to file to be compared (must be last)\n"
        "\n",
        prog);
    validate_handler_usage();
}

int get_output_file_info(RESULT r, OUTPUT_FILE_INFO& fi) {
    fi.path = r.name;
    return 0;
}

int main(int argc, char** argv) {
    int retval;

    if (argc > 1) {
      if (is_arg(argv[1], "h") || is_arg(argv[1], "help")) {
        usage(argv[0]);
        exit(0);
      } else if (is_arg(argv[1], "v") || is_arg(argv[1], "version")) {
        printf("%s\n", SVN_VERSION);
        exit(0);
      }
    }
    if (argc < 3) {
        usage(argv[0]);
        exit(0);
    }

    retval = validate_handler_init(argc, argv);
    if (retval) exit(1);

    void* data1, *data2;
    RESULT r1, r2;
    bool match;

    standalone = true;

    sprintf(r1.xml_doc_in, "<file_ref><file_name>%s</file_name></file_ref>", argv[argc-2]);
    sprintf(r2.xml_doc_in, "<file_ref><file_name>%s</file_name></file_ref>", argv[argc-1]);

    retval = init_result(r1, data1);
    if (retval) {
        fprintf(stderr, "init_result(r1) returned %d\n", retval);
        exit(1);
    }
    printf("init_result(r1) succeeded\n");
    retval = init_result(r2, data2);
    if (retval) {
        fprintf(stderr, "init_result(r2) returned %d\n", retval);
        exit(1);
    }
    printf("init_result(r2) succeeded\n");
    retval = compare_results(r1, data1, r2, data2, match);
    if (retval) {
        fprintf(stderr, "compare_results() returned %d\n", retval);
        exit(1);
    }
    printf("compare_results returned %s\n", match?"true - result match":"false - results don't match");
    retval = cleanup_result(r1, data1);
    if (retval) {
        fprintf(stderr, "cleanup_result(r1) returned %d\n", retval);
        exit(1);
    }
    printf("cleanup_result(r1) succeeded\n");
    retval = cleanup_result(r2, data2);
    if (retval) {
        fprintf(stderr, "cleanup_result(r2) returned %d\n", retval);
        exit(1);
    }
    printf("cleanup_result(r2) succeeded\n");
}
