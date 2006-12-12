// Berkeley Open Infrastructure for Network Computing
// http://boinc.berkeley.edu
// Copyright (C) 2006 University of California
//
// This is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation;
// either version 2.1 of the License, or (at your option) any later version.
//
// This software is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//
// To view the GNU Lesser General Public License visit
// http://www.gnu.org/copyleft/lesser.html
// or write to the Free Software Foundation, Inc.,
// 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

// This program is run in a version directory.
// The main BOINC directory is ../../..

#include <stdio.h>

#include "filesys.h"

#define MAIN_DIR "../../.."

int prepare_prev_dir() {
    char prev_dir[256];
    int retval;

    sprintf(prev_dir, "%s/prev_version", MAIN_DIR);
    if (is_dir(prev_dir)) {
        retval = clean_out_dir(prev_dir);
        if (retval) return retval;
    } else {
        retval = boinc_mkdir(prev_dir);
        if (retval) return retval;
    }
    return 0;
}

int move_to_prev(char* file) {
    char oldname[1024], newname[1024];
    sprintf(oldname, "%s/%s", MAIN_DIR, file);
    sprintf(newname, "%s/prev_version/%s", MAIN_DIR, file);
    return boinc_rename(oldname, newname);
}

int move_to_main(char* file) {
    char newname[1024];
    sprintf(newname, "%s/%s", MAIN_DIR, file);
    return boinc_rename(file, newname);
}

int main() {
    int retval;

    retval = prepare_prev_dir();
    if (retval) exit(retval);
    move_to_prev("boinc.exe");
    move_to_prev("boincmgr.exe");
    move_to_main("boinc.exe");
    move_to_main("boincmgr.exe");
}
