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
#include "error_numbers.h"

// Converts a character string of a decimal number to hexadecimal string
//
static void c2x(char *what) {
    char buf[3];
    char num = atoi(what);
    char d1 = num / 16;
    char d2 = num % 16;
    int abase1, abase2;
    if(what==NULL) {
        fprintf(stderr, "error: c2x: unexpected NULL pointer what\n");
    }
    if (d1 < 10) abase1 = 48;
    else abase1 = 55;
    if (d2 < 10) abase2 = 48;
    else abase2 = 55;
    buf[0] = d1+abase1;
    buf[1] = d2+abase2;
    buf[2] = 0;

    strcpy(what, buf);
}

// Escape a URL, converting the non alphanumeric characters to
// %XY where XY is their hexadecimal equivalent
//
static void escape_url(char *in, char* out) {
    int x, y;
    if(in==NULL) {
        fprintf(stderr, "error: escape_url: unexpected NULL pointer in\n");
    }
    if(out==NULL) {
        fprintf(stderr, "error: escape_url: unexpected NULL pointer out\n");
    }
    for (x=0, y=0; in[x]; ++x) {
        if (isalnum(in[x]) || in[x]=='.') {
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

// Gets the pathname of a file
//
void get_pathname(FILE_INFO* fip, char* path) {
    if(fip==NULL) {
        fprintf(stderr, "error: get_pathname: unexpected NULL pointer fip\n");
    }
    if(path==NULL) {
        fprintf(stderr, "error: get_pathname: unexpected NULL pointer path\n");
    }
    PROJECT* p = fip->project;
    char buf[256];
    // for testing purposes, it's handy to allow a FILE_INFO without
    // an associated PROJECT.
    //
    if (p) {
        escape_url(p->master_url, buf);
        sprintf(path, "%s/%s", buf, fip->name);
    } else {
        strcpy(path, fip->name);
    }
}

// Returns the location of a numbered slot directory
//
void get_slot_dir(int slot, char* path) {
    if(path==NULL) {
        fprintf(stderr, "error: get_slot_dir: unexpected NULL pointer path\n");
    }
    if(slot<0) {
        fprintf(stderr, "error: get_slot_dir: negative slot\n");
    }
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

// Returns the location of a numbered slot directory
//
int make_slot_dir(int slot) {
    if(slot<0) {
        fprintf(stderr, "error: make_slot_dir: negative slot\n");
        return ERR_NEG;
    }
    char buf[256];
    CreateDirectory("slots", NULL);
    get_slot_dir(slot, buf);
    CreateDirectory(buf, NULL);
    return 0;
}

#else

// Create the directory for the project p
//
int make_project_dir(PROJECT& p) {
    char buf[256];

    escape_url(p.master_url, buf);
    mkdir(buf, 0777);
    return 0;
}

// Create the slot directory for the specified slot #
//
int make_slot_dir(int slot) {
    char buf[256];
    if(slot<0) {
        fprintf(stderr, "error: make_slot_dir: negative slot\n");
        return ERR_NEG;
    }
    mkdir("slots", 0777);
    get_slot_dir(slot, buf);
    mkdir(buf, 0777);
    return 0;
}

#endif

// Returns a filename used for prefs backup
//
int make_prefs_backup_name(PREFS& prefs, char* name) {
    if(name==NULL) {
        fprintf(stderr, "error: make_prefs_backup_name: unexpected NULL pointer name\n");
        return ERR_NULL;
    }
    sprintf(name, "prefs_backup_%d", prefs.mod_time);
    return 0;
}

