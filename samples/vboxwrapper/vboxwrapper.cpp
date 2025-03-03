// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2010-2022 University of California
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
// see https://github.com/BOINC/boinc/wiki/VboxApps
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
// --memory_size_mb How much memory (in MB) to give the VM. Overrides the
//                  value in vbox_job.xml if its present.
// --sporadic       This is a sporadic app; negotiate with client

// Contributors:
// Rom Walton
// David Anderson
// Andrew J. Younge (ajy4490 AT umiacs DOT umd DOT edu)
// Jie Wu <jiewu AT cern DOT ch>
// Daniel Lombraña González <teleyinex AT gmail DOT com>
// Marius Millea <mariusmillea AT gmail DOT com>

#define RESTART_DELAY   300
    // if a VM operation (suspend, resume, snapshot) fails,
    // exit and restart after this delay.

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
#include "vbox_common.h"
#include "vbox_vboxmanage.h"

#include "vboxwrapper.h"

using std::vector;
using std::string;

APP_INIT_DATA aid;
string slot_dir_path;
string project_dir_path;
string shared_dir;
    // this is 'shared' if <enable_shared_directory/>,
    // or '.' (i.e. the slot directory) if <shared_slot_dir/>.

bool shared_file_exists(std::string& filename) {
    char path[MAXPATHLEN];
    if (filename.empty()) return false;
    snprintf(path, sizeof(path), "%s/%s", shared_dir.c_str(), filename.c_str());
    return boinc_file_exists(path);
}

void shared_delete_file(std::string& filename) {
    char path[MAXPATHLEN];
    snprintf(path, sizeof(path), "%s/%s", shared_dir.c_str(), filename.c_str());
    boinc_delete_file(path);
}

int shared_stat(std::string& filename, struct stat* stat_file) {
    char path[MAXPATHLEN];
    snprintf(path, sizeof(path), "%s/%s", shared_dir.c_str(), filename.c_str());
    return stat(path, stat_file);
}

bool read_fraction_done(double& frac_done, VBOX_VM& vm) {
    char path[MAXPATHLEN];
    char buf[256];
    double temp, frac = 0;

    snprintf(path, sizeof(path), "%s/%s",
        shared_dir.c_str(), vm.fraction_done_filename.c_str()
    );
    FILE* f = fopen(path, "r");
    if (!f) return false;

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
    return true;
}

void read_completion_file_info(
    unsigned long& exit_code, bool& is_notice, string& message, VBOX_VM& vm
) {
    char path[MAXPATHLEN];
    char buf[1024];

    exit_code = 0;
    message = "";

    snprintf(path, sizeof(path), "%s/%s",
        shared_dir.c_str(), vm.completion_trigger_file.c_str()
    );
    FILE* f = fopen(path, "r");
    if (f) {
        if (fgets(buf, sizeof(buf), f) != NULL) {
            exit_code = atoi(buf);
        }
        if (fgets(buf, sizeof(buf), f) != NULL) {
            is_notice = atoi(buf) != 0;
        }
        while (fgets(buf, sizeof(buf), f) != NULL) {
            message += buf;
        }
        fclose(f);
    }
}

void read_temporary_exit_file_info(
    int& temp_delay, bool& is_notice, string& message, VBOX_VM& vm
) {
    char path[MAXPATHLEN];
    char buf[1024];

    temp_delay = 0;
    message = "";

    snprintf(path, sizeof(path), "%s/%s",
        shared_dir.c_str(), vm.temporary_exit_trigger_file.c_str()
    );
    FILE* f = fopen(path, "r");
    if (f) {
        if (fgets(buf, sizeof(buf), f) != NULL) {
            temp_delay = atoi(buf);
        }
        if (fgets(buf, sizeof(buf), f) != NULL) {
            is_notice = atoi(buf) != 0;
        }
        while (fgets(buf, sizeof(buf), f) != NULL) {
            message += buf;
        }
        fclose(f);
    }
}

// set CPU and network throttling if needed
//
void set_throttles(VBOX_VM& vm) {
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
void set_floppy_image(VBOX_VM& vm) {
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

            snprintf(buf, sizeof(buf), "%d", aid.userid);
            scratch += "BOINC_USERID=" + string(buf) + "\n";

            snprintf(buf, sizeof(buf), "%d", aid.hostid);
            scratch += "BOINC_HOSTID=" + string(buf) + "\n";

            snprintf(buf, sizeof(buf), "%.17g", aid.user_total_credit);
            scratch += "BOINC_USER_TOTAL_CREDIT=" + string(buf) + "\n";

            snprintf(buf, sizeof(buf), "%.17g", aid.host_total_credit);
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
        snprintf(buf, sizeof(buf), "http://localhost:%d", vm.pf_host_port);
        vboxlog_msg("Detected: Web Application Enabled (%s)", buf);
        boinc_web_graphics_url(buf);
    }
}

// set remote desktop information if needed
//
void report_remote_desktop_info(VBOX_VM& vm) {
    char buf[256];
    if (vm.rd_host_port) {
        snprintf(buf, sizeof(buf), "localhost:%d", vm.rd_host_port);
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
        snprintf(path, sizeof(path), "%s/%s", shared_dir.c_str(), filename);
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
                vboxlog_msg(
                    "boinc_send_trickle_up() failed: %s (%d)",
                    boincerror(retval), retval
                );
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
        snprintf(path, sizeof(path), "%s/%s", shared_dir.c_str(), filename);
        if (!boinc_file_exists(path)) continue;
        if (!vm.intermediate_upload_files[i].reported && !vm.intermediate_upload_files[i].ignore) {
            vboxlog_msg(
                "Reporting an intermediate file. (%s)",
                vm.intermediate_upload_files[i].file.c_str()
            );
            retval = boinc_upload_file(vm.intermediate_upload_files[i].file);
            if (retval) {
                vboxlog_msg(
                    "boinc_upload_file() failed: %s", boincerror(retval)
                );
                vm.intermediate_upload_files[i].ignore = true;
            } else {
                vm.intermediate_upload_files[i].reported = true;
            }
        } else if (vm.intermediate_upload_files[i].reported && !vm.intermediate_upload_files[i].ignore) {
            retval = boinc_upload_status(vm.intermediate_upload_files[i].file);
            if (!retval) {
                vboxlog_msg(
                    "Intermediate file uploaded. (%s)",
                    vm.intermediate_upload_files[i].file.c_str()
                );
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
    snprintf(buf, sizeof(buf),
        "<cpu_time>%f</cpu_time>", last_trickle_report_time
    );
    int retval = boinc_send_trickle_up(const_cast<char*>("cpu_time"), buf);

    if (retval) {
        vboxlog_msg("Sending Trickle-Up Event failed (%d).", retval);
    }
}

void usage() {
    vboxlog_msg(
        "Options:\n"
        "   --trickle X\n"
        "   --nthreads N\n"
        "   --memory_size_mb X\n"
        "   --vmimage F\n"
        "   --register_only\n"
        "   --sporadic\n"
    );
    boinc_finish(EXIT_INIT_FAILURE);
}

int main(int argc, char** argv) {
    int retval = 0;
    int loop_iteration = 0;
    BOINC_OPTIONS boinc_options;
    VBOX_CHECKPOINT checkpoint;
    VBOX_VM* pVM = NULL;
    double desired_checkpoint_interval = 0;
    double random_checkpoint_factor = 0;
    double elapsed_time = 0;
    double fraction_done = 0;
    double trickle_period = 0;
    double total_cpu_time = 0;
        // job CPU time counting previous episodes as well
    double starting_cpu_time = 0;
    double last_heartbeat_elapsed_time = 0;
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
    bool initial_heartbeat_check = true;
    int backtoback_heartbeat_failure = 1;
    double net_usage_timer = 600;
    int vm_image = 0;
    unsigned long vm_exit_code = 0;
    bool is_notice = false;
    int temp_delay = 86400;
    time_t last_heartbeat_mod_time = 0;
    string message;
    char buf[256];
    char path[MAXPATHLEN];
    bool is_sporadic = false;
    bool register_only = false;

    // seed random numbers
    //
#ifdef _WIN32
    srand((unsigned int)GetCurrentProcessId());
#else
    srand((unsigned int)getpid());
#endif

    // Initialize diagnostics system
    //
    boinc_init_diagnostics(BOINC_DIAG_DEFAULTS);

    // Parse command line
    //
    for (int i=1; i<argc; i++) {
        if (!strcmp(argv[i], "--trickle")) {
            trickle_period = atof(argv[++i]);
        } else if (!strcmp(argv[i], "--nthreads")) {
            ncpus = atof(argv[++i]);
        } else if (!strcmp(argv[i], "--memory_size_mb")) {
            memory_size_mb = atof(argv[++i]);
        } else if (!strcmp(argv[i], "--vmimage")) {
            vm_image = atoi(argv[++i]);
        } else if (!strcmp(argv[i], "--register_only")) {
            register_only = true;
        } else if (!strcmp(argv[i], "--sporadic")) {
            is_sporadic = true;
        } else {
            vboxlog_msg("Unrecognized option %s", argv[i]);
            usage();
        }
    }

    // Configure BOINC Runtime System environment
    //
    memset(&boinc_options, 0, sizeof(boinc_options));
    boinc_options.main_program = true;
    boinc_options.check_heartbeat = true;
    boinc_options.handle_process_control = true;
    boinc_init_options(&boinc_options);

    // Log version
    //
    vboxlog_msg("vboxwrapper version %d", VBOXWRAPPER_RELEASE);

    // Initialize system services
    //
#ifdef _WIN32
#ifdef USE_WINSOCK
    WSADATA wsdata;
    retval = WSAStartup( MAKEWORD( 1, 1 ), &wsdata);
    if (retval) {
        vboxlog_msg("ERROR: Cannot initialize winsock: %d", retval);
        boinc_finish(retval);
    }
#endif
#endif

    // initialize globals

    boinc_parse_init_data_file();
    boinc_get_init_data(aid);
    if (boinc_is_standalone()) {
        project_dir_path = "project";
    } else {
        project_dir_path = aid.project_dir;
    }
    boinc_getcwd(path);
    slot_dir_path = path;

    vboxlog_msg("BOINC client version: %d.%d.%d",
        aid.major_version, aid.minor_version, aid.release
    );

    // Initialize VM Hypervisor
    //
    pVM = (VBOX_VM*) new VBOX_VM();
    retval = pVM->initialize();
    if (retval) {
        vboxlog_msg("ERROR: VM initialization returned %d", retval);
        // don't postpone the task but rather just fail it.
        // If the hypervisor does not initialize correctly
        // the configuration is probably wrong
        // and it will just keep failing to initialize.
        //
        boinc_finish(retval);
    }
    pVM->register_only= register_only;

    // Display trickle value if specified
    //
    if (trickle_period > 0.0) {
        vboxlog_msg(
            "Feature: Enabling trickle-ups (Interval: %f)", trickle_period
        );
    }

    // Check for architecture incompatibilities
    //
#if defined(_WIN32) && defined(_M_IX86)
    if (strstr(aid.host_info.os_version, "x64")) {
        vboxlog_msg("64-bit version of BOINC is required, please upgrade. Rescheduling execution for a later date.");
        boinc_temporary_exit(86400, "Architecture incompatibility detected.");
    }
#endif

    // Record what version of VirtualBox was used.
    //
    if (!pVM->virtualbox_version_display.empty()) {
        vboxlog_msg("Detected: %s", pVM->virtualbox_version_display.c_str());
    }

    // Record if anonymous platform was used.
    //
    if (boinc_file_exists((project_dir_path + string("/app_info.xml")).c_str())) {
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

    // Check for invalid configurations.
    //
    if (aid.using_sandbox && aid.vbox_window) {
        vboxlog_msg("Invalid configuration detected.");
        vboxlog_msg("NOTE: BOINC cannot be installed as a service and run VirtualBox in headful mode at the same time.");
        boinc_temporary_exit(86400, "Incompatible configuration detected.");
    }

    // Check against known incompatible versions of VirtualBox.
    // VirtualBox 4.2.6 crashes during snapshot operations
    // and 4.2.18 fails to restore from snapshots properly.
    //

    if ((pVM->virtualbox_version_raw.find("4.2.6") != std::string::npos)
        || (pVM->virtualbox_version_raw.find("4.2.18") != std::string::npos)
        || (pVM->virtualbox_version_raw.find("4.3.0") != std::string::npos)
    ) {
        vboxlog_msg("Incompatible version of VirtualBox detected. Please upgrade to a later version.");
        boinc_temporary_exit(86400,
            "Incompatible version of VirtualBox detected; please upgrade.",
            true
        );
    }

    // Check to see if the system is in a state in which
    // we expect to be able to run VirtualBox successfully.
    // Sometimes the system is in a weird state after a
    // reboot and the system needs a little bit of time.
    //
    if (!pVM->is_system_ready(message)) {
        vboxlog_msg("ERROR: VBoxManage list hostinfo failed");
        boinc_finish(1);
    }

    // parse vbox_job.xml
    //
    retval = pVM->parse();
    if (retval) {
        vboxlog_msg("ERROR: Cannot parse vbox_job.xml: %d", retval);
        boinc_finish(retval);
    }

    // check for illegal config combinations
    //
    if (pVM->share_slot_dir) {
        if (pVM->enable_shared_directory) {
            vboxlog_msg("ERROR: can't use both <enable_shared_directory> and <share_slot_dir> in vbox_job.xml");
            boinc_finish(EXIT_INIT_FAILURE);
        }
        if (!pVM->copy_to_shared.empty()) {
            vboxlog_msg("ERROR: can't use both <copy_to_shared> and <share_slot_dir> in vbox_job.xml");
            boinc_finish(EXIT_INIT_FAILURE);
        }
    }
    if (!pVM->copy_to_shared.empty() && !pVM->enable_shared_directory) {
        vboxlog_msg("ERROR: <copy_to_shared> requires <enable_shared_directory> in vbox_job.xml");
        boinc_finish(EXIT_INIT_FAILURE);
    }
    if (pVM->share_project_dir) {
        if (pVM->enable_scratch_directory) {
            vboxlog_msg("ERROR: can't use both <enable_scratchdirectory> and <share_project_dir> in vbox_job.xml");
            boinc_finish(EXIT_INIT_FAILURE);
        }
    }

    // log heartbeat info
    //
    if (pVM->heartbeat_filename.size()) {
        vboxlog_msg("Detected: Heartbeat check (file: '%s' every %f seconds)",
            pVM->heartbeat_filename.c_str(), pVM->minimum_heartbeat_interval
        );
    }

    // create shared dirs as needed
    //
    if (pVM->enable_shared_directory) {
        if (boinc_file_exists("shared")) {
            if (!is_dir("shared")) {
                vboxlog_msg("ERROR: 'shared' exists but is not a directory.");
            }
        } else {
            retval = boinc_mkdir("shared");
            if (retval) {
                vboxlog_msg("ERROR: couldn't create shared directory: %s.", boincerror(retval));
            }
        }
    }
    if (pVM->enable_scratch_directory) {
        snprintf(path, sizeof(path), "%s/scratch", project_dir_path.c_str());
        if (boinc_file_exists(path)) {
            if (!is_dir(path)) {
                vboxlog_msg("ERROR: 'scratch' exists but is not a directory.");
            }
        } else {
            retval = boinc_mkdir(path);
            if (retval) {
                vboxlog_msg("ERROR: couldn't create scratch directory: %s.", boincerror(retval));
            }
        }
    }

    if (pVM->enable_shared_directory) {
        shared_dir = "shared";
    }
    if (pVM->share_slot_dir) {
        shared_dir = ".";
    }

    if (is_sporadic) {
        retval = boinc_sporadic_dir(shared_dir.c_str());
        if (retval) {
            vboxlog_msg("ERROR: couldn't create sporadic files: %s.", boincerror(retval));
            exit(1);
        }
    }

    // Copy files to the shared directory
    //
    for (long unsigned int i=0; i<pVM->copy_to_shared.size(); ++i) {
        string source = pVM->copy_to_shared[i];
        string destination = string("shared/") + source;
        if (!boinc_file_exists(destination.c_str())) {
            if (!boinc_copy(source.c_str(), destination.c_str())) {
                vboxlog_msg("Successfully copied '%s' to the shared directory.", source.c_str());
            } else {
                vboxlog_msg("Failed to copy '%s' to the shared directory.", source.c_str());
            }
        }
    }

    if (pVM->copy_cmdline_to_shared) {
        snprintf(path, sizeof(path), "%s/%s", shared_dir.c_str(), "cmdline");
        FILE* f = fopen(path, "wb");
        if (!f) {
            vboxlog_msg("Couldn't create shared/cmdline");
        } else {
            for (int i=1; i<argc; i++) {
                fprintf(f, "%s ", argv[i]);
            }
            fclose(f);
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
            snprintf(buf, sizeof(buf), "%s.%s",
                FLOPPY_IMAGE_FILENAME, FLOPPY_IMAGE_FILENAME_EXTENSION
            );
            pVM->floppy_image_filename = buf;
        }
    } else {
        // make a VM name based on result name
        //
        pVM->vm_master_name += md5_string(std::string(aid.result_name)).substr(0, 16);
        pVM->vm_master_description = aid.result_name;
        if (vm_image) {
            snprintf(buf, sizeof(buf), "%s_%d.%s",
                IMAGE_FILENAME, vm_image, IMAGE_FILENAME_EXTENSION
            );
            pVM->image_filename = buf;
        }
        if (pVM->enable_floppyio) {
            snprintf(buf, sizeof(buf), "%s_%d.%s",
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

    // cpu count: cmdline arg overrides config file
    //
    if (aid.ncpus > 1.0 || ncpus > 1.0) {
        if (ncpus > 32.0) {
            vboxlog_msg("WARNING: Virtualbox only allows up to 32 processors to be allocated to a VM, resetting to 32.  (%f allocated)", ncpus);
            ncpus = 32.0;
        }
        if (ncpus) {
            snprintf(buf, sizeof(buf), "%d", (int)ceil(ncpus));
        } else {
            snprintf(buf, sizeof(buf), "%d", (int)ceil(aid.ncpus));
        }
        pVM->vm_cpu_count = buf;
    } else {
        pVM->vm_cpu_count = "1";
    }

    // memory size: cmdline arg overrides config file
    //
    if (memory_size_mb) {
        pVM->memory_size_mb = memory_size_mb;
    }

    if (pVM->memory_size_mb < MIN_MEMORY_SIZE_MB) {
        vboxlog_msg("Memory size %dMB is too small; setting to %dMB",
            pVM->memory_size_mb, MIN_MEMORY_SIZE_MB
        );
        pVM->memory_size_mb = MIN_MEMORY_SIZE_MB;
    }

    if (aid.vbox_window && !aid.using_sandbox) {
        pVM->headless = false;
    }

    // Restore from checkpoint
    //
    checkpoint.parse();
    pVM->pf_host_port = checkpoint.webapi_port;
    pVM->rd_host_port = checkpoint.remote_desktop_port;
    elapsed_time = checkpoint.elapsed_time;
    starting_cpu_time = checkpoint.cpu_time;
    total_cpu_time = starting_cpu_time;
    last_checkpoint_elapsed_time = elapsed_time;
    last_heartbeat_elapsed_time = elapsed_time;
    last_checkpoint_cpu_time = starting_cpu_time;
    initial_heartbeat_check = true;


    // Choose a random interleave value for checkpoint intervals
    // to stagger disk I/O.
    //
    if (!pVM->disable_automatic_checkpoints) {
        random_checkpoint_factor = drand() * 600;

        vboxlog_msg(
            "Feature: Checkpoint interval offset (%d seconds)",
            (int)random_checkpoint_factor
        );

        // Record what the minimum checkpoint interval is.
        //
        vboxlog_msg(
            "Detected: Minimum checkpoint interval (%f seconds)",
            pVM->minimum_checkpoint_interval
        );
    }

    // Should we even try to start things up?
    //
    if (pVM->job_duration && (elapsed_time > pVM->job_duration)) {
        return EXIT_TIME_LIMIT_EXCEEDED;
    }

    retval = pVM->run(total_cpu_time > 0);
    if (retval) {
        // All 'failure to start' errors are unrecoverable by default
        vboxlog_msg("ERROR: VM failed to start");
        bool   unrecoverable_error = true;
        bool   skip_cleanup = false;
        bool   do_dump_hypervisor_logs = false;
        string error_reason;
        string  temp_reason = "";

        if (VBOXWRAPPER_ERR_RECOVERABLE == retval) {
            error_reason = pVM->get_error(ENV_UNCLEAN);
            unrecoverable_error = false;
            temp_reason = pVM->get_error(VM_ENV);
        }
        else if (ERR_NOT_EXITED == retval) {
            error_reason = pVM->get_error(VM_RUNNING);
            unrecoverable_error = false;
            temp_reason = pVM->get_error(VM_ENV);
        }
        else if (ERR_INVALID_PARAM == retval) {
            error_reason = pVM->get_error(NO_HA);
            skip_cleanup = true;
            retval = ERR_EXEC;
        }
        else if (retval == (int)RPC_S_SERVER_UNAVAILABLE) {
            error_reason = pVM->get_error(VBOX_SNAPSHOT);
            skip_cleanup = true;
            retval = ERR_EXEC;
        }
        else if (retval == (int)VBOX_E_INVALID_OBJECT_STATE) {
            error_reason = pVM->get_error(SESSION_LOCK);
            unrecoverable_error = false;
            temp_reason = pVM->get_error(VM_ENV);
        }
        else {
            do_dump_hypervisor_logs = true;
        }

        if (unrecoverable_error) {
            if (pVM->online) pVM->capture_screenshot();

            checkpoint.update(elapsed_time, total_cpu_time);

        }

        pVM->report_clean(
            unrecoverable_error, skip_cleanup, do_dump_hypervisor_logs,
            retval, error_reason, temp_delay, temp_reason,
            total_cpu_time, last_checkpoint_cpu_time, fraction_done,
            bytes_sent, bytes_received
        );
    }

    // Report the VM pid to BOINC so BOINC can deal with it when needed.
    //
    vboxlog_msg("Reporting VM Process ID to BOINC.");
    retval = boinc_report_app_status_aux(
        total_cpu_time,
        last_checkpoint_cpu_time,
        fraction_done,
        pVM->vm_pid,
        bytes_sent,
        bytes_received,
        0
    );

    // Wait for up to 5 minutes for the VM to switch states.
    // A system under load can take a while.
    // Since the poll function can wait for up to 60 seconds
    // to execute a command we need to make this time based instead
    // of iteration based.
    //
    timeout = dtime() + 300;
    do {
        pVM->poll(true);
        if (pVM->online && !pVM->restoring) break;
        boinc_sleep(1.0);
    } while (timeout >= dtime());

    // Lower the VM process priority after it has successfully brought itself online.
    //
    pVM->lower_vm_process_priority();

    // Is the VM still running? If not, why not?
    //
    if (!pVM->online) {
        // All 'failure to start' errors are unrecoverable by default
        bool   unrecoverable_error = true;
        bool   skip_cleanup = false;
        bool   do_dump_hypervisor_logs = false;
        string error_reason;
        string temp_reason = "";

        if (pVM->is_logged_failure_vm_extensions_disabled()) {
            error_reason = pVM->get_error(HA_OFF);
            retval = ERR_EXEC;
        } else if (pVM->is_logged_failure_vm_extensions_not_supported()) {
            error_reason = pVM->get_error(NO_HA);
        } else if (pVM->is_logged_failure_vm_extensions_in_use()) {
            error_reason = pVM->get_error(LOCKED_HA);
            unrecoverable_error = false;
            temp_reason = pVM->get_error(FOREIGN_HYPERV);
            temp_delay = 86400;
        } else if (pVM->is_logged_failure_host_out_of_memory()) {
            error_reason = pVM->get_error(NO_MEM);
            unrecoverable_error = false;
            temp_reason = pVM->get_error(TEMP_NO_MEM);
        } else if (timeout <= dtime()) {
            error_reason = pVM->get_error(NOT_ONLINE);
            unrecoverable_error = false;
            do_dump_hypervisor_logs = true;
            temp_reason = pVM->get_error(NO_ONLINE);
            temp_delay = 86400;
        }

        if (unrecoverable_error) {
            checkpoint.update(elapsed_time, total_cpu_time);
        }

        pVM->report_clean(
            unrecoverable_error, skip_cleanup, do_dump_hypervisor_logs,
            retval, error_reason, temp_delay, temp_reason,
            total_cpu_time, last_checkpoint_cpu_time, fraction_done,
            bytes_sent, bytes_received
        );
    }

    set_floppy_image(*pVM);
    report_web_graphics_url(*pVM);
    report_remote_desktop_info(*pVM);
    checkpoint.webapi_port = pVM->pf_host_port;
    checkpoint.remote_desktop_port = pVM->rd_host_port;
    checkpoint.update(elapsed_time, total_cpu_time);

    // Force throttling on our first pass through the loop
    boinc_status.reread_init_data_file = true;

    int poll_iteration = 0;

    while (1) {
        // Begin stopwatch timer
        stopwatch_starttime = dtime();
        loop_iteration += 1;

        // Discover the VM's current state
        retval = pVM->poll();

        if (retval) {
            vboxlog_msg("WARNING: Vboxwrapper poll command returned %d.", (retval));
            poll_iteration += 1;

            if (poll_iteration > 600){
                vboxlog_msg("ERROR: Vboxwrapper poll command failed too often. ");
                pVM->reset_vm_process_priority();
                pVM->capture_screenshot();
                pVM->cleanup();
                pVM->dump_hypervisor_logs(true);
                boinc_finish(EXIT_ABORTED_BY_CLIENT);
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
                }
                else {
                    elapsed_time += stopwatch_elapsedtime;
                }
            }

            continue;
        }

        poll_iteration = 0;

        // Write updates for the graphics application's use
        if (pVM->enable_graphics_support) {
            boinc_write_graphics_status(
                total_cpu_time, elapsed_time, fraction_done
            );
        }

        if (boinc_status.no_heartbeat || boinc_status.quit_request) {
            pVM->reset_vm_process_priority();
            if (pVM->enable_vm_savestate_usage) {
                retval = pVM->create_snapshot(elapsed_time);
                if (!retval) {
                    checkpoint.update(elapsed_time, total_cpu_time);
                    boinc_checkpoint_completed();
                }
                pVM->stop();
            } else {
                pVM->poweroff();
            }
            boinc_temporary_exit(86400);
        }
        if (boinc_status.abort_request) {
            pVM->reset_vm_process_priority();
            pVM->capture_screenshot();
            pVM->cleanup();
            pVM->dump_hypervisor_logs(true);
            boinc_finish(EXIT_ABORTED_BY_CLIENT);
        }
        if (pVM->heartbeat_filename.size()) {
            int backtoback = 3;

            if (elapsed_time >= (last_heartbeat_elapsed_time + pVM->minimum_heartbeat_interval/backtoback)) {

                int return_code = BOINC_SUCCESS;
                bool should_exit = false;
                struct stat heartbeat_stat;

                if (!shared_file_exists(pVM->heartbeat_filename)) {
                    vboxlog_msg("VM Heartbeat file specified, but missing.");
                    return_code = ERR_FILE_MISSING;
                    should_exit = true;
                } else {

                    if (shared_stat(pVM->heartbeat_filename, &heartbeat_stat)) {
                        // Error
                        vboxlog_msg("VM Heartbeat file specified, but missing file system status. (errno = '%d')", errno);
                        return_code = ERR_STAT;
                        should_exit = true;
                    } else {

                        if (initial_heartbeat_check) {
                            // Force the next check to be successful
                            last_heartbeat_mod_time = heartbeat_stat.st_mtime - 1;
                        }

                        if (heartbeat_stat.st_mtime > last_heartbeat_mod_time) {
                            // Heartbeat successful
                            backtoback_heartbeat_failure = 1;
                        } else {
                            // Timestamps are not always monotonuous
                            // or in sync between guest and host
                            // e.g. after a suspend/resume
                            // or when local time switches between
                            // normal time and DST
                            //
                            // Instead of fiddling around with complex checks
                            // simply count backtoback inconsistencies
                            //
                            if (backtoback_heartbeat_failure < backtoback) {
                                backtoback_heartbeat_failure++;
                            } else {
                                vboxlog_msg("VM Heartbeat file specified, but missing heartbeat.");
                                return_code = ERR_TIMEOUT;
                                should_exit = true;
                            }
                        }
                    }
                }

                if (should_exit) {
                    pVM->reset_vm_process_priority();
                    pVM->capture_screenshot();
                    pVM->cleanup();
                    pVM->dump_hypervisor_logs(true);
                    boinc_finish(return_code);
                }

                last_heartbeat_mod_time = heartbeat_stat.st_mtime;
                last_heartbeat_elapsed_time = elapsed_time;
                initial_heartbeat_check = false;
            }
        }
        if (shared_file_exists(pVM->completion_trigger_file)) {
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
        if (shared_file_exists(pVM->temporary_exit_trigger_file)) {
            vboxlog_msg("VM Temporary Exit File Detected.");
            read_temporary_exit_file_info(temp_delay, is_notice, message, *pVM);
            if (message.size()) {
                vboxlog_msg("VM Temporary Exit Message: %s.", message.c_str());
            }
            shared_delete_file(pVM->temporary_exit_trigger_file);
            pVM->reset_vm_process_priority();
            retval = pVM->create_snapshot(elapsed_time);
            if (!retval) {
                checkpoint.update(elapsed_time, total_cpu_time);
                boinc_checkpoint_completed();
            }
            pVM->poweroff();
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
                if ((unsigned)retval == VBOX_E_INVALID_OBJECT_STATE) {
                    vboxlog_msg("ERROR: VM task failed to pause, rescheduling task for a later time.");
                    pVM->poweroff();
                    snprintf(buf, sizeof(buf),
                        "VM suspend failed. Will exit and restart in %d sec.",
                        RESTART_DELAY
                    );
                    boinc_temporary_exit(RESTART_DELAY, buf);
                }
            }
        } else {
            if (pVM->suspended) {
                retval = pVM->resume();
                if ((unsigned)retval == VBOX_E_INVALID_OBJECT_STATE) {
                    vboxlog_msg("ERROR: VM task failed to resume, rescheduling task for a later time.");
                    pVM->poweroff();
                    snprintf(buf, sizeof(buf),
                        "VM resume failed. Will exit and restart in %d sec.",
                        RESTART_DELAY
                    );
                    boinc_temporary_exit(RESTART_DELAY, buf);
                }
            }

            // stuff to do every 10 secs (everything else is 1/sec)
            //
            if ((loop_iteration % 10) == 0) {
                total_cpu_time = starting_cpu_time + pVM->get_vm_cpu_time();
                check_trickle_triggers(*pVM);
                check_intermediate_uploads(*pVM);
            }

            if (pVM->job_duration) {
                fraction_done = elapsed_time / pVM->job_duration;
            } else if (pVM->fraction_done_filename.size() > 0) {
                if (!read_fraction_done(fraction_done, *pVM)) {
                    // Report a non-zero fraction done so that BOINC will not attempt to use CPU Time and
                    // deadline as a means to calculate fraction done when a fraction done file is
                    // specified.
                    //
                    fraction_done = 0.001;
                }
            }
            if (fraction_done > 1.0) {
                fraction_done = 1.0;
            }
            boinc_report_app_status(
                total_cpu_time,
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
                vboxlog_msg("Status Report: CPU Time: '%f'", total_cpu_time);
                if (aid.global_prefs.daily_xfer_limit_mb) {
                    vboxlog_msg("Status Report: Network Bytes Sent (Total): '%f'", bytes_sent);
                    vboxlog_msg("Status Report: Network Bytes Received (Total): '%f'", bytes_received);
                }

                pVM->dump_hypervisor_status_reports();
            }

            // Real VM checkpoints (snapshots) are expensive,
            // don't do them very often.
            //
            // If the project has disabled automatic checkpoints,
            // just report that we have successfully completed the checkpoint
            // as soon as the API reports that we should checkpoint.
            //
            if (boinc_time_to_checkpoint()) {
                if ((elapsed_time >= last_checkpoint_elapsed_time + desired_checkpoint_interval + random_checkpoint_factor)
                    || pVM->disable_automatic_checkpoints
                ) {
                    // Basic interleave factor is only needed once.
                    if (random_checkpoint_factor > 0) {
                        random_checkpoint_factor = 0.0;
                    }

                    // Checkpoint
                    retval = pVM->create_snapshot(elapsed_time);
                    if (retval) {
                        // Let BOINC clean up the environment which should
                        // release any file/mutex locks and then attempt
                        // to resume from a previous snapshot.
                        //
                        vboxlog_msg("ERROR: Checkpoint maintenance failed, rescheduling task for a later time. (%d)", retval);
                        pVM->poweroff();
                        snprintf(buf, sizeof(buf),
                            "VM snapshot failed. Will exit and restart in %d sec.",
                            RESTART_DELAY
                        );
                        boinc_temporary_exit(RESTART_DELAY, buf);
                    } else {
                        // tell BOINC we've successfully created a checkpoint.
                        //
                        checkpoint.update(elapsed_time, total_cpu_time);
                        last_checkpoint_elapsed_time = elapsed_time;
                        last_checkpoint_cpu_time = total_cpu_time;
                        boinc_checkpoint_completed();
                    }
                }
            }

            // Send elapsed-time trickle message if needed
            //
            if (trickle_period) {
                check_trickle_period(elapsed_time, trickle_period);
            }

            // Changes detected, re-read preferences
            //
            if (boinc_status.reread_init_data_file) {
                boinc_status.reread_init_data_file = false;

                vboxlog_msg("Preference change detected");

                boinc_parse_init_data_file();
                boinc_get_init_data(aid);
                set_throttles(*pVM);

                desired_checkpoint_interval = aid.checkpoint_period;
                if (pVM->minimum_checkpoint_interval > aid.checkpoint_period) {
                    desired_checkpoint_interval = pVM->minimum_checkpoint_interval;
                }

                vboxlog_msg(
                    "Setting checkpoint interval to %d seconds. (Higher value of (Preference: %d seconds) or (Vbox_job.xml: %d seconds))",
                    (int)desired_checkpoint_interval,
                    (int)aid.checkpoint_period,
                    (int)pVM->minimum_checkpoint_interval
                );
            }

            // if the VM has a maximum amount of time it is allowed to run,
            // shut it down gacefully and exit.
            //
            if (pVM->job_duration && (elapsed_time > pVM->job_duration)) {
                pVM->cleanup();

                if (pVM->enable_cern_dataformat) {
                    FILE* output = fopen("output", "w");
                    if (output) {
                        fprintf(output, "Work Unit completed!\n");
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
                bytes_received,
                0
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
#ifdef USE_WINSOCK
    WSACleanup();
#endif
#endif

    return 0;
}
