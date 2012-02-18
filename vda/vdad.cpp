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

// vdad - volunteer data archival daemon
//
// Enumerates files needing updating from the DB.
// Creates the corresponding tree of META_CHUNKs, CHUNKs,
// and VDA_CHUNK_HOSTs.
// Calls the recovery routines to initiate transfers,
// update the DB, etc.

#include <sys/stat.h>
#include <sys/types.h>
#include <vector>

using std::vector;

#include "boinc_db.h"

#include "error_numbers.h"
#include "util.h"
#include "filesys.h"

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
//   subdirs fname_0 ... fname_m,
//     each containing a same-named symbolic link to the corresponding chunk
//
// The size of these chunks is returned in "size"
//
int encode(const char* dir, const char* fname, CODING& c, double& size) {
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
        if (i == 0) {
            file_size(target_path, size);
        }
    }
    return 0;
}

// initialize a meta-chunk: encode it,
// then recursively initialize its meta-chunk children
//
int META_CHUNK::init(const char* dir, const char* fname, POLICY& p, int level) {
    double size;
    CODING& c = p.codings[level];
    int retval = encode(dir, fname, c, size);
    if (retval) return retval;
    p.chunk_sizes[level] = size;
    if (level+1 < p.coding_levels) {
        for (int i=0; i<c.m; i++) {
            char child_dir[1024], child_fname[1024];
            sprintf(child_fname, "%s_%d", fname, i);
            sprintf(child_dir, "%s/%s", dir, child_fname);
            META_CHUNK* mc = new META_CHUNK();
            retval = mc->init(child_dir, child_fname, p, level+1);
            if (retval) return retval;
            children.push_back(mc);
        }
    } else {
        for (int i=0; i<c.m; i++) {
            CHUNK* cp = new CHUNK(this, p.chunk_sizes[level], i);
            children.push_back(cp);
        }
    }
    return 0;
}

// initialize a file: create its directory hierarchy
// and expand out its encoding tree,
// leaving only the bottom-level chunks
//
int VDA_FILE_AUX::init() {
    char buf[1024];
    meta_chunk = new META_CHUNK();
    int retval = meta_chunk->init(dir, name, policy, 0);
    if (retval) return retval;
    sprintf(buf, "%s/chunk_sizes.txt", dir);
    FILE* f = fopen(buf, "w");
    for (int i=0; i<policy.coding_levels; i++) {
        fprintf(f, "%.0f\n", policy.chunk_sizes[i]);
    }
    fclose(f);
    return 0;
}

int META_CHUNK::get_state(
    const char* dir, const char* fname, POLICY& p, int level
) {
    int retval;

    CODING& c = p.codings[level];
    if (level+1 < p.coding_levels) {
        for (int i=0; i<c.m; i++) {
            char child_dir[1024], child_fname[1024];
            sprintf(child_fname, "%s_%d", fname, i);
            sprintf(child_dir, "%s/%s", dir, child_fname);
            META_CHUNK* mc = new META_CHUNK();
            retval = mc->get_state(child_dir, child_fname, p, level+1);
            if (retval) return retval;
            children.push_back(mc);
        }
    } else {
        for (int i=0; i<c.m; i++) {
            CHUNK* ch = new CHUNK(this, p.chunk_sizes[level], i);
            children.push_back(ch);
        }
    }
    return 0;
}

int get_chunk_numbers(VDA_CHUNK_HOST& vch, vector<int>& chunk_numbers) {
    char* p, *q;
    p = vch.name;

    // find the last __ in filename
    //
    while (1) {
        q = strstr(p, "__");
        if (!q) {
            if (p == vch.name) return ERR_NOT_FOUND;
        } else {
            break;
        }
        p = q;
    }
    p += 2;
    while (p) {
        int i = atoi(p);
        chunk_numbers.push_back(i);
        p = strchr(p, '_');
    }
    return 0;
}

// get the state of an already-initialized file:
// expand the encoding tree,
// enumerate the VDA_HOST_CHUNKs from the DB
// and put them in the appropriate lists
//
int VDA_FILE_AUX::get_state() {
    char buf[256];

    sprintf(buf, "%s/chunk_sizes.txt", dir);
    FILE* f = fopen(buf, "r");
    if (!f) return -1;
    for (int i=0; i<policy.coding_levels; i++) {
        int n = fscanf(f, "%lf\n", &(policy.chunk_sizes[i]));
        if (n != 1) return -1;
    }
    fclose(f);
    int retval = meta_chunk->get_state(dir, name, policy, 0);
    if (retval) return retval;
    DB_VDA_CHUNK_HOST vch;
    sprintf(buf, "where vda_file_id=%d", id);
    while (1) {
        retval = vch.enumerate(buf);
        if (retval == ERR_DB_NOT_FOUND) break;
        if (retval) return retval;
        vector<int> chunk_numbers;
        retval = get_chunk_numbers(vch, chunk_numbers);
        if (retval) return retval;
        if ((int)(chunk_numbers.size()) != policy.coding_levels) {
            return -1;
        }
        META_CHUNK* mc = meta_chunk;
        for (int i=0; i<policy.coding_levels; i++) {
            if (i == policy.coding_levels-1) {
                CHUNK* c = (CHUNK*)(mc->children[chunk_numbers[i]]);
                VDA_CHUNK_HOST* vchp = new VDA_CHUNK_HOST();
                *vchp = vch;
                c->hosts.insert(vchp);
            } else {
                mc = (META_CHUNK*)(mc->children[chunk_numbers[i]]);
            }
        }

    }
    return 0;
}

int handle_file(VDA_FILE_AUX& vf) {
    int retval;
    char buf[1024];

    // read the policy file
    //
    sprintf(buf, "%s/boinc_meta.txt", vf.dir);
    retval = vf.policy.parse(buf);
    if (retval) {
        fprintf(stderr, "Can't parse policy file %s\n", buf);
        return retval;
    }
    if (vf.inited) {
        retval = vf.get_state();
        if (retval) return retval;
    } else {
        retval = vf.init();
        if (retval) return retval;
    }
    vf.meta_chunk->recovery_plan();
    vf.meta_chunk->recovery_action(dtime());
    return 0;
}

// handle files
//
bool scan_files() {
    DB_VDA_FILE vf;
    bool found = false;
    int retval;

    while (vf.enumerate("need_update<>0")) {
        VDA_FILE_AUX vfa(vf);
        found = true;
        retval = handle_file(vfa);
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
#if 1
    VDA_FILE_AUX vf;
    memset(&vf, 0, sizeof(vf));
    strcpy(vf.dir, "/mydisks/b/users/boincadm/vda_test");
    strcpy(vf.name, "file.ext");
    handle_file(vf);
    exit(0);
#endif
    while(1) {
        bool action = scan_files();
        action |= scan_chunks();
        if (!action) boinc_sleep(5.);
    }
}
