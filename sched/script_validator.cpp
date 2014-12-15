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
// so that you can do your validation in Python, PHP, Perl, bash, etc.
//
// cmdline args to this program:
// --init_script "scriptname arg1 ... argn"
// --compare_script "scriptname arg1 ... argn"
//
// The init script checks the validity of a result,
// e.g. that the output files have the proper format.
// It returns zero if the files are valid
//
// The compare script compares two results.
// If returns zero if the output files are equivalent.
//
// arg1 ... argn represent cmdline args to be passed to the scripts.
// The options for init_script are:
//
// files        list of paths of output files of the result
// result_id    result ID
// runtime      task runtime
//
// Additional options for compare_script, for the second result:
// files2       list of paths of output files
// result_id2   result ID
// runtime2     task runtime
//
// "arg1 ... argn" can be omitted,
// in which case only the output file paths are passed to the scripts.

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
vector<string> init_script, compare_script;
    // first element is script path, other elements are args

void parse_cmdline() {
    for (int i=1; i<g_argc; i++) {
        if (!strcmp(g_argv[i], "--init_script")) {
            init_script = split(g_argv[++i], ' ');
            if (init_script.size() == 1) {
                init_script.push_back(string("files"));
            }
        } else if (!strcmp(g_argv[i], "--compare_script")) {
            compare_script = split(g_argv[++i], ' ');
            if (compare_script.size() == 1) {
                compare_script.push_back("files");
                compare_script.push_back("files2");
            }
        }
    }
    if (!init_script.size() && !compare_script.size()) {
        log_messages.printf(MSG_CRITICAL,
            "script names missing from command line\n"
        );
        exit(1);
    }
}

int init_result(RESULT& result, void*&) {
    unsigned int i, j;
    char buf[256];

    if (first) {
        parse_cmdline();
        first = false;
    }
    if (!init_script.size()) return 0;
    vector<string> paths;
    int retval;
    retval = get_output_file_paths(result, paths);
    if (retval) {
        fprintf(stderr, "get_output_file_paths() returned %d\n", retval);
        return retval;
    }
    char cmd[4096];
    sprintf(cmd, "../bin/%s", init_script[0].c_str());
    for (i=1; i<init_script.size(); i++) {
        string& s = init_script[i];
        if (s == "files") {
            for (j=0; j<paths.size(); j++) {
                strcat(cmd, " ");
                strcat(cmd, paths[j].c_str());
            }
        } else if (s == "runtime") {
            sprintf(buf, " %f", result.elapsed_time);
            strcat(cmd, buf);
        } else if (s == "result_id") {
            sprintf(buf, " %d", result.id);
            strcat(cmd, buf);
        }
    }
    retval = system(cmd);
    if (retval) {
        return retval;
    }
    return 0;
}

int compare_results(RESULT& r1, void*, RESULT const& r2, void*, bool& match) {
    unsigned int i, j;
    char buf[256];

    if (first) {
        parse_cmdline();
        first = false;
    }
    if (!compare_script.size()) {
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
    sprintf(cmd, "../bin/%s", compare_script[0].c_str());
    for (i=1; i<compare_script.size(); i++) {
        string& s = compare_script[i];
        if (s == "files") {
            for (j=0; j<paths1.size(); j++) {
                strcat(cmd, " ");
                strcat(cmd, paths1[j].c_str());
            }
        } else if (s == "files2") {
            for (j=0; j<paths2.size(); j++) {
                strcat(cmd, " ");
                strcat(cmd, paths2[j].c_str());
            }
        } else if (s == "runtime") {
            sprintf(buf, " %f", r1.elapsed_time);
            strcat(cmd, buf);
        } else if (s == "result_id") {
            sprintf(buf, " %d", r1.id);
            strcat(cmd, buf);
        } else if (s == "runtime2") {
            sprintf(buf, " %f", r2.elapsed_time);
            strcat(cmd, buf);
        } else if (s == "result_id2") {
            sprintf(buf, " %d", r2.id);
            strcat(cmd, buf);
        }
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

