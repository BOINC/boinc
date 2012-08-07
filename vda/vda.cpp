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

// cmdline interface for VDA (volunteer data archival)
//
// usages:
// vda add path     add a file

#include <stdio.h>
#include <unistd.h>
#include <set>

#include "boinc_db.h"
#include "filesys.h"
#include "sched_config.h"
#include "sched_util.h"
#include "util.h"

#include "vda_lib.h"

using std::set;

void usage() {
    fprintf(stderr, "Usage: vda [add|remove|retrieve|status] path\n");
    exit(1);
}

void show_msg(char* msg) {
    printf("%s", msg);
}

inline const char* indent(int n) {
    static const char *buf = "                                            ";
    static int len = strlen(buf);
    return buf + len - n*3;
}

void META_CHUNK::print_status(int level) {
    printf("%smeta-chunk %s\n", indent(level), name);
    for (unsigned int i=0; i<children.size(); i++) {
        if (bottom_level) {
            CHUNK* c = (CHUNK*)children[i];
            c->print_status(level+1);
        } else {
            META_CHUNK* mc = (META_CHUNK*)children[i];
            mc->print_status(level+1);
        }
    }
}

void CHUNK::print_status(int level) {
    printf("%schunk %s\n", indent(level), name);
    set<VDA_CHUNK_HOST*>::iterator i;
    for (i=hosts.begin(); i!=hosts.end(); i++) {
        VDA_CHUNK_HOST* ch = *i;
        ch->print_status(level+1);
    }
}

void VDA_CHUNK_HOST::print_status(int level) {
    printf("%shost %d\n", indent(level), host_id);
    printf("%spresent on host: %s\n",
        indent(level), present_on_host?"yes":"no"
    );
    printf("%stransfer in progress: %s\n",
        indent(level), transfer_in_progress?"yes":"no"
    );
    printf("%stransfer wait: %s\n",
        indent(level), transfer_wait?"yes":"no"
    );
    if (transfer_request_time) {
        printf("%stransfer request time: %s\n",
            indent(level), time_to_string(transfer_request_time)
        );
    }
    if (transfer_send_time) {
        printf("%stransfer send time: %s\n",
            indent(level), time_to_string(transfer_send_time)
        );
    }
}

int handle_add(const char* path) {
    char dir[256], filename[256], buf[1024];
    DB_VDA_FILE vf;
    POLICY policy;
    double size;
    int retval;

    retval = file_size(path, size);
    if (retval) {
        fprintf(stderr, "no file %s\n", path);
        return -1;
    }

    strcpy(dir, path);
    char* p = strrchr(dir, '/');
    *p = 0;
    strcpy(filename, p+1);

    // make sure there's a valid policy file in the dir
    //
    sprintf(buf, "%s/boinc_meta.txt", dir);
    retval = policy.parse(buf);
    if (retval) {
        fprintf(stderr, "Can't parse policy file.\n");
        return -1;
    }

    // add a DB record and mark it for update
    //
    vf.create_time = dtime();
    strcpy(vf.dir, dir);
    strcpy(vf.file_name, filename);
    vf.size = size;
    vf.chunk_size = 0;  // don't know this yet; set by vdad
    vf.need_update = 1;
    vf.initialized = 0;
    vf.retrieving = 0;
    retval = vf.insert();
    if (retval) {
        fprintf(stderr, "Can't insert DB record\n");
        return -1;
    }
    return 0;
}

int handle_remove(const char* name) {
    DB_VDA_FILE vf;
    char buf[1024];
    sprintf(buf, "where file_name='%s'", name);
    int retval = vf.lookup(buf);
    if (retval) return retval;

    // delete DB records
    //
    DB_VDA_CHUNK_HOST ch;
    sprintf(buf, "vda_file_id=%d", vf.id);
    ch.delete_from_db_multi(buf);
    vf.delete_from_db();

    // remove symlink from download hier
    //
    dir_hier_path(name, config.download_dir, config.uldl_dir_fanout, buf);
    unlink(buf);

    // remove encoded data and directories
    //
    retval = chdir(vf.dir);
    if (retval) perror("chdir");
    retval = system("/bin/rm -r [0-9]* Coding data.vda chunk_sizes.txt");
    if (retval) perror("system");
    return 0;
}

int handle_retrieve(const char* name) {
    DB_VDA_FILE vf;
    char buf[1024];
    sprintf(buf, "where file_name='%s'", name);
    int retval = vf.lookup(buf);
    if (retval) return retval;
    retval = vf.update_field("retrieving=1");
    return retval;
}

int handle_status(const char* name) {
    DB_VDA_FILE dvf;
    char buf[1024];
    sprintf(buf, "where file_name='%s'", name);
    int retval = dvf.lookup(buf);
    if (retval) return retval;

    VDA_FILE_AUX vf = dvf;
    sprintf(buf, "%s/boinc_meta.txt", vf.dir);
    retval = vf.policy.parse(buf);
    if (retval) {
        log_messages.printf(MSG_CRITICAL, "Can't parse policy file %s\n", buf);
        return retval;
    }
    retval = vf.get_state();
    if (retval) {
        log_messages.printf(MSG_CRITICAL, "Can't get file state: %d\n", retval);
        return retval;
    }
    printf("status for file %s:", vf.file_name);
    vf.meta_chunk->print_status(0);

    return 0;
}

int main(int argc, char** argv) {
    int retval = config.parse_file();
    if (retval) {
        fprintf(stderr, "can't parse config file\n");
        exit(1);
    }
    retval = boinc_db.open(
        config.db_name, config.db_host, config.db_user, config.db_passwd
    );
    if (retval) {
        fprintf(stderr, "can't open DB\n");
        exit(1);
    }
    for (int i=1; i<argc; i++) {
        if (!strcmp(argv[i], "add")) {
            if (argc != 3) usage();
            retval = handle_add(argv[++i]);
            if (retval) {
                fprintf(stderr, "error %d: %s\n", retval, boincerror(retval));
            } else {
                printf("file added successfully\n");
            }
            exit(retval);
        }
        if (!strcmp(argv[i], "remove")) {
            if (argc != 3) usage();
            retval = handle_remove(argv[++i]);
            if (retval) {
                fprintf(stderr, "error %d: %s\n", retval, boincerror(retval));
            } else {
                printf("file removed successfully\n");
            }
            exit(retval);
        }
        if (!strcmp(argv[i], "retrieve")) {
            if (argc != 3) usage();
            retval = handle_retrieve(argv[++i]);
            if (retval) {
                fprintf(stderr, "error %d: %s\n", retval, boincerror(retval));
            } else {
                printf("file retrieval started\n");
            }
            exit(retval);
        }
        if (!strcmp(argv[i], "status")) {
            if (argc != 3) usage();
            retval = handle_status(argv[++i]);
            if (retval) {
                fprintf(stderr, "error %d: %s\n", retval, boincerror(retval));
            }
            exit(retval);
        }
        usage();
    }
    usage();
}
