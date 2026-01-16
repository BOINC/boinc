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

#ifdef _WIN32
#include "boinc_win.h"
#include "win_util.h"

#if defined(_MSC_VER)
#define getcwd      _getcwd
#define stricmp     _stricmp
#endif

#else
#include <algorithm>
#include <sstream>
#include <fstream>
#include <string>
#endif

using std::string;

#include "diagnostics.h"
#include "filesys.h"
#include "parse.h"
#include "str_util.h"
#include "str_replace.h"
#include "base64.h"
#include "md5_file.h"
#include "util.h"
#include "error_numbers.h"
#include "procinfo.h"
#include "network.h"
#include "boinc_api.h"

#include "floppyio.h"
#include "vboxlogging.h"
#include "vboxwrapper.h"
#include "vbox_vboxmanage.h"

bool is_boinc_client_version_newer(int maj, int min, int rel) {
    if (maj < aid.major_version) return true;
    if (maj > aid.major_version) return false;
    if (min < aid.minor_version) return true;
    if (min > aid.minor_version) return false;
    if (rel < aid.release) return true;
    return false;
}

// t1 > t2
static bool is_timestamp_newer(VBOX_TIMESTAMP& t1, VBOX_TIMESTAMP& t2) {
    if (t1.hours > t2.hours) return true;
    if (t1.hours < t2.hours) return false;
    if (t1.minutes > t2.minutes) return true;
    if (t1.minutes < t2.minutes) return false;
    if (t1.seconds > t2.seconds) return true;
    if (t1.seconds < t2.seconds) return false;
    if (t1.milliseconds > t2.milliseconds) return true;
    return false;
}

VBOX_BASE::VBOX_BASE() : VBOX_JOB() {
    VBOX_JOB::clear();
    virtualbox_profile_directory.clear();
    virtualbox_install_directory.clear();
    virtualbox_guest_additions.clear();
    virtualbox_version_raw.clear();
    virtualbox_version_display.clear();
    pFloppy = NULL;
    vm_log.clear();
    vm_log_timestamp.hours = 0;
    vm_log_timestamp.minutes = 0;
    vm_log_timestamp.seconds = 0;
    vm_log_timestamp.milliseconds = 0;
    vm_master_name.clear();
    vm_master_description.clear();
    vm_name.clear();
    vm_cpu_count.clear();
    image_filename.clear();
    iso_image_filename.clear();
    cache_disk_filename.clear();
    floppy_image_filename.clear();
    current_cpu_time = 0.0;
    suspended = false;
    network_suspended = false;
    online = false;
    saving = false;
    restoring = false;
    crashed = false;
    started_successfully = false;
    register_only = false;
    rd_host_port = 0;
    headless = true;
    log_pointer = 0;
    vm_pid = 0;

#ifdef _WIN32
    vm_pid_handle = 0;
#endif
}

VBOX_BASE::~VBOX_BASE() {
    if (pFloppy) {
        delete pFloppy;
        pFloppy = NULL;
    }

#ifdef _WIN32
    if (vm_pid_handle) {
        CloseHandle(vm_pid_handle);
        vm_pid_handle = NULL;
    }
#endif
}

int VBOX_VM::run(bool do_restore_snapshot) {
    int retval;

    retval = is_registered();
    if (ERR_TIMEOUT == retval) {
        vboxlog_msg("Error: Timeout");

        return VBOXWRAPPER_ERR_RECOVERABLE;

    } else if (ERR_NOT_FOUND == retval) {

        if (is_vm_machine_configuration_available()) {
            retval = register_vm();

            if (retval){

                vboxlog_msg("Could not register");
                return retval;
            }
        } else {
            if (is_disk_image_registered()) {
                // Handle the case where a previous instance of the same projects VM
                // was already initialized for the current slot directory but aborted
                // while the task was suspended and unloaded from memory.
                retval = deregister_stale_vm();

                if (retval){

                    vboxlog_msg("Could not deregister stale VM");
                    return retval;
                }
            }
            retval = create_vm();

            if (retval){
                vboxlog_msg("Could not create VM");
                return retval;
            }
        }
    }

    // The user has requested that we exit after registering the VM,
    // so return an error to stop further processing.
    if (register_only) return ERR_FOPEN;

    // If we are restarting an already registered VM, then the vm_name variable
    // will be empty right now, so populate it with the master name so all of the
    // various other functions will work.
    vm_name = vm_master_name;

    // Check to see if the VM is already in a running state, if so, poweroff.
    poll2(false);
    if (online) {
        vboxlog_msg("VM was running");
        retval = poweroff();
        if (retval){
            vboxlog_msg("Could not stop running VM");
            return ERR_NOT_EXITED;
        }
    }

    // If our last checkpoint time is greater than 0,
    // restore from the previously saved snapshot
    //
    if (do_restore_snapshot) {
        retval = restore_snapshot();
        if (retval){
            vboxlog_msg("Could not restore from snapshot");
            return retval;
        }
    }

    // Has BOINC signaled that we should quit?
    // Try to prevent starting the VM in an environment
    // where we might be terminated any second.
    // This can happen if BOINC has been told to shutdown or the volunteer has
    // told BOINC to switch to a different project.
    //
    if (boinc_status.no_heartbeat || boinc_status.quit_request) {
        return VBOXWRAPPER_ERR_RECOVERABLE;
    }

    // Start the VM
    //
    retval = start();
    if (retval){
        vboxlog_msg("Could not start ");
        return retval;
    }

    return 0;
}

void VBOX_VM::cleanup() {
    poweroff();
    deregister_vm(true);

    // Give time enough for external processes to finish the cleanup process
    boinc_sleep(5.0);
}

void VBOX_VM::dump_hypervisor_logs(bool include_error_logs) {
    string local_system_log;
    string local_vm_log;
    string local_startup_log;
    string local_trace_log;
    unsigned long vm_exit_code = 0;

    get_system_log(local_system_log);
    get_vm_log(local_vm_log);
    get_trace_log(local_trace_log);
    get_startup_log(local_startup_log);
    get_vm_exit_code(vm_exit_code);

    if (include_error_logs) {
        dump_screenshot();
        fprintf(
            stderr,
            "\n"
            "    Hypervisor System Log:\n\n"
            "%s\n"
            "    VM Execution Log:\n\n"
            "%s\n"
            "    VM Startup Log:\n\n"
            "%s\n"
            "    VM Trace Log:\n\n"
            "%s",
            local_system_log.c_str(),
            local_vm_log.c_str(),
            local_startup_log.c_str(),
            local_trace_log.c_str()
       );
    }

    if (vm_exit_code) {
        fprintf(stderr,
            "\n"
            "    VM Exit Code: %d (0x%x)\n\n",
            (unsigned int)vm_exit_code,
            (unsigned int)vm_exit_code
       );
    }
}

void VBOX_VM::report_clean(
    bool unrecoverable_error,
    bool skip_cleanup,
    bool do_dump_hypervisor_logs,
    int retval,
    string error_reason,
    int temp_delay,
    string temp_reason,
    double total_cpu_time,
    double last_checkpoint_cpu_time,
    double fraction_done,
    double bytes_sent,
    double bytes_received
) {
    if (unrecoverable_error) {

        // Attempt to cleanup the VM and exit.
        if (!skip_cleanup) {
            cleanup();
        }
        if (error_reason.size()) {
            vboxlog_msg("\n%s", error_reason.c_str());
        }
        if (do_dump_hypervisor_logs) {
            dump_hypervisor_logs(true);
        }
        boinc_finish(retval);
    } else {
        // if the VM is already running notify BOINC about the process ID
        // so it can clean up the environment.
        // We should be safe to run after that.
        //
        if (vm_pid) {
            retval = boinc_report_app_status_aux(
                total_cpu_time,
                last_checkpoint_cpu_time,
                fraction_done,
                vm_pid,
                bytes_sent,
                bytes_received,
                0
            );
        }

        // Give the BOINC API time to report the pid to BOINC.
        //
        boinc_sleep(5.0);

        if (error_reason.size()) {
            vboxlog_msg("\n%s", error_reason.c_str());
        }

        if (do_dump_hypervisor_logs) {
            dump_hypervisor_logs(true);
        }

        // Exit and let BOINC clean up the rest.
        //
        boinc_temporary_exit(temp_delay, temp_reason.c_str());
    }
}

string VBOX_BASE::get_error(int num){

    const char* args[] = {"   BOINC will be notified that it needs to clean up the environment.\n \
        This is a temporary problem and so this job will be rescheduled for another time.\n",

          "   NOTE: VM was already running.\n \
              BOINC will be notified that it needs to clean up the environment.\n \
              This might be a temporary problem and so this job will be rescheduled for another time.\n",

          "   NOTE: VirtualBox has reported an improperly configured virtual machine. It was configured to require\n \
              hardware acceleration for virtual machines, but your processor does not support the required feature.\n \
              Please report this issue to the project so that it can be addressed.\n \
              Error Code: ERR_CPU_VM_EXTENSIONS_DISABLED\n",

          "   VboxSvc crashed while attempting to restore the current snapshot.  This is a critical\n \
              operation and this job cannot be recovered.\n",

          "   NOTE: VM session lock error encountered.\n \
              BOINC will be notified that it needs to clean up the environment.\n \
              This might be a temporary problem and so this job will be rescheduled for another time.\n",

          "   NOTE: BOINC has detected that your computer's processor supports hardware acceleration for\n \
              virtual machines but the hypervisor failed to successfully launch with this feature enabled.\n \
              This means that the hardware acceleration feature has been disabled in the computer's BIOS.\n \
              Please enable this feature in your computer's BIOS.\n \
              Intel calls it 'VT-x'\n \
              AMD calls it 'AMD-V'\n \
              More information can be found here: https://en.wikipedia.org/wiki/X86_virtualization\n \
              Error Code: ERR_CPU_VM_EXTENSIONS_DISABLED\n",

          "   NOTE: VirtualBox hypervisor reports that another hypervisor has locked the hardware acceleration\n \
              for virtual machines feature in exclusive mode.\n",

          "   NOTE: VirtualBox has failed to allocate enough memory to start the configured virtual machine.\n \
              This might be a temporary problem and so this job will be rescheduled for another time.\n",

          "   NOTE: VM failed to enter an online state within the timeout period.\n \
              This might be a temporary problem and so this job will be rescheduled for another time.\n",

          "VM environment needs to be cleaned up.",

          "Foreign VM Hypervisor locked hardware acceleration features.",

          "VM Hypervisor was unable to allocate enough memory to start VM.",

          "VM Hypervisor failed to enter an online state in a timely fashion."
    };

    std::vector<string> v(args, args + 13);
    return v[num];
}

string VBOX_BASE::read_vm_log(){

    string line;
    size_t line_pos;
    size_t line_end;
    size_t line_start;
    string msg;
    static string state = "poweredoff";
    string virtualbox_vm_log;
    virtualbox_vm_log = vm_master_name + "/Logs/VBox.log";

    string console = "Console: Machine state changed to \'";

    std::ifstream  src(virtualbox_vm_log.c_str(), std::ios::binary);

    if ((src.is_open()) && (src.seekg(log_pointer))) {
        while (std::getline(src, line)) {
            line_start = line.find(console);
            if (line_start != string::npos) {
                line_start += console.size();
                line_end = line.find("\'", line_start);
                msg = line.substr(line_start, line_end - line_start);
                sanitize_format(msg);
                state = msg;
                std::transform(state.begin(), state.end(), state.begin(), ::tolower);
            }

            line_pos = line.find("Guest Log:");
            if (line_pos != string::npos) {
                msg = line.substr(line_pos, line.size() - line_pos);
                sanitize_format(msg);
                sanitize_output(msg);
                vboxlog_msg(msg.c_str());

            }
            log_pointer = src.tellg();
        }
        return state;
    }
    else return "Error in parsing the log file";
}

// Dump any new guest log messages which are generated
// by applications running within the guest VM.
//
void VBOX_BASE::dump_vmguestlog_entries() {
    string local_vm_log;
    string line;
    size_t line_pos;
    VBOX_TIMESTAMP current_timestamp;
    string msg;
    string virtualbox_vm_log;
    virtualbox_vm_log = vm_master_name + "/Logs/VBox.log";
    int check;

    if (boinc_file_exists(virtualbox_vm_log.c_str())) {
        std::ifstream  src(virtualbox_vm_log.c_str(), std::ios::in);
        while (std::getline(src, line)) {
            line_pos = line.find("Guest Log:");
            if (line_pos != string::npos) {
                check = sscanf(
                    line.c_str(),
                    "%d:%d:%d.%d",
                    &current_timestamp.hours, &current_timestamp.minutes,
                    &current_timestamp.seconds, &current_timestamp.milliseconds
                );

                if (is_timestamp_newer(current_timestamp, vm_log_timestamp) && check == 4) {
                    vm_log_timestamp = current_timestamp;
                    msg = line.substr(line_pos, line.size() - line_pos);

                    sanitize_format(msg);

                    vboxlog_msg(msg.c_str());
                }
            }
        }
    }
}

int VBOX_BASE::dump_screenshot() {
    int    retval;
    char   screenshot_md5[33];
    double nbytes;
    char*  buf = NULL;
    size_t n;
    FILE*  f = NULL;
    string screenshot_encoded;
    string screenshot_location;

    screenshot_location = slot_dir_path + "/" + SCREENSHOT_FILENAME;

    if (boinc_file_exists(screenshot_location.c_str())) {

        // Compute MD5 hash for raw file
        retval = md5_file(screenshot_location.c_str(), screenshot_md5, nbytes, false);
        if (retval) return retval;

        buf = (char*)malloc((size_t)nbytes);
        if (!buf) {
            vboxlog_msg("Failed to allocate buffer for screenshot image dump.");
            return ERR_MALLOC;
        }
        f = fopen(screenshot_location.c_str(), "rb");
        if (!f) {
            vboxlog_msg("Failed to open screenshot image file. (%s)", screenshot_location.c_str());
            free(buf);
            return ERR_FOPEN;
        }

        n = fread(buf, 1, (size_t)nbytes, f);
        if (n != nbytes) {
            vboxlog_msg("Failed to read screenshot image file into buffer.");
        }

        screenshot_encoded = r_base64_encode(buf, n);

        fclose(f);
        free(buf);

        fprintf(
            stderr,
            "\n"
            "Screen Shot Information (Base64 Encoded PNG):\n"
            "MD5 Signature: %s\n"
            "Data: %s\n"
            "\n",
            screenshot_md5,
            screenshot_encoded.c_str()
       );
    }

    return 0;
}

bool VBOX_BASE::is_vm_machine_configuration_available() {
    string vm_machine_configuration_file;

    vm_machine_configuration_file = slot_dir_path + "/" + vm_master_name + "/" + vm_master_name + ".vbox";
    if (boinc_file_exists(vm_machine_configuration_file.c_str())) {
        return true;
    }
    return false;
}


// Updates for VirtualBox errors dealing with CPU exceleration can be found here:
// https://www.virtualbox.org/browser/vbox/trunk/src/VBox/VMM/VMMR3/HM.cpp#L599
//

bool VBOX_BASE::is_logged_failure_vm_extensions_disabled() {
    if (vm_log.find("VERR_VMX_MSR_LOCKED_OR_DISABLED") != string::npos) return true;
    if (vm_log.find("VERR_SVM_DISABLED") != string::npos) return true;

    // VirtualBox 4.3.x or better
    if (vm_log.find("VERR_VMX_MSR_VMXON_DISABLED") != string::npos) return true;
    if (vm_log.find("VERR_VMX_MSR_SMX_VMXON_DISABLED") != string::npos) return true;

    // VirtualBox 5.x or better
    if (vm_log.find("VERR_VMX_MSR_VMX_DISABLED") != string::npos) return true;
    if (vm_log.find("VERR_VMX_MSR_ALL_VMX_DISABLED") != string::npos) return true;
    if (vm_log.find("VERR_VMX_MSR_LOCKING_FAILED") != string::npos) return true;

    return false;
}

bool VBOX_BASE::is_logged_failure_vm_extensions_in_use() {
    if (vm_log.find("VERR_VMX_IN_VMX_ROOT_MODE") != string::npos) return true;
    if (vm_log.find("VERR_SVM_IN_USE") != string::npos) return true;
    return false;
}

bool VBOX_BASE::is_logged_failure_vm_extensions_not_supported() {
    if (vm_log.find("VERR_VMX_NO_VMX") != string::npos) return true;
    if (vm_log.find("VERR_SVM_NO_SVM") != string::npos) return true;
    return false;
}

bool VBOX_BASE::is_logged_failure_vm_powerup() {
    if (vm_log.find("Power up failed (vrc=VINF_SUCCESS, rc=E_FAIL (0X80004005))") != string::npos) return true;
    return false;
}

bool VBOX_BASE::is_logged_failure_host_out_of_memory() {
    if (vm_log.find("VERR_EM_NO_MEMORY") != string::npos) return true;
    if (vm_log.find("VERR_NO_MEMORY") != string::npos) return true;
    return false;
}

bool VBOX_BASE::is_logged_failure_guest_job_out_of_memory() {
    if (vm_log.find("EXIT_OUT_OF_MEMORY") != string::npos) return true;
    return false;
}

bool VBOX_BASE::is_virtualbox_version_newer(int maj, int min, int rel) {
    int vbox_major = 0, vbox_minor = 0, vbox_release = 0;
    if (3 == sscanf(virtualbox_version_raw.c_str(), "%d.%d.%d", &vbox_major, &vbox_minor, &vbox_release)) {
        if (maj < vbox_major) return true;
        if (maj > vbox_major) return false;
        if (min < vbox_minor) return true;
        if (min > vbox_minor) return false;
        if (rel < vbox_release) return true;
    }
    return true;
}

int VBOX_BASE::get_system_log(
    string& log, bool tail_only, unsigned int buffer_size
) {
    string virtualbox_system_log;
    string::iterator iter;
    int retval = BOINC_SUCCESS;

    // Locate and read log file
    virtualbox_system_log = virtualbox_profile_directory + "/VBoxSVC.log";

    if (boinc_file_exists(virtualbox_system_log.c_str())) {
        if (tail_only) {
            // Keep only the last 8k if it is larger than that.
            read_file_string(virtualbox_system_log.c_str(), log, buffer_size, true);
        } else {
            read_file_string(virtualbox_system_log.c_str(), log);
        }

        sanitize_output(log);

        if (tail_only) {
            if (log.size() >= (buffer_size - 115)) {
                // Look for the next whole line of text.
                iter = log.begin();
                while (iter != log.end()) {
                    if (*iter == '\n') {
                        log.erase(iter);
                        break;
                    }
                    iter = log.erase(iter);
                }
            }
        }
    }

    return retval;
}

int VBOX_BASE::get_vm_log(
    string& log, bool tail_only, unsigned int buffer_size
) {
    string virtualbox_vm_log;
    string::iterator iter;
    int retval = BOINC_SUCCESS;

    // Locate and read log file
    virtualbox_vm_log = vm_master_name + "/Logs/VBox.log";

    if (boinc_file_exists(virtualbox_vm_log.c_str())) {
        if (tail_only) {
            // Keep only the last 8k if it is larger than that.
            read_file_string(virtualbox_vm_log.c_str(), log, buffer_size, true);
        } else {
            read_file_string(virtualbox_vm_log.c_str(), log);
        }

        sanitize_output(log);

        if (tail_only) {
            if (log.size() >= (buffer_size - 115)) {
                // Look for the next whole line of text.
                iter = log.begin();
                while (iter != log.end()) {
                    if (*iter == '\n') {
                        log.erase(iter);
                        break;
                    }
                    iter = log.erase(iter);
                }
            }
        }

    }

    return retval;
}

int VBOX_BASE::get_trace_log(
    string& log, bool tail_only, unsigned int buffer_size
) {
    string vm_trace_log;
    string::iterator iter;
    int retval = BOINC_SUCCESS;

    // Locate and read log file
    vm_trace_log = TRACELOG_FILENAME;

    if (boinc_file_exists(vm_trace_log.c_str())) {
        if (tail_only) {
            // Keep only the last 8k if it is larger than that.
            read_file_string(vm_trace_log.c_str(), log, buffer_size, true);
        } else {
            read_file_string(vm_trace_log.c_str(), log);
        }

        sanitize_output(log);

        if (tail_only) {
            if (log.size() >= (buffer_size - 115)) {
                // Look for the next whole line of text.
                iter = log.begin();
                while (iter != log.end()) {
                    if (*iter == '\n') {
                        log.erase(iter);
                        break;
                    }
                    iter = log.erase(iter);
                }
            }
        }
    } else {
        retval = ERR_NOT_FOUND;
    }
    return retval;
}

int VBOX_BASE::get_startup_log(
    string& log, bool tail_only, unsigned int buffer_size
) {
    string virtualbox_startup_log;
    string::iterator iter;
    int retval = BOINC_SUCCESS;

    // Locate and read log file
    virtualbox_startup_log = vm_master_name + "/Logs/VBoxStartup.log";

    if (boinc_file_exists(virtualbox_startup_log.c_str())) {
        if (tail_only) {
            // Keep only the last 8k if it is larger than that.
            read_file_string(virtualbox_startup_log.c_str(), log, buffer_size, true);
        } else {
            read_file_string(virtualbox_startup_log.c_str(), log);
        }

        sanitize_output(log);

        if (tail_only) {
            if (log.size() >= (buffer_size - 115)) {
                // Look for the next whole line of text.
                iter = log.begin();
                while (iter != log.end()) {
                    if (*iter == '\n') {
                        log.erase(iter);
                        break;
                    }
                    iter = log.erase(iter);
                }
            }
        }
    }
    return retval;
}

int VBOX_BASE::read_floppy(string& data) {
    if (enable_floppyio && pFloppy) {
        data = pFloppy->receive();
        return 0;
    }
    return 1;
}

int VBOX_BASE::write_floppy(string& data) {
    if (enable_floppyio && pFloppy) {
        pFloppy->send(data);
        return 0;
    }
    return 1;
}

void VBOX_BASE::sanitize_format(string& output) {
    // Check for special characters used by printf and render them harmless
    string::iterator iter = output.begin();
    while (iter != output.end()) {
        if (*iter == '%') {
            // If we find '%', insert an additional '%' so that the we end up with
            // "%%" in its place.  This with cause printf() type functions to print
            // % within the formatted output.
            //
            iter = output.insert(iter+1, '%');
            ++iter;
        } else {
            ++iter;
        }
    }
}

#ifdef _WIN32
void VBOX_BASE::sanitize_output(string& output) {
    // Remove \r from the log spew
    string::iterator iter = output.begin();
    while (iter != output.end()) {
        if (*iter == '\r') {
            iter = output.erase(iter);
        } else {
            ++iter;
        }
    }
}
#else
void VBOX_BASE::sanitize_output(string& ) {}
#endif

// If there are errors we can recover from, process them here.
//
int VBOX_BASE::vbm_popen(string& command, string& output, const char* item, bool log_error, bool retry_failures, unsigned int timeout, bool log_trace) {
    int retval = 0;
    int retry_count = 0;
    double sleep_interval = 1.0;
    string retry_notes;

    // Initialize command line
    command = "VBoxManage -q " + command;

    do {
        retval = vbm_popen_raw(command, output , timeout);
        sanitize_output(command);
        sanitize_output(output);

        if (log_trace) {
            vbm_trace(command, output, retval);
        }

        if (retval) {
            // VirtualBox designed the concept of sessions
            // to prevent multiple applications using the VirtualBox COM API
            // (virtualbox.exe, vboxmanage.exe) from modifying the same VM
            // at the same time.
            //
            // The problem here is that vboxwrapper uses vboxmanage.exe
            // to modify and control the VM.
            // Vboxmanage.exe can only maintain the session lock
            // for as long as it takes it to run.
            // So that means 99% of the time that a VM is running under BOINC
            // technology it is running without a session lock.
            //
            // If a volunteer opens another VirtualBox management application
            // and goes poking around that application can acquire
            // the session lock and not give it up for some time.
            //
            // If we detect that condition retry the desired command.
            //
            // Experiments performed by jujube suggest changing
            // the sleep interval to an exponential style backoff
            // would increase our chances of success
            // in situations where the previous lock is held
            // by a previous instance of vboxmanage whose instance data
            // hasn't been // cleaned up within vboxsvc yet.
            //
            // Error Code: VBOX_E_INVALID_OBJECT_STATE (0x80bb0007)
            //
            if (VBOX_E_INVALID_OBJECT_STATE == (unsigned int)retval) {
                if ((output.find("Cannot attach medium") != string::npos) &&
                    (output.find("the media type") != string::npos) &&
                    (output.find("MultiAttach") != string::npos) &&
                    (output.find("can only be attached to machines that were created with VirtualBox 4.0 or later") != string::npos)) {
                        // VirtualBox occasionally writes the 'MultiAttach'
                        // attribute to the disk entry in VirtualBox.xml
                        // although this is not allowed there.
                        // As a result all VMs trying to connect that disk fail.
                        // Report the error back immediately without a retry.
                        //
                        break;
                } else {
                    if (retry_notes.find("Another VirtualBox management") == string::npos) {
                        retry_notes += "Another VirtualBox management application has locked the session for\n";
                        retry_notes += "this VM. BOINC cannot properly monitor this VM\n";
                        retry_notes += "and so this job will be aborted.\n\n";
                    }
                    if (retry_count) {
                        sleep_interval *= 2;
                    }
                }
            }

            // VboxManage has to be able to communicate with vboxsvc
            // in order to actually issue a command.
            // In cases where we detect CO_E_SERVER_EXEC_FAILURE,
            // we should just automatically try the command again.
            // Vboxmanage wasn't even able to issue the desired command anyway.
            //
            // Experiments performed by jujube suggest changing
            // the sleep interval to an exponential
            // style backoff would increase our chances of success.
            //
            // Error Code: CO_E_SERVER_EXEC_FAILURE (0x80080005)
            //
            if (CO_E_SERVER_EXEC_FAILURE == (unsigned int)retval) {
                if (retry_notes.find("Unable to communicate with VirtualBox") == string::npos) {
                    retry_notes += "Unable to communicate with VirtualBox.  VirtualBox may need to\n";
                    retry_notes += "be reinstalled.\n\n";
                }
                if (retry_count) {
                    sleep_interval *= 2;
                }
            }

            // Retry?

            if (!retry_failures &&
                    (VBOX_E_INVALID_OBJECT_STATE != (unsigned int)retval) &&
                    (CO_E_SERVER_EXEC_FAILURE != (unsigned int)retval)
               ) {
                break;
            }

            // Timeout?
            if (retry_count >= 5) {
                retval = ERR_TIMEOUT;
                break;
            }

            retry_count++;
            boinc_sleep(sleep_interval);
        }
    } while (retval);

    // Add all relevant notes to the output string and log errors
    //
    if (retval && log_error) {
        if (!retry_notes.empty()) {
            output += "\nNotes:\n\n" + retry_notes;
        }
        vboxlog_msg(
            "Error in %s for VM: %d\nCommand:\n%s\nOutput:\n%s",
            item,
            retval,
            command.c_str(),
            output.c_str()
        );
    }
    return retval;
}

// Execute the vbox manage application and copy the output to the buffer.
//
int VBOX_BASE::vbm_popen_raw(
        string& command, string& output ,
#ifdef _WIN32
        unsigned int timeout
#else
        unsigned int
#endif
) {
    size_t errcode_start;
    size_t errcode_end;
    string errcode;
    int retval = BOINC_SUCCESS;

    // Reset output buffer
    output.clear();

#ifdef _WIN32

    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    SECURITY_ATTRIBUTES sa;
    SECURITY_DESCRIPTOR sd;
    HANDLE hReadPipe = NULL, hWritePipe = NULL;
    void* pBuf = NULL;
    DWORD dwCount = 0;
    unsigned long ulExitCode = 0;
    unsigned long ulExitTimeout = 0;

    memset(&si, 0, sizeof(si));
    memset(&pi, 0, sizeof(pi));
    memset(&sa, 0, sizeof(sa));
    memset(&sd, 0, sizeof(sd));

    InitializeSecurityDescriptor(&sd, SECURITY_DESCRIPTOR_REVISION);
    SetSecurityDescriptorDacl(&sd, true, NULL, false);

    sa.nLength = sizeof(SECURITY_ATTRIBUTES);
    sa.bInheritHandle = TRUE;
    sa.lpSecurityDescriptor = &sd;

    if (!CreatePipe(&hReadPipe, &hWritePipe, &sa, NULL)) {
        vboxlog_msg("CreatePipe failed (%d).", GetLastError());
        goto CLEANUP;
    }
    SetHandleInformation(hReadPipe, HANDLE_FLAG_INHERIT, 0);

    si.cb = sizeof(STARTUPINFO);
    si.dwFlags |= STARTF_FORCEOFFFEEDBACK | STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
    si.wShowWindow = SW_HIDE;
    si.hStdOutput = hWritePipe;
    si.hStdError = hWritePipe;
    si.hStdInput = NULL;

    // Execute command
    if (!CreateProcess(
        NULL,
        (LPTSTR)command.c_str(),
        NULL,
        NULL,
        TRUE,
        CREATE_NO_WINDOW,
        NULL,
        NULL,
        &si,
        &pi
    )) {
        vboxlog_msg("CreateProcess failed (%d).", GetLastError());
        goto CLEANUP;
    }

    // Wait until process has completed
    while(1) {
        if (timeout == 0) {
            WaitForSingleObject(pi.hProcess, INFINITE);
        }

        GetExitCodeProcess(pi.hProcess, &ulExitCode);

        // Copy stdout/stderr to output buffer.
        // Handle in the loop so that we can copy the pipe as it is populated
        // and prevent the child process from blocking
        // in case the output is bigger than pipe buffer.
        //
        PeekNamedPipe(hReadPipe, NULL, NULL, NULL, &dwCount, NULL);
        if (dwCount) {
            pBuf = malloc(dwCount+1);
            memset(pBuf, 0, dwCount+1);

            if (ReadFile(hReadPipe, pBuf, dwCount, &dwCount, NULL)) {
                output += (char*)pBuf;
            }

            free(pBuf);
        }

        if (ulExitCode != STILL_ACTIVE) break;

        // Timeout?
        if (ulExitTimeout >= (timeout * 1000)) {
            if (!TerminateProcess(pi.hProcess, EXIT_FAILURE)) {
                vboxlog_msg("TerminateProcess failed (%d).", GetLastError());
            }
            ulExitCode = 0;
            retval = ERR_TIMEOUT;
            Sleep(1000);
        }

        Sleep(250);
        ulExitTimeout += 250;
    }

CLEANUP:
    if (pi.hThread) CloseHandle(pi.hThread);
    if (pi.hProcess) CloseHandle(pi.hProcess);
    if (hReadPipe) CloseHandle(hReadPipe);
    if (hWritePipe) CloseHandle(hWritePipe);

    if ((ulExitCode != 0) || (!pi.hProcess)) {

        // Determine the real error code by parsing the output
        errcode_start = output.find("(0x");
        if (errcode_start != string::npos) {
            errcode_start += 1;
            errcode_end = output.find(")", errcode_start);
            errcode = output.substr(errcode_start, errcode_end - errcode_start);

            sscanf(errcode.c_str(), "%x", &retval);
        }

        // If something couldn't be found, just return ERR_FOPEN
        if (!retval) retval = ERR_FOPEN;
    }
#else
    char buf[256];
    FILE* fp;

    // redirect stderr to stdout for the child process so we can trap it with popen.
    string modified_command = command + " 2>&1";

    // Execute command
    fp = popen(modified_command.c_str(), "r");
    if (fp == NULL) {
        vboxlog_msg("vbm_popen popen failed (%d).", errno);
        retval = ERR_FOPEN;
    } else {
        // Copy output to buffer
        while (fgets(buf, 256, fp)) {
            output += buf;
        }

        // Close stream
        pclose(fp);

        if (output.find("VBoxManage: not found") != string::npos) {
            return ERR_NOT_FOUND;
        }

        // Determine the real error code by parsing the output
        errcode_start = output.find("(0x");
        if (errcode_start != string::npos) {
            errcode_start += 1;
            errcode_end = output.find(")", errcode_start);
            errcode = output.substr(errcode_start, errcode_end - errcode_start);

            sscanf(errcode.c_str(), "%x", &retval);
        }
    }

#endif

    // Is this a RPC_S_SERVER_UNAVAILABLE returned by vboxmanage?
    if (output.find("RPC_S_SERVER_UNAVAILABLE") != string::npos) {
        retval = RPC_S_SERVER_UNAVAILABLE;
    }
    return retval;
}

void VBOX_BASE::vbm_replay(string& command) {
    FILE* f = fopen(REPLAYLOG_FILENAME, "a");
    if (f) {
        fprintf(f, "%s\n", command.c_str());
        fclose(f);
    }
}

void VBOX_BASE::vbm_trace(string& command, string& output, int retval) {
    char buf[256];
    int pid;
    struct tm tm;

    vbm_replay(command);

    time_t x = time(0);
#ifdef _WIN32
    pid = GetCurrentProcessId();
    localtime_s(&tm, &x);
#else
    pid = getpid();
    localtime_r(&x, &tm);
#endif

    strftime(buf, sizeof(buf)-1, "%Y-%m-%d %H:%M:%S", &tm);

    FILE* f = fopen(TRACELOG_FILENAME, "a");
    if (f) {
        fprintf(f, "%s (%d): ", buf, pid);
        fprintf(f, "\nCommand: %s\nExit Code: %d\nOutput:\n%s\n",
            command.c_str(),
            retval,
            output.c_str()
        );
        fclose(f);
    }
}
