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

#if defined(_MSC_VER) || defined(__MINGW32__)
#define popen       _popen
#define pclose      _pclose
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


// Execute the vbox manage application and copy the output to the
// designated buffer.
//
bool virtualbox_vbm_popen(std::string& arguments, char* pBuffer, int nSize) {
    FILE* fp;
    char buf[256];
    std::string strCommand;
    std::string strTemp;

    // Initialize return buffer
    memset(pBuffer, 0, nSize);

    // Initialize command line
    strCommand = "VBoxManage -q " + arguments;

    fp = popen(strCommand.c_str(), "r");
    if (fp == NULL){
        fprintf(
            stderr,
            "%s vbm_popen popen failed! cmd = '%s', errno = %d\n",
            boinc_msg_prefix(buf, sizeof(buf)),
            strCommand.c_str(),
            errno
        );
        return false;
    }

    while (fgets(buf, 256, fp)) {
        strTemp += buf;
    }

    pclose(fp);

    strncpy(pBuffer, strTemp.c_str(), nSize-1);

    return true;
}


// Returns the current directory in which the executable resides.
//
int virtualbox_generate_vm_root_dir( std::string& dir ) {
    TCHAR root_dir[256];

#ifdef _WIN32
    _getcwd(root_dir, (sizeof(root_dir)*sizeof(TCHAR)));
#else
    getcwd(root_dir, (sizeof(root_dir)*sizeof(TCHAR)));
#endif

    dir = root_dir;

    if (!dir.empty()) {
        return 1;
    }
    return 0;
}


// Generate a unique virtual machine name for a given task instance
// Rules:
//   1. Must be unique
//   2. Must identifity itself as being part of BOINC
//   3. Must be file system compatible
//
int virtualbox_generate_vm_name( std::string& name ) {
    APP_INIT_DATA aid;
    boinc_get_init_data_p( &aid );

    name.empty();
    name = "boinc_";

    if (boinc_is_standalone()) {
        name += "standalone";
    } else {
        name += aid.wu_name;
    }

    if (!name.empty()) {
        return 1;
    }
    return 0;
}






































