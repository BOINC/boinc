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

#ifdef _WIN32
#include <io.h>
#else
#include <unistd.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/resource.h>
#endif
#include <sys/types.h>
#include <sys/signal.h>
#include <fcntl.h>
#include <ctype.h>
#include <signal.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>

#include "client_types.h"
#include "client_state.h"
#include "filesys.h"
#include "file_names.h"
#include "log_flags.h"
#include "parse.h"

#include "app.h"
#include "api.h"

// take a string containing some words.
// return an array of pointers to the null-terminated words.
// Modifies the string arg.
//
void parse_command_line(char* p, char** argv) {
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
}

static void print_argv(char** argv) {
    int i;
    for (i=0; argv[i]; i++) {
        fprintf(stderr, "argv[%d]: %s\n", i, argv[i]);
    }
}

ACTIVE_TASK::ACTIVE_TASK() {
    result = NULL;
    wup = NULL;
    app_version = NULL;
    slot = 0;
    exit_status = 0;
    signal = 0;
    strcpy(dirname, "");
    cpu_time = 0;
}

int ACTIVE_TASK::init(RESULT* rp) {
    result = rp;
    wup = rp->wup;
    app_version = wup->avp;
    return 0;
}

int ACTIVE_TASK::start(bool first_time) {
    char exec_name[256], file_path[256], link_path[256],temp[256];
    char* argv[100];
    unsigned int i;
    FILE_REF file_ref;
    FILE_INFO* fip;
    int retval;
    char prefs_path[256],init_path[256];
    FILE *prefs_fd,*init_file;
    APP_IN app_prefs;

    if(first_time) prev_cpu_time = 0;
    cpu_time = 0;
    // These should be chosen in a better manner
    app_prefs.graphics.xsize = 640;
    app_prefs.graphics.ysize = 480;
    app_prefs.graphics.refresh_period = 5;
    app_prefs.checkpoint_period = 5;
    app_prefs.poll_period = 5;
    app_prefs.cpu_time = prev_cpu_time;

    // Write out the app prefs
    sprintf( prefs_path, "%s/%s", dirname, CORE_TO_APP_FILE );
    prefs_fd = fopen( prefs_path, "wb" );
    if( !prefs_fd ) {
        if( log_flags.task_debug ) {
            printf( "Failed to open core to app prefs file %s.\n", prefs_path );
        }
        return -1;
    }
    rewind( prefs_fd );
    write_core_file( prefs_fd,app_prefs );
    fclose(prefs_fd);

    sprintf( init_path, "%s/%s", dirname, BOINC_INIT_FILE );
    init_file = fopen( init_path, "wb" );
    if( !init_file ) {
        if( log_flags.task_debug ) {
            printf( "Failed to open init file %s.\n", init_path );
        }
        return -1;
    }
    rewind( init_file );

    // make a link to the executable
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
            sprintf( temp, "../../%s", file_path );
            write_init_file( init_file, temp, file_ref.fd, 1 );
        }
    }

    // hook up the output files
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
            write_init_file( init_file, temp, file_ref.fd, 0 );
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
        boinc_resolve_link( exec_name, temp );
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

#ifdef macintosh
#endif

    state = PROCESS_RUNNING;
    return 0;
}

void ACTIVE_TASK::request_exit(int seconds) {
    int retval;
    retval = kill(pid, SIGTERM);
    sleep(seconds);
    if(retval) kill(pid, SIGKILL);
}

int ACTIVE_TASK_SET::insert(ACTIVE_TASK* atp) {
    int retval;

    get_slot_dir(atp->slot, atp->dirname);
    clean_out_dir(atp->dirname);
    retval = atp->start(true);
    if (retval) return retval;
    active_tasks.push_back(atp);
    return 0;
}

// check for child process exit
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
        if( GetExitCodeProcess( atp->pid_handle,&exit_code ) ) {
            // Get the elapsed CPU time
            // Factor this into the equivalent of a S@H etime function?
            if( GetProcessTimes( atp->pid_handle, &creation_time, &exit_time, &kernel_time, &user_time ) ) {
                tKernel.LowPart = kernel_time.dwLowDateTime;
                tKernel.HighPart = kernel_time.dwHighDateTime;
	
                tUser.LowPart = user_time.dwLowDateTime;
                tUser.HighPart = user_time.dwHighDateTime;
	
                // Runtimes in 100-nanosecond units
                totTime = tKernel.QuadPart + tUser.QuadPart;

                atp->result->cpu_time = (totTime / 10000000.0);
            } else {
                atp->result->cpu_time = ((double)clock())/CLOCKS_PER_SEC;
            }
            if( exit_code != STILL_ACTIVE ) {
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

#ifdef unix
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
    atp->result->cpu_time = rs.ru_utime.tv_sec + rs.ru_utime.tv_usec/1.e6;
    if (WIFEXITED(stat)) {
        atp->state = PROCESS_EXITED;
        atp->exit_status = WEXITSTATUS(stat);
        atp->result->exit_status = atp->exit_status;
    } else if (WIFSIGNALED(stat)) {
        atp->state = PROCESS_WAS_SIGNALED;
        atp->signal = WTERMSIG(stat);
        atp->result->exit_status = atp->signal;
    } else {
        atp->state = PROCESS_EXIT_UNKNOWN;
        atp->result->exit_status = -1;
    }
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

ACTIVE_TASK* ACTIVE_TASK_SET::lookup_pid(int pid) {
    unsigned int i;
    ACTIVE_TASK* atp;

    for (i=0; i<active_tasks.size(); i++) {
        atp = active_tasks[i];
        if (atp->pid == pid) return atp;
    }
    return 0;
}

void ACTIVE_TASK_SET::suspend_all() {
    unsigned int i;
    ACTIVE_TASK* atp;
    for (i=0; i<active_tasks.size(); i++) {
        atp = active_tasks[i];
        atp->suspend();
    }
}

void ACTIVE_TASK_SET::unsuspend_all() {
    unsigned int i;
    ACTIVE_TASK* atp;
    for (i=0; i<active_tasks.size(); i++) {
        atp = active_tasks[i];
        atp->unsuspend();
    }
}

void ACTIVE_TASK_SET::exit_tasks() {
    unsigned int i;
    ACTIVE_TASK* atp;
    for (i=0; i<active_tasks.size(); i++) {
        atp = active_tasks[i];
        atp->request_exit(0);
    }
}

#ifdef _WIN32
void ACTIVE_TASK::suspend() {
    prev_cpu_time = cpu_time;
	// figure out a way to do this, perhaps via trigger file?
	//kill(atp->pid, SIGSTOP);
}

void ACTIVE_TASK::unsuspend() {
	// figure out a way to do this, perhaps via trigger file?
	//kill(atp->pid, SIGCONT);
}
#else
void ACTIVE_TASK::suspend() {
    prev_cpu_time = cpu_time;
	kill(this->pid, SIGSTOP);
}

void ACTIVE_TASK::unsuspend() {
	kill(this->pid, SIGCONT);
}
#endif

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

bool ACTIVE_TASK::update_time() {
    FILE* app_fp;
    char app_path[256];
    APP_OUT ao;

    sprintf(app_path, "%s/%s", dirname, APP_TO_CORE_FILE);
    app_fp = fopen(app_path, "r");
    if(!app_fp) return false;
    parse_app_file(app_fp, ao);
    if(!ao.checkpointed) return false;
    cpu_time = ao.cpu_time_at_checkpoint + prev_cpu_time;
    return true;
}

bool ACTIVE_TASK_SET::poll_time() {
    ACTIVE_TASK* atp;
    unsigned int i;
    bool updated;
    for(i=0; i<active_tasks.size(); i++) {
        atp = active_tasks[i];
        updated |= atp->update_time();
    }
    return updated;
}

int ACTIVE_TASK::write(FILE* fout) {
    fprintf(fout,
        "<active_task>\n"
        "    <project_master_url>%s</project_master_url>\n"
        "    <result_name>%s</result_name>\n"
        "    <app_version_num>%d</app_version_num>\n"
        "    <slot>%d</slot>\n"
        "    <cpu_time>%f</cpu_time>\n"
        "    <prev_cpu_time>%f</prev_cpu_time>\n"
        "</active_task>\n",
        result->project->master_url,
        result->name,
        app_version->version_num,
        slot,
        cpu_time,
        prev_cpu_time
    );
    return 0;
}

int ACTIVE_TASK::parse(FILE* fin, CLIENT_STATE* cs) {
    char buf[256], result_name[256], project_master_url[256];
    int app_version_num=0;
    PROJECT* project;

    strcpy(result_name, "");
    strcpy(project_master_url, "");
    cpu_time = 0;
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
        else if (parse_double(buf, "<cpu_time>", cpu_time)) continue;
	else if (parse_double(buf, "<prev_cpu_time>", prev_cpu_time)) continue;
        else fprintf(stderr, "ACTIVE_TASK::parse(): unrecognized %s\n", buf);
    }
    return -1;
}

int ACTIVE_TASK_SET::write(FILE* fout) {
    unsigned int i;
    fprintf(fout, "<active_task_set>\n");
    for (i=0; i<active_tasks.size(); i++) {
        active_tasks[i]->write(fout);
    }
    fprintf(fout, "</active_task_set>\n");
    return 0;
}

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
