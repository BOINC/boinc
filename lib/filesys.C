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

#ifdef HAVE_SYS_STATVFS_H
#include <sys/statvfs.h>
#endif
#ifdef HAVE_SYS_STATVFS_H
#define STATFS statvfs
#else
#define STATFS statfs
#endif

#ifdef _WIN32
#include <windows.h>
#include <io.h>
#include <winsock.h>
#include <direct.h>

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

int is_dir(char* path) {
    struct stat sbuf;
    memset(&sbuf, 0, sizeof(struct stat));
    stat(path, &sbuf);
    return sbuf.st_mode & S_IFDIR;
}

// Open a directory
//
DIRREF dir_open(char* p) {
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
            return -1;
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
                return -1;
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

// Delete the file located at path
//
int file_delete(char* path) {
    int retval;

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
int file_size(char* path, double& size) {
    struct stat sbuf;
    int retval;

    retval = stat(path, &sbuf);
    if (retval) return retval;
    size = (double)sbuf.st_size;
    return 0;
}

// removes all files from specified directory
//
int clean_out_dir(char* dirpath) {
    char filename[256], path[256];
    int retval;
    DIRREF dirp;
    
    dirp = dir_open(dirpath);
    if (!dirp) return -1;
    while (1) {
        strcpy(filename, "");
        retval = dir_scan(filename, dirp, sizeof(filename));
        if (retval) break;
        sprintf(path, "%s%s%s", dirpath, PATH_SEPARATOR, filename);
        clean_out_dir(path);
        boinc_rmdir(path);
        retval = file_delete(path);
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
int dir_size(char* dirpath, double& size) {
    char filename[256], subdir[256];
    int retval=0;
    DIRREF dirp;
    double x;

    size = 0;
    dirp = dir_open(dirpath);
    if (!dirp) return -1;
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
        } else if (retval == -1) {
            retval = file_size(subdir, x);
            if (retval) continue;
            size += x;
        } else {
            break;
        }
    }
    dir_close(dirp);
    return 0;
}

int boinc_rename(char* old, char* newf) {
#ifdef _WIN32
    unlink(newf);
#endif
    return rename(old, newf);
}

int boinc_mkdir(char* name) {
#ifdef _WIN32
    return CreateDirectory(name, NULL);
#else
    return mkdir(name, 0777);
#endif
}

int boinc_rmdir(char* name) {
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
	pGetDiskFreeSpaceEx = (FreeFn)GetProcAddress(GetModuleHandle("kernel32.dll"), "GetDiskFreeSpaceExA");
	if(pGetDiskFreeSpaceEx) {
		ULARGE_INTEGER TotalNumberOfFreeBytes;
		ULARGE_INTEGER TotalNumberOfBytes;
		pGetDiskFreeSpaceEx(NULL, NULL, &TotalNumberOfBytes, &TotalNumberOfFreeBytes);
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
    total_space = (double)fs_info.f_bsize * (double)fs_info.f_blocks;
    free_space = (double)fs_info.f_bsize * (double)fs_info.f_bavail;
#else
#error Need to specify a method to obtain free/total disk space
#endif
#endif
    return 0;
}
