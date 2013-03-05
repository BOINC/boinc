// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2013 University of California
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

#include <map>
#include <string>
#include <vector>

using std::map;
using std::string;
using std::vector;

struct INFILE {
    char src_path[256];
    char dst_path[256];
};

struct JOB {
    char job_name[256];
    string cmdline_args;
    vector<INFILE> infiles;
    bool all_output_files;
    vector<string> outfiles;
};

struct LOCAL_FILE {
    char md5[64];
    double nbytes;
};

struct SUBMIT_REQ {
    char batch_name[256];
    char app_name[256];
    vector<JOB> jobs;
    map<string, LOCAL_FILE> local_files;
        // maps local path to info about file
    int batch_id;
};
