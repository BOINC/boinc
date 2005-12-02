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

// initialization and starting of applications

#include "cpp.h"

#ifdef _WIN32
#include "boinc_win.h"
#else
#include "config.h"
#if HAVE_SYS_TIME_H
#include <sys/time.h>
#endif
#if HAVE_SYS_RESOURCE_H
#include <sys/resource.h>
#endif
#if HAVE_SYS_IPC_H
#include <sys/ipc.h>
#endif
#if HAVE_SYS_WAIT_H
#include <sys/wait.h>
#endif
#include <unistd.h>
#include <cerrno>
#endif

#ifdef __EMX__
#include <process.h>
#endif

using std::vector;

#include "filesys.h"
#include "error_numbers.h"
#include "util.h"
#include "shmem.h"
#include "client_msgs.h"
#include "client_state.h"
#include "file_names.h"

#include "app.h"

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

// make a unique key for core/app shared memory segment
//
int ACTIVE_TASK::get_shmem_seg_name() {
#ifdef _WIN32
    int     i = 0;
    char    szSharedMemoryName[256];
    HANDLE  hSharedMemoryHandle = 0;

    for (i=0; i<1024; i++) {
        sprintf(szSharedMemoryName, "%sboinc_%d", SHM_PREFIX, i);
        hSharedMemoryHandle = create_shmem(szSharedMemoryName, 1024, NULL, true);
        if (hSharedMemoryHandle) break;
    }

    if (!hSharedMemoryHandle) {
        return ERR_SHMGET;
    }
    detach_shmem(hSharedMemoryHandle, NULL);

    sprintf(szSharedMemoryName, "boinc_%d", i);
    strcpy(shmem_seg_name, szSharedMemoryName);

#else
    char init_data_path[256];
    sprintf(init_data_path, "%s%s%s", slot_dir, PATH_SEPARATOR, INIT_DATA_FILE);

	// ftok() only works if there's a file at the given location
	//
    FILE* f = boinc_fopen(init_data_path, "w");
    if (f) fclose(f);
    shmem_seg_name = ftok(init_data_path, slot);
    if (shmem_seg_name == -1) return ERR_SHMEM_NAME;
#endif
    return 0;
}

// write the app init file.
// This is done before starting the app,
// and when project prefs have changed during app execution
//
int ACTIVE_TASK::write_app_init_file() {
    APP_INIT_DATA aid;
    FILE *f;
    char init_data_path[256], project_dir[256], project_path[256];
    int retval;

    memset(&aid, 0, sizeof(aid));

    aid.major_version = BOINC_MAJOR_VERSION;
    aid.minor_version = BOINC_MINOR_VERSION;
    aid.release = BOINC_RELEASE;
    aid.app_version = app_version->version_num;
    safe_strcpy(aid.app_name, wup->app->name);
    safe_strcpy(aid.user_name, wup->project->user_name);
    safe_strcpy(aid.team_name, wup->project->team_name);
    if (wup->project->project_specific_prefs.length()) {
        aid.project_preferences = strdup(wup->project->project_specific_prefs.c_str());
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
#ifdef _WIN32
    strcpy(aid.shmem_seg_name, shmem_seg_name);
#else
    aid.shmem_seg_name = shmem_seg_name;
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

    aid.host_info = gstate.host_info;
    aid.global_prefs = gstate.global_prefs;
    aid.proxy_info = gstate.proxy_info;
    retval = write_init_data_file(f, aid);
    fclose(f);
    return retval;
}

// set up a 'symbolic link' in the slot dir to the given file
// (or copy the file to slot dir)
//
static int setup_file(
    WORKUNIT* wup, FILE_INFO* fip, FILE_REF& fref,
    char* file_path, char* slot_dir
) {
    char link_path[256], buf[256];
    int retval;

    sprintf(link_path,
        "%s%s%s", slot_dir, PATH_SEPARATOR,
        strlen(fref.open_name)?fref.open_name:fip->name
    );
    sprintf(buf, "..%s..%s%s", PATH_SEPARATOR, PATH_SEPARATOR, file_path );
    if (fref.copy_file) {
        retval = boinc_copy(file_path, link_path);
        if (retval) {
            msg_printf(wup->project, MSG_ERROR, "Can't copy %s to %s", file_path, link_path);
            return retval;
        }
    } else {
        // if anonymous platform, link may already be there
        //
        if (wup->project->anonymous_platform && boinc_file_exists(link_path)) {
            return 0;
        }
        retval = boinc_link(buf, link_path);
        if (retval) {
            msg_printf(wup->project, MSG_ERROR, "Can't link %s to %s", file_path, link_path);
            return retval;
        }
    }
    return 0;
}

// Start a task in a slot directory.
// This includes setting up soft links,
// passing preferences, and starting the process
//
// Current dir is top-level BOINC dir
//
// postcondition: ACTIVE_TASK::task_state is set correctly
//
int ACTIVE_TASK::start(bool first_time) {
    char exec_name[256], file_path[256], buf[256], exec_path[256];
    unsigned int i;
    FILE_REF fref;
    FILE_INFO* fip;
    int retval;

    SCOPE_MSG_LOG scope_messages(log_messages, CLIENT_MSG_LOG::DEBUG_TASK);
    scope_messages.printf("ACTIVE_TASK::start(first_time=%d)\n", first_time);

    if (result->aborted_via_gui) {
        task_state = PROCESS_ABORTED;
        result->state = RESULT_COMPUTE_ERROR;
        result->exit_status = ERR_ABORTED_VIA_GUI;
        gstate.report_result_error(*result, "Aborted by user");
        return ERR_ABORTED_VIA_GUI;
    }

    if (first_time) {
        checkpoint_cpu_time = 0;
    }
    current_cpu_time = checkpoint_cpu_time;
    episode_start_cpu_time = checkpoint_cpu_time;
    cpu_time_at_last_sched = checkpoint_cpu_time;
    fraction_done = 0;

    if (!app_client_shm.shm) {
        retval = get_shmem_seg_name();
        if (retval) {
            msg_printf(wup->project, MSG_ERROR,
                "Can't get shared memory segment name: %s", boincerror(retval)
            );
            return retval;
        }
    }

    // this must go AFTER creating shmem,
    // since the shmem name is part of the file
    //
    retval = write_app_init_file();
    if (retval) return retval;

    // set up applications files
    //
    strcpy(exec_name, "");
    for (i=0; i<app_version->app_files.size(); i++) {
        fref = app_version->app_files[i];
        fip = fref.file_info;
        get_pathname(fip, file_path);
        if (fref.main_program) {
            if (is_image_file(fip->name)) {
                msg_printf(wup->project, MSG_ERROR,
                    "Main program %s is an image file", fip->name
                );
                return ERR_NO_SIGNATURE;
            }
            if (!fip->executable && !wup->project->anonymous_platform) {
                msg_printf(wup->project, MSG_ERROR,
                    "Main program %s is not executable", fip->name
                );
                return ERR_NO_SIGNATURE;
            }
            safe_strcpy(exec_name, fip->name);
            safe_strcpy(exec_path, file_path);
        }
        // anonymous platform may use different files than
        // when the result was started
        //
        if (first_time || wup->project->anonymous_platform) {
            retval = setup_file(wup, fip, fref, file_path, slot_dir);
            if (retval) return retval;
        }
    }
    if (!strlen(exec_name)) {
        msg_printf(wup->project, MSG_ERROR,
            "No main program specified"
        );
        return ERR_NOT_FOUND;
    }

    // set up input files
    //
    for (i=0; i<wup->input_files.size(); i++) {
        fref = wup->input_files[i];
        fip = fref.file_info;
        get_pathname(fref.file_info, file_path);
        if (first_time) {
            retval = setup_file(wup, fip, fref, file_path, slot_dir);
            if (retval) return retval;
        }
    }

    // set up output files
    //
    for (i=0; i<result->output_files.size(); i++) {
        fref = result->output_files[i];
        fip = fref.file_info;
        get_pathname(fref.file_info, file_path);
        if (first_time) {
            retval = setup_file(wup, fip, fref, file_path, slot_dir);
            if (retval) return retval;
        }
    }

    link_user_files();

#ifdef _WIN32
    PROCESS_INFORMATION process_info;
    STARTUPINFO startup_info;
    char slotdirpath[256];
    std::string cmd_line;
    char error_msg[1024];

    memset(&process_info, 0, sizeof(process_info));
    memset(&startup_info, 0, sizeof(startup_info));
    //startup_info.cb = sizeof(startup_info);
    //startup_info.dwFlags = STARTF_USESHOWWINDOW;
    //startup_info.wShowWindow = SW_HIDE;

    if (!quitRequestEvent) {
        sprintf(buf, "%s%s", QUIT_PREFIX, shmem_seg_name);
        quitRequestEvent = CreateEvent(0, FALSE, FALSE, buf);
        if (quitRequestEvent == NULL) return ERR_INVALID_EVENT;
    }

    // create core/app share mem segment if needed
    //
    if (!app_client_shm.shm) {
        sprintf(buf, "%s%s", SHM_PREFIX, shmem_seg_name);
        shm_handle = create_shmem(buf, sizeof(SHARED_MEM),
            (void **)&app_client_shm.shm, false
        );
        if (shm_handle == NULL) return ERR_SHMGET;
    }
    app_client_shm.reset_msgs();

    // NOTE: in Windows, stderr is redirected in boinc_init_diagnostics();

    cmd_line = exec_path + std::string(" ") + wup->command_line;
    relative_to_absolute(slot_dir, slotdirpath);
    bool success = false;
    for (i=0; i<5; i++) {
        if (CreateProcess(exec_path,
            (LPSTR)cmd_line.c_str(),
            NULL,
            NULL,
            FALSE,
            CREATE_NEW_PROCESS_GROUP|CREATE_NO_WINDOW|IDLE_PRIORITY_CLASS,
            NULL,
            slotdirpath,
            &startup_info,
            &process_info
        )) {
            success = true;
            break;
        }
        windows_error_string(error_msg, sizeof(error_msg));
        msg_printf(wup->project, MSG_ERROR, "CreateProcess() failed - %s", error_msg);
        boinc_sleep(drand());
    }
    if (!success) {
        task_state = PROCESS_COULDNT_START;
        gstate.report_result_error(*result, "CreateProcess() failed - %s", error_msg);
        return ERR_EXEC;
    }
    pid = process_info.dwProcessId;
    pid_handle = process_info.hProcess;
    thread_handle = process_info.hThread;
	
#elif defined(__EMX__)

    char* 	argv[100];
	char	current_dir[_MAX_PATH];

    // Set up core/app shared memory seg if needed
    //
    if (!app_client_shm.shm) {
        retval = create_shmem(
            shmem_seg_name, sizeof(SHARED_MEM), (void**)&app_client_shm.shm
        );
        if (retval) {
            msg_printf(
                wup->project, MSG_ERROR,
                "Can't create shared memory: %s", boincerror(retval)
            );
            return retval;
        }
    }
    app_client_shm.reset_msgs();

	// save current dir
	getcwd( current_dir, sizeof(current_dir));

	// chdir() into the slot directory
	//
	retval = chdir(slot_dir);
	if (retval) {
		msg_printf(wup->project, MSG_ERROR,
			"chdir(%s) failed: %s\n", slot_dir, boincerror(retval)
        );
		return retval;
	}

	// hook up stderr to a specially-named file
	//
	//freopen(STDERR_FILE, "a", stderr);

	argv[0] = exec_name;
	char cmdline[8192];
	strcpy(cmdline, wup->command_line.c_str());
	parse_command_line(cmdline, argv+1);
	debug_print_argv(argv);
	sprintf(buf, "..%s..%s%s", PATH_SEPARATOR, PATH_SEPARATOR, exec_path );
	pid = spawnv(P_NOWAIT, buf, argv);
	if (pid == -1) {
		msg_printf(wup->project, MSG_ERROR,
			"spawn(%s) failed: %s\n", buf, boincerror(retval)
        );
		chdir(current_dir);
        return ERR_EXEC;
	}

	// restore current dir
	chdir(current_dir);
    scope_messages.printf("ACTIVE_TASK::start(): forked process: pid %d\n", pid);

    // set idle process priority
    if (setpriority(PRIO_PROCESS, pid, PROCESS_IDLE_PRIORITY)) {
        perror("setpriority");
    }

#else
    char* argv[100];

    // Set up core/app shared memory seg if needed
    //
    if (!app_client_shm.shm) {
        retval = create_shmem(
            shmem_seg_name, sizeof(SHARED_MEM), (void**)&app_client_shm.shm
        );
        if (retval) {
            msg_printf(
                wup->project, MSG_ERROR,
                "Can't create shared memory: %s", boincerror(retval)
            );
            return retval;
        }
    }
    app_client_shm.reset_msgs();

    pid = fork();
    if (pid == -1) {
        task_state = PROCESS_COULDNT_START;
        gstate.report_result_error(*result, "fork() failed: %s", strerror(errno));
        msg_printf(wup->project, MSG_ERROR, "fork() failed: %s", strerror(errno));
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
            fflush(NULL);
            _exit(retval);
        }

        // hook up stderr to a specially-named file
        //
        freopen(STDERR_FILE, "a", stderr);

        argv[0] = exec_name;
        char cmdline[8192];
        strcpy(cmdline, wup->command_line.c_str());
        parse_command_line(cmdline, argv+1);
        debug_print_argv(argv);
        sprintf(buf, "..%s..%s%s", PATH_SEPARATOR, PATH_SEPARATOR, exec_path );
        retval = execv(buf, argv);
        msg_printf(wup->project, MSG_ERROR,
            "execv(%s) failed: %s\n", buf, boincerror(retval)
        );
        perror("execv");
        fflush(NULL);
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
    task_state = PROCESS_EXECUTING;
    return 0;
}

// Resume the task if it was previously running; otherwise start it
// Postcondition: "state" is set correctly
//
int ACTIVE_TASK::resume_or_start() {
    const char* str = "??";
    int retval;

    switch (task_state) {
    case PROCESS_UNINITIALIZED:
        if (scheduler_state == CPU_SCHED_UNINITIALIZED) {
            if (!boinc_file_exists(slot_dir)) {
                make_slot_dir(slot);
            }
            retval = clean_out_dir(slot_dir);
            if (retval) {
                retval = rename_slot_dir(slot);
                if (!retval) retval = make_slot_dir(slot);
            }
            retval = start(true);
            str = "Starting";
        } else {
            retval = start(false);
            str = "Restarting";
        }
        if (retval) {
            task_state = PROCESS_COULDNT_START;
            return retval;
        }
        break;
    case PROCESS_SUSPENDED:
        retval = unsuspend();
        if (retval) {
            msg_printf(
                wup->project,
                MSG_ERROR,
                "ACTIVE_TASK::resume_or_start(): could not unsuspend active_task"
            );
            task_state = PROCESS_COULDNT_START;
            return retval;
        }
        str = "Resuming";
        break;
    case PROCESS_EXECUTING:
        return 0;
        break;
    default:
        msg_printf(result->project, MSG_ERROR,
            "resume_or_start(): unexpected process state %d", task_state
        );
        return 0;
    }
    msg_printf(result->project, MSG_INFO,
        "%s result %s using %s version %d",
        str,
        result->name,
        app_version->app->name,
        app_version->version_num
    );
    return 0;
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
            gstate.report_result_error(
                *(atp->result),
                "One or more missing files"
            );
            iter = active_tasks.erase(iter);
            delete atp;
            continue;
        }

        if (atp->scheduler_state != CPU_SCHED_SCHEDULED
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

        msg_printf(atp->wup->project, MSG_INFO,
            "Resuming computation for result %s using %s version %d",
            atp->result->name,
            atp->app_version->app->name,
            atp->app_version->version_num
        );
        retval = atp->start(false);

        if (retval) {
            msg_printf(atp->wup->project, MSG_ERROR,
                "Task restart failed: %s\n", boincerror(retval)
            );
            gstate.report_result_error(
                *(atp->result),
                "Couldn't restart app: %d", retval
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


const char *BOINC_RCSID_be8bae8cbb = "$Id$";
