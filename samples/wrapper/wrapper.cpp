// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2014 University of California
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

// BOINC wrapper - lets you use non-BOINC apps with BOINC
// See https://github.com/BOINC/boinc/wiki/WrapperApp
//
// cmdline options:
// --device N       macro-substitute N for $GPU_DEVICE_NUM
//                  in worker cmdlines and env values
// --nthreads X     macro-substitute X for $NTHREADS
//                  in worker cmdlines and env values
// --trickle X      send a trickle-up message reporting runtime every X sec
//                  of runtime (use this for credit granting
//                  if your app does its own job management)
// --use_tstp       use SIGTSTP instead of SIGSTOP to suspend children
//                  (Unix only).  SIGTSTP can be caught.
//                  Use this if the wrapped program is itself a wrapper.
// --passthrough_child
//                  cmdline args beyond this one are passed to
//                  tasks for which <append_cmdline_args> is set in job.xml
//
// Handles:
// - suspend/resume/quit/abort
// - reporting CPU time
// - loss of heartbeat from client
// - checkpointing
//      (at the level of task; or potentially within task)
//
// Contributor: Andrew J. Younge (ajy4490@umiacs.umd.edu)

// comment out the following to disable checking that
// executables are signed.
// Doing so introduces a security vulnerability.
//
#define CHECK_EXECUTABLES

#ifndef _WIN32
#include "config.h"
#endif
#include <stdio.h>
#include <vector>
#include <string>
#include <algorithm>
#ifdef _WIN32
#include "boinc_win.h"
#include "win_util.h"
#else
#ifdef HAVE_SYS_WAIT_H
#include <sys/wait.h>
#endif
#include <sys/types.h>
#include <sys/stat.h>
#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif
#ifdef HAVE_SYS_RESOURCE_H
#include <sys/resource.h>
#endif
#include <unistd.h>
#endif

#include "version.h"
#if !(defined(_WIN32) || defined(__APPLE__))
#include "svn_version.h"
#endif
#include "boinc_api.h"
#include "app_ipc.h"
#include "graphics2.h"
#include "boinc_zip.h"
#include "error_numbers.h"
#include "filesys.h"
#include "parse.h"
#include "proc_control.h"
#include "procinfo.h"
#include "str_util.h"
#include "str_replace.h"
#include "util.h"
#include <regex>

using std::vector;
using std::string;

bool use_tstp = false;

//#define DEBUG

#ifdef DEBUG
inline void debug_msg(const char* x) {
    fprintf(stderr, "[DEBUG] %s\n", x);
}
#else
#define debug_msg(x)
#endif

#define JOB_FILENAME "job.xml"
#define CHECKPOINT_FILENAME "wrapper_checkpoint.txt"

#define POLL_PERIOD 1.0

int nthreads = 1;
int gpu_device_num = -1;
double runtime = 0;
    // run time this session
double trickle_period = 0;
bool enable_graphics_support = false;
vector<string> unzip_filenames;
string zip_filename;
vector<std::regex> zip_patterns;
APP_INIT_DATA aid;

struct TASK {
    string application;
    string exec_dir;
        // optional execution directory;
        // macro-substituted
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
        // append wrapper args following --passthrough_child
    bool multi_process;
    bool forward_slashes;
    double time_limit;
    int priority;

    // dynamic stuff follows
    double current_cpu_time;
        // most recently measured CPU time of this task
    double final_cpu_time;
        // final CPU time of this task
    double starting_cpu;
        // how much CPU time was used by tasks before this one
    bool suspended;
    double elapsed_time;
#ifdef _WIN32
    HANDLE pid_handle;
    DWORD pid;
    struct _stat last_stat;     // mod time of checkpoint file
#else
    int pid;
    struct stat last_stat;
    double start_rusage;        // getrusage() CPU time at start of task
#endif
    bool stat_first;

    int parse(XML_PARSER&);
    void substitute_macros();
    bool poll(int& status);
    int run(const vector<string> &child_args);
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
            char* p = fgets(buf, sizeof(buf), f);
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
             bufsize += (1 + (int)vsetenv[j].length());
        }
        bufsize++; // add a final byte for array null ptr
        *env_vars = new char[bufsize];
        memset(*env_vars, 0, sizeof(char) * bufsize);
        char* p = *env_vars;
        // copy each env string to a buffer for the process
        for (vector<string>::iterator it = vsetenv.begin();
            it != vsetenv.end() && len < bufsize-1;
            ++it
        ) {
            strncpy(p, it->c_str(), it->length());
            len = (int)strlen(p);
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

// replace s1 with s2
//
void str_replace_all(char* buf, const char* s1, const char* s2) {
    char buf2[64000];
    const size_t s1_len = strlen(s1);
    while (1) {
        char* p = strstr(buf, s1);
        if (!p) break;
        strcpy(buf2, p+s1_len);
        strcpy(p, s2);
        strcat(p, buf2);
    }
}

// replace s1 with s2
// http://stackoverflow.com/questions/2896600/how-to-replace-all-occurrences-of-a-character-in-string
//
void str_replace_all(string &str, const string& s1, const string& s2) {
    size_t start_pos = 0;
    while((start_pos = str.find(s1, start_pos)) != string::npos) {
        str.replace(start_pos, s1.length(), s2);
        start_pos += s2.length(); // Handles case where 's1' is a substring of 's2'
    }
}

// macro-substitute strings from job.xml
// $PROJECT_DIR -> project directory
// $NTHREADS --> --nthreads arg if present, else 1
// $GPU_DEVICE_NUM --> gpu_device_num from init_data.xml, or --device arg
// $PWD --> current directory
//
void macro_substitute(string &str) {
    const char* pd = strlen(aid.project_dir)?aid.project_dir:".";
    char nt[256], cwd[1024];

    sprintf(nt, "%d", nthreads);
#ifdef _WIN32
    GetCurrentDirectory(sizeof(cwd), cwd);
#else
    getcwd(cwd, sizeof(cwd));
#endif

#ifdef DEBUG
    fprintf(stderr, "[DEBUG] macro_substitute '%s'\n", str.c_str());
    fprintf(stderr, "[DEBUG] replacing $PROJECT_DIR with '%s'\n", pd);
    fprintf(stderr, "[DEBUG] replacing $NTHREADS with '%s'\n", nt);
    fprintf(stderr, "[DEBUG] replacing $PWD with '%s'\n", cwd);
#endif
    str_replace_all(str, "$PROJECT_DIR", pd);
    str_replace_all(str, "$NTHREADS", nt);
    str_replace_all(str, "$PWD", cwd);

    if (aid.gpu_device_num >= 0) {
        gpu_device_num = aid.gpu_device_num;
    }
    if (gpu_device_num >= 0) {
        char buf[256];
        sprintf(buf, "%d", gpu_device_num);
        str_replace_all(str, "$GPU_DEVICE_NUM", buf);
#ifdef DEBUG
        fprintf(stderr, "[DEBUG] replacing $GPU_DEVICE_NUM with '%s'\n", buf);
#endif
    }
#ifdef DEBUG
    fprintf(stderr, "[DEBUG] macro_substitute result '%s'\n", str.c_str());
#endif
}

// make a list of files in the slot directory,
// and write to "initial_file_list"
//
void get_initial_file_list() {
    char fname[256];
    vector<string> initial_files;
    DIRREF d = dir_open(".");
    while (!dir_scan(fname, d, sizeof(fname))) {
        initial_files.push_back(fname);
    }
    dir_close(d);
    FILE* f = fopen("initial_file_list_temp", "w");
    for (string &s: initial_files) {
        fprintf(f, "%s\n", s.c_str());
    }
    fclose(f);
    int retval = boinc_rename("initial_file_list_temp", "initial_file_list");
    if (retval) {
        fprintf(stderr, "boinc_rename() error: %d\n", retval);
        exit(1);
    }
}

void read_initial_file_list(vector<string>& files) {
    char buf[256];
    FILE* f = fopen("initial_file_list", "r");
    if (!f) return;
    while (fgets(buf, sizeof(buf), f)) {
        strip_whitespace(buf);
        files.push_back(string(buf));
    }
    fclose(f);
}

// if any zipped input files are present, unzip and remove them
//
void do_unzip_inputs() {
    for (unsigned int i=0; i<unzip_filenames.size(); i++) {
        string zipfilename = unzip_filenames[i];
        if (boinc_file_exists(zipfilename.c_str())) {
            string path;
            boinc_resolve_filename_s(zipfilename.c_str(), path);
            int retval = boinc_zip(UNZIP_IT, path, NULL);
            if (retval) {
                fprintf(stderr, "boinc_unzip() error: %d\n", retval);
                exit(1);
            }
            retval = boinc_delete_file(zipfilename.c_str());
            if (retval) {
                fprintf(stderr, "boinc_delete_file() error: %d\n", retval);
            }
        }
    }
}

bool in_vector(string s, vector<string>& v) {
    for (string &s2: v) {
        if (s == s2) return true;
    }
    return false;
}

// get the list of output files to zip
//
void get_zip_inputs(ZipFileList &files) {
    vector<string> initial_files;
    char fname[256];

    read_initial_file_list(initial_files);
    DIRREF d = dir_open(".");
    while (!dir_scan(fname, d, sizeof(fname))) {
        string filename = string(fname);
        if (in_vector(filename, initial_files)) continue;
        for (const auto& re: zip_patterns) {
            if (std::regex_search(filename, re)) {
                files.push_back(filename);
                break;
            }
        }
    }
    dir_close(d);
}

// if the zipped output file is not present,
// create the zip in a temp file, then rename it
//
void do_zip_outputs() {
    if (zip_filename.empty()) return;
    string path;
    boinc_resolve_filename_s(zip_filename.c_str(), path);
    if (boinc_file_exists(path.c_str())) return;
    ZipFileList infiles;
    get_zip_inputs(infiles);
    int retval = boinc_zip(ZIP_IT, string("temp.zip"), &infiles);
    if (retval) {
        fprintf(stderr, "boinc_zip() failed: %d\n", retval);
        exit(1);
    }
    retval = boinc_rename("temp.zip", path.c_str());
    if (retval) {
        fprintf(stderr, "failed to rename temp.zip: %d\n", retval);
        exit(1);
    }
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
    forward_slashes = false;
    time_limit = 0;
    priority = PROCESS_PRIORITY_LOWEST;

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
            exec_dir = buf;
            continue;
        }
        else if (xp.parse_str("setenv", buf, sizeof(buf))) {
            vsetenv.push_back(buf);
            continue;
        }
        else if (xp.parse_string("stdin_filename", stdin_filename)) continue;
        else if (xp.parse_string("stdout_filename", stdout_filename)) continue;
        else if (xp.parse_string("stderr_filename", stderr_filename)) continue;
        else if (xp.parse_str("command_line", buf, sizeof(buf))) {
            command_line = buf;
            continue;
        }
        else if (xp.parse_string("checkpoint_filename", checkpoint_filename)) continue;
        else if (xp.parse_string("fraction_done_filename", fraction_done_filename)) continue;
        else if (xp.parse_double("weight", weight)) continue;
        else if (xp.parse_bool("daemon", is_daemon)) continue;
        else if (xp.parse_bool("forward_slashes", forward_slashes)) continue;
        else if (xp.parse_bool("multi_process", multi_process)) continue;
        else if (xp.parse_bool("append_cmdline_args", append_cmdline_args)) continue;
        else if (xp.parse_double("time_limit", time_limit)) continue;
        else if (xp.parse_int("priority", priority)) continue;
    }
    return ERR_XML_PARSE;
}

void TASK::substitute_macros() {
    if (!exec_dir.empty()) {
        macro_substitute(exec_dir);
    }
    for (unsigned int i = 0; i < vsetenv.size(); i++) {
        macro_substitute(vsetenv[i]);
    }
    if (!command_line.empty()) {
        macro_substitute(command_line);
    }
}

int parse_unzip_input(XML_PARSER& xp) {
    char buf2[256];
    string s;
    while (!xp.get_tag()) {
        if (xp.match_tag("/unzip_input")) {
            return 0;
        }
        if (xp.parse_string("zipfilename", s)) {
            unzip_filenames.push_back(s);
            continue;
        }
        fprintf(stderr,
            "%s unexpected tag in job.xml: %s\n",
            boinc_msg_prefix(buf2, sizeof(buf2)), xp.parsed_tag
        );
    }
    return ERR_XML_PARSE;
}

int parse_zip_output(XML_PARSER& xp) {
    char buf[256];
    while (!xp.get_tag()) {
        if (xp.match_tag("/zip_output")) {
            return 0;
        }
        if (xp.parse_string("zipfilename", zip_filename)) {
            continue;
        }
        if (xp.parse_str("filename", buf, sizeof(buf))) {
            // Compile pattern using std::regex. Try ECMAScript (default),
            // then extended as fallback.
            try {
                zip_patterns.emplace_back(buf);
            } catch (const std::regex_error& e) {
                fprintf(stderr,
                    "regex compilation failed for pattern '%s': %s\n",
                    buf, e.what());
                return ERR_XML_PARSE;
            }
            continue;
        }
        fprintf(stderr,
            "%s unexpected tag in job.xml: %s\n",
            boinc_msg_prefix(buf, sizeof(buf)), xp.parsed_tag
        );
    }
    return ERR_XML_PARSE;
}

int parse_job_file() {
    MIOFILE mf;
    char buf[256], buf2[256];

    boinc_resolve_filename(JOB_FILENAME, buf, sizeof(buf));
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
        }
        if (xp.match_tag("unzip_input")) {
            parse_unzip_input(xp);
            continue;
        }
        if (xp.match_tag("zip_output")) {
            parse_zip_output(xp);
            continue;
        }
        if (xp.parse_bool("enable_graphics_support", enable_graphics_support)) continue;
        fprintf(stderr,
            "%s unexpected tag in job.xml: %s\n",
            boinc_msg_prefix(buf2, sizeof(buf2)), xp.parsed_tag
        );
    }
    fclose(f);
    return ERR_XML_PARSE;
}

int start_daemons(const vector<string>& child_args) {
    for (TASK& task: daemons) {
        int retval = task.run(child_args);
        if (retval) return retval;
    }
    return 0;
}

void kill_daemons() {
    vector<int> daemon_pids;
    for (TASK& task: daemons) {
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
            FILE_SHARE_READ|FILE_SHARE_WRITE,
            &sa,
            OPEN_ALWAYS,
            0, 0
        );
    } else if (!strcmp(mode, "a")) {
        HANDLE hAppend = CreateFile(
            path,
            GENERIC_WRITE,
            FILE_SHARE_READ|FILE_SHARE_WRITE,
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

void backslash_to_slash(char* p) {
    while (1) {
        char* q = strchr(p, '\\');
        if (!q) break;
        *q = '/';
    }
}

int TASK::run(const vector<string> &child_args) {
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

    if (!boinc_file_exists(app_path)) {
        fprintf(stderr, "application %s missing\n", app_path);
        exit(1);
    }

    // Optionally append wrapper's pass-through args
    // to those in the job file.
    //
    if (append_cmdline_args) {
        for (const string& arg: child_args) {
            command_line += string(" ");
            command_line += arg;
        }
    }

    // resolve "boinc_resolve(...)" phrases in command-line
    const size_t boinc_resolve_prefix_len = strlen("boinc_resolve(");
    while (1) {
        char lbuf[16384];
        char fname[1024];
        char *from, *to;

        strncpy (lbuf, command_line.c_str(), sizeof(lbuf));
        lbuf[sizeof(lbuf)-1] = '\0';
        from = strstr(lbuf, "boinc_resolve(");
        if (!from) {
            break;
        }
        to = strchr(from, ')');
        if (!to) {
            fprintf(stderr, "missing ')' after 'boinc_resolve('\n");
            exit(1);
        }
        *to = 0;
        boinc_resolve_filename(from + boinc_resolve_prefix_len, fname, sizeof(fname));
#ifdef _WIN32
        if(forward_slashes) {
            backslash_to_slash(fname);
        } else {
            slash_to_backslash(fname);
        }
#endif
        *from = 0;
        command_line = string(lbuf) + string(fname) + string(to+1);
    }

    fprintf(stderr, "%s wrapper: running %s (%s)\n",
        boinc_msg_prefix(buf, sizeof(buf)), app_path, command_line.c_str()
    );

    // decide on subprocess priority.  User prefs trump job.xml
    //
    int priority_val = 0;
    if (aid.no_priority_change) {
        priority_val = 0;
    } else {
        if (aid.process_priority > CONFIG_PRIORITY_UNSPECIFIED) {
            // priority coming from the client is on scale where 0 is idle.
            // we use the scale where 1 is idle
            //
            priority_val = process_priority_value(aid.process_priority+1);
        } else {
            priority_val = process_priority_value(priority);
        }
    }

#ifdef _WIN32
    PROCESS_INFORMATION process_info;
    STARTUPINFO startup_info;
    string command;

    slash_to_backslash(app_path);
    memset(&process_info, 0, sizeof(process_info));
    memset(&startup_info, 0, sizeof(startup_info));

    if (ends_with((string)app_path, ".bat") || ends_with((string)app_path, ".cmd")) {
        command = string("cmd.exe /c \"") + app_path + string("\" ") + command_line;
    } else {
        command = string("\"") + app_path + string("\" ") + command_line;
    }

    // pass std handles to app
    //
    startup_info.dwFlags = STARTF_USESTDHANDLES;
    if (stdout_filename != "") {
        boinc_resolve_filename_s(stdout_filename.c_str(), stdout_path);
        startup_info.hStdOutput = win_fopen(stdout_path.c_str(), "a");
    } else {
        // Redirecting child stdout to wrapper stderr here is not a typo
        startup_info.hStdOutput = (HANDLE)_get_osfhandle(_fileno(stderr));
    }
    if (stdin_filename != "") {
        boinc_resolve_filename_s(stdin_filename.c_str(), stdin_path);
        startup_info.hStdInput = win_fopen(stdin_path.c_str(), "r");
    }
    if (stderr_filename != "") {
        boinc_resolve_filename_s(stderr_filename.c_str(), stderr_path);
        startup_info.hStdError = win_fopen(stderr_path.c_str(), "a");
    } else {
        startup_info.hStdError = (HANDLE)_get_osfhandle(_fileno(stderr));
    }

    if (startup_info.hStdOutput == INVALID_HANDLE_VALUE) {
        fprintf(stderr, "Error: startup_info.hStdOutput is invalid\n");
    }
    if ((stdin_filename != "") && (startup_info.hStdInput == INVALID_HANDLE_VALUE)) {
        fprintf(stderr, "Error: startup_info.hStdInput is invalid\n");
    }
    if (startup_info.hStdError == INVALID_HANDLE_VALUE) {
        fprintf(stderr, "Error: startup_info.hStdError is invalid\n");
    }

    // setup environment vars if needed
    //
    int nvars = (int)vsetenv.size();
    char* env_vars = NULL;
    if (nvars > 0) {
        set_up_env_vars(&env_vars, nvars);
    }

    BOOL success;
    success = CreateProcess(
        NULL,
        (LPSTR)command.c_str(),
        NULL,
        NULL,
        TRUE,        // bInheritHandles
        CREATE_NO_WINDOW|priority_val,
        (LPVOID) env_vars,
        exec_dir.empty()?NULL:exec_dir.c_str(),
        &startup_info,
        &process_info
    );
    if (!success) {
        char error_msg[1024];
        windows_format_error_string(GetLastError(), error_msg, sizeof(error_msg));
        fprintf(stderr, "can't run app: %s\n", error_msg);

        fprintf(stderr, "Error: command is '%s'\n", command.c_str());
        fprintf(stderr, "Error: exec_dir is '%s'\n", exec_dir.c_str());

        if (env_vars) delete [] env_vars;
        return ERR_EXEC;
    }
    if (env_vars) delete [] env_vars;
    pid_handle = process_info.hProcess;
    pid = process_info.dwProcessId;
    CloseHandle(process_info.hThread);
#else
    int retval;
    char* argv[256];
    char arglist[4096];
    FILE* stdout_file;
    FILE* stdin_file;
    FILE* stderr_file;

    struct rusage ru;
    getrusage(RUSAGE_CHILDREN, &ru);
    start_rusage = (float)ru.ru_utime.tv_sec + ((float)ru.ru_utime.tv_usec)/1e+6;

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
        if (priority_val) {
            setpriority(PRIO_PROCESS, 0, priority_val);
        }
        if (!exec_dir.empty()) {
            retval = chdir(exec_dir.c_str());
            if (retval) {
                fprintf(stderr,
                    "%s chdir() to %s failed with %d\n",
                    boinc_msg_prefix(buf, sizeof(buf)),
                    exec_dir.c_str(),
                    retval
                );
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

    fprintf(stderr, "%s wrapper: created child process %d\n",
        boinc_msg_prefix(buf, sizeof(buf)), (int)pid
    );

    suspended = false;
    elapsed_time = 0;
    return 0;
}

// return true if task exited; in that case also return its exit status
// (zero means it completed successfully)
//
bool TASK::poll(int& status) {
    char buf[256];
    if (time_limit && elapsed_time > time_limit) {
        fprintf(stderr,
            "%s task %s reached time limit %.0f\n",
            boinc_msg_prefix(buf, sizeof(buf)),
            application.c_str(), time_limit
        );
        kill();
        status = 0;
        return true;
    }
#ifdef _WIN32
    unsigned long exit_code;
    if (GetExitCodeProcess(pid_handle, &exit_code)) {
        if (exit_code != STILL_ACTIVE) {
            status = exit_code;
            final_cpu_time = current_cpu_time;
            fprintf(stderr, "%s %s exited; CPU time %f\n",
                boinc_msg_prefix(buf, sizeof(buf)),
                application.c_str(), final_cpu_time
            );
            CloseHandle(pid_handle);
            return true;
        }
    }
#else
    int wpid;
    struct rusage ru;

    wpid = waitpid(pid, &status, WNOHANG);
    if (wpid) {
        getrusage(RUSAGE_CHILDREN, &ru);
        final_cpu_time = (float)ru.ru_utime.tv_sec + ((float)ru.ru_utime.tv_usec)/1e+6;
        final_cpu_time -= start_rusage;
        fprintf(stderr, "%s %s exited; CPU time %f\n",
            boinc_msg_prefix(buf, sizeof(buf)),
            application.c_str(), final_cpu_time
        );

        if (WIFEXITED(status)) {
            status = WEXITSTATUS(status);
        }
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
#ifdef _WIN32
    kill_descendants();
#else
    kill_descendants(pid);
#endif
}
#ifdef _WIN32
void TASK::stop() {
    if (multi_process) {
        suspend_or_resume_descendants(false);
    } else {
        suspend_or_resume_process(pid, false);
    }
    suspended = true;
}
#else
void TASK::stop() {
    if (multi_process) {
        suspend_or_resume_descendants(false, use_tstp);
    }
    else {
        suspend_or_resume_process(pid, false, use_tstp);
    }
    suspended = true;
}
#endif

void TASK::resume() {
    if (multi_process) {
        suspend_or_resume_descendants(true);
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
#ifndef ANDROID
    // the Android GUI doesn't show CPU time,
    // and process_tree_cpu_time() crashes sometimes
    //
    double x = process_tree_cpu_time(pid);
    // if the process has exited, the above could return zero.
    // So update carefully.
    //
    if (x > current_cpu_time) {
        current_cpu_time = x;
    }
#endif
    return current_cpu_time;
}

void poll_boinc_messages(TASK& task) {
    BOINC_STATUS status;
    boinc_get_status(&status);
    //fprintf(stderr, "wrapper: polling\n");
    if (status.no_heartbeat) {
        debug_msg("wrapper: kill");
        task.kill();
        kill_daemons();
        exit(0);
    }
    if (status.quit_request) {
        debug_msg("wrapper: quit");
        task.kill();
        kill_daemons();
        exit(0);
    }
    if (status.abort_request) {
        debug_msg("wrapper: abort");
        task.kill();
        kill_daemons();
        exit(0);
    }
    if (status.suspended) {
        if (!task.suspended) {
            debug_msg("wrapper: suspend");
            task.stop();
        }
    } else {
        if (task.suspended) {
            debug_msg("wrapper: resume");
            task.resume();
        }
    }
}

// see if it's time to send trickle-up reporting elapsed time
//
void check_trickle_period() {
    char buf[256];
    static double last_trickle_report_time = 0;

    if ((runtime - last_trickle_report_time) < trickle_period) {
        return;
    }
    last_trickle_report_time = runtime;
    sprintf(buf,
        "<cpu_time>%f</cpu_time>", last_trickle_report_time
    );
    boinc_send_trickle_up(
        const_cast<char*>("cpu_time"), buf
    );
}

// Support for multiple tasks.
// We keep a checkpoint file that says how many tasks we've completed
// and how much CPU time and runtime has been used so far
//
void write_checkpoint(int ntasks_completed, double cpu, double rt) {
    boinc_begin_critical_section();
    FILE* f = fopen(CHECKPOINT_FILENAME, "w");
    if (!f) {
        boinc_end_critical_section();
        return;
    }
    fprintf(f, "%d %f %f\n", ntasks_completed, cpu, rt);
    fclose(f);
    boinc_checkpoint_completed();
}

// read the checkpoint file;
// return nonzero if it's missing or bad format
//
int read_checkpoint(int& ntasks_completed, double& cpu, double& rt) {
    int nt;
    double c, r;

    ntasks_completed = 0;
    cpu = 0;
    rt = 0;
    FILE* f = fopen(CHECKPOINT_FILENAME, "r");
    if (!f) return ERR_FOPEN;
    int n = fscanf(f, "%d %lf %lf", &nt, &c, &r);
    fclose(f);
    if (n != 3) return -1;

    ntasks_completed = nt;
    cpu = c;
    rt = r;
    return 0;
}

// if the given file is a soft link of the form ../../project_dir/x,
// return x, else return empty string
//
string resolve_proj_soft_link(const char* project_dir, const char* file) {
    char buf[1024], physical_name[1024];
    FILE* fp = boinc_fopen(file, "r");
    if (!fp) {
        return string("");
    }
    buf[0] = 0;
    char* p = fgets(buf, sizeof(buf), fp);
    fclose(fp);
    if (!p) {
        return string("");
    }
    if (!parse_str(buf, "<soft_link>", physical_name, sizeof(physical_name))) {
        return string("");
    }
    snprintf(buf, sizeof(buf), "../../%s/", project_dir);
    if (strstr(physical_name, buf) != physical_name) {
        return string("");
    }
    return string(physical_name + strlen(buf));
}

// Check whether executable files (tasks and daemons) are code-signed.
// The client supplies a list of app version files, which are code-signed.
// For each executable file:
// - check that it's a soft link
// - check that it's of the form ../../project_url/x
// - check that "x" is in the list of app version files
//
void check_execs(vector<TASK> &t) {
    for (unsigned int i=0; i<t.size(); i++) {
        TASK &task = t[i];
        string phys_name = resolve_proj_soft_link(
            aid.project_dir, task.application.c_str()
        );
        if (phys_name.empty()) {
            fprintf(stderr, "task executable %s is not a link\n",
                phys_name.c_str()
            );
            boinc_finish(1);
        }
        if (std::find(aid.app_files.begin(), aid.app_files.end(), phys_name) == aid.app_files.end()) {
            fprintf(stderr, "task executable %s is not in app version\n",
                task.application.c_str()
            );
            boinc_finish(1);
        }
    }
}

void check_executables() {
    if (aid.app_files.size() == 0) return;
    check_execs(tasks);
    check_execs(daemons);
}

void usage() {
    fprintf(stderr,
        "Options:\n"
        "   --nthreads N\n"
        "   --device N\n"
        "   --sporadic\n"
        "   --trickle X\n"
        "   --version\n"
        "   --use_tstp\n"
        "   --passthrough_child\n"
    );
    boinc_finish(EXIT_INIT_FAILURE);
}

int main(int argc, char** argv) {
    BOINC_OPTIONS options;
    int retval, ntasks_completed;
    unsigned int i;
    double total_weight=0, weight_completed=0;
    double checkpoint_cpu_time;
        // total CPU time at last checkpoint
    char buf[256];
    bool is_sporadic = false;
    bool passthrough_child = false;
    vector<string> child_args;

    fprintf(stderr, "%s wrapper (%d.%d.%d): starting\n",
        boinc_msg_prefix(buf, sizeof(buf)),
        BOINC_MAJOR_VERSION, BOINC_MINOR_VERSION, WRAPPER_RELEASE
    );

#ifdef _WIN32
    SetPriorityClass(GetCurrentProcess(), NORMAL_PRIORITY_CLASS);
#endif

    for (int j=1; j<argc; j++) {
        if (passthrough_child) {
            child_args.push_back(argv[j]);
        } else if (!strcmp(argv[j], "--nthreads")) {
            nthreads = atoi(argv[++j]);
        } else if (!strcmp(argv[j], "--device")) {
            gpu_device_num = atoi(argv[++j]);
        } else if (!strcmp(argv[j], "--sporadic")) {
            is_sporadic = true;
        } else if (!strcmp(argv[j], "--trickle")) {
            trickle_period = atof(argv[++j]);
#if !(defined(_WIN32) || defined(__APPLE__))
        } else if (!strcmp(argv[j], "--version") || !strcmp(argv[j], "-v")) {
            fprintf(stderr, "%s\n", SVN_VERSION);
            boinc_finish(0);
#endif
        } else if (!strcmp(argv[j], "--use_tstp")) {
            use_tstp = true;
        } else if (!strcmp(argv[j], "--passthrough_child")) {
            passthrough_child = true;
        } else {
            fprintf(stderr, "Unrecognized option %s\n", argv[j]);
            usage();
        }
    }

    retval = parse_job_file();
    if (retval) {
        fprintf(stderr, "%s can't parse job file: %d\n",
            boinc_msg_prefix(buf, sizeof(buf)),
            retval
        );
        boinc_finish(retval);
    }

    retval = read_checkpoint(ntasks_completed, checkpoint_cpu_time, runtime);
    if (retval) {
        // this is the first time we've run; unzip inputs
        do_unzip_inputs();
        write_checkpoint(0, 0, 0);

        // If we're going to zip output files,
        // make a list of files present at this point so we can exclude them.
        //
        if (!zip_filename.empty()) {
            get_initial_file_list();
        }
    }

    // do initialization after getting initial file list,
    // in case we're supposed to zip stderr.txt
    //
    memset(&options, 0, sizeof(options));
    options.main_program = true;
    options.check_heartbeat = true;
    options.handle_process_control = true;

    boinc_init_options(&options);
    boinc_get_init_data(aid);

#ifdef CHECK_EXECUTABLES
    check_executables();
#endif

    if (ntasks_completed > (int)tasks.size()) {
        fprintf(stderr,
            "%s Checkpoint file: ntasks_completed too large: %d > %d\n",
            boinc_msg_prefix(buf, sizeof(buf)),
            ntasks_completed, (int)tasks.size()
        );
        boinc_finish(1);
    }
    for (i=0; i<tasks.size(); i++) {
        total_weight += tasks[i].weight;
        // need to substitute macros after boinc_init_options() and boinc_get_init_data()
        tasks[i].substitute_macros();
    }

    if (is_sporadic) {
        retval = boinc_sporadic_dir(".");
        if (retval) {
            fprintf(stderr, "can't create sporadic files\n");
            boinc_finish(retval);
        }
    }

    retval = start_daemons(child_args);
    if (retval) {
        fprintf(stderr,
            "%s start_daemons(): %d\n",
            boinc_msg_prefix(buf, sizeof(buf)),
            retval
        );
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
        retval = task.run(child_args);

        if (retval) {
            boinc_finish(retval);
        }
        int counter = 0;
        while (1) {
            int status;
            if (task.poll(status)) {
                if (status) {
                    fprintf(stderr,
                        "%s app exit status: 0x%x\n",
                        boinc_msg_prefix(buf, sizeof(buf)),
                        status
                    );
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
#ifdef DEBUG
            fprintf(stderr,
                "%s cpu time %f, checkpoint CPU time %f frac done %f\n",
                boinc_msg_prefix(buf, sizeof(buf)),
                task.starting_cpu + cpu_time,
                checkpoint_cpu_time,
                frac_done + delta
            );
#endif
            boinc_report_app_status(
                task.starting_cpu + cpu_time,
                checkpoint_cpu_time,
                frac_done + delta
            );
            if (task.has_checkpointed()) {
                cpu_time = task.cpu_time();
                checkpoint_cpu_time = task.starting_cpu + cpu_time;
                write_checkpoint(i, checkpoint_cpu_time, runtime);
            }

            if (trickle_period) {
                check_trickle_period();
            }

            if (enable_graphics_support) {
                boinc_write_graphics_status(
                    task.starting_cpu + cpu_time,
                    checkpoint_cpu_time + task.elapsed_time,
                    frac_done + task.weight/total_weight
                );
            }

            boinc_sleep(POLL_PERIOD);
            if (!task.suspended) {
                task.elapsed_time += POLL_PERIOD;
                runtime += POLL_PERIOD;
            }
            counter++;
        }
        checkpoint_cpu_time = task.starting_cpu + task.final_cpu_time;
#ifdef DEBUG
        fprintf(stderr, "%s cpu time %f, checkpoint CPU time %f frac done %f\n",
            boinc_msg_prefix(buf, sizeof(buf)),
            task.starting_cpu + task.final_cpu_time,
            checkpoint_cpu_time,
            frac_done + task.weight/total_weight
        );
#endif
        boinc_report_app_status(
            task.starting_cpu + task.final_cpu_time,
            checkpoint_cpu_time,
            frac_done + task.weight/total_weight
        );
        write_checkpoint(i+1, checkpoint_cpu_time, runtime);
        weight_completed += task.weight;
    }
    kill_daemons();
    do_zip_outputs();
    boinc_finish(0);
}

