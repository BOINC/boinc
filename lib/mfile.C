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
// 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

#if defined(_WIN32) && !defined(__STDWX_H__) && !defined(_BOINC_WIN_) && !defined(_AFX_STDAFX_H_)
#include "boinc_win.h"
#endif

#ifndef _WIN32
#include "config.h"
#include <cstdio>
#include <cstdlib>
#include <cstdarg>
#include <cstring>
#include <string>
#include <cerrno>
#if HAVE_MALLOC_H
#include <malloc.h>
#endif
#include <unistd.h>

using namespace std;
#endif

#include "filesys.h"
#include "error_numbers.h"
#include "mfile.h"


MFILE::MFILE() {
    buf = 0;
    len = 0;
}

MFILE::~MFILE() {
    if (buf) free(buf);
}

int MFILE::open(const char* path, const char* mode) {
    f = boinc_fopen(path, mode);
    if (!f) return ERR_FOPEN;
    return 0;
}

int MFILE::vprintf(const char* format, va_list ap) {
    char buf2[65536];
    int n, k;

    k = vsprintf(buf2, format, ap);
    n = (int)strlen(buf2);
    buf = (char*)realloc(buf, len+n+1);
    if (!buf) {
        errno = ERR_MALLOC;
        return ERR_MALLOC;
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
    buf = (char *)realloc( buf, len+(size*nitems)+1 );
    if (!buf) {
        errno = ERR_MALLOC;
        return 0;
    }
    memcpy( buf+len, ptr, size*nitems );
    len += (int)size*(int)nitems;
    buf[len] = 0;
    return nitems;
}

int MFILE::_putchar(char c) {
    buf = (char*)realloc(buf, len+1+1);
    if (!buf) {
        errno = ERR_MALLOC;
        return EOF;
    }
    buf[len] = c;
    len++;
    buf[len] = 0;
    return c;
}

int MFILE::puts(const char* p) {
    int n = (int)strlen(p);
    buf = (char*)realloc(buf, len+n+1);
    if (!buf) {
        errno = ERR_MALLOC;
        return EOF;
    }
    strncpy(buf+len, p, n);
    len += n;
    buf[len] = 0;
    return n;
}

int MFILE::close() {
    int retval = flush();
    fclose(f);
    free(buf);
    buf = 0;
    return retval;
}

int MFILE::flush() {
    int n, old_len = len;

    n = fwrite(buf, 1, len, f);
    len = 0;
    if (n != old_len) return ERR_FWRITE;
    if (fflush(f)) return ERR_FFLUSH;
    if (fsync(fileno(f)) < 0) return ERR_FSYNC;
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

const char *BOINC_RCSID_8de9facdd7 = "$Id$";
