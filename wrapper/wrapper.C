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
// wrapper program - lets you use non-BOINC apps with BOINC
//
// Handles:
// - suspend/resume/quit/abort
// - reporting CPU time
// - loss of heartbeat from core client
//
// Does NOT handle:
// - checkpointing
// If your app does checkpointing,
// and there's some way to figure out when it's done it,
// this program could be modified to report to the core client.
//
// See http://boinc.berkeley.edu/wrapper.php for details

#include <stdio.h>
#include <vector>
#include <string>
#ifdef _WIN32
#include "boinc_win.h"
#else
#include <unistd.h>
#include <sys/wait.h>
#endif

#include "boinc_api.h"
#include "diagnostics.h"
#include "filesys.h"
#include "parse.h"
#include "util.h"
#include "error_numbers.h"

using std::vector;
using std::string;

struct TASK {
    string application;
    string stdin_filename;
    string stdout_filename;
    string command_line;
#ifdef _WIN32
    HANDLE pid_handle;
    HANDLE thread_handle;
#else
    int pid;
#endif
    int parse(FILE*);
    bool poll(int& status);
    int run();
    void kill();
    void stop();
    void resume();
};

vector<TASK> tasks;

bool app_suspended = false;

int TASK::parse(FILE* f) {
    char tag[256], contents[1024], buf[256];
    while(get_tag(f, tag, contents)) {
        if (!strcmp(tag, "/task")) {
            return 0;
        }
        if (!strcmp(tag, "application")) {
            application = contents;
            continue;
        }
        if (!strcmp(tag, "stdin_filename")) {
            stdin_filename = contents;
            continue;
        }
        if (!strcmp(tag, "stdout_filename")) {
            stdout_filename = contents;
            continue;
        }
        if (!strcmp(tag, "command_line")) {
            command_line = contents;
            continue;
        }
    }
    return ERR_XML_PARSE;
}

int parse_job_file() {
    char tag[256], contents[1024], buf[256];
    boinc_resolve_filename("job.xml", buf, 1024);
    FILE* f = boinc_fopen(buf, "r");
    if (!f) return ERR_FOPEN;
    get_tag(f, tag);
    if (strstr(tag, "?xml")) get_tag(f, tag);
    if (strcmp(tag, "job_desc")) return ERR_XML_PARSE;
    while(get_tag(f, tag, contents)) {
        if (!strcmp(tag, "/job_desc")) {
            return 0;
        }
        if (!strcmp(tag, "task")) {
            TASK task;
            int retval = task.parse(f);
            if (!retval) {
                tasks.push_back(task);
            }
        }
    }
    return ERR_XML_PARSE;
}

// the "state file" might tell us which app we're in the middle of,
// what the starting CPU time is, etc.
//
void parse_state_file() {
}

int TASK::run() {
	FILE* stdout_file;
	FILE* stdin_file;

    boinc_resolve_filename_s(application.c_str(), application);

	// open stdout, stdin if file names are given
	//
	if (stdout_filename != "") {
		stdout_file = freopen(stdout_filename.c_str(), "w", stdout);
		if (!stdout_file) return ERR_FOPEN;
	}
	if (stdin != "") {
		stdin_file = freopen(stdin_filename.c_str(), "w", stdin);
		if (!stdin_file) return ERR_FOPEN;
	}
#ifdef _WIN32

    PROCESS_INFORMATION process_info;
    STARTUPINFO startup_info;
    memset(&process_info, 0, sizeof(process_info));
    memset(&startup_info, 0, sizeof(startup_info));

    // pass std handles to app
    //
    startup_info.dwFlags = STARTF_USESTDHANDLES;
    startup_info.hStdIn = GetStdHandle(STD_IN_HANDLE);
    startup_info.hStdOut = GetStdHandle(STD_OUT_HANDLE);
    startup_info.hStdError = GetStdHandle(STD_ERROR_HANDLE);
             
    if (!CreateProcess(application.c_str(),
        (LPSTR)application.c_str(),
        NULL,
        NULL,
        TRUE,		// bInheritHandles
        CREATE_NO_WINDOW|IDLE_PRIORITY_CLASS,
        NULL,
        NULL,
        &startup_info,
        &process_info
    )) {
        return ERR_EXEC;
    }
    pid_handle = process_info.hProcess;
    thread_handle = process_info.hThread;
#else
    int retval;
    char progname[256], buf[256];
    char* argv[2];
    pid = fork();
    if (pid == -1) {
        boinc_finish(ERR_FORK);
    }
    if (pid == 0) {
        strcpy(buf, application.c_str());
        argv[0] = buf;
        argv[1] = 0;
        retval = execv(buf, argv);
        exit(ERR_EXEC);
    }
#endif
    return 0;
}

bool TASK::poll(int& status) {
#ifdef _WIN32
    unsigned long exit_code;
    if (GetExitCodeProcess(pid_handle, &exit_code)) {
        if (exit_code != STILL_ACTIVE) {
            status = exit_code;
            return true;
        }
    }
#else
    int wpid, stat;
    wpid = waitpid(pid, &stat, WNOHANG);
    if (wpid) {
        status = stat;
        return true;
    }
#endif
    return false;
}

void TASK::kill() {
#ifdef _WIN32
    TerminateProcess(pid_handle, -1);
#else
    ::kill(pid, SIGKILL);
#endif
}

void TASK::stop() {
#ifdef _WIN32
    SuspendThread(thread_handle);
#else
    ::kill(pid, SIGSTOP);
#endif
}

void TASK::resume() {
#ifdef _WIN32
    ResumeThread(thread_handle);
#else
    ::kill(pid, SIGCONT);
#endif
}

void poll_boinc_messages(TASK& task) {
    BOINC_STATUS status;
    boinc_get_status(&status);
    if (status.no_heartbeat) {
        task.kill();
        exit(0);
    }
    if (status.quit_request) {
        task.kill();
        exit(0);
    }
    if (status.abort_request) {
        task.kill();
        exit(0);
    }
    if (status.suspended) {
        if (!app_suspended) {
            task.stop();
            app_suspended = true;
        }
    } else {
        if (app_suspended) {
            task.resume();
            app_suspended = false;
        }
    }
}

double cpu_time() {
#ifdef _WIN32
    FILETIME creation_time, exit_time, kernel_time, user_time;
    ULARGE_INTEGER tKernel, tUser;
    LONGLONG totTime;

    GetProcessTimes(
        pid_handle, &creation_time, &exit_time, &kernel_time, &user_time
    );

    tKernel.LowPart  = kernel_time.dwLowDateTime;
    tKernel.HighPart = kernel_time.dwHighDateTime;
    tUser.LowPart    = user_time.dwLowDateTime;
    tUser.HighPart   = user_time.dwHighDateTime;
    totTime = tKernel.QuadPart + tUser.QuadPart;

    double cpu = totTime / 1.e7;
    return cpu;
#else
    static double t=0, cpu;
    if (t) {
        double now = dtime();
        cpu += now-t;
        t = now;
    } else {
        t = dtime();
    }
    return cpu;
#endif
}

void send_status_message() {
    boinc_report_app_status(cpu_time(), 0, 0);
}

int main(int argc, char** argv) {
    BOINC_OPTIONS options;
    int retval;

    boinc_init_diagnostics(
        BOINC_DIAG_DUMPCALLSTACKENABLED |
        BOINC_DIAG_HEAPCHECKENABLED |
        BOINC_DIAG_MEMORYLEAKCHECKENABLED |
        BOINC_DIAG_TRACETOSTDERR |
        BOINC_DIAG_REDIRECTSTDERR
    );

    memset(&options, 0, sizeof(options));
    options.main_program = true;
    options.check_heartbeat = true;
    options.handle_process_control = true;

    fprintf(stderr, "wrapper: starting\n");
    boinc_init_options(&options);
    retval = parse_job_file();
    if (retval) {
        fprintf(stderr, "can't parse job file: %d\n", retval);
        boinc_finish(retval);
    }
    if (tasks.size() != 1) {
        fprintf(stderr, "multiple tasks not supported\n");
        boinc_finish(1);
    }

    parse_state_file();

    TASK& task = tasks[0];
    retval = task.run();
    if (retval) {
        fprintf(stderr, "can't run app: %d\n", retval);
        boinc_finish(retval);
    }
    while(1) {
        int status;
        if (task.poll(status)) {
            boinc_finish(status);
        }
        poll_boinc_messages(task);
        send_status_message();
        boinc_sleep(1.);
    }
}

#ifdef _WIN32

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR Args, int WinMode) {
    LPSTR command_line;
    char* argv[100];
    int argc;

    command_line = GetCommandLine();
    argc = parse_command_line( command_line, argv );
    return main(argc, argv);
}
#endif
