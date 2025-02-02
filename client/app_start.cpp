// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2022 University of California
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

// initialization and starting of applications

#include "cpp.h"

#ifdef _WIN32
#include "boinc_win.h"
#include "win_util.h"
#else
#include "config.h"
#if HAVE_SCHED_SETSCHEDULER && defined (__linux__)
#include <sched.h>
#endif
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
#include <sys/stat.h>
#include <string>
#endif

#ifdef __EMX__
#include <process.h>
#endif

#if(!defined (_WIN32) && !defined (__EMX__))
#include <fcntl.h>
#endif

#include <vector>

using std::vector;
using std::string;

#include "base64.h"
#include "error_numbers.h"
#include "filesys.h"
#include "shmem.h"
#include "str_replace.h"
#include "str_util.h"
#include "util.h"

#include "async_file.h"
#include "client_msgs.h"
#include "client_state.h"
#include "file_names.h"
#include "result.h"
#include "sandbox.h"

#ifdef _WIN32
#include "run_app_windows.h"
#endif

#include "cs_proxy.h"

#include "app.h"


#ifdef _WIN32
// Dynamically link to these functions at runtime;
// otherwise BOINC cannot run on Win98

// CreateEnvironmentBlock
typedef BOOL (WINAPI *tCEB)(LPVOID *lpEnvironment, HANDLE hToken, BOOL bInherit);
// DestroyEnvironmentBlock
typedef BOOL (WINAPI *tDEB)(LPVOID lpEnvironment);

#endif

// print each string in an array
//
#ifndef _WIN32
static void debug_print_argv(char** argv) {
    msg_printf(0, MSG_INFO, "[task] Arguments:");
    for (int i=0; argv[i]; i++) {
        msg_printf(0, MSG_INFO, "[task]    argv[%d]: %s\n", i, argv[i]);
    }
}
#endif

// For apps that use coprocessors, append "--device x" to the command line.
// NOTE: this is deprecated.  Use app_init_data instead.
//
static void coproc_cmdline(
    int rsc_type, RESULT* rp, double ninstances, char* cmdline, int cmdline_len
) {
    char buf[256];
    COPROC* coproc = &coprocs.coprocs[rsc_type];
    for (int j=0; j<ninstances; j++) {
        int k = rp->coproc_indices[j];
        // sanity check
        //
        if (k < 0 || k >= coproc->count) {
            msg_printf(0, MSG_INTERNAL_ERROR,
                "coproc_cmdline: coproc index %d out of range", k
            );
            k = 0;
        }
        snprintf(buf, sizeof(buf), " --device %d", coproc->device_nums[k]);
        strlcat(cmdline, buf, cmdline_len);
    }
}

// Make a unique key for client/app shared memory segment.
// Windows: also create and attach to the segment.
//
int ACTIVE_TASK::get_shmem_seg_name() {
#ifdef _WIN32
    int i;
    char seg_name[256];

    bool try_global = (sandbox_account_service_token != NULL);
    for (i=0; i<1024; i++) {
        snprintf(seg_name, sizeof(seg_name), "%sboinc_%d", SHM_PREFIX, i);
        shm_handle = create_shmem(
            seg_name, sizeof(SHARED_MEM), (void**)&app_client_shm.shm,
            try_global
        );
        if (shm_handle) break;
    }
    if (!shm_handle) return ERR_SHMGET;
    snprintf(shmem_seg_name, sizeof(shmem_seg_name), "boinc_%d", i);
#else
    char init_data_path[MAXPATHLEN];
#ifndef __EMX__
    // shmem_seg_name is not used with mmap() shared memory
    if (app_version->api_version_at_least(6, 0)) {
        shmem_seg_name = -1;
        return 0;
    }
#endif
    snprintf(init_data_path, sizeof(init_data_path), "%s/%s", slot_dir, INIT_DATA_FILE);

    // ftok() only works if there's a file at the given location
    //
    if (!boinc_file_exists(init_data_path)) {
        FILE* f = boinc_fopen(init_data_path, "w");
        if (f) {
            fclose(f);
        } else {
            msg_printf(wup->project, MSG_INTERNAL_ERROR,
                "error: can't open file for shmem seg name"
            );
        }
    }
    shmem_seg_name = ftok(init_data_path, 1);
    if (shmem_seg_name == -1) {
        msg_printf(wup->project, MSG_INTERNAL_ERROR,
            "error: can't open file for shmem seg name: %d", errno
        );
        perror("ftok");
        return ERR_SHMEM_NAME;
    }
#endif
    return 0;
}

void ACTIVE_TASK::init_app_init_data(APP_INIT_DATA& aid) {
    PROJECT* project = wup->project;
    aid.major_version = BOINC_MAJOR_VERSION;
    aid.minor_version = BOINC_MINOR_VERSION;
    aid.release = BOINC_RELEASE;
    aid.app_version = app_version->version_num;
    safe_strcpy(aid.app_name, wup->app->name);
    if (strlen(wup->plan_class)) {
        // BUDA jobs
        safe_strcpy(aid.plan_class, wup->plan_class);
    } else {
        safe_strcpy(aid.plan_class, app_version->plan_class);
    }
    safe_strcpy(aid.symstore, project->symstore);
    safe_strcpy(aid.acct_mgr_url, gstate.acct_mgr_info.master_url);
    if (project->project_specific_prefs.length()) {
        aid.project_preferences = strdup(
            project->project_specific_prefs.c_str()
        );
    }
    aid.userid = project->userid;
    aid.teamid = project->teamid;
    aid.hostid = project->hostid;
    safe_strcpy(aid.user_name, project->user_name);
    safe_strcpy(aid.team_name, project->team_name);
    safe_strcpy(aid.project_dir, project->project_dir_absolute());
    relative_to_absolute("", aid.boinc_dir);
    safe_strcpy(aid.authenticator, project->authenticator);
    aid.slot = slot;
#ifdef _WIN32
    aid.client_pid = GetCurrentProcessId();
#else
    aid.client_pid = getpid();
#endif
    safe_strcpy(aid.wu_name, wup->name);
    safe_strcpy(aid.result_name, result->name);
    aid.user_total_credit = project->user_total_credit;
    aid.user_expavg_credit = project->user_expavg_credit;
    aid.host_total_credit = project->host_total_credit;
    aid.host_expavg_credit = project->host_expavg_credit;
    double rrs = gstate.runnable_resource_share(RSC_TYPE_CPU);
    if (rrs) {
        aid.resource_share_fraction = project->resource_share/rrs;
    } else {
        aid.resource_share_fraction = 1;
    }
    aid.host_info = gstate.host_info;
    aid.proxy_info = working_proxy_info;
    aid.global_prefs = gstate.global_prefs;
    aid.starting_elapsed_time = checkpoint_elapsed_time;
    aid.using_sandbox = g_use_sandbox;
    aid.vm_extensions_disabled = gstate.host_info.p_vm_extensions_disabled;
    aid.rsc_fpops_est = wup->rsc_fpops_est;
    aid.rsc_fpops_bound = wup->rsc_fpops_bound;
    aid.rsc_memory_bound = wup->rsc_memory_bound;
    aid.rsc_disk_bound = wup->rsc_disk_bound;
    aid.computation_deadline = result->computation_deadline();
    int rt = result->resource_usage.rsc_type;
    if (rt) {
        COPROC& cp = coprocs.coprocs[rt];
        if (coproc_type_name_to_num(cp.type) >= 0) {
            // Standardized vendor name ("NVIDIA", "ATI" or "intel_gpu")
            safe_strcpy(aid.gpu_type, cp.type);
        } else {
            // For other vendors, use vendor name as returned by OpenCL
            safe_strcpy(aid.gpu_type, cp.opencl_prop.vendor);
        }
        int k = result->coproc_indices[0];
        if (k<0 || k>=cp.count) {
            msg_printf(0, MSG_INTERNAL_ERROR,
                "init_app_init_data(): coproc index %d out of range", k
            );
            k = 0;
        }
        aid.gpu_device_num = cp.device_nums[k];
        aid.gpu_opencl_dev_index = cp.opencl_device_indexes[k];
        aid.gpu_usage = result->resource_usage.coproc_usage;
    } else {
        safe_strcpy(aid.gpu_type, "");
        aid.gpu_device_num = -1;
        aid.gpu_opencl_dev_index = -1;
        aid.gpu_usage = 0;
    }
    aid.ncpus = result->resource_usage.avg_ncpus;
    aid.vbox_window = cc_config.vbox_window;
    aid.checkpoint_period = gstate.global_prefs.disk_interval;
    aid.fraction_done_start = 0;
    aid.fraction_done_end = 1;
#ifdef _WIN32
    safe_strcpy(aid.shmem_seg_name, shmem_seg_name);
#else
    aid.shmem_seg_name = shmem_seg_name;
#endif
    aid.wu_cpu_time = checkpoint_cpu_time;
    APP_VERSION* avp = app_version;
    for (unsigned int i=0; i<avp->app_files.size(); i++) {
        FILE_REF& fref = avp->app_files[i];
        aid.app_files.push_back(string(fref.file_name));
    }
    aid.no_priority_change = cc_config.no_priority_change;
    aid.process_priority = cc_config.process_priority;
    aid.process_priority_special = cc_config.process_priority_special;
}

// write the app init file.
// This is done before starting or restarting the app,
// and when project prefs have changed during app execution
//
int ACTIVE_TASK::write_app_init_file(APP_INIT_DATA& aid) {
    FILE *f;
    char init_data_path[MAXPATHLEN];

#if 0
    msg_printf(wup->project, MSG_INFO,
        "writing app_init.xml for %s; slot %d rt %s gpu_device_num %d", result->name, slot, aid.gpu_type, aid.gpu_device_num
    );
#endif

    snprintf(init_data_path, sizeof(init_data_path), "%s/%s", slot_dir, INIT_DATA_FILE);

    // delete the file using the switcher (Unix)
    // in case it's owned by another user and we don't have write access
    //
    delete_project_owned_file(init_data_path, false);
    f = boinc_fopen(init_data_path, "w");
    if (!f) {
        msg_printf(wup->project, MSG_INTERNAL_ERROR,
            "Failed to open init file %s",
            init_data_path
        );
        return ERR_FOPEN;
    }

    int retval = write_init_data_file(f, aid);
    fclose(f);
    return retval;
}

// Given a logical name of the form D1/D2/.../Dn/F,
// create the directories D1 ... Dn in the slot dir
//
static int create_dirs_for_logical_name(
    const char* name, const char* slot_dir
) {
    char buf[1024];
    char dir_path[MAXPATHLEN];
    int retval;

    safe_strcpy(buf, name);
    safe_strcpy(dir_path, slot_dir);
    char* p = buf;
    while (1) {
        char* q = strchr(p, '/');
        if (!q) break;
        *q = 0;
        safe_strcat(dir_path, "/");
        safe_strcat(dir_path, p);
        retval = boinc_mkdir(dir_path);
        if (retval) return retval;
        p = q+1;
    }
    return 0;
}

static void prepend_prefix(APP_VERSION* avp, char* in, char* out, int len) {
    if (strlen(avp->file_prefix)) {
        snprintf(out, len, "%.16s/%.200s", avp->file_prefix, in);
    } else {
        strlcpy(out, in, len);
    }
}

// an input/output file must be copied if either
// - the FILE_REFERENCE says so or
// - the APP_VERSION has a non-empty file_prefix
//
bool ACTIVE_TASK::must_copy_file(FILE_REF& fref, bool is_io_file) {
    if (fref.copy_file) return true;
    if (is_io_file && strlen(app_version->file_prefix)) return true;
    return false;
}

// set up a file reference, given a slot dir and project dir.
// This means:
// 1) copy the file to slot dir, if reference is by copy
// 2) else make a soft link
//
int ACTIVE_TASK::setup_file(
    FILE_INFO* fip, FILE_REF& fref, char* file_path, bool input, bool is_io_file
) {
    char link_path[MAXPATHLEN], rel_file_path[MAXPATHLEN], open_name[256];
    int retval;
    PROJECT* project = result->project;

    if (log_flags.slot_debug) {
        msg_printf(wup->project, MSG_INFO,
            "setup_file: %s (%s)", file_path, input?"input":"output"
        );
    }

    if (strlen(fref.open_name)) {
        if (is_io_file) {
            prepend_prefix(
                app_version, fref.open_name, open_name, sizeof(open_name)
            );
        } else {
            safe_strcpy(open_name, fref.open_name);
        }
        retval = create_dirs_for_logical_name(open_name, slot_dir);
        if (retval) return retval;
        snprintf(link_path, sizeof(link_path), "%s/%s", slot_dir, open_name);
    } else {
        snprintf(link_path, sizeof(link_path), "%s/%s", slot_dir, fip->name);
    }

    snprintf(rel_file_path, sizeof(rel_file_path), "../../%s", file_path );

    if (boinc_file_exists(link_path)) {
        return 0;
    }

    if (must_copy_file(fref, is_io_file)) {
        if (input) {
            // the file may be there already (async copy case)
            //
            if (boinc_file_exists(link_path)) {
                return 0;
            }
            if (fip->nbytes > ASYNC_FILE_THRESHOLD) {
                ASYNC_COPY* ac = new ASYNC_COPY;
                retval = ac->init(this, fip, file_path, link_path);
                if (retval) return retval;
                return ERR_IN_PROGRESS;
            } else {
                retval = boinc_copy(file_path, link_path);
                if (retval) {
                    msg_printf(project, MSG_INTERNAL_ERROR,
                        "Can't copy %s to %s: %s", file_path, link_path,
                        boincerror(retval)
                    );
                    return retval;
                }
                retval = fip->set_permissions(link_path);
                if (retval) return retval;
            }
        }
        return 0;
    }

#ifdef _WIN32
    retval = make_soft_link(project, link_path, rel_file_path);
    if (retval) return retval;
#else
    if (project->use_symlinks) {
        retval = symlink(rel_file_path, link_path);
    } else {
        retval = make_soft_link(project, link_path, rel_file_path);
    }
    if (retval) return retval;
#endif
    if (g_use_sandbox) set_to_project_group(link_path);
    return 0;
}

int ACTIVE_TASK::link_user_files() {
    PROJECT* project = wup->project;
    unsigned int i;
    FILE_REF fref;
    FILE_INFO* fip;
    char file_path[MAXPATHLEN];

    for (i=0; i<project->user_files.size(); i++) {
        fref = project->user_files[i];
        fip = fref.file_info;
        if (fip->status != FILE_PRESENT) continue;
        get_pathname(fip, file_path, sizeof(file_path));
        setup_file(fip, fref, file_path, true, false);
    }
    return 0;
}

int ACTIVE_TASK::copy_output_files() {
    char slotfile[MAXPATHLEN], projfile[256], open_name[256];
    unsigned int i;
    for (i=0; i<result->output_files.size(); i++) {
        FILE_REF& fref = result->output_files[i];
        if (!must_copy_file(fref, true)) continue;
        FILE_INFO* fip = fref.file_info;
        prepend_prefix(
            app_version, fref.open_name, open_name, sizeof(open_name)
        );
        snprintf(slotfile, sizeof(slotfile), "%.*s/%.*s", DIR_LEN, slot_dir, FILE_LEN, open_name);
        get_pathname(fip, projfile, sizeof(projfile));
        int retval = boinc_rename(slotfile, projfile);
        // the rename fails if the output file isn't there.
        //
        if (retval) {
            if (retval == ERR_FILE_MISSING) {
                if (log_flags.slot_debug) {
                    msg_printf(wup->project, MSG_INFO,
                        "[slot] output file %s missing, not copying", slotfile
                    );
                }
            } else {
                msg_printf(wup->project, MSG_INTERNAL_ERROR,
                    "Can't rename output file %s to %s: %s",
                    slotfile, projfile, boincerror(retval)
                );
            }
        } else {
            if (log_flags.slot_debug) {
                msg_printf(wup->project, MSG_INFO,
                    "[slot] renamed %s to %s", slotfile, projfile
                );
            }
        }
    }
    return 0;
}

static int get_priority(bool is_high_priority) {
    int p = is_high_priority?cc_config.process_priority_special:cc_config.process_priority;
#ifdef _WIN32
    switch (p) {
    case CONFIG_PRIORITY_LOWEST: return IDLE_PRIORITY_CLASS;
    case CONFIG_PRIORITY_LOW: return BELOW_NORMAL_PRIORITY_CLASS;
    case CONFIG_PRIORITY_NORMAL: return NORMAL_PRIORITY_CLASS;
    case CONFIG_PRIORITY_HIGH: return ABOVE_NORMAL_PRIORITY_CLASS;
    case CONFIG_PRIORITY_HIGHEST: return HIGH_PRIORITY_CLASS;
    case CONFIG_PRIORITY_REALTIME: return REALTIME_PRIORITY_CLASS;
    }
    return is_high_priority ? BELOW_NORMAL_PRIORITY_CLASS : IDLE_PRIORITY_CLASS;
#else
    switch (p) {
    case CONFIG_PRIORITY_LOWEST: return PROCESS_IDLE_PRIORITY;
    case CONFIG_PRIORITY_LOW: return PROCESS_MEDIUM_PRIORITY;
    case CONFIG_PRIORITY_NORMAL: return PROCESS_NORMAL_PRIORITY;
    case CONFIG_PRIORITY_HIGH: return PROCESS_ABOVE_NORMAL_PRIORITY;
    case CONFIG_PRIORITY_HIGHEST: return PROCESS_HIGH_PRIORITY;
    case CONFIG_PRIORITY_REALTIME: return PROCESS_REALTIME_PRIORITY;
    }
    return is_high_priority ? PROCESS_MEDIUM_PRIORITY : PROCESS_IDLE_PRIORITY;
#endif
}

// set up slot dir; copy or link app version and input files.
// If copy a big file, setup_file() will start an async copy
// and return ERR_IN_PROGRESS.
//
int ACTIVE_TASK::setup_slot_dir(char *buf, unsigned int buf_len) {
    unsigned int i;
    int retval;
    FILE_REF fref;
    FILE_INFO *fip;
    char file_path[MAXPATHLEN];
    char path[1024];

    *buf = 0;

    sprintf(path, "%s/boinc_setup_complete", slot_dir);
    if (boinc_file_exists(path)) return 0;

    // make sure the needed files exist
    //
    retval = gstate.task_files_present(result, true, &fip);
    if (retval) {
        snprintf(
            buf, buf_len,
            "Task file %s: %s",
            fip->name, boincerror(retval)
        );
        return retval;
    }

    // set up app version files
    //
    for (i=0; i<app_version->app_files.size(); i++) {
        fref = app_version->app_files[i];
        fip = fref.file_info;
        get_pathname(fip, file_path, sizeof(file_path));
        if (fref.main_program) {
            if (is_image_file(fip->name)) {
                snprintf(buf, buf_len,
                    "Main program %s is an image file", fip->name
                );
                return ERR_NO_SIGNATURE;
            }
            if (!fip->executable && !wup->project->anonymous_platform) {
                snprintf(buf, buf_len,
                    "Main program %s is not executable", fip->name
                );
                return ERR_NO_SIGNATURE;
            }
        }
        retval = setup_file(fip, fref, file_path, true, false);
        if (retval == ERR_IN_PROGRESS) {
            return retval;
        } else if (retval) {
            safe_strcpy(buf, "Can't link app version file");
            return retval;
        }
    }

    // set up input, output files
    //
    for (i=0; i<wup->input_files.size(); i++) {
        fref = wup->input_files[i];
        fip = fref.file_info;
        get_pathname(fref.file_info, file_path, sizeof(file_path));
        retval = setup_file(fip, fref, file_path, true, true);
        if (retval == ERR_IN_PROGRESS) {
            return retval;
        } else if (retval) {
            strcpy(buf, "Can't link input file");
            return retval;
        }
    }
    for (i=0; i<result->output_files.size(); i++) {
        fref = result->output_files[i];
        if (must_copy_file(fref, true)) continue;
        fip = fref.file_info;
        get_pathname(fref.file_info, file_path, sizeof(file_path));
        retval = setup_file(fip, fref, file_path, false, true);
        if (retval) {
            strcpy(buf, "Can't link output file");
            return retval;
        }
    }

    link_user_files();
        // don't check retval here

    FILE *f = fopen(path, "w");
    fclose(f);
    return 0;
}

// Start a task in a slot directory.
// set up the slot dir if needed
//
// Current dir is top-level BOINC dir
//
// postcondition:
// If any error occurs
//   ACTIVE_TASK::task_state is PROCESS_COULDNT_START
//   report_result_error() is called
// else
//   ACTIVE_TASK::task_state is PROCESS_EXECUTING
//
int ACTIVE_TASK::start() {
    char exec_name[256], file_path[MAXPATHLEN], buf[MAXPATHLEN], exec_path[MAXPATHLEN];
    char cmdline[80000];    // 64KB plus some extra
    unsigned int i;
    FILE_REF fref;
    FILE_INFO *fip;
    int retval;
    APP_INIT_DATA aid;
    bool high_priority;
#ifdef _WIN32
    bool success = false;
    LPVOID environment_block=NULL;
#endif

    // is an async copy in progress for this job?
    //
    if (async_copy) {
        if (log_flags.task_debug) {
            msg_printf(wup->project, MSG_INFO,
                "[task_debug] ACTIVE_TASK::start(): async file copy already in progress"
            );
        }
        return 0;
    }

    // use special slot for test app
    //
    if (wup->project->app_test) {
        strcpy(slot_dir, "slots/app_test");
    }

    // set up the slot dir
    //
    retval = setup_slot_dir(buf, sizeof(buf));
    if (retval == ERR_IN_PROGRESS) {
        set_task_state(PROCESS_COPY_PENDING, "start");
        return 0;
    }
    if (retval) goto error;

    // run process at above idle priority if it
    // - uses coprocs
    // - uses less than one CPU
    // - is a wrapper
    //
    high_priority = false;
    if (result->resource_usage.rsc_type) high_priority = true;
    if (result->resource_usage.avg_ncpus < 1) high_priority = true;
    if (app_version->is_wrapper) high_priority = true;

    current_cpu_time = checkpoint_cpu_time;
    elapsed_time = checkpoint_elapsed_time;
    fraction_done = checkpoint_fraction_done;

    graphics_request_queue.init(result->name);        // reset message queues
    process_control_queue.init(result->name);

    bytes_sent_episode = 0;
    bytes_received_episode = 0;

    if (!app_client_shm.shm) {
        retval = get_shmem_seg_name();
        if (retval) {
            snprintf(buf, sizeof(buf),
                "Can't get shared memory segment name: %s",
                boincerror(retval)
            );
            goto error;
        }
    }

    // this must go AFTER creating shmem name,
    // since the shmem name is part of the file
    //
    init_app_init_data(aid);
    retval = write_app_init_file(aid);
    if (retval) {
        snprintf(buf, sizeof(buf),
            "Can't write init file: %s", boincerror(retval)
        );
        goto error;
    }

    // get main prog filename and path
    //
    safe_strcpy(exec_name, "");
    for (i=0; i<app_version->app_files.size(); i++) {
        fref = app_version->app_files[i];
        fip = fref.file_info;
        if (fref.main_program) {
            get_pathname(fip, file_path, sizeof(file_path));
            safe_strcpy(exec_name, fip->name);
            safe_strcpy(exec_path, file_path);
        }
    }
    if (!strlen(exec_name)) {
        safe_strcpy(buf, "No main program specified");
        retval = ERR_NOT_FOUND;
        goto error;
    }

    // remove temporary exit file from last run
    //
    snprintf(file_path, sizeof(file_path), "%s/%s",
        slot_dir, TEMPORARY_EXIT_FILE
    );
    delete_project_owned_file(file_path, true);

    if (cc_config.exit_before_start) {
        msg_printf(0, MSG_INFO, "about to start a job; exiting");
        exit(0);
    }

#ifdef _WIN32
    PROCESS_INFORMATION process_info;
    STARTUPINFO startup_info;
    char slotdirpath[MAXPATHLEN];
    char error_msg[1024];
    char error_msg2[1024];
    DWORD last_error = 0;

    memset(&process_info, 0, sizeof(process_info));
    memset(&startup_info, 0, sizeof(startup_info));
    startup_info.cb = sizeof(startup_info);

    // suppress 2-sec rotating hourglass cursor on startup
    //
    startup_info.dwFlags = STARTF_FORCEOFFFEEDBACK;

    app_client_shm.reset_msgs();

    if (cc_config.run_apps_manually) {
        // fill in client's PID so we won't think app has exited
        //
        pid = GetCurrentProcessId();
        process_handle = GetCurrentProcess();
        set_task_state(PROCESS_EXECUTING, "start");
        return 0;
    }

    snprintf(cmdline, sizeof(cmdline),
        "%s %s %s",
        exec_path, wup->command_line.c_str(), result->resource_usage.cmdline
    );
    if (!app_version->api_version_at_least(7, 5)) {
        int rt = result->resource_usage.rsc_type;
        if (rt) {
            coproc_cmdline(rt, result, result->resource_usage.coproc_usage, cmdline, sizeof(cmdline));
        }
    }

    relative_to_absolute(slot_dir, slotdirpath);
    int prio_mask;
    if (cc_config.no_priority_change) {
        prio_mask = 0;
    } else {
        prio_mask = get_priority(high_priority);
    }

    for (i=0; i<5; i++) {
        last_error = 0;
        if (sandbox_account_service_token != NULL) {

            if (!CreateEnvironmentBlock(&environment_block, sandbox_account_service_token, FALSE)) {
                if (log_flags.task) {
                    windows_format_error_string(GetLastError(), error_msg, sizeof(error_msg));
                    msg_printf(wup->project, MSG_INFO,
                        "Process environment block creation failed: %s", error_msg
                    );
                }
            }

            if (CreateProcessAsUser(
                sandbox_account_service_token,
                exec_path,
                cmdline,
                NULL,
                NULL,
                FALSE,
                CREATE_NEW_PROCESS_GROUP|CREATE_NO_WINDOW|prio_mask|CREATE_UNICODE_ENVIRONMENT,
                environment_block,
                slotdirpath,
                &startup_info,
                &process_info
            )) {
                success = true;
                break;
            } else {
                last_error = GetLastError();
                windows_format_error_string(last_error, error_msg, sizeof(error_msg), exec_path);
                msg_printf(wup->project, MSG_INTERNAL_ERROR,
                    "Process creation failed: %s - error code %d (0x%x)",
                    error_msg, last_error, last_error
                );
            }

            if (!DestroyEnvironmentBlock(environment_block)) {
                if (log_flags.task) {
                    windows_format_error_string(GetLastError(), error_msg, sizeof(error_msg2));
                    msg_printf(wup->project, MSG_INFO,
                        "Process environment block cleanup failed: %s",
                        error_msg2
                    );
                }
            }

        } else {
            if (CreateProcess(
                exec_path,
                cmdline,
                NULL,
                NULL,
                FALSE,
                CREATE_NEW_PROCESS_GROUP|CREATE_NO_WINDOW|prio_mask,
                NULL,
                slotdirpath,
                &startup_info,
                &process_info
            )) {
                success = true;
                break;
            } else {
                last_error = GetLastError();
                windows_format_error_string(last_error, error_msg, sizeof(error_msg), exec_path);
                msg_printf(wup->project, MSG_INTERNAL_ERROR,
                    "Process creation failed: %s - error code %d (0x%x)",
                    error_msg, last_error, last_error
                );
            }
        }
        boinc_sleep(drand());
    }

    if (!success) {
        snprintf(buf, sizeof(buf), "CreateProcess() failed - %s", error_msg);

        if (last_error == ERROR_NOT_ENOUGH_MEMORY) {
            // if CreateProcess() failed because system is low on memory,
            // treat this like a temporary exit;
            // retry in 10 min, and give up after 100 times
            //
            bool will_restart;
            handle_temporary_exit(will_restart, 600, "not enough memory", false);
            if (will_restart) return 0;
        }
        retval = ERR_EXEC;
        goto error;
    }
    pid = process_info.dwProcessId;
    process_handle = process_info.hProcess;
    CloseHandle(process_info.hThread);  // thread handle is not used

#ifdef _WIN64
    // if host has multiple processor groups (i.e. > 64 processors)
    // see which one was used for this job, and show it
    //
    if (log_flags.task_debug && gstate.host_info.n_processor_groups > 0) {
        msg_printf(wup->project, MSG_INFO,
            "[task_debug] task is running in processor group %d",
            get_processor_group(process_handle)
        );
    }
#endif
#elif defined(__EMX__)

    char* argv[100];
    char current_dir[_MAX_PATH];

    // Set up client/app shared memory seg if needed
    //
    if (!app_client_shm.shm) {
        retval = create_shmem(
            shmem_seg_name, sizeof(SHARED_MEM), (void**)&app_client_shm.shm
        );
        if (retval) {
            return retval;
        }
    }
    app_client_shm.reset_msgs();

    // save current dir
    getcwd(current_dir, sizeof(current_dir));

    // chdir() into the slot directory
    //
    retval = chdir(slot_dir);
    if (retval) {
        snprintf(buf, sizeof(buf), "Can't change directory to %s: %s", slot_dir, boincerror(retval));
        goto error;
    }

    // hook up stderr to a specially-named file
    //
    //freopen(STDERR_FILE, "a", stderr);

    argv[0] = exec_name;
    safe_strcpy(cmdline, wup->command_line.c_str());
    if (strlen(result->cmdline)) {
        safe_strcat(cmdline, " ");
        safe_strcat(cmdline, result->cmdline);
    }
    parse_command_line(cmdline, argv+1);
    if (log_flags.task_debug) {
        debug_print_argv(argv);
    }
    snprintf(buf, sizeof(buf), "../../%s", exec_path );
    pid = spawnv(P_NOWAIT, buf, argv);
    if (pid == -1) {
        snprintf(buf, sizeof(buf), "Process creation failed: %s\n", boincerror(retval));
        chdir(current_dir);
        retval = ERR_EXEC;
        goto error;
    }

    // restore current dir
    chdir(current_dir);

    if (log_flags.task_debug) {
        msg_printf(wup->project, MSG_INFO,
            "[task_debug] ACTIVE_TASK::start(): forked process: pid %d\n", pid
        );
    }

    if (!cc_config.no_priority_change) {
        int priority = get_priority(high_priority);
        if (setpriority(PRIO_PROCESS, pid, priority)) {
            perror("setpriority");
        }
    }

#else
    // Unix/Linux/Mac case

    char* argv[100];
    char current_dir[1024];

    if (getcwd(current_dir, sizeof(current_dir)) == NULL) {
        snprintf(buf, sizeof(buf), "Can't get cwd");
        goto error;
    }

    snprintf(cmdline, sizeof(cmdline),
        "%s %s",
        wup->command_line.c_str(), result->resource_usage.cmdline
    );

    if (!app_version->api_version_at_least(7, 5)) {
        int rt = result->resource_usage.rsc_type;
        if (rt) {
            coproc_cmdline(rt, result, result->resource_usage.coproc_usage, cmdline, sizeof(cmdline));
        }
    }

    // Set up client/app shared memory seg if needed
    //
    if (!app_client_shm.shm) {
#ifdef ANDROID
        if (true) {
#else
        if (app_version->api_version_at_least(6, 0)) {
#endif
            // Use mmap() shared memory
            snprintf(buf, sizeof(buf), "%s/%s", slot_dir, MMAPPED_FILE_NAME);
            if (g_use_sandbox) {
                if (!boinc_file_exists(buf)) {
                    int fd = open(buf, O_RDWR | O_CREAT, 0660);
                    if (fd >= 0) {
                        close (fd);
                        if (g_use_sandbox) set_to_project_group(buf);
                    }
                }
            }
            retval = create_shmem_mmap(
                buf, sizeof(SHARED_MEM), (void**)&app_client_shm.shm
            );
            if (retval) {
                msg_printf(wup->project, MSG_INTERNAL_ERROR,
                    "ACTIVE_TASK::start(): can't create memory-mapped file: %s",
                    boincerror(retval)
                );
                return retval;
            }
        } else {
            // Use shmget() shared memory
            retval = create_shmem(
                shmem_seg_name, sizeof(SHARED_MEM), gstate.boinc_project_gid,
                (void**)&app_client_shm.shm
            );

            if (retval) {
                needs_shmem = true;
                destroy_shmem(shmem_seg_name);
                return retval;
            }
        }
        needs_shmem = false;
    }
    app_client_shm.reset_msgs();

    if (cc_config.run_apps_manually) {
        pid = getpid();     // use the client's PID
        set_task_state(PROCESS_EXECUTING, "start");
        return 0;
    }
    pid = fork();
    if (pid == -1) {
        snprintf(buf, sizeof(buf), "fork() failed: %s", strerror(errno));
        retval = ERR_FORK;
        goto error;
    }
    if (pid == 0) {
        // from here on we're running in a new process.
        // If an error happens,
        // exit nonzero so that the client knows there was a problem.

        // don't pass stdout to the app
        //
        int fd = open("/dev/null", O_RDWR);
        dup2(fd, STDOUT_FILENO);
        close(fd);

        // prepend to library path:
        // - the project dir (../../projects/X)
        // - the slot dir (.)
        // - the BOINC dir (../..)
        // (Mac) /usr/local/cuda/lib/
        // We use relative paths in case higher-level dirs
        // are not readable to the account under which app runs
        //
        char libpath[8192];
        char newlibs[256];
        snprintf(newlibs, sizeof(newlibs), "../../%s:.:../..", wup->project->project_dir());
#ifdef __APPLE__
        safe_strcat(newlibs, ":/usr/local/cuda/lib/");
#endif
        char* p = getenv("LD_LIBRARY_PATH");
        if (p) {
            snprintf(libpath, sizeof(libpath), "%s:%s", newlibs, p);
        } else {
            safe_strcpy(libpath, newlibs);
        }
        setenv("LD_LIBRARY_PATH", libpath, 1);

        // On the Mac, do the same for DYLD_LIBRARY_PATH
        //
#ifdef __APPLE__
        p = getenv("DYLD_LIBRARY_PATH");
        if (p) {
            snprintf(libpath, sizeof(libpath), "%s:%s", newlibs, p);
        } else {
            safe_strcpy(libpath, newlibs);
        }
        setenv("DYLD_LIBRARY_PATH", libpath, 1);
#endif

        retval = chdir(slot_dir);
        if (retval) {
            perror("chdir");
            fflush(NULL);
            _exit(errno);
        }

#if 0
        // set stack size limit to the max.
        // Some BOINC apps have reported problems with exceeding
        // small stack limits (e.g. 8 MB)
        // and it seems like the best thing to raise it as high as possible
        //
        struct rlimit rlim;
#define MIN_STACK_LIMIT 64000000
        getrlimit(RLIMIT_STACK, &rlim);
        if (rlim.rlim_cur != RLIM_INFINITY && rlim.rlim_cur <= MIN_STACK_LIMIT) {
            if (rlim.rlim_max == RLIM_INFINITY || rlim.rlim_max > MIN_STACK_LIMIT) {
                rlim.rlim_cur = MIN_STACK_LIMIT;
            } else {
                rlim.rlim_cur = rlim.rlim_max;
            }
            setrlimit(RLIMIT_STACK, &rlim);
        }
#endif

        // hook up stderr to a specially-named file
        //
        if (freopen(STDERR_FILE, "a", stderr) == NULL) {
            _exit(errno);
        }

        // lower our priority if needed
        //
        if (!cc_config.no_priority_change) {
#if HAVE_SETPRIORITY
            int priority = get_priority(high_priority);
            if (setpriority(PRIO_PROCESS, 0, priority)) {
                perror("setpriority");
            }
#endif
#if HAVE_SCHED_SETSCHEDULER && defined(SCHED_IDLE) && defined (__linux__)
            if (!high_priority) {
                struct sched_param sp;
                sp.sched_priority = 0;
                if (sched_setscheduler(0, SCHED_IDLE, &sp)) {
                    perror("app_start sched_setscheduler(SCHED_IDLE)");
                }
            }
#endif
        }

        // Run the application program.
        // If using account-based sandboxing, use a helper app
        // to do this, to set the right user ID
        //
        snprintf(buf, sizeof(buf), "../../%.1024s", exec_path);
        if (g_use_sandbox) {
            char switcher_path[MAXPATHLEN];
            snprintf(switcher_path, sizeof(switcher_path),
                "../../%s/%s",
                SWITCHER_DIR, SWITCHER_FILE_NAME
            );
            argv[0] = const_cast<char*>(SWITCHER_FILE_NAME);
            argv[1] = buf;
            argv[2] = exec_name;
            parse_command_line(cmdline, argv+3);
            if (log_flags.task_debug) {
                debug_print_argv(argv);
            }
            // Files written by projects have user boinc_project
            // and group boinc_project,
            // so they must be world-readable so BOINC CLient can read them
            //
            umask(2);
            retval = execv(switcher_path, argv);
        } else {
            argv[0] = buf;
            parse_command_line(cmdline, argv+1);
            retval = execv(buf, argv);
        }
        fprintf(stderr,
            "Process creation (%s) failed: %s, errno=%d\n",
            buf, boincerror(retval), errno
        );
        perror("execv");
        fflush(NULL);
        _exit(errno);
    }

    // parent process (client) continues here
    //
    if (log_flags.task_debug) {
        msg_printf(wup->project, MSG_INFO,
            "[task_debug] ACTIVE_TASK::start(): forked process: pid %d\n", pid
        );
    }

#endif
    set_task_state(PROCESS_EXECUTING, "start");
    return 0;

error:
    // here on error; "buf" contains error message, "retval" is nonzero
    //

    // if failed to run program, it's possible that the executable was munged.
    // Verify the app version files to detect this
    // and trigger another download if it's the case
    //
    if (retval == ERR_EXEC) {
        gstate.verify_app_version_files(result);
    }

    if (log_flags.task_debug) {
        msg_printf(wup->project, MSG_INFO,
            "[task_debug] couldn't start app: %s", buf
        );
    }

    char err_msg[4096];
    snprintf(err_msg, sizeof(err_msg), "couldn't start app: %.256s", buf);
    gstate.report_result_error(*result, err_msg);

    set_task_state(PROCESS_COULDNT_START, "start");
    return retval;
}

// Resume the task if it was previously running; otherwise start it
// "first_time" is set if the slot dir is empty
// Postcondition: "state" is set correctly
//
int ACTIVE_TASK::resume_or_start(bool first_time) {
    const char* str = "??";
    int retval;

    switch (task_state()) {
    case PROCESS_UNINITIALIZED:
        str = (first_time)?"Starting":"Restarting";
        retval = start();
        if ((retval == ERR_SHMGET) || (retval == ERR_SHMAT)) {
            return retval;
        }
        if (retval) {
            set_task_state(PROCESS_COULDNT_START, "resume_or_start1");
            return retval;
        }
        break;
    case PROCESS_SUSPENDED:
        retval = unsuspend();
        if (retval) {
            msg_printf(wup->project, MSG_INTERNAL_ERROR,
                "Couldn't resume task %s", result->name
            );
            set_task_state(PROCESS_COULDNT_START, "resume_or_start2");
            return retval;
        }
        str = "Resuming";
        break;
    default:
        msg_printf(result->project, MSG_INTERNAL_ERROR,
            "Unexpected state %d for task %s", task_state(), result->name
        );
        return 0;
    }
    if (log_flags.task && first_time) {
        msg_printf(result->project, MSG_INFO,
            "Starting task %s", result->name
        );
    }
    if (log_flags.cpu_sched) {
        char buf[256];
        safe_strcpy(buf, "");
        if (strlen(app_version->plan_class)) {
            snprintf(buf, sizeof(buf), " (%s)", app_version->plan_class);
        }
        msg_printf(result->project, MSG_INFO,
            "[cpu_sched] %s task %s using %s version %d%s in slot %d",
            str,
            result->name,
            app_version->app->name,
            app_version->version_num,
            buf,
            slot
        );
    }
    return 0;
}
