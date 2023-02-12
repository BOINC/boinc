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

#ifndef BOINC_MIOFILE_H
#define BOINC_MIOFILE_H

#include "boinc_stdio.h"

#include <string>

#include "mfile.h"


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
    char* wbuf;
    int len;
    const char* buf;
public:
    FILE* f;

    MIOFILE();
    ~MIOFILE();
    void init_mfile(MFILE*);
    void init_file(FILE*);
    void init_buf_read(const char*);
    void init_buf_write(char*, int len);
    int printf(const char* format, ...);
    char* fgets(char*, int);
    int _ungetc(int);
    bool eof();
    inline int _getc() {
        if (f) {
            return boinc::fgetc(f);
        }
        return (*buf)?(*buf++):EOF;
    }
};

extern int copy_element_contents(MIOFILE& in, const char* end_tag, char* p, int len);
extern int copy_element_contents(MIOFILE& in, const char* end_tag, std::string&);

#endif
