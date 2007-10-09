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

#ifndef _MIOFILE_
#define _MIOFILE_

#ifndef _WIN32
#include <string>
#endif

#include "mfile.h"

#ifdef _USING_FCGI_
#include "fcgi_stdio.h"
#endif

// MIOFILE lets you do formatted I/O to either a FILE or a memory buffer,
// depending on how you initialize it.
//
// output:
//  init_file(): output goes to the FILE* that you specify
//  init_mfile(): output goes to an MFILE (i.e. a memory buffer);
//     you can call MFILE::get_buf() to get the results
// input:
//  init_file(): input comes from the FILE* that you specify
//  init_buf(): input comes from the buffer you specify.
//   This string is not modified.
//
// Why is this here?  Because on Windows (9x, maybe all)
// you can't do fdopen() on a socket.

class MIOFILE {
    MFILE* mf;
    FILE* f;
    char* wbuf;
    int len;
	const char* buf;
public:
    MIOFILE();
    ~MIOFILE();
    void init_mfile(MFILE*);
    void init_file(FILE*);
    void init_buf_read(const char*);
	void init_buf_write(char*, int len);
    int printf(const char* format, ...);
    char* fgets(char*, int);
    int _ungetc(int);
    inline int _getc() {
        if (f) {
#ifdef _USING_FCGI_
            return FCGI_fgetc(f);
#else
            return getc(f);
#endif
        }
        return (*buf)?(*buf++):EOF;
    }
};

extern int copy_element_contents(MIOFILE& in, const char* end_tag, char* p, int len);
extern int copy_element_contents(MIOFILE& in, const char* end_tag, std::string&);

#endif
