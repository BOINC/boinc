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
#include "boinc_win.h"
#endif

#ifndef _WIN32
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <string>
#include <errno.h>
#if HAVE_MALLOC_H
#include <malloc.h>
#endif

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
    if (!f) return -1;
    return 0;
}

int MFILE::vprintf(const char* format, va_list ap) {
    char buf2[20000];
    int n, k;

    k = vsprintf(buf2, format, ap);
    n = (int)strlen(buf2);
    buf = (char*)realloc(buf, len+n);
    if (!buf) {
        errno = ERR_MALLOC;
        return -1;
    }
    strncpy(buf+len, buf2, n);
    len += n;
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
    buf = (char *)realloc( buf, len+(size*nitems) );
    if (!buf) {
        errno = ERR_MALLOC;
        return 0;
    }
    memcpy( buf+len, ptr, size*nitems );
    len += (int)size*(int)nitems;
    return nitems;
}

int MFILE::_putchar(char c) {
    buf = (char*)realloc(buf, len+1);
    if (!buf) {
        errno = ERR_MALLOC;
        return EOF;
    }
    buf[len] = c;
    len++;
    return c;
}

int MFILE::puts(const char* p) {
    int n = (int)strlen(p);
    buf = (char*)realloc(buf, len+n);
    if (!buf) {
        errno = ERR_MALLOC;
        return EOF;
    }
    strncpy(buf+len, p, n);
    len += n;
    return n;
}

int MFILE::close() {
    fwrite(buf, 1, len, f);
    free(buf);
    buf = 0;
    return fclose(f);
}

int MFILE::flush() {
    fwrite(buf, 1, len, f);
    len = 0;
    return fflush(f);
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
