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

// BOINC GAHP (Grid ASCII Helper Protocol) daemon

#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <map>
#include <set>
#include <string>
#include <vector>

using std::map;
using std::pair;
using std::set;
using std::string;
using std::vector;

bool async_mode = true;

// represents a command.
// if out is NULL the command is in progress;
// otherwise it's the output
//
struct COMMAND {
    char* in;
    char* out;
};

typedef map<int, COMMAND*> COMMANDS;
COMMANDS commands;

struct INFILE {
    char src_path[256];
    char dst_path[256];
};

struct JOB {
    char job_name[256];
    vector<string> args;
    vector<INFILE> infiles;
    bool all_output_files;
    vector<string> outfiles;
};

struct SUBMIT_REQ {
    char batch_name[256];
    char app_name[256];
    vector<JOB> jobs;
};

void compute_md5(string& path) {
}

// Get a list of the input files used by the batch.
// Get their MD5s.
// See if they're already on the server.
// If not, upload them.
//
int process_input_files(SUBMIT_REQ& req) {
    unsigned int i, j;

    // get the set of source paths w/o dups
    //
    set<string> files;
    for (i=0; i<req.jobs.size(); i++) {
        JOB& job = req.jobs[i];
        for (j=0; j<job.infiles.size(); j++) {
            INFILE infile = job.infiles[j];
            files.insert(infile.src_path);
        }
    }

    // compute the MD5s of these files
    //
    map<string, string> md5s;
    set<string>::iterator iter = files.begin();
    while (iter != files.end()) {
        string s = *iter;
        compute_md5(s);
        iter++;
    }



}

int parse_boinc_submit(COMMAND& c, char* p, SUBMIT_REQ& req) {
    strcpy(req.batch_name, strtok_r(NULL, " ", &p));
    strcpy(req.app_name, strtok_r(NULL, " ", &p));
    int njobs = atoi(strtok_r(NULL, " ", &p));
    for (int i=0; i<njobs; i++) {
        JOB job;
        strcpy(job.job_name, strtok_r(NULL, " ", &p));
        int nargs = atoi(strtok_r(NULL, " ", &p));
        for (int j=0; j<nargs; j++) {
            string arg = strtok_r(NULL, " ", &p);
            job.args.push_back(arg);
        }
        int ninfiles = atoi(strtok_r(NULL, " ", &p));
        for (int j=0; j<ninfiles; j++) {
            INFILE infile;
            strcpy(infile.src_path, strtok_r(NULL, " ", &p));
            strcpy(infile.dst_path, strtok_r(NULL, " ", &p));
            job.infiles.push_back(infile);
        }
        char* q = strtok_r(NULL, " ", &p);
        if (!strcmp(q, "ALL")) {
            job.all_output_files = true;
        } else {
            job.all_output_files = false;
            int noutfiles = atoi(q);
            for (int j=0; j<noutfiles; j++) {
                string outfile = strtok_r(NULL, " ", &p);
                job.outfiles.push_back(outfile);
            }
        }
        req.jobs.push_back(job);
    }
    return 0;
}

int submit_jobs(SUBMIT_REQ req) {
    return 0;
}

void handle_boinc_submit(COMMAND& c, char* p) {
    SUBMIT_REQ req;
    int retval;
    retval = parse_boinc_submit(c, p, req);
    process_input_files(req);
    submit_jobs(req);
}

void* handle_command_aux(void* q) {
    COMMAND &c = *((COMMAND*)q);
    char *p;

    strtok_r(c.in, " ", &p);
    char* cmd = strtok_r(NULL, " ", &p);
    char* id = strtok_r(NULL, " ", &p);
    if (!strcmp(cmd, "BOINC_SUBMIT")) {
        handle_boinc_submit(c, p);
    } else {
        sleep(10);
        char buf[256];
        sprintf(buf, "handled command %s", c.in);
        c.out = strdup(buf);
    }
    return NULL;
}

int handle_command(COMMAND& c) {
    char cmd[256];
    int id;

    // Handle synchronous commands
    //
    sscanf(c.in, "%s", cmd);
    if (!strcmp(cmd, "VERSION")) {
        printf("1.0\n");
    } else if (!strcmp(cmd, "QUIT")) {
        exit(0);
    } else if (!strcmp(cmd, "RESULTS")) {
        sscanf(c.in, "%s %d", cmd, &id);
        COMMANDS::iterator i = commands.find(id);
        if (i == commands.end()) {
            printf("command %d not found\n", id);
            return 0;
        }
        COMMAND *c2 = i->second;
        if (c2->out) {
            printf("command %d result: %s\n", id, c2->out);
            free(c2->out);
            free(c2->in);
            free(c2);
            commands.erase(i);
        } else {
            printf("command %d not finished\n", id);
        }
        return 0;
    } else {

        // Handle asynchronous commands
        //
        int n = sscanf(c.in, "%s %d", cmd, &id);
        if (n != 2) {
            fprintf(stderr, "invalid command: %s\n", c.in);
            return -1;
        }
        COMMAND *cp = new COMMAND;
        cp->in = c.in;
        cp->out = NULL;
        printf("inserting cmd %d\n", id);
        commands.insert(pair<int, COMMAND*>(id, cp));
        if (async_mode) {
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
        } else {
            handle_command_aux(cp);
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

int main() {
    char* p;
    int retval;
    while (1) {
        p = get_cmd();
        if (p == NULL) break;
        COMMAND c;
        c.in = p;
        retval = handle_command(c);
        if (retval) break;
    }
}
