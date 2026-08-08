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

#ifndef BOINC_MFILE_H
#define BOINC_MFILE_H

#include <cstdio>
#include <cstdarg>

// MFILE provides memory-buffered output with a FILE-type interface.
// BOINC uses this in a couple of places:
// 1) in the GUI RPC system (since in Windows you can't fdopen a socket)
// 2) in the API, to support a primitive form of checkpointing:
//    Write all your output (and restart file) to MFILEs.
//    The output is buffered in memory.
//    Then close or flush all the MFILEs;
//    all the buffers will be flushed to disk, almost atomically.

class MFILE {
    char* buf;      // NULL-terminated
    int len;
    FILE* f;
public:
    MFILE();
    ~MFILE();
    int open(const char* path, const char* mode);
    int _putchar(char);
    int puts(const char*);
    int vprintf(const char* format, va_list);
    int printf(const char* format, ...);
    size_t write(const void *, size_t size, size_t nitems);
    int close();
    int flush();
    long tell() const;
    void get_buf(char*&, int&);
        // get the MFILE's internal buffer and its length.
        // The caller assumes ownership of the buffer and must free() it.
        // The MFILE's buffer is set to empty
};

#endif
