// The contents of this file are subject to the BOINC Public License
// Version 1.0 (the "License"); you may not use this file except in
// compliance with the License. You may obtain a copy of the License at
// http://boinc.berkeley.edu/license_1.0.txt
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

#include "cpp.h"

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
#if HAVE_SYS_IPC_H
#include <sys/ipc.h>
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
#include <math.h>
#include <stdlib.h>

#include "client_state.h"
#include "client_types.h"
#include "error_numbers.h"
#include "filesys.h"
#include "file_names.h"
#include "parse.h"
#include "shmem.h"
#include "util.h"

#include "app.h"

// value for setpriority(2)
static const int PROCESS_IDLE_PRIORITY = 19;

// Goes through an array of strings, and prints each string
//
static int debug_print_argv(char** argv) {
    int i;

    log_messages.printf(ClientMessages::DEBUG_TASK, "Arguments:");
    ++log_messages;
    for (i=0; argv[i]; i++) {
        log_messages.printf(
            ClientMessages::DEBUG_TASK,
            "argv[%d]: %s\n", i, argv[i]
        );
    }
    --log_messages;

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
    graphics_requested_mode = MODE_HIDE_GRAPHICS;
    graphics_request_time = time(0);
    graphics_acked_mode = MODE_UNSUPPORTED;
    graphics_mode_before_ss = MODE_HIDE_GRAPHICS;
    current_cpu_time = working_set_size = 0;

	fraction_done = 0;
	frac_rate_of_change = 0;
	last_frac_done = 0;
	recent_change = 0;
	last_frac_update = 0;
	starting_cpu_time = 0;
	checkpoint_cpu_time = 0;
	current_cpu_time = 0;
	working_set_size = 0;
}

int ACTIVE_TASK::init(RESULT* rp) {
    result = rp;
    wup = rp->wup;
    app_version = wup->avp;
    max_cpu_time = rp->wup->rsc_fpops_bound/gstate.host_info.p_fpops;
    max_disk_usage = rp->wup->rsc_disk_bound;
    max_mem_usage = rp->wup->rsc_memory_bound;

    return 0;
}

int ACTIVE_TASK::link_user_files() {
    PROJECT* project = wup->project;
    unsigned int i;
    FILE_REF fref;
    FILE_INFO* fip;
    char link_path[256], buf[256], file_path[256];
    int retval;

    for (i=0; i<project->user_files.size(); i++) {
        fref = project->user_files[i];
        fip = fref.file_info;
        if (fip->status != FILE_PRESENT) continue;
        get_pathname(fip, file_path);
        sprintf(link_path, "%s%s%s", slot_dir, PATH_SEPARATOR, strlen(fref.open_name)?fref.open_name:fip->name);
        sprintf(buf, "..%s..%s%s", PATH_SEPARATOR, PATH_SEPARATOR, file_path);
        retval = boinc_link(buf, link_path);
        if (retval) return retval;
    }
    return 0;
}

int ACTIVE_TASK::write_app_init_file(APP_INIT_DATA& aid) {
    FILE *f;
    char init_data_path[256];
    int retval;

    memset(&aid, 0, sizeof(aid));

    safe_strcpy(aid.app_name, wup->app->name);
    safe_strcpy(aid.user_name, wup->project->user_name);
    safe_strcpy(aid.team_name, wup->project->team_name);
    if (wup->project->project_specific_prefs.length()) {
        strcpy(aid.app_preferences, wup->project->project_specific_prefs.c_str());
    }
    aid.user_total_credit = wup->project->user_total_credit;
    aid.user_expavg_credit = wup->project->user_expavg_credit;
    aid.host_total_credit = wup->project->host_total_credit;
    aid.host_expavg_credit = wup->project->host_expavg_credit;
    aid.checkpoint_period = gstate.global_prefs.disk_interval;
    aid.fraction_done_update_period = DEFAULT_FRACTION_DONE_UPDATE_PERIOD;
    aid.shm_key = 0;
    // when writing the wu_cpu_time to the app init file
    // use the total cpu time from the start of the session
    // (starting_cpu_time) rather than the total cpu time
    // since the last checkpoint (checkpoint_cpu_time).
    aid.wu_cpu_time = starting_cpu_time;

    sprintf(init_data_path, "%s%s%s", slot_dir, PATH_SEPARATOR, INIT_DATA_FILE);
    f = fopen(init_data_path, "w");
    if (!f) {
        msg_printf(wup->project, MSG_ERROR,
            "Failed to open core-to-app prefs file %s",
            init_data_path
        );
        return ERR_FOPEN;
    }

    // make a unique key for core/app shared memory segment
    //
#ifdef _WIN32
    sprintf(aid.comm_obj_name, "boinc_%d", slot);
#elif HAVE_SYS_IPC_H
    aid.shm_key = ftok(init_data_path, slot);
#else
#error shared memory key generation unimplemented
#endif

    retval = write_init_data_file(f, aid);
    fclose(f);
    return retval;
}

// Start a task in a slot directory.
// This includes setting up soft links,
// passing preferences, and starting the process
//
// Current dir is top-level BOINC dir
//
int ACTIVE_TASK::start(bool first_time) {
    char exec_name[256], file_path[256], link_path[256], buf[256], exec_path[256];
    unsigned int i;
    FILE_REF file_ref;
    FILE_INFO* fip;
    int retval;
    char graphics_data_path[256], fd_init_path[256];
    FILE *f;
    GRAPHICS_INFO gi;
    APP_INIT_DATA aid;

    ScopeMessages scope_messages(log_messages, ClientMessages::DEBUG_TASK);
    scope_messages.printf("ACTIVE_TASK::start(first_time=%d)\n", first_time);

    if (first_time) {
        checkpoint_cpu_time = 0;
    }
    current_cpu_time = checkpoint_cpu_time;
    starting_cpu_time = checkpoint_cpu_time;
    fraction_done = 0;

    gi.xsize = 800;
    gi.ysize = 600;
    gi.refresh_period = 0.1;

    retval = write_app_init_file(aid);
    if (retval) return retval;

    sprintf(graphics_data_path, "%s%s%s", slot_dir, PATH_SEPARATOR, GRAPHICS_DATA_FILE);
    f = fopen(graphics_data_path, "w");
    if (!f) {
        msg_printf(wup->project, MSG_ERROR,
            "Failed to open core-to-app graphics prefs file %s",
            graphics_data_path
        );
        return ERR_FOPEN;
    }
    retval = write_graphics_file(f, &gi);
    fclose(f);

    sprintf(fd_init_path, "%s%s%s", slot_dir, PATH_SEPARATOR, FD_INIT_FILE);
    f = fopen(fd_init_path, "w");
    if (!f) {
        msg_printf(wup->project, MSG_ERROR, "Failed to open init file %s", fd_init_path);
        return ERR_FOPEN;
    }

    // make soft links to the executable(s)
    //
    for (i=0; i<app_version->app_files.size(); i++) {
        FILE_REF fref = app_version->app_files[i];
        fip = fref.file_info;
        get_pathname(fip, file_path);
        if (fref.main_program) {
            safe_strcpy(exec_name, fip->name);
            safe_strcpy(exec_path, file_path);
        }
        if (first_time) {
            sprintf(link_path, "%s%s%s", slot_dir, PATH_SEPARATOR, strlen(fref.open_name)?fref.open_name:fip->name);
            sprintf(buf, "..%s..%s%s", PATH_SEPARATOR, PATH_SEPARATOR, file_path);
            retval = boinc_link(buf, link_path);
            scope_messages.printf("ACTIVE_TASK::start(): Linking %s to %s\n", file_path, link_path);
            if (retval) {
                msg_printf(wup->project, MSG_ERROR, "Can't link %s to %s", file_path, link_path);
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
                sprintf(buf, "..%s..%s%s", PATH_SEPARATOR, PATH_SEPARATOR, file_path );
                scope_messages.printf("ACTIVE_TASK::start(): link %s to %s\n", file_path, link_path);
                if (file_ref.copy_file) {
                    retval = boinc_copy(file_path, link_path);
                    if (retval) {
                        msg_printf(wup->project, MSG_ERROR, "Can't copy %s to %s", file_path, link_path);
                        fclose(f);
                        return retval;
                    }
				} else {
                    retval = boinc_link(buf, link_path);
                    if (retval) {
                        msg_printf(wup->project, MSG_ERROR, "Can't link %s to %s", file_path, link_path);
                        fclose(f);
                        return retval;
                    }
                }
            }
        } else {
            sprintf(buf, "..%s..%s%s", PATH_SEPARATOR, PATH_SEPARATOR, file_path);
            retval = write_fd_init_file(f, buf, file_ref.fd, true);
            if (retval) return retval;
        }
    }

    // hook up the output files using BOINC soft links
    //
    for (i=0; i<result->output_files.size(); i++) {
        file_ref = result->output_files[i];
        get_pathname(file_ref.file_info, file_path);
        if (strlen(file_ref.open_name)) {
            if (first_time) {
                sprintf(link_path, "%s%s%s", slot_dir, PATH_SEPARATOR, file_ref.open_name);
                sprintf(buf, "..%s..%s%s", PATH_SEPARATOR, PATH_SEPARATOR, file_path );
                scope_messages.printf("ACTIVE_TASK::start(): link %s to %s\n", file_path, link_path);
                retval = boinc_link(buf, link_path);
                if (retval) {
                    msg_printf(wup->project, MSG_ERROR, "Can't link %s to %s", file_path, link_path);
                    fclose(f);
                    return retval;
                }
            }
        } else {
            sprintf(buf, "..%s..%s%s", PATH_SEPARATOR, PATH_SEPARATOR, file_path);
            retval = write_fd_init_file(f, buf, file_ref.fd, false);
            if (retval) return retval;
        }
    }

    fclose(f);

    link_user_files();

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

    sprintf(buf, "%s%s", QUIT_PREFIX, aid.comm_obj_name);
    quitRequestEvent = CreateEvent(0, FALSE, FALSE, buf);

    // create core/app share mem segment
    //
    sprintf(buf, "%s%s", SHM_PREFIX, aid.comm_obj_name);
    shm_handle = create_shmem(buf, APP_CLIENT_SHMEM_SIZE,
        (void **)&app_client_shm.shm
    );
    app_client_shm.reset_msgs();

    // NOTE: in Windows, stderr is redirected within boinc_init();

    sprintf(cmd_line, "%s %s", exec_path, wup->command_line);
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
            NULL, win_error, 0, (LPTSTR)&lpMsgBuf, 0, errorargs
        );

        state = PROCESS_COULDNT_START;
        result->active_task_state = PROCESS_COULDNT_START;
        if (win_error) {
            gstate.report_result_error(*result, win_error, "CreateProcess(): %s", (LPTSTR)&lpMsgBuf);
            LocalFree(lpMsgBuf);
            return ERR_EXEC;
        }
        msg_printf(wup->project, MSG_ERROR, "CreateProcess: %s", (LPCTSTR)lpMsgBuf);
        LocalFree(lpMsgBuf);
    }
    pid = process_info.dwProcessId;
    pid_handle = process_info.hProcess;
    thread_handle = process_info.hThread;
#else
    char* argv[100];

    // Set up core/app shared memory seg
    //
    shm_key = aid.shm_key;
    if (!create_shmem(
        shm_key, APP_CLIENT_SHMEM_SIZE, (void**)&app_client_shm.shm)
    ) {
        app_client_shm.reset_msgs();
    }

    pid = fork();
    if (pid == -1) {
        state = PROCESS_COULDNT_START;
        result->active_task_state = PROCESS_COULDNT_START;
        gstate.report_result_error(*result, -1, "fork(): %s", strerror(errno));
        msg_printf(wup->project, MSG_ERROR, "fork(): %s", strerror(errno));
        return ERR_FORK;
    }
    if (pid == 0) {
        // from here on we're running in a new process.
        // If an error happens, exit nonzero so that the core client
        // knows there was a problem.

        // chdir() into the slot directory
        //
        retval = chdir(slot_dir);
        if (retval) {
            perror("chdir");
            _exit(retval);
        }

        // hook up stderr to a specially-named file
        //
        freopen(STDERR_FILE, "a", stderr);

        argv[0] = exec_name;
        parse_command_line(wup->command_line, argv+1);
        debug_print_argv(argv);
        sprintf(buf, "..%s..%s%s", PATH_SEPARATOR, PATH_SEPARATOR, exec_path );
        retval = execv(buf, argv);
        msg_printf(wup->project, MSG_ERROR, "execv failed: %d\n", retval);
        perror("execv");
        _exit(errno);
    }

    scope_messages.printf("ACTIVE_TASK::start(): forked process: pid %d\n", pid);

    // set idle process priority
#ifdef HAVE_SETPRIORITY
    if (setpriority(PRIO_PROCESS, pid, PROCESS_IDLE_PRIORITY)) {
        perror("setpriority");
    }
#endif

#endif
    state = PROCESS_RUNNING;
    result->active_task_state = PROCESS_RUNNING;
    return 0;
}

// Send a quit signal.
// Normally this is caught by the process, which can checkpoint
//
int ACTIVE_TASK::request_exit() {
#ifdef _WIN32
    return !SetEvent(quitRequestEvent);
#else
    return kill(pid, SIGQUIT);
#endif
}

// send a kill signal.
// This is not caught by the process
//
int ACTIVE_TASK::kill_task() {
#ifdef _WIN32
    return !TerminateProcess(pid_handle, -1);
#else
    return kill(pid, SIGKILL);
#endif
}

#if !defined(HAVE_WAIT4) && defined(HAVE_WAIT3)
#include <map>
struct proc_info_t {
  int status;
  rusage r;
  proc_info_t() {};
  proc_info_t(int s, const rusage &ru);
};

proc_info_t::proc_info_t(int s, const rusage &ru) : status(s), r(ru) {}

pid_t wait4(pid_t pid, int *statusp, int options, struct rusage *rusagep) {
  static std::map<pid_t,proc_info_t> proc_info;
  pid_t tmp_pid=0;

  if (!pid) {
    return wait3(statusp,options,rusagep);
  } else {
    if (proc_info.find(pid) == proc_info.end()) {
      do {
        tmp_pid=wait3(statusp,options,rusagep);
        if ((tmp_pid>0) && (tmp_pid != pid)) {
	  proc_info[tmp_pid]=proc_info_t(*statusp,*rusagep);
	  if (!(options && WNOHANG)) {
	    tmp_pid=0;
	  }
        } else {
	  return pid;
        }
      } while (!tmp_pid);
    } else {
      *statusp=proc_info[pid].status;
      *rusagep=proc_info[pid].r;
      proc_info.erase(pid);
      return pid;
    }
  }
}
#endif

bool ACTIVE_TASK::task_exited() {
#ifdef _WIN32
    unsigned long exit_code;
    if (GetExitCodeProcess(pid_handle, &exit_code)) {
        if (exit_code != STILL_ACTIVE) {
            return true;
        }
    }
#else
    int my_pid, stat;
    struct rusage rs;

    my_pid = wait4(pid, &stat, WNOHANG, &rs);
    if (my_pid == pid) {
        double x = rs.ru_utime.tv_sec + rs.ru_utime.tv_usec/1.e6;
        result->final_cpu_time = current_cpu_time =
            checkpoint_cpu_time = starting_cpu_time + x;
        return true;
    }
#endif
    return false;
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

#if 0
// Deallocate memory to prevent unneeded reporting of memory leaks
//
void ACTIVE_TASK_SET::free_mem() {
    vector<ACTIVE_TASK*>::iterator at_iter;
    ACTIVE_TASK *at;

    at_iter = active_tasks.begin();
    while (at_iter != active_tasks.end()) {
        at = active_tasks[0];
        at_iter = active_tasks.erase(at_iter);
        delete at;
    }
}
#endif

// Do period checks on running apps:
// - get latest CPU time and % done info
// - check if any has exited, and clean up
// - see if any has exceeded its CPU or disk space limits, and abort it
//
bool ACTIVE_TASK_SET::poll() {
    bool action;

    action = check_app_exited();
    if (action) return true;
    action = check_rsc_limits_exceeded();
    if (action) return true;
    if (get_status_msgs()) {
        gstate.set_client_state_dirty("ACTIVE_TASK_SET::poll");
    }
    return false;
}

bool ACTIVE_TASK_SET::check_app_exited() {
    ACTIVE_TASK* atp;

    ScopeMessages scope_messages(log_messages, ClientMessages::DEBUG_TASK);

#ifdef _WIN32
    unsigned long exit_code;
    bool found = false;

    for (int i=0; i<active_tasks.size(); i++) {
        atp = active_tasks[i];
        if (GetExitCodeProcess(atp->pid_handle, &exit_code)) {
            if (exit_code != STILL_ACTIVE) {
                atp->get_status_msg();
                atp->result->final_cpu_time = atp->checkpoint_cpu_time;
                found = true;
                if (atp->state == PROCESS_ABORT_PENDING) {
                    atp->state = PROCESS_ABORTED;
                    atp->result->active_task_state = PROCESS_ABORTED;
                    gstate.report_result_error(
                        *(atp->result), 0, "process was aborted by core client"
                    );
                } else {
                    atp->state = PROCESS_EXITED;
                    atp->exit_status = exit_code;
                    atp->result->exit_status = atp->exit_status;
                    atp->result->active_task_state = PROCESS_EXITED;
                    //if a nonzero error code, then report it
                    if (exit_code) {
                        gstate.report_result_error(
                            *(atp->result), 0,
                            "process exited with code %d (0x%x)",
                            exit_code, exit_code
                        );
                    }
                }
                CloseHandle(atp->pid_handle);
                CloseHandle(atp->thread_handle);
                CloseHandle(atp->quitRequestEvent);
                atp->read_stderr_file();
                clean_out_dir(atp->slot_dir);

                // detach from shared mem.  This will destroy shmem seg
                // since we're the last attachment
                //
                if (atp->app_client_shm.shm) {
                    detach_shmem(atp->shm_handle, atp->app_client_shm.shm);
                    atp->app_client_shm.shm = NULL;
                }
            }
        }
    }
    if (found) return true;
#else
    int pid;
    int stat;
    struct rusage rs;

    pid = wait4(0, &stat, WNOHANG, &rs);
    if (pid > 0) {
        scope_messages.printf("ACTIVE_TASK_SET::check_app_exited(): process %d is done\n", pid);
        atp = lookup_pid(pid);
        if (!atp) {
            msg_printf(NULL, MSG_ERROR, "ACTIVE_TASK_SET::check_app_exited(): pid %d not found\n", pid);
            return true;
        }
        atp->get_status_msg();
        atp->result->final_cpu_time = atp->checkpoint_cpu_time;
        if (atp->state == PROCESS_ABORT_PENDING) {
            atp->state = PROCESS_ABORTED;
            atp->result->active_task_state = PROCESS_ABORTED;
            gstate.report_result_error(
                *(atp->result), 0, "process was aborted by core client"
            );
        } else {
            if (WIFEXITED(stat)) {
                atp->state = PROCESS_EXITED;
                atp->exit_status = WEXITSTATUS(stat);
                atp->result->exit_status = atp->exit_status;
                atp->result->active_task_state = PROCESS_EXITED;

                // If exit_status is nonzero, then we don't need to upload the
                // output files
                //
                if(atp->exit_status) {
                    gstate.report_result_error(
                        *(atp->result), 0,
                        "process exited with code %d (0x%x)",
                        atp->exit_status, atp->exit_status
                    );
                }
                scope_messages.printf("ACTIVE_TASK_SET::check_app_exited(): process exited: status %d\n", atp->exit_status);
            } else if (WIFSIGNALED(stat)) {
                atp->state = PROCESS_WAS_SIGNALED;
                atp->signal = WTERMSIG(stat);
                atp->result->signal = atp->signal;
                atp->result->active_task_state = PROCESS_WAS_SIGNALED;
                gstate.report_result_error(
                    *(atp->result), 0, "process got signal %d", atp->signal
                );
                scope_messages.printf("ACTIVE_TASK_SET::check_app_exited(): process got signal %d\n", atp->signal);
            } else {
                atp->state = PROCESS_EXIT_UNKNOWN;
                atp->result->state = PROCESS_EXIT_UNKNOWN;
            }
        }

        atp->read_stderr_file();
        clean_out_dir(atp->slot_dir);

        // detach from and destroy share mem
        //
        if (atp->app_client_shm.shm) {
            detach_shmem(atp->app_client_shm.shm);
            atp->app_client_shm.shm = NULL;
        }
        destroy_shmem(atp->shm_key);

        return true;
    }
#endif
    return false;
}

// if an app has exceeded its maximum CPU time, abort it
//
bool ACTIVE_TASK::check_max_cpu_exceeded() {
    if (current_cpu_time > max_cpu_time) {
        msg_printf(result->project, MSG_INFO,
            "Aborting result %s: exceeded CPU time limit %f\n",
            result->name, max_cpu_time);
        abort();
        return true;
    }
    return false;
}

// if an app has exceeded its maximum disk usage, abort it
//
bool ACTIVE_TASK::check_max_disk_exceeded() {
    double disk_usage;
    int retval;

    // don't do disk check too often
    //
    retval = current_disk_usage(disk_usage);
    if (retval) {
        msg_printf(0, MSG_ERROR, "Can't get application disk usage");
    } else {
        if (disk_usage > max_disk_usage) {
            msg_printf(
                result->project, MSG_INFO,
                "Aborting result %s: exceeded disk limit %f\n",
                result->name, max_disk_usage
            );
            abort();
            return true;
        }
    }
    return false;
}

#if 0
// if an app has exceeded its maximum allowed memory, abort it
//
bool ACTIVE_TASK::check_max_mem_exceeded() {
    // TODO: calculate working set size elsewhere
    if (working_set_size > max_mem_usage || working_set_size/1048576 > gstate.global_prefs.max_memory_mbytes) {
        msg_printf(
            result->project, MSG_INFO,
            "Aborting result %s: exceeded memory limit %f\n",
            result->name,
            min(max_mem_usage, gstate.global_prefs.max_memory_mbytes*1048576)
        );
        abort();
        return true;
    }
    return false;
}
#endif

// Check if any of the active tasks have exceeded their
// resource limits on disk, CPU time or memory
//
bool ACTIVE_TASK_SET::check_rsc_limits_exceeded() {
    unsigned int j;
    ACTIVE_TASK *atp;
    static time_t last_disk_check_time = 0;

    for (j=0;j<active_tasks.size();j++) {
        atp = active_tasks[j];
        if (atp->check_max_cpu_exceeded()) return true;
        //else if (atp->check_max_mem_exceeded()) return true;
        else if (time(0)>last_disk_check_time + gstate.global_prefs.disk_interval) {
            last_disk_check_time = time(0);
            if (atp->check_max_disk_exceeded()) return true;
        }
    }

    return false;
}

// If process is running, send it a kill signal
// This is done when
// 1) project is reset or detached
// 2) app has exceeded CPU, disk, or mem limits
//
int ACTIVE_TASK::abort() {
    if (state == PROCESS_RUNNING) {
        state = PROCESS_ABORT_PENDING;
        result->active_task_state = PROCESS_ABORT_PENDING;
        kill_task();
    } else {
        state = PROCESS_ABORTED;
    }
    return 0;
}

// check for the stderr file, copy to result record
//
bool ACTIVE_TASK::read_stderr_file() {
    char stderr_file[MAX_BLOB_LEN];
    char path[256];
    int n;

    sprintf(path, "%s%s%s", slot_dir, PATH_SEPARATOR, STDERR_FILE);
    FILE* f = fopen(path, "r");
    if (f) {
        n = fread(stderr_file, 1, sizeof(stderr_file)-1, f);
        fclose(f);
        if (n < 0) return false;
        stderr_file[n] = '\0';
        result->stderr_out += "<stderr_txt>\n";
        result->stderr_out += stderr_file;
        result->stderr_out += "\n</stderr_txt>\n";
        result->stderr_out = result->stderr_out.substr(0,MAX_BLOB_LEN-1);
        return true;
    }
    return false;
}

int ACTIVE_TASK::request_reread_prefs() {
    int retval;
    APP_INIT_DATA aid;
    
    link_user_files();

    retval = write_app_init_file(aid);
    if (retval) return retval;
    app_client_shm.send_graphics_msg(
        CORE_APP_GFX_SEG, GRAPHICS_MSG_REREAD_PREFS, 0
    );
    return 0;
}

void ACTIVE_TASK_SET::request_reread_prefs(PROJECT* project) {
    unsigned int i;
    ACTIVE_TASK* atp;

    for (i=0; i<active_tasks.size(); i++) {
        atp = active_tasks[i];
        if (atp->result->project != project) continue;
        atp->request_reread_prefs();
    }
}

void ACTIVE_TASK::request_graphics_mode(int mode) {
    app_client_shm.send_graphics_msg(
        CORE_APP_GFX_SEG, GRAPHICS_MSG_SET_MODE, mode
    );
    graphics_requested_mode = mode;
}

void ACTIVE_TASK::check_graphics_mode_ack() {
    int msg, mode;
    if (app_client_shm.get_graphics_msg(APP_CORE_GFX_SEG, msg, mode)) {
        if (msg == GRAPHICS_MSG_SET_MODE) {
            graphics_acked_mode = mode;
            if (mode != MODE_FULLSCREEN) {
                graphics_mode_before_ss = mode;
            }
        }
    }
}

// send quit signal to all tasks in the project
// (or all tasks, if zero).
// If they don't exit in 5, send them a kill signal
// and wait up to 5 more seconds to exit.
// TODO: unsuspend active tasks so they have a chance to checkpoint
//
int ACTIVE_TASK_SET::exit_tasks(PROJECT* proj) {
    request_tasks_exit(proj);

    // Wait 5 seconds for them to exit normally; if they don't then kill them
    //
    if (wait_for_exit(5, proj)) {
        kill_tasks(proj);
    }
    wait_for_exit(5, proj);

    // get final checkpoint_cpu_times
    //
    get_status_msgs();

    return 0;
}

// Wait up to wait_time seconds for processes to exit
// If proj is zero, wait for all processes, else that project's
// NOTE: it's bad form to sleep, but it would be complex to avoid it here
//
int ACTIVE_TASK_SET::wait_for_exit(double wait_time, PROJECT* proj) {
    bool all_exited;
    unsigned int i,n;
    ACTIVE_TASK *atp;

    for (i=0; i<10; i++) {
        all_exited = true;

        for (n=0; n<active_tasks.size(); n++) {
            atp = active_tasks[n];
            if (proj && atp->wup->project != proj) continue;
            if (!atp->task_exited()) {
                all_exited = false;
                break;
            }
        }

        if (all_exited) return 0;
        boinc_sleep(wait_time/10.0);
    }

    return ERR_NOT_EXITED;
}

int ACTIVE_TASK_SET::abort_project(PROJECT* project) {
    vector<ACTIVE_TASK*>::iterator task_iter;
    ACTIVE_TASK* atp;

    exit_tasks(project);
    task_iter = active_tasks.begin();
    while (task_iter != active_tasks.end()) {
        atp = *task_iter;
        if (atp->result->project == project) {
            task_iter = active_tasks.erase(task_iter);
        } else {
            task_iter++;
        }
    }
    return 0;
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

// Find the ACTIVE_TASK in the current set with the matching result
//
ACTIVE_TASK* ACTIVE_TASK_SET::lookup_result(RESULT* result) {
    for (active_tasks_v::iterator i = active_tasks.begin();
         i != active_tasks.end(); ++i)
    {
        ACTIVE_TASK* atp = *i;
        if (atp->result == result) {
            return atp;
        }
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
        if (atp->suspend()) {
            msg_printf(
                atp->wup->project,
                MSG_ERROR,
                "ACTIVE_TASK_SET::suspend_all(): could not suspend active_task"
            );
        }
    }
}

// resume all currently running tasks
//
void ACTIVE_TASK_SET::unsuspend_all() {
    unsigned int i;
    ACTIVE_TASK* atp;
    for (i=0; i<active_tasks.size(); i++) {
        atp = active_tasks[i];
        if (atp->unsuspend()) {
            msg_printf(
                atp->wup->project,
                MSG_ERROR,
                "ACTIVE_TASK_SET::unsuspend_all(): could not unsuspend active_task"
            );
        }
    }
}

// Send quit signal to all currently running tasks
//
void ACTIVE_TASK_SET::request_tasks_exit(PROJECT* proj) {
    unsigned int i;
    ACTIVE_TASK *atp;
    for (i=0; i<active_tasks.size(); i++) {
        atp = active_tasks[i];
        if (proj && atp->wup->project != proj) continue;
        atp->request_exit();
    }
}

// Send kill signal to all currently running tasks
// Don't wait for them to exit
//
void ACTIVE_TASK_SET::kill_tasks(PROJECT* proj) {
    unsigned int i;
    ACTIVE_TASK *atp;
    for (i=0; i<active_tasks.size(); i++) {
        atp = active_tasks[i];
        if (proj && atp->wup->project != proj) continue;
        atp->kill_task();
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
    msg_printf(NULL, MSG_ERROR, "ACTIVE_TASK_SET::remove(): not found\n");
    return 1;
}

// Restart active tasks without wiping and reinitializing slot directories
//
int ACTIVE_TASK_SET::restart_tasks() {
    vector<ACTIVE_TASK*>::iterator iter;
    ACTIVE_TASK* atp;
    int retval;

    ScopeMessages scope_messages(log_messages, ClientMessages::DEBUG_TASK);

    iter = active_tasks.begin();
    while (iter != active_tasks.end()) {
        atp = *iter;
        atp->init(atp->result);
        get_slot_dir(atp->slot, atp->slot_dir);
        atp->result->is_active = true;
        msg_printf(atp->wup->project, MSG_INFO,
            "Restarting computation for result %s using %s version %.2f",
            atp->result->name,
            atp->app_version->app->name,
            atp->app_version->version_num/100.
        );
        retval = atp->start(false);
        if (retval) {
            msg_printf(atp->wup->project, MSG_ERROR, "ACTIVE_TASKS::restart_tasks(); restart failed: %d\n", retval);
            atp->result->active_task_state = PROCESS_COULDNT_START;
            gstate.report_result_error(
                *(atp->result), retval,
                "Couldn't restart the app for this result: %d", retval
            );
            active_tasks.erase(iter);
        } else {
            iter++;
        }
    }
    return 0;
}

// compute frac_rate_of_change
//
void ACTIVE_TASK::estimate_frac_rate_of_change(double now) {
    if (last_frac_update == 0) {
        last_frac_update = now;
        last_frac_done = fraction_done;
        recent_change = 0;
    } else {
        recent_change += (fraction_done - last_frac_done);
        int tdiff = (int)(now-last_frac_update);
        if (tdiff>0) {
            double recent_frac_rate_of_change = max(0.0, recent_change) / tdiff;
            if (frac_rate_of_change == 0) {
                frac_rate_of_change = recent_frac_rate_of_change;
            } else {
                double x = exp(-1*log(2.0)/20.0);
                frac_rate_of_change = frac_rate_of_change*x + recent_frac_rate_of_change*(1-x);
            }
            last_frac_update = now;
            last_frac_done = fraction_done;
            recent_change = 0;
        }
    }
}

// See if the app has placed a new message in shared mem
// (with CPU done, frac done etc.)
// If so parse it and return true.
//
bool ACTIVE_TASK::get_status_msg() {
    char msg_buf[SHM_SEG_SIZE];
    if (app_client_shm.get_msg(msg_buf, APP_CORE_WORKER_SEG)) {
//        last_status_msg_time = (time_t)now;
        fraction_done = current_cpu_time = checkpoint_cpu_time = 0.0;
        parse_double(msg_buf, "<fraction_done>", fraction_done);
        parse_double(msg_buf, "<current_cpu_time>", current_cpu_time);
        parse_double(msg_buf, "<checkpoint_cpu_time>", checkpoint_cpu_time);
        parse_double(msg_buf, "<working_set_size>", working_set_size);
        return true;
    }
    return false;
}

// check for CPU-time msgs from active tasks.
// Return true if any of them has changed its checkpoint_cpu_time
// (since in that case we need to write state file)
//
bool ACTIVE_TASK_SET::get_status_msgs() {
    unsigned int i;
    ACTIVE_TASK *atp;
    double now = dtime(), old_time;
    bool action = false;

    for (i=0; i<active_tasks.size(); i++) {
        atp = active_tasks[i];
        old_time = atp->checkpoint_cpu_time;
        if (atp->get_status_msg()) {
            atp->estimate_frac_rate_of_change(now);
            if (old_time != atp->checkpoint_cpu_time) {
                action = true;
            }
        }
    }
    return action;
}

// Returns the estimated time to completion (in seconds) of this task,
// based on current reported CPU time and fraction done
//
double ACTIVE_TASK::est_time_to_completion() {
    if (fraction_done <= 0 || fraction_done > 1 || frac_rate_of_change <= 0) {
        return -1;
    }
    return (current_cpu_time / fraction_done) - current_cpu_time;
    //return (1.0-fraction_done)/frac_rate_of_change;
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

// Get the next available free slot, or returns -1 if all slots are full
//
int ACTIVE_TASK_SET::get_free_slot(int total_slots) {
    unsigned int i;
    int j;
    bool found;

    if (active_tasks.size() >= (unsigned int)total_slots) {
        return -1;
    }

    for (j=0; j<total_slots; j++) {
        found = false;
        for (i=0; i<active_tasks.size(); i++) {
            if (active_tasks[i]->slot == j) {
                found = true;
                break;
            }
        }
        if (!found) return j;
    }

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
                msg_printf(
                    NULL, MSG_ERROR,
                    "ACTIVE_TASK::parse(): project not found: %s\n",
                    project_master_url
                );
                return ERR_NULL;
            }
            result = cs->lookup_result(project, result_name);
            if (!result) {
                msg_printf(
                    project, MSG_ERROR, "ACTIVE_TASK::parse(): result not found\n"
                );
                return ERR_NULL;
            }

            // various sanity checks
            //
            if (result->got_server_ack
                || result->ready_to_report
                || result->state != RESULT_FILES_DOWNLOADED
            ) {
                msg_printf(project, MSG_ERROR,
                    "ACTIVE_TASK::parse(): result is in wrong state\n"
                );
                return ERR_BAD_RESULT_STATE;
            }

            wup = result->wup;
            app_version = cs->lookup_app_version(
                result->app, app_version_num
            );
            if (!app_version) {
                msg_printf(
                    project, MSG_ERROR,
                    "ACTIVE_TASK::parse(): app_version not found\n"
                );
                return ERR_NULL;
            }
            return 0;
        }
        else if (parse_str(buf, "<result_name>", result_name, sizeof(result_name))) continue;
        else if (parse_str(buf, "<project_master_url>", project_master_url, sizeof(project_master_url))) continue;
        else if (parse_int(buf, "<app_version_num>", app_version_num)) continue;
        else if (parse_int(buf, "<slot>", slot)) continue;
        else if (parse_double(buf, "<checkpoint_cpu_time>", checkpoint_cpu_time)) continue;
        else msg_printf(NULL, MSG_ERROR, "ACTIVE_TASK::parse(): unrecognized %s\n", buf);
    }
    return ERR_XML_PARSE;
}

// Write XML information about this active task set
//
int ACTIVE_TASK_SET::write(FILE* fout) {
    unsigned int i;
    int retval;

    fprintf(fout, "<active_task_set>\n");
    for (i=0; i<active_tasks.size(); i++) {
        retval = active_tasks[i]->write(fout);
        if (retval) return retval;
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
            msg_printf(NULL, MSG_ERROR, "ACTIVE_TASK_SET::parse(): unrecognized %s\n", buf);
        }
    }
    return 0;
}

// return an app with pre-ss mode WINDOW, if there is one
// else return an app with pre-ss mode HIDE, if there is one
// else return NULL
//
ACTIVE_TASK* ACTIVE_TASK_SET::get_graphics_capable_app() {
    unsigned int i;
    ACTIVE_TASK* atp;

    for (i=0; i<active_tasks.size(); i++) {
        atp = active_tasks[i];
        if (atp->graphics_mode_before_ss == MODE_WINDOW) {
            return atp;
        }
    }
    for (i=0; i<active_tasks.size(); i++) {
        atp = active_tasks[i];
        if (atp->graphics_mode_before_ss == MODE_HIDE_GRAPHICS) {
            return atp;
        }
    }
    return NULL;
}

// return an app (if any) with given requested mode
//
ACTIVE_TASK* ACTIVE_TASK_SET::get_app_requested(int req_mode) {
    unsigned int i;
    ACTIVE_TASK* atp;

    for (i=0; i<active_tasks.size(); i++) {
        atp = active_tasks[i];
        if (atp->graphics_requested_mode == req_mode) {
            return atp;
        }
    }
    return NULL;
}

void ACTIVE_TASK_SET::save_app_modes() {
    unsigned int i;
    ACTIVE_TASK* atp;

    for (i=0; i<active_tasks.size(); i++) {
        atp = active_tasks[i];
        atp->graphics_mode_before_ss = atp->graphics_acked_mode;
    }
}

void ACTIVE_TASK_SET::hide_apps() {
    unsigned int i;
    ACTIVE_TASK* atp;

    for (i=0; i<active_tasks.size(); i++) {
        atp = active_tasks[i];
        atp->request_graphics_mode(MODE_HIDE_GRAPHICS);
    }
}

void ACTIVE_TASK_SET::restore_apps() {
    unsigned int i;
    ACTIVE_TASK* atp;

    for (i=0; i<active_tasks.size(); i++) {
        atp = active_tasks[i];
        if (atp->graphics_requested_mode != atp->graphics_mode_before_ss) {
            atp->request_graphics_mode(atp->graphics_mode_before_ss);
        }
    }
}

void ACTIVE_TASK_SET::check_graphics_mode_ack() {
    unsigned int i;
    ACTIVE_TASK* atp;

    for (i=0; i<active_tasks.size(); i++) {
        atp = active_tasks[i];
        atp->check_graphics_mode_ack();
    }
}

bool ACTIVE_TASK::supports_graphics() {
    return (graphics_acked_mode != MODE_UNSUPPORTED);
}
