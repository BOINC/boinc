// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2008 University of California
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

// wrapper.C
// wrapper program - lets you use non-BOINC apps with BOINC
//
// Handles:
// - suspend/resume/quit/abort
// - reporting CPU time
// - loss of heartbeat from core client
// - checkpointing
//      (at the level of task; or potentially within task)
//
// See http://boinc.berkeley.edu/wrapper.php for details
// Contributor: Andrew J. Younge (ajy4490@umiacs.umd.edu)

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

#define JOB_FILENAME "job.xml"
#define CHECKPOINT_FILENAME "wrapper_checkpoint.txt"

#define POLL_PERIOD 1.0

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
    int parse(XML_PARSER&);
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

vector<TASK> tasks;
APP_INIT_DATA aid;
bool graphics = false;

int TASK::parse(XML_PARSER& xp) {
    char tag[1024], buf[8192], buf2[8192];
    bool is_tag;

    weight = 1;
    final_cpu_time = 0;
    stat_first = true;
    while (!xp.get(tag, sizeof(tag), is_tag)) {
        if (!is_tag) {
            fprintf(stderr, "%s TASK::parse(): unexpected text %s\n",
                boinc_msg_prefix(), tag
            );
            continue;
        }
        if (!strcmp(tag, "/task")) {
            return 0;
        }
        else if (xp.parse_string(tag, "application", application)) continue;
        else if (xp.parse_string(tag, "stdin_filename", stdin_filename)) continue;
        else if (xp.parse_string(tag, "stdout_filename", stdout_filename)) continue;
        else if (xp.parse_string(tag, "stderr_filename", stderr_filename)) continue;
        else if (xp.parse_str(tag, "command_line", buf, sizeof(buf))) {
            while (1) {
                char* p = strstr(buf, "$PROJECT_DIR");
                if (!p) break;
                strcpy(buf2, p+strlen("$PROJECT_DIR"));
                strcpy(p, aid.project_dir);
                strcat(p, buf2);
            }
            command_line = buf;
            continue;
        }
        else if (xp.parse_string(tag, "checkpoint_filename", checkpoint_filename)) continue;
        else if (xp.parse_string(tag, "fraction_done_filename", fraction_done_filename)) continue;
        else if (xp.parse_double(tag, "weight", weight)) continue;
    }
    return ERR_XML_PARSE;
}

int parse_job_file() {
    MIOFILE mf;
    char tag[1024], buf[256];
    bool is_tag;

    boinc_resolve_filename(JOB_FILENAME, buf, 1024);
    FILE* f = boinc_fopen(buf, "r");
    if (!f) {
        fprintf(stderr, "%s can't open job file %s\n", boinc_msg_prefix(), buf);
        return ERR_FOPEN;
    }
    mf.init_file(f);
    XML_PARSER xp(&mf);

    if (!xp.parse_start("job_desc")) return ERR_XML_PARSE;
    while (!xp.get(tag, sizeof(tag), is_tag)) {
        if (!is_tag) {
            fprintf(stderr, "%s SCHED_CONFIG::parse(): unexpected text %s\n",
                boinc_msg_prefix(), tag
            );
            continue;
        }
        if (!strcmp(tag, "/job_desc")) {
            fclose(f);
            return 0;
        }
        if (!strcmp(tag, "task")) {
            TASK task;
            int retval = task.parse(xp);
            if (!retval) {
                tasks.push_back(task);
            }
        }
    }
    fclose(f);
    return ERR_XML_PARSE;
}

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
        boinc_msg_prefix(), app_path, command_line.c_str()
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
        windows_error_string(error_msg, sizeof(error_msg));
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
    TerminateProcess(pid_handle, -1);
#else
    ::kill(pid, SIGKILL);
#endif
}

void TASK::stop() {
#ifdef _WIN32
    suspend_or_resume_threads(pid, 0, false);
#else
    ::kill(pid, SIGSTOP);
#endif
    suspended = true;
}

void TASK::resume() {
#ifdef _WIN32
    suspend_or_resume_threads(pid, 0, true);
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
    TASK& task, double frac_done, double checkpoint_cpu_time
) {
    double current_cpu_time = task.starting_cpu + task.cpu_time();
    boinc_report_app_status(
        current_cpu_time,
        checkpoint_cpu_time,
        frac_done
    );
}

// Support for multiple tasks.
// We keep a checkpoint file that says how many tasks we've completed
// and how much CPU time has been used so far
//
void write_checkpoint(int ntasks_completed, double cpu) {
    FILE* f = fopen(CHECKPOINT_FILENAME, "w");
    if (!f) return;
    fprintf(f, "%d %f\n", ntasks_completed, cpu);
    fclose(f);
}

void read_checkpoint(int& ntasks_completed, double& cpu) {
    int nt;
    double c;

    ntasks_completed = 0;
    cpu = 0;
    FILE* f = fopen(CHECKPOINT_FILENAME, "r");
    if (!f) return;
    int n = fscanf(f, "%d %lf", &nt, &c);
    fclose(f);
    if (n != 2) return;
    ntasks_completed = nt;
    cpu = c;
}

int main(int argc, char** argv) {
    BOINC_OPTIONS options;
    int retval, ntasks_completed;
    unsigned int i;
    double total_weight=0, weight_completed=0;
    double checkpoint_cpu_time;
        // overall CPU time at last checkpoint

    for (i=1; i<(unsigned int)argc; i++) {
        if (!strcmp(argv[i], "--graphics")) {
            graphics = true;
        }
    }

    memset(&options, 0, sizeof(options));
    options.main_program = true;
    options.check_heartbeat = true;
    options.handle_process_control = true;
    if (graphics) {
        options.backwards_compatible_graphics = true;
    }

    boinc_init_options(&options);
    fprintf(stderr, "wrapper: starting\n");

    boinc_get_init_data(aid);

    retval = parse_job_file();
    if (retval) {
        fprintf(stderr, "can't parse job file: %d\n", retval);
        boinc_finish(retval);
    }

    read_checkpoint(ntasks_completed, checkpoint_cpu_time);
    if (ntasks_completed > (int)tasks.size()) {
        fprintf(stderr,
            "Checkpoint file: ntasks_completed too large: %d > %d\n",
            ntasks_completed, (int)tasks.size()
        );
        boinc_finish(1);
    }
    for (i=0; i<tasks.size(); i++) {
        total_weight += tasks[i].weight;
    }
    for (i=0; i<tasks.size(); i++) {
        TASK& task = tasks[i];
        if ((int)i<ntasks_completed) {
            weight_completed += task.weight;
            continue;
        }
        double frac_done = weight_completed/total_weight;

        task.starting_cpu = checkpoint_cpu_time;
        retval = task.run(argc, argv);
        if (retval) {
            boinc_finish(retval);
        }
        while (1) {
            int status;
            if (task.poll(status)) {
                if (status) {
                    fprintf(stderr, "app exit status: 0x%x\n", status);
                    // On Unix, if the app is non-executable,
                    // the child status will be 0x6c00.
                    // If we return this the client will treat it
                    // as recoverable, and restart us.
                    // We don't want this, so return an 8-bit error code.
                    //
                    boinc_finish(EXIT_CHILD_FAILED);
                }
                break;
            }
            poll_boinc_messages(task);
            double task_fraction_done = task.fraction_done();
            double delta = task_fraction_done*task.weight/total_weight;
            send_status_message(task, frac_done+delta, checkpoint_cpu_time);
            if (task.has_checkpointed()) {
                checkpoint_cpu_time = task.starting_cpu + task.cpu_time();
                write_checkpoint(i, checkpoint_cpu_time);
            }
            boinc_sleep(POLL_PERIOD);
        }
        checkpoint_cpu_time = task.starting_cpu + task.final_cpu_time;
        write_checkpoint(i+1, checkpoint_cpu_time);
        weight_completed += task.weight;
    }
    boinc_finish(0);
}

#ifdef _WIN32

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR Args, int WinMode) {
    LPSTR command_line;
    char* argv[100];
    int argc;

    command_line = GetCommandLine();
    argc = parse_command_line(command_line, argv);
    return main(argc, argv);
}
#endif
