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

#ifndef _MFILE_
#define _MFILE_

#include <stdio.h>
#include <stdarg.h>

// MFILE supports a primitive form of checkpointing.
// Write all your output (and restart file) to MFILEs.
// The output is buffered in memory.
// Then close or flush all the MFILEs;
// all the buffers will be flushed to disk, almost atomically.

class MFILE {
    char* buf;
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
