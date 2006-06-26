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

#if (defined(__APPLE__) && defined(__i386__))
#include <mach-o/loader.h>
#include <mach-o/fat.h>
#include <mach/machine.h>
#include <libkern/OSByteOrder.h>
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
static void debug_print_argv(char** argv) {
    int i;

    msg_printf(0, MSG_INFO, "Arguments:");
    for (i=0; argv[i]; i++) {
        msg_printf(0, MSG_INFO,
            "   argv[%d]: %s\n", i, argv[i]
        );
    }
}

// create a file (new_link) which contains an XML
// reference to existing file.
//
static int make_link(const char *existing, const char *new_link) {
    FILE *fp;

    fp = fopen(new_link, "w");
    if (!fp) return ERR_FOPEN;
    fprintf(fp, "<soft_link>%s</soft_link>\n", existing);
    fclose(fp);
#ifdef SANDBOX
    return boinc_chown(new_link, gstate.boinc_project_gid);
#else
    return 0;
#endif
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
        sprintf(link_path, "%s/%s", slot_dir, strlen(fref.open_name)?fref.open_name:fip->name);
        sprintf(buf, "../../%s", file_path);
        retval = make_link(buf, link_path);
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
    sprintf(init_data_path, "%s/%s", slot_dir, INIT_DATA_FILE);

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
    safe_strcpy(aid.symstore, wup->project->symstore);
    safe_strcpy(aid.acct_mgr_url, gstate.acct_mgr_info.acct_mgr_url);
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
    double rrs = gstate.runnable_resource_share();
    if (rrs) {
        aid.resource_share_fraction = wup->project->resource_share/rrs;
    } else {
        aid.resource_share_fraction = 1;
    }
    aid.rsc_fpops_est = wup->rsc_fpops_est;
    aid.rsc_fpops_bound = wup->rsc_fpops_bound;
    aid.rsc_memory_bound = wup->rsc_memory_bound;
    aid.rsc_disk_bound = wup->rsc_disk_bound;
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

    sprintf(init_data_path, "%s/%s", slot_dir, INIT_DATA_FILE);
    f = boinc_fopen(init_data_path, "w");
    if (!f) {
        msg_printf(wup->project, MSG_ERROR,
            "Failed to open init file %s",
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
        "%s/%s",
        slot_dir, strlen(fref.open_name)?fref.open_name:fip->name
    );
    sprintf(buf, "../../%s", file_path );
    if (fref.copy_file) {
        retval = boinc_copy(file_path, link_path);
        if (retval) {
            msg_printf(wup->project, MSG_ERROR,
                "Can't copy %s to %s", file_path, link_path
            );
            return retval;
        }
    } else {
        // if anonymous platform, link may already be there
        //
        if (wup->project->anonymous_platform && boinc_file_exists(link_path)) {
            return 0;
        }
        retval = make_link(buf, link_path);
        if (retval) {
            msg_printf(wup->project, MSG_ERROR,
                "Can't link %s to %s", file_path, link_path
            );
            return retval;
        }
    }
    return 0;
}

int ACTIVE_TASK::copy_output_files() {
    char slotfile[256], projfile[256];
    unsigned int i;
    for (i=0; i<result->output_files.size(); i++) {
        FILE_REF& fref = result->output_files[i];
        if (!fref.copy_file) continue;
        FILE_INFO* fip = fref.file_info;
        sprintf(slotfile, "%s/%s", slot_dir, fref.open_name);
        get_pathname(fip, projfile);
        int retval = boinc_rename(slotfile, projfile);
        if (retval) {
            msg_printf(wup->project, MSG_ERROR,
                "Can't rename output file %s", fip->name
            );
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
// postcondition:
// If any error occurs
//   ACTIVE_TASK::task_state is PROCESS_COULDNT_START
//   report_result_error() is called
//  else
//   ACTIVE_TASK::task_state is PROCESS_EXECUTING
//
int ACTIVE_TASK::start(bool first_time) {
    char exec_name[256], file_path[256], buf[256], exec_path[256];
    unsigned int i;
    FILE_REF fref;
    FILE_INFO* fip;
    int retval;
#ifdef _WIN32
    std::string cmd_line;
#endif

    if (log_flags.task_debug) {
        msg_printf(0, MSG_INFO,
            "ACTIVE_TASK::start(first_time=%d)\n", first_time
        );
    }

    if (wup->project->verify_files_on_app_start) {
        retval = gstate.input_files_available(result, true);
        if (retval) {
            strcpy(buf, "Input file missing or invalid");
            goto error;
        }
    }

    if (first_time) {
        checkpoint_cpu_time = 0;
        checkpoint_wall_time = gstate.now;
    }
    episode_start_wall_time = gstate.now;
    current_cpu_time = checkpoint_cpu_time;
    episode_start_cpu_time = checkpoint_cpu_time;
    cpu_time_at_last_sched = checkpoint_cpu_time;

    if (!app_client_shm.shm) {
        retval = get_shmem_seg_name();
        if (retval) {
            sprintf(buf,
                "Can't get shared memory segment name: %s",
                boincerror(retval)
            );
            goto error;
        }
    }

    // this must go AFTER creating shmem,
    // since the shmem name is part of the file
    //
    retval = write_app_init_file();
    if (retval) {
        strcpy(buf, "Can't write init file");
        goto error;
    }

    // set up applications files
    //
    strcpy(exec_name, "");
    for (i=0; i<app_version->app_files.size(); i++) {
        fref = app_version->app_files[i];
        fip = fref.file_info;
        get_pathname(fip, file_path);
        if (fref.main_program) {
            if (is_image_file(fip->name)) {
                sprintf(buf, "Main program %s is an image file", fip->name);
                retval = ERR_NO_SIGNATURE;
                goto error;
            }
            if (!fip->executable && !wup->project->anonymous_platform) {
                sprintf(buf, "Main program %s is not executable", fip->name);
                retval = ERR_NO_SIGNATURE;
                goto error;
            }
            safe_strcpy(exec_name, fip->name);
            safe_strcpy(exec_path, file_path);
        }
        // anonymous platform may use different files than
        // when the result was started
        //
        if (first_time || wup->project->anonymous_platform) {
            retval = setup_file(wup, fip, fref, file_path, slot_dir);
            if (retval) {
                strcpy(buf, "Can't link input file");
                goto error;
            }
        }
    }
    if (!strlen(exec_name)) {
        strcpy(buf, "No main program specified");
        retval = ERR_NOT_FOUND;
        goto error;
    }

    // set up input, output files
    //
    if (first_time) {
        for (i=0; i<wup->input_files.size(); i++) {
            fref = wup->input_files[i];
            fip = fref.file_info;
            get_pathname(fref.file_info, file_path);
            retval = setup_file(wup, fip, fref, file_path, slot_dir);
            if (retval) {
                strcpy(buf, "Can't link input file");
                goto error;
            }
        }
        for (i=0; i<result->output_files.size(); i++) {
            fref = result->output_files[i];
            if (fref.copy_file) continue;
            fip = fref.file_info;
            get_pathname(fref.file_info, file_path);
            retval = setup_file(wup, fip, fref, file_path, slot_dir);
            if (retval) {
                strcpy(buf, "Can't link output file");
                goto error;
            }
        }
    }

    link_user_files();

#ifdef _WIN32
    PROCESS_INFORMATION process_info;
    STARTUPINFO startup_info;
    char slotdirpath[256];
    char error_msg[1024];

    memset(&process_info, 0, sizeof(process_info));
    memset(&startup_info, 0, sizeof(startup_info));
    //startup_info.cb = sizeof(startup_info);
    //startup_info.dwFlags = STARTF_USESHOWWINDOW;
    //startup_info.wShowWindow = SW_HIDE;

    if (!quitRequestEvent) {
        sprintf(buf, "%s%s", QUIT_PREFIX, shmem_seg_name);
        quitRequestEvent = CreateEvent(0, FALSE, FALSE, buf);
        if (quitRequestEvent == NULL) {
            strcpy(buf, "Can't create event");
            retval = ERR_INVALID_EVENT;
            goto error;
        }
    }

    // create core/app share mem segment if needed
    //
    if (!app_client_shm.shm) {
        sprintf(buf, "%s%s", SHM_PREFIX, shmem_seg_name);
        shm_handle = create_shmem(buf, sizeof(SHARED_MEM),
            (void **)&app_client_shm.shm, false
        );
        if (shm_handle == NULL) {
            strcpy(buf, "Can't create shared memory");
            retval = ERR_SHMGET;
            goto error;
        }
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
        msg_printf(wup->project, MSG_ERROR,
            "Process creation failed: %s", error_msg
        );
        boinc_sleep(drand());
    }
    if (!success) {
        sprintf(buf, "CreateProcess() failed - %s", error_msg);
        retval = ERR_EXEC;
        goto error;
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
            sprintf(buf, "Can't create shared memory: %s", boincerror(retval));
            goto error;
        }
    }
    app_client_shm.reset_msgs();

	// save current dir
	getcwd( current_dir, sizeof(current_dir));

	// chdir() into the slot directory
	//
	retval = chdir(slot_dir);
	if (retval) {
		sprintf(buf, "Can't change directory: %s", slot_dir, boincerror(retval));
		goto error;
	}

	// hook up stderr to a specially-named file
	//
	//freopen(STDERR_FILE, "a", stderr);

	argv[0] = exec_name;
	char cmdline[8192];
	strcpy(cmdline, wup->command_line.c_str());
	parse_command_line(cmdline, argv+1);
    if (log_flags.task_debug) {
        debug_print_argv(argv);
    }
	sprintf(buf, "../../%s", exec_path );
	pid = spawnv(P_NOWAIT, buf, argv);
	if (pid == -1) {
		sprintf(buf, "Process creation failed: %s\n", buf, boincerror(retval));
		chdir(current_dir);
        retval = ERR_EXEC;
        goto error;
	}

	// restore current dir
	chdir(current_dir);

    if (log_flags.task_debug) {
        msg_printf(0, MSG_INFO,
            "ACTIVE_TASK::start(): forked process: pid %d\n", pid
        );
    }

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
            shmem_seg_name, sizeof(SHARED_MEM), gstate.boinc_project_gid,
            (void**)&app_client_shm.shm
        );
        if (retval) {
            sprintf(buf, "Can't create shared memory: %s", boincerror(retval));
            goto error;
        }
    }
    app_client_shm.reset_msgs();

#if (defined(__APPLE__) && defined(__i386__))
    // PowerPC apps emulated on i386 Macs crash if running graphics
    powerpc_emulated_on_i386 = ! is_native_i386_app(exec_path);
#endif

    pid = fork();
    if (pid == -1) {
        sprintf(buf, "fork() failed: %s", strerror(errno));
        retval = ERR_FORK;
        goto error;
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

        // set idle process priority
#ifdef HAVE_SETPRIORITY
        if (setpriority(PRIO_PROCESS, 0, PROCESS_IDLE_PRIORITY)) {
            perror("setpriority");
        }
#endif
        char cmdline[8192];
        strcpy(cmdline, wup->command_line.c_str());
        sprintf(buf, "../../%s", exec_path );
#ifdef SANDBOX
        char switcher_path[100];
        sprintf(switcher_path, "../../%s/%s", SWITCHER_DIR, SWITCHER_FILE_NAME);
        argv[0] = SWITCHER_FILE_NAME;
        argv[1] = buf;
        argv[2] = exec_name;
        parse_command_line(cmdline, argv+3);
        if (log_flags.task_debug) {
            debug_print_argv(argv);
        }
        retval = execv(switcher_path, argv);
#else
        argv[0] = exec_name;
        parse_command_line(cmdline, argv+1);
        retval = execv(buf, argv);
#endif
        msg_printf(wup->project, MSG_ERROR,
            "Process creation (%s) failed: %s\n", buf, boincerror(retval)
        );
        perror("execv");
        fflush(NULL);
        _exit(errno);
    }

    if (log_flags.task_debug) {
        msg_printf(0, MSG_INFO,
            "ACTIVE_TASK::start(): forked process: pid %d\n", pid
        );
    }

#endif
    task_state = PROCESS_EXECUTING;
    return 0;

    // go here on error; "buf" contains error message, "retval" is nonzero
    //
error:
    gstate.report_result_error(*result, buf);
    task_state = PROCESS_COULDNT_START;
    return retval;
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
            msg_printf(wup->project, MSG_ERROR,
                "Couldn't resume task %s", result->name
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
            "Unexpected state %d for task %s", task_state, result->name
        );
        return 0;
    }
    if (log_flags.task) {
        msg_printf(result->project, MSG_INFO,
            "%s task %s using %s version %d",
            str,
            result->name,
            app_version->app->name,
            app_version->version_num
        );
    }
    return 0;
}

#if (defined(__APPLE__) && defined(__i386__))

union headeru {
    fat_header fat;
    mach_header mach;
};

// Read the mach-o headers to determine the architectures supported by executable file.
// Returns 1 if application can run natively on i386 Macs, else returns 0.
int ACTIVE_TASK::is_native_i386_app(char* exec_path) {
    FILE *f;
    int result = 0;
    
    headeru myHeader;
    fat_arch fatHeader;
    
    uint32_t n, i, len;
    
    f = boinc_fopen(exec_path, "rb");
    if (!f)
        return result;          // Should never happen
    
    myHeader.fat.magic = 0;
    myHeader.fat.nfat_arch = 0;
    
    fread(&myHeader, 1, sizeof(fat_header), f);
    switch (myHeader.mach.magic) {
    case MH_MAGIC:
        if (myHeader.mach.cputype == CPU_TYPE_I386)
            result = 1;        // Single-architecture i386 file
        break;
    case FAT_CIGAM:
        n = _OSSwapInt32(myHeader.fat.nfat_arch);   // Multiple architecture (fat) file
        for (i=0; i<n; i++) {
            len = fread(&fatHeader, 1, sizeof(fat_arch), f);
            if (len < sizeof(fat_arch))
                break;          // Should never happen
            if (fatHeader.cputype == OSSwapConstInt32(CPU_TYPE_I386)) {
                result = 1;
                break;
            }
        }
        break;
    default:
        break;
    }

    fclose (f);
    return result;
}
#endif

const char *BOINC_RCSID_be8bae8cbb = "$Id$";
