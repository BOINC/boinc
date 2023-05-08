// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2023 University of California
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

#ifndef BOINC_FILESYS_H
#define BOINC_FILESYS_H

#if defined(_WIN32) && !defined(__CYGWIN32__)
#include "boinc_win.h"
#else
#include <dirent.h>
#include <grp.h>
#include <sys/types.h>
#include <sys/param.h>
#ifdef __cplusplus
#include <string>
#endif
#endif // WIN32

#include "boinc_stdio.h"

#ifndef MAXPATHLEN
#define MAXPATHLEN 4096
#endif

// use the following in format codes in snprintf
//
#define DIR_LEN 2048
#define FILE_LEN 256

#define FILE_RETRY_INTERVAL 5
    // On Windows, retry for this period of time, since some other program
    // (virus scan, defrag, index) may have the file open.

#ifdef __cplusplus
extern "C" {
#endif
    extern int boinc_delete_file(const char*);
    extern int boinc_touch_file(const char *path);
    extern FILE* boinc_fopen(const char* path, const char* mode);
        // like fopen(), except:
        // retry a few times on failure
        // Unix: set close-on-exec flag
    extern int boinc_copy(const char* orig, const char* newf);
    extern int boinc_rename(const char* old, const char* newf);
    extern int boinc_mkdir(const char*);
#ifdef _WIN32
    extern int boinc_allocate_file(const char*, double size);
#else
    extern int boinc_copy_attributes(const char* orig, const char* newf);
    extern int boinc_chown(const char*, gid_t);
#endif
    extern int boinc_rmdir(const char*);
    extern void boinc_getcwd(char*);
    extern void relative_to_absolute(const char* relname, char* path);
    extern int boinc_make_dirs(const char*, const char*);
    extern char boinc_failed_file[MAXPATHLEN];
    extern int is_file(const char* path);
    extern int is_dir(const char* path);
    extern int is_file_follow_symlinks(const char* path);
    extern int is_dir_follow_symlinks(const char* path);
    extern int is_symlink(const char* path);
    extern int boinc_truncate(const char*, double);
    extern int boinc_file_exists(const char* path);
    extern int boinc_file_or_symlink_exists(const char* path);
#ifdef _WIN32
    extern FILE* boinc_temp_file(
        const char* dir, const char* prefix, char* temp_path, double size
    );
#else
    extern FILE* boinc_temp_file(const char* dir, const char* prefix, char* temp_path);
#endif
    extern void boinc_path_to_dir(const char* path, char* dir);
        // given a file path, get path of its directory
        // (i.e. remove the last / and what follows)

#ifdef __cplusplus
}
#endif

// C++ specific prototypes/defines

#ifdef __cplusplus

extern int file_size(const char*, double&);
extern int file_size_alloc(const char*, double&);
extern int dir_size(const char* dirpath, double&, bool recurse=true);
extern int dir_size_alloc(const char* dirpath, double&, bool recurse=true);
extern int clean_out_dir(const char*);
extern int get_filesystem_info(double& total, double& free, char* path=const_cast<char *>("."));
extern bool is_path_absolute(const std::string path);

// read files into memory.
// Use only for non-binary files; returns null-terminated string.
//
extern int read_file_malloc(
    const char* path, char*& result, size_t max_len=0, bool tail=false
);
extern int read_file_string(
    const char* path, std::string& result, size_t max_len=0, bool tail=false
);


// TODO TODO TODO
// remove this code - the DirScanner class does the same thing.
// But need to rewrite a couple of places that use it
//
#if defined(_WIN32) && !defined(__CYGWIN32__)
typedef struct _DIR_DESC {
    char path[MAXPATHLEN];
    bool first;
    void* handle;
} DIR_DESC;
typedef DIR_DESC *DIRREF;
#else
typedef DIR *DIRREF;
#endif

extern DIRREF dir_open(const char*);
extern int dir_scan(char*, DIRREF, int);
extern int dir_scan(std::string&, DIRREF);
extern void dir_close(DIRREF);

extern bool is_dir_empty(const char*);

class DirScanner {
#if defined(_WIN32) && !defined(__CYGWIN32__)
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

struct FILE_LOCK {
#if defined(_WIN32) && !defined(__CYGWIN32__)
    HANDLE handle;
#else
    int fd;
#endif
    bool locked;
    FILE_LOCK();
    ~FILE_LOCK();
    int lock(const char* filename);
    int unlock(const char* filename);
};

#endif // c++

#endif // double-inclusion protection

