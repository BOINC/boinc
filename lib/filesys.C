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
// 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

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
#include <sys/time.h>
#include <unistd.h>
#include <dirent.h>

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
#include "str_util.h"
#include "error_numbers.h"
#include "filesys.h"

#ifdef _USING_FCGI_
#include "fcgi_stdio.h"
#endif


using std::string;

char boinc_failed_file[256];

// routines for enumerating the entries in a directory

int is_file(const char* path) {
    struct stat sbuf;
    int retval = stat(path, &sbuf);
    return (!retval && (sbuf.st_mode & S_IFREG));
}

int is_dir(const char* path) {
    struct stat sbuf;
    int retval = stat(path, &sbuf);
    return (!retval && (sbuf.st_mode & S_IFDIR));
}

// Open a directory
//
DIRREF dir_open(const char* p) {
    DIRREF dirp;
#ifdef _WIN32
    if (!is_dir(p)) return NULL;
    dirp = (DIR_DESC*) calloc(sizeof(DIR_DESC), 1);
    dirp->first = true;
    safe_strcpy(dirp->path, p);
    strcat(dirp->path, "\\*");
    dirp->handle = INVALID_HANDLE_VALUE;
#else
    dirp = opendir(p);
    if (!dirp) return NULL;
#endif
    return dirp;
}

// Scan through a directory and return the next file name in it
//
int dir_scan(char* p, DIRREF dirp, int p_len) {
#ifdef _WIN32
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
                if (p) strlcpy(p, data.cFileName, p_len);
                return 0;
            }
        } else {
            if (FindNextFile(dirp->handle, &data)) {
                if (!strcmp(data.cFileName, ".")) continue;
                if (!strcmp(data.cFileName, "..")) continue;
                if (p) strlcpy(p, data.cFileName, p_len);
                return 0;
            } else {
                FindClose(dirp->handle);
                dirp->handle = INVALID_HANDLE_VALUE;
                return 1;
            }
        }
    }
#else
    while (1) {
        dirent* dp = readdir(dirp);
        if (dp) {
            if (!strcmp(dp->d_name, ".")) continue;
            if (!strcmp(dp->d_name, "..")) continue;
            if (p) strlcpy(p, dp->d_name, p_len);
            return 0;
        } else {
            return ERR_READDIR;
        }
    }
#endif
}

// Close a directory
//
void dir_close(DIRREF dirp) {
#ifdef _WIN32
    if (dirp->handle != INVALID_HANDLE_VALUE) {
        FindClose(dirp->handle);
        dirp->handle = INVALID_HANDLE_VALUE;
    }
    free(dirp);
#else
    if (dirp) {
        closedir(dirp);
    }
#endif
}

DirScanner::DirScanner(string const& path) {
#ifdef _WIN32
    first = true;
    handle = INVALID_HANDLE_VALUE;
    if (!is_dir((char*)path.c_str())) {
        return;
    }
    dir = path + "\\*";
#else
    dirp = opendir(path.c_str());
#endif
}

// Scan through a directory and return the next file name in it
//
bool DirScanner::scan(string& s) {
#ifdef _WIN32
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
#else
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
#endif
}

DirScanner::~DirScanner() {
#ifdef _WIN32
    if (handle != INVALID_HANDLE_VALUE) {
        FindClose(handle);
    }
#else
    if (dirp) {
        closedir(dirp);
    }
#endif
}

static int boinc_delete_file_aux(const char* path) {
#ifdef _WIN32
    if (!DeleteFile(path)) {
        return ERR_UNLINK;
    }
#else
    int retval = unlink(path);
    if (retval) return ERR_UNLINK;
#endif
    return 0;
}

// Delete the file located at path
//
int boinc_delete_file(const char* path) {
    int retval = 0;

    if (!boinc_file_exists(path)) {
        return 0;
    }
    retval = boinc_delete_file_aux(path);
    if (retval) {
        double start = dtime();
        do {
            boinc_sleep(drand()*2);       // avoid lockstep
            retval = boinc_delete_file_aux(path);
            if (!retval) break;
        } while (dtime() < start + FILE_RETRY_INTERVAL);
    }
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

int boinc_truncate(const char* path, double size) {
    int retval;
#if defined(_WIN32) &&  !defined(__CYGWIN32__)
    // the usual Windows nightmare.
    // There's another function, SetEndOfFile(),
    // that supposedly works with files over 2GB,
    // but it uses HANDLES
    //
    int fd = _open(path, _O_RDWR, 0);
    if (fd == -1) return ERR_TRUNCATE;
    retval = _chsize(fd, (long)size);
    _close(fd);
#else
    retval = truncate(path, (off_t)size);
#endif
    if (retval) return ERR_TRUNCATE;
    return 0;
}


// return total size of files in directory and its subdirectories
// Special version for Win because stat() is slow, can be avoided
//
int dir_size(const char* dirpath, double& size, bool recurse) {
#ifdef WIN32
    char path2[_MAX_PATH];
    sprintf(path2, "%s/*", dirpath);
    size = 0.0;
    WIN32_FIND_DATA findData;
    HANDLE hFind = ::FindFirstFile(path2, &findData);
    if (INVALID_HANDLE_VALUE != hFind) {
        do {
            if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
                if (!recurse) continue;
                if (!strcmp(findData.cFileName, ".")) continue;
                if (!strcmp(findData.cFileName, "..")) continue;

                double dsize = 0;
                char buf[_MAX_PATH];
                ::sprintf(buf, "%s/%s", dirpath, findData.cFileName);
                dir_size(buf, dsize, recurse);
                size += dsize;
            } else {
                size += findData.nFileSizeLow + ((__int64)(findData.nFileSizeHigh) << 32);
            }
        } while (FindNextFile(hFind, &findData));
		::FindClose(hFind);
    }  else {
        return ERR_OPENDIR;
    }
#else
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
        sprintf(subdir, "%s/%s", dirpath, filename);

        if (is_dir(subdir)) {
            if (recurse) {
                retval = dir_size(subdir, x);
                if (retval) continue;
                size += x;
            }
        } else {
            retval = file_size(subdir, x);
            if (retval) continue;
            size += x;
        }
    }
    dir_close(dirp);
#endif
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
        double start = dtime();
        do {
            boinc_sleep(drand()*2);
            f = _fsopen(path, mode, _SH_DENYNO);
                // _SH_DENYNO makes the file sharable while open
            if (f) break;
        } while (dtime() < start + FILE_RETRY_INTERVAL);
    }
#else
    // Unix - if call was interrupted, retry a few times
    //
    if (!f) {
        for (int i=0; i<5; i++) {
            boinc_sleep(drand());
            if (errno != EINTR) break;
            f = fopen(path, mode);
            if (f) break;
        }
    }
    if (f) {
        fcntl(fileno(f), F_SETFD, FD_CLOEXEC);
    }
#endif
    return f;
}


int boinc_file_exists(const char* path) {
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
    fp = fopen(path, "w");
    if (fp) {
        fclose(fp);
        return 0;
    }
    return -1;
}

int boinc_copy(const char* orig, const char* newf) {
#ifdef _WIN32
    if (!CopyFile(orig, newf, FALSE)) {     // FALSE means overwrite OK
        return GetLastError();
    }
    return 0;
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

static int boinc_rename_aux(const char* old, const char* newf) {
#ifdef _WIN32
    boinc_delete_file(newf);
    if (MoveFile(old, newf)) return 0;
    return GetLastError();
#else
    return rename(old, newf);
#endif
}

int boinc_rename(const char* old, const char* newf) {
    int retval=0;

    retval = boinc_rename_aux(old, newf);
    if (retval) {
        double start = dtime();
        do {
            boinc_sleep(drand()*2);       // avoid lockstep
            retval = boinc_rename_aux(old, newf);
            if (!retval) break;
        } while (dtime() < start + FILE_RETRY_INTERVAL);
    }
    return retval;
}

// make a dir that's owner and group RWX
//
int boinc_mkdir(const char* path) {
    if (is_dir(path)) return 0;
#ifdef _WIN32
    if (!CreateDirectory(path, NULL)) {
        return GetLastError();
    }
    return 0;
#else
    mode_t old_mask = umask(0);
    int retval = mkdir(path, 0771);
    umask(old_mask);
    return retval;
#endif
}

int boinc_rmdir(const char* name) {
#ifdef _WIN32
    if (!RemoveDirectory(name)) {
        return ERR_RMDIR;
    }
#else
    int retval = rmdir(name);
    if (retval) return ERR_RMDIR;
#endif
    return 0;
}

#ifndef _WIN32
int boinc_chown(const char* path, gid_t gid) {
    if (gid) {
        if (chown(path, (uid_t)-1, gid)) {
            return ERR_CHOWN;
        }
    }
    return 0;
}
#endif

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
        sprintf(newpath, "%s/%s", oldpath, q);
        retval = boinc_mkdir(newpath);
        if (retval) return retval;
        strcpy(oldpath, newpath);
        q = p+1;
    }
    return 0;
}


FILE_LOCK::FILE_LOCK() {
#ifndef _WIN32
    fd = -1;
#endif
}
FILE_LOCK::~FILE_LOCK() {
#ifndef _WIN32
    if (fd >= 0) close(fd);
#endif
}

int FILE_LOCK::lock(const char* filename) {
#if defined(_WIN32) && !defined(__CYGWIN32__)
    handle = CreateFile(
        filename, GENERIC_WRITE,
        0, 0, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0
    );
    if (handle == INVALID_HANDLE_VALUE) {
        return -1;
    }
    return 0;

#else
    if (fd<0) {
        fd = open(
            filename, O_WRONLY|O_CREAT, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH
        );
    }
    if (fd<0) {
        return -1;
    }

    struct flock fl;
    fl.l_type=F_WRLCK;
    fl.l_whence=SEEK_SET;
    fl.l_start=0;
    fl.l_len=0;
    if (-1 != fcntl(fd, F_SETLK, &fl)) return 0;
    return -1;
#endif
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
    boinc_delete_file(filename);
    return 0;
}

void boinc_getcwd(char* path) {
    char* p __attribute__ ((unused)) = getcwd(path, 256);
}

void relative_to_absolute(const char* relname, char* path) {
    char* p __attribute__ ((unused)) = getcwd(path, 256);
    if (strlen(relname)) {
        strcat(path, "/");
        strcat(path, relname);
    }
}

// get total and free space on current filesystem (in bytes)
//
int get_filesystem_info(double &total_space, double &free_space, char* path) {
#ifdef _WIN32
    char buf[256];
    boinc_getcwd(buf);
    FreeFn pGetDiskFreeSpaceEx;
    pGetDiskFreeSpaceEx = (FreeFn)GetProcAddress(
        GetModuleHandle("kernel32.dll"), "GetDiskFreeSpaceExA"
    );
    if (pGetDiskFreeSpaceEx) {
        ULARGE_INTEGER TotalNumberOfFreeBytes;
        ULARGE_INTEGER TotalNumberOfBytes;
        ULARGE_INTEGER TotalNumberOfBytesFreeToCaller;
        pGetDiskFreeSpaceEx(
            buf, &TotalNumberOfBytesFreeToCaller, &TotalNumberOfBytes,
            &TotalNumberOfFreeBytes
        );
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
        GetDiskFreeSpace(
            buf, &dwSectPerClust, &dwBytesPerSect, &dwFreeClusters,
            &dwTotalClusters
        );
        free_space = (double)dwFreeClusters * dwSectPerClust * dwBytesPerSect;
        total_space = (double)dwTotalClusters * dwSectPerClust * dwBytesPerSect;
    }
#else
#ifdef STATFS
    struct STATFS fs_info;

    STATFS(path, &fs_info);
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
