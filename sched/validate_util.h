// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2008 University of California
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

#ifndef BOINC_VALIDATE_UTIL_H
#define BOINC_VALIDATE_UTIL_H

#include <vector>
#include <string>

#include "boinc_db_types.h"

// bit of a misnomer - this actually taken from the <file_ref> elements
// of result.xml_doc_in
//
struct OUTPUT_FILE_INFO {
    std::string phys_name;
    std::string path;
    std::string logical_name;
    bool optional;
        // sample_bitwise_validator: not an error if this file is missing
    bool no_validate;
        // sample_bitwise_validator: don't compare this file

    int parse(XML_PARSER&);
};

extern int get_output_file_info(RESULT const& result, OUTPUT_FILE_INFO&);
extern int get_output_file_infos(
    RESULT const& result, std::vector<OUTPUT_FILE_INFO>&
);
extern int get_output_file_path(RESULT const& result, std::string&);
extern int get_output_file_paths(
    RESULT const& result, std::vector<std::string>&
);
extern int get_logical_name(
    RESULT& result, std::string& path, std::string& name
);

extern int get_credit_from_wu(WORKUNIT&, std::vector<RESULT>& results, double&);

extern bool standalone;
    // if set, look for output files in the current directory,
    // not the upload hierarchy.
    // used by validator_test.
extern void remove_random_from_filename(const char* in, char* out);

extern int validate_handler_init(int argc, char** argv);
extern void validate_handler_usage();

#endif
