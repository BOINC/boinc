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

// This file is an attempt to fix some of the epidemic build problems
// caused by libraries redefining the C stdio.h functions that has struck.
// It's far better to use overloads in a C++ namespace to fix these issues
// for a variety of reasons.

#ifndef BOINC_STDIO_H
#define BOINC_STDIO_H

#ifdef _USING_FCGI_
#define NO_FCGI_DEFINES
#include <fcgi_stdio.h>
#define FILE      FCGI_FILE
#define stdin     FCGI_stdin
#define stdout    FCGI_stdout
#define stderr    FCGI_stderr
#else
#if defined(__cplusplus) || defined(c_plusplus)
#include <cstdio>
#else
#include <stdio.h>
#endif
#endif

#if defined(__cplusplus) || defined(c_plusplus)

#include <cstring>
#include <cstdarg>

namespace boinc {
    inline FILE* fopen(const char* path, const char* mode) {
#ifdef _USING_FCGI_
        return FCGI_fopen(path, mode);
#else
        return ::fopen(path, mode);
#endif
    }

    inline int fgetc(FILE *f) {
#ifdef _USING_FCGI_
        return FCGI_fgetc(f);
#else
        return ::fgetc(f);
#endif
    }

    inline int fprintf(FILE *fp, const char *format, ...) {
        va_list va;
        va_start(va, format);
#ifdef _USING_FCGI_
        int i=FCGI_vfprintf(fp,format,va);
#else
        int i=::vfprintf(fp,format,va);
#endif
        va_end(va);
        return i;
    }

    inline int fileno(FILE *f) {
#ifdef _USING_FCGI_
        return FCGI_fileno(f);
#else
#ifdef _WIN32
        return ::_fileno(f);
#else
        return ::fileno(f);
#endif
#endif
    }

    inline int fclose(FILE *f) {
#ifdef _USING_FCGI_
        return FCGI_fclose(f);
#else
        return ::fclose(f);
#endif
    }

    inline size_t fread(void *buf, size_t i, size_t j, FILE *fp) {
#ifdef _USING_FCGI_
        return FCGI_fread(buf,i,j,fp);
#else
        return ::fread(buf,i,j,fp);
#endif
    }

    inline int feof(FILE *f) {
#ifdef _USING_FCGI_
        return FCGI_feof(f);
#else
        return ::feof(f);
#endif
    }

    inline size_t fwrite(void *buf, size_t i, size_t j, FILE *fp) {
#ifdef _USING_FCGI_
        return FCGI_fwrite(buf,i,j,fp);
#else
        return ::fwrite(buf,i,j,fp);
#endif
    }

    inline FILE* fdopen(int fd, const char *mode) {
#ifdef _USING_FCGI_
        return FCGI_fdopen(fd,mode);
#else
        return ::fdopen(fd,mode);
#endif
    }

    inline void perror(const char *s) {
#ifdef _USING_FCGI_
        FCGI_perror(s);
#else
        ::perror(s);
#endif
    }

    inline char *fgets(char *str, int size, FILE *fp) {
#ifdef _USING_FCGI_
        return FCGI_fgets(str,size,fp);
#else
        return ::fgets(str,size,fp);
#endif
    }

    inline int fflush(FILE *file) {
#ifdef _USING_FCGI_
        return FCGI_fflush(file);
#else
        return ::fflush(file);
#endif
    }

    inline int ftell(FILE *f) {
#ifdef _USING_FCGI_
        return FCGI_ftell(f);
#else
        return ::ftell(f);
#endif
    }

    inline int vfprintf(FILE *fp, const char *format, va_list ap) {
#ifdef _USING_FCGI_
        return FCGI_vfprintf(fp,format,ap);
#else
        return ::vfprintf(fp,format,ap);
#endif
    }

    inline int ungetc(int c, FILE *f) {
#ifdef _USING_FCGI_
        return FCGI_ungetc(c,f);
#else
        return ::ungetc(c,f);
#endif
    }

    inline FILE* freopen(const char *path, const char *mode, FILE *file) {
#ifdef _USING_FCGI_
        return FCGI_freopen(path,mode,file);
#else
        return ::freopen(path,mode,file);
#endif
    }

    inline int fscanf(FILE *fp, const char *format, ...) {
        char buf[BUFSIZ];
        boinc::fgets(buf,BUFSIZ,fp);
        va_list va;
        va_start(va, format);
        int i=::vsscanf(buf,format,va);
        va_end(va);
        return i;
    }

    inline int fputs(const char *str, FILE *fp) {
#ifdef _USING_FCGI_
        return FCGI_fputs(str,fp);
#else
        return ::fputs(str,fp);
#endif
    }

    inline int ferror(FILE *f) {
#ifdef _USING_FCGI_
        return FCGI_ferror(f);
#else
        return ::ferror(f);
#endif
    }

}

#endif
#endif // BOINC_STDIO_H
