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

#ifdef _WIN32
#include "boinc_win.h"
#else
#include <string.h>
#endif

#include "error_numbers.h"
#include "filesys.h"

#include "app.h"
#include "client_state.h"

#include "async_file.h"

using std::vector;

vector<ASYNC_VERIFY*> async_verifies;
vector<ASYNC_COPY*> async_copies;

int ASYNC_COPY::init(
    ACTIVE_TASK* _atp, const char* from_path, const char* _to_path
) {
    atp = _atp;
    strcpy(to_path, _to_path);
    in = fopen(from_path, "rb");
    if (!in) return ERR_FOPEN;
    strcpy(temp_path, to_path);
    char* p = strrchr(temp_path, '/');
    strcpy(p+1, "copy_temp");
    out = fopen(temp_path, "wb");
    if (!out) {
        fclose(in);
        return ERR_FOPEN;
    }
    return 0;
}

ASYNC_COPY::ASYNC_COPY() {
    in = out = NULL;
    atp = NULL;
}

ASYNC_COPY::~ASYNC_COPY() {
    if (in) fclose(in);
    if (out) fclose(out);
    if (atp) {
        atp->async_copy = NULL;
    }
}

// copy a 64KB chunk.
// return nonzero if we're done (success or fail)
//
int ASYNC_COPY::copy_chunk() {
    unsigned char buf[64*1024];
    int retval;

    int n = fread(buf, 1, sizeof(buf), in);
    if (n <= 0) {
        // copy done.  rename temp file
        //
        fclose(in);
        fclose(out);
        in = out = NULL;
        retval = boinc_rename(temp_path, to_path);
        if (retval) {
            error(retval);
            return 1;
        }
        // If task is still scheduled, start it.
        //
        if (atp->scheduler_state == CPU_SCHED_SCHEDULED) {
            retval = atp->start(true);
            if (retval) {
                error(retval);
            }
        }
        return 1;       // tell caller we're done
    } else {
        int m = fwrite(buf, 1, n, out);
        m = 0;
        if (m != n) {
            error(ERR_FWRITE);
            return 1;
        }
    }
    return 0;
}

// handle the failure of a copy; error out the result
//
void ASYNC_COPY::error(int retval) {
    atp->set_task_state(PROCESS_COULDNT_START, "ASYNC_COPY::error");
    gstate.report_result_error(
        *(atp->result), "Couldn't copy file: %s", boincerror(retval)
    );
    gstate.request_schedule_cpus("start failed");
}

void remove_async_copy(ASYNC_COPY* acp) {
    vector<ASYNC_COPY*>::iterator i = async_copies.begin();
    while (i != async_copies.end()) {
        if (*i == acp) {
            async_copies.erase(i);
            break;
        }
        i++;
    }
    delete acp;
}

int ASYNC_VERIFY::verify_chunk() {
    return 0;
}

// If there are any async file operations,
// do a 64KB chunk of the first one and return true.
//
// Note: if there are lots of pending operations,
// it's better to finish the oldest one before starting the rest
//
bool do_async_file_ops() {
    if (async_copies.size()) {
        ASYNC_COPY* acp = async_copies[0];
        if (acp->copy_chunk()) {
            async_copies.erase(async_copies.begin());
            delete acp;
        }
        return true;
    }
    if (async_verifies.size()) {
        if (async_verifies[0]->verify_chunk()) {
            async_verifies.erase(async_verifies.begin());
        }
        return true;
    }
    return false;
}

