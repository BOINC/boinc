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

#ifdef macintosh
#include "seti_mac.h"
#else
#include <sys/stat.h>
#endif

#ifdef MAC_OS_X
#include <sys/types.h>
#include <sys/stat.h>
#endif

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

#ifdef _WIN32
#include <io.h>
#include <winsock.h>
#include <direct.h>
#endif

#include "util.h"
#include "error_numbers.h"
#include "filesys.h"

#ifdef _WIN32
//static char path[256];
//static HANDLE handle;
//static int first;
#endif

char failed_file[256];

// routines for enumerating the entries in a directory

// Open a directory
//
DIRREF dir_open(char* p) {
    DIRREF dirp;

#ifdef HAVE_DIRENT_H
    dirp = opendir(p);
    if (!dirp) return NULL;
#endif
#ifdef _WIN32
	dirp = (DIR_DESC*) calloc(sizeof(DIR_DESC), 1);
    dirp->first = true;
	strcpy(dirp->path, p);
	strcat(dirp->path, "\\*");
	dirp->handle = INVALID_HANDLE_VALUE;
#endif
#ifdef macintosh
    SayErr("\pdir_open called (empty function)");	/* CAF Temp */
#endif
    return dirp;
}

// Scan through a directory and return the next file name in it
//
int dir_scan(char* p, DIRREF dirp) {
#ifdef HAVE_DIRENT_H
    while (1) {
	dirent* dp = readdir(dirp);
	if (dp) {
	    if (dp->d_name[0] == '.') continue;
	    if (p) strcpy(p, dp->d_name);
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
		if (p) strcpy(p, data.cFileName);
		return 0;
	    }
	} else {
	    if (FindNextFile(dirp->handle, &data)) {
		if (data.cFileName[0] == '.') continue;
		if (p) strcpy(p, data.cFileName);
		return 0;
	    } else {
		FindClose(dirp->handle);
		dirp->handle = INVALID_HANDLE_VALUE;
		return 1;
	    }
	}
    }
#endif
#ifdef macintosh
	SayErr("\pdir_scan called (empty function)");	/* CAF Temp */
	return 1;
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
#ifdef macintosh
    SayErr("\pdir_close called (empty function)");	/* CAF Temp */
#endif
}

// Delete the file located at path
//
int file_delete(char* path) {
    int retval,i;

    for (i=0; i<2; i++) {
#ifdef HAVE_UNISTD_H
        retval = unlink(path);
#endif
#if ( defined(_WIN32) || defined(macintosh) )
        retval = remove(path);
#endif
        if (!retval) break;
        if (i==0) boinc_sleep(3);
    }
    if (retval) {
	strcpy(failed_file, path);
	return ERR_UNLINK;
    }
    return 0;
}

// get file size
int file_size(char* path, int& size) {
    struct stat sbuf;
    int retval;

    retval = stat(path, &sbuf);
    if (retval) return retval;
    size = sbuf.st_size;
    return 0;
}

// create a file (new_link) which contains an XML
// reference to existing file.
//
int boinc_link(char *existing, char *new_link) {
    FILE *fp;

    fp = fopen(new_link, "w");
    if (!fp) return ERR_FOPEN;
    fprintf(fp, "<soft_link>%s</soft_link>\n", existing);
    fclose(fp);

    return 0;
}

// Goes through directory specified by dirpath and removes all files from it
//
int clean_out_dir(char* dirpath) {
    char filename[256], path[256];
    int retval;
    DIRREF dirp;
    
    dirp = dir_open(dirpath);
    if (!dirp) return -1;
    while (1) {
	strcpy(filename,"");
        retval = dir_scan(filename, dirp);
        if (retval) break;
        sprintf(path, "%s/%s", dirpath, filename);
        retval = file_delete(path);
        if (retval) {
            dir_close(dirp);
            return retval;
        }
    }
    dir_close(dirp);
    return 0;
}

// Goes recursively through directory specified by dirpath and returns the
// total size of files in it and its subdirectories
//
double dir_size(char* dirpath) {
    char filename[256], subdir[256];
    int retval,temp;
    double cur_size = 0;
    DIRREF dirp;

    dirp = dir_open(dirpath);
    if (!dirp) return -1;
    while (1) {
        retval = dir_scan(filename, dirp);
        if (retval) break;
        sprintf(subdir, "%s/%s", dirpath, filename);
        cur_size += dir_size(subdir);
        retval = file_size(subdir, temp);
        if (retval) {
            dir_close(dirp);
            return cur_size;
        }
        cur_size += temp;
    }
    dir_close(dirp);
    return cur_size;
}

int boinc_rename(char* old, char* newf) {
#ifdef _WIN32
    unlink(newf);
#endif
    return rename(old, newf);
}

#ifdef _WIN32
void full_path(char* relname, char* path) {
    _getcwd(path, 256);
    strcat(path, PATH_SEPARATOR);
    strcat(path, relname);
}
#endif
