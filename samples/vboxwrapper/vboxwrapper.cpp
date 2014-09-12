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
// --vmimage file   Use "file" as the VM image.
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
// Andrew J. Younge (ajy4490 AT umiacs DOT umd DOT edu)
// Jie Wu <jiewu AT cern DOT ch>
// Daniel Lombraña González <teleyinex AT gmail DOT com>
// Rom Walton
// David Anderson

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
#include <unistd.h>
#endif

#include "version.h"
#include "boinc_api.h"
#include "diagnostics.h"
#include "filesys.h"
#include "md5_file.h"
#include "parse.h"
#include "str_util.h"
#include "str_replace.h"
#include "util.h"
#include "error_numbers.h"
#include "procinfo.h"
#include "vboxwrapper.h"
#include "vbox.h"

using std::vector;
using std::string;

double elapsed_time = 0;
    // job's total elapsed time (over all sessions)
double trickle_period = 0;

bool is_boinc_client_version_newer(APP_INIT_DATA& aid, int maj, int min, int rel) {
    if (maj < aid.major_version) return true;
    if (maj > aid.major_version) return false;
    if (min < aid.minor_version) return true;
    if (min > aid.minor_version) return false;
    if (rel < aid.release) return true;
    return false;
}

char* vboxwrapper_msg_prefix(char* sbuf, int len) {
    char buf[256];
    struct tm tm;
    struct tm *tmp = &tm;
    int n;

    time_t x = time(0);
#ifdef _WIN32
#ifdef __MINGW32__
    if ((tmp = localtime(&x)) == NULL)
#else
    if (localtime_s(&tm, &x) == EINVAL)
#endif
#else
    if (localtime_r(&x, &tm) == NULL)
#endif
    {
        strcpy(sbuf, "localtime() failed");
        return sbuf;
    }
    if (strftime(buf, sizeof(buf)-1, "%Y-%m-%d %H:%M:%S", tmp) == 0) {
        strcpy(sbuf, "strftime() failed");
        return sbuf;
    }
#ifdef _WIN32
    n = _snprintf(sbuf, len, "%s (%d):", buf, GetCurrentProcessId());
#else
    n = snprintf(sbuf, len, "%s (%d):", buf, getpid());
#endif
    if (n < 0) {
        strcpy(sbuf, "sprintf() failed");
        return sbuf;
    }
    sbuf[len-1] = 0;    // just in case
    return sbuf;
}

int VBOX_VM::parse_port_forward(XML_PARSER& xp) {
    int host_port=0, guest_port=0, nports=1;
    bool is_remote;
    while (!xp.get_tag()) {
        if (xp.match_tag("/port_forward")) {
            if (!host_port) {
                fprintf(stderr, "parse_port_forward: unspecified host port\n");
                return ERR_XML_PARSE;
            }
            if (!guest_port) {
                fprintf(stderr, "parse_port_forward: unspecified guest port\n");
                return ERR_XML_PARSE;
            }
            PORT_FORWARD pf;
            pf.host_port = host_port;
            pf.guest_port = guest_port;
            pf.is_remote = is_remote;
            for (int i=0; i<nports; i++) {
                port_forwards.push_back(pf);
                pf.host_port++;
                pf.guest_port++;
            }
            return 0;
        }
        else if (xp.parse_bool("is_remote", is_remote)) continue;
        else if (xp.parse_int("host_port", host_port)) continue;
        else if (xp.parse_int("guest_port", guest_port)) continue;
        else if (xp.parse_int("nports", nports)) continue;
        else {
            fprintf(stderr, "parse_port_forward: unparsed %s\n", xp.parsed_tag);
        }
    }
    return ERR_XML_PARSE;
}

int parse_job_file(VBOX_VM& vm) {
    MIOFILE mf;
    string str;
    char buf[1024], buf2[256];

    boinc_resolve_filename(JOB_FILENAME, buf, sizeof(buf));
    FILE* f = boinc_fopen(buf, "r");
    if (!f) {
        fprintf(stderr,
            "%s can't open job file %s\n",
            vboxwrapper_msg_prefix(buf2, sizeof(buf2)), buf
        );
        return ERR_FOPEN;
    }
    mf.init_file(f);
    XML_PARSER xp(&mf);

    if (!xp.parse_start("vbox_job")) return ERR_XML_PARSE;
    while (!xp.get_tag()) {
        if (!xp.is_tag) {
            fprintf(stderr, "%s parse_job_file(): unexpected text %s\n",
                vboxwrapper_msg_prefix(buf, sizeof(buf)), xp.parsed_tag
            );
            continue;
        }
        if (xp.match_tag("/vbox_job")) {
            fclose(f);
            return 0;
        }
        else if (xp.parse_string("vm_disk_controller_type", vm.vm_disk_controller_type)) continue;
        else if (xp.parse_string("vm_disk_controller_model", vm.vm_disk_controller_model)) continue;
        else if (xp.parse_string("os_name", vm.os_name)) continue;
        else if (xp.parse_string("memory_size_mb", vm.memory_size_mb)) continue;
        else if (xp.parse_double("job_duration", vm.job_duration)) continue;
        else if (xp.parse_double("minimum_checkpoint_interval", vm.minimum_checkpoint_interval)) continue;
        else if (xp.parse_string("fraction_done_filename", vm.fraction_done_filename)) continue;
        else if (xp.parse_bool("enable_cern_dataformat", vm.enable_cern_dataformat)) continue;
        else if (xp.parse_bool("enable_network", vm.enable_network)) continue;
        else if (xp.parse_bool("network_bridged_mode", vm.network_bridged_mode)) continue;
        else if (xp.parse_bool("enable_shared_directory", vm.enable_shared_directory)) continue;
        else if (xp.parse_bool("enable_floppyio", vm.enable_floppyio)) continue;
        else if (xp.parse_bool("enable_cache_disk", vm.enable_cache_disk)) continue;
        else if (xp.parse_bool("enable_isocontextualization", vm.enable_isocontextualization)) continue;
        else if (xp.parse_bool("enable_remotedesktop", vm.enable_remotedesktop)) continue;
        else if (xp.parse_int("pf_guest_port", vm.pf_guest_port)) continue;
        else if (xp.parse_int("pf_host_port", vm.pf_host_port)) continue;
        else if (xp.parse_string("copy_to_shared", str)) {
            vm.copy_to_shared.push_back(str);
            continue;
        }
        else if (xp.parse_string("trickle_trigger_file", str)) {
            vm.trickle_trigger_files.push_back(str);
            continue;
        }
        else if (xp.parse_string("completion_trigger_file", str)) {
            vm.completion_trigger_file = str;
            continue;
        }
        else if (xp.match_tag("port_forward")) {
            vm.parse_port_forward(xp);
        }
        fprintf(stderr, "%s parse_job_file(): unexpected tag %s\n",
            vboxwrapper_msg_prefix(buf, sizeof(buf)), xp.parsed_tag
        );
    }
    fclose(f);
    return ERR_XML_PARSE;
}

void write_checkpoint(double elapsed, double cpu, VBOX_VM& vm) {
    FILE* f = fopen(CHECKPOINT_FILENAME, "w");
    if (!f) return;
    fprintf(f, "%f %f %d %d\n", elapsed, cpu, vm.pf_host_port, vm.rd_host_port);
    fclose(f);
}

void read_checkpoint(double& elapsed, double& cpu, VBOX_VM& vm) {
    double c;
    double e;
    int pf_host;
    int rd_host;
    elapsed = 0.0;
    cpu = 0.0;
    vm.pf_host_port = 0;
    vm.rd_host_port = 0;
    FILE* f = fopen(CHECKPOINT_FILENAME, "r");
    if (!f) return;
    int n = fscanf(f, "%lf %lf %d %d", &e, &c, &pf_host, &rd_host);
    fclose(f);
    if (n != 4) return;
    elapsed = e;
    cpu = c;
    vm.pf_host_port = pf_host;
    vm.rd_host_port = rd_host;
}

void read_fraction_done(double& frac_done, VBOX_VM& vm) {
    char buf[256];
    double temp, frac = 0;

    FILE* f = fopen(vm.fraction_done_filename.c_str(), "r");
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
                fprintf(stderr,
                    "%s can't write init_data.xml to floppy abstration device\n",
                    vboxwrapper_msg_prefix(buf, sizeof(buf))
                );
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
void VBOX_VM::set_web_graphics_url() {
    char buf[256];
    for (unsigned int i=0; i<port_forwards.size(); i++) {
        PORT_FORWARD& pf = port_forwards[i];
        if (pf.guest_port == pf_guest_port) {
            sprintf(buf, "http://localhost:%d", pf.host_port);
            boinc_web_graphics_url(buf);
            break;
        }
    }
}

// set remote desktop information if needed
//
void set_remote_desktop_info(APP_INIT_DATA& /* aid */, VBOX_VM& vm) {
    char buf[256];

    if (vm.rd_host_port) {
        // Write info to disk
        //
        MIOFILE mf;
        FILE* f = boinc_fopen(REMOTEDESKTOP_FILENAME, "w");
        mf.init_file(f);

        mf.printf(
            "<remote_desktop>\n"
            "  <host_port>%d</host_port>\n"
            "</remote_desktop>\n",
            vm.rd_host_port
        );

        fclose(f);

        sprintf(buf, "localhost:%d", vm.rd_host_port);
        boinc_remote_desktop_addr(buf);
    }
}

void extract_completion_file_info(VBOX_VM& vm, unsigned long& exit_code, string& message) {
    char path[MAXPATHLEN];
    char buf[1024];

    sprintf(path, "shared/%s", vm.completion_trigger_file.c_str());
    FILE* f = fopen(path, "r");
    if (f) {
        message = "";
        if (fgets(buf, 1024, f) != NULL) {
            exit_code = atoi(buf);
        }
        while (fgets(buf, 1024, f) != NULL) {
            message += buf;
        }
        fclose(f);
    }
}

// check for trickle trigger files, and send trickles if find them.
//
void VBOX_VM::check_trickle_triggers() {
    char filename[256], path[MAXPATHLEN], buf[256];
    for (unsigned int i=0; i<trickle_trigger_files.size(); i++) {
        strcpy(filename, trickle_trigger_files[i].c_str());
        sprintf(path, "shared/%s", filename);
        if (!boinc_file_exists(path)) continue;
        string text;
        int retval = read_file_string(path, text);
        if (retval) {
            fprintf(stderr,
                "%s can't read trickle trigger file %s\n",
                vboxwrapper_msg_prefix(buf, sizeof(buf)), filename
            );
        } else {
            retval = boinc_send_trickle_up(
                filename, const_cast<char*>(text.c_str())
            );
            if (retval) {
                fprintf(stderr,
                    "%s boinc_send_trickle_up() failed: %s\n",
                    vboxwrapper_msg_prefix(buf, sizeof(buf)), boincerror(retval)
                );
            } else {
                fprintf(stderr,
                    "%s sent trickle-up of variety %s\n",
                    vboxwrapper_msg_prefix(buf, sizeof(buf)), filename
                );
            }
        }
        boinc_delete_file(path);
    }
}

// see if it's time to send trickle-up reporting elapsed time
//
void check_trickle_period() {
    char buf[256];
    static double last_trickle_report_time = 0;

    if ((elapsed_time - last_trickle_report_time) < trickle_period) {
        return;
    }
    last_trickle_report_time = elapsed_time;
    fprintf(
        stderr,
        "%s Status Report: Trickle-Up Event.\n",
        vboxwrapper_msg_prefix(buf, sizeof(buf))
    );
    sprintf(buf,
        "<cpu_time>%f</cpu_time>", last_trickle_report_time
    );
    int retval = boinc_send_trickle_up(
        const_cast<char*>("cpu_time"), buf
    );
    if (retval) {
        fprintf(
            stderr,
            "%s Sending Trickle-Up Event failed (%d).\n",
            vboxwrapper_msg_prefix(buf, sizeof(buf)),
            retval
        );
    }
}

int main(int argc, char** argv) {
    int retval;
    int loop_iteration = 0;
    BOINC_OPTIONS boinc_options;
    VBOX_VM vm;
    APP_INIT_DATA aid;
    double random_checkpoint_factor = 0;
    double fraction_done = 0;
    double current_cpu_time = 0;
    double starting_cpu_time = 0;
    double last_checkpoint_time = 0;
    double last_status_report_time = 0;
    double stopwatch_starttime = 0;
    double stopwatch_endtime = 0;
    double stopwatch_elapsedtime = 0;
    double sleep_time = 0;
    double bytes_sent = 0;
    double bytes_received = 0;
    double ncpus = 0;
    double timeout = 0.0;
    bool report_net_usage = false;
    double net_usage_timer = 600;
	int vm_image = 0;
    unsigned long vm_exit_code = 0;
    string message;
    char buf[256];


    for (int i=1; i<argc; i++) {
        if (!strcmp(argv[i], "--trickle")) {
            trickle_period = atof(argv[++i]);
        }
        if (!strcmp(argv[i], "--nthreads")) {
            ncpus = atof(argv[++i]);
        }
        if (!strcmp(argv[i], "--vmimage")) {
            vm_image = atoi(argv[++i]);
        }
        if (!strcmp(argv[i], "--register_only")) {
            vm.register_only = true;
        }
    }

    memset(&boinc_options, 0, sizeof(boinc_options));
    boinc_options.main_program = true;
    boinc_options.check_heartbeat = true;
    boinc_options.handle_process_control = true;
    boinc_init_options(&boinc_options);

    // Prepare environment for detecting system conditions
    //
    boinc_get_init_data_p(&aid);

    // Log banner
    //
    fprintf(
        stderr,
        "%s vboxwrapper (%d.%d.%d): starting\n",
        vboxwrapper_msg_prefix(buf, sizeof(buf)),
        BOINC_MAJOR_VERSION,
        BOINC_MINOR_VERSION,
        VBOXWRAPPER_RELEASE
    );

    // Log important information
    //

    // Choose a random interleave value for checkpoint intervals to stagger disk I/O.
    // 
    struct stat vm_image_stat;
    if (-1 == stat(IMAGE_FILENAME_COMPLETE, &vm_image_stat)) {
        srand((int)time(NULL));
    } else {
        srand((int)(vm_image_stat.st_mtime * time(NULL)));
    }
    random_checkpoint_factor = (double)(((int)(drand() * 100000.0)) % 600);
    fprintf(
        stderr,
        "%s Feature: Checkpoint interval offset (%d seconds)\n",
        vboxwrapper_msg_prefix(buf, sizeof(buf)),
        (int)random_checkpoint_factor
    );

    // Display trickle value if specified
    //
    if (trickle_period > 0.0) {
        fprintf(
            stderr,
            "%s Feature: Enabling trickle-ups (Interval: %f)\n",
            vboxwrapper_msg_prefix(buf, sizeof(buf)), trickle_period
        );
    }

    // Initialize system services
    // 
#if defined(_WIN32) && defined(USE_WINSOCK)
    WSADATA wsdata;
    retval = WSAStartup( MAKEWORD( 1, 1 ), &wsdata);
    if (retval) {
        fprintf(
            stderr,
            "%s can't initialize winsock: %d\n",
            vboxwrapper_msg_prefix(buf, sizeof(buf)),
            retval
        );
        boinc_finish(retval);
    }
#endif

    // Check for architecture incompatibilities
    // 
#if defined(_WIN32) && defined(_M_IX86)
    if (strstr(aid.host_info.os_version, "x64")) {
        fprintf(
            stderr,
            "%s 64-bit version of BOINC is required, please upgrade. Rescheduling execution for a later date.\n",
            vboxwrapper_msg_prefix(buf, sizeof(buf))
        );
        boinc_temporary_exit(86400, "Architecture incompatibility detected.");
    }
#endif

    // Initialize VM Hypervisor
    //
    retval = vm.initialize();
    if (retval) {
        fprintf(
            stderr,
            "%s Could not detect VM Hypervisor. Rescheduling execution for a later date.\n",
            vboxwrapper_msg_prefix(buf, sizeof(buf))
        );
        boinc_temporary_exit(86400, "Detection of VM Hypervisor failed.");
    }

    // Record what version of VirtualBox was used.
    // 
    if (!vm.virtualbox_version.empty()) {
        fprintf(
            stderr,
            "%s Detected: VirtualBox %s\n",
            vboxwrapper_msg_prefix(buf, sizeof(buf)),
            vm.virtualbox_version.c_str()
        );
    }

    // Record if anonymous platform was used.
    // 
    if (boinc_file_exists((std::string(aid.project_dir) + std::string("/app_info.xml")).c_str())) {
        fprintf(
            stderr,
            "%s Detected: Anonymous Platform Enabled\n",
            vboxwrapper_msg_prefix(buf, sizeof(buf))
        );
    }

    // Record if the sandboxed configuration is going to be used.
    //
    if (aid.using_sandbox) {
        fprintf(
            stderr,
            "%s Detected: Sandbox Configuration Enabled\n",
            vboxwrapper_msg_prefix(buf, sizeof(buf))
        );
    }

    // Record which mode VirtualBox should be started in.
    //
    if (aid.vbox_window || boinc_is_standalone()) {
        fprintf(
            stderr,
            "%s Detected: Headless Mode Disabled\n",
            vboxwrapper_msg_prefix(buf, sizeof(buf))
        );
        vm.headless = false;
    }

    // Check for invalid confgiurations.
    //
    if (aid.using_sandbox && aid.vbox_window) {
        vboxwrapper_msg_prefix(buf, sizeof(buf));
        fprintf(
            stderr,
            "%s Invalid configuration detected.\n"
            "%s NOTE: BOINC cannot be installed as a service and run VirtualBox in headfull mode at the same time.\n",
            buf,
            buf
        );
        boinc_temporary_exit(86400, "Incompatible configuration detected.");
    }

    // Check against known incompatible versions of VirtualBox.  
    // VirtualBox 4.2.6 crashes during snapshot operations
    // and 4.2.18 fails to restore from snapshots properly.
    //
    if ((vm.virtualbox_version.find("4.2.6") != std::string::npos) || 
        (vm.virtualbox_version.find("4.2.18") != std::string::npos) || 
        (vm.virtualbox_version.find("4.3.0") != std::string::npos) ) {
        fprintf(
            stderr,
            "%s Incompatible version of VirtualBox detected. Please upgrade to a later version.\n",
            vboxwrapper_msg_prefix(buf, sizeof(buf))
        );
        boinc_temporary_exit(86400,
            "Incompatible version of VirtualBox detected; please upgrade.",
            true
        );
    }

    // Check to see if the system is in a state in which we expect to be able to run
    // VirtualBox successfully.  Sometimes the system is in a wierd state after a
    // reboot and the system needs a little bit of time.
    //
    if (!vm.is_system_ready(message)) {
        fprintf(
            stderr,
            "%s Could not communicate with VM Hypervisor. Rescheduling execution for a later date.\n",
            vboxwrapper_msg_prefix(buf, sizeof(buf))
        );
        boinc_temporary_exit(86400, message.c_str());
    }

    // Parse Job File
    //
    retval = parse_job_file(vm);
    if (retval) {
        fprintf(
            stderr,
            "%s can't parse job file: %d\n",
            vboxwrapper_msg_prefix(buf, sizeof(buf)),
            retval
        );
        boinc_finish(retval);
    }

    // Record which mode VirtualBox should be started in.
    //
    fprintf(
        stderr,
        "%s Detected: Minimum checkpoint interval (%f seconds)\n",
        vboxwrapper_msg_prefix(buf, sizeof(buf)),
        vm.minimum_checkpoint_interval
    );

    // Validate whatever configuration options we can
    //
    if (vm.enable_shared_directory) {
        if (boinc_file_exists("shared")) {
            if (!is_dir("shared")) {
                fprintf(
                    stderr,
                    "%s 'shared' exists but is not a directory.\n",
                    vboxwrapper_msg_prefix(buf, sizeof(buf))
                );
            }
        } else {
            retval = boinc_mkdir("shared");
            if (retval) {
                fprintf(stderr,
                    "%s couldn't created shared directory: %s.\n",
                    vboxwrapper_msg_prefix(buf, sizeof(buf)),
                    boincerror(retval)
                );
            }
        }
    }

    // Copy files to the shared directory
    //
    if (vm.enable_shared_directory && vm.copy_to_shared.size()) {
        for (vector<string>::iterator iter = vm.copy_to_shared.begin(); iter != vm.copy_to_shared.end(); iter++) {
            string source = *iter;
            string destination = string("shared/") + *iter;
            if (!boinc_file_exists(destination.c_str())) {
                if (!boinc_copy(source.c_str(), destination.c_str())) {
                    fprintf(stderr,
                        "%s successfully copied '%s' to the shared directory.\n",
                        vboxwrapper_msg_prefix(buf, sizeof(buf)),
                        source.c_str()
                    );
                } else {
                    fprintf(stderr,
                        "%s failed to copy '%s' to the shared directory.\n",
                        vboxwrapper_msg_prefix(buf, sizeof(buf)),
                        source.c_str()
                    );
                }
            }
        }
    }

    // Configure Instance specific VM Parameters
    //
    vm.vm_master_name = "boinc_";
    vm.image_filename = IMAGE_FILENAME_COMPLETE;
    if (boinc_is_standalone()) {
        vm.vm_master_name += "standalone";
        vm.vm_master_description = "standalone";
        if (vm.enable_floppyio) {
            sprintf(buf, "%s.%s",
                FLOPPY_IMAGE_FILENAME, FLOPPY_IMAGE_FILENAME_EXTENSION
            );
            vm.floppy_image_filename = buf;
        }
    } else {
        vm.vm_master_name += md5_string(std::string(aid.result_name)).substr(0, 16);
        vm.vm_master_description = aid.result_name;
		if (vm_image) {
            sprintf(buf, "%s_%d.%s",
                IMAGE_FILENAME, vm_image, IMAGE_FILENAME_EXTENSION
            );
            vm.image_filename = buf;
		}
        if (vm.enable_floppyio) {
            sprintf(buf, "%s_%d.%s",
                FLOPPY_IMAGE_FILENAME, aid.slot,
                FLOPPY_IMAGE_FILENAME_EXTENSION
            );
            vm.floppy_image_filename = buf;
        }
    }
    if (vm.enable_cache_disk) {
        vm.cache_disk_filename = CACHE_DISK_FILENAME;
    }
    if (vm.enable_isocontextualization) {
        vm.iso_image_filename = ISO_IMAGE_FILENAME;
    }
    if (aid.ncpus > 1.0 || ncpus > 1.0) {
        if (ncpus) {
            sprintf(buf, "%d", (int)ceil(ncpus));
        } else {
            sprintf(buf, "%d", (int)ceil(aid.ncpus));
        }
        vm.vm_cpu_count = buf;
    } else {
        vm.vm_cpu_count = "1";
    }

    if (aid.vbox_window && !aid.using_sandbox) {
        vm.headless = false;
    }

    // Restore from checkpoint
    //
    read_checkpoint(elapsed_time, current_cpu_time, vm);
    starting_cpu_time = current_cpu_time;
    last_checkpoint_time = current_cpu_time;

    // Should we even try to start things up?
    //
    if (vm.job_duration && (elapsed_time > vm.job_duration)) {
        return EXIT_TIME_LIMIT_EXCEEDED;
    }

    retval = vm.run((current_cpu_time > 0));
    if (retval) {
        // All 'failure to start' errors are unrecoverable by default
        bool   unrecoverable_error = true;
        bool   skip_cleanup = false;
        bool   do_dump_hypervisor_logs = false;
        string error_reason;
        const char*  temp_reason = "";
        int    temp_delay = 86400;

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
        } else if (vm.is_logged_failure_vm_extensions_disabled()) {
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
        } else if (vm.is_logged_failure_vm_extensions_not_supported()) {
            error_reason =
                "   NOTE: VirtualBox has reported an improperly configured virtual machine. It was configured to require\n"
                "    hardware acceleration for virtual machines, but your processor does not support the required feature.\n"
                "    Please report this issue to the project so that it can be addresssed.\n";
        } else if (vm.is_logged_failure_vm_extensions_in_use()) {
            error_reason =
                "   NOTE: VirtualBox hypervisor reports that another hypervisor has locked the hardware acceleration\n"
                "    for virtual machines feature in exclusive mode.\n";
            unrecoverable_error = false;
            temp_reason = "Forign VM Hypervisor locked hardware acceleration features.";
            temp_delay = 86400;
        } else if (vm.is_logged_failure_host_out_of_memory()) {
            error_reason =
                "   NOTE: VirtualBox has failed to allocate enough memory to start the configured virtual machine.\n"
                "    This might be a temporary problem and so this job will be rescheduled for another time.\n";
            unrecoverable_error = false;
            temp_reason = "VM Hypervisor was unable to allocate enough memory to start VM.";
        } else if (vm.is_virtualbox_error_recoverable(retval)) {
            error_reason =
                "   NOTE: VM session lock error encountered.\n"
                "    BOINC will be notified that it needs to clean up the environment.\n"
                "    This might be a temporary problem and so this job will be rescheduled for another time.\n";
            unrecoverable_error = false;
            temp_reason = "VM environment needed to be cleaned up.";
        } else {
            do_dump_hypervisor_logs = true;
        }

        if (unrecoverable_error) {
            // Attempt to cleanup the VM and exit.
            if (!skip_cleanup) {
                vm.cleanup();
            }
            write_checkpoint(elapsed_time, current_cpu_time, vm);

            if (error_reason.size()) {
                fprintf(
                    stderr,
                    "%s\n"
                    "%s",
                    vboxwrapper_msg_prefix(buf, sizeof(buf)),
                    error_reason.c_str()
                );
            }

            if (do_dump_hypervisor_logs) {
                vm.dump_hypervisor_logs(true);
            }

            boinc_finish(retval);
        } else {
            // if the VM is already running notify BOINC about the process ID so it can
            // clean up the environment.  We should be safe to run after that.
            //
            if (vm.vm_pid) {
                retval = boinc_report_app_status_aux(
                    current_cpu_time,
                    last_checkpoint_time,
                    fraction_done,
                    vm.vm_pid,
                    bytes_sent,
                    bytes_received
                );
            }
 
            // Give the BOINC API time to report the pid to BOINC.
            //
            boinc_sleep(5.0);

            if (error_reason.size()) {
                fprintf(
                    stderr,
                    "%s\n"
                    "%s",
                    vboxwrapper_msg_prefix(buf, sizeof(buf)),
                    error_reason.c_str()
                );
            }

            // Exit and let BOINC clean up the rest.
            //
            boinc_temporary_exit(temp_delay, temp_reason);
        }
    }

    // Report the VM pid to BOINC so BOINC can deal with it when needed.
    //
    vboxwrapper_msg_prefix(buf, sizeof(buf));
    fprintf(
        stderr,
        "%s Reporting VM Process ID to BOINC.\n",
        buf
    );
    retval = boinc_report_app_status_aux(
        current_cpu_time,
        last_checkpoint_time,
        fraction_done,
        vm.vm_pid,
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
        vm.poll(false);
        if (vm.online && !vm.restoring) break;
        boinc_sleep(1.0);
    } while (timeout >= dtime());

    // Lower the VM process priority after it has successfully brought itself online.
    //
    vm.lower_vm_process_priority();

    // Log our current state 
    vm.poll(true);

    // Did we timeout?
    if (timeout <= dtime()) {
        vboxwrapper_msg_prefix(buf, sizeof(buf));
        fprintf(
            stderr,
            "%s NOTE: VM failed to enter an online state within the timeout period.\n"
            "%s   This might be a temporary problem and so this job will be rescheduled for another time.\n",
            buf,
            buf
        );
        vm.reset_vm_process_priority();
        vm.poweroff();
        boinc_temporary_exit(86400,
            "VM Hypervisor failed to enter an online state in a timely fashion."
        );
    }

    set_floppy_image(aid, vm);
    //set_port_forwarding_info(aid, vm);
    vm.set_web_graphics_url();
    set_remote_desktop_info(aid, vm);
    write_checkpoint(elapsed_time, current_cpu_time, vm);

    // Force throttling on our first pass through the loop
    boinc_status.reread_init_data_file = true;

    while (1) {
        // Begin stopwatch timer
        stopwatch_starttime = dtime();
        loop_iteration += 1;

        // Discover the VM's current state
        vm.poll();

        if (boinc_status.no_heartbeat || boinc_status.quit_request) {
            vm.reset_vm_process_priority();
            vm.poweroff();
            boinc_temporary_exit(86400);
        }
        if (boinc_status.abort_request) {
            vm.reset_vm_process_priority();
            vm.cleanup();
            vm.dump_hypervisor_logs(true);
            boinc_finish(EXIT_ABORTED_BY_CLIENT);
        }
        if (vm.is_logged_completion_file_exists()) {
            vm.reset_vm_process_priority();
            vm.cleanup();
            fprintf(
                stderr,
                "%s VM Completion File Detected.\n",
                vboxwrapper_msg_prefix(buf, sizeof(buf))
            );
            extract_completion_file_info(vm, vm_exit_code, message);
            boinc_finish(vm_exit_code);
        }
        if (!vm.online) {
            // Is this a type of event we can recover from?
            if (vm.is_logged_failure_host_out_of_memory()) {
                vboxwrapper_msg_prefix(buf, sizeof(buf));
                fprintf(
                    stderr,
                    "%s NOTE: VirtualBox has failed to allocate enough memory to continue.\n"
                    "%s   This might be a temporary problem and so this job will be rescheduled for another time.\n",
                    buf,
                    buf
                );
                vm.reset_vm_process_priority();
                vm.poweroff();
                boinc_temporary_exit(86400, "VM Hypervisor was unable to allocate enough memory.");
            } else {
                vm.cleanup();
                if (vm.crashed || (elapsed_time < vm.job_duration)) {
                    fprintf(
                        stderr,
                        "%s VM Premature Shutdown Detected.\n",
                        vboxwrapper_msg_prefix(buf, sizeof(buf))
                    );
                    vm.dump_hypervisor_logs(true);
                    vm.get_vm_exit_code(vm_exit_code);
                    if (vm_exit_code) {
                        boinc_finish(vm_exit_code);
                    } else {
                        boinc_finish(EXIT_ABORTED_BY_CLIENT);
                    }
                } else {
                    fprintf(
                        stderr,
                        "%s Virtual machine exited.\n",
                        vboxwrapper_msg_prefix(buf, sizeof(buf))
                    );
                    vm.dump_hypervisor_logs(false);
                    boinc_finish(0);
                }
            }
        } else {
            // Check to see if the guest VM has any log messages that indicate that we need need
            // to take action.
            if (vm.is_logged_failure_guest_job_out_of_memory()) {
                fprintf(
                    stderr,
                    "%s ERROR: VM reports there is not enough memory to finish the task.\n",
                    vboxwrapper_msg_prefix(buf, sizeof(buf))
                );
                vm.reset_vm_process_priority();
                vm.dump_hypervisor_logs(true);
                vm.poweroff();
                boinc_finish(EXIT_OUT_OF_MEMORY);
            }
        }
        if (boinc_status.suspended) {
            if (!vm.suspended) {
                retval = vm.pause();
                if (retval && vm.is_virtualbox_error_recoverable(retval)) {
                    fprintf(
                        stderr,
                        "%s ERROR: VM task failed to pause, rescheduling task for a later time.\n",
                        vboxwrapper_msg_prefix(buf, sizeof(buf))
                    );
                    vm.poweroff();
                    boinc_temporary_exit(86400, "VM job unmanageable, restarting later.");
               }
            }
        } else {
            if (vm.suspended) {
                retval = vm.resume();
                if (retval && vm.is_virtualbox_error_recoverable(retval)) {
                    fprintf(
                        stderr,
                        "%s ERROR: VM task failed to resume, rescheduling task for a later time.\n",
                        vboxwrapper_msg_prefix(buf, sizeof(buf))
                    );
                    vm.poweroff();
                    boinc_temporary_exit(86400, "VM job unmanageable, restarting later.");
               }
            }

            // stuff to do every 10 secs (everything else is 1/sec)
            //
            if ((loop_iteration % 10) == 0) {
                current_cpu_time = starting_cpu_time + vm.get_vm_cpu_time();
                vm.check_trickle_triggers();
            }

            if (vm.job_duration) {
                fraction_done = elapsed_time / vm.job_duration;
            } else if (vm.fraction_done_filename.size() > 0) {
                read_fraction_done(fraction_done, vm);
            }
            if (fraction_done > 1.0) {
                fraction_done = 1.0;
            }
            boinc_report_app_status(
                current_cpu_time,
                last_checkpoint_time,
                fraction_done
            );

            // write status report to stderr at regular intervals
            //
            if ((elapsed_time - last_status_report_time) >= 6000.0) {
                last_status_report_time = elapsed_time;
                if (vm.job_duration) {
                    fprintf(
                        stderr,
                        "%s Status Report: Job Duration: '%f'\n",
                        vboxwrapper_msg_prefix(buf, sizeof(buf)),
                        vm.job_duration
                    );
                }
                if (elapsed_time) {
                    fprintf(
                        stderr,
                        "%s Status Report: Elapsed Time: '%f'\n",
                        vboxwrapper_msg_prefix(buf, sizeof(buf)),
                        elapsed_time
                    );
                }
                fprintf(
                    stderr,
                    "%s Status Report: CPU Time: '%f'\n",
                    vboxwrapper_msg_prefix(buf, sizeof(buf)),
                    current_cpu_time
                );
                if (aid.global_prefs.daily_xfer_limit_mb) {
                    fprintf(
                        stderr,
                        "%s Status Report: Network Bytes Sent (Total): '%f'\n",
                        vboxwrapper_msg_prefix(buf, sizeof(buf)),
                        bytes_sent
                    );
                    fprintf(
                        stderr,
                        "%s Status Report: Network Bytes Received (Total): '%f'\n",
                        vboxwrapper_msg_prefix(buf, sizeof(buf)),
                        bytes_received
                    );
                }

                vm.dump_hypervisor_status_reports();
            }

            if (boinc_time_to_checkpoint()) {
                // Only peform a VM checkpoint every ten minutes or so.
                //
                if (elapsed_time >= last_checkpoint_time + vm.minimum_checkpoint_interval + random_checkpoint_factor) {
                    // Basic interleave factor is only needed once.
                    if (random_checkpoint_factor > 0) {
                        random_checkpoint_factor = 0.0;
                    }

                    // Checkpoint
                    retval = vm.create_snapshot(elapsed_time);
                    if (retval) {
                        // Let BOINC clean-up the environment which should release any file/mutex locks and then attempt
                        // to resume from a previous snapshot.
                        //
                        fprintf(
                            stderr,
                            "%s ERROR: Checkpoint maintenance failed, rescheduling task for a later time. (%d)\n",
                            vboxwrapper_msg_prefix(buf, sizeof(buf)),
                            retval
                        );
                        vm.poweroff();
                        boinc_temporary_exit(86400, "VM job unmanageable, restarting later.");
                    } else {
                        // tell BOINC we've successfully created a checkpoint.
                        //
                        last_checkpoint_time = elapsed_time;
                        write_checkpoint(elapsed_time, current_cpu_time, vm);
                        boinc_checkpoint_completed();
                    }
                }
            }

            // send elapsed-time trickle message if needed
            //
            if (trickle_period) {
                check_trickle_period();
            }

            if (boinc_status.reread_init_data_file) {
                boinc_status.reread_init_data_file = false;

                fprintf(
                    stderr,
                    "%s Preference change detected\n",
                    vboxwrapper_msg_prefix(buf, sizeof(buf))
                );

                boinc_parse_init_data_file();
                boinc_get_init_data_p(&aid);
                set_throttles(aid, vm);

                fprintf(
                    stderr,
                    "%s Checkpoint Interval is now %d seconds.\n",
                    vboxwrapper_msg_prefix(buf, sizeof(buf)),
                    (int)aid.checkpoint_period
                );
            }

            // if the VM has a maximum amount of time it is allowed to run,
            // shut it down gacefully and exit.
            //
            if (vm.job_duration && (elapsed_time > vm.job_duration)) {
                vm.cleanup();

                if (vm.enable_cern_dataformat) {
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

        if (vm.enable_network) {
            if (boinc_status.network_suspended) {
                if (!vm.network_suspended) {
                    vm.set_network_access(false);
                }
            } else {
                if (vm.network_suspended) {
                    vm.set_network_access(true);
                }
            }
        }

        // report network usage every 10 min so the client can enforce quota
        //
        if (aid.global_prefs.daily_xfer_limit_mb
            && vm.enable_network
            && !vm.suspended
        ) {
            net_usage_timer -= POLL_PERIOD;
            if (net_usage_timer <= 0) {
                net_usage_timer = 600;
                double sent, received;
                retval = vm.get_vm_network_bytes_sent(sent);
                if (!retval && (sent != bytes_sent)) {
                    bytes_sent = sent;
                    report_net_usage = true;
                }
                retval = vm.get_vm_network_bytes_received(received);
                if (!retval && (received != bytes_received)) {
                    bytes_received = received;
                    report_net_usage = true;
                }
            }
        }

        if (report_net_usage) {
            retval = boinc_report_app_status_aux(
                elapsed_time,
                last_checkpoint_time,
                fraction_done,
                vm.vm_pid,
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
        if (!boinc_status.suspended && !vm.suspended) {
            if (sleep_time > 0) {
                elapsed_time += POLL_PERIOD;
            } else {
                elapsed_time += stopwatch_elapsedtime;
            }
        }
    }

#if defined(_WIN32) && defined(USE_WINSOCK)
    WSACleanup();
#endif
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
