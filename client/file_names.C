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

#include "filesys.h"
#include "error_numbers.h"
#include "file_names.h"

// Converts a character string of a decimal number to hexadecimal string
//
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

// Escape a URL, converting the non alphanumeric characters to
// %XY where XY is their hexadecimal equivalent
//
static void escape_url(char *in, char* out) {
    int x, y;
    for (x=0, y=0; in[x]; ++x) {
        if (isalnum(in[x]) || in[x]=='.' || in[x]=='-' || in[x]=='_') {
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

// Escape a URL for the project directory, cutting off the "http://", converting
// '\' '/' and ' ' to '_', and converting the non alphanumeric characters to
// %XY where XY is their hexadecimal equivalent
//
static void escape_project_url(char *in, char* out) {
    int x, y;
    char *temp;
    
    temp = strstr(in,"://");
    if (temp) {
        in = temp + strlen("://");
    }
    for (x=0, y=0; in[x]; ++x) {
        if (isalnum(in[x]) || in[x]=='.' || in[x]=='-' || in[x]=='_') {
            out[y] = in[x];
            ++y;
        } else if (in[x] == '/' || in[x] == '\\' || in[x] == ' ') {
            out[y] = '_';
            ++y;
        } else {
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
    PROJECT* p = fip->project;
    char buf[256];

    // for testing purposes, it's handy to allow a FILE_INFO without
    // an associated PROJECT.
    //
    if (p) {
        escape_project_url(p->master_url, buf);
        sprintf(path, "%s%s%s", buf, PATH_SEPARATOR, fip->name);
    } else {
        strcpy(path, fip->name);
    }
}

// Returns the location of a numbered slot directory
//
void get_slot_dir(int slot, char* path) {
    sprintf(path, "%s%s%d", SLOTS_DIR, PATH_SEPARATOR, slot);
}

#ifdef _WIN32

// Double check permissions for CreateDirectory

int make_project_dir(PROJECT& p) {
    char buf[256],buf2[256];

    CreateDirectory(PROJECTS_DIR, NULL);
    escape_project_url(p.master_url, buf);
    sprintf( buf2, "%s%s%s", PROJECTS_DIR, PATH_SEPARATOR, buf );
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
    CreateDirectory(SLOTS_DIR, NULL);
    get_slot_dir(slot, buf);
    CreateDirectory(buf, NULL);
    return 0;
}

#else

// Create the directory for the project p
//
int make_project_dir(PROJECT& p) {
    char buf[256],buf2[256];

    mkdir(PROJECTS_DIR, 0777);
    escape_project_url(p.master_url, buf);
    sprintf( buf2, "%s%s%s", PROJECTS_DIR, PATH_SEPARATOR, buf );
    mkdir(buf2, 0777);
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
    mkdir(SLOTS_DIR, 0777);
    get_slot_dir(slot, buf);
    mkdir(buf, 0777);
    return 0;
}

#endif

// Returns a filename used for prefs backup
//
int make_prefs_backup_name(PREFS& prefs, char* name) {
    sprintf(name, "prefs_backup_%d", prefs.mod_time);
    return 0;
}

