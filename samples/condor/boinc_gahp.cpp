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

// BOINC GAHP (Grid ASCII Helper Protocol) daemon

// Notes:
// - This is currently Unix-only (mostly because of its use of pthreads)
//   but with some work it could be made to run on Windows

#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <map>
#include <set>
#include <string>
#include <vector>
#include <time.h>

#include "str_util.h"
#include "md5_file.h"
#include "parse.h"
#include "remote_submit.h"
#include "svn_version.h"
#include "version.h"

#define BOINC_GAHP_VERSION "1.0.2"

using std::map;
using std::pair;
using std::set;
using std::string;
using std::vector;

//#define DEBUG
    // if set, handle commands synchronously rather than
    // handling them in separate threads
    // Also print more errors

extern size_t strlcpy(char*, const char*, size_t);

char project_url[256];
char authenticator[256];
char response_prefix[256];

// The following are used to implement "asynchronous mode",
// in which we send "R" when a command has completed.
// Synchronization is needed in case 2 commands complete around the same time.
//
bool wrote_r = false;
bool async_mode = false;

#define BPRINTF(fmt, ...) \
    printf( "%s" fmt, response_prefix, ##__VA_ARGS__ ); \

struct SUBMIT_REQ {
    char batch_name[256];
    char app_name[256];
    vector<JOB> jobs;
    int batch_id;
    JOB_PARAMS job_params;
    char app_version_num[256];
};

struct LOCAL_FILE {
    char boinc_name[256];
        // the MD5 followed by filename extension if any
    double nbytes;
};

// represents a command.
//
struct COMMAND {
    int id;
    char cmd[256];
    char* in;
        // the input, in a malloc'd buffer
    char* out;
        // if NULL the command is in progress;
        // otherwise the output in a malloc'd buffer

    SUBMIT_REQ submit_req;
    FETCH_OUTPUT_REQ fetch_output_req;
    vector<string> abort_job_names;
    vector<string> batch_names;
    char batch_name[256];
    double lease_end_time;
    double min_mod_time;

    COMMAND(char* _in) {
        id = 0;
        lease_end_time = 0.0;
        min_mod_time = 0.0;
        in = _in;
        out = NULL;
    }
    ~COMMAND() {
        if (in) free(in);
        if (out) free(out);
    }
    int parse_command();
    int parse_submit(char*);
    int parse_query_batches(char*);
    int parse_fetch_output(char*);
    int parse_abort_jobs(char*);
    int parse_retire_batch(char*);
    int parse_set_lease(char*);
};

vector<COMMAND*> commands;

void filename_extension(const char* path, char* ext) {
    const char* p = strrchr(path, '/');
    if (!p) p = path;
    const char* q = strrchr(p, '.');
    if (q) {
        strcpy(ext, q);
    } else {
        strcpy(ext, "");
    }
}

int compute_boinc_name(string path, LOCAL_FILE& f) {
    char md5[64], ext[256];
    int retval = md5_file(path.c_str(), md5, f.nbytes);
    if (retval) return retval;
    filename_extension(path.c_str(), ext);
    sprintf(f.boinc_name, "%s%s", md5, ext);
    return 0;
}

const char *escape_str(const string &str) {
	static string out;
	out = "";
	for (const char *ptr = str.c_str(); *ptr; ptr++) {
		switch ( *ptr ) {
		case ' ':
		case '\\':
		case '\r':
		case '\n':
			out += '\\';
		default:
			out += *ptr;
		}
	}
	return out.c_str();
}

// Get a list of the input files used by the batch.
// Get their MD5s.
// See if they're already on the server.
// If not, upload them.
//
int process_input_files(SUBMIT_REQ& req, string& error_msg) {
    unsigned int i, j;
    int retval;
    char buf[1024];
    map<string, LOCAL_FILE> local_files;
        // maps local path to info about file


    // get the set of unique source paths
    //
    set<string> unique_paths;
    for (i=0; i<req.jobs.size(); i++) {
        JOB& job = req.jobs[i];
        for (j=0; j<job.infiles.size(); j++) {
            INFILE& infile = job.infiles[j];
            unique_paths.insert(infile.src_path);
        }
    }

    // compute the BOINC names (md5.ext) of these files,
    // and make a map from path to BOINC name and size (LOCAL_FILE)
    //
    set<string>::iterator iter = unique_paths.begin();
    while (iter != unique_paths.end()) {
        string s = *iter;
        LOCAL_FILE lf;
        retval = compute_boinc_name(s, lf);
        if (retval) return retval;
        local_files.insert(std::pair<string, LOCAL_FILE>(s, lf));
        ++iter;
    }

    // ask the server which files it doesn't already have.
    //
    map<string, LOCAL_FILE>::iterator map_iter;
    map_iter = local_files.begin();
    vector<string> boinc_names, paths;
    vector<int> absent_files;
    while (map_iter != local_files.end()) {
        LOCAL_FILE lf = map_iter->second;
        paths.push_back(map_iter->first);
        boinc_names.push_back(lf.boinc_name);
        ++map_iter;
    }
    retval = query_files(
        project_url,
        authenticator,
        boinc_names,
        req.batch_id,
        absent_files,
        error_msg
    );
    if (retval) {
#ifdef DEBUG
        printf("query_files() failed (%d): %s\n", retval, error_msg.c_str());
#endif
        return retval;
    }

    // upload the missing files.
    //
    vector<string> upload_boinc_names, upload_paths;
    for (unsigned int i=0; i<absent_files.size(); i++) {
        int j = absent_files[i];
        upload_boinc_names.push_back(boinc_names[j]);
        upload_paths.push_back(paths[j]);
    }
    retval = upload_files(
        project_url,
        authenticator,
        upload_paths,
        upload_boinc_names,
        req.batch_id,
        error_msg
    );
    if (retval) {
#ifdef DEBUG
        printf("upload_files() failed (%d): %s\n", retval, error_msg.c_str());
#endif
        return retval;
    }

    // fill in the physical file names in the submit request
    //
    for (unsigned int i=0; i<req.jobs.size(); i++) {
        JOB& job = req.jobs[i];
        for (unsigned int j=0; j<job.infiles.size(); j++) {
            INFILE& infile = job.infiles[j];
            map<string, LOCAL_FILE>::iterator iter = local_files.find(infile.src_path);
            LOCAL_FILE& lf = iter->second;
            sprintf(infile.physical_name, "%s", lf.boinc_name);
        }
    }

    return 0;
}

// parse the text coming from Condor
//
int COMMAND::parse_submit(char* p) {
    strlcpy(submit_req.batch_name, strtok_r(NULL, " ", &p), sizeof(submit_req.batch_name));
    strlcpy(submit_req.app_name, strtok_r(NULL, " ", &p), sizeof(submit_req.app_name));
    int njobs = atoi(strtok_r(NULL, " ", &p));
    for (int i=0; i<njobs; i++) {
        JOB job;
        strlcpy(job.job_name, strtok_r(NULL, " ", &p), sizeof(job.job_name));
        int nargs = atoi(strtok_r(NULL, " ", &p));
        for (int j=0; j<nargs; j++) {
            string arg = strtok_r(NULL, " ", &p);
            job.cmdline_args += arg + " ";
        }
        int ninfiles = atoi(strtok_r(NULL, " ", &p));
        for (int j=0; j<ninfiles; j++) {
            INFILE infile;
            infile.mode = FILE_MODE_LOCAL_STAGED;
            strlcpy(infile.src_path, strtok_r(NULL, " ", &p), sizeof(infile.src_path));
            strlcpy(infile.logical_name, strtok_r(NULL, " ", &p), sizeof(infile.logical_name));
            job.infiles.push_back(infile);
        }
        submit_req.jobs.push_back(job);
    }

    JOB_PARAMS jp;
    char *chr = NULL;
    chr = strtok_r(NULL, " ", &p);
    if (chr != NULL) {
      jp.rsc_fpops_est = atof(chr);
      jp.rsc_fpops_bound = atof(strtok_r(NULL, " ", &p));
      jp.rsc_memory_bound = atof(strtok_r(NULL, " ", &p));
      jp.rsc_disk_bound = atof(strtok_r(NULL, " ", &p));
      jp.delay_bound = atof(strtok_r(NULL, " ", &p));
      strlcpy(submit_req.app_version_num, strtok_r(NULL, " ", &p), sizeof(submit_req.app_version_num));
    }

    submit_req.job_params = jp;

    return 0;
}

// To avoid a race condition with file deletion:
// - create a batch record
// - create batch/file associations, and upload files
// - create jobs
//
void handle_submit(COMMAND& c) {
    SUBMIT_REQ& req = c.submit_req;
    TEMPLATE_DESC td;
    int retval;
    string error_msg, s;
    char buf[1024];

    retval = get_templates(
        project_url, authenticator, req.app_name, NULL, td, error_msg
    );
    if (retval) {
        sprintf(buf, "error\\ getting\\ templates:\\ %d\\ ", retval);
        s = string(buf) + escape_str(error_msg);
        c.out = strdup(s.c_str());
        return;
    }
    double expire_time = time(0) + 3600;
    retval = create_batch(
        project_url, authenticator, req.batch_name, req.app_name, expire_time,
        req.batch_id, error_msg
    );
    if (retval) {
        sprintf(buf, "error\\ creating\\ batch:\\ %d\\ ", retval);
        s = string(buf) + escape_str(error_msg);
        c.out = strdup(s.c_str());
        return;
    }
    retval = process_input_files(req, error_msg);
    if (retval) {
        sprintf(buf, "error\\ processing\\ input\\ files:\\ %d\\ ", retval);
        s = string(buf) + escape_str(error_msg);
        c.out = strdup(s.c_str());
        return;
    }

    retval = submit_jobs_params(
        project_url, authenticator,
	req.app_name, req.batch_id, req.jobs, error_msg,
	req.job_params, atoi(req.app_version_num)
    );
    if (retval) {
        sprintf(buf, "error\\ submitting\\ jobs:\\ %d\\ ", retval);
        s = string(buf) + escape_str(error_msg);
    } else {
        s = "NULL";
    }
    c.out = strdup(s.c_str());
}

int COMMAND::parse_query_batches(char* p) {
    min_mod_time = atof(strtok_r(NULL, " ", &p));
    int n = atoi(strtok_r(NULL, " ", &p));
    for (int i=0; i<n; i++) {
        char* q = strtok_r(NULL, " ", &p);
        batch_names.push_back(string(q));
    }
    return 0;
}

void handle_query_batches(COMMAND& c) {
    QUERY_BATCH_SET_REPLY reply;
    char buf[256];
    string error_msg, s;
    int retval = query_batch_set(
        project_url, authenticator, c.min_mod_time, c.batch_names,
        reply, error_msg
    );
    if (retval) {
        sprintf(buf, "error\\ querying\\ batch:\\ %d\\ ", retval);
        s = string(buf) + escape_str(error_msg);
    } else {
        s = string("NULL");
        sprintf(buf, " %f", reply.server_time);
        s += string(buf);
        int i = 0;
        for (unsigned int j=0; j<reply.batch_sizes.size(); j++) {
            int n = reply.batch_sizes[j];
            sprintf(buf, " %d", n);
            s += string(buf);
            for (int k=0; k<n; k++) {
                JOB_STATUS &j = reply.jobs[i++];
                sprintf(buf, " %s %s", j.job_name.c_str(), j.status.c_str());
                s += string(buf);
            }
        }
    }
    c.out = strdup(s.c_str());
}

// <job name> <dir> <stderr_filename>
//      <ALL | SOME>
//      <#files>
//          <src name> <dst>
//          ...
//
int COMMAND::parse_fetch_output(char* p) {
    char* q = strtok_r(NULL, " ", &p);
    if (!q) return -1;
    strlcpy(fetch_output_req.job_name, q, sizeof(fetch_output_req.job_name));

    q = strtok_r(NULL, " ", &p);
    if (!q) return -1;
    strlcpy(fetch_output_req.dir, q, sizeof(fetch_output_req.dir));

    q = strtok_r(NULL, " ", &p);
    if (!q) return -1;
    fetch_output_req.stderr_filename = string(q);

    q = strtok_r(NULL, " ", &p);
    if (!q) return -1;
    if (!strcmp(q, "ALL")) {
        fetch_output_req.fetch_all = true;
    } else if (!strcmp(q, "SOME")) {
        fetch_output_req.fetch_all = false;
    } else {
        return -1;
    }

    int nfiles = atoi(strtok_r(NULL, " ", &p));
    for (int i=0; i<nfiles; i++) {
        OUTFILE of;
        strlcpy(of.src, strtok_r(NULL, " ", &p), sizeof(of.src));
        strlcpy(of.dest, strtok_r(NULL, " ", &p), sizeof(of.dest));
        fetch_output_req.file_descs.push_back(of);
    }
    return 0;
}

// does the job have a single output file whose name ends w/ .zip?
//
bool zipped_output(TEMPLATE_DESC& td) {
    if (td.output_files.size() != 1) return false;
    return ends_with(td.output_files[0].c_str(), ".zip");
}

int output_file_index(TEMPLATE_DESC& td, const char* lname) {
    for (unsigned int i=0; i<td.output_files.size(); i++) {
        if (!strcmp(lname, td.output_files[i].c_str())) {
            return (int)i;
        }
    }
    return -1;
}

void handle_fetch_output(COMMAND& c) {
    string error_msg;
    char buf[1024];
    char path[1024];
    int retval;
    unsigned int i;
    COMPLETED_JOB_DESC cjd;
    FETCH_OUTPUT_REQ &req = c.fetch_output_req;
    string s = "NULL";
    TEMPLATE_DESC td;

    // get the output template
    //
    retval = get_templates(
        project_url, authenticator, NULL, req.job_name, td, error_msg
    );
    if (retval) {
        sprintf(buf, "error\\ getting\\ templates:\\ %d\\ ", retval);
        s = string(buf) + escape_str(error_msg);
        c.out = strdup(s.c_str());
        return;
    }

    // get the job status
    //
    retval = query_completed_job(
        project_url, authenticator, req.job_name, cjd, error_msg
    );
    if (retval) {
        sprintf(buf, "query_completed_job()\\ returned\\ %d\\ ", retval);
        s = string(buf) + escape_str(error_msg);
        goto done;
    }
    sprintf(buf, " %d %f %f", cjd.exit_status, cjd.elapsed_time, cjd.cpu_time);
    s += string(buf);

    // write stderr output to specified file
    //
    if (cjd.canonical_resultid || cjd.error_resultid) {
        if (req.stderr_filename[0] == '/') {
            sprintf(path, "%s", req.stderr_filename.c_str());
        } else {
            sprintf(path, "%s/%s", req.dir, req.stderr_filename.c_str());
        }
        FILE* f = fopen(path, "a");
        if (!f) {
            sprintf(buf, "can't\\ open\\ stderr\\ output\\ file\\ %s ", path);
            s = string(buf);
            goto done;
        }
        fprintf(f, "%s", cjd.stderr_out.c_str());
        fclose(f);
    }

    if (zipped_output(td)) {
        // the job's output file is a zip archive.  Get it and unzip
        //
        sprintf(path, "%s/%s_output.zip", req.dir, req.job_name);
        retval = get_output_file(
            project_url, authenticator, req.job_name, 0, path, error_msg
        );
        if (retval) {
            sprintf(buf, "get_output_file()\\ returned\\ %d\\ ", retval);
            s = string(buf) + escape_str(error_msg);
        } else {
            sprintf(buf, "cd %s; unzip -o %s_output.zip >/dev/null", req.dir, req.job_name);
            retval = system(buf);
            if (retval) {
                s = string("unzip\\ failed");
            } else {
                unlink(path);
            }
        }
    } else if (req.fetch_all) {
        for (i=0; i<td.output_files.size(); i++) {
            sprintf(path, "%s/%s", req.dir, td.output_files[i].c_str());
            retval = get_output_file(
                project_url, authenticator, req.job_name, i, path, error_msg
            );
            if (retval) {
                sprintf(buf, "get_output_file()\\ returned\\ %d\\ ", retval);
                s = string(buf) + escape_str(error_msg);
                break;
            }
        }
    } else {
        for (i=0; i<req.file_descs.size(); i++) {
            char* lname = req.file_descs[i].src;
            int j = output_file_index(td, lname);
            if (j < 0) {
                if (i >= td.output_files.size()) {
                      sprintf(buf, "too\\ many\\ output\\ files\\ specified\\ submit:%u\\ template:%lu",
                          i, td.output_files.size()
                      );
                    s = string(buf);
                    goto done;
                }
                j = i;
            }
            sprintf(path, "%s/%s", req.dir, lname);
            retval = get_output_file(
                project_url, authenticator, req.job_name, i, path, error_msg
            );
            if (retval) {
                sprintf(buf, "get_output_file()\\ returned\\ %d\\ ", retval);
                s = string(buf) + escape_str(error_msg);
                break;
            }
        }
    }

    // We've fetched all required output files; now move or rename them.
    // Use system("mv...") rather than rename(),
    // since the latter doesn't work across filesystems
    //
    for (i=0; i<req.file_descs.size(); i++) {
        char dst_path[4096];
        OUTFILE& of = req.file_descs[i];
        if (!strcmp(of.src, of.dest)) continue;
        if (of.dest[0] == '/') {
            strlcpy(dst_path, of.dest, sizeof(dst_path));
        } else {
            sprintf(dst_path, "%s/%s", req.dir, of.dest);
        }
        sprintf(buf, "mv '%s/%s' '%s'", req.dir, of.src, dst_path);
        retval = system(buf);
        if (retval) {
            s = string("mv\\ failed");
        }
    }
done:
    c.out = strdup(s.c_str());
}

int COMMAND::parse_abort_jobs(char* p) {
    while (1) {
        char* job_name = strtok_r(NULL, " ", &p);
        if (!job_name) break;
        abort_job_names.push_back(string(job_name));
    }
    return 0;
}

void handle_abort_jobs(COMMAND& c) {
    string error_msg;
    int retval = abort_jobs(
        project_url, authenticator, c.abort_job_names, error_msg
    );
    string s;
    char buf[256];
    if (retval) {
        sprintf(buf, "abort_jobs()\\ returned\\ %d\\ ", retval);
        s = string(buf) + escape_str(error_msg);
    } else {
        s = "NULL";
    }
    c.out = strdup(s.c_str());
}

int COMMAND::parse_retire_batch(char* p) {
    strlcpy(batch_name, strtok_r(NULL, " ", &p), sizeof(batch_name));
    return 0;
}

void handle_retire_batch(COMMAND& c) {
    string error_msg;
    int retval = retire_batch(
        project_url, authenticator, c.batch_name, error_msg
    );
    string s;
    char buf[256];
    if (retval) {
        sprintf(buf, "retire_batch()\\ returned\\ %d\\ ", retval);
        s = string(buf) + escape_str(error_msg);
    } else {
        s = "NULL";
    }
    c.out = strdup(s.c_str());
}

int COMMAND::parse_set_lease(char* p) {
    strlcpy(batch_name, strtok_r(NULL, " ", &p), sizeof(batch_name));
    lease_end_time = atof(strtok_r(NULL, " ", &p));
    return 0;
}

void handle_set_lease(COMMAND& c) {
    string error_msg;
    int retval = set_expire_time(
        project_url, authenticator, c.batch_name, c.lease_end_time, error_msg
    );
    string s;
    char buf[256];
    if (retval) {
        sprintf(buf, "set_lease()\\ returned\\ %d\\ ", retval);
        s = string(buf) + escape_str(error_msg);
    } else {
        s = "NULL";
    }
    c.out = strdup(s.c_str());
}

void handle_ping(COMMAND& c) {
    string error_msg, s;
    char buf[256];
    int retval = ping_server(project_url, error_msg);
    if (retval) {
        sprintf(buf, "ping_server\\ returned\\ %d\\ ", retval);
        s = string(buf) + escape_str(error_msg);
    } else {
        s = "NULL";
    }
    c.out = strdup(s.c_str());
}

void* handle_command_aux(void* q) {
    COMMAND &c = *((COMMAND*)q);
    if (!strcasecmp(c.cmd, "BOINC_SUBMIT")) {
        handle_submit(c);
    } else if (!strcasecmp(c.cmd, "BOINC_QUERY_BATCHES")) {
        handle_query_batches(c);
    } else if (!strcasecmp(c.cmd, "BOINC_FETCH_OUTPUT")) {
        handle_fetch_output(c);
    } else if (!strcasecmp(c.cmd, "BOINC_ABORT_JOBS")) {
        handle_abort_jobs(c);
    } else if (!strcasecmp(c.cmd, "BOINC_RETIRE_BATCH")) {
        handle_retire_batch(c);
    } else if (!strcasecmp(c.cmd, "BOINC_SET_LEASE")) {
        handle_set_lease(c);
    } else if (!strcasecmp(c.cmd, "BOINC_PING")) {
        handle_ping(c);
    } else {
        c.out = strdup("Unknown command");
    }
    flockfile(stdout);
    if ( async_mode && !wrote_r ) {
        BPRINTF("R\n");
        fflush(stdout);
        wrote_r = true;
    }
    funlockfile(stdout);
    return NULL;
}

int COMMAND::parse_command() {
    int retval;
    char *p;

    int n = sscanf(in, "%s %d", cmd, &id);
    if (n != 2) {
        return -1;
    }
    char* q = strtok_r(in, " ", &p);
    q = strtok_r(NULL, " ", &p);
    if (!strcasecmp(cmd, "BOINC_SUBMIT")) {
        retval = parse_submit(p);
    } else if (!strcasecmp(cmd, "BOINC_QUERY_BATCHES")) {
        retval = parse_query_batches(p);
    } else if (!strcasecmp(cmd, "BOINC_FETCH_OUTPUT")) {
        retval = parse_fetch_output(p);
    } else if (!strcasecmp(cmd, "BOINC_ABORT_JOBS")) {
        retval = parse_abort_jobs(p);
    } else if (!strcasecmp(cmd, "BOINC_RETIRE_BATCH")) {
        retval = parse_retire_batch(p);
    } else if (!strcasecmp(cmd, "BOINC_SET_LEASE")) {
        retval = parse_set_lease(p);
    } else if (!strcasecmp(cmd, "BOINC_PING")) {
        retval = 0;
    } else {
        retval = -1;
    }
    return retval;
}

void print_version(bool startup) {
    BPRINTF("%s$GahpVersion: %s %s BOINC\\ %s\\ GAHP $\n", startup ? "" : "S ", BOINC_GAHP_VERSION, __DATE__, BOINC_VERSION_STRING);
}

int n_results() {
    int n = 0;
    vector<COMMAND*>::iterator i;
    for (i = commands.begin(); i != commands.end(); ++i) {
        COMMAND *c2 = *i;
        if (c2->out) {
            n++;
        }
    }
    return n;
}

// p is a malloc'ed buffer
//
int handle_command(char* p) {
    char cmd[256];
    int id, retval;

    cmd[0] = '\0';
    sscanf(p, "%s", cmd);
    if (!strcasecmp(cmd, "VERSION")) {
        print_version(false);
    } else if (!strcasecmp(cmd, "COMMANDS")) {
        BPRINTF("S ASYNC_MODE_OFF ASYNC_MODE_ON BOINC_ABORT_JOBS BOINC_FETCH_OUTPUT BOINC_PING BOINC_QUERY_BATCHES BOINC_RETIRE_BATCH BOINC_SELECT_PROJECT BOINC_SET_LEASE BOINC_SUBMIT COMMANDS QUIT RESULTS VERSION\n");
    } else if (!strcasecmp(cmd, "RESPONSE_PREFIX")) {
        flockfile(stdout);
        BPRINTF("S\n");
        strlcpy(response_prefix, p+strlen("RESPONSE_PREFIX "), sizeof(response_prefix));
        funlockfile(stdout);
    } else if (!strcasecmp(cmd, "ASYNC_MODE_ON")) {
        flockfile(stdout);
        BPRINTF("S\n");
        async_mode = true;
        funlockfile(stdout);
    } else if (!strcasecmp(cmd, "ASYNC_MODE_OFF")) {
        flockfile(stdout);
        BPRINTF("S\n");
        async_mode = false;
        funlockfile(stdout);
    } else if (!strcasecmp(cmd, "QUIT")) {
        exit(0);
    } else if (!strcasecmp(cmd, "RESULTS")) {
        flockfile(stdout);
        int cnt = n_results();
        BPRINTF("S %d\n", cnt);
        vector<COMMAND*>::iterator i = commands.begin();
        int j = 0;
        while (i != commands.end() && j < cnt) {
            COMMAND *c2 = *i;
            if (c2->out) {
                BPRINTF("%d %s\n", c2->id, c2->out);
                delete c2;
                i = commands.erase(i);
                j++;
            } else {
                ++i;
            }
        }
        wrote_r = false;
        funlockfile(stdout);
    } else if (!strcasecmp(cmd, "BOINC_SELECT_PROJECT")) {
        int n = sscanf(p, "%s %s %s", cmd, project_url, authenticator);
        if (n ==3) {
            BPRINTF("S\n");
        } else {
            BPRINTF("E\n");
        }
    } else {
        // asynchronous commands go here
        //
        COMMAND *cp = new COMMAND(p);
        p = NULL;
        retval = cp->parse_command();
        if (retval) {
            BPRINTF("E\n");
            delete cp;
            return 0;
        }
#ifdef DEBUG
        handle_command_aux(cp);
        BPRINTF("result: %s\n", cp->out);
        delete cp;
#else
        printf("S\n");
        commands.push_back(cp);
        pthread_t thread_handle;
        pthread_attr_t thread_attrs;
        pthread_attr_init(&thread_attrs);
        pthread_attr_setstacksize(&thread_attrs, 256*1024);
        retval = pthread_create(
            &thread_handle, &thread_attrs, &handle_command_aux, cp
        );
        if (retval) {
            fprintf(stderr, "can't create thread\n");
            return -1;
        }
#endif
    }
    free(p);
    return 0;
}

// read a line from stdin (possibly very long).
// Return it in a malloc'd buffer
//
char* get_cmd() {
    static const int buf_inc = 16384;
    char* p = (char*)malloc(buf_inc);
    if (!p) return NULL;
    int len = 0;
    int buf_size = buf_inc;
    while (1) {
        char c = fgetc(stdin);
        if (c == EOF) {
            free(p);
            return NULL;
        }
        if (c == '\n') {
            p[len] = 0;
            if ( len > 0 && p[len-1] == '\r' ) {
                p[len-1] = 0;
            }
            return p;
        }
        p[len++] = c;
        if (len == buf_size) {
            p = (char*)realloc(p, len+buf_inc);
            buf_size += buf_inc;
        }
    }
}

void read_config() {
    FILE* f = fopen("config.txt", "r");
    if (!f) {
        fprintf(stderr, "no config.txt\n");
        return;
    }
    fgets(project_url, 256, f);
    strip_whitespace(project_url);
    fgets(authenticator, 256, f);
    strip_whitespace(authenticator);
    fclose(f);
    if (!strlen(project_url)) {
        fprintf(stderr, "no project URL given\n");
        exit(1);
    }
    if (!strlen(authenticator)) {
        fprintf(stderr, "no authenticator given\n");
        exit(1);
    }
}

int main(int argc, char*argv[]) {
    if (argc>1) {
        if (!strcmp(argv[1],"--version")) {
            fprintf(stderr,SVN_VERSION"\n");
            return 0;
        }
    }
    read_config();
    strcpy(response_prefix, "");
    print_version(true);
    fflush(stdout);
    while (1) {
        char* p = get_cmd();
        if (p == NULL) break;
        handle_command(p);
        fflush(stdout);
    }
    return 0;
}
