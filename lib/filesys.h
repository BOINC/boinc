// The contents of this file are subject to the BOINC Public License
// Version 1.0 (the "License"); you may not use this file except in
// compliance with the License. You may obtain a copy of the License at
// http://boinc.berkeley.edu/license_1.0.txt
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

#ifndef _FILESYS_
#define _FILESYS_

#ifdef _WIN32

struct DIR_DESC {
    char path[256];
    bool first;
    void* handle;
};
typedef DIR_DESC *DIRREF;
#define PATH_SEPARATOR "\\"

#endif

#ifndef _WIN32

#include <stdio.h>
#include <string>
using std::string;

#ifdef HAVE_DIRENT_H
#include <dirent.h>
typedef DIR *DIRREF;
#endif

#define PATH_SEPARATOR "/"

#endif

// TODO TODO TODO
// remove this code - the DirScanner class does the same thing.
// But need to rewrite a couple of places that use it
//
extern DIRREF dir_open(const char*);
extern int dir_scan(char*, DIRREF, int);
int dir_scan(string&, DIRREF);
extern void dir_close(DIRREF);

extern int boinc_delete_file(const char*);
extern int file_size(const char*, double&);
extern int clean_out_dir(const char*);
extern int dir_size(const char* dirpath, double&);
extern FILE* boinc_fopen(const char* path, const char* mode);
extern int boinc_file_exists(const char* path);
extern int boinc_copy(const char* orig, const char* newf);
extern int boinc_rename(const char* old, const char* newf);
extern int boinc_mkdir(const char*);
extern int boinc_rmdir(const char*);
extern int lock_file(char*);
#ifdef _WIN32
extern void full_path(char* relname, char* path);
#endif
extern int get_filesystem_info(double& total, double& free);
extern int boinc_make_dirs(char*, char*);

class DirScanner {
#ifdef _WIN32
    string dir;
    bool first;
    void* handle;
#endif
#ifdef HAVE_DIRENT_H
    DIR* dirp;
#endif
public:
    DirScanner(string const& path);
    ~DirScanner();
    bool scan(string& name);    // return true if file returned
};

#endif

