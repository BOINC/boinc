// This file is part of BOINC.
// https://boinc.berkeley.edu
// Copyright (C) 2024 University of California
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

// wsl_wrapper: wrapper for WSL apps on Windows.
// implementation:
// We use two WSL_CMDs:
// one to run the app, and get its process group ID,
//      and to capture its stdout and stderr
// another to run a shell for monitoring and control operations (ps, kill)
//      (this has less overhead than a shell per op)
//
// Typically, the app has a control script as well as an executable.
//      This might be written in bash or perl,
//      since these languages are in all distros
// The control script might:
//      resolve link files and create symbolic links
//      if the workflow has multiple steps, keep track of where we are
//      run the executable(s)
// Also:
//      If the executable finishes, write a finish file 'boinc_done'
//      If the executable fails, write a file 'boinc_fail'
//      If the script exits without doing either, the job fails
//
// checkpointing: the wrapper signals that a checkpoint has been written
// by touching a file 'boinc_checkpoint_done'

#include <cstdio>
#include <string>

#include "boinc_win.h"
#include "util.h"
#include "win_util.h"
#include "boinc_api.h"
#include "app_ipc.h"

using std::string;

#define VERBOSE 0

WSL_CMD app_wc;
WSL_CMD ctl_wc;
int seqno = 0;
int pgid;       // process group ID of job
bool running;
double checkpoint_cpu_time = 0;
APP_INIT_DATA aid;

#define CMD_TIMEOUT 10.0
#define POLL_PERIOD 1.0

struct RSC_USAGE {
    double cpu_time;
    double wss;
    void clear() {
        cpu_time = 0;
        wss = 0;
    }
};

int error(const char* where, int retval) {
    fprintf(stderr, "%s failed: %d\n", where, retval);
    return retval;
}

// launch application and get process group ID
//
int launch(const char* distro, const char* cmd) {
    char launch_cmd[256];
    sprintf(launch_cmd, "echo $$; %s; touch boinc_job_done\n", cmd);
    int retval = app_wc.setup();
    if (retval) return error("app setup", retval);
    retval = app_wc.run_command(distro, launch_cmd, true);
    if (retval) return error("app run_command", retval);
    string reply;
    retval = read_from_pipe(app_wc.out_read, app_wc.proc_handle, reply, CMD_TIMEOUT, "\n");
    if (retval) return error("app read_from_pipe", retval);
    pgid = atoi(reply.c_str());
#if VERBOSE
    fprintf(stderr, "reply: [%s]\n", reply.c_str());
    fprintf(stderr, "pgid: %d\n", pgid);
#endif
    running = true;

    // set up control channel
    //
    retval = ctl_wc.setup();
    if (retval) return error("ctl setup", retval);
    retval = ctl_wc.run_command(distro, "", true);
    if (retval) return error("ctl run_command", retval);
    return 0;
}

enum JOB_STATUS {JOB_IN_PROGRESS, JOB_SUCCESS, JOB_FAIL};

// Get app resources usage.
// if processes in group still exist, return cpu time and wss
// else if finish file found, return SUCCESS
// else return FAIL
//
JOB_STATUS poll_app(RSC_USAGE &ru) {
    char cmd[256];
    sprintf(cmd, "ps --no-headers -g %d -o cputime,rss; echo EOM\n", pgid);
    write_to_pipe(ctl_wc.in_write, cmd);
    string reply;
    int retval = read_from_pipe(
        ctl_wc.out_read, ctl_wc.proc_handle, reply, CMD_TIMEOUT, "EOM"
    );
    if (retval) {
        error("poll read", retval);
        return JOB_FAIL;
    }
#if VERBOSE
    fprintf(stderr, "ps reply: [%s]\n", reply.c_str());
#endif
    ru.clear();
    int nlines = 0;
    // the first line produced by the ps command is the rather unhelpful
    // 'your 131072x1 screen is bogus; expect trouble'
    // I can't figure out how to prevent this.
    //
    for (string line: split(reply, '\n')) {
        int n, d=0, h, m, s, wss;
        const char* p = line.c_str();
        if (strchr(p, '-')) {
            n = sscanf(line.c_str(), "%d-%d:%d:%d %d", &d, &h, &m, &s, &wss);
            if (n == 5) {
                ru.cpu_time += d*86400 + h*3600 + m*60 + s;
                ru.wss += wss * 1024;
                nlines++;
            }
        } else {
            n = sscanf(line.c_str(), "%d:%d:%d %d", &h, &m, &s, &wss);
            if (n == 4) {
                ru.cpu_time += h*3600 + m*60 + s;
                ru.wss += wss * 1024;
                nlines++;
            }
        }
    }
    if (nlines == 0) {
        if (boinc_file_exists("boinc_job_done")) return JOB_SUCCESS;
        fprintf(stderr, "no finish file\n");
        return JOB_FAIL;
    }
    return JOB_IN_PROGRESS;
}

int suspend() {
    char cmd[256];
    sprintf(cmd, "kill -STOP %d\n", -pgid);     // negative means whole process group
    running = false;
#if VERBOSE
    fprintf(stderr, "sending %s\n", cmd);
#endif
    return write_to_pipe(ctl_wc.in_write, cmd);
}
int resume() {
    char cmd[256];
    sprintf(cmd, "kill -CONT %d\n", -pgid);
    running = true;
#if VERBOSE
    fprintf(stderr, "sending %s\n", cmd);
#endif
    return write_to_pipe(ctl_wc.in_write, cmd);
}
int abort_job() {
    char cmd[256];
    sprintf(cmd, "kill -KILL %d\n", -pgid);
#if VERBOSE
    fprintf(stderr, "sending %s\n", cmd);
#endif
    return write_to_pipe(ctl_wc.in_write, cmd);
}

void poll_client_msgs() {
    BOINC_STATUS status;
    boinc_get_status(&status);
    if (status.no_heartbeat || status.quit_request || status.abort_request) {
        fprintf(stderr, "got quite/abort from client\n");
        abort_job();
        exit(0);
    }
    if (status.suspended) {
#if VERBOSE
        fprintf(stderr, "suspended\n");
#endif
        if (running) suspend();
    } else {
#if VERBOSE
        fprintf(stderr, "not suspended\n");
#endif
        if (!running) resume();
    }
}

int main(int argc, char** argv) {
    const char *os_name_regexp=".*", *os_version_regexp=".*", *pass_thru="";
    for (int i=1; i<argc; i++) {
        if (!strcmp(argv[i], "--os_name_regexp")) {
            os_name_regexp = argv[++i];
        } else if (!strcmp(argv[i], "--os_version_regexp")) {
            os_version_regexp = argv[++i];
        } else if (!strcmp(argv[i], "--pass_thru")) {
            pass_thru = argv[++i];
        } else {
            fprintf(stderr, "unknown option %s\n", argv[i]);
            exit(1);
        }
    }

    BOINC_OPTIONS options;
    memset(&options, 0, sizeof(options));
    options.main_program = true;
    options.check_heartbeat = true;
    options.handle_process_control = true;
    boinc_init_options(&options);

    string distro_name;
    if (boinc_is_standalone()) {
        SetCurrentDirectoryA("C:/ProgramData/BOINC/slots/test");
        distro_name = "Ubuntu-22.04";
    } else {
        boinc_get_init_data(aid);
        WSL_DISTRO *distro = aid.host_info.wsl_distros.find_match(os_name_regexp, os_version_regexp);
        if (!distro) {
            fprintf(stderr, "can't find distro\n");
            exit(1);
        }
        distro_name = distro->distro_name;
    }

    string main_cmd = "./main ";
    main_cmd += pass_thru;
    if (launch(distro_name.c_str(), main_cmd.c_str())) {
        fprintf(stderr, "launch failed\n");
        exit(1);
    }
    while (1) {
        // poll period for message from client must be 1 sec.
        poll_client_msgs();
        // poll period for app status could be greater, but we'll use 1 sec
        RSC_USAGE ru;
        switch (poll_app(ru)) {
        case JOB_FAIL:
            fprintf(stderr, "job failed\n");
            boinc_finish(1);
            goto done;
        case JOB_SUCCESS:
            fprintf(stderr, "job succeeded\n");
            boinc_finish(0);
            goto done;
        }
#if VERBOSE
        fprintf(stderr, "job in progress; cpu: %f wss: %f\n", ru.cpu_time, ru.wss);
#endif
        boinc_report_app_status(ru.cpu_time, checkpoint_cpu_time, 0);
        boinc_sleep(POLL_PERIOD);
    }
    done: return 0;
}
