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
#endif

#else
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <unistd.h>
#endif

using std::string;

#include "diagnostics.h"
#include "filesys.h"
#include "parse.h"
#include "str_util.h"
#include "str_replace.h"
#include "util.h"
#include "error_numbers.h"
#include "procinfo.h"
#include "network.h"
#include "boinc_api.h"
#include "floppyio.h"
#include "vboxwrapper.h"
#include "vbox_common.h"


VBOX_BASE::VBOX_BASE() {
    virtualbox_home_directory.clear();
    virtualbox_install_directory.clear();
    virtualbox_guest_additions.clear();
    virtualbox_version.clear();
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
    vm_disk_controller_type.clear();
    vm_disk_controller_model.clear();
    os_name.clear();
    memory_size_mb = 0.0;
    image_filename.clear();
    iso_image_filename.clear();
    cache_disk_filename.clear();
    floppy_image_filename.clear();
    job_duration = 0.0;
    current_cpu_time = 0.0;
    minimum_checkpoint_interval = 600.0;
    fraction_done_filename.clear();
    suspended = false;
    network_suspended = false;
    online = false;
    saving = false;
    restoring = false;
    crashed = false;
    enable_cern_dataformat = false;
    enable_shared_directory = false;
    enable_floppyio = false;
    enable_cache_disk = false;
    enable_isocontextualization = false;
    enable_remotedesktop = false;
    enable_gbac = false;
    register_only = false;
    enable_network = false;
    network_bridged_mode = false;
    pf_guest_port = 0;
    pf_host_port = 0;
    headless = true;
    vm_pid = 0;
    vboxsvc_pid = 0;
#ifdef _WIN32
    vm_pid_handle = 0;
    vboxsvc_pid_handle = 0;
#endif

    // Initialize default values
    vm_disk_controller_type = "ide";
    vm_disk_controller_model = "PIIX4";
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
    if (vboxsvc_pid_handle) {
        CloseHandle(vboxsvc_pid_handle);
        vboxsvc_pid_handle = NULL;
    }
#endif
}

int VBOX_BASE::initialize() {
    return ERR_EXEC;
}

int VBOX_BASE::create_vm() {
    return ERR_EXEC;
}

int VBOX_BASE::register_vm() {
    return ERR_EXEC;
}

int VBOX_BASE::deregister_vm(bool delete_media) {
    return ERR_EXEC;
}

int VBOX_BASE::deregister_stale_vm() {
    return ERR_EXEC;
}

void VBOX_BASE::poll(bool /*log_state*/) {
}

int VBOX_BASE::start() {
    return ERR_EXEC;
}

int VBOX_BASE::stop() {
    return ERR_EXEC;
}
int VBOX_BASE::poweroff() {
    return ERR_EXEC;
}

int VBOX_BASE::pause() {
    return ERR_EXEC;
}

int VBOX_BASE::resume() {
    return ERR_EXEC;
}

int VBOX_BASE::create_snapshot(double elapsed_time) {
    return ERR_EXEC;
}

int VBOX_BASE::cleanup_snapshots(bool delete_active) {
    return ERR_EXEC;
}

int VBOX_BASE::restore_snapshot() {
    return ERR_EXEC;
}

int VBOX_BASE::run(bool do_restore_snapshot) {
    int retval;

    retval = is_registered();
    if (ERR_TIMEOUT == retval) {

        return VBOXWRAPPER_ERR_RECOVERABLE;

    } else if (ERR_NOT_FOUND == retval) {

        if (is_vm_machine_configuration_available()) {
            retval = register_vm();
            if (retval) return retval;
        } else {
            if (is_hdd_registered()) {
                // Handle the case where a previous instance of the same projects VM
                // was already initialized for the current slot directory but aborted
                // while the task was suspended and unloaded from memory.
                retval = deregister_stale_vm();
                if (retval) return retval;
            }
            retval = create_vm();
            if (retval) return retval;
        }

    }

    // The user has requested that we exit after registering the VM, so return an
    // error to stop further processing.
    if (register_only) return ERR_FOPEN;

    // If we are restarting an already registered VM, then the vm_name variable
    // will be empty right now, so populate it with the master name so all of the
    // various other functions will work.
    vm_name = vm_master_name;

    // Check to see if the VM is already in a running state, if so, poweroff.
    poll(false);
    if (online) {
        retval = poweroff();
        if (retval) return ERR_NOT_EXITED;
    }

    // If our last checkpoint time is greater than 0, restore from the previously
    // saved snapshot
    if (do_restore_snapshot) {
        retval = restore_snapshot();
        if (retval) return retval;
    }

    // Has BOINC signaled that we should quit?
    // Try to prevent starting the VM in an environment where we might be terminated any
    // second.  This can happen if BOINC has been told to shutdown or the volunteer has 
    // told BOINC to switch to a different project.
    //
    if (boinc_status.no_heartbeat || boinc_status.quit_request) {
        return VBOXWRAPPER_ERR_RECOVERABLE;
    }

    // Start the VM
    retval = start();
    if (retval) return retval;

    return 0;
}

void VBOX_BASE::cleanup() {
    poweroff();
    deregister_vm(true);

    // Give time enough for external processes to finish the cleanup process
    boinc_sleep(5.0);
}

void VBOX_BASE::dump_hypervisor_logs(bool include_error_logs) {
    string local_system_log;
    string local_vm_log;
    string local_trace_log;
    unsigned long vm_exit_code = 0;

    get_system_log(local_system_log);
    get_vm_log(local_vm_log);
    get_trace_log(local_trace_log);
    get_vm_exit_code(vm_exit_code);

    if (include_error_logs) {
        fprintf(
            stderr,
            "\n"
            "    Hypervisor System Log:\n\n"
            "%s\n"
            "    VM Execution Log:\n\n"
            "%s\n"
            "    VM Trace Log:\n\n"
            "%s",
            local_system_log.c_str(),
            local_vm_log.c_str(),
            local_trace_log.c_str()
        );
    }

    if (vm_exit_code) {
        fprintf(
            stderr,
            "\n"
            "    VM Exit Code: %d (0x%x)\n\n",
            (unsigned int)vm_exit_code,
            (unsigned int)vm_exit_code
        );
    }
}

void VBOX_BASE::dump_hypervisor_status_reports() {
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

// Dump any new guest log messages which are generated by applications running within
// the guest VM.
void VBOX_BASE::dump_vmguestlog_entries() {
    string local_vm_log;
    string line;
    size_t eol_pos;
    size_t eol_prev_pos;
    size_t line_pos;
    VBOX_TIMESTAMP current_timestamp;
    string msg;
    char buf[256];

    get_vm_log(local_vm_log, true, 16*1024);

    eol_prev_pos = 0;
    eol_pos = local_vm_log.find("\n");
    while (eol_pos != string::npos) {
        line = local_vm_log.substr(eol_prev_pos, eol_pos - eol_prev_pos);

        line_pos = line.find("Guest Log:");
        if (line_pos != string::npos) {
            sscanf(
                line.c_str(),
                "%d:%d:%d.%d",
                &current_timestamp.hours, &current_timestamp.minutes,
                &current_timestamp.seconds, &current_timestamp.milliseconds
            );

            if (is_timestamp_newer(current_timestamp, vm_log_timestamp)) {
                vm_log_timestamp = current_timestamp;
                msg = line.substr(line_pos, line.size() - line_pos);

                fprintf(
                    stderr,
                    "%s %s\n",
                    vboxwrapper_msg_prefix(buf, sizeof(buf)),
                    msg.c_str()
                );
            }
        }

        eol_prev_pos = eol_pos + 1;
        eol_pos = local_vm_log.find("\n", eol_prev_pos);
    }
}

int VBOX_BASE::is_registered() {
    return ERR_NOT_FOUND;
}

bool VBOX_BASE::is_system_ready(std::string& message) {
    return false;
}

bool VBOX_BASE::is_vm_machine_configuration_available() {
    string virtual_machine_slot_directory;
    string vm_machine_configuration_file;
    APP_INIT_DATA aid;

    boinc_get_init_data_p(&aid);
    get_slot_directory(virtual_machine_slot_directory);

    vm_machine_configuration_file = virtual_machine_slot_directory + "/" + vm_master_name + "/" + vm_master_name + ".vbox";
    if (boinc_file_exists(vm_machine_configuration_file.c_str())) {
        return true;
    }
    return false;
}

bool VBOX_BASE::is_hdd_registered() {
    return false;
}

bool VBOX_BASE::is_extpack_installed() {
    return false;
}

bool VBOX_BASE::is_virtualbox_installed() {
    return false;
}

bool VBOX_BASE::is_logged_failure_vm_extensions_disabled() {
    if (vm_log.find("VERR_VMX_MSR_LOCKED_OR_DISABLED") != string::npos) return true;
    if (vm_log.find("VERR_SVM_DISABLED") != string::npos) return true;

    // VirtualBox 4.3.x or better
    if (vm_log.find("VERR_VMX_MSR_VMXON_DISABLED") != string::npos) return true;
    if (vm_log.find("VERR_VMX_MSR_SMX_VMXON_DISABLED") != string::npos) return true;

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
    if (3 == sscanf(virtualbox_version.c_str(), "%d.%d.%d", &vbox_major, &vbox_minor, &vbox_release)) {
        if (maj < vbox_major) return true;
        if (maj > vbox_major) return false;
        if (min < vbox_minor) return true;
        if (min > vbox_minor) return false;
        if (rel < vbox_release) return true;
    }
    return false;
}

int VBOX_BASE::get_install_directory(std::string& dir) {
    return ERR_EXEC;
}

int VBOX_BASE::get_version_information(std::string& version) {
    return ERR_EXEC;
}

int VBOX_BASE::get_guest_additions(std::string& dir) {
    return ERR_EXEC;
}

// Returns the current directory in which the executable resides.
//
int VBOX_BASE::get_slot_directory(string& dir) {
    char slot_dir[256];

    getcwd(slot_dir, sizeof(slot_dir));
    dir = slot_dir;

    if (!dir.empty()) {
        return 1;
    }
    return 0;
}

int VBOX_BASE::get_default_network_interface(std::string& iface) {
    return ERR_EXEC;
}

int VBOX_BASE::get_vm_network_bytes_sent(double& sent) {
    return ERR_EXEC;
}

int VBOX_BASE::get_vm_network_bytes_received(double& received) {
    return ERR_EXEC;
}

int VBOX_BASE::get_vm_process_id() {
    return 0;
}

int VBOX_BASE::get_vm_exit_code(unsigned long& exit_code) {
    return ERR_EXEC;
}

double VBOX_BASE::get_vm_cpu_time() {
    return 0.0;
}

int VBOX_BASE::get_system_log(string& log, bool tail_only, unsigned int buffer_size) {
    string slot_directory;
    string virtualbox_system_log_src;
    string virtualbox_system_log_dst;
    string::iterator iter;
    int retval = BOINC_SUCCESS;
    char buf[256];

    // Where should we copy temp files to?
    get_slot_directory(slot_directory);

    // Locate and read log file
    virtualbox_system_log_src = virtualbox_home_directory + "/VBoxSVC.log";
    virtualbox_system_log_dst = slot_directory + "/VBoxSVC.log";

    if (boinc_file_exists(virtualbox_system_log_src.c_str())) {
        // Skip having to deal with various forms of file locks by just making a temp
        // copy of the log file.
        boinc_copy(virtualbox_system_log_src.c_str(), virtualbox_system_log_dst.c_str());
    } else {
        fprintf(
            stderr,
            "%s WARNING: Stale VirtualBox System Log used.\n",
            vboxwrapper_msg_prefix(buf, sizeof(buf))
        );
    }

    if (boinc_file_exists(virtualbox_system_log_dst.c_str())) {
        if (tail_only) {
            // Keep only the last 8k if it is larger than that.
            read_file_string(virtualbox_system_log_dst.c_str(), log, buffer_size, true);
        } else {
            read_file_string(virtualbox_system_log_dst.c_str(), log);
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
        fprintf(
            stderr,
            "%s WARNING: Stale VirtualBox System Log Not Found.\n",
            vboxwrapper_msg_prefix(buf, sizeof(buf))
        );
        retval = ERR_NOT_FOUND;
    }

    return retval;
}

int VBOX_BASE::get_vm_log(string& log, bool tail_only, unsigned int buffer_size) {
    string slot_directory;
    string virtualbox_vm_log_src;
    string virtualbox_vm_log_dst;
    string::iterator iter;
    int retval = BOINC_SUCCESS;
    char buf[256];

    // Where should we copy temp files to?
    get_slot_directory(slot_directory);

    // Locate and read log file
    virtualbox_vm_log_src = vm_master_name + "/Logs/VBox.log";
    virtualbox_vm_log_dst = slot_directory + "/VBox.log";

    if (boinc_file_exists(virtualbox_vm_log_src.c_str())) {
        // Skip having to deal with various forms of file locks by just making a temp
        // copy of the log file.
        boinc_copy(virtualbox_vm_log_src.c_str(), virtualbox_vm_log_dst.c_str());
    } else {
        fprintf(
            stderr,
            "%s WARNING: Stale VirtualBox VM Log used.\n",
            vboxwrapper_msg_prefix(buf, sizeof(buf))
        );
    }

    if (boinc_file_exists(virtualbox_vm_log_dst.c_str())) {
        if (tail_only) {
            // Keep only the last 8k if it is larger than that.
            read_file_string(virtualbox_vm_log_dst.c_str(), log, buffer_size, true);
        } else {
            read_file_string(virtualbox_vm_log_dst.c_str(), log);
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
        fprintf(
            stderr,
            "%s WARNING: Stale VirtualBox VM Log Not Found.\n",
            vboxwrapper_msg_prefix(buf, sizeof(buf))
        );
        retval = ERR_NOT_FOUND;
    }

    return retval;
}

int VBOX_BASE::get_trace_log(string& log, bool tail_only, unsigned int buffer_size) {
    string slot_directory;
    string vm_trace_log;
    string::iterator iter;
    int retval = BOINC_SUCCESS;

    // Where should we copy temp files to?
    get_slot_directory(slot_directory);

    // Locate and read log file
    vm_trace_log = slot_directory + "/" + TRACELOG_FILENAME;

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

int VBOX_BASE::set_network_access(bool enabled) {
    return ERR_EXEC;
}

int VBOX_BASE::set_cpu_usage(int percentage) {
    return ERR_EXEC;
}

int VBOX_BASE::set_network_usage(int kilobytes) {
    return ERR_EXEC;
}

int VBOX_BASE::read_floppy(std::string& data) {
    if (enable_floppyio && pFloppy) {
        data = pFloppy->receive();
        return 0;
    }
    return 1;
}

int VBOX_BASE::write_floppy(std::string& data) {
    if (enable_floppyio && pFloppy) {
        pFloppy->send(data);
        return 0;
    }
    return 1;
}

void VBOX_BASE::lower_vm_process_priority() {
}

void VBOX_BASE::reset_vm_process_priority() {
}

void VBOX_BASE::sanitize_output(std::string& output) {
#ifdef _WIN32
    // Remove \r from the log spew
    string::iterator iter = output.begin();
    while (iter != output.end()) {
        if (*iter == '\r') {
            iter = output.erase(iter);
        } else {
            ++iter;
        }
    }
#endif
}

VBOX_VM::VBOX_VM() {
    VBOX_BASE::VBOX_BASE();
}

VBOX_VM::~VBOX_VM() {
    VBOX_BASE::~VBOX_BASE();
}
