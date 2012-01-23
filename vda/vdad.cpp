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

#include "boinc_db.h"

#include "util.h"

// encode a meta-chunk.
// precondition: "dir" contains a file "fname".
// postcondition: dir contains
//   a subdir Coding with encoded chunks
//   subdirs fname_k0 ... fname_mn,
//     each containing a same-named symbolic link to the corresponding chunk
//
int encode(ENCODING& e) {
}

int init_file(VDA_FILE& vf) {
}

void handle_file(VDA_FILE& vf) {
    if (!vf.inited) {
        init_file(vf);
    }
}

bool scan_files() {
    DB_VDA_FILE vf;
    bool found = false;

    while (vf.enumerate("need_update<>0")) {
        found = true;
        handle_file(vf);
    }
    return found;
}

void handle_chunk(VDA_CHUNK_HOST& ch) {
}

bool scan_chunks() {
    DB_VDA_CHUNK_HOST ch;
    char buf[256];
    bool found = false;

    sprintf(buf, "transition_time < %f", dtime());
    while (ch.enumerate(buf)) {
        found = true;
        handle_chunk(ch);
    }
    return found;
}

int main(int argc, char** argv) {
    while(1) {
        bool action = scan_files();
        action |= scan_chunks();
        if (!action) boinc_sleep(5.);
    }
}
