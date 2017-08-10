// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2012 University of California
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

// asynchronous file operations
//

#ifndef BOINC_ASYNC_FILE_H
#define BOINC_ASYNC_FILE_H

#include <vector>

#ifdef _WIN32
#include "zlib.h"
#else
#include <zlib.h>
#endif

#include "str_replace.h"
#include "filesys.h"
#include "md5.h"

struct FILE_INFO;
struct ACTIVE_TASK;

#define ASYNC_FILE_THRESHOLD    1e7
    // use async ops for files exceeding this size

// Used to copy a file from project dir to slot dir;
// when done, start the task again.
//
struct ASYNC_COPY {
    ACTIVE_TASK* atp;
    FILE_INFO* fip;
    FILE* in, *out;
    char to_path[MAXPATHLEN], temp_path[MAXPATHLEN];

    ASYNC_COPY();
    ~ASYNC_COPY();

    int init(
        ACTIVE_TASK*, FILE_INFO*, const char* from_path, const char* _to_path
    );
    int copy_chunk();
    void error(int);
};

// Used to verify and possibly decompress a file
// after it has been downloaded.
// When done, mark it as present.
//
struct ASYNC_VERIFY {
    FILE_INFO* fip;
    md5_state_t md5_state;
    FILE* in, *out;
    gzFile gzin;
    char inpath[MAXPATHLEN], temp_path[MAXPATHLEN], outpath[MAXPATHLEN];

    ASYNC_VERIFY(){
      fip = NULL;
      in = NULL;
      out = NULL;
      gzin = NULL;
      safe_strcpy(inpath, "");
      safe_strcpy(temp_path, "");
      safe_strcpy(outpath, "");
    };
    ~ASYNC_VERIFY(){};

    int init(FILE_INFO*);
    int verify_chunk();
    void finish();
    void error(int);
};

extern std::vector<ASYNC_VERIFY*> async_verifies;
extern std::vector<ASYNC_COPY*> async_copies;

extern void remove_async_copy(ASYNC_COPY*);
extern void remove_async_verify(ASYNC_VERIFY*);
inline bool have_async_file_op() {
    return (async_verifies.size() || async_copies.size());
}
extern void do_async_file_op();

#endif
