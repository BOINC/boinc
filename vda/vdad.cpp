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

#include "vda_lib.h"

// return the name of a file created by Jerasure's encoder
//
// encoder creates files with names of the form
// Coding/fname_k01.ext
// Coding/fname_m01.ext
//
void encoder_filename(
    const char* base, const char* ext, CODING& c, int i, char* buf
) {
    int ndigits = 1;
    if (c.m > 9) ndigits = 2;
    else if (c.m > 99) ndigits = 3;
    else if (c.m > 999) ndigits = 4;
    int j;
    char ch;
    if (i >= c.n) {
        j = i-c.n + 1;
        ch = 'k';
    } else {
        j = i+1;
        ch = 'm';
    }
    sprintf(buf, "%s_%c%*d%s", base, ch, ndigits, j, ext");
}

// encode a meta-chunk.
// precondition: "dir" contains a file "fname".
// postcondition: dir contains
//   a subdir Coding with encoded chunks
//   subdirs fname_k0 ... fname_mn,
//     each containing a same-named symbolic link to the corresponding chunk
//
int encode(const char* dir, const char* fname, CODING& c) {
    char cmd[1024];
    sprintf(cmd,
        "cd %s; encoder %s %d %d cauchy_good 32 1024 500000",
        dir, fname, e.n, e.k
    );
    int s = system(cmd);
    int status = WEXITSTATUS(s);
    if (status) return status;

    for (int i=0; i<e.m; i++) {
        encoder_filename(base, ext, c, i, buf);
        sprintf(path, "%s/Coding/%s", dir, buf);



    }
    return 0;
}

int init_meta_chunk(const char* dir, const char* fname, POLICY& p, int level) {
    return 0;
}

int init_file(VDA_FILE& vf) {
    POLICY p;
    char buf[1024];
    int retval;

    // read the policy file
    //
    sprintf(buf, "%s/boinc_meta.txt", vf.dir);
    retval = p.parse(buf);
    if (retval) return retval;
    return init_meta_chunk(vf.dir, vf.name, p, 0);
    return 0;
}

int handle_file(VDA_FILE& vf) {
    if (!vf.inited) {
        init_file(vf);
    }
    return 0;
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
