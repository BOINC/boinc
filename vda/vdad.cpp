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
#include "sched_config.h"

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
// Assume there's no extension
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
    sprintf(buf, "%s_%c%0*d.%s", base, ch, ndigits, j, ext);
}

#define DATA_FILENAME "data"
#define DATA_EXT "ext"

// encode a meta-chunk.
// precondition: "dir" contains a file "data".
// postcondition: dir contains
//   a subdir Coding with encoded chunks
//   subdirs data.0 .. data.m
//     each containing a same-named symbolic link to the corresponding chunk
//
// The size of these chunks is returned in "size"
//
int encode(const char* dir, CODING& c, double& size) {
    char cmd[1024];
    sprintf(cmd,
        "cd %s; /mydisks/b/users/boincadm/vda_test/encoder %s.%s %d %d cauchy_good 32 1024 500000",
        dir, DATA_FILENAME, DATA_EXT, c.n, c.k
    );
    printf("%s\n", cmd);
    int s = system(cmd);
    if (WIFEXITED(s)) {
        int status = WEXITSTATUS(s);
        if (status != 32) return -1;    // encoder returns 32 for some reason
    }

    // make symlinks
    //
    for (int i=0; i<c.m; i++) {
        char enc_filename[1024], target_path[1024], chunk_name[1024];
        char dir_name[1024], link_name[1024];
        encoder_filename(DATA_FILENAME, DATA_EXT, c, i, enc_filename);
        sprintf(target_path, "%s/Coding/%s", dir, enc_filename);
        sprintf(chunk_name, "%s.%d", DATA_FILENAME, i);
        sprintf(dir_name, "%s/%s", dir, chunk_name);
        int retval = mkdir(dir_name, 0777);
        if (retval) {
            perror("mkdir");
            return retval;
        }
        sprintf(link_name, "%s/%s.%s", dir_name, DATA_FILENAME, DATA_EXT);
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

META_CHUNK::META_CHUNK(VDA_FILE_AUX* d, META_CHUNK* p, int index) {
    dfile = d;
    parent = p;
    if (parent) {
        if (strlen(parent->name)) {
            sprintf(name, "%s.%d", parent->name, index);
        } else {
            sprintf(name, "%d", index);
        }
    } else {
        strcpy(name, "");
    }
}

// initialize a meta-chunk:
// encode it, then recursively initialize its meta-chunk children
//
int META_CHUNK::init(const char* dir, POLICY& p, int level) {
    double size;

    CODING& c = p.codings[level];
    int retval = encode(dir, c, size);
    if (retval) return retval;
    p.chunk_sizes[level] = size;

    if (level+1 < p.coding_levels) {
        for (int i=0; i<c.m; i++) {
            char child_dir[1024];
            sprintf(child_dir, "%s/%s.%d", dir, DATA_FILENAME, i);
            META_CHUNK* mc = new META_CHUNK(dfile, parent, i);
            retval = mc->init(child_dir, p, level+1);
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
    char buf[1024], buf2[1024];
    sprintf(buf, "%s/%s.%s", dir, DATA_FILENAME, DATA_EXT);
    sprintf(buf2, "%s/%s", dir, name);
    int retval = symlink(buf2, buf);
    if (retval) {
        return ERR_SYMLINK;
    }
    meta_chunk = new META_CHUNK(this, NULL, 0);
    retval = meta_chunk->init(dir, policy, 0);
    if (retval) return retval;
    sprintf(buf, "%s/chunk_sizes.txt", dir);
    FILE* f = fopen(buf, "w");
    for (int i=0; i<policy.coding_levels; i++) {
        fprintf(f, "%.0f\n", policy.chunk_sizes[i]);
    }
    fclose(f);
    return 0;
}

int META_CHUNK::get_state(const char* dir, POLICY& p, int level) {
    int retval;

    CODING& c = p.codings[level];
    if (level+1 < p.coding_levels) {
        for (int i=0; i<c.m; i++) {
            char child_dir[1024];
            sprintf(child_dir, "%s/%s.%d", dir, DATA_FILENAME, i);
            META_CHUNK* mc = new META_CHUNK(dfile, this, i);
            retval = mc->get_state(child_dir, p, level+1);
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
    char* p = vch.name;
    while (1) {
        p = strchr(p, '.');
        if (!p) break;
        p++;
        int i = atoi(p);
        chunk_numbers.push_back(i);
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
    meta_chunk = new META_CHUNK(this, NULL, 0);
    int retval = meta_chunk->get_state(dir, policy, 0);
    if (retval) return retval;
    DB_VDA_CHUNK_HOST vch;
    sprintf(buf, "where vda_file_id=%d", id);
    while (1) {
        retval = vch.enumerate(buf);
        if (retval == ERR_DB_NOT_FOUND) break;
        if (retval) return retval;
        vector<int> chunk_numbers;
        retval = get_chunk_numbers(vch, chunk_numbers);
        if (retval) {
            log_messages.printf(MSG_CRITICAL,
                "get_chunk_numbers(): %d\n", retval
            );
            return retval;
        }
        if ((int)(chunk_numbers.size()) != policy.coding_levels) {
            log_messages.printf(MSG_CRITICAL,
                "wrong get_chunk_numbers: got %d, expected %d\n",
                (int)(chunk_numbers.size()), policy.coding_levels
            );
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

int handle_file(VDA_FILE_AUX& vf, DB_VDA_FILE& dvf) {
    int retval;
    char buf[1024];

    log_messages.printf(MSG_NORMAL, "processing file %s\n", vf.name);

    // read the policy file
    //
    sprintf(buf, "%s/boinc_meta.txt", vf.dir);
    retval = vf.policy.parse(buf);
    if (retval) {
        log_messages.printf(MSG_CRITICAL, "Can't parse policy file %s\n", buf);
        return retval;
    }
    if (vf.initialized) {
        retval = vf.get_state();
        if (retval) {
            log_messages.printf(MSG_CRITICAL, "vf.get_state failed %d\n", retval);
            return retval;
        }
    } else {
        retval = vf.init();
        if (retval) {
            log_messages.printf(MSG_CRITICAL, "vf.init failed %d\n", retval);
            return retval;
        }
        dvf.update_field("initialized=1");
    }
    retval = vf.meta_chunk->recovery_plan();
    if (retval) {
        log_messages.printf(MSG_CRITICAL, "vf.recovery_plan failed %d\n", retval);
        return retval;
    }
    retval = vf.meta_chunk->recovery_action(dtime());
    if (retval) {
        log_messages.printf(MSG_CRITICAL, "vf.recovery_action failed %d\n", retval);
        return retval;
    }
    return 0;
}

// handle files
//
bool scan_files() {
    DB_VDA_FILE vf;
    bool found = false;
    int retval;

    while (1) {
        retval = vf.enumerate("where need_update<>0");
        if (retval == ERR_DB_NOT_FOUND) break;
        if (retval) {
            log_messages.printf(MSG_CRITICAL, "VDA_FILE enumerate failed\n");
            exit(1);
        }
        VDA_FILE_AUX vfa(vf);
        found = true;
        retval = handle_file(vfa, vf);
        if (retval) {
            log_messages.printf(
                MSG_CRITICAL, "handle_file() failed: %d\n", retval
            );
            exit(1);
        } else {
            vf.need_update = 0;
            vf.update();
        }
    }
    return found;
}

// this host is declared dead; deal with the loss of data
//
int handle_host(DB_HOST& h) {
    DB_VDA_CHUNK_HOST ch;
    char buf[256];
    int retval;

    log_messages.printf(MSG_NORMAL, "processing dead host %d\n", h.id);

    sprintf(buf, "where host_id=%d", h.id);
    while (1) {
        retval = ch.enumerate(buf);
        if (retval == ERR_DB_NOT_FOUND) break;
        if (retval) return retval;
        log_messages.printf(MSG_NORMAL, "   updating file%d\n", ch.vda_file_id);
        DB_VDA_FILE vf;
        retval = vf.lookup_id(ch.vda_file_id);
        if (retval) {
            log_messages.printf(MSG_CRITICAL,
                "   file lookup failed%d\n", ch.vda_file_id
            );
            return retval;
        }
        retval = vf.update_field("need_update=1");
        if (retval) {
            log_messages.printf(MSG_CRITICAL,
                "   file update failed%d\n", ch.vda_file_id
            );
            return retval;
        }
    }
    return 0;
}

// handle timed-out hosts
//
bool scan_hosts() {
    DB_HOST h;
    char buf[256];
    int retval;
    bool found = false;

    sprintf(buf, "where cpu_efficiency < %f", dtime());
    while (1) {
        retval = h.enumerate(buf);
        if (retval == ERR_DB_NOT_FOUND) break;
        if (retval) {
            log_messages.printf(MSG_CRITICAL, "host.enumerate() failed\n");
            exit(1);
        }
        found = true;
        retval = handle_host(h);
        if (retval) {
            log_messages.printf(MSG_CRITICAL, "handle_host() failed: %d\n", retval);
            exit(1);
        }
        retval = h.update_field("cpu_efficiency=1e12");
        if (retval) {
            log_messages.printf(MSG_CRITICAL, "h.update_field() failed: %d\n", retval);
            exit(1);
        }

    }
    return found;
}

int main(int argc, char** argv) {
    for (int i=1; i<argc; i++) {
        if (!strcmp(argv[i], "-d") || !strcmp(argv[i], "--debug_level")) {
            int dl = atoi(argv[++i]);
            log_messages.set_debug_level(dl);
            if (dl == 4) g_print_queries = true;
        }
    }
    int retval = config.parse_file();
    if (retval) {
        log_messages.printf(MSG_CRITICAL, "can't parse config file\n");
        exit(1);
    }
    retval = boinc_db.open(
        config.db_name, config.db_host, config.db_user, config.db_passwd
    );
    if (retval) {
        log_messages.printf(MSG_CRITICAL, "can't open DB\n");
        exit(1);
    }

#if 0
    VDA_FILE_AUX vf;
    memset(&vf, 0, sizeof(vf));
    strcpy(vf.dir, "/mydisks/b/users/boincadm/vda_test");
    strcpy(vf.name, "file.ext");
    handle_file(vf);
    exit(0);
#endif
    while(1) {
        bool action = scan_files();
        action |= scan_hosts();
        if (!action) boinc_sleep(5.);
    }
}
