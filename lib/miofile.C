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

#ifdef _WIN32
#include "boinc_win.h"
#endif

#ifndef _WIN32
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

void MIOFILE::init_buf(char* _buf) {
    buf = _buf;
}

int MIOFILE::printf(const char* format, ...) {
    int retval;

    va_list ap;
    va_start(ap, format);
    if (mf) {
        retval = mf->vprintf(format, ap);
    } else {
        retval = vfprintf(f, format, ap);
    }
    va_end(ap);
    return retval;
}

char* MIOFILE::fgets(char* dst, int len) {
    if (f) {
        return ::fgets(dst, len, f);
    }
    char* q = strchr(buf, '\n');
    if (!q) return 0;

    q++;
    int n = (int)(q - buf);
    if (n > len-1) n = len-1;
    memcpy(dst, buf, n);
    dst[n] = 0;

    buf = q;
    return dst;
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
        n = strlen(buf);
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

const char *BOINC_RCSID_37339d4dc0 = "$Id$";
