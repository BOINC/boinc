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

// C++ interfaces to remote job submission and file management RPCs
// See http://boinc.berkeley.edu/trac/wiki/RemoteJobs#Cinterface

#ifndef REMOTE_SUBMIT_H
#define REMOTE_SUBMIT_H

#include <stdio.h>
#include <string>
#include <vector>
#include <map>

#include "parse.h"

using std::string;
using std::vector;
using std::map;

struct INFILE {
    char physical_name[256];    // BOINC physical name
    char src_path[256];         // path on submit machine
    char logical_name[256];
        // filename on execution machine.
        // not used; could be used to check consistency w/ input template
};

struct JOB {
    char job_name[256];
    string cmdline_args;
    vector<INFILE> infiles;
};

struct LOCAL_FILE {
    char md5[64];
    double nbytes;
};

struct JOB_STATUS {
    string job_name;
    string status;
    JOB_STATUS(){}
};

struct QUERY_BATCH_SET_REPLY {
    double server_time;         // server time at start of query
    vector<int> batch_sizes;    // how many jobs in each of the queried batches
    vector<JOB_STATUS> jobs;    // the jobs, sequentially
};

struct OUTFILE {
    char src[256];      // logical name
    char dest[256];     // name or path on submit host
};

struct FETCH_OUTPUT_REQ {
    char job_name[256];
    char dir[256];
    bool fetch_all;
    string stderr_filename;
    vector<OUTFILE> file_descs;
};

struct TEMPLATE_DESC {
    vector<string> input_files;
    vector<string> output_files;

    int parse(XML_PARSER&);
};

// describes a job that's completed, successfully or not
//
struct COMPLETED_JOB_DESC {
    int canonical_resultid;
    int error_mask;
    int error_resultid;     // if error_mask is nonzero, this may be set
    // the following fields describe either the canonical or error result
    int exit_status;
    double elapsed_time;
    double cpu_time;
    string stderr_out;

    COMPLETED_JOB_DESC(){}
    int parse(XML_PARSER&);
};

//////////////////////////


extern int query_files(
    const char* project_url,
    const char* authenticator,
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
    double expire_time,
    int &batch_id,
    string& error_msg
);

extern int submit_jobs(
    const char* project_url,
    const char* authenticator,
    char app_name[256],
    int batch_id,
    vector<JOB> jobs,
    string& error_msg
);

extern int estimate_batch(
    const char* project_url,
    const char* authenticator,
    char app_name[256],
    vector<JOB> jobs,
    double& est_makespan,
    string& error_msg
);

// Return the short status of the jobs in a given set of batches
// Used by the Condor interface
//
extern int query_batch_set(
    const char* project_url,
    const char* authenticator,
    double min_mod_time,
    vector<string> &batch_names,
    QUERY_BATCH_SET_REPLY& reply,
    string& error_msg
);

struct BATCH_STATUS {
    int id;
    char name[256];             // name of batch
    char app_name[256];
    int state;                  // see lib/common_defs.h
    int njobs;                  // how many jobs in batch
    int nerror_jobs;            // how many jobs errored out
    double fraction_done;       // how much of batch is done (0..1)
    double create_time;         // when batch was created
    double expire_time;         // when it will expire
    double est_completion_time;     // estimated completion time
    double completion_time;     // if completed, actual completion time
    double credit_estimate;     // original estimate for credit
    double credit_canonical;    // if completed, granted credit

    int parse(XML_PARSER&);
    void print();
};

// Return a list of this user's batches
//
extern int query_batches(
    const char* project_url,
    const char* authenticator,
    vector<BATCH_STATUS>& batches,
    string& error_msg
);

struct JOB_STATE {
    int id;
    char name[256];
    int canonical_instance_id;      // it job completed successfully,
                                    // the ID of the canonical instance
    int n_outfiles;                 // number of output files

    int parse(XML_PARSER&);
    void print();
};

// Return the state of jobs in a given batch
// (can specify by either ID or name)
//
extern int query_batch(
    const char* project_url,
    const char* authenticator,
    int batch_id,
    const char batch_name[256],
    vector<JOB_STATE>& jobs,
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
    vector<string> &job_names,
    string& error_msg
);

extern int query_completed_job(
    const char* project_url,
    const char* authenticator,
    const char* job_name,
    COMPLETED_JOB_DESC&,
    string& error_msg
);

extern int get_templates(
    const char* project_url,
    const char* authenticator,
    const char* app_name,   // either this
    const char* job_name,   // or this must be non-NULL
    TEMPLATE_DESC&,
    string& error_msg
);

extern int retire_batch(
    const char* project_url,
    const char* authenticator,
    const char* batch_name,
    string& error_msg
);

extern int set_expire_time(
    const char* project_url,
    const char* authenticator,
    const char* batch_name,
    double expire_time,
    string& error_msg
);

extern int ping_server(
    const char* project_url,
    string& error_msg
);

#endif
