// The contents of this file are subject to the Mozilla Public License
// Version 1.0 (the "License"); you may not use this file except in
// compliance with the License. You may obtain a copy of the License at
// http://www.mozilla.org/MPL/ 
// 
// Software distributed under the License is distributed on an "AS IS"
// basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the
// License for the specific language governing rights and limitations
// under the License. 
// 
// The Original Code is the Berkeley Open Infrastructure for Network Computing. 
// 
// The Initial Developer of the Original Code is the SETI@home project.
// Portions created by the SETI@home project are Copyright (C) 2002
// University of California at Berkeley. All Rights Reserved. 
// 
// Contributor(s):
//

// Abstraction of a set of executing applications,
// connected to I/O files in various ways.
// Shouldn't depend on CLIENT_STATE.

#include "windows_cpp.h"
#include "error_numbers.h"

#ifdef _WIN32
#include <io.h>
#include <windows.h>
#endif
#if HAVE_UNISTD_H
#include <unistd.h>
#endif
#if HAVE_SYS_WAIT_H
#include <sys/wait.h>
#endif
#if HAVE_SYS_TIME_H
#include <sys/time.h>
#endif
#if HAVE_SYS_RESOURCE_H
#include <sys/resource.h>
#endif
#if HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#if HAVE_SYS_SIGNAL_H
#include <sys/signal.h>
#endif
#if HAVE_FCNTL_H
#include <fcntl.h>
#endif
#if HAVE_SIGNAL_H
#include <signal.h>
#endif

#include <ctype.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>

#include "client_state.h"
#include "client_types.h"
#include "filesys.h"
#include "file_names.h"
#include "log_flags.h"
#include "parse.h"
#include "util.h"

#include "app.h"
#include "boinc_api.h"

// take a string containing some space separated words.
// return an array of pointers to the null-terminated words.
// Modifies the string arg.
// TODO: use strtok here
int parse_command_line(char* p, char** argv) {
    char** pp = argv;
    bool space = true;

    while (*p) {
        if (isspace(*p)) {
            *p = 0;
            space = true;
        } else {
            if (space) {
                *pp++ = p;
                space = false;
            }
        }
        p++;
    }
    *pp++ = 0;

    return 0;
}

// Goes through an array of strings, and prints each string
//
static int print_argv(char** argv) {
    int i;

    for (i=0; argv[i]; i++) {
        fprintf(stderr, "argv[%d]: %s\n", i, argv[i]);
    }

    return 0;
}

ACTIVE_TASK::ACTIVE_TASK() {
    result = NULL;
    wup = NULL;
    app_version = NULL;
    pid = 0;
    slot = 0;
    state = PROCESS_UNINITIALIZED;
    exit_status = 0;
    signal = 0;
    strcpy(dirname, "");
}

int ACTIVE_TASK::init(RESULT* rp) {
    result = rp;
    wup = rp->wup;
    app_version = wup->avp;
    
    return 0;
}

// Start a task in a slot directory.  This includes setting up soft links,
// passing preferences, and starting the process
//
// WHAT ARE ASSUMPTIONS ABOUT CURRENT DIR??
// SHOULD REPLACE ../.. stuff
//
int ACTIVE_TASK::start(bool first_time) {
    char exec_name[256], file_path[256], link_path[256], temp[256];
    char* argv[100];
    unsigned int i;
    FILE_REF file_ref;
    FILE_INFO* fip;
    int retval;
    char prefs_path[256], init_path[256];
    FILE *prefs_fd,*init_file;
    APP_INIT_DATA aid;

    if (first_time) {
        checkpoint_cpu_time = 0;
    }
    current_cpu_time = checkpoint_cpu_time;
    starting_cpu_time = checkpoint_cpu_time;
    fraction_done = 0;

    //app_prefs.graphics.xsize = 640;
    //app_prefs.graphics.ysize = 480;
    //app_prefs.graphics.refresh_period = 5;

    memset(&aid, 0, sizeof(aid));
    // TODO: fill in the app prefs, user name, team name, etc.
    aid.checkpoint_period = DEFAULT_CHECKPOINT_PERIOD;
    aid.fraction_done_update_period = DEFAULT_FRACTION_DONE_UPDATE_PERIOD;
    aid.wu_cpu_time = checkpoint_cpu_time;

    sprintf(prefs_path, "%s/%s", dirname, INIT_DATA_FILE);
    prefs_fd = fopen(prefs_path, "wb");
    if (!prefs_fd) {
        if (log_flags.task_debug) {
            printf("Failed to open core to app prefs file %s.\n", prefs_path);
        }
        return ERR_FOPEN;
    }
    retval = write_init_data_file(prefs_fd, aid);
    fclose(prefs_fd);

    sprintf(init_path, "%s/%s", dirname, FD_INIT_FILE);
    init_file = fopen(init_path, "wb");
    if (!init_file) {
        if(log_flags.task_debug) {
            printf( "Failed to open init file %s.\n", init_path );
        }
        return ERR_FOPEN;
    }

    // make soft links to the executable(s)
    //
    for (i=0; i<app_version->app_files.size(); i++) {
        fip = app_version->app_files[i].file_info;
        get_pathname(fip, file_path);
        if (i == 0) {
            strcpy(exec_name, fip->name);
        }
        if (first_time) {
            sprintf(link_path, "%s/%s", dirname, fip->name);
            sprintf(temp, "../../%s", file_path );
            retval = boinc_link( temp, link_path);
            if (log_flags.task_debug) {
                printf("link %s to %s\n", file_path, link_path);
            }
            if (retval) {
                perror("link");
                fclose( init_file );
                return retval;
            }
        }
    }

    // create symbolic links, and hook up descriptors, for input files
    //
    for (i=0; i<wup->input_files.size(); i++) {
        file_ref = wup->input_files[i];
        get_pathname(file_ref.file_info, file_path);
        if (strlen(file_ref.open_name)) {
            if (first_time) {
                sprintf(link_path, "%s/%s", dirname, file_ref.open_name);
                sprintf(temp, "../../%s", file_path );
                if (log_flags.task_debug) {
                    printf("link %s to %s\n", file_path, link_path);
                }
                retval = boinc_link( temp, link_path);
                if (retval) {
                    perror("link");
                    fclose( init_file );
                    return retval;
                }
            }
        } else {
            sprintf(temp, "../../%s", file_path);
            write_fd_init_file(init_file, temp, file_ref.fd, 1);
        }
    }

    // hook up the output files using BOINC soft links
    //
    for (i=0; i<result->output_files.size(); i++) {
        file_ref = result->output_files[i];
        get_pathname(file_ref.file_info, file_path);
        if (strlen(file_ref.open_name)) {
            if (first_time) {
                creat(file_path, 0660);
                sprintf(link_path, "%s/%s", dirname, file_ref.open_name);
                sprintf(temp, "../../%s", file_path );
                if (log_flags.task_debug) {
                    printf("link %s to %s\n", file_path, link_path);
                }
                retval = boinc_link( temp, link_path);
                if (retval) {
                    fclose( init_file );
                    perror("link");
                    return retval;
                }
            }
        } else {
            sprintf( temp, "../../%s", file_path );
            write_fd_init_file(init_file, temp, file_ref.fd, 0);
        }
    }

    fclose( init_file );

#ifdef HAVE_UNISTD_H
#ifdef HAVE_SYS_TYPES_H
    pid = fork();
    if (pid == 0) {
        
        // from here on we're running in a new process.
        // If an error happens, exit nonzero so that the core client
        // knows there was a problem.

        // chdir() into the slot directory
        //
        retval = chdir(dirname);
        if (retval) {
            perror("chdir");
            exit(retval);
        }

        // hook up stderr to a specially-named file
        //
        freopen(STDERR_FILE, "a", stderr);

        argv[0] = exec_name;
        parse_command_line(wup->command_line, argv+1);
        if (log_flags.task_debug) print_argv(argv);
        boinc_resolve_filename(exec_name, temp);
        retval = execv(temp, argv);
        fprintf(stderr, "execv failed: %d\n", retval);
        perror("execv");
        exit(1);
    }
    if (log_flags.task_debug) printf("forked process: pid %d\n", pid);
#endif
#endif

#ifdef _WIN32
    PROCESS_INFORMATION process_info;
    STARTUPINFO startup_info;
    HINSTANCE inst;

    memset( &process_info, 0, sizeof( process_info ) );
    memset( &startup_info, 0, sizeof( startup_info ) );
    startup_info.cb = sizeof(startup_info);
    startup_info.lpReserved = NULL;
    startup_info.lpDesktop = "";

    // hook up stderr to a specially-named file (do this inside the new process)
    //
    //freopen(STDERR_FILE, "a", stderr);

    // Need to condense argv into a single string
    //if (log_flags.task_debug) print_argv(argv);
    //
    sprintf( temp, "%s/%s", dirname, exec_name );
    boinc_resolve_link( temp, exec_name );
    if( !CreateProcess( exec_name,
        wup->command_line,
        NULL, // not sure about this for security
        NULL, // not sure about this for security
        FALSE,
        CREATE_NEW_PROCESS_GROUP|NORMAL_PRIORITY_CLASS,
        NULL,
        dirname,
        &startup_info,
        &process_info )
    ) {
        state = GetLastError();
        LPVOID lpMsgBuf;
        FormatMessage( 
            FORMAT_MESSAGE_ALLOCATE_BUFFER | 
            FORMAT_MESSAGE_FROM_SYSTEM | 
            FORMAT_MESSAGE_IGNORE_INSERTS,
            NULL,
            state,
            0, // Default language
            (LPTSTR) &lpMsgBuf,
            0,
            NULL
        );
        MessageBox( NULL, (LPCTSTR)lpMsgBuf, "Error", MB_OK | MB_ICONINFORMATION );
    }
    pid_handle = process_info.hProcess;

#endif

    state = PROCESS_RUNNING;
    return 0;
}

// Sends a request to the process of this active task to exit.  If it
// doesn't exist within a set time (seconds), the process is terminated
//
void ACTIVE_TASK::request_exit(int seconds) {
    int retval;
    if(seconds<0) {
        fprintf(stderr, "error: ACTIVE_TASK.request_exit: negative seconds\n");
        seconds=0;
    }
#if HAVE_SIGNAL_H
#if HAVE_SYS_TYPES_H
    retval = kill(pid, SIGTERM);
    boinc_sleep(seconds);
    while(retval) retval=kill(pid, SIGKILL);
#endif
#endif
#ifdef _WIN32
    retval = TerminateProcess(pid_handle, -1);//exit codes should be changed
    boinc_sleep(seconds);
    while(retval) retval=TerminateProcess(pid_handle, -1);
#endif
}

// Inserts an active task into the ACTIVE_TASK_SET and starts it up
//
int ACTIVE_TASK_SET::insert(ACTIVE_TASK* atp) {
    int retval;

    get_slot_dir(atp->slot, atp->dirname);
    clean_out_dir(atp->dirname);
    retval = atp->start(true);
    if (retval) return retval;
    active_tasks.push_back(atp);
    return 0;
}

// Checks if any child processes have exited and records their final CPU time
//
bool ACTIVE_TASK_SET::poll() {
    int stat;
    ACTIVE_TASK* atp = NULL;
    char path[256];
    int n;

#ifdef _WIN32
    unsigned long exit_code;
    int i;
    FILETIME creation_time, exit_time, kernel_time, user_time;
    ULARGE_INTEGER tKernel, tUser;
    LONGLONG totTime;

    for (i=0; i<active_tasks.size(); i++) {
        atp = active_tasks[i];
        if (GetExitCodeProcess(atp->pid_handle, &exit_code)) {
            // Get the elapsed CPU time
            if (GetProcessTimes(atp->pid_handle, &creation_time, &exit_time, &kernel_time, &user_time)) {
                tKernel.LowPart = kernel_time.dwLowDateTime;
                tKernel.HighPart = kernel_time.dwHighDateTime;
	
                tUser.LowPart = user_time.dwLowDateTime;
                tUser.HighPart = user_time.dwHighDateTime;
	
                // Runtimes in 100-nanosecond units
                totTime = tKernel.QuadPart + tUser.QuadPart;

                atp->result->cpu_time = (totTime / 10000000.0);
            } else {
                // This probably isn't correct
                atp->result->cpu_time = ((double)clock())/CLOCKS_PER_SEC;
            }
            if (exit_code != STILL_ACTIVE) {
                // Not sure how to incorporate the other states (WAS_SIGNALED, etc)
                atp->state = PROCESS_EXITED;
                atp->exit_status = exit_code;
                atp->result->exit_status = atp->exit_status;
            }
        } else {
            // Not sure what to do here
        }
    }

    if( atp == NULL ) {
        return false;
    }
#endif

#if HAVE_SYS_RESOURCE_H
#if HAVE_SYS_WAIT_H
#if HAVE_SYS_TIME_H
    struct rusage rs;
    int pid;

    pid = wait3(&stat, WNOHANG, &rs);
    if (pid <= 0) return false;
    if (log_flags.task_debug) printf("got signal for process %d\n", pid);
    atp = lookup_pid(pid);
    if (!atp) {
        fprintf(stderr, "ACTIVE_TASK_SET::poll(): pid %d not found\n", pid);
        return true;
    }
    double x = rs.ru_utime.tv_sec + rs.ru_utime.tv_usec/1.e6;
    atp->result->final_cpu_time = atp->starting_cpu_time + x;
    if (WIFEXITED(stat)) {
        atp->state = PROCESS_EXITED;
        atp->exit_status = WEXITSTATUS(stat);
        atp->result->exit_status = atp->exit_status;
        if (log_flags.task_debug) printf("process exited status%d\n", atp->exit_status);
    } else if (WIFSIGNALED(stat)) {
        atp->state = PROCESS_WAS_SIGNALED;
        atp->signal = WTERMSIG(stat);
        atp->result->exit_status = atp->signal;
        if (log_flags.task_debug) printf("process was signaled %d\n", atp->signal);
    } else {
        atp->state = PROCESS_EXIT_UNKNOWN;
        atp->result->exit_status = -1;
    }
#endif
#endif
#endif

    // check for the stderr file, copy to result record
    //
    sprintf(path, "%s/%s", atp->dirname, STDERR_FILE);
    FILE* f = fopen(path, "r");
    if (f) {
        n = fread(atp->result->stderr_out, 1, STDERR_MAX_LEN, f);
        atp->result->stderr_out[STDERR_MAX_LEN-1] = 0;
        fclose(f);
    }

    clean_out_dir(atp->dirname);

    return true;
}

// Find the ACTIVE_TASK in the current set with the matching PID
//
ACTIVE_TASK* ACTIVE_TASK_SET::lookup_pid(int pid) {
    unsigned int i;
    ACTIVE_TASK* atp;

    for (i=0; i<active_tasks.size(); i++) {
        atp = active_tasks[i];
        if (atp->pid == pid) return atp;
    }
    return NULL;
}

// Sends a suspend request to all currently running computation processes
//
void ACTIVE_TASK_SET::suspend_all() {
    unsigned int i;
    ACTIVE_TASK* atp;
    for (i=0; i<active_tasks.size(); i++) {
        atp = active_tasks[i];
        atp->suspend();
    }
}

// Sends a resume signal to all currently running computation processes
//
void ACTIVE_TASK_SET::unsuspend_all() {
    unsigned int i;
    ACTIVE_TASK* atp;
    for (i=0; i<active_tasks.size(); i++) {
        atp = active_tasks[i];
        atp->unsuspend();
    }
}

// Attempts to exit all currently running computation processes, either
// via exit request or termination
//
void ACTIVE_TASK_SET::exit_tasks() {
    unsigned int i;
    ACTIVE_TASK *atp;
    for (i=0; i<active_tasks.size(); i++) {
        atp = active_tasks[i];
        atp->request_exit(0);
        atp->check_app_status_files();
    }
}

#ifdef _WIN32
// Send a suspend request to the ACTIVE_TASK
//
void ACTIVE_TASK::suspend() {
	// figure out a way to do this, perhaps via trigger file?
	//kill(atp->pid, SIGSTOP);
}

// Send a resume request to the ACTIVE_TASK
//
void ACTIVE_TASK::unsuspend() {
	// figure out a way to do this, perhaps via trigger file?
	//kill(atp->pid, SIGCONT);
}
#else
// Send a suspend request to the ACTIVE_TASK
//
void ACTIVE_TASK::suspend() {
    kill(this->pid, SIGSTOP);
}

// Send a resume request to the ACTIVE_TASK
//
void ACTIVE_TASK::unsuspend() {
    kill(this->pid, SIGCONT);
}
#endif

// Remove an ACTIVE_TASK from the set, assumes that the task has
// already been shut down via request_exit or similar means
//
int ACTIVE_TASK_SET::remove(ACTIVE_TASK* atp) {
    vector<ACTIVE_TASK*>::iterator iter;

    iter = active_tasks.begin();
    while (iter != active_tasks.end()) {
        if (*iter == atp) {
            active_tasks.erase(iter);
            return 0;
        }
        iter++;
    }
    fprintf(stderr, "ACTIVE_TASK_SET::remove(): not found\n");
    return 1;
}

// Restart active tasks without wiping and reinitializing slot directories
//
int ACTIVE_TASK_SET::restart_tasks() {
    vector<ACTIVE_TASK*>::iterator iter;
    ACTIVE_TASK* atp;
    int retval;

    iter = active_tasks.begin();
    while (iter != active_tasks.end()) {
        atp = *iter;
        get_slot_dir(atp->slot, atp->dirname);
        retval = atp->start(false);
        if (log_flags.task) {
            printf("restarting application for result %s\n", atp->result->name);
        }
        if (retval) {
            fprintf(stderr, "ACTIVE_TASKS::restart_tasks(); restart failed: %d\n", retval);
            active_tasks.erase(iter);
        } else {
            iter++;
        }
    }
    return 0;
}

// See if the app has generated new checkpoint CPU or fraction-done files.
// If so read them and return true.
//
bool ACTIVE_TASK::check_app_status_files() {
    FILE* f;
    char app_path[256];
    bool found = false;

    sprintf(app_path, "%s/%s", dirname, CHECKPOINT_CPU_FILE);
    f = fopen(app_path, "r");
    if (f) {
        found = true;
        parse_checkpoint_cpu_file(f, checkpoint_cpu_time);
        fclose(f);
    }

    sprintf(app_path, "%s/%s", dirname, FRACTION_DONE_FILE);
    f = fopen(app_path, "r");
    if (f) {
        found = true;
        parse_fraction_done_file(
            f, current_cpu_time, fraction_done
        );
        fclose(f);
    }
    return found;
}

// Poll each of the currently running tasks and get their CPU time
//
bool ACTIVE_TASK_SET::poll_time() {
    ACTIVE_TASK* atp;
    unsigned int i;
    bool updated;

    for(i=0; i<active_tasks.size(); i++) {
        atp = active_tasks[i];
        updated |= atp->check_app_status_files();
    }
    return updated;
}

// Write XML data about this ACTIVE_TASK
//
int ACTIVE_TASK::write(FILE* fout) {
    fprintf(fout,
        "<active_task>\n"
        "    <project_master_url>%s</project_master_url>\n"
        "    <result_name>%s</result_name>\n"
        "    <app_version_num>%d</app_version_num>\n"
        "    <slot>%d</slot>\n"
        "    <checkpoint_cpu_time>%f</checkpoint_cpu_time>\n"
        "</active_task>\n",
        result->project->master_url,
        result->name,
        app_version->version_num,
        slot,
        checkpoint_cpu_time
    );
    return 0;
}

// Parse XML information about an active task
//
int ACTIVE_TASK::parse(FILE* fin, CLIENT_STATE* cs) {
    char buf[256], result_name[256], project_master_url[256];
    int app_version_num=0;
    PROJECT* project;

    strcpy(result_name, "");
    strcpy(project_master_url, "");
    while (fgets(buf, 256, fin)) {
        if (match_tag(buf, "</active_task>")) {
            project = cs->lookup_project(project_master_url);
            if (!project) {
                fprintf(stderr,
                    "ACTIVE_TASK::parse(): project not found: %s\n",
                    project_master_url
                );
                return -1;
            }
            result = cs->lookup_result(project, result_name);
            if (!result) {
                fprintf(stderr, "ACTIVE_TASK::parse(): result not found\n");
                return -1;
            }
            wup = result->wup;
            app_version = cs->lookup_app_version(
                result->app, app_version_num
            );
            if (!app_version) {
                fprintf(stderr, "ACTIVE_TASK::parse(): app_version not found\n");
                return -1;
            }
            return 0;
        }
        else if (parse_str(buf, "<result_name>", result_name)) continue;
        else if (parse_str(buf, "<project_master_url>", project_master_url)) continue;
        else if (parse_int(buf, "<app_version_num>", app_version_num)) continue;
        else if (parse_int(buf, "<slot>", slot)) continue;
	else if (parse_double(buf, "<checkpoint_cpu_time>", checkpoint_cpu_time)) continue;
        else fprintf(stderr, "ACTIVE_TASK::parse(): unrecognized %s\n", buf);
    }
    return -1;
}

// Write XML information about this active task set
//
int ACTIVE_TASK_SET::write(FILE* fout) {
    unsigned int i;

    fprintf(fout, "<active_task_set>\n");
    for (i=0; i<active_tasks.size(); i++) {
        active_tasks[i]->write(fout);
    }
    fprintf(fout, "</active_task_set>\n");
    return 0;
}

// Parse XML information about an active task set
//
int ACTIVE_TASK_SET::parse(FILE* fin, CLIENT_STATE* cs) {
    ACTIVE_TASK* atp;
    char buf[256];
    int retval;

    while (fgets(buf, 256, fin)) {
        if (match_tag(buf, "</active_task_set>")) return 0;
        else if (match_tag(buf, "<active_task>")) {
            atp = new ACTIVE_TASK;
            retval = atp->parse(fin, cs);
            if (!retval) active_tasks.push_back(atp);
            else delete atp;
        } else {
            fprintf(stderr, "ACTIVE_TASK_SET::parse(): unrecognized %s\n", buf);
        }
    }
    return 0;
}
