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

#include "windows_cpp.h"

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
#endif

#include "util.h"
#include "error_numbers.h"
#include "filesys.h"

#ifdef _WIN32
static char path[256];
static HANDLE handle;
static int first;
#endif

char failed_file[256];

// routines for enumerating the entries in a directory

// Open a directory
int dir_open(char* p, DIR* dirp) {
    if(p==NULL) {
        fprintf(stderr, "error: dir_open: unexpected NULL pointer p\n");
        return ERR_NULL;
    }
#ifdef HAVE_DIRENT_H
    dirp = opendir(p);
    if (!dirp) return ERR_OPENDIR;
#endif
#ifdef _WIN32
    strcpy(path, p);
    strcat(path, "\\*");
    first = 1;
    handle = INVALID_HANDLE_VALUE;
    // This is probably incomplete
#endif
#ifdef macintosh
    SayErr("\pdir_open called (empty function)");	/* CAF Temp */
#endif
    return 0;
}

// Scan through a directory and return the next file name in it
//
int dir_scan(char* p, DIR *dirp) {
    if(p==NULL) {
        fprintf(stderr, "error: dir_scan: unexpected NULL pointer p\n");
        return ERR_NULL;
    }
    if( !dirp )
        return -1;
#ifdef HAVE_DIRENT_H
    while (1) {
	dirent* dp = readdir(dirp);
	if (dp) {
	    if (dp->d_name[0] == '.') continue;
	    if (p) strcpy(p, dp->d_name);
	    return 0;
	} else {
	    closedir(dirp);
	    dirp = 0;
	    return -1;
	}
    }
#endif
#ifdef _WIN32
    WIN32_FIND_DATA data;
    while (1) {
	if (first) {
	    first = 0;
	    handle = FindFirstFile(path, &data);
	    if (handle == INVALID_HANDLE_VALUE) {
		return -1;
	    } else {
		if (data.cFileName[0] == '.') continue;
		if (p) strcpy(p, data.cFileName);
		return 0;
	    }
	} else {
	    if (FindNextFile(handle, &data)) {
		if (data.cFileName[0] == '.') continue;
		if (p) strcpy(p, data.cFileName);
		return 0;
	    } else {
		FindClose(handle);
		handle = INVALID_HANDLE_VALUE;
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
void dir_close( DIR* dirp ) {
#ifdef HAVE_DIRENT_H
    if (dirp) {
	closedir(dirp);
	dirp = 0;
    }
#endif
#ifdef _WIN32
    if (handle == INVALID_HANDLE_VALUE) {
	FindClose(handle);
	handle = INVALID_HANDLE_VALUE;
    }
#endif
#ifdef macintosh
	SayErr("\pdir_close called (empty function)");	/* CAF Temp */
#endif
}

// Delete the file located at path
//
int file_delete(char* path) {
    int retval,i;
    if(path==NULL) {
        fprintf(stderr, "error: file_delete: unexpected NULL pointer path\n");
        return ERR_NULL;
    }
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
    if(path==NULL) {
        fprintf(stderr, "error: file_size: unexpected NULL pointer path\n");
        return ERR_NULL;
    }
    retval = stat(path, &sbuf);
    if (retval) return retval;
    size = sbuf.st_size;
    return 0;
}

/* boinc_link creates a file (new_link) which contains an XML
   reference (soft link) to existing. */

int boinc_link( char *existing, char *new_link ) {
    FILE *fp;
    if(existing==NULL) {
        fprintf(stderr, "error: boinc_link: unexpected NULL pointer existing\n");
        return ERR_NULL;
    }
    if(new_link==NULL) {
        fprintf(stderr, "error: boinc_link: unexpected NULL pointer new_link\n");
        return ERR_NULL;
    }
    fp = fopen( new_link, "wb" );
    if (!fp) return ERR_FOPEN;
    rewind( fp );
    fprintf( fp, "<soft_link>%s</soft_link>\n", existing );
    fclose( fp );

    return 0;
}

// Goes through directory specified by dirpath and removes all files from it
//
int clean_out_dir(char* dirpath) {
    char filename[256], path[256];
    int retval;
    DIR dirp;
    if(dirpath==NULL) {
        fprintf(stderr, "error: clean_out_dir: unexpected NULL pointer dirpath\n");
        return ERR_NULL;
    }
    retval = dir_open(dirpath,&dirp);
    if (retval) return retval;
    while (1) {
        retval = dir_scan(filename,&dirp);
        if (retval) break;
        sprintf(path, "%s/%s", dirpath, filename);
        retval = file_delete(path);
        if (retval) {
            dir_close(&dirp);
            return retval;
        }
    }
    dir_close(&dirp);
    return 0;
}

// Goes recursively through directory specified by dirpath and returns the
// total size of files in it and its subdirectories
//
double dir_size(char* dirpath) {
    char filename[256], *path;
    int retval,temp;
    double cur_size = 0;
    DIR dirp;

    if(dirpath==NULL) {
        fprintf(stderr, "error: dir_size: unexpected NULL pointer dirpath\n");
        return ERR_NULL;
    }
    path = (char *)malloc( 256*sizeof( char ) );
    retval = dir_open(dirpath,&dirp);
    if (retval) return 0;
    while (1) {
        retval = dir_scan(filename,&dirp);
        if (retval) break;
        sprintf(path, "%s/%s", dirpath, filename);
        cur_size += dir_size( path );
        retval = file_size(path,temp);
        if (retval) {
            dir_close(&dirp);
            return cur_size;
        }
        cur_size += temp;
    }
    dir_close(&dirp);
    return cur_size;
}
