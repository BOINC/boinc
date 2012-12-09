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
#else
#include "config.h"
#include <cstdio>
#include <cstdlib>
#include <cstdarg>
#include <cstring>
#include <string>
#include <cerrno>
#include <unistd.h>
#endif

#include "error_numbers.h"
#include "filesys.h"

#include "mfile.h"

MFILE::MFILE() {
    buf = (char*)malloc(64*1024);
    len = 0;
}

MFILE::~MFILE() {
    if (buf) free(buf);
}

int MFILE::open(const char* path, const char* mode) {
    f = boinc_fopen(path, mode);
    if (!f) return ERR_FOPEN;
    if (!buf) buf = (char*)malloc(64*1024);
    return 0;
}

// seems like Win's realloc is stupid,  Make it smart.
//
static inline char* realloc_aux(char* p, size_t len) {
    if (!p) {
        return (char*)malloc(64*1024);
    }
#ifdef _WIN32
    if (_msize(p) >= (unsigned int)len) return p;
    return (char*) realloc(p, len*2);
#else
    return (char*) realloc(p, len);
#endif
}

#define BUFSIZE 100000

int MFILE::vprintf(const char* format, va_list ap) {
    char buf2[BUFSIZE];
    int n, k;

    k = vsnprintf(buf2, BUFSIZE, format, ap);
    if (k<=-1 || k>=BUFSIZE) {
        fprintf(stderr, "ERROR: buffer too small in MFILE::vprintf()\n");
        fprintf(stderr, "ERROR: format: %s\n", format);
        fprintf(stderr, "ERROR: k=%d, BUFSIZE=%d\n", k, BUFSIZE);
        return -1;
    }
    n = (int)strlen(buf2);
    buf = (char*)realloc_aux(buf, len+n+1);
    if (!buf) {
        fprintf(stderr,
            "ERROR: realloc() failed in MFILE::vprintf(); len %d n %d\n",
            len, n
        );
        exit(1);
    }
    strncpy(buf+len, buf2, n);
    len += n;
    buf[len] = 0;
    return k;
}

int MFILE::printf(const char* format, ...) {
    int n;
    va_list ap;

    va_start(ap, format);
    n = MFILE::vprintf(format, ap);
    va_end(ap);
    return n;
}

size_t MFILE::write(const void *ptr, size_t size, size_t nitems) {
    buf = (char *)realloc_aux( buf, len+(size*nitems)+1 );
    if (!buf) {
        fprintf(stderr,
            "ERROR: realloc() failed in MFILE::write(); len %d size %lu nitems %lu\n",
            len, size, nitems
        );
        exit(1);
    }
    memcpy( buf+len, ptr, size*nitems );
    len += (int)size*(int)nitems;
    buf[len] = 0;
    return nitems;
}

int MFILE::_putchar(char c) {
    buf = (char*)realloc_aux(buf, len+1+1);
    if (!buf) {
        fprintf(stderr,
            "ERROR: realloc() failed in MFILE::_putchar(); len %d\n", len
        );
        exit(1);
    }
    buf[len] = c;
    len++;
    buf[len] = 0;
    return c;
}

int MFILE::puts(const char* p) {
    int n = (int)strlen(p);
    buf = (char*)realloc_aux(buf, len+n+1);
    if (!buf) {
        fprintf(stderr,
            "ERROR: realloc() failed in MFILE::puts() len %d n %d\n", len, n
        );
        exit(1);
    }
    strncpy(buf+len, p, n);
    len += n;
    buf[len] = 0;
    return n;
}

int MFILE::close() {
    int retval = 0;
    if (f) {
        flush();
        fclose(f);
        f = NULL;
    }
    if (buf) {
        free(buf);
        buf = NULL;
    }
    return retval;
}

int MFILE::flush() {
    int n, old_len = len;

    n = (int)fwrite(buf, 1, len, f);
    len = 0;
    if (n != old_len) return ERR_FWRITE;
    if (fflush(f)) return ERR_FFLUSH;
#ifndef _WIN32
    if (fsync(fileno(f)) < 0) return ERR_FSYNC;
#endif
    return 0;
}

long MFILE::tell() const {
    return f ? ftell(f) : -1;
}

void MFILE::get_buf(char*& b, int& l) {
    b = buf;
    l = len;
    buf = 0;
    len = 0;
}

