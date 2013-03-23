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

#include "md5_file.h"
#include "parse.h"
#include "remote_submit.h"

using std::map;
using std::pair;
using std::set;
using std::string;
using std::vector;

char project_url[256];
char authenticator[256];

bool debug_mode = true;

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

    COMMAND(char* _in) {
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
};

vector<COMMAND*> commands;

int compute_md5(string path, LOCAL_FILE& f) {
    return md5_file(path.c_str(), f.md5, f.nbytes);
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

    // get the set of unique source paths
    //
    set<string> unique_paths;
    for (i=0; i<req.jobs.size(); i++) {
        JOB& job = req.jobs[i];
        for (j=0; j<job.infiles.size(); j++) {
            INFILE infile = job.infiles[j];
            unique_paths.insert(infile.src_path);
        }
    }

    // compute the MD5s of these files,
    // and make a map from path to MD5 and size (LOCAL_FILE)
    //
    set<string>::iterator iter = unique_paths.begin();
    while (iter != unique_paths.end()) {
        string s = *iter;
        LOCAL_FILE lf;
        retval = compute_md5(s, lf);
        if (retval) return retval;
        req.local_files.insert(std::pair<string, LOCAL_FILE>(s, lf));
        iter++;
    }

    // ask the server which files it doesn't already have.
    //
    map<string, LOCAL_FILE>::iterator map_iter;
    map_iter = req.local_files.begin();
    vector<string> md5s, paths;
    vector<int> absent_files;
    while (map_iter != req.local_files.end()) {
        LOCAL_FILE lf = map_iter->second;
        paths.push_back(map_iter->first);
        md5s.push_back(lf.md5);
        map_iter++;
    }
    retval = query_files(
        project_url,
        authenticator,
        paths,
        md5s,
        req.batch_id,
        absent_files,
        error_msg
    );
    if (retval) return retval;

    // upload the missing files.
    //
    vector<string> upload_md5s, upload_paths;
    for (unsigned int i=0; i<absent_files.size(); i++) {
        int j = absent_files[i];
        upload_md5s.push_back(md5s[j]);
        upload_paths.push_back(paths[j]);
    }
    retval = upload_files(
        project_url,
        authenticator,
        upload_paths,
        upload_md5s,
        req.batch_id,
        error_msg
    );
    if (retval) return retval;
    return 0;
}

// parse the text coming from Condor
//
int COMMAND::parse_submit(char* p) {
    strcpy(submit_req.batch_name, strtok_r(NULL, " ", &p));
    strcpy(submit_req.app_name, strtok_r(NULL, " ", &p));
    int njobs = atoi(strtok_r(NULL, " ", &p));
    for (int i=0; i<njobs; i++) {
        JOB job;
        strcpy(job.job_name, strtok_r(NULL, " ", &p));
        int nargs = atoi(strtok_r(NULL, " ", &p));
        for (int j=0; j<nargs; j++) {
            string arg = strtok_r(NULL, " ", &p);
            job.cmdline_args += arg + " ";
        }
        int ninfiles = atoi(strtok_r(NULL, " ", &p));
        for (int j=0; j<ninfiles; j++) {
            INFILE infile;
            strcpy(infile.src_path, strtok_r(NULL, " ", &p));
            strcpy(infile.dst_filename, strtok_r(NULL, " ", &p));
            job.infiles.push_back(infile);
        }
        submit_req.jobs.push_back(job);
    }
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
        sprintf(buf, "error getting templates: %d\n", retval);
        s = string(buf) + error_msg;
        c.out = strdup(s.c_str());
        return;
    }
    retval = create_batch(
        project_url, authenticator, req.batch_name, req.app_name, req.batch_id, error_msg
    );
    if (retval) {
        sprintf(buf, "error creating batch: %d ", retval);
        s = string(buf) + error_msg;
        c.out = strdup(s.c_str());
        return;
    }
    retval = process_input_files(req, error_msg);
    if (retval) {
        sprintf(buf, "error processing input files: %d ", retval);
        s = string(buf) + error_msg;
        c.out = strdup(s.c_str());
        return;
    }
    retval = submit_jobs(project_url, authenticator, req, error_msg);
    if (retval) {
        sprintf(buf, "error submitting jobs: %d ", retval);
        s = string(buf) + error_msg;
    } else {
        s = "NULL";
    }
    c.out = strdup(s.c_str());
}

int COMMAND::parse_query_batches(char* p) {
    int n = atoi(strtok_r(NULL, " ", &p));
    for (int i=0; i<n; i++) {
        char* q = strtok_r(NULL, " ", &p);
        batch_names.push_back(string(q));
    }
    return 0;
}

void handle_query_batches(COMMAND&c) {
    QUERY_BATCH_REPLY reply;
    char buf[256];
    string error_msg, s;
    int retval = query_batches(
        project_url, authenticator, c.batch_names, reply, error_msg
    );
    if (retval) {
        sprintf(buf, "error querying batch: %d ", retval);
        s = string(buf) + error_msg;
    } else {
        s = string("NULL ");
        for (unsigned int i=0; i<reply.jobs.size(); i++) {
            QUERY_BATCH_JOB &j = reply.jobs[i];
            sprintf(buf, "%s %s ", j.job_name.c_str(), j.status.c_str());
            s += string(buf);
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
    strcpy(fetch_output_req.job_name, q);

    q = strtok_r(NULL, " ", &p);
    if (!q) return -1;
    strcpy(fetch_output_req.dir, q);

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
        strcpy(of.src, strtok_r(NULL, " ", &p));
        strcpy(of.dest, strtok_r(NULL, " ", &p));
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
        sprintf(buf, "error getting templates: %d\n", retval);
        s = string(buf) + error_msg;
        c.out = strdup(s.c_str());
        return;
    }

    // get the job status
    //
    retval = query_completed_job(
        project_url, authenticator, req.job_name, cjd, error_msg
    );
    if (retval) {
        sprintf(buf, "query_completed_job() returned %d ", retval);
        s = string(buf) + error_msg;
        goto done;
    }
    sprintf(buf, " %d %f %f", cjd.exit_status, cjd.elapsed_time, cjd.cpu_time);
    s += string(buf);

    // write stderr output to specified file
    //
    if (cjd.canonical_resultid || cjd.error_resultid) {
        sprintf(path, "%s/%s", req.dir, req.stderr_filename.c_str());
        FILE* f = fopen(path, "w");
        if (!f) {
            sprintf(buf, "can't open stderr output file %s ", path);
            s = string(buf);
            goto done;
        }
        fprintf(f, "%s", cjd.stderr_out.c_str());
        fclose(f);
    }

    if (zipped_output(td)) {
        // the job's output file is a zip archive.  Get it and unzip
        //
        sprintf(path, "%s/temp.zip", req.dir);
        retval = get_output_file(
            project_url, authenticator, req.job_name, 0, path, error_msg
        );
        if (retval) {
            sprintf(buf, "get_output_file() returned %d ", retval);
            s = string(buf) + error_msg;
        } else {
            sprintf(buf, "cd %s; unzip temp.zip");
            retval = system(buf);
            if (retval) {
                s = string("unzip failed");
            }
        }
    } else if (req.fetch_all) {
        for (i=0; i<td.output_files.size(); i++) {
            sprintf(path, "%s/%s", req.dir, td.output_files[i].c_str());
            retval = get_output_file(
                project_url, authenticator, req.job_name, i, path, error_msg
            );
            if (retval) {
                sprintf(buf, "get_output_file() returned %d ", retval);
                s = string(buf) + error_msg;
                break;
            }
        }
    } else {
        for (i=0; i<req.file_descs.size(); i++) {
            char* lname = req.file_descs[i].src;
            int j = output_file_index(td, lname);
            if (j < 0) {
                sprintf(buf, "requested file %s not in template", lname);
                s = string(buf);
                goto done;
            }
            sprintf(path, "%s/%s", req.dir, lname);
            retval = get_output_file(
                project_url, authenticator, req.job_name, i, path, error_msg
            );
            if (retval) {
                sprintf(buf, "get_output_file() returned %d ", retval);
                s = string(buf) + error_msg;
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
            strcpy(dst_path, of.dest);
        } else {
            sprintf(dst_path, "%s/%s", req.dir, of.dest);
        }
        sprintf(buf, "mv %s/%s %s", req.dir, of.src, dst_path);
        retval = system(buf);
        if (retval) {
            s = string("mv failed");
        }
    }
done:
    c.out = strdup(s.c_str());
}

int COMMAND::parse_abort_jobs(char* p) {
    strcpy(batch_name, strtok_r(NULL, " ", &p));
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
        project_url, authenticator, string(c.batch_name),
        c.abort_job_names, error_msg
    );
    string s;
    char buf[256];
    if (retval) {
        sprintf(buf, "abort_jobs() returned %d \n", retval);
        s = string(buf) + error_msg;
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
        sprintf(buf, "ping_server returned %d \n", retval);
        s = string(buf) + error_msg;
    } else {
        s = "NULL";
    }
    c.out = strdup(s.c_str());
}

void* handle_command_aux(void* q) {
    COMMAND &c = *((COMMAND*)q);
    if (!strcmp(c.cmd, "BOINC_SUBMIT")) {
        handle_submit(c);
    } else if (!strcmp(c.cmd, "BOINC_QUERY_BATCHES")) {
        handle_query_batches(c);
    } else if (!strcmp(c.cmd, "BOINC_FETCH_OUTPUT")) {
        handle_fetch_output(c);
    } else if (!strcmp(c.cmd, "BOINC_ABORT_JOBS")) {
        handle_abort_jobs(c);
    } else if (!strcmp(c.cmd, "BOINC_PING")) {
        handle_ping(c);
    } else {
        c.out = strdup("Unknown command");
    }
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
    if (!strcmp(cmd, "BOINC_SUBMIT")) {
        retval = parse_submit(p);
    } else if (!strcmp(cmd, "BOINC_QUERY_BATCHES")) {
        retval = parse_query_batches(p);
    } else if (!strcmp(cmd, "BOINC_FETCH_OUTPUT")) {
        retval = parse_fetch_output(p);
    } else if (!strcmp(cmd, "BOINC_ABORT_JOBS")) {
        retval = parse_abort_jobs(p);
    } else if (!strcmp(cmd, "BOINC_PING")) {
        retval = 0;
    } else {
        retval = -1;
    }
    return retval;
}

// p is a malloc'ed buffer
//
int handle_command(char* p) {
    char cmd[256];
    int id;

    sscanf(p, "%s", cmd);
    printf("cmd: %s\n", cmd);
    if (!strcmp(cmd, "VERSION")) {
        printf("1.0\n");
    } else if (!strcmp(cmd, "QUIT")) {
        exit(0);
    } else if (!strcmp(cmd, "RESULTS")) {
        vector<COMMAND*>::iterator i = commands.begin();
        while (i != commands.end()) {
            COMMAND *c2 = *i;
            if (c2->out) {
                printf("%d %s\n", c2->id, c2->out);
                free(c2->out);
                free(c2->in);
                free(c2);
                i = commands.erase(i);
            } else {
                i++;
            }
        }
    } else if (!strcmp(cmd, "BOINC_SELECT_PROJECT")) {
        int n = sscanf(p, "%s %s %s", cmd, project_url, authenticator);
        if (n ==3) {
            printf("S\n");
        } else {
            printf("E\n");
        }
    } else {
        // asynchronous commands go here
        //
        COMMAND *cp = new COMMAND(p);
        int retval = cp->parse_command();
        if (retval) {
            printf("E\n");
            delete cp;
            return 0;
        }
        if (debug_mode) {
            handle_command_aux(cp);
            printf("result: %s\n", cp->out);
            delete cp;
        } else {
            printf("S\n");
            commands.push_back(cp);
            pthread_t thread_handle;
            pthread_attr_t thread_attrs;
            pthread_attr_init(&thread_attrs);
            pthread_attr_setstacksize(&thread_attrs, 32768);
            int retval = pthread_create(
                &thread_handle, &thread_attrs, &handle_command_aux, cp
            );
            if (retval) {
                fprintf(stderr, "can't create thread\n");
                return -1;
            }
        }
    }
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
            return NULL;
        }
        if (c == '\n') {
            p[len] = 0;
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
        exit(1);
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

int main() {
    read_config();
    while (1) {
        char* p = get_cmd();
        if (p == NULL) break;
        handle_command(p);
        fflush(stdout);
    }
}
