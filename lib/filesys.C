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

#ifdef _WIN32
#include "stdafx.h"
#endif

#ifndef _WIN32
#include <stdio.h>
#include <fcntl.h>

#include <sys/stat.h>

#include <time.h>
#include <string.h>
#include <stdlib.h>

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
#ifdef __FreeBSD__
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

char failed_file[256];

// routines for enumerating the entries in a directory

int is_file(char* path) {
    struct stat sbuf;
    memset(&sbuf, 0, sizeof(struct stat));
    stat(path, &sbuf);
    return sbuf.st_mode & S_IFREG;
}

int is_dir(const char* path) {
    struct stat sbuf;
    memset(&sbuf, 0, sizeof(struct stat));
    stat(path, &sbuf);
    return sbuf.st_mode & S_IFDIR;
}

// Open a directory
//
DIRREF dir_open(const char* p) {
    DIRREF dirp;

#ifdef HAVE_DIRENT_H
    dirp = opendir(p);
    if (!dirp) return NULL;
#endif
#ifdef _WIN32
    if(!is_dir(p)) return NULL;
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
            if (dp->d_name[0] == '.') continue;
            if (p) safe_strncpy(p, dp->d_name, p_len);
            return 0;
        } else {
            return ERR_READDIR;
        }
    }
#endif
#ifdef _WIN32
    WIN32_FIND_DATA data;
    while (1) {
        if (dirp->first) {
            dirp->first = false;
            dirp->handle = FindFirstFile(dirp->path, &data);
            if (dirp->handle == INVALID_HANDLE_VALUE) {
                return ERR_READDIR;
            } else {
                if (data.cFileName[0] == '.') continue;
                if (p) safe_strncpy(p, data.cFileName, p_len);
                return 0;
            }
        } else {
            if (FindNextFile(dirp->handle, &data)) {
                if (data.cFileName[0] == '.') continue;
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
#endif
#ifdef _WIN32
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
#endif

#ifdef _WIN32
    first = true;
    handle = INVALID_HANDLE_VALUE;
    if(!is_dir((char*)path.c_str())) {
        return;
    }
    dir = path + "\\*";
#endif
}

// Scan through a directory and return the next file name in it
//
bool DirScanner::scan(string& s) {
#ifdef HAVE_DIRENT_H
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
#endif
}

// Close a directory
DirScanner::~DirScanner() {
#ifdef HAVE_DIRENT_H
    if (dirp) {
        closedir(dirp);
    }
#endif
#ifdef _WIN32
    if (handle != INVALID_HANDLE_VALUE) {
        FindClose(handle);
    }
#endif
}


// Delete the file located at path
//
int boinc_delete_file(const char* path) {
    int retval;

    if (!boinc_file_exists(path)) {
        return 0;
    }
#ifdef HAVE_UNISTD_H
    retval = unlink(path);
#endif
#if ( defined(_WIN32) || defined(macintosh) )
    retval = remove(path);
#endif
    if (retval) {
        safe_strcpy(failed_file, path);
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
    if (retval) return retval;
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
    if (!dirp) return ERR_OPENDIR;
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

// on Windows: if fopen fails, try again for 5 seconds
// (since the file might be open by FastFind, Diskeeper etc.)
//
FILE* boinc_fopen(const char* path, const char* mode) {
    FILE* f;

    f = fopen(path, mode);
#ifdef _WIN32
    if (!f) {
        for (int i = 0; i < 5; i++) {
            boinc_sleep(1.0);
            f = fopen(path, mode);
            if (f) {
                break;
            }
        }
    }
#endif
    return f;
}


int boinc_file_exists(const char* path) {
   struct stat buf;

   if (stat(path, &buf)) {
       return false;
   }
   return true;
}


int boinc_copy(const char* orig, const char* newf) {
#ifdef _WIN32
	if(CopyFile(orig, newf, FALSE)) {
		return 0;
    } else {
		return GetLastError();
    }
#else
	char cmd[256];
	sprintf(cmd, "cp %s %s", orig, newf);
	return system(cmd);
#endif
}

int boinc_rename(const char* old, const char* newf) {
#ifdef _WIN32
    unlink(newf);
#endif
    return rename(old, newf);
}

int boinc_mkdir(const char* name) {
#ifdef _WIN32
    return CreateDirectory(name, NULL);
#else
    return mkdir(name, 0777);
#endif
}

int boinc_rmdir(const char* name) {
#ifdef _WIN32
    return RemoveDirectory(name);
#else
    return rmdir(name);
#endif
}

#ifdef _WIN32
void full_path(char* relname, char* path) {
    _getcwd(path, 256);
    strcat(path, PATH_SEPARATOR);
    strcat(path, relname);
}
#endif

// get total and free space on current filesystem (in bytes)
//
int get_filesystem_info(double &total_space, double &free_space) {
#ifdef _WIN32
	FreeFn pGetDiskFreeSpaceEx;
	pGetDiskFreeSpaceEx = (FreeFn)GetProcAddress(GetModuleHandle("kernel32.dll"),
                                                 "GetDiskFreeSpaceExA");
	if(pGetDiskFreeSpaceEx) {
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
