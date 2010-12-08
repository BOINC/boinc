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


// Generate a unique virtual machine name for a given task instance
// Rules:
//   1. Must be unique
//   2. Must identifity itself as being part of BOINC
//   3. Must be file system compatible
//
int virtualbox_generate_vm_name( std::string& name ) {
    APP_INIT_DATA* aidp = NULL;

    name.empty();
    name = "boinc_";

    if (boinc_is_standalone()) {
        name += "standalone";
    } else {
        boinc_get_init_data_p( aidp );
        name += aidp->wu_name;
    }

    if (!name.empty()) {
        return 1;
    }
    return 0;
}




