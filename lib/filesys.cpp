// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2008 University of California
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

#if   defined(_WIN32) && !defined(__STDWX_H__)
#include "boinc_win.h"
#elif defined(_WIN32) && defined(__STDWX_H__)
#include "stdwx.h"
#define MAXPATHLEN 4096
#endif

#if defined(__MINGW32__)
#include <fcntl.h>
#endif

#ifdef _MSC_VER
#define getcwd  _getcwd
#endif

#if !defined(_WIN32) || defined(__CYGWIN32__)
#include "config.h"
#ifdef _USING_FCGI_
#include "boinc_fcgi.h"
#else
#include <cstdio>
#endif
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

#if HAVE_SYS_RESOURCE_H
#include <sys/resource.h>
#endif
#if HAVE_SYS_MOUNT_H
#if HAVE_SYS_PARAM_H
#include <sys/param.h>
#endif
#include <sys/mount.h>
#endif

#if HAVE_SYS_STATVFS_H
#include <sys/statvfs.h>
#define STATFS statvfs
#elif HAVE_SYS_STATFS_H
#include <sys/statfs.h>
#define STATFS statfs
#else
#define STATFS statfs
#endif
#endif

#include "util.h"
#include "str_util.h"
#include "str_replace.h"
#include "error_numbers.h"
#include "filesys.h"

#ifdef _WIN32
typedef BOOL (CALLBACK* FreeFn)(LPCSTR, PULARGE_INTEGER, PULARGE_INTEGER, PULARGE_INTEGER);
#endif

using std::string;

char boinc_failed_file[256];

// routines for enumerating the entries in a directory

int is_file(const char* path) {
    struct stat sbuf;
#ifdef _WIN32
    int retval = stat(path, &sbuf);
#else
    int retval = lstat(path, &sbuf);
#endif
    return (!retval && (((sbuf.st_mode) & S_IFMT) == S_IFREG));
}

int is_dir(const char* path) {
    struct stat sbuf;
#ifdef _WIN32
    int retval = stat(path, &sbuf);
#else
    int retval = lstat(path, &sbuf);
#endif
    return (!retval && (((sbuf.st_mode) & S_IFMT) == S_IFDIR));
}

int is_dir_follow_symlinks(const char* path) {
    struct stat sbuf;
    int retval = stat(path, &sbuf);
    return (!retval && (((sbuf.st_mode) & S_IFMT) == S_IFDIR));
}

#ifndef _WIN32
int is_symlink(const char* path) {
    struct stat sbuf;
    int retval = lstat(path, &sbuf);
    return (!retval && S_ISLNK(sbuf.st_mode));
}
#endif

// Open a directory
//
DIRREF dir_open(const char* p) {
    DIRREF dirp;
#ifdef _WIN32
    if (!is_dir(p)) return NULL;
    dirp = (DIR_DESC*) calloc(sizeof(DIR_DESC), 1);
    if (!dirp) {
        fprintf(stderr, "calloc() failed in dir_open()\n");
        return NULL;
    }
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
    WIN32_FIND_DATAA data;
    while (1) {
        if (dirp->first) {
            dirp->first = false;
            dirp->handle = FindFirstFileA(dirp->path, &data);
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
            if (FindNextFileA(dirp->handle, &data)) {
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

bool is_dir_empty(const char *p) {
    char file[256];

    DIRREF dir = dir_open(p);
    if (!dir) return true;

    bool retval = (dir_scan(file, dir, sizeof(file)) != 0);
    dir_close(dir);
    return retval;
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
    WIN32_FIND_DATAA data;
    while (1) {
        if (first) {
            first = false;
            handle = FindFirstFileA(dir.c_str(), &data);
            if (handle == INVALID_HANDLE_VALUE) {
                return false;
            } else {
                if (data.cFileName[0] == '.') continue;
                s = data.cFileName;
                return true;
            }
        } else {
            if (FindNextFileA(handle, &data)) {
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
    if (!DeleteFileA(path)) {
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
    int retval;

#if defined(_WIN32) && !defined(__CYGWIN32__) && !defined(__MINGW32__)
    struct __stat64 sbuf;
    retval = _stat64(path, &sbuf);
#else
    struct stat sbuf;
    retval = stat(path, &sbuf);
#endif
    if (retval) return ERR_NOT_FOUND;
    size = (double)sbuf.st_size;
    return 0;
}

int boinc_truncate(const char* path, double size) {
    int retval;
#if defined(_WIN32) && !defined(__CYGWIN32__)
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

// remove everything from specified directory
//
int clean_out_dir(const char* dirpath) {
    char filename[256], path[MAXPATHLEN];
    int retval;
    DIRREF dirp;

    dirp = dir_open(dirpath);
    if (!dirp) return 0;    // if dir doesn't exist, it's empty
    while (1) {
        strcpy(filename, "");
        retval = dir_scan(filename, dirp, sizeof(filename));
        if (retval) break;
        sprintf(path, "%s/%s", dirpath,  filename);
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

// return total size of files in directory and optionally its subdirectories
// Win: use special version because stat() is slow, can be avoided
// Unix: follow symbolic links
//
int dir_size(const char* dirpath, double& size, bool recurse) {
#ifdef WIN32
    char path2[_MAX_PATH];
    sprintf(path2, "%s/*", dirpath);
    size = 0.0;
    WIN32_FIND_DATAA findData;
    HANDLE hFind = ::FindFirstFileA(path2, &findData);
    if (INVALID_HANDLE_VALUE == hFind) return ERR_OPENDIR;
    do {
        if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            if (!recurse) continue;
            if (!strcmp(findData.cFileName, ".")) continue;
            if (!strcmp(findData.cFileName, "..")) continue;

            double dsize = 0;
            char buf[_MAX_PATH];
            ::sprintf(buf, "%s/%s", dirpath, findData.cFileName);
            dir_size(buf, dsize, true);
            size += dsize;
        } else {
            size += findData.nFileSizeLow + ((__int64)(findData.nFileSizeHigh) << 32);
        }
    } while (FindNextFileA(hFind, &findData));
	::FindClose(hFind);
#else
    char filename[1024], subdir[1024];
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
        } else if (is_file(subdir)) {
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
    // if opening for read, and file isn't there,
    // leave now (avoid 5-second delay!!)
    //
    if (strchr(mode, 'r')) {
        if (!boinc_file_exists(path)) {
            return 0;
        }
    }
#ifndef _USING_FCGI_
    FILE *f = fopen(path, mode);
#else
    FCGI_FILE *f = FCGI::fopen(path,mode);
#endif

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
#ifndef _USING_FCGI_
            f = fopen(path, mode);
#else
            f = FCGI::fopen(path, mode);
#endif
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

// same, but doesn't traverse symlinks
//
int boinc_file_or_symlink_exists(const char* path) {
    struct stat buf;
#ifdef _WIN32
    if (stat(path, &buf)) {
#else
    if (lstat(path, &buf)) {
#endif
        return false;     // stat() returns zero on success
    }
    return true;
}

// returns zero on success, nonzero if didn't touch file
//
int boinc_touch_file(const char *path) {

    if (boinc_file_exists(path)) {
        return 0;
    }
#ifndef _USING_FCGI_
    FILE *fp = fopen(path, "w");
#else
    FCGI_FILE *fp = FCGI::fopen(path, "w");
#endif
    if (fp) {
        fclose(fp);
        return 0;
    }
    return -1;
}

int boinc_copy(const char* orig, const char* newf) {
#ifdef _WIN32
    if (!CopyFileA(orig, newf, FALSE)) {     // FALSE means overwrite OK
        return GetLastError();
    }
    return 0;
#elif defined(__EMX__)
    char cmd[1024];
    sprintf(cmd, "copy \"%s\" \"%s\"", orig, newf);
    return system(cmd);
#else
    // POSIX requires that shells run from an application will use the 
    // real UID and GID if different from the effective UID and GID.  
    // Mac OS 10.4 did not enforce this, but OS 10.5 does.  Since 
    // system() invokes a shell, it may not properly copy the file's 
    // ownership or permissions when called from the BOINC Client 
    // under sandbox security, so we copy the file directly.
    FILE *src, *dst;
    int m, n;
    int retval = 0;
    struct stat sbuf;
    unsigned char buf[65536];
    src = boinc_fopen(orig, "r");
    if (!src) return ERR_FOPEN;
    dst = boinc_fopen(newf, "w");
    if (!dst) {
        fclose(src);
        return ERR_FOPEN;
    }
    while (1) {
        n = fread(buf, 1, sizeof(buf), src);
        if (n <= 0) break;
        m = fwrite(buf, 1, n, dst);
        if (m != n) {
            retval = ERR_FWRITE;
            break;
        }
    }
    fclose(src);
    fclose(dst);
    // Copy file's ownership, permissions to the extent we are allowed
    lstat(orig, &sbuf);             // Get source file's info
    chown(newf, sbuf.st_uid, sbuf.st_gid);
    chmod(newf, sbuf.st_mode);
    return retval;
#endif
}

static int boinc_rename_aux(const char* old, const char* newf) {
#ifdef _WIN32
    if (MoveFileExA(old, newf, MOVEFILE_REPLACE_EXISTING|MOVEFILE_WRITE_THROUGH)) return 0;
    return GetLastError();
#else
    int retval = rename(old, newf);
    if (retval) return ERR_RENAME;
    return 0;
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
    if (!CreateDirectoryA(path, NULL)) {
        return GetLastError();
    }
#else
    mode_t old_mask = umask(0);
    int retval = mkdir(path, 0771);
    umask(old_mask);
    if (retval) return ERR_MKDIR;
#endif
    return 0;
}

int boinc_rmdir(const char* name) {
#ifdef _WIN32
    if (!RemoveDirectoryA(name)) {
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
    strcpy(oldpath, dirpath);

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
    locked = false;
}

FILE_LOCK::~FILE_LOCK() {
#ifndef _WIN32
    if (fd >= 0) close(fd);
#endif
}

int FILE_LOCK::lock(const char* filename) {
#if defined(_WIN32) && !defined(__CYGWIN32__)
    handle = CreateFileA(
        filename, GENERIC_WRITE,
        0, 0, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0
    );
    if (handle == INVALID_HANDLE_VALUE) {
        return GetLastError();
    }
#else
    if (fd<0) {
        fd = open(filename, O_WRONLY|O_CREAT, S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH);
    }
    if (fd<0) {
        return ERR_OPEN;
    }

    struct flock fl;
    fl.l_type = F_WRLCK;
    fl.l_whence = SEEK_SET;
    fl.l_start = 0;
    fl.l_len = 0;
    if (fcntl(fd, F_SETLK, &fl) == -1) {
        return ERR_FCNTL;
    }
#endif
    locked = true;
    return 0;
}

int FILE_LOCK::unlock(const char* filename) {
#if defined(_WIN32) && !defined(__CYGWIN32__)
    CloseHandle(handle);
#else
    close(fd);
    fd = -1;
#endif
    boinc_delete_file(filename);
    locked = false;
    return 0;
}

void boinc_getcwd(char* path) {
#ifdef _WIN32
    getcwd(path, 256);
#else
    char* p 
#ifdef __GNUC__
      __attribute__ ((unused))
#endif
      = getcwd(path, 256);
#endif
}

void relative_to_absolute(const char* relname, char* path) {
    boinc_getcwd(path);
    if (strlen(relname)) {
        strcat(path, "/");
        strcat(path, relname);
    }
}

// get total and free space on current filesystem (in bytes)
//
#ifdef _WIN32
int get_filesystem_info(double &total_space, double &free_space, char*) {
    char buf[256];
    boinc_getcwd(buf);
    FreeFn pGetDiskFreeSpaceEx;
    pGetDiskFreeSpaceEx = (FreeFn)GetProcAddress(
        GetModuleHandleA("kernel32.dll"), "GetDiskFreeSpaceExA"
    );
    if (pGetDiskFreeSpaceEx) {
        ULARGE_INTEGER TotalNumberOfFreeBytes;
        ULARGE_INTEGER TotalNumberOfBytes;
        ULARGE_INTEGER FreeBytesAvailable;
        pGetDiskFreeSpaceEx(
            buf, &FreeBytesAvailable, &TotalNumberOfBytes,
            &TotalNumberOfFreeBytes
        );
        signed __int64 uMB;
        uMB = FreeBytesAvailable.QuadPart / (1024 * 1024);
        free_space = uMB * 1024.0 * 1024.0;
        uMB = TotalNumberOfBytes.QuadPart / (1024 * 1024);
        total_space = uMB * 1024.0 * 1024.0;
    } else {
        DWORD dwSectPerClust;
        DWORD dwBytesPerSect;
        DWORD dwFreeClusters;
        DWORD dwTotalClusters;
        GetDiskFreeSpaceA(
            buf, &dwSectPerClust, &dwBytesPerSect, &dwFreeClusters,
            &dwTotalClusters
        );
        free_space = (double)dwFreeClusters * dwSectPerClust * dwBytesPerSect;
        total_space = (double)dwTotalClusters * dwSectPerClust * dwBytesPerSect;
    }
#else
int get_filesystem_info(double &total_space, double &free_space, char* path) {
#ifdef STATFS
    struct STATFS fs_info;

    STATFS(path, &fs_info);
#if HAVE_SYS_STATVFS_H
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
    char buf[8192], *p, path[MAXPATHLEN];
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

