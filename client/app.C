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
#include "boinc_win.h"
#endif

#ifndef _WIN32

#include <cerrno>
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
#include <csignal>

#include <cctype>
#include <ctime>
#include <cstdio>
#include <cmath>
#include <cstdlib>

#endif

#include "client_state.h"
#include "client_types.h"
#include "error_numbers.h"
#include "filesys.h"
#include "file_names.h"
#include "parse.h"
#include "shmem.h"
#include "util.h"
#include "app_ipc.h"

#include "client_msgs.h"
#include "app.h"

using std::vector;
using std::max;
using std::min;

// ways an active task can be started by the client
//
#define TASK_RESUME     0   // process suspended; call unsuspend()
#define TASK_RESTART    1   // process uninitalized; call start(false)
#define TASK_START      2   // process uninitalized; call start(true)

// value for setpriority(2)
static const int PROCESS_IDLE_PRIORITY = 19;

// Goes through an array of strings, and prints each string
//
static int debug_print_argv(char** argv) {
    int i;

    log_messages.printf(CLIENT_MSG_LOG::DEBUG_TASK, "Arguments:");
    ++log_messages;
    for (i=0; argv[i]; i++) {
        log_messages.printf(
            CLIENT_MSG_LOG::DEBUG_TASK,
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
    scheduler_state = CPU_SCHED_UNINITIALIZED;
    exit_status = 0;
    signal = 0;
    strcpy(slot_dir, "");
    graphics_mode_requested = MODE_HIDE_GRAPHICS;
    graphics_mode_sent = 0;
    graphics_mode_acked = MODE_UNSUPPORTED;
    graphics_mode_before_ss = MODE_HIDE_GRAPHICS;

    fraction_done = 0;
    frac_rate_of_change = 0;
    last_frac_done = 0;
    recent_change = 0;
    last_frac_update = 0;
    episode_start_cpu_time = 0;
    cpu_time_at_last_sched = 0;
    checkpoint_cpu_time = 0;
    current_cpu_time = 0;
    vm_size = 0;
    resident_set_size = 0;
    have_trickle_down = false;
    pending_suspend_via_quit = false;
#ifdef _WIN32
    pid_handle = 0;
    thread_handle = 0;
    quitRequestEvent = 0;
    shm_handle = 0;
#endif
}

ACTIVE_TASK::~ACTIVE_TASK() {
#ifdef _WIN32
    if (pid_handle) CloseHandle(pid_handle);
    if (thread_handle) CloseHandle(thread_handle);
    if (quitRequestEvent) CloseHandle(quitRequestEvent);
    // detach from shared mem.
    // This will destroy shmem seg since we're the last attachment
    //
    if (app_client_shm.shm) {
        detach_shmem(shm_handle, app_client_shm.shm);
        app_client_shm.shm = NULL;
    }
#else
    // detach from and destroy share mem
    //
    if (app_client_shm.shm) {
        detach_shmem(app_client_shm.shm);
        app_client_shm.shm = NULL;
    }
    destroy_shmem(shm_key);
#endif
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

// write the app init file.
// This is done before starting the app,
// and when project prefs have changed during app execution
//
int ACTIVE_TASK::write_app_init_file(APP_INIT_DATA& aid) {
    FILE *f;
    char init_data_path[256], project_dir[256], project_path[256];
    int retval;

    memset(&aid, 0, sizeof(aid));

    aid.core_version = gstate.core_client_major_version*100 + gstate.core_client_minor_version;
    safe_strcpy(aid.app_name, wup->app->name);
    safe_strcpy(aid.user_name, wup->project->user_name);
    safe_strcpy(aid.team_name, wup->project->team_name);
    if (wup->project->project_specific_prefs.length()) {
        strcpy(aid.project_preferences, wup->project->project_specific_prefs.c_str());
    }
    get_project_dir(wup->project, project_dir);
    relative_to_absolute(project_dir, project_path);
    strcpy(aid.project_dir, project_path);
    relative_to_absolute("", aid.boinc_dir);
    strcpy(aid.authenticator, wup->project->authenticator);
    aid.slot = slot;
    strcpy(aid.wu_name, wup->name);
    aid.user_total_credit = wup->project->user_total_credit;
    aid.user_expavg_credit = wup->project->user_expavg_credit;
    aid.host_total_credit = wup->project->host_total_credit;
    aid.host_expavg_credit = wup->project->host_expavg_credit;
    aid.checkpoint_period = gstate.global_prefs.disk_interval;
    aid.fraction_done_update_period = DEFAULT_FRACTION_DONE_UPDATE_PERIOD;
    aid.fraction_done_start = 0;
    aid.fraction_done_end = 1;
#ifndef _WIN32
    aid.shm_key = 0;
#endif
    // wu_cpu_time is the CPU time at start of session,
    // not the checkpoint CPU time
    // At the start of an episode these are equal, but not in the middle!
    //
    aid.wu_cpu_time = episode_start_cpu_time;

    sprintf(init_data_path, "%s%s%s", slot_dir, PATH_SEPARATOR, INIT_DATA_FILE);
    f = boinc_fopen(init_data_path, "w");
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
    int     i = 0;
    char    szSharedMemoryName[256];
    HANDLE  hSharedMemoryHandle;

    do {
        memset(szSharedMemoryName, '\0', sizeof(szSharedMemoryName));
        sprintf(szSharedMemoryName, "boinc_%d", slot);
        i++;
    } while((!(hSharedMemoryHandle = create_shmem(szSharedMemoryName, 1024, NULL))) || (1024 < i));

    if (hSharedMemoryHandle)
        CloseHandle(hSharedMemoryHandle);

    if (1024 < i)
        return ERR_SEMOP;

    strcpy(aid.comm_obj_name, szSharedMemoryName);
#elif HAVE_SYS_IPC_H
    aid.shm_key = ftok(init_data_path, slot);
#else
#error shared memory key generation unimplemented
#endif

    aid.host_info = gstate.host_info;
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

    SCOPE_MSG_LOG scope_messages(log_messages, CLIENT_MSG_LOG::DEBUG_TASK);
    scope_messages.printf("ACTIVE_TASK::start(first_time=%d)\n", first_time);

    if (first_time) {
        checkpoint_cpu_time = 0;
    }
    current_cpu_time = checkpoint_cpu_time;
    episode_start_cpu_time = checkpoint_cpu_time;
    cpu_time_at_last_sched = checkpoint_cpu_time;
    fraction_done = 0;

    gi.xsize = 800;
    gi.ysize = 600;
    gi.refresh_period = 0.1;

    retval = write_app_init_file(aid);
    if (retval) return retval;

    sprintf(graphics_data_path, "%s%s%s", slot_dir, PATH_SEPARATOR, GRAPHICS_DATA_FILE);
    f = boinc_fopen(graphics_data_path, "w");
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
    f = boinc_fopen(fd_init_path, "w");
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

    memset(&process_info, 0, sizeof(process_info));
    memset(&startup_info, 0, sizeof(startup_info));
    startup_info.cb = sizeof(startup_info);
    startup_info.lpReserved = NULL;
    startup_info.lpDesktop = "";

    sprintf(buf, "%s%s", QUIT_PREFIX, aid.comm_obj_name);
    quitRequestEvent = CreateEvent(0, FALSE, FALSE, buf);

    // create core/app share mem segment
    //
    sprintf(buf, "%s%s", SHM_PREFIX, aid.comm_obj_name);
    shm_handle = create_shmem(buf, sizeof(SHARED_MEM),
        (void **)&app_client_shm.shm
    );
    if (shm_handle == NULL) return ERR_SHMGET;
    app_client_shm.reset_msgs();

    // NOTE: in Windows, stderr is redirected within boinc_init();

    sprintf(cmd_line, "%s %s", exec_path, wup->command_line);
    relative_to_absolute(slot_dir, slotdirpath);
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
        char szError[1024];
        windows_error_string(szError, sizeof(szError));

        state = PROCESS_COULDNT_START;
        result->active_task_state = PROCESS_COULDNT_START;
        gstate.report_result_error(*result, ERR_EXEC, "CreateProcess() failed - %s", szError);
        msg_printf(wup->project, MSG_ERROR, "CreateProcess() failed - %s", szError);
        return ERR_EXEC;
    }
    pid = process_info.dwProcessId;
    pid_handle = process_info.hProcess;
    thread_handle = process_info.hThread;
#else
    char* argv[100];

    // Set up core/app shared memory seg
    //
    shm_key = aid.shm_key;
    retval = create_shmem(
        shm_key, sizeof(SHARED_MEM), (void**)&app_client_shm.shm
    );
    if (retval) {
        msg_printf(
            wup->project, MSG_ERROR, "Can't create shared mem: %d", retval
        );
        return retval;
    }
    app_client_shm.reset_msgs();

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
        msg_printf(wup->project, MSG_ERROR,
            "execv(%s) failed: %d\n", buf, retval
        );
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
    scheduler_state = CPU_SCHED_RUNNING;
    return 0;
}

// Send a quit signal.
// Normally this is caught by the process, which can checkpoint
//
int ACTIVE_TASK::request_exit() {
    app_client_shm.shm->process_control_request.send_msg("<quit/>");
    return 0;
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

// We have sent a quit signal to the process; see if it's exited.
// This is called when the core client exits,
// or when a project is detached or reset
//
bool ACTIVE_TASK::task_exited() {
    bool exited = false;
    if (state != PROCESS_RUNNING) return true;
#ifdef _WIN32
    unsigned long exit_code;
    if (GetExitCodeProcess(pid_handle, &exit_code)) {
        if (exit_code != STILL_ACTIVE) {
            exited = true;
        }
    }
#else
    int my_pid, stat;
    struct rusage rs;

    my_pid = wait4(pid, &stat, WNOHANG, &rs);
    if (my_pid == pid) {
        exited = true;
    }
#endif
    if (exited) {
        state = PROCESS_EXITED;
    }
    return exited;
}

// preempts a task
//
int ACTIVE_TASK::preempt(bool quit_task) {
    int retval;

    if (quit_task) {
        retval = request_exit();
        pending_suspend_via_quit = true;
    } else {
        retval = suspend();
    }

    if (retval) {
        msg_printf(
            wup->project,
            MSG_ERROR,
            "ACTIVE_TASK::preempt(): could not %s active_task",
            (quit_task ? "quit" : "suspend")
        );
        return retval;
    }
    scheduler_state = CPU_SCHED_PREEMPTED;

    msg_printf(result->project, MSG_INFO,
        "Preempting computation for result %s (%s)",
        result->name,
        (quit_task ? "quit" : "suspend")
    );
    return 0;
}

// Resume the task if it was previously running
// Otherwise, start it
//
int ACTIVE_TASK::resume_or_start() {
    static const char process_start_types[3][15] = {
        "Resuming",
        "Restarting",
        "Starting"
    };
    int retval;
    int task_start_type;

    if (state == PROCESS_RUNNING) {
#if _WIN32
        unsigned long exit_code;
        GetExitCodeProcess(pid_handle, &exit_code);
        if (exit_code != STILL_ACTIVE) {
            handle_exited_app(exit_code);
        }
#else
        int exited_pid;
        int stat;
        struct rusage rs;

        if ((exited_pid = wait4(0, &stat, WNOHANG, &rs)) == pid) {
            handle_exited_app(stat, rs);
        }
#endif
    }
    if (state == PROCESS_UNINITIALIZED) {
        if (scheduler_state == CPU_SCHED_UNINITIALIZED) {
            if (!boinc_file_exists(slot_dir)) {
                make_slot_dir(slot);
            }
            retval = clean_out_dir(slot_dir);
            retval = start(true);
            task_start_type = TASK_START;
        } else {
            retval = start(false);
            task_start_type = TASK_RESTART;
        }
        if (retval) return retval;
    } else {
        retval = unsuspend();
        if (retval) {
            msg_printf(
                wup->project,
                MSG_ERROR,
                "ACTIVE_TASK::resume_or_start(): could not unsuspend active_task"
            );
            return retval;
        }
        scheduler_state = CPU_SCHED_RUNNING;
        task_start_type = TASK_RESUME;
    }
    msg_printf(result->project, MSG_INFO,
        "%s computation for result %s using %s version %.2f",
        process_start_types[task_start_type],
        result->name,
        app_version->app->name,
        app_version->version_num/100.
    );
    return 0;
}

#ifdef _WIN32
bool ACTIVE_TASK::handle_exited_app(unsigned long exit_code) {
    get_msg();
    result->final_cpu_time = checkpoint_cpu_time;
    if (state == PROCESS_ABORT_PENDING) {
        state = PROCESS_ABORTED;
        result->active_task_state = PROCESS_ABORTED;
    } else {
        state = PROCESS_EXITED;
        exit_status = exit_code;

        //if a nonzero error code, then report it
        //
        if (exit_code) {
            char szError[1024];
            gstate.report_result_error(
                *result, 0,
                "%s - exit code %d (0x%x)",
                windows_format_error_string(exit_code, szError, sizeof(szError)),
                exit_code, exit_code
            );
        } else {
            if (pending_suspend_via_quit) {
                pending_suspend_via_quit = false;
                state = PROCESS_UNINITIALIZED;
                if (app_client_shm.shm) {
                    detach_shmem(shm_handle, app_client_shm.shm);
                    app_client_shm.shm = NULL;
                }
            } else if (!finish_file_present()) {
                state = PROCESS_IN_LIMBO;
            }
            return true;
        }
        result->exit_status = exit_status;
        result->active_task_state = PROCESS_EXITED;
    }
    read_stderr_file();
    clean_out_dir(slot_dir);
    return true;
}
#else
bool ACTIVE_TASK::handle_exited_app(int stat, struct rusage rs) {
    SCOPE_MSG_LOG scope_messages(log_messages, CLIENT_MSG_LOG::DEBUG_TASK);

    get_msg();
    result->final_cpu_time = checkpoint_cpu_time;
    if (state == PROCESS_ABORT_PENDING) {
        state = PROCESS_ABORTED;
        result->active_task_state = PROCESS_ABORTED;
    } else {
        if (WIFEXITED(stat)) {
            state = PROCESS_EXITED;
            exit_status = WEXITSTATUS(stat);

            // If exit_status is nonzero,
            // then we don't need to upload the output files
            //
            if (exit_status) {
                gstate.report_result_error(
                    *result, 0,
                    "process exited with code %d (0x%x)",
                    exit_status, exit_status
                );
            } else {
                if (pending_suspend_via_quit) {
                    pending_suspend_via_quit = false;
                    state = PROCESS_UNINITIALIZED;

                    // destroy shm, since restarting app will re-create it
                    //
                    if (app_client_shm.shm) {
                        detach_shmem(app_client_shm.shm);
                        app_client_shm.shm = NULL;
                    }
                    destroy_shmem(shm_key);
                } else if (!finish_file_present()) {
                    // The process looks like it exited normally
                    // but there's no "finish file".
                    // Assume it was externally killed,
                    // and just leave it there
                    // (assume user is about to exit core client)
                    //
                    state = PROCESS_IN_LIMBO;
                }
                return true;
            }
            result->exit_status = exit_status;
            result->active_task_state = PROCESS_EXITED;
            scope_messages.printf(
                "ACTIVE_TASK::handle_exited_app(): process exited: status %d\n",
                exit_status
            );
        } else if (WIFSIGNALED(stat)) {
            int signal = WTERMSIG(stat);

            // if the process was externally killed, allow it to restart.
            //
            switch(signal) {
            case SIGHUP:
            case SIGINT:
            case SIGQUIT:
            case SIGKILL:
            case SIGTERM:
            case SIGSTOP:
                state = PROCESS_IN_LIMBO;
                return true;
            }
            exit_status = stat;
            result->exit_status = exit_status;
            state = PROCESS_WAS_SIGNALED;
            signal = signal;
            result->signal = signal;
            result->active_task_state = PROCESS_WAS_SIGNALED;
            gstate.report_result_error(
                *result, 0, "process got signal %d", signal
            );
            scope_messages.printf("ACTIVE_TASK::handle_exited_app(): process got signal %d\n", signal);
        } else {
            state = PROCESS_EXIT_UNKNOWN;
            result->state = PROCESS_EXIT_UNKNOWN;
        }
    }

    read_stderr_file();
    clean_out_dir(slot_dir);
    return true;
}
#endif

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
    send_heartbeats();
    send_trickle_downs();
    graphics_poll();
    action |= check_rsc_limits_exceeded();
    if (get_msgs()) {
        action = true;
    }
    if (action) {
        gstate.set_client_state_dirty("ACTIVE_TASK_SET::poll");
    }
    return action;
}

bool ACTIVE_TASK::finish_file_present() {
    char path[256];
    sprintf(path, "%s%s%s", slot_dir, PATH_SEPARATOR, BOINC_FINISH_CALLED_FILE);
    return boinc_file_exists(path);
}

void ACTIVE_TASK_SET::send_trickle_downs() {
    unsigned int i;
    ACTIVE_TASK* atp;
    bool sent;
    for (i=0; i<active_tasks.size(); i++) {
        atp = active_tasks[i];
        if (atp->state == PROCESS_IN_LIMBO
            || atp->state == PROCESS_UNINITIALIZED
        ) {
            continue;
        }
        if (atp->have_trickle_down) {
            sent = atp->app_client_shm.shm->trickle_down.send_msg("<have_trickle_down/>\n");
            if (sent) atp->have_trickle_down = false;
        }
    }
}

void ACTIVE_TASK_SET::send_heartbeats() {
    unsigned int i;
    ACTIVE_TASK* atp;

    for (i=0; i<active_tasks.size(); i++) {
        atp = active_tasks[i];
        if (atp->state == PROCESS_IN_LIMBO
            || atp->state == PROCESS_UNINITIALIZED
        ) {
            continue;
        }
        bool foo = atp->app_client_shm.shm->heartbeat.send_msg("<heartbeat/>\n");
        //msg_printf(atp->wup->project, MSG_INFO, "send heartbeat: %d", foo );
    }
}

bool ACTIVE_TASK_SET::check_app_exited() {
    ACTIVE_TASK* atp;
    bool found = false;

    SCOPE_MSG_LOG scope_messages(log_messages, CLIENT_MSG_LOG::DEBUG_TASK);

#ifdef _WIN32
    unsigned long exit_code;
    unsigned int i;

    for (i=0; i<active_tasks.size(); i++) {
        atp = active_tasks[i];
        if (atp->scheduler_state != CPU_SCHED_RUNNING) continue;
        if (atp->state == PROCESS_IN_LIMBO ||
            atp->state == PROCESS_UNINITIALIZED
        ) continue;
        if (GetExitCodeProcess(atp->pid_handle, &exit_code)) {
            if (exit_code != STILL_ACTIVE) {
                scope_messages.printf("ACTIVE_TASK_SET::check_app_exited(): Process exited with code %d\n", exit_code);
                found = true;
                atp->handle_exited_app(exit_code);
            }
        }
    }
#else
    int pid;
    int stat;
    struct rusage rs;

    if ((pid = wait4(0, &stat, WNOHANG, &rs)) > 0) {
        scope_messages.printf("ACTIVE_TASK_SET::check_app_exited(): process %d is done\n", pid);
        atp = lookup_pid(pid);
        if (!atp) {
            msg_printf(NULL, MSG_ERROR, "ACTIVE_TASK_SET::check_app_exited(): pid %d not found\n", pid);
            return false;
        }
        atp->handle_exited_app(stat, rs);
        found = true;
    }
#endif
    return found;
}

// if an app has exceeded its maximum CPU time, abort it
//
bool ACTIVE_TASK::check_max_cpu_exceeded() {
    if (current_cpu_time > max_cpu_time) {
        msg_printf(result->project, MSG_INFO,
            "Aborting result %s: exceeded CPU time limit %f\n",
            result->name, max_cpu_time
        );
        abort_task("Maximum CPU time exceeded");
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
                "Aborting result %s: exceeded disk limit: %f > %f\n",
                result->name, disk_usage, max_disk_usage
            );
            abort_task("Maximum disk usage exceeded");
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
        abort_task("Maximum memory usage exceeded");
        return true;
    }
    return false;
}
#endif

bool ACTIVE_TASK::check_max_mem_exceeded() {
    if (max_mem_usage != 0 && resident_set_size*1024 > max_mem_usage) {
        msg_printf(
            result->project, MSG_INFO,
            "Aborting result %s: exceeded memory limit %f\n",
            result->name,
            max_mem_usage
        );
        abort_task("Maximum memory usage exceeded");
        return true;
    }
    return false;
}

bool ACTIVE_TASK_SET::vm_limit_exceeded(double vm_limit) {
    unsigned int i;
    ACTIVE_TASK *atp;

    double total_vm_usage = 0;

    for (i=0; i<active_tasks.size(); ++i) {
        atp = active_tasks[i];
        if (atp->state != PROCESS_RUNNING) continue;
        total_vm_usage += atp->vm_size;
    }

    return (total_vm_usage > vm_limit);
}

// Check if any of the active tasks have exceeded their
// resource limits on disk, CPU time or memory
//
bool ACTIVE_TASK_SET::check_rsc_limits_exceeded() {
    unsigned int j;
    ACTIVE_TASK *atp;
    static time_t last_disk_check_time = 0;

    for (j=0;j<active_tasks.size();j++) {
        atp = active_tasks[j];
        if (atp->scheduler_state != CPU_SCHED_RUNNING) continue;
        if (atp->state != PROCESS_RUNNING) continue;
        if (atp->check_max_cpu_exceeded()) return true;
        else if (atp->check_max_mem_exceeded()) return true;
        else if (time(0)>last_disk_check_time + gstate.global_prefs.disk_interval) {
            last_disk_check_time = time(0);
            if (atp->check_max_disk_exceeded()) return true;
        }
    }

    return false;
}

// If process is running, send it a kill signal
// This is done when app has exceeded CPU, disk, or mem limits
//
int ACTIVE_TASK::abort_task(char* msg) {
    if (state == PROCESS_RUNNING) {
        state = PROCESS_ABORT_PENDING;
        result->active_task_state = PROCESS_ABORT_PENDING;
        kill_task();
    } else {
        state = PROCESS_ABORTED;
    }
    gstate.report_result_error(*result, ERR_RSC_LIMIT_EXCEEDED, msg);
    return 0;
}

// check for the stderr file, copy to result record
//
bool ACTIVE_TASK::read_stderr_file() {
    char stderr_file[MAX_BLOB_LEN];
    char path[256];
    int n;

    sprintf(path, "%s%s%s", slot_dir, PATH_SEPARATOR, STDERR_FILE);
    if (boinc_file_exists(path)) {
        FILE* f = fopen(path, "r");
        n = fread(stderr_file, 1, sizeof(stderr_file)-1, f);
        fclose(f);
        if (n < 0) return false;
        stderr_file[n] = '\0';
        result->stderr_out += "<stderr_txt>\n";
        result->stderr_out += stderr_file;
        const char* stderr_txt_close = "\n</stderr_txt>\n";

        // truncate stderr output to 64KB;
        // it's unlikely that more than that will be useful
        //
        result->stderr_out = result->stderr_out.substr(0, MAX_BLOB_LEN-1-strlen(stderr_txt_close));
        result->stderr_out += stderr_txt_close;
        return true;
    }
    return false;
}

// tell a running app to reread project preferences.
// This is called when project prefs change,
// or when a user file has finished downloading.
//
int ACTIVE_TASK::request_reread_prefs() {
    int retval;
    APP_INIT_DATA aid;

    link_user_files();

    retval = write_app_init_file(aid);
    if (retval) return retval;
    app_client_shm.shm->graphics_request.send_msg(
        xml_graphics_modes[MODE_REREAD_PREFS]
    );
    return 0;
}

// tell all running apps of a project to reread prefs
//
void ACTIVE_TASK_SET::request_reread_prefs(PROJECT* project) {
    unsigned int i;
    ACTIVE_TASK* atp;

    for (i=0; i<active_tasks.size(); i++) {
        atp = active_tasks[i];
        if (atp->result->project != project) continue;
        if (atp->state == PROCESS_IN_LIMBO
            || atp->state == PROCESS_UNINITIALIZED
        ) {
            continue;
        }
        atp->request_reread_prefs();
    }
}


// send quit signal to all tasks in the project
// (or all tasks, if proj==0).
// If they don't exit in 5 seconds,
// send them a kill signal and wait up to 5 more seconds to exit.
// This is called when the core client exits,
// or when a project is detached or reset
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
    get_msgs();

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
            delete atp;
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
void ACTIVE_TASK_SET::suspend_all(bool leave_apps_in_memory) {
    unsigned int i;
    ACTIVE_TASK* atp;
    for (i=0; i<active_tasks.size(); i++) {
        atp = active_tasks[i];
        if (leave_apps_in_memory) {
            if (atp->scheduler_state != CPU_SCHED_RUNNING && atp->suspend()) {
                msg_printf(
                    atp->wup->project,
                    MSG_ERROR,
                    "ACTIVE_TASK_SET::suspend_all(): could not suspend active_task"
                );
            }
        } else {
            if (atp->state == PROCESS_RUNNING && atp->request_exit()) {
                msg_printf(
                    atp->wup->project,
                    MSG_ERROR,
                    "ACTIVE_TASK_SET::suspend_all(): could not quit active_task"
                );
            } else {
                atp->pending_suspend_via_quit = true;
            }
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
        if (atp->scheduler_state != CPU_SCHED_RUNNING) continue;
        if (atp->state == PROCESS_RUNNING) {
#if _WIN32
            unsigned long exit_code;
            GetExitCodeProcess(atp->pid_handle, &exit_code);
            if (exit_code != STILL_ACTIVE) {
                atp->handle_exited_app(exit_code);
            }
#else
            int exited_pid;
            int stat;
            struct rusage rs;

            if ((exited_pid = wait4(0, &stat, WNOHANG, &rs)) == atp->pid) {
                atp->handle_exited_app(stat, rs);
            }
#endif
        }
        if (atp->state == PROCESS_UNINITIALIZED) {
            //atp->pending_suspend_via_quit = false;
            if (atp->start(false)) {
                msg_printf(
                    atp->wup->project,
                    MSG_ERROR,
                    "ACTIVE_TASK_SET::unsuspend_all(): could not restart active_task"
                );
            }
        } else if (atp->state == PROCESS_RUNNING && atp->unsuspend()) {
            msg_printf(
                atp->wup->project,
                MSG_ERROR,
                "ACTIVE_TASK_SET::unsuspend_all(): could not unsuspend active_task"
            );
        }
    }
}

// Send quit signal to all currently running tasks
// This is called when the core client exits,
// or when a project is detached or reset
//
void ACTIVE_TASK_SET::request_tasks_exit(PROJECT* proj) {
    unsigned int i;
    ACTIVE_TASK *atp;
    for (i=0; i<active_tasks.size(); i++) {
        atp = active_tasks[i];
        if (proj && atp->wup->project != proj) continue;
        if (atp->state != PROCESS_RUNNING) continue;
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
        if (atp->state != PROCESS_RUNNING) continue;
        atp->kill_task();
    }
}

// suspend a task
//
int ACTIVE_TASK::suspend() {
    app_client_shm.shm->process_control_request.send_msg("<suspend/>");
    return 0;
}

// resume a suspended task
//
int ACTIVE_TASK::unsuspend() {
    app_client_shm.shm->process_control_request.send_msg("<resume/>");
    return 0;
}

// Remove an ACTIVE_TASK from the set.
// Does NOT delete the ACTIVE_TASK object.
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
    return ERR_NOT_FOUND;
}

// Restart active tasks without wiping and reinitializing slot directories
// Called at init, with max_tasks = ncpus
//
int ACTIVE_TASK_SET::restart_tasks(int max_tasks) {
    vector<ACTIVE_TASK*>::iterator iter;
    ACTIVE_TASK* atp;
    RESULT* result;
    int retval, num_tasks_started;

    SCOPE_MSG_LOG scope_messages(log_messages, CLIENT_MSG_LOG::DEBUG_TASK);

    num_tasks_started = 0;
    iter = active_tasks.begin();
    while (iter != active_tasks.end()) {
        atp = *iter;
        result = atp->result;
        atp->init(atp->result);
        get_slot_dir(atp->slot, atp->slot_dir);
        if (!gstate.input_files_available(result)) {
            msg_printf(atp->wup->project, MSG_ERROR, "ACTIVE_TASKS::restart_tasks(); missing files\n");
            atp->result->active_task_state = PROCESS_COULDNT_START;
            gstate.report_result_error(
                *(atp->result), ERR_FILE_MISSING,
                "One or more missing files"
            );
            iter = active_tasks.erase(iter);
            delete atp;
            continue;
        }

        if (atp->scheduler_state != CPU_SCHED_RUNNING
            || num_tasks_started >= max_tasks
        ) {
            msg_printf(atp->wup->project, MSG_INFO,
                "Deferring computation for result %s",
                atp->result->name
            );

            atp->scheduler_state = CPU_SCHED_PREEMPTED;
            iter++;
            continue;
        }

        result->is_active = true;

        msg_printf(atp->wup->project, MSG_INFO,
            "Resuming computation for result %s using %s version %.2f",
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
            iter = active_tasks.erase(iter);
            delete atp;
        } else {
            ++num_tasks_started;
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

// There's a new trickle file.
// Move it from slot dir to project dir
//
int ACTIVE_TASK::move_trickle_file() {
    char project_dir[256], new_path[256], old_path[256];
    int retval;

    get_project_dir(result->project, project_dir);
    sprintf(old_path, "%s%strickle_up.xml", slot_dir, PATH_SEPARATOR);
    sprintf(new_path,
        "%s%strickle_up_%s_%d.xml",
        project_dir, PATH_SEPARATOR, result->name, (int)time(0)
    );
    retval = boinc_rename(old_path, new_path);

    // if can't move it, remove
    //
    if (retval) {
        boinc_delete_file(old_path);
        return ERR_RENAME;
    }
    return 0;
}

// See if the app has placed a new message in shared mem
// (with CPU done, frac done etc.)
// If so parse it and return true.
//
bool ACTIVE_TASK::get_msg() {
    char msg_buf[MSG_CHANNEL_SIZE];
    bool found = false;
    int retval;

    if (app_client_shm.shm->app_status.get_msg(msg_buf)) {
        fraction_done = current_cpu_time = checkpoint_cpu_time = 0.0;
        parse_double(msg_buf, "<fraction_done>", fraction_done);
        parse_double(msg_buf, "<current_cpu_time>", current_cpu_time);
        parse_double(msg_buf, "<checkpoint_cpu_time>", checkpoint_cpu_time);
        parse_double(msg_buf, "<vm_size>", vm_size);
        parse_double(msg_buf, "<resident_set_size>", resident_set_size);
        found = true;
    }
    if (app_client_shm.shm->trickle_up.get_msg(msg_buf)) {
        if (match_tag(msg_buf, "<have_new_trickle_up/>")) {
            retval = move_trickle_file();
            if (!retval) {
                wup->project->sched_rpc_pending = true;
            }
        }
        found = true;
    }
    return found;
}

// check for msgs from active tasks.
// Return true if any of them has changed its checkpoint_cpu_time
// (since in that case we need to write state file)
//
bool ACTIVE_TASK_SET::get_msgs() {
    unsigned int i;
    ACTIVE_TASK *atp;
    double now = dtime(), old_time;
    bool action = false;

    for (i=0; i<active_tasks.size(); i++) {
        atp = active_tasks[i];
        if (atp->state == PROCESS_IN_LIMBO
            || atp->state == PROCESS_UNINITIALIZED
        ) {
            continue;
        }
        old_time = atp->checkpoint_cpu_time;
        if (atp->get_msg()) {
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

// Get the next free slot
//
int ACTIVE_TASK_SET::get_free_slot() {
    unsigned int i;
    int j;
    bool found;

    for (j=0; ; j++) {
        found = false;
        for (i=0; i<active_tasks.size(); i++) {
            if (active_tasks[i]->slot == j) {
                found = true;
                break;
            }
        }
        if (!found) return j;
    }
    return -1;   // probably never get here
}

int ACTIVE_TASK::write(MIOFILE& fout) {
    fout.printf(
        "<active_task>\n"
        "    <project_master_url>%s</project_master_url>\n"
        "    <result_name>%s</result_name>\n"
        "    <app_version_num>%d</app_version_num>\n"
        "    <slot>%d</slot>\n"
        "    <scheduler_state>%d</scheduler_state>\n"
        "    <checkpoint_cpu_time>%f</checkpoint_cpu_time>\n"
        "    <fraction_done>%f</fraction_done>\n"
        "    <current_cpu_time>%f</current_cpu_time>\n"
        "</active_task>\n",
        result->project->master_url,
        result->name,
        app_version->version_num,
        slot,
        scheduler_state,
        checkpoint_cpu_time,
        fraction_done,
        current_cpu_time
    );
    return 0;
}

int ACTIVE_TASK::parse(MIOFILE& fin) {
    char buf[256], result_name[256], project_master_url[256];
    int app_version_num=0;
    PROJECT* project;

    SCOPE_MSG_LOG scope_messages(log_messages, CLIENT_MSG_LOG::DEBUG_TASK);

    strcpy(result_name, "");
    strcpy(project_master_url, "");
    scheduler_state = CPU_SCHED_RUNNING;

    while (fin.fgets(buf, 256)) {
        if (match_tag(buf, "</active_task>")) {
            project = gstate.lookup_project(project_master_url);
            if (!project) {
                msg_printf(
                    NULL, MSG_ERROR,
                    "ACTIVE_TASK::parse(): project not found: %s\n",
                    project_master_url
                );
                return ERR_NULL;
            }
            result = gstate.lookup_result(project, result_name);
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
            app_version = gstate.lookup_app_version(
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
        else if (parse_int(buf, "<scheduler_state>", scheduler_state)) continue;
        else if (parse_double(buf, "<checkpoint_cpu_time>", checkpoint_cpu_time)) continue;
        else if (parse_double(buf, "<fraction_done>", fraction_done)) continue;
        else if (parse_double(buf, "<current_cpu_time>", current_cpu_time)) continue;
        else scope_messages.printf("ACTIVE_TASK::parse(): unrecognized %s\n", buf);
    }
    return ERR_XML_PARSE;
}

// Write XML information about this active task set
//
int ACTIVE_TASK_SET::write(MIOFILE& fout) {
    unsigned int i;
    int retval;

    fout.printf("<active_task_set>\n");
    for (i=0; i<active_tasks.size(); i++) {
        retval = active_tasks[i]->write(fout);
        if (retval) return retval;
    }
    fout.printf("</active_task_set>\n");
    return 0;
}

// Parse XML information about an active task set
//
int ACTIVE_TASK_SET::parse(MIOFILE& fin) {
    ACTIVE_TASK* atp;
    char buf[256];
    int retval;

    SCOPE_MSG_LOG scope_messages(log_messages, CLIENT_MSG_LOG::DEBUG_TASK);

    while (fin.fgets(buf, 256)) {
        if (match_tag(buf, "</active_task_set>")) return 0;
        else if (match_tag(buf, "<active_task>")) {
            atp = new ACTIVE_TASK;
            retval = atp->parse(fin);
            if (!retval) active_tasks.push_back(atp);
            else delete atp;
        } else scope_messages.printf("ACTIVE_TASK_SET::parse(): unrecognized %s\n", buf);
    }
    return 0;
}

////// GRAPHICS STUFF STARTS HERE ///////////////

void ACTIVE_TASK::request_graphics_mode(int mode) {
    graphics_mode_requested = mode;
    graphics_mode_sent = 0;
}

bool ACTIVE_TASK::send_graphics_mode(int mode) {
    bool sent = app_client_shm.shm->graphics_request.send_msg(
        xml_graphics_modes[mode]
    );
    //msg_printf(NULL, MSG_INFO, "%d requested mode %d: sent %d", time(0), mode, sent);        
    return sent;
}

void ACTIVE_TASK::check_graphics_mode_ack() {
    int mode;
    char buf[MSG_CHANNEL_SIZE];
    if (app_client_shm.shm->graphics_reply.get_msg(buf)) {
        mode = app_client_shm.decode_graphics_msg(buf);
        //msg_printf(NULL, MSG_INFO, "got graphics ack %d", mode);
        if (mode != MODE_REREAD_PREFS) {
            graphics_mode_acked = mode;
        }
    }
}

#if 0
// return an app with pre-ss mode WINDOW, if there is one
// else return an app with pre-ss mode HIDE, if there is one
// else return NULL
//
ACTIVE_TASK* ACTIVE_TASK_SET::get_graphics_capable_app() {
    unsigned int i;
    ACTIVE_TASK* atp;

    for (i=0; i<active_tasks.size(); i++) {
        atp = active_tasks[i];
        if (atp->scheduler_state != CPU_SCHED_RUNNING) continue;
        if (atp->graphics_mode_before_ss == MODE_WINDOW) {
            return atp;
        }
    }
    for (i=0; i<active_tasks.size(); i++) {
        atp = active_tasks[i];
        if (atp->scheduler_state != CPU_SCHED_RUNNING) continue;
        if (atp->graphics_mode_before_ss == MODE_HIDE_GRAPHICS) {
            return atp;
        }
    }
    return NULL;
}
#endif

// return an app (if any) with given requested mode
//
ACTIVE_TASK* ACTIVE_TASK_SET::get_app_graphics_mode_requested(int req_mode) {
    unsigned int i;
    ACTIVE_TASK* atp;

    for (i=0; i<active_tasks.size(); i++) {
        atp = active_tasks[i];
        if (atp->scheduler_state != CPU_SCHED_RUNNING) continue;
        if (atp->graphics_mode_requested == req_mode) {
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
        if (atp->scheduler_state != CPU_SCHED_RUNNING) continue;
        atp->graphics_mode_before_ss = atp->graphics_mode_acked;
        //msg_printf(NULL, MSG_INFO, "saved mode %d", atp->graphics_mode_acked);
    }
}

void ACTIVE_TASK_SET::hide_apps() {
    unsigned int i;
    ACTIVE_TASK* atp;

    for (i=0; i<active_tasks.size(); i++) {
        atp = active_tasks[i];
        if (atp->scheduler_state != CPU_SCHED_RUNNING) continue;
        atp->request_graphics_mode(MODE_HIDE_GRAPHICS);
    }
}

// return apps to the mode they were in before screensaving started
//
void ACTIVE_TASK_SET::restore_apps() {
    unsigned int i;
    ACTIVE_TASK* atp;

    for (i=0; i<active_tasks.size(); i++) {
        atp = active_tasks[i];
        if (atp->scheduler_state != CPU_SCHED_RUNNING) continue;
        if (atp->graphics_mode_requested != atp->graphics_mode_before_ss) {
            atp->request_graphics_mode(atp->graphics_mode_before_ss);
        }
    }
}

void ACTIVE_TASK_SET::graphics_poll() {
    unsigned int i;
    ACTIVE_TASK* atp;
    bool sent;

    for (i=0; i<active_tasks.size(); i++) {
        atp = active_tasks[i];
        if (atp->scheduler_state != CPU_SCHED_RUNNING ||
            atp->state != PROCESS_RUNNING
        ) continue;
        if (atp->graphics_mode_requested != atp->graphics_mode_sent) {
            sent = atp->send_graphics_mode(atp->graphics_mode_requested);
            if (sent) {
                atp->graphics_mode_sent = atp->graphics_mode_requested;
            }
            //msg_printf(NULL, MSG_INFO, "sending graphics req %d: %d", sent);
        }
        atp->check_graphics_mode_ack();
    }
}

bool ACTIVE_TASK::supports_graphics() {
    return (graphics_mode_acked != MODE_UNSUPPORTED);
}

