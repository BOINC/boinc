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

#include <sys/stat.h>
#include <sys/types.h>

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
        ch = 'm';
    } else {
        j = i+1;
        ch = 'k';
    }
    sprintf(buf, "%s_%c%0*d%s", base, ch, ndigits, j, ext);
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
        "cd %s; /mydisks/b/users/boincadm/vda_test/encoder %s %d %d cauchy_good 32 1024 500000",
        dir, fname, c.n, c.k
    );
    printf("%s\n", cmd);
    int s = system(cmd);
    if (WIFEXITED(s)) {
        int status = WEXITSTATUS(s);
        if (status != 32) return -1;    // encoder returns 32 for some reason
    }

    char base[256], ext[256];
    strcpy(base, fname);
    char* p = strchr(base, '.');
    if (p) {
        strcpy(ext, p);
        *p = 0;
    } else {
        strcpy(ext, "");
    }
    for (int i=0; i<c.m; i++) {
        char enc_filename[1024], target_path[1024], chunk_name[1024];
        char dir_name[1024], link_name[1024];
        encoder_filename(base, ext, c, i, enc_filename);
        sprintf(target_path, "%s/Coding/%s", dir, enc_filename);
        sprintf(chunk_name, "%s_%d", fname, i);
        sprintf(dir_name, "%s/%s", dir, chunk_name);
        int retval = mkdir(dir_name, 0777);
        if (retval) {
            perror("mkdir");
            return retval;
        }
        sprintf(link_name, "%s/%s", dir_name, chunk_name);
        retval = symlink(target_path, link_name);
        if (retval) {
            perror("symlink");
            return retval;
        }
    }
    return 0;
}

int init_meta_chunk(const char* dir, const char* fname, POLICY& p, int level) {
    CODING& c = p.codings[level];
    int retval = encode(dir, fname, c);
    if (retval) return retval;
    if (level+1 < p.coding_levels) {
        for (int i=0; i<c.m; i++) {
            char child_dir[1024], child_fname[1024];
            sprintf(child_fname, "%s_%d", fname, i);
            sprintf(child_dir, "%s/%s", dir, child_fname);
            retval = init_meta_chunk(child_dir, child_fname, p, level+1);
            if (retval) return retval;
        }
    }
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
    if (retval) {
        fprintf(stderr, "Can't parse policy file\n");
        return retval;
    }
    return init_meta_chunk(vf.dir, vf.name, p, 0);
    return 0;
}

int handle_file(VDA_FILE& vf) {
    if (!vf.inited) {
        init_file(vf);
    }
    return 0;
}

// handle files
//
bool scan_files() {
    DB_VDA_FILE vf;
    bool found = false;
    int retval;

    while (vf.enumerate("need_update<>0")) {
        found = true;
        retval = handle_file(vf);
        if (retval) {
            fprintf(stderr, "handle_file() failed: %d\n", retval);
        } else {
            vf.need_update = 0;
            vf.update();
        }
    }
    return found;
}

void handle_chunk(VDA_CHUNK_HOST& ch) {
    DB_VDA_FILE
}

// handle timed-out transfers
//
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
    VDA_FILE vf;
    strcpy(vf.dir, "/mydisks/b/users/boincadm/vda_test");
    strcpy(vf.name, "file.ext");
    init_file(vf);
    exit(0);
    while(1) {
        bool action = scan_files();
        action |= scan_chunks();
        if (!action) boinc_sleep(5.);
    }
}
