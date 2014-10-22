// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2014 University of California
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

// A validator that runs scripts to check and compare results,
// so that you can do your validation on Python, Perl, bash, etc.
//
// cmdline args:
// --init_script scriptname
// --compare_script scriptname
//
// the init script is called as
// scriptname f1 ... fn
// where f1 ... fn are the output files of a job (there may be just one)
// It returns zero if the files are valid
//
// the compare script is called as
// scriptname f1 ... fn g1 ... gn
// where f1 ... fn are the output files of one job,
// and g1 ... gn are the output files are another job.
// It returns zero if the files are equivalent.

#include <sys/param.h>

#include "validate_util2.h"
#include "error_numbers.h"
#include "boinc_db.h"
#include "sched_util.h"
#include "validate_util.h"
#include "validator.h"

using std::string;
using std::vector;

bool first = true;
char init_script[MAXPATHLEN], compare_script[MAXPATHLEN];

void parse_cmdline() {
    strcpy(init_script, "");
    strcpy(compare_script, "");
    for (int i=1; i<g_argc; i++) {
        if (!strcmp(g_argv[i], "--init_script")) {
            sprintf(init_script, "../bin/%s", g_argv[++i]);
        } else if (!strcmp(g_argv[i], "--compare_script")) {
            sprintf(compare_script, "../bin/%s", g_argv[++i]);
        }
    }
    if (!strlen(init_script) && !strlen(compare_script)) {
        log_messages.printf(MSG_CRITICAL,
            "script names missing from command line\n"
        );
        exit(1);
    }
}

int init_result(RESULT& result, void*&) {
    if (first) {
        parse_cmdline();
        first = false;
    }
    if (!strlen(init_script)) return 0;
    vector<string> paths;
    int retval;
    retval = get_output_file_paths(result, paths);
    if (retval) {
        fprintf(stderr, "get_output_file_paths() returned %d\n", retval);
        return retval;
    }
    char cmd[4096];
    strcpy(cmd, init_script);
    for (unsigned int i=0; i<paths.size(); i++) {
        strcat(cmd, " ");
        strcat(cmd, paths[i].c_str());
    }
    retval = system(cmd);
    if (retval) {
        return retval;
    }
    return 0;
}

int compare_results(RESULT& r1, void*, RESULT const& r2, void*, bool& match) {
    if (first) {
        parse_cmdline();
        first = false;
    }
    if (!strlen(compare_script)) {
        match = true;
        return 0;
    }
    vector<string> paths1, paths2;
    int retval;
    retval = get_output_file_paths(r1, paths1);
    if (retval) {
        fprintf(stderr, "get_output_file_paths() returned %d\n", retval);
        return retval;
    }
    retval = get_output_file_paths(r2, paths2);
    if (retval) {
        fprintf(stderr, "get_output_file_paths() returned %d\n", retval);
        return retval;
    }
    char cmd[4096];
    strcpy(cmd, compare_script);
    for (unsigned int i=0; i<paths1.size(); i++) {
        strcat(cmd, " ");
        strcat(cmd, paths1[i].c_str());
    }
    for (unsigned int i=0; i<paths2.size(); i++) {
        strcat(cmd, " ");
        strcat(cmd, paths2[i].c_str());
    }
    retval = system(cmd);
    if (retval) {
        match = false;
    } else {
        match = true;
    }
    return 0;
}

int cleanup_result(RESULT const&, void*) {
    return 0;
}

