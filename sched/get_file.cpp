// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2008 University of California
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

// upload a file from a host
//
// get_file [options]
// --host_id N              ID of host to upload from
// --file_name name         file name
// [ --url x ]              URL of upload server (can specify several)
// [ --max_latency x ]      max latency, seconds (default 1 week)
// [ --max_nbytes x ]       max file size (default 1 GB)
//
// Run from the project root dir.

#include "config.h"

#if HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <sys/param.h>
#include <stdlib.h>
#include <string>
#include <time.h>

#include "backend_lib.h"

#include "sched_config.h"
#include "sched_util.h"
#include "svn_version.h"

void usage() {
    fprintf(stderr, "Gets a file from a host.\n"
        "Usage: get_file [options]\n\n"
        "  --host_id id           host DB ID\n"
        "  --file_name name       Name of file\n"
        "  [-- url X]             URL of upload server\n"
        "  [ -v | --version ]     Show version\n"
        "  [ -h | --help ]        Show help\n"
    );
}

int main(int argc, char** argv) {
    int i, retval;
    char file_name[256];
    int host_id;
    vector<const char*> urls;
    double max_latency = 7*86400;
    double max_nbytes = 1e9;

    strcpy(file_name, "");
    host_id = 0;

    check_stop_daemons();

    for (i=1; i<argc; i++) {
        if (is_arg(argv[i], "host_id")) {
            if (!argv[++i]) {
                usage();
                exit(1);
            }
            host_id = atoi(argv[i]);
        } else if (is_arg(argv[i], "file_name")) {
            if (!argv[++i]) {
                usage();
                exit(1);
            }
            strcpy(file_name, argv[i]);
        } else if (is_arg(argv[i], "h") || is_arg(argv[i], "help")) {
            usage();
            exit(0);
        } else if (is_arg(argv[i], "v") || is_arg(argv[i], "version")) {
            printf("%s\n", SVN_VERSION);
            exit(0);
        } else if (is_arg(argv[i], "url")) {
            urls.push_back(argv[++i]);
        } else if (is_arg(argv[i], "max_latency")) {
            max_latency = atof(argv[++i]);
        } else if (is_arg(argv[i], "max_nbytes")) {
            max_nbytes = atof(argv[++i]);
        } else {
            usage();
            exit(1);
        }
    }

    if (!strlen(file_name) || host_id == 0) {
        usage();
        exit(1);
    }

    retval = config.parse_file();
    if (retval) {
        fprintf(stderr, "Can't parse config.xml: %s\n", boincerror(retval));
        exit(1);
    }

    retval = boinc_db.open(
        config.db_name, config.db_host, config.db_user, config.db_passwd
    );
    if (retval) {
        fprintf(stderr, "boinc_db.open failed: %s\n", boincerror(retval));
        exit(1);
    }

    if (urls.size() == 0) {
        urls.push_back(config.upload_url);
    }

    R_RSA_PRIVATE_KEY key;
    bool generate_upload_certificate = !config.dont_generate_upload_certificates;
    if (generate_upload_certificate) {
        char keypath[MAXPATHLEN];
        sprintf(keypath, "%s/upload_private", config.key_dir);
        retval = read_key_file(keypath, key);
        if (retval) {
            fprintf(stderr, "can't read key\n");
            exit(1);
        }
    }

    retval = create_get_file_msg(
        host_id, file_name, urls, max_nbytes,
        dtime() + max_latency,
        generate_upload_certificate, key
    );

    if (retval) {
        fprintf(stderr, "get_file() failed: %s\n", boincerror(retval));
    }
    boinc_db.close();
}

const char *BOINC_RCSID_37238a0141 = "$Id$";
