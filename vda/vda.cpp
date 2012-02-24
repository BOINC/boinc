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

#include "boinc_db.h"
#include "filesys.h"
#include "sched_config.h"
#include "util.h"

#include "vda_lib.h"

void usage() {
    fprintf(stderr, "Usage: vda {add} path\n");
    exit(1);
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

    // make sure there's a policy file in the dir
    //
    sprintf(buf, "%s/boinc_meta.txt", dir);
    retval = policy.parse(buf);
    if (retval) {
        fprintf(stderr, "Can't parse policy file.\n");
        return -1;
    }

    // add a DB record and schedule it for updating
    //
    vf.create_time = dtime();
    strcpy(vf.dir, dir);
    strcpy(vf.name, filename);
    vf.size = size;
    vf.need_update = 1;
    vf.initialized = 0;
    retval = vf.insert();
    if (retval) {
        fprintf(stderr, "Can't insert DB record\n");
        return -1;
    }
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
                fprintf(stderr, "error %d\n", retval);
            } else {
                printf("file added successfully\n");
            }
            exit(retval);
        }
        usage();
    }
    usage();
}
