// $Id$
//
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

/* to allow prototypes using 'bool' in ANSI-C.  Note that gcc defines
   bool as an INT, and MS VC++ up to version 4.2 also does.  However
   more recent versions of MS VC++ define bool as CHAR.  */
#if (!defined __cplusplus) && (!defined bool)
#if ((defined(_MSC_VER)) && (_MSC_VER > 1020))
#define bool char
#else
#define bool int
#endif /* defined(_MSC_VER) && (_MSC_VER > 1020) */
#endif /* (!defined __cplusplus) && (!defined bool) */

#ifdef _WIN32

typedef struct _DIR_DESC {
    char path[256];
    bool first;
    void* handle;
} DIR_DESC;

typedef DIR_DESC *DIRREF;
#define PATH_SEPARATOR "\\"

#else
#include <stdio.h>
#include <dirent.h>

#ifdef __cplusplus
#include <string>
#endif

typedef DIR *DIRREF;
#define PATH_SEPARATOR "/"

#endif /* !WIN32 */

#ifdef __cplusplus
extern "C" {
#endif
  extern int boinc_delete_file(const char*);
  extern int clean_out_dir(const char*);
  extern FILE* boinc_fopen(const char* path, const char* mode);
  extern int boinc_copy(const char* orig, const char* newf);
  extern int boinc_rename(const char* old, const char* newf);
  extern int boinc_mkdir(const char*);
  extern int boinc_rmdir(const char*);
  extern int lock_file(char*);
  extern void relative_to_absolute(char* relname, char* path);
  extern int boinc_make_dirs(char*, char*);
  extern char boinc_failed_file[256];
  int is_dir(const char* path);

  /* we suitably defined 'bool' as 'int' in ANSI-C */
  extern bool boinc_file_exists(const char* path);

#ifdef __cplusplus
}
#endif

/* C++ specific prototypes/defines follow here */
#ifdef __cplusplus
extern int file_size(const char*, double&);
extern int dir_size(const char* dirpath, double&);
extern int get_filesystem_info(double& total, double& free);


// TODO TODO TODO
// remove this code - the DirScanner class does the same thing.
// But need to rewrite a couple of places that use it
//
extern DIRREF dir_open(const char*);
extern int dir_scan(char*, DIRREF, int);
int dir_scan(std::string&, DIRREF);
extern void dir_close(DIRREF);


class DirScanner {
#ifdef _WIN32
    std::string dir;
    bool first;
    void* handle;
#else
    DIR* dirp;
#endif
public:
    DirScanner(std::string const& path);
    ~DirScanner();
    bool scan(std::string& name);    // return true if file returned
};

#ifndef _WIN32

// search PATH, find the directory that a program is in, if any
//
extern int get_file_dir(char* filename, char* dir);

#endif


#endif /* c++ */

#endif /* double-inclusion protection */

