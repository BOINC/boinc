// The contents of this file are subject to the Mozilla Public License
// Version 1.0 (the "License"); you may not use this file except in
// compliance with the License. You may obtain a copy of the License at
// http://www.mozilla.org/MPL/ 
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
#include <windows.h>
#endif

#include <stdio.h>
#include <sys/stat.h>
#include <ctype.h>

#include "file_names.h"

static void c2x(char *what) {
    char buf[3];
    char num = atoi(what);
    char d1 = num / 16;
    char d2 = num % 16;
    int abase1, abase2;
    if (d1 < 10) abase1 = 48;
    else abase1 = 55;
    if (d2 < 10) abase2 = 48;
    else abase2 = 55;
    buf[0] = d1+abase1;
    buf[1] = d2+abase2;
    buf[2] = 0;

    strcpy(what, buf);
}

static void escape_url(char *in, char* out) {
    int x, y;
    for (x=0, y=0; in[x]; ++x) {
        if (isalnum(in[x])) {
            out[y] = in[x];
            ++y;
        }
        else {
            out[y] = '%';
            ++y;
            out[y] = 0;
            char buf[256];
            sprintf(buf, "%d", (char)in[x]);
            c2x(buf);
            strcat(out, buf);
            y += 2;
        }
    }
    out[y] = 0;
}

void get_pathname(FILE_INFO* fip, char* path) {
    PROJECT* p = fip->project;
    char buf[256];

    escape_url(p->master_url, buf);
    sprintf(path, "%s/%s", buf, fip->name);
}

void get_slot_dir(int slot, char* path) {
    sprintf(path, "slots/%d", slot);
}

#ifdef _WIN32

// Double check permissions for CreateDirectory

int make_project_dir(PROJECT& p) {
    char buf[256];

    escape_url(p.master_url, buf);
    CreateDirectory(buf, NULL);
    return 0;
}

int make_slot_dir(int slot) {
    char buf[256];
    CreateDirectory("slots", NULL);
    get_slot_dir(slot, buf);
    CreateDirectory(buf, NULL);
    return 0;
}

#else

int make_project_dir(PROJECT& p) {
    char buf[256];

    escape_url(p.master_url, buf);
    mkdir(buf, 0777);
    return 0;
}

int make_slot_dir(int slot) {
    char buf[256];
    mkdir("slots", 0777);
    get_slot_dir(slot, buf);
    mkdir(buf, 0777);
    return 0;
}

int make_prefs_backup_name(PREFS& prefs, char* name) {
    sprintf(name, "prefs_backup_%d", prefs.mod_time);
    return 0;
}

#endif
