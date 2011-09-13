// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2010 University of California
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
// - checkpointing
//      (at the level of task; or potentially within task)
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
#include "vbox.h"

using std::vector;

#define IMAGE_FILENAME "vm_image.vdi"
#define JOB_FILENAME "vbox_job.xml"
#define CHECKPOINT_FILENAME "vbox_checkpoint.txt"
#define POLL_PERIOD 1.0

int parse_job_file(VBOX_VM& vm) {
    MIOFILE mf;
    char buf[256], buf2[256];

    boinc_resolve_filename(JOB_FILENAME, buf, 1024);
    FILE* f = boinc_fopen(buf, "r");
    if (!f) {
        fprintf(stderr,
            "%s can't open job file %s\n",
            boinc_msg_prefix(buf2, sizeof(buf2)), buf
        );
        return ERR_FOPEN;
    }
    mf.init_file(f);
    XML_PARSER xp(&mf);

    if (!xp.parse_start("vbox_job")) return ERR_XML_PARSE;
    while (!xp.get_tag()) {
        if (!xp.is_tag) {
            fprintf(stderr, "%s parse_job_file(): unexpected text %s\n",
                boinc_msg_prefix(buf, sizeof(buf)), xp.parsed_tag
            );
            continue;
        }
        if (xp.match_tag("/vbox_job")) {
            fclose(f);
            return 0;
        }
        else if (xp.parse_string("os_name", vm.os_name)) continue;
        else if (xp.parse_string("memory_size_mb", vm.memory_size_mb)) continue;
        else if (xp.parse_bool("enable_network", vm.enable_network)) continue;
        else if (xp.parse_bool("enable_shared_directory", vm.enable_shared_directory)) continue;
        fprintf(stderr, "%s parse_job_file(): unexpected tag %s\n",
            boinc_msg_prefix(buf, sizeof(buf)), xp.parsed_tag
        );
    }
    fclose(f);
    return ERR_XML_PARSE;
}

void write_checkpoint(double cpu) {
    boinc_begin_critical_section();
    FILE* f = fopen(CHECKPOINT_FILENAME, "w");
    if (!f) return;
    fprintf(f, "%f\n", cpu);
    fclose(f);
    boinc_checkpoint_completed();
}

void read_checkpoint(double& cpu) {
    double c;
    cpu = 0;
    FILE* f = fopen(CHECKPOINT_FILENAME, "r");
    if (!f) return;
    int n = fscanf(f, "%lf", &c);
    fclose(f);
    if (n != 1) return;
    cpu = c;
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

int main(int argc, char** argv) {
    BOINC_OPTIONS boinc_options;
    double elapsed_time = 0.0;
    double checkpoint_cpu_time = 0.0;
    bool is_running = false;
    char buf[256];
    int retval;
    VBOX_VM vm;
    APP_INIT_DATA aid;
    double trickle_period = 0, trickle_cpu_time = 0;;

    memset(&boinc_options, 0, sizeof(boinc_options));
    boinc_options.main_program = true;
    boinc_options.check_heartbeat = true;
    boinc_options.handle_process_control = true;
    boinc_init_options(&boinc_options);

    for (int i=1; i<argc; i++) {
        if (!strcmp(argv[i], "--trickle")) {
            trickle_period = atof(argv[++i]);
        }
    }

    fprintf(
        stderr,
        "%s vboxwrapper: starting\n",
        boinc_msg_prefix(buf, sizeof(buf))
    );

    retval = parse_job_file(vm);
    if (retval) {
        fprintf(
            stderr,
            "%s can't parse job file: %d\n",
            boinc_msg_prefix(buf, sizeof(buf)),
            retval
        );
        boinc_finish(retval);
    }
    boinc_get_init_data_p(&aid);
    vm.vm_name = "boinc_";
    if (boinc_is_standalone()) {
        vm.vm_name += "standalone";
        vm.image_filename = IMAGE_FILENAME;
    } else {
        vm.vm_name += aid.result_name;
        sprintf(buf, "%s_%d", IMAGE_FILENAME, aid.slot);
        vm.image_filename = buf;
        boinc_rename(IMAGE_FILENAME, buf);
    }

    read_checkpoint(checkpoint_cpu_time);

    retval = vm.run();
    if (retval) {
        boinc_finish(retval);
    }

    set_throttles(aid, vm);

    int vm_pid;
    bool have_vm_pid = false;
    bool reported_vm_pid = false;

    while (1) {
        vm.poll();
        is_running = vm.is_running();

        if (boinc_status.no_heartbeat || boinc_status.quit_request) {
            vm.stop();
            write_checkpoint(checkpoint_cpu_time);
            boinc_temporary_exit(0);
        }
        if (boinc_status.abort_request) {
            vm.cleanup();
            write_checkpoint(checkpoint_cpu_time);
            boinc_finish(EXIT_ABORTED_BY_CLIENT);
        }
        if (!is_running) {
            vm.cleanup();
            write_checkpoint(checkpoint_cpu_time);
            boinc_finish(0);
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
            if (!have_vm_pid) {
                retval = vm.get_vm_process_id(vm_pid);
                if (!retval) {
                    have_vm_pid = true;
                }
            }
            if (have_vm_pid && !reported_vm_pid) {
                vector<int> v;
                v.push_back(vm_pid);
                boinc_report_app_status_aux(
                    elapsed_time, checkpoint_cpu_time, 0, &v
                );
            } else {
                boinc_report_app_status(elapsed_time, checkpoint_cpu_time, 0);
            }
            if (trickle_period) {
                trickle_cpu_time += POLL_PERIOD;
                if (trickle_cpu_time >= trickle_period) {
                    sprintf(buf, "<cpu_time>%f</cpu_time>", trickle_cpu_time);
                    boinc_send_trickle_up(const_cast<char*>("cpu_time"), buf);
                    trickle_cpu_time = 0;
                }
            }

            // if we've been running for at least the scheduling period,
            // do a checkpoint and temporary exit;
            // the client will run us again if it wants.
            // 
            if (elapsed_time > aid.global_prefs.cpu_scheduling_period()) {
                vm.stop();
                write_checkpoint(checkpoint_cpu_time);
                boinc_temporary_exit(0);
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
            boinc_parse_init_data_file();
            boinc_get_init_data_p(&aid);
            set_throttles(aid, vm);
        }
        boinc_sleep(POLL_PERIOD);
    }
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
