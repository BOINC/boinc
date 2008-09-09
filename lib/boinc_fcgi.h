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
#ifdef _USING_FCGI_
#include <fcgi_stdio.h>
// Undefine the macros that can be replaced with appropriate overloads
#undef fread
#undef fwrite
#undef fscanf
#undef setbuf
#undef fputs
#undef fgets
#undef fgetc
#undef vfprintf
#undef fprintf
#undef fopen
#undef fflush
#undef fclose
#undef freopen
#undef ferror
#undef feof
#endif
#ifndef BOINC_FCGI_H
#define BOINC_FCGI_H

// This file is an attempt to fix some of the epidemic build problems
// caused by libraries redefining the C stdio.h functions that has struck.  
// It's far better to use overloads in a C++ namespace to fix these issues 
// for a variety of reasons.

#ifdef _USING_FCGI_

#if defined(__cplusplus) || defined(c_plusplus)

namespace FCGI {

FCGI_FILE *fopen(const char *path, const char *mode);

FCGI_FILE *freopen(const char *path, const char *mode, FCGI_FILE *file);

int fflush(FCGI_FILE *file);

int fprintf(FCGI_FILE *fp, const char *format, ...);

int vfprintf(FCGI_FILE *fp, const char *format, va_list ap);

char *fgets(char *str, int size, FCGI_FILE *fp);

int fputs(const char *str, FCGI_FILE *fp);

int fclose(FCGI_FILE *f);

size_t fread(void *buf, size_t i, size_t j, FCGI_FILE *fp);

size_t fwrite(void *buf, size_t i, size_t j, FCGI_FILE *fp);

int fscanf(FCGI_FILE *fp, const char *format, ...);

void setbuf(FCGI_FILE *f, char *buf);

int fgetc(FCGI_FILE *f);

int feof(FCGI_FILE *f);

int ferror(FCGI_FILE *f);

// More left to do here.  Just the minimum for BOINC done.

};  // end of namespace FCGI
 
using namespace FCGI;
#endif // __cplusplus

#endif // _USING_FCGI_
#endif // BOINC_FCGI_H
