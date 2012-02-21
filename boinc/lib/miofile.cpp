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
#include <string>
#include <cstring>
#include <cstdarg>
#endif

#include "error_numbers.h"
#include "miofile.h"

using std::string;

MIOFILE::MIOFILE() {
    mf = 0;
    f = 0;
    buf = 0;
}

MIOFILE::~MIOFILE() {
}

void MIOFILE::init_mfile(MFILE* _mf) {
    mf = _mf;
}

void MIOFILE::init_file(FILE* _f) {
    f = _f;
}

void MIOFILE::init_buf_read(const char* _buf) {
    buf = _buf;
}

void MIOFILE::init_buf_write(char* _buf, int _len) {
    wbuf = _buf;
    len = _len;
	wbuf[0] = 0;
}

bool MIOFILE::eof() {
    if (f) {
        if (!feof(f)) {
            return false;
        }
    }
    return true;
}

#ifndef _USING_FCGI_

int MIOFILE::printf(const char* format, ...) {
    int retval;

    va_list ap;
    va_start(ap, format);
    if (mf) {
        retval = mf->vprintf(format, ap);
    } else if (f) {
        retval = vfprintf(f, format, ap);
    } else {
        size_t cursize = strlen(wbuf);
        size_t remaining_len = len - cursize;
        retval = vsnprintf(wbuf+cursize, remaining_len, format, ap);
    }
    va_end(ap);
    return retval;
}

#endif

char* MIOFILE::fgets(char* dst, int dst_len) {
    if (f) {
#ifndef _USING_FCGI_
        return ::fgets(dst, dst_len, f);
#else
        return FCGI::fgets(dst, dst_len, f);
#endif
    }
    const char* q = strchr(buf, '\n');
    if (!q) return 0;

    q++;
    int n = (int)(q - buf);
    if (n > dst_len-1) n = dst_len-1;
    memcpy(dst, buf, n);
    dst[n] = 0;

    buf = q;
    return dst;
}

int MIOFILE::_ungetc(int c) {
    if (f) {
#ifdef _USING_FCGI_
        return FCGI_ungetc(c, f);
#else
        return ungetc(c, f);
#endif
    } else {
        buf--;
        // NOTE: we assume that the char being pushed
        // is what's already there
        //*buf = c;
    }
    return c;
}

// copy from a file to static buffer
//
int copy_element_contents(MIOFILE& in, const char* end_tag, char* p, int len) {
    char buf[256];
    int n;

    strcpy(p, "");
    while (in.fgets(buf, 256)) {
        if (strstr(buf, end_tag)) {
            return 0;
        }
        n = (int)strlen(buf);
        if (n >= len-1) return ERR_XML_PARSE;
        strcat(p, buf);
        len -= n;
    }
    return ERR_XML_PARSE;
}

int copy_element_contents(MIOFILE& in, const char* end_tag, string& str) {
    char buf[256];

    str = "";
    while (in.fgets(buf, 256)) {
        if (strstr(buf, end_tag)) {
            return 0;
        }
        str += buf;
    }
    fprintf(stderr, "copy_element_contents(): no end tag\n");
    return ERR_XML_PARSE;
}

