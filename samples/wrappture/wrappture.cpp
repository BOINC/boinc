// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2010 University of California
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

// A wrapper for Rappture applications.
// Similar to the standard wrapper except:
//
// - Doesn't contain main(); must be called from a main program
//      that parses the Rappture control file
// - parse the app's stdout (in a separate thread);
//      look for "progress markers", convert to fraction done

#include <stdio.h>
#include <vector>
#include <string>
#ifdef _WIN32
#include "boinc_win.h"
#include "win_util.h"
#else
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/resource.h>
#include <pthread.h>
#include <unistd.h>
#include "procinfo.h"
#endif

#include "boinc_api.h"
#include "diagnostics.h"
#include "filesys.h"
#include "parse.h"
#include "str_util.h"
#include "str_replace.h"
#include "util.h"
#include "error_numbers.h"

#include "wrappture.h"

#define POLL_PERIOD 1.0

#ifdef _WIN32
extern int suspend_or_resume_threads(
    std::vector<int> pids, DWORD threadid, bool resume, bool check_exempt
);
#endif

using std::vector;
using std::string;

struct TASK {
    string application;
    string stdin_filename;
    string stdout_filename;
    string stderr_filename;
    string checkpoint_filename;
        // name of task's checkpoint file, if any
    string fraction_done_filename;
        // name of file where app will write its fraction done
    string command_line;
    double weight;
        // contribution of this task to overall fraction done
    double final_cpu_time;
    double starting_cpu;
        // how much CPU time was used by tasks before this in the job file
    bool suspended;
    double wall_cpu_time;
        // for estimating CPU time on Win98/ME and Mac
#ifdef _WIN32
    HANDLE pid_handle;
    DWORD pid;
    HANDLE thread_handle;
    struct _stat last_stat;    // mod time of checkpoint file
#else
    int pid;
    struct stat last_stat;
#endif
    bool stat_first;
    bool poll(int& status);
    int run(int argc, char** argv);
    void kill();
    void stop();
    void resume();
	double cpu_time();
    inline bool has_checkpointed() {
        bool changed = false;
        if (checkpoint_filename.size() == 0) return false;
        struct stat new_stat;
        int retval = stat(checkpoint_filename.c_str(), &new_stat);
        if (retval) return false;
        if (!stat_first && new_stat.st_mtime != last_stat.st_mtime) {
            changed = true;
        }
        stat_first = false;
        last_stat.st_mtime = new_stat.st_mtime;
        return changed;
    }
    inline double fraction_done() {
        if (fraction_done_filename.size() == 0) return 0;
        FILE* f = fopen(fraction_done_filename.c_str(), "r");
        if (!f) return 0;
        double frac;
        int n = fscanf(f, "%lf", &frac);
        fclose(f);
        if (n != 1) return 0;
        if (frac < 0) return 0;
        if (frac > 1) return 1;
        return frac;
    }
};

APP_INIT_DATA aid;
double fraction_done, checkpoint_cpu_time;

#ifdef _WIN32
// CreateProcess() takes HANDLEs for the stdin/stdout.
// We need to use CreateFile() to get them.  Ugh.
//
HANDLE win_fopen(const char* path, const char* mode) {
	SECURITY_ATTRIBUTES sa;
	memset(&sa, 0, sizeof(sa));
	sa.nLength = sizeof(sa);
	sa.bInheritHandle = TRUE;

	if (!strcmp(mode, "r")) {
		return CreateFile(
			path,
			GENERIC_READ,
			FILE_SHARE_READ,
			&sa,
			OPEN_EXISTING,
			0, 0
		);
	} else if (!strcmp(mode, "w")) {
		return CreateFile(
			path,
			GENERIC_WRITE,
			FILE_SHARE_WRITE,
			&sa,
			OPEN_ALWAYS,
			0, 0
		);
	} else if (!strcmp(mode, "a")) {
		HANDLE hAppend = CreateFile(
			path,
			GENERIC_WRITE,
			FILE_SHARE_WRITE,
			&sa,
			OPEN_ALWAYS,
			0, 0
		);
        SetFilePointer(hAppend, 0, NULL, FILE_END);
        return hAppend;
	} else {
		return 0;
	}
}
#endif

void slash_to_backslash(char* p) {
    while (1) {
        char* q = strchr(p, '/');
        if (!q) break;
        *q = '\\';
    }
}

int TASK::run(int argct, char** argvt) {
    string stdout_path, stdin_path, stderr_path;
    char app_path[1024], buf[256];

    if (checkpoint_filename.size()) {
        boinc_delete_file(checkpoint_filename.c_str());
    }
    if (fraction_done_filename.size()) {
        boinc_delete_file(fraction_done_filename.c_str());
    }

    strcpy(buf, application.c_str());
    char* p = strstr(buf, "$PROJECT_DIR");
    if (p) {
        p += strlen("$PROJECT_DIR");
        sprintf(app_path, "%s%s", aid.project_dir, p);
    } else {
        boinc_resolve_filename(buf, app_path, sizeof(app_path));
    }

    // Append wrapper's command-line arguments to those in the job file.
    //
    for (int i=1; i<argct; i++){
        command_line += string(" ");
        command_line += argvt[i];
    }

    fprintf(stderr, "%s wrapper: running %s (%s)\n",
        boinc_msg_prefix(buf, sizeof(buf)), app_path, command_line.c_str()
    );

#ifdef _WIN32
    PROCESS_INFORMATION process_info;
    STARTUPINFO startup_info;
    string command;

    slash_to_backslash(app_path);
    memset(&process_info, 0, sizeof(process_info));
    memset(&startup_info, 0, sizeof(startup_info));
    command = string("\"") + app_path + string("\" ") + command_line;

    // pass std handles to app
    //
    startup_info.dwFlags = STARTF_USESTDHANDLES;
	if (stdout_filename != "") {
		boinc_resolve_filename_s(stdout_filename.c_str(), stdout_path);
		startup_info.hStdOutput = win_fopen(stdout_path.c_str(), "a");
	}
	if (stdin_filename != "") {
		boinc_resolve_filename_s(stdin_filename.c_str(), stdin_path);
		startup_info.hStdInput = win_fopen(stdin_path.c_str(), "r");
	}
    if (stderr_filename != "") {
        boinc_resolve_filename_s(stderr_filename.c_str(), stderr_path);
        startup_info.hStdError = win_fopen(stderr_path.c_str(), "a");
    } else {
        startup_info.hStdError = win_fopen(STDERR_FILE, "a");
    }

    if (!CreateProcess(
        app_path,
        (LPSTR)command.c_str(),
        NULL,
        NULL,
        TRUE,		// bInheritHandles
        CREATE_NO_WINDOW|IDLE_PRIORITY_CLASS,
        NULL,
        NULL,
        &startup_info,
        &process_info
    )) {
        char error_msg[1024];
        windows_format_error_string(GetLastError(), error_msg, sizeof(error_msg));
        fprintf(stderr, "can't run app: %s\n", error_msg);
        return ERR_EXEC;
    }
    pid_handle = process_info.hProcess;
    pid = process_info.dwProcessId;
    thread_handle = process_info.hThread;
    SetThreadPriority(thread_handle, THREAD_PRIORITY_IDLE);
#else
    int retval, argc;
    char progname[256];
    char* argv[256];
    char arglist[4096];
	FILE* stdout_file;
	FILE* stdin_file;
	FILE* stderr_file;

    pid = fork();
    if (pid == -1) {
        perror("fork(): ");
        return ERR_FORK;
    }
    if (pid == 0) {
		// we're in the child process here
		//
		// open stdout, stdin if file names are given
		// NOTE: if the application is restartable,
		// we should deal with atomicity somehow
		//
		if (stdout_filename != "") {
			boinc_resolve_filename_s(stdout_filename.c_str(), stdout_path);
			stdout_file = freopen(stdout_path.c_str(), "a", stdout);
			if (!stdout_file) return ERR_FOPEN;
		}
		if (stdin_filename != "") {
			boinc_resolve_filename_s(stdin_filename.c_str(), stdin_path);
			stdin_file = freopen(stdin_path.c_str(), "r", stdin);
			if (!stdin_file) return ERR_FOPEN;
		}
        if (stderr_filename != "") {
            boinc_resolve_filename_s(stderr_filename.c_str(), stderr_path);
            stderr_file = freopen(stderr_path.c_str(), "a", stderr);
            if (!stderr_file) return ERR_FOPEN;
        }

		// construct argv
        // TODO: use malloc instead of stack var
        //
        argv[0] = app_path;
        strlcpy(arglist, command_line.c_str(), sizeof(arglist));
        argc = parse_command_line(arglist, argv+1);
        setpriority(PRIO_PROCESS, 0, PROCESS_IDLE_PRIORITY);
        retval = execv(app_path, argv);
        perror("execv() failed: ");
        exit(ERR_EXEC);
    }
#endif
    wall_cpu_time = 0;
    suspended = false;
    return 0;
}

bool TASK::poll(int& status) {
    if (!suspended) wall_cpu_time += POLL_PERIOD;
#ifdef _WIN32
    unsigned long exit_code;
    if (GetExitCodeProcess(pid_handle, &exit_code)) {
        if (exit_code != STILL_ACTIVE) {
            status = exit_code;
            final_cpu_time = cpu_time();
            return true;
        }
    }
#else
    int wpid, stat;
    struct rusage ru;

    wpid = wait4(pid, &status, WNOHANG, &ru);
    if (wpid) {
        final_cpu_time = (float)ru.ru_utime.tv_sec + ((float)ru.ru_utime.tv_usec)/1e+6;
        return true;
    }
#endif
    return false;
}

void TASK::kill() {
#ifdef _WIN32
    TerminateProcess(pid_handle, static_cast<UINT>(-1));
#else
    ::kill(pid, SIGKILL);
#endif
}

void TASK::stop() {
#ifdef _WIN32
    std::vector<int> pids;
    pids.push_back(pid);
    suspend_or_resume_threads(pids, 0, false, true);
#else
    ::kill(pid, SIGSTOP);
#endif
    suspended = true;
}

void TASK::resume() {
#ifdef _WIN32
    std::vector<int> pids;
    pids.push_back(pid);
    suspend_or_resume_threads(pids, 0, true, true);
#else
    ::kill(pid, SIGCONT);
#endif
    suspended = false;
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
        if (!task.suspended) {
            task.stop();
        }
    } else {
        if (task.suspended) {
            task.resume();
        }
    }
}

double TASK::cpu_time() {
#ifdef _WIN32
    double x;
    int retval = boinc_process_cpu_time(pid_handle, x);
    if (retval) return wall_cpu_time;
    return x;
#elif defined(__APPLE__)
    // There's no easy way to get another process's CPU time in Mac OS X
    //
    return wall_cpu_time;
#else
    return linux_cpu_time(pid);
#endif
}

void send_status_message(
    TASK& task, double frac_done, double checkpoint_cpu_time_value
) {
    double current_cpu_time = task.starting_cpu + task.cpu_time();
    boinc_report_app_status(
        current_cpu_time,
        checkpoint_cpu_time_value,
        frac_done
    );
}

#define PROGRESS_MARKER "=RAPPTURE-PROGRESS=>"

// the following runs in a thread and parses the app's stdout file,
// looking for progress tags
//
#ifdef _WIN32
DWORD WINAPI parse_app_stdout(void*) {
#else
static void block_sigalrm() {
    sigset_t mask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGALRM);
    pthread_sigmask(SIG_BLOCK, &mask, NULL);
}

// we create a thread to parse the stdout from the worker app
//
static void* parse_app_stdout(void*) {
    block_sigalrm();
#endif
    char buf[1024];
    FILE* f;
    while (1) {
        f = boinc_fopen("rappture_stdout.txt", "r");
        if (f) break;
        boinc_sleep(1.);
    }

    while (1) {
        if (fgets(buf, sizeof(buf), f)) {
            if (strstr(buf, PROGRESS_MARKER)) {
                fraction_done = atof(buf+strlen(PROGRESS_MARKER))/100;
            }
        } else {
            boinc_sleep(1.);
        }
    }
}

int create_parser_thread() {
#ifdef _WIN32
    DWORD parser_thread_id;
    if (!CreateThread(NULL, 0, parse_app_stdout, 0, 0, &parser_thread_id)) {
        return ERR_THREAD;
    }
#else
    pthread_t parser_thread_handle;
    pthread_attr_t thread_attrs;
    pthread_attr_init(&thread_attrs);
    pthread_attr_setstacksize(&thread_attrs, 16384);
    int retval = pthread_create(
        &parser_thread_handle, &thread_attrs, parse_app_stdout, NULL
    );
    if (retval) {
        return ERR_THREAD;
    }
#endif
    return 0;
}

int boinc_run_rappture_app(const char* program, const char* cmdline) {
    TASK task;
    int retval;
    BOINC_OPTIONS options;

    memset(&options, 0, sizeof(options));
    options.main_program = true;
    options.check_heartbeat = true;
    options.handle_process_control = true;
    boinc_init_options(&options);

    task.application = program;
    task.command_line = cmdline;
    task.stdout_filename = "rappture_stdout.txt";

    retval = task.run(0, 0);
    create_parser_thread();
    while (1) {
        int status;
        if (task.poll(status)) {
            if (status) {
                boinc_finish(EXIT_CHILD_FAILED);
            }
            break;
        }
        poll_boinc_messages(task);
        send_status_message(task, fraction_done, checkpoint_cpu_time);
        boinc_sleep(POLL_PERIOD);
    }
    return 0;
}
