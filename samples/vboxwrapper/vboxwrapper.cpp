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

// vboxwrapper.cpp
// vboxwrapper program - lets you use VirtualBox VMs with BOINC
//
// Handles:
// - suspend/resume/quit/abort
// - reporting CPU time
// - loss of heartbeat from core client
// - checkpointing
//      (at the level of task; or potentially within task)
//
// See http://boinc.berkeley.edu/wrapper.php for details
// Contributor: Andrew J. Younge (ajy4490@umiacs.umd.edu)
// Contributor: Jie Wu <jiewu AT cern DOT ch>
// Contributor: Daniel Lombraña González <teleyinex AT gmail DOT com>
//

#ifdef _WIN32
#include "boinc_win.h"
#include "win_util.h"
#else
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <vector>
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

#define IMAGE_FILENAME "vm_image.vdi"
#define JOB_FILENAME "vbox_job.xml"
#define CHECKPOINT_FILENAME "vbox_checkpoint.txt"
#define POLL_PERIOD 1.0

int parse_job_file(VBOX_VM& vm) {
    MIOFILE mf;
    char buf[256], buf2[256], tag[256];
    bool is_tag;

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
    while (!xp.get(tag, sizeof(tag), is_tag)) {
        if (!is_tag) {
            fprintf(stderr, "%s parse_job_file(): unexpected text %s\n",
                boinc_msg_prefix(buf, sizeof(buf)), tag
            );
            continue;
        }
        if (!strcmp(tag, "/vbox_job")) {
            fclose(f);
            return 0;
        }
        else if (xp.parse_string(tag, "os_name", vm.os_name)) continue;
        else if (xp.parse_string(tag, "memory_size_mb", vm.memory_size_mb)) continue;
        else if (xp.parse_bool(tag, "enable_network", vm.enable_network)) continue;
        else if (xp.parse_bool(tag, "enable_shared_directory", vm.enable_shared_directory)) continue;
        fprintf(stderr, "%s parse_job_file(): unexpected tag %s\n",
            boinc_msg_prefix(buf, sizeof(buf)), tag
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


int main(int, char**) {
    BOINC_OPTIONS boinc_options;
    double current_cpu_time = 0.0;
    double checkpoint_cpu_time = 0.0;
    bool is_running = false;
    char buf[256];
    int retval;
    VBOX_VM vm;
    APP_INIT_DATA aid;

    memset(&boinc_options, 0, sizeof(boinc_options));
    boinc_options.main_program = true;
    boinc_options.check_heartbeat = true;
    boinc_options.handle_process_control = true;
    boinc_init_options(&boinc_options);

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
        }
        if (boinc_time_to_checkpoint()) {
            boinc_checkpoint_completed();
            checkpoint_cpu_time += current_cpu_time;
            current_cpu_time = 0.0;
        }
        if (is_running) {
            current_cpu_time += 1.0;
            boinc_report_app_status(current_cpu_time, checkpoint_cpu_time, 0.0);
        }
        boinc_sleep(POLL_PERIOD);
    }
    
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
