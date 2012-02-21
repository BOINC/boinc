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

// vboxwrapper [options]     BOINC VirtualBox wrapper
// see: http://boinc.berkeley.edu/trac/wiki/VboxApps
// Options:
// --trickle X      send a trickle message reporting elapsed time every X secs
//                  (use this for credit granting if your app does its
//                  own job management, like CernVM).
//
// Handles:
// - suspend/resume/quit/abort
// - reporting CPU time
// - loss of heartbeat from core client
//
// Contributors:
// Andrew J. Younge (ajy4490 AT umiacs DOT umd DOT edu)
// Jie Wu <jiewu AT cern DOT ch>
// Daniel Lombraña González <teleyinex AT gmail DOT com>
// Rom Walton
// David Anderson

// To debug a VM within the BOINC/VboxWrapper framework:
// 1. Launch BOINC with --exit_before_start
// 2. When BOINC exits, launch the VboxWrapper with the register_only
// 3. Set the VBOX_USER_HOME environment variable to the vbox directory
//    under the slot directory.
//    This changes where the VirtualBox applications look for the
//    root VirtualBox configuration files.
//    It may or may not apply to your installation of VirtualBox.
//    It depends on where your copy of VirtualBox came from
//    and what type of system it is installed on.
// 4. Now Launch the VM using the VirtualBox UI.
//    You should now be able to interact with your VM.

#ifdef _WIN32
#include "boinc_win.h"
#include "win_util.h"
#else
#include <vector>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <math.h>
#include <string>
#include <unistd.h>
#endif

#include "boinc_api.h"
#include "diagnostics.h"
#include "filesys.h"
#include "parse.h"
#include "str_util.h"
#include "str_replace.h"
#include "util.h"
#include "error_numbers.h"
#include "procinfo.h"
#include "vboxwrapper.h"
#include "vbox.h"

using std::vector;


char* vboxwrapper_msg_prefix(char* sbuf, int len) {
    char buf[256];
    struct tm tm;
    struct tm *tmp = &tm;
    int n;

    time_t x = time(0);
    if (x == -1) {
        strcpy(sbuf, "time() failed");
        return sbuf;
    }
#ifdef _WIN32
#ifdef __MINGW32__
    if ((tmp = localtime(&x)) == NULL) {
#else
    if (localtime_s(&tm, &x) == EINVAL) {
#endif
#else
    if (localtime_r(&x, &tm) == NULL) {
#endif
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


int parse_job_file(VBOX_VM& vm) {
    MIOFILE mf;
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
        else if (xp.parse_string("os_name", vm.os_name)) continue;
        else if (xp.parse_string("memory_size_mb", vm.memory_size_mb)) continue;
        else if (xp.parse_double("job_duration", vm.job_duration)) continue;
        else if (xp.parse_bool("enable_cern_dataformat", vm.enable_cern_dataformat)) continue;
        else if (xp.parse_bool("enable_network", vm.enable_network)) continue;
        else if (xp.parse_bool("enable_shared_directory", vm.enable_shared_directory)) continue;
        else if (xp.parse_bool("enable_floppyio", vm.enable_floppyio)) continue;
        else if (xp.parse_bool("enable_remotedesktop", vm.enable_remotedesktop)) continue;
        else if (xp.parse_int("pf_guest_port", vm.pf_guest_port)) continue;
        else if (xp.parse_int("pf_host_port", vm.pf_host_port)) continue;
        fprintf(stderr, "%s parse_job_file(): unexpected tag %s\n",
            vboxwrapper_msg_prefix(buf, sizeof(buf)), xp.parsed_tag
        );
    }
    fclose(f);
    return ERR_XML_PARSE;
}

void write_checkpoint(double cpu, VBOX_VM& vm) {
    FILE* f = fopen(CHECKPOINT_FILENAME, "w");
    if (!f) return;
    fprintf(f, "%f %d %d\n", cpu, vm.pf_host_port, vm.rd_host_port);
    fclose(f);
}

void read_checkpoint(double& cpu, VBOX_VM& vm) {
    double c;
    int pf_host;
    int rd_host;
    cpu = 0.0;
    vm.pf_host_port = 0;
    vm.rd_host_port = 0;
    FILE* f = fopen(CHECKPOINT_FILENAME, "r");
    if (!f) return;
    int n = fscanf(f, "%lf %d %d", &c, &pf_host, &rd_host);
    fclose(f);
    if (n != 3) return;
    cpu = c;
    vm.pf_host_port = pf_host;
    vm.rd_host_port = rd_host;
}

// set CPU and network throttling if needed
//
void set_throttles(APP_INIT_DATA& aid, VBOX_VM& vm) {
    double x = aid.global_prefs.cpu_usage_limit;
    if (x && x<100) {
        vm.set_cpu_usage_fraction(x/100.);
    }

    // vbox doesn't distinguish up and down bandwidth; use the min of the prefs
    //
    x = aid.global_prefs.max_bytes_sec_up;
    double y = aid.global_prefs.max_bytes_sec_down;
    if (y) {
        if (!x || y<x) {
            x = y;
        }
    }
    if (x) {
        vm.set_network_max_bytes_sec(x);
    }
}

// If the Floppy device has been specified, initialize its state so that
// it contains the contents of the init_data.xml file.  In theory this
// would allow network enabled VMs to know about proxy server configurations
// either specified by the volunteer or automatically detected by the
// core client.
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
            scratch  = "BOINC_USERNAME=" + std::string(aid.user_name) + "\n";
            scratch += "BOINC_AUTHENTICATOR=" + std::string(aid.authenticator) + "\n";

            sprintf(buf, "%d", aid.userid);
            scratch += "BOINC_USERID=" + std::string(buf) + "\n";

            sprintf(buf, "%d", aid.hostid);
            scratch += "BOINC_HOSTID=" + std::string(buf) + "\n";

            sprintf(buf, "%.17g", aid.user_total_credit);
            scratch += "BOINC_USER_TOTAL_CREDIT=" + std::string(buf) + "\n";

            sprintf(buf, "%.17g", aid.host_total_credit);
            scratch += "BOINC_HOST_TOTAL_CREDIT=" + std::string(buf) + "\n";
        }
        vm.write_floppy(scratch);
    }
}

// set port forwarding information if needed
//
void set_port_forwarding_info(APP_INIT_DATA& /* aid */, VBOX_VM& vm) {
    char buf[256];

    if (vm.pf_guest_port && vm.pf_host_port) {
        // Write info to disk
        //
        MIOFILE mf;
        FILE* f = boinc_fopen(PORTFORWARD_FILENAME, "w");
        mf.init_file(f);

        mf.printf(
            "<port_forwarding>\n"
            "  <rule>\n"
            "    <host_port>%d</host_port>\n"
            "    <guest_port>%d</guest_port>\n"
            "  </rule>\n"
            "</port_forwarding>\n",
            vm.pf_host_port,
            vm.pf_guest_port
        );

        fclose(f);

        sprintf(buf, "http://localhost:%d", vm.pf_host_port);
        boinc_web_graphics_url(buf);
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

int main(int argc, char** argv) {
    int retval;
    BOINC_OPTIONS boinc_options;
    VBOX_VM vm;
    APP_INIT_DATA aid;
    double elapsed_time = 0.0;
    double trickle_period = 0.0;
    double trickle_cpu_time = 0.0;
    double fraction_done = 0.0;
    double checkpoint_cpu_time = 0.0;
    double last_status_report_time = 0.0;
    double stopwatch_time = 0.0;
    double sleep_time = 0.0;
    double bytes_sent = 0.0;
    double bytes_received = 0.0;
    double ncpus = 0.0;
    bool report_vm_pid = false;
    bool report_net_usage = false;
    int vm_pid = 0;
    unsigned long vm_exit_code = 0;
    std::string vm_log;
    std::string system_log;
    char buf[256];

    memset(&boinc_options, 0, sizeof(boinc_options));
    boinc_options.main_program = true;
    boinc_options.check_heartbeat = true;
    boinc_options.handle_process_control = true;
    boinc_init_options(&boinc_options);

    for (int i=1; i<argc; i++) {
        if (!strcmp(argv[i], "--trickle")) {
            trickle_period = atof(argv[++i]);
        }
        if (!strcmp(argv[i], "--nthreads")) {
            ncpus = atof(argv[++i]);
        }
        if (!strcmp(argv[i], "--register_only")) {
            vm.register_only = true;
        }
    }

    fprintf(
        stderr,
        "%s vboxwrapper: starting\n",
        vboxwrapper_msg_prefix(buf, sizeof(buf))
    );

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

    boinc_get_init_data_p(&aid);
    vm.vm_master_name = "boinc_";
    vm.image_filename = IMAGE_FILENAME_COMPLETE;
    if (boinc_is_standalone()) {
        vm.vm_master_name += "standalone";
        if (vm.enable_floppyio) {
            sprintf(buf, "%s.%s", FLOPPY_IMAGE_FILENAME, FLOPPY_IMAGE_FILENAME_EXTENSION);
            vm.floppy_image_filename = buf;
        }
    } else {
        vm.vm_master_name += aid.result_name;
        if (vm.enable_floppyio) {
            sprintf(buf, "%s_%d.%s", FLOPPY_IMAGE_FILENAME, aid.slot, FLOPPY_IMAGE_FILENAME_EXTENSION);
            vm.floppy_image_filename = buf;
        }
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

    // Restore from checkpoint
    read_checkpoint(checkpoint_cpu_time, vm);
    elapsed_time = checkpoint_cpu_time;

    // Should we even try to start things up?
    if (vm.job_duration && (elapsed_time > vm.job_duration)) {
        return ERR_RSC_LIMIT_EXCEEDED;
    }

    retval = vm.run();
    if (retval) {
        // Get logs before cleanup
        vm.get_system_log(system_log);
        vm.get_vm_log(vm_log);

        // Cleanup
        vm.cleanup();
        write_checkpoint(elapsed_time, vm);

        fprintf(
            stderr,
            "%s VM failed to start.\n",
            vboxwrapper_msg_prefix(buf, sizeof(buf))
        );
        if ((vm_log.find("VERR_VMX_MSR_LOCKED_OR_DISABLED") != std::string::npos) || (vm_log.find("VERR_SVM_DISABLED") != std::string::npos)) {
            fprintf(
                stderr,
                "%s NOTE: BOINC has detected that your processor supports hardware acceleration for virtual machines\n"
                "    but the hypervisor failed to successfully launch with this feature enabled. This means that the\n"
                "    hardware acceleration feature has been disabled in the computers BIOS. Please enable this\n"
                "    feature in your BIOS.\n"
                "    Intel Processors call it 'VT-x'\n"
                "    AMD Processors call it 'AMD-V'\n"
                "    More information can be found here: http://en.wikipedia.org/wiki/X86_virtualization\n"
                "    Error Code: ERR_CPU_VM_EXTENSIONS_DISABLED\n",
                vboxwrapper_msg_prefix(buf, sizeof(buf))
            );
        } else if ((vm_log.find("VERR_VMX_IN_VMX_ROOT_MODE") != std::string::npos) || (vm_log.find("VERR_SVM_IN_USE") != std::string::npos)) {
            fprintf(
                stderr,
                "%s NOTE: VirtualBox hypervisor reports that another hypervisor has locked the hardware acceleration\n"
                "    for virtual machines feature in exclusive mode. You'll either need to reconfigure the other hypervisor\n"
                "    to not use the feature exclusively or just let BOINC run this project in software emulation mode.\n"
                "    Error Code: ERR_CPU_VM_EXTENSIONS_DISABLED\n",
                vboxwrapper_msg_prefix(buf, sizeof(buf))
            );
        } else if ((vm_log.find("VERR_VMX_NO_VMX") != std::string::npos) || (vm_log.find("VERR_SVM_NO_SVM") != std::string::npos)) {
            fprintf(
                stderr,
                "%s NOTE: VirtualBox has reported an improperly configured virtual machine. It was configured to require\n"
                "    hardware acceleration for virtual machines, but your processor does not support the required feature.\n"
                "    Please report this issue to the project so that it can be addresssed.\n",
                vboxwrapper_msg_prefix(buf, sizeof(buf))
            );
        } else {
            fprintf(
                stderr,
                "%s Hypervisor System Log:\n\n"
                "%s\n"
                "%s VM Execution Log:\n\n"
                "%s\n",
                vboxwrapper_msg_prefix(buf, sizeof(buf)),
                system_log.c_str(),
                vboxwrapper_msg_prefix(buf, sizeof(buf)),
                vm_log.c_str()
            );
        }

        boinc_finish(retval);
    }

    set_floppy_image(aid, vm);
    set_port_forwarding_info(aid, vm);
    set_remote_desktop_info(aid, vm);
    set_throttles(aid, vm);
    write_checkpoint(elapsed_time, vm);

    while (1) {
        // Begin stopwatch timer
        stopwatch_time = dtime();

        // Discover the VM's current state
        vm.poll();

        if (boinc_status.no_heartbeat || boinc_status.quit_request) {
            vm.stop();
            write_checkpoint(checkpoint_cpu_time, vm);
            boinc_temporary_exit(0);
        }
        if (boinc_status.abort_request) {
            vm.cleanup();
            write_checkpoint(elapsed_time, vm);
            boinc_finish(EXIT_ABORTED_BY_CLIENT);
        }
        if (!vm.online) {
            if (vm.crashed || (elapsed_time < vm.job_duration)) {
                vm.get_system_log(system_log);
                vm.get_vm_log(vm_log);
                vm.get_vm_exit_code(vm_exit_code);
            }
            vm.cleanup();
            write_checkpoint(elapsed_time, vm);

            if (vm.crashed || (elapsed_time < vm.job_duration)) {
                fprintf(
                    stderr,
                    "%s VM Premature Shutdown Detected.\n"
                    "    Hypervisor System Log:\n\n"
                    "%s\n"
                    "    VM Execution Log:\n\n"
                    "%s\n"
                    "    VM Exit Code: %d (0x%x)\n\n",
                    vboxwrapper_msg_prefix(buf, sizeof(buf)),
                    system_log.c_str(),
                    vm_log.c_str(),
                    (unsigned int)vm_exit_code,
                    (unsigned int)vm_exit_code
                );
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
                boinc_finish(0);
            }
        }
        if (boinc_status.suspended) {
            if (!vm.suspended) {
                vm.pause();
            }
        } else {
            if (vm.suspended) {
                vm.resume();
            }

            elapsed_time += POLL_PERIOD;

            if (!vm_pid) {
                vm.get_vm_process_id(vm_pid);
                if (vm_pid) {
                    vm.reset_vm_process_priority();
                    report_vm_pid = true;
                }
            }

            if (boinc_time_to_checkpoint()) {
                checkpoint_cpu_time = elapsed_time;
                write_checkpoint(checkpoint_cpu_time, vm);
                if (vm.job_duration) {
                    fraction_done = elapsed_time / vm.job_duration;
                    if (fraction_done > 1.0) {
                        fraction_done = 1.0;
                    }
                }
                boinc_report_app_status(
                    elapsed_time,
                    checkpoint_cpu_time,
                    fraction_done
                );
                if ((elapsed_time - last_status_report_time) >= 6000.0) {
                    last_status_report_time = elapsed_time;
                    if (aid.global_prefs.daily_xfer_limit_mb) {
                        fprintf(
                            stderr,
                            "%s Status Report: Job Duration: '%f', Elapsed Time: '%f', Network Bytes Sent (Total): '%f', Network Bytes Received (Total): '%f'\n",
                            vboxwrapper_msg_prefix(buf, sizeof(buf)),
                            vm.job_duration,
                            elapsed_time,
                            bytes_sent,
                            bytes_received
                        );
                    } else {
                        fprintf(
                            stderr,
                            "%s Status Report: Job Duration: '%f', Elapsed Time: '%f'\n",
                            vboxwrapper_msg_prefix(buf, sizeof(buf)),
                            vm.job_duration,
                            elapsed_time
                        );
                    }
                }
                boinc_checkpoint_completed();
            }

            if (report_vm_pid || report_net_usage) {
                retval = boinc_report_app_status_aux(
                    elapsed_time,
                    checkpoint_cpu_time,
                    fraction_done,
                    vm_pid,
                    bytes_sent,
                    bytes_received
                );
                if (!retval) {
                    report_vm_pid = false;
                    report_net_usage = false;
                }
            }

            if (trickle_period) {
                trickle_cpu_time += POLL_PERIOD;
                if (trickle_cpu_time >= trickle_period) {
                    sprintf(buf, "<cpu_time>%f</cpu_time>", trickle_cpu_time);
                    boinc_send_trickle_up(const_cast<char*>("cpu_time"), buf);
                    trickle_cpu_time = 0;
                }
            }

            // if the VM has a maximum amount of time it is allowed to run,
            // shut it down gacefully and exit.
            //
            if (vm.job_duration && (elapsed_time > vm.job_duration)) {
                vm.cleanup();
                write_checkpoint(elapsed_time, vm);
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
        }

        // report network usage every 10 min so the client can enforce quota
        //
        static double net_usage_timer=600;
        if (aid.global_prefs.daily_xfer_limit_mb
            && vm.enable_network
            && !vm.suspended
        ) {
            net_usage_timer -= POLL_PERIOD;
            if (net_usage_timer <= 0) {
                net_usage_timer = 600;
                double sent, received;
                retval = vm.get_network_bytes_sent(sent);
                if (!retval && (sent != bytes_sent)) {
                    bytes_sent = sent;
                    report_net_usage = true;
                }
                retval = vm.get_network_bytes_received(received);
                if (!retval && (received != bytes_received)) {
                    bytes_received = received;
                    report_net_usage = true;
                }
            }
        }

        // Sleep for the remainder of the polling period
        sleep_time = POLL_PERIOD - (dtime() - stopwatch_time);
        if (sleep_time > 0) {
            boinc_sleep(sleep_time);
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
