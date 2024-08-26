// This file is part of BOINC.
// http://boinc.berkeley.edu
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
//      when done, write a finish file
//
// checkpointing: the wrapper conveys a checkpoint request
// by touching a file.
// The app can check this file to decide when to checkpoint.
// It signals that a checkpoint has been written
// by touching another file

#include <cstdio>
#include <string>

#include "boinc_win.h"
#include "win_util.h"
#include "boinc_api.h"

using std::string;

WSL_CMD app_wc;
WSL_CMD ctl_wc;
int seqno = 0;
int pgid;       // process group ID of job
bool running;

#define CMD_TIMEOUT 10.0

struct RSC_USAGE {
    double cpu_time;
    double wss;
    void clear() {
        cpu_time = 0;
        wss = 0;
    }
};

// launch application and get process group ID
//
int launch(const char* distro, const char* cmd) {
    char launch_cmd[256];
    sprintf(launch_cmd, "echo $$; %s\n", cmd);
    int retval = app_wc.run_command(distro, launch_cmd, &proc_handle);
    if (retval) return retval;
    string reply;
    retval = read_from_pipe(wc.out_read, proc_handle, reply, CMD_TIMEOUT, "\n");
    if (retval) return retval;
    pgid = atoi(reply.c_str());
    printf("reply: [%s]\n", reply.c_str());
    printf("pgid: %d\n", pid);
    running = true;
}

// Get app resources usage.
// if processes in group still exist, return cpu time and wss
// else if finish file found, return job_done = true
// else return nonzero
//
int poll_app(int pid, int seqno, RSC_USAGE &ru, bool& job_done) {
    char cmd[256], buf[256];
    sprintf(buf, "EOM %d", seqno);
    string eom = buf;
    sprintf(cmd, "ps u -g %d ; echo 'EOM %d'\n", pid, seqno);
    string reply;
    retval = read_from_pipe(
        wc.out_read, proc_handle, reply, CMD_TIMEOUT, "EOM"
    );
    ru.clear();
    for (string s: split(reply, '\n')) {
        n = sscanf("%f %f", s.c_str(), &cpu_time, &wss);
        if (n == 2) {
            ru.cpu_time += cpu;
            ru.wss += wss;
        }
    }
    return 0;
}

int suspend() {
    char cmd[256];
    sprintf(cmd, "kill STOP %d\n", pgid);
    running = false;
    return write_to_pipe(ctl_wc.out_write, cmd);
}
int resume() {
    char cmd[256];
    sprintf(cmd, "kill CONT %d\n", pgid);
    running = true;
    return write_to_pipe(ctl_wc.out_write, cmd);
}
int abort() {
    char cmd[256];
    sprintf(cmd, "kill KILL %d\n", pgid);
    return write_to_pipe(ctl_wc.out_write, cmd);
}

void poll_client_msgs() {
    BOINC_STATUS status;
    boinc_get_status(&status);
    if (status.no_heartbeat || status.quit_request || status.abort_request) }
        abort();
        exit(0);
    }
    if (status.suspended) {
        if (running) suspend();
    } else {
        if (!running) resume();
    }
}

int main(int argc, char** argv) {
    int retval = app_wc.setup();
    if (retval) {
        fprintf(stderr, "setup() failed: %s\n", boincerror(retval));
        exit(1);
    }
    retval = launch("Ubuntu-22.04", "./main 1");
    if (retval) {
        fprintf(stderr, "launch() failed: %s\n", boincerror(retval));
        exit(1);
    }
    while (1) {
        // poll period for message from client must be 1 sec.
        poll_client_msgs();
        // poll period for app status could be greater, but we'll use 1 sec
        poll_app(ru);
        boinc_report_app_status();
        boinc_sleep(POLL_PERIOD);
        retval = poll(rsc_usage);
    }
    boinc_finish(0);
}
