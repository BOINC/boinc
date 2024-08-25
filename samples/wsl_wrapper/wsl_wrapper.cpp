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

// read from the pipe until
// - get a line with 'EOM seqno'
// - the given process (if any) doesn't exist
// - the given timeout (if any) is reached
// - a read fails
//
// return error if any of last 3
int read_to_eom(HANDLE pipe, int seqno, HANDLE proc_handle, double timeout, string& out) {
    char buf[1024];
    DWORD avail, nread, exit_code;
    bool ret;
    sprintf(buf, "EOM %d", seqno);
    string eom = buf;
    double elapsed = 0;
    out = "";
    while (1) {
        PeekNamedPipe(pipe, NULL, 0, NULL, &avail, NULL);
        if (avail) {
            ret = ReadFile(pipe, buf, sizeof(buf) - 1, &nread, NULL);
            if (!ret) return -1;
            buf[nread] = 0;
            out += buf;
            if (out.find(eom) != std::string::npos) {
                return 0;
            }
        } else {
            Sleep(200);
            elapsed += .2;
            if (timeout && elapsed > timeout) {
                return -1;
            }
            if (proc_handle) {
                ret = GetExitCodeProcess(proc_handle, &exit_code);
                if (!ret) return -1;
                if (exit_code != STILL_ACTIVE) return -1;
            }
        }
    }
}

int get_pid(string s) {
    return atoi(s.c_str());
}

// if processes in group still exist, return cpu time and wss
// else if finish file found, return job_done = true
// else return nonzero
//
int poll(int pid, int seqno, double& cpu_time, double& wss, bool& job_done) {
    char cmd[256];
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
    int retval = read_to_eom(wc.out_read, seqno, proc_handle, 0, reply);
    if (retval) exit(1);
    int pid = get_pid(reply);
    printf("pid: %d\n", pid);
    printf("reply: [%s]\n", reply.c_str());
    exit(1);
}
