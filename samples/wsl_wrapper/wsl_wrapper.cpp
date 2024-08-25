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

#include <cstdio>
#include <string>

#include "boinc_win.h"
#include "win_util.h"

using std::string;

int get_pid(string s) {
    return atoi(s.c_str());
}

// if processes in group still exist, return cpu time and wss
// else if finish file found, return job_done = true
// else return nonzero
//
int poll(int pid, int seqno, double& cpu_time, double& wss, bool& job_done) {
    char cmd[256], buf[256];
    sprintf(buf, "EOM %d", seqno);
    string eom = buf;
    sprintf(cmd, "ps u -g %d ; echo 'EOM %d'", pid, seqno);
    return 0;
}

int suspend(int pid) {
    char cmd[256];
    sprintf(cmd, "kill STOP %d", pid);
    return 0;
}
int resume(int pid) {
    char cmd[256];
    sprintf(cmd, "kill CONT %d", pid);
    return 0;
}
int abort(int pid) {
    char cmd[256];
    sprintf(cmd, "kill KILL %d", pid);
    return 0;
}

int main(int argc, char** argv) {
    WSL_CMD wc;
    HANDLE proc_handle;
    int seqno = 1;
    if (wc.setup()) exit(1);
    if (wc.run_command("Ubuntu-22.04", "./main 1", &proc_handle)) exit(1);
    string reply;
    int retval = read_from_pipe(wc.out_read, proc_handle, reply);
    if (retval) exit(1);
    int pid = get_pid(reply);
    printf("pid: %d\n", pid);
    printf("reply: [%s]\n", reply.c_str());
    exit(1);
}
