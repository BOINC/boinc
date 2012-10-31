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
// See http://boinc.berkeley.edu/trac/wiki/WrapperApp for details
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
#include <sys/time.h>
#include <sys/resource.h>
#include <unistd.h>
#endif

#include "boinc_api.h"
#include "diagnostics.h"
#include "error_numbers.h"
#include "filesys.h"
#include "parse.h"
#include "proc_control.h"
#include "procinfo.h"
#include "str_util.h"
#include "str_replace.h"
#include "util.h"

#define JOB_FILENAME "job.xml"
#define CHECKPOINT_FILENAME "wrapper_checkpoint.txt"

#define POLL_PERIOD 1.0

using std::vector;
using std::string;
int nthreads = 1;

struct TASK {
    string application;
    string exec_dir;
        // optional execution directory;
        // macro-substituted for $PROJECT_DIR and $NTHREADS
    vector<string> vsetenv;
        // vector of strings for environment variables 
        // macro-substituted
    string stdin_filename;
    string stdout_filename;
    string stderr_filename;
    string checkpoint_filename;
        // name of task's checkpoint file, if any
    string fraction_done_filename;
        // name of file where app will write its fraction done
    string command_line;
        // macro-substituted
    double weight;
        // contribution of this task to overall fraction done
    bool is_daemon;
    bool append_cmdline_args;
    bool multi_process;

    // dynamic stuff follows
    double current_cpu_time;
        // most recently measure CPU time of this task
    double final_cpu_time;
        // final CPU time of this task
    double starting_cpu;
        // how much CPU time was used by tasks before this one
    bool suspended;
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

        // read the last line of the file
        //
        fseek(f, -32, SEEK_END);
        double temp, frac = 0;
        while (!feof(f)) {
            char buf[256];
            char* p = fgets(buf, 256, f);
            if (p == NULL) break;
            int n = sscanf(buf, "%lf", &temp);
            if (n == 1) frac = temp;
        }
        fclose(f);
        if (frac < 0) return 0;
        if (frac > 1) return 1;
        return frac;
    }

#ifdef _WIN32
    // Windows uses a "null-terminated sequence of null-terminated strings"
    // to represent env vars.
    // I guess arg/argv didn't cut it for them.
    //
    void set_up_env_vars(char** env_vars, const int nvars) {
        int bufsize = 0;
        int len = 0;
        for (int j = 0; j < nvars; j++) {
             bufsize += (1 + vsetenv[j].length());
        }
        bufsize++; // add a final byte for array null ptr
        *env_vars = new char[bufsize];
        memset(*env_vars, 0, sizeof(char) * bufsize);
        char* p = *env_vars;
        // copy each env string to a buffer for the process
        for (vector<string>::iterator it = vsetenv.begin();
            it != vsetenv.end() && len < bufsize-1;
            it++
        ) {
            strncpy(p, it->c_str(), it->length());
            len = strlen(p);
            p += len + 1; // move pointer ahead
        }
    }
#else
    void set_up_env_vars(char*** env_vars, const int nvars) {
        *env_vars = new char*[nvars+1];
            // need one more than the # of vars, for a NULL ptr at the end
        memset(*env_vars, 0x00, sizeof(char*) * (nvars+1));
        // get all environment vars for this task
        for (int i = 0; i < nvars; i++) {
            (*env_vars)[i] = const_cast<char*>(vsetenv[i].c_str());
        }
    }
#endif
};

vector<TASK> tasks;
vector<TASK> daemons;
APP_INIT_DATA aid;

// replace s1 with s2
//
void str_replace_all(char* buf, const char* s1, const char* s2) {
    char buf2[64000];
    while (1) {
        char* p = strstr(buf, s1);
        if (!p) break;
        strcpy(buf2, p+strlen(s1));
        strcpy(p, s2);
        strcat(p, buf2);
    }
}

// macro-substitute strings from job.xml
// $PROJECT_DIR -> project directory
// $NTHREADS --> --nthreads arg if present, else 1
//
void macro_substitute(char* buf) {
    const char* pd = strlen(aid.project_dir)?aid.project_dir:".";
    str_replace_all(buf, "$PROJECT_DIR", pd);
    char nt[256];
    sprintf(nt, "%d", nthreads);
    str_replace_all(buf, "$NTHREADS", nt);
}

int TASK::parse(XML_PARSER& xp) {
    char buf[8192];

    weight = 1;
    current_cpu_time = 0;
    final_cpu_time = 0;
    stat_first = true;
    pid = 0;
    is_daemon = false;
    multi_process = false;
    append_cmdline_args = false;

    while (!xp.get_tag()) {
        if (!xp.is_tag) {
            fprintf(stderr, "%s TASK::parse(): unexpected text %s\n",
                boinc_msg_prefix(buf, sizeof(buf)), xp.parsed_tag
            );
            continue;
        }
        if (xp.match_tag("/task")) {
            return 0;
        }
        else if (xp.parse_string("application", application)) continue;
        else if (xp.parse_str("exec_dir", buf, sizeof(buf))) {
            macro_substitute(buf);
            exec_dir = buf;
            continue;  
        }
        else if (xp.parse_str("setenv", buf, sizeof(buf))) {
            macro_substitute(buf);
            vsetenv.push_back(buf);
            continue;
        }
        else if (xp.parse_string("stdin_filename", stdin_filename)) continue;
        else if (xp.parse_string("stdout_filename", stdout_filename)) continue;
        else if (xp.parse_string("stderr_filename", stderr_filename)) continue;
        else if (xp.parse_str("command_line", buf, sizeof(buf))) {
            macro_substitute(buf);
            command_line = buf;
            continue;
        }
        else if (xp.parse_string("checkpoint_filename", checkpoint_filename)) continue;
        else if (xp.parse_string("fraction_done_filename", fraction_done_filename)) continue;
        else if (xp.parse_double("weight", weight)) continue;
        else if (xp.parse_bool("daemon", is_daemon)) continue;
        else if (xp.parse_bool("multi_process", multi_process)) continue;
        else if (xp.parse_bool("append_cmdline_args", append_cmdline_args)) continue;
    }
    return ERR_XML_PARSE;
}

int parse_job_file() {
    MIOFILE mf;
    char buf[256], buf2[256];

    boinc_resolve_filename(JOB_FILENAME, buf, 1024);
    FILE* f = boinc_fopen(buf, "r");
    if (!f) {
        fprintf(stderr,
            "%s can't open job file %s\n",
            boinc_msg_prefix(buf2, sizeof(buf2)), buf
        );
        return ERR_FOPEN;
    }
    mf.init_file(f);
    XML_PARSER xp(&mf);

    if (!xp.parse_start("job_desc")) return ERR_XML_PARSE;
    while (!xp.get_tag()) {
        if (!xp.is_tag) {
            fprintf(stderr,
                "%s unexpected text in job.xml: %s\n",
                boinc_msg_prefix(buf2, sizeof(buf2)), xp.parsed_tag
            );
            continue;
        }
        if (xp.match_tag("/job_desc")) {
            fclose(f);
            return 0;
        }
        if (xp.match_tag("task")) {
            TASK task;
            int retval = task.parse(xp);
            if (!retval) {
                if (task.is_daemon) {
                    daemons.push_back(task);
                } else {
                    tasks.push_back(task);
                }
            }
            continue;
        } else {
            fprintf(stderr,
                "%s unexpected tag in job.xml: %s\n",
                boinc_msg_prefix(buf2, sizeof(buf2)), xp.parsed_tag
            );
        }
    }
    fclose(f);
    return ERR_XML_PARSE;
}

int start_daemons(int argc, char** argv) {
    for (unsigned int i=0; i<daemons.size(); i++) {
        TASK& task = daemons[i];
        int retval = task.run(argc, argv);
        if (retval) return retval;
    }
    return 0;
}

void kill_daemons() {
    vector<int> daemon_pids;
    for (unsigned int i=0; i<daemons.size(); i++) {
        TASK& task = daemons[i];
        if (task.pid) {
            daemon_pids.push_back(task.pid);
        }
    }
    kill_all(daemon_pids);
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

    // Optionally append wrapper's command-line arguments
    // to those in the job file.
    //
    if (append_cmdline_args) {
        for (int i=1; i<argct; i++){
            command_line += string(" ");
            command_line += argvt[i];
        }
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

    // setup environment vars if needed
    //
    int nvars = vsetenv.size();
    char* env_vars = NULL;
    if (nvars > 0) {
        set_up_env_vars(&env_vars, nvars);
    }

    BOOL success;
    if (ends_with((string)app_path, ".bat")) {
        char cmd[1024];
        sprintf(cmd, "cmd.exe /c %s", command.c_str());
        success = CreateProcess(
            "cmd.exe",
            (LPSTR)cmd,
            NULL,
            NULL,
            TRUE,        // bInheritHandles
            CREATE_NO_WINDOW|IDLE_PRIORITY_CLASS,
            (LPVOID) env_vars,
            exec_dir.empty()?NULL:exec_dir.c_str(),
            &startup_info,
            &process_info
        );
    } else {
        success = CreateProcess(
            app_path,
            (LPSTR)command.c_str(),
            NULL,
            NULL,
            TRUE,        // bInheritHandles
            CREATE_NO_WINDOW|IDLE_PRIORITY_CLASS,
            (LPVOID) env_vars,
            exec_dir.empty()?NULL:exec_dir.c_str(),
            &startup_info,
            &process_info
        );
    }
    if (!success) {
        char error_msg[1024];
        windows_error_string(error_msg, sizeof(error_msg));
        fprintf(stderr, "can't run app: %s\n", error_msg);
        if (env_vars) delete [] env_vars;
        return ERR_EXEC;
    }
    if (env_vars) delete [] env_vars;
    pid_handle = process_info.hProcess;
    pid = process_info.dwProcessId;
    thread_handle = process_info.hThread;
    SetThreadPriority(thread_handle, THREAD_PRIORITY_IDLE);
#else
    int retval;
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
            if (!stdout_file) {
                fprintf(stderr, "Can't open %s for stdout; exiting\n", stdout_path.c_str());
                return ERR_FOPEN;
            }
        }
        if (stdin_filename != "") {
            boinc_resolve_filename_s(stdin_filename.c_str(), stdin_path);
            stdin_file = freopen(stdin_path.c_str(), "r", stdin);
            if (!stdin_file) {
                fprintf(stderr, "Can't open %s for stdin; exiting\n", stdin_path.c_str());
                return ERR_FOPEN;
            }
        }
        if (stderr_filename != "") {
            boinc_resolve_filename_s(stderr_filename.c_str(), stderr_path);
            stderr_file = freopen(stderr_path.c_str(), "a", stderr);
            if (!stderr_file) {
                fprintf(stderr, "Can't open %s for stderr; exiting\n", stderr_path.c_str());
                return ERR_FOPEN;
            }
        }

        // construct argv
        // TODO: use malloc instead of stack var
        //
        argv[0] = app_path;
        strlcpy(arglist, command_line.c_str(), sizeof(arglist));
        parse_command_line(arglist, argv+1);
        setpriority(PRIO_PROCESS, 0, PROCESS_IDLE_PRIORITY);
        if (!exec_dir.empty()) {
            retval = chdir(exec_dir.c_str());
            if (!retval) {
                fprintf(stderr, "chdir() to %s failed\n", exec_dir.c_str());
                exit(1);
            }
        }

        // setup environment variables (if any)
        //
        const int nvars = vsetenv.size();
        char** env_vars = NULL;
        if (nvars > 0) {
            set_up_env_vars(&env_vars, nvars);
            retval = execve(app_path, argv, env_vars);
        } else {
            retval = execv(app_path, argv);
        }
        perror("execv() failed: ");
        exit(ERR_EXEC);
    }  // pid = 0 i.e. child proc of the fork
#endif
    suspended = false;
    return 0;
}

bool TASK::poll(int& status) {
#ifdef _WIN32
    unsigned long exit_code;
    if (GetExitCodeProcess(pid_handle, &exit_code)) {
        if (exit_code != STILL_ACTIVE) {
            status = exit_code;
            final_cpu_time = cpu_time();
            if (final_cpu_time < current_cpu_time) {
                final_cpu_time = current_cpu_time;
            }
            return true;
        }
    }
#else
    int wpid;
    struct rusage ru;

    wpid = wait4(pid, &status, WNOHANG, &ru);
    if (wpid) {
        getrusage(RUSAGE_CHILDREN, &ru);
        final_cpu_time = (float)ru.ru_utime.tv_sec + ((float)ru.ru_utime.tv_usec)/1e+6;
        if (final_cpu_time < current_cpu_time) {
            final_cpu_time = current_cpu_time;
        }
        return true;
    }
#endif
    return false;
}

// kill this task (gracefully if possible) and any other subprocesses
//
void TASK::kill() {
    kill_daemons();
#ifdef _WIN32
    kill_descendants();
#else
    kill_descendants(pid);
#endif
}

void TASK::stop() {
    if (multi_process) {
        suspend_or_resume_descendants(0, false);
    } else {
        suspend_or_resume_process(pid, false);
    }
    suspended = true;
}

void TASK::resume() {
    if (multi_process) {
        suspend_or_resume_descendants(0, true);
    } else {
        suspend_or_resume_process(pid, true);
    }
    suspended = false;
}

// Get the CPU time of the app while it's running.
// This totals the CPU time of all the descendant processes,
// so it shouldn't be called too frequently.
//
double TASK::cpu_time() {
    current_cpu_time = process_tree_cpu_time(pid);
    return current_cpu_time;
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

// Support for multiple tasks.
// We keep a checkpoint file that says how many tasks we've completed
// and how much CPU time has been used so far
//
void write_checkpoint(int ntasks_completed, double cpu) {
    boinc_begin_critical_section();
    FILE* f = fopen(CHECKPOINT_FILENAME, "w");
    if (!f) return;
    fprintf(f, "%d %f\n", ntasks_completed, cpu);
    fclose(f);
    boinc_checkpoint_completed();
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
        // total CPU time at last checkpoint

    for (int j=1; j<argc; j++) {
        if (!strcmp(argv[j], "--nthreads")) {
            nthreads = atoi(argv[++j]);
        }
    }

    memset(&options, 0, sizeof(options));
    options.main_program = true;
    options.check_heartbeat = true;
    options.handle_process_control = true;

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

    retval = start_daemons(argc, argv);
    if (retval) {
        fprintf(stderr, "start_daemons(): %d\n", retval);
        kill_daemons();
        boinc_finish(retval);
    }

    // loop over tasks
    //
    for (i=0; i<tasks.size(); i++) {
        TASK& task = tasks[i];
        if ((int)i<ntasks_completed) {
            weight_completed += task.weight;
            continue;
        }
        double frac_done = weight_completed/total_weight;
        double cpu_time = 0;

        task.starting_cpu = checkpoint_cpu_time;
        retval = task.run(argc, argv);
        if (retval) {
            boinc_finish(retval);
        }
        int counter = 0;
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
                    kill_daemons();
                    boinc_finish(EXIT_CHILD_FAILED);
                }
                break;
            }
            poll_boinc_messages(task);
            double task_fraction_done = task.fraction_done();
            double delta = task_fraction_done*task.weight/total_weight;

            // getting CPU time of task tree is inefficient,
            // so do it only every 10 sec
            //
            if (counter%10 == 0) {
                cpu_time = task.cpu_time();
            }
            boinc_report_app_status(
                task.starting_cpu + cpu_time,
                checkpoint_cpu_time,
                frac_done + delta
            );
            if (task.has_checkpointed()) {
                cpu_time = task.cpu_time();
                checkpoint_cpu_time = task.starting_cpu + cpu_time;
                write_checkpoint(i, checkpoint_cpu_time);
            }
            boinc_sleep(POLL_PERIOD);
            counter++;
        }
        checkpoint_cpu_time = task.starting_cpu + task.final_cpu_time;
        write_checkpoint(i+1, checkpoint_cpu_time);
        weight_completed += task.weight;
    }
    kill_daemons();
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
