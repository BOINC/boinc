// The contents of this file are subject to the BOINC Public License
// Version 1.0 (the "License"); you may not use this file except in
// compliance with the License. You may obtain a copy of the License at
// http://boinc.berkeley.edu/license_1.0.txt
// 
// Software distributed under the License is distributed on an "AS IS"
// basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the
// License for the specific language governing rights and limitations
// under the License. 
// 
// The Original Code is the Berkeley Open Infrastructure for Network Computing. 
// 
// The Initial Developer of the Original Code is the SETI@home project.
// Portions created by the SETI@home project are Copyright (C) 2002
// University of California at Berkeley. All Rights Reserved. 
// 
// Contributor(s):
//

#include "windows_cpp.h"

#ifdef _WIN32
#include <afxwin.h>
#endif

#include <stdio.h>
#include <sys/stat.h>
#include <ctype.h>

#include "filesys.h"
#include "error_numbers.h"
#include "message.h"
#include "util.h"

#include "file_names.h"

void escape_project_url(char *in, char* out) {
    escape_url_readable(in, out);
    char& last = out[strlen(out)-1];
    // remove trailing _
    if (last == '_') {
        last = '\0';
    }
}

// Gets the pathname of a file
//
void get_pathname(FILE_INFO* fip, char* path) {
    PROJECT* p = fip->project;
    char buf[256];

    // for testing purposes, it's handy to allow a FILE_INFO without
    // an associated PROJECT.
    //
    if (p) {
        escape_project_url(p->master_url, buf);
        sprintf(path, "%s%s%s%s%s", PROJECTS_DIR, PATH_SEPARATOR, buf, PATH_SEPARATOR, fip->name);
    } else {
        safe_strncpy(path, fip->name, sizeof(fip->name));
    }
}

// Returns the location of a numbered slot directory
//
void get_slot_dir(int slot, char* path) {
    sprintf(path, "%s%s%d", SLOTS_DIR, PATH_SEPARATOR, slot);
}

// Create the directory for the project p
//
int make_project_dir(PROJECT& p) {
    char buf[256],buf2[256];

    boinc_mkdir(PROJECTS_DIR);
    escape_project_url(p.master_url, buf);
    sprintf(buf2, "%s%s%s", PROJECTS_DIR, PATH_SEPARATOR, buf);
    boinc_mkdir(buf2);
    return 0;
}

int remove_project_dir(PROJECT& p) {
    char buf[256],buf2[256];

    escape_project_url(p.master_url, buf);
    sprintf(buf2, "%s%s%s", PROJECTS_DIR, PATH_SEPARATOR, buf);
    clean_out_dir(buf2);
    boinc_rmdir(buf2);
    return 0;
}

// Create the slot directory for the specified slot #
//
int make_slot_dir(int slot) {
    char buf[256];
    if(slot<0) {
        msg_printf(NULL, MSG_ERROR, "make_slot_dir(): negative slot\n");
        return ERR_NEG;
    }
    boinc_mkdir(SLOTS_DIR);
    get_slot_dir(slot, buf);
    boinc_mkdir(buf);
    return 0;
}

void get_account_filename(char* master_url, char* path) {
    char buf[256];
    escape_project_url(master_url, buf);
    sprintf(path, "account_%s.xml", buf);
}

bool is_account_file(char* filename) {
    return (strstr(filename, "account_") == filename);
}
