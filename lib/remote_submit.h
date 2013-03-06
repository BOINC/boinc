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

// C++ interfaces to remote job submissions and file management RPCs

#include <stdio.h>
#include <string>
#include <vector>
#include <map>

using std::string;
using std::vector;
using std::map;

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

struct QUERY_BATCH_JOB {
    string job_name;
    string status;
    QUERY_BATCH_JOB(){}
};

struct QUERY_BATCH_REPLY {
    vector<QUERY_BATCH_JOB> jobs;
};

struct FETCH_OUTPUT_REQ {
    char job_name[256];
    char dir[256];
    vector<string> file_names;
};

struct TEMPLATE_DESC {
    vector<string> input_files;
    vector<string> output_files;

    int parse(XML_PARSER&);
};

//////////////////////////


extern int query_files(
    const char* project_url,
    const char* authenticator,
    vector<string> &paths,
    vector<string> &md5s,
    int batch_id,
    vector<int> &absent_files,
    string& error_msg
);

extern int upload_files (
    const char* project_url,
    const char* authenticator,
    vector<string> &paths,
    vector<string> &md5s,
    int batch_id,
    string& error_msg
);

extern int create_batch(
    const char* project_url,
    const char* authenticator,
    const char* batch_name,
    const char* app_name,
    int &batch_id,
    string& error_msg
);

extern int submit_jobs(
    const char* project_url,
    const char* authenticator,
    SUBMIT_REQ &req,
    string& error_msg
);

extern int query_batch(
    const char* project_url,
    const char* authenticator,
    string batch_name,
    QUERY_BATCH_REPLY& reply,
    string& error_msg
);

extern int get_output_file(
    const char* project_url,
    const char* authenticator,
    const char* job_name,
    int file_num,
    const char* dst_path,
    string& error_msg
);

extern int abort_jobs(
    const char* project_url,
    const char* authenticator,
    string batch_name,
    vector<string> &job_names,
    string& error_msg
);

extern int get_templates(
    const char* project_url,
    const char* authenticator,
    const char* app_name,
    TEMPLATE_DESC&,
    string& error_msg
);
