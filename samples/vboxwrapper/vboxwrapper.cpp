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
#include "vm.h"
#include "vbox.h"

#define JOB_FILENAME "job.xml"
#define CHECKPOINT_FILENAME "vboxwrapper_checkpoint.txt"

#define POLL_PERIOD 1.0

APP_INIT_DATA aid;


int parse_job_file() {
    MIOFILE mf;
    char tag[1024], buf[256], buf2[256];
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

    if (!xp.parse_start("job_desc")) return ERR_XML_PARSE;
    while (!xp.get(tag, sizeof(tag), is_tag)) {
        if (!is_tag) {
            fprintf(stderr,
                "%s SCHED_CONFIG::parse(): unexpected text %s\n",
                boinc_msg_prefix(buf2, sizeof(buf2)), tag
            );
            continue;
        }
        if (!strcmp(tag, "/job_desc")) {
            fclose(f);
            return 0;
        }
        if (!strcmp(tag, "vm")) {
            vm.parse(xp);
        }
    }
    fclose(f);
    return ERR_XML_PARSE;
}

void write_checkpoint(double cpu) {
    FILE* f = fopen(CHECKPOINT_FILENAME, "w");
    if (!f) return;
    fprintf(f, "%f\n", cpu);
    fclose(f);
}

void read_checkpoint(double& cpu) {
    double c;
    cpu = 0;
    FILE* f = fopen(CHECKPOINT_FILENAME, "r");
    if (!f) return;
    int n = fscanf(f, "%lf", &c);
    fclose(f);
    if (n != 2) return;
    cpu = c;
}

int main(int argc, char** argv) {
    BOINC_OPTIONS boinc_options;
    BOINC_STATUS boinc_status;
    int retval;

    memset(&boinc_options, 0, sizeof(boinc_options));
    boinc_options.main_program = true;
    boinc_options.check_heartbeat = true;
    boinc_options.handle_process_control = true;
    boinc_init_options(&boinc_options);

    fprintf(stderr, "vboxwrapper: starting\n");

    boinc_get_init_data(aid);

    retval = parse_job_file();
    if (retval) {
        fprintf(stderr, "can't parse job file: %d\n", retval);
        boinc_finish(retval);
    }

    retval = vm.run();
    if (retval) {
        boinc_finish(retval);
    }

    while (1) {
        vm.poll();
        boinc_get_status(&boinc_status);
        if (boinc_status.no_heartbeat || boinc_status.quit_request) {
            vm.stop();
            exit(0);
        }
        if (boinc_status.abort_request) {
            // TODO: Unregister VM and Delete VM before exiting.
            vm.stop();
            exit(0);
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
        boinc_sleep(POLL_PERIOD);
    }
    
    vm.stop();
    boinc_finish(0);
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
