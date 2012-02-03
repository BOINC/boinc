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
// asynchronous disk operations
//

#ifndef _ASYNC_FILE_
#define _ASYNC_FILE_

#include <vector>

struct FILE_INFO;
struct ACTIVE_TASK;

struct ASYNC_VERIFY {
    FILE_INFO* fip;

    int verify_chunk();
};

struct ASYNC_COPY {
    ACTIVE_TASK* atp;
    FILE* in, *out;
    char to_path[1024], temp_path[1024];

    ASYNC_COPY();
    ~ASYNC_COPY();

    int init(ACTIVE_TASK* _atp, const char* from_path, const char* _to_path);
    int copy_chunk();
    void error(int);
};

extern std::vector<ASYNC_VERIFY*> async_verifies;
extern std::vector<ASYNC_COPY*> async_copies;

extern void remove_async_copy(ASYNC_COPY*);
extern bool do_async_file_ops();

#endif
