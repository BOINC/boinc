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
#include <afxwin.h>
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
#include "graphics_api.h"

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
    strcpy(slot_dir, "");
}

int ACTIVE_TASK::init(RESULT* rp) {
    result = rp;
    wup = rp->wup;
    app_version = wup->avp;
    max_cpu_time = rp->wup->max_processing;
    max_disk_usage = rp->wup->max_disk;
    
    return 0;
}

// Start a task in a slot directory.  This includes setting up soft links,
// passing preferences, and starting the process
//
// Current dir is top-level BOINC dir
//
int ACTIVE_TASK::start(bool first_time) {
    char exec_name[256], file_path[256], link_path[256], temp[256], exec_path[256];
    unsigned int i;
    FILE_REF file_ref;
    FILE_INFO* fip;
    int retval;
    char init_data_path[256], graphics_data_path[256], fd_init_path[256];
    FILE *f;
    APP_INIT_DATA aid;
    GRAPHICS_INFO gi;

    if (first_time) {
        checkpoint_cpu_time = 0;
    }
    current_cpu_time = checkpoint_cpu_time;
    starting_cpu_time = checkpoint_cpu_time;
    fraction_done = 0;

    gi.xsize = 800;
    gi.ysize = 600;
    gi.graphics_mode = MODE_WINDOW;
    gi.refresh_period = 0.1;

    memset(&aid, 0, sizeof(aid));

    // TODO: fill in the app prefs, user name, team name, etc.
    aid.checkpoint_period = DEFAULT_CHECKPOINT_PERIOD;
    aid.fraction_done_update_period = DEFAULT_FRACTION_DONE_UPDATE_PERIOD;
    aid.wu_cpu_time = checkpoint_cpu_time;

    sprintf(init_data_path, "%s%s%s", slot_dir, PATH_SEPARATOR, INIT_DATA_FILE);
    f = fopen(init_data_path, "w");
    if (!f) {
        if (log_flags.task_debug) {
            printf("Failed to open core to app prefs file %s.\n", init_data_path);
        }
        return ERR_FOPEN;
    }
    write_init_data_file(f, aid);
    
    fclose(f);

    sprintf(graphics_data_path, "%s%s%s", slot_dir, PATH_SEPARATOR, GRAPHICS_DATA_FILE);
    f = fopen(graphics_data_path, "w");
    if (!f) {
        if (log_flags.task_debug) {
            printf("Failed to open core to app graphics prefs file %s.\n", graphics_data_path);
        }
        return ERR_FOPEN;
    }
    retval = write_graphics_file(f, &gi);
    fclose(f);

    sprintf(fd_init_path, "%s%s%s", slot_dir, PATH_SEPARATOR, FD_INIT_FILE);
    f = fopen(fd_init_path, "w");
    if (!f) {
        if(log_flags.task_debug) {
            printf("Failed to open init file %s.\n", fd_init_path);
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
            strcpy(exec_path, file_path);
        }
        if (first_time) {
            sprintf(link_path, "%s%s%s", slot_dir, PATH_SEPARATOR, fip->name);
            sprintf(temp, "..%s..%s%s", PATH_SEPARATOR, PATH_SEPARATOR, file_path );
            retval = boinc_link( temp, link_path);
            if (log_flags.task_debug) {
                printf("link %s to %s\n", file_path, link_path);
            }
            if (retval) {
                perror("link");
                fclose(f);
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
                sprintf(link_path, "%s%s%s", slot_dir, PATH_SEPARATOR, file_ref.open_name);
                sprintf(temp, "..%s..%s%s", PATH_SEPARATOR, PATH_SEPARATOR, file_path );
                if (log_flags.task_debug) {
                    printf("link %s to %s\n", file_path, link_path);
                }
                retval = boinc_link(temp, link_path);
                if (retval) {
                    perror("link");
                    fclose(f);
                    return retval;
                }
            }
        } else {
            sprintf(temp, "..%s..%s%s", PATH_SEPARATOR, PATH_SEPARATOR, file_path);
            write_fd_init_file(f, temp, file_ref.fd, 1);
        }
    }

    // hook up the output files using BOINC soft links
    //
    for (i=0; i<result->output_files.size(); i++) {
        file_ref = result->output_files[i];
        get_pathname(file_ref.file_info, file_path);
        if (strlen(file_ref.open_name)) {
            if (first_time) {
                // the following is a relic of using hard links.  not needed
                //int fd = creat(file_path, 0660);
                //close(fd);
                sprintf(link_path, "%s%s%s", slot_dir, PATH_SEPARATOR, file_ref.open_name);
                sprintf(temp, "..%s..%s%s", PATH_SEPARATOR, PATH_SEPARATOR, file_path );
                if (log_flags.task_debug) {
                    printf("link %s to %s\n", file_path, link_path);
                }
                retval = boinc_link(temp, link_path);
                if (retval) {
                    fclose(f);
                    perror("link");
                    return retval;
                }
            }
        } else {
            sprintf(temp, "..%s..%s%s", PATH_SEPARATOR, PATH_SEPARATOR, file_path);
            write_fd_init_file(f, temp, file_ref.fd, 0);
        }
    }

    fclose(f);

    sprintf(temp, "%s%s%s", slot_dir, PATH_SEPARATOR, SUSPEND_QUIT_FILE);
    file_delete(temp);

#ifdef _WIN32
    PROCESS_INFORMATION process_info;
    STARTUPINFO startup_info;
    char slotdirpath[256];
    char cmd_line[512];
    int win_error;

    memset( &process_info, 0, sizeof( process_info ) );
    memset( &startup_info, 0, sizeof( startup_info ) );
    startup_info.cb = sizeof(startup_info);
    startup_info.lpReserved = NULL;
    startup_info.lpDesktop = "";

    // NOTE: in Windows, stderr is redirected within boinc_init();

    sprintf( cmd_line, "%s %s", exec_path, wup->command_line );
    // Need to condense argv into a single string
    //if (log_flags.task_debug) print_argv(argv);
    //
    full_path(slot_dir, slotdirpath);
    if (!CreateProcess(exec_path,
        cmd_line,
        NULL,
        NULL,
        FALSE,
        CREATE_NEW_PROCESS_GROUP|CREATE_NO_WINDOW|IDLE_PRIORITY_CLASS,
        NULL,
        slotdirpath,
        &startup_info,
        &process_info
    )) {
        win_error = GetLastError();
        char *errorargs[] = {app_version->app_name,"","","",""};
        LPVOID lpMsgBuf;
        FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_FROM_SYSTEM,
            NULL, win_error, 0, (LPTSTR)&lpMsgBuf, 0, errorargs);

        // check for an error; if there is one, set error information for the currect result
        if(win_error) {
            gstate.report_project_error(*result, win_error, (LPTSTR)&lpMsgBuf,CLIENT_COMPUTING);
            LocalFree(lpMsgBuf);
            return -1;
        }
        fprintf(stdout, "CreateProcess: %s\n", (LPCTSTR)lpMsgBuf);
        LocalFree(lpMsgBuf);
    }
    pid_handle = process_info.hProcess;
    thread_handle = process_info.hThread;
#else
    char* argv[100];
    pid = fork();
    if (pid == 0) {
        
        // from here on we're running in a new process.
        // If an error happens, exit nonzero so that the core client
        // knows there was a problem.

        // chdir() into the slot directory
        //
        retval = chdir(slot_dir);
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
        boinc_resolve_filename(exec_name, temp, sizeof(temp));
        retval = execv(temp, argv);
        fprintf(stderr, "execv failed: %d\n", retval);
        perror("execv");
        exit(1);
    }
    if (log_flags.task_debug) printf("forked process: pid %d\n", pid);
#endif
    state = PROCESS_RUNNING;
    result->active_task_state = PROCESS_RUNNING;
    result->client_state = CLIENT_COMPUTING;
    return 0;
}

// Sends a request to the process of this active task to exit.
// If it doesn't exit within a set time (seconds), the process is terminated
//
int ACTIVE_TASK::request_exit() {
    char susp_file[256];

    get_slot_dir(slot, slot_dir);
    sprintf(susp_file, "%s%s%s", slot_dir, PATH_SEPARATOR, SUSPEND_QUIT_FILE);
    FILE *fp = fopen(susp_file, "w");
    if (!fp) return ERR_FOPEN;
    write_suspend_quit_file(fp, false, true);
    fclose(fp);
    return 0;
}

int ACTIVE_TASK::kill_task() {
#ifdef _WIN32
    TerminateProcess(pid_handle, -1);
    return 0;
#else
    return kill(pid, SIGKILL);
#endif
}

// Inserts an active task into the ACTIVE_TASK_SET and starts it up
//
int ACTIVE_TASK_SET::insert(ACTIVE_TASK* atp) {
    int retval;

    get_slot_dir(atp->slot, atp->slot_dir);
    clean_out_dir(atp->slot_dir);
    retval = atp->start(true);
    if (retval) return retval;
    active_tasks.push_back(atp);
    return 0;
}

// Checks if any child processes have exited and records their final CPU time
//
bool ACTIVE_TASK_SET::poll() {
    ACTIVE_TASK* atp;
    unsigned int j;

#ifdef _WIN32
    unsigned long exit_code;
    FILETIME creation_time, exit_time, kernel_time, user_time;
    ULARGE_INTEGER tKernel, tUser;
    LONGLONG totTime;
    bool found = false;

    for (int i=0; i<active_tasks.size(); i++) {
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
            }
            atp->result->final_cpu_time = atp->checkpoint_cpu_time;
            if (exit_code != STILL_ACTIVE) {
                found = true;
                if (atp->state == PROCESS_ABORT_PENDING) {
                    atp->state = PROCESS_ABORTED;
		    atp->result->active_task_state = PROCESS_ABORTED;
		    gstate.report_project_error(*(atp->result), 0, "process was aborted\n",CLIENT_COMPUTING);
                } else {
                    atp->state = PROCESS_EXITED;
                    atp->exit_status = exit_code;
                    atp->result->exit_status = atp->exit_status;
                    atp->result->active_task_state = PROCESS_EXITED;
		    //if a nonzero error code, then report it
		    if(exit_code)
		      {
			gstate.report_project_error(*(atp->result),0,"process exited with a non zero exit code\n",CLIENT_COMPUTING);
		      }
                }
                CloseHandle(atp->pid_handle);
                CloseHandle(atp->thread_handle);
                atp->read_stderr_file();
                clean_out_dir(atp->slot_dir);
            }
        }
    }
    if (found) return true;
#else
    struct rusage rs;
    int pid;
    int stat;

    pid = wait3(&stat, WNOHANG, &rs);
    if (pid > 0) {
        if (log_flags.task_debug) printf("process %d is done\n", pid);
        atp = lookup_pid(pid);
        if (!atp) {
            fprintf(stderr, "ACTIVE_TASK_SET::poll(): pid %d not found\n", pid);
            return true;
        }
        double x = rs.ru_utime.tv_sec + rs.ru_utime.tv_usec/1.e6;
        atp->result->final_cpu_time = atp->starting_cpu_time + x;
        if (atp->state == PROCESS_ABORT_PENDING) {
            atp->state = PROCESS_ABORTED;
	    atp->result->active_task_state =  PROCESS_ABORTED;
	    gstate.report_project_error(*(atp->result),0,"process was aborted\n",CLIENT_COMPUTING);
        } else {
            if (WIFEXITED(stat)) {
                atp->state = PROCESS_EXITED;
                atp->exit_status = WEXITSTATUS(stat);
                atp->result->exit_status = atp->exit_status;
		atp->result->active_task_state = PROCESS_EXITED;
		
		//if exit_status != 0, then we don't need to upload the files for the result of this app 
		if(atp->exit_status)
		  {
		    gstate.report_project_error(*(atp->result),0,"process exited with a nonzero exit code\n",CLIENT_COMPUTING);
		  }
		 if (log_flags.task_debug) printf("process exited: status %d\n", atp->exit_status);
            } else if (WIFSIGNALED(stat)) {
                atp->state = PROCESS_WAS_SIGNALED;
                atp->signal = WTERMSIG(stat);
                atp->result->signal = atp->signal;
		atp->result->active_task_state = PROCESS_WAS_SIGNALED;
		gstate.report_project_error(*(atp->result),0,"process was signaled\n",CLIENT_COMPUTING);
                if (log_flags.task_debug) printf("process was signaled: %d\n", atp->signal);
            } else {
	      atp->state = PROCESS_EXIT_UNKNOWN;
	      atp->result->state  = PROCESS_EXIT_UNKNOWN;
            }
        }

        atp->read_stderr_file();
        clean_out_dir(atp->slot_dir);

        return true;
    }
#endif

    // check for processes that have exceeded their maximum CPU time
    // and abort them
    //
    for (j=0; j<active_tasks.size(); j++) {
        atp = active_tasks[j];
        if (atp->current_cpu_time > atp->max_cpu_time) {
            fprintf(stderr, "Aborting task: exceeded CPU time limit %f\n", atp->max_cpu_time);
            atp->abort();
            return true;
        }
    }
    return false;
}

int ACTIVE_TASK::abort() {
    state = PROCESS_ABORT_PENDING;
    result->active_task_state = PROCESS_ABORT_PENDING;
    return kill_task();
}

// check for the stderr file, copy to result record
//
bool ACTIVE_TASK::read_stderr_file() {
    char path[256];
    int n;

    sprintf(path, "%s%s%s", slot_dir, PATH_SEPARATOR, STDERR_FILE);
    FILE* f = fopen(path, "r");
    if (f) {
        n = fread(result->stderr_out, 1, STDERR_MAX_LEN, f);
        result->stderr_out[n] = 0;
        fclose(f);
            return true;
    }
    return false;
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

// suspend all currently running tasks
//
void ACTIVE_TASK_SET::suspend_all() {
    unsigned int i;
    ACTIVE_TASK* atp;
    for (i=0; i<active_tasks.size(); i++) {
        atp = active_tasks[i];
        if(atp->suspend())
	{
	 fprintf(stderr, "ACTIVE_TASK_SET::exit_tasks(): could not suspend active_task\n");
	} 
    }
}

// resume all currently running tasks
//
void ACTIVE_TASK_SET::unsuspend_all() {
    unsigned int i;
    ACTIVE_TASK* atp;
    for (i=0; i<active_tasks.size(); i++) 
      {
        atp = active_tasks[i];
        if(atp->unsuspend())
	  {
	    fprintf(stderr, "ACTIVE_TASK_SET::exit_tasks(): could not suspend active_task\n");
	  }
      } 
}

// initiate exit of all currently running tasks
//
void ACTIVE_TASK_SET::exit_tasks() {
    unsigned int i;
    ACTIVE_TASK *atp;
    for (i=0; i<active_tasks.size(); i++) {
        atp = active_tasks[i];
        if(atp->request_exit())
	{
	  fprintf(stderr, "ACTIVE_TASK_SET::exit_tasks(): could not suspend active_task\n");
	}
    }
}

// suspend a task
//
int ACTIVE_TASK::suspend() {
#ifdef _WIN32
	SuspendThread( thread_handle );
#else
    kill(pid, SIGSTOP);
#endif
    return 0;
}

// resume a suspended task
//
int ACTIVE_TASK::unsuspend() {
#ifdef _WIN32
	ResumeThread( thread_handle );
#else
    kill(pid, SIGCONT);
#endif
    return 0;
}

// Remove an ACTIVE_TASK from the set.
// Do this only if you're sure that the process has exited.
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
        atp->init(atp->result);
        get_slot_dir(atp->slot, atp->slot_dir);
        retval = atp->start(false);
        if (log_flags.task) {
            printf("restarting application for result %s\n", atp->result->name);
        }
        if (retval) {
            fprintf(stderr, "ACTIVE_TASKS::restart_tasks(); restart failed: %d\n", retval);
	    atp->result->active_task_state = PROCESS_COULDNT_START;
	    atp->result->client_state = CLIENT_COMPUTING;
	    gstate.report_project_error(*(atp->result),retval,"Couldn't restart the app for this result.\n",CLIENT_COMPUTING);
	   
            active_tasks.erase(iter);
        } else {
            iter++;
        }
    }
    return 0;
}

// See if the app has generated a new fraction-done file.
// If so read it and return true.
//
bool ACTIVE_TASK::check_app_status_files() {
    FILE* f;
    char path[256];
    bool found = false;
    int retval;

    sprintf(path, "%s%s%s", slot_dir, PATH_SEPARATOR, FRACTION_DONE_FILE);
    f = fopen(path, "r");
    if (f) {
        found = true;
        parse_fraction_done_file(f, fraction_done, current_cpu_time, checkpoint_cpu_time);
        fclose(f);
        retval = file_delete(path);
        if (retval) {
            fprintf(stderr,
                "ACTIVE_TASK.check_app_status_files: could not delete %s: %d\n",
                path, retval
            );
        }
    }
    return found;
}

// Returns the estimated time to completion (in seconds) of this task,
// based on current reported CPU time and fraction done
//
double ACTIVE_TASK::est_time_to_completion() {
    if (fraction_done <= 0 || fraction_done > 1) {
        return -1;
    }
    return (current_cpu_time / fraction_done) - current_cpu_time;
}

// size of output files and files in slot dir
//
int ACTIVE_TASK::current_disk_usage(double& size) {
    double x;
    unsigned int i;
    int retval;
    FILE_INFO* fip;
    char path[256];

    retval = dir_size(slot_dir, size);
    if (retval) return retval;
    for (i=0; i<result->output_files.size(); i++) {
        fip = result->output_files[i].file_info;
        get_pathname(fip, path);
        retval = file_size(path, x);
        if (!retval) size += x;
    }
    return 0;
}

// Poll each of the currently running tasks and get their CPU time
//
bool ACTIVE_TASK_SET::poll_time() {
    ACTIVE_TASK* atp;
    unsigned int i;
    bool updated = false;

    for (i=0; i<active_tasks.size(); i++) {
        atp = active_tasks[i];
        updated |= atp->check_app_status_files();
    }

    return updated;
}

// Gets the next available free slot, or returns -1 if all slots are full
// TODO: don't use malloc here
//
int ACTIVE_TASK_SET::get_free_slot(int total_slots) {
    unsigned int i;
    char *slot_status;

    if (active_tasks.size() >= (unsigned int)total_slots) {
        return -1;
    }

    slot_status = (char *)calloc( sizeof(char), total_slots );
    if (!slot_status) return -1;
    
    for (i=0; i<active_tasks.size(); i++) {
        if (active_tasks[i]->slot >= 0 && active_tasks[i]->slot < total_slots) {
            slot_status[active_tasks[i]->slot] = 1;
        }
    }

    for (i=0; i<(unsigned int)total_slots; i++) {
        if (!slot_status[i]) {
            free(slot_status);
            return i;
        }
    }

    free(slot_status);
    return -1;
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
        else if (parse_str(buf, "<result_name>", result_name, sizeof(result_name))) continue;
        else if (parse_str(buf, "<project_master_url>", project_master_url, sizeof(project_master_url))) continue;
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
