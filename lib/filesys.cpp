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

#if defined(_WIN32)
#include "boinc_win.h"
#define MAXPATHLEN 4096
#endif

#if defined(__MINGW32__)
#include <fcntl.h>
#endif

#if !defined(_WIN32) || defined(__CYGWIN32__)
#include "config.h"
#include <fcntl.h>
#include <cerrno>
#include <sys/stat.h>
#include <sys/file.h>
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

#if defined(ANDROID) && __ANDROID_API__ < 19
    #undef HAVE_SYS_STATVFS_H
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

#include "error_numbers.h"
#include "filesys.h"
#include "str_replace.h"
#include "str_util.h"
#include "util.h"

#ifdef __APPLE__
#include "mac_spawn.h"
#endif

#ifdef _WIN32
typedef BOOL (CALLBACK* FreeFn)(LPCSTR, PULARGE_INTEGER, PULARGE_INTEGER, PULARGE_INTEGER);
#endif

using std::string;

char boinc_failed_file[MAXPATHLEN];

// routines for enumerating the entries in a directory

int is_file(const char* path) {
#ifdef _WIN32
    DWORD dwAttrib = GetFileAttributesA(path);
    return (dwAttrib != INVALID_FILE_ATTRIBUTES
        && !(dwAttrib & FILE_ATTRIBUTE_DIRECTORY)
    );
#else
    struct stat sbuf;
    int retval = lstat(path, &sbuf);
    return (!retval && (((sbuf.st_mode) & S_IFMT) == S_IFREG));
#endif
}

int is_dir(const char* path) {
#ifdef _WIN32
    DWORD dwAttrib = GetFileAttributesA(path);
    return (dwAttrib != INVALID_FILE_ATTRIBUTES
        && (dwAttrib & FILE_ATTRIBUTE_DIRECTORY)
    );
#else
    struct stat sbuf;
    int retval = lstat(path, &sbuf);
    return (!retval && (((sbuf.st_mode) & S_IFMT) == S_IFDIR));
#endif
}

#ifndef _WIN32

int is_file_follow_symlinks(const char* path) {
    struct stat sbuf;
    int retval = stat(path, &sbuf);
    return (!retval && (((sbuf.st_mode) & S_IFMT) == S_IFREG));
}

int is_dir_follow_symlinks(const char* path) {
    struct stat sbuf;
    int retval = stat(path, &sbuf);
    return (!retval && (((sbuf.st_mode) & S_IFMT) == S_IFDIR));
}

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
    safe_strcat(dirp->path, "\\*");
    dirp->handle = INVALID_HANDLE_VALUE;
#else
    dirp = opendir(p);
    if (!dirp) return NULL;
#endif
    return dirp;
}

// Scan through a directory and return:
// 0 if an entry was found (the entry is returned in p)
// ERR_NOT_FOUND if we reached the end of the directory
// ERR_READDIR if some other error occurred
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
                DWORD ret = GetLastError();
                FindClose(dirp->handle);
                dirp->handle = INVALID_HANDLE_VALUE;
                if (ret == ERROR_NO_MORE_FILES) {
                    return ERR_NOT_FOUND;
                }
                return ERR_READDIR;
            }
        }
    }
#else
    while (1) {
        errno = 0;
        dirent* dp = readdir(dirp);
        if (dp) {
            if (!strcmp(dp->d_name, ".")) continue;
            if (!strcmp(dp->d_name, "..")) continue;
            if (p) strlcpy(p, dp->d_name, p_len);
            return 0;
        } else {
            if (errno) return ERR_READDIR;
            return ERR_NOT_FOUND;
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
    char file[MAXPATHLEN];

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
#if defined(_WIN32) && !defined(__CYGWIN32__) && !defined(__MINGW32__)
    HANDLE h = CreateFileA(path, 0, FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE, 0, OPEN_EXISTING, 0, 0);
    if (h == INVALID_HANDLE_VALUE) return ERR_STAT;
    LARGE_INTEGER lisize;
    if (GetFileSizeEx(h, &lisize)) {
        size = (double) lisize.QuadPart;
        CloseHandle(h);
        return 0;
    }
    return ERR_STAT;
#else
    int retval;
    struct stat sbuf;
    retval = stat(path, &sbuf);
    if (retval) return ERR_NOT_FOUND;
    size = (double)sbuf.st_size;
    return 0;
#endif
}

// get file allocation size, i.e. how much disk space does it use.
// This can be less than the file size on compressed filesystems,
// or if the file has holes.
// It can also be slightly more.
//
int file_size_alloc(const char* path, double& size) {
#if defined(_WIN32) && !defined(__CYGWIN32__) && !defined(__MINGW32__)
    DWORD hi;
    DWORD lo = GetCompressedFileSizeA(path, &hi);
    if (lo != INVALID_FILE_SIZE) {
        ULONGLONG x = (((ULONGLONG)hi) << 32) + lo;
        size = (double) x;
        return 0;
    }
    return ERR_STAT;
#else
    int retval;
    struct stat sbuf;
    retval = stat(path, &sbuf);
    if (retval) return ERR_NOT_FOUND;
#ifdef _WIN32 // cygwin, mingw
    size = (double)sbuf.st_size;
#else
    size = ((double)sbuf.st_blocks)*512.;
#endif
    return 0;
#endif
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
    char filename[MAXPATHLEN], path[MAXPATHLEN];
    int retval;
    DIRREF dirp;

    dirp = dir_open(dirpath);
    if (!dirp) return 0;    // if dir doesn't exist, it's empty
    while (1) {
        safe_strcpy(filename, "");
        retval = dir_scan(filename, dirp, sizeof(filename));
        if (retval) break;

        snprintf(path, sizeof(path), "%.*s/%.*s", DIR_LEN, dirpath, FILE_LEN, filename);
        path[sizeof(path)-1] = 0;

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
    char buf[_MAX_PATH];
    char path2[_MAX_PATH];
    double dsize = 0.0;
    WIN32_FIND_DATAA findData;

    size = 0.0;
    snprintf(path2, sizeof(path2), "%s/*", dirpath);
    path2[sizeof(path2)-1] = 0;

    HANDLE hFind = ::FindFirstFileA(path2, &findData);
    if (INVALID_HANDLE_VALUE == hFind) return ERR_OPENDIR;
    do {
        if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            if (!recurse) continue;
            if (!strcmp(findData.cFileName, ".")) continue;
            if (!strcmp(findData.cFileName, "..")) continue;

            dsize = 0.0;

            snprintf(buf, sizeof(buf), "%.*s/%.*s", DIR_LEN, dirpath, FILE_LEN, findData.cFileName);
            buf[sizeof(buf)-1] = 0;

            dir_size(buf, dsize, true);
            size += dsize;
        } else {
            size += findData.nFileSizeLow + ((__int64)(findData.nFileSizeHigh) << 32);
        }
    } while (FindNextFileA(hFind, &findData));

    ::FindClose(hFind);
#else
    char filename[MAXPATHLEN], subdir[MAXPATHLEN];
    int retval=0;
    DIRREF dirp;
    double x;

    size = 0.0;
    dirp = dir_open(dirpath);
    if (!dirp) return ERR_OPENDIR;
    while (1) {
        retval = dir_scan(filename, dirp, sizeof(filename));
        if (retval) break;

        snprintf(subdir, sizeof(subdir), "%.*s/%.*s", DIR_LEN, dirpath, FILE_LEN, filename);
        subdir[sizeof(subdir)-1] = 0;

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

// return total allocated size of files in directory and optionally its subdirectories
// Win: use special version because stat() is slow, can be avoided
// Unix: follow symbolic links
//
int dir_size_alloc(const char* dirpath, double& size, bool recurse) {
#ifdef WIN32
    char buf[_MAX_PATH];
    char path2[_MAX_PATH];
    double dsize = 0.0;
    WIN32_FIND_DATAA findData;

    size = 0.0;
    snprintf(path2, sizeof(path2), "%s/*", dirpath);
    path2[sizeof(path2)-1] = 0;

    HANDLE hFind = ::FindFirstFileA(path2, &findData);
    if (INVALID_HANDLE_VALUE == hFind) return ERR_OPENDIR;
    do {
        snprintf(buf, sizeof(buf), "%.*s/%.*s", DIR_LEN, dirpath, FILE_LEN, findData.cFileName);
        buf[sizeof(buf)-1] = 0;
        if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            if (!recurse) continue;
            if (!strcmp(findData.cFileName, ".")) continue;
            if (!strcmp(findData.cFileName, "..")) continue;

            dsize = 0.0;
            dir_size_alloc(buf, dsize, true);
            size += dsize;
        } else {
            double s;
            if (file_size_alloc(buf, s) == 0) {
                size += s;
            }
        }
    } while (FindNextFileA(hFind, &findData));

    ::FindClose(hFind);
#else
    char filename[MAXPATHLEN], subdir[MAXPATHLEN];
    int retval=0;
    DIRREF dirp;
    double x;

    size = 0.0;
    dirp = dir_open(dirpath);
    if (!dirp) return ERR_OPENDIR;
    while (1) {
        retval = dir_scan(filename, dirp, sizeof(filename));
        if (retval) break;

        snprintf(subdir, sizeof(subdir), "%.*s/%.*s", DIR_LEN, dirpath, FILE_LEN, filename);
        subdir[sizeof(subdir)-1] = 0;

        if (is_dir(subdir)) {
            if (recurse) {
                retval = dir_size_alloc(subdir, x);
                if (retval) continue;
                size += x;
            }
        } else if (is_file(subdir)) {
            retval = file_size_alloc(subdir, x);
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
    FILE *f = boinc::fopen(path, mode);

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
            f = boinc::fopen(path, mode);
            if (f) break;
        }
    }
    if (f) {
        if (-1 == fcntl(boinc::fileno(f), F_SETFD, FD_CLOEXEC)) {
            boinc::fclose(f);
            return 0;
        }
    }
#endif
    return f;
}

// returns true if anything (file, dir, whatever) exists at given path;
// name is misleading.
//
int boinc_file_exists(const char* path) {
#ifdef _WIN32
    // don't use _stat64 because it doesn't work with VS2015, XP client
    DWORD dwAttrib = GetFileAttributesA(path);
    return dwAttrib != INVALID_FILE_ATTRIBUTES;
#else
    struct stat buf;
    if (stat(path, &buf)) {
        return false;     // stat() returns zero on success
    }
    return true;
#endif
}

#if 0
// same, but doesn't traverse symlinks
//
int boinc_file_or_symlink_exists(const char* path) {
#ifdef _WIN32
    return boinc_file_exists(path);
#else
    struct stat buf;
    if (lstat(path, &buf)) {
        return false;     // stat() returns zero on success
    }
    return true;
#endif
}
#endif

// returns zero on success, nonzero if didn't touch file
//
int boinc_touch_file(const char *path) {
    if (boinc_file_exists(path)) {
        return 0;
    }
    FILE *fp = boinc::fopen(path, "w");
    if (fp) {
        boinc::fclose(fp);
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
    char cmd[2*MAXPATHLEN+4+2+4+1]; // 2 blanks, 4 '"'s, 1 terminating 0
    snprintf(cmd, sizeof(cmd), "copy \"%s\" \"%s\"", orig, newf);
    return system(cmd);
#else
    // POSIX requires that shells run from an application will use the
    // real UID and GID if different from the effective UID and GID.
    // Mac OS 10.4 did not enforce this, but OS 10.5 does.  Since
    // system() invokes a shell, it may not properly copy the file's
    // ownership or permissions when called from the BOINC Client
    // under sandbox security, so we copy the file directly.
    //
    FILE *src, *dst;
    int m, n;
    int retval = 0;
    unsigned char buf[65536];
    src = boinc_fopen(orig, "r");
    if (!src) return ERR_FOPEN;
    dst = boinc_fopen(newf, "w");
    if (!dst) {
        boinc::fclose(src);
        return ERR_FOPEN;
    }
    while (1) {
        n = boinc::fread(buf, 1, sizeof(buf), src);
        if (n <= 0) {
            // could be either EOF or an error.
            // Check for error case.
            //
            if (!boinc::feof(src)) {
                retval = ERR_FREAD;
            }
            break;
        }
        m = boinc::fwrite(buf, 1, n, dst);
        if (m != n) {
            retval = ERR_FWRITE;
            break;
        }
    }
    if (boinc::fclose(src)){
       boinc::fclose(dst);
       return ERR_FCLOSE;
    }

    if (boinc::fclose(dst)){
       return ERR_FCLOSE;
    }
    return retval;
#endif
}

#ifndef _WIN32
// Copy file's ownership and permissions to the extent we are allowed
//
int boinc_copy_attributes(const char* orig, const char* newf) {
    struct stat sbuf;

    // Get source file's info
    //
    if (lstat(orig, &sbuf)) {
        return ERR_STAT;
    }
    if (chmod(newf, sbuf.st_mode)) {
        return ERR_CHMOD;
    }
    if (chown(newf, sbuf.st_uid, sbuf.st_gid)) {
        return ERR_CHOWN;
    }
    return 0;
}
#endif

static int boinc_rename_aux(const char* old, const char* newf) {
#ifdef _WIN32
    // MOVEFILE_COPY_ALLOWED is needed if destination is on another volume (move is simulated by copy&delete)
    if (MoveFileExA(old, newf, MOVEFILE_COPY_ALLOWED | MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH)) return 0;
    return GetLastError();
#else
    // rename() doesn't work between filesystems.
    // So if it fails, try the "mv" command, which does work
    //
    int retval = rename(old, newf);
    if (retval) {
        char buf[MAXPATHLEN+MAXPATHLEN];
        snprintf(buf, sizeof(buf), "mv \"%s\" \"%s\"", old, newf);
#ifdef __APPLE__
        // system() is deprecated in Mac OS 10.10.
        // Apple says to call posix_spawn instead.
        retval = callPosixSpawn(buf);
#else
        retval = system(buf);
#endif
    }
    if (retval) return ERR_RENAME;
    return 0;
#endif
}

int boinc_rename(const char* old, const char* newf) {
    int retval=0;

    retval = boinc_rename_aux(old, newf);
    if (retval) {
        // if the rename failed, and the file exists,
        // retry a few times
        //
        if (!boinc_file_exists(old)) return ERR_FILE_MISSING;
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
    char buf[MAXPATHLEN], oldpath[MAXPATHLEN], newpath[MAXPATHLEN];
    int retval;
    char *p, *q;

    if (strlen(filepath) + strlen(dirpath) > MAXPATHLEN-1) return ERR_BUFFER_OVERFLOW;
    safe_strcpy(buf, filepath);
    safe_strcpy(oldpath, dirpath);

    q = buf;
    while(*q) {
        p = strchr(q, '/');
        if (!p) break;
        *p = 0;
        snprintf(newpath, sizeof(newpath), "%.*s/%.*s", DIR_LEN, oldpath, FILE_LEN, q);
        newpath[sizeof(newpath)-1] = 0;
        retval = boinc_mkdir(newpath);
        if (retval) return retval;
        safe_strcpy(oldpath, newpath);
        q = p+1;
    }
    return 0;
}


FILE_LOCK::FILE_LOCK() {
#if defined(_WIN32) && !defined(__CYGWIN32__)
  handle = INVALID_HANDLE_VALUE;
#else
    fd = -1;
#endif
    locked = false;
}

FILE_LOCK::~FILE_LOCK() {
#if !defined(_WIN32) || defined(__CYGWIN32__)
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
        // ENOSYS means file locking is not implemented in this FS.
        // In this case just return success (i.e. don't actually do locking)
        //
        if (errno != ENOSYS) {
            return ERR_FCNTL;
        }
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
    getcwd(path, MAXPATHLEN);
#else
    char* p
#ifdef __GNUC__
      __attribute__ ((unused))
#endif
      = getcwd(path, MAXPATHLEN);
#endif
}

void relative_to_absolute(const char* relname, char* path) {
    boinc_getcwd(path);
    if (strlen(relname)) {
        strcat(path, "/");
        strcat(path, relname);
    }
}


#if defined(_WIN32)
int boinc_allocate_file(const char* path, double size) {
    int retval = 0;
    HANDLE h = CreateFileA(
        path,
        GENERIC_WRITE,
        0,
        NULL,
        OPEN_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );
    if (h == INVALID_HANDLE_VALUE) return ERR_FOPEN;
    LARGE_INTEGER sz;
    sz.LowPart = fmod(size, 4294967296.);
    sz.HighPart = (LONG)(size/4294967296.);
    if (SetFilePointerEx(h, sz, NULL, FILE_BEGIN) == 0) {
        retval = ERR_FOPEN;
    }
    if (!retval && SetEndOfFile(h) == 0) {
        retval = ERR_FOPEN;
    }
    CloseHandle(h);
    return retval;
}

FILE* boinc_temp_file(
    const char* dir, const char* prefix, char* temp_path, double size
) {
    GetTempFileNameA(dir, prefix, 0, temp_path);
    boinc_allocate_file(temp_path, size);
    return boinc_fopen(temp_path, "wb");
}

#else

// Unix version: use mkstemp.  tempnam() prioritizes an env var
// in deciding where to put temp file

FILE* boinc_temp_file(const char* dir, const char* prefix, char* temp_path) {
    sprintf(temp_path, "%s/%s_XXXXXX", dir, prefix);
    int fd = mkstemp(temp_path);
    if (fd < 0) {
        return 0;
    }
    return boinc::fdopen(fd, "wb");
}

#endif

void boinc_path_to_dir(const char* path, char* dir) {
    strcpy(dir, path);
    char* p = strrchr(dir, '/');
    if (p) {
        *p = 0;
    } else {
        strcpy(dir, ".");
    }
}

// get total and free space on current filesystem (in bytes)
//
#ifdef _WIN32
int get_filesystem_info(double &total_space, double &free_space, char*) {
    char cwd[MAXPATHLEN];
    ULARGE_INTEGER TotalNumberOfFreeBytes;
    ULARGE_INTEGER TotalNumberOfBytes;
    ULARGE_INTEGER FreeBytesAvailable;
    signed __int64 uMB;

    boinc_getcwd(cwd);
    GetDiskFreeSpaceExA(
        cwd,
        &FreeBytesAvailable,
        &TotalNumberOfBytes,
        &TotalNumberOfFreeBytes
    );

    uMB = FreeBytesAvailable.QuadPart / (1024 * 1024);
    free_space = uMB * 1024.0 * 1024.0;
    uMB = TotalNumberOfBytes.QuadPart / (1024 * 1024);
    total_space = uMB * 1024.0 * 1024.0;

#else
int get_filesystem_info(double &total_space, double &free_space, char* path) {
#ifdef STATFS
    struct STATFS fs_info;

    int retval = STATFS(path, &fs_info);
    if (retval) {
        boinc::perror("statvfs");
        return ERR_STATFS;
    }
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

bool is_path_absolute(const std::string path) {
#ifdef _WIN32
    if (path.length() >= 3 && isalpha(path[0]) && path[1] == ':' && (path[2] == '\\' || path[2] == '/')) {
        // c:\file
        return true;
    }
    if (path.length() >= 2 && (path[0] == '\\' || path[0] == '/') && (path[1] == '\\' || path[1] == '/')) {
        // \\server\file
        // \\?\c:\file
        return true;
    }
    return false;
#else
    return path.length() >= 1 && path[0] == '/';
#endif
}

// read file (at most max_len chars, if nonzero) into malloc'd buf
//
#ifdef _USING_FCGI_
int read_file_malloc(const char* path, char*& buf, size_t, bool) {
#else
int read_file_malloc(const char* path, char*& buf, size_t max_len, bool tail) {
#endif
    int retval;
    double size;

    // Win: if another process has this file open for writing,
    // wait for up to 5 seconds.
    // This is because when a job exits, the write to stderr.txt
    // sometimes (inexplicably) doesn't appear immediately

#ifdef _WIN32
    for (int i=0; i<5; i++) {
        HANDLE h = CreateFileA(
            path,
            GENERIC_WRITE,
            0,
            NULL,
            OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL,
            NULL
        );
        if (h != INVALID_HANDLE_VALUE) {
            CloseHandle(h);
            break;
        }
        boinc_sleep(1);
    }
#endif

    retval = file_size(path, size);
    if (retval) return retval;

    // Note: the fseek() below won't work unless we use binary mode in fopen

    FILE *f = boinc::fopen(path, "rb");
    if (!f) return ERR_FOPEN;

#ifndef _USING_FCGI_
    if (max_len && size > max_len) {
        if (tail) {
            fseek(f, (long)size-(long)max_len, SEEK_SET);
        }
        size = max_len;
    }
#endif
    size_t isize = (size_t)size;
    buf = (char*)malloc(isize+1);
    if (!buf) {
        boinc::fclose(f);
        return ERR_MALLOC;
    }
    size_t n = boinc::fread(buf, 1, isize, f);
    buf[n] = 0;
    boinc::fclose(f);
    return 0;
}

// read file (at most max_len chars, if nonzero) into string
//
int read_file_string(
    const char* path, string& result, size_t max_len, bool tail
) {
    result.erase();
    int retval;
    char* buf;

    retval = read_file_malloc(path, buf, max_len, tail);
    if (retval) return retval;
    result = buf;
    free(buf);
    return 0;
}
