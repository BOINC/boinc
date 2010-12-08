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

#include "diagnostics.h"
#include "filesys.h"
#include "parse.h"
#include "str_util.h"
#include "str_replace.h"
#include "util.h"
#include "error_numbers.h"
#include "procinfo.h"
#include "boinc_api.h"
#include "vbox.h"
#include "vm.h"

VM vm;

int VM::parse(XML_PARSER& xp) {
    char tag[1024], buf[8192];
    bool is_tag;

    while (!xp.get(tag, sizeof(tag), is_tag)) {
        if (!is_tag) {
            fprintf(stderr, "%s VM::parse(): unexpected text %s\n",
                boinc_msg_prefix(buf, sizeof(buf)), tag
            );
            continue;
        }
        if (!strcmp(tag, "/vm")) {
            return 0;
        }
        else if (xp.parse_string(tag, "stdin_filename", stdin_filename)) continue;
        else if (xp.parse_string(tag, "stdout_filename", stdout_filename)) continue;
        else if (xp.parse_string(tag, "stderr_filename", stderr_filename)) continue;
        else if (xp.parse_string(tag, "checkpoint_filename", checkpoint_filename)) continue;
        else if (xp.parse_string(tag, "fraction_done_filename", fraction_done_filename)) continue;
    }
    return ERR_XML_PARSE;
}

int VM::run() {
    int retval;
    retval = virtualbox_initialize();
    if (retval) return retval;
    retval = virtualbox_startvm();
    if (retval) return retval;
    return 0;
}

void VM::poll() {
}

void VM::stop() {
    virtualbox_stopvm();
    virtualbox_cleanup();
}

void VM::pause() {
    virtualbox_pausevm();
    suspended = true;
}

void VM::resume() {
    virtualbox_resumevm();
    suspended = false;
}
