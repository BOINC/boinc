// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2010-2012 University of California
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

// BOINC VirtualBox wrapper; lets you run apps in VMs
// see: http://boinc.berkeley.edu/trac/wiki/VboxApps
//
// usage: vboxwrapper [options]
//
// --trickle X      send a trickle-up message reporting elapsed time every X sec
//                  (use this for credit granting if your app does its
//                  own job management, like CernVM).
// --nthreads N     create a VM with N threads.
// --vmimage N      Use "vm_image_N" as the VM image.
//                  This lets you create an app version with several images,
//                  and the app_plan function can decide which one to use
//                  for the particular host.
// --register_only  Register the VM but don't run it.
//                  Useful for debugging; see the wiki page
//
// Handles:
// - suspend/resume/quit/abort
// - reporting CPU time
// - loss of heartbeat from client
// - checkpoint (using snapshots)
// - a bunch of other stuff; see the wiki page
//
// Contributors:
// Rom Walton
// David Anderson
// Andrew J. Younge (ajy4490 AT umiacs DOT umd DOT edu)
// Jie Wu <jiewu AT cern DOT ch>
// Daniel Lombraña González <teleyinex AT gmail DOT com>

#ifdef _WIN32
#include "boinc_win.h"
#include "win_util.h"
#else
#include <vector>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <cmath>
#include <string>
#include <sstream>
#include <unistd.h>
#endif

#include "version.h"
#include "boinc_api.h"
#include "graphics2.h"
#include "diagnostics.h"
#include "filesys.h"
#include "md5_file.h"
#include "parse.h"
#include "str_util.h"
#include "str_replace.h"
#include "util.h"
#include "error_numbers.h"
#include "procinfo.h"
#include "floppyio.h"
#include "vboxlogging.h"
#include "vboxcheckpoint.h"
#include "vboxwrapper.h"
#include "vbox_common.h"
#ifdef _WIN32
#include "vbox_mscom42.h"
#include "vbox_mscom43.h"
#endif
#include "vbox_vboxmanage.h"


using std::vector;
using std::string;


void read_fraction_done(double& frac_done, VBOX_VM& vm) {
    char path[MAXPATHLEN];
    char buf[256];
    double temp, frac = 0;

    sprintf(path, "shared/%s", vm.fraction_done_filename.c_str());
    FILE* f = fopen(path, "r");
    if (!f) return;

    // read the last line of the file
    //
    fseek(f, -32, SEEK_END);
    while (!feof(f)) {
        char* p = fgets(buf, 256, f);
        if (p == NULL) break;
        int n = sscanf(buf, "%lf", &temp);
        if (n == 1) frac = temp;
    }
    fclose(f);

    if (frac < 0) {
        frac = 0;
    }
    if (frac > 1) {
        frac = 1;
    }

    frac_done = frac;
}

bool completion_file_exists(VBOX_VM& vm) {
    char path[MAXPATHLEN];
    sprintf(path, "shared/%s", vm.completion_trigger_file.c_str());
    if (vm.completion_trigger_file.size() && boinc_file_exists(path)) return true;
    return false;
}

void read_completion_file_info(unsigned long& exit_code, bool& is_notice, string& message, VBOX_VM& vm) {
    char path[MAXPATHLEN];
    char buf[1024];

    exit_code = 0;
    message = "";

    sprintf(path, "shared/%s", vm.completion_trigger_file.c_str());
    FILE* f = fopen(path, "r");
    if (f) {
        if (fgets(buf, 1024, f) != NULL) {
            exit_code = atoi(buf) != 0;
        }
        if (fgets(buf, 1024, f) != NULL) {
            is_notice = atoi(buf) != 0;
        }
        while (fgets(buf, 1024, f) != NULL) {
            message += buf;
        }
        fclose(f);
    }
}

bool temporary_exit_file_exists(VBOX_VM& vm) {
    char path[MAXPATHLEN];
    sprintf(path, "shared/%s", vm.temporary_exit_trigger_file.c_str());
    if (vm.temporary_exit_trigger_file.size() && boinc_file_exists(path)) return true;
    return false;
}

void read_temporary_exit_file_info(int& temp_delay, bool& is_notice, string& message, VBOX_VM& vm) {
    char path[MAXPATHLEN];
    char buf[1024];

    temp_delay = 0;
    message = "";

    sprintf(path, "shared/%s", vm.temporary_exit_trigger_file.c_str());
    FILE* f = fopen(path, "r");
    if (f) {
        if (fgets(buf, 1024, f) != NULL) {
            temp_delay = atoi(buf);
        }
        if (fgets(buf, 1024, f) != NULL) {
            is_notice = atoi(buf) != 0;
        }
        while (fgets(buf, 1024, f) != NULL) {
            message += buf;
        }
        fclose(f);
    }
}

void delete_temporary_exit_trigger_file(VBOX_VM& vm) {
    char path[MAXPATHLEN];
    sprintf(path, "shared/%s", vm.temporary_exit_trigger_file.c_str());
    boinc_delete_file(path);
}

// set CPU and network throttling if needed
//
void set_throttles(APP_INIT_DATA& aid, VBOX_VM& vm) {
    double x = 0, y = 0;

    // VirtualBox freaks out if the CPU Usage value is too low to actually
    // do any processing.  It probably wouldn't be so bad if the RDP interface
    // didn't also get hosed by it.
    //
    x = aid.global_prefs.cpu_usage_limit;
    // 0 means "no limit"
    //
    if (x == 0.0) x = 100;
    // For now set the minimum CPU Usage value to 1.
    //
    if (x < 1) x = 1;
    vm.set_cpu_usage((int)x);

    // vbox doesn't distinguish up and down bandwidth; use the min of the prefs
    //
    x = aid.global_prefs.max_bytes_sec_up;
    y = aid.global_prefs.max_bytes_sec_down;
    if (y) {
        if (!x || y < x) {
            x = y;
        }
    }
    if (x) {
        vm.set_network_usage((int)(x/1024));
    }

}

// If the Floppy device has been specified, initialize its state so that
// it contains the contents of the init_data.xml file.
// In theory this would allow network enabled VMs to know about
// proxy server configurations either specified by the volunteer
// or automatically detected by the client.
//
// CERN decided they only needed a small subset of the data and changed the
// data format to 'name=value\n' pairs.  So if we are running under their
// environment set things up accordingly.
//
void set_floppy_image(APP_INIT_DATA& aid, VBOX_VM& vm) {
    int retval;
    char buf[256];
    std::string scratch;

    if (vm.enable_floppyio) {
        scratch = "";
        if (!vm.enable_cern_dataformat) {
            retval = read_file_string(INIT_DATA_FILE, scratch);
            if (retval) {
                vboxlog_msg("WARNING: Cannot write init_data.xml to floppy abstration device");
            }
        } else {
            // Per: https://github.com/stig/json-framework/issues/48
            //
            // Use %.17g to represent doubles
            //
            scratch  = "BOINC_USERNAME=" + string(aid.user_name) + "\n";
            scratch += "BOINC_AUTHENTICATOR=" + string(aid.authenticator) + "\n";

            sprintf(buf, "%d", aid.userid);
            scratch += "BOINC_USERID=" + string(buf) + "\n";

            sprintf(buf, "%d", aid.hostid);
            scratch += "BOINC_HOSTID=" + string(buf) + "\n";

            sprintf(buf, "%.17g", aid.user_total_credit);
            scratch += "BOINC_USER_TOTAL_CREDIT=" + string(buf) + "\n";

            sprintf(buf, "%.17g", aid.host_total_credit);
            scratch += "BOINC_HOST_TOTAL_CREDIT=" + string(buf) + "\n";
        }
        vm.write_floppy(scratch);
    }
}

// if there's a port for web graphics, tell the client about it
//
void report_web_graphics_url(VBOX_VM& vm) {
    char buf[256];
    if (vm.pf_host_port && !boinc_file_exists("graphics_app")) {
        sprintf(buf, "http://localhost:%d", vm.pf_host_port);
        vboxlog_msg("Detected: Web Application Enabled (%s)", buf);
        boinc_web_graphics_url(buf);
    }
}

// set remote desktop information if needed
//
void report_remote_desktop_info(VBOX_VM& vm) {
    char buf[256];
    if (vm.rd_host_port) {
        sprintf(buf, "localhost:%d", vm.rd_host_port);
        vboxlog_msg("Detected: Remote Desktop Enabled (%s)", buf);
        boinc_remote_desktop_addr(buf);
    }
}

// check for trickle trigger files, and send trickles if find them.
//
void check_trickle_triggers(VBOX_VM& vm) {
    int retval;
    char filename[256], path[MAXPATHLEN];
    std::string text;
    for (unsigned int i=0; i<vm.trickle_trigger_files.size(); i++) {
        strcpy(filename, vm.trickle_trigger_files[i].c_str());
        sprintf(path, "shared/%s", filename);
        if (!boinc_file_exists(path)) continue;
        vboxlog_msg("Reporting a trickle. (%s)", filename);
        retval = read_file_string(path, text);
        if (retval) {
            vboxlog_msg("ERROR: can't read trickle trigger file %s", filename);
        } else {
            retval = boinc_send_trickle_up(
                filename, const_cast<char*>(text.c_str())
            );
            if (retval) {
                vboxlog_msg("boinc_send_trickle_up() failed: %s (%d)", boincerror(retval), retval);
            }
        }
        boinc_delete_file(path);
    }
}

// check for intermediate upload files, and send them if found.
//
void check_intermediate_uploads(VBOX_VM& vm) {
    int retval;
    char filename[256], path[MAXPATHLEN];
    for (unsigned int i=0; i<vm.intermediate_upload_files.size(); i++) {
        strcpy(filename, vm.intermediate_upload_files[i].file.c_str());
        sprintf(path, "shared/%s", filename);
        if (!boinc_file_exists(path)) continue;
        if (!vm.intermediate_upload_files[i].reported && !vm.intermediate_upload_files[i].ignore) {
            vboxlog_msg("Reporting an intermediate file. (%s)", vm.intermediate_upload_files[i].file.c_str());
            retval = boinc_upload_file(vm.intermediate_upload_files[i].file);
            if (retval) {
                vboxlog_msg("boinc_upload_file() failed: %s", boincerror(retval));
                vm.intermediate_upload_files[i].ignore = true;
            } else {
                vm.intermediate_upload_files[i].reported = true;
            }
        } else if (vm.intermediate_upload_files[i].reported && !vm.intermediate_upload_files[i].ignore) {
            retval = boinc_upload_status(vm.intermediate_upload_files[i].file);
            if (!retval) {
                vboxlog_msg("Intermediate file uploaded. (%s)", vm.intermediate_upload_files[i].file.c_str());
                vm.intermediate_upload_files[i].ignore = true;
            }
        }
    }
}

// see if it's time to send trickle-up reporting elapsed time
//
void check_trickle_period(double& elapsed_time, double& trickle_period) {
    char buf[256];
    static double last_trickle_report_time = 0;

    if ((elapsed_time - last_trickle_report_time) < trickle_period) {
        return;
    }
    last_trickle_report_time = elapsed_time;
    vboxlog_msg("Status Report: Trickle-Up Event.");
    sprintf(buf,
        "<cpu_time>%f</cpu_time>", last_trickle_report_time
    );
    int retval = boinc_send_trickle_up(
        const_cast<char*>("cpu_time"), buf
    );
    if (retval) {
        vboxlog_msg("Sending Trickle-Up Event failed (%d).", retval);
    }
}

int main(int argc, char** argv) {
    int retval = 0;
    int loop_iteration = 0;
    BOINC_OPTIONS boinc_options;
    APP_INIT_DATA aid;
    VBOX_VM* pVM = NULL;
    VBOX_CHECKPOINT checkpoint;
    double random_checkpoint_factor = 0;
    double elapsed_time = 0;
    double fraction_done = 0;
    double trickle_period = 0;
    double current_cpu_time = 0;
    double starting_cpu_time = 0;
    double last_checkpoint_cpu_time = 0;
    double last_checkpoint_elapsed_time = 0;
    double last_status_report_time = 0;
    double stopwatch_starttime = 0;
    double stopwatch_endtime = 0;
    double stopwatch_elapsedtime = 0;
    double sleep_time = 0;
    double bytes_sent = 0;
    double bytes_received = 0;
    double ncpus = 0;
    double memory_size_mb = 0;
    double timeout = 0.0;
    bool report_net_usage = false;
    double net_usage_timer = 600;
	int vm_image = 0;
    unsigned long vm_exit_code = 0;
    bool is_notice = false;
    int temp_delay = 86400;
    string message;
    char buf[256];


    // Initialize diagnostics system
    //
    boinc_init_diagnostics(BOINC_DIAG_DEFAULTS);

    // Configure BOINC Runtime System environment
    //
    memset(&boinc_options, 0, sizeof(boinc_options));
    boinc_options.main_program = true;
    boinc_options.check_heartbeat = true;
    boinc_options.handle_process_control = true;
    boinc_init_options(&boinc_options);

    // Log banner
    //
    vboxlog_msg("vboxwrapper (%d.%d.%d): starting", BOINC_MAJOR_VERSION, BOINC_MINOR_VERSION, VBOXWRAPPER_RELEASE);

    // Initialize system services
    // 
#ifdef _WIN32
    CoInitialize(NULL);
#ifdef USE_WINSOCK
    WSADATA wsdata;
    retval = WSAStartup( MAKEWORD( 1, 1 ), &wsdata);
    if (retval) {
        vboxlog_msg("ERROR: Cannot initialize winsock: %d", retval);
        boinc_finish(retval);
    }
#endif
#endif

    // Prepare environment for detecting system conditions
    //
    boinc_parse_init_data_file();
    boinc_get_init_data(aid);

#ifdef _WIN32
    // Determine what version of VirtualBox we are using via the registry. Use a
    // namespace specific version of the function because VirtualBox has been known
    // to change the registry location from time to time.
    //
    // NOTE: We cannot use COM to automatically detect which interfaces are installed
    //       on the machine because it will attempt to launch the 'vboxsvc' process
    //       without out environment variable changes and muck everything up.
    //
    string vbox_version;
    int vbox_major = 0, vbox_minor = 0;

    if (BOINC_SUCCESS != vbox42::VBOX_VM::get_version_information(vbox_version)) {
        vbox43::VBOX_VM::get_version_information(vbox_version);
    }
    if (!vbox_version.empty()) {
        sscanf(vbox_version.c_str(), "%d.%d", &vbox_major, &vbox_minor);
        if ((4 == vbox_major) && (2 == vbox_minor)) {
            pVM = (VBOX_VM*) new vbox42::VBOX_VM();
        }
        if ((4 == vbox_major) && (3 == vbox_minor)) {
            pVM = (VBOX_VM*) new vbox43::VBOX_VM();
        }
    }
    if (!pVM) {
        pVM = (VBOX_VM*) new vboxmanage::VBOX_VM();
    }
#else
    pVM = (VBOX_VM*) new vboxmanage::VBOX_VM();
#endif

    // Parse command line parameters
    //
    for (int i=1; i<argc; i++) {
        if (!strcmp(argv[i], "--trickle")) {
            trickle_period = atof(argv[++i]);
        }
        if (!strcmp(argv[i], "--ncpus")) {
            ncpus = atof(argv[++i]);
        }
        if (!strcmp(argv[i], "--memory_size_mb")) {
            memory_size_mb = atof(argv[++i]);
        }
        if (!strcmp(argv[i], "--vmimage")) {
            vm_image = atoi(argv[++i]);
        }
        if (!strcmp(argv[i], "--register_only")) {
            pVM->register_only = true;
        }
    }

    // Choose a random interleave value for checkpoint intervals to stagger disk I/O.
    // 
    struct stat vm_image_stat;
    if (-1 == stat(IMAGE_FILENAME_COMPLETE, &vm_image_stat)) {
        srand((int)time(NULL));
    } else {
        srand((int)(vm_image_stat.st_mtime * time(NULL)));
    }
    random_checkpoint_factor = (double)(((int)(drand() * 100000.0)) % 600);
    vboxlog_msg("Feature: Checkpoint interval offset (%d seconds)", (int)random_checkpoint_factor);

    // Display trickle value if specified
    //
    if (trickle_period > 0.0) {
        vboxlog_msg("Feature: Enabling trickle-ups (Interval: %f)", trickle_period);
    }

    // Check for architecture incompatibilities
    // 
#if defined(_WIN32) && defined(_M_IX86)
    if (strstr(aid.host_info.os_version, "x64")) {
        vboxlog_msg("64-bit version of BOINC is required, please upgrade. Rescheduling execution for a later date.");
        boinc_temporary_exit(86400, "Architecture incompatibility detected.");
    }
#endif

    // Initialize VM Hypervisor
    //
    retval = pVM->initialize();
    if (retval) {
        vboxlog_msg("Could not detect VM Hypervisor. Rescheduling execution for a later date.");
        boinc_temporary_exit(86400, "Detection of VM Hypervisor failed.");
    }

    // Record what version of VirtualBox was used.
    // 
    if (!pVM->virtualbox_version.empty()) {
        vboxlog_msg("Detected: %s", pVM->virtualbox_version.c_str());
    }

    // Record if anonymous platform was used.
    // 
    if (boinc_file_exists((std::string(aid.project_dir) + std::string("/app_info.xml")).c_str())) {
        vboxlog_msg("Detected: Anonymous Platform Enabled");
    }

    // Record if the sandboxed configuration is going to be used.
    //
    if (aid.using_sandbox) {
        vboxlog_msg("Detected: Sandbox Configuration Enabled");
    }

    // Record which mode VirtualBox should be started in.
    //
    if (aid.vbox_window || boinc_is_standalone()) {
        vboxlog_msg("Detected: Headless Mode Disabled");
        pVM->headless = false;
    }

    // Check for invalid confgiurations.
    //
    if (aid.using_sandbox && aid.vbox_window) {
        vboxlog_msg("Invalid configuration detected.");
        vboxlog_msg("NOTE: BOINC cannot be installed as a service and run VirtualBox in headfull mode at the same time.");
        boinc_temporary_exit(86400, "Incompatible configuration detected.");
    }

    // Check against known incompatible versions of VirtualBox.  
    // VirtualBox 4.2.6 crashes during snapshot operations
    // and 4.2.18 fails to restore from snapshots properly.
    //
    if ((pVM->virtualbox_version.find("4.2.6") != std::string::npos) || 
        (pVM->virtualbox_version.find("4.2.18") != std::string::npos) || 
        (pVM->virtualbox_version.find("4.3.0") != std::string::npos) ) {
        vboxlog_msg("Incompatible version of VirtualBox detected. Please upgrade to a later version.");
        boinc_temporary_exit(86400,
            "Incompatible version of VirtualBox detected; please upgrade.",
            true
        );
    }

    // Check to see if the system is in a state in which we expect to be able to run
    // VirtualBox successfully.  Sometimes the system is in a wierd state after a
    // reboot and the system needs a little bit of time.
    //
    if (!pVM->is_system_ready(message)) {
        vboxlog_msg("Could not communicate with VM Hypervisor. Rescheduling execution for a later date.");
        boinc_temporary_exit(86400, message.c_str());
    }

    // Parse Job File
    //
    retval = pVM->parse();
    if (retval) {
        vboxlog_msg("ERROR: Cannot parse job file: %d", retval);
        boinc_finish(retval);
    }

    // Record what the minimum checkpoint interval is.
    //
    vboxlog_msg("Detected: Minimum checkpoint interval (%f seconds)", pVM->minimum_checkpoint_interval);

    // Validate whatever configuration options we can
    //
    if (pVM->enable_shared_directory) {
        if (boinc_file_exists("shared")) {
            if (!is_dir("shared")) {
                vboxlog_msg("ERROR: 'shared' exists but is not a directory.");
            }
        } else {
            retval = boinc_mkdir("shared");
            if (retval) {
                vboxlog_msg("ERROR: couldn't created shared directory: %s.", boincerror(retval));
            }
        }
    }

    // Copy files to the shared directory
    //
    if (pVM->enable_shared_directory && pVM->copy_to_shared.size()) {
        for (vector<string>::iterator iter = pVM->copy_to_shared.begin(); iter != pVM->copy_to_shared.end(); ++iter) {
            string source = *iter;
            string destination = string("shared/") + *iter;
            if (!boinc_file_exists(destination.c_str())) {
                if (!boinc_copy(source.c_str(), destination.c_str())) {
                    vboxlog_msg("Successfully copied '%s' to the shared directory.", source.c_str());
                } else {
                    vboxlog_msg("Failed to copy '%s' to the shared directory.", source.c_str());
                }
            }
        }
    }

    // Configure Instance specific VM Parameters
    //
    pVM->vm_master_name = "boinc_";
    pVM->image_filename = IMAGE_FILENAME_COMPLETE;
    if (boinc_is_standalone()) {
        pVM->vm_master_name += "standalone";
        pVM->vm_master_description = "standalone";
        if (pVM->enable_floppyio) {
            sprintf(buf, "%s.%s",
                FLOPPY_IMAGE_FILENAME, FLOPPY_IMAGE_FILENAME_EXTENSION
            );
            pVM->floppy_image_filename = buf;
        }
    } else {
        pVM->vm_master_name += md5_string(std::string(aid.result_name)).substr(0, 16);
        pVM->vm_master_description = aid.result_name;
		if (vm_image) {
            sprintf(buf, "%s_%d.%s",
                IMAGE_FILENAME, vm_image, IMAGE_FILENAME_EXTENSION
            );
            pVM->image_filename = buf;
		}
        if (pVM->enable_floppyio) {
            sprintf(buf, "%s_%d.%s",
                FLOPPY_IMAGE_FILENAME, aid.slot,
                FLOPPY_IMAGE_FILENAME_EXTENSION
            );
            pVM->floppy_image_filename = buf;
        }
    }
    if (pVM->enable_cache_disk) {
        pVM->cache_disk_filename = CACHE_DISK_FILENAME;
    }
    if (pVM->enable_isocontextualization) {
        pVM->iso_image_filename = ISO_IMAGE_FILENAME;
    }
    if (aid.ncpus > 1.0 || ncpus > 1.0) {
        if (ncpus) {
            sprintf(buf, "%d", (int)ceil(ncpus));
        } else {
            sprintf(buf, "%d", (int)ceil(aid.ncpus));
        }
        pVM->vm_cpu_count = buf;
    } else {
        pVM->vm_cpu_count = "1";
    }
    if (pVM->memory_size_mb > 1.0 || memory_size_mb > 1.0) {
        if (memory_size_mb) {
            sprintf(buf, "%d", (int)ceil(memory_size_mb));
        } else {
            sprintf(buf, "%d", (int)ceil(pVM->memory_size_mb));
        }
    }
    if (aid.vbox_window && !aid.using_sandbox) {
        pVM->headless = false;
    }

    // Restore from checkpoint
    //
    checkpoint.parse();
    elapsed_time = checkpoint.elapsed_time;
    current_cpu_time = checkpoint.cpu_time;
    pVM->pf_host_port = checkpoint.webapi_port;
    pVM->rd_host_port = checkpoint.remote_desktop_port;
    last_checkpoint_elapsed_time = elapsed_time;
    starting_cpu_time = current_cpu_time;
    last_checkpoint_cpu_time = current_cpu_time;

    // Should we even try to start things up?
    //
    if (pVM->job_duration && (elapsed_time > pVM->job_duration)) {
        return EXIT_TIME_LIMIT_EXCEEDED;
    }

    retval = pVM->run((current_cpu_time > 0));
    if (retval) {
        // All 'failure to start' errors are unrecoverable by default
        bool   unrecoverable_error = true;
        bool   skip_cleanup = false;
        bool   do_dump_hypervisor_logs = false;
        string error_reason;
        const char*  temp_reason = "";

        if (VBOXWRAPPER_ERR_RECOVERABLE == retval) {
            error_reason =
                "    BOINC will be notified that it needs to clean up the environment.\n"
                "    This is a temporary problem and so this job will be rescheduled for another time.\n";
            unrecoverable_error = false;
            temp_reason = "VM environment needed to be cleaned up.";
        } else if (ERR_NOT_EXITED == retval) {
            error_reason =
                "   NOTE: VM was already running.\n"
                "    BOINC will be notified that it needs to clean up the environment.\n"
                "    This might be a temporary problem and so this job will be rescheduled for another time.\n";
            unrecoverable_error = false;
            temp_reason = "VM environment needed to be cleaned up.";
        } else if (ERR_INVALID_PARAM == retval) {
            unrecoverable_error = false;
            temp_reason = "Please upgrade BOINC to the latest version.";
            temp_delay = 86400;
        } else if (retval == (int)RPC_S_SERVER_UNAVAILABLE) {
            error_reason =
                "    VboxSvc crashed while attempting to restore the current snapshot.  This is a critical\n"
                "    operation and this job cannot be recovered.\n";
            skip_cleanup = true;
            retval = ERR_EXEC;
        } else if (retval == (int)VBOX_E_INVALID_OBJECT_STATE) {
            error_reason =
                "   NOTE: VM session lock error encountered.\n"
                "    BOINC will be notified that it needs to clean up the environment.\n"
                "    This might be a temporary problem and so this job will be rescheduled for another time.\n";
            unrecoverable_error = false;
            temp_reason = "VM environment needed to be cleaned up.";
        } else if (pVM->is_logged_failure_vm_extensions_disabled()) {
            error_reason =
                "   NOTE: BOINC has detected that your computer's processor supports hardware acceleration for\n"
                "    virtual machines but the hypervisor failed to successfully launch with this feature enabled.\n"
                "    This means that the hardware acceleration feature has been disabled in the computer's BIOS.\n"
                "    Please enable this feature in your computer's BIOS.\n"
                "    Intel calls it 'VT-x'\n"
                "    AMD calls it 'AMD-V'\n"
                "    More information can be found here: http://en.wikipedia.org/wiki/X86_virtualization\n"
                "    Error Code: ERR_CPU_VM_EXTENSIONS_DISABLED\n";
            retval = ERR_EXEC;
        } else if (pVM->is_logged_failure_vm_extensions_not_supported()) {
            error_reason =
                "   NOTE: VirtualBox has reported an improperly configured virtual machine. It was configured to require\n"
                "    hardware acceleration for virtual machines, but your processor does not support the required feature.\n"
                "    Please report this issue to the project so that it can be addresssed.\n";
        } else if (pVM->is_logged_failure_vm_extensions_in_use()) {
            error_reason =
                "   NOTE: VirtualBox hypervisor reports that another hypervisor has locked the hardware acceleration\n"
                "    for virtual machines feature in exclusive mode.\n";
            unrecoverable_error = false;
            temp_reason = "Forign VM Hypervisor locked hardware acceleration features.";
            temp_delay = 86400;
        } else if (pVM->is_logged_failure_host_out_of_memory()) {
            error_reason =
                "   NOTE: VirtualBox has failed to allocate enough memory to start the configured virtual machine.\n"
                "    This might be a temporary problem and so this job will be rescheduled for another time.\n";
            unrecoverable_error = false;
            temp_reason = "VM Hypervisor was unable to allocate enough memory to start VM.";
        } else {
            do_dump_hypervisor_logs = true;
        }

        if (unrecoverable_error) {
            // Attempt to cleanup the VM and exit.
            if (!skip_cleanup) {
                pVM->cleanup();
            }

            checkpoint.update(elapsed_time, current_cpu_time);

            if (error_reason.size()) {
                vboxlog_msg("\n%s", error_reason.c_str());
            }

            if (do_dump_hypervisor_logs) {
                pVM->dump_hypervisor_logs(true);
            }

            boinc_finish(retval);
        } else {
            // if the VM is already running notify BOINC about the process ID so it can
            // clean up the environment.  We should be safe to run after that.
            //
            if (pVM->vm_pid) {
                retval = boinc_report_app_status_aux(
                    current_cpu_time,
                    last_checkpoint_cpu_time,
                    fraction_done,
                    pVM->vm_pid,
                    bytes_sent,
                    bytes_received
                );
            }
 
            // Give the BOINC API time to report the pid to BOINC.
            //
            boinc_sleep(5.0);

            if (error_reason.size()) {
                vboxlog_msg("\n%s", error_reason.c_str());
            }

            // Exit and let BOINC clean up the rest.
            //
            boinc_temporary_exit(temp_delay, temp_reason);
        }
    }

    // Report the VM pid to BOINC so BOINC can deal with it when needed.
    //
    vboxlog_msg("Reporting VM Process ID to BOINC.");
    retval = boinc_report_app_status_aux(
        current_cpu_time,
        last_checkpoint_cpu_time,
        fraction_done,
        pVM->vm_pid,
        bytes_sent,
        bytes_received
    );

    // Wait for up to 5 minutes for the VM to switch states.
    // A system under load can take a while.
    // Since the poll function can wait for up to 60 seconds
    // to execute a command we need to make this time based instead
    // of iteration based.
    //
    timeout = dtime() + 300;
    do {
        pVM->poll(false);
        if (pVM->online && !pVM->restoring) break;
        boinc_sleep(1.0);
    } while (timeout >= dtime());

    // Lower the VM process priority after it has successfully brought itself online.
    //
    pVM->lower_vm_process_priority();

    // Log our current state 
    pVM->poll(true);

    // Did we timeout?
    if (!pVM->online && (timeout <= dtime())) {
        vboxlog_msg("NOTE: VM failed to enter an online state within the timeout period.");
        vboxlog_msg("  This might be a temporary problem and so this job will be rescheduled for another time.");
        pVM->reset_vm_process_priority();
        pVM->poweroff();
        pVM->dump_hypervisor_logs(true);
        boinc_temporary_exit(86400,
            "VM Hypervisor failed to enter an online state in a timely fashion."
        );
    }

    set_floppy_image(aid, *pVM);
    report_web_graphics_url(*pVM);
    report_remote_desktop_info(*pVM);
    checkpoint.webapi_port = pVM->pf_host_port;
    checkpoint.remote_desktop_port = pVM->rd_host_port;
    checkpoint.update(elapsed_time, current_cpu_time);

    // Force throttling on our first pass through the loop
    boinc_status.reread_init_data_file = true;

    while (1) {
        // Begin stopwatch timer
        stopwatch_starttime = dtime();
        loop_iteration += 1;

        // Discover the VM's current state
        pVM->poll();

        // Write updates for the graphics application's use
        if (pVM->enable_graphics_support) {
            boinc_write_graphics_status(current_cpu_time, elapsed_time, fraction_done);
        }

        if (boinc_status.no_heartbeat || boinc_status.quit_request) {
            pVM->reset_vm_process_priority();
            pVM->poweroff();
            boinc_temporary_exit(86400);
        }
        if (boinc_status.abort_request) {
            pVM->reset_vm_process_priority();
            pVM->cleanup();
            pVM->dump_hypervisor_logs(true);
            boinc_finish(EXIT_ABORTED_BY_CLIENT);
        }
        if (completion_file_exists(*pVM)) {
            vboxlog_msg("VM Completion File Detected.");
            read_completion_file_info(vm_exit_code, is_notice, message, *pVM);
            if (message.size()) {
                vboxlog_msg("VM Completion Message: %s.", message.c_str());
            }
            pVM->reset_vm_process_priority();
            pVM->cleanup();
            if (is_notice) {
                boinc_finish_message(vm_exit_code, message.c_str(), is_notice);
            } else {
                boinc_finish(vm_exit_code);
            }
        }
        if (temporary_exit_file_exists(*pVM)) {
            vboxlog_msg("VM Temporary Exit File Detected.");
            read_temporary_exit_file_info(temp_delay, is_notice, message, *pVM);
            if (message.size()) {
                vboxlog_msg("VM Temporary Exit Message: %s.", message.c_str());
            }
            delete_temporary_exit_trigger_file(*pVM);
            pVM->reset_vm_process_priority();
            pVM->stop();
            if (is_notice) {
                boinc_temporary_exit(temp_delay, message.c_str(), is_notice);
            } else {
                boinc_temporary_exit(temp_delay);
            }
        }
        if (!pVM->online) {
            // Is this a type of event we can recover from?
            if (pVM->is_logged_failure_host_out_of_memory()) {
                vboxlog_msg("NOTE: VirtualBox has failed to allocate enough memory to continue.");
                vboxlog_msg("  This might be a temporary problem and so this job will be rescheduled for another time.");
                pVM->reset_vm_process_priority();
                pVM->poweroff();
                boinc_temporary_exit(86400, "VM Hypervisor was unable to allocate enough memory.");
            } else {
                pVM->cleanup();
                if (pVM->crashed || (elapsed_time < pVM->job_duration)) {
                    vboxlog_msg("VM Premature Shutdown Detected.");
                    pVM->dump_hypervisor_logs(true);
                    pVM->get_vm_exit_code(vm_exit_code);
                    if (vm_exit_code) {
                        boinc_finish(vm_exit_code);
                    } else {
                        boinc_finish(EXIT_ABORTED_BY_CLIENT);
                    }
                } else {
                    vboxlog_msg("Virtual machine exited.");
                    pVM->dump_hypervisor_logs(false);
                    boinc_finish(0);
                }
            }
        } else {
            // Check to see if the guest VM has any log messages that indicate that we need need
            // to take action.
            if (pVM->is_logged_failure_guest_job_out_of_memory()) {
                vboxlog_msg("ERROR: VM reports there is not enough memory to finish the task.");
                pVM->reset_vm_process_priority();
                pVM->dump_hypervisor_logs(true);
                pVM->poweroff();
                boinc_finish(EXIT_OUT_OF_MEMORY);
            }
        }
        if (boinc_status.suspended) {
            if (!pVM->suspended) {
                retval = pVM->pause();
                if (retval && (VBOX_E_INVALID_OBJECT_STATE == retval)) {
                    vboxlog_msg("ERROR: VM task failed to pause, rescheduling task for a later time.");
                    pVM->poweroff();
                    boinc_temporary_exit(86400, "VM job unmanageable, restarting later.");
               }
            }
        } else {
            if (pVM->suspended) {
                retval = pVM->resume();
                if (retval && (VBOX_E_INVALID_OBJECT_STATE == retval)) {
                    vboxlog_msg("ERROR: VM task failed to resume, rescheduling task for a later time.");
                    pVM->poweroff();
                    boinc_temporary_exit(86400, "VM job unmanageable, restarting later.");
               }
            }

            // stuff to do every 10 secs (everything else is 1/sec)
            //
            if ((loop_iteration % 10) == 0) {
                current_cpu_time = starting_cpu_time + pVM->get_vm_cpu_time();
                check_trickle_triggers(*pVM);
                check_intermediate_uploads(*pVM);
            }

            if (pVM->job_duration) {
                fraction_done = elapsed_time / pVM->job_duration;
            } else if (pVM->fraction_done_filename.size() > 0) {
                read_fraction_done(fraction_done, *pVM);
            }
            if (fraction_done > 1.0) {
                fraction_done = 1.0;
            }
            boinc_report_app_status(
                current_cpu_time,
                last_checkpoint_cpu_time,
                fraction_done
            );

            // write status report to stderr at regular intervals
            //
            if ((elapsed_time - last_status_report_time) >= 6000.0) {
                last_status_report_time = elapsed_time;
                if (pVM->job_duration) {
                    vboxlog_msg("Status Report: Job Duration: '%f'", pVM->job_duration);
                }
                if (elapsed_time) {
                    vboxlog_msg("Status Report: Elapsed Time: '%f'", elapsed_time);
                }
                vboxlog_msg("Status Report: CPU Time: '%f'", current_cpu_time);
                if (aid.global_prefs.daily_xfer_limit_mb) {
                    vboxlog_msg("Status Report: Network Bytes Sent (Total): '%f'", bytes_sent);
                    vboxlog_msg("Status Report: Network Bytes Received (Total): '%f'", bytes_received);
                }

                pVM->dump_hypervisor_status_reports();
            }

            if (boinc_time_to_checkpoint()) {
                // Only peform a VM checkpoint every ten minutes or so.
                //
                if (elapsed_time >= last_checkpoint_elapsed_time + pVM->minimum_checkpoint_interval + random_checkpoint_factor) {
                    // Basic interleave factor is only needed once.
                    if (random_checkpoint_factor > 0) {
                        random_checkpoint_factor = 0.0;
                    }

                    // Checkpoint
                    retval = pVM->create_snapshot(elapsed_time);
                    if (retval) {
                        // Let BOINC clean-up the environment which should release any file/mutex locks and then attempt
                        // to resume from a previous snapshot.
                        //
                        vboxlog_msg("ERROR: Checkpoint maintenance failed, rescheduling task for a later time. (%d)", retval);
                        pVM->poweroff();
                        boinc_temporary_exit(86400, "VM job unmanageable, restarting later.");
                    } else {
                        // tell BOINC we've successfully created a checkpoint.
                        //
                        checkpoint.update(elapsed_time, current_cpu_time);
                        last_checkpoint_elapsed_time = elapsed_time;
                        last_checkpoint_cpu_time = current_cpu_time;
                        boinc_checkpoint_completed();
                    }
                }
            }

            // send elapsed-time trickle message if needed
            //
            if (trickle_period) {
                check_trickle_period(elapsed_time, trickle_period);
            }

            if (boinc_status.reread_init_data_file) {
                boinc_status.reread_init_data_file = false;

                vboxlog_msg("Preference change detected");

                boinc_parse_init_data_file();
                boinc_get_init_data_p(&aid);
                set_throttles(aid, *pVM);

                vboxlog_msg("Checkpoint Interval is now %d seconds.", (int)aid.checkpoint_period);
            }

            // if the VM has a maximum amount of time it is allowed to run,
            // shut it down gacefully and exit.
            //
            if (pVM->job_duration && (elapsed_time > pVM->job_duration)) {
                pVM->cleanup();

                if (pVM->enable_cern_dataformat) {
                    FILE* output = fopen("output", "w");
                    if (output) {
                        fprintf(
                            output,
                            "Work Unit completed!\n"
                        );
                        fclose(output);
                    }
                }

                boinc_finish(0);
            }
        }

        if (pVM->enable_network) {
            if (boinc_status.network_suspended) {
                if (!pVM->network_suspended) {
                    pVM->set_network_access(false);
                }
            } else {
                if (pVM->network_suspended) {
                    pVM->set_network_access(true);
                }
            }
        }

        // report network usage every 10 min so the client can enforce quota
        //
        if (aid.global_prefs.daily_xfer_limit_mb
            && pVM->enable_network
            && !pVM->suspended
        ) {
            net_usage_timer -= POLL_PERIOD;
            if (net_usage_timer <= 0) {
                net_usage_timer = 600;
                double sent, received;
                retval = pVM->get_vm_network_bytes_sent(sent);
                if (!retval && (sent != bytes_sent)) {
                    bytes_sent = sent;
                    report_net_usage = true;
                }
                retval = pVM->get_vm_network_bytes_received(received);
                if (!retval && (received != bytes_received)) {
                    bytes_received = received;
                    report_net_usage = true;
                }
            }
        }

        if (report_net_usage) {
            retval = boinc_report_app_status_aux(
                elapsed_time,
                last_checkpoint_cpu_time,
                fraction_done,
                pVM->vm_pid,
                bytes_sent,
                bytes_received
            );
            if (!retval) {
                report_net_usage = false;
            }
        }

        stopwatch_endtime = dtime();
        stopwatch_elapsedtime = stopwatch_endtime - stopwatch_starttime;

        // user may have changed system clock, so do sanity checks
        //
        if (stopwatch_elapsedtime < 0) {
            stopwatch_elapsedtime = 0;
        }
        if (stopwatch_elapsedtime > 60) {
            stopwatch_elapsedtime = 0;
        }

        // Sleep for the remainder of the polling period
        //
        sleep_time = POLL_PERIOD - stopwatch_elapsedtime;
        if (sleep_time > 0) {
            boinc_sleep(sleep_time);
        }

        // if VM is running, increment elapsed time
        //
        if (!boinc_status.suspended && !pVM->suspended) {
            if (sleep_time > 0) {
                elapsed_time += POLL_PERIOD;
            } else {
                elapsed_time += stopwatch_elapsedtime;
            }
        }
    }

#ifdef _WIN32
    CoUninitialize();
#ifdef USE_WINSOCK
    WSACleanup();
#endif
#endif

    return 0;
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
