// Berkeley Open Infrastructure for Network Computing
// http://boinc.berkeley.edu
// Copyright (C) 2005 University of California
//
// This is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation;
// either version 2.1 of the License, or (at your option) any later version.
//
// This software is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//
// To view the GNU Lesser General Public License visit
// http://www.gnu.org/copyleft/lesser.html
// or write to the Free Software Foundation, Inc.,
// 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

// wrapper.C
// wrapper program - lets you use a non-BOINC app with BOINC
//
// Handles:
// - suspend/resume/quit/abort
// - copying input/output files
// - reporting CPU time
// - loss of heartbeat from core client
//
// Takes an input file "job.xml" of the form
// <job_desc>
//    <application>NAME</application>
//    <input_file>NAME0</input_file>
//    ...
//    <output_file>NAME0</output_file>
//    ...
// </job_desc>

#include <stdio.h>
#include <vector>
#include <string>
#ifdef _WIN32
#else
#include <unistd.h>
#include <sys/wait.h>
#endif

#include "boinc_api.h"
#include "filesys.h"
#include "util.h"
#include "error_numbers.h"

using std::vector;
using std::string;

vector<string> input_files;
vector<string> output_files;
string application;
bool first = true;
int pid;
bool app_suspended = false;

int copy_input_files() {
    int retval;
    for (unsigned int i=0; i<input_files.size(); i++) {
        string filename;
        boinc_resolve_filename_s(input_files[i].c_str(), filename);
        retval = boinc_copy(filename.c_str(), input_files[i].c_str());
        if (retval) return retval;
    }
    return 0;
}

int copy_output_files() {
    int retval;
    for (unsigned int i=0; i<output_files.size(); i++) {
        string filename;
        boinc_resolve_filename_s(output_files[i].c_str(), filename);
        retval = boinc_copy(input_files[i].c_str(), filename.c_str());
        if (retval) return retval;
    }
    return 0;
}

int parse_job_file() {
    char buf[1024];
    boinc_resolve_filename("job.xml", buf, 1024);
    FILE* f = boinc_fopen(buf, "r");
}

void parse_state_file() {
}

int run_application(char** argv) {
    int retval;

#ifdef _WIN32
#else
    char progname[256];
    pid = fork();
    if (pid == -1) {
        boinc_finish(ERR_FORK);
    }
    if (pid == 0) {
        strcpy(progname, application.c_str());
        retval = execv(progname, argv);
        exit(ERR_EXEC);
    }
#endif
}

bool poll_application(int& status) {
#ifdef _WIN32
#else
    int wpid, stat;
    wpid = waitpid(pid, &stat, WNOHANG);
    if (wpid) {
        status = stat;
        return true;
    }
    return false;
#endif
}

void kill_app() {
#ifdef _WIN32
#else
    kill(pid, SIGKILL);
#endif
}

void poll_boinc_messages() {
    BOINC_STATUS status;
    boinc_get_status(&status);
    if (status.no_heartbeat) {
        kill_app();
        exit(0);
    }
    if (status.quit_request) {
        kill(pid, SIGKILL);
        exit(0);
    }
    if (status.abort_request) {
        kill(pid, SIGKILL);
        exit(0);
    }
    if (status.suspended) {
        if (!app_suspended) {
            kill(pid, SIGSTOP);
            app_suspended = true;
        }
    } else {
        if (app_suspended) {
            kill(pid, SIGCONT);
            app_suspended = false;
        }
    }
}

int main(int argc, char** argv) {
    BOINC_OPTIONS options;
    int retval;

    memset(&options, 0, sizeof(options));
    options.main_program = true;
    options.check_heartbeat = true;
    options.handle_process_control = true;

    boinc_init_options(&options);
    retval = parse_job_file();
    if (retval) {
        boinc_finish(retval);
    }

    parse_state_file();

    if (first) {
        retval = copy_input_files();
    }

    retval = run_application(argv);
    while(1) {
        int status;
        if (poll_application(status)) {
            copy_output_files();
            boinc_finish(status);
        }
        poll_boinc_messages();
        boinc_sleep(1.);
    }
}
