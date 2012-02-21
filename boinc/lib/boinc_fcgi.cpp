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

// This file is an attempt to fix some of the epidemic build problems
// caused by libraries redefining the C stdio.h functions that has struck.  
// It's far better to use overloads in a C++ namespace to fix these issues 
// for a variety of reasons.

#include "boinc_fcgi.h"
#include <cstring>

#ifdef _USING_FCGI_

namespace FCGI {

FCGI_FILE *fopen(const char *path, const char *mode) { 
    return FCGI_fopen(path,mode); 
}

FCGI_FILE *freopen(const char *path, const char *mode, FCGI_FILE *file) { 
    return FCGI_freopen(path,mode,file); 
}

int fflush(FCGI_FILE *file) {
    return FCGI_fflush(file);
}

int fprintf(FCGI_FILE *fp, const char *format, ...) {
    va_list va;
    va_start(va, format);
    int i=FCGI_vfprintf(fp,format,va);
    va_end(va);
    return i;
}

int vfprintf(FCGI_FILE *fp, const char *format, va_list ap) {
    return FCGI_vfprintf(fp,format,ap);
}

char *fgets(char *str, int size, FCGI_FILE *fp) {
    return FCGI_fgets(str,size,fp);
}

int fputs(const char *str, FCGI_FILE *fp) {
    return FCGI_fputs(str,fp);
}

int fclose(FCGI_FILE *f) {
    return FCGI_fclose(f);
}

int fgetc(FCGI_FILE *f) {
    return FCGI_fgetc(f);
}

size_t fread(void *buf, size_t i, size_t j, FCGI_FILE *fp) {
    return FCGI_fread(buf,i,j,fp);
}

size_t fwrite(void *buf, size_t i, size_t j, FCGI_FILE *fp) {
    return FCGI_fwrite(buf,i,j,fp);
}

int fscanf(FCGI_FILE *fp, const char *format, ...) {
    char buf[BUFSIZ];
    fgets(buf,BUFSIZ,fp);
    buf[strlen(buf)-1]=0;
    va_list va;
    va_start(va, format);
    int i=vsscanf(buf,format,va);
    va_end(va);
    return i;
}

int feof(FCGI_FILE *f) {
    return FCGI_feof(f);
}

int ferror(FCGI_FILE *f) {
    return FCGI_ferror(f);
}

void perror(const char *s) {
    return FCGI_perror(s);
}

};  // end of namespace FCGI
 

#endif // _USING_FCGI_
