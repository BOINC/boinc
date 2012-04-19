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

#ifndef H_VALIDATE_UTIL
#define H_VALIDATE_UTIL

#include <vector>
#include <string>

#include "boinc_db_types.h"

// bit of a misnomer - this actually taken from the <file_ref> elements
// of result.xml_doc_in
//
struct OUTPUT_FILE_INFO {
    std::string name;
    std::string path;
    bool optional;
    bool no_validate;

    int parse(XML_PARSER&);
};

extern int get_output_file_info(RESULT& result, OUTPUT_FILE_INFO&);
extern int get_output_file_infos(RESULT& result, std::vector<OUTPUT_FILE_INFO>&);
extern int get_output_file_path(RESULT& result, std::string&);
extern int get_output_file_paths(RESULT& result, std::vector<std::string>&);
extern int get_logical_name(
    RESULT& result, std::string& path, std::string& name
);

extern int get_credit_from_wu(WORKUNIT&, std::vector<RESULT>& results, double&);

extern bool standalone;
#endif
