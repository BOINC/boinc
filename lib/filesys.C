// Berkeley Open Infrastructure for Network Computing
// http://boinc.berkeley.edu
// Copyright (C) 2005 University of California
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

#if defined(_WIN32) && !defined(__STDWX_H__) && !defined(_BOINC_WIN_) && !defined(_AFX_STDAFX_H_)
#include "boinc_win.h"
#endif

#if !defined(_WIN32) || defined(__CYGWIN32__)
#include "config.h"
#include <cstdio>
#include <fcntl.h>
#include <cerrno>
#include <sys/stat.h>
#include <sys/file.h>
#include <ctime>
#include <cstring>
#include <cstdlib>

#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_DIRENT_H
#include <dirent.h>
#endif
#ifdef HAVE_SYS_RESOURCE_H
#include <sys/resource.h>
#endif
#ifdef HAVE_SYS_MOUNT_H
#ifdef HAVE_SYS_PARAM_H
#include <sys/param.h>
#endif
#include <sys/mount.h>
#endif

#ifdef HAVE_SYS_STATVFS_H
#include <sys/statvfs.h>
#define STATFS statvfs
#elif defined(HAVE_SYS_STATFS_H)
#include <sys/statfs.h>
#define STATFS statfs
#else
#define STATFS statfs
#endif
#endif

#ifdef _WIN32
typedef BOOL (CALLBACK* FreeFn)(LPCTSTR, PULARGE_INTEGER, PULARGE_INTEGER, PULARGE_INTEGER);
#endif

#include "util.h"
#include "error_numbers.h"
#include "filesys.h"

#ifdef _USING_FCGI_
#include "fcgi_stdio.h"
#endif

using std::string;

char boinc_failed_file[256];

// routines for enumerating the entries in a directory

bool is_file(const char* path) {
    struct stat sbuf;
    int retval = stat(path, &sbuf);
    return (!retval && (sbuf.st_mode & S_IFREG));
}

bool is_dir(const char* path) {
    struct stat sbuf;
    int retval = stat(path, &sbuf);
    return (!retval && (sbuf.st_mode & S_IFDIR));
}

// Open a directory
//
DIRREF dir_open(const char* p) {
    DIRREF dirp;

#ifdef HAVE_DIRENT_H
    dirp = opendir(p);
    if (!dirp) return NULL;
#elif defined(_WIN32)
    if (!is_dir(p)) return NULL;
    dirp = (DIR_DESC*) calloc(sizeof(DIR_DESC), 1);
    dirp->first = true;
    safe_strcpy(dirp->path, p);
    strcat(dirp->path, "\\*");
    dirp->handle = INVALID_HANDLE_VALUE;
#endif
    return dirp;
}

// Scan through a directory and return the next file name in it
//
int dir_scan(char* p, DIRREF dirp, int p_len) {
#ifdef HAVE_DIRENT_H
    while (1) {
        dirent* dp = readdir(dirp);
        if (dp) {
            if (!strcmp(dp->d_name, ".")) continue;
            if (!strcmp(dp->d_name, "..")) continue;
            if (p) safe_strncpy(p, dp->d_name, p_len);
            return 0;
        } else {
            return ERR_READDIR;
        }
    }
#elif defined(_WIN32)
    WIN32_FIND_DATA data;
    while (1) {
        if (dirp->first) {
            dirp->first = false;
            dirp->handle = FindFirstFile(dirp->path, &data);
            if (dirp->handle == INVALID_HANDLE_VALUE) {
                return ERR_READDIR;
            } else {
                // does Windows have "." and ".."?  well, just in case.
                //
                if (!strcmp(data.cFileName, ".")) continue;
                if (!strcmp(data.cFileName, "..")) continue;
                if (p) safe_strncpy(p, data.cFileName, p_len);
                return 0;
            }
        } else {
            if (FindNextFile(dirp->handle, &data)) {
                if (!strcmp(data.cFileName, ".")) continue;
                if (!strcmp(data.cFileName, "..")) continue;
                if (p) safe_strncpy(p, data.cFileName, p_len);
                return 0;
            } else {
                FindClose(dirp->handle);
                dirp->handle = INVALID_HANDLE_VALUE;
                return 1;
            }
        }
    }
#endif
}

// Close a directory
//
void dir_close(DIRREF dirp) {
#ifdef HAVE_DIRENT_H
    if (dirp) {
        closedir(dirp);
    }
#elif defined(_WIN32)
    if (dirp->handle != INVALID_HANDLE_VALUE) {
        FindClose(dirp->handle);
        dirp->handle = INVALID_HANDLE_VALUE;
    }
    free(dirp);
#endif
}

DirScanner::DirScanner(string const& path) {
#ifdef HAVE_DIRENT_H
    dirp = opendir(path.c_str());
#elif defined(_WIN32)
    first = true;
    handle = INVALID_HANDLE_VALUE;
    if (!is_dir((char*)path.c_str())) {
        return;
    }
    dir = path + "\\*";
#endif
}

// Scan through a directory and return the next file name in it
//
bool DirScanner::scan(string& s) {
#ifdef HAVE_DIRENT_H
    if (!dirp) return false;

    while (1) {
        dirent* dp = readdir(dirp);
        if (dp) {
            if (dp->d_name[0] == '.') continue;
            s = dp->d_name;
            return true;
        } else {
            return false;
        }
    }
#elif defined(_WIN32)
    WIN32_FIND_DATA data;
    while (1) {
        if (first) {
            first = false;
            handle = FindFirstFile(dir.c_str(), &data);
            if (handle == INVALID_HANDLE_VALUE) {
                return false;
            } else {
                if (data.cFileName[0] == '.') continue;
                s = data.cFileName;
                return true;
            }
        } else {
            if (FindNextFile(handle, &data)) {
                if (data.cFileName[0] == '.') continue;
                s = data.cFileName;
                return true;
            } else {
                FindClose(handle);
                handle = INVALID_HANDLE_VALUE;
                return false;
            }
        }
    }
#endif
}

// Close a directory
DirScanner::~DirScanner() {
#ifdef HAVE_DIRENT_H
    if (dirp) {
        closedir(dirp);
    }
#elif defined(_WIN32)
    if (handle != INVALID_HANDLE_VALUE) {
        FindClose(handle);
    }
#endif
}


// Delete the file located at path
//
int boinc_delete_file(const char* path) {
    int retval = 0;

    if (!boinc_file_exists(path)) {
        return 0;
    }
#ifdef _WIN32
    for (int i=0; i<5; i++) {
        retval = remove(path);
        if (!retval) break;
        boinc_sleep(drand());       // avoid lockstep
    }
#else
    retval = unlink(path);
#endif
    if (retval) {
        safe_strcpy(boinc_failed_file, path);
        return ERR_UNLINK;
    }
    return 0;
}

// get file size
//
int file_size(const char* path, double& size) {
    struct stat sbuf;
    int retval;

    retval = stat(path, &sbuf);
    if (retval) return ERR_NOT_FOUND;
    size = (double)sbuf.st_size;
    return 0;
}

// removes all files from specified directory
//
int clean_out_dir(const char* dirpath) {
    char filename[256], path[256];
    int retval;
    DIRREF dirp;

    dirp = dir_open(dirpath);
    if (!dirp) return 0;    // if dir doesn't exist, it's empty
    while (1) {
        strcpy(filename, "");
        retval = dir_scan(filename, dirp, sizeof(filename));
        if (retval) break;
        sprintf(path, "%s%s%s", dirpath, PATH_SEPARATOR, filename);
        clean_out_dir(path);
        boinc_rmdir(path);
        retval = boinc_delete_file(path);
        if (retval) {
            dir_close(dirp);
            return retval;
        }
    }
    dir_close(dirp);
    return 0;
}

// return total size of files in directory and its subdirectories
//
int dir_size(const char* dirpath, double& size) {
    char filename[256], subdir[256];
    int retval=0;
    DIRREF dirp;
    double x;

    size = 0;
    dirp = dir_open(dirpath);
    if (!dirp) return ERR_OPENDIR;
    while (1) {
        retval = dir_scan(filename, dirp, sizeof(filename));
        if (retval) break;
        sprintf(subdir, "%s%s%s", dirpath, PATH_SEPARATOR, filename);

        // We don't know if this entry is a file or a directory.
        // dir_size() will return -1 if it's a file
        //
        retval = dir_size(subdir, x);
        if (retval == 0) {
            size += x;
        } else {
            retval = file_size(subdir, x);
            if (retval) continue;
            size += x;
        }
    }
    dir_close(dirp);
    return 0;
}

FILE* boinc_fopen(const char* path, const char* mode) {
    FILE* f;

    // if opening for read, and file isn't there,
    // leave now (avoid 5-second delay!!)
    //
    if (strchr(mode, 'r')) {
        if (!boinc_file_exists(path)) {
            return 0;
        }
    }
    f = fopen(path, mode);
#ifdef _WIN32
    // on Windows: if fopen fails, try again for 5 seconds
    // (since the file might be open by FastFind, Diskeeper etc.)
    //
    if (!f) {
        for (int i=0; i<5; i++) {
            boinc_sleep(drand());
            f = fopen(path, mode);
            if (f) break;
        }
    }
#else
    // Unix - if call was interrupted, retry a few times
    //
    if (!f) {
        for (int i=0; i<5; i++) {
            if (errno != EINTR) break;
            f = fopen(path, mode);
            if (f) break;
        }
    }
#endif
    return f;
}


bool boinc_file_exists(const char* path) {
   struct stat buf;

   if (stat(path, &buf)) {
       return false;     // stat() returns zero on success
   }
   return true;
}

// returns zero on success, nonzero if didn't touch file
//
int boinc_touch_file(const char *path) {
    FILE *fp;
                                                                                                                                                                               
    if (boinc_file_exists(path)) {
        return 0;
    }
                                                                                                                                                                               
    if ((fp=fopen(path, "w"))) {
        fclose(fp);
        return 0;
    }
                                                                                                                                                                               
    return -1;
}

int boinc_copy(const char* orig, const char* newf) {
#ifdef _WIN32
    if (CopyFile(orig, newf, FALSE)) {
        return 0;
    } else {
        return GetLastError();
    }
#elif defined(__EMX__)
    char cmd[256];
    sprintf(cmd, "copy %s %s", orig, newf);
    return system(cmd);
#else
    char cmd[256];
    sprintf(cmd, "cp %s %s", orig, newf);
    return system(cmd);
#endif
}

int boinc_rename(const char* old, const char* newf) {
#ifdef _WIN32
    int retval=0;
    boinc_delete_file(newf);
    for (int i=0; i<5; i++) {
        retval = rename(old, newf);
        if (!retval) break;
        boinc_sleep(drand());       // avoid lockstep
    }
    return retval;
#else
    return rename(old, newf);
#endif
}

int boinc_mkdir(const char* path) {
    if (is_dir(path)) return 0;
#ifdef _WIN32
    return !CreateDirectory(path, NULL);
#else
    return mkdir(path, 0777);
#endif
}

int boinc_rmdir(const char* name) {
#ifdef _WIN32
    return !RemoveDirectory(name);
#else
    return rmdir(name);
#endif
}

// if "filepath" is of the form a/b/c,
// create directories dirpath/a, dirpath/a/b etc.
//
int boinc_make_dirs(const char* dirpath, const char* filepath) {
    char buf[1024], oldpath[1024], newpath[1024];
    int retval;
    char *p, *q;

    if (strlen(filepath) + strlen(dirpath) > 1023) return ERR_BUFFER_OVERFLOW;
    strcpy(buf, filepath);

    q = buf;
    while(*q) {
        p = strchr(q, '/');
        if (!p) break;
        *p = 0;
        sprintf(newpath, "%s%s%s", oldpath, PATH_SEPARATOR, q);
        retval = boinc_mkdir(newpath);
        if (retval) return retval;
        strcpy(oldpath, newpath);
        q = p+1;
    }
    return 0;
}


int FILE_LOCK::lock(const char* filename) {
    int retval=0;
#if defined(_WIN32) && !defined(__CYGWIN32__)
    handle = CreateFile(
        filename, GENERIC_WRITE,
        0, 0, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0
    );
    if (handle == INVALID_HANDLE_VALUE) {
        retval = 1;
    }

    // some systems have both!
#elif defined(HAVE_LOCKF) && !defined(__APPLE__)
    fd = open(filename, O_WRONLY|O_CREAT, 0644);
    retval = lockf(fd, F_TLOCK, 0);
#elif HAVE_FLOCK
    fd = open(filename, O_WRONLY|O_CREAT, 0644);
    retval = flock(fd, LOCK_EX|LOCK_NB);
    // must leave file-handle open
#else
    no file lock mechanism;
#endif

    return retval;
}

int FILE_LOCK::unlock(const char* filename) {
#if defined(_WIN32) && !defined(__CYGWIN32__)
    if (!CloseHandle(handle)) {
        perror("FILE_LOCK::unlock(): close failed.");
    }
#else
    if (close(fd)) {
        perror("FILE_LOCK::unlock(): close failed.");
    }
#endif
    if (boinc_delete_file(filename) != 0) {
        perror("FILE_LOCK::unlock: delete failed.");
    }
    return 0;
}

void relative_to_absolute(const char* relname, char* path) {
#if defined(_WIN32) && !defined(__CYGWIN32__)
    _getcwd(path, 256);
#else
    getcwd(path, 256);
#endif
    if (strlen(relname)) {
        strcat(path, PATH_SEPARATOR);
        strcat(path, relname);
    }
}

// get total and free space on current filesystem (in bytes)
//
int get_filesystem_info(double &total_space, double &free_space) {
#ifdef _WIN32
    FreeFn pGetDiskFreeSpaceEx;
    pGetDiskFreeSpaceEx = (FreeFn)GetProcAddress(GetModuleHandle("kernel32.dll"),
                                                 "GetDiskFreeSpaceExA");
    if (pGetDiskFreeSpaceEx) {
        ULARGE_INTEGER TotalNumberOfFreeBytes;
        ULARGE_INTEGER TotalNumberOfBytes;
        ULARGE_INTEGER TotalNumberOfBytesFreeToCaller;
        pGetDiskFreeSpaceEx(NULL, &TotalNumberOfBytesFreeToCaller, &TotalNumberOfBytes, &TotalNumberOfFreeBytes);
        signed __int64 uMB;
        uMB = TotalNumberOfFreeBytes.QuadPart / (1024 * 1024);
        free_space = uMB * 1024.0 * 1024.0;
        uMB = TotalNumberOfBytes.QuadPart / (1024 * 1024);
        total_space = uMB * 1024.0 * 1024.0;
    } else {
        DWORD dwSectPerClust;
        DWORD dwBytesPerSect;
        DWORD dwFreeClusters;
        DWORD dwTotalClusters;
        GetDiskFreeSpace(NULL, &dwSectPerClust, &dwBytesPerSect, &dwFreeClusters, &dwTotalClusters);
        free_space = (double)dwFreeClusters * dwSectPerClust * dwBytesPerSect;
        total_space = (double)dwTotalClusters * dwSectPerClust * dwBytesPerSect;
    }
#else
#ifdef STATFS
    struct STATFS fs_info;

    STATFS(".", &fs_info);
#ifdef HAVE_SYS_STATVFS_H
    total_space = (double)fs_info.f_frsize * (double)fs_info.f_blocks;
    free_space = (double)fs_info.f_frsize * (double)fs_info.f_bavail;
#else
    total_space = (double)fs_info.f_bsize * (double)fs_info.f_blocks;
    free_space = (double)fs_info.f_bsize * (double)fs_info.f_bavail;
#endif
#else
#error Need to specify a method to obtain free/total disk space
#endif
#endif
    return 0;
}

#ifndef _WIN32

int get_file_dir(char* filename, char* dir) {
    char buf[8192], *p, path[256];
    struct stat sbuf;
    int retval;

    p = getenv("PATH");
    if (!p) return ERR_NOT_FOUND;
    strcpy(buf, p);

    p = strtok(buf, ":");
    while (p) {
        sprintf(path, "%s/%s", p, filename);
        retval = stat(path, &sbuf);
        if (!retval && (sbuf.st_mode & 0111)) {
            strcpy(dir, p);
            return 0;
        }
        p = strtok(0, ":");
    }
    return ERR_NOT_FOUND;
}


#endif

const char *BOINC_RCSID_636c8d709b = "$Id$";
